// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
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
	VoxieSnd a program to understand how the sound functions work on the VX1.
	4/11/2019 -- updated 22/6/2021

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
enum MUSICAL_NOTES {
 B1	 	= 6173,

 C2 	= 6540,
 Db2 	= 6929,
 D2		= 7341,
 Eb2    = 7778,
 E2		= 8240,
 F2     = 8730,
 Gb2    = 9249,
 G2		= 9799,
 Ab2	= 10382,
 A2     = 11000,
 Bb2 	= 11654, 
 B2		= 12347,
 
 C3 	= 13081,
 Db3 	= 13859,
 D3		= 14683,
 Eb3    = 15556,
 E3		= 16481,
 F3     = 17461,
 Gb3    = 18499,
 G3		= 19599,
 Ab3	= 20765,
 A3     = 22000,
 Bb3 	= 23308, 
 B3		= 24694,

 C4 	= 26162,
 Db4 	= 27718,
 D4		= 29366,
 Eb4    = 31112,
 E4		= 32962,
 F4     = 34922,
 Gb4    = 36999,
 G4		= 39199,
 Ab4	= 41530,
 A4     = 44000,
 Bb4 	= 46616, 
 B4		= 49388,

 C5 	= 52325,
 Db5 	= 55436,
 D5		= 58732,
 Eb5    = 62225,
 E5		= 65922,
 F5     = 69845,
 Gb5    = 73998,
 G5		= 78399,
 Ab5	= 83060,
 A5     = 88000,
 Bb5 	= 93232, 
 B5		= 98776,

 C6 	= 104650,
 Db6 	= 110873,
 D6		= 117465,
 Eb6    = 124450,
 E6		= 131851,
 F6     = 139691,
 Gb6    = 147997,
 G6		= 156798,
 Ab6	= 166121,
 A6     = 176000,
 Bb6 	= 186465, 
 B6		= 197553,

 C7 	= 209300,
 Db7 	= 221746,
 D7		= 234931,
 Eb7    = 248901,
 E7		= 263702,
 F7     = 279382,
 Gb7    = 295995,
 G7		= 313596,
 Ab7	= 332243,
 A7     = 352000,
 Bb7 	= 372931, 
 B7		= 395106,

 C8 	= 418600,
 Db8 	= 443492,
 D8		= 469863,
 Eb8    = 497803,
 E8		= 527404,
 F8     = 558765,
 Gb8    = 591991,
 G8		= 627192,
 Ab8	= 664487,
 A8     = 704000,
 Bb8 	= 745862, 
 B8		= 790213,

};

int noteArray[78] = {B1, 
					 C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, B2,
					 C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, B3,
					 C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, B4,
					 C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, B5,
					 C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, B6,
					 C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, B7,
					 C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, B8,					  
					};

static voxie_wind_t vw;
static voxie_frame_t vf;

static int frequency;
#define NOTE_MAX 4
static int notes[NOTE_MAX]; // the notes to play
static float notesAmp[NOTE_MAX]; // the note's amplitude

static double gTim;
static int noteOn = -1;
static int gn = 0;
static double noteDuration;
static int gnsamps;


// my custom audio callback function - can handle mulitple notes together and amplitude for each note... 
// uses a few global variables
// notes[] array the notes to be played... 0 means no note.... -1 means no more notes afterwards
// notesAmp the amplitude of note 1 is default level 0 is slience 2 is twice as load
// noteDuration a timer for the note's duration
// noteOn variable to open or close the note (-1 play / 0 not play)
static void mvmix (int *sampleBuffer, int sampleRate) {

	if (noteDuration > gTim) {
		noteOn = -1;
	} else {
		noteOn = 0;
		voxie_setaudplaycb(0); // kills itself from the callback when the note has finished. 
	}
	static int cnt[2]; int i, c, n;

	int nMax = 0;
	for (i = 0; i < NOTE_MAX; i++ ) { if (notes[i] != 0) nMax++; if (notes[i] == -1) { nMax = i; break; } }  

	for(i=0;i<sampleRate;i++,sampleBuffer+=vw.playnchans) {	
		//cnt[] 0 = left channel, 1 = right channel, 
		sampleBuffer[0] = (((cnt[0]&(1<<20))-(1<<19))&noteOn) * (notesAmp[(gn) % nMax]); cnt[0] += notes[(gn) % nMax];  	// send to the LEFT
		sampleBuffer[1] = (((cnt[1]&(1<<20))-(1<<19))&noteOn) * (notesAmp[(gn) % nMax]); cnt[1] += notes[(gn + 1) % nMax];  // send to the RIGHT
	}
	gn++;
	if (gn >= nMax) gn = 0;
	gnsamps = sampleRate; // just needed for debugging
			
}

// An example of an custom audio callback.
// Once called this function will always be called on a seperate thread. Use a global varible to turn it on / off
static int gPlaySound = -1; // if this is set to 0 it will mute. It will play when set to -1 
static int frequencyToPlay = 26162; // middle C note
static void simplemix (int *sampleBuffer, int sampleRate)
{
	static int cnt[2]; 
	int i, c; // c is number of channels to play (vw.playnchans)

	for(i=0;i<sampleRate;i++,sampleBuffer+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)
		{ sampleBuffer[c] = ((cnt[c]&(1<<20))-(1<<19))&gPlaySound; cnt[c] += frequencyToPlay; }
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
	int debug = 1, ovxbut[4], vxnplays, col[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00}, buttonDelay = 0, mTime = 0, 
	testMusic = 0, mx = -1, mxVol = 100, mxLoop = 0,   mxVolDefLevel = 100, mxFade = 0, sampleNo = 0; 
	float mxPitch = 1.f;
	int frequency = 0;
	int rootNote = 24;
	int octave = 0;
	// global variables

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }

	
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ MessageBox(0,"Error: voxie_init() failed","",MB_OK); return(-1); }

	
	// load in the sound files.
	voxie_mountzip("snd.zip");



	while (!voxie_breath(&in)) // Breath is called once per volume sweep.
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		gTim = tim;

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

#if 1
	if (buttonDelay < mTime) {
	if (voxie_keystat(0x02) == 1 && mx == -1 ) {  
		switch(sampleNo) {
			default:
			case 0:
				// use -1 to play 
				// mono
				mx = voxie_playsound("mx1.flac", -1, mxVol, mxVol,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music

			
				if (mxVol == -1) mxFade = 2;
				break;
			case 1:
				mx = voxie_playsound("blowup2.flac", 0, mxVol, mxVol,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music
				break;
				
			case 2: 
				// stereo
				mx = voxie_playsound("test.wav", 0, mxVol, 0,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music
				mx = voxie_playsound("test.wav", 1, 0, mxVol,1.0); mxLoopTim = mxSampLength + tim; buttonDelay = mTime + 10; testMusic = 1; // 1 start music
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

		if (voxie_keystat(0x4a) == 1) { sampleNo -= 1; buttonDelay = mTime + 10; }
		if (voxie_keystat(0x4e) == 1) { sampleNo += 1; buttonDelay = mTime + 10; }


		if (voxie_keystat(0x0c) == 1 &&  rootNote > 0) { rootNote--; }
		if (voxie_keystat(0x0d) == 1 &&  rootNote < 50) { rootNote++; }


		// sync volume to mxVol
		// staggered volume ot make chords sound a bit nicer
		notesAmp[0] = mxVol/20; 
		notesAmp[1] = mxVol/25;
		notesAmp[2] = mxVol/30;
		notesAmp[3] = mxVol/35;

		// play standard notes -- like a piano 
		if (voxie_keystat(0x1e) ) { // kb a  c
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[1 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x11) ) { // kb w  C#
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[2 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x1f) ) { // kb s D
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[3 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x12) ) { // kb e D#
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[4 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x20) ) { // kb d e
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[5 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x21) ) { // kb f f
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[6 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x14) ) { // kb t f#
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[7 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x22) ) { // kb g g
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[8 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x15) ) { // kb y g#
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[9 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x23) ) { // kb h a
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[10 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x16) ) { // kb u a#
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[11 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x24) ) { // kb j b
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[12 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}
		if (voxie_keystat(0x25) ) { // kb k c
			noteDuration = 0.01 + tim;
			notes[0] = noteArray[13 + rootNote]; // frequency of note
			notes[1] = -1;
			voxie_setaudplaycb(mvmix);
		}


		// play chord
		if (voxie_keystat(0x2c) ) { //Major Root+4semitones+7semitones

		noteDuration = 0.01 + tim;
		notes[0] = noteArray[rootNote]; // frequency of note
		notes[1] = noteArray[rootNote+4]; // frequency of note
		notes[2] = noteArray[rootNote+7]; // frequency of note 
		notes[3] = 0;


		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2d) ) { //Minor Root+3semitones+7semitones

		noteDuration = 0.01 + tim;
	
	
		notes[0] = noteArray[rootNote]; // frequency of note
		notes[1] = noteArray[rootNote+3]; // frequency of note
		notes[2] = noteArray[rootNote+7]; // frequency of note 
		notes[3] = 0; 
	
		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2e) ) { //Diminished  Root+3semitones+6semitones

		noteDuration = 0.01 + tim;
		notes[0] = noteArray[rootNote]; // frequency of note
		notes[1] = noteArray[rootNote+3]; // frequency of note
		notes[2] = noteArray[rootNote+6]; // frequency of note 
		notes[3] = 0; 


		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x2f) ) { // Major 7th Root+4semitones+7semitones+11semitones

		noteDuration = 0.01 + tim;
		notes[0] = (noteArray[rootNote]);
		notes[1] = (noteArray[rootNote+4]);
		notes[2] = (noteArray[rootNote+7]);
		notes[3] = (noteArray[rootNote+11]);

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x30) ) { // Minor 7th Root+3semitones+7semitones+10semitones

		noteDuration = 0.01 + tim;
		notes[0] = (noteArray[rootNote]);
		notes[1] = (noteArray[rootNote+3]);
		notes[2] = (noteArray[rootNote+7]); 
		notes[3] = (noteArray[rootNote+10]); 


		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x31) ) { // Sus 2nd Root+2semitones+7semitones

		noteDuration = 0.01 + tim;
		notes[0] = (noteArray[rootNote]);
		notes[1] = (noteArray[rootNote+2]);
		notes[2] = (noteArray[rootNote+7]); 
		notes[3] = 0;

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x32) ) { //  make random noise

		noteDuration = 0.01 + tim;
		notes[0] = rand() % 10000 + 50; // frequency of note
		notes[1] = rand() % 5000 + 10000; // frequency of note
		notes[2] = 0; // frequency of note
		notes[3] = 0; // frequency of note

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x33) ) { // slide up
		frequency += 50;
		noteDuration = 0.05 + tim;
		notes[0] = C4 + frequency; // main sweep
		notes[1] = C2 + frequency; // main sweep
		notes[2] = -2;
		voxie_setaudplaycb(mvmix);

		}

		if (voxie_keystat(0x34) ) { // slide down
		frequency -= 200;
		noteDuration = 0.01 + tim;
		notes[0] = C6 + frequency; // C5 note plus frequency sweeo...
		notes[1] = 0; // mute channel / play no note
		notes[2] = 0; // mute channel / play no note
		notes[3] = 0; // mute channel / play no note

		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x35) ) { // vibrato
		frequency = cos(tim * 100) * 500;
		noteDuration = 0.10 + tim;
		notes[0] = (noteArray[rootNote]) + frequency; // C5 note plus frequency sweeo...
		notes[1] = -1; // mute rest of channels


		voxie_setaudplaycb(mvmix);
		}

		if (voxie_keystat(0x33) == 0 && voxie_keystat(0x34) == 0) { 
			frequency = mxVol;
		}


	}
	

#else




#endif
		// final update loop for frame
		if (debug == 1) {
		avgdtim += (dtim-avgdtim)*.1;

		voxie_debug_print6x8_(30,75,0xffffff,-1,"This program is all about the sound functions. Use the numeric keys to explore voxie_playsound functions.\nMost of the alpha keys use the custom audio callback function");	
		
		voxie_debug_print6x8_(30,100,0xffc080,-1,"samp = %d VPS %5.1f, t:%d, test Music:%d mxVol:%d mx: %d mxFade: %d, mxLoop: %d ",sampleNo,1.0/avgdtim, mTime, testMusic, mxVol, mx, mxFade, mxLoop);	
		voxie_debug_print6x8_(30,112,0xffc080,-1,"Numpad - / + change sample  1 = Play Music, 2 = Stop Music, 3 = Change Pitch, 4 = Lower Vol, 5 = Incease Vol, 6 Loop, 7 Fade, 8 mute, 9 start timer 0 - stop time ");	

		voxie_debug_print6x8_(30,150,0xff00ff,-1,"Play Paino Notes top two rows of keys... \nA,W,S,D,F,T,G,Y,H,J,K = C C# D D# E F F# G G# A B\n - and + transpose current transpose note = %d", rootNote);	

		voxie_debug_print6x8_(30,200,0x00ffff,-1,"CHORDS / Sound Effects using voxie_setaudplycb Press Z to / (bottom row of keys) to make some SFX using voxie_setaudplaycb() Root Note = %d   ", rootNote);	
		voxie_debug_print6x8_(30,211,0x0030ff,-1,"Z = Major\nX = Minor\nC =  Diminished\nV = Maj 7\nB = Min 7\nN = Sus2\nM = Noise <\n = Slide Up\n> Slide Down\n/ = vibrato", rootNote);	


		voxie_debug_print6x8_(30,330,0xff0000,-1,"vw.playsamprate = %d gnsamps = %d",vw.playsamprate, gnsamps );	
	



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
