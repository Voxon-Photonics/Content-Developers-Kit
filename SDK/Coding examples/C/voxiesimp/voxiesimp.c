#if 0
!if 1

	#Visual C makefile:
voxiesimp.exe: voxiesimp.c voxiebox.h; cl /TP voxiesimp.c /Ox /MT /link user32.lib
	del voxiesimp.obj

!else

	#GNU C makefile:
voxiesimp.exe: voxiesimp.c; gcc voxiesimp.c -o voxiesimp.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

	//Voxiebox example for C. Requires voxiebox.dll to be in path.
	//NOTE:voxiebox.dll is compiled as 64-bit mode. Be sure to compile for 64-bit target or else voxie_load() will fail.
	//To compile, type "nmake voxiesimp.c" at a VC command prompt, or set up in environment using hints from makefile above.
	//Code by Ken Silverman
#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif

static voxie_wind_t vw;

static int gcnti[2], gbstat = 0;
static void mymix (int *ibuf, int nsamps)
{
	static int cnt[2]; int i, c;
	for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)
			{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&gbstat; cnt[c] += gcnti[c]; }
}

	//Rotate vectors a & b around their common plane, by ang
static void rotvex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim;
	int i, j, x, y, z, mousx = 256, mousy = 256, mousz = 0;
	point3d pp, rr, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int vxnplays;


	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }

	if (vw.playsamprate)
	{
		gcnti[0] = (262<<21)/vw.playsamprate;
		gcnti[1] = (392<<21)/vw.playsamprate;
		voxie_setaudplaycb(mymix);
	}

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);

		for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break;
		}

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		if (voxie_keystat(0x39) == 1) { voxie_playsound("c:/windows/media/tada.wav",0,100,100,1.f); }

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
				  if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

#if 0
			//Ridiculously slow brute force way to fill entire volume
		for(z=0;z<vw.framepervol*24;z++)
			for(y=vf.y0;y<vf.y1;y++)
				for(x=vf.x0;x<vf.x1;x++)
					*(int *)((z/24)*vf.fp + y*vf.p + x*4 + vf.f) |= (1<<(z%24));
#endif

			//draw wireframe box
		if (!vw.clipshape)
		{
			voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		}
		else
		{
			int n = 64;
			for(j=-64;j<=64;j++)
			{
				if (j == -62) j = 62;
				for(i=0;i<n;i++)
				{
					voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f,
											cos((float)(i+1)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+1)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f, 0xffffff);
				}
			}

			n = 32;
			for(i=0;i<n;i++)
			{
				voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, -vw.aspz,
										cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, +vw.aspz, 0xffffff);
			}
		}
#if 1
		if (vw.clipshape != 1) { vw.clipshape = 1; voxie_init(&vw); }

		for(i=0;i<16;i++)
			for(j=0;j<4;j++)
			{
				if ((!j) && (i&1)) continue;
				for(z=-1;z<=1;z++)
				{
					float fx, fy, fz;
					fx = cos((double)i*(PI*2.0/16.0)) * ((double)j+.85) * vw.aspr/4.0;
					fy = sin((double)i*(PI*2.0/16.0)) * ((double)j+.85) * vw.aspr/4.0;
					fz = (float)z*vw.aspz*0.65f;
					voxie_drawsph(&vf,fx,fy,fz,.08f,1,0xffffff);
				}
			}

		//rr.x = 0.39f; dd.x = 0.00f; ff.x = 0.00f; pp.x = ((float)(mousx&511))/256.0f-1.0f;
		//rr.y = 0.00f; dd.y = 0.39f; ff.y = 0.00f; pp.y = ((float)(mousy&511))/256.0f-1.0f;
		//rr.z = 0.00f; dd.z = 0.00f; ff.z = 0.39f; pp.z = 0.0f;
		//voxie_drawspr(&vf,"caco.kvs",&pp,&rr,&dd,&ff,0x808080);

#elif 0
		static tiletype img = {0};
		if (!img.f) { img.x = 512; img.y = 512; img.p = (INT_PTR)img.x*4; img.f = (INT_PTR)malloc(img.p*img.y); }

		for(y=0;y<img.y;y++)
			for(x=0;x<img.x;x++)
			{
				int rgb24 = (128<<16)+(y<<8)+x, hgt8 = sqrt(256.f*256.f - (x-256)*(x-256) - (y-256)*(y-256));
				*(int *)(img.p*y + (x<<2) + img.f) = rgb24 + (hgt8<<24);
			}

		point3d pp, rr, dd, ff;
		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
		voxie_drawheimap(&vf,(char *)&img, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);
#elif 0
			//draw various primitives
		voxie_drawvox(&vf,-0.5f,0.0f,0.0f,0xffffff);
		voxie_drawlin(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz,0x010101);
		voxie_drawsph(&vf,pos.x,pos.y,pos.z,0.1f,0,0xffffff);
		pt[0].x =-0.833; pt[0].y =-0.178; pt[0].z =-vw.aspz; pt[0].p2 = 1;
		pt[1].x =-0.298; pt[1].y =-0.178; pt[1].z =     0.0; pt[1].p2 = 2;
		pt[2].x =-0.536; pt[2].y = 0.129; pt[2].z =+vw.aspz; pt[2].p2 = 0;
		voxie_drawpol(&vf,pt,3,0xffffff);

		rr.x = 0.2; dd.x = 0.0;          pp.x = -0.7;
		rr.y = 0.0; dd.y = cos(tim)*0.4; pp.y = +0.7 + dd.y*-.5;
		rr.z = 0.0; dd.z = sin(tim)*0.4; pp.z = dd.z*-.5;
		voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"HI * %d",(int)tim);

		rr.x = 0.39f; dd.x = 0.00f; ff.x = 0.00f; pp.x = ((float)(mousx&511))/256.0f-1.0f;
		rr.y = 0.00f; dd.y = 0.39f; ff.y = 0.00f; pp.y = ((float)(mousy&511))/256.0f-1.0f;
		rr.z = 0.00f; dd.z = 0.00f; ff.z = 0.39f; pp.z = 0.0f;
		voxie_drawspr(&vf,"caco.kvs",&pp,&rr,&dd,&ff,0x808080);
		pp.x += (rr.x + dd.x + ff.x)*-0.5f;
		pp.y += (rr.y + dd.y + ff.y)*-0.5f;
		pp.z += (rr.z + dd.z + ff.z)*-0.5f;
		voxie_drawcube(&vf,&pp,&rr,&dd,&ff,1,0xffffff);
#elif 0
		{ //moire
		static point3d pos[2], inc[2], tpos[2], tinc[2];
		static int inited = 0;

		if (!inited)
		{
			inited = 1;
			for(i=0;i<2;i++)
			{
				pos[i].x = (float)(rand()-16384)/16384.f; inc[i].x = (float)(rand()-16384)/32768.f;
				pos[i].y = (float)(rand()-16384)/16384.f; inc[i].y = (float)(rand()-16384)/32768.f;
				pos[i].z = (float)(rand()-16384)/16384.f; inc[i].z = (float)(rand()-16384)/32768.f;
			}
		}
		for(i=0;i<2;i++)
		{
			pos[i].x += inc[i].x*dtim; if (fabs(pos[i].x) > vw.aspx-.1) inc[i].x = fabs(inc[i].x)*((pos[i].x<0.0)*2-1);
			pos[i].y += inc[i].y*dtim; if (fabs(pos[i].y) > vw.aspy-.1) inc[i].y = fabs(inc[i].y)*((pos[i].y<0.0)*2-1);
			pos[i].z += inc[i].z*dtim; if (fabs(pos[i].z) > vw.aspz-.1) inc[i].z = fabs(inc[i].z)*((pos[i].z<0.0)*2-1);
			tpos[i] = pos[i]; tinc[i] = inc[i];
		}
		for(j=0;j<32;j++)
		{
			for(z=-1;z<=1;z+=2)
				for(y=-1;y<=1;y+=2)
					for(x=-1;x<=1;x+=2)
					{
						voxie_drawlin (&vf,tpos[0].x*x,tpos[0].y*y,tpos[0].z*z,
												 tpos[1].x*x,tpos[1].y*y,tpos[1].z*z,0x80c080);
						//voxie_drawcone(&vf,tpos[0].x,tpos[0].y,tpos[0].z,.02f,
						//                   tpos[1].x,tpos[1].y,tpos[1].z,.02f,1,0x80c080);
					}
			for(i=0;i<2;i++)
			{
				tpos[i].x -= tinc[i].x*.10f; if (tpos[i].x <-vw.aspx+.1) { tinc[i].x = -fabs(tinc[i].x); tpos[i].x = (-vw.aspx+.1)*2.f-tpos[i].x; }
													  if (tpos[i].x > vw.aspx-.1) { tinc[i].x = +fabs(tinc[i].x); tpos[i].x = ( vw.aspx-.1)*2.f-tpos[i].x; }
				tpos[i].y -= tinc[i].y*.10f; if (tpos[i].y <-vw.aspy+.1) { tinc[i].y = -fabs(tinc[i].y); tpos[i].y = (-vw.aspy+.1)*2.f-tpos[i].y; }
													  if (tpos[i].y > vw.aspy-.1) { tinc[i].y = +fabs(tinc[i].y); tpos[i].y = ( vw.aspy-.1)*2.f-tpos[i].y; }
				tpos[i].z -= tinc[i].z*.10f; if (tpos[i].z <-vw.aspz+.1) { tinc[i].z = -fabs(tinc[i].z); tpos[i].z = (-vw.aspz+.1)*2.f-tpos[i].z; }
													  if (tpos[i].z > vw.aspz-.1) { tinc[i].z = +fabs(tinc[i].z); tpos[i].z = ( vw.aspz-.1)*2.f-tpos[i].z; }
			}
		}
		}
#elif 0
		static float ang = 0.f;
		point2d vert[4];
		float f, g = .02f;
		int j;

		f = vw.aspz-g;
		pp.x = ((float)(mousx&511))/256.0f-1.0f;
		pp.y = ((float)(mousy&511))/256.0f-1.0f;
		ang += (float)in.dmousz*.001f;
		for(i=0;i<4;i++)
		{
			vert[i].x = pp.x + cos((float)i*PI*.5f+ang)*f - sin((float)i*PI*.5f+ang)*f;
			vert[i].y = pp.y + sin((float)i*PI*.5f+ang)*f + cos((float)i*PI*.5f+ang)*f;
		}
		for(i=4-1,i=0;i<4;j=i,i++)
		{
			voxie_drawcone(&vf,vert[i].x,vert[i].y,-f,g,vert[j].x,vert[j].y,-f,g,1,0xffffff);
			voxie_drawcone(&vf,vert[i].x,vert[i].y,+f,g,vert[j].x,vert[j].y,+f,g,1,0xffffff);
			voxie_drawcone(&vf,vert[i].x,vert[i].y,-f,g,vert[i].x,vert[i].y,+f,g,1,0xffffff);
		}
#elif 0
		point3d pr, pd, pf;
		float f, g = .02f, fx, fy, fz;

		if (fabs(vw.aspz-0.41f) > 1e-3) { vw.aspz = 0.41f; voxie_init(&vw); }
		fx = 0.5f; fy = 0.f; fz = 0.f;
		f = 0.492;
		pr.x =   f; pd.x = 0.f; pf.x = 0.f;
		pr.y = 0.f; pd.y =   f; pf.y = 0.f;
		pr.z = 0.f; pd.z = 0.f; pf.z =   f;
		rotvex(tim,&pr,&pd);
		for(g=-1.f;g<=1.f;g+=2.f)
			for(f=-1.f;f<=1.f;f+=2.f)
			{
				voxie_drawcone(&vf,(- pr.x + pd.x*f + pf.x*g)*.5f + fx, (- pr.y + pd.y*f + pf.y*g)*.5f + fy, (- pr.z + pd.z*f + pf.z*g)*.5f + fz, 0.007f,
										 (+ pr.x + pd.x*f + pf.x*g)*.5f + fx, (+ pr.y + pd.y*f + pf.y*g)*.5f + fy, (+ pr.z + pd.z*f + pf.z*g)*.5f + fz, 0.007f, 1, 0xffffff);
				voxie_drawcone(&vf,(+ pr.x*f - pd.x + pf.x*g)*.5f + fx, (+ pr.y*f - pd.y + pf.y*g)*.5f + fy, (+ pr.z*f - pd.z + pf.z*g)*.5f + fz, 0.007f,
										 (+ pr.x*f + pd.x + pf.x*g)*.5f + fx, (+ pr.y*f + pd.y + pf.y*g)*.5f + fy, (+ pr.z*f + pd.z + pf.z*g)*.5f + fz, 0.007f, 1, 0xffffff);
				voxie_drawcone(&vf,(+ pr.x*f + pd.x*g - pf.x)*.5f + fx, (+ pr.y*f + pd.y*g - pf.y)*.5f + fy, (+ pr.z*f + pd.z*g - pf.z)*.5f + fz, 0.007f,
										 (+ pr.x*f + pd.x*g + pf.x)*.5f + fx, (+ pr.y*f + pd.y*g + pf.y)*.5f + fy, (+ pr.z*f + pd.z*g + pf.z)*.5f + fz, 0.007f, 1, 0xffffff);
			}
#elif 0
		if (fabs(vw.aspz-0.41f) > 1e-3) { vw.aspz = 0.41f; voxie_init(&vw); }
		voxie_drawcone(&vf,(+cos(.8f)*.492f - sin(.8f)*-.492f)*.5f + .5f, (+sin(.8f)*.492f - cos(.8f)*.492f)*.5f, -.492f*.5f, 0.007f,
								 (+cos(.8f)*.492f + sin(.8f)*-.492f)*.5f + .5f, (+sin(.8f)*.492f + cos(.8f)*.492f)*.5f, -.492f*.5f, 0.007f, 1, 0xffffff);
#elif 0
		int i, j;
		float f, g, ff, gg, c;

		f = 6;
		voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,vw.aspx*f,vw.aspy*f,vw.aspz*f);

			//Rounded cones flying around torus coil
		for(i=-128;i<128;i+=2)
		{
			j = i+1;
			f = cos(i*PI*2/16+tim)+4; ff = (i+tim*2)*PI/128;
			g = cos(j*PI*2/16+tim)+4; gg = (j+tim*2)*PI/128;
			c = cos((i+64)*PI/128)*.125+1.0;
			voxie_drawcone(&vf,cos(ff)*f,sin(ff)*f,sin(i*PI*2/16+tim),.2,
									 cos(gg)*g,sin(gg)*g,sin(j*PI*2/16+tim),.3,1,(((int)min(max(c*.80*256,0),255))<<16) + (((int)min(max(c*.90*256,0),255))<<8) + (((int)min(max(c*1.0+256,0),255))));
		}

			//Water drops flying around circle
		for(i=-64;i<64;i+=4)
		{
			f = (i  )*PI*2/64+tim;
			g = (i+3)*PI*2/64+tim;
			voxie_drawcone(&vf,cos(f)*4,sin(f)*4,0,.01,cos(g)*4,sin(g)*4,0,.25,1,0xc0c060>>0);
		}

			//Blue nucleus
		voxie_drawcone(&vf,0,0,0,1.2,0,0,0,1.2,1,0x60c0a0>>0);

#else

			//texture-mapped tetrahedron..
		poltex_t vt[4]; int mesh[16], i = 0;
		double c, s;
		float f = vw.aspz; //0.29;
		vt[0].x =-f; vt[0].y =-f; vt[0].z =-f; vt[0].u = 0.f; vt[0].v = 0.f; vt[0].col = 0xffffff;
		vt[1].x =-f; vt[1].y =+f; vt[1].z =+f; vt[1].u = 1.f; vt[1].v = 0.f; vt[1].col = 0xffffff;
		vt[2].x =+f; vt[2].y =-f; vt[2].z =+f; vt[2].u = 1.f; vt[2].v = 1.f; vt[2].col = 0xffffff;
		vt[3].x =+f; vt[3].y =+f; vt[3].z =-f; vt[3].u = 0.f; vt[3].v = 1.f; vt[3].col = 0xffffff;
		c = cos(tim*.5); s = sin(tim*.5);
		for(i=0;i<4;i++)
		{
			d = vt[i].x; vt[i].x = d*c + vt[i].y*s; vt[i].y = d*s - vt[i].y*c;
			d = vt[i].x; vt[i].x = d*c + vt[i].z*s; vt[i].z = d*s - vt[i].z*c;
			d = vt[i].y; vt[i].y = d*c + vt[i].z*s; vt[i].z = d*s - vt[i].z*c;
		}
		i = 0;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1; /*-1 = end of polygonal facet*/
		mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;
		voxie_drawmeshtex(&vf,"usflag.png",vt,4,mesh,i,2,0x404040);
#endif

			//make pos bounce around
		pos.x += inc.x*dtim; if (fabs(pos.x) > vw.aspx-.1) inc.x = fabs(inc.x)*((pos.x<0.0)*2-1);
		pos.y += inc.y*dtim; if (fabs(pos.y) > vw.aspy-.1) inc.y = fabs(inc.y)*((pos.y<0.0)*2-1);
		pos.z += inc.z*dtim; if (fabs(pos.z) > vw.aspz-.1) inc.z = fabs(inc.z)*((pos.z<0.0)*2-1);

			//draw silly test debug stuff
		x = 64; y = 128;
		voxie_debug_drawrectfill(x-60,y-60,x+60,y+60,0x0080ff);
		voxie_debug_drawcircfill(x,y,50,0x404040);
		voxie_debug_drawcirc(x,y,50,0xff8000);
		voxie_debug_drawline(x,y,cos(tim*PI*2)*50+x,sin(tim*PI*2)*50+y,0x00ff00);
		voxie_debug_drawhlin(x-20,x+20,y+20,0xff0000);
		voxie_debug_drawpix(x-15,y-20,0xffffff);
		voxie_debug_drawpix(x+15,y-20,0xffffff);
		voxie_debug_print6x8_(x-30,y-60,0xffc080,-1,"Hello %5.2f",tim);

		voxie_debug_drawcircfill(1400,50,50,0xff8000);
		voxie_debug_drawcircfill(640,300,50,0xff8000);


		for(i=0;i<vxnplays;i++)
		{
			voxie_debug_print6x8_(0,256+i*8,0xffffff,-1,"%04x %04x %04x %04x %04x %04x %04x %04x",
				(unsigned short)vx[i].but,(unsigned short)vx[i].tx0,(unsigned short)vx[i].ty0,
				(unsigned short)vx[i].tx1,(unsigned short)vx[i].ty1,
				(unsigned short)vx[i].lt,(unsigned short)vx[i].rt,
				(unsigned short)vx[i].hat);
		}


		voxie_frame_end(); voxie_getvw(&vw);

		if (voxie_keystat(0x3a)) //CapsLock:Intentional crash :P
		{
			*(int *)0 = -17; //cause page fault
			printf("%d",1/(rand()&0)); //cause int /0
		}

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
