// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
voxie2d.exe: voxie2d.c voxiebox.h; cl /TP voxie2d.c /Ox /MT /link user32.lib
	del voxie2d.obj
!ifdef false
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	voxie_wind_t vw;
	double tim = 0.0, otim, dtim;
	point3d ball[100], balv[100];
	int balc[100];
	float balr[100];
	int i, j;

	if (voxie_load(&vw) < 0) { return(-1); }
	if (voxie_init(&vw) < 0) { return(-1); }

	for(i=0;i<100;i++)
	{
		ball[i].x = (float)((rand()&32767)-16384)/16384.0;
		ball[i].y = (float)((rand()&32767)-16384)/16384.0;
		ball[i].z = (float)((rand()&32767)-16384)/65536.0;
		balv[i].x = (float)((rand()&32767)-16384)/16384.0;
		balv[i].y = (float)((rand()&32767)-16384)/16384.0;
		balv[i].z = (float)((rand()&32767)-16384)/65536.0;
		balr[i] = (float)(rand()&32767)/262144.0;
		balc[i] = (((rand()&32767) + ((rand()&32767)<<15))&0x010101)*0xff;
	}

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		if (voxie_keystat(0x1)) { voxie_quitloop(); } //ESC: quit

		if (voxie_keystat(0x3) == 1) //'2': set 2D mode
		{
			if (vw.voxie_vol >= 0)
			{
				vw.voxie_vol = -vw.voxie_vol; //negative vol will behave as 0; old value is remembered upon restore
				vw.framepervol = 2; //hack to reduce flicker in 2D mode
				voxie_init(&vw);
			}

		}
		if (voxie_keystat(0x4) == 1) //'3': set 3D mode
		{
			if (vw.voxie_vol < 0)
			{
				vw.voxie_vol = -vw.voxie_vol;
				vw.framepervol = 8; //restore (assumes VX1 15Hz)
				voxie_init(&vw);
			}
		}

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

		if (vw.voxie_vol < 0) //2D mode..
		{
			static tiletype img = {0};
			float fx, fy;
			int x, y, t;

			if (!img.f) { img.x = 512; img.y = 512; img.p = (INT_PTR)img.x*4; img.f = (INT_PTR)malloc(img.p*img.y); }

				//Clear bitmap with colorful scrolling background pattern
			t = int(tim*64);
			for(y=0;y<img.y;y++)
				for(x=0;x<img.x;x++)
				{
					*(int *)(img.p*y + (x<<2) + img.f) = (128<<16) + ((y+t)<<8) + (x-t);
				}

				//Draws 100 circles to bitmap
			for(i=0;i<100;i++)
			{
				float f, bx, by, br, br2;
				int x0, y0, x1, y1;
				bx = ball[i].x*(img.x/2) + img.x/2;
				by = ball[i].y*(img.y/2) + img.y/2;
				br = balr[i]*(img.x/2); br2 = br*br;
				y0 = max(by-br,0); y1 = min(by+br,img.y);
				for(y=y0;y<y1;y++)
				{
					f = br2 - (y-by)*(y-by); if (f <= 0.f) continue;
					f = sqrt(f);
					x0 = max((int)(bx-f),    0);
					x1 = min((int)(bx+f),img.x);
					for(x=x0;x<x1;x++) { *(int *)(img.p*y + (x<<2) + img.f) |= balc[i]; }
				}
			}

				//Draw 2D bitmap..
			point3d pp, rr, dd, ff;
			rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x = 0.f; pp.x = rr.x*-.5f;
			rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y = 0.f; pp.y = dd.y*-.5f;
			rr.z =         0.f; dd.z =         0.f; ff.z = 0.f; pp.z = 0.f;
			voxie_drawheimap(&vf,(char *)&img, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);
		}
		else //3D mode..
		{
				//Draw wireframe box around volume
			voxie_drawbox(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz,1,0xffffff);

				//Draw balls in 3D
			for(i=0;i<100;i++)
			{
				voxie_drawsph(&vf, ball[i].x,ball[i].y,ball[i].z,balr[i],0,balc[i]);
			}
		}

		while (i = voxie_keyread())
		{
			if ((((i>>8)&255) == 0x1e) && (i&0xc0000)) { MessageBox(0,"hi","bleh",MB_OK); } //Ctrl+A
		}
		for(i=0,j=0;i<256;i++) { if (voxie_keystat(i)) { voxie_debug_print6x8_(j*16,64,0xffffff,0x000000,"%02x",i); j++; } }

		voxie_frame_end(); voxie_getvw(&vw);

			//Animate balls
		for(i=0;i<100;i++)
		{
			ball[i].x += balv[i].x*dtim;
			ball[i].y += balv[i].y*dtim;
			ball[i].z += balv[i].z*dtim;

			if (fabs(ball[i].x) > vw.aspx) balv[i].x *= -1;
			if (fabs(ball[i].y) > vw.aspy) balv[i].y *= -1;
			if (fabs(ball[i].z) > vw.aspz) balv[i].z *= -1;
		}
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
