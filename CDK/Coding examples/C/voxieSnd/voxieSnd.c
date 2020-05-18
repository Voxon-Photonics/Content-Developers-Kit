#if 0
!if 1

	#Visual C makefile:
mvxsnd.exe: voxieSnd.c voxiebox.h; cl /TP voxieSnd.c /Ox /MT /link user32.lib
	del voxieSnd.obj

!else

	#GNU C makefile:
voxieSnd.exe: voxieSnd.c; gcc voxieSnd.c -o voxieSnd.exe -pipe -O3 -s -m64

!endif
!if 0
#endif
/*
	VoxieSnd a program to understand how the sound functions work on the VX1
	4/11/2019 

*/

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

// musical notes

#define C4 	263
#define Db4 277
#define D4  293
#define E4 	329
#define F4 	349
#define Gb4 370
#define G4 	391
#define Ab4 415
#define A4  440
#define Bb 	466
#define B4 	494
#define C5  523





static voxie_wind_t vw;
static voxie_frame_t vf;
static int gcnti[2], gbstat = 0;
static int frequency;
static int notes[4];

static double gTim;
static int noteOn = -1;
static double noteDuration;
static int gnsamps;
static void mvmix (int *ibuf, int nsamps) {

	if (noteDuration > gTim) {
		noteOn = -1;
	} else {
		noteOn = 0;
	}
	static int cnt[2]; int i, c, n;


	n = 0;
	for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)	{ 

		if (n < 10 && notes[2] != 0) { ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&noteOn; cnt[c] += notes[2]; }
		else if (n < 20 && notes[3] != 0) { ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&noteOn; cnt[c] += notes[3]; }
		else ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&noteOn; cnt[c] += notes[c];		
		n++;
		if (n > 30) n = 0;

	
	}

	gnsamps = nsamps;
			

}



static void mymix (int *ibuf, int nsamps)
{
	static int cnt[2]; int i, c;
	for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)
			//{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&gbstat; cnt[c] += gcnti[c]; }
			{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&noteOn; cnt[c] += notes[c]; }
}





//  functions
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






// main program
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim,  avgdtim = 0.0, mxLoopTim = 0, mxSampLength = 1.75, startTime = 0, stopTime = 0;
	int i, mousx = 256, mousy = 256, mousz = 0;
	point3d ss, pp, rr, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int debug = 1, ovxbut[4], vxnplays, col[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00}, buttonDelay = 0, mTime = 0, numframes = 0,
	testMusic = 0, mx = -1, mxVol = 100, mxLoop = 0,   mxVolDefLevel = 100, mxFade = 0, sampleNo = 0; 
	float mxPitch = 1.f;
	int frequency = 0;
	// global variables

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }

	/*		if (vw.playsamprate)
	{


    	notes[0] = (rand() % 1000 + 500<<21)/vw.playsamprate; // frequency of note
		//notes[0] = (262<<21)/vw.playsamprate; // frequency of note
		//notes[1] = (523<<21)/vw.playsamprate;
		//notes[2] = (1046<<21)/vw.playsamprate;
		

		voxie_setaudplaycb(mymix);
	} */



	while (!voxie_breath(&in)) // Breath must mean its updating loop
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		gTim = tim;
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);

				for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1

		}

		voxie_mountzip("snd.zip");


		// put controls here keys here
		if (voxie_keystat(0x1)) { voxie_quitloop(); }
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

			//draw wireframe box
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		//voxie_drawvox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

#if 1
		// put stuff here
	if (buttonDelay < mTime) {

	if (voxie_keystat(0x02) == 1 && mx == -1 ) {  
		switch(sampleNo) {
	case 0:
		mx = voxie_playsound("mx1.flac", 0, mxVol, mxVol,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music
		if (mxVol == -1) mxFade = 2;
		break;
	case 1:
		mx = voxie_playsound("blowup2.flac", 0, mxVol, mxVol,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music
		break;
		}
	}


	
		if (voxie_keystat(0x03) == 1) { voxie_playsound_update(mx, 0, 0, 0, 1.0); mx = -1; buttonDelay = mTime + 10; testMusic = 0; } // 2 stop music
		if (voxie_keystat(0x04) == 1) { voxie_playsound_update(mx, 0, mxVol, mxVol, mxPitch * 2); buttonDelay = mTime + 10; testMusic = 2; } // 3 speed up music
		if (voxie_keystat(0x05) == 1) { mxVol -= 10;   voxie_playsound_update(mx, 0, mxVol, mxVol, mxPitch); buttonDelay = mTime + 5; } // 4 increase volume
		if (voxie_keystat(0x06) == 1) { mxVol += 10;   voxie_playsound_update(mx, 0, mxVol, mxVol, mxPitch); buttonDelay = mTime + 5; } // 5 decrease volume
		if (voxie_keystat(0x07) == 1) { buttonDelay = mTime + 10 ; if (mxLoop == 0) { mxLoop = 1;} else { mxLoop = 0;} } // 6 loop sound effect
		if (voxie_keystat(0x08) == 1) { buttonDelay = mTime + 10 ; mxFade = rand () % 2;  } // fade out
		if (voxie_keystat(0x09) == 1) { buttonDelay = mTime + 10 ; voxie_playsound_update(mx, 0, -1, -1, mxPitch);  }
		if (voxie_keystat(0x0a) == 1) { startTime = tim;  }
		if (voxie_keystat(0x0b) == 1) { stopTime = tim;  }

		if (voxie_keystat(0x10) == 1) { sampleNo -= 1; buttonDelay = mTime + 10; }
		if (voxie_keystat(0x1e) == 1) { sampleNo += 1; buttonDelay = mTime + 10; }


		if (voxie_keystat(0x12) == 1 ) {
		noteDuration = 1 + tim;

		notes[0] = (mxVol <<21)/vw.playsamprate; // frequency of note
		notes[1] = (mxVol <<21)/vw.playsamprate; // frequency of note
		notes[2] = (mxVol / 2<<21)/vw.playsamprate;
		notes[3] = (mxVol * 2<<21)/vw.playsamprate;
		
		//notes[0] = (262<<21)/vw.playsamprate; // frequency of note
		//notes[1] = (523<<21)/vw.playsamprate;
		//notes[2] = (1046<<21)/vw.playsamprate;
		

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x35) == 1 ) {
		noteDuration = 0.25 + tim;


		notes[0] = (262<<21)/vw.playsamprate; // frequency of note
		notes[1] = (329<<21)/vw.playsamprate; // frequency of note
		notes[2] = (440<<21)/vw.playsamprate; // frequency of note
		notes[3] = (523<<21)/vw.playsamprate; // frequency of note
		//notes[0] = (262<<21)/vw.playsamprate; // frequency of note
		//notes[1] = (523<<21)/vw.playsamprate;
		//notes[2] = (1046<<21)/vw.playsamprate;
		

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2c) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (C4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (C4 / 2<<21)/vw.playsamprate; // frequency of note
		notes[2] = (C4 * 2<<21)/vw.playsamprate;
		notes[3] = (C4 <<21)/vw.playsamprate;
		
		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2d) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (D4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (D4 / 2<<21)/vw.playsamprate; // frequency of note
		notes[2] = (D4 * 2<<21)/vw.playsamprate;
		notes[3] = (D4<<21)/vw.playsamprate;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2e) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (C4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (E4<<21)/vw.playsamprate; // frequency of note
		notes[2] = (G4<<21)/vw.playsamprate;
		notes[3] = (C5<<21)/vw.playsamprate;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2f) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (C4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (E4<<21)/vw.playsamprate; // frequency of note
		notes[2] = (F4<<21)/vw.playsamprate;
		notes[3] = (C5<<21)/vw.playsamprate;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x30) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (G4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (G4<<21)/vw.playsamprate; // frequency of note
		notes[2] = 0;
		notes[3] = 0;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x31) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (A4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (A4<<21)/vw.playsamprate; // frequency of note
		notes[2] = 0;
		notes[3] = 0;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x32) ) {

		noteDuration = 0.01 + tim;
		notes[0] = (C4<<21)/vw.playsamprate; // frequency of note
		notes[1] = (C4 / 2<<21)/vw.playsamprate; // frequency of note
		notes[2] = (rand() % 500<<21)/vw.playsamprate;
		notes[3] = (rand() % 500<<21)/vw.playsamprate;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x33) ) {
		frequency++;
		noteDuration = 0.01 + tim;
		notes[0] = (C4 + frequency<<21)/vw.playsamprate; // frequency of note
		notes[1] = ((C4 / 2) + frequency<<21)/vw.playsamprate; // frequency of note
		notes[2] = 0;
		notes[3] = 0;

		voxie_setaudplaycb(mvmix);

		}

		if (voxie_keystat(0x33) == 0 && voxie_keystat(0x34) == 0) {

			frequency = mxVol;


		}


		if (voxie_keystat(0x34) ) {
		frequency--;
		noteDuration = 0.01 + tim;
		notes[0] = (C5 + frequency<<21)/vw.playsamprate; // frequency of note
		notes[1] = ((C5 / 2) + frequency<<21)/vw.playsamprate; // frequency of note
		notes[2] = 0;
		notes[3] = 0;

		voxie_setaudplaycb(mvmix);
		}


		if (voxie_keystat(0x1f) ) {
		frequency += rand()% 5 - 3;
		noteDuration = 0.01 + tim;
		notes[0] = 0; // frequency of note
		notes[1] = ((C5 / 2) + frequency<<21)/vw.playsamprate; // frequency of note
		notes[2] = (C4 - frequency<<21)/vw.playsamprate; // fr
		notes[3] = ((C5 / 2) - frequency<<21)/vw.playsamprate;

		voxie_setaudplaycb(mvmix);
		}

			


	}
	

#else




#endif
		// final update loop for frame
		if (debug == 1) {
		avgdtim += (dtim-avgdtim)*.1;
		voxie_debug_print6x8_(30,68,0xffc080,-1,"samp = %d VPS %5.1f, t:%d, test Music:%d mxVol:%d mx: %d mxFade: %d, mxLoop: %d ",sampleNo,1.0/avgdtim, mTime, testMusic, mxVol, mx, mxFade, mxLoop);	
		voxie_debug_print6x8_(30,90,0xffc080,-1,"1 = Play Music, 2 = Stop Music, 3 = Change Pitch, 4 = Lower Vol, 5 = Incease Vol, 6 Loop, 7 Fade, 8 mute, 9 start timer 0 - stop time ",1.0/avgdtim, mTime, testMusic, mxVol, mx, mxFade, mxLoop);	
		voxie_debug_print6x8_(30,110,0x00ffff,-1,"Press Z to / (bottom row of keys) to make some SFX using voxie_setaudplaycb() ",1.0/avgdtim, mTime, testMusic, mxVol, mx, mxFade, mxLoop);	
		voxie_debug_print6x8_(30,130,0xff0000,-1,"vw.playsamprate = %d gnsamps = %d",vw.playsamprate, gnsamps );	
		numframes++;
	}

		voxie_frame_end(); voxie_getvw(&vw);
		


		if (mxLoop == 1 && mxLoopTim < tim) {		
				mx = voxie_playsound("blowup2.flac", 0, mxVol, mxVol,1.0); mxLoopTim = tim + mxSampLength; buttonDelay = mTime + 10; testMusic = 1;
		}

		switch(mxFade) {
			case 0: 
			break;
			case 1: // fade out
			mxVol -= 2;
			
			if (mxVol <= 0) {
				mxVol = -1;
				mxFade = 0;
			}
			voxie_playsound_update(mx, 0, mxVol, mxVol, mxPitch);
			break;
			case 2: // fade in
			mxVol += 2;
			voxie_playsound_update(mx, 0, mxVol, mxVol, mxPitch);
			if (mxVol >= mxVolDefLevel) {
				mxVol = mxVolDefLevel;
				mxFade = 0;
			}

		}

		mTime++;
		dtim = tim-otim;

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
