#if 0
voxietst.exe: voxietst.obj voxiebox.h
	link       voxietst.obj user32.lib
	del voxietst.obj
voxietst.obj: voxietst.c; cl /c /TP voxietst.c /Ox /MT
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_wind_t vw;
	voxie_frame_t vf;
	voxie_inputs_t in;
	voxie_nav_t nav;
	point3d curs = {0.0,0.0,0.0};
	double tim = 0.0, otim, dtim;
	int i;

	if (voxie_load(&vw) < 0) { MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	//vw.usecol = 0; vw.hacks &= ~1; /*not exclusive mouse*/
	if (voxie_init(&vw) < 0) { return(-1); }

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		voxie_nav_read(0,&nav);

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
				  if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,2000.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

		curs.x += nav.dx*.000050f;
		curs.y += nav.dy*.000050f;
		curs.z += nav.dz*.000025f;

		curs.x += in.dmousx*.01f;
		if (!(in.bstat&2)) curs.y += in.dmousy*.01f;
						  else curs.z += in.dmousy*.01f;

		curs.x = min(max(curs.x,    -vw.aspx),+vw.aspx     );
		curs.y = min(max(curs.y,    -vw.aspy),+vw.aspy     );
		curs.z = min(max(curs.z,.05f-vw.aspz),+vw.aspz-.05f);

		if (voxie_keystat(0xcb) == 1) { curs.x -= 0.1f; }
		if (voxie_keystat(0xcd) == 1) { curs.x += 0.1f; }
		if (voxie_keystat(0xc8) == 1) { curs.y -= 0.1f; }
		if (voxie_keystat(0xd0) == 1) { curs.y += 0.1f; }
		if (voxie_keystat(0x9d) == 1) { curs.z -= 0.1f; }
		if (voxie_keystat(0x52) == 1) { curs.z += 0.1f; }
		if (voxie_keystat(0xc9) == 1) { curs.z -= 0.1f; }
		if (voxie_keystat(0xd1) == 1) { curs.z += 0.1f; }

		//voxie_drawsph(&vf,curs.x,curs.y,curs.z,0.2f,1,7);
		//voxie_drawcone(&vf,curs.x-0.15f,curs.y+0.2f,curs.z-0.05f,0.1f,
		//                   curs.x+0.15f,curs.y-0.2f,curs.z+0.05f,0.2f,1,0x404040);

#if 0
		{
		poltex_t vt[4]; i = 0;
		vt[0].x =-0.4; vt[0].y =-0.4; vt[0].z =-0.4; vt[0].col = 0xffffff;
		vt[1].x =-0.4; vt[1].y =+0.4; vt[1].z =+0.4; vt[1].col = 0xffffff;
		vt[2].x =+0.4; vt[2].y =-0.4; vt[2].z =+0.4; vt[2].col = 0xffffff;
		vt[3].x =+0.4; vt[3].y =+0.4; vt[3].z =-0.4; vt[3].col = 0xffffff;
		voxie_drawmeshtex(&vf,0,vt,4,0,0,0,0xffffff);
		}
#endif
#if 0
		{
		poltex_t vt[4]; int mesh[256]; i = 0;
		vt[0].x =-0.4; vt[0].y =-0.4; vt[0].z =-0.4; vt[0].col = 0xffffff;
		vt[1].x =-0.4; vt[1].y =+0.4; vt[1].z =+0.4; vt[1].col = 0xffffff;
		vt[2].x =+0.4; vt[2].y =-0.4; vt[2].z =+0.4; vt[2].col = 0xffffff;
		vt[3].x =+0.4; vt[3].y =+0.4; vt[3].z =-0.4; vt[3].col = 0xffffff;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = -1; mesh[i++] = 1; mesh[i++] = 3;
		voxie_drawmeshtex(&vf,0,vt,4,mesh,i,1,0xffffff);
		}
#endif
#if 0
		{
		double d, c, s;
		float xm, ym, zm, xa, ya, za;
		int j, x, y, z, xn, yn, zn;
		poltex_t vt[4], vt2[4]; int mesh[256];
		c = cos(tim); s = sin(tim);

		vt[0].x =-1; vt[0].y =-1; vt[0].z =-1; vt[0].col = 0xffffff;
		vt[1].x =-1; vt[1].y =+1; vt[1].z =+1; vt[1].col = 0xffffff;
		vt[2].x =+1; vt[2].y =-1; vt[2].z =+1; vt[2].col = 0xffffff;
		vt[3].x =+1; vt[3].y =+1; vt[3].z =-1; vt[3].col = 0xffffff;
		for(i=4-1;i>=0;i--)
		{
			d = vt[i].x; vt[i].x = vt[i].x*c - vt[i].y*s; vt[i].y = vt[i].y*c + d*s;
			d = vt[i].x; vt[i].x = vt[i].x*c - vt[i].z*s; vt[i].z = vt[i].z*c + d*s;
			d = vt[i].y; vt[i].y = vt[i].y*c - vt[i].z*s; vt[i].z = vt[i].z*c + d*s;
			d = .04; vt[i].x *= d; vt[i].y *= d; vt[i].z *= d;
		}
		i = 0;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1;
		mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;

		xn = max((int)(vw.aspx/vw.aspx*16.f),1); d = 1.f/(float)xn; xm = vw.aspx*d*2.f; xa = (float)(1-xn)*vw.aspx*d;
		yn = max((int)(vw.aspy/vw.aspx*16.f),1); d = 1.f/(float)yn; ym = vw.aspy*d*2.f; ya = (float)(1-yn)*vw.aspy*d;
		zn = max((int)(vw.aspz/vw.aspx*16.f),1); d = 1.f/(float)zn; zm = vw.aspz*d*2.f; za = (float)(1-zn)*vw.aspz*d;
		for(z=zn-1;z>=0;z--)
			for(y=yn-1;y>=0;y--)
				for(x=xn-1;x>=0;x--)
				{
					for(j=4-1;j>=0;j--)
					{
						vt2[j].x = vt[j].x + (float)x*xm + xa;
						vt2[j].y = vt[j].y + (float)y*ym + ya;
						vt2[j].z = vt[j].z + (float)z*zm + za;
					}
					voxie_drawmeshtex(&vf,0,vt2,4,mesh,i,2,0xffffff);
				}
		}
#endif
#if 0
		{
		poltex_t vt[4]; int mesh[256]; i = 0;
		vt[0].x =-0.4; vt[0].y =-0.4; vt[0].z =-0.4; vt[0].col = 0xffffff;
		vt[1].x =-0.4; vt[1].y =+0.4; vt[1].z =+0.4; vt[1].col = 0xffffff;
		vt[2].x =+0.4; vt[2].y =-0.4; vt[2].z =+0.4; vt[2].col = 0xffffff;
		vt[3].x =+0.4; vt[3].y =+0.4; vt[3].z =-0.4; vt[3].col = 0xffffff;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1;
		mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;
		voxie_drawmeshtex(&vf,0,vt,4,mesh,i,3,0xffffff);
		}
#endif
#if 1
		for(i=1;i<=7;i++)
		{
			float fx, fy, fz;
			int col;
			fx = ((float)i-4.f)*.25f; fy = 0.f; fz = sin(tim+(float)i)*(vw.aspz-.1f);
			col = ((i&1) + (((i&2)>>1)<<8) + (((i&4)>>2)<<16))*255;
			voxie_drawsph(&vf,fx   ,fy-.25   ,fz   ,0.1f,1,col);
			voxie_drawbox(&vf,fx-.1,fy    -.1,fz-.1,
									fx+.1,fy    +.1,fz+.1,1,col);
			voxie_drawbox(&vf,fx-.1,fy+.25-.1,fz-.1,
									fx+.1,fy+.25+.1,fz+.1,2,col);
		}
#endif

		voxie_frame_end(); voxie_getvw(&vw); Sleep(5);
	}

	voxie_uninit(0);
	return(0);
}

#if 0
!endif
#endif
