#ifndef VOXIEBOX_LIB_H
#define VOXIEBOX_LIB_H

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define INT_PTR int
#endif
#include <stdarg.h>
#include <stdio.h>
#pragma pack(push,1)

//--------------------------------------------------------------------------------------------------

	//Window (VOXIEBOX_DLL must create, own, and control it)
typedef struct { float x, y; } point2d;
typedef struct { float x, y, z; } point3d;

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
#if defined(_WIN32)
void (__cdecl *voxie_loadini_int)(voxie_wind_t *vw);
void (__cdecl *voxie_getvw      )(voxie_wind_t *vw);
int  (__cdecl *voxie_init       )(voxie_wind_t *vw);
void (__cdecl *voxie_uninit_int )(int);
void (__cdecl *voxie_mountzip    )(char *fnam);
void (__cdecl *voxie_free        )(char *fnam);
__int64 (__cdecl *voxie_getversion )(void);
#else
extern void voxie_loadini_int(voxie_wind_t *vw);
extern void voxie_getvw      (voxie_wind_t *vw);
extern int  voxie_init       (voxie_wind_t *vw);
extern void voxie_uninit_int (int);
extern void voxie_mountzip   (char *fnam);
extern void voxie_free       (char *fnam);
extern __int64 *voxie_getversion)(void);
#endif

	//Input devices
typedef struct { int bstat, obstat, dmousx, dmousy, dmousz; } voxie_inputs_t;
#if defined(_WIN32)
HWND   (__cdecl *voxie_gethwnd )(int disp);
int    (__cdecl *voxie_breath  )(voxie_inputs_t *);
void   (__cdecl *voxie_quitloop)(void);
double (__cdecl *voxie_klock   )(void);
int    (__cdecl *voxie_keystat )(int);
int    (__cdecl *voxie_keyread )(void);
#else
extern int    voxie_breath  (voxie_inputs_t *);
extern void   voxie_quitloop(void);
extern double voxie_klock   (void);
extern int    voxie_keystat (int);
extern int    voxie_keyread (void);
#endif

typedef struct { short but, lt, rt, tx0, ty0, tx1, ty1, hat; } voxie_xbox_t;
#if defined(_WIN32)
int    (__cdecl *voxie_xbox_read )(int id, voxie_xbox_t *vx);
void   (__cdecl *voxie_xbox_write)(int id, float lmot, float rmot);
#else
extern int    voxie_xbox_read (int id, voxie_xbox_t *vx);
extern void   voxie_xbox_write(int id, float lmot, float rmot);
#endif

typedef struct { float dx, dy, dz, ax, ay, az; int but; } voxie_nav_t;
#if defined(_WIN32)
int    (__cdecl *voxie_nav_read )(int id, voxie_nav_t *nav);
#else
extern int    voxie_nav_read (int id, voxie_nav_t *nav);
#endif

typedef struct { point3d pt, vec; int navx, navy, but; } voxie_laser_t;
#if defined(_WIN32)
int    (__cdecl *voxie_laser_read)(int id, voxie_laser_t *las);
#else
extern int    voxie_laser_read (int id, voxie_laser_t *las);
#endif


	//Menus:
#if defined(_WIN32)
enum {MENU_TEXT=0, MENU_LINE, MENU_BUTTON, MENU_HSLIDER=MENU_BUTTON+4, MENU_VSLIDER, MENU_EDIT, MENU_EDIT_DO, MENU_TOGGLE, MENU_PICKFILE};
void (__cdecl *voxie_menu_reset)(int (*menu_update)(int id, char *st, double val, int how, void *userdata), void *userdata, char *bkfilnam);
void (__cdecl *voxie_menu_addtab)(char *st, int x, int y, int xs, int ys);
void (__cdecl *voxie_menu_additem)(char *st, int x, int y, int xs, int ys, int id, int type, int down, int col, double v, double v0, double v1, double vstp0, double vstp1);
void (__cdecl *voxie_menu_updateitem)(int id, char *st, int down, double v);
#else
TODO..
#endif

	//Touch controls:
typedef struct { char *st; int x0, y0, xs, ys, fcol, bcol, keycode; } touchkey_t;
#if defined(_WIN32)
void (__cdecl *voxie_touch_custom)(const touchkey_t *touchkey, int num);
#else
TODO..
#endif

	//Frame (low level - double buffer)
#define VOXIEFRAME_FLAGS_BUFFERED 0 //default (buffers voxie gfx commands internally for potential speedup)
#define VOXIEFRAME_FLAGS_IMMEDIATE 1 //use this if calling voxie gfx from multiple threads (ex: custom user multithread)
typedef struct { INT_PTR f, p, x, y; } tiletype;
typedef struct
{
	INT_PTR f, p, fp; int x, y, flags, drawplanes, x0, y0, x1, y1;
	float xmul, ymul, zmul, xadd, yadd, zadd;
	tiletype f2d;
} voxie_frame_t;
#if defined(_WIN32)
void (__cdecl *voxie_doscreencap )(int);
void (__cdecl *voxie_setview     )(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1);
void (__cdecl *voxie_setmaskplane)(voxie_frame_t *vf, float x0, float y0, float z0, float nx, float ny, float nz);
int  (__cdecl *voxie_frame_start )(voxie_frame_t *vf);
void (__cdecl *voxie_flush       )(void);
void (__cdecl *voxie_frame_end   )(void);
void (__cdecl *voxie_setleds     )(int id, int r, int g, int b);
void (__cdecl *voxie_project     )(int disp, int dir, float x, float y, int z, float *xo, float *yo);
#else
extern void voxie_doscreencap (int);
extern void voxie_setview     (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1);
extern void voxie_setmaskplane(voxie_frame_t *vf, float x0, float y0, float z0, float nx, float ny, float nz);
extern int  voxie_frame_start (voxie_frame_t *vf);
extern void voxie_flush       (void);
extern void voxie_frame_end   (void);
extern void voxie_setleds     (int id, int r, int g, int b);
extern void voxie_project     (int disp, int dir, float x, float y, int z, float *xo, float *yo);
#endif

	//Graphics (medium level)
#define FILLMODE_DOT  0
#define FILLMODE_LINE 1
#define FILLMODE_SURF 2
#define FILLMODE_SOL  3
typedef struct { float x, y, z; int p2; } pol_t;
typedef struct { float x, y, z, u, v; int col; } poltex_t;
#if defined(_WIN32)
void (__cdecl *voxie_drawvox    )(voxie_frame_t *vf, float fx, float fy, float fz, int col);
void (__cdecl *voxie_drawbox    )(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
void (__cdecl *voxie_drawlin    )(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
void (__cdecl *voxie_drawpol    )(voxie_frame_t *vf, pol_t *pt, int n, int col);
void (__cdecl *voxie_drawmeshtex)(voxie_frame_t *vf, char *fnam, poltex_t *vt, int vtn, int *mesh, int meshn, int flags, int col);
void (__cdecl *voxie_drawsph    )(voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col);
void (__cdecl *voxie_drawcone   )(voxie_frame_t *vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
int  (__cdecl *voxie_drawspr    )(voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col);
int  (__cdecl *voxie_drawspr_ext)(voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col, float forcescale, float fdrawratio, int flags);
#else
extern void voxie_drawvox    (voxie_frame_t *vf, float fx, float fy, float fz, int col);
extern void voxie_drawbox    (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
extern void voxie_drawlin    (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
extern void voxie_drawpol    (voxie_frame_t *vf, pol_t *pt, int n, int col);
extern void voxie_drawmeshtex(voxie_frame_t *vf, char *fnam, poltex_t *vt, int vtn, int *mesh, int meshn, int flags, int col);
extern void voxie_drawsph    (voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col);
extern void voxie_drawcone   (voxie_frame_t *vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
extern int  voxie_drawspr    (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col);
extern int  voxie_drawspr_ext(voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col, float forcescale, float fdrawratio, int flags);
#endif

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
	int defer_load_posori, gotzip;
	int forcemip, lockmip; //0=not locked, 1-4 = lock mip 0-3
	int saveasstl; //usually 0, set to +1 or -1 to write current levels to STL, function sets this back to 0 when done.
	int nfiles; //# files in zip
	int n, cnt, nummin, nummax; //<- from anim_t
	char filespec[MAX_PATH];    //<- from anim_t
	float xsc, ysc, zsc;
} voxie_dicom_t;
#if defined(_WIN32)
void  (__cdecl *voxie_printalph )(voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *st);
void  (__cdecl *voxie_drawcube  )(voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f, int fillmode, int col);
float (__cdecl *voxie_drawheimap)(voxie_frame_t *vf, char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags);
void  (__cdecl *voxie_drawdicom )(voxie_frame_t *vf, voxie_dicom_t *vd, const char *gfilnam, point3d *gp, point3d *gr, point3d *gd, point3d *gf, int *animn, int *loaddone);
#else
extern void  voxie_printalph (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *st);
extern void  voxie_drawcube  (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f, int fillmode, int col);
extern float voxie_drawheimap(voxie_frame_t *vf, char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags);
extern void  voxie_drawdicom (voxie_frame_t *vf, voxie_dicom_t *vd, const char *gfilnam, point3d *gp, point3d *gr, point3d *gd, point3d *gf, int *animn, int *loaddone);
#endif

	//Render stuff to 2D debug window (menu background)
#if defined(_WIN32)
void (__cdecl *voxie_debug_print6x8    )(int x, int y, int fcol, int bcol, const char *st);
void (__cdecl *voxie_debug_drawpix     )(int x, int y, int col);
void (__cdecl *voxie_debug_drawhlin    )(int x0, int x1, int y, int col);
void (__cdecl *voxie_debug_drawline    )(float x0, float y0, float x1, float y1, int col);
void (__cdecl *voxie_debug_drawcirc    )(int xc, int yc, int r, int col);
void (__cdecl *voxie_debug_drawrectfill)(int x0, int y0, int x1, int y1, int col);
void (__cdecl *voxie_debug_drawcircfill)(int x, int y, int r, int col);
#endif

	//Sound
#if defined(_WIN32)
int  (__cdecl *voxie_playsound)(const char *fnam, int chan, int volperc0, int volperc1, float frqmul);
void (__cdecl *voxie_playsound_update)(int handle, int chan, int volperc0, int volperc1, float frqmul);
void (__cdecl *voxie_setaudplaycb)(void (*userplayfunc)(int *samps, int nframes));
void (__cdecl *voxie_setaudreccb )(void (*userrecfunc )(int *samps, int nframes));
#else
extern int  voxie_playsound(const char *fnam, int chan, int volperc0, int volperc1, float frqmul);
extern void voxie_playsound_update (int handle, int chan, int volperc0, int volperc1, float frqmul);
extern void voxie_setaudplaycb(void (*userplayfunc)(int *samps, int nframes));
extern void voxie_setaudreccb (void (*userrecfunc )(int *samps, int nframes));
#endif

	//.REC playback
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
#if defined(_WIN32)
int   (__cdecl *voxie_rec_open)(voxie_rec_t *vr, char *fnam, char *port, int flags);
int   (__cdecl *voxie_rec_play)(voxie_rec_t *vr, voxie_frame_t *vf, int domode);
void  (__cdecl *voxie_rec_close)(voxie_rec_t *vr);
#else
TODO..
#endif

enum { ATTRIB_ISDIR=1, ATTRIB_INZIP=2, ATTRIB_RDONLY=4, ATTRIB_HIDDEN=8 };
typedef struct { __int64 size; int year; char month, day, dayofweek, hour, minute, second; short attrib; char name[MAX_PATH]; } kzfileinfo_t;
#if defined(_WIN32)
	//High-level (easy) picture loading function:
int  (__cdecl *kpzload        )(const char *, INT_PTR *, INT_PTR *, INT_PTR *, INT_PTR *);
	//Low-level PNG/JPG functions:
int  (__cdecl *kpgetdim       )(const char *, int, int *, int *);
int  (__cdecl *kprender       )(const char *, int, INT_PTR, int, int, int, int, int);
	//Ken's ZIP functions:
int  (__cdecl *kzaddstack     )(const char *);
void (__cdecl *kzuninit       )(void);
struct kzfind_t;
kzfind_t *(__cdecl *kzfindfilestart)(const char *st); //pass wildcard string
int (__cdecl *kzfindfile)(kzfind_t *find, kzfileinfo_t *fileinfo); //returns 1:found, 0:~found, NOTE:keep calling until ret 0 else mem leak ;P
struct kzfile_t;
void *(__cdecl *kzsetfil       )(FILE *);
kzfile_t *(__cdecl *kzopen      )(const char *);
unsigned int (__cdecl *kzread )(kzfile_t *, void *, unsigned int);
unsigned int (__cdecl *kzfilelength)(kzfile_t *);
int  (__cdecl *kzseek         )(kzfile_t *, int, int);
unsigned int (__cdecl *kztell )(kzfile_t *);
int  (__cdecl *kzgetc         )(kzfile_t *);
int  (__cdecl *kzeof          )(kzfile_t *);
void (__cdecl *kzclose        )(kzfile_t *);
#else
	//High-level (easy) picture loading function:
extern int  kpzload        (const char *, INT_PTR *, INT_PTR *, INT_PTR *, INT_PTR *);
	//Low-level PNG/JPG functions:
extern int  kpgetdim       (const char *, int, int *, int *);
extern int  kprender       (const char *, int, INT_PTR, int, int, int, int, int);
	//Ken's ZIP functions:
extern int  kzaddstack     (const char *);
extern void kzuninit       (void);
struct kzfind_t;
extern kzfind_t *(__cdecl *kzfindfilestart)(const char *st); //pass wildcard string
extern int (__cdecl *kzfindfile)(kzfind_t *find, kzfileinfo_t *fileinfo); //returns 1:found, 0:~found, NOTE:keep calling until ret 0 else mem leak ;P
struct kzfile_t;
extern kzfile_t *kzsetfil      (FILE *);
extern kzfile_t *kzopen        (const char *);
extern unsigned int kzread (kzfile_t *, void *, unsigned int);
extern unsigned int kzfilelength (kzfile_t *);
extern int  kzseek         (kzfile_t *, int, int);
extern unsigned int kztell (kzfile_t *);
extern int  kzgetc         (kzfile_t *);
extern int  kzeof          (kzfile_t *);
extern void kzclose        (kzfile_t *);
#endif


//--------------------------------------------------------------------------------------------------

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
	voxie_mountzip     = (  void (__cdecl *)(char*         ))GetProcAddress(hvoxie,"voxie_mountzip");
	voxie_free         = (  void (__cdecl *)(char*         ))GetProcAddress(hvoxie,"voxie_free");
	voxie_getversion   = (__int64 (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_getversion");
	voxie_gethwnd      = (  HWND (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_gethwnd");
	voxie_breath       = (   int (__cdecl *)(voxie_inputs_t*))GetProcAddress(hvoxie,"voxie_breath");
	voxie_quitloop     = (  void (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_quitloop");
	voxie_klock        = (double (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_klock");
	voxie_keystat      = (   int (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_keystat");
	voxie_keyread      = (   int (__cdecl *)(void          ))GetProcAddress(hvoxie,"voxie_keyread");
	voxie_xbox_read    = (   int (__cdecl *)(int, voxie_xbox_t *))GetProcAddress(hvoxie,"voxie_xbox_read");
	voxie_xbox_write   = (  void (__cdecl *)(int, float, float))GetProcAddress(hvoxie,"voxie_xbox_write");
	voxie_nav_read     = (   int (__cdecl *)(int, voxie_nav_t *))GetProcAddress(hvoxie,"voxie_nav_read");
	voxie_laser_read   = (   int (__cdecl *)(int, voxie_laser_t *))GetProcAddress(hvoxie,"voxie_laser_read");
	voxie_menu_reset   = (  void (__cdecl *)(int(*)(int,char*,double,int,void*),void*,char*))GetProcAddress(hvoxie,"voxie_menu_reset");
	voxie_menu_addtab  = (  void (__cdecl *)(char*,int,int,int,int))GetProcAddress(hvoxie,"voxie_menu_addtab");
	voxie_menu_additem = (  void (__cdecl *)(char*,int,int,int,int,int,int,int,int,double,double,double,double,double))GetProcAddress(hvoxie,"voxie_menu_additem");
	voxie_menu_updateitem=( void (__cdecl *)(int,char*,int,double))GetProcAddress(hvoxie,"voxie_menu_updateitem");
	voxie_touch_custom = (  void (__cdecl *)(const touchkey_t *,int))GetProcAddress(hvoxie,"voxie_touch_custom");
	voxie_doscreencap  = (  void (__cdecl *)(int           ))GetProcAddress(hvoxie,"voxie_doscreencap");
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
	voxie_drawmeshtex  = (  void (__cdecl *)(voxie_frame_t*,char*,poltex_t*,int,int*,int,int,int))GetProcAddress(hvoxie,"voxie_drawmeshtex");
	voxie_drawsph      = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,int,int))GetProcAddress(hvoxie,"voxie_drawsph");
	voxie_drawcone     = (  void (__cdecl *)(voxie_frame_t*,float,float,float,float,float,float,float,float,int,int))GetProcAddress(hvoxie,"voxie_drawcone");
	voxie_drawspr      = (   int (__cdecl *)(voxie_frame_t*,const char*,point3d*,point3d*,point3d*,point3d*,int))GetProcAddress(hvoxie,"voxie_drawspr");
	voxie_drawspr_ext  = (   int (__cdecl *)(voxie_frame_t*,const char*,point3d*,point3d*,point3d*,point3d*,int,float,float,int))GetProcAddress(hvoxie,"voxie_drawspr_ext");
	voxie_printalph    = (  void (__cdecl *)(voxie_frame_t*,point3d*,point3d*,point3d*,int,const char*))GetProcAddress(hvoxie,"voxie_printalph");
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
	voxie_playsound    = (   int (__cdecl *)(const char*,int,int,int,float))GetProcAddress(hvoxie,"voxie_playsound");
	voxie_playsound_update = (void (__cdecl *)(int,int,int,int,float))GetProcAddress(hvoxie,"voxie_playsound_update");
	voxie_setaudplaycb = (  void (__cdecl *)(void(*userplayfunc)(int*,int)))GetProcAddress(hvoxie,"voxie_setaudplaycb");
	voxie_setaudreccb  = (  void (__cdecl *)(void(*userrecfunc)(int*,int)))GetProcAddress(hvoxie,"voxie_setaudreccb");
	voxie_rec_open     = (   int (__cdecl *)(voxie_rec_t*,char*,char*,int))GetProcAddress(hvoxie,"voxie_rec_open");
	voxie_rec_play     = (   int (__cdecl *)(voxie_rec_t*,voxie_frame_t*,int))GetProcAddress(hvoxie,"voxie_rec_play");
	voxie_rec_close    = (  void (__cdecl *)(voxie_rec_t*))                GetProcAddress(hvoxie,"voxie_rec_close");
	kpzload            = (   int (__cdecl *)(const char*,INT_PTR*,INT_PTR*,INT_PTR*,INT_PTR*)) GetProcAddress(hvoxie,"kpzload");
	kpgetdim           = (   int (__cdecl *)(const char*,int,int*,int*))                  GetProcAddress(hvoxie,"kpgetdim");
	kprender           = (   int (__cdecl *)(const char*,int,INT_PTR,int,int,int,int,int))GetProcAddress(hvoxie,"kprender");
	kzaddstack         = (   int (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzaddstack");
	kzuninit           = (  void (__cdecl *)(void))       GetProcAddress(hvoxie,"kzuninit");
	kzsetfil           = (void * (__cdecl *)(FILE*))      GetProcAddress(hvoxie,"kzsetfil");
	kzopen             = (kzfile_t * (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzopen");
	kzfindfilestart    = (kzfind_t * (__cdecl *)(const char*))GetProcAddress(hvoxie,"kzfindfilestart");
	kzfindfile         = (   int (__cdecl *)(kzfind_t*,kzfileinfo_t*))GetProcAddress(hvoxie,"kzfindfile");
	kzread             = (unsigned int (__cdecl *)(kzfile_t*,void*,unsigned int))GetProcAddress(hvoxie,"kzread");
	kzfilelength       = (unsigned int (__cdecl *)(kzfile_t*))  GetProcAddress(hvoxie,"kzfilelength");
	kzseek             = (   int (__cdecl *)(kzfile_t*,int,int))GetProcAddress(hvoxie,"kzseek");
	kztell             = (unsigned int (__cdecl *)(kzfile_t*)) GetProcAddress(hvoxie,"kztell");
	kzgetc             = (   int (__cdecl *)(kzfile_t*))       GetProcAddress(hvoxie,"kzgetc");
	kzeof              = (   int (__cdecl *)(kzfile_t*))       GetProcAddress(hvoxie,"kzeof");
	kzclose            = (  void (__cdecl *)(kzfile_t*))       GetProcAddress(hvoxie,"kzclose");
#endif

	voxie_loadini_int(vw);
	return(0);
}

void voxie_uninit (int mode)
{
	voxie_uninit_int(mode);
#if defined(_WIN32)
	if (!mode) { if (hvoxie) { FreeLibrary(hvoxie); hvoxie = 0; } }
#endif
}

	//Extension for C/C++ allowing use of this function printf-style
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

	//Extension for C/C++ allowing use of this function printf-style
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

//--------------------------------------------------------------------------------------------------

#pragma pack(pop)
#endif
