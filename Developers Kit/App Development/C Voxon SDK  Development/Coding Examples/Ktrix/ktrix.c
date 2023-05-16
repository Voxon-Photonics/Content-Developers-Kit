#if 0
ktrix.exe: ktrix.c voxiebox.h; cl /TP ktrix.c /Ox /MT /link
!ifdef 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "moxie.h"

/* Some interesting tricks you can do with the Voxiebox code. Demonstrated by Ken Silverman in an interactive class Feb 2023
 *
 *
 */


static voxie_wind_t vw;

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	point3d pp = {0.f,0.f,0.f}, pr, pd, pf, offs = {0.f,0.f,0.f}; //x=right,y=down,z=forward
	double tim = 0.0, otim, dtim;
	float f = 0.f;
	int i, j, x, y, z, n;
	#define PI 3.14159265358979323846264
	static int hei[256*256];

	if (voxie_load(&vw) < 0) { return(-1); }
	if (voxie_init(&vw) < 0) { return(-1); }

	for(y=0;y<256;y++)
		for(x=0;x<256;x++)
		{
			z = (int)(sin((float)x*.2f)*64 + cos((float)y*.2f)*64 + 128);
			hei[y*256+x] = (z*0x010101 ^ ((rand()+(rand()<<15))&0x3f3f3f)) + (z<<24); //generate random colors with sine-like height ;)
		}

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		while (i = voxie_keyread())
		{
			switch(i&0xff)
			{
				case '1': vw.clipshape = 0; voxie_init(&vw); break;
				case '2': vw.clipshape = 1; voxie_init(&vw); break;
			}
		}

		if (vw.useemu)
		{
			i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1); //']' - '['
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

		f = tim;
		pr.x = cos(f); pd.x = sin(f); pf.x = 0.0f; pp.x = 0.0f;
		pr.y =-sin(f); pd.y = cos(f); pf.y = 0.0f; pp.y = 0.0f;
		pr.z =   0.0f; pd.z =   0.0f; pf.z = 1.0f; pp.z = 0.0f;
		moxie_setcam(&pp,&pr,&pd,&pf);

		if (vw.clipshape == 0)
		{
			voxie_drawbox(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz,1,0xffffff); //draw rectangular wireframe box
		}
		else
		{
				//Render circle using single call ;)
			point3d pp, rr, dd;
			rr.x = vw.aspr*2.5f; rr.y =  0.f; rr.z = 0.f;
			dd.x = 0.f;          dd.y = rr.x; dd.z = 0.f;
			pp.x = -vw.aspr;     pp.y = -vw.aspr;
			for(j=-1;j<=1;j+=2) { pp.z = vw.aspz*j; voxie_printalph_ext(&vf,&pp,&rr,&dd,.001f,0xffffff,"O"); }

			n = 32;
			for(i=0;i<n;i++)
			{
				voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, -vw.aspz,
										cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, +vw.aspz, 0xffffff);
			}
		}

		pr.x = 1.0f; pd.x = 0.0f; pf.x = 0.0f; pp.x = 0.f;
		pr.y = 0.0f; pd.y = 1.0f; pf.y = 0.0f; pp.y = 0.f;
		pr.z = 0.0f; pd.z = 0.0f; pf.z = 1.0f; pp.z = 0.f;
		moxie_drawspr(&vf,"caco.kv6",&pp,&pr,&pd,&pf,0x404040);

		pr.x = 1.0f; pd.x = 0.0f; pf.x = 0.0f; pp.x = 0.5f;
		pr.y = 0.0f; pd.y = 1.0f; pf.y = 0.0f; pp.y = 0.f;
		pr.z = 0.0f; pd.z = 0.0f; pf.z = 1.0f; pp.z = 0.f;
		moxie_drawspr(&vf,"caco.kv6",&pp,&pr,&pd,&pf,0x404040);

		pr.x = 5.0f; pd.x = 0.0f; pf.x = 0.0f; pp.x = (pr.x + pd.x + pf.x)*-.5f + offs.x;
		pr.y = 0.0f; pd.y = 5.0f; pf.y = 0.0f; pp.y = (pr.y + pd.y + pf.y)*-.5f + offs.y;
		pr.z = 0.0f; pd.z = 0.0f; pf.z =-0.2f; pp.z = (pr.z + pd.z + pf.z)*-.5f + offs.z;

		tiletype tt;
		tt.f = (INT_PTR)hei;
		tt.x = 256; tt.p = tt.x*4;
		tt.y = 256;
		moxie_drawheimap(&vf,(char *)&tt,&pp,&pr,&pd,&pf,-1,0,(1<<3)+(1<<1)+(1<<2));

		if (voxie_keystat(0xcb)) offs.x -= dtim; //left arrow
		if (voxie_keystat(0xcd)) offs.x += dtim; //right arrow

		voxie_frame_end(); voxie_getvw(&vw);

		if (voxie_keystat(0x01)) { voxie_quitloop(); } //ESC: quit
	}

	voxie_uninit(0); //shut down voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
