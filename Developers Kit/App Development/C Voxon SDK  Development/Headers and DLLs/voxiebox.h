#ifndef VOXIEBOX_LIB_H
#define VOXIEBOX_LIB_H

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define BEFUN (__cdecl*
#define AFFUN )
#define STRUC
#ifndef EXTRN
#define EXTERN
#else
#define EXTERN EXTRN
#endif

#else

#include <X11/Xlib.h>
#include <unistd.h>
#define __int64 long long
#define INT_PTR intptr_t
#define MAX_PATH 260
#define BEFUN
#define AFFUN
#define STRUC struct
#define EXTERN extern

#endif

#include <stdarg.h>
#include <stdio.h>
#pragma pack(push,1)

#ifdef  __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
	//Ken's Picture LIBrary (KPLIB)
enum { ATTRIB_ISDIR=1, ATTRIB_INZIP=2, ATTRIB_RDONLY=4, ATTRIB_HIDDEN=8, ATTRIB_ISDRIVE=16 };
typedef struct { __int64 size; int year; char month, day, dayofweek, hour, minute, second; short attrib; char zipnam[MAX_PATH], filnam[MAX_PATH]; } kzfileinfo_t;

EXTERN int  BEFUN kzaddstack AFFUN (const char *);
EXTERN void BEFUN kzuninit AFFUN (void);
struct kzfind_t;
EXTERN STRUC kzfind_t * BEFUN kzfindfilestart AFFUN (const char *st); //pass wildcard string
EXTERN int BEFUN kzfindfile AFFUN (STRUC kzfind_t *find, kzfileinfo_t *fileinfo); //returns 1:found, 0:~found, NOTE:keep calling until ret 0 else mem leak ;P
	//Ken's ZIP functions:
struct kzfile_t;
EXTERN STRUC kzfile_t * BEFUN kzsetfil   AFFUN (FILE *);
EXTERN STRUC kzfile_t * BEFUN kzopen     AFFUN (const char *);
EXTERN STRUC kzfile_t * BEFUN kzopen_ext AFFUN (const char *, const char *, const char *);
EXTERN unsigned int BEFUN kzread AFFUN (STRUC kzfile_t *, void *, unsigned int);
EXTERN unsigned int BEFUN kzfilelength AFFUN (STRUC kzfile_t *);
EXTERN unsigned int BEFUN kztell AFFUN (STRUC kzfile_t *);
EXTERN int  BEFUN kzseek         AFFUN (STRUC kzfile_t *, int, int);
EXTERN int  BEFUN kzgetc         AFFUN (STRUC kzfile_t *);
EXTERN int  BEFUN kzeof          AFFUN (STRUC kzfile_t *);
EXTERN void BEFUN kzclose        AFFUN (STRUC kzfile_t *);
	//Low-level PNG/JPG functions:
EXTERN int  BEFUN kpgetdim       AFFUN (const char *, int, int *, int *);
EXTERN int  BEFUN kprender       AFFUN (const char *, int, INT_PTR, int, int, int, int, int);
	//High-level (easy) picture loading function:
EXTERN int  BEFUN kpzload        AFFUN (const char *, INT_PTR *, INT_PTR *, INT_PTR *, INT_PTR *);
//--------------------------------------------------------------------------------------------------

	//Window (VOXIEBOX_DLL must create, own, and control it)
typedef struct { float x, y; } point2d;
#ifndef POINT3D_T
#define POINT3D_T
typedef struct { float x, y, z; } point3d;
#endif

#define MAXDISP 3
typedef struct
{
	point2d keyst[8];
	int colo_r, colo_g, colo_b;
	int mono_r, mono_g, mono_b;
	int mirrorx, mirrory;
} voxie_disp_t;
typedef struct
{
	int useemu; float emuhang, emuvang, emudist;                                                                        //Emulation
	int xdim, ydim, projrate, framepervol, usecol, dispnum, bitspervol; voxie_disp_t disp[MAXDISP];                     //Display
	int hwsync_frame0, hwsync_phase, hwsync_amp[4], hwsync_pha[4], hwsync_levthresh, voxie_vol;                         //Actuator
	int ilacemode, drawstroke, dither, smear, usekeystone, flip, menu_on_voxie; float aspx, aspy, aspz, gamma, density; //Render
	int sndfx_vol, voxie_aud, excl_audio, sndfx_aud[2], playsamprate, playnchans, recsamprate, recnchans;               //Audio
	int isrecording, hacks, dispcur, sndfx_nspk, hwdispnum;                                                             //Misc.
	float freq, phase;                                                                                                  //Obsolete

	int thread_override_hack; //0:default thread behavior, 1..n:force n threads for voxie_drawspr()/voxie_drawheimap(); bound to: {1 .. #CPU cores (1 less on hw)}
	int motortyp; //0=DCBrush+A*, 1=CP+FreqIn+A*, 2=BL_Airplane+A*, 3=VSpin1.0+CP+FreqIn, 4=VSpin1.0+BL+A4915, 5=VSpin1.0+BL+WS2408
	int clipshape; //0=rectangle (vw.aspx,vw.aspy), 1=circle (vw.aspr)
	int goalrpm, cpmaxrpm, ianghak, ldotnum, normhax;
	unsigned char upndow; //0=sawtooth, 1=triangle
	unsigned char scrshape; //0=helix, 1=peanut/hurricane
	unsigned char filler[2];
	int nblades; //0=VX1 (not spinner), 1=/|, 2=/|/|, ..
	int usejoy; //-1=none, 0=joyInfoEx, 1=XInput
	int dimcaps;
	float emugam;
	float asprmin;
	float sync_usb_offset;
	int sensemask[3], outcol[3];
	float aspr, sawtoothrat;
} voxie_wind_t;
EXTERN void BEFUN voxie_loadini_int AFFUN (voxie_wind_t *vw);
EXTERN void BEFUN voxie_getvw       AFFUN (voxie_wind_t *vw);
EXTERN int  BEFUN voxie_init        AFFUN (voxie_wind_t *vw);
EXTERN void BEFUN voxie_uninit_int  AFFUN (int);
EXTERN void BEFUN voxie_mountzip    AFFUN (const char *fnam);
EXTERN void BEFUN voxie_free        AFFUN (const char *fnam);
EXTERN __int64 BEFUN voxie_getversion AFFUN (void);

	//Input devices
typedef struct { int bstat, obstat, dmousx, dmousy, dmousz; } voxie_inputs_t;
#if defined(_WIN32)
EXTERN HWND   BEFUN voxie_gethwnd  AFFUN (int disp);
#else
EXTERN __int64 BEFUN voxie_gethwnd  AFFUN (int disp);
#endif
EXTERN int    BEFUN voxie_breath   AFFUN (voxie_inputs_t *);
EXTERN void   BEFUN voxie_quitloop AFFUN (void);
EXTERN double BEFUN voxie_klock    AFFUN (void);
EXTERN int    BEFUN voxie_keystat  AFFUN (int);
EXTERN int    BEFUN voxie_keyread  AFFUN (void);

typedef struct { short but, lt, rt, tx0, ty0, tx1, ty1, hat; } voxie_xbox_t;
EXTERN int    BEFUN voxie_xbox_read  AFFUN (int id, voxie_xbox_t *vx);
EXTERN void   BEFUN voxie_xbox_write AFFUN (int id, float lmot, float rmot);

typedef struct { float dx, dy, dz, ax, ay, az; int but; } voxie_nav_t;
EXTERN int    BEFUN voxie_nav_read AFFUN (int id, voxie_nav_t *nav);

	//Touch controls (lo-level)
EXTERN int    BEFUN voxie_touch_read AFFUN (int *i, int *x, int *y, int *j);

	//Touch controls (hi-level buttons):
typedef struct { const char *st; int x0, y0, xs, ys, fcol, bcol, keycode; } touchkey_t;
EXTERN void BEFUN voxie_touch_custom AFFUN (const touchkey_t *touchkey, int num);

	//Menus:
enum {MENU_TEXT=0, MENU_LINE, MENU_BUTTON, MENU_HSLIDER=MENU_BUTTON+4, MENU_VSLIDER, MENU_EDIT, MENU_EDIT_DO, MENU_TOGGLE, MENU_PICKFILE};
EXTERN void BEFUN voxie_menu_reset      AFFUN (int (*menu_update)(int id, char *st, double val, int how, void *userdata), void *userdata, char *bkfilnam);
EXTERN void BEFUN voxie_menu_addtab     AFFUN (const char *st, int x, int y, int xs, int ys);
EXTERN void BEFUN voxie_menu_additem    AFFUN (const char *st, int x, int y, int xs, int ys, int id, int type, int down, int col, double v, double v0, double v1, double vstp0, double vstp1);
EXTERN void BEFUN voxie_menu_updateitem AFFUN (int id, char *st, int down, double v);

	//Frame (low level - double buffer)
#define VOXIEFRAME_FLAGS_BUFFERED 0 //default (buffers voxie gfx commands internally for potential speedup)
#define VOXIEFRAME_FLAGS_IMMEDIATE 1 //use this if calling voxie gfx from multiple threads (ex: custom user multithread)
#ifndef TILETYPE_T
#define TILETYPE_T
typedef struct { INT_PTR f, p, x, y; } tiletype;
#endif
typedef struct
{
	INT_PTR f, p, fp; int x, y, flags, drawplanes, x0, y0, x1, y1;
	float xmul, ymul, zmul, xadd, yadd, zadd;
	tiletype f2d;
} voxie_frame_t;
enum {VOLCAP_OFF=0, VOLCAP_FRAME_PLY, VOLCAP_FRAME_PNG, VOLCAP_FRAME_REC, VOLCAP_VIDEO_REC, VOLCAP_FRAME_VCB, VOLCAP_VIDEO_VCBZIP};
EXTERN void BEFUN voxie_volcap       AFFUN (const char *filnam, int volcap_mode, int target_vps);
EXTERN void BEFUN voxie_setview      AFFUN (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1);
EXTERN void BEFUN voxie_setmaskplane AFFUN (voxie_frame_t *vf, float x0, float y0, float z0, float nx, float ny, float nz);
EXTERN void BEFUN voxie_setnorm      AFFUN (float nx, float ny, float nz);
EXTERN int  BEFUN voxie_frame_start  AFFUN (voxie_frame_t *vf);
EXTERN void BEFUN voxie_flush        AFFUN (void);
EXTERN void BEFUN voxie_frame_end    AFFUN (void);
EXTERN void BEFUN voxie_setleds      AFFUN (int id, int r, int g, int b);
EXTERN void BEFUN voxie_project      AFFUN (int disp, int dir, float x, float y, int z, float *xo, float *yo);

	//Graphics (medium level)
#define FILLMODE_DOT  0
#define FILLMODE_LINE 1
#define FILLMODE_SURF 2
#define FILLMODE_SOL  3
typedef struct { float x, y, z; int p2; } pol_t;
typedef struct { float x, y, z, u, v; int col; } poltex_t;
typedef struct { float x0, y0, z0, sc, x1, y1, z1, dum; } extents_t;
EXTERN void BEFUN voxie_drawvox     AFFUN (voxie_frame_t *vf, float fx, float fy, float fz, int col);
EXTERN void BEFUN voxie_drawbox     AFFUN (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
EXTERN void BEFUN voxie_drawlin     AFFUN (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
EXTERN void BEFUN voxie_drawpol     AFFUN (voxie_frame_t *vf, pol_t *pt, int n, int col);
EXTERN void BEFUN voxie_drawmeshtex AFFUN (voxie_frame_t *vf, const char *fnam, const poltex_t *vt, int vtn, const int *mesh, int meshn, int flags, int col);
EXTERN void BEFUN voxie_drawsph     AFFUN (voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col);
EXTERN void BEFUN voxie_drawcone    AFFUN (voxie_frame_t *vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
EXTERN int  BEFUN voxie_drawspr_getextents AFFUN (const char *fnam, extents_t *zo, int flags);
EXTERN int  BEFUN voxie_drawspr     AFFUN (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col);
EXTERN int  BEFUN voxie_drawspr_ext AFFUN (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col, float forcescale, float fdrawratio, int flags);

	//Graphics (high level)
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
	int defer_load_posori, gotzip, validfil;
	int forcemip, lockmip; //0=not locked, 1-4 = lock mip 0-3
	int saveasstl; //usually 0, set to +1 or -1 to write current levels to STL, function sets this back to 0 when done.
	int nfiles; //# files in zip
	int n, cnt, nummin, nummax; //<- from anim_t
	char filespec[MAX_PATH];    //<- from anim_t
	float xsc, ysc, zsc;
	STRUC kzfile_t *kzfil;
	__int64 valsumn, valsums, valsumss;
	int valmin, valmax;
	int compmode, ncomp, multiframe, mirrorz;
	int zslice, zslice0, zslicemin, zslicemax, zsizmal;
	int bitmal, bitmsb, bituse, issign;     //placement of data in (assumed&hard-coded) 16-bit pixel value
	float reslop, reinte;                   //silly transformation (usually 1.f,0.f respectively)
	float wincen, winwid;                   //suggested window of 16-bit values for 8-bit display
	int usepal, pal[256];
} voxie_dicom_t;
EXTERN void  BEFUN voxie_printalph     AFFUN (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *st);
EXTERN void  BEFUN voxie_printalph_ext AFFUN (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, float rad, int col, const char *st);
EXTERN void  BEFUN voxie_drawcube      AFFUN (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f, int fillmode, int col);
EXTERN float BEFUN voxie_drawheimap    AFFUN (voxie_frame_t *vf, char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags);
EXTERN void  BEFUN voxie_drawdicom     AFFUN (voxie_frame_t *vf, voxie_dicom_t *vd, const char *gfilnam, point3d *gp, point3d *gr, point3d *gd, point3d *gf, int *animn, int *loaddone);

	//Render stuff to 2D debug window (menu background)
EXTERN void BEFUN voxie_debug_print6x8     AFFUN (int x, int y, int fcol, int bcol, const char *st);
EXTERN void BEFUN voxie_debug_drawpix      AFFUN (int x, int y, int col);
EXTERN void BEFUN voxie_debug_drawhlin     AFFUN (int x0, int x1, int y, int col);
EXTERN void BEFUN voxie_debug_drawline     AFFUN (float x0, float y0, float x1, float y1, int col);
EXTERN void BEFUN voxie_debug_drawcirc     AFFUN (int xc, int yc, int r, int col);
EXTERN void BEFUN voxie_debug_drawrectfill AFFUN (int x0, int y0, int x1, int y1, int col);
EXTERN void BEFUN voxie_debug_drawcircfill AFFUN (int x, int y, int r, int col);
EXTERN void BEFUN voxie_debug_drawtile     AFFUN (tiletype *src, int x0, int y0);

	//Sound
EXTERN int  BEFUN voxie_playsound        AFFUN (const char *fnam, int chan, int volperc0, int volperc1, float frqmul);
EXTERN void BEFUN voxie_playsound_update AFFUN (int handle, int chan, int volperc0, int volperc1, float frqmul);
EXTERN void BEFUN voxie_playsound_seek   AFFUN (int handle, double sec, int seektyp);
EXTERN void BEFUN voxie_setaudplaycb     AFFUN (void (*userplayfunc)(int *samps, int nframes));
EXTERN void BEFUN voxie_setaudreccb      AFFUN (void (*userrecfunc )(int *samps, int nframes));

	//.REC playback
typedef struct
{
	STRUC kzfile_t *kzfil; //Warning: do not use this from user code - for internal use only.
	double timleft;
	float *frametim, playspeed;
	int *frameseek, framemal, kztableoffs, error;
	int playmode, framecur, framenum;
	int currep, animmode/*0=forward, 1=ping-pong, 2=reverse*/;
	int playercontrol[4];
} voxie_rec_t;
EXTERN int   BEFUN voxie_rec_open  AFFUN (voxie_rec_t *vr, voxie_wind_t *vw, char *fnam, char *port, int flags);
EXTERN int   BEFUN voxie_rec_play  AFFUN (voxie_rec_t *vr, voxie_frame_t *vf, int domode);
EXTERN void  BEFUN voxie_rec_close AFFUN (voxie_rec_t *vr);

#ifdef  __cplusplus
}
#endif

//--------------------------------------------------------------------------------------------------

#if defined(EXTRN)
#if defined(_WIN32)
EXTERN HINSTANCE hvoxie;
#endif
EXTERN int voxie_load (voxie_wind_t *vw);
#else
#if defined(_WIN32)
HINSTANCE hvoxie = 0;
#endif
int voxie_load (voxie_wind_t *vw)
{
#if defined(_WIN32)
	hvoxie = LoadLibrary("voxiebox.dll"); if (!hvoxie) return(-1);

	voxie_loadini_int  = (  void (__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie,"voxie_loadini_int");
	voxie_getvw        = (  void (__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie,"voxie_getvw");
	voxie_init         = (   int (__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie,"voxie_init");
	voxie_uninit_int   = (  void (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_uninit_int");
	voxie_mountzip     = (  void (__cdecl *)(const char*   ))GetProcAddress(hvoxie,"voxie_mountzip");
	voxie_free         = (  void (__cdecl *)(const char*   ))GetProcAddress(hvoxie,"voxie_free");
	voxie_getversion   = (__int64 (__cdecl *)(void         ))GetProcAddress(hvoxie,"voxie_getversion");
	voxie_gethwnd      = (  HWND (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_gethwnd");
	voxie_breath       = (   int (__cdecl *)(voxie_inputs_t*))GetProcAddress(hvoxie,"voxie_breath");
	voxie_quitloop     = (  void (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_quitloop");
	voxie_klock        = (double (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_klock");
	voxie_keystat      = (   int (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_keystat");
	voxie_keyread      = (   int (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_keyread");
	voxie_xbox_read    = (   int (__cdecl *)(int, voxie_xbox_t *))GetProcAddress(hvoxie,"voxie_xbox_read");
	voxie_xbox_write   = (  void (__cdecl *)(int, float, float))GetProcAddress(hvoxie,"voxie_xbox_write");
	voxie_nav_read     = (   int (__cdecl *)(int, voxie_nav_t *))GetProcAddress(hvoxie,"voxie_nav_read");
	voxie_touch_read   = (   int (__cdecl *)(int*,int*,int*,int*))GetProcAddress(hvoxie,"voxie_touch_read");
	voxie_menu_reset   = (  void (__cdecl *)(int(*)(int,char*,double,int,void*),void*,char*))GetProcAddress(hvoxie,"voxie_menu_reset");
	voxie_menu_addtab  = (  void (__cdecl *)(const char*,int,int,int,int))GetProcAddress(hvoxie,"voxie_menu_addtab");
	voxie_menu_additem = (  void (__cdecl *)(const char*,int,int,int,int,int,int,int,int,double,double,double,double,double))GetProcAddress(hvoxie,"voxie_menu_additem");
	voxie_menu_updateitem=( void (__cdecl *)(int,char*,int,double))GetProcAddress(hvoxie,"voxie_menu_updateitem");
	voxie_touch_custom = (  void (__cdecl *)(const touchkey_t *,int))GetProcAddress(hvoxie,"voxie_touch_custom");
	voxie_volcap       = (  void (__cdecl *)(const char *,int,int))GetProcAddress(hvoxie,"voxie_volcap");
	voxie_setview      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float))GetProcAddress(hvoxie,"voxie_setview");
	voxie_setmaskplane = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float))GetProcAddress(hvoxie,"voxie_setmaskplane");
	voxie_frame_start  = (   int (__cdecl *)(voxie_frame_t*))GetProcAddress(hvoxie,"voxie_frame_start");
	voxie_flush        = (  void (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_flush");
	voxie_frame_end    = (  void (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_frame_end");
	voxie_setleds      = (  void (__cdecl *)(int,int,int,int))GetProcAddress(hvoxie,"voxie_setleds");
	voxie_project      = (  void (__cdecl *)(int,int,float,float,int,float*,float*))GetProcAddress(hvoxie,"voxie_project");
	voxie_drawvox      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,int))GetProcAddress(hvoxie,"voxie_drawvox");
	voxie_drawbox      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float,int,int))GetProcAddress(hvoxie,"voxie_drawbox");
	voxie_drawlin      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float,int))GetProcAddress(hvoxie,"voxie_drawlin");
	voxie_drawpol      = (  void (__cdecl *)(voxie_frame_t*,pol_t*,int,int))GetProcAddress(hvoxie,"voxie_drawpol");
	voxie_drawmeshtex  = (  void (__cdecl *)(voxie_frame_t*,const char*,const poltex_t*,int,const int*,int,int,int))GetProcAddress(hvoxie,"voxie_drawmeshtex");
	voxie_drawsph      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,int,int))GetProcAddress(hvoxie,"voxie_drawsph");
	voxie_drawcone     = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float,float,float,int,int))GetProcAddress(hvoxie,"voxie_drawcone");
	voxie_drawspr_getextents = (int (__cdecl *)(const char*,extents_t*,int))GetProcAddress(hvoxie,"voxie_drawspr_getextents");
	voxie_drawspr      = (   int (__cdecl *)(voxie_frame_t*,const char*,point3d*,point3d*,point3d*,point3d*,int))GetProcAddress(hvoxie,"voxie_drawspr");
	voxie_drawspr_ext  = (   int (__cdecl *)(voxie_frame_t*,const char*,point3d*,point3d*,point3d*,point3d*,int,float,float,int))GetProcAddress(hvoxie,"voxie_drawspr_ext");
	voxie_printalph    = (  void (__cdecl *)(voxie_frame_t*,point3d*,point3d*,point3d*,int,const char*))GetProcAddress(hvoxie,"voxie_printalph");
	voxie_printalph_ext= (  void (__cdecl *)(voxie_frame_t*,point3d*,point3d*,point3d*,float,int,const char*))GetProcAddress(hvoxie,"voxie_printalph_ext");
	voxie_drawcube     = (  void (__cdecl *)(voxie_frame_t*,point3d*,point3d*,point3d*,point3d*,int,int))GetProcAddress(hvoxie,"voxie_drawcube");
	voxie_drawheimap   = ( float (__cdecl *)(voxie_frame_t*,char*,point3d*,point3d*,point3d*,point3d*,int,int,int))GetProcAddress(hvoxie,"voxie_drawheimap");
	voxie_drawdicom    = (  void (__cdecl *)(voxie_frame_t*,voxie_dicom_t*,const char *,point3d*,point3d*,point3d*,point3d*,int*,int*))GetProcAddress(hvoxie,"voxie_drawdicom");
	voxie_debug_print6x8     = (void (__cdecl *)(int x, int y, int fcol, int bcol, const char *st))      GetProcAddress(hvoxie,"voxie_debug_print6x8");
	voxie_debug_drawpix      = (void (__cdecl *)(int x, int y, int col))                                 GetProcAddress(hvoxie,"voxie_debug_drawpix");
	voxie_debug_drawhlin     = (void (__cdecl *)(int x0, int x1, int y, int col))                        GetProcAddress(hvoxie,"voxie_debug_drawhlin");
	voxie_debug_drawline     = (void (__cdecl *)(float x0, float y0, float x1, float y1, int col))       GetProcAddress(hvoxie,"voxie_debug_drawline");
	voxie_debug_drawcirc     = (void (__cdecl *)(int xc, int yc, int r, int col))                        GetProcAddress(hvoxie,"voxie_debug_drawcirc");
	voxie_debug_drawrectfill = (void (__cdecl *)(int x0, int y0, int x1, int y1, int col))               GetProcAddress(hvoxie,"voxie_debug_drawrectfill");
	voxie_debug_drawcircfill = (void (__cdecl *)(int x, int y, int r, int col))                          GetProcAddress(hvoxie,"voxie_debug_drawcircfill");
	voxie_debug_drawtile     = (void (__cdecl *)(tiletype *src, int x0, int y0))                         GetProcAddress(hvoxie,"voxie_debug_drawtile");
	voxie_playsound        = ( int (__cdecl *)(const char*,int,int,int,float))GetProcAddress(hvoxie,"voxie_playsound");
	voxie_playsound_update = (void (__cdecl *)(int,int,int,int,float))        GetProcAddress(hvoxie,"voxie_playsound_update");
	voxie_playsound_seek   = (void (__cdecl *)(int,double,int))               GetProcAddress(hvoxie,"voxie_playsound_seek");
	voxie_setaudplaycb = (  void (__cdecl *)(void(*userplayfunc)(int*,int)))GetProcAddress(hvoxie,"voxie_setaudplaycb");
	voxie_setaudreccb  = (  void (__cdecl *)(void(*userrecfunc)(int*,int)))GetProcAddress(hvoxie,"voxie_setaudreccb");
	voxie_rec_open     = (   int (__cdecl *)(voxie_rec_t*,voxie_wind_t*,char*,char*,int))GetProcAddress(hvoxie,"voxie_rec_open");
	voxie_rec_play     = (   int (__cdecl *)(voxie_rec_t*,voxie_frame_t*,int))GetProcAddress(hvoxie,"voxie_rec_play");
	voxie_rec_close    = (  void (__cdecl *)(voxie_rec_t*))                GetProcAddress(hvoxie,"voxie_rec_close");
	kzaddstack         = (   int (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzaddstack");
	kzuninit           = (  void (__cdecl *)(void))       GetProcAddress(hvoxie,"kzuninit");
	kzfindfilestart    = (kzfind_t * (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzfindfilestart");
	kzfindfile         = (   int (__cdecl *)(kzfind_t*,kzfileinfo_t*))GetProcAddress(hvoxie,"kzfindfile");
	kzsetfil           = (kzfile_t * (__cdecl *)(FILE*))GetProcAddress(hvoxie,"kzsetfil");
	kzopen             = (kzfile_t * (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzopen");
	kzopen_ext         = (kzfile_t * (__cdecl *)(const char*,const char*,const char*))GetProcAddress(hvoxie,"kzopen_ext");
	kzread             = (unsigned int (__cdecl *)(kzfile_t*,void*,unsigned int))GetProcAddress(hvoxie,"kzread");
	kzfilelength       = (unsigned int (__cdecl *)(kzfile_t*))GetProcAddress(hvoxie,"kzfilelength");
	kztell             = (unsigned int (__cdecl *)(kzfile_t*))GetProcAddress(hvoxie,"kztell");
	kzseek             = (   int (__cdecl *)(kzfile_t*,int,int))GetProcAddress(hvoxie,"kzseek");
	kzgetc             = (   int (__cdecl *)(kzfile_t*))GetProcAddress(hvoxie,"kzgetc");
	kzeof              = (   int (__cdecl *)(kzfile_t*))GetProcAddress(hvoxie,"kzeof");
	kzclose            = (  void (__cdecl *)(kzfile_t*))GetProcAddress(hvoxie,"kzclose");
	kpgetdim           = (   int (__cdecl *)(const char*,int,int*,int*))                  GetProcAddress(hvoxie,"kpgetdim");
	kprender           = (   int (__cdecl *)(const char*,int,INT_PTR,int,int,int,int,int))GetProcAddress(hvoxie,"kprender");
	kpzload            = (   int (__cdecl *)(const char*,INT_PTR*,INT_PTR*,INT_PTR*,INT_PTR*)) GetProcAddress(hvoxie,"kpzload");
#endif

	voxie_loadini_int(vw);
	return(0);
}
#endif

#if defined(EXTRN)
EXTERN void voxie_uninit (int mode);
#else
void voxie_uninit (int mode)
{
	voxie_uninit_int(mode);
#if defined(_WIN32)
	if (!mode) { if (hvoxie) { FreeLibrary(hvoxie); hvoxie = 0; } }
#endif
}
#endif

	//Extension for C/C++ allowing use of this function printf-style
#if defined(EXTRN)
EXTERN void voxie_printalph_ (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *fmt, ...);
#else
void voxie_printalph_ (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist,fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
#else
	if (vsprintf((char *)&st,fmt,arglist)) st[sizeof(st)-1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_printalph(vf,p,r,d,col,st);
}
#endif

	//Extension for C/C++ allowing use of this function printf-style
#if defined(EXTRN)
EXTERN void voxie_printalph_ext_ (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, float rad, int col, const char *fmt, ...);
#else
void voxie_printalph_ext_ (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, float rad, int col, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist,fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
#else
	if (vsprintf((char *)&st,fmt,arglist)) st[sizeof(st)-1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_printalph_ext(vf,p,r,d,rad,col,st);
}
#endif

	//Extension for C/C++ allowing use of this function printf-style
#if defined(EXTRN)
EXTERN void voxie_debug_print6x8_ (int x, int y, int fcol, int bcol, const char *fmt, ...);
#else
void voxie_debug_print6x8_ (int x, int y, int fcol, int bcol, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist,fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
#else
	if (vsprintf((char *)&st,fmt,arglist)) st[sizeof(st)-1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_debug_print6x8(x,y,fcol,bcol,st);
}
#endif
//--------------------------------------------------------------------------------------------------

#pragma pack(pop)
#endif
