// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0

voxieatom.exe: voxieatom.c voxiebox.h; cl /TP voxieatom.c /Ox /MT /link
	del voxieatom.obj

!ifdef false
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

static voxie_wind_t vw;

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	point3d tp;
	double tim = 0.0, otim, dtim, c0, s0, c1, s1;
	static const point3d pt[] =
	{
		-0.25608,+0.12510,+0.00000, /*0*/
		-0.32615,-0.11613,-0.00001, /*1*/
		+0.15337,+0.26294,+0.00001, /*2*/
		+0.22466,-0.21760,-0.00003, /*3*/
		-0.04379,-0.24280,+0.00005, /*4*/
		+0.29938,+0.03840,+0.00004, /*5*/
		-0.00162,-0.12853,-0.00001, /*6*/
		+0.17906,+0.02082,+0.00002, /*7*/
		-0.09114,-0.01940,-0.00001, /*8*/
		-0.04027,+0.10988,-0.00003, /*9*/
		-0.14446,+0.19342,-0.00003, /*10*/
		+0.09713,+0.12767,-0.00002, /*11*/
		+0.13120,-0.10479,-0.00003, /*12*/
		-0.22862,-0.00681,+0.00001, /*13*/
		-0.35520,+0.16797,+0.00041, /*14*/
		-0.35032,-0.14326,+0.10275, /*15*/
		-0.41675,-0.08398,-0.05137, /*16*/
		-0.28393,-0.20247,-0.05142, /*17*/
		+0.16730,+0.29648,+0.10278, /*18*/
		+0.24950,+0.26234,-0.05136, /*19*/
		+0.08514,+0.33066,-0.05137, /*20*/
		+0.24783,-0.24560,-0.10280, /*21*/
		+0.31638,-0.18880,+0.05133, /*22*/
		+0.17931,-0.30235,+0.05137, /*23*/
	};
	static point3d pt2[sizeof(pt)/sizeof(pt[0])];
	static const int lin[] =
	{
		8, 9, 6, 8, 8,13, 9,11, 9,10, 6,12, 4, 6, 0,13,
		1,13, 7,11, 2,11, 3,12, 5, 7, 0,10, 7,12, 0,14,
		1,15, 1,16, 1,17, 2,18, 2,19, 2,20, 3,21, 3,22,
		3,23,
	};
	float connradius = 0.005f, molradius = 0.035f;
	int i;

	if (voxie_load(&vw) < 0) { return(-1); }
	vw.usecol = 0;
	if (voxie_init(&vw) < 0) { return(-1); }

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		if (voxie_keystat(0x1)) { voxie_quitloop(); } //ESC: quit

		if (voxie_keystat(0x39) == 1) { voxie_playsound("blowup2.flac",-1,100,100,1.f); }

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

			//draw wireframe box around volume
		voxie_drawbox(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz,1,0xffffff);

		c0 = cos(tim*.5f); s0 = sin(tim*.5f);
		c1 = cos(tim*.5f); s1 = sin(tim*.5f);
		for(i=sizeof(pt)/sizeof(pt[0])-1;i>=0;i--)
		{
			tp.x = pt[i].x*c0 - pt[i].y*s0;
			tp.y = pt[i].y*c0 + pt[i].x*s0;
			tp.z = pt[i].z;

			pt2[i].x = tp.x*c1 - tp.z*s1;
			pt2[i].y = tp.y;
			pt2[i].z = tp.z*c1 + tp.x*s1;

			voxie_drawsph(&vf,pt2[i].x,pt2[i].y,pt2[i].z,molradius,0,0xffffff);
			//voxie_drawsph(&vf,pt2[i].x,pt2[i].y,pt2[i].z,molradius,1,0xffffff);
		}
		for(i=sizeof(lin)/sizeof(lin[0])-2;i>=0;i-=2)
		{
			//voxie_drawlin(&vf,pt2[lin[i  ]].x,pt2[lin[i  ]].y,pt2[lin[i  ]].z,
			//                  pt2[lin[i+1]].x,pt2[lin[i+1]].y,pt2[lin[i+1]].z,0xffffff);
			voxie_drawcone(&vf,pt2[lin[i  ]].x,pt2[lin[i  ]].y,pt2[lin[i  ]].z,connradius,
									 pt2[lin[i+1]].x,pt2[lin[i+1]].y,pt2[lin[i+1]].z,connradius,0,0xffffff);
		}

		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0);
	return(0);
}

#if 0
!endif
#endif
