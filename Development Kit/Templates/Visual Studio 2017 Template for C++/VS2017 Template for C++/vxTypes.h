#pragma once
/*
  Voxie Types Header contains definitions of all Voxon related types.


*/


#ifndef VX_TYPES
#define VX_TYPES

#include <windows.h>
#include <memory>

typedef struct { float x, y; } point2d;
typedef struct { float x, y, z; } point3d;
typedef struct { float x, y, z; int p2; } pol_t;
typedef struct { float x, y, z, u, v; int col; } poltex_t;
typedef struct { INT_PTR f, p, x, y; } tiletype;

#define MAXDISP 3
#define VOXIE_DICOM_MIPS 4 //NOTE:limited to 6 by dicom_gotz

/**
 * Voxie Display Struct - holds settings for each projector 
 */
typedef struct
{
	point2d keyst[8];
	int colo_r, colo_g, colo_b;
	int mono_r, mono_g, mono_b;
	int mirrorx, mirrory;
} voxie_disp_t;

/**
 * Voxie Window Struct - create
 */
typedef struct
{
	int useemu; float emuhang, emuvang, emudist;                                                                        //Emulation
	int xdim, ydim, projrate, framepervol, usecol, dispnum, bitspervol; voxie_disp_t disp[MAXDISP];                     //Display
	int hwsync_frame0, hwsync_phase, hwsync_amp[4], hwsync_pha[4], hwsync_levthresh, voxie_vol;                         //Actuator
	int ilacemode, drawstroke, dither, smear, usekeystone, flip, menu_on_voxie; float aspx, aspy, aspz, gamma, density; //Render
	int sndfx_vol, voxie_aud, excl_audio, sndfx_aud[2], playsamprate, playnchans, recsamprate, recnchans;               //Audio
	int isrecording, hacks, dispcur;                                                                                    //Misc.
	double freq, phase;                                                                                                 //Obsolete

	int thread_override_hack; //0:default thread behavior, 1..n:force n threads for voxie_drawspr()/voxie_drawheimap(); bound to: {1 .. #CPU cores (1 less on hw)}
	int motortyp; //0=DCBrush+A*, 1=CP+FreqIn+A*, 2=BL_Airplane+A*, 3=VSpin1.0+CP+FreqIn, 4=VSpin1.0+BL+A4915, 5=VSpin1.0+BL+WS2408
	int clipshape; //0=rectangle (vw.aspx,vw.aspy), 1=circle (vw.aspr)
	int goalrpm, cpmaxrpm, ianghak, ldotnum, reserved0;
	int upndow; //0=sawtooth, 1=triangle
	int nblades; //0=VX1 (not spinner), 1=/|, 2=/|/|, ..
	int usejoy; //-1=none, 0=joyInfoEx, 1=XInput
	int dimcaps;
	float emugam;
	float asprmin;
	float sync_usb_offset;
	int sensemask[3], outcol[3];
	float aspr, sawtoothrat;
} voxie_wind_t;



typedef struct
{
	INT_PTR f, p, fp; int x, y, flags, drawplanes, x0, y0, x1, y1;
	float xmul, ymul, zmul, xadd, yadd, zadd;
	tiletype f2d;
} voxie_frame_t;

	//Input devices
typedef struct { int bstat, obstat, dmousx, dmousy, dmousz; } voxie_inputs_t;
typedef struct { short but, lt, rt, tx0, ty0, tx1, ty1, hat; } voxie_xbox_t;
typedef struct { float dx, dy, dz, ax, ay, az; int but; } voxie_nav_t;
typedef struct { point3d pt, vec; int navx, navy, but; } voxie_laser_t;
typedef struct { char *st; int x0, y0, xs, ys, fcol, bcol, keycode; } touchkey_t;

#define VOXIE_DICOM_MIPS 4 //NOTE:limited to 6 by voxie_dicom_t:gotz bit mask trick
typedef struct
{
	signed short *mip[VOXIE_DICOM_MIPS]; //3D grid for each mip level; x lsb .. z msb
	int *gotz;                       //bit array holding which z slices have loaded so far
	point3d rulerpos[3];
	point3d slicep, slicer, sliced, slicef;
	float reserved, timsincelastmove, detail, level[2];
	int xsiz, ysiz, zsiz;            //Dimensions
	int zloaded;                     //number of Z slices loaded so far
	int color[2], autodetail, usedmip, slicemode, drawstats, ruler, flags;
	int defer_load_posori, gotzip;
	int forcemip, lockmip; //0=not locked, 1-4 = lock mip 0-3
	int saveasstl; //usually 0, set to +1 or -1 to write current levels to STL, function sets this back to 0 when done.
	int nfiles; //# files in zip
	int n, cnt, nummin, nummax; //<- from anim_t
	char filespec[MAX_PATH];    //<- from anim_t
	float xsc, ysc, zsc;
} voxie_dicom_t;

typedef struct
{
	FILE *fil; //Warning: do not use this from user code - for internal use only.
	double timleft;
	float *frametim;
	int *frameseek, framemal, kztableoffs, error;
	int playmode, framecur, framenum;
	int currep, animmode/*0=forward, 1=ping-pong, 2=reverse*/;
	int playercontrol[4];
} voxie_rec_t;








#endif