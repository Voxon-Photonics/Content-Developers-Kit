// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
fractals.exe: fractals.obj fractals_64.obj
	link       fractals.obj fractals_64.obj user32.lib
	del fractals.obj
fractals.obj:    fractals.c voxiebox.h; cl /c /TP fractals.c /Ox /GFy /MT
fractals_64.obj: fractals_64.asm      ; ml64 /c fractals_64.asm

#fractals.exe: fractals.c; \Dev-Cpp\bin\gcc fractals.c -o fractals.exe -pipe -O3 -s
!if 0
#endif

#include <stdlib.h>
#include <math.h>
#include <intrin.h>
#include <process.h>
#include "voxiebox.h"
static voxie_wind_t vw;
#define PI 3.14159265358979323

static double gtim;

	//Rotate vectors a & b around their common plane, by ang
static void rotate_vex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

	//Useful helper math ;)
#if 1
#define rcpss(f) _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(f)))
#define rsqrtss(f) _mm_cvtss_f32(_mm_rsqrt_ps(_mm_set_ss(f)))
#else
#define rcpss(f) (1.0/f)          //in case above doesn't compile
#define rsqrtss(f) (1.0/sqrtf(f)) //in case above doesn't compile
#endif

static int palcur[8];

extern "C"
{
extern void getvox_xform_avx2 (int vtn, float *wx, float *wy, float *wz, float *px, float *py, float *pz, poltex_t *vt, int ix, int iy, int iz);
extern int getvox_mandelbulb_avx2 (int wptr, float *wx, float *wy, float *wz, float *px, float *py, float *pz, int *cntind, int *outcol);
__declspec(align(32)) int palcur2[8];
__declspec(align(32)) char compactlut[256*8];

__declspec(align(32)) unsigned int qqxoff[8], qqyoff[8], qqzoff[8];
__declspec(align(32)) unsigned int qqxadd8[8], qqyadd8[8], qqzadd8[8];
__declspec(align(32)) float qqxmul[8], qqymul[8], qqzmul[8];
__declspec(align(32)) float qqpxs[8], qqpys[8], qqpzs[8];
__declspec(align(32)) float qqrxs[8], qqrys[8], qqrzs[8];
__declspec(align(32)) float qqdxs[8], qqdys[8], qqdzs[8];
__declspec(align(32)) float qqfxs[8], qqfys[8], qqfzs[8];
}
static const int paltab[][sizeof(palcur)/sizeof(palcur[0])] =
{
	0x000000, 0x0000ff, 0xff0000, 0x00ff00, 0x000000, 0x0000ff, 0xff0000, 0x00ff00,
	0x000000, 0x0000ff, 0xff0000, 0x00ff00, 0xff00ff, 0x00ffff, 0xffff00, 0xffffff,
	0xffffff, 0xffff00, 0x00ffff, 0xff00ff, 0x00ff00, 0xff0000, 0x0000ff, 0x000000,
	0xff0000, 0xffff00, 0xff9900, 0x00ff00, 0x00ffff, 0x0000ff, 0x9900ff, 0x000000,
};

int rgb (float r, float g, float b)
{
	return min(max((int)r,0),255)*65536 +
			 min(max((int)g,0),255)*256 +
			 min(max((int)b,0),255);
}

	//shape funcs..

int getvox_mandelbulb (point3d p)
{
	point3d w;
	float x, y, z, x2, y2, z2, x4, y4, z4, k1, k2, k3, k4, k5, k6;
	int i;

	w = p;
	for(i=sizeof(palcur)/sizeof(palcur[0])-1;i>0;i--)
	{
#if 0
		x = w.x; x2 = x*x; x4 = x2*x2;
		y = w.y; y2 = y*y; y4 = y2*y2;
		z = w.z; z2 = z*z; z4 = z2*z2;

			//Based on fast algo here: https://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
			//1rsqrt 48* 16+
		k3 = x2 + z2;
		k2 = rsqrtss(k3*k3*k3*k3*k3*k3*k3);
		k1 = x4 + y4 + z4 - 6.0*y2*z2 - 6.0*x2*y2 + 2.0*z2*x2;
		k4 = x2 - y2 + z2;

		w.x = 64.0*x*y*z*(x2-z2)*k4*(x4-6.0*x2*z2+z4)*k1*k2;
		w.y =-16.0*y2*k3*k4*k4 + k1*k1;
		w.z = -8.0*y*k4*(x4*x4 - 28.0*x4*x2*z2 + 70.0*x4*z4 - 28.0*x2*z2*z4 + z4*z4)*k1*k2;
#else
		x = w.x; x2 = x*x; x4 = x2*x2;
		y = w.y; y2 = y*y;
		z = w.z; z2 = z*z; z4 = z2*z2;

			//^ optimized by Ken:
			//1rsqrt 33* 9+
		k3 = x2 + z2; k5 = x4 + z4; k6 = x2*z2; k2 = k3*k3;
		k4 = k3 - y2; k1 = k4*k4 - k3*y2*4.f;
		k2 = rsqrtss(k2*k2*k2*k3) * k1*k4*y;

		w.x = (k6*6.f - k5)*(z2 - x2)*x*z*k2*64.f;
		w.y = k1*k1 - y2*k3*k4*k4*16.f;
		w.z = ((k6*-28.f + k5)*k5 + x4*z4*68.f)*k2*-8.f;
#endif
		w.x += p.x; w.y += p.y; w.z += p.z;
		if (w.x*w.x + w.y*w.y + w.z*w.z > 20.f) break;
	}
	return palcur[i];
}

	//use only for testing getvox_mandelbulb_avx2() algo
static void getvox_mandelbulb_c (int wptr, float *wx, float *wy, float *wz, float *px, float *py, float *pz, int *cntind, int *outcol)
{
	float x, y, z, x2, y2, z2, x4, z4, k1, k2, k3, k4, k5, k6;
	int i, ie, rptr;

	for(rptr=0;rptr<wptr;rptr+=ie)  //Doesn't work! - count reaches -1?
	{
		ie = min(rptr+8,wptr)-rptr;
		for(i=0;i<ie;i++)
		{
			x = wx[rptr+i]; x2 = x*x; x4 = x2*x2;
			y = wy[rptr+i]; y2 = y*y;
			z = wz[rptr+i]; z2 = z*z; z4 = z2*z2;

			k3 = x2 + z2; k5 = x4 + z4; k6 = x2*z2;
			k4 = k3 - y2; k1 = k4*k4 - k3*y2*4.f;
			k2 = k3*k3; k2 = rsqrtss(k2*k2*k2*k3) * k1*k4*y;

			wx[wptr] = (k6*6.f - k5)*(z2 - x2)*x*z*k2*64.f;
			wy[wptr] = k1*k1 - y2*k3*k4*k4*16.f;
			wz[wptr] = ((k6*-28.f + k5)*k5 + x4*z4*68.f)*k2*-8.f;

			wx[wptr] += px[rptr+i]; px[wptr] = px[rptr+i];
			wy[wptr] += py[rptr+i]; py[wptr] = py[rptr+i];
			wz[wptr] += pz[rptr+i]; pz[wptr] = pz[rptr+i];
			cntind[wptr] = cntind[rptr+i]-65536;
			if ((wx[wptr]*wx[wptr] + wy[wptr]*wy[wptr] + wz[wptr]*wz[wptr] > 20.f) || (cntind[wptr] < 65536))
				  { outcol[cntind[wptr]&65535] = palcur2[cntind[wptr]>>16]; }
			else { wptr++; }
		}
	}
	//return(wptr);
}

static void mul_quat (float s1, float a1, float b1, float c1, float s2, float a2, float b2, float c2, float &s3, float &a3, float &b3, float &c3)
{
	s3 = s1*s2 - a1*a2 - b1*b2 - c1*c2;
	a3 = b1*c2 - c1*b2 + s1*a2 + s2*a1;
	b3 = c1*a2 - a1*c2 + s1*b2 + s2*b1;
	c3 = a1*b2 - b1*a2 + s1*c2 + s2*c1;
}
int getvox_quatmat_rabbit (point3d p)
{
		//3D fractal ("rabbit") by Peter Houska, (c) 2007
		//http://stud3.tuwien.ac.at/~e9907459
	float x, y, z, r, g, v, iter, iter_dec, in_s, in_a, in_b, in_c, s3, a3, b3, c3, s, a, b, c, lambda_s, lambda_a, lambda_b, lambda_c, m_s, m_a, m_b, m_c;

	x = p.x; y = p.y; z = p.z;

	iter=1;
	iter_dec=1.0/32; //smaller values for iter_dec => more accurate set-boundary approximation
		//quaternion q is represented by: q = s + i*a + j*b + k*c
	in_s=x*1.25+0.5; in_a=y*1.25; in_b=z*1.25; in_c=0.0;
	s3=in_s; a3=in_a; b3=in_b; c3=in_c;
	s=in_s; a=in_a; b=in_b; c=in_c;
	lambda_s=-0.57; lambda_a=1.0; lambda_b=0.0; lambda_c=0.0;
	m_s=0.0; m_a=0.0; m_b=0.0; m_c=0.0;
	do
	{
			//self-squaring iteration-function: q=lambda*q*(1-q)
		mul_quat(lambda_s,lambda_a,lambda_b,lambda_c, s,a,b,c, s3,a3,b3,c3);
		m_s = 1-s; m_a = -a; m_b = -b; m_c = -c;
		mul_quat(s3,a3,b3,c3, m_s,m_a,m_b,m_c, s,a,b,c);
		if (s*s + a*a + b*b + c*c >= 4) return(0); //distance-test: if already too far away => air voxel
		iter -= iter_dec;
	} while (iter > 0);
	v = 0.7*sqrt(x*x+y*y+z*z); //"spherical" color scheme
	r = v-.75; r = exp(r*r*-8.f)*255.f;
	g = v-.50; g = exp(g*g*-8.f)*255.f;
	b = v-.25; b = exp(b*b*-8.f)*255.f;
	return(rgb(r,g,b));
}

int getvox_bristorbrot (point3d p)
{
	point3d z = p, oz;
	int i;

	for(i=16;i>0;i--)
	{
		oz = z;
		z.x = oz.x*oz.x - oz.y*oz.y - oz.z*oz.z + p.x;
		z.y = (oz.x*2.f - oz.z)*oz.y + p.y;
		z.z = (oz.x*2.f + oz.y)*oz.z + p.z;
		if (z.x*z.x + z.y*z.y + z.z*z.z > 200.f) break; //return 0;
	}
	return palcur[i&7];
}

int getvox_tomwaves (point3d p) //by Tomasz Dobrowolski, 07/19/2014, originally for Graphcalc
{
	float co, si, px, py, pz, fx, fy, fz, d1, d2, d3;
	co = cos(gtim*.5);
	si = sin(gtim*.425);
	px = p.x*p.x*1.5-.50;
	py = p.y*p.y*1.5-.50;
	pz = p.z*p.z*4.0-.25;
	fx = px+co*.5; fy = py-si*.5; fz = pz+fabs(si)*.4; d1 = fx*fx+fy*fy+fz*fz;
	fx = px-co*.5; fy = py+si*.5; fz = pz+fabs(co)*.4; d2 = fx*fx+fy*fy+fz*fz;
	fz = -p.z*2-co*.1;
	d3 = p.x*p.x+p.y*p.y+fz*fz*fz;
	if (rcpss(d1) + rcpss(d2) + rcpss(d3) > 20) return(rgb(d1*256,d2*256,d3*256));
	return 0;
}

int getvox_hillebrand (point3d p)
{
	float x, y, z, d, c, ca, tt, xx, yy, zz, j, r, g, b;

	x = (p.x*.5+.5)*128;
	y = (p.y*.5+.5)*128;
	z = (p.z*.5+.5)*128;

	d = (x-64)*(x-64) + (y-64)*(y-64) + (z-64)*(z-64);
	c = 0;
	ca = 128+(int)((sin(x*.5)+cos(y*.4)+cos(z*.2))*16);
	if (d > 60*60) c = ca;
	else if ((x > 64-12) && (x < 64+12) && (y > 64-12) && (y < 64+12) && (z > 64-12) && (z < 64+12))
		c = ca+32;
	else
	{
		tt = 0;
		xx = ((int)(x-13)&31); if ((xx < 2) || ((xx >= 4) && (xx < 6))) tt++;
		yy = ((int)(y-13)&31); if ((yy < 2) || ((yy >= 4) && (yy < 6))) tt++;
		zz = ((int)(z-13)&31); if ((zz < 2) || ((zz >= 4) && (zz < 6))) tt++;
		if (tt >= 2) c = ca-32;
	}

	j = (c+16)*(c+16);
	r = j/(1156/4);
	g = j/(1256/4);
	b = j/(1656/4);
	if (c <= 0) return 0;
	return rgb(r,g,b);
}

int getvox_tominterf (point3d p)
{
	float x, y, z, r, g, b, fx, fx2, z2, f;

	x = p.x; y = p.y; z = p.z;

	fx = x+.5; fx2 = x-.5;
	z2 = (sin(fx*fx*10 + y*y*10+gtim) + sin(fx2*fx2*10 + y*y*10+gtim))*.05;
	f = max((1 - x*x - y*y)*2, 0);
	if (z < (z2+.2)*f && z > (z2-.2)*f)
	{
		r = x*50+128;
		g = z*128+128;
		b = y*50+128;
		return rgb(r,g,b);
	}
	return 0;
}

int getvox_pacman (point3d p) //by Ken Silverman, originally for Evaldraw
{
	static float glob[4];
	float x, y, z, ox, oy, ax, ay, fx, fy, fz, ix, iy, iz, r, g, b, d, t;

	x = p.x; y = p.y; z = p.z; t = gtim;

	if (fabs(z) > .34) return(0); //early-out
	if (fabs(x*x+y*y-.554) > .45) return(0); //early-out

	if (glob[0] != t) //pre-calculate cos&sin
	{     //eat counter :)
		glob[0] = t; t *= .7;
		glob[2] = cos(t); glob[3] = sin(t);
		t += .3; glob[1] = .5-.5*sin(t*16+cos(t*16)*.5);
	}

		//Put dots in motion are circle
	ox = x; oy = y;
	x = glob[2]*ox + glob[3]*oy;
	y = glob[2]*oy - glob[3]*ox;

	ax = fabs(x); ay = fabs(y); //Draw 8 dots w/symmetry
	fx = (max(ax,ay)-cos(PI/8)/sqrt(2.0));
	fy = (min(ax,ay)-sin(PI/8)/sqrt(2.0));
	if (fx*fx + fy*fy + z*z < .06*.06)
		{ r = 192; g = 255; b = 255; return(rgb(r,g,b)); }

		//Put Pacman in motion around circle
	x = (glob[2]*ox - glob[3]*oy)*3+2;
	y = (glob[2]*oy + glob[3]*ox)*3;
	z *= 3;

		//Render Pacman's eyes
	ix = .40; iy = .50; iz = .70;
	fx = fabs(x)-ix; fy = y-iy; fz = z+iz;
	if (fx*fx + fy*fy + fz*fz < .24*.24)
	{
		fx = fabs(x+glob[2]*.20)-ix;
		fy = (y-iy-.10-fabs(glob[3])*.20);
		fz = z+iz+.10;
		if (fx*fx + fy*fy + fz*fz < .15*.15) r = 0; else r = 255;
		g = r; b = r; return(rgb(r,g,b));
	}

	d = x*x+y*y+z*z;
	if (d > 1*1) return(0); //Pacman's sphere
	if (fabs(z-.1) < y*glob[1]) return(0); //mouth

	if (d < .9*.9) { r =  96; g =  96; b = 48; } //Mouth
				 else { r = 192; g = 192; b = 96; } //Body
	return(rgb(r,g,b));
}

int getvox_julia (point3d p)
{
	float x, y, z, x2, y2, z2, fx, fy, fz, d1, d2, r, g, b;
	int i;

		//Original Julia script by Inigo Quilez
	x = p.x*1.5; x2 = x*x;
	y = p.y*1.5; y2 = y*y;
	z = p.z*1.5; z2 = z*z;
	d1 = 1e32; d2 = 1e32;
	i = 9;
	do
	{
		y *= x; z *= x;
		x = x2-y2-z2-.7; x2 = x*x;
		y = y*2+.2; y2 = y*y;
		z = z*2-.5; z2 = z*z;
		if (x2+y2+z2 > 4.f) return(0);
		d1 = min(d1,x2);
		i--;
	} while (i > 0);
	d1 = sqrt(d1)*2.0;
	fx = x-.5; fy = y-.3; fz = z-.2;
	d2 = sqrt(fx*fx+fy*fy+fz*fz)*0.3;
	r = 112 + 145*d1 - 20*d2;
	g = 112 + 163*d1 - 20*d2;
	b =  92 + 163*d1 - 30*d2;
	return(rgb(r,g,b));
}

int getvox_tomquat (point3d p)
{
	float x, y, z, r, g, b, mcw, mcx, mcy, mcz, cx, cy, cz, cw, _cw, _cx, _cy, _cz;
	int i, it;

	x = p.x; y = p.y; z = p.z;
	mcw = y*0.5-.25;
	mcx = x*0.5+.6;
	mcy = z*0.5+.6;
	mcz = 0; //t*.1-1;

	cw = z;
	cx = x;
	cy = 0;
	cz = y;

	//it = t*.2+1;
	it = 10;
	for(i=0; i<it; i++)
	{
		  //do quat mul:
	  _cw = cw*cw - cx*cx - cy*cy - cz*cz  +  mcw;
	  _cx = cw*cx + cx*cw + cy*cz - cz*cy  +  mcx;
	  _cy = cw*cy + cy*cw + cz*cx - cx*cz  +  mcy;
	  _cz = cw*cz + cz*cw + cx*cy - cy*cx  +  mcz;
	  cw = -_cw;
	  cx = -_cx;
	  cy = -_cy;
	  cz = -_cz;
	  if (cw*cw + cx*cx + cy*cy + cz*cz > 4) return(0); // divergent point!
	}

	r = x*12*it + 75; //red
	g = y*12*it + 25; //green
	b = z*12*it +100; //blue
	return(rgb(r,g,b));
}

int getvox_waves (point3d p) //by Ken Silverman, originally for Graphcalc
{
	float f, a, b;
	f = p.x-.5; a = sin((sqrt(f*f + p.y*p.y + p.z*p.z)-gtim*.25)*32);
	f = p.x+.5; b = sin((sqrt(f*f + p.y*p.y + p.z*p.z)-gtim*.25)*32);
	return (a+b > 1.f)*0xffffff;
	//a = (a-b)*256; return rgb(a,a,a);
}

int getvox_roundedbox (point3d p)
{
	float f;
	p.x = fmaxf(fabsf(p.x)-0.3f,0.f);
	p.y = fmaxf(fabsf(p.y)-0.4f,0.f);
	p.z = fmaxf(fabsf(p.z)-0.1f,0.f);
	f = .05f - fminf(fmaxf(p.x,fmaxf(p.y,p.z)),0.f);
	return (p.x*p.x + p.y*p.y + p.z*p.z < f*f)*0xffff00;
}
int getvox_torus (point3d p) { p.x = sqrtf(p.x*p.x + p.y*p.y) - .2f; return (p.x*p.x + p.z*p.z <= .1f*.1f)*0xff00ff; }
int getvox_sphere (point3d p) { return (p.x*p.x + p.y*p.y + p.z*p.z <= 0.3f*0.3f)*0x00ffff; }

static const char *getvox_func_names[] =
{
	"mandelbulb AVX2",
	"mandelbulb C",
	"quatmat_rabbit",
	"bristorbrot",
	"tomwaves",
	"hillebrand",
	"tominterf",
	"pacman",
	"julia",
	"tomquat",
	"waves",
	"roundedbox",
	"torus",
	"sphere",
};

int (*getvox_funcs[])(point3d) =
{                         //Ken's ratings:
	0,                     //hack:getvox_mandelbulb_avx2
	getvox_mandelbulb,     //great
	getvox_quatmat_rabbit, //great
	getvox_bristorbrot,    //good
	getvox_tomwaves,       //weird & cool
	getvox_hillebrand,     //weird but neat
	getvox_tominterf,      //weird but cool
	getvox_pacman,         //nice
	getvox_julia,          //ok
	getvox_tomquat,        //ok
	getvox_waves,          //ok
	getvox_roundedbox,     //boring
	getvox_torus,          //boring
	getvox_sphere,         //boring
};

	//3D positioning based on algo here:
	//http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
#define PHI3D 1.22074408460575947536
static const unsigned int ixadd = (unsigned int)(4294967296.0/(PHI3D            ));
static const unsigned int iyadd = (unsigned int)(4294967296.0/(PHI3D*PHI3D      ));
static const unsigned int izadd = (unsigned int)(4294967296.0/(PHI3D*PHI3D*PHI3D));

static int gammlut[256];
typedef struct
{
	voxie_frame_t *vf;
	point3d pp, pr, pd, pf;
	int funcid, th, i0, i1;
} iter_3d_thread_args;
unsigned __stdcall iter_3d_thread (void *pargs)
{
	iter_3d_thread_args *args = (iter_3d_thread_args *)pargs;
	point3d p_world;
	__declspec(align(32)) poltex_t vt[1024];
	float f;
	unsigned int ix, iy, iz;
	int i, j, k, col, dp = 0x404040, vtn = 0, vtot = 0;

	i = args->i0;
	ix = (unsigned)i*ixadd;
	iy = (unsigned)i*iyadd;
	iz = (unsigned)i*izadd;

	if (getvox_funcs[args->funcid])
	{
		for(;i<args->i1;i++)
		{
			*(int *)&f = (ix>>9)^0x40400000; ix += ixadd; vt[vtn].x = (f-3.f)*vw.aspx;//{-vw.aspx..+vw.aspx}
			*(int *)&f = (iy>>9)^0x40400000; iy += iyadd; vt[vtn].y = (f-3.f)*vw.aspy;//{-vw.aspy..+vw.aspy}
			*(int *)&f = (iz>>9)^0x40400000; iz += izadd; vt[vtn].z = (f-3.f)*vw.aspz;//{-vw.aspz..+vw.aspz}
			p_world.x = vt[vtn].x*args->pr.x + vt[vtn].y*args->pd.x + vt[vtn].z*args->pf.x + args->pp.x;
			p_world.y = vt[vtn].x*args->pr.y + vt[vtn].y*args->pd.y + vt[vtn].z*args->pf.y + args->pp.y;
			p_world.z = vt[vtn].x*args->pr.z + vt[vtn].y*args->pd.z + vt[vtn].z*args->pf.z + args->pp.z;

			col = getvox_funcs[args->funcid](p_world);

				//dither algo - need for good color representation
			dp = (dp&0x7f7f7f) + (gammlut[(col>>16)&255]<<16) + (gammlut[(col>>8)&255]<<8) + gammlut[col&255];
			col = (dp&0x808080); if (!col) continue;
			vt[vtn].col = (col<<1)-(col>>7) /*((col*255)>>7)*/;

			vtn++; if (vtn >= sizeof(vt)/sizeof(vt[0])) { voxie_drawmeshtex(args->vf,0,vt,vtn,0,0,0,0xffffff); vtot += vtn; vtn = 0; } //buffer full - render voxel batch
		}
		if (vtn) { voxie_drawmeshtex(args->vf,0,vt,vtn,0,0,0,0xffffff); vtot += vtn; } //flush (render) remaining voxels in buffer
	}
	else //crazy fast&convoluted (;-P) avx2 code specifically written for just mandelbulb..
	{
			// 96:5.69e6
			//128:5.77e6
			//160:5.82e6
			//192:5.84e6/1.88e6 = 3.106x faster
			//224:5.80e6
			//256:5.71e6
		#define MEMSIZ 192 //should be multiple of 8 and also <=~1024 to prevent stack overrun!
		#define ITERMAX (sizeof(palcur)/sizeof(palcur[0]))
		#define MEMMAL (MEMSIZ*ITERMAX+8)
		__declspec(align(32)) float px[MEMMAL], py[MEMMAL], pz[MEMMAL];
		__declspec(align(32)) float wx[MEMMAL], wy[MEMMAL], wz[MEMMAL];
		__declspec(align(32)) int cntind[MEMMAL]; //hi16:fractal cnt, lo16:orig memsiz index
		__declspec(align(32)) int outcol[MEMSIZ]; //result array

		for(j=MEMSIZ-1;j>=0;j--) { cntind[j] = j + (8<<16); }

		while (i < args->i1)
		{
			vtn = min(i+MEMSIZ,args->i1)-i;
#if 0
			for(j=0;j<vtn;j++)
			{
				//*(int *)&f = (ix>>9)^0x40400000; ix += ixadd; vt[j].x = (f-3.f)*vw.aspx;//{-vw.aspx..+vw.aspx}
				//*(int *)&f = (iy>>9)^0x40400000; iy += iyadd; vt[j].y = (f-3.f)*vw.aspy;//{-vw.aspy..+vw.aspy}
				//*(int *)&f = (iz>>9)^0x40400000; iz += izadd; vt[j].z = (f-3.f)*vw.aspz;//{-vw.aspz..+vw.aspz}
				vt[j].x = ((float)(signed int)ix)*(vw.aspx/2147483648.f); ix += ixadd;
				vt[j].y = ((float)(signed int)iy)*(vw.aspy/2147483648.f); iy += iyadd;
				vt[j].z = ((float)(signed int)iz)*(vw.aspz/2147483648.f); iz += izadd;
				px[j] = vt[j].x*args->pr.x + vt[j].y*args->pd.x + vt[j].z*args->pf.x + args->pp.x;
				py[j] = vt[j].x*args->pr.y + vt[j].y*args->pd.y + vt[j].z*args->pf.y + args->pp.y;
				pz[j] = vt[j].x*args->pr.z + vt[j].y*args->pd.z + vt[j].z*args->pf.z + args->pp.z;
			}
			memcpy(wx,px,vtn*sizeof(wx[0]));
			memcpy(wy,py,vtn*sizeof(wy[0]));
			memcpy(wz,pz,vtn*sizeof(wz[0]));
#else
			getvox_xform_avx2(vtn,wx,wy,wz,px,py,pz,vt,ix,iy,iz);
			ix += ixadd*(unsigned)MEMSIZ;
			iy += iyadd*(unsigned)MEMSIZ;
			iz += izadd*(unsigned)MEMSIZ;
#endif
			i += vtn;
			//getvox_mandelbulb_c(vtn,wx,wy,wz,px,py,pz,cntind,outcol);
			getvox_mandelbulb_avx2(vtn,wx,wy,wz,px,py,pz,cntind,outcol);

			k = 0;
			for(j=0;j<vtn;j++)
			{
				col = (((dp&0x7f7f7f) + outcol[j])&0x808080); if (!col) continue; //NOTE:outcol uses burnt-in gamma!
				vt[k].x = vt[j].x; vt[k].y = vt[j].y; vt[k].z = vt[j].z; vt[k].col = (col<<1)-(col>>7) /*((col*255)>>7)*/; k++;
			}
			if (k) voxie_drawmeshtex(args->vf,0,vt,k,0,0,0,0xffffff);
			vtot += vtn; vtn = 0;
		}
	}

	//voxie_debug_print6x8_(0,args->th*8+256,0xffffff,-1,"%d",vtot);

	if (args->th >= 0) _endthreadex(0);
	return 0;
}

static int gmenuc = -1;
static float gmenuv = 0;
enum menuStates
{
	MENU_PREV_SHAPE, MENU_NEXT_SHAPE, MENU_TARGET_VPS,
	MENU_RESET_POSORI, MENU_TOGGLE_PALLETE, MENU_CONTROL_INVERT,
	MENU_CONTROL_NORMAL
};
static int menu_fractals_update (int id, char *st, double v, int how, void *userdata)
{
	gmenuc = id;
	switch(id)
	{
		case MENU_TARGET_VPS: gmenuc = MENU_TARGET_VPS; gmenuv = (int)v; break;
	}
	return(1);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	point3d pp, pr, pd, pf;
	double tim = 0.0, otim, dtim;
	float f, scale, targvps = 30.f, ogamma = -1.f;
	int i, j, k, key, funcid = 0, ndots = (1<<21), palind = 0, ctrldir = 1, resetposori = 1;
	voxie_nav_t nav[4] = {0};
	int onavbut[4];

	if (voxie_load(&vw) < 0) return -1;
	vw.usecol = 1; //Override color rendering
	if (voxie_init(&vw) < 0) return -1;

		//add menu
	voxie_menu_reset(menu_fractals_update,0,0);
	voxie_menu_addtab("Fractals",350,0,650,400);
	voxie_menu_additem("Previous Shape" , 60, 25,200,75,MENU_PREV_SHAPE    ,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Next Shape"     ,400, 25,200,75,MENU_NEXT_SHAPE    ,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("Target VPS", 100,125,450,64,   MENU_TARGET_VPS ,MENU_HSLIDER ,5,0xffff80,15,5,30,1,5); //.., start, low, high, min tick, maj tick);
	voxie_menu_additem("<-detail\t\t\t\t\t\t\t speed->",100,200,64,64, 0,MENU_TEXT,0,0xffff00,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("Control Method:", 55,265, 64,64,                  0,MENU_TEXT    ,0,0x00ff80,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Normal"         ,250,240,175,64,MENU_CONTROL_NORMAL,MENU_BUTTON+1,1,0x808080,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Invert"         ,425,240,175,64,MENU_CONTROL_INVERT,MENU_BUTTON+2,0,0x808080,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("Cycle Palette"  , 70,320,200,64,MENU_TOGGLE_PALLETE,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Reset Pos&Ori"  ,370,320,200,64,MENU_RESET_POSORI  ,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);

	for(i=0;i<256;i++) //init compactlut..
	{
		for(k=0,j=0;j<8;j++) { if ( ((1<<j)&i)) { compactlut[i*8+k] = j; k++; } }
		for(    j=0;j<8;j++) { if (!((1<<j)&i)) { compactlut[i*8+k] = j; k++; } }
	}

		//get thread count
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int nthreads = max(sysinfo.dwNumberOfProcessors-(!vw.useemu),1);

	HANDLE *thread_handles = (HANDLE *)malloc(nthreads*sizeof(thread_handles[0]));
	iter_3d_thread_args *thread_args = (iter_3d_thread_args *)malloc(nthreads*sizeof(thread_args[0]));

		//main loop
	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; gtim = tim;

		while (key = voxie_keyread()) //get buffered key (low 8 bits is ASCII code, next 8 bits is keyboard scancode - for more exotic keys)
		{
			if ((char)(key>>8) == 1) { voxie_quitloop(); } //ESC:quit
			if (((char)key == '-') || ((char)key == '_')) { targvps = fmaxf(targvps-1.f,  3.f); }
			if (((char)key == '+') || ((char)key == '=')) { targvps = fminf(targvps+1.f,120.f); }
			if ((char)key == ',') { palind--; if (palind < 0) palind = sizeof(paltab)/sizeof(palcur)-1; }
			if ((char)key == '.') { palind++; if (palind >= sizeof(paltab)/sizeof(palcur)) palind = 0; }
			if (((char)key == 'I') || ((char)key == 'i')) { ctrldir = -ctrldir; }
			if (((char)key == '/') || ((char)key == ' ')) { resetposori = 1; }
			if (((char)key == ';') || ((unsigned char)(key>>8) == 0xc9)) { funcid--; if (funcid < 0) funcid = sizeof(getvox_funcs)/sizeof(getvox_funcs[0])-1; /*resetposori = 1;*/ } //PGUP
			if (((char)key == '\'') || ((unsigned char)(key>>8) == 0xd1)) { funcid++; if (funcid >= sizeof(getvox_funcs)/sizeof(getvox_funcs[0])) funcid = 0; /*resetposori = 1;*/ } //PGDN
		}

		if (gmenuc != -1)
		{
			if (gmenuc == MENU_TARGET_VPS)     { targvps = gmenuv; }
			if (gmenuc == MENU_TOGGLE_PALLETE) { palind++; if (palind >= sizeof(paltab)/sizeof(palcur)) palind = 0; }
			if (gmenuc == MENU_CONTROL_INVERT) { ctrldir =-1; }
			if (gmenuc == MENU_CONTROL_NORMAL) { ctrldir = 1; }
			if (gmenuc == MENU_RESET_POSORI)   { resetposori = 1; }
			if (gmenuc == MENU_PREV_SHAPE)     { funcid--; if (funcid < 0) { funcid = sizeof(getvox_funcs)/sizeof(getvox_funcs[0])-1; } resetposori = 1; } //PGUP
			if (gmenuc == MENU_NEXT_SHAPE)     { funcid++; if (funcid >= sizeof(getvox_funcs)/sizeof(getvox_funcs[0])) { funcid = 0; } resetposori = 1; } //PGDN
			gmenuc = -1;
		}

		for(i=0;i<4;i++) { onavbut[i] = nav[i].but; voxie_nav_read(i,&nav[i]); }

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
				  if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		if (resetposori) { resetposori = 0; pp = {0.f,0.f,0.f}, pr = {1.f,0.f,0.f}, pd = {0.f,1.f,0.f}, pf = {0.f,0.f,1.f}; scale = 1.f; }

		if (voxie_keystat(0x1e) || (nav[0].but&1)) scale /= pow(1.5,dtim); //A
		if (voxie_keystat(0x2c) || (nav[0].but&2)) scale *= pow(1.5,dtim); //Z
		f = scale*dtim*ctrldir*.005f;
		nav[0].dx -= ((voxie_keystat(0xcb)!=0) - (voxie_keystat(0xcd)!=0))*128;
		nav[0].dy -= ((voxie_keystat(0xc8)!=0) - (voxie_keystat(0xd0)!=0))*128;
		nav[0].dz -= ((voxie_keystat(0x9d)!=0) - (voxie_keystat(0x52)!=0))*128;
		pp.x -= (nav[0].dx*pr.x + nav[0].dy*pd.x + nav[0].dz*pf.x)*f;
		pp.y -= (nav[0].dx*pr.y + nav[0].dy*pd.y + nav[0].dz*pf.y)*f;
		pp.z -= (nav[0].dx*pr.z + nav[0].dy*pd.z + nav[0].dz*pf.z)*f;
		rotate_vex(nav[0].az*dtim*ctrldir*-.005f,&pr,&pd);
		rotate_vex(nav[0].ay*dtim*ctrldir*-.005f,&pd,&pf);
		rotate_vex(nav[0].ax*dtim*ctrldir*-.005f,&pr,&pf);

		if (vw.gamma != ogamma) //used for dithering in thread
		{
			ogamma = vw.gamma;
			for(i=0;i<256;i++) { gammlut[i] = (int)(pow(((float)i/255.f),vw.gamma)*128.f); }
		}

		//------------------------------------------------------------------------
			//start render
		voxie_frame_start(&vf); vf.flags |= VOXIEFRAME_FLAGS_IMMEDIATE;
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, +vw.aspx, +vw.aspy, +vw.aspz);

			//draw wireframe box
		voxie_drawbox(&vf, -vw.aspx, -vw.aspy, -vw.aspz, +vw.aspx, +vw.aspy, +vw.aspz, 1, 0xffffff);

		for(i=0;i<sizeof(palcur)/sizeof(palcur[0]);i++)
		{
			palcur[i] = paltab[palind][i];
			palcur2[i] = (gammlut[(palcur[i]>>16)&255]<<16) + (gammlut[(palcur[i]>>8)&255]<<8) + gammlut[palcur[i]&255]; //optimization:burnt-in gamma ;)
		}

		ndots += (int)((1.f/targvps - dtim)*4194304.f); //Auto-adjust ;)
		ndots = min(max(ndots, 1<<15), 1<<27);

		for(i=8-1;i>=0;i--)
		{
			qqxoff[i] = ixadd*(unsigned)i; qqxadd8[i] = ixadd*8;
			qqyoff[i] = iyadd*(unsigned)i; qqyadd8[i] = iyadd*8;
			qqzoff[i] = izadd*(unsigned)i; qqzadd8[i] = izadd*8;
			qqxmul[i] = vw.aspx/2147483648.f;
			qqymul[i] = vw.aspy/2147483648.f;
			qqzmul[i] = vw.aspz/2147483648.f;
			qqpxs[i] = pp.x      ; qqpys[i] = pp.y      ; qqpzs[i] = pp.z      ;
			qqrxs[i] = pr.x*scale; qqrys[i] = pr.y*scale; qqrzs[i] = pr.z*scale;
			qqdxs[i] = pd.x*scale; qqdys[i] = pd.y*scale; qqdzs[i] = pd.z*scale;
			qqfxs[i] = pf.x*scale; qqfys[i] = pf.y*scale; qqfzs[i] = pf.z*scale;
		}

#if 1
			//Start dot render threads
		int dotsperthread = ndots/nthreads;
		for(i=0;i<nthreads;i++)
		{
			thread_args[i].th = i;
			thread_args[i].i0 = (i+0)*dotsperthread;
			thread_args[i].i1 = (i+1)*dotsperthread;
			thread_args[i].vf = &vf;
			thread_args[i].funcid = funcid;
			thread_args[i].pp = pp;
			thread_args[i].pr.x = pr.x*scale; thread_args[i].pr.y = pr.y*scale; thread_args[i].pr.z = pr.z*scale;
			thread_args[i].pd.x = pd.x*scale; thread_args[i].pd.y = pd.y*scale; thread_args[i].pd.z = pd.z*scale;
			thread_args[i].pf.x = pf.x*scale; thread_args[i].pf.y = pf.y*scale; thread_args[i].pf.z = pf.z*scale;
			thread_handles[i] = (HANDLE)_beginthreadex(NULL, 0, &iter_3d_thread, &thread_args[i], 0, NULL);
		}
		WaitForMultipleObjects(nthreads, thread_handles, TRUE, INFINITE); //Wait for threads to finish up
		for(i=nthreads-1;i>=0;i--) { CloseHandle(thread_handles[i]); }
#else
		thread_args[0].th = -1;
		thread_args[0].i0 = 0;
		thread_args[0].i1 = ndots;
		thread_args[0].vf = &vf;
		thread_args[0].funcid = funcid;
		thread_args[0].pp = pp;
		thread_args[0].pr.x = pr.x*scale; thread_args[0].pr.y = pr.y*scale; thread_args[0].pr.z = pr.z*scale;
		thread_args[0].pd.x = pd.x*scale; thread_args[0].pd.y = pd.y*scale; thread_args[0].pd.z = pd.z*scale;
		thread_args[0].pf.x = pf.x*scale; thread_args[0].pf.y = pf.y*scale; thread_args[0].pf.z = pf.z*scale;
		iter_3d_thread((void *)&thread_args[0]);
#endif

		voxie_debug_print6x8_(0,64,0xffffff,-1,"Target vps: %.1f\n"
															"Actual vps: %.1f\n"
															"Dots: %i\n"
															"Func#: %i (%s)\n"
															"Pal#: %i",targvps,1.f/dtim,ndots,funcid,getvox_func_names[funcid],palind);
		voxie_debug_print6x8_(0,116,0x00ff00,-1,"Holographic Fractal Explorer:\n"
															 "\tStarted by Juraj Tomori (https://github.com/jtomori)\n"
															 "\tHeavily modified & optimized by Ken Silverman\n"
															 "\tSDF functions: https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm");
		voxie_debug_print6x8_(0,158,0xff9000,-1,"Controls:\n"
															 "\t- + or - =\t\t\t  Adjust target volume rate\n"
															 "\tPGUP/DN or ; '\t\t Select function (shape)\n"
															 "\t, .\t\t\t\t\t\tSelect palette (fractals only)\n"
															 "\tA Z or SpaceNav But  Scale Object\n"
															 "\tArrows RCTRL KP0\t  Scroll pos\n"
															 "\tSpaceNav\t\t\t\t Scroll pos&ori\n"
															 "\t/ Space\t\t\t\t  Reset pos&ori\n"
															 "\tShift/Ctrl + [/]\t  Adjust emulation viewpoint/size\n"
															 "\tI\t\t\t\t\t\t  Invert control direction (i.e. control camera vs object)");

		voxie_frame_end();
		//------------------------------------------------------------------------
		voxie_getvw(&vw); //get any menu ('\') changes
	}

	voxie_uninit(0); //close window / unload voxiebox.dll
	return 0;
}

#if 0
!endif
#endif
