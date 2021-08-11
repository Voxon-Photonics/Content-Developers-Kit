#if 0 //To compile, type: nmake poly2vox.c

!if 1

	#setvc6:
poly2vox.exe: poly2vox.obj kplib.obj poly2vox.c;
			link poly2vox.obj kplib.obj /opt:nowin98 user32.lib
	del poly2vox.obj
poly2vox.obj: poly2vox.c; cl /c /TP poly2vox.c /Ox /GFy /MD
kplib.obj:    kplib.c;    cl /c /TP kplib.c    /Ox /GFy /MD

!else

	#setvc9_64:
poly2vox.exe: poly2vox.obj kplib.obj poly2vox.c;
			link poly2vox.obj kplib.obj
	del poly2vox.obj
poly2vox.obj: poly2vox.c; cl /c /TP poly2vox.c /Ox /GFy /MT
kplib.obj:    kplib.c;    cl /c /TP kplib.c    /Ox /GFy /MT

!endif

!if 0
#endif

#if 0

POLY2VOX.C by Ken Silverman (http://advsys.net/ken)

License for this code:
	* No commercial exploitation please
	* Do not remove my name or credit
	* You may distribute modified code/executables but please make it clear that it is modified

#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <emmintrin.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#define MAX_PATH 260

#define PRINTALLOC 0
#define PRINTTIME 1

#if !defined(max)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef _MSC_VER
#include <basetsd.h>
#endif
#ifdef __GNUC__
#include <stdint.h>
#define INT_PTR intptr_t
#define UINT_PTR uintptr_t
#endif

	//High-level (easy) picture loading function:
extern void kpzload (const char *, INT_PTR *, int *, int *, int *);
	//ZIP functions:
extern int kzaddstack (const char *);
extern void kzuninit (void);
extern INT_PTR kzopen (const char *);
extern int kzread (void *, int);
extern int kzfilelength (void);
extern int kzseek (int, int);
extern int kztell (void);
extern int kzgetc (void);
extern int kzeof (void);
extern void kzclose (void);
extern void kzfindfilestart (const char *);
extern int kzfindfile (char *);

#define VSID 1024   //hard-coded VXL x&y dimensions
#define MAXZDIM 256 //hard-coded VXL z dimension

#define VOXMODE 1  //0=uncompressed bit buffer (LMAXDIM+LMAYDIM+LMAZDIM < ~30), 1=Voxlap style RLE (more scalable)

#define LMAXDIM 12
#define LMAYDIM 12
#define LMAZDIM 12
#define MAXDIM (1<<LMAXDIM)
#define MAYDIM (1<<LMAYDIM)
#define MAZDIM (1<<LMAZDIM)
//#define KMOD32(a) (a)
#define KMOD32(a) ((a)&31)

typedef struct { int x, y; } lpoint2d;
typedef struct { int x, y, z; } ipoint3d;
typedef struct { unsigned short x, y, z, dum; int prev, itri; } hashdat_t;
typedef struct { int x, y, z, i; } trityp;
typedef struct { INT_PTR f; int p, x, y; int namoff; } pictyp;
typedef struct { union { struct { float x, y, z; }; float a[3]; }; } point3d;
typedef struct { union { struct { float x, y, z; }; float a[3]; }; float u, v; } vertyp;

	//Starting maximums: bytes per unit:
static int maxobjects =  128; // 8 (    1KB)
static int maxnamsiz =  4096; // 1 (    4KB)
static int maxmat =      512; // 8 (    4KB)
static int maxpics =     256; //20 (    5KB)
static int maxtris =   32768; //16 (  512KB)
static int maxverts =  32768; //20 (  640KB)
static int maxhash = 1048576; //12 (12288KB)

	//Dynamic arrays:
static int *trioff = 0, *vertoff = 0;
static vertyp *vert = 0;
static trityp *tri = 0;
static pictyp *pic = 0;
static char *nambuf = 0; //NOTE:used for both mat&pic names
static int *matstr = 0;
static int *colrgb = 0;
static hashdat_t *hashdat = 0;

static int numobjects = 0, numverts = 0, numtris = 0, numpics = 0, namplc = 0, matnum;

	//>poly2vox 3ds\\canyon.3ds canyon.vxl
	//LHASHEAD: load&rend:    all:    hashead:
	//   15:   102.620e9cc 233.270e9cc 128KB
	//   16:    47.191e9cc             256KB
	//   17:    22.054e9cc             512KB
	//   18:    13.630e9cc  42.543e9cc   1MB
	//   19:    11.138e9cc               2MB
	//   20:     9.386e9cc  24.268e9cc   4MB
	//   21:     9.508e9cc               8MB
	//   22:     8.633e9cc              16MB
	//   23:     8.658e9cc              32MB
#define LHASHEAD 20
static int hashead[1<<LHASHEAD], hashcnt = 0;

#define MAXCSIZ 1028
static char tempbuf[max(4096,MAXCSIZ)];

static int xsiz, ysiz, zsiz, rendermode = 0, gtexfilter = 4, savetype, gotzip = 0;
static float lastx[MAYDIM], xpiv, ypiv, zpiv;
static point3d gclipmin = {-1e32,-1e32,-1e32}, gclipmax = {+1e32,+1e32,+1e32};
static int gclipx0, gclipy0, gclipz0, gclipx1, gclipy1, gclipz1;

static char palfilnam[MAX_PATH] = "", texfilnam[MAX_PATH] = "";

static int min0[MAYDIM], max0[MAYDIM]; //MAXY
static int min1[MAXDIM], max1[MAXDIM]; //MAXX
static int min2[MAYDIM], max2[MAYDIM]; //MAXY

static int xstart[MAXDIM+1];
static unsigned short xyoffs[MAXDIM][MAYDIM+1]; //2MB

static void vox_free(void);

static void quitout (char *str)
{
	if (str) puts(str);

	if (hashdat) free(hashdat);
	if (colrgb) free(colrgb);
	if (matstr) free(matstr);
	if (nambuf) free(nambuf);
	if (pic) free(pic);
	if (tri) free(tri);
	if (vert) free(vert);
	if (vertoff) free(vertoff);
	if (trioff) free(trioff);
	vox_free();

	exit(0);
}

#define cvtss2si(f) _mm_cvt_ss2si(_mm_set_ss(f))
#define cvttss2si(f) _mm_cvtt_ss2si(_mm_set_ss(f))

static _inline void fcossin (float a, float *c, float *s) { (*c) = cos(a); (*s) = sin(a); }

#define GETHASH(x,y,z) (((x+y)*(x-y)+z) & ((1<<LHASHEAD)-1))
static int ghashofx = 0, ghashofy = 0, ghashofz = 0;
static void hash_init (void) { memset(hashead,-1,sizeof(hashead)); hashcnt = 0; }
static int hash_get (int x, int y, int z)
{
	__int64 q;
	int i, hashval;

	hashval = GETHASH(x,y,z);
	q = (__int64)((y<<16)+x) + (((_int64)z)<<32);
	for(i=hashead[hashval];i>=0;i=hashdat[i].prev)
		if (*(__int64 *)&hashdat[i].x == q) return(hashdat[i].itri);
	return(-1);
}
static void hash_set (int x, int y, int z, int v)
{
	__int64 q;
	int i, hashval;

	hashval = GETHASH(x,y,z);
	q = (__int64)((y<<16)+x) + (((_int64)z)<<32);
	for(i=hashead[hashval];i>=0;i=hashdat[i].prev)
		if (*(__int64 *)&hashdat[i].x == q) { hashdat[i].itri = v; return; }
	if (hashcnt >= maxhash)
	{
		maxhash += (maxhash>>3);
#if (PRINTALLOC == 1)
		printf("maxhash=%d (%dMB)\n",maxhash,(maxhash*12)>>20);
#endif
		if (!(hashdat = (hashdat_t *)realloc(hashdat,sizeof(hashdat_t)*maxhash))) quitout("realloc failed: hashdat");
	}
	hashdat[hashcnt].prev = hashead[hashval]; hashead[hashval] = hashcnt;
	*(__int64 *)&hashdat[hashcnt].x = q;
	hashdat[hashcnt].itri = v;
	hashcnt++;
}

static int picsloaded = 0;
static int palfreq[64][64][64];
static unsigned char rpal[256], gpal[256], bpal[256];
static int rdist[129], gdist[129], bdist[129];
static unsigned char closestcol[64][64][64];
static unsigned char getclosestcol (int c32, int searchnum)
{
	int i, r, g, b, dist, mindist, *rlookup, *glookup, *blookup;
	unsigned char col, bestcol;

	if (!picsloaded) return(c32);

	r = ((c32>>16)&255); g = ((c32>>8)&255); b = (c32&255);
	col = closestcol[r>>2][g>>2][b>>2]; if (col != 255) return(col);

	rlookup = (int *)&rdist[64-(r>>2)];
	glookup = (int *)&gdist[64-(g>>2)];
	blookup = (int *)&bdist[64-(b>>2)];

	mindist = 0x7fffffff;
	for(i=0;i<searchnum;i++)
	{
		dist  = glookup[gpal[i]>>2]; if (dist >= mindist) continue;
		dist += rlookup[rpal[i]>>2]; if (dist >= mindist) continue;
		dist += blookup[bpal[i]>>2]; if (dist >= mindist) continue;
		mindist = dist; bestcol = (unsigned char)i;
	}
	closestcol[r>>2][g>>2][b>>2] = bestcol;
	return(bestcol);
}

static _inline int mulshr9 (int a, int d) { return((int)(((__int64)a*d)>>9)); }

static void genpal (void)
{
	unsigned short *s;
	int i, j, c, r, g, b, p, v, x, y, z, xx, yy, zz, xxx, yyy, zzz, *dc;

	j = 0;
	for(i=64;i>=0;i--)
	{
		//j = (i-64)*(i-64);
		rdist[i] = rdist[128-i] = j*30;
		gdist[i] = gdist[128-i] = j*59;
		bdist[i] = bdist[128-i] = j*11;
		j += 129-(i<<1);
	}

	memset(closestcol,-1,sizeof(closestcol));
	c = (rand()<<17);
	for(i=0;i<256;i++)
	{
		if (!i)
			{ rpal[i] = gpal[i] = bpal[i] = 0; }
		else
		{
			v = 0;
			for(j=1024;j>0;j--)
			{
				c = (c*27584621)+1;
				dc = &palfreq[0][0][((unsigned)c)>>14];
				p = dc[0];
				r = ((c>>26)&63); if (r !=  0) p += dc[-64*64];
										if (r != 63) p += dc[+64*64];
				g = ((c>>20)&63); if (g !=  0) p += dc[-   64];
										if (g != 63) p += dc[+   64];
				b = ((c>>14)&63); if (b !=  0) p += dc[-    1];
										if (b != 63) p += dc[+    1];
				if (p > v) { v = p; rpal[i] = (r<<2); gpal[i] = (g<<2); bpal[i] = (b<<2); }
			}
		}

		xx = max((rpal[i]>>2)-2,0); xxx = min((rpal[i]>>2)+2,63);
		yy = max((gpal[i]>>2)-2,0); yyy = min((gpal[i]>>2)+2,63);
		zz = max((bpal[i]>>2)-2,0); zzz = min((bpal[i]>>2)+2,63);
		for(g=yy;g<=yyy;g++)
		{
			y = gdist[(gpal[i]>>2)-g+64]; if (y >= 512) continue;
			for(r=xx;r<=xxx;r++)
			{
				x = rdist[(rpal[i]>>2)-r+64]+y; if (x >= 512) continue;
				for(b=zz;b<=zzz;b++)
				{
					z = bdist[(bpal[i]>>2)-b+64]+x; if (z >= 512) continue;
					palfreq[r][g][b] = mulshr9(palfreq[r][g][b],z);
				}
			}
		}
	}
}

	//Fancy interpolation filter for 4x4 matrix (256 samples, 4 coefficients)
	//NOTE: this is higher quality than bilinear filter (which uses 2x2 matrix)
static short lut4x4[256*4];
static int lut4x4inited = 0;
static float calclut4x4 (float x)
{
	x = fabs(x); //4x4 filter (spline16)
	if (x < 1) return(((x-1.8)*x - 0.2)*x + 1.0);
	x -= 1.0; return(((x*(-1.0/3.0) + 0.8)*x - (7.0/15.0))*x);
}
static void initlut4x4 (void)
{
	int i, k;
	float f;

	lut4x4inited = 1;
	for(i=0;i<256;i++)
	{
		f = ((float)i) / 256.0;
		lut4x4[i*4+3] = cvtss2si(calclut4x4(f-2.0)*-32768.0);
		lut4x4[i*4+2] = cvtss2si(calclut4x4(f-1.0)*-32768.0);
		lut4x4[i*4+1] = cvtss2si(calclut4x4(f+0.0)*-32768.0);
		lut4x4[i*4+0] = cvtss2si(calclut4x4(f+1.0)*-32768.0);
	}
}

static int getpix32 (int x, int y, int z)
{
	float fx, fy, fz, fx1, fy1, fz1, fx2, fy2, fz2, fu, fv;
	float f, nx, ny, nz, ux, uy, uz, vx, vy, vz;
	int i, j, k, u, v, tu, tv, uu, vv, wu, wv, r, g, b, rr, gg, bb;

	x += ghashofx; y += ghashofy; z += ghashofz;
	i = hash_get(x,y,z); if (i < 0) return(0x808080);
	u = tri[i].i; j = ~colrgb[u];
	if ((colrgb[u] >= 0) || (!pic[j].f))
	{
		if ((!picsloaded) && (savetype < 2)) return(u);
		return(colrgb[u]); //solid color
	}

	if ((pic[j].x <= 0) || (pic[j].y <= 0)) return(0);
	fx = vert[tri[i].x].x; fx1 = vert[tri[i].y].x-fx; fx2 = vert[tri[i].z].x-fx;
	fy = vert[tri[i].x].y; fy1 = vert[tri[i].y].y-fy; fy2 = vert[tri[i].z].y-fy;
	fz = vert[tri[i].x].z; fz1 = vert[tri[i].y].z-fz; fz2 = vert[tri[i].z].z-fz;
	nx = fy1*fz2 - fz1*fy2;
	ny = fz1*fx2 - fx1*fz2;
	nz = fx1*fy2 - fy1*fx2;

		//Cramer's rule derivation:
		//u(&vert[tri[i].x]) = 0.0, v(&vert[tri[i].x]) = 0.0
		//u(&vert[tri[i].y]) = 1.0, v(&vert[tri[i].y]) = 0.0
		//u(&vert[tri[i].z]) = 0.0, v(&vert[tri[i].z]) = 1.0
		//fx1*ux + fy1*uy + fz1*uz = 1    fx1*vx + fy1*vy + fz1*vz = 0
		//fx2*ux + fy2*uy + fz2*uz = 0    fx2*vx + fy2*vy + fz2*vz = 1
		// nx*ux +  ny*uy +  nz*uz = 0     nx*vx +  ny*vy +  nz*vz = 0
	f = fx1*(fy2*nz-fz2*ny) + fy1*(fz2*nx-fx2*nz) + fz1*(fx2*ny-fy2*nx);
	if (f != 0)
	{
		f = 1.0 / f;
		ux = (fy2*nz-fz2*ny)*f; uy = (fz2*nx-fx2*nz)*f; uz = (fx2*ny-fy2*nx)*f;
		vx = (fz1*ny-fy1*nz)*f; vy = (fx1*nz-fz1*nx)*f; vz = (fy1*nx-fx1*ny)*f;
	} else { ux = uy = uz = vx = vy = vz = 0; }
	fu = ((float)x-fx)*ux + ((float)y-fy)*uy + ((float)z-fz)*uz;
	fv = ((float)x-fx)*vx + ((float)y-fy)*vy + ((float)z-fz)*vz;
	fx = (vert[tri[i].y].u-vert[tri[i].x].u)*fu +
		  (vert[tri[i].z].u-vert[tri[i].x].u)*fv + vert[tri[i].x].u;
	fy = (vert[tri[i].y].v-vert[tri[i].x].v)*fu +
		  (vert[tri[i].z].v-vert[tri[i].x].v)*fv + vert[tri[i].x].v;

	if (gtexfilter != 4)
	{
			//Nearest texture filter
		u = cvtss2si(((float)pic[j].x)*fx);
		v = cvtss2si(((float)pic[j].y)*fy);
		if (u < 0) u += ((pic[j].x-1-u)/pic[j].x)*pic[j].x; else u %= pic[j].x;
		if (v < 0) v += ((pic[j].y-1-v)/pic[j].y)*pic[j].y; else v %= pic[j].y;
		j = *(int *)((pic[j].y-1-v)*pic[j].p + (u<<2) + pic[j].f);
	}
	else
	{
			//Fancy 4x4 texture filter
		u = cvtss2si(((float)(pic[j].x<<8))*fx); u -= 384;
		v = cvtss2si(((float)(pic[j].y<<8))*fy); v += 384;
		if (u < 0) u += (((pic[j].x<<8)-1-u)/(pic[j].x<<8))*(pic[j].x<<8); else u %= (pic[j].x<<8);
		if (v < 0) v += (((pic[j].y<<8)-1-v)/(pic[j].y<<8))*(pic[j].y<<8); else v %= (pic[j].y<<8);
		v = (pic[j].y<<8)-1-v;

		if (!lut4x4inited) initlut4x4();
		r = g = b = 0;
		for(vv=0,tv=(v>>8);vv<4;vv++)
		{
			rr = gg = bb = 0;
			for(uu=0,tu=(u>>8);uu<4;uu++)
			{
				k = (*(int *)(tv*pic[j].p + (tu<<2) + pic[j].f));
				wu = lut4x4[(u&255)*4+uu];
				rr += ((((k>>16)&255)*wu)>>8);
				gg += ((((k>> 8)&255)*wu)>>8);
				bb += ((( k     &255)*wu)>>8);
				tu++; if (tu >= pic[j].x) tu = 0;
			}
			wv = lut4x4[(v&255)*4+vv];
			r += ((rr*wv)>>8);
			g += ((gg*wv)>>8);
			b += ((bb*wv)>>8);
			tv++; if (tv >= pic[j].y) tv = 0;
		}
		r = min(max(r>>14,0),255);
		g = min(max(g>>14,0),255);
		b = min(max(b>>14,0),255);
		j = (r<<16)+(g<<8)+b;
	}
	return(j);
}

//EQUIVEC code begins -----------------------------------------------------

#define PI 3.141592653589793
#define GOLDRAT 0.3819660112501052 //Golden Ratio: 1 - 1/((sqrt(5)+1)/2)
typedef struct
{
	float fibx[45], fiby[45];
	float azval[20], zmulk, zaddk;
	int fib[47], aztop, npoints;
	point3d *p;  //For memory version :/
	int pcur;
} equivectyp;
static equivectyp equivec;

static _inline int dmulshr0 (int a, int d, int s, int t) { return(a*d + s*t); }

static void equiind2vec (int i, float *x, float *y, float *z)
{
	float r;
	(*z) = (float)i*equivec.zmulk + equivec.zaddk; r = sqrt(1.f - (*z)*(*z));
	fcossin((float)i*(GOLDRAT*PI*2),x,y); (*x) *= r; (*y) *= r;
}

	//(Pass n=0 to free buffer)
static int equimemset (int n)
{
	int z;

	if (equivec.pcur == n) return(1); //Don't recalculate if same #
	if (equivec.p) { free(equivec.p); equivec.p = 0; }
	if (!n) return(1);

		//Init vector positions (equivec.p) for memory version
	if (!(equivec.p = (point3d *)malloc(((n+1)&~1)*sizeof(point3d))))
		return(0);

	equivec.pcur = n;
	equivec.zmulk = 2 / (float)n; equivec.zaddk = equivec.zmulk*.5 - 1.0;
	for(z=n-1;z>=0;z--)
		equiind2vec(z,&equivec.p[z].x,&equivec.p[z].y,&equivec.p[z].z);
	if (n&1) //Hack for when n=255 and want a <0,0,0> vector
		{ equivec.p[n].x = equivec.p[n].y = equivec.p[n].z = 0; }
	return(1);
}

	//Very fast; good quality, requires equivec.p[] :/
static int equivec2indmem (float x, float y, float z)
{
	int b, i, j, k, bestc;
	float xy, zz, md, d;

	xy = atan2(y,x); //atan2 is 150 clock cycles!
	j = ((*(int *)&z)&0x7fffffff);
	bestc = equivec.aztop;
	do
	{
		if (j < *(int *)&equivec.azval[bestc]) break;
		bestc--;
	} while (bestc);

	zz = z + 1.f;
	i = cvtss2si(equivec.fibx[bestc]*xy + equivec.fiby[bestc]*zz - .5);
	bestc++;
	j = cvtss2si(equivec.fibx[bestc]*xy + equivec.fiby[bestc]*zz - .5);

	k = dmulshr0(equivec.fib[bestc+2],i,equivec.fib[bestc+1],j);
	if ((unsigned int)k < equivec.npoints)
	{
		md = equivec.p[k].x*x + equivec.p[k].y*y + equivec.p[k].z*z;
		j = k;
	} else md = -2.f;
	b = bestc+3;
	do
	{
		i = equivec.fib[b] + k;
		if ((unsigned int)i < equivec.npoints)
		{
			d = equivec.p[i].x*x + equivec.p[i].y*y + equivec.p[i].z*z;
			if (*(int *)&d > *(int *)&md) { md = d; j = i; }
		}
		b--;
	} while (b != bestc);
	return(j);
}

static void equivecinit (int n)
{
	float t0, t1;
	int z;

		//Init constants for ind2vec
	equivec.npoints = n;
	equivec.zmulk = 2 / (float)n; equivec.zaddk = equivec.zmulk*.5 - 1.0;

	equimemset(n);

		//Init Fibonacci table
	equivec.fib[0] = 0; equivec.fib[1] = 1;
	for(z=2;z<47;z++) equivec.fib[z] = equivec.fib[z-2]+equivec.fib[z-1];

		//Init fibx/y LUT
	t0 = .5 / PI; t1 = (float)n * -.5;
	for(z=0;z<45;z++)
	{
		t0 = -t0; equivec.fibx[z] = (float)equivec.fib[z+2]*t0;
		t1 = -t1; equivec.fiby[z] = ((float)equivec.fib[z+2]*GOLDRAT - (float)equivec.fib[z])*t1;
	}

	t0 = 1 / ((float)n * PI);
	for(equivec.aztop=0;equivec.aztop<20;equivec.aztop++)
	{
		t1 = 1 - (float)equivec.fib[(equivec.aztop<<1)+6]*t0; if (t1 < 0) break;
		equivec.azval[equivec.aztop+1] = sqrt(t1);
	}
}

static void equivecuninit (void) { equimemset(0); }

//EQUIVEC code ends -------------------------------------------------------

static int getnorm8 (int x, int y, int z)
{
	float fx, fy, fz, fx1, fy1, fz1, fx2, fy2, fz2, f;
	int i;

	x += ghashofx; y += ghashofy; z += ghashofz;
	i = hash_get(x,y,z); if (i < 0) return(0);

	fx = vert[tri[i].x].x; fx1 = vert[tri[i].y].x-fx; fx2 = vert[tri[i].z].x-fx;
	fy = vert[tri[i].x].y; fy1 = vert[tri[i].y].y-fy; fy2 = vert[tri[i].z].y-fy;
	fz = vert[tri[i].x].z; fz1 = vert[tri[i].y].z-fz; fz2 = vert[tri[i].z].z-fz;
	fx = fz1*fy2 - fy1*fz2; //Note: inverted from normal!
	fy = fx1*fz2 - fz1*fx2;
	fz = fy1*fx2 - fx1*fy2;
	f = sqrt(fx*fx + fy*fy + fz*fz); if (f > 0) f = 1.0 / f;
		//NOTE: don't use rsqrtss: sqrt must be accurate here
	return(equivec2indmem(fx*f,fy*f,fz*f));
}

//FLOODFILL2D/3D begins -----------------------------------------------------

#define FILLBUFSIZ 65536 //Use realloc instead!
typedef struct { unsigned short x, y, z0, z1; } cpoint4d; //512K
static cpoint4d fbuf[FILLBUFSIZ];

#ifdef _MSC_VER
#ifndef _WIN64
static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }
#else
static __forceinline unsigned int bsf (unsigned int a) { unsigned long r; _BitScanForward(&r,a); return(r); }
static __forceinline unsigned int bsr (unsigned int a) { unsigned long r; _BitScanReverse(&r,a); return(r); }
#endif
#else
#define bsf(r) ({ long __r=(r); __asm__ __volatile__ ("bsf %0, %0;": "=a" (__r): "a" (__r): "cc"); __r; })
#define bsr(r) ({ long __r=(r); __asm__ __volatile__ ("bsr %0, %0;": "=a" (__r): "a" (__r): "cc"); __r; })
#endif

static __forceinline int dntil0 (int *iptr, int z, int zmax)
{
	//   //This line does the same thing (but slow & brute force)
	//while ((z < zmax) && (iptr[z>>5]&(1<<KMOD32(z)))) z++; return(z);
	int i;
		//WARNING: zmax must be multiple of 32!
	i = (iptr[z>>5]|((1<<KMOD32(z))-1)); z &= ~31;
	while (i == -1)
	{
		z += 32; if (z >= zmax) return(zmax);
		i = iptr[z>>5];
	}
	return(bsf(~i)+z);
}

static __forceinline int dntil1 (int *iptr, int z, int zmax)
{
	//   //This line does the same thing (but slow & brute force)
	//while ((z < zmax) && (!(iptr[z>>5]&(1<<KMOD32(z))))) z++; return(z);
	int i;
		//WARNING: zmax must be multiple of 32!
	i = (iptr[z>>5]&(-1<<KMOD32(z))); z &= ~31;
	while (!i)
	{
		z += 32; if (z >= zmax) return(zmax);
		i = iptr[z>>5];
	}
	return(bsf(i)+z);
}

//--------------------------------------------------------------------------------------------------
#if (VOXMODE == 0)

static int *vbit = 0; //32MB

static int vox_init (void)
{
	int i = ((MAXDIM*MAYDIM*MAZDIM)>>3);
	vbit = (int *)malloc(i); if (!vbit) return(0);
	memset(vbit,0,i);
	return(1);
}

static void vox_free (void) { if (vbit) free(vbit); }

static __forceinline int  vox_test  (int x, int y, int z) { int *p = &vbit[((x*MAYDIM+y)*MAZDIM+z)>>5]; return(p[0] &  (1<<KMOD32(z))); }
static __forceinline void vox_clear (int x, int y, int z) { int *p = &vbit[((x*MAYDIM+y)*MAZDIM+z)>>5];        p[0] &=~(1<<KMOD32(z)) ; }
static __forceinline void vox_set   (int x, int y, int z) { int *p = &vbit[((x*MAYDIM+y)*MAZDIM+z)>>5];        p[0] |= (1<<KMOD32(z)) ; }
static __forceinline void vox_xor   (int x, int y, int z) { int *p = &vbit[((x*MAYDIM+y)*MAZDIM+z)>>5];        p[0] ^= (1<<KMOD32(z)) ; }

	//Set all bits in vbit from (x,y,z0) to (x,y,z1-1) to 0's
static __forceinline void vox_setzrange0 (int x, int y, int z0, int z1)
{
	int z, ze, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
	if (!((z0^z1)&~31)) { iptr[z0>>5] &= ((~(-1<<KMOD32(z0)))|(-1<<KMOD32(z1))); return; }
	z = (z0>>5); ze = (z1>>5);
	iptr[z] &=~(-1<<KMOD32(z0)); for(z++;z<ze;z++) iptr[z] = 0;
	iptr[z] &= (-1<<KMOD32(z1));
}

	//Set all bits in vbit from (x,y,z0) to (x,y,z1-1) to 1's
static __forceinline void vox_setzrange1 (int x, int y, int z0, int z1)
{
	int z, ze, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
	if (!((z0^z1)&~31)) { iptr[z0>>5] |= ((~(-1<<KMOD32(z1)))&(-1<<KMOD32(z0))); return; }
	z = (z0>>5); ze = (z1>>5);
	iptr[z] |= (-1<<KMOD32(z0)); for(z++;z<ze;z++) iptr[z] = -1;
	iptr[z] |=~(-1<<KMOD32(z1));
}

	//Invert all bits in vbit from (x,y,z0) to (x,y,z1-1)
static __forceinline void vox_xorzrange (int x, int y, int z0, int z1)
{
	int z, ze, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
	if (!((z0^z1)&~31)) { iptr[z0>>5] ^= (-1<<KMOD32(z1))^(-1<<KMOD32(z0)); return; }
	z = (z0>>5); ze = (z1>>5);
	iptr[z] ^= (-1<<KMOD32(z0)); for(z++;z<ze;z++) iptr[z] ^= -1;
	iptr[z] ^=~(-1<<KMOD32(z1));
}

static __forceinline int vox_dntil0 (int x, int y, int z)
{
	//   //This line does the same thing (but slow & brute force)
	//while ((z < zsiz) && (iptr[z>>5]&(1<<KMOD32(z)))) z++; return(z);
	int i, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
		//WARNING: zsiz must be multiple of 32!
	i = (iptr[z>>5]|((1<<KMOD32(z))-1)); z &= ~31;
	while (i == -1)
	{
		z += 32; if (z >= zsiz) return(zsiz);
		i = iptr[z>>5];
	}
	return(bsf(~i)+z);
}

static __forceinline int vox_dntil1 (int x, int y, int z)
{
	//   //This line does the same thing (but slow & brute force)
	//while ((z < zsiz) && (!(iptr[z>>5]&(1<<KMOD32(z))))) z++; return(z);
	int i, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
		//WARNING: zsiz must be multiple of 32!
	i = (iptr[z>>5]&(-1<<KMOD32(z))); z &= ~31;
	while (!i)
	{
		z += 32; if (z >= zsiz) return(zsiz);
		i = iptr[z>>5];
	}
	return(bsf(i)+z);
}

static __forceinline int vox_uptil1 (int x, int y, int z)
{
	//   //This line does the same thing (but slow & brute force)
	//while ((z > 0) && (!(iptr[(z-1)>>5]&(1<<KMOD32(z-1))))) z--; return(z);
	int i, *iptr = &vbit[((x*MAYDIM+y)*MAZDIM)>>5];
		//WARNING: zsiz must be multiple of 32!
	if (!z) return(0); //Prevent possible crash
	i = (iptr[(z-1)>>5]&((1<<KMOD32(z))-1)); z &= ~31;
	while (!i)
	{
		z -= 32; if (z < 0) return(0);
		i = iptr[z>>5];
	}
	return(bsr(i)+z+1);
}

	//For savevxl(): does boolean and of 4 neighbors of (x,y) for voxels 0-255 and returns bit buffer in vbitemp
static void vox_and4neighs_retbits (int x, int y, int *vbitemp)
{
	static const int dirx[4] = {-1,+1,0,0};
	static const int diry[4] = {0,0,-1,+1};
	int i, j, nx, ny, *v;

	for(j=(MAXZDIM>>5)-1;j>=0;j--) vbitemp[j] = -1;

		//Collect the 4 neighbor columns (x-1,x+1,y-1,y+1) into vbitemp:
	for(i=4-1;i>=0;i--)
	{
		nx = dirx[i]+x; ny = diry[i]+y; if ((unsigned)(nx|ny) >= VSID) continue;
		v = &vbit[((nx*MAYDIM+ny)*MAZDIM)>>5];
		for(j=(MAXZDIM>>5)-1;j>=0;j--) vbitemp[j] &= v[j];
	}
}

static void vox_copyslabxy_shiftupz (int xd, int yd, int xs, int ys, int zs)
{
	int s, d, v, z;

	s = (xs*MAYDIM+ys)*MAZDIM;
	d = (xd*MAYDIM+yd)*MAZDIM;
	for(z=0;z<zsiz;z+=32)
	{
		v = 0;
		if  (zs+z    < zsiz)             v  = (vbit[ (s+zs+z)>>5   ]>>KMOD32( zs));
		if ((zs+z+32 < zsiz) && (zs&31)) v += (vbit[((s+zs+z)>>5)+1]<<KMOD32(-zs));
		vbit[(d+z)>>5] = v;
	}
	memset(&vbit[s>>5],0,(zsiz+31)>>5);
}

#elif (VOXMODE == 1)

	//Set all bits in iptr from (z0) to (z1-1) to 0's
static __forceinline void bit_setzrange0 (int *iptr, int z0, int z1)
{
	int z, ze;
	if (!((z0^z1)&~31)) { iptr[z0>>5] &= ((~(-1<<KMOD32(z0)))|(-1<<KMOD32(z1))); return; }
	z = (z0>>5); ze = (z1>>5);
	iptr[z] &=~(-1<<KMOD32(z0)); for(z++;z<ze;z++) iptr[z] = 0;
	iptr[z] &= (-1<<KMOD32(z1));
}

	//Set all bits in iptr from (z0) to (z1-1) to 1's
static __forceinline void bit_setzrange1 (int *iptr, int z0, int z1)
{
	int z, ze;
	if (!((z0^z1)&~31)) { iptr[z0>>5] |= ((~(-1<<KMOD32(z1)))&(-1<<KMOD32(z0))); return; }
	z = (z0>>5); ze = (z1>>5);
	iptr[z] |= (-1<<KMOD32(z0)); for(z++;z<ze;z++) iptr[z] = -1;
	iptr[z] |=~(-1<<KMOD32(z1));
}

	//rle_t: unsigned short n, z0, z1, z0, z1, ..
static unsigned short *rle = 0;
static int *rlebit = 0, rlemal = 0;
static int rlehead[MAXDIM*MAYDIM];

static unsigned short zlst[MAXZDIM]; //temp array

	//      n: # structs to alloc
	//returns: bit index or crash if realloc fails
static int vox_bitalloc (int n)
{
	static int gind = 0;
	int i, oi, ie, i0, i1, cnt;

	i = gind; oi = i; ie = rlemal-n;
	for(cnt=1;1;cnt--)
	{
		for(;i<ie;i=i1+1)
		{
			i0 = dntil0(rlebit,i   ,ie  ); if (i0 >= ie) break;
			i1 = dntil1(rlebit,i0+1,i0+n); if (i1-i0 < n) continue;
			bit_setzrange1(rlebit,i0,i0+n); gind = i0+n; return(i0);
		}
		cnt--; if (cnt < 0) break;
		i = 0; ie = min(oi,rlemal-n);
	}

	i = rlemal;
	rlemal = max((rlemal>>2)+rlemal,rlemal+n); //grow space by ~25%
	rle = (unsigned short *)realloc(rle,rlemal*sizeof(rle[0])); if (!rle) { quitout("realloc(rle) failed"); }
	rlebit = (int *)realloc(rlebit,((((int)rlemal+31)>>5)<<2)+16); if (!rlebit) { quitout("realloc(rlebit) failed"); }
	bit_setzrange1(rlebit,i  ,i+n);
	bit_setzrange0(rlebit,i+n,rlemal);
	gind = i+n;
	return(i);
}
static void vox_bitdealloc (int i, int n) { bit_setzrange0(rlebit,i,i+n); }

static int rle_setzrange0 (unsigned short *b0, int n0, unsigned short *b1, int z0, int z1)
{
	int i, n1 = 0;
	for(i=0;i<n0;i+=2)
	{
		if ((b0[i  ] < z0) || (b0[i  ] > z1)) { b1[n1] = b0[i  ]; n1++; }
		if ((z0 >  b0[i]) && (z0 <= b0[i+1])) { b1[n1] = z0; n1++; }
		if ((z1 >= b0[i]) && (z1 <  b0[i+1])) { b1[n1] = z1; n1++; }
		if ((b0[i+1] < z0) || (b0[i+1] > z1)) { b1[n1] = b0[i+1]; n1++; }
	}
	return(n1);
}
static int rle_setzrange1 (unsigned short *b0, int n0, unsigned short *b1, int z0, int z1)
{
	int i, n1 = 0;
	if (!n0) { b1[0] = z0; b1[1] = z1; n1 = 2; }
	else
	{
		if (z0 <= b0[0]) { b1[n1] = z0; n1++; }
		if (z1 <  b0[0]) { b1[n1] = z1; n1++; }
		for(i=0;i<n0;i+=2)
		{
			if ((b0[i  ] < z0) || (b0[i  ] > z1)) { b1[n1] = b0[i  ]; n1++; }
			if ((b0[i+1] < z0) || (b0[i+1] > z1)) { b1[n1] = b0[i+1]; n1++; }
			if (i < n0-2)
			{
				if ((z0 >  b0[i+1]) && (z0 <= b0[i+2])) { b1[n1] = z0; n1++; }
				if ((z1 >= b0[i+1]) && (z1 <  b0[i+2])) { b1[n1] = z1; n1++; }
			}
		}
		if (z0 >  b0[n0-1]) { b1[n1] = z0; n1++; }
		if (z1 >= b0[n0-1]) { b1[n1] = z1; n1++; }
	}
	return(n1);
}
static int rle_xorzrange (unsigned short *b0, int n0, unsigned short *b1, int z0, int z1)
{
	int i, n1 = 0;
	if (!n0) { b1[0] = z0; b1[1] = z1; n1 = 2; }
	else
	{
		if (z0 < b0[0]) { b1[n1] = z0; n1++; }
		if (z1 < b0[0]) { b1[n1] = z1; n1++; }
		for(i=0;i<n0;i++)
		{
			if ((b0[i] != z0) && (b0[i] != z1)) { b1[n1] = b0[i]; n1++; }
			if (i < n0-1)
			{
				if ((z0 > b0[i]) && (z0 < b0[i+1])) { b1[n1] = z0; n1++; }
				if ((z1 > b0[i]) && (z1 < b0[i+1])) { b1[n1] = z1; n1++; }
			}
		}
		if (z0 > b0[n0-1]) { b1[n1] = z0; n1++; }
		if (z1 > b0[n0-1]) { b1[n1] = z1; n1++; }
	}
	return(n1);
}

static int vox_init (void)
{
	rlemal = 1048576;
	rle = (unsigned short *)malloc(rlemal*sizeof(rle[0])); if (!rle) return(0);
	rlebit = (int *)malloc((rlemal+31)>>3); if (!rlebit) return(0);
	memset(rlehead,-1,MAXDIM*MAYDIM*sizeof(rlehead[0]));
	return(1);
}

static void vox_free (void)
{
	if (rlebit) { free(rlebit); rlebit = 0; }
	if (rle) { free(rle); rle = 0; }
}

static int vox_test (int x, int y, int z)
{
	int i, ind, n;

	ind = rlehead[x*MAYDIM+y]; if (ind < 0) return(0);
	n = rle[ind];
	for(i=ind+1;i<=ind+n;i+=2) if (z < rle[i+1]) return(z >= rle[i]);
	return(0);
}

static void vox_clear (int x, int y, int z)
{
	int i, n;
	i = rlehead[x*MAYDIM+y]; if (i < 0) return;
	vox_bitdealloc(i,rle[i]+1);
	n = rle_setzrange0(&rle[i+1],rle[i],zlst,z,z+1); if (!n) { rlehead[x*MAYDIM+y] = -1; return; }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));
}
static void vox_set (int x, int y, int z)
{
	int i, n;
	i = rlehead[x*MAYDIM+y];
	if (i < 0) { zlst[0] = z; zlst[1] = z+1; n = 2; }
			else { vox_bitdealloc(i,rle[i]+1); n = rle_setzrange1(&rle[i+1],rle[i],zlst,z,z+1); }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));
}
static void vox_xor (int x, int y, int z)
{
	int i, n;
	i = rlehead[x*MAYDIM+y];
	if (i < 0) { zlst[0] = z; zlst[1] = z+1; n = 2; }
			else { vox_bitdealloc(i,rle[i]+1); n = rle_xorzrange(&rle[i+1],rle[i],zlst,z,z+1); if (!n) { rlehead[x*MAYDIM+y] = -1; return; } }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));
}

static void vox_setzrange0 (int x, int y, int z0, int z1)
{
	int i, n;
	i = rlehead[x*MAYDIM+y]; if (i < 0) return;
	vox_bitdealloc(i,rle[i]+1);
	n = rle_setzrange0(&rle[i+1],rle[i],zlst,z0,z1); if (!n) { rlehead[x*MAYDIM+y] = -1; return; }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));
}
static void vox_setzrange1 (int x, int y, int z0, int z1)
{
	int i, n;
	i = rlehead[x*MAYDIM+y];
	if (i < 0) { zlst[0] = z0; zlst[1] = z1; n = 2; }
			else { vox_bitdealloc(i,rle[i]+1); n = rle_setzrange1(&rle[i+1],rle[i],zlst,z0,z1); }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));

}
static void vox_xorzrange (int x, int y, int z0, int z1)
{
	int i, n;
	i = rlehead[x*MAYDIM+y];
	if (i < 0) { zlst[0] = z0; zlst[1] = z1; n = 2; }
			else { vox_bitdealloc(i,rle[i]+1); n = rle_xorzrange(&rle[i+1],rle[i],zlst,z0,z1); if (!n) { rlehead[x*MAYDIM+y] = -1; return; } }
	i = vox_bitalloc(n+1); rlehead[x*MAYDIM+y] = i; rle[i] = n;
	memcpy(&rle[i+1],zlst,n*sizeof(rle[0]));
}

	//rle[] = {5,13,18,23};
	//          11111111112222222222
	//012345678901234567890123456789
	//-----11111111-----11111-------
static int vox_dntil0 (int x, int y, int z)
{
	int i, ind, n;

	//while ((z < zsiz) && (vox_test(x,y,z))) z++;

	ind = rlehead[x*MAYDIM+y];
	if (ind >= 0)
	{
		n = rle[ind];
		for(i=ind+1;i<=ind+n;i+=2)
		{
			if (z < rle[i  ]) break;
			if (z < rle[i+1]) { z = rle[i+1]; break; }
		}
	}
	return(z);
}

static int vox_dntil1 (int x, int y, int z)
{
	int i, ind, n;

	//while ((z < zsiz) && (!vox_test(x,y,z))) z++;

	ind = rlehead[x*MAYDIM+y]; if (ind < 0) { z = zsiz; }
	else
	{
		n = rle[ind];
		for(i=ind+1;i<=ind+n;i+=2)
		{
			if (z < rle[i  ]) { z = rle[i]; break; }
			if (z < rle[i+1]) break;
		}
		if (i > ind+n) z = zsiz;
	}
	return(z);
}

static int vox_uptil1 (int x, int y, int z)
{
	int i, ind, n;

	//while ((z > 0) && (!vox_test(x,y,z-1))) z--;

	ind = rlehead[x*MAYDIM+y]; if (ind < 0) { z = 0; }
	else
	{
		n = rle[ind];
		for(i=ind+n-1;i>ind;i-=2)
		{
			if (z > rle[i+1]) { z = rle[i+1]; break; }
			if (z > rle[i  ]) break;
		}
		if (i <= ind) z = 0;
	}
	return(z);
}

static void vox_and4neighs_retbits (int x, int y, int *vbitemp)
{
	static const int dirx[4] = {-1,+1,0,0};
	static const int diry[4] = {0,0,-1,+1};
	int i, j, ind, n, nx, ny, vbitemp2[MAXZDIM>>5];

	for(j=(MAXZDIM>>5)-1;j>=0;j--) vbitemp[j] = -1;

		//Collect the 4 neighbor columns (x-1,x+1,y-1,y+1) into vbitemp:
	for(i=4-1;i>=0;i--)
	{
		nx = dirx[i]+x; ny = diry[i]+y; if ((unsigned)(nx|ny) >= VSID) continue;

		ind = rlehead[nx*MAYDIM+ny]; if (ind < 0) continue;
		memset(vbitemp2,0,sizeof(vbitemp2));
		n = rle[ind];
		for(j=ind+1;j<=ind+n;j+=2) bit_setzrange1(vbitemp2,rle[j],rle[j+1]);
		for(j=(MAXZDIM>>5)-1;j>=0;j--) vbitemp[j] &= vbitemp2[j];
	}
}

static void vox_copyslabxy_shiftupz (int xd, int yd, int xs, int ys, int zs)
{
	int i, ind;
	ind = rlehead[xs*MAYDIM+ys];
	if ((xs != xd) || (ys != yd)) { rlehead[xd*MAYDIM+yd] = ind; rlehead[xs*MAYDIM+ys] = -1; }
	for(i=rle[ind]+ind;i>ind;i--) rle[i] -= zs;
}

#endif

//--------------------------------------------------------------------------------------------------

static void floodfill3dbits (int x, int y, int z) //Conducts on 0's and sets to 1's
{
	int i, j, z0, z1, i0, i1;
	cpoint4d a;

	if (vox_test(x,y,z)) return;
	a.x = x; a.z0 = vox_uptil1(x,y,z);
	a.y = y; a.z1 = vox_dntil1(x,y,z+1);
	vox_setzrange1(x,y,a.z0,a.z1);
	i0 = 0; i1 = 0; goto floodfill3dskip;
	do
	{
		a = fbuf[i0]; i0 = ((i0+1)&(FILLBUFSIZ-1));
floodfill3dskip:;
		for(j=3;j>=0;j--)
		{
			if (j&1) { x = a.x+(j&2)-1; if ((unsigned int)x >= xsiz) continue; y = a.y; }
				 else { y = a.y+(j&2)-1; if ((unsigned int)y >= ysiz) continue; x = a.x; }

			if (vox_test(x,y,a.z0)) { z0 = vox_dntil0(x,y,a.z0); z1 = z0; }
									 else { z0 = vox_uptil1(x,y,a.z0); z1 = a.z0; }
			while (z1 < a.z1)
			{
				z1 = vox_dntil1(x,y,z1);
				fbuf[i1].x = x; fbuf[i1].y = y;
				fbuf[i1].z0 = z0; fbuf[i1].z1 = z1;
				i1 = ((i1+1)&(FILLBUFSIZ-1));
				if (i0 == i1) puts("WARNING: floodfill3d stack overflow!");
				vox_setzrange1(x,y,z0,z1);
				z0 = vox_dntil0(x,y,z1); z1 = z0;
			}
		}
	} while (i0 != i1);
}

//FLOODFILL2D/3D ends -------------------------------------------------------

static void renderfilltri (int faceind)
{
	int i, j, ind[3], sy[3], sx0, sx1, x, y, z, v;
	float f, u, fx, fy, fz, fx1, fy1, fz1, fx2, fy2, fz2;

	ind[0] = tri[faceind].x; ind[1] = tri[faceind].y; ind[2] = tri[faceind].z;

	fx = vert[ind[0]].x; fx1 = vert[ind[1]].x-fx; fx2 = vert[ind[2]].x-fx;
	fy = vert[ind[0]].y; fy1 = vert[ind[1]].y-fy; fy2 = vert[ind[2]].y-fy;
	fz = vert[ind[0]].z; fz1 = vert[ind[1]].z-fz; fz2 = vert[ind[2]].z-fz;
	f = 1.0f/(fx1*fy2 - fy1*fx2);
	fx1 = (fx1*fz2 - fz1*fx2)*f;
	fy1 = (fz1*fy2 - fy1*fz2)*f;
	fz -= (fy1*fx + fx1*fy);

	for(i=2;i>=0;i--) sy[i] = (int)vert[ind[i]].y;
	for(i=2,j=0;i>=0;j=i--)
	{
		if (sy[j] >= sy[i]) continue;
		f = (vert[ind[i]].x-vert[ind[j]].x)/(vert[ind[i]].y-vert[ind[j]].y);
		for(y=sy[j]+1;y<=sy[i];y++)
			lastx[y] = ((float)y-vert[ind[j]].y)*f + vert[ind[j]].x;
	}
	for(i=2,j=0;i>=0;j=i--)
	{
		if (sy[j] <= sy[i]) continue;
		f = (vert[ind[j]].x-vert[ind[i]].x)/(vert[ind[j]].y-vert[ind[i]].y);
		for(y=sy[i]+1;y<=sy[j];y++)
		{
			u = ((float)y-vert[ind[i]].y)*f + vert[ind[i]].x;
			sx0 = (int)min(lastx[y],u);
			sx1 = (int)max(lastx[y],u);
			for(x=sx0+1;x<=sx1;x++)
			{
				z = ((int)(x*fy1 + y*fx1 + fz));
				vox_xor(x,y,z);
				hash_set(x,y,z,faceind);
			}
		}
	}
}

static void renderfillfinish (void)
{
	int i, x, y, z;

		//convert xor-style geometry to solid geometry
	for(x=0;x<xsiz;x++)
		for(y=0;y<ysiz;y++)
		{
			z = -1;
			do
			{
				i = vox_dntil1(x,y,z+1); if (i >= zsiz) break;
				z = vox_dntil1(x,y,i+1);
				vox_setzrange1(x,y,i+1,z); if (z >= zsiz) break;
				vox_clear(x,y,z); //?
			} while (z < zsiz);
		}
}

static void canseerange (point3d *p0, point3d *p1)
{
	ipoint3d a, c, d, p, i;
	point3d f, g;
	int cnt;

	a.x = cvtss2si(p0->x-.5f); a.y = cvtss2si(p0->y-.5f); a.z = cvtss2si(p0->z-.5f);
	c.x = cvtss2si(p1->x-.5f); c.y = cvtss2si(p1->y-.5f); c.z = cvtss2si(p1->z-.5f);
	cnt = 0;

		  if (c.x <  a.x) { d.x = -1; f.x = p0->x-a.x;   g.x = (p0->x-p1->x)*1024; cnt += a.x-c.x; }
	else if (c.x != a.x) { d.x =  1; f.x = a.x+1-p0->x; g.x = (p1->x-p0->x)*1024; cnt += c.x-a.x; }
	else f.x = g.x = 0;
		  if (c.y <  a.y) { d.y = -1; f.y = p0->y-a.y;   g.y = (p0->y-p1->y)*1024; cnt += a.y-c.y; }
	else if (c.y != a.y) { d.y =  1; f.y = a.y+1-p0->y; g.y = (p1->y-p0->y)*1024; cnt += c.y-a.y; }
	else f.y = g.y = 0;
		  if (c.z <  a.z) { d.z = -1; f.z = p0->z-a.z;   g.z = (p0->z-p1->z)*1024; cnt += a.z-c.z; }
	else if (c.z != a.z) { d.z =  1; f.z = a.z+1-p0->z; g.z = (p1->z-p0->z)*1024; cnt += c.z-a.z; }
	else f.z = g.z = 0;

	p.x = cvtss2si(f.x*g.z - f.z*g.x); i.x = cvtss2si(g.x);
	p.y = cvtss2si(f.y*g.z - f.z*g.y); i.y = cvtss2si(g.y);
	p.z = cvtss2si(f.y*g.x - f.x*g.y); i.z = cvtss2si(g.z);
	for(;cnt;cnt--)
	{
			//use a.x, a.y, a.z
		if (a.x < min0[a.y]) min0[a.y] = a.x;
		if (a.x > max0[a.y]) max0[a.y] = a.x;
		if (a.z < min1[a.x]) min1[a.x] = a.z;
		if (a.z > max1[a.x]) max1[a.x] = a.z;
		if (a.z < min2[a.y]) min2[a.y] = a.z;
		if (a.z > max2[a.y]) max2[a.y] = a.z;

		if (((p.x|p.y) >= 0) && (a.z != c.z)) { a.z += d.z; p.x -= i.x; p.y -= i.y; }
		else if ((p.z >= 0) && (a.x != c.x))  { a.x += d.x; p.x += i.z; p.z -= i.y; }
		else                                  { a.y += d.y; p.y += i.z; p.z += i.x; }
	}
}

static void renderface1tri (int faceind)
{
	point3d n, *p0, *p1, *p2;
	float f, x0, x1, y0, y1, rx, ry, k0, k1;
	int i, x, y, z, z0, z1, minx, maxx, miny, maxy;

	p0 = (point3d *)&vert[tri[faceind].x];
	p1 = (point3d *)&vert[tri[faceind].y];
	p2 = (point3d *)&vert[tri[faceind].z];

	x0 = p0->x; x1 = p1->x; if (x0 > x1) { f = x0; x0 = x1; x1 = f; }
	if (p2->x < x0) x0 = p2->x;
	if (p2->x > x1) x1 = p2->x;
	y0 = p0->y; y1 = p1->y; if (y0 > y1) { f = y0; y0 = y1; y1 = f; }
	if (p2->y < y0) y0 = p2->y;
	if (p2->y > y1) y1 = p2->y;
	minx = cvtss2si(x0-.5f); miny = cvtss2si(y0-.5f);
	maxx = cvtss2si(x1-.5f); maxy = cvtss2si(y1-.5f);
	for(i=miny;i<=maxy;i++) { min0[i] = 0x7fffffff; max0[i] = 0x80000000; }
	for(i=minx;i<=maxx;i++) { min1[i] = 0x7fffffff; max1[i] = 0x80000000; }
	for(i=miny;i<=maxy;i++) { min2[i] = 0x7fffffff; max2[i] = 0x80000000; }

	canseerange(p0,p1);
	canseerange(p1,p2);
	canseerange(p2,p0);

	n.x = (p1->z-p0->z)*(p2->y-p1->y) - (p1->y-p0->y) * (p2->z-p1->z);
	n.y = (p1->x-p0->x)*(p2->z-p1->z) - (p1->z-p0->z) * (p2->x-p1->x);
	n.z = (p1->y-p0->y)*(p2->x-p1->x) - (p1->x-p0->x) * (p2->y-p1->y);
	f = 1.0f / (float)sqrt(n.x*n.x + n.y*n.y + n.z*n.z); if (n.z < 0.f) f = -f;
	n.x *= f; n.y *= f; n.z *= f;

		//Change this to "if (0)" for cool bump-mapping effects on kerolamp&skull
	if (n.z > .01f)
	{
		f = -1.0f / n.z; rx = n.x*f; ry = n.y*f;
		k0 = ((n.x>=0)-p0->x)*rx + ((n.y>=0)-p0->y)*ry - ((n.z>=0)-p0->z) + .5f;
		k1 = ((n.x< 0)-p0->x)*rx + ((n.y< 0)-p0->y)*ry - ((n.z< 0)-p0->z) - .5f;
	}
	else { rx = 0.f; ry = 0.f; k0 = -2147000000.0f; k1 = 2147000000.0f; }

	for(y=miny;y<=maxy;y++)
		for(x=min0[y];x<=max0[y];x++)
		{
			f = (float)x*rx + (float)y*ry; z0 = cvtss2si(f+k0); z1 = cvtss2si(f+k1);
			if (z0 < min1[x]) z0 = min1[x];
			if (z1 > max1[x]) z1 = max1[x];
			if (z0 < min2[y]) z0 = min2[y];
			if (z1 > max2[y]) z1 = max2[y];

			vox_setzrange1(x,y,z0,z1+1);
			for(z=z0;z<=z1;z++) { hash_set(x,y,z,faceind); }
		}
}

	//Minimal voxel implementation
static void renderline0 (point3d *bp0, point3d *bp1, int col)
{
	float dx, dy, dz;
	point3d p0, p1;
	ipoint3d l0, l1, l2;
	int j;

	p0 = *bp0; p1 = *bp1;

	l0.x = cvtss2si(p0.x+.5f); l0.y = cvtss2si(p0.y+.5f); l0.z = cvtss2si(p0.z+.5f);
	l1.x = cvtss2si(p1.x+.5f); l1.y = cvtss2si(p1.y+.5f); l1.z = cvtss2si(p1.z+.5f);

	dx = (p1.x)-(p0.x); dy = (p1.y)-(p0.y); dz = (p1.z)-(p0.z);
	if ((fabs(dz) >= fabs(dx)) && (fabs(dz) >= fabs(dy)))
	{
		if (l0.z > l1.z) { l2 = l0; l0 = l1; l1 = l2; p0 = p1; }
		dz = 1.0f / dz; dx *= dz; dy *= dz; p0.x -= p0.z*dx; p0.y -= p0.z*dy;
		for(l2.z=l0.z;l2.z<l1.z;l2.z++)
		{
			l2.x = cvtss2si(((float)l2.z)*dx + p0.x);
			l2.y = cvtss2si(((float)l2.z)*dy + p0.y);
			vox_set(l2.x,l2.y,l2.z);
			hash_set(l2.x,l2.y,l2.z,col);
		}
	}
	else if (fabs(dx) >= fabs(dy))
	{
		if (l0.x > l1.x) { l2 = l0; l0 = l1; l1 = l2; p0 = p1; }
		dx = 1.0f / dx; dy *= dx; dz *= dx; p0.y -= p0.x*dy; p0.z -= p0.x*dz;
		for(l2.x=l0.x;l2.x<l1.x;l2.x++)
		{
			l2.y = cvtss2si(((float)l2.x)*dy + p0.y);
			l2.z = cvtss2si(((float)l2.x)*dz + p0.z);
			vox_set(l2.x,l2.y,l2.z);
			hash_set(l2.x,l2.y,l2.z,col);
		}
	}
	else
	{
		if (l0.y > l1.y) { l2 = l0; l0 = l1; l1 = l2; p0 = p1; }
		dy = 1.0f / dy; dx *= dy; dz *= dy; p0.x -= p0.y*dx; p0.z -= p0.y*dz;
		for(l2.y=l0.y;l2.y<l1.y;l2.y++)
		{
			l2.x = cvtss2si(((float)l2.y)*dx + p0.x);
			l2.z = cvtss2si(((float)l2.y)*dz + p0.z);
			vox_set(l2.x,l2.y,l2.z);
			hash_set(l2.x,l2.y,l2.z,col);
		}
	}
}

static void renderline0tri (int i)
{
	renderline0((point3d *)&vert[tri[i].x],(point3d *)&vert[tri[i].y],i);
	renderline0((point3d *)&vert[tri[i].y],(point3d *)&vert[tri[i].z],i);
	renderline0((point3d *)&vert[tri[i].z],(point3d *)&vert[tri[i].x],i);
}

	//Based on cansee from voxlap5.c (Supercover)
static void renderline1 (point3d *p0, point3d *p1, int col)
{
	ipoint3d a, c, d, p, i;
	point3d f, g;
	int cnt, j;

	a.x = cvtss2si(p0->x-.5f); a.y = cvtss2si(p0->y-.5f); a.z = cvtss2si(p0->z-.5f);
	vox_set(a.x,a.y,a.z);
	hash_set(a.x,a.y,a.z,col);
	c.x = cvtss2si(p1->x-.5f); c.y = cvtss2si(p1->y-.5f); c.z = cvtss2si(p1->z-.5f);
	cnt = 0;

		  if (c.x <  a.x) { d.x = -1; f.x = p0->x-a.x;   g.x = (p0->x-p1->x)*1024; cnt += a.x-c.x; }
	else if (c.x != a.x) { d.x =  1; f.x = a.x+1-p0->x; g.x = (p1->x-p0->x)*1024; cnt += c.x-a.x; }
	else f.x = g.x = 0;
		  if (c.y <  a.y) { d.y = -1; f.y = p0->y-a.y;   g.y = (p0->y-p1->y)*1024; cnt += a.y-c.y; }
	else if (c.y != a.y) { d.y =  1; f.y = a.y+1-p0->y; g.y = (p1->y-p0->y)*1024; cnt += c.y-a.y; }
	else f.y = g.y = 0;
		  if (c.z <  a.z) { d.z = -1; f.z = p0->z-a.z;   g.z = (p0->z-p1->z)*1024; cnt += a.z-c.z; }
	else if (c.z != a.z) { d.z =  1; f.z = a.z+1-p0->z; g.z = (p1->z-p0->z)*1024; cnt += c.z-a.z; }
	else f.z = g.z = 0;

	p.x = cvtss2si(f.x*g.z - f.z*g.x); i.x = cvtss2si(g.x);
	p.y = cvtss2si(f.y*g.z - f.z*g.y); i.y = cvtss2si(g.y);
	p.z = cvtss2si(f.y*g.x - f.x*g.y); i.z = cvtss2si(g.z);
	for(;cnt;cnt--)
	{
		if (((p.x|p.y) >= 0) && (a.z != c.z)) { a.z += d.z; p.x -= i.x; p.y -= i.y; }
		else if ((p.z >= 0) && (a.x != c.x))  { a.x += d.x; p.x += i.z; p.z -= i.y; }
		else                                  { a.y += d.y; p.y += i.z; p.z += i.x; }
		vox_set(a.x,a.y,a.z);
		hash_set(a.x,a.y,a.z,col);
	}
}

static void renderline1tri (int i)
{
	renderline1((point3d *)&vert[tri[i].x],(point3d *)&vert[tri[i].y],i);
	renderline1((point3d *)&vert[tri[i].y],(point3d *)&vert[tri[i].z],i);
	renderline1((point3d *)&vert[tri[i].z],(point3d *)&vert[tri[i].x],i);
}

static void renderface0tri (int faceind)
{
	float f, u, fx, fy, fz, fx1, fy1, fz1, fx2, fy2, fz2;
	int i, j, k, l, ind[3], sy[3], sx0, sx1, x, y, z, i0, i1, i2;

	ind[0] = tri[faceind].x; ind[1] = tri[faceind].y; ind[2] = tri[faceind].z;

#if 0
	  //Use normal vector to calculate best axis
	fx = vert[ind[0]].x; fx1 = vert[ind[1]].x-fx; fx2 = vert[ind[2]].x-fx;
	fy = vert[ind[0]].y; fy1 = vert[ind[1]].y-fy; fy2 = vert[ind[2]].y-fy;
	fz = vert[ind[0]].z; fz1 = vert[ind[1]].z-fz; fz2 = vert[ind[2]].z-fz;
	fx = fy1*fz2 - fz1*fy2;
	fy = fz1*fx2 - fx1*fz2;
	fz = fx1*fy2 - fy1*fx2;
	if ((fabs(fz) >= fabs(fx)) && (fabs(fz) >= fabs(fy))) k = 0;
	else if (fabs(fx) >= fabs(fy)) k = 1; else k = 2;
#endif

	for(k=0;k<3;k++)
	{
		i0 = k; i2 = k-1; if (i2 < 0) i2 = 2; i1 = 3-i0-i2;
		fx = vert[ind[0]].a[i0]; fx1 = vert[ind[1]].a[i0]-fx; fx2 = vert[ind[2]].a[i0]-fx;
		fy = vert[ind[0]].a[i1]; fy1 = vert[ind[1]].a[i1]-fy; fy2 = vert[ind[2]].a[i1]-fy;
		fz = vert[ind[0]].a[i2]; fz1 = vert[ind[1]].a[i2]-fz; fz2 = vert[ind[2]].a[i2]-fz;
		f = 1.0f/(fx1*fy2 - fy1*fx2);
		fx1 = (fx1*fz2 - fz1*fx2)*f;
		fy1 = (fz1*fy2 - fy1*fz2)*f;
		fz -= (fy1*fx + fx1*fy);

		for(i=2;i>=0;i--) sy[i] = (int)vert[ind[i]].a[i1];
		for(i=2,j=0;i>=0;j=i--)
		{
			if (sy[j] >= sy[i]) continue;
			f = (vert[ind[i]].a[i0]-vert[ind[j]].a[i0])/(vert[ind[i]].a[i1]-vert[ind[j]].a[i1]);
			for(y=sy[j]+1;y<=sy[i];y++)
				lastx[y] = ((float)y-vert[ind[j]].a[i1])*f + vert[ind[j]].a[i0];
		}
		for(i=2,j=0;i>=0;j=i--)
		{
			if (sy[j] <= sy[i]) continue;
			f = (vert[ind[j]].a[i0]-vert[ind[i]].a[i0])/(vert[ind[j]].a[i1]-vert[ind[i]].a[i1]);
			for(y=sy[i]+1;y<=sy[j];y++)
			{
				u = ((float)y-vert[ind[i]].a[i1])*f + vert[ind[i]].a[i0];
				sx0 = (int)min(lastx[y],u);
				sx1 = (int)max(lastx[y],u);
				for(x=sx0+1;x<=sx1;x++)
				{
					z = (int)(x*fy1 + y*fx1 + fz);
					switch (k)
					{
						case 0: vox_set(x,y,z); hash_set(x,y,z,faceind); break;
						case 1: vox_set(z,x,y); hash_set(z,x,y,faceind); break;
						case 2: vox_set(y,z,x); hash_set(y,z,x,faceind); break;
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

static int findchar (char *st, char ch)
{
	int i;
	for(i=0;st[i];i++) if (st[i] == ch) return(i);
	return(i);
}

static int loadasc (char *fnam)
{
	int i, j, w, x, y, z, op, p, hidden, leng, lastface = 0, fleng = 0, tbufr = 0;
	unsigned char tbuf[4096];

		//Make material index #0 a dummy color (black)
	matstr[0] = namplc;
	strcpy(&nambuf[namplc],"r0g0b0a0"); namplc += strlen(&nambuf[namplc])+1;
	colrgb[0] = 0; matnum = 1;

		//Tri-mesh, Vertices: 16     Faces: 8
		//Vertex 0:  X: -128.239471     Y: -201...
		//Face 0:    A:0 B:1 C:2 AB:1 BC:1 CA:0
	if (!kzopen(fnam)) return(0);
	printf("Reading %s\n",fnam);
	fleng = kzfilelength();

	hidden = 0;
	while (1)
	{
			//Read 1 line and remove spaces at beginning...
		leng = -1;
		while (1)
		{
#if 0
			j = kzgetc(); if (j < 0) goto file_eof;
#else
				//Much faster than above!
			if (!(tbufr&(sizeof(tbuf)-1))) kzread(tbuf,sizeof(tbuf));
			if (tbufr >= fleng) goto file_eof;
			j = tbuf[tbufr&(sizeof(tbuf)-1)]; tbufr++;
#endif
			if ((j < 32) && (leng >= 0)) { tempbuf[leng] = 0; break; }
			if ((j > 32) && (leng == -1)) leng = 0;
			if ((leng >= 0) && (leng < sizeof(tempbuf)-1)) { tempbuf[leng] = (unsigned char)j; leng++; }
		}

		if ((leng >= 8) && (!memcmp(tempbuf,"Tri-mesh",8)))
		{
			hidden = 0;
			trioff[numobjects] = numtris;
			vertoff[numobjects] = numverts;
			numobjects++;
			if (numobjects >= maxobjects)
			{
				maxobjects <<= 1;
#if (PRINTALLOC == 1)
				printf("maxobjects=%d (%dKB)\n",maxobjects,(maxobjects*8)>>10);
#endif
				if (!(trioff = (int *)realloc(trioff,sizeof(int)*maxobjects))) quitout("realloc failed: trioff");
				if (!(vertoff = (int *)realloc(vertoff,sizeof(int)*maxobjects))) quitout("realloc failed: vertoff");
			}
			continue;
		}
		if ((leng >= 6) && (!memcmp(tempbuf,"Hidden",6))) { hidden = 1; continue; }
		if ((leng >= 6) && (!memcmp(tempbuf,"Vertex",6)))
		{
			if ((tempbuf[7] < '0') || (tempbuf[7] > '9') || (hidden)) continue;
			op = 7; p = findchar(&tempbuf[op],':')+op;
			x = findchar(&tempbuf[p],'X')+p;
			y = findchar(&tempbuf[x],'Y')+x;
			z = findchar(&tempbuf[y],'Z')+y;
			tempbuf[x] = 0; tempbuf[y] = 0; tempbuf[z] = 0;
			j = atoi(&tempbuf[op])+vertoff[numobjects-1];
			if ((unsigned int)j >= maxverts)
			{
				maxverts += (maxverts>>1);
#if (PRINTALLOC == 1)
				printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
				if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
			}
			vert[j].x = (float)atof(&tempbuf[x+2]);
			vert[j].y = (float)atof(&tempbuf[y+2]);
			vert[j].z = (float)atof(&tempbuf[z+2]);
			if (j >= numverts) numverts = j+1;
			continue;
		}
		if ((leng >= 4) && (!memcmp(tempbuf,"Face",4)))
		{
			if ((tempbuf[5] < '0') || (tempbuf[5] > '9') || (hidden)) continue;
			op = 5; p = findchar(&tempbuf[op],':')+op;
			x = findchar(&tempbuf[p],'A')+p;
			y = findchar(&tempbuf[x],'B')+x;
			z = findchar(&tempbuf[y],'C')+y;
			w = findchar(&tempbuf[z],'A')+z;
			tempbuf[x] = 0; tempbuf[y] = 0; tempbuf[z] = 0; tempbuf[w] = 0;
			j = atoi(&tempbuf[op])+trioff[numobjects-1];
			if ((unsigned int)j >= maxtris)
			{
				maxtris += (maxtris>>1);
#if (PRINTALLOC == 1)
				printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
				if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
			}
			tri[j].x = atoi(&tempbuf[x+2])+vertoff[numobjects-1];
			tri[j].y = atoi(&tempbuf[y+2])+vertoff[numobjects-1];
			tri[j].z = atoi(&tempbuf[z+2])+vertoff[numobjects-1];
			lastface = j;
			if (j >= numtris) numtris = j+1;
			continue;
		}
		if ((leng >= 8) && (!memcmp(tempbuf,"Material",8))) //Material:"PALM TREE TRUNK"
		{
			x = 8;
			y = findchar(&tempbuf[x],'\"')+x;
			z = findchar(&tempbuf[y+1],'\"')+y+1;
			y++; tempbuf[z] = 0; //&tempbuf[y] is now string in quotes (quotes stripped)

			for(j=matnum-1;j>0;j--)
				if (!strcmp(&nambuf[matstr[j]],&tempbuf[y]))
					{ tri[lastface].i = j; break; }
			if (!j) //Material string not in list, so add it...
			{
				j = strlen(&tempbuf[y]);
				if (matnum >= maxmat)
				{
					maxmat <<= 1;
#if (PRINTALLOC == 1)
					printf("maxmat=%d (%dKB)\n",maxmat,(maxmat*8)>>10);
#endif
					if (!(matstr = (int *)realloc(matstr,sizeof(int)*maxmat))) quitout("realloc failed: matstr");
					if (!(colrgb = (int *)realloc(colrgb,sizeof(int)*maxmat))) quitout("realloc failed: colrgb");
				}
				if (namplc+j >= maxnamsiz)
				{
					maxnamsiz = max(maxnamsiz<<1,namplc+j+1);
#if (PRINTALLOC == 1)
					printf("maxnamsiz=%d (%dKB)\n",maxnamsiz,(maxnamsiz*1)>>10);
#endif
					if (!(nambuf = (char *)realloc(nambuf,sizeof(char)*maxnamsiz))) quitout("realloc failed: nambuf");
				}
				tri[lastface].i = matnum;
				matstr[matnum] = namplc; matnum++;
				memcpy(&nambuf[namplc],&tempbuf[y],j+1); namplc += j+1;
			}
		}
	}
file_eof:;
	kzclose();
	trioff[numobjects] = numtris;
	vertoff[numobjects] = numverts;

	if (matnum <= 2)
	{
		if (numobjects < 256)
		{
			for(i=0;i<numobjects;i++)
			{
				colrgb[i+1] = ((rand()&127)+64)+(((rand()&127)+64)<<8)+(((rand()&127)+64)<<16);
				for(z=trioff[i];z<trioff[i+1];z++) tri[z].i = i+1;
			}
			matnum = numobjects+1;
		}
		else
		{
			for(i=0;i<numtris;i++) tri[i].i = 1;
			colrgb[1] = 0x808080; matnum = 2;
		}
	}
	else
	{
		for(i=0;i<matnum;i++)
		{
			colrgb[i] = ((rand()&127)+64)+(((rand()&127)+64)<<8)+(((rand()&127)+64)<<16);
			x = 24; y = 0xff000000;
			for(j=0;nambuf[matstr[i]+j];j++)
			{
				switch(nambuf[matstr[i]+j])
				{
					case 'B': case 'b': x =  0; y = 0x000000ff; colrgb[i] &= ~y; break;
					case 'G': case 'g': x =  8; y = 0x0000ff00; colrgb[i] &= ~y; break;
					case 'R': case 'r': x = 16; y = 0x00ff0000; colrgb[i] &= ~y; break;
					case 'A': case 'a': x = 24; y = 0xff000000; colrgb[i] &= ~y; break;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						colrgb[i] = (((colrgb[i]&y)*10+((nambuf[matstr[i]+j]-48)<<x))&y)+(colrgb[i]&~y);
						break;
				}
			}
		}
	}

	return(1);
}

static void chunkread(int p);

static char *ASCIIZReader (int p)
{
	int i, j = 0;
	while ((i = kzgetc()) > 0)
		if (j < sizeof(tempbuf)-1) tempbuf[j++] = i;
	tempbuf[j] = 0;
	return(tempbuf);
}

static void ObjBlockReader (int p)
{
	ASCIIZReader(p);
	chunkread(p); //Read rest of chunks inside this one.

	numobjects++;
	if (numobjects >= maxobjects)
	{
		maxobjects <<= 1;
#if (PRINTALLOC == 1)
		printf("maxobjects=%d (%dKB)\n",maxobjects,(maxobjects*8)>>10);
#endif
		if (!(trioff = (int *)realloc(trioff,sizeof(int)*maxobjects))) quitout("realloc failed: trioff");
		if (!(vertoff = (int *)realloc(vertoff,sizeof(int)*maxobjects))) quitout("realloc failed: vertoff");
	}
	trioff[numobjects] = numtris;
	vertoff[numobjects] = numverts;
}

static void VertListReader (int p)
{
	unsigned short s;
	int nv;
	float c[3];

	if (!kzread(&s,sizeof(s))) return;
	nv = (int)s;
	//printf("vert %d\n",nv);
	while (nv-- > 0)
	{
		if (!kzread(&c,sizeof(c))) return;

		if ((unsigned int)numverts >= maxverts)
		{
			maxverts += (maxverts>>1);
#if (PRINTALLOC == 1)
			printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
			if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
		}
		vert[numverts].x = c[0];
		vert[numverts].y = c[1];
		vert[numverts].z = c[2];
		numverts++;
	}
}

static void FaceListReader (int p)
{
	unsigned short s, c[3], flg;
	int nv;

	if (!kzread(&s,sizeof(s))) return;
	nv = (int)s;
	//printf("face %d\n",nv);
	while (nv-- > 0)
	{
		if (!kzread(&c,sizeof(c))) return;
			//CA:flg&1, BC:flg&2, AB:flg&4, UWrap:flg&8, VWrap:flg&16
		if (!kzread(&flg,sizeof(flg))) return;

		if ((unsigned int)numtris >= maxtris)
		{
			maxtris += (maxtris>>1);
#if (PRINTALLOC == 1)
			printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
			if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
		}
		tri[numtris].x = ((int)c[0])+vertoff[numobjects];
		tri[numtris].y = ((int)c[1])+vertoff[numobjects];
		tri[numtris].z = ((int)c[2])+vertoff[numobjects];
		numtris++;
	}
	chunkread(p); //Read rest of chunks inside this one.
}

static int matstring2num (char *st)
{
	int j, k;

	for(j=matnum-1;j>0;j--)
		if (!strcmp(&nambuf[matstr[j]],st))
			break;
	if (!j) //Material string not in list, so add it...
	{
		k = strlen(st);
		if (matnum >= maxmat)
		{
			maxmat <<= 1;
#if (PRINTALLOC == 1)
			printf("maxmat=%d (%dKB)\n",maxmat,(maxmat*8)>>10);
#endif
			if (!(matstr = (int *)realloc(matstr,sizeof(int)*maxmat))) quitout("realloc failed: matstr");
			if (!(colrgb = (int *)realloc(colrgb,sizeof(int)*maxmat))) quitout("realloc failed: colrgb");
		}
		if (namplc+k >= maxnamsiz)
		{
			maxnamsiz = max(maxnamsiz<<1,namplc+k+1);
#if (PRINTALLOC == 1)
			printf("maxnamsiz=%d (%dKB)\n",maxnamsiz,(maxnamsiz*1)>>10);
#endif
			if (!(nambuf = (char *)realloc(nambuf,sizeof(char)*maxnamsiz))) quitout("realloc failed: nambuf");
		}
		j = matnum;
		matstr[matnum] = namplc;
		colrgb[matnum] = 0; matnum++;
		memcpy(&nambuf[namplc],st,k+1); namplc += k+1;
	}
	return(j);
}

static void FaceMatReader (int p)
{
	unsigned short s, nf;
	int n, j;

	ASCIIZReader(p);
	//printf("%s\n",tempbuf);
	j = matstring2num(tempbuf);

	if (!kzread(&s,sizeof(s))) return;
	n = (int)s;
	while (n-- > 0)
	{
		if (!kzread(&nf,sizeof(nf))) return;
		//printf("tri[%d].i = %d\n",((int)nf)+trioff[numobjects],j);
		tri[((int)nf)+trioff[numobjects]].i = j;
	}
}

static void MapListReader (int p)
{
	unsigned short s;
	int nv, j;
	float c[2];

	if (!kzread(&s,sizeof(s))) return;
	nv = (int)s; j = 0;
	while (nv-- > 0)
	{
		if (!kzread(&c,sizeof(c))) return;
		vert[vertoff[numobjects]+j].u = c[0];
		vert[vertoff[numobjects]+j].v = c[1];
		j++;
	}
}

static int lastmat = 0;
static void MatNameReader (int p)
{
	ASCIIZReader(p);
	lastmat = matstring2num(tempbuf);
	//printf("%s\n",tempbuf);
}

static int picstring2num (char *st)
{
	int j, k;

	for(j=numpics-1;j>=0;j--)
		if (!strcmp(&nambuf[pic[j].namoff],st))
			break;
	if (j < 0) //Material string not in list, so add it...
	{
		k = strlen(st);

		if (numpics >= maxpics)
		{
			maxpics <<= 1;
#if (PRINTALLOC == 1)
			printf("maxpics=%d, (%dKB)\n",maxpics,(maxpics*20)>>10);
#endif
			if (!(pic = (pictyp *)realloc(pic,sizeof(pictyp)*maxpics))) quitout("realloc failed: pic");
		}
		if (namplc+k >= maxnamsiz)
		{
			maxnamsiz = max(maxnamsiz<<1,namplc+k+1);
#if (PRINTALLOC == 1)
			printf("maxnamsiz=%d (%dKB)\n",maxnamsiz,(maxnamsiz*1)>>10);
#endif
			if (!(nambuf = (char *)realloc(nambuf,sizeof(char)*maxnamsiz))) quitout("realloc failed: nambuf");
		}
		j = numpics;
		pic[numpics].namoff = namplc;
		numpics++;
		memcpy(&nambuf[namplc],st,k+1); namplc += k+1;
	}
	return(j);
}

static void MapFileReader (int p)
{
	ASCIIZReader(p);
	//printf("%s\n",tempbuf);
	colrgb[lastmat] = ~(picstring2num(tempbuf));
}

static void RGBFReader (int p)
{
	float c[3];
	if (!kzread(&c,sizeof(c))) return;
	if (colrgb[lastmat] < 0) return; //use texture map instead!
	colrgb[lastmat] = (((int)c[0])<<16) + (((int)c[1])<<8) + ((int)c[2]);
	//printf("color=0x%08x\n",colrgb[lastmat]);

}

static void RGBBReader (int p)
{
	unsigned char c[3];
	if (!kzread(&c,sizeof(c))) return;
	if (colrgb[lastmat] < 0) return; //use texture map instead!
	colrgb[lastmat] = (((int)c[0])<<16)+(((int)c[1])<<8)+((int)c[2]);
	//printf("color=0x%08x\n",colrgb[lastmat]);
}

struct
{
	unsigned short id;
	void (*func)(int p);
} chunknames[] =
{
	{0x0010,RGBFReader},     //RGB float
	{0x0011,RGBBReader},     //RGB char
	{0x4D4D,0},              //Main
	{0x3D3D,0},              //Object Mesh
	{0x4000,ObjBlockReader}, //Object Block
	{0x4100,0},              //Tri-Mesh
	{0x4110,VertListReader}, //Vertex list
	{0x4120,FaceListReader}, //Face list
	{0x4130,FaceMatReader},  //Face material
	{0x4140,MapListReader},  //Mappings list
	{0xAFFF,0},              //Material
	{0xA000,MatNameReader},  //Material name
	{0xA020,0},              //Diffuse color
	{0xA200,0},              //Texture map
	{0xA300,MapFileReader},  //Texture filename
};

static void chunkread (int p)
{
	int n, pc, leng;
	unsigned short id;

	while (kztell() < p)
	{
		pc = kztell();
		if (!kzread(&id,2)) return;
		if (!kzread(&leng,4)) return; if (!leng) return;
		//printf("id=%04x, leng=%d\n",id,leng);
		for (n=(sizeof(chunknames)/sizeof(chunknames[0]))-1;n>=0;n--)
			if (chunknames[n].id == id)
			{
				pc += leng;
				if (chunknames[n].func) chunknames[n].func(pc);
										 else chunkread(pc);
				kzseek(pc,SEEK_SET);
				break;
			}
		if (n < 0) kzseek(pc+leng,SEEK_SET);
	}
}

static int load3ds (char *fnam)
{
	int leng;

		//Make material index #0 a dummy color (black)
	matstr[0] = namplc;
	strcpy(&nambuf[namplc],"r0g0b0a0"); namplc += strlen(&nambuf[namplc])+1;
	matnum = 1;

	trioff[0] = 0;
	vertoff[0] = 0;

	if (!kzopen(fnam)) return(0);
	printf("Reading %s\n",fnam);
	kzseek(0,SEEK_END); leng = kztell(); kzseek(0,SEEK_SET);
	chunkread(leng);
	kzclose();

	return(1);
}

//-------------------------------------  MD2 LIBRARY BEGINS ----------------------------------------
	//This MD2 code is based on the source code from David Henry (tfc_duke(at)hotmail.com)
	//   http://tfc.duke.free.fr/us/tutorials/models/md2.htm
	//He probably wouldn't recognize it if he looked at it though :)


typedef struct
{  int id, vers, skinxsiz, skinysiz, framebytes; //id:"IPD2", vers:8
	int numskins, numverts, numuv, numtris, numglcmds, numframes;
	int ofsskins, ofsuv, ofstris, ofsframes, ofsglcmds, ofseof; //ofsskins: skin names (64 bytes each)
} md2head_t;

typedef struct { unsigned char v[3], ni; } md2vert_t; //compressed vertex coords (x,y,z)
typedef struct
{  point3d mul, add; //scale&translation vector
	char name[16];    //frame name
	md2vert_t verts[1]; //first vertex of this frame
} md2frame_t;

typedef struct
{
	int numframes, numverts, numglcmds, framebytes, *glcmds;
	float scale;
	char *frames;
} md2model;

static point3d *vertlist = 0;

static void md2free (md2model *m)
{
	if (vertlist) { free(vertlist); vertlist = 0; }
	if (m->glcmds) { free(m->glcmds); m->glcmds = 0; }
	if (m->frames) { free(m->frames); m->frames = 0; }
}

static int md2load (md2model *m, const char *filename)
{
	md2head_t head;
	char *buf, st[MAX_PATH+64];
	int i;

	memset(m,0,sizeof(md2model));
	m->glcmds = 0; m->numframes = m->numverts = m->numglcmds = 0; m->scale = 1; //.01;

	if (!kzopen(filename)) return(0);
	kzread((char *)&head,sizeof(md2head_t));
	if ((head.id != 0x32504449) && (head.vers != 8)) { kzclose(); return(0); } //"IDP2"

	m->numframes = head.numframes;
	m->numverts = head.numverts;
	m->numglcmds = head.numglcmds;
	m->framebytes = head.framebytes;
	m->frames = (char *)malloc(m->numframes*head.framebytes);
	m->glcmds = (int *)malloc(m->numglcmds*sizeof(int));
	kzseek(head.ofsframes,SEEK_SET); kzread((char *)m->frames,m->numframes*head.framebytes);
	kzseek(head.ofsglcmds,SEEK_SET); kzread((char *)m->glcmds,m->numglcmds*sizeof(int));

	strcpy(st,filename);
	for(i=strlen(st)-1;i>0;i--)
		if ((st[i] == '/') || (st[i] == '\\')) { i++; break; }
	kzseek(head.ofsskins,SEEK_SET); kzread(&st[i],64);
	for(;st[i];i++) if (st[i] == ':') st[i] = 0; //Hack for certain files
	kzclose();

	pic[0].namoff = namplc; numpics = 1; i = strlen(st);
	if (namplc+i >= maxnamsiz)
	{
		maxnamsiz = max(maxnamsiz<<1,namplc+i+1);
#if (PRINTALLOC == 1)
		printf("maxnamsiz=%d (%dKB)\n",maxnamsiz,(maxnamsiz*1)>>10);
#endif
		if (!(nambuf = (char *)realloc(nambuf,sizeof(char)*maxnamsiz))) quitout("realloc failed: nambuf");
	}
	memcpy(&nambuf[namplc],st,i+1); namplc += i+1;
	colrgb[0] = -1;

	vertlist = (point3d *)malloc(m->numverts*sizeof(point3d));

	return(1);
}

static void md2draw (md2model *m, int cframe, int nframe, float interp)
{
	point3d p0, p1, m0, m1, a0, a1;
	md2frame_t *f0, *f1;
	unsigned char *c0, *c1;
	int i, j, onumverts, trit, *iptr;
	float f, g;

		//create current&next frame's vertex list from whole list
	cframe = min(max(cframe,0),m->numframes-1);
	if ((unsigned int)nframe >= (unsigned int)m->numframes) nframe = cframe;
	f0 = (md2frame_t *)&m->frames[cframe*m->framebytes];
	f1 = (md2frame_t *)&m->frames[nframe*m->framebytes];
	f = interp; g = 1-f;
	m0.x = f0->mul.x*m->scale*g; m1.x = f1->mul.x*m->scale*f;
	m0.y = f0->mul.y*m->scale*g; m1.y = f1->mul.y*m->scale*f;
	m0.z = f0->mul.z*m->scale*g; m1.z = f1->mul.z*m->scale*f;
	a0.x = f0->add.x*m->scale; a0.x = (f1->add.x*m->scale-a0.x)*f+a0.x;
	a0.y = f0->add.y*m->scale; a0.y = (f1->add.y*m->scale-a0.y)*f+a0.y;
	a0.z = f0->add.z*m->scale; a0.z = (f1->add.z*m->scale-a0.z)*f+a0.z;
	c0 = &f0->verts[0].v[0]; c1 = &f1->verts[0].v[0];
	m0.x = -m0.x; m1.x = -m1.x; a0.x = -a0.x;

	f = 1*32.0;
	m0.x *= f; m0.y *= f; m0.z *= f;
	m1.x *= f; m1.y *= f; m1.z *= f;
	a0.x *= f; a0.y *= f; a0.z *= f;

	for(i=m->numverts-1;i>=0;i--) //interpolate&scale vertices to avoid ugly animation
	{
		vertlist[i].z = c0[(i<<2)+0]*m0.x + c1[(i<<2)+0]*m1.x + a0.x;
		vertlist[i].y = c0[(i<<2)+2]*m0.z + c1[(i<<2)+2]*m1.z + a0.z;
		vertlist[i].x = c0[(i<<2)+1]*m0.y + c1[(i<<2)+1]*m1.y + a0.y;
	}

	for(iptr=m->glcmds;i=(*iptr++);)
	{
		if (i < 0) { i = -i; trit = 0; /*TRI_FAN*/ } else { trit = 1; /*TRI_STRIP*/ }

		if ((unsigned int)(numverts+i) >= maxverts)
		{
			maxverts = max(maxverts+(maxverts>>1),numverts+i);
#if (PRINTALLOC == 1)
			printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
			if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
		}
		j = i; onumverts = numverts;
		for(;i>0;i--,iptr+=3)
		{
			vert[numverts].x =-vertlist[iptr[2]].z;
			vert[numverts].y = vertlist[iptr[2]].x;
			vert[numverts].z = vertlist[iptr[2]].y;
			vert[numverts].u = (((float *)iptr)[0]);
			vert[numverts].v = 1-(((float *)iptr)[1]);
			numverts++;
		}

		if ((unsigned int)(numtris+j-2) >= maxtris)
		{
			maxtris = max(maxtris+(maxtris>>1),numtris+j-2);
#if (PRINTALLOC == 1)
			printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
			if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
		}

		for(i=2;i<j;i++)
		{
			if (!trit)
			{
				tri[numtris].x = onumverts; //TRI_FAN
				tri[numtris].y = onumverts+i-1;
				tri[numtris].z = onumverts+i;
			}
			else
			{
				tri[numtris].x = onumverts+i-2+(i&1); //TRI_STRIP
				tri[numtris].y = onumverts+i-1-(i&1);
				tri[numtris].z = onumverts+i;
			}
			tri[numtris].i = 0;
			numtris++;
		}
	}
}

//--------------------------------------  MD2 LIBRARY ENDS -----------------------------------------
//-------------------------------------  MD3 LIBRARY BEGINS ----------------------------------------

typedef struct { char nam[64]; int i; } md3shader_t; //ascz path of shader, shader index
typedef struct { int i[3]; } md3tri_t; //indices of tri
typedef struct { float u, v; } md3uv_t;
typedef struct { signed short x, y, z; unsigned char nlat, nlng; } md3xyzn_t; //xyz are [10:6] ints

typedef struct
{
	point3d min, max, cen; //bounding box&origin
	float r; //radius of bounding sphere
	char nam[16]; //ascz frame name
} md3frame_t;

typedef struct
{
	char nam[64]; //ascz tag name
	point3d p, x, y, z; //tag object pos&orient
} md3tag_t;

typedef struct
{
	int id; //IDP3(0x33806873)
	char nam[64]; //ascz surface name
	int flags; //?
	int numframes, numshaders, numverts, numtris; //numframes same as md3head,max shade=~256,vert=~4096,tri=~8192
	md3tri_t *tris;       //file format: rel offs from md3surf
	md3shader_t *shaders; //file format: rel offs from md3surf
	md3uv_t *uv;          //file format: rel offs from md3surf
	md3xyzn_t *xyzn;      //file format: rel offs from md3surf
	int ofsend;
} md3surf_t;

typedef struct
{
	int id, vers; //id=IDP3(0x33806873), vers=15
	char nam[64]; //ascz path in PK3
	int flags; //?
	int numframes, numtags, numsurfs, numskins; //max=~1024,~16,~32,numskins=artifact of MD2; use shader field instead
	md3frame_t *frames; //file format: abs offs
	md3tag_t *tags;     //file format: abs offs
	md3surf_t *surfs;   //file format: abs offs
	int eof;           //file format: abs offs
} md3head_t;

typedef struct
{
	int mdnum; //MD2=2, MD3=3. NOTE: must be first in structure!
	md3head_t head;
	int frame0, frame1, cframe, nframe, fpssc, usesalpha;
	float oldtime, curtime, interpol, scale;
	unsigned int texid;
} md3model;

static int md3load (md3model *m, const char *filnam)
{
	char st[MAX_PATH+2], bst[MAX_PATH+2];
	int i, j, surfi, ofsurf, maxverts, bsc;
	md3surf_t *s;

#ifdef _WIN64
	printf("Sorry, MD3 loader broken in 64-bit mode due to pointers in md3*_t structs :/\n"); return(0);
#endif

	if (!kzopen(filnam)) return(0);

	memset(m,0,sizeof(md3model)); m->mdnum = 3;
	m->texid = 0; m->scale = .01;

	kzread(&m->head,sizeof(md3head_t));
	if ((m->head.id != 0x33504449) && (m->head.vers != 15)) { kzclose(); return(0); } //"IDP3"

	ofsurf = (int)m->head.surfs; maxverts = 0;

	kzseek((int)m->head.frames,SEEK_SET); i = m->head.numframes*sizeof(md3frame_t);
	m->head.frames = (md3frame_t *)malloc(i); if (!m->head.frames) { kzclose(); return(0); }
	kzread(m->head.frames,i);

	kzseek((int)m->head.tags,SEEK_SET); i = m->head.numtags*sizeof(md3tag_t);
	m->head.tags = (md3tag_t *)malloc(i); if (!m->head.tags) { kzclose(); return(0); }
	kzread(m->head.tags,i);

	kzseek((int)m->head.surfs,SEEK_SET); i = m->head.numsurfs*sizeof(md3surf_t);
	m->head.surfs = (md3surf_t *)malloc(i); if (!m->head.surfs) { kzclose(); return(0); }
	for(surfi=0;surfi<m->head.numsurfs;surfi++)
	{
		s = &m->head.surfs[surfi];
		kzseek(ofsurf,SEEK_SET); kzread(s,sizeof(md3surf_t));

		kzseek(ofsurf+((int)(s->tris)),SEEK_SET); i = s->numtris*sizeof(md3tri_t);
		s->tris = (md3tri_t *)malloc(i); if (!s->tris) { kzclose(); return(0); }
		kzread(s->tris,i);

		kzseek(ofsurf+((int)(s->shaders)),SEEK_SET); i = s->numshaders*sizeof(md3shader_t);
		s->shaders = (md3shader_t *)malloc(i); if (!s->shaders) { kzclose(); return(0); }
		kzread(s->shaders,i);

		kzseek(ofsurf+((int)(s->uv)),SEEK_SET); i = s->numverts*sizeof(md3uv_t);
		s->uv = (md3uv_t *)malloc(i); if (!s->uv) { kzclose(); return(0); }
		kzread(s->uv,i);

		kzseek(ofsurf+((int)(s->xyzn)),SEEK_SET); i = s->numframes*s->numverts*sizeof(md3xyzn_t);
		s->xyzn = (md3xyzn_t *)malloc(i); if (!s->xyzn) { kzclose(); return(0); }
		kzread(s->xyzn,i);

		if (s->numverts > maxverts) maxverts = s->numverts;
		ofsurf += s->ofsend;
	}
	kzclose();

	m->frame0 = 0; m->frame1 = m->head.numframes;
	m->cframe = 0; m->nframe = m->cframe+1; if (m->nframe >= m->head.numframes) m->nframe = 0;
	m->fpssc = 8;

	strcpy(st,filnam);
	for(i=0,j=0;st[i];i++) if ((st[i] == '/') || (st[i] == '\\')) j = i+1;
	st[j] = '*'; st[j+1] = 0;

	kzfindfilestart(st); bsc = -1; bst[0] = 0;
	while (kzfindfile(st))
	{
		if (st[0] == '\\') continue;

		for(i=0,j=0;st[i];i++) if (st[i] == '.') j = i+1;
		if ((!stricmp(&st[j],"JPG")) || (!stricmp(&st[j],"PNG")) || (!stricmp(&st[j],"GIF")) ||
			 (!stricmp(&st[j],"PCX")) || (!stricmp(&st[j],"TGA")) || (!stricmp(&st[j],"BMP")) ||
			 (!stricmp(&st[j],"CEL")))
		{
			for(i=0;st[i];i++) if (st[i] != filnam[i]) break;
			if (i > bsc) { bsc = i; strcpy(bst,st); }
		}
	}
	//if (!mdloadskin(&m->texid,&m->usesalpha,bst)) ;//bad!
	pic[0].namoff = namplc; numpics = 1; i = strlen(bst);
	if (namplc+i >= maxnamsiz)
	{
		maxnamsiz = max(maxnamsiz<<1,namplc+i+1);
#if (PRINTALLOC == 1)
		printf("maxnamsiz=%d (%dKB)\n",maxnamsiz,(maxnamsiz*1)>>10);
#endif
		if (!(nambuf = (char *)realloc(nambuf,sizeof(char)*maxnamsiz))) quitout("realloc failed: nambuf");
	}
	memcpy(&nambuf[namplc],bst,i+1); namplc += i+1;
	colrgb[0] = -1;

	vertlist = (point3d *)malloc(maxverts*sizeof(point3d));
	return(1);
}

static void md3draw (md3model *m, int cframe, int nframe, float interp)
{
	point3d p0, p1, m0, m1, a0, a1;
	md3frame_t *f0, *f1;
	md3xyzn_t *v0, *v1;
	int i, j, k, surfi;
	float f, g;
	md3surf_t *s;

	cframe = min(max(cframe,0),m->head.numframes-1);
	if ((unsigned int)nframe >= (unsigned int)m->head.numframes) nframe = cframe;

		//create current&next frame's vertex list from whole list
	f0 = (md3frame_t *)&m->head.frames[cframe];
	f1 = (md3frame_t *)&m->head.frames[nframe];
	f = interp; g = 1-f;
	m0.x = (1.0/64.0)*m->scale*g; m1.x = (1.0/64.0)*m->scale*f;
	m0.y = (1.0/64.0)*m->scale*g; m1.y = (1.0/64.0)*m->scale*f;
	m0.z = (1.0/64.0)*m->scale*g; m1.z = (1.0/64.0)*m->scale*f;
	a0.x = f0->cen.x*m->scale; a0.x = (f1->cen.x*m->scale-a0.x)*f+a0.x;
	a0.y = f0->cen.y*m->scale; a0.y = (f1->cen.y*m->scale-a0.y)*f+a0.y;
	a0.z = f0->cen.z*m->scale; a0.z = (f1->cen.z*m->scale-a0.z)*f+a0.z;
	m0.x = -m0.x; m1.x = -m1.x; a0.x = -a0.x;

	f = 1*32.0;
	m0.x *= f; m0.y *= f; m0.z *= f;
	m1.x *= f; m1.y *= f; m1.z *= f;
	a0.x *= f; a0.y *= f; a0.z *= f;

	for(surfi=0;surfi<m->head.numsurfs;surfi++)
	{
		s = &m->head.surfs[surfi];
		v0 = &s->xyzn[cframe*s->numverts];
		v1 = &s->xyzn[nframe*s->numverts];
		for(i=s->numverts-1;i>=0;i--) //interpolate&scale vertices to avoid ugly animation
		{
			vertlist[i].z = ((float)v0[i].x)*m0.x + ((float)v1[i].x)*m1.x + a0.x;
			vertlist[i].y = ((float)v0[i].z)*m0.z + ((float)v1[i].z)*m1.z + a0.z;
			vertlist[i].x = ((float)v0[i].y)*m0.y + ((float)v1[i].y)*m1.y + a0.y;
		}

		i = s->numtris*3;
		if ((unsigned int)(numverts+i) >= maxverts)
		{
			maxverts = max(maxverts+(maxverts>>1),numverts+i);
#if (PRINTALLOC == 1)
			printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
			if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
		}

		i = s->numtris;
		if ((unsigned int)(numtris+i) >= maxtris)
		{
			maxtris = max(maxtris+(maxtris>>1),numtris+i);
#if (PRINTALLOC == 1)
			printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
			if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
		}

		//if (m->usesalpha) ???

		for(i=s->numtris-1;i>=0;i--)
		{
			tri[numtris].x = numverts;
			tri[numtris].y = numverts+1;
			tri[numtris].z = numverts+2;
			tri[numtris].i = 0;
			numtris++;

			for(j=0;j<3;j++)
			{
				k = s->tris[i].i[j];
				vert[numverts].x =-vertlist[k].z;
				vert[numverts].y = vertlist[k].x;
				vert[numverts].z = vertlist[k].y;
				vert[numverts].u = s->uv[k].u;
				vert[numverts].v = 1-s->uv[k].v;
				numverts++;
			}
		}
	}

#if 0
		//precalc:
	float sinlut256[256+(256>>2)];
	for(i=0;i<sizeof(sinlut256)/sizeof(sinlut256[0]);i++) sinlut256[i] = sin(((float)i)*(PI*2/255.0));

		//normal to xyz:
	md3vert_t *mv = &md3vert[?];
	z = sinlut256[mv->nlng+(256>>2)];
	x = sinlut256[mv->nlat]*z;
	y = sinlut256[mv->nlat+(256>>2)]*z;
	z = sinlut256[mv->nlng];
#endif
}

static void md3free (md3model *m)
{
	md3surf_t *s;
	int surfi;

	//if (gtexmalloc) { free((void *)gtexmalloc); gtexmalloc = 0; }
	if (vertlist) { free(vertlist); vertlist = 0; }
	//glDeleteLists(m->texid,1);
	if (m->head.surfs)
	{
		for(surfi=m->head.numsurfs-1;surfi>=0;surfi--)
		{
			s = &m->head.surfs[surfi];
			if (s->xyzn) free(s->xyzn);
			if (s->uv) free(s->uv);
			if (s->shaders) free(s->shaders);
			if (s->tris) free(s->tris);
		}
		free(m->head.surfs);
	}
	if (m->head.tags) free(m->head.tags);
	if (m->head.frames) free(m->head.frames);
}

//--------------------------------------  MD3 LIBRARY ENDS -----------------------------------------
//-------------------------------------  OBJ LIBRARY BEGINS ----------------------------------------

#if 0
//Example .OBJ file shown between (and not including) the dashes (.. to skip sections):
//--------------------------------------------------------
# 3ds Max Wavefront OBJ Exporter v0.97b - (c)2007 guruware
# File Created: 26.08.2013 22:45:58

mtllib ogre0.mtl

#
# object Ogre_Geo_Body
#

v  -3.1161 96.4567 9.9431
v  -2.7476 95.1829 10.1754
..
v  -0.8580 92.9942 9.3052
v  -0.8644 93.0095 9.3045
# 7935 vertices

vn -0.3295 0.0763 0.9411
vn -0.1819 0.1965 0.9635
..
vn 0.4060 0.7465 0.5272
vn 0.2687 0.8893 0.3700
# 15391 vertex normals

vt 0.4914 0.9635 0.7656
vt 0.4948 0.9509 0.7714
..
vt 0.4875 0.9292 0.7497
vt 0.4876 0.9293 0.7497
# 7629 texture coords

g Ogre_Geo_Body
usemtl Ogre_Body
s off
f 1/1/1 2/2/1 3/3/1
f 3/3/2 4/4/2 1/1/2
..
f 7670/7562/14753 7650/7542/14719 7619/7511/14722
f 7619/7511/14722 7640/7532/14754 7670/7562/14753
s 1
f 7671/7563/14755 7672/7564/14756 7673/7565/14757
f 7673/7565/14757 7674/7566/14758 7671/7563/14755
..
f 7923/704/15390 7881/360/15390 7927/360/15390
f 7927/360/15391 7928/708/15391 7923/704/15391
# 15742 faces
//--------------------------------------------------------
#endif

static char glightmodchar = 0;
static int loadobj (char *fnam)
{
	#define OBJMAXVERTS 256
	int i, j, k, pass, leng, fleng, offs[3], ind[OBJMAXVERTS*3], tbufr, curmat;
	unsigned char tbuf[4096];
	char mtlfilnam[MAX_PATH] = "", fnam2[MAX_PATH+4];

	if (numpics) colrgb[0] = -1; else colrgb[0] = 0x808080;
	matnum = 1; curmat = 0;

	for(i=0;i<3;i++) offs[i] = -1;

	if (!kzopen(fnam)) return(0);
	printf("Reading %s\n",fnam);
	fleng = kzfilelength();

	for(pass=0;pass<3;pass++)
	{
		kzseek(0,SEEK_SET); tbufr = 0;

		offs[pass] = numverts;
		while (1)
		{
				//Read 1 line and remove spaces at beginning...
			leng = -1;
			while (1)
			{
#if 0
				j = kzgetc(); if (j < 0) goto file_eof;
#else
					//Much faster than above!
				if (!(tbufr&(sizeof(tbuf)-1))) kzread(tbuf,sizeof(tbuf));
				if (tbufr >= fleng) goto file_eof;
				j = tbuf[tbufr&(sizeof(tbuf)-1)]; tbufr++;
#endif
				if ((j < 32) && (leng >= 0)) { tempbuf[leng] = 0; break; }
				if ((j > 32) && (leng == -1)) leng = 0;
				if ((leng >= 0) && (leng < sizeof(tempbuf)-1)) { tempbuf[leng] = (unsigned char)j; leng++; }
			}

			if ((tempbuf[0] == '#') || (!tempbuf[0])) continue;

				//vertices (v), vertex normals (vn), and vertex texture coords (vt) go on same list
				//then new list for faces (triangles) with x,y,z,u,v all together is generated later
			if ((tempbuf[0] == 'v') && ((tempbuf[1] == ' ') || ((tempbuf[1] == 't') && (tempbuf[2] == ' '))))
			{
					  if (tempbuf[1] == ' ') { if (pass != 0) continue; i = 2; } //vertices
				else if (tempbuf[1] == 't') { if (pass != 1) continue; i = 3; } //texture coords

				if ((unsigned int)numverts >= maxverts)
				{
					maxverts += (maxverts>>1);
#if (PRINTALLOC == 1)
					printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
					if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
				}

				sscanf(&tempbuf[i],"%f %f %f",&vert[numverts].x,&vert[numverts].y,&vert[numverts].z);
				numverts++;
			}
			else if ((tempbuf[0] == 'f') && (tempbuf[1] == ' '))
			{
				if (pass != 2) continue;

				if ((unsigned int)(numverts+OBJMAXVERTS) >= maxverts)
				{
					maxverts = max(maxverts+(maxverts>>1),numverts+OBJMAXVERTS);
#if (PRINTALLOC == 1)
					printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
					if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
				}
				if ((unsigned int)(numtris+OBJMAXVERTS) >= maxtris)
				{
					maxtris = max(maxtris+(maxtris>>1),numtris+OBJMAXVERTS);
#if (PRINTALLOC == 1)
					printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
					if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
				}

				j = 0; k = 0;
				do
				{
					while ((k < leng) && (tempbuf[k] != ' ')) { k++; } if (k >= leng) break;
					while ((k < leng) && (tempbuf[k] == ' ')) { k++; } if (k >= leng) break;
					for(i=0;i<3;i++) ind[j*3+i] = 0;
					for(i=0;i<3;i++)
					{
						while ((k < leng) && (tempbuf[k] >= '0') && (tempbuf[k] <= '9'))
							{ ind[j*3+i] = ind[j*3+i]*10 + (tempbuf[k]-'0'); k++; }
						if (tempbuf[k] != '/') break;
						k++;
					}
					j++; if (j >= OBJMAXVERTS) { printf("Warning: limiting face to %d vertices\n",OBJMAXVERTS); break; }
				} while (1);

				for(i=j*3-1;i>=0;i--) ind[i]--;
				for(i=j-1;i>=0;i--)
				{
					vert[i+numverts].x = vert[ind[i*3+0]+offs[0]].z;
					vert[i+numverts].y = vert[ind[i*3+0]+offs[0]].x;
					vert[i+numverts].z = vert[ind[i*3+0]+offs[0]].y;
					vert[i+numverts].u = vert[ind[i*3+1]+offs[1]].x; //u
					vert[i+numverts].v = vert[ind[i*3+1]+offs[1]].y; //v
				}
				for(i=1;i<j-1;i++)
				{
					tri[numtris].x = numverts+0;
					tri[numtris].y = numverts+i;
					tri[numtris].z = numverts+i+1;
					tri[numtris].i = curmat;
					numtris++;
				}
				numverts += j;
			}
			else if ((tempbuf[0] == 'u') && (tempbuf[1] == 's') && (tempbuf[2] == 'e') && (tempbuf[3] == 'm') && (tempbuf[4] == 't') && (tempbuf[5] == 'l') && (tempbuf[6] == ' ') && (pass == 2))
			{
				curmat = matstring2num(&tempbuf[7]);
			}
			else if ((tempbuf[0] == 'm') && (tempbuf[1] == 't') && (tempbuf[2] == 'l') && (tempbuf[3] == 'l') && (tempbuf[4] == 'i') && (tempbuf[5] == 'b') && (tempbuf[6] == ' ') && (pass == 2))
			{
				strcpy(mtlfilnam,&tempbuf[7]);
			}
		}
file_eof:;
	}
	kzclose();

		//Delete temp memory in verts[] used for other purposes; necessary for proper scaling&centering!
	for(i=numtris-1;i>=0;i--) { tri[i].x -= offs[2]; tri[i].y -= offs[2]; tri[i].z -= offs[2]; }
	for(i=offs[2];i<numverts;i++) { vert[i-offs[2]] = vert[i]; }
	numverts -= offs[2];

	//-----------------------------------------------------------------------------------------------

	for(i=1;i<matnum;i++) colrgb[i] = ((rand()&127)+64)+(((rand()&127)+64)<<8)+(((rand()&127)+64)<<16);

		//OBJ:
		//----
		//mtllib men0.mtl
		//usemtl wire_188188188
		//usemtl female0_1380003812_mat001
		//usemtl male0_1380003812_mat001
		//
		//
		//MTL:
		//----
		//#comment
		//newmtl wire_188188188
		//   Ka 0.7373 0.7373 0.7373
		//newmtl female0_1380003812_mat001
		//   Ka 0.5880 0.5880 0.5880
		//   map_Ka C:\xyz\female0_1380003812_color.jpg

	printf("Reading %s..",mtlfilnam);
	if (!kzopen(mtlfilnam))
	{
		int slash0, slash1;
		for(i=strlen(fnam     )-1,slash0=0;i>=0;i--) if ((fnam     [i] == '\\') || (fnam     [i] == '/')) { slash0 = i+1; break; }
		for(i=strlen(mtlfilnam)-1,slash1=0;i>=0;i--) if ((mtlfilnam[i] == '\\') || (mtlfilnam[i] == '/')) { slash1 = i+1; break; }
		strcpy(fnam2,fnam);
		strcpy(&fnam2[slash0],&mtlfilnam[slash1]);
		printf("\nReading %s..",fnam2);
		if (!kzopen(fnam2)) { printf(" NOT FOUND\n"); return(1); }
	}
	printf(".. found\n");

	fleng = kzfilelength();
	tbufr = 0; curmat = 0;
	while (1)
	{
			//Read 1 line and remove spaces at beginning...
		leng = -1;
		while (1)
		{
#if 0
			j = kzgetc(); if (j < 0) goto file_eof2;
#else
				//Much faster than above!
			if (!(tbufr&(sizeof(tbuf)-1))) kzread(tbuf,sizeof(tbuf));
			if (tbufr >= fleng) goto file_eof2;
			j = tbuf[tbufr&(sizeof(tbuf)-1)]; tbufr++;
#endif
			if ((j < 32) && (leng >= 0)) { tempbuf[leng] = 0; break; }
			if ((j > 32) && (leng == -1)) leng = 0;
			if ((leng >= 0) && (leng < sizeof(tempbuf)-1)) { tempbuf[leng] = (unsigned char)j; leng++; }
		}

		if ((tempbuf[0] == '#') || (!tempbuf[0])) continue;

		if ((tempbuf[0] == 'n') && (tempbuf[1] == 'e') && (tempbuf[2] == 'w') && (tempbuf[3] == 'm') && (tempbuf[4] == 't') && (tempbuf[5] == 'l') && (tempbuf[6] == ' '))
		{
			for(curmat=matnum-1;curmat>0;curmat--)
				if (!strcmp(&nambuf[matstr[curmat]],&tempbuf[7]))
					break;
		}

		else if ((tempbuf[0] == 'K') && (((!glightmodchar) && (tempbuf[1] !=           ' ')) ||
													(( glightmodchar) && (tempbuf[1] == glightmodchar))) && (tempbuf[2] == ' '))
		{
			if (curmat > 0)
			{
				float fr, fg, fb;
				sscanf(&tempbuf[3],"%f %f %f",&fr,&fg,&fb);
				colrgb[curmat] = (min(max(cvttss2si(fr*255.f),0),255)<<16) +
									  (min(max(cvttss2si(fg*255.f),0),255)<< 8) +
									  (min(max(cvttss2si(fb*255.f),0),255)    );
			}
		}
		else if ((tempbuf[0] == 'm') && (tempbuf[1] == 'a') && (tempbuf[2] == 'p') && (tempbuf[3] == '_') && (tempbuf[4] == 'K') && (((!glightmodchar) && (tempbuf[5] !=           ' ')) ||
																																											  (( glightmodchar) && (tempbuf[5] == glightmodchar))) && (tempbuf[6] == ' '))
		{
			if (curmat > 0) { i = picstring2num(&tempbuf[7]); if (i >= 0) colrgb[curmat] = ~i; }
		}
	}
file_eof2:;
	kzclose();

	return(1);
}
//--------------------------------------  OBJ LIBRARY ENDS -----------------------------------------
//-------------------------------------  STL LIBRARY BEGINS ----------------------------------------
	//   //STL binary format:
	//char filler[80];
	//unsigned int numtris;
	//for(i=0;i<numtris;i++)
	//{
	//   point3d norm, v[3]; //vertices are CCW and must be + coords
	//   short filler;
	//}
static int loadstl (char *fnam)
{
	float fx, fy, fz, xmin, ymin, zmin, xmax, ymax, zmax;
	int i, j;
	short s;
	char buf[50];

	if (numpics) colrgb[0] = -1; else colrgb[0] = 0x808080;
	matnum = 1;

	if (!kzopen(fnam)) return(0);
	printf("Reading %s\n",fnam);
	kzseek(80,SEEK_SET);
	kzread(&numtris,4);

	if ((unsigned int)(numtris*3) > maxverts)
	{
		maxverts = numtris*3;
#if (PRINTALLOC == 1)
		printf("maxverts=%d (%dKB)\n",maxverts,(maxverts*20)>>10);
#endif
		if (!(vert = (vertyp *)realloc(vert,sizeof(vertyp)*maxverts))) quitout("realloc failed: vert");
	}
	if ((unsigned int)(numtris) > maxtris)
	{
		maxtris = numtris;
#if (PRINTALLOC == 1)
		printf("maxtris=%d (%dKB)\n",maxtris,(maxtris*16)>>10);
#endif
		if (!(tri = (trityp *)realloc(tri,sizeof(trityp)*maxtris))) quitout("realloc failed: tri");
	}

	xmin = 1e32; ymin = 1e32; zmin = 1e32;
	xmax =-1e32; ymax =-1e32; zmax =-1e32;

	for(i=0;i<numtris;i++)
	{
		kzread(buf,50);

		for(j=3-1;j>=0;j--)
		{
			fx =  (*(float *)&buf[j*12+16]); if (fx < xmin) xmin = fx; if (fx > xmax) xmax = fx;
			fy =  (*(float *)&buf[j*12+12]); if (fy < ymin) ymin = fy; if (fy > ymax) ymax = fy;
			fz = -(*(float *)&buf[j*12+20]); if (fz < zmin) zmin = fz; if (fz > zmax) zmax = fz;
			vert[j+numverts].x = fx;
			vert[j+numverts].y = fy;
			vert[j+numverts].z = fz;
			vert[j+numverts].u = 0;
			vert[j+numverts].v = 0;
		}
		tri[i].x = numverts+0;
		tri[i].y = numverts+1;
		tri[i].z = numverts+2;

		s = *(short *)&buf[48];
		if (!(s&0x8000))
		{
			tri[i].i = 0; //no color hack in STL
		}
		else
		{
			if (matnum >= maxmat)
			{
				maxmat <<= 1;
				if (!(colrgb = (int *)realloc(colrgb,sizeof(int)*maxmat))) quitout("realloc failed: colrgb");
			}
			tri[i].i = matnum;
			colrgb[matnum] = (((s>>10)&31)<<19) + (((s>>5)&31)<<11) + ((s&31)<<3); matnum++;
		}

		numverts += 3;
	}
	kzclose();

	fx = (xmin+xmax)*.5;
	fy = (ymin+ymax)*.5;
	fz = (zmin+zmax)*.5;
	for(i=numverts-1;i>=0;i--) { vert[i].x -= fx; vert[i].y -= fy; vert[i].z -= fz; }
	return(1);
}
//--------------------------------------  STL LIBRARY ENDS -----------------------------------------


static int gcframe = 0, gnframe = -1;
static float ginterp = 0.f;
static int loadmd2 (char *fnam)
{
	md2model mymodel;

	printf("Reading %s\n",fnam);
	if (!md2load(&mymodel,fnam)) return(0);
	md2draw(&mymodel,gcframe,gnframe,ginterp);
	md2free(&mymodel);
	return(1);
}

static int loadmd3 (char *fnam)
{
	md3model mymodel;
	printf("Reading %s\n",fnam);
	if (!md3load(&mymodel,fnam)) return(0);
	md3draw(&mymodel,gcframe,gnframe,ginterp);
	md3free(&mymodel);
	return(1);
}

static void hollowfix (int xsiz, int ysiz, int zsiz)
{
	int i, x, y, z;

		//Set all seeable 0's to 1's
	if (savetype != 3)
	{
		for(x=0;x<xsiz;x++)
			for(y=0;y<ysiz;y++)
				{ floodfill3dbits(x,y,0); floodfill3dbits(x,y,zsiz-1); }
		for(x=0;x<xsiz;x++)
			for(z=0;z<zsiz;z++)
				{ floodfill3dbits(x,0,z); floodfill3dbits(x,ysiz-1,z); }
		for(y=0;y<ysiz;y++)
			for(z=0;z<zsiz;z++)
				{ floodfill3dbits(0,y,z); floodfill3dbits(xsiz-1,y,z); }
	}
	else
	{
		for(x=0;x<xsiz;x++)
			for(y=0;y<ysiz;y++)
				floodfill3dbits(x,y,0);
	}

		//Invert all solid & air
	for(x=0;x<xsiz;x++)
		for(y=0;y<ysiz;y++)
			vox_xorzrange(x,y,0,zsiz);

		//Make surface voxels solid again
	for(i=hashcnt-1;i>=0;i--)
	{
		x = hashdat[i].x; if ((x < gclipx0) || (x >= gclipx1)) continue;
		y = hashdat[i].y; if ((y < gclipy0) || (y >= gclipy1)) continue;
		z = hashdat[i].z; if ((z < gclipz0) || (z >= gclipz1)) continue;
		vox_set(x,y,z);
	}
}

	//Note: only cuts off right/front/down side :/
static void compact_siz (void)
{
	int i, x, y, z, z0, z1, xmin, ymin, zmin, xmax, ymax, zmax, nxsiz, nysiz, nzsiz;

		//First find extents of solid
	xmin = xsiz; xmax = 0;
	ymin = ysiz; ymax = 0;
	zmin = zsiz; zmax = 0;
	for(x=0;x<xsiz;x++)
		for(y=0;y<ysiz;y++)
		{
			z0 = vox_dntil1(x,y,   0); if (z0 >= zsiz) continue;
			z1 = vox_uptil1(x,y,zsiz);
			xmin = min(xmin,x ); xmax = max(xmax,x );
			ymin = min(ymin,y ); ymax = max(ymax,y );
			zmin = min(zmin,z0); zmax = max(zmax,z1);
		}

	printf("x:%d..%d, y:%d..%d, z:%d..%d\n",xmin,xmax,ymin,ymax,zmin,zmax);
	nxsiz = xmax-xmin+1; nysiz = ymax-ymin+1; nzsiz = zmax-zmin+1;
	if ((nxsiz <= 0) || (nysiz <= 0) || (nzsiz <= 0)) { xsiz = 0; ysiz = 0; zsiz = 0; return; }

	if (xmin|ymin|zmin)
	{
		xpiv -= (float)xmin; ypiv -= (float)ymin; zpiv -= (float)zmin;
		ghashofx = xmin; ghashofy = ymin; ghashofz = zmin;

			//Move voxel solids to corner
		for(x=0;x<nxsiz;x++)
			for(y=0;y<nysiz;y++)
				vox_copyslabxy_shiftupz(x,y,x+xmin,y+ymin,zmin);
	}

	xsiz = nxsiz; ysiz = nysiz; zsiz = nzsiz;
}

static void getpal (void)
{
	int i, j, k, x, y, z;

	if (picsloaded > 0)
	{
		if (palfilnam[0]) i = kzopen(palfilnam); else i = 0;
		if (i)
		{
			for(i=0;i<256;i++)
			{
				rpal[i] = (kzgetc()<<2);
				gpal[i] = (kzgetc()<<2);
				bpal[i] = (kzgetc()<<2);
			}
			kzclose();

			j = 0;
			for(i=64;i>=0;i--)
			{
				//j = (i-64)*(i-64);
				rdist[i] = rdist[128-i] = j*30;
				gdist[i] = gdist[128-i] = j*59;
				bdist[i] = bdist[128-i] = j*11;
				j += 129-(i<<1);
			}

			memset(closestcol,-1,sizeof(closestcol));
		}
		else
		{
				//Generate frequency distribution
			memset(palfreq,0,sizeof(palfreq));
			for(x=0;x<xsiz;x++)
				for(y=0;y<ysiz;y++)
					for(z=vox_dntil1(x,y,0);z<zsiz;z=vox_dntil1(x,y,z+1)) //find surface voxels
					{
						j = 0;
						if ((x == 0)      || (!vox_test(x-1,y,z))) j |= 1;
						if ((x == xsiz-1) || (!vox_test(x+1,y,z))) j |= 2;
						if ((y == 0)      || (!vox_test(x,y-1,z))) j |= 4;
						if ((y == ysiz-1) || (!vox_test(x,y+1,z))) j |= 8;
						if ((z == 0)      || (!vox_test(x,y,z-1))) j |= 16;
						if ((z == zsiz-1) || (!vox_test(x,y,z+1))) j |= 32;
						if (!j) continue;

						k = getpix32(x,y,z);
						palfreq[0][0][((k>>6)&0x3f000)+((k>>4)&0xfc0)+((k>>2)&0x3f)]++;
					}

			genpal();
		}
	}
	else if (matnum <= 1)
	{
		colrgb[0] = 192+32;
	}
}

static void writepal (FILE *fil)
{
	int i, j, k, x, y, z;

	if (picsloaded > 0)
	{
			//Store close-fit palette (object has textures)
		for(i=0;i<256;i++)
		{
			fputc(rpal[i]>>2,fil);
			fputc(gpal[i]>>2,fil);
			fputc(bpal[i]>>2,fil);
		}
	}
	else
	{
			//Generate interesting color palette
		if (matnum <= 1)
		{
			for(z=0;z<64;z++) { fputc(z,fil); fputc(0,fil); fputc(0,fil); }
			for(z=0;z<64;z++) { fputc(0,fil); fputc(z,fil); fputc(0,fil); }
			for(z=0;z<64;z++) { fputc(0,fil); fputc(0,fil); fputc(z,fil); }
			for(z=0;z<64;z++) { fputc(z,fil); fputc(z,fil); fputc(z,fil); }
		}
		else
		{
			for(i=0;i<matnum;i++)
			{
				fputc((colrgb[i]>>18)&63,fil);
				fputc((colrgb[i]>>10)&63,fil);
				fputc((colrgb[i]>> 2)&63,fil);
			}
				//Generate darker colors for palette
			k = ((256*2+i*1)/3);
			j = 1; y = (k-matnum)/(matnum-1); x = y-1;
			for(;i<k;i++)
			{
				if (!y) { fputc(0,fil); fputc(0,fil); fputc(0,fil); continue; }
				fputc(max((((colrgb[j]>>16)&255)*x)/(y<<2),0),fil);
				fputc(max((((colrgb[j]>> 8)&255)*x)/(y<<2),0),fil);
				fputc(max((((colrgb[j]    )&255)*x)/(y<<2),0),fil);
				j++; if (j >= matnum) { j = 1; x--; }
			}
				//Generate brighter colors for palette
			y = (256-i)/(matnum-1); x = y+1;
			for(;i<256;i++)
			{
				if (!y) { fputc(0,fil); fputc(0,fil); fputc(0,fil); continue; }
				fputc(min((((colrgb[j]>>16)&255)*x)/(y<<2),63),fil);
				fputc(min((((colrgb[j]>> 8)&255)*x)/(y<<2),63),fil);
				fputc(min((((colrgb[j]    )&255)*x)/(y<<2),63),fil);
				j++; if (j >= matnum) { j = 1; x++; }
			}
		}
	}
}

static void savevox (char *fnam)
{
	FILE *fil;
	int i, j, k, p, x, y, z;

	if (rendermode < 2) hollowfix(xsiz,ysiz,zsiz);
	compact_siz();
	printf("Writing %s (%dx%dx%d)\n",fnam,xsiz,ysiz,zsiz);

	getpal();

	if (!(fil = fopen(fnam,"wb"))) quitout("Could not write file");
	fwrite(&xsiz,4,1,fil);
	fwrite(&ysiz,4,1,fil);
	fwrite(&zsiz,4,1,fil);

	for(x=0;x<xsiz;x++)
		for(y=0;y<ysiz;y++)
			for(z=0;z<zsiz;z++)
			{
				if (!vox_test(x,y,z)) { fputc(255,fil); continue; }

				j = 0;
				if ((x == 0)      || (!vox_test(x-1,y,z))) j |= 1;
				if ((x == xsiz-1) || (!vox_test(x+1,y,z))) j |= 2;
				if ((y == 0)      || (!vox_test(x,y-1,z))) j |= 4;
				if ((y == ysiz-1) || (!vox_test(x,y+1,z))) j |= 8;
				if ((z == 0)      || (!vox_test(x,y,z-1))) j |= 16;
				if ((z == zsiz-1) || (!vox_test(x,y,z+1))) j |= 32;
				if (!j) { fputc(0,fil); continue; }

				fputc(getclosestcol(getpix32(x,y,z),255),fil);
			}

	writepal(fil);

	fclose(fil);
}

static void savekvx (char *fnam, int numips)
{
	FILE *fil;
	int i, j, k, l, x, y, z, o, p, m, oneupm, seekpos;
	int nxsiz, nysiz, nzsiz, vis, r, g, b, n, xx, yy, zz, xxx, yyy, zzz;
	unsigned char voxdat[MAZDIM+4];

	if (rendermode < 2) hollowfix(xsiz,ysiz,zsiz);
	compact_siz();
	printf("Writing %s (%dx%dx%d)\n",fnam,xsiz,ysiz,zsiz);

	getpal();

	nxsiz = xsiz; nysiz = ysiz; nzsiz = zsiz;

	if (!(fil = fopen(fnam,"wb"))) quitout("Could not write file");
	for(m=0;m<numips;m++)
	{
		oneupm = (1<<m);

		seekpos = ftell(fil);
		i = 0; fwrite(&i,4,1,fil);
		fwrite(&nxsiz,4,1,fil);
		fwrite(&nysiz,4,1,fil);
		fwrite(&nzsiz,4,1,fil);
		i = (((int)(xpiv*256.0))>>m); fwrite(&i,4,1,fil);
		i = (((int)(ypiv*256.0))>>m); fwrite(&i,4,1,fil);
		i = (((int)(zpiv*256.0))>>m); fwrite(&i,4,1,fil);
		i = ((nxsiz+1)<<2) + (((nysiz+1)*nxsiz)<<1); //xoffset+xyoffset table sizes
		fseek(fil,i,SEEK_CUR);

		for(x=0;x<nxsiz;x++)
		{
			xstart[x] = i;
			for(y=0;y<nysiz;y++)
			{
				xyoffs[x][y] = (short)(i-xstart[x]);
				j = 0;
				xxx = (x<<m); yyy = (y<<m);
				for(z=vox_dntil1(x,y,0);z<nzsiz;z=vox_dntil1(x,y,z+1))
				{
						//Get vis
					vis = 0;
					if ((x ==       0) || (!vox_test(x-1,y,z))) vis |= 1;
					if ((x == nxsiz-1) || (!vox_test(x+1,y,z))) vis |= 2;
					if ((y ==       0) || (!vox_test(x,y-1,z))) vis |= 4;
					if ((y == nysiz-1) || (!vox_test(x,y+1,z))) vis |= 8;
					if ((z ==       0) || (!vox_test(x,y,z-1))) vis |= 16;
					if ((z == nzsiz-1) || (!vox_test(x,y,z+1))) vis |= 32;
					if (!vis) continue;

						//Get col
					r = g = b = n = 0; zzz = (z<<m);
					for(xx=min(xxx+oneupm,xsiz)-1;xx>=xxx;xx--)
						for(yy=min(yyy+oneupm,ysiz)-1;yy>=yyy;yy--)
							for(zz=min(zzz+oneupm,zsiz)-1;zz>=zzz;zz--)
							{
								if (hash_get(xx,yy,zz) < 0) continue;
								o = getpix32(xx,yy,zz);
								r += ((o>>16)&255);
								g += ((o>> 8)&255);
								b += ( o     &255);
								n++;
							}
					if (n)
					{
						r = (r+(n>>1))/n;
						g = (g+(n>>1))/n;
						b = (b+(n>>1))/n;
					}

					if ((!j) || (z > voxdat[0]+voxdat[1]))
					{
						if (j) { fwrite(voxdat,j,1,fil); i += j; }
						voxdat[0] = z; voxdat[1] = voxdat[2] = 0; j = 3;
					}
					voxdat[1]++; voxdat[2] |= vis; voxdat[j++] = getclosestcol((r<<16)+(g<<8)+b,256);
				}
				if (j) { fwrite(voxdat,j,1,fil); i += j; }
			}
			xyoffs[x][y] = (short)(i-xstart[x]);
		}
		xstart[x] = i;

		j = ftell(fil)-seekpos-4;
		fseek(fil,seekpos,SEEK_SET);
		fwrite(&j,4,1,fil);
		fseek(fil,seekpos+28,SEEK_SET);
		fwrite(xstart,(nxsiz+1)<<2,1,fil);
		for(i=0;i<nxsiz;i++) fwrite(&xyoffs[i][0],(nysiz+1)<<1,1,fil);
		fseek(fil,seekpos+j+4,SEEK_SET);

		if (m < numips-1) //Halve voxel object in all dimensions (which corrupts it)
		{
			if (nxsiz&1)
			{
				for(y=0;y<nysiz;y++)
					for(z=0;z<nzsiz;z++)
						{ if (vox_test(nxsiz-1,y,z)) vox_set(nxsiz,y,z); else vox_clear(nxsiz,y,z); }
				nxsiz++;
			}
			if (nysiz&1)
			{
				for(x=0;x<nxsiz;x++)
					for(z=0;z<nzsiz;z++)
						{ if (vox_test(x,nysiz-1,z)) vox_set(x,nysiz,z); else vox_clear(x,nysiz,z); }
				nysiz++;
			}
			if (nzsiz&1)
			{
				for(x=0;x<nxsiz;x++)
					for(y=0;y<nysiz;y++)
						{ if (vox_test(x,y,nzsiz-1)) vox_set(x,y,nzsiz); else vox_clear(x,y,nzsiz); }
				nzsiz++;
			}

			nxsiz >>= 1; nysiz >>= 1; nzsiz >>= 1;
			for(x=0;x<nxsiz;x++)
				for(y=0;y<nysiz;y++)
					for(z=0;z<nzsiz;z++)
					{
						j = 0; //# air voxels
						if (vox_test(x*2  ,y*2  ,z*2  )) j++;
						if (vox_test(x*2  ,y*2  ,z*2+1)) j++;
						if (vox_test(x*2  ,y*2+1,z*2  )) j++;
						if (vox_test(x*2  ,y*2+1,z*2+1)) j++;
						if (vox_test(x*2+1,y*2  ,z*2  )) j++;
						if (vox_test(x*2+1,y*2  ,z*2+1)) j++;
						if (vox_test(x*2+1,y*2+1,z*2  )) j++;
						if (vox_test(x*2+1,y*2+1,z*2+1)) j++;
						if (j >= 4) vox_set(x,y,z); else vox_clear(x,y,z); //NOTE: voxel values are inversed from SLAB6.C; Use >= 4
					}
		}
	}

	writepal(fil);
	fclose(fil);
}

static void savekv6 (char *fnam)
{
	FILE *fil;
	float f;
	int i, j, k, x, y, z, numvoxs;
	unsigned short u;

	if (rendermode < 2) hollowfix(xsiz,ysiz,zsiz);
	compact_siz();
	printf("Writing %s (%dx%dx%d)\n",fnam,xsiz,ysiz,zsiz);

	if (!(fil = fopen(fnam,"wb"))) quitout("Could not write file");

	i = 0x6c78764b;
	fwrite(&i,4,1,fil);
	fwrite(&xsiz,4,1,fil);
	fwrite(&ysiz,4,1,fil);
	fwrite(&zsiz,4,1,fil);
	f = (float)xpiv; fwrite(&f,4,1,fil); //Copy to f just in case I change *piv to double someday...
	f = (float)ypiv; fwrite(&f,4,1,fil);
	f = (float)zpiv; fwrite(&f,4,1,fil);

	equivecinit(255);

	numvoxs = 0;
	fwrite(&numvoxs,4,1,fil); //to be rewritten later

	for(x=0;x<xsiz;x++)
	{
		xstart[x] = 0;
		for(y=0;y<ysiz;y++)
		{
			xyoffs[x][y] = 0;
			for(z=vox_dntil1(x,y,0);z<zsiz;z=vox_dntil1(x,y,z+1))
			{
				j = 0;
				if ((x == 0)      || (!vox_test(x-1,y,z))) j |= 1;
				if ((x == xsiz-1) || (!vox_test(x+1,y,z))) j |= 2;
				if ((y == 0)      || (!vox_test(x,y-1,z))) j |= 4;
				if ((y == ysiz-1) || (!vox_test(x,y+1,z))) j |= 8;
				if ((z == 0)      || (!vox_test(x,y,z-1))) j |=16;
				if ((z == zsiz-1) || (!vox_test(x,y,z+1))) j |=32;
				if (!j) continue;

				k = getpix32(x,y,z);
				fputc((k&255),fil);
				fputc((k>>8)&255,fil);
				fputc((k>>16)&255,fil);
				fputc(128,fil);
				fputc(z&255,fil); fputc(z>>8,fil);
				fputc(j,fil); //vis (0-63)
				fputc(getnorm8(x,y,z),fil); //dir (0-255)

				xyoffs[x][y]++; xstart[x]++; numvoxs++;
			}
		}
	}

	for(x=0;x<xsiz;x++) { i = xstart[x]; fwrite(&i,4,1,fil); }
	for(x=0;x<xsiz;x++)
		for(y=0;y<ysiz;y++) { u = xyoffs[x][y]; fwrite(&u,2,1,fil); }

	j = ftell(fil);
	fseek(fil,28,SEEK_SET);
	fwrite(&numvoxs,4,1,fil);
	fseek(fil,j,SEEK_SET);
	fclose(fil);
}

	//07/09/2004: See RAYRECU4.BAS (beginning of compilestack) for derivation.
	//   Code is equivalent to the original compilebitstack() converted from
	//VOXLAP5.C's compilestack (and first used in POLY2VOX). The main difference
	//is this version is much faster because it uses dntil0()/dntil1()! :) The
	//other difference is that n0[] is destroyed in this routine (speed hack).
	// Inputs: x,y: column to compress (0=air,1=solid)
	//         n0[MAXZDIM]: neighbor bitbuf (n0&n1&n2&n3): (0=air,1=solid)
	//Outputs: cbuf[MAXCSIZ]: compressed output buffer
	//Returns: n: length of compressed buffer (in bytes)
static int compilebitstack (int x, int y, int *n0, char *cbuf)
{
	ipoint3d p;
	int cz0, cz1, uz0, uz1, cz, n, onext, stat;

	p.x = x; p.y = y;
	uz1 = -1; cz = 0; n = 0; onext = 0;
	while (1)
	{
		uz0 = vox_dntil1(x,y,uz1+1); if (uz0 >= zsiz) break;
		uz1 = vox_dntil0(x,y,uz0+1);
		if (uz1 < zsiz) n0[(uz1-1)>>5] &= ~(1<<KMOD32(uz1-1)); else uz1 = zsiz;
		cz0 = uz0; stat = 0;
		while (1)
		{
			cz1 = dntil1(n0,cz0+1,MAXZDIM); if (cz1 > uz1) cz1 = uz1;
			if (!stat)
			{
				cbuf[onext] = ((n-onext)>>2); onext = n;
				cbuf[n+1] = cz0;
				cbuf[n+2] = cz1-1;
				cbuf[n+3] = cz;
				n += 4;
			}
			for(p.z=cz0;p.z<cz1;p.z++)
			{
				*(int *)&cbuf[n] = ((getpix32(p.x,p.y,p.z)&0xffffff)|0x80000000);
				n += 4;
			}
			if ((cz1 < uz1) && (stat))
			{
				cbuf[onext] = ((n-onext)>>2); onext = n;
				cbuf[n+1] = cz1;
				cbuf[n+2] = cz1-1;
				cbuf[n+3] = cz1;
				n += 4;
			}
			cz0 = dntil0(n0,cz1+1,MAXZDIM); if (cz0 >= uz1) break;
			stat = 1;
		}
		cz = uz1;
	}
	cbuf[onext] = 0;
	return(n);
}

static lpoint2d vxledge[2];
static void savevxl (char *fnam)
{
	FILE *fil;
	int i, x, y, z, *v, *v0, *v1;
	double d[3];
	int vbitemp[MAXZDIM>>5];

	if (rendermode < 2)
	{
			//Make sure bottom is solid
		for(x=0;x<VSID;x++)
			for(y=0;y<VSID;y++)
				vox_set(x,y,MAXZDIM-1);

			//Make edges solid
		for(i=0;i<2;i++)
		{
			for(x=vxledge[0].x;x<=vxledge[1].x;x++) vox_setzrange1(x,vxledge[i].y,0,MAXZDIM);
			for(y=vxledge[0].y;y<=vxledge[1].y;y++) vox_setzrange1(vxledge[i].x,y,0,MAXZDIM);
		}
		hollowfix(xsiz,ysiz,zsiz);

		//NOTE: should clean up edges here, but how?
	}
	printf("Writing %s (%dx%dx%d)\n",fnam,xsiz,ysiz,zsiz);

	if (!(fil = fopen(fnam,"wb"))) quitout("Could not write file");

	i = 0x09072000; fwrite(&i,4,1,fil); //Version
	i = VSID; fwrite(&i,4,1,fil);
	i = VSID; fwrite(&i,4,1,fil);

		//Make sure starting camera is close to ground
	z = MAXZDIM;
	for(x=512-5;x<=512+5;x++)
		for(y=512-5;y<=512+5;y++)
			z = min(z,vox_dntil1(x,y,0));
	z -= 5;

	d[0] = 512.0; d[1] = 512.0; d[2] = (double)z; fwrite(d,24,1,fil);
	d[0] = 1.0; d[1] = 0.0; d[2] = 0.0; fwrite(d,24,1,fil);
	d[0] = 0.0; d[1] = 0.0; d[2] = 1.0; fwrite(d,24,1,fil);
	d[0] = 0.0; d[1] =-1.0; d[2] = 0.0; fwrite(d,24,1,fil);

		//Make sure bottom is solid
	for(x=0;x<VSID;x++)
		for(y=0;y<VSID;y++)
			vox_set(x,y,MAXZDIM-1);

	for(x=0;x<VSID;x++)
		for(y=0;y<VSID;y++)
		{
			vox_and4neighs_retbits(x,y,vbitemp);
			i = compilebitstack(x,y,vbitemp,tempbuf);
			fwrite(tempbuf,i,1,fil);
		}
	fclose(fil);
}

#if (PRINTTIME == 1)
static double klock (void)
{
	static double rper = 1e-32;
	__int64 q;
	if (rper == 1e-32) { QueryPerformanceFrequency((LARGE_INTEGER *)&q); rper = 1.0/(double)q; }
	QueryPerformanceCounter((LARGE_INTEGER *)&q);
	return((double)q*rper);
}
#endif

static int loadpoly (char *ofnam)
{
	char fnam[MAX_PATH+4], *cptr;
	int i, fnlng, slash;

	strcpy(fnam,ofnam); fnlng = strlen(fnam);
	for(i=fnlng-1,slash=0;i>=0;i--) if ((fnam[i] == '\\') || (fnam[i] == '/')) { slash = i+1; break; }

	if ((fnlng >= 4) && (!strcmp(&fnam[fnlng-4],".zip"))) { kzaddstack(fnam); gotzip = 1; }
	else
	{
		if (fnlng >= 4)
		{
			cptr = &fnam[fnlng-4];
			if ((!stricmp(cptr,".md3")) && (loadmd3(fnam))) return(1);
			if ((!stricmp(cptr,".md2")) && (loadmd2(fnam))) return(1);
			if ((!stricmp(cptr,".3ds")) && (load3ds(fnam))) return(1);
			if ((!stricmp(cptr,".asc")) && (loadasc(fnam))) return(1);
			if ((!stricmp(cptr,".obj")) && (loadobj(fnam))) return(1);
			if ((!stricmp(cptr,".stl")) && (loadstl(fnam))) return(1);
		}
		strcpy(fnam,ofnam);
		strcpy(&fnam[fnlng],".md3"); if (loadmd3(fnam)) return(1);
		strcpy(&fnam[fnlng],".md2"); if (loadmd2(fnam)) return(1);
		strcpy(&fnam[fnlng],".3ds"); if (load3ds(fnam)) return(1);
		strcpy(&fnam[fnlng],".asc"); if (loadasc(fnam)) return(1);
		strcpy(&fnam[fnlng],".obj"); if (loadobj(fnam)) return(1);
		strcpy(&fnam[fnlng],".stl"); if (loadstl(fnam)) return(1);

		strcpy(&fnam[fnlng],".zip"); kzaddstack(fnam); gotzip = 1;
	}

	if (fnlng >= 4)
	{
		cptr = &fnam[fnlng-4];
		strcpy(cptr,".md3"); if ((loadmd3(fnam)) || (loadmd3(&fnam[slash]))) return(1);
		strcpy(cptr,".md2"); if ((loadmd2(fnam)) || (loadmd2(&fnam[slash]))) return(1);
		strcpy(cptr,".3ds"); if ((load3ds(fnam)) || (load3ds(&fnam[slash]))) return(1);
		strcpy(cptr,".asc"); if ((loadasc(fnam)) || (loadasc(&fnam[slash]))) return(1);
		strcpy(cptr,".obj"); if ((loadobj(fnam)) || (loadobj(&fnam[slash]))) return(1);
		strcpy(cptr,".stl"); if ((loadstl(fnam)) || (loadstl(&fnam[slash]))) return(1);
	}
	kzfindfilestart("*.*");
	while (kzfindfile(fnam))
	{
		fnlng = strlen(fnam); if ((fnlng < 4) || (fnam[0] != '|')) continue;
		cptr = &fnam[fnlng-4];
		if ((!stricmp(cptr,".md3")) && (loadmd3(fnam))) return(1);
		if ((!stricmp(cptr,".md2")) && (loadmd2(fnam))) return(1);
		if ((!stricmp(cptr,".3ds")) && (load3ds(fnam))) return(1);
		if ((!stricmp(cptr,".asc")) && (loadasc(fnam))) return(1);
		if ((!stricmp(cptr,".obj")) && (loadobj(fnam))) return(1);
		if ((!stricmp(cptr,".stl")) && (loadstl(fnam))) return(1);
	}
	return(0);
}

static void showhelp (void)
{
	printf("POLY2VOX [input] [output] [/v#] [/s#] [/f#] [/n#] [/r#] [/m#] [/x#] [/p(file)]\n");
	printf("by Ken Silverman (http://advsys.net/ken)  Compiled: %s\n\n",__DATE__);
	printf("Converts models from polygon to voxel format.\n");
	printf("Supported polygon formats: ASC,3DS,MD2,MD3,OBJ,STL\n");
	printf("Supported   voxel formats: VOX,KVX,KV6,VXL (default:KV6)\n");
	printf("Supported texture formats: PNG,JPG,TGA,GIF,CEL,PCX,BMP,DDS\n");
	printf("POLY2VOX can load files out of a ZIP file.\n\n");
	printf(" /v#  Specify voxel size of longest dimension. 1-1024, <=256 for KVX\n");
	printf(" /s#  Specify explicit scale factor. Use this to ensure the size of all frames\n");
	printf("      is consistent. This factor depends on the coordinate system used by the\n");
	printf("      polygon model, so it can be anything. Run without the scale factor first\n");
	printf("      to find a reasonble starting value to try.\n");
	printf(" /f#  Specify frame number (MD2/MD3 only)\n");
	printf(" /n#  Specify next frame number for interpolation (MD2/MD3 only)\n");
	printf(" /l#  Specify texture interpolation method: {1:nearest, 4:4x4 (default)}\n");
	printf(" /r#  Specify frame interpolation ratio: {0.0-1.0}, default:0.0 (MD2/MD3 only)\n");
	printf(" /m#  Specify number of mips to save: 1,5, default:5. (KVX only)\n");
	printf(" /k?  Specify illumination model for OBJ (Ex: /ka, /kd, /ks, /ke)\n");
	printf(" /y   Polygon render (default)  /y2: Polygon render using supercover\n");
	printf(" /w   Wireframe render          /w2: Wireframe render using supercover\n");
	printf(" /x   Experimental xor-style render for gap-less models;buggy color conversion\n");
	printf(" /c   Center model in bounding box (default is to use polygon file's 0,0,0)\n");
	printf(" /cm  Center of model is centroid\n");
	printf(" /b(l/r/b/f/u/d)# Clip boundary (Left/Right/Back/Front/Up/Down). Ex: /bu-1.2\n");
	printf(" /t(file)  Select a texture file (if not specified in polygon model).\n");
	printf(" /p(file)  Specify Build-style palette (first 768 bytes of file, range:0-63)\n");
	printf(" /z(file)  Specify a ZIP file to mount. Files inside seen as local dir.\n\n");
	printf("Examples:\n");
	printf(" poly2vox bike                        (finds bike.*, writes bike.kvx, size=128)\n");
	printf(" poly2vox bike.3ds bike.kv6 /v250   (reads bike.3ds, writes bike.kv6, size=250)\n");
	printf(" poly2vox land land.vxl        (finds land.*, writes land.vxl, size=1024^2*256)\n");
	printf(" poly2vox pig.md2 pig.kvx /v128 /f2 /n3 /r.5       (convert interpolated frame)\n");
	printf(" poly2vox trooper.md2 trooper.kvx /s.115 /f0        (use explicit scale factor)\n");
	printf(" poly2vox trooper.md2 trooper.kvx /s0.115 /f0 /ppalette.dat      (user palette)\n");
	printf(" for /L %%i in (0,1,50) do start /i poly2vox /zmonst.zip monst%%i.obj     (batch)\n");
}

void main (int argc, char **argv)
{
#if (PRINTTIME == 1)
	double t0, t1;
#endif
	point3d oldp;
	float f, g, h, fx, fy, fz, fx0, fy0, fz0, fx1, fy1, fz1, forcescale = 0;
	int i, j, k, x, y, z, infilarg, outfilarg, voxdim = 128, numips = 5, centmode = 0, doclip = 0;
	char fnam[MAX_PATH], infilnam[MAX_PATH], outfilnam[MAX_PATH], *cptr;

#if (PRINTTIME == 1)
	t0 = klock();
#endif

	if (argc < 2) { showhelp(); exit(0); }
	infilarg = -1; outfilarg = -1;
	for(i=1;i<argc;i++)
	{
		if ((argv[i][0] != '-') && (argv[i][0] != '/'))
		{
				  if (infilarg < 0) { infilarg = i; }
			else if (outfilarg < 0) { outfilarg = i; }
			else { printf("ERROR: Too many files specified\n"); showhelp(); exit(0); }
			continue;
		}
		j = argv[i][1]; if ((j >= 'a') && (j <= 'z')) j -= 32;
		switch(j)
		{
			case 'V': voxdim = min(max(atoi(&argv[i][2]),1),max(max(MAXDIM,MAYDIM),MAZDIM)); break;
			case 'S': forcescale = atof(&argv[i][2]); break;
			case 'F': gcframe = atoi(&argv[i][2]); break;
			case 'N': gnframe = atoi(&argv[i][2]); break;
			case 'R': ginterp = atof(&argv[i][2]); break;
			case 'L': gtexfilter = atoi(&argv[i][2]); break;
			case 'K': glightmodchar = argv[i][2]; break;
			case 'M': numips = atoi(&argv[i][2]); if (numips <= 1) numips = 1; else numips = 5; break;
			case 'P': j = 2; if (!argv[i][2]) { i++; j -= 2; } strcpy(palfilnam,&argv[i][j]); break;
			case 'T': j = 2; if (!argv[i][2]) { i++; j -= 2; } strcpy(texfilnam,&argv[i][j]); break;
			case 'Z': j = 2; if (!argv[i][2]) { i++; j -= 2; } kzaddstack(&argv[i][j]); gotzip = 1; break;
			case 'Y': rendermode = 0; if (argv[i][2] == '2') rendermode = 1; break;
			case 'W': rendermode = 2; if (argv[i][2] == '2') rendermode = 3; break;
			case 'X': rendermode = 4; break;
			case 'C': centmode = 1; j = argv[i][2]; if ((j == 'M') || (j == 'm')) centmode = 2; break;
			case 'B':
				if ((argv[i][2] == 'L') || (argv[i][2] == 'l')) { gclipmin.y = atof(&argv[i][3]); doclip |= 1; }
				if ((argv[i][2] == 'R') || (argv[i][2] == 'r')) { gclipmax.y = atof(&argv[i][3]); doclip |= 2; }
				if ((argv[i][2] == 'B') || (argv[i][2] == 'b')) { gclipmin.x = atof(&argv[i][3]); doclip |= 4; }
				if ((argv[i][2] == 'F') || (argv[i][2] == 'f')) { gclipmax.x = atof(&argv[i][3]); doclip |= 8; }
				if ((argv[i][2] == 'U') || (argv[i][2] == 'u')) { gclipmax.z = atof(&argv[i][3]); doclip |=16; }
				if ((argv[i][2] == 'D') || (argv[i][2] == 'd')) { gclipmin.z = atof(&argv[i][3]); doclip |=32; }
				break;
		}
	}
	if (infilarg < 0) { printf("ERROR: Must specify input file\n"); showhelp(); exit(0); }

	if (argv[infilarg][0] == '\"') //Strip quotes
		  { strcpy(infilnam,&argv[infilarg][1]); infilnam[max(strlen(infilnam)-1,0)] = 0; }
	else { strcpy(infilnam,argv[infilarg]); }
	if (outfilarg >= 0)
	{
		if (argv[outfilarg][0] == '\"') //Strip quotes
			  { strcpy(outfilnam,&argv[outfilarg][1]); outfilnam[max(strlen(outfilnam)-1,0)] = 0; }
		else { strcpy(outfilnam,argv[outfilarg]); }
	}

	if (!vox_init()) quitout("vox_init() failed");
	trioff  = (      int *)malloc(sizeof(int)*maxobjects   ); if (!trioff ) quitout("malloc failed: trioff");
	vertoff = (      int *)malloc(sizeof(int)*maxobjects   ); if (!vertoff) quitout("malloc failed: vertoff");
	matstr  = (      int *)malloc(sizeof(int)*maxmat       ); if (!matstr ) quitout("malloc failed: matstr");
	colrgb  = (      int *)malloc(sizeof(int)*maxmat       ); if (!colrgb ) quitout("malloc failed: colrgb");
	nambuf  = (     char *)malloc(sizeof(char)*maxnamsiz   ); if (!nambuf ) quitout("malloc failed: nambuf");
	pic     = (   pictyp *)malloc(sizeof(pictyp)*maxpics   ); if (!pic    ) quitout("malloc failed: pic");
	tri     = (   trityp *)malloc(sizeof(trityp)*maxtris   ); if (!tri    ) quitout("malloc failed: tri");
	vert    = (   vertyp *)malloc(sizeof(vertyp)*maxverts  ); if (!vert   ) quitout("malloc failed: vert");
	hashdat = (hashdat_t *)malloc(sizeof(hashdat_t)*maxhash); if (!hashdat) quitout("malloc failed: hashdat");

	if (texfilnam[0]) //force texture filename
	{
		i = strlen(texfilnam);
		pic[numpics].namoff = namplc;
		numpics++;
		memcpy(&nambuf[namplc],texfilnam,i+1); namplc += i+1;
	}

	if (!loadpoly(infilnam)) quitout("Can't find input file");

	savetype = 2;
	if (outfilarg >= 0)
	{
		strcpy(fnam,outfilnam); _strlwr(fnam);
			  if (strstr(fnam,".vox")) savetype = 0;
		else if (strstr(fnam,".kvx")) savetype = 1;
		else if (strstr(fnam,".kv6")) savetype = 2;
		else if (strstr(fnam,".vxl")) savetype = 3;
	}

	for(i=0;i<numpics;i++)
	{
		strcpy(fnam,&nambuf[pic[i].namoff]);
		for(j=strlen(fnam)-1;j>0;j--) { if ((fnam[j] == '\\') && (fnam[j-1] == '\\')) break; } //Skip network address
		if (!j) { kpzload(fnam,&pic[i].f,&pic[i].p,&pic[i].x,&pic[i].y); if (pic[i].f) goto donepic; }

		for(j=strlen(fnam)-1;j>=0;j--) { if ((fnam[j] == '\\') || (fnam[j] == '/')) break; } if (j < 0) goto donepic;
		strcpy(fnam,&nambuf[pic[i].namoff+j+1]);
		kpzload(fnam,&pic[i].f,&pic[i].p,&pic[i].x,&pic[i].y); if (pic[i].f) goto donepic;

		for(k=strlen(infilnam)-1;k>=0;k--) { if ((infilnam[k] == '\\') || (infilnam[k] == '/')) break; } if (k < 0) goto donepic;
		strcpy(fnam,infilnam);
		strcpy(&fnam[k+1],&nambuf[pic[i].namoff+j+1]);
		kpzload(fnam,&pic[i].f,&pic[i].p,&pic[i].x,&pic[i].y); if (pic[i].f) goto donepic;

donepic:;
		if (pic[i].f) { printf(" %s loaded successfully\n",fnam); picsloaded++; }
		else printf(" %s NOT FOUND or not supported format\n",&nambuf[pic[i].namoff]);
	}

	if ((!numpics) && (gotzip))
	{
		kzfindfilestart("*");
		while (kzfindfile(fnam))
		{
			if (fnam[0] != '|') continue;
			kpzload(fnam,&pic[0].f,&pic[0].p,&pic[0].x,&pic[0].y); if (!pic[0].f) continue;
			printf("Found %s\n",&fnam[1]);
			picsloaded++; numpics++; colrgb[0] = -1; break;
		}
	}


		//TEMP HACK to swap x&y axes!!!
	//for(z=0;z<numverts;z++)
	//{
	//   oldp = *(point3d *)&vert[z];
	//   vert[z].x = oldp.y;
	//   vert[z].y =-oldp.x;
	//   vert[z].z = oldp.z;
	//}

// Scale data to specified resolution ---------------------------------------

	fx0 = 1e32f; fx1 = -1e32f;
	fy0 = 1e32f; fy1 = -1e32f;
	fz0 = 1e32f; fz1 = -1e32f;
	for(z=0;z<numverts;z++)
	{
		if (vert[z].x < fx0) fx0 = vert[z].x;
		if (vert[z].y < fy0) fy0 = vert[z].y;
		if (vert[z].z < fz0) fz0 = vert[z].z;
		if (vert[z].x > fx1) fx1 = vert[z].x;
		if (vert[z].y > fy1) fy1 = vert[z].y;
		if (vert[z].z > fz1) fz1 = vert[z].z;
	}

	//printf("%9.6f,%9.6f,%9.6f\n",fx0,fy0,fz0);
	//printf("%9.6f,%9.6f,%9.6f\n",fx1,fy1,fz1);

	if (savetype != 3)
	{
		if (forcescale > 0)
			f = forcescale;
		else
		{
			h = (MAYDIM-.1f)/(fx1-fx0); //Find maximum scale factor (g)
			f = (MAXDIM-.1f)/(fy1-fy0); if (f < h) h = f;
			f = (MAZDIM-.1f)/(fz1-fz0); if (f < h) h = f;
			g = fx1-fx0; i = 'x';
			if (fy1-fy0 > g) { g = fy1-fy0; i = 'y'; }
			if (fz1-fz0 > g) { g = fz1-fz0; i = 'z'; }
			f = ((float)voxdim-.1f) / g;
			if (f > h)
			{
				f = h; voxdim = cvtss2si(h*g);
				printf(" NOTE: %cdim limited to %d\n",i,voxdim);
			}
			if (voxdim <= 0) quitout("error: voxdim must be > 0");
		}
		xsiz = cvtss2si((fy1-fy0)*f+.5); if ((savetype < 2) && (xsiz > 256)) printf(" WARNING: xsiz > 256!");
		ysiz = cvtss2si((fx1-fx0)*f+.5); if ((savetype < 2) && (ysiz > 256)) printf(" WARNING: ysiz > 256!");
		zsiz = cvtss2si((fz1-fz0)*f+.5); if ((savetype < 2) && (zsiz > 256-savetype)) printf(" WARNING: zsiz > %d!",256-savetype);

		fy = fy0; fx = fx0; fz = fz1;
	}
	else
	{
		xsiz = VSID; ysiz = VSID; zsiz = MAXZDIM;
		if ((MAXDIM < xsiz) || (MAYDIM < ysiz) || (MAZDIM < zsiz))
			quitout("ERROR: compiled with maximum dimensions too low");

		f = (ysiz-.1f)/(fx1-fx0);
		h = (xsiz-.1f)/(fy1-fy0); if (h < f) f = h;
		h = (zsiz-.1f)/(fz1-fz0); if (h < f) f = h;

			//Hack to center object in converted space:
			//   (fy0-?)*f+.05 = ysiz - ((fy1-?)*f+.05)
			//   (fx0-?)*f+.05 = xsiz - ((fx1-?)*f+.05)
			//   (?-fz0)*f+.05 = zsiz - ((?-fz1)*f+.05)
		g = 1.0 / f;
		fy = ((fy0+fy1) - (ysiz-.05*2)*g)*.5;
		fx = ((fx0+fx1) - (xsiz-.05*2)*g)*.5;
		fz = ((zsiz-.05*2)*g + (fz0+fz1))*.5;

		vxledge[0].x = (int)((fy0-fy)*f+.05f); if (vxledge[0].x < 0) vxledge[0].x = 0;
		vxledge[0].y = (int)((fx0-fx)*f+.05f); if (vxledge[0].y < 0) vxledge[0].y = 0;
		vxledge[1].x = (int)((fy1-fy)*f+.05f); if (vxledge[1].x >= VSID) vxledge[1].x = VSID-1;
		vxledge[1].y = (int)((fx1-fx)*f+.05f); if (vxledge[1].y >= VSID) vxledge[1].y = VSID-1;
	}
	printf("Scale factor used (voxel/polygon units):%9.6f\n",f);

	if (doclip)
	{
		//printf("%d,%d,%d\n",xsiz,ysiz,zsiz);

		//printf("%12g,%12g,%12g\n",gclipmin.x,gclipmin.y,gclipmin.z);
		//printf("%12g,%12g,%12g\n",gclipmax.x,gclipmax.y,gclipmax.z);

		oldp = gclipmin;
		gclipmin.x = (oldp.x-fx)*f+.05f;
		gclipmin.y = (oldp.y-fy)*f+.05f;
		gclipmin.z = (fz-oldp.z)*f+.05f;
		oldp = gclipmax;
		gclipmax.x = (oldp.x-fx)*f+.05f;
		gclipmax.y = (oldp.y-fy)*f+.05f;
		gclipmax.z = (fz-oldp.z)*f+.05f;
		h = gclipmin.z; gclipmin.z = gclipmax.z; gclipmax.z = h;

		//printf("%12g,%12g,%12g\n",gclipmin.x,gclipmin.y,gclipmin.z);
		//printf("%12g,%12g,%12g\n",gclipmax.x,gclipmax.y,gclipmax.z);
	}
	for(z=0;z<numverts;z++)
	{
		oldp = *(point3d *)&vert[z];
		vert[z].x = (oldp.y-fy)*f+.05f;
		vert[z].y = (oldp.x-fx)*f+.05f;
		vert[z].z = (fz-oldp.z)*f+.05f;
	}
	oldp.x = 0.f; oldp.y = 0.f; oldp.z = 0.f;
	xpiv = (oldp.y-fy)*f+.05f;
	ypiv = (oldp.x-fx)*f+.05f;
	zpiv = (fz-oldp.z)*f+.05f;
	if (savetype == 1) zpiv = ((float)zsiz)*.5; //Ugly hack
	if (centmode == 1) //center model extents around origin
	{
		xpiv = ((float)xsiz)*.5;
		ypiv = ((float)ysiz)*.5;
		zpiv = ((float)zsiz)*.5;
	}

// Render geometry to vox buffer and colors to sparse hash ---------------------

	hash_init();

	switch(rendermode)
	{
		case 0: for(i=numtris-1;i>=0;i--) { renderface0tri(i); }                     break; //Minimal face fill (best!)
		case 1: for(i=numtris-1;i>=0;i--) { renderface1tri(i); }                     break; //Supercover face fill
		case 2: for(i=numtris-1;i>=0;i--) { renderline0tri(i); }                     break; //Minimal wireframe
		case 3: for(i=numtris-1;i>=0;i--) { renderline1tri(i); }                     break; //Supercover wireframe
		case 4: for(i=numtris-1;i>=0;i--) { renderfilltri(i);  } renderfillfinish(); break; //xor style fill
	}

	if (doclip)
	{
		gclipx0 = cvtss2si(min(max(gclipmin.y+.5f,0),xsiz)); gclipx1 = cvtss2si(min(max(gclipmax.y+.5f,0),xsiz));
		gclipy0 = cvtss2si(min(max(gclipmin.x+.5f,0),ysiz)); gclipy1 = cvtss2si(min(max(gclipmax.x+.5f,0),ysiz));
		gclipz0 = cvtss2si(min(max(gclipmin.z+.5f,0),zsiz)); gclipz1 = cvtss2si(min(max(gclipmax.z+.5f,0),zsiz));

		printf("clip: (%d,%d,%d)-(%d,%d,%d)\n",gclipx0,gclipy0,gclipz0,gclipx1,gclipy1,gclipz1);

		if (doclip& 1) { for(y=      0;y<ysiz   ;y++) for(x=      0;x<gclipx0;x++) { vox_setzrange0(x,y,      0,   zsiz); } }
		if (doclip& 2) { for(y=      0;y<ysiz   ;y++) for(x=gclipx1;x<xsiz   ;x++) { vox_setzrange0(x,y,      0,   zsiz); } }
		if (doclip& 4) { for(y=      0;y<gclipy0;y++) for(x=      0;x<xsiz   ;x++) { vox_setzrange0(x,y,      0,   zsiz); } }
		if (doclip& 8) { for(y=gclipy1;y<ysiz   ;y++) for(x=      0;x<xsiz   ;x++) { vox_setzrange0(x,y,      0,   zsiz); } }
		if (doclip&16) { for(y=      0;y<ysiz   ;y++) for(x=      0;x<xsiz   ;x++) { vox_setzrange0(x,y,      0,gclipz0); } }
		if (doclip&32) { for(y=      0;y<ysiz   ;y++) for(x=      0;x<xsiz   ;x++) { vox_setzrange0(x,y,gclipz1,   zsiz); } }
	}
	else
	{
		gclipx0 = 0; gclipy0 = 0; gclipz0 = 0; gclipx1 = xsiz; gclipy1 = ysiz; gclipz1 = zsiz;
	}

	if (centmode == 2) //center of model is centroid
	{
		int xsum, ysum, zsum, vsum, z0, z1;

		xsum = 0; ysum = 0; zsum = 0; vsum = 0;
		for(x=0;x<xsiz;x++)
			for(y=0;y<ysiz;y++)
			{
				z1 = -1;
				do
				{
					z0 = vox_dntil1(x,y,z1+1); if (z0 >= zsiz) break;
					z1 = vox_dntil1(x,y,z0+1);

						//use x,y,z0..z1-1
					i = z1-z0;
					xsum += i*x;
					ysum += i*y;
					zsum += (((z0+z1-1)*i)>>1);
					vsum += i;

					if (z1 >= zsiz) break;
				} while (z1 < zsiz);
			}

		f = 1.f/(float)vsum;
		xpiv = (float)xsum*f;
		ypiv = (float)ysum*f;
		zpiv = (float)xsum*f;
	}

	if (outfilarg < 0)
	{
		strcpy(fnam,infilnam);
		for(i=0,cptr=0;fnam[i];i++) if (fnam[i] == '.') cptr = &fnam[i]; //replace extension with kv6
		if (!cptr) cptr = &fnam[strlen(fnam)];
		strcpy(cptr,".kv6");

			//If EXE has full path, it probably means this was run from explorer (filename dragged onto exe).
			//In this case, it is better to save in the file's directory, and not the default user dir.
		for(i=0;1;i++)
		{
			if (argv[0][i] == 0) //a ':' indicates exe has full path
			{
				for(i=strlen(fnam)-1;i>=0;i--) //Strip off directory
					if ((fnam[i] == '\\') || (fnam[i] == '/')) break;
				savekv6(&fnam[i+1]);
				break;
			}
			if (argv[0][i] == ':') { savekv6(fnam); break; } //full path
		}
	}
	else
	{
		switch(savetype)
		{
			case 0: savevox(outfilnam); break;
			case 1: savekvx(outfilnam,numips); break;
			case 2: savekv6(outfilnam); break;
			case 3: savevxl(outfilnam); break;
		}
	}

	while (numpics > 0)
	{
		numpics--;
		if (pic[numpics].f) free((void *)pic[numpics].f);
	}

#if (PRINTTIME == 1)
	t1 = klock();
	printf("%.2f seconds\n",t1-t0);
#endif

	quitout("Done.");
}

#if 0
	int xsiz, ysiz, zsiz, xpivot, ypivot, zpivot;
	int xstart[xsiz+1];
	short xyoffs[xsiz][ysiz+1];
	char rawslabdata[?];

	//sptr = xstart[x] + xyoffs[x][y];   //start of slabs (x,y)
	//eptr = xstart[x] + xyoffs[x][y+1]; //end of slabs (x,y)
	char slabztop;
	char slabzleng;
	char slabbackfacecullinfo; //Low 6 bits used to tell which of the 6 faces are exposed.
	char col[slabzleng];       //The array of colors from top to bottom
#endif

#if 0
!endif
#endif
