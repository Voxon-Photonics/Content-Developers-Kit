#if 0

voxiemin.exe: voxiemin.c voxiebox.h; cl /TP voxiemin.c /Ox /MT /link
	del voxiemin.obj

# voxiebox.c->o
#     comp:   rend/emu:
#-O0:  5sec   40ms/270ms
#-O1: 11sec   30ms/190ms
#-O2: 17sec   27ms/160ms
#-O3: 29sec   25ms/155ms
#OBJS=voxiemin.o voxiebox.o snd2.o ksnd64.o hid4lin.o kplib.o kxwin.o
#OPTS=-pipe -s -O0 -mfpu=crypto-neon-fp-armv8
#LIBS=-lm -lrt -lX11 -lpthread -lasound -lusb
#
##SNDINCS=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/src/hello_pi/libs/ilclient -DOMX_SKIP64BIT
##LIBS+=-L/opt/vc/lib/ -lopenmaxil -lvcos -lbcm_host -lpthread -lm -L/opt/vc/src/hello_pi/libs/ilclient -lilclient
#
#voxiemin: $(OBJS); gcc $(OBJS) -o voxiemin $(OPTS) $(LIBS)
#voxiemin.o: voxiemin.c voxiebox.h; gcc -c -x c++ voxiemin.c -o voxiemin.o $(OPTS)
#voxiebox.o: voxiebox.c           ; gcc -c -x c++ voxiebox.c -o voxiebox.o $(OPTS)
#snd2.o    : snd2.c               ; gcc -c        snd2.c     -o snd2.o     $(OPTS) $(SNDINCS)
#ksnd64.o  : ksnd64.c             ; gcc -c -x c++ ksnd64.c   -o ksnd64.o   $(OPTS) -DUSEKZ
#hid4lin.o : hid4lin.c            ; gcc -c -x c++ hid4lin.c  -o hid4lin.o  $(OPTS)
#kplib.o   : kplib.c              ; gcc -c -x c++ kplib.c    -o kplib.o    $(OPTS) -DFUNCPREFIX="extern \"C\""
#kxwin.o   : kxwin.c kxwin.h      ; gcc -c -x c++ kxwin.c    -o kxwin.o    $(OPTS)

!ifdef false
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

static voxie_wind_t vw;

#if defined(_WIN32)
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
#else
int main (int argc, char **argv)
#endif
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	point3d pp, pr, pd, pf;
	double tim = 0.0, otim, dtim;
	point3d ball[100], balv[100];
	int balc[100];
	float balr[100];
	int i;

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

		if (voxie_keystat(0x39) == 1) { voxie_playsound("blowup2.flac",-1,100,100,1.f); }

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

			//draw wireframe box around volume
		voxie_drawbox(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz,1,0xffffff);
		voxie_drawlin(&vf,-.1, -.1, -.1,
								+.1, +.1, +.1, 0xffffff);

		for(i=0;i<100;i++)
		{
			voxie_drawsph(&vf, ball[i].x,ball[i].y,ball[i].z,balr[i],0,balc[i]);
			ball[i].x += balv[i].x*dtim;
			ball[i].y += balv[i].y*dtim;
			ball[i].z += balv[i].z*dtim;

			if (fabs(ball[i].x) > vw.aspx) balv[i].x *= -1;
			if (fabs(ball[i].y) > vw.aspy) balv[i].y *= -1;
			if (fabs(ball[i].z) > vw.aspz) balv[i].z *= -1;
		}

		//voxie_drawcone(&vf,0.4f,-0.3f, 0.1f, 0.1f,
		//                   0.7f,-0.7f, 0.2f, 0.2f,1,0x202020);

		pr.x = cos(tim); pd.x = sin(tim); pf.x = 0.0f; pp.x =-0.5f;
		pr.y =-sin(tim); pd.y = cos(tim); pf.y = 0.0f; pp.y = 0.0f;
		pr.z = 0.0f; pd.z = 0.0f; pf.z = 1.0f; pp.z = 0.1f;
		//voxie_drawspr(&vf,"caco.kv6",&pp,&pr,&pd,&pf,0xc0c0c0);

		char st[] = "@party";
		for(i=0;i<sizeof(st)/sizeof(st[0]);i++)
		{
			pr.x = 0.2; pd.x = 0.0; pp.x =-0.5 + i*.2;
			pr.y = 0.0; pd.y = 0.0; pp.y =+0.5 + sin(tim+(float)i)*0.2;
			pr.z = 0.0; pd.z = 0.4; pp.z = 0.0;
			voxie_printalph_(&vf,&pp,&pr,&pd,0xffffff,"%c",st[i]);
		}
		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
