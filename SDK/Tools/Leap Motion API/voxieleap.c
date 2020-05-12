#if 0
voxieleap.exe: voxieleap.c voxiebox.h; cl /TP voxieleap.c /Ox /MT /Ic:\LeapSDK\include /link user32.lib
	del voxieleap.obj
!if 0
#endif

#include "voxiebox.h"
#include <process.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

#define USELEAP 1

//--------------------------------------------------------------------------------------------------
#if (USELEAP != 0)
	//Based on leap3/leaptest.c (12/01/2017)
#pragma comment(lib,"c:/LeapSDK/lib/x64/LeapC.lib")
#include "LeapC.h"
static LEAP_CONNECTION leap_con;
static LEAP_TRACKING_EVENT leap_frame[2] = {0,0}; //FIFO for thread safety
static int leap_framecnt = 0, leap_iscon = 0, leap_isrun = 0;
static void leap_thread (void *_)
{
	LEAP_CONNECTION_MESSAGE msg;
	eLeapRS r;
	while (leap_isrun)
	{
		r = LeapPollConnection(leap_con,1000,&msg); if (r != eLeapRS_Success) continue;
		switch (msg.type)
		{
			case eLeapEventType_Tracking: memcpy(&leap_frame[leap_framecnt&1],msg.tracking_event,sizeof(LEAP_TRACKING_EVENT)); leap_framecnt++; break;
			case eLeapEventType_Connection:     leap_iscon = 1; break;
			case eLeapEventType_ConnectionLost: leap_iscon = 0; break;
			default: break;
		}
	}
}
void leap_uninit (void) { LeapDestroyConnection(leap_con); leap_isrun = 0; }
LEAP_CONNECTION *leap_init (void)
{
	eLeapRS r;
	r = LeapCreateConnection(0,&leap_con); if (r != eLeapRS_Success) return(&leap_con);
	r = LeapOpenConnection(leap_con);      if (r != eLeapRS_Success) return(&leap_con);
	leap_isrun = 1; _beginthread(leap_thread,0,0);
	while (!leap_iscon) Sleep(15);
	return(&leap_con);
}
LEAP_TRACKING_EVENT *leap_getframe (void) { return(&leap_frame[leap_framecnt&1]); }
#else
//--------------------------------------------------------------------------------------------------
	//Leap emulation using mouse :/
typedef struct { float x, y, z; } LEAP_VECTOR;
typedef struct { LEAP_VECTOR prev_joint, next_joint; float width; } LEAP_BONE;
typedef struct { LEAP_BONE bones[4]; LEAP_VECTOR tip_velocity, stabilized_tip_position; uint32_t is_extended; } LEAP_DIGIT;
typedef struct { LEAP_VECTOR position, stabilized_position, velocity, normal; float width; LEAP_VECTOR direction; } LEAP_PALM;
typedef enum { eLeapHandType_Left, eLeapHandType_Right } eLeapHandType;
typedef struct { eLeapHandType type; float confidence, pinch_distance, grab_angle, pinch_strength, grab_strength; LEAP_PALM palm; LEAP_DIGIT digits[5]; LEAP_BONE arm; } LEAP_HAND;
typedef struct { uint32_t nHands; LEAP_HAND *pHands; } LEAP_TRACKING_EVENT;

void leap_uninit (void) { }
void *leap_init (void) { }
LEAP_TRACKING_EVENT *leap_getframe (void)
{
	static LEAP_HAND leap_hand;
	static LEAP_TRACKING_EVENT leap_frame;
	static float cmousx = 0.f, cmousy = 150.f, cmousz = 0.f;

	cmousy += (float)((voxie_keystat(0xc9)!=0)-(voxie_keystat(0xd1)!=0))*gdtim*128.f;
	cmousx += ((float)gpin->dmousx)*+.20f; cmousx = min(max(cmousx,  0.f - 1.00f*100.f),  0.f + 1.00f*100.f);
	cmousy += ((float)gpin->dmousz)*+.05f; cmousy = min(max(cmousy,150.f - 0.57f*100.f),150.f + 0.57f*100.f);
	cmousz += ((float)gpin->dmousy)*+.20f; cmousz = min(max(cmousz,  0.f - 1.00f*100.f),  0.f + 1.00f*100.f);
	leap_frame.nHands = 1;
	leap_frame.pHands = &leap_hand;
	leap_hand.type = eLeapHandType_Right;
	leap_hand.palm.position.x = cmousx;
	leap_hand.palm.position.y = cmousy;
	leap_hand.palm.position.z = cmousz;
	leap_hand.palm.direction.x = 0.f; leap.palm.normal.x = 0.f;
	leap_hand.palm.direction.y = 1.f; leap.palm.normal.y = 0.f;
	leap_hand.palm.direction.z = 0.f; leap.palm.normal.z = 1.f;

	return(&leap_frame);
}
#endif
//--------------------------------------------------------------------------------------------------

static void drawkey (voxie_frame_t *vf, float fx, float fy, float fz, float *keyoff, int keyn, float ft, int col, int isfill)
{
	pol_t *pt;
	float c, s, ty, tz;
	int i, j;

	pt = (pol_t *)_alloca(keyn*sizeof(pt[0])); if (!pt) return;

	if (ft < 0.f) ft = 0.f;
	c = cos(ft*2.0);
	s = sin(ft*2.0);
	for(i=keyn-1,j=0;j<keyn;i=j,j++)
	{
		pt[i].x = keyoff[i*2+0]+fx; ty = keyoff[i*2+1]; tz = 0.0;
		pt[i].y = c*(ty-keyoff[1]) - s*(tz          ) + keyoff[1] + fy;
		pt[i].z = c*(tz          ) + s*(ty-keyoff[1]) +           + fz;
		pt[i].p2 = j;
	}

	if (!isfill)
	{
		for(i=keyn-1,j=0;j<keyn;i=j,j++)
			voxie_drawlin(vf,pt[i].x,pt[i].y,pt[i].z,pt[j].x,pt[j].y,pt[j].z,col);
	}
	else
	{
		voxie_drawpol(vf,pt,keyn,col);
	}
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_wind_t vw;
	voxie_frame_t vf;
	voxie_inputs_t in;

	#define PTMAX 65536
	typedef struct { int col, tool, islin; float x, y, z, dx, dy, dz; } pt_t;
	static pt_t pt[PTMAX];
	int ptn = 0;

	#define BALLMAX 64
	typedef struct
	{
		float x, y, z, r;
		float xv, yv, zv;
		int col, ch;
	} ball_t;
	static ball_t ball[BALLMAX];
	int balln;

	LEAP_TRACKING_EVENT *frame;
	LEAP_HAND *hand;
	LEAP_PALM *palm;
	LEAP_DIGIT *digit;
	LEAP_VECTOR *vec;
	point3d palmp, palmr, palmd, palmf;

	point3d pad = {0.f,0.f,0.f}, opad;

	point3d pp, rr, dd, ff;
	double tim = 0.0, otim, dtim, rdtim;
	float f, g, fx, fy, fz, d, vx, vy, vz;
	int i, j, k, m, x, y, islin = 0, numframes = 0, curcol = 7, curtool = 0, rendmode = 0, clickmode = 1;
	int pal24[8] = {0x000000,0x0000ff,0x00ff00,0x00ffff,0xff0000,0xff00ff,0xffff00,0xffffff};

	leap_init();

	if (voxie_load(&vw) < 0) { MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	//vw.usecol = 0;
	//vw.aspz = 0.2f;
	if (voxie_init(&vw) < 0) { MessageBox(0,"Error: voxie_init() failed","",MB_OK); return(-1); }

	balln = 26;
	for(i=0;i<balln;i++)
	{
		ball[i].ch = i+'A';
		ball[i].x = (rand()/32768.0*2.0 - 1.0)*vw.aspx;
		ball[i].y = (rand()/32768.0*2.0 - 1.0)*vw.aspy;
		ball[i].z = (rand()/32768.0*2.0 - 1.0)*vw.aspz;
		ball[i].r = .12*rand()/32768.0+.08;
		ball[i].xv = (rand()-16384)/32768.0;
		ball[i].yv = (rand()-16384)/32768.0;
		ball[i].zv = (rand()-16384)/32768.0;
		ball[i].col = pal24[(rand()%6)+1];
	}
	ball[0].col = 7;

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; rdtim = 1.0/dtim;

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		if (voxie_keystat(0x2)) { rendmode = 0; clickmode = 1; }
		if (voxie_keystat(0x3)) { rendmode = 1; }
		if (voxie_keystat(0x4)) { rendmode = 2; }

		while (i = voxie_keyread())
		{
		}

		if (vw.useemu)
		{
			i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
			if (i)
			{
					  if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
				else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,2000.0); //Ctrl+[,]
				else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
				voxie_init(&vw);
			}
		}

		voxie_frame_start(&vf);
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

		f = 1.f; voxie_setview(&vf,vw.aspx*-f,vw.aspy*-f,vw.aspz*-f,vw.aspx*f,vw.aspy*f,vw.aspz*f);

#if (!USELEAP)
		gpin = &in; gdtim = dtim; //Hack for emulation!
#endif
		frame = leap_getframe();

		if (rendmode == 0) //line paint
		{
			voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,pal24[curcol]); //draw wireframe box

			if (in.bstat) clickmode = 1;

			if ((frame) && (frame->nHands > 0))
			{
				hand = &frame->pHands[0];
				digit = &hand->digits[1];
				//vec = &digit->bones[3].next_joint; //index finger of 1st hand
				vec = &digit->stabilized_tip_position;

				opad = pad;
				pad.x = vec->x*+.01f     ; pad.x = min(max(pad.x,-vw.aspx),vw.aspx);
				pad.y = vec->z*+.01f     ; pad.y = min(max(pad.y,-vw.aspy),vw.aspy);
				pad.z = vec->y*-.01f+1.5f; pad.z = min(max(pad.z,-vw.aspz),vw.aspz);

				for(i=1;i<=7;i++)
				{
					static const char notelut[8] = {-1,0,2,4,5,7,9,11};
					fx = (float)(i-4)*vw.aspx*.3f;
					fy = vw.aspy*-0.75f;
					fz = vw.aspz*0.95f;
					if ((fabs(pad.x-fx) < vw.aspx*.12f) && (pad.y <= fy) && (pad.z >= fz) && (curcol != i))
						{ curcol = i; voxie_playsound("c:/windows/media/ding.wav",-1,500,500,pow(2.f,(float)notelut[i]/12.f)*.45f); }
				}
				if ((pad.x < vw.aspx*-.65f) && (fabs(pad.y-vw.aspy*-0.60f) < .12f) && (pad.z > vw.aspz*0.95f) && (curtool != 0)) { curtool = 0; voxie_playsound("c:/windows/media/tada.wav",-1,200,200,2.5f); }
				if ((pad.x < vw.aspx*-.65f) && (fabs(pad.y-vw.aspy*-0.36f) < .12f) && (pad.z > vw.aspz*0.95f) && (curtool != 1)) { curtool = 1; voxie_playsound("c:/windows/media/tada.wav",-1,200,200,2.7f); }
				if ((pad.x < -vw.aspx+.35f) && (pad.y > vw.aspy-.2f) && (pad.z > vw.aspz*0.95f) && (ptn)) { ptn = 0; voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,1.f); }

				if (curtool == 0)
				{
					voxie_drawlin(&vf,pad.x-.1f,pad.y+.0f,pad.z+.0f,pad.x+.1f,pad.y+.0f,pad.z+.0f,pal24[curcol]);
					voxie_drawlin(&vf,pad.x+.0f,pad.y-.1f,pad.z+.0f,pad.x+.0f,pad.y+.1f,pad.z+.0f,pal24[curcol]);
					voxie_drawlin(&vf,pad.x+.0f,pad.y+.0f,pad.z-.1f,pad.x+.0f,pad.y+.0f,pad.z+.1f,pal24[curcol]);
				}
				if (curtool == 1) voxie_drawsph(&vf,pad.x,pad.y,pad.z,.05f,0,pal24[curcol]);

				if (((vec->y < -.3f) && (!clickmode)) || ((in.bstat&1) && (clickmode)))
				{
					//if (!(in.obstat&1)) islin = 0;
					if ((!(numframes&3)) || (curtool))
					{
						i = ptn&(PTMAX-1); ptn++;
						pt[i].col = curcol;
						pt[i].islin = islin; islin = 1;
						pt[i].tool = curtool;
						pt[i].x = pad.x;
						pt[i].y = pad.y;
						pt[i].z = pad.z;
						pt[i].dx = (vec->x - digit->bones[3].prev_joint.x)*+.2f;
						pt[i].dy = (vec->z - digit->bones[3].prev_joint.z)*+.2f;
						pt[i].dz = (vec->y - digit->bones[3].prev_joint.y)*-.2f;
						//if (!(numframes&15)) voxie_playsound("c:/windows/media/ir_begin.wav",-1,50,50,2.0f); //highlights slow fps :/
					}
				} else islin = 0;
			}

			for(i=1;i<=7;i++)
			{
				fx = (float)(i-4)*vw.aspx*.3f;
				voxie_drawbox(&vf,fx-vw.aspx*.12f,vw.aspy*-1.00f,vw.aspz*0.95f,
										fx+vw.aspx*.12f,vw.aspy*-0.75f,vw.aspz*1.00f,(curcol==i)+1,pal24[i]);
			}

			voxie_drawlin(&vf,vw.aspx*-.95f,vw.aspy*-0.60f,vw.aspz*0.9f,
									vw.aspx*-.70f,vw.aspy*-0.60f,vw.aspz*0.9f,pal24[curcol]);
			voxie_drawbox(&vf,vw.aspx*-1.f,vw.aspy*-0.6f-.12f,vw.aspz*1.f,vw.aspx*-.65f,vw.aspy*-0.60f+.12f,vw.aspz*1.f,(curtool==0),pal24[curcol]);

			for(i=-6;i<=6;i++)
				voxie_drawsph(&vf,vw.aspx*-.82f+(float)i*.02f,vw.aspy*-0.36f,vw.aspz*0.9f,.02f,0,pal24[curcol]);
			voxie_drawbox(&vf,vw.aspx*-1.f,vw.aspy*-0.36f-.12f,vw.aspz*1.f,vw.aspx*-.65f,vw.aspy*-0.36f+.12f,vw.aspz*1.f,(curtool==1),pal24[curcol]);

			rr.x = 0.10f; dd.x = 0.00f; pp.x =-vw.aspx+.04f;
			rr.y = 0.00f; dd.y = 0.15f; pp.y = vw.aspy-.16f;
			rr.z = 0.00f; dd.z = 0.00f; pp.z = vw.aspz;
			voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"CLS");
			voxie_drawbox(&vf,pp.x,vw.aspy-.2f,pp.z,-vw.aspx+.35f,vw.aspy,pp.z,1,pal24[curcol]);


			if (in.bstat&~in.obstat&2) { ptn = 0; voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,1.f); }

			for(j=max(ptn-PTMAX,0);j<ptn;j++)
			{
				i = (j&(PTMAX-1));
				if (pt[i].islin) voxie_drawlin(&vf,pt[i].x,pt[i].y,pt[i].z,pt[(i-1)&(PTMAX-1)].x,pt[(i-1)&(PTMAX-1)].y,pt[(i-1)&(PTMAX-1)].z,pal24[pt[i].col]);
				if (pt[i].tool)
				{
					voxie_drawsph(&vf,pt[i].x,pt[i].y,pt[i].z,.02f,0,pal24[pt[i].col]);
					//voxie_drawlin(&vf,pt[i].x,pt[i].y,pt[i].z,pt[i].x+pt[i].dx,pt[i].y+pt[i].dy,pt[i].z+pt[i].dz,pal24[pt[i].col]);
				}
			}
			Sleep(5);
		}
		else if (rendmode == 1) //piano
		{
			static point3d fing[20], ofing[20];
			static int fillmode = 1;
			static double keytim[26];

			if (voxie_keystat(0x39) == 1) fillmode ^= 1;

			if (frame)
			{
				for(i=min(frame->nHands,4)-1;i>=0;i--)
				{
					hand = &frame->pHands[i];
					for(j=0;j<5;j++)
					{
						digit = &hand->digits[j];
						if (!digit->is_extended) continue;
						//vec = &digit->bones[3].next_joint;
						vec = &digit->stabilized_tip_position;

						ofing[i*5+j] = fing[i*5+j];
						fing[i*5+j].x = vec->x*+.01f     ; fing[i*5+j].x = min(max(fing[i*5+j].x,-vw.aspx),vw.aspx);
						fing[i*5+j].y = vec->z*+.01f     ; fing[i*5+j].y = min(max(fing[i*5+j].y,-vw.aspy),vw.aspy);
						fing[i*5+j].z = vec->y*-.01f+1.5f; fing[i*5+j].z = min(max(fing[i*5+j].z,-vw.aspz),vw.aspz);

						for(f=.03f;f<=.06f;f+=.01f)
							voxie_drawsph(&vf,fing[i*5+j].x,fing[i*5+j].y,fing[i*5+j].z,f,0,0xffffff);

						for(y=0;y<2;y++)
						{
							for(x=0;x<=7;x++)
							{
								static const char notelut[8] = {0,2,4,5,7,9,11,12};
								fx = (float)(x-4)*.25f;
								if ((fing[i*5+j].x >= fx+0.00f) && (fing[i*5+j].x < fx+.22f) && (fing[i*5+j].y > ((float)y-.5f)*1.1f-0.35f) && (fing[i*5+j].y < ((float)y-.5f)*1.1f+0.35f) && (fing[i*5+j].z >= +0.0f) && (ofing[i*5+j].z < +0.0f))
								{
										//Ignore if over black key..
									for(m=0;m<=5;m++)
									{
										if (m == 2) continue;
										fx = (float)(x-4)*.25f;
										if ((fing[i*5+j].x >= fx+.14f) && (fing[i*5+j].x < fx+.31f) && (fing[i*5+j].y > ((float)y-.5f)*1.1f-0.35f) && (fing[i*5+j].y < ((float)y-.5f)*1.1f+0.05f)) break;
									}
									if (m <= 5) continue;

									k = ((int)(fabs(ofing[i*5+j].z-fing[i*5+j].z)*500.f));
									keytim[notelut[x]+y*13] = tim+0.25;
									voxie_playsound("fminst2.flac",-1,k,k,pow(2.f,(float)notelut[x]/12.f)*(float)(2-y)*.5f);
								}
							}
							for(x=0;x<=5;x++)
							{
								static const char notelut[6] = {1,3,-1,6,8,10};
								if (x == 2) continue;
								fx = (float)(x-4)*.25f;
								if ((fing[i*5+j].x >= fx+.14f) && (fing[i*5+j].x < fx+.31f) && (fing[i*5+j].y > ((float)y-.5f)*1.1f-0.35f) && (fing[i*5+j].y < ((float)y-.5f)*1.1f+0.05f) && (fing[i*5+j].z >= -0.19f) && (ofing[i*5+j].z < -0.19f))
								{
									k = (int)(fabs(ofing[i*5+j].z-fing[i*5+j].z)*500.f);
									keytim[notelut[x]+y*13] = tim+0.25;
									voxie_playsound("fminst2.flac",-1,k,k,pow(2.f,(float)notelut[x]/12.f)*(float)(2-y)*.5f);
								}
							}
						}
					}
				}
			}

			for(y=0;y<2;y++)
			{
				for(x=0;x<=7;x++)
				{
					static const char notelut[8] = {0,2,4,5,7,9,11,12};
					fx = ((float)(x-4))*.25f+.125f;
					fy = ((float)y-.5f)*1.1f;
					fz = 0.f;

						// lefkey:  midkey:  rigkey:  reckey:
						// 4---5      6-7      5---4  0----1
						// |   |      | |      |   |  |    |
						// |   0-1  4-5 0-1  1-0   |  |    |
						// |     |  |     |  |     |  |    |
						// 3-----2  3-----2  2-----3  3----2
					static float lefkey[6][2] = {+.03f,+.05f, +.11f,+.05f, +.11f,+.35f, -.11f,+.35f, -.11f,-.35f, +.03f,-.35f};
					static float midkey[8][2] = {+.04f,+.05f, +.11f,+.05f, +.11f,+.35f, -.11f,+.35f, -.11f,+.05f, -.04f,+.05f, -.04f,-.35f, +.04f,-.35f};
					static float rigkey[6][2] = {-.03f,+.05f, -.11f,+.05f, -.11f,+.35f, +.11f,+.35f, +.11f,-.35f, -.03f,-.35f};
					static float reckey[4][2] = {-.11f,-.35f, +.11f,-.35f, +.11f,+.35f, -.11f,+.35f};

					f = keytim[notelut[x]+y*13]-tim;
						  if ((1<<x)&((1<<0)|(1<<3))) { drawkey(&vf,fx,fy,fz,(float *)lefkey,6,f,0xffffff,fillmode); }
					else if ((1<<x)&((1<<2)|(1<<6))) { drawkey(&vf,fx,fy,fz,(float *)rigkey,6,f,0xffffff,fillmode); }
					else if ((1<<x) == (1<<7))       { drawkey(&vf,fx,fy,fz,(float *)reckey,4,f,0xffffff,fillmode); }
					else                             { drawkey(&vf,fx,fy,fz,(float *)midkey,8,f,0xffffff,fillmode); }
				}
				for(x=0;x<=5;x++)
				{
					static const char notelut[6] = {1,3,-1,6,8,10};
					static float reckey[4][2] = {-.08f,-.35f, +.08f,-.35f, +.08f,+.05f, -.08f,+.05f};
					if (x == 2) continue;
					fx = ((float)(x-4))*.25f+.25f;
					fy = ((float)y-.5f)*1.1f;
					fz = -.19f;
					f = keytim[notelut[x]+y*13]-tim;
					drawkey(&vf,fx,fy,fz,(float *)reckey,4,f,0xffffff,0);
				}
			}
		}
		else if (rendmode == 2) //magpoolballs
		{
			voxie_drawbox(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz,1,0x00ffff); //draw wireframe box

			if ((frame) && (frame->nHands > 0))
			{
				hand = &frame->pHands[0];
				digit = &hand->digits[1];
				//vec = &digit->bones[3].next_joint; //index finger of 1st hand
				vec = &digit->stabilized_tip_position;

				opad = pad;
				pad.x = vec->x*+.01f     ; pad.x = min(max(pad.x,-vw.aspx),vw.aspx);
				pad.y = vec->z*+.01f     ; pad.y = min(max(pad.y,-vw.aspy),vw.aspy);
				pad.z = vec->y*-.01f+1.5f; pad.z = min(max(pad.z,-vw.aspz),vw.aspz);

				ball[0].x = pad.x; ball[0].y = pad.y; ball[0].z = pad.z; ball[0].xv = 0.f; ball[0].yv = 0.f; ball[0].zv = 0.f; //Hack for balls demo
			}

			//for(i=0;i<balln;i++) ball[i].zv += dtim*4; //Gravity
			for(i=0;i<balln;i++)
			{
				if (i)
				{
					ball[i].x += ball[i].xv*dtim;
					ball[i].y += ball[i].yv*dtim;
					ball[i].z += ball[i].zv*dtim;
					if ((ball[i].z+ball[i].r > -vw.aspz) && (fabs(ball[i].x) < vw.aspx) && (fabs(ball[i].y) < vw.aspy))
					{
						if (ball[i].x-ball[i].r <-vw.aspx) { ball[i].x = (-vw.aspx+ball[i].r)*2-ball[i].x; ball[i].xv = fabs(ball[i].xv)*+.9; }
						if (ball[i].x+ball[i].r > vw.aspx) { ball[i].x = (+vw.aspx-ball[i].r)*2-ball[i].x; ball[i].xv = fabs(ball[i].xv)*-.9; }
						if (ball[i].y-ball[i].r <-vw.aspy) { ball[i].y = (-vw.aspy+ball[i].r)*2-ball[i].y; ball[i].yv = fabs(ball[i].yv)*+.9; }
						if (ball[i].y+ball[i].r > vw.aspy) { ball[i].y = (+vw.aspy-ball[i].r)*2-ball[i].y; ball[i].yv = fabs(ball[i].yv)*-.9; }
					}
					if (ball[i].z-ball[i].r <-vw.aspz) { ball[i].z = (-vw.aspz+ball[i].r)*2-ball[i].z; ball[i].zv = fabs(ball[i].zv)*+.9; }
					if (ball[i].z+ball[i].r > vw.aspz) { ball[i].z = (+vw.aspz-ball[i].r)*2-ball[i].z; ball[i].zv = fabs(ball[i].zv)*-.9; }
				}
				for(j=0;j<balln;j++)
				{
					if (i == j) continue;
					d = sqrt((ball[i].x-ball[j].x)*(ball[i].x-ball[j].x)
							 + (ball[i].y-ball[j].y)*(ball[i].y-ball[j].y)
							 + (ball[i].z-ball[j].z)*(ball[i].z-ball[j].z)) - (ball[i].r+ball[j].r);
					if (d < 0)
					{
						vx = ball[j].x-ball[i].x;
						vy = ball[j].y-ball[i].y;
						vz = ball[j].z-ball[i].z;
						f = 1.0/sqrt(vx*vx+vy*vy+vz*vz); vx *= f; vy *= f; vz *= f;
						g = ball[i].r*ball[i].r + ball[j].r*ball[j].r;
						ball[i].x += vx*d*.5f; ball[i].xv += vx*d*(ball[j].r*ball[j].r/g)*rdtim;
						ball[i].y += vy*d*.5f; ball[i].yv += vy*d*(ball[j].r*ball[j].r/g)*rdtim;
						ball[i].z += vz*d*.5f; ball[i].zv += vz*d*(ball[j].r*ball[j].r/g)*rdtim;
						ball[j].x -= vx*d*.5f; ball[j].xv -= vx*d*(ball[i].r*ball[i].r/g)*rdtim;
						ball[j].y -= vy*d*.5f; ball[j].yv -= vy*d*(ball[i].r*ball[i].r/g)*rdtim;
						ball[j].z -= vz*d*.5f; ball[j].zv -= vz*d*(ball[i].r*ball[i].r/g)*rdtim;
						ball[i].xv = min(max(ball[i].xv,-vw.aspx),vw.aspx); ball[j].xv = min(max(ball[j].xv,-vw.aspx),vw.aspx);
						ball[i].yv = min(max(ball[i].yv,-vw.aspy),vw.aspy); ball[j].yv = min(max(ball[j].yv,-vw.aspy),vw.aspy);
						ball[i].zv = min(max(ball[i].zv,-vw.aspz),vw.aspz); ball[j].zv = min(max(ball[j].zv,-vw.aspz),vw.aspz);
					}
				}

					//Friction
				ball[i].xv *= exp(dtim*-.1);
				ball[i].yv *= exp(dtim*-.1);
				ball[i].zv *= exp(dtim*-.1);

				voxie_drawsph(&vf,ball[i].x,ball[i].y,ball[i].z,ball[i].r,0,ball[i].col);

				rr.x = 0.10f; dd.x =                    0.00f; pp.x = ball[i].x - rr.x*.5f - dd.x*.5f;
				rr.y = 0.00f; dd.y = cos(tim+(double)i)*0.15f; pp.y = ball[i].y - rr.y*.5f - dd.y*.5f;
				rr.z = 0.00f; dd.z = sin(tim+(double)i)*0.15f; pp.z = ball[i].z - rr.z*.5f - dd.z*.5f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%c",ball[i].ch);
			}
			Sleep(5);
		}

		voxie_frame_end(); voxie_getvw(&vw); numframes++;
	}

	voxie_uninit(0);
	leap_uninit();

	return(0);
}

#if 0
!endif
#endif
