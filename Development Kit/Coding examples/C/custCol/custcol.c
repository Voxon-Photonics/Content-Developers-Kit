#if 0
custcol.exe: custcol.c voxiebox.h; cl /TP custcol.c /Ox /MT /link user32.lib
	del custcol.obj
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

static voxie_wind_t vw;

static int setmode (int i)
{
	switch(i)
	{
		case '1': vw.usecol = 1; vw.ilacemode = 0; return(1); //Full color (BRbG)
		case '2': if (vw.dispnum != 2) break; vw.usecol = 1; vw.ilacemode = 4; return(1); //G_RB (dispnum 2 only)
		case '3': if (vw.dispnum != 3) break; vw.usecol = 1; vw.ilacemode = 5; return(1); //G_R_B (dispnum 3 only)
		case '4': if (vw.dispnum < 3) { vw.sensemask[0]=0xff0000; vw.outcol[0]=0xff0000; vw.sensemask[1]=0x00ff00; vw.outcol[1]=0x00ff00; } //2col
										 else { vw.sensemask[0]=0xff0000; vw.outcol[0]=0xff0000; vw.sensemask[1]=0x00ff00; vw.outcol[1]=0x00ff00; vw.sensemask[2]=0x0000ff; vw.outcol[2]=0x0000ff; } //default - R/G/B in order
			vw.usecol = 1; vw.ilacemode = 6; return(1);
		case '5': if (vw.dispnum < 3) { vw.sensemask[0]=0xff0000; vw.outcol[0]=0xff0000; vw.sensemask[1]=0x0000ff; vw.outcol[1]=0x0000ff; } //2col
										 else { vw.sensemask[0]=0xff0000; vw.outcol[0]=0xff0000; vw.sensemask[1]=0x0000ff; vw.outcol[1]=0x0000ff; vw.sensemask[2]=0x00ff00; vw.outcol[2]=0x00ff00; } //swap G&B proj. but scene still proper color
			vw.usecol = 1; vw.ilacemode = 6; return(1);
		case '6': if (vw.dispnum < 3) { vw.sensemask[0]=0x00ff00; vw.outcol[0]=0x00ff00; vw.sensemask[1]=0x0000ff; vw.outcol[1]=0x0000ff; } //2col
										 else { vw.sensemask[0]=0x00ff00; vw.outcol[0]=0x00ff00; vw.sensemask[1]=0x0000ff; vw.outcol[1]=0x0000ff; vw.sensemask[2]=0xff0000; vw.outcol[2]=0xff0000; } //cycle proj. colors but scene still proper color
			vw.usecol = 1; vw.ilacemode = 6; return(1);
		case '7': if (vw.dispnum < 3) { vw.sensemask[0]=0x00ff00; vw.outcol[0]=0x00ff00; vw.sensemask[1]=0xff0000; vw.outcol[1]=0xff0000; } //2col
										 else { vw.sensemask[0]=0xff0000; vw.outcol[0]=0x00ffff; vw.sensemask[1]=0x00ff00; vw.outcol[1]=0xff00ff; vw.sensemask[2]=0x0000ff; vw.outcol[2]=0xffff00; } //(cyan/magenta/yellow hack; colors intentionally incorrect)
			vw.usecol = 1; vw.ilacemode = 6; return(1);
		case 'q': vw.usecol = 0; vw.ilacemode = 0; return(1); //Mono White   //these 7 work for any dispnum
		case 'w': vw.usecol =-1; vw.ilacemode = 0; return(1); //Mono Red
		case 'e': vw.usecol =-2; vw.ilacemode = 0; return(1); //Mono Green
		case 'r': vw.usecol =-3; vw.ilacemode = 0; return(1); //Mono Yellow
		case 't': vw.usecol =-4; vw.ilacemode = 0; return(1); //Mono Blue
		case 'y': vw.usecol =-5; vw.ilacemode = 0; return(1); //Mono Magenta
		case 'u': vw.usecol =-6; vw.ilacemode = 0; return(1); //Mono Cyan
	}
	return(0);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	double tim = 0.0, otim, dtim;
	float f;
	int i, j, x, y;
	point3d pp, rr, dd, ff;

	if (voxie_load(&vw) < 0) { MessageBox(0,"Bad","",MB_OK); return(-1); }
	if (cmdline[0]) { setmode(cmdline[0]); }
	if (voxie_init(&vw) < 0) { return(-1); }

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		while (i = voxie_keyread())
		{
			switch(i&255)
			{
				case 27: voxie_quitloop(); break;
				default: if (setmode(i&255)) voxie_init(&vw); break;
			}
		}

		if (vw.useemu)
		{
			i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1);
			if (i)
			{
					  if (voxie_keystat(0xb8)|voxie_keystat(0xdd)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.0); //(Ralt|RMenu)+[,]
				else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = min(max(vw.emudist-(float)i*dtim*2048.0,400.0),4000.0); //Ctrl+[,]
				else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
				voxie_init(&vw);
			}
		}

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

#if 1
		rr.x = 1.5f; dd.x = 0.0f; ff.x = 0.0f; pp.x = 0.f;
		rr.y = 0.0f; dd.y = 1.5f; ff.y = 0.0f; pp.y =-0.25f;
		rr.z = 0.0f; dd.z = 0.0f; ff.z = 1.5f; pp.z = 0.17f;
		voxie_drawspr(&vf,"caco.kv6",&pp,&rr,&dd,&ff,0x808080);

		voxie_drawsph(&vf,-0.5f,+.65f,+0.2f,.12f,1,0xff0000);
		voxie_drawsph(&vf, 0.0f,+.65f,+0.2f,.12f,1,0x00ff00);
		voxie_drawsph(&vf,+0.5f,+.65f,+0.2f,.12f,1,0x0000ff);
		for(i=0;i<64;i++)
		{
			voxie_drawsph(&vf,-0.5f,+.65f,-0.2f,(float)i*.12f/63.f,0,0xff0000);
			voxie_drawsph(&vf, 0.0f,+.65f,-0.2f,(float)i*.12f/63.f,0,0x00ff00);
			voxie_drawsph(&vf,+0.5f,+.65f,-0.2f,(float)i*.12f/63.f,0,0x0000ff);
		}
#else
			//RGB color cube..
		{
		static tiletype src[6] = {0};
		for(i=0;i<6;i++)
		{
			if (src[i].f) continue;
			src[i].x = 64; src[i].y = 64; src[i].p = src[i].x*4; src[i].f = (INT_PTR)malloc(src[i].p*src[i].y*sizeof(int));
			for(y=0;y<64;y++)
				for(x=0;x<64;x++)
				{
					if (i == 0) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+0)) + (x<<(2+ 8));
					if (i == 1) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+0)) + (x<<(2+16));
					if (i == 2) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+8)) + (x<<(2+16));
					if (i == 3) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+0)) + (x<<(2+ 8)) + 0xff0000;
					if (i == 4) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+0)) + (x<<(2+16)) + 0x00ff00;
					if (i == 5) *(int *)(src[i].f + src[i].p*y + x*4) = (y<<(2+8)) + (x<<(2+16)) + 0x0000ff;
				}
		}
		static const int corns[6][4][3] =
		{
			-1,-1,-1, -1,+1,-1, -1,+1,+1, -1,-1,+1,
			-1,-1,-1, +1,-1,-1, +1,-1,+1, -1,-1,+1,
			-1,-1,-1, +1,-1,-1, +1,+1,-1, -1,+1,-1,
			+1,-1,-1, +1,+1,-1, +1,+1,+1, +1,-1,+1,
			-1,+1,-1, +1,+1,-1, +1,+1,+1, -1,+1,+1,
			-1,-1,+1, +1,-1,+1, +1,+1,+1, -1,+1,+1,
		};
		static const int mesh[5] = {0,1,2,3,-1};
		poltex_t vt[4];
		for(i=0;i<4;i++) { vt[i].u = (float)((i == 1) || (i == 2)); vt[i].v = (float)(i >= 2); vt[i].col = 0x404040; }
		f = min(min(vw.aspx,vw.aspy),vw.aspz)*.99f;
		for(i=0;i<6;i++)
		{
			for(j=0;j<4;j++)
			{
				vt[j].x = (float)corns[i][j][0]*f;
				vt[j].y = (float)corns[i][j][1]*f;
				vt[j].z = (float)corns[i][j][2]*f;
			}
			voxie_drawmeshtex(&vf,(char *)&src[i],vt,4,(int *)mesh,5,2+8,0x404040);
		}
		}
#endif
voxie_debug_print6x8_(32,65,0x00ff00,-1,"custCol press 1 - 7 and Q,W,E,R,T,Y to change color modes");

		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
