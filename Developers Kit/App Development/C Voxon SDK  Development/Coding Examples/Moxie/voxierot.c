#if 0

voxierot.exe: voxierot.obj voxiebox.h
	link       voxierot.obj user32.lib
	del voxierot.obj
voxierot.obj: voxierot.c; cl /c /TP voxierot.c /Ox /MT

#voxierot: voxierot.c voxiebox.h; gcc voxierot.c -x c++ -o voxierot -O2 -L'${PWD}' -Wl,-R. -lvoxon -lm

!ifdef false
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#if !defined(_WIN32)
#define _alloca alloca
#define MessageBox(a,b,c,d) printf("%s %s\n",c,b);
#define Sleep(a) usleep(a*1000)
#endif
#include "moxie.h"
#define PI 3.14159265358979323

#if !defined(max)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

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

static void drawlotsajunk (voxie_wind_t *vw, voxie_frame_t *vf, double tim)
{
	int i, j;

	for(i=-1;i<=1;i+=2)
	{
		point3d p = {(float)i*0.5,0.0,0.0}, r = {1.f,0.f,0.f}, d = {0.f,1.f,0.f}, f = {0.f,0.f,1.f}; moxie_drawspr(vf,"caco.kv6",&p,&r,&d,&f,0x404040);
	}

	moxie_drawsph(vf,0.f,-.5f,0.f,0.1f,1,0x808080);
	moxie_drawcone(vf,.0f,-.5f,+.0f, 0.08f, +.4f,-.4f-.5f,+.4f, 0.f,1,0x404040);
	moxie_drawlin(vf, .0f,-.5f,+.0f,        -.4f,-.4f-.5f,-.4f,       0xffff00);

	{
	poltex_t vt[4]; int mesh[256]; i = 0;
	vt[0].x =-0.4; vt[0].y =-0.4-.5f; vt[0].z =-0.4; vt[0].col = 0xffffff;
	vt[1].x =-0.4; vt[1].y =+0.4-.5f; vt[1].z =+0.4; vt[1].col = 0xffffff;
	vt[2].x =+0.4; vt[2].y =-0.4-.5f; vt[2].z =+0.4; vt[2].col = 0xffffff;
	vt[3].x =+0.4; vt[3].y =+0.4-.5f; vt[3].z =-0.4; vt[3].col = 0xffffff;
	mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = -1; mesh[i++] = 1; mesh[i++] = 3;
	moxie_drawmeshtex(vf,0,vt,4,mesh,i,1,0xffffff);
	}

	for(i=1;i<=7;i++)
	{
		float fx, fy, fz;
		int col;
		fx = ((float)i-4.f)*.25f; fy = .7f; fz = sin(tim+(float)i)*(vw->aspz-.1f);
		col = ((i&1) + (((i&2)>>1)<<8) + (((i&4)>>2)<<16))*255;
		moxie_drawbox(vf,fx-.1,fy-.1,fz-.1,
							  fx+.1,fy+.1,fz+.1,1,col);
		if (i != 3) continue;
		for(j=1024;j>0;j--)
		{
			moxie_drawvox(vf,fx+(float)((rand()&32767)-16384)/16383.f*.1f,
								  fy+(float)((rand()&32767)-16384)/16383.f*.1f,
								  fz+(float)((rand()&32767)-16384)/16383.f*.1f,
								  col);
		}
	}

	{
	point3d p, r, d, f;
	r.x = cos(tim)* .5f; d.x = sin(tim)*.5f; f.x = 0.f; p.x = .7f - (r.x + d.x + f.x)*.5f;
	r.y = sin(tim)*-.5f; d.y = cos(tim)*.5f; f.y = 0.f; p.y =-.6f - (r.y + d.y + f.y)*.5f;
	r.z =           0.f; d.z =          0.f; f.z = .5f; p.z =+.0f - (r.z + d.z + f.z)*.5f;
	moxie_drawcube(vf,&p,&r,&d,&f,1,0x808080);
	}

	{
	point3d p, r, d, f;
	r.x = cos(tim)* .05f; d.x = sin(tim)*.15f; p.x = .7f - r.x*4.f - d.x*.5f;
	r.y = sin(tim)*-.05f; d.y = cos(tim)*.15f; p.y =-.6f - r.y*4.f - d.y*.5f;
	r.z =           0.f; d.z =          0.f; p.z =+.0f - (r.z + d.z)*.5f;
	moxie_printalph_(vf,&p,&r,&d,0xc0c0c0,"hello %i",rand()&63);
	}

	if ((fabs(gcamr.z) == 0.f) && (fabs(gcamd.z) == 0.f)) //NOTE: heightmaps don't currently support rotation off the x-y plane ;P
	{
		point3d p, r, d, f;
		r.x =  .5f; d.x = 0.f; f.x = 0.f; p.x =-.7f - (r.x + d.x + f.x)*.5f;
		r.y =  0.f; d.y = .5f; f.y = 0.f; p.y =-.6f - (r.y + d.y + f.y)*.5f;
		r.z =  0.f; d.z = 0.f; f.z =-.1f; p.z =+.0f - (r.z + d.z + f.z)*.5f;
		moxie_drawheimap(vf,"canyon.png",&p,&r,&d,&f,-1,0,(1<<1)+(1<<2));
	}
}

#if defined(_WIN32)
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
#else
int main (int argc, char **argv)
#endif
{
	voxie_wind_t vw;
	voxie_frame_t vf;
	voxie_inputs_t in;
	voxie_nav_t nav;
	point3d camp = {0.0,0.0,0.0}, camr = {1.f,0.f,0.f}, camd = {0.f,1.f,0.f}, camf = {0.f,0.f,1.f}, piv = {0.f,0.f,0.f};
	double tim = 0.0, otim, dtim;
	float f, fx, fy, fz;
	int i, j, navdir = 1, setpiv = 0;

	if (voxie_load(&vw) < 0) { MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) { return(-1); }

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		voxie_nav_read(0,&nav);

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
				  if (voxie_keystat(0xb8)|voxie_keystat(0xdd)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.0); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = min(max(vw.emudist-(float)i*dtim*2048.0,400.0),4000.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
		if (!vw.clipshape) voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		else
		{
			int n = 64;
			for(j=-1;j<=1;j+=2)
				for(i=0;i<n;i++)
				{
					voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz,
											cos((float)(i+1)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+1)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz, 0xffffff);
				}
		}

			//Space: toggle volume type
		if (voxie_keystat(0x39) == 1) { vw.clipshape = !vw.clipshape; voxie_init(&vw); }

		//--------------------------------------------------------------------------------------------
		//3D Camera control..

		if (voxie_keystat(0x21) == 1) navdir = -navdir; //F:flip
		if (voxie_keystat(0x1c) == 1) setpiv = !setpiv; //L.Enter: set pivot mode

		if (!setpiv) //Move/rotate/zoom around pivot
		{
				//Move cam (Arrows,KP0,RCtrl,spacenav)
			fx = (nav.dx*+.00005f + (float)((voxie_keystat(0xcd)!=0) - (voxie_keystat(0xcb)!=0))*dtim*-.5f)*navdir + piv.x; //Right-Left
			fy = (nav.dy*+.00005f + (float)((voxie_keystat(0xd0)!=0) - (voxie_keystat(0xc8)!=0))*dtim*-.5f)*navdir + piv.y; //Down-Up
			fz = (nav.dz*+.00005f + (float)((voxie_keystat(0x52)!=0) - (voxie_keystat(0x9d)!=0))*dtim*-.5f)*navdir + piv.z; //KP0-RCTRL
			f = 1.f/(camr.x*camr.x + camr.y*camr.y + camr.z*camr.z);
			camp.x += (fx*camr.x + fy*camr.y + fz*camr.z)*f;
			camp.y += (fx*camd.x + fy*camd.y + fz*camd.z)*f;
			camp.z += (fx*camf.x + fy*camf.y + fz*camf.z)*f;

				//Rotate cam (,., PGUP,PGDN,spacenav)
			f = camr.y; camr.y = camd.x; camd.x = f; //Transpose
			f = camr.z; camr.z = camf.x; camf.x = f;
			f = camd.z; camd.z = camf.y; camf.y = f;
			rotvex((nav.ax*+.0001f                                                                  )*navdir,&camr,&camf);
			rotvex((nav.ay*+.0001f - (float)((voxie_keystat(0xc9)!=0)-(voxie_keystat(0xd1)!=0))*dtim)*navdir,&camd,&camf); //PGUP - PGDN
			rotvex((nav.az*+.0001f - (float)((voxie_keystat(0x34)!=0)-(voxie_keystat(0x33)!=0))*dtim)*navdir,&camr,&camd); //. - ,
			f = camr.y; camr.y = camd.x; camd.x = f; //Transpose
			f = camr.z; camr.z = camf.x; camf.x = f;
			f = camd.z; camd.z = camf.y; camf.y = f;

				//Scale cam (A/Z,spacenav buttons)
			if ((nav.but&1) || (voxie_keystat(0x1e))) { f = pow(2.0,dtim); camr.x *= f; camr.y *= f; camr.z *= f; camd.x *= f; camd.y *= f; camd.z *= f; camf.x *= f; camf.y *= f; camf.z *= f; }
			if ((nav.but&2) || (voxie_keystat(0x2c))) { f = pow(0.5,dtim); camr.x *= f; camr.y *= f; camr.z *= f; camd.x *= f; camd.y *= f; camd.z *= f; camf.x *= f; camf.y *= f; camf.z *= f; }

			f = 1.f/(camr.x*camr.x + camr.y*camr.y + camr.z*camr.z);
			camp.x -= (piv.x*camr.x + piv.y*camr.y + piv.z*camr.z)*f;
			camp.y -= (piv.x*camd.x + piv.y*camd.y + piv.z*camd.z)*f;
			camp.z -= (piv.x*camf.x + piv.y*camf.y + piv.z*camf.z)*f;
		}
		else //move pivot point (L.Enter to toggle)
		{
			fx = (nav.dx*+.00005f + (float)((voxie_keystat(0xcd)!=0) - (voxie_keystat(0xcb)!=0))*dtim*-.5f); //Right-Left
			fy = (nav.dy*+.00005f + (float)((voxie_keystat(0xd0)!=0) - (voxie_keystat(0xc8)!=0))*dtim*-.5f); //Down-Up
			fz = (nav.dz*+.00005f + (float)((voxie_keystat(0x52)!=0) - (voxie_keystat(0x9d)!=0))*dtim*-.5f); //KP0-RCTRL
			piv.x = min(max(piv.x+fx,-vw.aspx),vw.aspx);
			piv.y = min(max(piv.y+fy,-vw.aspy),vw.aspy);
			piv.z = min(max(piv.z+fz,-vw.aspz),vw.aspz);
		}

		//if (voxie_keystat(0x2a) == 1)
		//{
		//   camp.x += (piv.x*camr.x + piv.y*camr.y + piv.z*camr.z);
		//   camp.y += (piv.x*camd.x + piv.y*camd.y + piv.z*camd.z);
		//   camp.z += (piv.x*camf.x + piv.y*camf.y + piv.z*camf.z);
		//   piv.x = 0; piv.y = 0; piv.z = 0;
		//}

			//Reset cam: (/)
		if (voxie_keystat(0x35) == 1)
		{
			camp.x = 0.f; camp.y = 0.f; camp.z = 0.f;
			camr.x = 1.f; camr.y = 0.f; camr.z = 0.f;
			camd.x = 0.f; camd.y = 1.f; camd.z = 0.f;
			camf.x = 0.f; camf.y = 0.f; camf.z = 1.f;
		}

		//--------------------------------------------------------------------------------------------
		//Render..

		moxie_setcam(&camp,&camr,&camd,&camf);
		drawlotsajunk(&vw,&vf,tim);

		//--------------------------------------------------------------------------------------------

			//Show pivot point
		voxie_drawsph(&vf,piv.x,piv.y,piv.z,0.05f,1,0xffffff);

			//For debug only
		voxie_debug_print6x8_(0,64,0xffffff,-1,"p: %+6.3f %+6.3f %+6.3f",camp.x,camp.y,camp.z);
		voxie_debug_print6x8_(0,72,0xffffff,-1,"r: %+6.3f %+6.3f %+6.3f",camr.x,camr.y,camr.z);
		voxie_debug_print6x8_(0,80,0xffffff,-1,"d: %+6.3f %+6.3f %+6.3f",camd.x,camd.y,camd.z);
		voxie_debug_print6x8_(0,88,0xffffff,-1,"f: %+6.3f %+6.3f %+6.3f",camf.x,camf.y,camf.z);

		voxie_frame_end(); voxie_getvw(&vw); Sleep(5);
	}

	voxie_uninit(0);
	return(0);
}

#if 0
!endif
#endif
