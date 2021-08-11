// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
voxiedraw.exe: voxiedraw.obj voxiebox.h
	link        voxiedraw.obj user32.lib
	del voxiedraw.obj
voxiedraw.obj: voxiedraw.c; cl /c /TP voxiedraw.c /Ox /MT
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

	//Working grid size..
#define XSIZ 256
#define YSIZ 256
#define ZSIZ 128 //<- wastes memory if this not multiple of 32 ;)
static int vox[XSIZ][YSIZ][(ZSIZ+31)>>5]; //1 bit per voxel ;)

#if 1

	//Simple version - 1 drawvox per set bit.
static void drawgrid (voxie_frame_t *vf)
{
	int x, y, z;

		//using voxie_drawvox() is fine due to the recent auto-batching optimization
	for(x=0;x<XSIZ;x++)
		for(y=0;y<YSIZ;y++)
			for(z=0;z<ZSIZ;z++)
			{
				if (!vox[x][y][z>>5]) { z |= 31; continue; } //early-out optimization: skip up to 31 bits if rest of word is clear
				if (!(vox[x][y][z>>5]&(1<<(z&31)))) continue; //skip bit x/y/z if clear
				voxie_drawvox(vf,(float)(x-(XSIZ>>1))*(2.f/(float)XSIZ),
									  (float)(y-(YSIZ>>1))*(2.f/(float)XSIZ),
									  (float)(z-(ZSIZ>>1))*(2.f/(float)XSIZ),0xffffff); //.01f,1,0xffffff);
			}
}

#else

	//Need more speed? voxie_drawmeshtex() is multithread safe .. obviously the multithread part is not implemented here.. ;P
static void drawgrid (voxie_frame_t *vf)
{
	static poltex_t vt[YSIZ*ZSIZ];
	int x, y, z, nv;

	for(x=0;x<XSIZ;x++)
	{
		nvt = 0;
		for(y=0;y<YSIZ;y++)
		{
			for(z=0;z<ZSIZ;z++)
			{
				if (!vox[x][y][z>>5]) { z |= 31; continue; } //early-out optimization: skip up to 31 bits if rest of word is clear
				if (!(vox[x][y][z>>5]&(1<<(z&31)))) continue; //skip bit x/y/z if clear
				vt[nvt].x = (float)(x-(XSIZ>>1))*(2.f/(float)XSIZ);
				vt[nvt].y = (float)(y-(YSIZ>>1))*(2.f/(float)XSIZ);
				vt[nvt].z = (float)(z-(ZSIZ>>1))*(2.f/(float)XSIZ);
				vt[nvt].col = 0xffffff;
				nvt++;
			}
		}
		if (nvt) voxie_drawmeshtex(vf,0,vt,nvt,0,0,0,0xffffff);
	}
}

#endif

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_wind_t vw;
	voxie_frame_t vf;
	voxie_inputs_t in;
	voxie_nav_t nav;

	static pol_t pt[4096];
	static int ptislin[4096];
	static int ptn = 0, ptstarti = 0;

	point3d tp, curs = {0.0,0.0,0.0}; //cursor ranges from -vw.asp? .. +vw.asp?
	double tim = 0.0, otim, dtim;
	int i, j, k, irad = 1, x, y, z, xx, yy, zz, mousx = 0, mousy = 0, mousz = 0;

	if (voxie_load(&vw) < 0) { MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	vw.usecol = 0;
	if (voxie_init(&vw) < 0) { return(-1); }

	memset(vox,0,sizeof(vox));

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;

		voxie_nav_read(0,&nav);

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1); //Handle rot&zoom ([/]) for emu
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

			//Cursor movement..
		curs.x += nav.dx*.000050f;
		curs.y += nav.dy*.000050f;
		curs.z += nav.dz*.000025f;
		curs.x += in.dmousx*.01f;
		if (!(in.bstat&2)) curs.y += in.dmousy*.01f;
						  else curs.z += in.dmousy*.01f;
		curs.x = min(max(curs.x,    -vw.aspx),+vw.aspx     );
		curs.y = min(max(curs.y,    -vw.aspy),+vw.aspy     );
		curs.z = min(max(curs.z,.05f-vw.aspz),+vw.aspz-.05f);

			//Quantize cursor to grid (every 0.1 in VX1 coords)
		tp.x = floor(curs.x*10.f+0.5f)/10.f;
		tp.y = floor(curs.y*10.f+0.5f)/10.f;
		tp.z = floor(curs.z*10.f+0.5f)/10.f;
		if (voxie_keystat(0xcb) == 1) { if (fabs(curs.x-tp.x) > .01f) curs.x = tp.x; else curs.x -= 0.1f; } //Left
		if (voxie_keystat(0xcd) == 1) { if (fabs(curs.x-tp.x) > .01f) curs.x = tp.x; else curs.x += 0.1f; } //Right
		if (voxie_keystat(0xc8) == 1) { if (fabs(curs.y-tp.y) > .01f) curs.y = tp.y; else curs.y -= 0.1f; } //Up
		if (voxie_keystat(0xd0) == 1) { if (fabs(curs.y-tp.y) > .01f) curs.y = tp.y; else curs.y += 0.1f; } //Down
		if (voxie_keystat(0x9d) == 1) { if (fabs(curs.z-tp.z) > .01f) curs.z = tp.z; else curs.z -= 0.1f; } //RCtrl
		if (voxie_keystat(0x52) == 1) { if (fabs(curs.z-tp.z) > .01f) curs.z = tp.z; else curs.z += 0.1f; } //KP0
		if (voxie_keystat(0xc9) == 1) { if (fabs(curs.z-tp.z) > .01f) curs.z = tp.z; else curs.z -= 0.1f; } //PGUP
		if (voxie_keystat(0xd1) == 1) { if (fabs(curs.z-tp.z) > .01f) curs.z = tp.z; else curs.z += 0.1f; } //PDGN

#if 0
			//A very simple & crappy attempt at a 3D polygon editor.. obviously unfinished.
			//Draw polygons by pressing space bar at different points (similar to the Build Editor)
		if (voxie_keystat(0x39) == 1) { pt[ptn].x = tp.x; pt[ptn].y = tp.y; pt[ptn].z = tp.z; pt[ptn].p2 = ptstarti; if (ptn > ptstarti) pt[ptn-1].p2 = ptn; ptn++; } //Space (plot next point of polygon)
		if (voxie_keystat(0x1c) == 1) { ptstarti = ptn; } //L.Enter (finish polygon)
		if ((voxie_keystat(0x0e) == 1) && (ptn > 0)) { ptn--; ptstarti = pt[ptn].p2; pt[ptn-1].p2 = ptstarti; } //Backspace (undo last point drawn)

			//Draw polygons..
		for(i=0;i<ptn;i++)
		{
			voxie_drawsph(&vf,pt[i].x,pt[i].y,pt[i].z,0.02f,1,0xffffff);
			j = pt[i].p2; voxie_drawlin(&vf,pt[i].x,pt[i].y,pt[i].z,pt[j].x,pt[j].y,pt[j].z,0xffffff);
			if (j < i)
			{
				for(k=j;k<=i;k++) pt[k].p2 -= j;
				voxie_drawpol(&vf,&pt[j],i+1-j,0xffffff);
				for(k=j;k<=i;k++) pt[k].p2 += j;
			}
		}

		//if (ptn > ptstarti)
		//{
		//   voxie_drawlin(&vf,pt[ptstarti].x,pt[ptstarti].y,pt[ptstarti].z,tp.x,tp.y,tp.z,0xffffff);
		//   voxie_drawlin(&vf,pt[ptn-1   ].x,pt[ptn-1   ].y,pt[ptn-1   ].z,tp.x,tp.y,tp.z,0xffffff);
		//}

		if (!(in.bstat&2)) { for(y=-10;y<=10;y++) for(x=-10;x<10;x++) voxie_drawvox(&vf,x*.1f,y*.1f,tp.z,0xffffff); }
						  else { for(z=-10;z<=10;z++) for(x=-10;x<10;x++) voxie_drawvox(&vf,x*.1f,tp.y,z*.1f,0xffffff); }
		voxie_drawsph(&vf,curs.x,curs.y,curs.z,0.05f+sin(tim*16.f)*.01f,0,0xffffff);
		voxie_drawsph(&vf,tp.x,tp.y,tp.z,0.02f,1,0xffffff);
#else
			//Simple 3D plotter controls..
		if (voxie_keystat(0x0e) == 1) { memset(vox,0,sizeof(vox)); } //Backspace:clear grid
		if (voxie_keystat(0x33) == 1) { irad = max(irad-1,0); } //,:dec brush rad
		if (voxie_keystat(0x34) == 1) { irad++;               } //.:inc brush rad
		if (voxie_keystat(0xd2)|voxie_keystat(0xd3)|in.bstat|nav.but) //Ins:paint, Del:erase
		{
			xx = (int)(curs.x*XSIZ/2.f)+(XSIZ>>1);
			yy = (int)(curs.y*XSIZ/2.f)+(YSIZ>>1);
			zz = (int)(curs.z*XSIZ/2.f)+(ZSIZ>>1);
			i = ((voxie_keystat(0xd2) != 0) || (in.bstat&1) | (nav.but&1)); //i = is paint?

				//paint/erase sphere in voxel grid (brute force for simplicity; could be much faster for large brushes ;)
			for(x=-irad;x<=irad;x++)
				for(y=-irad;y<=irad;y++)
					for(z=-irad;z<=irad;z++)
					{
						if ((unsigned)(xx+x) >= XSIZ) continue;
						if ((unsigned)(yy+y) >= YSIZ) continue;
						if ((unsigned)(zz+z) >= ZSIZ) continue;
						if (x*x + y*y + z*z > irad*irad) continue;
						if (i) vox[xx+x][yy+y][(zz+z)>>5] |= (1<<((zz+z)&31)); //paint bits
						  else vox[xx+x][yy+y][(zz+z)>>5] &=~(1<<((zz+z)&31)); //erase bits
					}
		}

#if 0
			//Slow falling effect (x random voxels drop by 1 per frame rendered)
		for(i=(1<<18);i>0;i--)
		{
			x = ((rand()&32767)*(XSIZ-1))>>15;
			y = ((rand()&32767)*(YSIZ-1))>>15;
			z = ((rand()&32767)*(ZSIZ-1))>>15; if (z == ZSIZ-1) continue;
			if (( (vox[x][y][(z  )>>5]&(1<<((z  )&31)))) &&
				 (!(vox[x][y][(z+1)>>5]&(1<<((z+1)&31)))))
			{
				vox[x][y][(z  )>>5] &=~(1<<( z   &31));
				vox[x][y][(z+1)>>5] |= (1<<((z+1)&31));
			}
		}
#elif 0
			//Fast falling effect (every voxel drops by 1; limited to 20x/sec)
		if ((int)(tim*20.0) > (int)(otim*20.0))
			for(x=0;x<XSIZ;x++)
				for(y=0;y<YSIZ;y++)
				{
					for(z=((ZSIZ+31)>>5)-1;z>0;z--) vox[x][y][z] = (vox[x][y][z]<<1)+(((unsigned)vox[x][y][z-1])>>31);
					vox[x][y][z] <<= 1;
				}
#endif

		drawgrid(&vf);

		voxie_drawsph(&vf,curs.x,curs.y,curs.z,(float)irad*2.f/(float)XSIZ,1,0xffffff); //show cursor
#endif

		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0);
	return(0);
}

#if 0
!endif
#endif
