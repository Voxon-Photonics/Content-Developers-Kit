#if 0
voxiedemo.exe: voxiedemo.obj
	 link       voxiedemo.obj user32.lib
	 del voxiedemo.obj
voxiedemo.obj: voxiedemo.c voxiebox.h; cl /c /TP voxiedemo.c /GFy /MT /EHsc
!if 0
#endif

	// /Ox too slow!

#define USELEAP 0
#define USEMAG6D 0

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <intrin.h>
#include "voxiebox.h"
#if (USEMAG6D)
#define MAG6D_MAIN
#include "mag6dll.h"
static int gusemag6d;
#endif
#define PI 3.14159265358979323

static voxie_wind_t vw;
static voxie_frame_t vf;

static int gxres, gyres;

enum
{
	RENDMODE_TITLE=0,  RENDMODE_PHASESYNC,  RENDMODE_KEYSTONECAL,   RENDMODE_LOCALCAL,
	RENDMODE_HEIGHTMAP,RENDMODE_VOXIEPLAYER,RENDMODE_PLATONICSOLIDS,RENDMODE_FLYINGSTUFF,
	RENDMODE_CHESS,    RENDMODE_PACKER,     RENDMODE_PARATROOPER,   RENDMODE_DOTMUNCH,
	RENDMODE_SNAKETRON,RENDMODE_FLYSTOMP,   RENDMODE_MODELANIM,
	RENDMODE_END
};
static const char *iconnam[] =
{
	"title.kv6", "mode0.kv6", "mode1.kv6", "mode2.kv6",
	"mode3.kv6", "mode4.kv6", "mode5.kv6", "mode6.kv6",
	"mode7.kv6", "mode8.kv6", "mode9.kv6", "mode10.kv6",
	"mode11.kv6","mode12.kv6","mode13.kv6",
};
static char *iconst[] =
{
	"Select App","Phase sync",  "Keystone Cal",   "Wavy Plane",
	"Height Map","Voxie Player","Platonic Solids","Flying Stuff",
	"Chess",     "Packer",      "Paratrooper",    "Dot Munch",
	"Snake Tron","FlyStomp",    "Model / Anim"
};

enum
{
		//Generic names
	MENU_RESET, MENU_PAUSE, MENU_PREV, MENU_NEXT, MENU_ENTER, MENU_LOAD, MENU_SAVE,
	MENU_LEFT, MENU_RIGHT, MENU_UP, MENU_DOWN, MENU_CEILING, MENU_FLOOR,
	MENU_WHITE, MENU_RED, MENU_GREEN, MENU_BLUE, MENU_CYAN, MENU_MAGENTA, MENU_YELLOW,

		//2 state switches
	MENU_AUTOMOVEOFF, MENU_AUTOMOVEON,
	MENU_AUTOCYCLEOFF, MENU_AUTOCYCLEON,
	MENU_TEXEL_NEAREST, MENU_TEXEL_BILINEAR,
	MENU_SLICEDITHEROFF, MENU_SLICEDITHERON,
	MENU_TEXTUREOFF, MENU_TEXTUREON,
	MENU_SLICEOFF, MENU_SLICEON,
	MENU_DRAWSTATSOFF, MENU_DRAWSTATSON,

		//Other
	MENU_AUTOROTATEOFF, MENU_AUTOROTATEX, MENU_AUTOROTATEY, MENU_AUTOROTATEZ, MENU_AUTOROTATESPD,
	MENU_FRAME_PREV, MENU_FRAME_NEXT,
	MENU_SOL0, MENU_SOL1, MENU_SOL2, MENU_SOL3,
	MENU_HINT, MENU_DIFFICULTY, MENU_SPEED,
	MENU_ZIGZAG, MENU_SINE, MENU_HUMP_LEVEL,
	MENU_DISP_CUR, MENU_DISP_END=MENU_DISP_CUR+MAXDISP-1, MENU_DISP_ALL,
	MENU_ROTLEFT5, MENU_ROTRIGHT5, MENU_ROTLEFT, MENU_ROTRIGHT,
};


static char *gmediadir = 0;

static void genpath_voxiedemo_exe (const char *filnam, char *tbuf, int bufleng)
{
	static char gtbuf[MAX_PATH] = {0};
	static int gtleng = 0;
	int i;

	if (!gtbuf[0])
	{
		//Full path to exe (i.e. c:\kenstuff\voxon\voxiedemo.exe)
#if defined(_WIN32)
		gtleng = GetModuleFileName(0,gtbuf,bufleng-strlen(filnam)); //returns strlen?
#else
		char tbuf2[32];
		sprintf(tbuf2,"/proc/%d/exe",getpid());
		gtleng = min(readlink(tbuf2,gtbuf,bufleng-strlen(filnam),bufleng-strlen(filnam)-1); if (gtleng >= 0) gtbuf[gtleng] = 0;
#endif
		gtleng = strlen(gtbuf);
		while ((gtleng > 0) && (gtbuf[gtleng-1] != '\\') && (gtbuf[gtleng-1] != '/')) gtleng--;
	}
	i = strlen(filnam);
	memmove(&tbuf[gtleng],filnam,i+1);
	memcpy(tbuf,gtbuf,gtleng);
}

static void genpath_voxiedemo_media (const char *filnam, char *tbuf, int bufleng)
{
	char nst[MAX_PATH];

	genpath_voxiedemo_exe("",nst,bufleng);
	if (gmediadir) strcat(nst,gmediadir);
	strcat(nst,filnam);
	strcpy(tbuf,nst);
}

static int ghitkey = 0; //generic simulated keypress keyboard scancode for menu hacks
static int gdrawstats = 0, gautocycle = 0;
//static int gautorotateax = -1, gautorotatespd = 5;
static int gautorotatespd[3] = {0,0,0};
//--------------------------------------------------------------------------------------------------
	//Title globals
static double gtimbore = 0.0;
static int menu_title_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_UP:    ghitkey = 0xc8; break;
		case MENU_LEFT:  ghitkey = 0xcb; break;
		case MENU_RIGHT: ghitkey = 0xcd; break;
		case MENU_DOWN:  ghitkey = 0xd0; break;
		case MENU_ENTER: ghitkey = 0x1c; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Phasesync globals
static int gphasesync_curmode = 0, gphasesync_cursc = 4;
static int menu_phasesync_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_ZIGZAG:     gphasesync_curmode = 0; break;
		case MENU_SINE:       gphasesync_curmode = 1; break;
		case MENU_HUMP_LEVEL: gphasesync_cursc = 16>>(((int)v)-1); break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Keystonecal globals
typedef struct { float curx, cury, curz; } keystone_t;
static keystone_t keystone = {0};
static int grabdispall = 0;
static int menu_keystonecal_update (int id, char *st, double v, int how, void *userdata)
{
	keystone_t *keystone = (keystone_t *)userdata;
	switch(id)
	{
		case MENU_CEILING: keystone->curz = -vw.aspz; break;
		case MENU_FLOOR:   keystone->curz = +vw.aspz; break;
		case MENU_ROTLEFT5: ghitkey = 0x33; break;
		case MENU_ROTRIGHT5:ghitkey = 0x34; break;
		case MENU_ROTLEFT:  ghitkey = 0xb3; break;
		case MENU_ROTRIGHT: ghitkey = 0xb4; break;
		case MENU_DISP_CUR: case MENU_DISP_CUR+1: case MENU_DISP_CUR+2:
			vw.dispcur = id-MENU_DISP_CUR; grabdispall = 0;
			voxie_init(&vw);
			break;
		case MENU_DISP_ALL: grabdispall = 1; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Localcal globals
//--------------------------------------------------------------------------------------------------
	//Heightmap globals
typedef struct { char file[MAX_PATH*2+2]; point3d sp, sr, sd, sf, p, r, d, f; int colorkey, usesidebyside, mapzen; } dem_t;
#define GDEMMAX 256
static dem_t gdem[GDEMMAX];
static int gdemi = 0, gdemn = 0, gheightmap_flags = 7;
static int menu_heightmap_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_PREV:           ghitkey = 0xc9; break;
		case MENU_NEXT:           ghitkey = 0xd1; break;
		case MENU_AUTOCYCLEOFF:   gautocycle = 0;         break;
		case MENU_AUTOCYCLEON:    gautocycle = 1;         break;
		case MENU_TEXEL_NEAREST:  gheightmap_flags &= ~2; break;
		case MENU_TEXEL_BILINEAR: gheightmap_flags |=  2; break;
		case MENU_SLICEDITHEROFF: gheightmap_flags &= ~1; break;
		case MENU_SLICEDITHERON:  gheightmap_flags |=  1; break;
		case MENU_TEXTUREOFF:     gheightmap_flags &= ~4; break;
		case MENU_TEXTUREON:      gheightmap_flags |=  4; break;
		case MENU_RESET:          ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Voxieplayer globals
typedef struct { char file[MAX_PATH]; int mode, rep; } rec_t;
#define GRECMAX 256
static rec_t grec[GRECMAX];
static int greci = 0, grecn = 0, gautocycleall = 1, gvoxieplayer_ispaused = 0;
static int menu_voxieplayer_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_PREV:         ghitkey = 0xc9; break;
		case MENU_NEXT:         ghitkey = 0xd1; break;
		case MENU_DRAWSTATSOFF: gdrawstats = 0; break;
		case MENU_DRAWSTATSON:  gdrawstats = 1; break;
		case MENU_AUTOCYCLEOFF: gautocycleall = 0; break;
		case MENU_AUTOCYCLEON:  gautocycleall = 1; break;
		case MENU_PAUSE:        gvoxieplayer_ispaused = !gvoxieplayer_ispaused; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Platonicsolids globals
static int platonic_col = 7, platonic_solmode = 2, platonic_ispaused = 0;
static int menu_platonic_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_SOL0:    platonic_solmode = 0; break;
		case MENU_SOL1:    platonic_solmode = 1; break;
		case MENU_SOL2:    platonic_solmode = 2; break;
		case MENU_SOL3:    platonic_solmode = 3; break;
		case MENU_WHITE:   platonic_col = 7; break;
		case MENU_RED:     platonic_col = 4; break;
		case MENU_GREEN:   platonic_col = 2; break;
		case MENU_BLUE:    platonic_col = 1; break;
		case MENU_CYAN:    platonic_col = 3; break;
		case MENU_MAGENTA: platonic_col = 5; break;
		case MENU_YELLOW:  platonic_col = 6; break;
		case MENU_PAUSE:   platonic_ispaused = !platonic_ispaused; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Flyingstuff globals
static char gmyst[20+1] = {"Voxiebox by VOXON :)"};
//--------------------------------------------------------------------------------------------------
	//Chess globals
static int gchesscol[2] = {0x808080,0x104010};
static int gchessailev[2] = {0,5};
static int gchess_automove = 0, gchess_hint = 0;
static float gchesstime = 1.0;
static int menu_chess_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_HINT: gchess_hint = 1; break;
		case MENU_AUTOMOVEOFF: gchess_automove = 0; break;
		case MENU_AUTOMOVEON: gchess_automove = 1; break;
		case MENU_DIFFICULTY: gchessailev[1] = (int)v; break;
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Packer globals
static int gpakrendmode = 4, gpakxdim = 6, gpakydim = 3;
static int menu_packer_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_SOL0:  gpakrendmode = 1; break;
		case MENU_SOL1:  gpakrendmode = 2; break;
		case MENU_SOL2:  gpakrendmode = 4; break;
		case MENU_SOL3:  gpakrendmode = 6; break;
		case MENU_UP:    ghitkey = 0xc8; break;
		case MENU_LEFT:  ghitkey = 0xcb; break;
		case MENU_RIGHT: ghitkey = 0xcd; break;
		case MENU_DOWN:  ghitkey = 0xd0; break;
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Paratrooper globals
static int menu_paratrooper_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Dotmunch globals
typedef struct { int xsiz, ysiz, zsiz, xwrap, ywrap, zwrap; char *board; } mun_t;
#define MUNMAX 16
static int munlev = 0, munn = 2;
static mun_t gmun[MUNMAX] =
{
	21,19,1, 1,0,0,
	".Mxxxxxxxx.xxxxxxxxM."\
	".P...x...x.x...x...P."\
	".xxxxxxxxxxxxxxxxxxx."\
	".x...x.x.....x.x...x."\
	".Mxxxx.xxx.xxx.xxxxM."\
	".....x...x.x...x....."\
	".....x.xxxxxxx.x....."\
	".....x.x.....x.x....."\
	"xxxxxxxx.....xxxxxxxx"\
	".....x.x.....x.x....."\
	".....x.xxxxxxx.x....."\
	".....x.x.....x.x....."\
	".xxxxxxxxx.xxxxxxxxx."\

	".P...x...x.x...x...P."\
	".xxx.xxxxxSxxxxx.xxx."\
	"...x.x.x.....x.x.x..."\
	".xxxxx.xxx.xxx.xxxxx."\
	".x.......x.x.......x."\
	".xxxxxxxxxxxxxxxxxxx.",


	11,11,5, 0,0,0,
	"..........."\
	"....xxxxxP."\
	"....x....x."\
	"x...xxxM.x."\
	"x......x.x."\
	"xxxxxP.x.x."\
	"x....x.xxx."\
	"x....x....."\
	"x....x....."\
	"x....x....."\
	"Sxxxxx....."\

	"..........."\
	"..........."\
	"..........."\
	"x......x..."\
	"..........."\
	"x....x....."\
	"..........."\
	"..........."\
	"..........."\
	"..........."\
	"x....x....."\

	".......xxxx"\
	".......x..x"\
	".......x..x"\
	"x......x..x"\
	"..........x"\
	"x....x....x"\
	"..........x"\
	"..........x"\
	"..........x"\
	"..........x"\
	"x....x.xxxx"\

	".......x..."\
	"..........."\
	"..........."\
	"x.........x"\
	"..........."\
	"x....x....."\
	"..........."\
	"..........."\
	"..........."\
	"..........."\
	"x....x.x..."\

	"....xxxMxxx"\
	"....x.....x"\
	"....x.....x"\
	"xxxxx.....M"\
	"..........."\
	"xxxxxx....."\
	"x....x....."\
	"x....x....."\
	"x....x....."\
	"x....x....."\
	"Sxxxxxxx...",
};
static mun_t *cmun = &gmun[munlev];
static int menu_dotmunch_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_PREV:  ghitkey = 0xc9; break;
		case MENU_NEXT:  ghitkey = 0xd1; break;
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Snaketron globals
static int gminplays = 2, gdualnav = 0;
static int gsnaketime = 0, gsnakenumpels = 2, gsnakenumtetras = 2;
static float gsnakepelspeed = 0.25f;
static int gsnakecol[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00};
static int menu_snaketron_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_SPEED:
			switch((int)v)
			{
				case 1: gsnakepelspeed = 0.f; break;
				case 2: gsnakepelspeed = 0.0625f; break;
				case 3: gsnakepelspeed = 0.125f; break;
				case 4: gsnakepelspeed = 0.25f; break;
				case 5: gsnakepelspeed = 0.5f; break;
				case 6: gsnakepelspeed = 1.f; break;
				case 7: gsnakepelspeed = 2.f; break;
			}
			ghitkey = 0x27;
			break;
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Flystomp globals
static int menu_flystomp_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_RESET: ghitkey = 0x0e; break;
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------
	//Modelanim globals
typedef struct
{
	char file[MAX_PATH], snd[MAX_PATH];
	tiletype *tt;
	point3d p, p_init;
	float ang[3], sc, ang_init[3];
	float fps, defscale;
	int filetyp, mode, n, mal, cnt, loaddone; //filetyp:/*0=PNG,*/ 1=KV6,etc..
} anim_t;
#define GANIMMAX 256
static anim_t ganim[GANIMMAX];
static int ganimi = 0, ganimn = 0;
static int gmodelanim_pauseit = 0, gslicemode = 0;
static int grotatex = 0, grotatey = 0, grotatez = 5;
static point3d gslicep, gslicer, gsliced, gslicef;
static int menu_modelanim_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_PREV:          ghitkey = 0xc9; break;
		case MENU_NEXT:          ghitkey = 0xd1; break;
		case MENU_FRAME_PREV:    ghitkey = 0xc8; break;
		case MENU_FRAME_NEXT:    ghitkey = 0xd0; break;
		case MENU_DRAWSTATSOFF:  gdrawstats = 0; break;
		case MENU_DRAWSTATSON:   gdrawstats = 1; break;
		case MENU_AUTOCYCLEOFF:  gautocycle = 0; break;
		case MENU_AUTOCYCLEON:   gautocycle = 1; break;
		case MENU_SLICEOFF:      gslicemode = 0; break;
		case MENU_SLICEON:       gslicemode = 1;
										 gslicep.x = 0.f; gslicer.x = .05f; gsliced.x = 0.f; gslicef.x = 0.f;
										 gslicep.y = 0.f; gslicer.y = 0.f; gsliced.y = .05f; gslicef.y = 0.f;
										 gslicep.z = 0.f; gslicer.z = 0.f; gsliced.z = 0.f; gslicef.z = .05f; break;
		case MENU_RESET:         ghitkey = 0x0e; break;
		case MENU_PAUSE:         gmodelanim_pauseit = !gmodelanim_pauseit; break;
		case MENU_LOAD:          ghitkey = 0x26; break;
		case MENU_SAVE:          ghitkey = 0x1f; break;

#if 0
		case MENU_AUTOROTATEOFF: gautorotateax =-1; break;
		case MENU_AUTOROTATEX:   gautorotateax = 0; break;
		case MENU_AUTOROTATEY:   gautorotateax = 1; break;
		case MENU_AUTOROTATEZ:   gautorotateax = 2; break;
		case MENU_AUTOROTATESPD: gautorotatespd = (int)v; break;
#else
		case MENU_AUTOROTATEX:   gautorotatespd[0] = (int)v; break;
		case MENU_AUTOROTATEY:   gautorotatespd[1] = (int)v; break;
		case MENU_AUTOROTATEZ:   gautorotatespd[2] = (int)v; break;
#endif
	}
	return(1);
}
//--------------------------------------------------------------------------------------------------

typedef struct { int rendmode; char file[MAX_PATH]; } mus_t;
#define GMUSMAX 32
static mus_t gmus[GRECMAX];
static int gmusn = 0;

static int keymenu[2] = {0x29,0x39}; //menu key is: ` or tilda w/no shift by default
static int keyremap[8][7] =
{ //Lef  Rig  Up  Dow  LBut RBut  Mid
	0xcb,0xcd,0xc8,0xd0,0x9d,0x36,0x1c, //P1
	0x4b,0x4d,0x48,0x50,0xb5,0x37,0x00, //Q1
	0x24,0x26,0x17,0x25,0x2c,0x2d,0x00, //P2
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Q2
	0x20,0x22,0x13,0x21,0x1e,0x1f,0x00, //P3
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Q3
	0x2f,0x16,0x15,0x31,0x14,0x23,0x00, //P4
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Q4
};

#define MAXFRAMESPERVOL 128
#define MAXPLANES (MAXFRAMESPERVOL*24)
static int clutmid[MAXPLANES], grendmode = RENDMODE_TITLE, gcurselmode = 0, gshowborder = 1, gmenutimeout = 60;
static int div3[MAXPLANES], mod3[MAXPLANES], div24[MAXPLANES], mod24[MAXPLANES], oneupmod24[MAXPLANES];

//--------------------------------------------------------------------------------------------------
#if (USELEAP)
	//Based on \leap3\leaptest.c (12/01/2017)
#include "c:/LeapSDK/\include/LeapC.h"
#pragma comment(lib,"c:/LeapSDK/lib/x64/LeapC.lib")
static LEAP_CONNECTION leap_con;
static LEAP_TRACKING_EVENT leap_frame[2] = {0,0}; //FIFO for thread safety
static int leap_framecnt = 0, leap_iscon = 0, leap_isrun = 0;
static void leap_thread (void *_)
{
	LEAP_CONNECTION_MESSAGE msg;
	eLeapRS res;
	while (leap_isrun)
	{
		res = LeapPollConnection(leap_con,1000,&msg); if (res != eLeapRS_Success) { /*printf("LeapPollConnection() = %d\n",res);*/ continue; }
		switch (msg.type)
		{
			case eLeapEventType_Connection:     leap_iscon = 1; break;
			case eLeapEventType_ConnectionLost: leap_iscon = 0; break;
			case eLeapEventType_Tracking: memcpy(&leap_frame[leap_framecnt&1],msg.tracking_event,sizeof(LEAP_TRACKING_EVENT)); leap_framecnt++; break;
			default:;
		}
	}
}
void leap_uninit (void) { LeapDestroyConnection(leap_con); leap_isrun = 0; }
LEAP_CONNECTION *leap_init (void)
{
	eLeapRS res;
	res = LeapCreateConnection(0,&leap_con); if (res != eLeapRS_Success) return(&leap_con);
	res = LeapOpenConnection(leap_con);      if (res != eLeapRS_Success) return(&leap_con);
	leap_isrun = 1; _beginthread(leap_thread,0,0);
	while (!leap_iscon) Sleep(15);
	return(&leap_con);
}
LEAP_TRACKING_EVENT *leap_getframe (void) { return(&leap_frame[leap_framecnt&1]); }
#endif
//--------------------------------------------------------------------------------------------------

	//Rotate vectors a & b around their common plane, by ang
static void rotate_vex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

	//NOTE:assumes axis is unit length!
static void rotate_ax (point3d *p, point3d *ax, float w) //10/26/2011:optimized algo :)
{
	point3d op;
	float f, c, s;

	c = cos(w); s = sin(w);

		//P = cross(AX,P)*s + dot(AX,P)*(1-c)*AX + P*c;
	op = (*p);
	f = (op.x*ax->x + op.y*ax->y + op.z*ax->z)*(1.f-c);
	p->x = (ax->y*op.z - ax->z*op.y)*s + ax->x*f + op.x*c;
	p->y = (ax->z*op.x - ax->x*op.z)*s + ax->y*f + op.y*c;
	p->z = (ax->x*op.y - ax->y*op.x)*s + ax->z*f + op.z*c;
}

//--------------------------------------------------------------------------------------------------

static point3d plat_vert[4+8+6+20+12];
static int plat_vertoffs[5+1], plat_mesh[5][80], plat_meshn[5];
static void platonic_init (void)
{
	static const int tetr_facei[] = {0,1,2, 0,3,1, 0,2,3, 1,3,2};
	static const int cube_facei[] = {0,1,3,2, 5,4,6,7, 1,0,4,5, 2,3,7,6, 0,2,6,4, 3,1,5,7};
	static const int octa_facei[] = {0,2,4, 2,1,4, 1,3,4, 3,0,4, 0,3,5, 3,1,5, 1,2,5, 2,0,5};
	static const int dode_facei[] = {14,13,12,11,10, 10,11,1,8,0, 11,12,2,9,1, 12,13,3,5,2, 13,14,4,6,3, 14,10,0,7,4, 15,16,17,18,19, 15,19,9,2,5, 16,15,5,3,6, 17,16,6,4,7, 18,17,7,0,8, 19,18,8,1,9};
	static const int icos_facei[] = {6,11,9, 6,1,11, 6,4,1, 6,0,4, 6,9,0, 11,7,9, 7,11,3, 1,3,11, 3,1,10, 4,10,1, 10,4,8, 0,8,4, 8,0,2, 9,2,0, 2,9,7, 5,3,10, 5,10,8, 5,7,3, 5,2,7, 5,8,2};
	static const int *facei;
	float phi, fx, fy, fz, a, b;
	int i, j, k, m, nv, nfacei, nvert, vertperface;

	nv = 0; plat_vertoffs[0] = nv;
	for(m=0;m<5;m++)
	{
		switch(m)
		{
			case 0: //Tetra (based on RIGIDLINE3D.KC)
				a = 1.0/sqrt(3.0);
				for(i=0;i<4;i++) //corners of cube
				{
					plat_vert[nv+i].x = (float)(1-((i+1)&2))*a;
					plat_vert[nv+i].y = (float)(1-(i&1)*2)*a;
					plat_vert[nv+i].z = (float)(1-(i&2))*a;
				}
				nvert = 4; vertperface = 3; facei = tetr_facei; nfacei = sizeof(tetr_facei)/sizeof(int); break;
			case 1: //Cube
				a = 1.0/sqrt(3.0);
				for(i=0;i<8;i++)
				{
					plat_vert[nv+i].x = (float)(((i>>0)&1)*2-1)*a;
					plat_vert[nv+i].y = (float)(((i>>1)&1)*2-1)*a;
					plat_vert[nv+i].z = (float)(((i>>2)&1)*2-1)*a;
				}
				nvert = 8; vertperface = 4; facei = cube_facei; nfacei = sizeof(cube_facei)/sizeof(int); break;
			case 2: //Octa (based on RIGIDLINE3D.KC)
				for(i=0;i<2;i++)
				{
					plat_vert[nv+i  ].x = i*2-1; plat_vert[nv+i  ].y =     0; plat_vert[nv+i  ].z =     0;
					plat_vert[nv+i+2].x =     0; plat_vert[nv+i+2].y = i*2-1; plat_vert[nv+i+2].z =     0;
					plat_vert[nv+i+4].x =     0; plat_vert[nv+i+4].y =     0; plat_vert[nv+i+4].z = i*2-1;
				}
				nvert = 6; vertperface = 3; facei = octa_facei; nfacei = sizeof(octa_facei)/sizeof(int); break;
			case 3: //Dodec (based on DODEC2.KC)
				phi = (sqrt(5.f)+1.f)*.5f;
				for(j=0,k=0;j<4;j++)
					for(i=0;i<5;i++,k++)
					{
						fx = cos(i*(PI*.4f));
						fy = sin(i*(PI*.4f));
						if (j < 2) fz = 1.f-phi*.5f; else { fx *= phi-1.f; fy *= phi-1.f; fz = phi*.5f; }
						if (j&1) { fx = -fx; fy = -fy; fz = -fz; }
						plat_vert[nv+k].x = fx; plat_vert[nv+k].y = fy; plat_vert[nv+k].z = fz;
					}
				nvert = 20; vertperface = 5; facei = dode_facei; nfacei = sizeof(dode_facei)/sizeof(int); break;
			case 4: //Icos (based on RIGIDLINE3D.KC)
				phi = (sqrt(5.f)+1.f)*.5f; a = phi*.5f; b = .5f;
				for(fx=-1.f,k=0;fx<=1.f;fx+=2.f)
					for(fy=-1.f;fy<=1.f;fy+=2.f,k++)
					{
						plat_vert[nv+k  ].x =  0.f; plat_vert[nv+k  ].y = fx*b; plat_vert[nv+k  ].z = fy*a;
						plat_vert[nv+k+4].x = fx*b; plat_vert[nv+k+4].y = fy*a; plat_vert[nv+k+4].z =  0.f;
						plat_vert[nv+k+8].x = fy*a; plat_vert[nv+k+8].y =  0.f; plat_vert[nv+k+8].z = fx*b;
					}
				nvert = 12; vertperface = 3; facei = icos_facei; nfacei = sizeof(icos_facei)/sizeof(int); break;
		}

		plat_meshn[m] = 0;
		for(i=0;i<nfacei;i+=vertperface)
		{
			memcpy(&plat_mesh[m][plat_meshn[m]],&facei[i],vertperface*sizeof(int)); plat_meshn[m] += vertperface;
			plat_mesh[m][plat_meshn[m]] = -1; plat_meshn[m]++;
		}
		nv += nvert; plat_vertoffs[m+1] = nv;
	}
}

static void draw_platonic (int ind, float posx, float posy, float posz, float rad, float rot, int fillmode, int col)
{
	poltex_t *vt;
	point3d p0, p1;
	float c, s;
	int i, vo, nv;

	vo = plat_vertoffs[ind]; nv = plat_vertoffs[ind+1]-vo;
	vt = (poltex_t *)_alloca(nv*sizeof(vt[0]));
	c = cos(rot); s = sin(rot);
	for(i=nv-1;i>=0;i--)
	{
		p0 = plat_vert[i+vo];
		p1.x = p0.x*c - p0.y*s; p1.y = p0.y*c + p0.x*s; p1.z = p0.z;
		p0.x = p1.x*c - p1.z*s; p0.z = p1.z*c + p1.x*s; p0.y = p1.y;
		p1.y = p0.y*c - p0.z*s; p1.z = p0.z*c + p0.y*s; p1.x = p0.x;
		vt[i].x = p1.x*rad + posx;
		vt[i].y = p1.y*rad + posy;
		vt[i].z = p1.z*rad + posz;
		vt[i].u = (float)(i > 0);
		vt[i].v = (float)(i > 1);
		vt[i].col = col;
	}
	voxie_drawmeshtex(&vf,0,vt,nv,plat_mesh[ind],plat_meshn[ind],fillmode,col);
}

static void packer_rot (int rotmsk, int *x, int *y, int *z)
{
	int j;
	if (rotmsk& 1) { j = (*x); (*x) = (*y); (*y) = -j; } if (rotmsk& 2) { (*x) = -(*x); (*y) = -(*y); }
	if (rotmsk& 4) { j = (*x); (*x) = (*z); (*z) = -j; } if (rotmsk& 8) { (*x) = -(*x); (*z) = -(*z); }
	if (rotmsk&16) { j = (*y); (*y) = (*z); (*z) = -j; } if (rotmsk&32) { (*y) = -(*y); (*z) = -(*z); }
}

static void drawcube_faces_vis (voxie_frame_t *vf, point3d *pp, point3d *rr, point3d *dd, point3d *ff, int fillmode, int col, int i, int n, int *cx, int *cy, int *cz)
{
	point3d vt[8];
	int j, k, m;

	vt[0] = *pp; vt[1].x = vt[0].x+rr->x; vt[1].y = vt[0].y+rr->y; vt[1].z = vt[0].z+rr->z;
	for(j=2-1;j>=0;j--) { vt[3-j].x = vt[j].x+dd->x; vt[3-j].y = vt[j].y+dd->y; vt[3-j].z = vt[j].z+dd->z; }
	for(j=4-1;j>=0;j--) { vt[j+4].x = vt[j].x+ff->x; vt[j+4].y = vt[j].y+ff->y; vt[j+4].z = vt[j].z+ff->z; }
	if (fillmode&3)
	{
		static const int lin[12][2] = {0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7};
		static const int dir[12][2][3] =
		{
			0,-1,0, 0,0,-1,  +1,0,0, 0,0,-1,  0,+1,0, 0,0,-1,  -1,0,0, 0,0,-1,
			0,-1,0, 0,0,+1,  +1,0,0, 0,0,+1,  0,+1,0, 0,0,+1,  -1,0,0, 0,0,+1,
			-1,0,0, 0,-1,0,  +1,0,0, 0,-1,0,  +1,0,0, 0,+1,0,  -1,0,0, 0,+1,0,
		};
		int i0, i1, v;

			// 0----1
			// | \  . \
			// |  3----2
			// 4 .| 5  |
			//   \|   .|
			//    7----6
		for(j=12-1;j>=0;j--)
		{
				// X .   X x   X .   X x
				// . .   . .   x .   x .
				// yes   no    no    yes
				//
				// X .   X x   X .   X x
				// . x   . x   x x   x x
				// yes   yes   yes   no

			v = 0;
			for(k=n-1;k;k--)
			{
				m = i+k; if (m >= n) m -= n;
				if ((cx[m] == cx[i]+dir[j][0][0]             ) && (cy[m] == cy[i]+dir[j][0][1]             ) && (cz[m] == cz[i]+dir[j][0][2]             )) v |= 1;
				if ((cx[m] == cx[i]             +dir[j][1][0]) && (cy[m] == cy[i]             +dir[j][1][1]) && (cz[m] == cz[i]             +dir[j][1][2])) v |= 2;
				if ((cx[m] == cx[i]+dir[j][0][0]+dir[j][1][0]) && (cy[m] == cy[i]+dir[j][0][1]+dir[j][1][1]) && (cz[m] == cz[i]+dir[j][0][2]+dir[j][1][2])) v |= 4;
			}
			if ((v == 1) || (v == 2) || (v == 7)) continue;

			i0 = lin[j][0]; i1 = lin[j][1];
			if (fillmode&1) voxie_drawlin(vf,vt[i0].x,vt[i0].y,vt[i0].z,vt[i1].x,vt[i1].y,vt[i1].z,col);
			if (fillmode&2) voxie_drawcone(vf,vt[i0].x,vt[i0].y,vt[i0].z,.06f,vt[i1].x,vt[i1].y,vt[i1].z,.06f,1,col>>2);
		}
	}
	if (fillmode&4)
	{                                // z-       z+       y-       x+       y+       x-
		static const int fac[6][4] = {0,1,2,3, 4,5,6,7, 0,1,5,4, 1,2,6,5, 2,3,7,6, 3,0,4,7};
		poltex_t pt[4];
		int mesh[5];

		for(j=6-1;j>=0;j--)
		{
			for(k=n-1;k;k--)
			{
				m = i+k; if (m >= n) m -= n;
				if ((j == 5) && (cx[m] == cx[i]-1) && (cy[m] == cy[i]  ) && (cz[m] == cz[i]  )) break;
				if ((j == 3) && (cx[m] == cx[i]+1) && (cy[m] == cy[i]  ) && (cz[m] == cz[i]  )) break;
				if ((j == 2) && (cx[m] == cx[i]  ) && (cy[m] == cy[i]-1) && (cz[m] == cz[i]  )) break;
				if ((j == 4) && (cx[m] == cx[i]  ) && (cy[m] == cy[i]+1) && (cz[m] == cz[i]  )) break;
				if ((j == 0) && (cx[m] == cx[i]  ) && (cy[m] == cy[i]  ) && (cz[m] == cz[i]-1)) break;
				if ((j == 1) && (cx[m] == cx[i]  ) && (cy[m] == cy[i]  ) && (cz[m] == cz[i]+1)) break;
			}
			if (k) continue;

			pt[0].x = vt[fac[j][0]].x; pt[0].y = vt[fac[j][0]].y; pt[0].z = vt[fac[j][0]].z; pt[0].u = 0.f; pt[0].v = 0.f; pt[0].col = 0xffffff;
			pt[1].x = vt[fac[j][1]].x; pt[1].y = vt[fac[j][1]].y; pt[1].z = vt[fac[j][1]].z; pt[1].u = 1.f; pt[1].v = 0.f; pt[1].col = 0xffffff;
			pt[2].x = vt[fac[j][2]].x; pt[2].y = vt[fac[j][2]].y; pt[2].z = vt[fac[j][2]].z; pt[2].u = 1.f; pt[2].v = 1.f; pt[2].col = 0xffffff;
			pt[3].x = vt[fac[j][3]].x; pt[3].y = vt[fac[j][3]].y; pt[3].z = vt[fac[j][3]].z; pt[3].u = 0.f; pt[3].v = 1.f; pt[3].col = 0xffffff;
			mesh[0] = 0; mesh[1] = 1; mesh[2] = 2; mesh[3] = 3; mesh[4] = -1;
			voxie_drawmeshtex(vf,0,pt,4,mesh,5,2,col);
		}
	}
}

static void drawfootball (voxie_wind_t *vw, point3d *p, point3d *r, point3d *d, point3d *f, float wid, float lng, int col)
{
	int i, j;
	float g;

	for(j=-4;j<4;j+=2)
	{
		for(i=0;i<6;i++)
		{
			g = (float)i*PI*2.0/6.0;
			voxie_drawlin(&vf,p->x + (r->x*cos(g) + d->x*sin(g))*cos((j+0)*PI*.5/4)*wid + f->x*(j+0)*lng/4.0,
									p->y + (r->y*cos(g) + d->y*sin(g))*cos((j+0)*PI*.5/4)*wid + f->y*(j+0)*lng/4.0,
									p->z + (r->z*cos(g) + d->z*sin(g))*cos((j+0)*PI*.5/4)*wid + f->z*(j+0)*lng/4.0,//-vw->aspz),vw->aspz),
									p->x + (r->x*cos(g) + d->x*sin(g))*cos((j+2)*PI*.5/4)*wid + f->x*(j+2)*lng/4.0,
									p->y + (r->y*cos(g) + d->y*sin(g))*cos((j+2)*PI*.5/4)*wid + f->y*(j+2)*lng/4.0,
									p->z + (r->z*cos(g) + d->z*sin(g))*cos((j+2)*PI*.5/4)*wid + f->z*(j+2)*lng/4.0,//-vw->aspz),vw->aspz),
									col);
		}
		voxie_drawcone(&vf,p->x + f->x*(j+0)*lng/4.0,
								 p->y + f->y*(j+0)*lng/4.0,
								 p->z + f->z*(j+0)*lng/4.0,cos((j+0)*PI*.5/4)*wid,
								 p->x + f->x*(j+2)*lng/4.0,
								 p->y + f->y*(j+2)*lng/4.0,
								 p->z + f->z*(j+2)*lng/4.0,cos((j+2)*PI*.5/4)*wid,
								 1,col);
	}
}
static void drawship (voxie_wind_t *vw, point3d *p, point3d *r, point3d *d, point3d *f, float fthrust, int col)
{
	#define SHIPSC 1.8
	point3d fp;
	float g, h;
	int j;

	drawfootball(vw,p,r,d,f,.03*SHIPSC,.12*SHIPSC,col);
	h = .04*SHIPSC; g = -.12*SHIPSC;
	for(j=-1;j<=1;j+=2)
	{
		fp.x = p->x + r->x*j*h + f->x*g;
		fp.y = p->y + r->y*j*h + f->y*g;
		fp.z = p->z + r->z*j*h + f->z*g;
		drawfootball(vw,&fp,r,d,f,.016*SHIPSC,h+.02*SHIPSC,col);
	}
	fp.x = p->x - d->x*h*1.2 + f->x*g;
	fp.y = p->y - d->y*h*1.2 + f->y*g;
	fp.z = p->z - d->z*h*1.2 + f->z*g;
	drawfootball(vw,&fp,r,d,f,.016*SHIPSC,h+.02*SHIPSC,col);

	if (fthrust > 0.f)
	{
		h = ((float)(rand()&255))/5120.0*SHIPSC+.16*SHIPSC;
		fp.x = p->x - f->x*h;
		fp.y = p->y - f->y*h;
		fp.z = p->z - f->z*h;
		drawfootball(vw,&fp,r,d,f,.016*SHIPSC,h*.1,col);
	}
}

static void drawcube_thickwire (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, float rad, int col)
{
	int x, y;

	for(y=0;y<=1;y++)
		for(x=0;x<=1;x++)
		{
			voxie_drawcone(vf,(x1-x0)*x+x0,(y1-y0)*y+y0,z0,rad,
									(x1-x0)*x+x0,(y1-y0)*y+y0,z1,rad,1,col);
			voxie_drawcone(vf,(x1-x0)*x+x0,y0,(z1-z0)*y+z0,rad,
									(x1-x0)*x+x0,y1,(z1-z0)*y+z0,rad,1,col);
			voxie_drawcone(vf,x0,(y1-y0)*x+y0,(z1-z0)*y+z0,rad,
									x1,(y1-y0)*x+y0,(z1-z0)*y+z0,rad,1,col);
		}
}

static void drawcone_bot (float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1,
								  float cx, float cy, float cz, float nx0, float ny0, float nz0, float nx1, float ny1, float nz1, int col)
{
	point3d vec0, vec1, vec2;
	float f, c, s, t, t0, t1, ainc, px0, py0, pz0, px1, py1, pz1;

	vec0.x = x1-x0; vec0.y = y1-y0; vec0.z = z1-z0;
	f = 1.f/sqrt(vec0.x*vec0.x + vec0.y*vec0.y + vec0.z*vec0.z); vec0.x *= f; vec0.y *= f; vec0.z *= f;
	vec2.x = 0.f; vec2.y = 0.f; vec2.z = 1.f;
	vec1.x = vec2.y*vec0.z - vec2.z*vec0.y;
	vec1.y = vec2.z*vec0.x - vec2.x*vec0.z;
	vec1.z = vec2.x*vec0.y - vec2.y*vec0.x;

	if (col == 0x0000ff) ainc = PI/8.f;
						 else ainc = PI/12.f;
	for(f=ainc*.5f;f<PI*1.f;f+=ainc)
	{
		c = cos(f); s = sin(f);
		px0 = (vec1.x*c + vec2.x*s)*r0 + x0;
		py0 = (vec1.y*c + vec2.y*s)*r0 + y0;
		pz0 = (vec1.z*c + vec2.z*s)*r0 + z0;
		px1 = (vec1.x*c + vec2.x*s)*r1 + x1;
		py1 = (vec1.y*c + vec2.y*s)*r1 + y1;
		pz1 = (vec1.z*c + vec2.z*s)*r1 + z1;

		t0 = (px0-cx)*nx0 + (py0-cy)*ny0 + (pz0-cz)*nz0;
		t1 = (px1-cx)*nx0 + (py1-cy)*ny0 + (pz1-cz)*nz0;
		if ((t0 < 0.f) && (t1 < 0.f)) continue;
		if (t0 < 0.f) { t = t0/(t0-t1); px0 += (px1-px0)*t; py0 += (py1-py0)*t; pz0 += (pz1-pz0)*t; }
		if (t1 < 0.f) { t = t1/(t1-t0); px1 += (px0-px1)*t; py1 += (py0-py1)*t; pz1 += (pz0-pz1)*t; }

		t0 = (px0-cx)*nx1 + (py0-cy)*ny1 + (pz0-cz)*nz1;
		t1 = (px1-cx)*nx1 + (py1-cy)*ny1 + (pz1-cz)*nz1;
		if ((t0 < 0.f) && (t1 < 0.f)) continue;
		if (t0 < 0.f) { t = t0/(t0-t1); px0 += (px1-px0)*t; py0 += (py1-py0)*t; pz0 += (pz1-pz0)*t; }
		if (t1 < 0.f) { t = t1/(t1-t0); px1 += (px0-px1)*t; py1 += (py0-py1)*t; pz1 += (pz0-pz1)*t; }

		voxie_drawlin(&vf,px0,py0,pz0,px1,py1,pz1,col);
	}
}

	//Find shortest path between 2 line segments
	//Input: 2 line segments: a0-a1, b0-b1
	//Output: 2 intersection points: ai on segment a0-a1, bi on segment b0-b1
	//Returns: distance between ai&bi
static void roundcylminpath (point3d *a0, point3d *a1, point3d *b0, point3d *b1, point3d *ai, point3d *bi)
{
	point3d av, bv, ab;
	float k0, k1, k2, k3, k4, det, t, u;

	av.x = a1->x-a0->x; bv.x = b1->x-b0->x; ab.x = b0->x-a0->x;
	av.y = a1->y-a0->y; bv.y = b1->y-b0->y; ab.y = b0->y-a0->y;
	av.z = a1->z-a0->z; bv.z = b1->z-b0->z; ab.z = b0->z-a0->z;
	k0 = av.x*av.x + av.y*av.y + av.z*av.z;
	k1 = bv.x*bv.x + bv.y*bv.y + bv.z*bv.z;
	k2 = av.x*ab.x + av.y*ab.y + av.z*ab.z;
	k3 = bv.x*ab.x + bv.y*ab.y + bv.z*ab.z;
	k4 = av.x*bv.x + av.y*bv.y + av.z*bv.z;
		// k0*t - k4*u = k2
		//-k4*t + k1*u =-k3
	det = k0*k1 - k4*k4;
	if (det != 0.0)
	{
		det = 1.0/det;
		t = (k1*k2 - k3*k4)*det;
		u = (k2*k4 - k0*k3)*det;
	} else { t = 0.0; u = -k2/k3; }
	t = min(max(t,0.0),1.0);
	u = min(max(u,0.0),1.0);
	ai->x = av.x*t + a0->x; bi->x = bv.x*u + b0->x;
	ai->y = av.y*t + a0->y; bi->y = bv.y*u + b0->y;
	ai->z = av.z*t + a0->z; bi->z = bv.z*u + b0->z;
	//return(sqrt((bi->x-ai->x)*(bi->x-ai->x) + (bi->y-ai->y)*(bi->y-ai->y) + (bi->z-ai->z)*(bi->z-ai->z)));
}

static void vecrand (point3d *a, float mag) //UNIFORM spherical randomization (see spherand.c)
{
	float f;

	a->z = (float)rand()/(float)RAND_MAX*2.0-1.0;
	f = (float)rand()/(float)RAND_MAX*(PI*2.0); a->x = cos(f); a->y = sin(f);
	f = sqrt(1.0 - a->z*a->z)*mag; a->x *= f; a->y *= f; a->z *= mag;
}

//--------------------------------------------------------------------------------------------------

#define MAXDEP 8
static int rank[16] = {0,1,3,3,5,15,1024,0,0,0,-1024,-15,-5,-3,-3,-1};
static int gmove[16384], gmoven = 0;
static int moves[MAXDEP][320], csc[MAXDEP][320];
static int guseprune;

static int ksgn (int i) { if (i < 0) return(-1); if (i > 0) return(+1); return(0); }

	//Assumes (kingx,kingy) is turn's king (even if it's not)
static int ischeck (int board[8][8], int kingx, int kingy, int turn)
{
	int i, b, t, x, y, x0, y0, x1, y1;

	t = 1-turn*2;

		//check king
	x0 = max(kingx-1,0); x1 = min(kingx+1,7);
	y0 = max(kingy-1,0); y1 = min(kingy+1,7);
	for(y=y0;y<=y1;y++) for(x=x0;x<=x1;x++) if (board[y][x] == t*-6) return(1);

		//check pawn
	if (kingy != (1-turn)*7)
	{
		if ((kingx > 0) && (board[kingy+t][kingx-1] == -t)) return(1);
		if ((kingx < 7) && (board[kingy+t][kingx+1] == -t)) return(1);
	}

		//check knight
	static const int knix[8] = {+1,+2,+2,+1,-1,-2,-2,-1};
	static const int kniy[8] = {+2,+1,-1,-2,-2,-1,+1,+2};
	for(i=8-1;i>=0;i--)
	{
		x = knix[i]+kingx; if ((x < 0) || (x >= 8)) continue;
		y = kniy[i]+kingy; if ((y < 0) || (y >= 8)) continue;
		if (board[y][x] == t*-2) return(1);
	}

		//check others
	static const int dirx[8] = {-1,+1, 0, 0,-1,+1,-1,+1};
	static const int diry[8] = { 0, 0,-1,+1,-1,-1,+1,+1};
	static const int dirt[8] = {-4,-4,-4,-4,-3,-3,-3,-3};
	for(i=8-1;i>=0;i--)
	{
		x = kingx; y = kingy;
		do
		{
			x += dirx[i]; if ((x < 0) || (x >= 8)) break;
			y += diry[i]; if ((y < 0) || (y >= 8)) break;
			b = board[y][x]; if (!b) continue;
			b *= t; if ((b == -5) || (b == dirt[i])) return(1); //opp: queen=t*-5, bishop=t*-3, rook=t*-4
			break;
		} while (1);
	}

	return(0);
}

static int domove (int board[8][8], int *caststat, int *prevmove, int x0, int y0, int x1, int y1, int doit)
{
	int sc, o, n, t, kx, ky;

	sc = 0;
	o = board[y0][x0]; t = ksgn(o);
	n = board[y1][x1];

	(*prevmove) = y1*512 + x1*64 + y0*8 + x0;

	if (labs(o) == 1) //pawn
	{
			//promote to queen
			  if ((o > 0) && (y1 == 7)) { o = 5; sc += rank[5]-rank[1]; }
		else if ((o < 0) && (y1 == 0)) { o =-5; sc -= rank[5]-rank[1]; }

			//en passant
		if ((n == 0) && (x0 != x1))
		{
			if (doit) { voxie_playsound("getstuff.flac",-1,100,100,1.f); }
			if (board[y0][x1]) sc -= rank[board[y0][x1]&15];
			board[y0][x1] = 0;
		}
	}

	if (labs(o) == 6) //castle
	{
			//moving king kills castle opportunity
		if (o > 0) { (*caststat) +=  3-((*caststat)& 3)  ; }
				else { (*caststat) += 12-((*caststat)>>2)*4; }
		if (labs(x1-x0) == 2)
		{
			board[y1][(x1>4)*2+3] = t*4;
			board[y1][(x1>4)*7] = 0;
		}
	}

	if (labs(o) == 4) //rook: moving it kills castle opportunity
	{
		if (o > 0)
		{
				  if (x0 == 0) { if (!((*caststat)&1)) (*caststat) += 1; }
			else if (x0 == 7) { if (!((*caststat)&2)) (*caststat) += 2; }
		}
		else
		{
				  if (x0 == 0) { if (!((*caststat)&4)) (*caststat) += 4; }
			else if (x0 == 7) { if (!((*caststat)&8)) (*caststat) += 8; }
		}
	}

	board[y0][x0] = 0;
	if (board[y1][x1])
	{
		if (y1 == 0)
		{
				  if (x1 == 0) { if (!((*caststat)&1)) (*caststat) += 1; }
			else if (x1 == 7) { if (!((*caststat)&2)) (*caststat) += 2; }
		}
		else
		{
				  if (x1 == 0) { if (!((*caststat)&4)) (*caststat) += 4; }
			else if (x1 == 7) { if (!((*caststat)&8)) (*caststat) += 8; }
		}
		sc -= rank[n&15];
	}
	board[y1][x1] = o;

	if (doit)
	{
		gmove[gmoven] = (*prevmove); gmoven++;
		if (n)
		{
			if (labs(n) == 1) voxie_playsound("getstuff.flac",-1,100,100,1.f);
			if (labs(n) == 2) voxie_playsound("blowup.flac",-1,100,100,1.f);
			if (labs(n) == 3) voxie_playsound("blowup.flac",-1,100,100,1.f);
			if (labs(n) == 4) voxie_playsound("blowup.flac",-1,100,100,1.f);
			if (labs(n) == 5) voxie_playsound("death.flac",-1,100,100,1.f);
			if (labs(n) == 6) voxie_playsound("closdoor.flac",-1,100,100,1.f);
		}

		for(ky=0;ky<8;ky++)
			for(kx=0;kx<8;kx++)
			{
				if (board[ky][kx] != t*-6) continue;
				if (ischeck(board,kx,ky,1-(o<0))) voxie_playsound("alarm.flac",-1,100,100,1.f);
				goto break2;
			}
break2:;
	}

	return(sc*t);
}

static int domove_ret_sc_only (int board[8][8], int x0, int y0, int x1, int y1)
{
	int sc, o, n;

	sc = 0;
	o = board[y0][x0];
	n = board[y1][x1];

	if (labs(o) == 1)
	{
			//promote to queen
			  if ((o > 0) && (y1 == 7)) { sc = rank[5]-rank[1]; }
		else if ((o < 0) && (y1 == 0)) { sc = rank[1]-rank[5]; }

			//en passant
		if ((n == 0) && (x0 != x1) && (board[y0][x1])) sc = -rank[board[y0][x1]&15];
	}

	if (n) sc -= rank[n&15];
	return(ksgn(o)*sc);
}

static void undomove (int board[8][8], int x0, int y0, int x1, int y1, int patch0, int patch1)
{
	if ((labs(board[y1][x1]) == 1) && (x0 != x1) && (!patch1)) //undo en passant
		{ board[y0][x1] = -board[y1][x1]; }
	board[y0][x0] = patch0;
	board[y1][x1] = patch1;
	if ((labs(patch0) == 6) && (labs(x1-x0) == 2)) //undo castle
	{
		board[y1][(x1>4)*2+3] = 0;
		board[y1][(x1>4)*7] = ksgn(patch0)*4;
	}
}

static void move2xys (int k, int *x0, int *y0, int *x1, int *y1)
{
	(*x0) = ((k   )&7); (*y0) = ((k>>3)&7);
	(*x1) = ((k>>6)&7); (*y1) = ((k>>9)&7);
}

static int getvalmoves (int board[8][8], int caststat, int prevmove, int turn, int dep)
{
	int i, j, m, n, p, t, x, y, x0, y0, x1, y1, ox0, oy0, ox1, oy1, b, kx, ky, ocaststat, oprevmove, patch0, patch1;

	n = 0; t = 1-turn*2;
	for(y0=0;y0<8;y0++)
		for(x0=0;x0<8;x0++)
		{
			p = board[y0][x0]; if (ksgn(p) != t) continue;
			p = labs(p); m = (y0*8+x0)*65;

			if (p == 1) //pawn
			{
				if (!board[y0+t][x0])
				{
					moves[dep][n] = m + t*512; n++;
					if ((y0 == turn*5+1) && (!board[y0+t*2][x0]))
						{ moves[dep][n] = m + t*1024; n++; }
				}
				for(x=-1;x<=1;x+=2)
				{
					if ((x+x0 < 0) || (x+x0 >= 8)) continue;
					if (ksgn(board[y0+t][x+x0]) == -t) { moves[dep][n] = m + t*512 + x*64; n++; }
				}
				if (y0 == 4-turn) //capture en passant
				{
					move2xys(prevmove,&ox0,&oy0,&ox1,&oy1);
					if ((board[oy1][ox1] == -t) && (labs(oy1-oy0) == 2) && (labs(ox0-x0) == 1))
						{ moves[dep][n] = m + t*512 + (ox1-x0)*64; n++; }
				}
				continue;
			}
			if (p == 2) //knight
			{
				static const int knix[8] = {+1,+2,+2,+1,-1,-2,-2,-1};
				static const int kniy[8] = {+2,+1,-1,-2,-2,-1,+1,+2};
				static const int knia[8] = {1024+64,512+128,-512+128,-1024+64,-1024-64,-512-128,512-128,1024-64};
				for(i=8-1;i>=0;i--)
				{
					x = knix[i]+x0; if ((x < 0) || (x >= 8)) continue;
					y = kniy[i]+y0; if ((y < 0) || (y >= 8)) continue;
					if (ksgn(board[y][x]) == t) continue;
					moves[dep][n] = knia[i] + m; n++;
				}
				continue;
			}
			if (p == 6) //king
			{
				static const int kinx[8] = {-1, 0,+1,-1,+1,-1, 0,+1};
				static const int kiny[8] = {-1,-1,-1, 0, 0,+1,+1,+1};
				static const int kina[8] = {-9*64,-8*64,-7*64,-1*64,+1*64,+7*64,+8*64,+9*64};
				for(i=8-1;i>=0;i--)
				{
					x = kinx[i]+x0; if ((x < 0) || (x >= 8)) continue;
					y = kiny[i]+y0; if ((y < 0) || (y >= 8)) continue;
					if (ksgn(board[y][x]) == t) continue;
					moves[dep][n] = kina[i] + m; n++;
				}
				if ((x0 == 4) && (y0 == turn*7)) //castle
				{
						//long castle
					if ((board[y0][3] == 0) && (board[y0][2] == 0) && (board[y0][1] == 0) &&
						 (board[y0][0] == t*4) && ((caststat%(turn* 6+2)) < turn*3+1))
					{
						if ((!ischeck(board,4,y0,turn)) &&
							 (!ischeck(board,3,y0,turn)) &&
							 (!ischeck(board,2,y0,turn))) { moves[dep][n] = m - 128; n++; }
					}
					
						//short castle
					if ((board[y0][5] == 0) && (board[y0][6] == 0) &&
						 (board[y0][7] == t*4) && ((caststat%(turn*12+4)) < turn*6+2))
					{
						if ((!ischeck(board,4,y0,turn)) &&
							 (!ischeck(board,5,y0,turn)) &&
							 (!ischeck(board,6,y0,turn))) { moves[dep][n] = m + 128; n++; }
					}
				}
				continue;
			}
			if (p != 3) //horiz/vert (rook&queen)
			{
				for(i=1,j=x0;i<=j;i++)
				{
					b = board[y0][x0-i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(-64) + m; n++; if (b) break;
				}
				for(i=1,j=7-x0;i<=j;i++)
				{
					b = board[y0][x0+i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(+64) + m; n++; if (b) break;
				}
				for(i=1,j=y0;i<=j;i++)
				{
					b = board[y0-i][x0]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(-512) + m; n++; if (b) break;
				}
				for(i=1,j=7-y0;i<=j;i++)
				{
					b = board[y0+i][x0]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(+512) + m; n++; if (b) break;
				}
			}
			if (p != 4) //diag (bishop&queen)
			{
				for(i=1,j=min(x0,y0);i<=j;i++)
				{
					b = board[y0-i][x0-i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(-64-512) + m; n++; if (b) break;
				}
				for(i=1,j=min(7-x0,7-y0);i<=j;i++)
				{
					b = board[y0+i][x0+i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(+64+512) + m; n++; if (b) break;
				}
				for(i=1,j=min(x0,7-y0);i<=j;i++)
				{
					b = board[y0+i][x0-i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(-64+512) + m; n++; if (b) break;
				}
				for(i=1,j=min(7-x0,y0);i<=j;i++)
				{
					b = board[y0-i][x0+i]; if (ksgn(b) == t) break;
					moves[dep][n] = i*(+64-512) + m; n++; if (b) break;
				}
			}
		}

	if (dep == 0) //delete moves that would put king in check
	{
		for(y=0;y<8;y++)
			for(x=0;x<8;x++)
				if (board[y][x] == t*6) { kx = x; ky = y; goto foundking2; }
foundking2:;
		for(i=n-1;i>=0;i--)
		{
			move2xys(moves[dep][i],&x0,&y0,&x1,&y1);

			ocaststat = caststat; oprevmove = prevmove; patch0 = board[y0][x0]; patch1 = board[y1][x1];
			domove(board,&caststat,&prevmove,x0,y0,x1,y1,0);

			for(y=max(ky-1,0);y<=min(ky+1,7);y++)
				for(x=max(kx-2,0);x<=min(kx+2,7);x++)
					if (board[y][x] == t*6)
						if (ischeck(board,x,y,turn))
							{ n--; moves[0][i] = moves[0][n]; } //delete move - would put king in check

			undomove(board,x0,y0,x1,y1,patch0,patch1); caststat = ocaststat; prevmove = oprevmove;
		}
	}

	return(n);
}

static int isvalmove (int board[8][8], int caststat, int prevmove, int x0, int y0, int x1, int y1)
{
	int i, m, n, p;

	p = board[y0][x0]; if (p == 0) return(0);
	m = y1*512 + x1*64 + y0*8 + x0;
	n = getvalmoves(board,caststat,prevmove,p<0,0);
	for(i=n-1;i>=0;i--) if (moves[0][i] == m) return(1);
	return(0);
}

static int getcompmove_rec (int board[8][8], int *caststat, int *prevmove, int turn, int *bx0, int *by0, int *bx1, int *by1, int dep, int depmax, int obestsc)
{
	int i, j, k, sc, x0, y0, x1, y1, nx0, ny0, nx1, ny1, ocaststat, oprevmove, patch0, patch1, movesn, bsc;

	movesn = getvalmoves(board,*caststat,*prevmove,turn,dep); bsc = 0x80000001;

	for(i=movesn-1;i>=0;i--)
	{
		j = (rand()%(i+1)); k = moves[dep][j]; moves[dep][j] = moves[dep][i]; moves[dep][i] = k; //Shuffle
		move2xys(k,&x0,&y0,&x1,&y1);

		ocaststat = (*caststat); oprevmove = (*prevmove); patch0 = board[y0][x0]; patch1 = board[y1][x1];
		sc = domove(board,caststat,prevmove,x0,y0,x1,y1,0);
		if (dep < depmax)
			sc -= getcompmove_rec(board,caststat,prevmove,1-turn,&nx0,&ny0,&nx1,&ny1,dep+1,depmax,bsc-sc);
		undomove(board,x0,y0,x1,y1,patch0,patch1); (*caststat) = ocaststat; (*prevmove) = oprevmove;

		if (sc > bsc)
		{
			bsc = sc; if (!dep) { (*bx0) = x0; (*by0) = y0; (*bx1) = x1; (*by1) = y1; }
			if ((guseprune) && (sc >= -obestsc)) break; //alpha-beta prune
		}
	}
	return(bsc);
}

static int getcompmove (int board[8][8], int *caststat, int *prevmove, int turn, int *bx0, int *by0, int *bx1, int *by1, int depmax)
{
	int bsc;

	guseprune = (depmax >= 0); depmax = labs(depmax);
	bsc = getcompmove_rec(board,caststat,prevmove,turn,bx0,by0,bx1,by1,0,depmax-1,0x80000001);
	return(bsc != 0x80000001);
}

//--------------------------------------------------------------------------------------------------

static void drawchopper (float fx, float fy, float fz, float sc, float ha, float tim)
{
	static const char *ptroopnam[] = {"chopper.kv6","chopper_rotor.kv6","chopper_tail_rotor.kv6"};
	point3d pp, rr, dd, ff;
	float c, s, f, g;
	int i;

	c = cos(ha); s = sin(ha);
	for(i=0;i<3;i++)
	{
		if (i == 0)
		{
			f = sc*0.5f;
			rr.x =   f; rr.y = 0.f; rr.z = 0.f;
			dd.x = 0.f; dd.y =   f; dd.z = 0.f;
			ff.x = 0.f; ff.y = 0.f; ff.z =   f;
			pp.x = 0.f; pp.y = sc*.02f; pp.z = sc*.07f;
		}
		if (i == 1)
		{
			f = sc*0.5f; g = tim*16.f;
			rr.x = cos(g)*f; rr.y = sin(g)*f; rr.z = 0.f;
			dd.x =-sin(g)*f; dd.y = cos(g)*f; dd.z = 0.f;
			ff.x =      0.f; ff.y =      0.f; ff.z =   f;
			pp.x = 0.f; pp.y = sc*-.015f; pp.z = sc*.07f;
			f = 0.08f; pp.x += dd.x*f; pp.y += dd.y*f; pp.z += dd.z*f;
		}
		if (i == 2)
		{
			f = sc*0.125f; g = tim*8.f;
			rr.x =   f; rr.y =      0.f; rr.z =     0.f;
			dd.x = 0.f; dd.y = cos(g)*f; dd.z = sin(g)*f;
			ff.x = 0.f; ff.y =-sin(g)*f; ff.z = cos(g)*f;
			pp.x = 0.f; pp.y = sc*.32f; pp.z = sc*-.03f;
			f = 4.10f; pp.x -= dd.x*f; pp.y -= dd.y*f; pp.z -= dd.z*f;
			f = -1.54f; pp.x -= ff.x*f; pp.y -= ff.y*f; pp.z -= ff.z*f;
		}

		f = rr.x*c - rr.y*s; rr.y = rr.y*c + rr.x*s; rr.x = f;
		f = dd.x*c - dd.y*s; dd.y = dd.y*c + dd.x*s; dd.x = f;
		f = ff.x*c - ff.y*s; ff.y = ff.y*c + ff.x*s; ff.x = f;
		f = pp.x*c - pp.y*s; pp.y = pp.y*c + pp.x*s; pp.x = f;
		pp.x += fx; pp.y += fy; pp.z += fz;

		voxie_drawspr(&vf,ptroopnam[i],&pp,&rr,&dd,&ff,0xffffff);
	}
}

static void drawman (float fx, float fy, float fz, float sc, float ha, int ischute)
{
	#define PTMAX 256
	poltex_t pt[PTMAX];
	float fz2, f, g;
	int i, n;

	voxie_drawsph(&vf,fx,fy,fz-sc*.02f,sc*.012,1,0xffffff); //Head
	for(f=-1.f;f<=1.f;f+=0.5f) //Arms
	{
		voxie_drawsph(&vf,fx+cos(ha)*sc*f*.018f,fy+sin(ha)*sc*f*.018f,fz,sc*.008f,1,0x00ffff);
	}
	voxie_drawcone(&vf,fx,fy,fz-sc*.02f,sc*.010f, //Torso
							 fx,fy,fz+sc*.02f,sc*.010f,1,0x00ffff);
	for(f=-1.f;f<=1.f;f+=2.f)
	{
		voxie_drawcone(&vf,fx                   ,fy                   ,fz+sc*.02f,sc*.008f, //Legs
								 fx+cos(ha)*sc*.018f*f,fy+sin(ha)*sc*.018f*f,fz+sc*.04f,sc*.008f,1,0x00ffff);
	}
	if (ischute)
	{
		voxie_drawcone(&vf,fx-sc*.018f,fy,fz-sc*.02f,sc*.008f,
								 fx-sc*.040f,fy,fz-sc*.08f,sc*.008f,1,0xff00ff);
		voxie_drawcone(&vf,fx+sc*.018f,fy,fz-sc*.02f,sc*.008f,
								 fx+sc*.040f,fy,fz-sc*.08f,sc*.008f,1,0xff00ff);

		n = 4096; g = 2.f/(float)n; fz2 = g*.5f-1.f;
		n = ((n*6)>>4);
		for(i=0;i<n;i++,fz2+=g)
		{
			f = sqrt(1.f-fz2*fz2);
			pt[i&(PTMAX-1)].x = cos((double)i*(sqrt(5.0)-1.0)*PI)*f*sc*.07f + fx;
			pt[i&(PTMAX-1)].y = sin((double)i*(sqrt(5.0)-1.0)*PI)*f*sc*.07f + fy;
			pt[i&(PTMAX-1)].z = fz2*sc*.07f - .02f + fz;
			pt[i&(PTMAX-1)].col = 0xffffff;
			if ((i&(PTMAX-1)) == PTMAX-1) voxie_drawmeshtex(&vf,0,&pt[0],PTMAX,0,0,0,0xffffff);
		}
		if (i&(PTMAX-1)) voxie_drawmeshtex(&vf,0,&pt[0],i&(PTMAX-1),0,0,0,0xffffff);

	}
}

//--------------------------------------------------------------------------------------------------
static char mungetboard (mun_t *cmun, int x, int y, int z, char defchar)
{
	if (!cmun->xwrap) { if ((unsigned)x >= (unsigned)cmun->xsiz) return(defchar); } else { x %= cmun->xsiz; if (x < 0) x += cmun->xsiz; }
	if (!cmun->ywrap) { if ((unsigned)y >= (unsigned)cmun->ysiz) return(defchar); } else { y %= cmun->ysiz; if (y < 0) y += cmun->ysiz; }
	if (!cmun->zwrap) { if ((unsigned)z >= (unsigned)cmun->zsiz) return(defchar); } else { z %= cmun->zsiz; if (z < 0) z += cmun->zsiz; }
	return(cmun->board[(z*cmun->ysiz + y)*cmun->xsiz + x]);
}

static void drawlin (float x, float y, float z, float x0, float y0, float z0, float x1, float y1, float z1, int dir, int col)
{
	float t;
	int i;

	if (dir&1) { x0 *= -1.f; x1 *= -1.f; }
	if (dir&2) { y0 *= -1.f; y1 *= -1.f; }
	if (dir&4) { z0 *= -1.f; z1 *= -1.f; }
	i = (dir>>3);
	if (i == 1) { t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
	if (i == 2) { t = x0; x0 = z0; z0 = t; t = x1; x1 = z1; z1 = t; }
	if (i == 3) { t = y0; y0 = z0; z0 = t; t = y1; y1 = z1; z1 = t; }
	if (i == 4) { t = x0; x0 = y0; y0 = z0; z0 = t; t = x1; x1 = y1; y1 = z1; z1 = t; }
	if (i == 5) { t = x0; x0 = z0; z0 = y0; y0 = t; t = x1; x1 = z1; z1 = y1; y1 = t; }
	voxie_drawlin(&vf,x0+x,y0+y,z0+z,x1+x,y1+y,z1+z,col);
}

static void drawcyl (float x, float y, float z, float r, int col, int vis, int n2, int nlin /*should be multiple of 4*/)
{
	static const char dirlut[64] =
	{
		-1, 0, 1, 0, 8,22,18,28,
		 9,20,16,24, 8,44,40,24,
		16,14,13, 7,30, 0, 1, 4,
		26, 2, 3, 7,12,12,14,24,
		17,10, 8, 0, 2, 4, 5, 0,
		 0, 6, 7, 2, 8, 8,10,26,
		16,20,16, 7,36,20,16, 0,
		32,22,18, 2, 8, 8,10,-1
	};
	static const char dirlut2[8][3] =
	{
		7,15,31, 6,13,30, 5,14,27, 4,12,26,
		3,11,29, 2, 9,28, 1,10,25, 0, 8,24,
	};
	float f, g, c, s, c0, s0, c1, s1;
	int i, j, k, m, m0, m1, n, dir;

	n = 0; for(i=0;i<6;i++) { if (vis&(1<<i)) n++; }

		//64 cases
	dir = dirlut[vis];
	if (n == 0) { } //1 null
	else if (n == 1) //6 dead ends
	{
		for(i=0;i<nlin;i++)
		{
			c = cos((float)i*PI*2.0/(float)nlin)*r;
			s = sin((float)i*PI*2.0/(float)nlin)*r;
			for(j=0;j<n2*6/6;j++)
			{
				c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
				s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
				drawlin(x,y,z,r*s0,c*c0,s*c0,r*s1,c*c1,s*c1,dir,col);
			}
			drawlin(x,y,z,0,c,s,-r,c,s,dir,col);
		}
	}
	else if (n == 2)
	{
		if ((vis == 3) || (vis == 12) || (vis == 48)) //3 axes
		{
			for(i=0;i<nlin;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				drawlin(x,y,z,-r,c,s,r,c,s,dir,col);
			}
		}
		else //12 L-shaped
		{
			for(i=0;i<nlin;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r;
				s = sin((float)i*PI*2.0/(float)nlin)*r; s += r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
					drawlin(x,y,z,c,r-s*c0,r-s*s0,
									  c,r-s*c1,r-s*s1,dir,col);
				}
			}
		}
	}
	else if (n == 3)
	{
			//8 corners
		if ((vis == 21) || (vis == 22) || (vis == 25) || (vis == 26) ||
			 (vis == 37) || (vis == 38) || (vis == 41) || (vis == 42))
		{
			m = dirlut[vis];
			for(i=(int)(nlin*3/8);i<=nlin*7/8;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
					for(k=3-1;k>=0;k--)
						drawlin(x,y,z,r-c*c0,s,r-c*s0,
										  r-c*c1,s,r-c*s1,dirlut2[m][k],col);
				}
			}
		}
		else //12 half plusses
		{
			for(i=nlin/2+1;i<nlin;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				drawlin(x,y,z,-r,c,s,r,c,s,dir,col);
			}
			for(i=nlin/4;i<=nlin*3/4;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/n2);
					for(f=-1;f<=1;f+=2)
						drawlin(x,y,z,(r-c*c0)*f,s,r-c*s0,
										  (r-c*c1)*f,s,r-c*s1,dir,col);
				}
			}
		}
	}
	else if (n == 4)
	{
		if ((vis == 15) || (vis == 51) || (vis == 60)) //3 plusses
		{
			for(i=nlin/4;i<=nlin*3/4;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
					for(g=-1.f;g<=1.f;g+=2.f)
						for(f=-1.f;f<=1.f;f+=2.f)
							drawlin(x,y,z,(r-c*c0)*f,s,(r-c*s0)*g,
											  (r-c*c1)*f,s,(r-c*s1)*g,dir,col);
				}
			}
		}
		else //12 corners with 1 extra
		{
			for(i=((nlin*3)>>2);i<nlin;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				drawlin(x,y,z,-r,c,s,r,c,s,dir,col);
			}
			for(i=nlin/4;i<=nlin*2/4;i++)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
					for(f=-1.f;f<=1.f;f+=2.f)
					{
						drawlin(x,y,z,(r-c*c0)*f,s,r-c*s0,
										  (r-c*c1)*f,s,r-c*s1,dir,col);
						drawlin(x,y,z,(r-c*c0)*f,-(r-c*s0),-s,
										  (r-c*c1)*f,-(r-c*s1),-s,dir,col);
					}
				}
			}
			if (vis == 23) { m0 = 0; m1 = 1; }
			if (vis == 27) { m0 = 3; m1 = 2; }
			if (vis == 29) { m0 = 0; m1 = 2; }
			if (vis == 30) { m0 = 3; m1 = 1; }
			if (vis == 39) { m0 = 4; m1 = 5; }
			if (vis == 43) { m0 = 6; m1 = 7; }
			if (vis == 45) { m0 = 4; m1 = 6; }
			if (vis == 46) { m0 = 5; m1 = 7; }
			if (vis == 53) { m0 = 0; m1 = 4; }
			if (vis == 54) { m0 = 1; m1 = 5; }
			if (vis == 57) { m0 = 2; m1 = 6; }
			if (vis == 58) { m0 = 3; m1 = 7; }
			for(i=nlin*4/8;i>=nlin*3/8;i--)
			{
				c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
				s = sin((float)i*PI*2.0/(float)nlin)*r;
				for(j=0;j<n2;j++)
				{
					c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
					s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
					for(m=0;m<2;m++)
						for(k=3-1;k>=0;k--)
							drawlin(x,y,z,r-c*c0,s,r-c*s0,
											  r-c*c1,s,r-c*s1,dirlut2[(m1-m0)*m+m0][k],col);
				}
			}
		}
	}
	else if (n == 5) //6 half planes
	{
		for(i=nlin/4;i<=nlin*2/4;i++)
		{
			c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
			s = sin((float)i*PI*2.0/(float)nlin)*r;
			for(j=0;j<n2;j++)
			{
				c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
				s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
				for(g=-1.f;g<=1.f;g+=2.f)
					for(f=-1.f;f<=1.f;f+=2.f)
						drawlin(x,y,z,(r-c*c0)*f,s,(r-c*s0)*g,
										  (r-c*c1)*f,s,(r-c*s1)*g,dir,col);
			}
		}
		static int mv[4];
		if (vis == 31) { mv[0] = 0; mv[1] = 1; mv[2] = 2; mv[3] = 3; }
		if (vis == 47) { mv[0] = 4; mv[1] = 5; mv[2] = 6; mv[3] = 7; }
		if (vis == 55) { mv[0] = 0; mv[1] = 1; mv[2] = 4; mv[3] = 5; }
		if (vis == 59) { mv[0] = 2; mv[1] = 3; mv[2] = 6; mv[3] = 7; }
		if (vis == 61) { mv[0] = 0; mv[1] = 2; mv[2] = 4; mv[3] = 6; }
		if (vis == 62) { mv[0] = 1; mv[1] = 3; mv[2] = 5; mv[3] = 7; }
		for(i=nlin*4/8;i>=nlin*3/8;i--)
		{
			c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
			s = sin((float)i*PI*2.0/(float)nlin)*r;
			for(j=0;j<n2;j++)
			{
				c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
				s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
				for(m=0;m<4;m++)
					for(k=3-1;k>=0;k--)
						drawlin(x,y,z,r-c*c0,s,r-c*s0,
										  r-c*c1,s,r-c*s1,dirlut2[mv[m]][k],col);
			}
		}
	}
	else if (n == 6) //1 full axes
	{
		for(i=nlin*4/8;i>=nlin*3/8;i--)
		{
			c = cos((float)i*PI*2.0/(float)nlin)*r; c += r;
			s = sin((float)i*PI*2.0/(float)nlin)*r;
			for(j=0;j<n2;j++)
			{
				c0 = cos((float)j*PI/2.0/(float)n2); c1 = cos((float)(j+1)*PI/2.0/(float)n2);
				s0 = sin((float)j*PI/2.0/(float)n2); s1 = sin((float)(j+1)*PI/2.0/(float)n2);
				for(m=0;m<8;m++)
					for(k=3-1;k>=0;k--)
						drawlin(x,y,z,r-c*c0,s,r-c*s0,
										  r-c*c1,s,r-c*s1,dirlut2[m][k],col);
			}
		}
	}
}

static void drawbird (float x, float y, float z, float hang, float vang, float tang, float sc, int col)
{
	point3d pp, rr, dd, ff;
	float g;
	int i;

	for(i=-2;i<=2;i++)
	{
		pp.x = x;
		pp.y = y;
		pp.z = z;
		rr.x =-sin(hang); ff.x = cos(hang)*cos(vang);
		rr.y = cos(hang); ff.y = sin(hang)*cos(vang);
		rr.z = 0;         ff.z =           sin(vang);
		dd.x = ff.y*rr.z - ff.z*rr.y;
		dd.y = ff.z*rr.x - ff.x*rr.z;
		dd.z = ff.x*rr.y - ff.y*rr.x;

		rotate_vex(tang*(float)i,&rr,&dd);

		switch(labs(i))
		{
			case 0: g = .80; break;
			case 1: g = .18; break;
			case 2: pp.z += tang*-.40*sc;
					  g = fabs(sin(tang))*.02*sc;
					  pp.x += rr.x*g*(float)i;
					  pp.y += rr.y*g*(float)i;
					  pp.z += rr.z*g*(float)i;
					  g = .30; break;
		}

		g *= sc;

		if (i > 0) { rr.x = -rr.x; rr.y = -rr.y; rr.z = -rr.z; }
		rr.x *= g; rr.y *= g; rr.z *= g;
		dd.x *= g; dd.y *= g; dd.z *= g;
		ff.x *= g; ff.y *= g; ff.z *= g;

		switch(labs(i))
		{
			case 0: voxie_drawspr(&vf, "torso.kv6",&pp,&rr,&dd,&ff,col); break;
			case 1: voxie_drawspr(&vf,"wingl1.kv6",&pp,&rr,&dd,&ff,col); break;
			case 2: voxie_drawspr(&vf,"wingl2.kv6",&pp,&rr,&dd,&ff,col); break;
		}
	}
}

static float dist2plat2 (float px, float py, float pz, float x, float y, float z, float hx, float hy)
{
	float d, f, xx, yy, zz, hz;

	hz = 0.02f;
	xx = min(max(px,x-hx),x+hx);
	yy = min(max(py,y-hy),y+hy);
	zz = min(max(pz,z-hz),z+hz);
	d = (px-xx)*(px-xx) + (py-yy)*(py-yy) + (pz-zz)*(pz-zz);
	if (pz > z)
	{
		f = max(sqrt((px-x)*(px-x) + (py-y)*(py-y))-.05f,0.f);
		d = min(d,f*f);
	}
	return(d);
}

static void calcluts (void)
{
	int i, j;

	j = vw.framepervol*24;
	for(i=0;i<j;i++) //WARNING:do not reverse for loop!
	{
		clutmid[i] = (int)(cos((double)(i+.5)*PI*2.0/(float)j)*-256.0);
		div3[i] = i/3;
		div24[i] = (div3[i]>>3);
		mod3[i] = i%3;
		mod24[i] = i%24;
		oneupmod24[i] = (1<<mod24[i]);
	}
}

static void loadini (void)
{
	float f, animdefscale = 0.5f;
	int i, j, oficnt, ficnt, fileng, gotmun = 0, keymenucnt = 0;
	char *fibuf, och, *cptr, *eptr, ktid[] = "[VOXIEDEMO]\r\n";

	if (!kzopen("voxiedemo.ini")) return;
	fileng = kzfilelength();
	fibuf = (char *)malloc(fileng+1); if (!fibuf) { kzclose(); return; }
	kzread(fibuf,fileng);
	kzclose();

		//Preprocessor (#include)
	for(ficnt=0;ficnt<fileng;ficnt++)
	{
		oficnt = ficnt;
		cptr = &fibuf[ficnt];
		for(;ficnt<fileng;ficnt++) if (fibuf[ficnt] == '\r') break;
		och = fibuf[ficnt]; fibuf[ficnt] = 0;

		if (!memcmp(cptr,"#include ",9))
		{
			cptr += 9;
			if (cptr[0] == '<') { cptr++; for(i=0;cptr[i];i++) if (cptr[i] == '>') { cptr[i] = 0; break; } } //Strip <>
			if (cptr[0] == 34) { cptr++; for(i=0;cptr[i];i++) if (cptr[i] == 34) { cptr[i] = 0; break; } } //Strip ""
			if (kzopen(cptr))
			{
				int fileng2;

				ficnt++; memmove(&fibuf[oficnt],&fibuf[ficnt],fileng-ficnt); fileng += oficnt-ficnt; ficnt = oficnt; //remove #include line

				fileng2 = kzfilelength();
				fibuf = (char *)realloc(fibuf,fileng+fileng2+1); if (!fibuf) { kzclose(); return; }
				memmove(&fibuf[oficnt+fileng2],&fibuf[oficnt],fileng-ficnt); //shift file memory up
				kzread(&fibuf[oficnt],fileng2); //insert included file
				kzclose();

				fileng += fileng2;
			}
		} else { fibuf[ficnt] = och; }

		while ((ficnt < fileng) && (fibuf[ficnt] < 32)) { ficnt++; } ficnt--;
	}

	for(ficnt=0;ficnt<fileng;ficnt++) if (!memcmp(&fibuf[ficnt],ktid,strlen(ktid))) break;
	for(;ficnt<fileng;ficnt++)
	{
		cptr = &fibuf[ficnt];
		for(;ficnt<fileng;ficnt++) if (fibuf[ficnt] == '\r') break;
		och = fibuf[ficnt]; fibuf[ficnt] = 0;

		while (1)
		{
			eptr = 0;
			if (!memcmp(cptr,"rendmode="   , 9)) grendmode = min(max(strtol(&cptr[9],&eptr,0),0),RENDMODE_END-1);
			if (!memcmp(cptr,"showborder=" ,11)) gshowborder = min(max(strtol(&cptr[11],&eptr,0),0),1);
			if (!memcmp(cptr,"menutimeout=",12)) gmenutimeout = max(strtol(&cptr[12],&eptr,0),-1);
			if (!memcmp(cptr,"mountzip="   , 9)) voxie_mountzip(&cptr[9]);
			if (!memcmp(cptr,"mediadir="   , 9)) { i = (int)strlen(&cptr[9]); gmediadir = (char *)malloc(i+1); memcpy(gmediadir,&cptr[9],i+1); voxie_mountzip(gmediadir); }

			if (!memcmp(cptr,"recfile="    , 8)) { if (grecn < GRECMAX) { sprintf(grec[grecn].file,"%.*s",sizeof(grec[0].file)-1,&cptr[8]); grec[grecn].mode = 0; grec[grecn].rep = 1; grecn++; } }
			if (!memcmp(cptr,"recmode="    , 8)) { if (grecn) grec[grecn-1].mode = min(max(strtol(&cptr[8],&eptr,0),0),2); }
			if (!memcmp(cptr,"recrep="     , 7)) { if (grecn) grec[grecn-1].rep = max(strtol(&cptr[7],&eptr,0),0); }

			if (!memcmp(cptr,"demfile="    , 8))
			{
				if (gdemn < GDEMMAX)
				{
					sprintf(gdem[gdemn].file,"%.*s",sizeof(gdem[0].file)-1,&cptr[8]);
					f = 2.f/max(max(vw.aspx,vw.aspy),vw.aspz);
					gdem[gdemn].r.x = f*2.f; gdem[gdemn].d.x = 0.f; gdem[gdemn].f.x = 0.f;
					gdem[gdemn].r.y = 0.f; gdem[gdemn].d.y = f*2.f; gdem[gdemn].f.y = 0.f;
					gdem[gdemn].r.z = 0.f; gdem[gdemn].d.z = 0.f; gdem[gdemn].f.z =-.4; //vw.aspz*f;
					gdem[gdemn].p.x = gdem[gdemn].r.x*-.5f;
					gdem[gdemn].p.y = gdem[gdemn].d.y*-.5f;
					gdem[gdemn].p.z = gdem[gdemn].f.z*-.5f;

					gdem[gdemn].colorkey = 0x80ff00ff;
					gdem[gdemn].usesidebyside = 0;
					gdem[gdemn].mapzen = 0;
					gdemn++;
				}
			}
			if (!memcmp(cptr,"demcolorkey=",12))   { if (gdemn) gdem[gdemn-1].colorkey      = strtoul(&cptr[12],&eptr,0); }
			if (!memcmp(cptr,"demscalex="  ,10))   { if (gdemn) { f = strtod(&cptr[10],&eptr); gdem[gdemn-1].r.x *= f; gdem[gdemn-1].p.x = gdem[gdemn-1].r.x*-.5f; } }
			if (!memcmp(cptr,"demscaley="  ,10))   { if (gdemn) { f = strtod(&cptr[10],&eptr); gdem[gdemn-1].d.y *= f; gdem[gdemn-1].p.y = gdem[gdemn-1].d.y*-.5f; } }
			if (!memcmp(cptr,"demscalez="  ,10))   { if (gdemn) { f = strtod(&cptr[10],&eptr); gdem[gdemn-1].f.z *= f; gdem[gdemn-1].p.z = gdem[gdemn-1].f.z*-.5f; } }
			if (!memcmp(cptr,"demscale="   , 9))   { if (gdemn) { f = strtod(&cptr[ 9],&eptr); gdem[gdemn-1].f.z *= f; gdem[gdemn-1].p.z = gdem[gdemn-1].f.z*-.5f; } }
			if (!memcmp(cptr,"demsidebyside=",14)) { if (gdemn) gdem[gdemn-1].usesidebyside = min(max(strtol(&cptr[14],&eptr,0),0),1); }
			if (!memcmp(cptr,"demmapzen="  ,10))   { if (gdemn) gdem[gdemn-1].mapzen        = min(max(strtol(&cptr[10],&eptr,0),0),1); }

			if (!memcmp(cptr,"mystring="   , 9))   { sprintf(gmyst,"%.*s",sizeof(gmyst)-1,&cptr[9]); }

			if (!memcmp(cptr,"chesscol1="  ,10))   { gchesscol[0] = strtoul(&cptr[10],&eptr,0); }
			if (!memcmp(cptr,"chesscol2="  ,10))   { gchesscol[1] = strtoul(&cptr[10],&eptr,0); }
			if (!memcmp(cptr,"chessailev1=",12))   { gchessailev[0] = min(max(strtoul(&cptr[12],&eptr,0),0),8); }
			if (!memcmp(cptr,"chessailev2=",12))   { gchessailev[1] = min(max(strtoul(&cptr[12],&eptr,0),0),8); }
			if (!memcmp(cptr,"chesstime="  ,10))   { gchesstime = strtod(&cptr[10],&eptr); }

			if (!memcmp(cptr,"minplays="   , 9))   { gminplays = min(max(strtol(&cptr[9],&eptr,0),1),4); }
			if (!memcmp(cptr,"dualnav="    , 8))   { gdualnav  = min(max(strtol(&cptr[8],&eptr,0),0),1); }

			if (!memcmp(cptr,"pakrendmode=",12))   { gpakrendmode    = max(strtol(&cptr[12],&eptr,0),0); }
			if (!memcmp(cptr,"pakxdim="    , 8))   { gpakxdim        = max(strtol(&cptr[8],&eptr,0),0); }
			if (!memcmp(cptr,"pakydim="    , 8))   { gpakydim        = max(strtol(&cptr[8],&eptr,0),0); }

			if (!memcmp(cptr,"munxsiz="    , 8))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].xsiz = max(strtol(&cptr[8],&eptr,0),0); }
			if (!memcmp(cptr,"munysiz="    , 8))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].ysiz = max(strtol(&cptr[8],&eptr,0),0); }
			if (!memcmp(cptr,"munzsiz="    , 8))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].zsiz = max(strtol(&cptr[8],&eptr,0),0); }
			if (!memcmp(cptr,"munxwrap="   , 9))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].xwrap = min(max(strtol(&cptr[9],&eptr,0),0),1); }
			if (!memcmp(cptr,"munywrap="   , 9))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].ywrap = min(max(strtol(&cptr[9],&eptr,0),0),1); }
			if (!memcmp(cptr,"munzwrap="   , 9))   { if (!gotmun) { gotmun = 1; munn = 0; memset(gmun,0,sizeof(gmun)); } if (munn < MUNMAX) gmun[munn].zwrap = min(max(strtol(&cptr[9],&eptr,0),0),1); }
			if (!memcmp(cptr,"munboard="   , 9)) //scan ini for non-white chars until munxsiz*munysiz*munzsiz exhausted (or file ends)
			{
				if (munn < MUNMAX)
				{
					int nbytes;

					nbytes = min(max(gmun[munn].xsiz*gmun[munn].ysiz*gmun[munn].zsiz,1),1048576);
					gmun[munn].board = (char *)malloc(nbytes);

					j = 0;
					for(i=ficnt;i<fileng;i++)
					{
						if (fibuf[i] <= 32) continue;
						gmun[munn].board[j] = fibuf[i]; j++; if (j >= nbytes) break;
					}
					munn++;

					fibuf[ficnt] = och; ficnt = i; och = fibuf[ficnt]; //hack to move outer loop file pointer up
					break;
				}
			}

			if (!memcmp(cptr,"snaketime="  ,10))   { gsnaketime      = max(strtol(&cptr[10],&eptr,0),0); }
			if (!memcmp(cptr,"snakenumpels=",13))  { gsnakenumpels   = max(strtol(&cptr[13],&eptr,0),0); }
			if (!memcmp(cptr,"snakenumtetras=",15)){ gsnakenumtetras = max(strtol(&cptr[15],&eptr,0),0); }
			if (!memcmp(cptr,"snakepelspeed=",14)) { gsnakepelspeed  = strtod(&cptr[14],&eptr); }
			if ((!memcmp(cptr,"snakecol",8)) && (cptr[8] >= '1') && (cptr[8] <= '4') && (cptr[9] == '=')) { gsnakecol[(int)(cptr[8]-'1')]  = strtoul(&cptr[10],&eptr,0); }

			if (!memcmp(cptr,"animfile="   , 9))
			{
				if (ganimn < GANIMMAX)
				{
					sprintf(ganim[ganimn].file,"%.*s",sizeof(ganim[0].file)-1,&cptr[9]);
					ganim[ganimn].snd[0] = 0;
					f = animdefscale;
					ganim[ganimn].defscale = f;
					ganim[ganimn].p.x = 0.f; ganim[ganimn].p_init.x = 0.f;
					ganim[ganimn].p.y = 0.f; ganim[ganimn].p_init.y = 0.f;
					ganim[ganimn].p.z = 0.f; ganim[ganimn].p_init.z = 0.f;
					ganim[ganimn].ang[0] = 0.f; ganim[ganimn].ang_init[0] = 0.f;
					ganim[ganimn].ang[1] = 0.f; ganim[ganimn].ang_init[1] = 0.f;
					ganim[ganimn].ang[2] = 0.f; ganim[ganimn].ang_init[2] = 0.f;
					ganim[ganimn].sc = f;
					ganim[ganimn].fps = 2.0f;
					ganim[ganimn].tt = 0;
					ganim[ganimn].filetyp = 0; //0=PNG, 1=KV6/STL,etc..
					ganim[ganimn].mode = 0; //0=forward, 1=pingpong
					ganim[ganimn].n = 0;
					ganim[ganimn].mal = 0;
					ganim[ganimn].cnt = 0;
					ganim[ganimn].loaddone = 0;
					ganimn++;
				}
			}
			if (!memcmp(cptr,"animmode="   , 9)) { if (ganimn) ganim[ganimn-1].mode = min(max(strtol(&cptr[9],&eptr,0),0),1); }
			if (!memcmp(cptr,"animsnd="    , 8)) { if (ganimn) sprintf(ganim[ganimn-1].snd,"%.*s",sizeof(ganim[0].snd)-1,&cptr[8]); }
			if (!memcmp(cptr,"animfps="    , 8)) { if (ganimn) ganim[ganimn-1].fps = strtod(&cptr[8],&eptr); }
			if (!memcmp(cptr,"animscale="  ,10))
			{
				f = strtod(&cptr[10],&eptr);
				if (!ganimn) animdefscale = f;
				else
				{
					ganim[ganimn-1].defscale = f;
					ganim[ganimn-1].sc = f;
				}
			}

			if (!memcmp(cptr,"music_rendmode_",15))
			{
				i = strtol(&cptr[15],&eptr,0);
				for(j=0;j<gmusn;j++)
					if (i == gmus[j].rendmode) break;
				if (j < GMUSMAX)
				{
					gmus[j].rendmode = i;
					sprintf(gmus[j].file,"%.*s",sizeof(gmus[0].file)-1,&eptr[1]);
					if (j == gmusn) gmusn++;
				}
			}

			if (!memcmp(cptr,"key_",4))
			{
				if ((cptr[4] >= 'p') && (cptr[4] <= 'q') && (cptr[5] >= '1') && (cptr[5] <= '4'))
				{
					i = (cptr[4]-'p') + (cptr[5]-'1')*2;
					j = -1;
						  if (cptr[6] == 'l') j = 0;
					else if (cptr[6] == 'r') j = 1;
					else if (cptr[6] == 'u') j = 2;
					else if (cptr[6] == 'd') j = 3;
					else if (cptr[6] == 'a') j = 4;
					else if (cptr[6] == 'b') j = 5;
					else if (cptr[6] == 'm') j = 6;
					if (j >= 0)
					{
						if (cptr[7] == '=') keyremap[i][j] = strtoul(&cptr[8],&eptr,0);
					}
				}
				else if (!memcmp(&cptr[4],"menu=",5))
				{
					if (keymenucnt < 2)
					{
						keymenu[keymenucnt] = strtoul(&cptr[9],&eptr,0);
						keymenucnt++;
					}
				}
			}
			if (!eptr) break;
			cptr = eptr;

			while ((cptr[0] == 9) || (cptr[0] == 32)) cptr++;
			if (cptr[0] != ';') break;
			cptr++;
			while ((cptr[0] == 9) || (cptr[0] == 32)) cptr++;
			if ((cptr[0] == '/') && (cptr[1] == '/')) break;
			if (cptr[0] < 32) break;
		}

		fibuf[ficnt] = och;
		while ((ficnt < fileng) && (fibuf[ficnt] < 32)) { ficnt++; } ficnt--;
	}
	free(fibuf);

	for(i=gdemn-1;i>=0;i--)
	{
		gdem[i].sp = gdem[i].p;
		gdem[i].sr = gdem[i].r;
		gdem[i].sd = gdem[i].d;
		gdem[i].sf = gdem[i].f;
	}
}

int cmdline2arg (char *cmdline, char **argv)
{
	int i, j, k, inquote, argc;

		//Convert Windows command line into ANSI 'C' command line...
	argv[0] = (char *)"exe"; argc = 1; j = inquote = 0;
	for(i=0;cmdline[i];i++)
	{
		k = (((cmdline[i] != ' ') && (cmdline[i] != '\t')) || (inquote));
		if (cmdline[i] == '\"') inquote ^= 1;
		if (j < k) { argv[argc++] = &cmdline[i+inquote]; j = inquote+1; continue; }
		if ((j) && (!k))
		{
			if ((j == 2) && (cmdline[i-1] == '\"')) cmdline[i-1] = 0;
			cmdline[i] = 0; j = 0;
		}
	}
	if ((j == 2) && (cmdline[i-1] == '\"')) cmdline[i-1] = 0;
	argv[argc] = 0;
	return(argc);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
#if (USELEAP)
	LEAP_TRACKING_EVENT *frame;
	LEAP_HAND *hand;
	LEAP_PALM *palm;
	LEAP_DIGIT *digit;
	LEAP_VECTOR *vec;
	point3d palmp, palmr, palmd, palmf;
#endif
	voxie_inputs_t in;
	voxie_xbox_t vx[4] = {0};
	int ovxbut[4], vxnplays;
	voxie_nav_t nav[4] = {0};
	int onavbut[4];
	double tim = 0.0, otim, dtim, avgdtim = 0.0, cycletim = 0.0;
	point3d m6p, m6r, m6d, m6f, m6ru, m6du, m6fu; int m6but, m6cnt;
	float f, g, fx, fy, fz, fr, gx, gy, gz, x0, y0, z0, x1, y1, z1;
	int obstatus, bstatus = 0, numframes = 0, argc, argfilindex[2];
	int i, j, k, h, n, x, y, z, xx, yy, col, *wptr, *rptr, showphase, orendmode = -17;
	char *argv[MAX_PATH>>1], tbuf[MAX_PATH];

#if (USELEAP)
	leap_init();
#endif
#if (USEMAG6D)
	gusemag6d = mag6d_init();
#endif

	platonic_init();

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); }

	loadini(); //Load settings from voxiedemo.ini

	argc = cmdline2arg(cmdline,argv); argfilindex[0] = -1; argfilindex[1] = -1;
	for(i=1;i<argc;i++)
	{
		if ((argv[i][0] != '-') && (argv[i][0] != '/'))
		{
			if (argfilindex[0] < 0) { argfilindex[0] = i; continue; }
			if (argfilindex[1] < 0) { argfilindex[1] = i; continue; }
			continue;
		}
		if ((argv[i][1] == 'R') || (argv[i][1] == 'r')) { grendmode    = min(max(atol(&argv[i][2]), 0  ), RENDMODE_END-1); continue; }
		if ((argv[i][1] == 'E') || (argv[i][1] == 'e')) { vw.useemu    = min(max(atol(&argv[i][2]), 0  ),  1); vw.usekeystone = !vw.useemu; continue; }
		if ((argv[i][1] == 'C') || (argv[i][1] == 'c')) { vw.usecol    = min(max(atol(&argv[i][2]),-6  ),  1); continue; }
		if ((argv[i][1] == 'Z') || (argv[i][1] == 'z')) { vw.aspz      = min(max(atof(&argv[i][2]),.01f),1.f); continue; }
	}

	calcluts();

	if ((vw.nblades > 0) && (vw.aspx == vw.aspy)) vw.clipshape = 1;
	voxie_init(&vw); //Start video and audio.

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;
		obstatus = bstatus; fx = in.dmousx; fy = in.dmousy; fz = in.dmousz; bstatus = in.bstat;
		for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;
#if (!USEMAG6D)
			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1
#else
			memset(&vx[vxnplays],0,sizeof(vx[0]));
#endif
		}

		while (i = voxie_keyread())
		{
			if (((((i>>8)&255) == 0x3e) && (i&0x300000)) ||
				 ((((i>>8)&255) == 0x01) && (i&0x030000))) { voxie_quitloop(); break; } //Alt+F4 or Shift+ESC: quit
			if ((((i>>8)&255) == 0x1e) && (i&0xc0000)) { gautocycle = !gautocycle; cycletim = tim+5.0; continue; } //Ctrl+A
			//if ((((i>>8)&255) == 0x13) && (i&0xc0000)) { gautorotatespd = (!gautorotatespd)*5; continue; } //Ctrl+R
			if ((((i>>8)&255) == 0x13) && (i&0xc0000)) { gautorotatespd[2] = (!gautorotatespd[2])*5; continue; } //Ctrl+R
			if ((((i>>8)&255) == keymenu[0]) || (((i>>8)&255) == keymenu[1]))
			{
				if (grendmode != RENDMODE_TITLE) { gcurselmode = grendmode-1; grendmode = RENDMODE_TITLE; }
				gtimbore = tim+gmenutimeout;
			}

			switch(i&255)
			{
				case 27: //ESC: quit immediately
					if (((i>>8)&255) == 0x01)
					{
						if (((keymenu[0] == 0x01) || (keymenu[1] == 0x01)) && (!(i&0x30000))) break; //If menu key is ESC, require Shift+ESC to quit
						voxie_quitloop();
					}
					break;
				default:
					if (grendmode == RENDMODE_FLYINGSTUFF)
					{
						if ((i&255) == 8) { if (gmyst[0]) gmyst[strlen(gmyst)-1] = 0; } //Handle backspace
						else if (strlen(gmyst) < sizeof(gmyst)-1) sprintf(&gmyst[strlen(gmyst)],"%c",i&255);
					}
					break;
			}
		}

		if ((vx[0].but&~ovxbut[0])&(1<<5)) grendmode = RENDMODE_TITLE; //Back button

		if (grendmode != orendmode)
		{
			switch(grendmode)
			{
				case RENDMODE_TITLE:
					genpath_voxiedemo_media("voxiedemo_title.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_title_update,0,tbuf);
					voxie_menu_addtab("Select..",350,0,650,400);
					voxie_menu_additem("Up"    ,256, 42,128,96,MENU_UP   ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Left"  , 96,162,128,96,MENU_LEFT ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Select",256,162,128,96,MENU_ENTER,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Right" ,416,162,128,96,MENU_RIGHT,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Down"  ,256,282,128,96,MENU_DOWN ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_PHASESYNC:
					genpath_voxiedemo_media("voxiedemo_phasesync.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_phasesync_update,0,tbuf);
					voxie_menu_addtab("PhaseSync",350,0,650,400);

					voxie_menu_additem("Wireframe:",114, 96,128, 64,0              ,MENU_TEXT    ,0                    ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Zigzag"    ,244, 40,128,128,MENU_ZIGZAG    ,MENU_BUTTON+1,gphasesync_curmode==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Sine"      ,362, 40,128,128,MENU_SINE      ,MENU_BUTTON+2,gphasesync_curmode!=0,0x908070,0.0,0.0,0.0,0.0,0.0);

					i = (gphasesync_cursc == 1)*1 +
						 (gphasesync_cursc == 2)*2 +
						 (gphasesync_cursc == 4)*3 +
						 (gphasesync_cursc == 8)*4 +
						 (gphasesync_cursc ==16)*5;
					voxie_menu_additem("Hump Level",128,224,360, 64,MENU_HUMP_LEVEL,MENU_HSLIDER ,0                    ,0x908070,(double)i,1.0,5.0,1.0,1.0);

					break;

				case RENDMODE_KEYSTONECAL:
					genpath_voxiedemo_media("voxiedemo_keystonecal.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_keystonecal_update,&keystone,tbuf);
					voxie_menu_addtab("Keystone",350,0,650,400);

					voxie_menu_additem("Drag 8 corners to calibrate keystone.", 32, 16, 32, 64,0 ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					if (vw.nblades > 0) voxie_menu_additem("Drag middle of grid to align rotation axis." , 32, 36, 32, 64,0 ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("(Save button under File Menu)"         ,32, 56, 32, 64,0 ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Put Cursor on..", 64, 92,128, 64,0              ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Ceiling"        , 90,110, 96,80,MENU_CEILING   ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Floor"          , 90,200, 96,80,MENU_FLOOR     ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Rotate keystone..",350, 92,128, 64,0           ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Left 5"           ,320,110,96,80,MENU_ROTLEFT5 ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Right 5"          ,430,110,96,80,MENU_ROTRIGHT5,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Left .1"          ,320,200,96,80,MENU_ROTLEFT ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Right .1"         ,430,200,96,80,MENU_ROTRIGHT,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);

					if (vw.dispnum > 1)
					{
						voxie_menu_additem("\bSelect\nDisplay",10,332, 64, 64,0                   ,MENU_TEXT    ,0                                 ,0x908070, 0.0,0.0,0.0,0.0,0.0);
						for(i=0;i<vw.dispnum;i++)
						{
							tbuf[0] = i+'1'; tbuf[1] = 0;
							voxie_menu_additem(tbuf ,i*64+136,320, 64, 64,MENU_DISP_CUR+i     ,MENU_BUTTON+(i==0),(i==vw.dispcur)&&(!grabdispall),0x908070, 0.0,0.0,0.0,0.0,0.0);
						}
						voxie_menu_additem("All",i*64+136,320, 64, 64,MENU_DISP_ALL       ,MENU_BUTTON+2,grabdispall!=0,0x908070, 0.0,0.0,0.0,0.0,0.0);
					}

					break;

				case RENDMODE_LOCALCAL:
					genpath_voxiedemo_media("voxiedemo_wavyplane.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(0,0,tbuf);
					break;

				case RENDMODE_HEIGHTMAP:
					genpath_voxiedemo_media("voxiedemo_heimap.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_heightmap_update,0,tbuf);
					voxie_menu_addtab("HeightMap",350,0,650,400);

					voxie_menu_additem("Prev"         , 34, 32,280, 64,MENU_PREV          ,MENU_BUTTON+3,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Next"         ,338, 32,280, 64,MENU_NEXT          ,MENU_BUTTON+3,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("AutoCycle"    , 48,148, 72, 64,0                  ,MENU_TEXT    ,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"          ,168,122, 72, 64,MENU_AUTOCYCLEOFF  ,MENU_BUTTON+1,(gautocycle==0)        ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"           ,240,122, 72, 64,MENU_AUTOCYCLEON   ,MENU_BUTTON+2,(gautocycle!=0)        ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Texel Filter" , 14,236, 72, 64,0                  ,MENU_TEXT    ,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Near"         ,168,210, 72, 64,MENU_TEXEL_NEAREST ,MENU_BUTTON+1,(gheightmap_flags&2)==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Bilin"        ,240,210, 72, 64,MENU_TEXEL_BILINEAR,MENU_BUTTON+2,(gheightmap_flags&2)!=0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Slice Dither" ,320,148, 72, 64,0                  ,MENU_TEXT    ,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"          ,474,122, 72, 64,MENU_SLICEDITHEROFF,MENU_BUTTON+1,(gheightmap_flags&1)==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"           ,546,122, 72, 64,MENU_SLICEDITHERON ,MENU_BUTTON+2,(gheightmap_flags&1)!=0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Texture"      ,332,236, 72, 64,0                  ,MENU_TEXT    ,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"          ,474,210, 72, 64,MENU_TEXTUREOFF    ,MENU_BUTTON+1,(gheightmap_flags&4)==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"           ,546,210, 72, 64,MENU_TEXTUREON     ,MENU_BUTTON+2,(gheightmap_flags&4)!=0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Reset camera" , 32,304,586, 64,MENU_RESET         ,MENU_BUTTON+3,0                      ,0x908070,0.0,0.0,0.0,0.0,0.0);

					break;

				case RENDMODE_VOXIEPLAYER:
					genpath_voxiedemo_media("voxiedemo_playrec.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_voxieplayer_update,0,tbuf);
					voxie_menu_addtab("Play .REC",350,0,650,430);

					voxie_menu_additem("Prev"           ,148, 32,160, 96,MENU_PREV        ,MENU_BUTTON+3,0               ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Next"           ,340, 32,160, 96,MENU_NEXT        ,MENU_BUTTON+3,0               ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Draw stats"     ,173,179, 64, 64,0                ,MENU_TEXT    ,0               ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"            ,307,153, 64, 64,MENU_DRAWSTATSOFF,MENU_BUTTON+1,gdrawstats==0   ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"             ,371,153, 64, 64,MENU_DRAWSTATSON ,MENU_BUTTON+2,gdrawstats==1   ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Cycle all Demos",113,267, 64, 64,0                ,MENU_TEXT    ,0               ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("No"             ,307,243, 64, 64,MENU_AUTOCYCLEOFF,MENU_BUTTON+1,gautocycleall==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Yes"            ,371,243, 64, 64,MENU_AUTOCYCLEON ,MENU_BUTTON+2,gautocycleall!=0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Pause" , 32,334,586,64,MENU_PAUSE  ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_PLATONICSOLIDS:
					genpath_voxiedemo_media("voxiedemo_platonics.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_platonic_update,0,tbuf);
					voxie_menu_addtab("Platonics",350,0,650,400);

					voxie_menu_additem("Solid Mode:", 32, 32,128,64,0        ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Vertices"  , 32, 64,128,64,MENU_SOL0,MENU_BUTTON+1,platonic_solmode==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Wireframe" ,184, 64,128,64,MENU_SOL1,MENU_BUTTON  ,platonic_solmode==1,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Surfaces"  ,336, 64,128,64,MENU_SOL2,MENU_BUTTON  ,platonic_solmode==2,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Solid"     ,488, 64,128,64,MENU_SOL3,MENU_BUTTON+2,platonic_solmode==3,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Color:"   , 32,160,128,64,0        ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 0*85+32,192,75, 64,MENU_WHITE  ,MENU_BUTTON+1,platonic_col==7,0xc0c0c0, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 1*85+32,192,75, 64,MENU_RED    ,MENU_BUTTON  ,platonic_col==4,0xc04040, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 2*85+32,192,75, 64,MENU_GREEN  ,MENU_BUTTON  ,platonic_col==2,0x40c040, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 3*85+32,192,75, 64,MENU_BLUE   ,MENU_BUTTON  ,platonic_col==1,0x4040c0, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 4*85+32,192,75, 64,MENU_CYAN   ,MENU_BUTTON  ,platonic_col==3,0x40c0c0, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 5*85+32,192,75, 64,MENU_MAGENTA,MENU_BUTTON  ,platonic_col==5,0xc040c0, 0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("", 6*85+32,192,75, 64,MENU_YELLOW ,MENU_BUTTON+2,platonic_col==6,0xc0c040, 0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Pause" , 32,304,586,64,MENU_PAUSE  ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);

					break;

				case RENDMODE_FLYINGSTUFF:
					genpath_voxiedemo_media("voxiedemo_flystuff.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(0,0,tbuf);
					break;

				case RENDMODE_CHESS:
					genpath_voxiedemo_media("voxiedemo_chess.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_chess_update,0,tbuf);
					voxie_menu_addtab("Chess",350,0,650,410);
					voxie_menu_additem("Hint"      , 32, 32,586, 64,MENU_HINT       ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Auto move" ,172,156,128, 64,0               ,MENU_TEXT    ,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"       ,300,132, 64, 64,MENU_AUTOMOVEOFF,MENU_BUTTON+1,gchess_automove==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"        ,364,132, 64, 64,MENU_AUTOMOVEON ,MENU_BUTTON+2,gchess_automove!=0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Difficulty",128,222,400, 64,MENU_DIFFICULTY ,MENU_HSLIDER ,0                 ,0x908070,(double)gchessailev[1],1.0,6.0,1.0,1.0);
					voxie_menu_additem("Reset game", 32,324,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_PACKER:
					genpath_voxiedemo_media("voxiedemo_packer.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_packer_update,0,tbuf);
					voxie_menu_addtab("Packer",350,0,650,410);

						//Render mode: 1:line,2:thickline,4:surfs,6:surf&thickline
					voxie_menu_additem("Render Mode:",248, 22,128,64,0        ,MENU_TEXT    ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Thin line"   , 42, 54,128,64,MENU_SOL0,MENU_BUTTON+1,platonic_solmode==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Thick line"  ,186, 54,128,64,MENU_SOL1,MENU_BUTTON  ,platonic_solmode==1,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Surfaces"    ,330, 54,128,64,MENU_SOL2,MENU_BUTTON  ,platonic_solmode==2,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("All"         ,474, 54,128,64,MENU_SOL3,MENU_BUTTON+2,platonic_solmode==3,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Board Y--"    ,256,152,128,64,MENU_UP   ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Board X--"    ,116,192,128,64,MENU_LEFT ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Board X++"    ,396,192,128,64,MENU_RIGHT,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Board Y++"    ,256,232,128,64,MENU_DOWN ,MENU_BUTTON+3,0,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Reset game", 32,324,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_PARATROOPER:
					genpath_voxiedemo_media("voxiedemo_paratrooper.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_paratrooper_update,0,tbuf);
					voxie_menu_addtab("P.Trooper",350,0,650,400);
					voxie_menu_additem("Reset game", 32,304,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_DOTMUNCH:
					genpath_voxiedemo_media("voxiedemo_dotmunch.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_dotmunch_update,0,tbuf);
					voxie_menu_addtab("DotMunch",350,0,650,400);

					voxie_menu_additem("Prev Level"         ,110, 32,200, 96,MENU_PREV        ,MENU_BUTTON+3,0            ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Next Level"         ,342, 32,200, 96,MENU_NEXT        ,MENU_BUTTON+3,0            ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Reset game", 32,304,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_SNAKETRON:
					genpath_voxiedemo_media("voxiedemo_snaketron.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_snaketron_update,0,tbuf);
					voxie_menu_addtab("SnakeTron",350,0,650,400);

						  if (gsnakepelspeed < 0.75f/16.f)i = 1;
					else if (gsnakepelspeed < 0.75f/8.f) i = 2;
					else if (gsnakepelspeed < 0.75f/4.f) i = 3;
					else if (gsnakepelspeed < 0.75f/2.f) i = 4;
					else if (gsnakepelspeed < 0.75f    ) i = 5;
					else if (gsnakepelspeed < 0.75f*2.f) i = 6;
					else                                 i = 7;
					voxie_menu_additem("Pellet speed",128,124,400, 64,MENU_SPEED     ,MENU_HSLIDER ,0                 ,0x908070,(double)i,1.0,7.0,1.0,1.0);

					voxie_menu_additem("Reset game", 32,304,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_FLYSTOMP:
					genpath_voxiedemo_media("voxiedemo_flystomp.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_flystomp_update,0,tbuf);
					voxie_menu_addtab("FlyStomp",350,0,650,400);
					voxie_menu_additem("Reset game", 32,304,586, 64,MENU_RESET      ,MENU_BUTTON+3,0                 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					break;

				case RENDMODE_MODELANIM:
					genpath_voxiedemo_media("voxiedemo_modelanim.jpg",tbuf,sizeof(tbuf));
					voxie_menu_reset(menu_modelanim_update,0,tbuf);
					voxie_menu_addtab("ModelAnim",200,0,800,430);

					voxie_menu_additem("Prev Model"      , 80, 19,138, 64,MENU_PREV         ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Next Model"      ,226, 19,138, 64,MENU_NEXT         ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Prev Frame"      ,440, 19,138, 64,MENU_FRAME_PREV   ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Next Frame"      ,586, 19,138, 64,MENU_FRAME_NEXT   ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Draw stats"      , 60,129, 64, 64,0                 ,MENU_TEXT    ,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"             ,188,103, 64, 64,MENU_DRAWSTATSOFF ,MENU_BUTTON+1,gdrawstats==0 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"              ,252,103, 64, 64,MENU_DRAWSTATSON  ,MENU_BUTTON+2,gdrawstats==1 ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Cross section"   , 22,209, 64, 64,0                 ,MENU_TEXT    ,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"             ,188,183, 64, 64,MENU_SLICEOFF     ,MENU_BUTTON+1,gslicemode==0 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"              ,252,183, 64, 64,MENU_SLICEON      ,MENU_BUTTON+2,gslicemode==1 ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("AutoCycle"       ,450,129, 64, 64,0                 ,MENU_TEXT    ,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"             ,568,103, 64, 64,MENU_AUTOCYCLEOFF ,MENU_BUTTON+1,gautocycle==0 ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("On"              ,632,103, 64, 64,MENU_AUTOCYCLEON  ,MENU_BUTTON+2,gautocycle==1 ,0x908070,0.0,0.0,0.0,0.0,0.0);

					voxie_menu_additem("Reset pos&ori"   , 92,265,256, 64,MENU_RESET        ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Pause"           , 92,345,256, 64,MENU_PAUSE        ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Load All pos&ori",458,265,256, 64,MENU_LOAD         ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Save All pos&ori",458,345,256, 64,MENU_SAVE         ,MENU_BUTTON+3,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);

#if 0
					voxie_menu_addtab("AutoRot.",400,0,600,280);
					voxie_menu_additem("Axis"            ,140, 59, 64, 64,0                 ,MENU_TEXT    ,0             ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Off"             ,218, 33, 64, 64,MENU_AUTOROTATEOFF,MENU_BUTTON+1,gautorotateax==-1,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("X"               ,282, 33, 64, 64,MENU_AUTOROTATEX  ,MENU_BUTTON  ,gautorotateax==0,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Y"               ,346, 33, 64, 64,MENU_AUTOROTATEY  ,MENU_BUTTON  ,gautorotateax==1,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Z"               ,410, 33, 64, 64,MENU_AUTOROTATEZ  ,MENU_BUTTON+2,gautorotateax==2,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("Speed"           ,128,134,360, 64,MENU_AUTOROTATESPD,MENU_HSLIDER ,0             ,0x908070,(double)gautorotatespd,0.0,10.0,1.0,1.0);
#else
					voxie_menu_addtab("AutoRot.",400,0,600,380);
					voxie_menu_additem("Rotation axis"   ,224, 32, 64, 64,0                 ,MENU_TEXT    ,0                ,0x908070,0.0,0.0,0.0,0.0,0.0);
					voxie_menu_additem("X"               ,128, 64,360, 64,MENU_AUTOROTATEX  ,MENU_HSLIDER ,0                ,0x908070,(double)gautorotatespd[0],0.0,10.0,1.0,1.0);
					voxie_menu_additem("Y"               ,128,160,360, 64,MENU_AUTOROTATEY  ,MENU_HSLIDER ,0                ,0x908070,(double)gautorotatespd[1],0.0,10.0,1.0,1.0);
					voxie_menu_additem("Z"               ,128,256,360, 64,MENU_AUTOROTATEZ  ,MENU_HSLIDER ,0                ,0x908070,(double)gautorotatespd[2],0.0,10.0,1.0,1.0);
#endif
					break;

				default:
					voxie_menu_reset(0,0,0);
			}
			orendmode = grendmode; voxie_playsound_update(-1,-1,0,0,1.f);/*kill all sound*/
			for(j=0;j<gmusn;j++) { if (gmus[j].rendmode == grendmode) { voxie_playsound(gmus[j].file,-1,100,100,1.f); break; } }
		}

		if (vw.useemu)
		{
			static int autonudge = 0;
			i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1);
			if (i || autonudge)
			{
				if (i) autonudge = voxie_keystat(0x2a);
				if (autonudge) vw.emuhang += cos(tim*1.0)*(float)autonudge*.0005;
					  if (voxie_keystat(0xb8)|voxie_keystat(0xdd)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.0); //(Ralt|RMenu)+[,]
				else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = min(max(vw.emudist-(float)i*dtim*2048.0,400.0),4000.0); //Ctrl+[,]
				else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
				voxie_init(&vw);
			}
		}

#if (USEMAG6D)
		if (gusemag6d >= 0)
		{
			dpoint3d ipos, irig, idow, ifor;

			gusemag6d = mag6d_getnumdevices();
			if (gusemag6d > 0)
			{
				m6cnt = mag6d_read(0,&ipos,&irig,&idow,&ifor,&m6but);

				m6ru.x = irig.x; m6du.x = idow.x; m6fu.x = ifor.x;
				m6ru.y = irig.y; m6du.y = idow.y; m6fu.y = ifor.y;
				m6ru.z = irig.z; m6du.z = idow.z; m6fu.z = ifor.z;

				m6r.x = irig.x; m6d.x = idow.x; m6f.x = ifor.x; m6p.x = (   ipos.x);
				m6r.y =-irig.z; m6d.y =-idow.z; m6f.y =-ifor.z; m6p.y = (-1-ipos.z);
				m6r.z = irig.y; m6d.z = idow.y; m6f.z = ifor.y; m6p.z = (   ipos.y);
			}
		}
#endif
		for(i=0;i<4;i++)
		{
			onavbut[i] = nav[i].but; voxie_nav_read(i,&nav[i]);
		}

		voxie_frame_start(&vf);
		gxres = vf.x1-vf.x0;
		gyres = vf.y1-vf.y0;

		showphase = 0;
		switch(grendmode)
		{
			case RENDMODE_TITLE: //title/icon-based mode selection
				{
				static int inited = -1;
				static double timig = -1e32;
				static float fofy = 0.f;
				static int icperow, icnum;
				float fscale;

				if (inited <= 0)
				{
					if (inited < 0)
					{
						icperow = 3; icnum = sizeof(iconst)/sizeof(iconst[0])-1;
						voxie_mountzip("icons.zip");
					}
					inited = 1; gtimbore = tim+gmenutimeout;
				}

				fscale = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
				voxie_setview(&vf,-vw.aspx*fscale,-vw.aspy*fscale,-vw.aspz*fscale,+vw.aspx*fscale,+vw.aspy*fscale,+vw.aspz*fscale);

				if (tim > timig)
				{
					if ((min(vx[0].tx0,vx[0].tx1) < -16384) || (nav[0].dx < -100.0f)) { gtimbore = tim+gmenutimeout; timig = tim+0.25; gcurselmode = max(gcurselmode-1,0); }
					if ((max(vx[0].tx0,vx[0].tx1) > +16384) || (nav[0].dx > +100.0f)) { gtimbore = tim+gmenutimeout; timig = tim+0.25; gcurselmode = min(gcurselmode+1,icnum-1); }
					if ((max(vx[0].ty0,vx[0].ty1) > +16384) || (nav[0].dy < -100.0f)) { gtimbore = tim+gmenutimeout; timig = tim+0.25; if (gcurselmode >= icperow) gcurselmode -= icperow; }
					if ((min(vx[0].ty0,vx[0].ty1) < -16384) || (nav[0].dy > +100.0f)) { gtimbore = tim+gmenutimeout; timig = tim+0.25; gcurselmode = min(gcurselmode+icperow,icnum-1); }
				}
				if ((voxie_keystat(keyremap[0][0]) == 1) || (voxie_keystat(keyremap[1][0]) == 1) || (ghitkey == 0xcb) || ((vx[0].but&~ovxbut[0])&0x0004)) { gtimbore = tim+gmenutimeout; gcurselmode = max(gcurselmode-1,0); }
				if ((voxie_keystat(keyremap[0][1]) == 1) || (voxie_keystat(keyremap[1][1]) == 1) || (ghitkey == 0xcd) || ((vx[0].but&~ovxbut[0])&0x0008)) { gtimbore = tim+gmenutimeout; gcurselmode = min(gcurselmode+1,icnum-1); }
				if ((voxie_keystat(keyremap[0][2]) == 1) || (voxie_keystat(keyremap[1][2]) == 1) || (ghitkey == 0xc8) || ((vx[0].but&~ovxbut[0])&0x0001)) { gtimbore = tim+gmenutimeout; if (gcurselmode >= icperow) gcurselmode -= icperow; }
				if ((voxie_keystat(keyremap[0][3]) == 1) || (voxie_keystat(keyremap[1][3]) == 1) || (ghitkey == 0xd0) || ((vx[0].but&~ovxbut[0])&0x0002)) { gtimbore = tim+gmenutimeout; gcurselmode = min(gcurselmode+icperow,icnum-1); }
				if ((voxie_keystat(keyremap[0][4]) == 1) || (voxie_keystat(keyremap[1][4]) == 1) || (ghitkey == 0x1c) || (voxie_keystat(0x1c) == 1) || (voxie_keystat(0x9c) == 1) || ((vx[0].but&~ovxbut[0])&0xf3c0) || (nav[0].but&~onavbut[0]&1)) //Space, Enter
				{
					if (gcurselmode < 0) { gcurselmode = 0; gtimbore = tim+gmenutimeout; }
										 else { grendmode = gcurselmode+1; }
				}
				ghitkey = 0;
				if ((gmenutimeout >= 0) && (tim >= gtimbore) && (gcurselmode >= 0)) { gcurselmode = -1; }

				if (gcurselmode < 0)
				{
					point3d pp, rr, dd, ff;
					pp.x = 0.0f; pp.y = 0.0f; pp.z = 0.0f; f = fscale;
					rr.x = cos(tim)*f; rr.y = sin(tim)*f; rr.z = 0.0f;
					dd.x =-sin(tim)*f; dd.y = cos(tim)*f; dd.z = 0.0f;
					ff.x = 0.0f;       ff.y = 0.0f;       ff.z = f;
					voxie_drawspr(&vf,iconnam[0],&pp,&rr,&dd,&ff,0x404040);
					goto dofireworks;
				}

				f = -(((float)(gcurselmode/icperow)-icperow*0.5+0.5)*2.0*vw.aspx*fscale/icperow);
				fofy += (f-fofy)*.1;

				for(i=0;i<icnum;i++)
				{
					point3d pp, rr, dd, ff;
					pp.x = ((float)(i%icperow)-icperow*0.5+0.5)*2.0*vw.aspx*fscale/icperow;
					pp.y = ((float)(i/icperow)-icperow*0.5+0.5)*2.0*vw.aspx*fscale/icperow + fofy;
					pp.z = 0.0f; f = 1.6f/icperow*vw.aspx*fscale;
					if (i != gcurselmode) { rr.x = f; rr.y = 0.f; }
										  else { rr.x = cos(tim)*f; rr.y = sin(tim)*f; }
													  rr.z = 0.0f;
					dd.x =-rr.y; dd.y = rr.x; dd.z = 0.0f;
					ff.x = 0.0f; ff.y = 0.0f; ff.z = f;
					voxie_drawspr(&vf,iconnam[i+1],&pp,&rr,&dd,&ff,(i == gcurselmode)*0x707070 + 0x101010);
				}

				for(f=-.01f;f<=.01f;f+=.004f)
				{
					float g;
					fx = ((float)(gcurselmode%icperow)-icperow*0.5+0.5)*2.0*vw.aspx*fscale/icperow;
					fy = ((float)(gcurselmode/icperow)-icperow*0.5+0.5)*2.0*vw.aspx*fscale/icperow + fofy;
					g = f+sin(tim*2.f)*.04f + 0.8f*vw.aspx*fscale/icperow;
					//voxie_drawbox(&vf,fx-g,fy-g,-vw.aspz*fscale,fx+g,fy+g,vw.aspz*fscale,1,0xffffff);
					//voxie_drawbox(&vf,fx-g,fy-g,max(-vw.aspz*fscale,-g),fx+g,fy+g,min(g,vw.aspz*fscale),1,0xffffff);
					//voxie_drawsph(&vf,fx,fy,0.f,g,0,0xffffff);
					draw_platonic(3,fx,fy,0.f,g*1.7f,tim*.5,1,0xffffff);
				}

				{
				point3d pp, rr, dd;
				char tbuf[256];
				sprintf(tbuf,"%s",iconst[gcurselmode+1]); f = (float)strlen(tbuf)*.5;
				g = (cos(tim*2.f)+1.f)*(PI/4.f);

				rr.x = 0.12f; dd.x =        0.00f; pp.x = vw.aspx*fscale*+.00f - rr.x*f - dd.x*.5f;
				rr.y = 0.00f; dd.y = sin(g)*0.25f; pp.y = vw.aspy*fscale*+.87f - rr.y*f - dd.y*.5f;
				rr.z = 0.00f; dd.z = cos(g)*0.25f; pp.z = vw.aspz*fscale*+.00f - rr.z*f - dd.z*.5f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",iconst[gcurselmode+1]);
				}

				}
				break;

				{
				typedef struct { float x, y, z, vx, vy, vz, tim; int col; } fwork_t;
				#define FWORKMAX 64
				static fwork_t fwork[FWORKMAX];
dofireworks:;
				static int fworkn = 0;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

#if (USELEAP)
				frame = leap_getframe();
				if ((frame) && (frame->nHands > 0))
				{
					hand = &frame->pHands[0];
					digit = &hand->digits[1];
					vec = &digit->stabilized_tip_position;
					fx = vec->x*+.01f    ;
					fy = vec->z*+.01f    ;
					fz = vec->y*-.01f+2.f;
					voxie_drawsph(&vf,fx,fy,fz,.05f,0,0xffffff);
				}
#endif

				if ((fworkn < FWORKMAX) && ((int)(tim*3.0) != (int)(otim*3.0)))
				{
#if (USELEAP)
					if ((i >= 0) && (fabs(fx) <= vw.aspx) && (fabs(fy) <= vw.aspy) && (fabs(fz) <= vw.aspz))
					{
						fwork[fworkn].x = fx;
						fwork[fworkn].y = fy;
						fwork[fworkn].z = fz;
					}
					else
#endif
					{
					fwork[fworkn].x = ((rand()&32767)-16384)*(vw.aspx/16384.0);
					fwork[fworkn].y = ((rand()&32767)-16384)*(vw.aspy/16384.0);
					fwork[fworkn].z = vw.aspz;
					}

					fwork[fworkn].vx = ((rand()&32767)-16384)/131072.0;
					fwork[fworkn].vy = ((rand()&32767)-16384)/131072.0;
					fwork[fworkn].vz = ((rand()&32767)-65536)/131072.0*sqrt(vw.aspz/vw.aspx*5.0);
					fwork[fworkn].tim = -1;
					i = (rand()%6)+2;
					fwork[fworkn].col = (i&1)*0xff + ((i>>1)&1)*0xff00 + ((i>>2)&1)*0xff0000;
					fworkn++;
				}

				for(i=fworkn-1;i>=0;i--)
				{
					f = dtim*1.0;
					fwork[i].x += fwork[i].vx*f;
					fwork[i].y += fwork[i].vy*f;
					fwork[i].z += fwork[i].vz*f;
					fwork[i].vx *= pow(.9,dtim);
					fwork[i].vy *= pow(.9,dtim);
					fwork[i].vz *= pow(.9,dtim);
					fwork[i].vz += dtim*.3;
					if ((fwork[i].tim < 0.0) && ((fwork[i].z < vw.aspz*-.5) || (fwork[i].vz > 0.0))) fwork[i].tim = tim;
					if (fwork[i].z > vw.aspz*2.0) { fworkn--; fwork[i] = fwork[fworkn]; continue; }

					if (fwork[i].tim < 0.0)
					{
						voxie_drawlin(&vf,fwork[i].x,fwork[i].y,fwork[i].z,
												fwork[i].x+fwork[i].vx*0.25,
												fwork[i].y+fwork[i].vy*0.25,
												fwork[i].z+fwork[i].vz*0.25,fwork[i].col);
					}
					else
					{
						float fdotrad;
						f = tim-fwork[i].tim;
						if (f >= 2.0) { fworkn--; fwork[i] = fwork[fworkn]; continue; }
						n = 64; fr = sqrt(f)*0.15; fdotrad = (1.f-f/2.0)*.007f;
						for(j=0;j<n;j++)
						{
							fz = ((double)j+.5)/(double)n*2.0-1.0; f = sqrt(1.0 - fz*fz);
							g = (double)j*(PI*(sqrt(5.0)-1.0)); fx = cos(g)*f; fy = sin(g)*f;
							voxie_drawsph(&vf,fx*fr+fwork[i].x,
													fy*fr+fwork[i].y,
													fz*fr+fwork[i].z,fdotrad,0,fwork[i].col);
						}
					}

				}
				}
				break;

			case RENDMODE_PHASESYNC: //'`': heightmap from UDP
				{
				point3d pp, rr, dd;

				if ((voxie_keystat(keyremap[0][4]) == 1) || (voxie_keystat(keyremap[0][5]) == 1)) { gphasesync_curmode ^= 1; }
				if ((voxie_keystat(keyremap[0][0]) == 1) && (gphasesync_cursc < 16)) gphasesync_cursc <<= 1;
				if ((voxie_keystat(keyremap[0][1]) == 1) && (gphasesync_cursc >  1)) gphasesync_cursc >>= 1;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
				if (!gphasesync_curmode)
				{
					for(y=-16;y<=16;y+=16)
						for(x=-16;x<16;x+=gphasesync_cursc*2)
						{
							f = 1.f; if ((gphasesync_cursc == 16) && (!y)) f = -1.f;
							voxie_drawlin(&vf,(float)(x+gphasesync_cursc*0)*vw.aspx/16.f,(float)y*vw.aspy/16.f,-vw.aspz*f,
													(float)(x+gphasesync_cursc*1)*vw.aspx/16.f,(float)y*vw.aspy/16.f,+vw.aspz*f,0xffffff);
							voxie_drawlin(&vf,(float)(x+gphasesync_cursc*1)*vw.aspx/16.f,(float)y*vw.aspy/16.f,+vw.aspz*f,
													(float)(x+gphasesync_cursc*2)*vw.aspx/16.f,(float)y*vw.aspy/16.f,-vw.aspz*f,0xffffff);

							voxie_drawlin(&vf,(float)y*vw.aspx/16.f,(float)(x+gphasesync_cursc*0)*vw.aspy/16.f,-vw.aspz*f,
													(float)y*vw.aspx/16.f,(float)(x+gphasesync_cursc*1)*vw.aspy/16.f,+vw.aspz*f,0xffffff);
							voxie_drawlin(&vf,(float)y*vw.aspx/16.f,(float)(x+gphasesync_cursc*1)*vw.aspy/16.f,+vw.aspz*f,
													(float)y*vw.aspx/16.f,(float)(x+gphasesync_cursc*2)*vw.aspy/16.f,-vw.aspz*f,0xffffff);
						}
				}
				else
				{
					for(j=0;j<=4;j++)
					{
						int m, o;
						if ((gphasesync_cursc == 16) && (j == 2)) m = 0; else m = (vf.drawplanes>>1);
						for(o=0;o<vw.dispnum;o++)
						{
							k = (((gyres-2)*j)>>2);
							for(y=k;y<=k+1;y++)
								for(x=0;x<gxres;x++)
								{
									i = (((x*vf.drawplanes*16)/(gxres*gphasesync_cursc)+m)%vf.drawplanes); //if (vw.usecol > 0) i = div24[i]*24 + mod3[i]*8 + div3[mod24[i]];
									*(int *)(vf.fp*vw.framepervol*o + vf.fp*div24[i] + vf.p*y + x*4 + vf.f) |= oneupmod24[i];
								}
							k = (((gxres-2)*j)>>2);
							for(x=k;x<=k+1;x++)
								for(y=0;y<gyres;y++)
								{
									i = (((y*vf.drawplanes*16)/(gyres*gphasesync_cursc)+m)%vf.drawplanes); //if (vw.usecol > 0) i = div24[i]*24 + mod3[i]*8 + div3[mod24[i]];
									*(int *)(vf.fp*vw.framepervol*o + vf.fp*div24[i] + vf.p*y + x*4 + vf.f) |= oneupmod24[i];
								}
						}
					}
				}

				rr.x = 0.12f; dd.x = 0.00f;
				rr.y = 0.00f; dd.y = 0.16f;
				rr.z = 0.00f; dd.z = 0.00f;
				for(y=-1;y<=1;y+=2)
					for(x=-1;x<=1;x+=2)
					{
						pp.x = rr.x*-1.50f + (float)x*.20f; pp.y = dd.y*-.50f + (float)y*.25f; pp.z = -vw.aspz; voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"TOP");
						pp.x = rr.x*-1.50f + (float)x*.20f; pp.y = dd.y*-.50f + (float)y*.25f; pp.z =      0.f; voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"MID");
						pp.x = rr.x*-1.50f + (float)x*.80f; pp.y = dd.y*-.50f + (float)y*.80f; pp.z =      0.f; voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"MID");
						pp.x = rr.x*-1.50f + (float)x*.20f; pp.y = dd.y*-.50f + (float)y*.25f; pp.z = +vw.aspz; voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"BOT");
					}

				showphase = 1;
				break;
				}
			case RENDMODE_KEYSTONECAL: //quadrilateral projection compensation
#if 0
				if (vw.usecol <= 0)
				{
					for(y=0;y<120;y++)
						for(i=0;i<288;i++)
						{
							j = i/4;
							*(int *)(vf.f + vf.fp*(j/24) + vf.p*y + i*4) = (1<<(j%24));
						}
				}
				else
				{
					for(y=0;y<20;y++)
						for(i=0;i<288;i++)
						{
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y    ) + (      i)*4) = (65536<<((i/(4*3))%8));
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y+ 20) + (288-1-i)*4) = (65536<<((i/(4*3))%8));
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y+ 40) + (      i)*4) = (  256<<((i/(4*3))%8));
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y+ 60) + (288-1-i)*4) = (  256<<((i/(4*3))%8));
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y+ 80) + (      i)*4) = (    1<<((i/(4*3))%8));
							*(int *)(vf.f + vf.fp*(i/(24*4)) + vf.p*(y+100) + (288-1-i)*4) = (    1<<((i/(4*3))%8));
						}
					voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords
					for(y=0;y<50;y++)
						for(x=0;x<128;x++)
						{
							voxie_drawvox(&vf,x+72,y+132,x*4-256,0x0000ff);
							voxie_drawvox(&vf,x+72,y+182,x*4-256,0x00ff00);
							voxie_drawvox(&vf,x+72,y+232,x*4-256,0x00ffff);
							voxie_drawvox(&vf,x+72,y+282,x*4-256,0xff0000);
							voxie_drawvox(&vf,x+72,y+332,x*4-256,0xff00ff);
							voxie_drawvox(&vf,x+72,y+382,x*4-256,0xffff00);
							voxie_drawvox(&vf,x+72,y+432,x*4-256,0xffffff);
						}
				}
#else
				{
				static int cornind = 0, grabcornx, grabcorny, grabcornz;
				point3d fp, a0, a1, b0, b1, ai, bi, ci;
				int igind = -1;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				for(f=-0.5;f<=0.5;f+=0.5)
					for(g=-1.0;g<=1.0;g+=1.0)
					{
						voxie_drawlin(&vf,-vw.aspx  , vw.aspy*f,vw.aspz*g,vw.aspx  ,vw.aspy*f,vw.aspz*g,0xffffff);
						voxie_drawlin(&vf, vw.aspx*f,-vw.aspy  ,vw.aspz*g,vw.aspx*f,vw.aspy  ,vw.aspz*g,0xffffff);
					}
				for(f=-1.0;f<=1.0;f+=0.5)
					for(g=-1.0;g<=1.0;g+=0.5) voxie_drawlin(&vf,vw.aspx*f,vw.aspy*g,-vw.aspz,vw.aspx*f,vw.aspy*g,+vw.aspz,0xffffff);

				if (labs(vx[0].tx0) < 4096) vx[0].tx0 = 0;
				if (labs(vx[0].ty0) < 4096) vx[0].ty0 = 0;
				if (labs(vx[0].tx1) < 4096) vx[0].tx1 = 0;
				if (labs(vx[0].ty1) < 4096) vx[0].ty1 = 0;

#if (USELEAP)
				frame = leap_getframe();
				if ((frame) && (frame->nHands > 0))
				{
					hand = &frame->pHands[0];
					digit = &hand->digits[1];
					vec = &digit->stabilized_tip_position;
					keystone.curx = min(max(vec->x*+.01f    ,-vw.aspx),vw.aspx);
					keystone.cury = min(max(vec->z*+.01f    ,-vw.aspy),vw.aspy);
					keystone.curz = min(max(vec->y*-.01f+2.f,-vw.aspz),vw.aspz);
				}
#endif
				for(i=0;i<vw.dispnum;i++) { if (voxie_keystat(i+0x2) == 1) { vw.dispcur = i; voxie_init(&vw); } } //'1', '2'
				if ((bstatus&(~obstatus)&2) || (nav[0].but&(~onavbut[0])&2)) { vw.dispcur++; if (vw.dispcur >= vw.dispnum) { vw.dispcur = 0; } voxie_init(&vw); } //RMB

				f = (float)((voxie_keystat(0x34)!=0) - (voxie_keystat(0x33)!=0))*dtim*.25f;
				if (ghitkey == 0x33) { ghitkey = 0; f -= 5.0f*(PI/180.f); }
				if (ghitkey == 0x34) { ghitkey = 0; f += 5.0f*(PI/180.f); }
				if (ghitkey == 0xb3) { ghitkey = 0; f -= 0.1f*(PI/180.f); }
				if (ghitkey == 0xb4) { ghitkey = 0; f += 0.1f*(PI/180.f); }
				if (f != 0.f)
				{
					int i0, i1;
					i0 = vw.dispcur; i1 = vw.dispcur+1;
					if (voxie_keystat(0x38)|voxie_keystat(0xb8)|grabdispall) { i0 = 0; i1 = vw.dispnum; }
					for(k=i0;k<i1;k++)
					{
						for(j=8-1;j>=0;j--)
						{
							if (vw.nblades <= 0) fz = (float)((j>>2)*((vw.framepervol*24-1)>>1));
												 else fz = (float)((j>=4)*2-1)*vw.aspz;
							voxie_project(k,-1,vw.disp[k].keyst[j].x,vw.disp[k].keyst[j].y,fz,&gx,&gy);
							voxie_project(k,+1,gx*cos(f) - gy*sin(f),
													 gy*cos(f) + gx*sin(f),fz,&vw.disp[k].keyst[j].x,&vw.disp[k].keyst[j].y);
						}
					}
					voxie_init(&vw);
				}

					//Projection compensation GUI
				if ((!(bstatus&1)) && (!(vx[0].but&0xf000)) && (!(nav[0].but&1)))
				{
					keystone.curx = min(max(keystone.curx + fx*.01f   + vx[0].tx0*+.0000005f + nav[0].dx*dtim*.01f,-vw.aspx),vw.aspx);
					keystone.cury = min(max(keystone.cury + fy*.01f   + vx[0].ty0*-.0000005f + nav[0].dy*dtim*.01f,-vw.aspy),vw.aspy);
					keystone.curz +=                        fz*-.001f + vx[0].ty1*-.0000005f + (vx[0].rt-vx[0].lt)*.00005f + nav[0].dz*dtim*.01f;
					keystone.curz += ((voxie_keystat(keyremap[0][4]) || voxie_keystat(0xd1) || voxie_keystat(0x28))
								 - (voxie_keystat(keyremap[0][5]) || voxie_keystat(0xc9) || voxie_keystat(0x27)))*dtim; //ButA,PGDN - ButB,PGUP
					//if (bstatus&~obstatus&2) { if (keystone.curz >= 0.f) keystone.curz = -vw.aspz; else keystone.curz = vw.aspz; } //RMB sets cursor height to ceil/floor
					keystone.curz = min(max(keystone.curz,-vw.aspz),vw.aspz);

					fp.x = keystone.curx; fp.y = keystone.cury; fp.z = keystone.curz;
				}
				else
				{
					if ((!(obstatus&1)) && (!(ovxbut[0]&0xf000)))
					{
						if (vw.nblades <= 0) //obsolete? handles vw.flip
						{
							switch(vw.flip)
							{
								case 0: cornind = ((keystone.curx >= 0.f) ^ (keystone.cury >= 0.f))*1 + (keystone.cury >= 0.f)*2; break;
								case 1: cornind = ((keystone.cury <  0.f) ^ (keystone.curx >= 0.f))*1 + (keystone.curx >= 0.f)*2; break;
								case 2: cornind = ((keystone.curx <  0.f) ^ (keystone.cury <  0.f))*1 + (keystone.cury <  0.f)*2; break;
								case 3: cornind = ((keystone.cury >= 0.f) ^ (keystone.curx <  0.f))*1 + (keystone.curx <  0.f)*2; break;
							}
							cornind += (keystone.curz >= 0.f)*4;
						}
						else
						{
							grabcornx = (int)(floor(keystone.curx/vw.aspx*2.f+.5));
							grabcorny = (int)(floor(keystone.cury/vw.aspy*2.f+.5));
							grabcornz = (int)(floor(keystone.curz/vw.aspz*2.f+.5));
						}
					}
					gx = (fx*.0025f + vx[0].tx0*+.0000001f + nav[0].dx*dtim*.01f)*(1.f/16.f);
					gy = (fy*.0025f + vx[0].ty0*-.0000001f + nav[0].dy*dtim*.01f)*(1.f/16.f);

						//FUKFUKFUKFUKFUKFUKFUKFUK
					int i0, i1;
					i0 = vw.dispcur; i1 = vw.dispcur+1;
					if (voxie_keystat(0x38)|voxie_keystat(0xb8)|grabdispall) { i0 = 0; i1 = vw.dispnum; }
					for(i=i0;i<i1;i++)
					{
						if (vw.nblades <= 0) //obsolete? handles vw.flip
						{
							float fx2, fy2;
							fx2 = vw.disp[i].keyst[cornind].x; fy2 = vw.disp[i].keyst[cornind].y;
							if (vw.nblades <= 0) fz = (float)((cornind>>2)*((vw.framepervol*24-1)>>1));
												 else fz = (float)((cornind>=4)*2-1)*vw.aspz;
							voxie_project(i,-1,fx2   ,fy2   ,fz,&fx2,&fy2);
							voxie_project(i,+1,fx2+gx,fy2+gy,fz,&fx2,&fy2);
							vw.disp[i].keyst[cornind].x = fx2; vw.disp[i].keyst[cornind].y = fy2;
						}
						else
						{
							for(j=8-1;j>=0;j--)
							{
								if ((((j&3) == 0) || ((j&3) == 3)) && (grabcornx > 0)) continue;
								if ((((j&3) == 1) || ((j&3) == 2)) && (grabcornx < 0)) continue;
								if ((((j&3) == 0) || ((j&3) == 1)) && (grabcorny > 0)) continue;
								if ((((j&3) == 2) || ((j&3) == 3)) && (grabcorny < 0)) continue;
								if (( j <  4                     ) && (grabcornz > 0)) continue;
								if (( j >= 4                     ) && (grabcornz < 0)) continue;

								float fx2, fy2;
								fx2 = vw.disp[i].keyst[j].x; fy2 = vw.disp[i].keyst[j].y;
								if (vw.nblades <= 0) fz = (float)((j>>2)*((vw.framepervol*24-1)>>1));
													 else fz = (float)((j>=4)*2-1)*vw.aspz;
								voxie_project(i,-1,fx2   ,fy2   ,fz,&fx2,&fy2);
								voxie_project(i,+1,fx2+gx,fy2+gy,fz,&fx2,&fy2);
								vw.disp[i].keyst[j].x = fx2; vw.disp[i].keyst[j].y = fy2;
							}
						}
					}
					voxie_init(&vw);

					igind = cornind;
					if (vw.nblades <= 0)
					{
						fp.x = ((float)(keystone.curx >= 0)*2.f-1.f)*vw.aspx;
						fp.y = ((float)(keystone.cury >= 0)*2.f-1.f)*vw.aspy;
						fp.z = ((float)(keystone.curz >= 0)*2.f-1.f)*vw.aspz;
					}
					else
					{
						fp.x = (float)min(max(grabcornx,-1),1);
						fp.y = (float)min(max(grabcorny,-1),1);
						fp.z = (float)min(max(grabcornz,-1),1)*vw.aspz;
					}
				}

				f = .03f; //Draw cursor
				if (vw.dispnum == 1) col = 0xffffff;
				else
				{
					//if (vw.dispcur == 0) col = 0x00ff00;
					//if (vw.dispcur == 1) col = 0xff0000;
					//if (vw.dispcur == 2) col = 0x0000ff;
					if (vw.dispcur == 0) col = 0x00ff00;
					if (vw.dispcur == 1) col = 0x0000ff;
					if (vw.dispcur == 2) col = 0xff0000;
					if (voxie_keystat(0x38)|voxie_keystat(0xb8)|grabdispall) col = 0xffffff;
				}
				voxie_drawbox(&vf,fp.x-f,fp.y-f,fp.z-f,
										fp.x+f,fp.y+f,fp.z+f,2,col);


				{
				point3d pp, rr, dd;
				pp.x = 0.0f; rr.x = 0.12f; dd.x = 0.00f;
				pp.y =-0.2f; rr.y = 0.00f; dd.y = 0.16f;
				pp.z = 0.0f; rr.z = 0.00f; dd.z = 0.00f;
				voxie_printalph_(&vf,&pp,&rr,&dd,(rand()<<15)+rand(),"%d/%d",vw.dispcur,vw.dispnum);
				}

					//draw wireframe box
				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
				voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
				vw.clipshape = ((vw.nblades > 0) && (vw.aspx == vw.aspy));
				if (vw.clipshape)
				{
					n = 64;
					for(j=-64;j<=64;j++)
					{
						if (j == -62) j = 62;
						for(i=0;i<n;i++)
						{
							voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f,
													cos((float)(i+1)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+1)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f, 0xffffff);
						}
					}

					n = 32;
					for(i=0;i<n;i++)
					{
						voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, -vw.aspz,
												cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, +vw.aspz, 0xffffff);
					}
				}

				for(x=-3;x<=3;x+=2)
					for(y=-3;y<=3;y+=2)
						for(z=-1;z<=1;z+=2)
						{
							point3d p, r, d;
							r.x = 0.12f; d.x = 0.00f; p.x = (float)x*.25f - r.x*3*.5f - d.x*.5f;
							r.y = 0.00f; d.y = 0.12f; p.y = (float)y*.25f - r.y*3*.5f - d.y*.5f;
							r.z = 0.00f; d.z = 0.00f; p.z = (float)z*vw.aspz*.98f;
							if (z < 0) voxie_printalph_(&vf,&p,&r,&d,0xffffff,"TOP");
									else voxie_printalph_(&vf,&p,&r,&d,0xffffff,"BOT");
						}


				//if ((voxie_keystat(0x1f) == 1) && (voxie_keystat(0x1d) || voxie_keystat(0x9d))) //Ctrl+S: overwrite cal file
				//{
				//   FILE *fil;
				//   fil = fopen("voxiebox_keystone.ini","wb");
				//   if (fil)
				//   {
				//      fprintf(fil,"//WARNING:do not add settings to this file. This file is overwritten by VOXIEDEMO's keystone calibration Ctrl+S\r\n");
				//      for(i=0;i<=6;i+=6)
				//         for(j=i;j<i+2;j++)
				//            for(k=0;k<=2;k+=2)
				//            {
				//               fprintf(fil,"projx%d=%7.2f; projy%d=%7.2f;",j^k,vw.proj[j^k].x,j^k,vw.proj[j^k].y);
				//               if (k) fprintf(fil,"\r\n"); else fprintf(fil," ");
				//            }
				//
				//      fclose(fil);
				//      voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,1.f);
				//   }
				//}

				}
#endif
				break;
#if (USEMAG6D != 0)
			case RENDMODE_LOCALCAL: //MAGSTER.KC (requires mag6d controller)
				{
				#define FIFSIZ 4096
				static point3d fif[FIFSIZ][2], om6p, om6r, om6d, om6f;
				static int fifw = 0, om6cnt = -1;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				if (gusemag6d <= 0)
				{
					point3d pp, rr, dd;
					pp.x = vw.aspx*-.5; pp.y = -0.12f; pp.z = -vw.aspz+0.01f;
					rr.x = 0.08;        rr.y = 0.00;   rr.z = 0.00;
					dd.x = 0.00;        dd.y = 0.12;   dd.z = 0.00;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"MAG6D not found");
					break;
				}

				if (m6but&4) { fifw = 0; } //Clear rod list (RMB)
				if ((m6but&1) && (m6cnt != om6cnt)) //Write rod to list (LMB)
				{
					fif[fifw&(FIFSIZ-1)][0] = m6p; f = .25f;
					fif[fifw&(FIFSIZ-1)][1].x = m6p.x + m6f.x*f;
					fif[fifw&(FIFSIZ-1)][1].y = m6p.y + m6f.y*f;
					fif[fifw&(FIFSIZ-1)][1].z = m6p.z + m6f.z*f;
					fifw++;
				}

					//Rotate personal rods
				if (m6but&2)
				{
					for(i=max(fifw-FIFSIZ,0);i<fifw;i++)
					{
						k = (i&(FIFSIZ-1));
						for(j=0;j<2;j++)
						{
							float nfx, nfy, nfz;

							fx = fif[k][j].x-om6p.x;
							fy = fif[k][j].y-om6p.y;
							fz = fif[k][j].z-om6p.z;
							nfx = fx*om6r.x + fy*om6r.y + fz*om6r.z;
							nfy = fx*om6d.x + fy*om6d.y + fz*om6d.z;
							nfz = fx*om6f.x + fy*om6f.y + fz*om6f.z;
							fif[k][j].x = nfx*m6r.x + nfy*m6d.x + nfz*m6f.x + m6p.x;
							fif[k][j].y = nfx*m6r.y + nfy*m6d.y + nfz*m6f.y + m6p.y;
							fif[k][j].z = nfx*m6r.z + nfy*m6d.z + nfz*m6f.z + m6p.z;
						}
					}
				}

					//Draw rod fifo
				for(i=max(fifw-FIFSIZ,0);i<fifw;i++)
				{
					k = (i&(FIFSIZ-1));
					//voxie_drawvox(&vf,fif[k][0].x,fif[k][0].y,fif[k][0].z,0xffffff);
					voxie_drawlin(&vf,fif[k][0].x,fif[k][0].y,fif[k][0].z,
											fif[k][1].x,fif[k][1].y,fif[k][1].z,0xffffff);
				}

				om6p = m6p; om6r = m6r; om6d = m6d; om6f = m6f; om6cnt = m6cnt;
				}
				break;
#elif 0
			case RENDMODE_LOCALCAL: //old methods: waves (bstatus==0), hemispheres (bstatus==1), pyramids (bstatus==3), spheres&cubes (bstatus==2), sphere&cubes procedural (bstatus==4)
				k = bstatus;
				if (vx[0].but&(1<<15)) { k = 1; }
				if (vx[0].but&(1<<12)) { k = 2; }
				if (vx[0].but&(1<<14)) { k = 3; }
				if (vx[0].but&(1<<13)) { k = 4; }

				if (k == 0)
				{
					for(y=0;y<gyres;y++)
						for(x=0;x<gxres;x++)
						{
							double d = sqrt((double)((x-gxres*.5)*(x-gxres*.5) + (y-gyres*.5)*(y-gyres*.5)*.25))*.05 + tim*5;
							i = fmod(d/(PI*2.0),1.0)*vf.drawplanes; if (i >= (vf.drawplanes>>1)) i = vf.drawplanes-1-i;
							//if (vw.usecol > 0) i = div24[i]*24 + mod3[i]*8 + div3[mod24[i]];
							if (vw.drawstroke&1) *(int *)(vf.f + vf.fp*div24[                i] + vf.p*y + x*4) |= oneupmod24[                i];
							if (vw.drawstroke&2) *(int *)(vf.f + vf.fp*div24[vf.drawplanes-1-i] + vf.p*y + x*4) |= oneupmod24[vf.drawplanes-1-i];
						}
				}
				else if ((k == 1) || (k == 3))
				{
					static tiletype pyra = {0};
					if (!pyra.f)
					{
						pyra.x = gxres; pyra.y = gyres; pyra.p = (INT_PTR)pyra.x*4;
						pyra.f = (INT_PTR)malloc(pyra.p*pyra.y); if (!pyra.f) pyra.f = -1;
					}
					if (pyra.f == -1) break;

					for(y=0;y<gyres;y++)
						for(x=0;x<gxres;x++)
						{
							xx = ( (x+numframes)    &127)-64;
							yy = (((y+numframes)>>1)&127)-64;
							if (k == 1)
							{     //hemispheres
								i = 63*63 - (xx*xx + yy*yy); if (i < 0) { *(int *)(pyra.p*y + (x<<2) + pyra.f) = 0x12345678; continue; }
								i = sqrt((double)i)*4.0;
							}
							else { i = 255 - min(max(labs(xx),labs(yy))*4,255); } //pyramids
							xx = ((x+numframes)>>7);
							yy = ((y+numframes)>>8);
							j = ((yy + xx)%7)+1;
							*(int *)(pyra.p*y + (x<<2) + pyra.f) = ((j>>0)&1)*0xff + ((j>>1)&1)*0xff00 + ((j>>2)&1)*0xff0000 + (i<<24);
						}

					{
					point3d pp, rr, dd, ff;
					rr.x = vw.aspx*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
					rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
					rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
					voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
					voxie_drawheimap(&vf,(char *)&pyra, &pp,&rr,&dd,&ff, 0x12345678,-1,(!voxie_keystat(0x3b)) + (1<<2) + (1<<3));
					}
				}
				else if (k == 2) //render spheres,cubes,pyramid
				{
					pol_t pt[3];

					f = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
					voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f);

					n = 4;
					for(i=n-1;i>=0;i--)
					{
						pt[0].x = -0.60;                                          pt[0].y = -0.60;                                          pt[0].z = -0.20; pt[0].p2 = 1;
						pt[1].x = cos((float)(i+0)*PI*2.0/(float)n)*0.40+pt[0].x; pt[1].y = sin((float)(i+0)*PI*2.0/(float)n)*0.40+pt[0].y; pt[1].z = +0.20; pt[1].p2 = 2;
						pt[2].x = cos((float)(i+1)*PI*2.0/(float)n)*0.40+pt[0].x; pt[2].y = sin((float)(i+1)*PI*2.0/(float)n)*0.40+pt[0].y; pt[2].z = +0.20; pt[2].p2 = 0;
						voxie_drawpol(&vf,pt,3,0xffffff);
					}

					voxie_drawsph(&vf,0.00f,-0.60f,0.0f,0.20f,0,0xffffff);
					voxie_drawsph(&vf,0.60f,-0.60f,0.0f,0.20f,1,0xffffff);

					f = .19;
					voxie_drawbox(&vf,-0.60-f,+0.40-f,-f,-0.60+f,+0.40+f,+f,1,0xffffff);
					voxie_drawbox(&vf,+0.00-f,+0.40-f,-f, 0.00+f,+0.40+f,+f,2,0xffffff);
					voxie_drawbox(&vf,+0.60-f,+0.40-f,-f,+0.60+f,+0.40+f,+f,3,0xffffff);
				}
				else if (k == 4) //render procedural function style (slow!)
				{
					for(y=0;y<gyres;y++)
						for(x=0;x<gxres;x++)
							for(i=0;i<((vw.drawstroke==3)+1)*36;i++) //WARNING:obsolete!
							{
								int d2;
								//if (vw.usecol > 0) j = div24[i]*24 + mod3[i]*8 + div3[mod24[i]]; else
								j = i;
								z = clutmid[i]; wptr = (int *)(vf.f + vf.fp*div24[j] + vf.p*y);

								if (labs((labs(x-80)*8 + labs(y-140)*4) - (z+256)) < 10) wptr[x] |= oneupmod24[j]; //pyramid

								d2 = (x-180)*(x-180)*64 + (y-140)*(y-140)*16 + (z-0)*(z-0)*1;
								if ((d2 >= 248*248) && (d2 < 256*256)) wptr[x] |= oneupmod24[j]; //open sphere

								d2 = (x-280)*(x-280)*64 + (y-140)*(y-140)*16 + (z-0)*(z-0)*1;
								if (d2 < 256*256) wptr[x] |= oneupmod24[j]; //solid sphere

								if ((labs(x- 60) < 25) && (labs(y-380) < 50) && (labs(z) < 200))
									if (((labs(x- 60) > 23) + (labs(y-380) > 47) + (labs(z) > 175)) >= 2) wptr[x] |= oneupmod24[j]; //wire cube

								if ((labs(x-160) < 25) && (labs(y-380) < 50) && (labs(z) < 200))
									if ((labs(x-160) > 23) || (labs(y-380) > 47) || (labs(z) > 187)) wptr[x] |= oneupmod24[j]; //open cube

								if ((labs(x-260) < 25) && (labs(y-380) < 50) && (labs(z) < 200)) wptr[x] |= oneupmod24[j]; //solid cube
							}
				}
				break;
#else
			case RENDMODE_LOCALCAL:
				{
				static float stupiang = 0.f, stupiz = 0.f;

				vw.clipshape = 1;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				if (labs(vx[0].tx0) < 4096) vx[0].tx0 = 0;
				if (labs(vx[0].ty0) < 4096) vx[0].ty0 = 0;
				if (labs(vx[0].tx1) < 4096) vx[0].tx1 = 0;
				if (labs(vx[0].ty1) < 4096) vx[0].ty1 = 0;

				for(i=0;i<vw.dispnum;i++) { if (voxie_keystat(i+0x2) == 1) { vw.dispcur = i; voxie_init(&vw); } } //'1', '2'
				if ((bstatus&(~obstatus)&2) || (nav[0].but&(~onavbut[0])&2)) { vw.dispcur++; if (vw.dispcur >= vw.dispnum) { vw.dispcur = 0; } voxie_init(&vw); } //RMB

					//Projection compensation GUI
				if ((!(bstatus&1)) && (!(vx[0].but&0xf000)) && (!(nav[0].but&1)))
				{
					keystone.curx = min(max(keystone.curx + fx*.01f + vx[0].tx0*+.0000005f + nav[0].dx*dtim*.01f,-vw.aspx),vw.aspx);
					keystone.cury = min(max(keystone.cury + fy*.01f + vx[0].ty0*-.0000005f + nav[0].dy*dtim*.01f,-vw.aspy),vw.aspy);
					keystone.curz = 0.f;
				}

				f = .03f; //Draw cursor
				if (vw.dispnum == 1) col = 0xffffff;
				else
				{
					if (vw.dispcur == 0) col = 0x00ff00;
					if (vw.dispcur == 1) col = 0xff0000;
					if (vw.dispcur == 2) col = 0x0000ff;
				}
				voxie_drawbox(&vf,keystone.curx-f,keystone.cury-f,keystone.curz-f,
										keystone.curx+f,keystone.cury+f,keystone.curz+f,2,col);


				f = fz*-.001f + vx[0].ty1*-.0000005f + (vx[0].rt-vx[0].lt)*.00005f + nav[0].dz*dtim*.01f;
				f += ((voxie_keystat(keyremap[0][4]) || voxie_keystat(0xd1) || voxie_keystat(0x28))
							 - (voxie_keystat(keyremap[0][5]) || voxie_keystat(0xc9) || voxie_keystat(0x27)))*dtim; //ButA,PGDN - ButB,PGUP
				f = min(max(keystone.curz,-vw.aspz),vw.aspz);
				if (f != 0.f)
				{
					voxie_project(vw.dispcur,1,keystone.curx,keystone.cury,0.f,&fx,&fy);
					//calwarp2d_addblob(&gcalwarp2d[vw.dispcur],fx,fy,1.0 /*/32768.0*/,f);
					//calwarp2d_rendbmp(&gcalwarp2d[vw.dispcur]);
					//gen_izlut(vw.dispcur);
				}

				stupiang += (float)((voxie_keystat(0x34)!=0)-(voxie_keystat(0x33)!=0))*dtim*.25f;

				z = 0;
				for(i=2;i<128;i++)
					//for(z=-1;z<=1;z++)
					{
						fx = cos((double)i*(PI*2.0*(sqrt(5.0)-1.0)/2.0) + stupiang) * sqrt((double)i)*vw.aspr*0.085;
						fy = sin((double)i*(PI*2.0*(sqrt(5.0)-1.0)/2.0) + stupiang) * sqrt((double)i)*vw.aspr*0.085;
						fz = (float)z*vw.aspz*0.65f + stupiz;
						voxie_drawsph(&vf,fx,fy,fz,.05f,0,0xffffff);
					}

				break;
				}
#endif
			case RENDMODE_HEIGHTMAP: //render heightmap from file
				{
				static float avghgt = -1.f;
				static int autoadj = 1;
				float fval;

				if (labs(vx[0].tx0) < 4096) vx[0].tx0 = 0;
				if (labs(vx[0].ty0) < 4096) vx[0].ty0 = 0;
				if (labs(vx[0].tx1) < 4096) vx[0].tx1 = 0;
				if (labs(vx[0].ty1) < 4096) vx[0].ty1 = 0;

					//Cycle image
				if ((voxie_keystat(0xc9) == 1) || (ghitkey == 0xc9) || (voxie_keystat(0x27) == 1) || ((vx[0].but&~ovxbut[0])&((1<<14)|(1<<8))) || (nav[0].but&~onavbut[0]&1)) { ghitkey = 0; gdemi--; if (gdemi <      0) gdemi = max(gdemn-1,0); cycletim = tim+5.0; } //PGUP
				if ((voxie_keystat(0xd1) == 1) || (ghitkey == 0xd1) || (voxie_keystat(0x28) == 1) || ((vx[0].but&~ovxbut[0])&((1<<13)|(1<<9))) || (nav[0].but&~onavbut[0]&2)) { ghitkey = 0; gdemi++; if (gdemi >= gdemn) gdemi =              0; cycletim = tim+5.0; } //PGDN
				if ((gautocycle) && (tim > cycletim))
				{
					gdemi++; if (gdemi >= gdemn) gdemi = 0;
					cycletim = tim+5.0;
				}

				if ((voxie_keystat(0x35) == 1) || (ghitkey == 0x0e) || (voxie_keystat(0x0e) == 1) || ((vx[0].but&~ovxbut[0])&(1<<4))) //'/' or Backspace:Reset view
				{
					ghitkey = 0;
					gdem[gdemi].p = gdem[gdemi].sp;
					gdem[gdemi].r = gdem[gdemi].sr;
					gdem[gdemi].d = gdem[gdemi].sd;
					gdem[gdemi].f = gdem[gdemi].sf;
				}

					//auto-adjust height smoothly, using avghgt from previous voxie_drawheimap()
				//if (voxie_keystat(0x32) == 1) { autoadj = !autoadj; } //M:toggle autoadj
				if ((autoadj) && (avghgt >= 0.f) && (avghgt != 0x80000000))
				{
						//gdem[gdemi].p.z + gdem[gdemi].f.z*avghgt/256.0 = 0.0
					gdem[gdemi].p.z += (gdem[gdemi].f.z*avghgt/-256.f - gdem[gdemi].p.z)*.125f;
				}

					//Move (scroll/pan)
				gdem[gdemi].p.x += - nav[0].dx*dtim*.012f - nav[0].ax*dtim*.008f; //Mouse
				gdem[gdemi].p.y += - nav[0].dy*dtim*.012f - nav[0].ay*dtim*.008f;
				gdem[gdemi].p.x += ((float)((voxie_keystat(keyremap[0][0])!=0) - (voxie_keystat(keyremap[0][1])!=0)))*dtim; //Rig - Lef
				gdem[gdemi].p.y += ((float)((voxie_keystat(keyremap[0][2])!=0) - (voxie_keystat(keyremap[0][3])!=0)))*dtim; // Up - Dow
				//if (voxie_keystat(0x1e)) { gdem[gdemi].p.z += dtim*128.f/gdem[gdemi].f.z; } //A
				//if (voxie_keystat(0x1f)) { gdem[gdemi].p.z -= dtim*128.f/gdem[gdemi].f.z; } //S

				i = vx[0].tx0;
				j = vx[0].ty0;
				if (i|j)
				{
					f = ((float)i)*-.0000001f;
					g = ((float)j)*+.0000001f;
					gdem[gdemi].p.x += gdem[gdemi].r.x*f + gdem[gdemi].d.x*g;
					gdem[gdemi].p.y += gdem[gdemi].r.y*f + gdem[gdemi].d.y*g;
				}

				fval = (float)(((voxie_keystat(0x34)!=0) || voxie_keystat(keyremap[1][1])) -
									((voxie_keystat(0x33)!=0) || voxie_keystat(keyremap[1][0])))*dtim;
				fval += nav[0].az*dtim*.008f + vx[0].tx1*.0000005f;
				if (fval != 0.f)
				{
					float c, s, rd, u, v;

					fx = gdem[gdemi].r.x; gx = gdem[gdemi].d.x;
					fy = gdem[gdemi].r.y; gy = gdem[gdemi].d.y;
																			  gz = gdem[gdemi].f.z;

					c = cos(fval); s = sin(fval);
					g = gdem[gdemi].r.x;
					gdem[gdemi].r.x = gdem[gdemi].r.x*c + gdem[gdemi].r.y*s;
					gdem[gdemi].r.y = gdem[gdemi].r.y*c -               g*s;
					g = gdem[gdemi].d.x;
					gdem[gdemi].d.x = gdem[gdemi].d.x*c + gdem[gdemi].d.y*s;
					gdem[gdemi].d.y = gdem[gdemi].d.y*c -               g*s;

					rd = 1.f/(fx*gy - fy*gx);
					u = (gdem[gdemi].p.x*gy - gdem[gdemi].p.y*gx)*rd;
					v = (gdem[gdemi].p.y*fx - gdem[gdemi].p.x*fy)*rd;
					gdem[gdemi].p.x = gdem[gdemi].r.x*u + gdem[gdemi].d.x*v;
					gdem[gdemi].p.y = gdem[gdemi].r.y*u + gdem[gdemi].d.y*v;
				}

					//Zoom, preserving center
				fval = 1.f;
				if (voxie_keystat(0x1e)) fval *= pow(0.25,(double)dtim); //A
				if (voxie_keystat(0x2c)) fval *= pow(4.00,(double)dtim); //Z
				if (voxie_keystat(keyremap[0][4])) fval *= pow(4.00,(double)dtim); //ButA
				if (voxie_keystat(keyremap[0][5])) fval *= pow(0.25,(double)dtim); //ButB
				if (vx[0].lt) fval /= pow(1.0+vx[0].lt/64.0,(double)dtim); //Xbox
				if (vx[0].rt) fval *= pow(1.0+vx[0].rt/64.0,(double)dtim); //Xbox
				if (fz != 0.f) fval *= pow(0.5,(double)fz*.001f); //dmousz
				fval += nav[0].dz*dtim*.005f;
				if (fval != 1.f)
				{
					float rd, u, v;
					fx = gdem[gdemi].r.x; gx = gdem[gdemi].d.x;
					fy = gdem[gdemi].r.y; gy = gdem[gdemi].d.y;
																			  gz = gdem[gdemi].f.z;

					gdem[gdemi].r.x *= fval; gdem[gdemi].d.x *= fval;
					gdem[gdemi].r.y *= fval; gdem[gdemi].d.y *= fval;
																					  gdem[gdemi].f.z *= fval;

						//0 = gdem[gdemi].p.x + fx*u + gx*v
						//0 = gdem[gdemi].p.y + fy*u + gy*v
						//0 = gdem[gdemi].p.x'+ gdem[gdemi].r.x*u + gdem[gdemi].d.x*v
						//0 = gdem[gdemi].p.y'+ gdem[gdemi].r.y*u + gdem[gdemi].d.y*v

						//fx*u + fy*v = gdem[gdemi].p.x
						//gx*u + gy*v = gdem[gdemi].p.y
					rd = 1.f/(fx*gy - fy*gx);
					u = (gdem[gdemi].p.x*gy - gdem[gdemi].p.y*gx)*rd;
					v = (gdem[gdemi].p.y*fx - gdem[gdemi].p.x*fy)*rd;
					gdem[gdemi].p.x = gdem[gdemi].r.x*u + gdem[gdemi].d.x*v;
					gdem[gdemi].p.y = gdem[gdemi].r.y*u + gdem[gdemi].d.y*v;

						//gdem[gdemi].p.z_old +              gz*f/256.0 = 0.0
						//gdem[gdemi].p.z     + gdem[gdemi].f.z*f/256.0 = 0.0
					gdem[gdemi].p.z *= gdem[gdemi].f.z/gz;
				}

				if (vx[0].but&((1<<0)|(1<<1)))
				{
					gz = gdem[gdemi].f.z;

					//gdem[gdemi].f.z *= pow(4.0,(double)dtim*(double)vx[0].ty1*.00002);

					f = 1.0;
					if (vx[0].but&(1<<0)) f *= pow(0.25,(double)dtim);
					if (vx[0].but&(1<<1)) f *= pow(4.00,(double)dtim);
					gdem[gdemi].f.z /= f;

					gdem[gdemi].p.z *= gdem[gdemi].f.z/gz;
				}
				if (voxie_keystat(keyremap[1][4]) || voxie_keystat(keyremap[1][5]) || voxie_keystat(0xb5) || voxie_keystat(0x37))
				{
					gz = gdem[gdemi].f.z;

					f = 1.0;
					if (voxie_keystat(keyremap[1][5])) f *= pow(0.25,(double)dtim);
					if (voxie_keystat(keyremap[1][4])) f *= pow(4.00,(double)dtim);
					if (voxie_keystat(0x37)) f *= pow(0.25,(double)dtim);
					if (voxie_keystat(0xb5)) f *= pow(4.00,(double)dtim);
					gdem[gdemi].f.z /= f;

					gdem[gdemi].p.z *= gdem[gdemi].f.z/gz;
				}

				if (voxie_keystat(0x17) == 1) { gdem[gdemi].f.x *= -1.f; gdem[gdemi].f.y *= -1.f; gdem[gdemi].f.z *= -1.f; } //I

				if (voxie_keystat(0x3b) == 1) { gheightmap_flags ^= 1; }
				if (voxie_keystat(0x3c) == 1) { gheightmap_flags ^= 2; }
				if (voxie_keystat(0x3d) == 1) { gheightmap_flags ^= 4; }
				if (voxie_keystat(0x3f) == 1) { gheightmap_flags ^= 16; }
				if (gdem[gdemi].mapzen) gheightmap_flags |= 32; else gheightmap_flags &= ~32;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
				avghgt = voxie_drawheimap(&vf,gdem[gdemi].file,&gdem[gdemi].p,&gdem[gdemi].r,&gdem[gdemi].d,&gdem[gdemi].f, gdem[gdemi].colorkey,0 /*reserved*/,gheightmap_flags);
				if (avghgt == 0x80000000)
				{
						//Display error if bad image
					point3d pp, rr, dd;
					pp.x = 0.0; rr.x = 20.0; dd.x =   0.0;
					pp.y = 0.0; rr.y =  0.0; dd.y =  60.0;
					pp.z = 0.0; rr.z =  0.0; dd.z =   0.0;
					voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s not found",gdem[gdemi].file);
				}

				}
				break;
			case RENDMODE_VOXIEPLAYER: //Voxie player
				{
				static int oreci = -1, newframe = 0, validfil = 0;
				static voxie_rec_t vr = {0};

					//Cycle image
				if ((voxie_keystat(keyremap[0][4]) == 1) || (ghitkey == 0xc9) || ((vx[0].but&~ovxbut[0])&((1<<14)|(1<<8)))) { ghitkey = 0; greci--; if (greci <      0) greci = max(grecn-1,0); } //PGUP
				if ((voxie_keystat(keyremap[0][5]) == 1) || (ghitkey == 0xd1) || ((vx[0].but&~ovxbut[0])&((1<<13)|(1<<9)))) { ghitkey = 0; greci++; if (greci >= grecn) greci =              0; } //PGDN

				if ((!validfil) || (oreci != greci))
				{
					if (validfil > 0) { voxie_rec_close(&vr); validfil = 0; }

					oreci = greci;

					validfil = voxie_rec_open(&vr,grec[greci].file,0,0);
					if (validfil > 0)
					{
						switch(grec[greci].mode)
						{
							case 0: vr.framecur = 0;             break; //forward
							case 1: vr.framecur = 0;             break; //ping-pong
							case 2: vr.framecur = vr.framenum-1; break; //reverse
						}
						vr.timleft = vr.frametim[vr.framecur];

						vr.currep = 0; newframe = 1;
					}
				}
				if (validfil <= 0)
				{
					point3d pp, rr, dd;
					pp.x = 0.0; rr.x = 20.0; dd.x =   0.0;
					pp.y = 0.0; rr.y =  0.0; dd.y =  60.0;
					pp.z = 0.0; rr.z =  0.0; dd.z =   0.0;
					voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"File not found");
					break;
				}

				if ((voxie_keystat(keyremap[0][2]) == 1) || (voxie_keystat(keyremap[0][3]) == 1)) { gvoxieplayer_ispaused ^= 1; }
				if (voxie_keystat(keyremap[0][6]) == 1)
				{
					gautocycleall = !gautocycleall;
					if (!gautocycleall) voxie_playsound("c:/windows/media/recycle.wav",-1,200,200,1.f);
										else voxie_playsound("c:/windows/media/chimes.wav",-1,200,200,1.f);
				}

				voxie_rec_play(&vr,(newframe!=0)+1); newframe = 0;

				if (gvoxieplayer_ispaused)
				{
					point3d pp, rr, dd;
					char tbuf[256];
					pp.x =  0.00f; rr.x = 0.16; dd.x = 0.00;
					pp.y = -0.12f; rr.y = 0.00; dd.y = 0.24;
					pp.z = -vw.aspz+0.01f; rr.z = 0.00; dd.z = 0.00;
					strcpy(tbuf,"Paused");
					f = (float)strlen(tbuf)*.5; pp.x -= rr.x*f; pp.y -= rr.y*f; pp.z -= rr.z*f;
					voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",tbuf);
				}
				else
				{
					vr.timleft -= dtim;
					if (vr.timleft < 0.f)
					{
						newframe = 1;
						switch(grec[greci].mode)
						{
							case 0: vr.framecur++; break;
							case 1: if (!(vr.currep&1)) vr.framecur++; else vr.framecur--; break;
							case 2: vr.framecur--; break;
						}
						if ((unsigned)vr.framecur >= (unsigned)vr.framenum)
						{
							switch(grec[greci].mode)
							{
								case 0: vr.framecur = 0; break;
								case 1: if (!(vr.currep&1)) vr.framecur = vr.framenum-1; else vr.framecur = 0; break;
								case 2: vr.framecur = vr.framenum-1; break;
							}
							vr.currep++;
							if ((vr.currep >= grec[greci].rep) && (gautocycleall))
								{ vr.currep = 0; greci++; if (greci >= grecn) { greci = 0; } }
						}
						vr.timleft = max(vr.timleft+vr.frametim[vr.framecur],0.05);
					}
				}

				if (gdrawstats)
				{
					point3d pp, rr, dd;
					voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
					rr.x = 0.10f; dd.x = 0.00f; pp.x = vw.aspx*0.10f;
					rr.y = 0.00f; dd.y = 0.20f; pp.y = vw.aspy*-.99f;
					rr.z = 0.00f; dd.z = 0.00f; pp.z =-vw.aspz+0.01f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%d/%d",vr.framecur,vr.framenum);
				}

				}
				break;
			case RENDMODE_PLATONICSOLIDS: //Platonic solids
				{
				static double ptim = 0.0;
				static float xof = 0.f, yof = 0.f, zof = 0.f;
				int xn, yn;

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

				//for(i=1;i<=7;i++) if (voxie_keystat(i+0x3a)) platonic_col = i; //F1-F7
				if ((voxie_keystat(keyremap[0][2]) == 1) || (vx[0].but&~ovxbut[0])&(1<< 8)) { platonic_col--; if (platonic_col <= 0) platonic_col = 7; }
				if ((voxie_keystat(keyremap[0][3]) == 1) || (vx[0].but&~ovxbut[0])&(1<< 9)) { platonic_col++; if (platonic_col >= 8) platonic_col = 1; }

				xof += nav[0].dx*dtim*.008f;
				yof += nav[0].dy*dtim*.008f;
				zof += nav[0].dz*dtim*.008f;
				if (nav[0].but&~onavbut[0]&1) { xof = 0.f; yof = 0.f; zof = 0.f; }

				if (voxie_keystat(keyremap[0][6]) == 1) { platonic_ispaused ^= 1; }
				if (!platonic_ispaused) ptim += dtim;

				if (voxie_keystat(keyremap[0][0]) == 1) platonic_solmode = max(platonic_solmode-1,0);
				if (voxie_keystat(keyremap[0][1]) == 1) platonic_solmode = min(platonic_solmode+1,3);
				//if (voxie_keystat(0x2c)) platonic_solmode = 0; //Z
				//if (voxie_keystat(0x2d)) platonic_solmode = 1; //X
				//if (voxie_keystat(0x2e)) platonic_solmode = 2; //C
				//if (voxie_keystat(0x2f)) platonic_solmode = 3; //V
				if (vx[0].lt >= 128) platonic_solmode = 0;
				if (vx[0].rt >= 128) platonic_solmode = 2;

				xn = (int)(vw.aspx/vw.aspz+.49f);
				yn = (int)(vw.aspy/vw.aspz+.49f);
				for(y=0;y<yn;y++)
					for(x=0;x<xn;x++)
						draw_platonic((x+y*2+3)%5,(x+.5)*2.0*vw.aspx/(float)xn-vw.aspx+xof,
														  (y+.5)*2.0*vw.aspy/(float)yn-vw.aspy+yof,zof,vw.aspz,ptim*.5+x*.5+y,platonic_solmode,(platonic_col&1)*0xff + ((platonic_col>>1)&1)*0xff00 + ((platonic_col>>2)&1)*0xff0000);
				}
				break;
			case RENDMODE_FLYINGSTUFF: //Rotating text & flying crap
				{
				point3d pp, rr, dd, ff, pp2;
				static float ha = 0.f, va = 0.f;
				static int platcol24[5] = {0x010000,0x000100,0x010100,0x010001,0x000101};

				f = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
				voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f);

				va = tim; //cos(tim*1.0)*PI*0.5 + PI*0.5;

				for(i=0;i<5;i++)
				{
					ha = (float)i*(PI*2.0/5.0) + tim*.5;
					f = 0.327;
					if (vw.clipshape) f *= 1.4;
					draw_platonic(i,sin(ha)*f,cos(ha)*-f,sin((float)i*PI*4.0/5.0+tim*2.0)*0.05,0.20,tim*2.0,2,platcol24[i]*255);
				}

				//for(i=0;i<12;i++)
				//{
				//   float f = (sin(tim*2.0)*.25+.75)*0.2;
				//   voxie_drawsph(&vf,cos(tim+i*PI*2/12.0)*-1.024,
				//                     sin(tim+i*PI*2/12.0)*+1.055,
				//                     cos(tim*2.0+i*PI*4/12.0)*(vw.aspz/vw.aspx-(f-0.00625)),
				//                     f,0,0xffffff);
				//}

				for(i=0;i<20;i++)
				{
					ha = (float)i*(PI*2.0/20.0) + tim*.5;
					pp.x = sin(ha)*0.75;
					pp.y =-cos(ha)*0.75;
					pp.z = 0.f;

					if (vw.clipshape) { pp.x *= 1.43f; pp.y *= 1.43f; }

					rr.x = cos(ha); dd.x =-sin(ha)*sin(va);
					rr.y = sin(ha); dd.y = cos(ha)*sin(va);
					rr.z = 0.0;     dd.z =         cos(va);
					ff.x = rr.y*dd.z - rr.z*dd.y;
					ff.y = rr.z*dd.x - rr.x*dd.z;
					ff.z = rr.x*dd.y - rr.y*dd.x;
					f = 0.25;
					rr.x *= f; rr.y *= f; rr.z *= f;
					dd.x *= f; dd.y *= f; dd.z *= f;
					ff.x *= f; ff.y *= f; ff.z *= f;
					f = 2.65; dd.x *= f; dd.y *= f; dd.z *= f;
					f = 0.5; pp.x -= rr.x*f; pp.y -= rr.y*f; pp.z -= rr.z*f;
					f = 0.5; pp.x -= dd.x*f; pp.y -= dd.y*f; pp.z -= dd.z*f;

					//pp.z += 0.01f;
					//voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%c",gmyst[i]);
					//pp.z -= 0.02f;
					j = (i%5)+2; j = ((j>>2)&1)*0xff0000 + ((j>>1)&1)*0xff00 + (j&1)*0xff;
					for(f=-.5f;f<=.5f;f+=1.f/32.f)
					{
						pp2.x = pp.x + ff.x*f;
						pp2.y = pp.y + ff.y*f;
						pp2.z = pp.z + ff.z*f;
						voxie_printalph_(&vf,&pp2,&rr,&dd,j,"%c",gmyst[i]);
					}
				}

				//draw_platonic(1,0.0,0.0,0.0,0.2,tim*1.0,3,((int)(sin(tim*2.6)*127+128)) + ((int)(cos(tim*2.3)*127+128))*256 + ((int)(sin(tim*2.1)*127+128))*65536);

				}

				break;
			case RENDMODE_CHESS: //Chess
				{
				static const char *chessnam[] = {"pawn.kv6","knight.kv6","bishop.kv6","rook.kv6","queen.kv6","king.kv6"};
				static const float heioff[6] = {+.0758f,+.0375f,+.0250f,+.0475f,+.0138f,-.0113f};
				static const float sizoff[6] = {0.225,0.300,0.325,0.275,0.350,0.400};
				static double movetim = -1e32, timig = -1e32;
				static int inited = -1, cursx, cursy, fromx, fromy, tox, toy, caststat, prevmove, turn, win;
				static int board[8][8], sboard[8][8] =
				{
					 4, 2, 3, 5, 6, 3, 2, 4,
					 1, 1, 1, 1, 1, 1, 1, 1,
					 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0, 0, 0, 0,
					-1,-1,-1,-1,-1,-1,-1,-1,
					-4,-2,-3,-5,-6,-3,-2,-4,
				};

				f = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
				//voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f); //preserve cube aspect
				//voxie_setview(&vf,-1.f,-1.f,-.2f,+1.f,+1.f,+.2f); //ignore cube aspect and fill space
				voxie_setview(&vf,(-vw.aspx*f-1.f)*.5f,(-vw.aspy*f-1.f)*.5f,(-vw.aspz*f-.2f)*.5f,
										(+vw.aspx*f+1.f)*.5f,(+vw.aspy*f+1.f)*.5f,(+vw.aspz*f+.2f)*.5f); //average of above methods

				if ((voxie_keystat(0x0e) == 1) || (ghitkey == 0x0e) || (voxie_keystat(0x1c) == 1) && (voxie_keystat(0x1d)|voxie_keystat(0x9d))) //Backspace,Ctrl+Enter:reset game
				{
					ghitkey = 0;
					inited = 0; voxie_playsound("opendoor.flac",-1,100,100,1.f);
				}
				if (inited <= 0)
				{
					if (inited < 0) { voxie_mountzip("chessdat.zip"); voxie_playsound("opendoor.flac",-1,100,100,1.f); }

					memcpy(board,sboard,sizeof(board));
					cursx = 4; cursy = 1; fromx = -1; tox = -1;

						//if (caststat&1): no white long  castle
						//if (caststat&2): no white short castle
						//if (caststat&4): no black long  castle
						//if (caststat&8): no black short castle
					caststat = 0; prevmove = -1; turn = 0; win = -1;

					inited = 1;
				}

#if (USELEAP)
				frame = leap_getframe();
				if ((frame) && (frame->nHands > 0))
				{
					static int onearboard = 0, nearboard = 0;

					hand = &frame->pHands[0];
					digit = &hand->digits[1];
					vec = &digit->stabilized_tip_position;

					fx = vec->x*+.01f    ; fx = min(max(fx,-vw.aspx),vw.aspx);
					fy = vec->z*+.01f    ; fy = min(max(fy,-vw.aspy),vw.aspy);
					fz = vec->y*-.01f+2.f; fz = min(max(fz,-vw.aspz),vw.aspz);
					voxie_drawsph(&vf,fx,fy,fz,.05f,1,0xffffff);
					onearboard = nearboard; nearboard = (fz >= min(vw.aspz,0.2f));
					if ((nearboard > onearboard) && (tox < 0))
					{
						cursx = (int)((1.f+fx)*4.f);
						cursy = (int)((1.f-fy)*4.f);
						if ((cursx != fromx) || (cursy != fromy)) goto chesselect;
					}
				}
#endif

				if (tim > timig)
				{
					if ((min(vx[0].tx0,vx[0].tx1) < -16384) || (nav[0].dx < -100.0f)) { timig = tim+0.25; cursx = max(cursx-1,  0); }
					if ((max(vx[0].tx0,vx[0].tx1) > +16384) || (nav[0].dx > +100.0f)) { timig = tim+0.25; cursx = min(cursx+1,8-1); }
					if ((max(vx[0].ty0,vx[0].ty1) > +16384) || (nav[0].dy < -100.0f)) { timig = tim+0.25; cursy = min(cursy+1,8-1); }
					if ((min(vx[0].ty0,vx[0].ty1) < -16384) || (nav[0].dy > +100.0f)) { timig = tim+0.25; cursy = max(cursy-1,  0); }
				}
				if ((voxie_keystat(keyremap[0][0]) == 1) || (voxie_keystat(keyremap[2][0]) == 1) || ((vx[0].but&~ovxbut[0])&4)) { cursx = max(cursx-1,  0); }
				if ((voxie_keystat(keyremap[0][1]) == 1) || (voxie_keystat(keyremap[2][1]) == 1) || ((vx[0].but&~ovxbut[0])&8)) { cursx = min(cursx+1,8-1); }
				if ((voxie_keystat(keyremap[0][2]) == 1) || (voxie_keystat(keyremap[2][2]) == 1) || ((vx[0].but&~ovxbut[0])&1)) { cursy = min(cursy+1,8-1); }
				if ((voxie_keystat(keyremap[0][3]) == 1) || (voxie_keystat(keyremap[2][3]) == 1) || ((vx[0].but&~ovxbut[0])&2)) { cursy = max(cursy-1,  0); }
				if (((voxie_keystat(keyremap[0][4]) == 1) || (voxie_keystat(0x1c) == 1) || (voxie_keystat(0x9c) == 1) || ((vx[0].but&~ovxbut[0])&0xf3c0) || (nav[0].but&~onavbut[0]&1)) && (tox < 0)) //ButA, Enter
				{
chesselect:;        if ((fromx == cursx) && (fromy == cursy)) fromx = -1;
					else if (ksgn(board[cursy][cursx]) == 1-turn*2) { fromx = cursx; fromy = cursy; voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,3.f); }
					else if (fromx >= 0)
					{
						if (isvalmove(board,caststat,prevmove,fromx,fromy,cursx,cursy))
						{
							i = labs(board[fromy][fromx]);
							if (i == 1) voxie_playsound("zipguns.flac",-1,100,100,1.f);
							if (i == 2) voxie_playsound("shoot2.flac",-1,100,100,1.f);
							if (i == 3) voxie_playsound("shoot2.flac",-1,100,100,1.f);
							if (i == 4) voxie_playsound("shoot2.flac",-1,100,100,1.f);
							if (i == 5) voxie_playsound("shoot3.flac",-1,100,100,1.f);
							if (i == 6) voxie_playsound("warp.flac",-1,100,100,1.f);
							tox = cursx; toy = cursy; movetim = tim;
						}
						else
						{
							int kx, ky, t;
							t = 1-turn*2;
							for(ky=0;ky<8;ky++)
								for(kx=0;kx<8;kx++)
								{
									if (board[ky][kx] != t*6) continue;
									if (ischeck(board,kx,ky,turn))
										{ voxie_playsound("alarm.flac",-1,100,100,1.f); goto break2; }
								}
							voxie_playsound("bouncy.flac",-1,100,100,1.f);
break2:;          }
					}
				}

				if ((voxie_keystat(0x23) == 1) && (voxie_keystat(0x1d) || voxie_keystat(0x9d))) gchess_automove = !gchess_automove; //Ctrl+H
				if (((gchessailev[turn]) || (gchess_automove) || ((voxie_keystat(0x23) == 1) || (gchess_hint))) && (fromx < 0) && (win < 0)) //H:hint
				{
					gchess_hint = 0;
					i = gchessailev[turn]; if (!i) i = gchessailev[turn^1];
					if (!getcompmove(board,&caststat,&prevmove,turn,&fromx,&fromy,&tox,&toy,i)) goto dowin;
					movetim = voxie_klock();

					i = labs(board[fromy][fromx]);
					if (i == 1) voxie_playsound("zipguns.flac",-1,100,100,1.f);
					if (i == 2) voxie_playsound("shoot2.flac",-1,100,100,1.f);
					if (i == 3) voxie_playsound("shoot2.flac",-1,100,100,1.f);
					if (i == 4) voxie_playsound("shoot2.flac",-1,100,100,1.f);
					if (i == 5) voxie_playsound("shoot3.flac",-1,100,100,1.f);
					if (i == 6) voxie_playsound("warp.flac",-1,100,100,1.f);
				}
				if ((gchess_automove) && (win >= 0) && (tim >= movetim+10.0)) //restart in auto mode
					{ inited = 0; break; }

				n = getvalmoves(board,caststat,prevmove,turn,0);
				if (n == 0)
				{
dowin:;        if (win < 0)
						for(y=0;y<8;y++)
							for(x=0;x<8;x++)
								if (board[y][x] == (1-turn*2)*6)
								{
									if (!ischeck(board,x,y,turn)) //stalemate
										{ win = 2; voxie_playsound("shoot4.flac",-1,100,100,1.f); }
									else { win = 1-turn; voxie_playsound("closdoor.flac",-1,100,100,1.f); }
									goto foundking1;
								}
foundking1:;}

				if ((voxie_keystat(keyremap[0][5]) == 1) || ((vx[0].but&~ovxbut[0])&(1<<4))) //Backspace:reset
				{
					if (gmoven >= 2)
					{
						gmoven -= 2; voxie_playsound("ouch.flac",-1,100,100,1.f);

						for(y=0;y<8;y++) for(x=0;x<8;x++) board[y][x] = sboard[y][x];
						turn = 0; win = -1; caststat = 0; prevmove = -1;
						for(i=0;i<gmoven;i++)
						{
							move2xys(gmove[i],&fromx,&fromy,&tox,&toy);
							domove(board,&caststat,&prevmove,fromx,fromy,tox,toy,0);
							turn = 1-turn;
						}
						fromx = -1; tox = -1;
					}
				}

					//Draw grid lines
				//for(f=-1.f;f<=1.f;f+=.25f)
				//{
				//   voxie_drawlin(&vf,f,-1.f,.195f,f,+1.f,.195f,0xffffff);
				//   voxie_drawlin(&vf,-1.f,f,.195f,+1.f,f,.195f,0xffffff);
				//}

				for(y=0;y<8;y++)
					for(x=0;x<8;x++)
					{
						fx = ((float)x-3.5f)* .25f;
						fy = ((float)y-3.5f)*-.25f;

						//if ((x+y)&1) voxie_drawbox(&vf,fx-.12f,fy-.12f,.195f,fx+.12f,fy+.12f,.199f,2,0x0000ff); //filled squares (old; too bright)
						//voxie_drawbox(&vf,fx-.12f,fy-.12f,.195f,fx+.12f,fy+.12f,.199f,2,((x+y)&1)*(0xffff00-0x00ffff) + 0x00ffff); //filled squares (old; too bright)
						//voxie_drawbox(&vf,fx-.11f,fy-.11f,.17f,fx+.11f,fy+.11f,.22f,3,((x+y)&1)*(0x101000-0x001010) + 0x001010); //solid-filled boxes
						//for(f=-.11f;f<=.11f;f+=.016f) voxie_drawlin(&vf,fx+f,fy-.11f,.199f,fx+f,fy+.11f,.199f,((x+y)&1)*(0xffff00-0x00ffff) + 0x00ffff); //filled squares (new)

						poltex_t ptx[4];
						int mesh[5];
						f = 0.11f;
						ptx[0].x = fx-f; ptx[0].y = fy-f; ptx[0].z = 0.199f; ptx[0].u = 0.f; ptx[0].v = 0.f; ptx[0].col = 0xffffff;
						ptx[1].x = fx+f; ptx[1].y = fy-f; ptx[1].z = 0.199f; ptx[1].u = 1.f; ptx[1].v = 0.f; ptx[1].col = 0xffffff;
						ptx[2].x = fx+f; ptx[2].y = fy+f; ptx[2].z = 0.199f; ptx[2].u = 1.f; ptx[2].v = 1.f; ptx[2].col = 0xffffff;
						ptx[3].x = fx-f; ptx[3].y = fy+f; ptx[3].z = 0.199f; ptx[3].u = 0.f; ptx[3].v = 1.f; ptx[3].col = 0xffffff;
						mesh[0] = 0; mesh[1] = 1; mesh[2] = 2; mesh[3] = 3; mesh[4] = -1;
						voxie_drawmeshtex(&vf,0 /*"klab\\hole.png"*/,ptx,4,mesh,5,2,(((-((x+y)&1))&(0x010100-0x000101)) + 0x000101)*128);

							//draw chess piece
						if (board[y][x])
						{
							point3d pp, rr, dd, ff;
							i = labs(board[y][x])-1;
							pp.x = fx; pp.y = fy; pp.z = heioff[i];
							rr.x = sizoff[i]; rr.y = 0.0f; rr.z = 0.0f;
							dd.x = 0.0f; dd.y = sizoff[i]; dd.z = 0.0f; if (board[y][x] == -2) { rr.x *= -1.f; dd.y *= -1.f; }
							ff.x = 0.0f; ff.y = 0.0f; ff.z = sizoff[i];
							if ((tox >= 0) && (x == fromx) && (y == fromy))
							{
								f = (tim-movetim)/gchesstime;
								//pp.z -= (cos(f*PI*2.f)*-.5f + .5f)*.1f;
								//rotate_vex((cos(f*PI*2.f)*-.5f + .5f)*.25f,&dd,&ff);
								//rotate_vex(f*PI*2.0,&dd,&ff);
								f = cos(f*PI)*-.5f + .5f;
								f *= .25f;
								pp.x += (tox-fromx)*f;
								pp.y -= (toy-fromy)*f;
							}
							if (board[y][x] > 0) j = gchesscol[0];
							if (board[y][x] < 0) j = gchesscol[1];
							voxie_drawspr(&vf,chessnam[i],&pp,&rr,&dd,&ff,j);

							if (board[y][x] < 0)
							{
								f = sizoff[i]*.7f;
								rr.x = 0.f; rr.y = cos(.5f)*f; rr.z = sin(.5f)*f;
								dd.x = 0.f; dd.y =-sin(.5f)*f; dd.z = cos(.5f)*f;
								ff.x =   f; ff.y =        0.f; ff.z =        0.f;
								pp.x -= .07f; pp.y += .02f; pp.z -= .05f;
								if (board[y][x] == -1) pp.x += .02f;
								voxie_drawspr(&vf,"sword.kv6",&pp,&rr,&dd,&ff,0x808080);
							}
						}
					}

					//finish move
				if ((tox >= 0) && (tim >= movetim+gchesstime))
				{
					domove(board,&caststat,&prevmove,fromx,fromy,tox,toy,1);
					turn = 1-turn; fromx = -1; tox = -1;
				}

				if (win < 0)
				{
					if ((!gchessailev[turn]) && (tim >= movetim+gchesstime))
					{
						if (!gchess_automove)
						{
							fx = ((float)cursx-3.5f)*.25f; fy = ((float)cursy-3.5f)*-.25f;
							drawcube_thickwire(&vf,fx-.125f,fy-.125f,+.17f,fx+.125f,fy+.125f,+.20f,.01f,0xffffff);
						}
						if ((fromx >= 0) && (tox < 0))
						{
							x0 = ((float)fromx-3.5f)*.25f; y0 = ((float)fromy-3.5f)*-.25f;
							if ((fromx == cursx) && (fromy == cursy))
							{
								voxie_drawsph(&vf,x0,y0,.17f,.03f,1,0xffffff);
							}
							else
							{
								x1 = ((float)cursx-3.5f)*.25f; y1 = ((float)cursy-3.5f)*-.25f;
								f = 0.15/sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
								voxie_drawcone(&vf,x0,y0,.17f,.03f,(x0-x1)*f+x1,(y0-y1)*f+y1,.17f,.03f,1,0xffffff);
								voxie_drawcone(&vf,(x0-x1)*f+x1,(y0-y1)*f+y1,.17f,.05f,x1,y1,.17f,.001f,1,0xffffff);
							}
						}
					}
				}
				else
				{
					char buf[256];
					point3d pp, rr, dd;
					pp.x =  0.00f; rr.x = 0.16; dd.x = 0.00;
					pp.y = -0.12f; rr.y = 0.00; dd.y = 0.24;
					f = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
					pp.z = (-vw.aspz*f-.2f)*.5f+.01f; rr.z = 0.00; dd.z = 0.00;
					if (win < 2) strcpy(buf,"Checkmate!");
							  else strcpy(buf,"Stalemate");
					f = (float)strlen(buf)*.5; pp.x -= rr.x*f; pp.y -= rr.y*f; //pp.z -= rr.z*f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);
				}

				}
				break;
			case RENDMODE_PACKER: //Packer
				{
				#define PIECEDRAWFRAC 1.0
				#define MAXBSIZ 20
				static int bsizx, bsizy, bsizz;
				#define MOVETIM 0.125
				static const int shape[][4][3] =
				{
						//Fits in 2 blocks
					0, 0, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0, //Classic 2x2 box
					0, 0, 0,  1, 0, 0,  0, 1, 0,  0, 0, 1, //3D-only half axes
					0, 0, 0,  1, 0, 0,  0, 1, 0,  1, 0, 1, //3D-only bent ~

						//Fits in 3 blocks
					0, 0, 0,  0,-1, 0,  0, 1, 0,  1, 1, 0, //Classic 'L'
					0, 0, 0,  0,-1, 0,  1, 0, 0,  1, 1, 0, //Classic '~'
					0, 0, 0,  0,-1, 0,  1, 0, 0,  0, 1, 0, //Classic half +

						//Fits in 4 blocks
					0, 0, 0, -1, 0, 0,  1, 0, 0,  2, 0, 0, //Classic 4x1 line (WARNING:DO NOT ENABLE! would crash piece fitter because bsizz < 4!)
				};
				#define MAXSHAPES (sizeof(shape)/(sizeof(shape[0][0][0])*3*4))
				static const int shapecol[] = {0xfcfcfc,0xfc00fc,0xfcfc00,0x00fc00,0x00fcfc,0xfc0000,0x0000fc};
				static int board[MAXBSIZ][MAXBSIZ][MAXBSIZ] = {0}, piece = -1, rot, nrot, rotdir = -1, score = 0;
				static int piecex, piecey, piecez, opiecey, npiecex, npiecey, npiecez, doingmove = 0;
				static double droptimleft = 8.0, rottimleft = 0.0, timig = -1e32;
				int xx, yy, zz, nx, ny, nz, cnt, gotmove = 0, drawonsid, nshapes;

				//voxie_setview(&vf,-1.f,-1.f,-.2f,+1.f,+1.f,+.2f);
					//vw.aspx = 1.0
					//vw.aspy = 1.0
					//vw.aspz = 0.2
					//bsizx/2 = 0.372 / 1.0 = 0.372
					//bsizy/2 = 0.930 / 1.0 = 0.930
					//bsizz/2 = 0.186 / 0.2 = 0.930
				drawonsid = (vw.aspx > vw.aspy);
				//i = (vw.aspx > vw.aspy); if (i != vw.flip) { vw.flip = i; voxie_init(&vw); } //Hack:doesn't work right

					//Alt+Arrows: change board size
				if ((voxie_keystat(0xb8) && (voxie_keystat(0xcb) == 1)) || (ghitkey == 0xcb)) { ghitkey = 0; gpakxdim = max(gpakxdim-1,      2); score = 0; memset(board,0,sizeof(board)); piece = -1; }
				if ((voxie_keystat(0xb8) && (voxie_keystat(0xcd) == 1)) || (ghitkey == 0xcd)) { ghitkey = 0; gpakxdim = min(gpakxdim+1,MAXBSIZ); score = 0; memset(board,0,sizeof(board)); piece = -1; }
				if ((voxie_keystat(0xb8) && (voxie_keystat(0xc8) == 1)) || (ghitkey == 0xc8)) { ghitkey = 0; gpakydim = max(gpakydim-1,      2); score = 0; memset(board,0,sizeof(board)); piece = -1; }
				if ((voxie_keystat(0xb8) && (voxie_keystat(0xd0) == 1)) || (ghitkey == 0xd0)) { ghitkey = 0; gpakydim = min(gpakydim+1,MAXBSIZ); score = 0; memset(board,0,sizeof(board)); piece = -1; }
				bsizx = gpakxdim; bsizz = gpakydim;

				i = min(bsizx,bsizz); if (i < 2) break;
				switch(i)
				{
					case 2: nshapes = 3; break;
					case 3: nshapes = 6; break;
					default: nshapes = 7; break;
				}

				if (!drawonsid) { bsizy = min(max((int)(max(bsizx/vw.aspx,bsizz/vw.aspz)*vw.aspy),8),MAXBSIZ); f = max((bsizx*.5f)/vw.aspx,(bsizy*.5f)/vw.aspy); }
							  else { bsizy = min(max((int)(max(bsizx/vw.aspy,bsizz/vw.aspz)*vw.aspx),8),MAXBSIZ); f = max((bsizx*.5f)/vw.aspy,(bsizy*.5f)/vw.aspx); }
				f = max(f,(bsizz*.5f)/vw.aspz)*1.001;
				voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f);

				if (drawonsid)
				{
					j = 0;
					f = nav[j].dx; nav[j].dx = -nav[j].dy; nav[j].dy = f;
					f = nav[j].ax; nav[j].ax = -nav[j].ay; nav[j].ay = f;
				}

				if ((voxie_keystat(0x0e) == 1) || (ghitkey == 0x0e) || (voxie_keystat(keyremap[0][6]) == 1) || ((vx[0].but&~ovxbut[0])&(1<<4))) { ghitkey = 0; score = 0; memset(board,0,sizeof(board)); piece = -1; } //Backspace:reset
				if (voxie_keystat(keyremap[0][6]) == 1) { break; } //ButM:reset&title
				if (voxie_keystat(0x02) == 1) { gpakrendmode = 1; } //thin wireframe
				if (voxie_keystat(0x03) == 1) { gpakrendmode = 2; } //thick wireframe
				if (voxie_keystat(0x04) == 1) { gpakrendmode = 4; } //surface
				if (voxie_keystat(0x05) == 1) { gpakrendmode = 6; } //surface & thick wireframe

				if (piece < 0)
				{
					piece = rand()%nshapes; rot = (rand()&63); nrot = rot;
					piecex = ((bsizx-1)>>1); piecey = bsizy-2; piecez = ((bsizz-1)>>1); doingmove = 0;

						//Correct piece position if it starts out of bounds
					for(i=4-1;i>=0;i--)
					{
						x = shape[piece][i][0];
						y = shape[piece][i][1];
						z = shape[piece][i][2];
						packer_rot(nrot,&x,&y,&z);
						x += piecex; y += piecey; z += piecez;
							  if (x <       0) { piecex -= x;           }
						else if (x > bsizx-1) { piecex -= x-(bsizx-1); }
							  if (z <       0) { piecez -= z;           }
						else if (z > bsizz-1) { piecez -= z-(bsizz-1); }
					}

					npiecex = piecex; npiecey = piecey; npiecez = piecez;
				}

				if (!doingmove)
				{
					npiecex = piecex; npiecey = piecey; npiecez = piecez; rotdir = -1;

						//Position
					if (tim > timig)
					{
						j = 0;
						if ((min(vx[0].tx0,vx[0].tx1) < -16384) || ((j >= 0) && (nav[j].dx < -100.0f))) { timig = tim+0.25; npiecex = max(piecex-1,      0); doingmove = 1; }
						if ((max(vx[0].tx0,vx[0].tx1) > +16384) || ((j >= 0) && (nav[j].dx > +100.0f))) { timig = tim+0.25; npiecex = min(piecex+1,bsizx-1); doingmove = 1; }
						if ((max(vx[0].ty0,vx[0].ty1) > +16384) || ((j >= 0) && (nav[j].dz < -100.0f))) { timig = tim+0.25; npiecez = max(piecez-1,      0); doingmove = 1; }
						if ((min(vx[0].ty0,vx[0].ty1) < -16384) || ((j >= 0) && (nav[j].dz > +100.0f))) { timig = tim+0.25; npiecez = min(piecez+1,bsizz-1); doingmove = 1; }

							 //Rotation
						if (!gdualnav) j = 0; else j = 1;
						if (j >= 0)
						{
							if (nav[j].ax < -150.0f) { timig = tim+0.25; rotdir = 2; doingmove = 1; }
							if (nav[j].ax > +150.0f) { timig = tim+0.25; rotdir = 3; doingmove = 1; }
							if (nav[j].ay < -150.0f) { timig = tim+0.25; rotdir = 4; doingmove = 1; }
							if (nav[j].ay > +150.0f) { timig = tim+0.25; rotdir = 5; doingmove = 1; }
							if (nav[j].az < -150.0f) { timig = tim+0.25; rotdir = 0; doingmove = 1; }
							if (nav[j].az > +150.0f) { timig = tim+0.25; rotdir = 1; doingmove = 1; }
						}
					}
						  if ((voxie_keystat(keyremap[0][0]) == 1) || (voxie_keystat(keyremap[2][0]) == 1) || ((vx[0].but&~ovxbut[0])&4)) { npiecex = max(piecex-1,      0); doingmove = 1; }
					else if ((voxie_keystat(keyremap[0][1]) == 1) || (voxie_keystat(keyremap[2][1]) == 1) || ((vx[0].but&~ovxbut[0])&8)) { npiecex = min(piecex+1,bsizx-1); doingmove = 1; }
					else if ((voxie_keystat(keyremap[0][2]) == 1) || (voxie_keystat(keyremap[2][2]) == 1) || ((vx[0].but&~ovxbut[0])&1)) { npiecez = max(piecez-1,      0); doingmove = 1; }
					else if ((voxie_keystat(keyremap[0][3]) == 1) || (voxie_keystat(keyremap[2][3]) == 1) || ((vx[0].but&~ovxbut[0])&2)) { npiecez = min(piecez+1,bsizz-1); doingmove = 1; }
						//Rotation
					else if ((voxie_keystat(0xd2) == 1) || (voxie_keystat(keyremap[1][0]) == 1) || (((vx[0].but&~ovxbut[0])>> 8)&1)) { rotdir = 2; doingmove = 1; }
					else if ((voxie_keystat(0xd3) == 1) || (voxie_keystat(keyremap[1][1]) == 1) || (((vx[0].but&~ovxbut[0])>> 9)&1)) { rotdir = 3; doingmove = 1; }
					else if ((voxie_keystat(0xc7) == 1) || (voxie_keystat(keyremap[1][2]) == 1) || (((vx[0].but&~ovxbut[0])>>15)&1)) { rotdir = 4; doingmove = 1; }
					else if ((voxie_keystat(0xcf) == 1) || (voxie_keystat(keyremap[1][3]) == 1) || (((vx[0].but&~ovxbut[0])>>12)&1)) { rotdir = 5; doingmove = 1; }
					else if ((voxie_keystat(0xc9) == 1) || (voxie_keystat(keyremap[1][4]) == 1) || (((vx[0].but&~ovxbut[0])>>14)&1)) { rotdir = 0; doingmove = 1; }
					else if ((voxie_keystat(0xd1) == 1) || (voxie_keystat(keyremap[1][5]) == 1) || (((vx[0].but&~ovxbut[0])>>13)&1)) { rotdir = 1; doingmove = 1; }

					if (rotdir >= 0)
					{
						int nx, ny, nz;

						x = 1; y = 2; z = 3;
						packer_rot(rot,&x,&y,&z);

						if ((rotdir == 0) || (rotdir == 1)) { j = x; x = y; y = -j; } if (rotdir == 1) { x = -x; y = -y; }
						if ((rotdir == 2) || (rotdir == 3)) { j = x; x = z; z = -j; } if (rotdir == 3) { x = -x; z = -z; }
						if ((rotdir == 4) || (rotdir == 5)) { j = y; y = z; z = -j; } if (rotdir == 5) { y = -y; z = -z; }
						nx = x; ny = y; nz = z;

						for(nrot=64-1;nrot>=0;nrot--)
						{
							x = 1; y = 2; z = 3;
							packer_rot(nrot,&x,&y,&z);
							if ((nx == x) && (ny == y) && (nz == z)) break;
						}
					}

					if (doingmove)
					{
							//Correct piece position if it goes out of bounds on wall
						for(i=4-1;i>=0;i--)
						{
							x = shape[piece][i][0];
							y = shape[piece][i][1];
							z = shape[piece][i][2];
							packer_rot(nrot,&x,&y,&z);
							x += npiecex;
							y += npiecey;
							z += npiecez;
							if (x <      0) { npiecex++; i = 4; continue; }
							if (x >= bsizx) { npiecex--; i = 4; continue; }
							if (z <      0) { npiecez++; i = 4; continue; }
							if (z >= bsizz) { npiecez--; i = 4; continue; }

							if (((unsigned)y < (unsigned)bsizy) && (board[z][y][x])) { doingmove = 0; break; }

						}
						if ((nrot == rot) && (npiecex == piecex) && (npiecey == piecey) && (npiecez == piecez)) doingmove = 0;
						else
						{
							rottimleft = MOVETIM;
							if ((npiecex != piecex) || (npiecez != piecez)) voxie_playsound("c:/windows/media/notify.wav",-1,100,100,3.f);  //move
																					 else voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,3.f); //rotate
						}
					}
				}
				if (doingmove)
				{
					if (piece >= 0)
					{
						int cx[4], cy[4], cz[4];

						rottimleft -= dtim;
						if (rottimleft < 0.0) { rottimleft = 0.0; }

						for(i=0;i<4;i++)
						{
							cx[i] = shape[piece][i][0];
							cy[i] = shape[piece][i][1];
							cz[i] = shape[piece][i][2];
							packer_rot(rot,&cx[i],&cy[i],&cz[i]);
						}
						for(i=0;i<4;i++)
						{
							point3d pp, rr, dd, ff;
							float c, s;

							f = 1.0-(rottimleft/MOVETIM); c = cos(f*PI*.5); s = sin(f*PI*.5);

							rr.x = 1.0; dd.x = 0.0; ff.x = 0.0; pp.x = (float)cx[i] - .5;
							rr.y = 0.0; dd.y = 1.0; ff.y = 0.0; pp.y = (float)cy[i] - .5;
							rr.z = 0.0; dd.z = 0.0; ff.z = 1.0; pp.z = (float)cz[i] - .5;

							if (rotdir&1) s = -s;
							switch (rotdir>>1)
							{
								case 0: fx = pp.x; fy = pp.y; pp.x = c*fx + s*fy; pp.y = c*fy - s*fx;
										  fx = rr.x; fy = rr.y; rr.x = c*fx + s*fy; rr.y = c*fy - s*fx;
										  fx = dd.x; fy = dd.y; dd.x = c*fx + s*fy; dd.y = c*fy - s*fx;
										  fx = ff.x; fy = ff.y; ff.x = c*fx + s*fy; ff.y = c*fy - s*fx; break;
								case 1: fx = pp.x; fy = pp.z; pp.x = c*fx + s*fy; pp.z = c*fy - s*fx;
										  fx = rr.x; fy = rr.z; rr.x = c*fx + s*fy; rr.z = c*fy - s*fx;
										  fx = dd.x; fy = dd.z; dd.x = c*fx + s*fy; dd.z = c*fy - s*fx;
										  fx = ff.x; fy = ff.z; ff.x = c*fx + s*fy; ff.z = c*fy - s*fx; break;
								case 2: fx = pp.y; fy = pp.z; pp.y = c*fx + s*fy; pp.z = c*fy - s*fx;
										  fx = rr.y; fy = rr.z; rr.y = c*fx + s*fy; rr.z = c*fy - s*fx;
										  fx = dd.y; fy = dd.z; dd.y = c*fx + s*fy; dd.z = c*fy - s*fx;
										  fx = ff.y; fy = ff.z; ff.y = c*fx + s*fy; ff.z = c*fy - s*fx; break;
								default:; //must not rotate for (ignore) -1
							}

							pp.x += (npiecex-piecex)*f + piecex + .5 + (bsizx*-.5);
							pp.y += (npiecey-piecey)*f + piecey + .5 + (bsizy*-.5);
							pp.z += (npiecez-piecez)*f + piecez + .5 + (bsizz*-.5);

							if (drawonsid) { f = pp.x; pp.x = pp.y; pp.y = -f; f = rr.x; rr.x = rr.y; rr.y = -f; f = dd.x; dd.x = dd.y; dd.y = -f; f = ff.x; ff.x = ff.y; ff.y = -f; }
							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,1,0xffffff);

							f = (1.f-PIECEDRAWFRAC)*.5f;
							pp.x += (rr.x + dd.x + ff.x)*f;
							pp.y += (rr.y + dd.y + ff.y)*f;
							pp.z += (rr.z + dd.z + ff.z)*f;
							f = PIECEDRAWFRAC; rr.x *= f; rr.y *= f; rr.z *= f; dd.x *= f; dd.y *= f; dd.z *= f; ff.x *= f; ff.y *= f; ff.z *= f;

							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,2,shapecol[piece]>>2);
							drawcube_faces_vis(&vf,&pp,&rr,&dd,&ff,gpakrendmode,shapecol[piece],i,4,cx,cy,cz);
						}

						if (rottimleft <= 0) { doingmove = 0; piecex = npiecex; piecey = npiecey; piecez = npiecez; rot = nrot; }
					}
				}
				else
				{
					droptimleft -= dtim;
					j = 0;
					if ((droptimleft <= 0.0) || voxie_keystat(0x1c) || voxie_keystat(keyremap[0][4]) || ((vx[0].lt|vx[0].rt) > 0) || ((j >= 0) && ((nav[j].dy < -200.0f) || (nav[j].but&1))))
					{
						droptimleft = 8.0;
						rottimleft = MOVETIM; doingmove = 1; npiecex = piecex; npiecey = piecey-1; npiecez = piecez; nrot = rot;
						voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,8.f);
					}
					if (doingmove)
					{
						for(i=4-1;i>=0;i--)
						{
							x = shape[piece][i][0];
							y = shape[piece][i][1];
							z = shape[piece][i][2];
							packer_rot(nrot,&x,&y,&z);
							x += npiecex;
							y += npiecey;
							z += npiecez;
							if ((y < 0) || ((y < bsizy) && (board[z][y][x]))) //solidify if move would put piece out of bounds or intersect an existing block
							{
								voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,1.f);

								for(i=4-1;i>=0;i--)
								{
									x = shape[piece][i][0];
									y = shape[piece][i][1];
									z = shape[piece][i][2];
									packer_rot(rot,&x,&y,&z);
									x += piecex;
									y += piecey;
									z += piecez;
									board[z][y][x] = shapecol[piece];
								}

									//check for filled planes and delete/drop them
								for(y=bsizy-1;y>=0;y--)
								{
									j = 0; for(x=bsizx-1;x>=0;x--) for(z=bsizz-1;z>=0;z--) j += (board[z][y][x] != 0);
									if (j == bsizx*bsizz)
									{
										voxie_playsound("c:/windows/media/tada.wav",-1,100,100,1.f);
										voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,0.3f);
										voxie_playsound("c:/windows/media/recycle.wav",-1,300,300,0.5f);
										voxie_playsound("c:/windows/media/recycle.wav",-1,100,100,0.7f);

										for(k=y;k<bsizy-1;k++) for(x=bsizx-1;x>=0;x--) for(z=bsizz-1;z>=0;z--) board[z][k][x] = board[z][k+1][x];
										for(x=bsizx-1;x>=0;x--) for(z=bsizz-1;z>=0;z--) board[z][bsizy-1][x] = 0;
										score++;
									}
								}

								piece = -1;
								break;
							}
						}
					}

					//if (voxie_keystat(0xb8))
					//{
						//static int gfcnt = 0; board[(gfcnt/bsizx)%bsizz][rand()%1][gfcnt%bsizx] = shapecol[0]; gfcnt++;
						//for(z=0;z<bsizz;z++) for(x=0;x<bsizx;x++) board[z][0][x] = 0xff00ff;
					//}

					if (piece >= 0)
					{
						int cx[4], cy[4], cz[4];
						for(i=0;i<4;i++)
						{
							cx[i] = shape[piece][i][0];
							cy[i] = shape[piece][i][1];
							cz[i] = shape[piece][i][2];
							packer_rot(rot,&cx[i],&cy[i],&cz[i]);
							cx[i] += piecex;
							cy[i] += piecey;
							cz[i] += piecez;
						}
						for(i=0;i<4;i++)
						{
							point3d pp, rr, dd, ff;
							rr.x = 1.0; dd.x = 0.0; ff.x = 0.0; pp.x = (float)cx[i] + (bsizx*-.5);
							rr.y = 0.0; dd.y = 1.0; ff.y = 0.0; pp.y = (float)cy[i] + (bsizy*-.5);
							rr.z = 0.0; dd.z = 0.0; ff.z = 1.0; pp.z = (float)cz[i] + (bsizz*-.5);
							if (drawonsid) { f = pp.x; pp.x = pp.y; pp.y = -f; f = rr.x; rr.x = rr.y; rr.y = -f; f = dd.x; dd.x = dd.y; dd.y = -f; f = ff.x; ff.x = ff.y; ff.y = -f; }
							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,1,0xffffff);

							f = (1.f-PIECEDRAWFRAC)*.5f;
							pp.x += (rr.x + dd.x + ff.x)*f;
							pp.y += (rr.y + dd.y + ff.y)*f;
							pp.z += (rr.z + dd.z + ff.z)*f;
							f = PIECEDRAWFRAC; rr.x *= f; rr.y *= f; rr.z *= f; dd.x *= f; dd.y *= f; dd.z *= f; ff.x *= f; ff.y *= f; ff.z *= f;

							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,2,shapecol[piece]>>2);
							drawcube_faces_vis(&vf,&pp,&rr,&dd,&ff,gpakrendmode,shapecol[piece],i,4,cx,cy,cz);

							if (!drawonsid)
							{
								voxie_drawlin(&vf,pp.x+(rr.x+dd.x+ff.x)*.5,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,
														pp.x+(rr.x+dd.x+ff.x)*.5,bsizy*-.5f              ,pp.z+(rr.z+dd.z+ff.z)*.5,shapecol[piece]);
								//voxie_drawcone(&vf,pp.x+(rr.x+dd.x+ff.x)*.5,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,.05,
								//                   pp.x+(rr.x+dd.x+ff.x)*.5,bsizy*-.5f              ,pp.z+(rr.z+dd.z+ff.z)*.5,.05,1,shapecol[piece]>>2);
								for(f=pp.y+(rr.y+dd.y+ff.y)*.5f-fmod(tim,1.0);f>=bsizy*-.5f;f-=.5f)
								{
									voxie_drawsph(&vf,pp.x+(rr.x+dd.x+ff.x)*.5,f,pp.z+(rr.z+dd.z+ff.z)*.5,.05,0,shapecol[piece]>>2);
								}
							}
							else
							{
								voxie_drawlin(&vf,pp.x+(rr.x+dd.x+ff.x)*.5,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,
														bsizy*-.5f              ,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,shapecol[piece]);
								//voxie_drawcone(&vf,pp.x+(rr.x+dd.x+ff.x)*.5,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,.05,
								//                   bsizy*-.5f              ,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,.05,1,shapecol[piece]>>2);
								for(f=pp.x+(rr.x+dd.x+ff.x)*.5f-fmod(tim,1.0);f>=bsizy*-.5f;f-=.5f)
								{
									voxie_drawsph(&vf,f,pp.y+(rr.y+dd.y+ff.y)*.5,pp.z+(rr.z+dd.z+ff.z)*.5,.05,0,shapecol[piece]>>2);
								}
							}
						}
					}
				}

				for(z=0;z<bsizz;z++)
					for(y=0;y<bsizy;y++)
						for(x=0;x<bsizx;x++)
						{
							point3d pp, rr, dd, ff;

							if (!board[z][y][x]) continue;

							rr.x = 1.0; dd.x = 0.0; ff.x = 0.0; pp.x = (float)x + (bsizx*-.5);
							rr.y = 0.0; dd.y = 1.0; ff.y = 0.0; pp.y = (float)y + (bsizy*-.5);
							rr.z = 0.0; dd.z = 0.0; ff.z = 1.0; pp.z = (float)z + (bsizz*-.5);
							if (drawonsid) { f = pp.x; pp.x = pp.y; pp.y = -f; f = rr.x; rr.x = rr.y; rr.y = -f; f = dd.x; dd.x = dd.y; dd.y = -f; f = ff.x; ff.x = ff.y; ff.y = -f; }
							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,1,0xffffff);

							f = (1.f-PIECEDRAWFRAC)*.5f;
							pp.x += (rr.x + dd.x + ff.x)*f;
							pp.y += (rr.y + dd.y + ff.y)*f;
							pp.z += (rr.z + dd.z + ff.z)*f;
							f = PIECEDRAWFRAC; rr.x *= f; rr.y *= f; rr.z *= f; dd.x *= f; dd.y *= f; dd.z *= f; ff.x *= f; ff.y *= f; ff.z *= f;

							//voxie_drawcube(&vf,&pp,&rr,&dd,&ff,2,board[z][y][x]);

							int cx[19], cy[19], cz[19], xx, yy, zz, n;
							cx[0] = 0; cy[0] = 0; cz[0] = 0; n = 1;
							for(zz=-1;zz<=1;zz++)
								for(yy=-1;yy<=1;yy++)
									for(xx=-1;xx<=1;xx++)
									{
										i = labs(xx)+labs(yy)+labs(zz); if ((i == 0) || (i == 3)) continue;
										if ((unsigned)(x+xx) >= (unsigned)bsizx) continue;
										if ((unsigned)(y+yy) >= (unsigned)bsizy) continue;
										if ((unsigned)(z+zz) >= (unsigned)bsizz) continue;
										if (board[z+zz][y+yy][x+xx]) { cx[n] = xx; cy[n] = yy; cz[n] = zz; n++; }
									}
							drawcube_faces_vis(&vf,&pp,&rr,&dd,&ff,gpakrendmode,board[z][y][x],0,n,cx,cy,cz);
						}

				{ //Show play volume
				point3d pp, rr, dd, ff;
				rr.x = bsizx; dd.x = 0; ff.x = 0; pp.x = (bsizx*-.5);
				rr.y = 0; dd.y = bsizy; ff.y = 0; pp.y = (bsizy*-.5);
				rr.z = 0; dd.z = 0; ff.z = bsizz; pp.z = (bsizz*-.5);
				if (drawonsid) { f = pp.x; pp.x = pp.y; pp.y = -f; f = rr.x; rr.x = rr.y; rr.y = -f; f = dd.x; dd.x = dd.y; dd.y = -f; f = ff.x; ff.x = ff.y; ff.y = -f; }
				voxie_drawcube(&vf,&pp,&rr,&dd,&ff,1,0xffffff);
				}

				{
				char buf[256];
				point3d pp, rr, dd, ff;
				sprintf(buf,"SCORE:%d",score); f = (float)strlen(buf)*.5;
				rr.x = 0.76; dd.x = 0.00; pp.x = 0.00      - rr.x*f - dd.x*.5;
				rr.y = 0.00; dd.y = 0.00; pp.y = bsizy*.49 - rr.y*f - dd.y*.5;
				rr.z = 0.00; dd.z = 1.60; pp.z = 0.03      - rr.z*f - dd.z*.5;
				if (drawonsid) { f = pp.x; pp.x = pp.y; pp.y = -f; f = rr.x; rr.x = rr.y; rr.y = -f; f = dd.x; dd.x = dd.y; dd.y = -f; }
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);

				ff.x = rr.y*dd.z - rr.z*dd.y;
				ff.y = rr.z*dd.x - rr.x*dd.z;
				ff.z = rr.x*dd.y - rr.y*dd.x;
				for(i=0;i<4;i++)
				{
					pp.x += ff.x*.02f;
					pp.y += ff.y*.02f;
					pp.z += ff.z*.02f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);
				}
				}

				}

				break;
			case RENDMODE_PARATROOPER: //Paratrooper 3D
				{
				typedef struct { float x, y, z, xv, yv, zv, ang; int alive; } chop_t;
				#define CHOPMAX 16
				static chop_t chop[CHOPMAX];
				static int chopn = 0;

				typedef struct { float x, y, z; int alive; } man_t;
				#define MANMAX 256
				static man_t man[MANMAX];
				static int mann = 0;

				typedef struct { float x, y, z, xv, yv, zv; } bul_t;
				#define BULMAX 64
				static bul_t bul[BULMAX];
				static int buln = 0;

				typedef struct { float x, y, z; double timdone; } skull_t;
				#define SKULLMAX 16
				static skull_t skull[SKULLMAX];
				static int skulln = 0;

				typedef struct { float x, y, z, xv, yv, zv; int col; float dtim; } part_t;
				#define PARTMAX 4096
				static part_t part[PARTMAX];
				static int partn = 0;

				static int ptroop_inited = 0, gameover = 0, usenav = 0;
				static double timlastshot = 0.0;
				static float aimx = 0.f, aimy = 0.f;
				point3d tp, tp2;
				int closemen, choponcanon;

				if ((voxie_keystat(0x35) == 1) || (ghitkey == 0x0e) || (voxie_keystat(0x0e) == 1) || ((vx[0].but&~ovxbut[0])&(1<<4))) { ghitkey = 0; ptroop_inited = 0; } //'/' or Backspace:Reset game
				if (!ptroop_inited)
				{
					voxie_playsound_update(-1,-1,0,0,1.f);/*kill all sound*/
					//voxie_playsound("?.flac",-1,40,40,1.f);
					ptroop_inited = 1;
					chopn = 0;
					mann = 0;
					buln = 0;
					skulln = 0;
					partn = 0;
					gameover = 0;
				}

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				 //--------------------------------------------
					//Process cannon..
				aimx += in.dmousx*.003f;
				aimy += in.dmousy*.003f;

				if (nav[0].ax || nav[0].ay) { usenav = 1; }
				if (usenav)
				{
					//aimx = nav[0].ax*.01f;
					//aimy = nav[0].ay*.01f;
					aimx += nav[0].ax*.02f*dtim;
					aimy += nav[0].ay*.02f*dtim;
					//aimx += nav[0].dx*.01f;
					//aimy += nav[0].dy*.01f;
				}
				f = sqrt(aimx*aimx + aimy*aimy);
				if (f > 2.0) { f = 2.0/f; aimx *= f; aimy *= f; }

				point3d pp, rr, dd, ff;
				rr.x = .10f; rr.y = 0.f; rr.z = 0.f;
				dd.x = 0.f; dd.y = .10f; dd.z = 0.f;
				ff.x = 0.f; ff.y = 0.f; ff.z = .04f;
				pp.x = 0.f - (rr.x + dd.x + ff.x)*.5f;
				pp.y = 0.f - (rr.y + dd.y + ff.y)*.5f;
				pp.z = vw.aspz - (rr.z + dd.z + ff.z)*1.f;
				voxie_drawcube(&vf,&pp,&rr,&dd,&ff,2,0xffffff);

				if (!gameover)
				{
						//canon aim = tp + tp2*t
					tp.x = 0.f;          tp2.x = aimx    -tp.x;
					tp.y = 0.f;          tp2.y = aimy    -tp.y;
					tp.z = vw.aspz-.04f; tp2.z = -vw.aspz-tp.z;
					f = 1.0/sqrt(tp2.x*tp2.x + tp2.y*tp2.y + tp2.z*tp2.z); tp2.x *= f; tp2.y *= f; tp2.z *= f;

					voxie_drawsph(&vf,tp.x,tp.y,tp.z,.03f,1,0xff00ff);
					voxie_drawcone(&vf,tp.x,tp.y,tp.z,.014f,tp.x+tp2.x*.1f,tp.y+tp2.y*.1f,tp.z+tp2.z*.1f,.008f,0,0x00ffff);

					//f = (-vw.aspz - tp.z)/tp2.z;
					//g = ((float)((tp2.x>0.f)*2-1)*vw.aspx - tp.x)/tp2.x; if (g < f) f = g;
					//g = ((float)((tp2.y>0.f)*2-1)*vw.aspy - tp.y)/tp2.y; if (g < f) f = g;
					//voxie_drawsph(&vf,tp.x + tp2.x*f,tp.y + tp2.y*f,tp.z + tp2.z*f,.01f,0,0x010101);
					voxie_drawsph(&vf,aimx,aimy,-vw.aspz,.02f,1,0x010101);

					voxie_drawlin(&vf,tp.x+tp2.x*.1f, tp.y+tp2.y*.1f, tp.z+tp2.z*.1f,
											tp.x+tp2.x    , tp.y+tp2.y    , tp.z+tp2.z    ,0x404040);

					if (((in.bstat&1) || (nav[0].but)) && (fabs(tim-timlastshot) > 0.25) && (buln < BULMAX-1))
					{
						timlastshot = tim;
						bul[buln].x = tp.x;
						bul[buln].y = tp.y;
						bul[buln].z = tp.z;
						bul[buln].xv = tp2.x;
						bul[buln].yv = tp2.y;
						bul[buln].zv = tp2.z;
						buln++;
						voxie_playsound("plop.flac",-1,40,40,1.5f);
						voxie_playsound("blowup2.flac",-1,20,20,12.f);
					}
				}

				//--------------------------------------------
					//Process choppers..
				choponcanon = 0;
				if ((chopn < 2) && (chopn < CHOPMAX-1) && (!gameover))
				{
					chop[chopn].x = (float)(rand()&32767)/32768.f-.5f;
					chop[chopn].y = (float)(rand()&32767)/32768.f-.5f;
					chop[chopn].z = -vw.aspz+.05f;
					chop[chopn].ang = (float)(rand()&32767)/32768.f*PI*2.f;
					chop[chopn].xv = cos(chop[chopn].ang)*.25f;
					chop[chopn].yv = sin(chop[chopn].ang)*.25f;
					chop[chopn].zv = 0.f;
					chop[chopn].alive = 1;
					chop[chopn].x -= cos(chop[chopn].ang)*1.5f;
					chop[chopn].y -= sin(chop[chopn].ang)*1.5f;
					chopn++;
				}

				for(i=chopn-1;i>=0;i--)
				{
					drawchopper(chop[i].x,chop[i].y,chop[i].z,.5f,chop[i].ang+PI*.5,tim);
					chop[i].x += chop[i].xv*dtim;
					chop[i].y += chop[i].yv*dtim;
					chop[i].z += chop[i].zv*dtim;

					if (!chop[i].alive)
					{
						chop[i].ang += dtim*4.f;
						chop[i].zv += dtim*.5f;
						if (chop[i].z >= vw.aspz)
						{
							tp.x = chop[i].x;
							tp.y = chop[i].y;
							tp.z = chop[i].z;
							for(j=mann-1;j>=0;j--)
							{
								if ((man[j].x-tp.x)*(man[j].x-tp.x) +
									 (man[j].y-tp.y)*(man[j].y-tp.y) +
									 (man[j].z-tp.z)*(man[j].z-tp.z) >= .25f*.25f) continue;
								mann--; man[j] = man[mann];
							}

							if (tp.x*tp.x + tp.y*tp.y < .1f*.1f) choponcanon = 1;

							for(j=1024;j>0;j--)
							{
								if (partn >= PARTMAX-1) break;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].x = chop[i].x + fx*.1f;
								part[partn].y = chop[i].y + fy*.1f;
								part[partn].z = chop[i].z + fz*.1f;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].xv = fx*2.f;
								part[partn].yv = fy*2.f;
								part[partn].zv = fz*2.f;
								if (rand()&3) part[partn].col = 0xffffff;
											else part[partn].col = 0xff0000;
								part[partn].dtim = (rand()/32768.0)*1.f + 1.f;
								partn++;
							}

							chopn--; chop[i] = chop[chopn];
							voxie_playsound("blowup2.flac",-1,20,20,1.f);
							continue;
						}
					}

					if ((!(rand()&255)) && ((fabs(chop[i].x) > 0.10) || (fabs(chop[i].y) > 0.10)) && (fabs(chop[i].x) < vw.aspx) && (fabs(chop[i].y) < vw.aspy) && (mann < MANMAX-1))
					{
						man[mann].x = chop[i].x;
						man[mann].y = chop[i].y;
						man[mann].z = -.25f;
						man[mann].alive = 1;
						mann++;
					}

					if ((fabs(chop[i].x) >= vw.aspx+.2f) || (fabs(chop[i].y) >= vw.aspy+.2f)) //chopper off volume
					{
						chopn--; chop[i] = chop[chopn]; continue;
					}
				}

				//--------------------------------------------
					//Process mans..

				closemen = 0;
				for(i=mann-1;i>=0;i--)
				{
					drawman(man[i].x,man[i].y,man[i].z,1.f,atan2(man[i].x,-man[i].y)+cos(tim*6.f)*.25f,(man[i].z >= -.125f) && (man[i].alive) && (man[i].z < vw.aspz-.042f));
					if ((man[i].z < -.125f) || (!man[i].alive)) man[i].z = min(man[i].z+dtim*.40f,vw.aspz-.042f);
					else                                        man[i].z = min(man[i].z+dtim*.05f,vw.aspz-.042f);
					if (man[i].z >= vw.aspz-.042f)
					{
						if (!man[i].alive)
						{
							tp.x = man[i].x;
							tp.y = man[i].y;
							tp.z = man[i].z;

							if (skulln < SKULLMAX-1)
							{
								skull[skulln].x = man[i].x;
								skull[skulln].y = man[i].y;
								skull[skulln].z = man[i].z;
								skull[skulln].timdone = tim+1.0;
								skulln++;
							}
							mann--; man[i] = man[mann];
							for(j=mann-1;j>=0;j--)
							{
								if (i == j) continue;
								if ((man[j].x-tp.x)*(man[j].x-tp.x) +
									 (man[j].y-tp.y)*(man[j].y-tp.y) +
									 (man[j].z-tp.z)*(man[j].z-tp.z) >= .1f*.1f) continue;

								if (skulln < SKULLMAX-1)
								{
									skull[skulln].x = man[j].x;
									skull[skulln].y = man[j].y;
									skull[skulln].z = man[j].z;
									skull[skulln].timdone = tim+1.0;
									skulln++;
								}
								mann--; man[j] = man[mann];
							}
							continue;
						}

						f = man[i].x*man[i].x + man[i].y*man[i].y;
						if (fabs(f) > .01) f = dtim*.02f/sqrt(f); else f = 0.f;
						man[i].x -= man[i].x*f;
						man[i].y -= man[i].y*f;
						f = .06f;
						if ((fabs(man[i].x) < f) && (fabs(man[i].y) < f))
						{
							if (fabs(man[i].x) > fabs(man[i].y)) { man[i].x = (float)((man[i].x > 0.f)*2-1)*f; }
																	  else { man[i].y = (float)((man[i].y > 0.f)*2-1)*f; }
							closemen++;
						}
					}
				}

				if (((closemen >= 4) || (choponcanon)) && (!gameover))
				{
					for(j=4096;j>0;j--)
					{
						if (partn >= PARTMAX-1) break;
						do
						{
							fx = ((rand()/32768.f)-.5f)*2.f;
							fy = ((rand()/32768.f)-.5f)*2.f;
							fz = ((rand()/32768.f)-.5f)*2.f;
						} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
						part[partn].x = fx*.1f;
						part[partn].y = fy*.1f;
						part[partn].z = fz*.1f + vw.aspz;
						do
						{
							fx = ((rand()/32768.f)-.5f)*2.f;
							fy = ((rand()/32768.f)-.5f)*2.f;
							fz = ((rand()/32768.f)-.5f)*2.f;
						} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
						part[partn].xv = fx*1.f;
						part[partn].yv = fy*1.f;
						part[partn].zv = fz*1.f;
						if (rand()&3) part[partn].col = 0xffffff;
									else part[partn].col = 0x00ffff;
						part[partn].dtim = (rand()/32768.0)*1.f + 4.f;
						partn++;
					}
					voxie_playsound("blowup2.flac",-1,20,20,0.25f);
					gameover = 1;
				}

				//--------------------------------------------
					//Process buls..

				for(i=buln-1;i>=0;i--)
				{
					for(j=mann-1;j>=0;j--)
					{
						if (!man[j].alive) continue;
						tp.x = man[j].x-bul[i].x;
						tp.y = man[j].y-bul[i].y;
						tp.z = man[j].z-bul[i].z;
						if ((tp.x*tp.x + tp.y*tp.y < .1f*.1f) && (man[j].alive) && (man[j].z < vw.aspz-.042f) && (fabs(tp.z) < .2f))
						{
							for(f=1.f;f<1.5f;f+=.125f)
								voxie_playsound("plop.flac",-1,40,40,f);
							man[j].alive = 0;
							buln--; bul[i] = bul[buln]; goto contbul;
						}
					}
					for(j=chopn-1;j>=0;j--)
					{
						if (!chop[j].alive) continue;
						tp.x = chop[j].x-bul[i].x;
						tp.y = chop[j].y-bul[i].y;
						tp.z = chop[j].z-bul[i].z;
						if (tp.x*tp.x + tp.y*tp.y + tp.z*tp.z < .15f*.15f)
						{
							for(f=0.5f;f<1.5f;f+=.125f)
								voxie_playsound("plop.flac",-1,40,40,f);
							voxie_playsound("blowup2.flac",-1,20,20,3.f);

							for(k=256;k>0;k--)
							{
								if (partn >= PARTMAX-1) break;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].x = chop[i].x + fx*.1f;
								part[partn].y = chop[i].y + fy*.1f;
								part[partn].z = chop[i].z + fz*.1f;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].xv = fx*.5f + bul[i].xv;
								part[partn].yv = fy*.5f + bul[i].yv;
								part[partn].zv = fz*.5f;
								if (rand()&3) part[partn].col = 0xffffff;
											else part[partn].col = 0xff0000;
								part[partn].dtim = (rand()/32768.0)*1.f + 1.f;
								partn++;
							}
							chop[j].xv += bul[i].xv*.25f;
							chop[j].yv += bul[i].yv*.25f;

							chop[j].alive = 0;
							buln--; bul[i] = bul[buln]; goto contbul;
						}
					}

					voxie_drawsph(&vf,bul[i].x,bul[i].y,bul[i].z,.01f,1,0xffffff);
					bul[i].x += bul[i].xv*dtim;
					bul[i].y += bul[i].yv*dtim;
					bul[i].z += bul[i].zv*dtim;
					if ((fabs(bul[i].x) >= vw.aspx) || (fabs(bul[i].y) >= vw.aspy) || (fabs(bul[i].z) >= vw.aspz))
					{
						buln--; bul[i] = bul[buln]; goto contbul;
					}
contbul:;   }

				//--------------------------------------------
					//Process skulls..

				for(i=skulln-1;i>=0;i--)
				{
					rr.x = .10f; rr.y = 0.f; rr.z = 0.f;
					dd.x = 0.f; dd.y = .10f; dd.z = 0.f;
					ff.x = 0.f; ff.y = 0.f; ff.z = .10f;
					pp.x = skull[i].x;
					pp.y = skull[i].y;
					pp.z = skull[i].z;
					voxie_drawspr(&vf,"skullbones.kv6",&pp,&rr,&dd,&ff,0xffffff);

					if (tim >= skull[i].timdone) { skulln--; skull[i] = skull[skulln]; continue; }
				}

				//--------------------------------------------
					//Process particles..

				for(i=partn-1;i>=0;i--)
				{
					part[i].x += part[i].xv*dtim;
					part[i].y += part[i].yv*dtim;
					part[i].z += part[i].zv*dtim;
					part[i].xv *= pow(.125,dtim);
					part[i].yv *= pow(.125,dtim);
					part[i].zv *= pow(.125,dtim);
					part[i].zv += dtim*.5f;
					if (part[i].z > vw.aspz) { part[i].zv = -fabs(part[i].zv); }

					voxie_drawvox(&vf,part[i].x,part[i].y,part[i].z,part[i].col);
					part[i].dtim -= dtim;
					if (part[i].dtim < 0.f) { partn--; part[i] = part[partn]; continue; }
				}

					//draw wireframe box
				if (!vw.clipshape)
				{
					voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,+vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
				}
				else
				{
					n = 64;
					for(j=63;j<=64;j++)
						for(i=0;i<n;i++)
						{
							voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f,
													cos((float)(i+1)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+1)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f, 0xffffff);
						}
				}

				if (gameover)
				{
					char buf[256];
					point3d pp, rr, dd;
					pp.x =  0.00f; rr.x = 0.16; dd.x = 0.00;
					pp.y = -0.12f; rr.y = 0.00; dd.y = 0.24;
					pp.z = -vw.aspz+0.01f; rr.z = 0.00; dd.z = 0.00;
					strcpy(buf,"Game over");
					f = (float)strlen(buf)*.5; pp.x -= rr.x*f; pp.y -= rr.y*f; pp.z -= rr.z*f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);
				}


				}
				break;
#if 0
			case RENDMODE_?: //vox animation (dancing man)
				{
				typedef struct { short f, x, y, z; } dot_t;
				static dot_t *dot;
				static int dotn = 0, dotmal = 4194304;
				static int *frame;
				static int framen = 0, framemal = 1024;

				if (dotn == -1)
				{
					point3d pp, rr, dd;
					pp.x = 0.0; rr.x = 20.0; dd.x =   0.0;
					pp.y = 0.0; rr.y =  0.0; dd.y =  60.0;
					pp.z = 0.0; rr.z =  0.0; dd.z =   0.0;
					voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"File not found");
					break;
				}
				if (!dotn)
				{
					char *filbuf;
					int fileng, filcnt, ofilcnt, sign;

					if (!kzopen("positions.vox")) { dotn = -1; break; }
					fileng = kzfilelength();
					filbuf = (char *)malloc(fileng+1); if (!filbuf) { kzclose(); dotn = -1; break; }
					kzread(filbuf,fileng);
					kzclose();

					dot = (dot_t *)malloc(dotmal*sizeof(dot_t)); if (!dot) { free(filbuf); dotn = -1; break; }
					frame = (int *)malloc(framemal*sizeof(int)); if (!frame) { free(dot); free(filbuf); dotn = -1; break; }

					filcnt = 0;
					for(dotn=0;dotn<dotmal;dotn++)
					{
							//grab next line filbuf[ofilcnt].., filbuf[filcnt] is null terminator
						ofilcnt = filcnt; while (filbuf[filcnt] != 13) { filcnt++; if (filcnt >= fileng) break; }
						filbuf[filcnt] = 0; filcnt++; if (filcnt >= fileng) break;

							//sscanf(buf,"%d %d %d %d",&dot[dotn].f,&dot[dotn].x,&dot[dotn].y,&dot[dotn].z);
						j = 0; sign = 1; k = 0;
						for(i=ofilcnt;filbuf[i];i++)
						{
							if (filbuf[i] == ' ') { ((short *)(&dot[dotn].f))[k] = j*sign; j = 0; sign = 1; k++; continue; }
							if (filbuf[i] == '-') { sign = -sign; continue; }
							j = j*10+(filbuf[i]-'0');
						}
						((short *)(&dot[dotn].f))[k] = j*sign;

						if ((!dotn) || (dot[dotn].f > dot[dotn-1].f)) { frame[framen] = dotn; framen++; }
					}
					frame[framen] = dotn; free(filbuf);
				}

				voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords

				j = ((int)(tim*20)%framen);
				for(i=frame[j];i<frame[j+1];i++)
				{
					voxie_drawvox(&vf,(int)(dot[i].x*7.0/12.0),(int)(dot[i].y*7.0/6.0),255-(int)(dot[i].z*7.0),0xffffff);
				}

				{
				point3d pp, rr, dd;
				pp.x = 0.0; rr.x = 20.0; dd.x =   0.0;
				pp.y = 0.0; rr.y =  0.0; dd.y =  60.0;
				pp.z = 0.0; rr.z =  0.0; dd.z =   0.0;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%ddot %dfr",dotn,framen);
				}

				}
				break;
#endif
#if 0
			case RENDMODE_?: //Simple line drawing program.
				{
				#define FIFSIZ 65536
				typedef struct { point3d p; char col, islin, filler[2]; } fif_t;
				static fif_t fif[FIFSIZ];
				static int fifw = 0, islin = 0, curcol = 0xffffff;
				static point3d pos = {0.0,0.0,0.0}, opos = {0.0,0.0,0.0};
				float fscale;

				fscale = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
				voxie_setview(&vf,-vw.aspx*fscale,-vw.aspy*fscale,-vw.aspz*fscale,+vw.aspx*fscale,+vw.aspy*fscale,+vw.aspz*fscale);

				if ((voxie_keystat(0x35)||(voxie_keystat(0x0e)) || (vx[0].but&(1<<4))) { fifw = 0; } //'/' or Backspace:Clear rod list
				for(i=1;i<=7;i++) if (voxie_keystat(i+0x3a)) curcol = (i&1)*0xff + ((i>>1)&1)*0xff00 + ((i>>2)&1)*0xff0000; //F1-F7
				opos = pos;
				if (voxie_keystat(0xcb)) { pos.x -= dtim*.5f; }
				if (voxie_keystat(0xcd)) { pos.x += dtim*.5f; }
				if (voxie_keystat(0xc8)) { pos.y -= dtim*.5f; }
				if (voxie_keystat(0xd0)) { pos.y += dtim*.5f; }
				if (voxie_keystat(0x9d)) { pos.z -= dtim*.5f; }
				if (voxie_keystat(0x52)|voxie_keystat(0xdd)) { pos.z += dtim*.5f; }
				pos.x = min(max(pos.x+in.dmousx*.0050f,-vw.aspx*fscale),+vw.aspx*fscale);
				pos.y = min(max(pos.y+in.dmousy*.0050f,-vw.aspy*fscale),+vw.aspy*fscale);
				pos.z = min(max(pos.z-in.dmousz*.0005f,-vw.aspz*fscale),+vw.aspz*fscale);

				if (fabs(opos.x-pos.x) + fabs(opos.y-pos.y) + fabs(opos.z-pos.z) > 0.f)
				{
					if ((bstatus&1) || (voxie_keystat(0x9c))) { i = fifw&(FIFSIZ-1); fif[i].p = pos; fif[i].islin = islin; fif[i].col = curcol; fifw++; islin = 1; }
																	else { islin = 0; }
				}

				voxie_drawsph(&vf,pos.x,pos.y,pos.z,fabs(sin(tim*16.f))*.015f,0,curcol);

					//Draw rod fifo
				for(i=max(fifw-FIFSIZ,0)+1;i<fifw;i++)
				{
					j = ((i-1)&(FIFSIZ-1));
					k = ((i  )&(FIFSIZ-1));
					if (fif[k].islin) { voxie_drawlin(&vf,fif[j].p.x,fif[j].p.y,fif[j].p.z, fif[k].p.x,fif[k].p.y,fif[k].p.z,fif[k].col); }
					//voxie_drawsph(&vf,fif[k].p.x,fif[k].p.y,fif[k].p.z,.015f,0,fif[k].col);
				}
				}
				break;
#endif
#if 1
			case RENDMODE_DOTMUNCH: //Dot Munch
				{
				static unsigned char *got = 0;
				#define GHOSTFRAMES 2
				#define MUNCHFRAMES 9
				static const char *munnam[] =
				{
					"ghost_0.kv6","ghost_1.kv6",
					"munman_0.kv6","munman_1.kv6","munman_2.kv6","munman_3.kv6","munman_4.kv6","munman_5.kv6","munman_6.kv6","munman_7.kv6","munman_8.kv6"
				};
				static point3d smun, mun, nmun, vel, nvel, viewcent;
				#define MONMAX 16
				static point3d smpos[MONMAX], mpos[MONMAX], mgoal[MONMAX], mvel[MONMAX];
				static int nmon = 0;
				static int inited = -1, numdots = 0, mcol[4] = {0x200808,0x20181c,0x08201c,0x201808}; //mcol[4] = {0x802020,0x806070,0x208070,0x806020};
				static double timpower = -1e32, timdeath = 0.0, timnextlev = -1e32;
				static float fsc = .33f, fscgoal = .33f;
				float fx, fy, fz, fscale;
				int fakebut;
				char ch;

				if (labs(vx[0].tx0) + labs(vx[0].ty0) > 16384)
				{
					if (labs(vx[0].tx0) > labs(vx[0].ty0)) { if (vx[0].tx0 < 0) vx[0].but |= (1<<2); else vx[0].but |= (1<<3); }
																 else { if (vx[0].ty0 > 0) vx[0].but |= (1<<0); else vx[0].but |= (1<<1); }
				}
				if (labs(vx[0].tx1) + labs(vx[0].ty1) > 16384)
				{
					if (labs(vx[0].tx1) > labs(vx[0].ty1)) { if (vx[0].tx1 < 0) vx[0].but |= (1<<2); else vx[0].but |= (1<<3); }
																 else { if (vx[0].ty1 > 0) vx[0].but |= (1<<0); else vx[0].but |= (1<<1); }
				}
				if (vx[0].lt > 0) vx[0].but |= (1<<6);
				if (vx[0].rt > 0) vx[0].but |= (1<<7);

				fakebut = 0;
				if (cmun->zsiz == 1)
				{
					if (fabs(nav[0].ax) + fabs(nav[0].ay) > 100)
					{
						if (fabs(nav[0].ax) > fabs(nav[0].ay)) { if (nav[0].ax < 0.f) fakebut |= (1<<2); else fakebut |= (1<<3); }
																	 else { if (nav[0].ay < 0.f) fakebut |= (1<<0); else fakebut |= (1<<1); }
					}
				}
				else
				{
					if (fabs(nav[0].dx) > max(fabs(nav[0].dy),fabs(nav[0].dz)))
						{ if (fabs(nav[0].dx) > 100) { if (nav[0].dx < 0.f) fakebut |= (1<<2); else fakebut |= (1<<3); } }
					else if (fabs(nav[0].dy) > fabs(nav[0].dz))
						{ if (fabs(nav[0].dy) > 100) { if (nav[0].dy < 0.f) fakebut |= (1<<0); else fakebut |= (1<<1); } }
					else
						{ if (fabs(nav[0].dz) > 100) { if (nav[0].dz < 0.f) fakebut |= (1<<4); else fakebut |= (1<<5); } }
				}

				if (voxie_keystat(keyremap[0][6]) == 1)                            { inited = 0; munlev = 0;                    cmun = &gmun[munlev]; break; } //ButM:reset&title
				if ((voxie_keystat(0x0e) == 1) || (ghitkey == 0x0e) || ((vx[0].but&~ovxbut[0])&(1<<4))) { ghitkey = 0; inited = 0; munlev = 0; cmun = &gmun[munlev];        } //Backspace:reset
				if ((otim < timnextlev) && (tim >= timnextlev))                    { inited = 0; munlev = (munlev+1)%munn;      cmun = &gmun[munlev];        }
				if ((voxie_keystat(0xc9) == 1) || (ghitkey == 0xc9))  { ghitkey = 0; inited = 0; munlev = (munlev+munn-1)%munn; cmun = &gmun[munlev];        } //PGUP
				if ((voxie_keystat(0xd1) == 1) || (ghitkey == 0xd1))  { ghitkey = 0; inited = 0; munlev = (munlev+1)%munn;      cmun = &gmun[munlev];        } //PGDN

				if (inited <= 0)
				{
					if (inited < 0) { voxie_mountzip("mun_data.zip"); }

					inited = 1; timdeath = dtim*.5f; fsc = .15f; fscgoal = .33f;

					i = min(max(cmun->xsiz*cmun->ysiz*cmun->zsiz,1),1048576);
					got = (unsigned char *)realloc(got,i);

					numdots = 0; nmon = 0;
					for(z=0;z<cmun->zsiz;z++)
						for(y=0;y<cmun->ysiz;y++)
							for(x=0;x<cmun->xsiz;x++)
							{
								i = (z*cmun->ysiz+y)*cmun->xsiz+x; ch = cmun->board[i];
								if (ch == 'S') { smun.x = (float)x; smun.y = (float)y; smun.z = (float)z; }
								if (ch == 'M') { if (nmon < MONMAX) { smpos[nmon].x = (float)x; smpos[nmon].y = (float)y; smpos[nmon].z = (float)z; nmon++; } }
								if (ch != '.') { got[i] = 1; numdots++; }
							}
				}

				if (timdeath > 0.0)
				{
					timdeath -= dtim;
					if (timdeath <= 0.0)
					{
						fscgoal = .33f; mun = smun; nmun = mun;
						vel.x = -1.f; vel.y = 0.f; vel.z = 0.f;
						nvel.x = 0.f; nvel.y = 0.f; nvel.z = 0.f;
						for(i=0;i<nmon;i++) { mpos[i] = smpos[i]; mgoal[i] = mpos[i]; mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z = 0.f; }
					}
				}

				if (fscgoal < fsc) fsc = max(fsc - dtim*.2f,fscgoal);
				if (fscgoal > fsc) fsc = min(fsc + dtim*.2f,fscgoal);

					  if (voxie_keystat(keyremap[0][0]) || (voxie_keystat(keyremap[2][0]) == 1) || (vx[0].but&(1<<2)) || (fakebut&(1<<2))) { if (nmun.x > mun.x) { nmun.x -= 1.f; vel.x = -1.f; } nvel.x =-1.f; nvel.y = 0.f; nvel.z = 0.f; }
				else if (voxie_keystat(keyremap[0][1]) || (voxie_keystat(keyremap[2][1]) == 1) || (vx[0].but&(1<<3)) || (fakebut&(1<<3))) { if (nmun.x < mun.x) { nmun.x += 1.f; vel.x = +1.f; } nvel.x =+1.f; nvel.y = 0.f; nvel.z = 0.f; }
				else if (voxie_keystat(keyremap[0][2]) || (voxie_keystat(keyremap[2][2]) == 1) || (vx[0].but&(1<<0)) || (fakebut&(1<<0))) { if (nmun.y > mun.y) { nmun.y -= 1.f; vel.y = -1.f; } nvel.x = 0.f; nvel.y =-1.f; nvel.z = 0.f; }
				else if (voxie_keystat(keyremap[0][3]) || (voxie_keystat(keyremap[2][3]) == 1) || (vx[0].but&(1<<1)) || (fakebut&(1<<1))) { if (nmun.y < mun.y) { nmun.y += 1.f; vel.y = +1.f; } nvel.x = 0.f; nvel.y =+1.f; nvel.z = 0.f; }
				else if (voxie_keystat(keyremap[0][5]) || (voxie_keystat(keyremap[2][5]) == 1) || (vx[0].but&(1<<6)) || (fakebut&(1<<4))) { if (nmun.z > mun.z) { nmun.z -= 1.f; vel.z = -1.f; } nvel.x = 0.f; nvel.y = 0.f; nvel.z =-1.f; }
				else if (voxie_keystat(keyremap[0][4]) || (voxie_keystat(keyremap[2][4]) == 1) || (vx[0].but&(1<<7)) || (fakebut&(1<<5))) { if (nmun.z < mun.z) { nmun.z += 1.f; vel.z = +1.f; } nvel.x = 0.f; nvel.y = 0.f; nvel.z =+1.f; }
				if ((nmun.x == mun.x) && (nmun.y == mun.y) && (nmun.z == mun.z))
				{
					x = (int)mun.x; y = (int)mun.y; z = (int)mun.z;
					if ((nvel.x < 0.f) && (mungetboard(cmun,x-1,y,z,'.') != '.')) { nmun.x = mun.x-1.f; vel.x =-1.f; vel.y = 0.f; vel.z = 0.f; nvel.x = 0.f; }
					if ((nvel.x > 0.f) && (mungetboard(cmun,x+1,y,z,'.') != '.')) { nmun.x = mun.x+1.f; vel.x =+1.f; vel.y = 0.f; vel.z = 0.f; nvel.x = 0.f; }
					if ((nvel.y < 0.f) && (mungetboard(cmun,x,y-1,z,'.') != '.')) { nmun.y = mun.y-1.f; vel.x = 0.f; vel.y =-1.f; vel.z = 0.f; nvel.y = 0.f; }
					if ((nvel.y > 0.f) && (mungetboard(cmun,x,y+1,z,'.') != '.')) { nmun.y = mun.y+1.f; vel.x = 0.f; vel.y =+1.f; vel.z = 0.f; nvel.y = 0.f; }
					if ((nvel.z < 0.f) && (mungetboard(cmun,x,y,z-1,'.') != '.')) { nmun.z = mun.z-1.f; vel.x = 0.f; vel.y = 0.f; vel.z =-1.f; nvel.z = 0.f; }
					if ((nvel.z > 0.f) && (mungetboard(cmun,x,y,z+1,'.') != '.')) { nmun.z = mun.z+1.f; vel.x = 0.f; vel.y = 0.f; vel.z =+1.f; nvel.z = 0.f; }
				}
				if ((nmun.x == mun.x) && (nmun.y == mun.y) && (nmun.z == mun.z))
				{
					x = (int)mun.x; y = (int)mun.y; z = (int)mun.z;
					if (( vel.x < 0.f) && (mungetboard(cmun,x-1,y,z,'.') != '.')) { nmun.x = mun.x-1.f; vel.x =-1.f; vel.y = 0.f; vel.z = 0.f; }
					if (( vel.x > 0.f) && (mungetboard(cmun,x+1,y,z,'.') != '.')) { nmun.x = mun.x+1.f; vel.x =+1.f; vel.y = 0.f; vel.z = 0.f; }
					if (( vel.y < 0.f) && (mungetboard(cmun,x,y-1,z,'.') != '.')) { nmun.y = mun.y-1.f; vel.x = 0.f; vel.y =-1.f; vel.z = 0.f; }
					if (( vel.y > 0.f) && (mungetboard(cmun,x,y+1,z,'.') != '.')) { nmun.y = mun.y+1.f; vel.x = 0.f; vel.y =+1.f; vel.z = 0.f; }
					if (( vel.z < 0.f) && (mungetboard(cmun,x,y,z-1,'.') != '.')) { nmun.z = mun.z-1.f; vel.x = 0.f; vel.y = 0.f; vel.z =-1.f; }
					if (( vel.z > 0.f) && (mungetboard(cmun,x,y,z+1,'.') != '.')) { nmun.z = mun.z+1.f; vel.x = 0.f; vel.y = 0.f; vel.z =+1.f; }
				}
				if ((timdeath <= 0.0) && (tim >= timnextlev))
				{
					if (nmun.x < mun.x) { mun.x -= dtim*4.f; if (mun.x <= nmun.x) { mun.x = nmun.x; if ((cmun->xwrap) && (mun.x <        0.f)) { mun.x += cmun->xsiz; nmun = mun; } } }
					if (nmun.x > mun.x) { mun.x += dtim*4.f; if (mun.x >= nmun.x) { mun.x = nmun.x; if ((cmun->xwrap) && (mun.x >=cmun->xsiz)) { mun.x -= cmun->xsiz; nmun = mun; } } }
					if (nmun.y < mun.y) { mun.y -= dtim*4.f; if (mun.y <= nmun.y) { mun.y = nmun.y; if ((cmun->ywrap) && (mun.y <        0.f)) { mun.y += cmun->ysiz; nmun = mun; } } }
					if (nmun.y > mun.y) { mun.y += dtim*4.f; if (mun.y >= nmun.y) { mun.y = nmun.y; if ((cmun->ywrap) && (mun.y >=cmun->ysiz)) { mun.y -= cmun->ysiz; nmun = mun; } } }
					if (nmun.z < mun.z) { mun.z -= dtim*4.f; if (mun.z <= nmun.z) { mun.z = nmun.z; if ((cmun->zwrap) && (mun.z <        0.f)) { mun.z += cmun->zsiz; nmun = mun; } } }
					if (nmun.z > mun.z) { mun.z += dtim*4.f; if (mun.z >= nmun.z) { mun.z = nmun.z; if ((cmun->zwrap) && (mun.z >=cmun->zsiz)) { mun.z -= cmun->zsiz; nmun = mun; } } }
				}

				for(i=nmon-1;i>=0;i--)
				{
					if ((mgoal[i].x == mpos[i].x) && (mgoal[i].y == mpos[i].y) && (mgoal[i].z == mpos[i].z))
					{
						x = (int)(mpos[i].x); y = (int)(mpos[i].y); z = (int)(mpos[i].z);
						if ((mvel[i].x < 0.f) && (mungetboard(cmun,x-1,y,z,'.') != '.')) { mgoal[i].x = mpos[i].x-1.f; mvel[i].x =-1.f; mvel[i].y = 0.f; mvel[i].z = 0.f; }
						if ((mvel[i].x > 0.f) && (mungetboard(cmun,x+1,y,z,'.') != '.')) { mgoal[i].x = mpos[i].x+1.f; mvel[i].x =+1.f; mvel[i].y = 0.f; mvel[i].z = 0.f; }
						if ((mvel[i].y < 0.f) && (mungetboard(cmun,x,y-1,z,'.') != '.')) { mgoal[i].y = mpos[i].y-1.f; mvel[i].x = 0.f; mvel[i].y =-1.f; mvel[i].z = 0.f; }
						if ((mvel[i].y > 0.f) && (mungetboard(cmun,x,y+1,z,'.') != '.')) { mgoal[i].y = mpos[i].y+1.f; mvel[i].x = 0.f; mvel[i].y =+1.f; mvel[i].z = 0.f; }
						if ((mvel[i].z < 0.f) && (mungetboard(cmun,x,y,z-1,'.') != '.')) { mgoal[i].z = mpos[i].z-1.f; mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z =-1.f; }
						if ((mvel[i].z > 0.f) && (mungetboard(cmun,x,y,z+1,'.') != '.')) { mgoal[i].z = mpos[i].z+1.f; mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z =+1.f; }
					}
					if ((mgoal[i].x == mpos[i].x) && (mgoal[i].y == mpos[i].y) && (mgoal[i].z == mpos[i].z))
					{
						x = (int)(mpos[i].x); y = (int)(mpos[i].y); z = (int)(mpos[i].z);
						j = (z*cmun->ysiz + y)*cmun->xsiz + x;
						switch(rand()%6)
						{
							case 0: if ((mpos[i].x >            0) && (mungetboard(cmun,x-1,y,z,'.') != '.')) { mgoal[i].x = mpos[i].x-1.f; mvel[i].x =-1.f; mvel[i].y = 0.f; mvel[i].z = 0.f; } break;
							case 1: if ((mpos[i].x < cmun->xsiz-1) && (mungetboard(cmun,x+1,y,z,'.') != '.')) { mgoal[i].x = mpos[i].x+1.f; mvel[i].x =+1.f; mvel[i].y = 0.f; mvel[i].z = 0.f; } break;
							case 2: if ((mpos[i].y >            0) && (mungetboard(cmun,x,y-1,z,'.') != '.')) { mgoal[i].y = mpos[i].y-1.f; mvel[i].x = 0.f; mvel[i].y =-1.f; mvel[i].z = 0.f; } break;
							case 3: if ((mpos[i].y < cmun->ysiz-1) && (mungetboard(cmun,x,y+1,z,'.') != '.')) { mgoal[i].y = mpos[i].y+1.f; mvel[i].x = 0.f; mvel[i].y =+1.f; mvel[i].z = 0.f; } break;
							case 4: if ((mpos[i].z >            0) && (mungetboard(cmun,x,y,z-1,'.') != '.')) { mgoal[i].z = mpos[i].z-1.f; mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z =-1.f; } break;
							case 5: if ((mpos[i].z < cmun->zsiz-1) && (mungetboard(cmun,x,y,z+1,'.') != '.')) { mgoal[i].z = mpos[i].z+1.f; mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z =+1.f; } break;
						}
					}
					if ((timdeath <= 0.0) && (tim >= timnextlev))
					{
						if (mgoal[i].x < mpos[i].x) { mpos[i].x -= (float)(munlev*0+3)*dtim; if (mpos[i].x <= mgoal[i].x) mpos[i].x = mgoal[i].x; }
						if (mgoal[i].x > mpos[i].x) { mpos[i].x += (float)(munlev*0+3)*dtim; if (mpos[i].x >= mgoal[i].x) mpos[i].x = mgoal[i].x; }
						if (mgoal[i].y < mpos[i].y) { mpos[i].y -= (float)(munlev*0+3)*dtim; if (mpos[i].y <= mgoal[i].y) mpos[i].y = mgoal[i].y; }
						if (mgoal[i].y > mpos[i].y) { mpos[i].y += (float)(munlev*0+3)*dtim; if (mpos[i].y >= mgoal[i].y) mpos[i].y = mgoal[i].y; }
						if (mgoal[i].z < mpos[i].z) { mpos[i].z -= (float)(munlev*0+3)*dtim; if (mpos[i].z <= mgoal[i].z) mpos[i].z = mgoal[i].z; }
						if (mgoal[i].z > mpos[i].z) { mpos[i].z += (float)(munlev*0+3)*dtim; if (mpos[i].z >= mgoal[i].z) mpos[i].z = mgoal[i].z; }
					}
					if (fabs(mpos[i].x-mun.x) + fabs(mpos[i].y-mun.y) + fabs(mpos[i].z-mun.z) < .99f)
					{
						if (timpower >= tim)
						{
								//player kills ghost
							for(j=256;j>0;j--)
							{
								x = rand()%cmun->xsiz;
								y = rand()%cmun->ysiz;
								z = rand()%cmun->zsiz;
								if ((mungetboard(cmun,x,y,z,'.') != '.') && (fabs((float)x-mun.x) + fabs((float)y-mun.y) + fabs((float)z-mun.z) >= 4.f)) break;
							}
							mpos[i].x = x; mpos[i].y = y; mpos[i].z = 0.f; mgoal[i] = mpos[i];
							mvel[i].x = 0.f; mvel[i].y = 0.f; mvel[i].z = 0.f;

							voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,8.f);
						}
						else if (timdeath <= 0.0)
						{
								//ghost kills player
							voxie_playsound("c:/windows/media/recycle.wav",-1,500,500,0.3f);
							voxie_playsound("c:/windows/media/recycle.wav",-1,300,300,0.5f);
							voxie_playsound("c:/windows/media/recycle.wav",-1,100,100,0.7f);
							timdeath = 3.0; fscgoal = .6f;
						}
					}
				}

				x = (int)(mun.x+cmun->xsiz+.5f)%cmun->xsiz;
				y = (int)(mun.y+cmun->ysiz+.5f)%cmun->ysiz;
				z = (int)(mun.z+cmun->zsiz+.5f)%cmun->zsiz;
				i = (z*cmun->ysiz+y)*cmun->xsiz+x;
				if (got[i])
				{
					got[i] = 0;
					if (cmun->board[i] == 'P') { voxie_playsound("c:/windows/media/tada.wav",-1,100,100,1.f); timpower = tim+8.f; } //power pellet
												 else { voxie_playsound("c:/windows/media/notify.wav",-1,100,100,5.f); }
					numdots--;
					if (numdots <= 0)
					{
						voxie_playsound("c:/windows/media/tada.wav",-1,100,100,1.f); timnextlev = tim+3.f;
					}
				}

				if (cmun->zsiz == 1) //2D
				{
					fscale = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
					viewcent = mun;
					if ((vw.nblades > 0) && (vw.aspx == vw.aspy)) { fscale *= 2.f; viewcent.y -= vw.aspy*fscale; }
				}
				else //3D
				{
					fsc = 1.f;
					fscale = (float)cmun->zsiz*.5f/vw.aspz;
					viewcent.x = mun.x;
					viewcent.y = mun.y;
					if ((vw.nblades > 0) && (vw.aspx == vw.aspy)) viewcent.y -= vw.aspy*.5f*fscale;
					viewcent.z = (cmun->zsiz-1)*.5f;
				}
				voxie_setview(&vf,-vw.aspx*fscale,-vw.aspy*fscale,-vw.aspz*fscale,+vw.aspx*fscale,+vw.aspy*fscale,+vw.aspz*fscale);

				if (tim >= timnextlev)
				{     //draw muncher
					point3d pp, rr, dd, ff;
					pp.x = (mun.x-viewcent.x)*fsc;
					pp.y = (mun.y-viewcent.y)*fsc;
					pp.z = (mun.z-viewcent.z)*fsc;
					if (vel.z == 0.f)
					{
						rr.x = vel.y*fsc; rr.y = vel.x*-fsc; rr.z = 0.00f;
						dd.x = vel.x*fsc; dd.y = vel.y*fsc; dd.z = 0.00f;
						ff.x = 0.00f; ff.y = 0.00f; ff.z = fsc;
					}
					else
					{
						rr.x = fsc; rr.y = 0.00f; rr.z = 0.00f;
						dd.x = 0.00f; dd.y = 0.00f; dd.z = vel.z*fsc;
						ff.x = 0.00f; ff.y = -vel.z*fsc; ff.z = 0.00f;
					}
					if (timdeath > 0.0)
					{
						rotate_vex(timdeath*12.f,&rr,&dd);
						f = timdeath/3.0;
						rr.x *= f; rr.y *= f;
						dd.x *= f; dd.y *= f;
						ff.z *= f;
					}
					i = min(max((int)((fabs(fabs(nmun.x-mun.x)+fabs(nmun.y-mun.y)+fabs(nmun.z-mun.z)-.5f))*16.f+0.5f),0),MUNCHFRAMES-1)+GHOSTFRAMES;
					voxie_drawspr(&vf,munnam[i],&pp,&rr,&dd,&ff,0x101010);
				}

				for(i=0;i<nmon;i++)
				{     //draw ghost
					point3d pp, rr, dd, ff;
					pp.x = (mpos[i].x-viewcent.x)*fsc;
					pp.y = (mpos[i].y-viewcent.y)*fsc;
					pp.z = (mpos[i].z-viewcent.z)*fsc;
					if (mvel[i].z == 0.f)
					{
						rr.x = mvel[i].y*fsc; rr.y = mvel[i].x*-fsc; rr.z = 0.00f;
						dd.x = mvel[i].x*fsc; dd.y = mvel[i].y*fsc; dd.z = 0.00f;
						ff.x = 0.0f; ff.y = 0.0f; ff.z = fsc;
					}
					else
					{
						rr.x = fsc; rr.y = 0.00f; rr.z = 0.00f;
						dd.x = 0.00f; dd.y = 0.00f; dd.z = mvel[i].z*fsc;
						ff.x = 0.00f; ff.y = -mvel[i].z*fsc; ff.z = 0.00f;
					}
					switch((int)(fmod(tim*4.0,1.0)*4.f)&3)
					{
						case 0: j = 0; break;
						case 1: j = 1; break;
						case 2: j = 1; rr.x = -rr.x; rr.y = -rr.y; rr.z = -rr.z; break;
						case 3: j = 0; rr.x = -rr.x; rr.y = -rr.y; rr.z = -rr.z; break;
					}
					voxie_drawspr(&vf,munnam[j],&pp,&rr,&dd,&ff,mcol[i&3]);
				}

				for(z=cmun->zwrap*-8;z<cmun->zsiz+cmun->zwrap*8;z++)
					for(y=cmun->ywrap*-8;y<cmun->ysiz+cmun->ywrap*8;y++)
						for(x=cmun->xwrap*-8;x<cmun->xsiz+cmun->xwrap*8;x++)
						{
							if (mungetboard(cmun,x,y,z,'.') == '.') continue;

							fx = ((float)x-viewcent.x)*fsc;
							fy = ((float)y-viewcent.y)*fsc;
							fz = ((float)z-viewcent.z)*fsc;

							if (fz+fsc*.5f < -vw.aspz*fscale) continue;
							if (fz-fsc*.5f > +vw.aspz*fscale) continue;
							if (!vw.clipshape)
							{
								if (fx+fsc*.5f < -vw.aspx*fscale) continue;
								if (fy+fsc*.5f < -vw.aspy*fscale) continue;
								if (fx-fsc*.5f > +vw.aspx*fscale) continue;
								if (fy-fsc*.5f > +vw.aspy*fscale) continue;
							}
							else
							{
								if (sqrt(fx*fx + fy*fy)-fsc*sqrt(.5f) >= vw.aspr*fscale) continue;
							}

							i = ((z+cmun->zsiz)%cmun->zsiz)*cmun->xsiz*cmun->ysiz
							  + ((y+cmun->ysiz)%cmun->ysiz)*cmun->xsiz
							  + ((x+cmun->xsiz)%cmun->xsiz);
							if (got[i])
							{
								if (cmun->board[i] == 'P') //power pellet
									  voxie_drawsph(&vf,fx,fy,fz,fsc*.2f,1,0xffffff); //power pellet
								else voxie_drawsph(&vf,fx,fy,fz,fsc*.1f,0,0xffff00); //regular dot
							}

							j = 0;
							if (mungetboard(cmun,x-1,y,z,'.') != '.') j += (1<<0);
							if (mungetboard(cmun,x+1,y,z,'.') != '.') j += (1<<1);
							if (mungetboard(cmun,x,y-1,z,'.') != '.') j += (1<<2);
							if (mungetboard(cmun,x,y+1,z,'.') != '.') j += (1<<3);
							if (mungetboard(cmun,x,y,z-1,'.') != '.') j += (1<<4);
							if (mungetboard(cmun,x,y,z+1,'.') != '.') j += (1<<5);
#if 0
							i = 0x0000ff;
							if (timpower >= tim)
							{
								f = timpower-tim;
								if ((f > 1.0f) || (fabs(f-0.625) < .125) || (fabs(f-.125) < .125)) i = 0x00ffff;
							}

							switch (j)
							{
									//             11 2D cases (1 4 6|4 1): |  57 3D cases! (1 6 15 20 15|6 1)
									// ÚÄÄÄÂÄÄÄ¿ 0xa 0x3 0xb 0x3 0x9        |  1 6-way path: axes
									// ³   ³   ³ 0xc     0xc     0xc        |  6 5-way paths: Rubix centers
									// ÃÄÄÄÅÄÄÄ´ 0xe 0x3 0xf 0x3 0xd        | 15 4-way paths: 3 plus signs + 12 corners with 1 full axis
									// ³   ³   ³ 0xc     0xc     0xc        | 20 3-way paths: 8 corners + 12 half plus sign Packer pieces
									// ÀÄÄÄÁÄÄÄÙ 0x6 0x3 0x7 0x3 0x5        | 15 2-way paths: 3 straight lines + 12 Rubix edges
								case 0x3: drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz,  0.f, 0.f,0.f,  0.f, 0.f,0.f, i); break;
								case 0xc: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz,  0.f, 0.f,0.f,  0.f, 0.f,0.f, i); break;

								case 0xa: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f,  0.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f,  0.f, 0.f,0.f, i); break;
								case 0x9: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, +1.f,+1.f,0.f,  0.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f,  0.f, 0.f,0.f, i); break;
								case 0x6: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f,  0.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,+1.f,0.f,  0.f, 0.f,0.f, i); break;
								case 0x5: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f,  0.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f,  0.f, 0.f,0.f, i); break;

								case 0xe: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f,  0.f,-1.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f,  0.f,+1.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f, +1.f,+1.f,0.f, i); break;
								case 0xd: drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f,  0.f,-1.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, +1.f,+1.f,0.f,  0.f,+1.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f, -1.f,+1.f,0.f, i); break;
								case 0xb: drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f, -1.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f, +1.f, 0.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f, +1.f,+1.f,0.f, i); break;
								case 0x7: drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f, -1.f, 0.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,+1.f,0.f, +1.f, 0.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f, +1.f,-1.f,0.f, i); break;

								case 0xf: drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, +1.f,-1.f,0.f, +1.f,+1.f,0.f, i);
											 drawcone_bot(fx-fsc*.5f,fy,fz,fsc*.5f, fx+fsc*.5f,fy,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f, -1.f,+1.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,+1.f,0.f, +1.f,+1.f,0.f, i);
											 drawcone_bot(fx,fy-fsc*.5f,fz,fsc*.5f, fx,fy+fsc*.5f,fz,fsc*.5f, fx,fy,fz, -1.f,-1.f,0.f, +1.f,-1.f,0.f, i); break;
							}
#elif 0
							int m;
							m = 0x000001;
							if (timpower >= tim)
							{
								f = timpower-tim;
								if ((f > 1.0f) || (fabs(f-0.625) < .125) || (fabs(f-.125) < .125)) m = 0x010000;
							}

							for(i=0;i<6;i++)
							{
								float fx2, fy2, fz2;
								if (!(j&(1<<i))) continue;
								fx2 = fx; fy2 = fy; fz2 = fz;
								if (i == 0) fx2 -= fsc*.5f;
								if (i == 1) fx2 += fsc*.5f;
								if (i == 2) fy2 -= fsc*.5f;
								if (i == 3) fy2 += fsc*.5f;
								if (i == 4) fz2 -= fsc*.5f;
								if (i == 5) fz2 += fsc*.5f;
								//voxie_drawcone(&vf, fx,fy,fz,fsc*.25f, fx2,fy2,fz2,fsc*.25f, 1,0x000004);
								for(k=0;k<4;k++)
								{
									voxie_drawsph(&vf, (fx2-fx)*((float)k+.5f)/4.f + fx,
															 (fy2-fy)*((float)k+.5f)/4.f + fy,
															 (fz2-fz)*((float)k+.5f)/4.f + fz, fsc*.6f, 0,m);

									//draw_platonic(2,   (fx2-fx)*((float)k+.5f)/4.f + fx,
									//                   (fy2-fy)*((float)k+.5f)/4.f + fy,
									//                   (fz2-fz)*((float)k+.5f)/4.f + fz, fsc*1.f, 0.f,1,m);
								}
							}
#else
							int m;
							m = 0x80c0ff; i = 28;
							if (timpower >= tim)
							{
								f = timpower-tim;
								if ((f > 1.0f) || (fabs(f-0.625) < .125) || (fabs(f-.125) < .125)) { m = 0xffc080; i = 20; }
							}
							drawcyl(fx,fy,fz,fsc*.5f,m,j,6,i);
#endif
						}

				{
				point3d pp, rr, dd;
				char buf[64];
				pp.x = 0.0f; pp.y = vw.aspy*fscale*-.99f; pp.z = -.18f*vw.aspz*fscale;
				rr.x = 0.2f*fscale; rr.y = 0.0f; rr.z = 0.0f;
				dd.x = 0.0f; dd.y = 0.0f; dd.z = 0.36f*fscale;
				sprintf(buf,"%d 2GO",numdots);
				f = (float)strlen(buf)*.5; pp.x -= rr.x*f; pp.y -= rr.y*f; pp.z -= rr.z*f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);

				pp.x = 0.0f; pp.y = vw.aspy*fscale*+.99f; pp.z = -.18f*vw.aspz*fscale;
				rr.x = 0.2f*vw.aspy*fscale; rr.y = 0.0f; rr.z = 0.0f;
				dd.x = 0.0f; dd.y = 0.0f; dd.z = 0.36f*vw.aspy*fscale;
				sprintf(buf,"Level %d",munlev+1);
				f = (float)strlen(buf)*.5; pp.x -= rr.x*f; pp.y -= rr.y*f; pp.z -= rr.z*f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",buf);
				}

				//if (voxie_keystat(0x9c) == 1) timnextlev = tim+3.0; //debug only!

				if (tim < timnextlev)
				{
					point3d pp, rr, dd;
					pp.x =-1.00*fscale; rr.x = 0.15*fscale; dd.x = 0.00;
					pp.y = cos(tim*4.0)*-0.15*fscale; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*fscale;
					pp.z = sin(tim*4.0)*-0.15*fscale; rr.z = 0.00; dd.z = sin(tim*4.0)*0.30*fscale;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Level %d clear",munlev+1);
				}

				}
				break;
#endif
			case RENDMODE_SNAKETRON: //SnakeTron
				{
				#define SNAKEMAX 4096
				typedef struct { point3d p[SNAKEMAX], dir, odir; float rad, spd; int i0, i1, score, reset; } snake_t;
				static snake_t snake[4];

				#define PEL2MAX 16
				typedef struct { point3d p, v; int issph; } pel_t;
				static pel_t pel[PEL2MAX];
				static int peln;

				#define SPLODEMAX 16
				typedef struct { point3d p; double tim; } splode_t;
				static splode_t splode[SPLODEMAX];
				static int sploden = 0;

				static int inited = 0, gotx, numplays = 2, winner;
				static double timend = 0.0;
				int p;

				if (voxie_keystat(keyremap[0][6]) == 1) { inited = 0; break; } //ButM:reset&title
				if ((voxie_keystat(0x0e) == 1) || (ghitkey == 0x0e) || ((vx[0].but&~ovxbut[0])&(1<<4))) { ghitkey = 0; inited = 0; } //Backspace:reset
				if (!inited)
				{
					inited = 1; peln = gsnakenumpels+gsnakenumtetras;
					for(i=peln-1;i>=0;i--)
					{
						pel[i].p.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
						pel[i].p.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
						pel[i].p.z = (float)((rand()&32767)-16384)/16384.f*vw.aspz;
						vecrand(&pel[i].v,gsnakepelspeed);
						pel[i].issph = (i < gsnakenumpels);

					}
					for(i=0;i<4;i++) { snake[i].reset = 1; snake[i].score = 0; snake[i].odir.x = 0.f; snake[i].odir.y = 0.f; snake[i].odir.z = 0.f; }

					timend = (double)gsnaketime + tim;
					winner = -2;
				}

				i = 0;
				if (voxie_keystat(0x27) == 1) { gsnakepelspeed = max(gsnakepelspeed*0.5f, 0.f); if (gsnakepelspeed <= 1.f/16.f) gsnakepelspeed = 0.f; i = 1; } //;
				if (voxie_keystat(0x28) == 1) { if (gsnakepelspeed < 1.f/16.f) gsnakepelspeed = 1.f/16.f; gsnakepelspeed = min(gsnakepelspeed*2.0f,2.f); i = 1; } //'
				if ((i) || (ghitkey == 0x27))
				{
					for(i=peln-1;i>=0;i--)
					{
						f = pel[i].v.x*pel[i].v.x + pel[i].v.y*pel[i].v.y + pel[i].v.z*pel[i].v.z;
						if (f > 0.f)
						{
							f = gsnakepelspeed/sqrt(f);
							pel[i].v.x *= f;
							pel[i].v.y *= f;
							pel[i].v.z *= f;
						}
						else { vecrand(&pel[i].v,gsnakepelspeed); }
					}
				}

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				if ((gsnaketime <= 0) || (tim < timend))
				{
					numplays = max(vxnplays,gminplays);
					for(p=0;p<numplays;p++)
					{
						if (snake[p].reset)
						{
							snake[p].reset = 0;
							snake[p].i0 = 0; snake[p].i1 = 16; snake[p].spd = .02f; snake[p].rad = .05f;
							if (p == 0) { snake[p].dir.x = 0.f; snake[p].dir.y =-1.f; snake[p].dir.z = 0.f; fx = +0.1f; fy = +vw.aspy; fz = -0.1f; }
							if (p == 1) { snake[p].dir.x = 0.f; snake[p].dir.y =+1.f; snake[p].dir.z = 0.f; fx = -0.1f; fy = -vw.aspy; fz = -0.1f; }
							if (p == 2) { snake[p].dir.x =-1.f; snake[p].dir.y = 0.f; snake[p].dir.z = 0.f; fx = +vw.aspx; fy = -0.1f; fz = +0.1f; }
							if (p == 3) { snake[p].dir.x =+1.f; snake[p].dir.y = 0.f; snake[p].dir.z = 0.f; fx = -vw.aspx; fy = +0.1f; fz = +0.1f; }
							snake[p].odir.x = 1.f; snake[p].odir.y = 1.f; snake[p].odir.z = 1.f;
							for(i=snake[p].i0;i<snake[p].i1;i++)
							{
								snake[p].p[i].x = (float)i*snake[p].dir.x*snake[p].spd + fx;
								snake[p].p[i].y = (float)i*snake[p].dir.y*snake[p].spd + fy;
								snake[p].p[i].z = (float)i*snake[p].dir.z*snake[p].spd + fz;
							}
						}

						snake[p].spd = max(snake[p].spd-dtim*.01f,.02f);
						snake[p].rad = max(snake[p].rad-dtim*.01f,.05f);

						if ((int)(otim*20.0) != (int)(tim*20.0))
						{
							fx = nav[p].dx*.001f;
							fy = nav[p].dy*.001f;
							fz = nav[p].dz*.001f;

							if (gdualnav && (p)) //Super crappy AI
							{
								fx = cos(tim*1.1+(float)p);
								fy = cos(tim*1.3+(float)p);
								fz = cos(tim*1.7+(float)p);
							}

							if (voxie_keystat(keyremap[(p&3)*2][0])) { fx -= 1.f; } //Left
							if (voxie_keystat(keyremap[(p&3)*2][1])) { fx += 1.f; } //Right
							if (voxie_keystat(keyremap[(p&3)*2][2])) { fy -= 1.f; } //Up
							if (voxie_keystat(keyremap[(p&3)*2][3])) { fy += 1.f; } //Down
							if (voxie_keystat(keyremap[(p&3)*2][4])) { fz += 1.f; } //ButA
							if (voxie_keystat(keyremap[(p&3)*2][5])) { fz -= 1.f; } //ButB

							if (fabs(fx) + fabs(fy) + fabs(fz) > 0.f) { snake[p].dir.x = fx; snake[p].dir.y = fy; snake[p].dir.z = fz; }
							if (p < vxnplays)
							{
								if (p == 0) { fx = vx[p].tx0; fy =-vx[p].ty0; fz =-vx[p].ty1; }
								if (p == 1) { fx =-vx[p].tx0; fy = vx[p].ty0; fz =-vx[p].ty1; }
								if (p == 2) { fx =-vx[p].ty0; fy =-vx[p].tx0; fz =-vx[p].ty1; }
								if (p == 3) { fx = vx[p].ty0; fy = vx[p].tx0; fz =-vx[p].ty1; }
								fz += (vx[p].rt - vx[p].lt)*128.0 - (vx[p].but&(1<<9))*128;
								f = fx*fx + fy*fy + fz*fz;
								if (f > 4096.0*4096.0) { f = 1.0/sqrt(f); snake[p].dir.x = fx*f; snake[p].dir.y = fy*f; snake[p].dir.z = fz*f; }
							}

							j = (snake[p].i1-1)&(SNAKEMAX-1);
							if (fabs(snake[p].p[j].z + snake[p].dir.z*snake[p].spd) > vw.aspz) snake[p].dir.z = 0.f;

								//Normalize
							f = snake[p].dir.x*snake[p].dir.x + snake[p].dir.y*snake[p].dir.y + snake[p].dir.z*snake[p].dir.z;
							if (f == 0.f)
							{
								snake[p].dir = snake[p].odir;
								f = snake[p].dir.x*snake[p].dir.x + snake[p].dir.y*snake[p].dir.y + snake[p].dir.z*snake[p].dir.z;
							}
							if (f != 0.f)
							{
								f = 1.0/sqrt(f);
								snake[p].dir.x *= f; if (snake[p].dir.x != 0.f) snake[p].odir.x = snake[p].dir.x;
								snake[p].dir.y *= f; if (snake[p].dir.y != 0.f) snake[p].odir.y = snake[p].dir.y;
								snake[p].dir.z *= f; if (snake[p].dir.z != 0.f) snake[p].odir.z = snake[p].dir.z;
							}
							else
							{
								voxie_playsound("c:/windows/media/chord.wav",-1,100,100,1.f); snake[p].reset = 1;
								if (sploden < SPLODEMAX-1)
								{
									splode[sploden].p = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)];
									splode[sploden].tim = tim;
									sploden++;
								}
							}

							snake[p].i0++;
							i = snake[p].i1&(SNAKEMAX-1); j = (snake[p].i1-1)&(SNAKEMAX-1);
							snake[p].p[i].x = snake[p].p[j].x + snake[p].dir.x*snake[p].spd;
							snake[p].p[i].y = snake[p].p[j].y + snake[p].dir.y*snake[p].spd;
							if (!vw.clipshape)
							{
								if (snake[p].p[i].x < -vw.aspx) snake[p].p[i].x += vw.aspx*2.f; if (snake[p].p[i].x > +vw.aspx) snake[p].p[i].x -= vw.aspx*2.f;
								if (snake[p].p[i].y < -vw.aspy) snake[p].p[i].y += vw.aspy*2.f; if (snake[p].p[i].y > +vw.aspy) snake[p].p[i].y -= vw.aspy*2.f;
							}
							else
							{
								if (snake[p].p[i].x*snake[p].p[i].x + snake[p].p[i].y*snake[p].p[i].y >= vw.aspr*vw.aspr)
									{ snake[p].p[i].x *= -1.f; snake[p].p[i].y *= -1.f; }
							}
							snake[p].p[i].z = min(max(snake[p].p[j].z + snake[p].dir.z*snake[p].spd,-vw.aspz),+vw.aspz);
							snake[p].i1++;
							snake[p].score += (snake[p].i1-snake[p].i0)/16-1;

							for(j=numplays-1;j>=0;j--)
							{
								if (j == p) i = snake[j].i1-12; else i = snake[j].i1-1;
								for(;i>=snake[j].i0;i--)
								{
									fx = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].x-snake[j].p[i&(SNAKEMAX-1)].x;
									fy = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].y-snake[j].p[i&(SNAKEMAX-1)].y;
									fz = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].z-snake[j].p[i&(SNAKEMAX-1)].z;
									if (fx*fx + fy*fy + fz*fz >= snake[p].rad*1.5*snake[j].rad*1.5) continue;
									voxie_playsound("c:/windows/media/chord.wav",-1,100,100,1.f); snake[p].reset = 1;
									if (sploden < SPLODEMAX-1)
									{
										splode[sploden].p = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)];
										splode[sploden].tim = tim;
										sploden++;
									}
									break;
								}
							}

							for(i=peln-1;i>=0;i--)
							{
								pel[i].p.x += pel[i].v.x*dtim;
								pel[i].p.y += pel[i].v.y*dtim;
								if (!vw.clipshape)
								{
									if (pel[i].p.x < -vw.aspx) pel[i].p.x += vw.aspx*2.f; if (pel[i].p.x > +vw.aspx) pel[i].p.x -= vw.aspx*2.f;
									if (pel[i].p.y < -vw.aspy) pel[i].p.y += vw.aspy*2.f; if (pel[i].p.y > +vw.aspy) pel[i].p.y -= vw.aspy*2.f;
								}
								else
								{
									if (pel[i].p.x*pel[i].p.x + pel[i].p.y*pel[i].p.y >= vw.aspr*vw.aspr)
										{ pel[i].p.x *= -1.f; pel[i].p.y *= -1.f; }
								}
								pel[i].p.z += pel[i].v.z*dtim; if (fabs(pel[i].p.z) > vw.aspz) pel[i].v.z = fabs(pel[i].v.z)*(float)(1-(pel[i].p.z > 0.f)*2);

								fx = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].x-pel[i].p.x;
								fy = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].y-pel[i].p.y;
								fz = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)].z-pel[i].p.z;
								if (fx*fx + fy*fy + fz*fz < snake[p].rad*2.f*.1f)
								{
									if (pel[i].issph)
									{
										voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,1.f);
										snake[p].i0 = max(max(snake[p].i0-16,0),snake[p].i1-SNAKEMAX);
										pel[i].p.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
										pel[i].p.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										pel[i].p.z = (float)((rand()&32767)-16384)/16384.f*vw.aspz;
										vecrand(&pel[i].v,gsnakepelspeed);
									}
									else
									{
#if 0
											//IMO speed boost on tetrahedron looks silly
										voxie_playsound("c:/windows/media/chimes.wav",-1,100,100,2.f);
										snake[p].spd = .08f;
										//snake[p].rad = .10f;
#else
										voxie_playsound("c:/windows/media/chord.wav",-1,100,100,1.f); snake[p].reset = 1;
										if (sploden < SPLODEMAX-1)
										{
											splode[sploden].p = snake[p].p[(snake[p].i1-1)&(SNAKEMAX-1)];
											splode[sploden].tim = tim;
											sploden++;
										}
#endif
									}
								}
							}
						}
					}

						//Draw pels (spheres and tetrahedrons) only during active game
					for(i=peln-1;i>=0;i--)
					{
						if (pel[i].issph) voxie_drawsph(&vf,pel[i].p.x,pel[i].p.y,pel[i].p.z,0.05f,1,0xffffff);
										 else draw_platonic(0,pel[i].p.x,pel[i].p.y,pel[i].p.z,0.1f,tim*2.f,2,0xff00ff);

					}
				}

					//Draw snakes
				for(p=0;p<numplays;p++)
				{
					if ((gsnaketime > 0) && (winner != -2) && (winner != p)) continue;

					j = gsnakecol[p];
					for(i=snake[p].i0;i<snake[p].i1;i++)
					{
						f = snake[p].rad - min(snake[p].i1-1-i,12)/12.f*0.025f;
						if (i >= snake[p].i1-12) j = 0xffffff;
						voxie_drawsph(&vf,snake[p].p[i&(SNAKEMAX-1)].x,snake[p].p[i&(SNAKEMAX-1)].y,snake[p].p[i&(SNAKEMAX-1)].z,f,i==snake[p].i1-1,j);
					}
				}

					//Draw explosions
				for(i=sploden-1;i>=0;i--)
				{
					float fdotrad;
					f = tim-splode[i].tim; if (f >= 2.0) { sploden--; splode[i] = splode[sploden]; continue; }
					n = 128; fr = sqrt(f)*0.25; fdotrad = (1.f-f/2.0)*.007f;
					for(j=n-1;j>=0;j--)
					{
						fz = ((double)j+.5)/(double)n*2.0-1.0; f = sqrt(1.0 - fz*fz);
						g = (double)j*(PI*(sqrt(5.0)-1.0)); fx = cos(g)*f; fy = sin(g)*f;
						voxie_drawsph(&vf,fx*fr+splode[i].p.x,
												fy*fr+splode[i].p.y,
												fz*fr+splode[i].p.z,fdotrad,0,0xffffff);
					}
				}


				{  //Draw scores
				static const int cornlut[4] = {0,2,3,1};
				char buf[256];
				point3d pp, rr, dd;
				for(i=0;i<numplays;i++)
				{
					j = cornlut[i];
					rr.x = cos(((float)j+0.0)*PI*2.0/4.0);
					rr.y = sin(((float)j+0.0)*PI*2.0/4.0);
					rr.z = 0.f;
					dd.x =-sin(((float)j+0.0)*PI*2.0/4.0);
					dd.y = cos(((float)j+0.0)*PI*2.0/4.0);
					dd.z = 0.f;
					pp.x = dd.x*(vw.aspx-.2f);
					pp.y = dd.y*(vw.aspy-.2f);
					pp.z = vw.aspz*.95f;
					f = .12f; rr.x *= f; rr.y *= f; rr.z *= f;
					f = .20f; dd.x *= f; dd.y *= f; dd.z *= f;
					sprintf(buf,"%d",snake[i].score);
					pp.x -= (float)strlen(buf)*rr.x*.5;
					pp.y -= (float)strlen(buf)*rr.y*.5;
					pp.z -= (float)strlen(buf)*rr.z*.5;
					voxie_printalph_(&vf,&pp,&rr,&dd,gsnakecol[i],"%s",buf);
				}

					//Draw timer (if applicable)
				if (gsnaketime > 0)
				{
					rr.x = .12f; rr.y = 0.f; rr.z = 0.f;
					dd.x = 0.f; dd.y = .20f; dd.z = 0.f;
					pp.x = 0.f; pp.y = 0.f; pp.z = vw.aspz-0.01f;
					f = timend-tim;
					if (f > 0.f)
					{
						i = (int)(f)+1;
						if (i < 3600) { sprintf(buf,"%d:%02d",i/60,i%60); }
									else { sprintf(buf,"%d:%02d:%02d",i/3600,(i/60)%60,i%60); }
						p = 0xffffff;
					}
					else
					{
						if (winner == -2)
						{
							winner = 0; j = 0;
							for(i=numplays-1;i>0;i--)
								if (snake[i].score >= snake[winner].score)
									{ j = (snake[i].score == snake[winner].score); winner = i; }
							if (j) winner = -1;
						}
						if (winner < 0)
						{
							sprintf(buf,"Game Over");
							p = 0xffffff;
						}
						else
						{
								//Make winning snake go in circle
							if ((int)(otim*20.0) != (int)(tim*20.0))
							{
								p = winner;

								snake[p].i0++;
								i = snake[p].i1&(SNAKEMAX-1); j = (snake[p].i1-1)&(SNAKEMAX-1);
								snake[p].dir.x = cos(tim*0.5)*vw.aspx - snake[p].p[j].x;
								snake[p].dir.y = sin(tim*0.5)*vw.aspy - snake[p].p[j].y;
								snake[p].dir.z = sin(tim*4.0)*vw.aspz - snake[p].p[j].z;
								f = snake[p].dir.x*snake[p].dir.x + snake[p].dir.y*snake[p].dir.y + snake[p].dir.z*snake[p].dir.z;
								if (f > 0.f) { f = 1.f/sqrt(f); snake[p].dir.x *= f; snake[p].dir.y *= f; snake[p].dir.z *= f; }
								snake[p].p[i].x = snake[p].p[j].x + snake[p].dir.x*snake[p].spd;
								snake[p].p[i].y = snake[p].p[j].y + snake[p].dir.y*snake[p].spd;
								snake[p].p[i].z = snake[p].p[j].z + snake[p].dir.z*snake[p].spd;
								snake[p].i1++;
							}

							sprintf(buf,"Winner!");
							p = gsnakecol[winner];
						}
					}
					pp.x -= ((float)strlen(buf)*rr.x + dd.x)*.5;
					pp.y -= ((float)strlen(buf)*rr.y + dd.y)*.5;
					pp.z -= ((float)strlen(buf)*rr.z + dd.z)*.5;
					voxie_printalph_(&vf,&pp,&rr,&dd,p,"%s",buf);
				}

				}

				}
				break;
			case RENDMODE_FLYSTOMP: //FlyStomp
				{
				typedef struct { float x, y, z, xv, yv, zv, ha, va, onfloor, flaptim; int sc; } fly_t;
				#define FLYMAX 4
				static fly_t fly[FLYMAX];
				static int flyn = 0;

				typedef struct { float x, y, z, xs, ys; } plat_t;
				#define PLATMAX 16
				static plat_t plat[PLATMAX];
				static int platn = 0;

				typedef struct { float x, y, z, xv, yv, zv, rad; int col; float dtim; } part_t;
				#define PARTMAX 4096
				static part_t part[PARTMAX];
				static int partn = 0;

				static int inited = 0;
				point3d pp, rr, dd, ff;
				float fx, fy, fz, dmin, d;

				if (voxie_keystat(keyremap[0][6]) == 1) { inited = 0; break; } //ButM:reset&title
				if ((voxie_keystat(0x0e) == 1) || (ghitkey == 0x0e) || ((vx[0].but&~ovxbut[0])&(1<<4))) { ghitkey = 0; inited = 0; } //Backspace:reset
				if (!inited)
				{
					voxie_mountzip("flystomp.zip");
					inited = 1;

					flyn = 0;

					fly[flyn].x =-.5f; fly[flyn].xv = 0.f;
					fly[flyn].y =-.2f; fly[flyn].yv = 0.f;
					fly[flyn].z =+vw.aspz-.1f; fly[flyn].zv = 0.f; fly[flyn].ha = 0.f; fly[flyn].va = 0.f; fly[flyn].onfloor = 0.f; fly[flyn].sc = 0; fly[flyn].flaptim = -1.0; flyn++;

					fly[flyn].x =+.5f; fly[flyn].xv = 0.f;
					fly[flyn].y =+.2f; fly[flyn].yv = 0.f;
					fly[flyn].z =+vw.aspz-.1f; fly[flyn].zv = 0.f; fly[flyn].ha = 0.f; fly[flyn].va = 0.f; fly[flyn].onfloor = 0.f; fly[flyn].sc = 0; fly[flyn].flaptim = -1.0; flyn++;

					platn = 0;

					plat[platn].x = 0.f; plat[platn].y = 0.f; plat[platn].z = vw.aspz; plat[platn].xs = vw.aspx; plat[platn].ys = vw.aspy; platn++;
					plat[platn].x = -0.7f; plat[platn].y = -0.25f; plat[platn].z = -0.05f; plat[platn].xs = 0.2f; plat[platn].ys = 0.20f; platn++;
					plat[platn].x = +0.5f; plat[platn].y = -0.25f; plat[platn].z = +0.01f; plat[platn].xs = 0.3f; plat[platn].ys = 0.15f; platn++;
					plat[platn].x = -0.4f; plat[platn].y = +0.25f; plat[platn].z = -0.05f; plat[platn].xs = 0.3f; plat[platn].ys = 0.15f; platn++;
					plat[platn].x = +0.6f; plat[platn].y = +0.25f; plat[platn].z = +0.13f; plat[platn].xs = 0.2f; plat[platn].ys = 0.20f; platn++;
				}

				voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

				for(i=0;i<flyn;i++)
				{
#if 0
					if (i < vxnplays)
					{
						fly[i].xv += (float)(vx[i].tx0+vx[i].tx1)*dtim*+.00005f;
						fly[i].yv += (float)(vx[i].ty0+vx[i].ty1)*dtim*-.00005f;
					}
					if (voxie_keystat(keyremap[i*2][0])) { fly[i].xv -= dtim*1.f; } //Left
					if (voxie_keystat(keyremap[i*2][1])) { fly[i].xv += dtim*1.f; } //Right
					if (voxie_keystat(keyremap[i*2][2])) { fly[i].yv -= dtim*1.f; } //Up
					if (voxie_keystat(keyremap[i*2][3])) { fly[i].yv += dtim*1.f; } //Down
#else
					fx = 0.f;
					fy = 0.f;
					if (i < vxnplays)
					{
						fx += (float)(vx[i].tx0+vx[i].tx1)*dtim*+.00005f;
						fy += (float)(vx[i].ty0+vx[i].ty1)*dtim*-.00005f;
					}
					if (voxie_keystat(keyremap[i*2][0])) { fx -= dtim*1.f; } //Left
					if (voxie_keystat(keyremap[i*2][1])) { fx += dtim*1.f; } //Right
					if (voxie_keystat(keyremap[i*2][2])) { fy -= dtim*1.f; fly[i].va += (-.8f - fly[i].va)*.2; } //Up
					if (voxie_keystat(keyremap[i*2][3])) { fy += dtim*1.f; fly[i].va += (+.8f - fly[i].va)*.2; } //Down
					fly[i].va *= .95f;

					fly[i].ha += fx*2.5f;
					fly[i].xv += sin(fly[i].ha)*fy*.5f;
					fly[i].yv -= cos(fly[i].ha)*fy*.5f;
#endif
					if ((voxie_keystat(keyremap[i*2][4]) == 1) || (vx[i].but&~ovxbut[i]&(1<<12))) //ButA
					{
						fly[i].zv -= 0.25f;
						voxie_playsound("flap.flac",-1,100,100,1.f);
						fly[i].flaptim = tim;
					}

					fly[i].zv += dtim*1.f; //Gravity

						//Max speed
					f = sqrt(fly[i].xv*fly[i].xv + fly[i].yv*fly[i].yv);
					if (f >= 1.f) { f = 1.f/f; fly[i].xv *= f; fly[i].yv *= f;  }

					for(j=8-1;j>0;j--)
					{
						f = dtim*1.f;
						fx = fly[i].x + fly[i].xv*f*(float)((j>>0)&1);
						fy = fly[i].y + fly[i].yv*f*(float)((j>>1)&1);
						fz = fly[i].z + fly[i].zv*f*(float)((j>>2)&1);

						dmin = 1e32;
						for(k=platn-1;k>=0;k--)
						{
							d = dist2plat2(fx,fy,fz,plat[k].x,plat[k].y,plat[k].z,plat[k].xs,plat[k].ys);
							if (d < dmin) dmin = d;
						}
						if (dmin < .05f*.05f) continue;
						f = 1.f/dtim;
						fly[i].xv = (fx-fly[i].x)*f;
						fly[i].yv = (fy-fly[i].y)*f;
						fly[i].zv = (fz-fly[i].z)*f;
						fly[i].x = fx;
						fly[i].y = fy;
						fly[i].z = fz;
						break;
					}

					if (j < 7)
					{
						if (fly[i].onfloor < 0.f) voxie_playsound("plop.flac",-1,25,25,1.f);
						fly[i].onfloor = .25;
						fly[i].xv *= pow(.05,dtim);
						fly[i].yv *= pow(.05,dtim);
					}
					else
					{
						fly[i].onfloor -= dtim;
						fly[i].xv *= pow(.6,dtim);
						fly[i].yv *= pow(.6,dtim);
						fly[i].zv *= pow(.6,dtim);
					}


					if (fly[i].x < -vw.aspx) fly[i].x += vw.aspx*2.f;
					if (fly[i].x > +vw.aspx) fly[i].x -= vw.aspx*2.f;
					if (fly[i].y < -vw.aspy) fly[i].y += vw.aspy*2.f;
					if (fly[i].y > +vw.aspy) fly[i].y -= vw.aspy*2.f;
					if (fly[i].z < -vw.aspz) fly[i].zv = fabs(fly[i].zv);
					if (fly[i].z > +vw.aspz) fly[i].z = +vw.aspz;

#if 0
					fly[i].ha = atan2(-fly[i].xv,fly[i].yv);
#endif

					f = 0; if (tim-fly[i].flaptim < .25) f = sin((tim-fly[i].flaptim)*PI*2/.25)*.5;
					drawbird(fly[i].x,fly[i].y,fly[i].z,fly[i].ha+PI*.5f,PI*.5f+fly[i].va,f,.25f,(0x00ffff-0xffff00)*i + 0xffff00);

					for(j=0;j<flyn;j++)
					{
						if (i == j) continue;
						d = sqrt((fly[i].x-fly[j].x)*(fly[i].x-fly[j].x) +
									(fly[i].y-fly[j].y)*(fly[i].y-fly[j].y) +
									(fly[i].z-fly[j].z)*(fly[i].z-fly[j].z));
						if (d >= 0.1) continue;
						if (fly[i].z < fly[j].z)
						{
							for(k=256;k>0;k--)
							{
								if (partn >= PARTMAX-1) break;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].x = fly[j].x + fx*.1f;
								part[partn].y = fly[j].y + fy*.1f;
								part[partn].z = fly[j].z + fz*.1f;
								do
								{
									fx = ((rand()/32768.f)-.5f)*2.f;
									fy = ((rand()/32768.f)-.5f)*2.f;
									fz = ((rand()/32768.f)-.5f)*2.f;
								} while (fx*fx + fy*fy + fz*fz > 1.f*1.f);
								part[partn].xv = fx*.5f;
								part[partn].yv = fy*.5f;
								part[partn].zv = fz*.5f;
								part[partn].col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23);
								part[partn].rad = (rand()/32768.0)*0.01+0.005;
								part[partn].dtim = (rand()/32768.0)*1.f + 2.f;
								partn++;
							}

							fly[j].x = (float)(rand()&32767)/32768.0*2.0*vw.aspx - vw.aspx;
							fly[j].y = (float)(rand()&32767)/32768.0*2.0*vw.aspy - vw.aspy;
							fly[j].z = -vw.aspz;
							fly[i].sc++;
							voxie_playsound("blowup2.flac",-1,40,40,1.f);
						}
					}
				}

				for(i=0;i<platn;i++)
				{
					//voxie_drawbox(&vf,plat[i].x-plat[i].xs,plat[i].y-plat[i].ys,plat[i].z-plat[i].zs,
					//                  plat[i].x+plat[i].xs,plat[i].y+plat[i].ys,plat[i].z+plat[i].zs,1,0xffffff);

					pp.x = plat[i].x; pp.y = plat[i].y; pp.z = plat[i].z;
					rr.x = plat[i].xs*2.f; rr.y = 0.f; rr.z = 0.f;
					dd.x = 0.f; dd.y = plat[i].ys*2.f; dd.z = 0.f;
					ff.x = 0.f; ff.y = 0.f; ff.z = 0.25f;
					if (!i) voxie_drawspr(&vf,"lava.kv6",&pp,&rr,&dd,&ff,0x404040);
						else voxie_drawspr(&vf,"emuplatform.kv6",&pp,&rr,&dd,&ff,0x404040);

					voxie_drawcone(&vf,pp.x,pp.y,pp.z,-.02f,pp.x,pp.y,vw.aspz,-.02f,0,0x201810);
				}

					//Process particles..
				for(i=partn-1;i>=0;i--)
				{
					part[i].x += part[i].xv*dtim;
					part[i].y += part[i].yv*dtim;
					part[i].z += part[i].zv*dtim;
					part[i].xv *= pow(.125,dtim);
					part[i].yv *= pow(.125,dtim);
					part[i].zv *= pow(.125,dtim);
					part[i].zv += dtim*.5f;
					if (part[i].z > vw.aspz) { part[i].zv = -fabs(part[i].zv); }

					//voxie_drawvox(&vf,part[i].x,part[i].y,part[i].z,part[i].col);
					voxie_drawsph(&vf,part[i].x,part[i].y,part[i].z,part[i].rad,0,part[i].col);
					part[i].dtim -= dtim;
					if (part[i].dtim < 0.f)
					{
						partn--; part[i] = part[partn];
						continue;
					}
				}

				f = 0.f;
				for(i=1;i<4;i++)
				{
					rr.x = 0.12f; dd.x = 0.00f; pp.x = -0.4f - rr.x*f - dd.x*.5f;
					rr.y = 0.00f; dd.y = 0.00f; pp.y = -vw.aspy - rr.y*f - dd.y*.5f + (float)i*.01f;
					rr.z = 0.00f; dd.z = 0.25f; pp.z = -0.17f - rr.z*f - dd.z*.5f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffff00,"%d",fly[0].sc);

					rr.x = 0.12f; dd.x = 0.00f; pp.x = +0.3f - rr.x*f - dd.x*.5f;
					rr.y = 0.00f; dd.y = 0.00f; pp.y = -vw.aspy - rr.y*f - dd.y*.5f + (float)i*.01f;
					rr.z = 0.00f; dd.z = 0.25f; pp.z = -0.17f - rr.z*f - dd.z*.5f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0x00ffff,"%d",fly[1].sc);
				}

				}
				break;
			case RENDMODE_MODELANIM: //KV6/STL/animation display
				{
				static int colval = 0x404040;
				static int inited = 0;
				int xsiz, ysiz;

				if (labs(vx[0].tx0) < 4096) vx[0].tx0 = 0;
				if (labs(vx[0].ty0) < 4096) vx[0].ty0 = 0;
				if (labs(vx[0].tx1) < 4096) vx[0].tx1 = 0;
				if (labs(vx[0].ty1) < 4096) vx[0].ty1 = 0;

				voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords

				if (!gmodelanim_pauseit)
				{
					i = ganimi;
						//Change animation
					if ((voxie_keystat(0xc9) == 1) || (ghitkey == 0xc9) || (voxie_keystat(0x27) == 1) || ((vx[0].but&~ovxbut[0])&((1<<14)|(1<<8)))) { ghitkey = 0; ganimi--; if (ganimi <       0) ganimi = max(ganimn-1,0); voxie_playsound_update(-1,-1,0,0,1.f);/*kill all sound*/ cycletim = tim+5.0; } //PGUP
					if ((voxie_keystat(0xd1) == 1) || (ghitkey == 0xd1) || (voxie_keystat(0x28) == 1) || ((vx[0].but&~ovxbut[0])&((1<<13)|(1<<9)))) { ghitkey = 0; ganimi++; if (ganimi >= ganimn) ganimi =               0; voxie_playsound_update(-1,-1,0,0,1.f);/*kill all sound*/ cycletim = tim+5.0; } //PGDN
					if ((gautocycle) && (tim > cycletim))
					{
						ganimi++; if (ganimi >= ganimn) ganimi = 0; voxie_playsound_update(-1,-1,0,0,1.f);/*kill all sound*/
						cycletim = tim+5.0;
					}

					if ((i != ganimi) && (ganim[ganimi].snd[0]))
					{
						ganim[ganimi].cnt = 0;
						voxie_playsound(ganim[ganimi].snd,-1,100,100,1.f);
					}
				}

				if (voxie_keystat(0x34) == 1) { colval = min((colval&255)<<1,255)*0x10101; }
				if (voxie_keystat(0x33) == 1) { colval = max((colval&255)>>1,  1)*0x10101; }
				if (voxie_keystat(0x32) == 1) //M:toggle slicemode
				{
					gslicemode = !gslicemode;
					gslicep.x = 0.f; gslicer.x = .05f; gsliced.x = 0.f; gslicef.x = 0.f;
					gslicep.y = 0.f; gslicer.y = 0.f; gsliced.y = .05f; gslicef.y = 0.f;
					gslicep.z = 0.f; gslicer.z = 0.f; gsliced.z = 0.f; gslicef.z = .05f;
				}

				if (((voxie_keystat(0x1f) == 1) && (voxie_keystat(0x1d) || voxie_keystat(0x9d))) || (ghitkey == 0x1f)) //Ctrl+S
				{
					FILE *fil;
					ghitkey = 0;
					fil = fopen("anim.ini","w");
					if (fil)
					{
						fprintf(fil,"%d\n",ganimi);
						for(i=0;i<ganimn;i++)
							fprintf(fil,"%s,%f,%f,%f,%f,%f,%f,%f\n",ganim[i].file,ganim[i].p.x,ganim[i].p.y,ganim[i].p.z,ganim[i].ang[0],ganim[i].ang[1],ganim[i].ang[2],ganim[i].sc);
						fclose(fil);
					}
					MessageBeep(48);
				}

				if ((((voxie_keystat(0x26) == 1) && (voxie_keystat(0x1d) || voxie_keystat(0x9d))) || (ghitkey == 0x26)) || (!inited)) //Ctrl+L
				{
					char tbuf[MAX_PATH];
					FILE *fil;
					inited = 1; ghitkey = 0;
					fil = fopen("anim.ini","r");
					if (fil)
					{
						fscanf(fil,"%d\n",&ganimi); ganimi = max(min(ganimi,ganimn-1),0);
						while (!feof(fil))
						{
							i = 0;
							do { tbuf[i] = fgetc(fil); if (tbuf[i] == ',') break; i++; } while ((!feof(fil)) && (i < MAX_PATH-1) && (tbuf[i] != 13));
							tbuf[i] = 0;

							for(i=ganimn-1;i>=0;i--)
							{
								if (stricmp(ganim[i].file,tbuf)) continue;
								fscanf(fil,"%f,%f,%f,%f,%f,%f,%f",&ganim[i].p.x,&ganim[i].p.y,&ganim[i].p.z,&ganim[i].ang[0],&ganim[i].ang[1],&ganim[i].ang[2],&ganim[i].sc);
								ganim[i].defscale = ganim[i].sc;
								ganim[i].p_init.x = ganim[i].p.x;
								ganim[i].p_init.y = ganim[i].p.y;
								ganim[i].p_init.z = ganim[i].p.z;
								ganim[i].ang_init[0] = ganim[i].ang[0];
								ganim[i].ang_init[1] = ganim[i].ang[1];
								ganim[i].ang_init[2] = ganim[i].ang[2];
								break;
							}
							do { i = fgetc(fil); } while ((!feof(fil)) && (i != 10));
						}
						fclose(fil);
					}
				}

				if ((ganim[ganimi].n <= 0) && (ganim[ganimi].loaddone))
				{
					point3d pp, rr, dd;
					pp.x = 0.0; rr.x = 20.0; dd.x =  0.0;
					pp.y = 0.0; rr.y =  0.0; dd.y = 60.0;
					pp.z = 0.0; rr.z =  0.0; dd.z =  0.0;
					voxie_printalph(&vf,&pp,&rr,&dd,0xffffff,"File not found");
					break;
				}
				if (!ganim[ganimi].loaddone)
				{
					char tbuf[MAX_PATH];

					sprintf(tbuf,ganim[ganimi].file,ganim[ganimi].n+1);
					if ((!ganim[ganimi].n) || (strchr(ganim[ganimi].file,'%')))
					{
						if (ganim[ganimi].n >= ganim[ganimi].mal)
						{
							ganim[ganimi].mal = max(ganim[ganimi].mal<<1,256);
							ganim[ganimi].tt = (tiletype *)realloc(ganim[ganimi].tt,ganim[ganimi].mal*sizeof(tiletype));
						}

						if ((ganim[ganimi].snd[0]) && (!ganim[ganimi].n)) voxie_playsound(ganim[ganimi].snd,-1,100,100,1.f);

						i = strlen(tbuf);
						if ((i >= 4) && (tbuf[i-4] == '.') && ((tbuf[i-3] == 'P') || (tbuf[i-3] == 'p')) &&
																		  ((tbuf[i-2] == 'N') || (tbuf[i-2] == 'n')) &&
																		  ((tbuf[i-1] == 'G') || (tbuf[i-1] == 'g'))) ganim[ganimi].filetyp = 0; else ganim[ganimi].filetyp = 1;
						if (ganim[ganimi].filetyp == 0)
						{
							kpzload(tbuf,      &ganim[ganimi].tt[ganim[ganimi].n].f,(int *)&ganim[ganimi].tt[ganim[ganimi].n].p,
											(int *)&ganim[ganimi].tt[ganim[ganimi].n].x,(int *)&ganim[ganimi].tt[ganim[ganimi].n].y);
							ganim[ganimi].tt[ganim[ganimi].n].p &= 0xffffffffI64;
							ganim[ganimi].tt[ganim[ganimi].n].x &= 0xffffffffI64;
							ganim[ganimi].tt[ganim[ganimi].n].y &= 0xffffffffI64;
						}
						else
						{
							ganim[ganimi].tt[ganim[ganimi].n].f = (INT_PTR)malloc(strlen(tbuf)+1);
							if (ganim[ganimi].tt[ganim[ganimi].n].f) strcpy((char *)ganim[ganimi].tt[ganim[ganimi].n].f,tbuf);

							if (kzopen((const char *)ganim[ganimi].tt[ganim[ganimi].n].f)) kzclose();
							else ganim[ganimi].tt[ganim[ganimi].n].f = 0;
						}
					}
					else
					{
						ganim[ganimi].tt[ganim[ganimi].n].f = 0;
					}

					if (ganim[ganimi].tt[ganim[ganimi].n].f) { ganim[ganimi].n++; }
									  else { ganim[ganimi].loaddone = 1; if (!ganim[ganimi].n) break; }
				}

				if (voxie_keystat(0x19) == 1) { gmodelanim_pauseit ^= 1; } //P
				if (((gmodelanim_pauseit) && ((voxie_keystat(0xc9) == 1) || (voxie_keystat(0x27) == 1) || ((vx[0].but&~ovxbut[0])&((1<<14)|(1<<8))))) || (ghitkey == 0xc8)) { ghitkey = 0; ganim[ganimi].cnt--; if (ganim[ganimi].cnt < 0) ganim[ganimi].cnt = ganim[ganimi].n-1; cycletim = tim+5.0; } //PGUP
				if (((gmodelanim_pauseit) && ((voxie_keystat(0xd1) == 1) || (voxie_keystat(0x28) == 1) || ((vx[0].but&~ovxbut[0])&((1<<13)|(1<<9))))) || (ghitkey == 0xd0)) { ghitkey = 0; ganim[ganimi].cnt++; if (ganim[ganimi].cnt >= ganim[ganimi].n) ganim[ganimi].cnt = 0;  cycletim = tim+5.0; } //PGDN

				if ((!gmodelanim_pauseit) && ((int)(tim*ganim[ganimi].fps) != (int)(otim*ganim[ganimi].fps)))
				{
					if (ganim[ganimi].mode == 0) //forward animation
					{
						ganim[ganimi].cnt++;
						if (ganim[ganimi].cnt >= ganim[ganimi].n)
						{
							ganim[ganimi].cnt = 0;
							if ((ganim[ganimi].snd[0]) && (strchr(ganim[ganimi].file,'%'))) voxie_playsound(ganim[ganimi].snd,0,100,100,1.f);
						}
					}
					else //pingpong animation
					{
						if (ganim[ganimi].mode > 0) { ganim[ganimi].cnt++; if (ganim[ganimi].cnt >= ganim[ganimi].n) { ganim[ganimi].cnt--; ganim[ganimi].mode *= -1; } }
													  else { ganim[ganimi].cnt--; if (ganim[ganimi].cnt <                0) { ganim[ganimi].cnt++; ganim[ganimi].mode *= -1; } }
					}
				}

				if (ganim[ganimi].filetyp != 0)
				{
					point3d fp, fr, fd, ff;
					char tbuf[MAX_PATH];

					if (!gslicemode)
					{
						if (voxie_keystat(keyremap[1][4]) || voxie_keystat(keyremap[1][5]) || voxie_keystat(0x1e) || voxie_keystat(0x2c) || (vx[0].lt|vx[0].rt) || (in.dmousz != 0.0) || (nav[0].but) || ((bstatus&3) == 3)) //ButA, ButB, XBox, dmousz, nav
						{
							f = 1.f;
							if (vx[0].lt) f *= pow(1.0+vx[0].lt/256.0,(double)dtim);
							if (vx[0].rt) f /= pow(1.0+vx[0].rt/256.0,(double)dtim);
							if (in.dmousz != 0.0) f *= pow(0.5,(double)in.dmousz*.0005);
							if ((bstatus&3) == 3) f *= pow(0.5,(double)in.dmousy*-.01);
							if (voxie_keystat(keyremap[1][5])|voxie_keystat(0x1e)|(nav[0].but&1)) f *= pow(2.0,(double)dtim); //ButA
							if (voxie_keystat(keyremap[1][4])|voxie_keystat(0x2c)|(nav[0].but&2)) f *= pow(0.5,(double)dtim); //ButB

							f *= ganim[ganimi].sc;

							fr.x = 1.f; fr.y = 0.f; fr.z = 0.f;
							fd.x = 0.f; fd.y = 1.f; fd.z = 0.f;
							ff.x = 0.f; ff.y = 0.f; ff.z = 1.f;
							rotate_vex(ganim[ganimi].ang[0],&fr,&fd);
							rotate_vex(ganim[ganimi].ang[1],&fd,&ff);
							rotate_vex(ganim[ganimi].ang[2],&ff,&fr);

							g = f/ganim[ganimi].sc; ganim[ganimi].p.x *= g; ganim[ganimi].p.y *= g; ganim[ganimi].p.z *= g;

							ganim[ganimi].sc = f;
						}

						f = dtim*.5f;
						ganim[ganimi].p.x += vx[0].tx0*dtim*+.00002f + nav[0].dx*dtim*.005f;
						ganim[ganimi].p.y += vx[0].ty0*dtim*-.00002f + nav[0].dy*dtim*.005f;
						ganim[ganimi].p.z += (((vx[0].but>>0)&1)-((vx[0].but>>1)&1))*dtim*-.5f + nav[0].dz*dtim*.0025f;
						//if (voxie_keystat(0x2a)) { f *= 1.f/4.f; }
						//if (voxie_keystat(0x36)) { f *= 4.f/1.f; }
						if (voxie_keystat(keyremap[0][0])) { ganim[ganimi].p.x -= f; } //Left
						if (voxie_keystat(keyremap[0][1])) { ganim[ganimi].p.x += f; } //Right
						if (voxie_keystat(keyremap[0][2])) { ganim[ganimi].p.y -= f; } //Up
						if (voxie_keystat(keyremap[0][3])) { ganim[ganimi].p.y += f; } //Down
						if (voxie_keystat(keyremap[0][4])) { ganim[ganimi].p.z += f; } //ButA
						if (voxie_keystat(keyremap[0][5])) { ganim[ganimi].p.z -= f; } //ButB

						if (!(bstatus&3))
						{
							ganim[ganimi].ang[0] += in.dmousx*+.01;
							ganim[ganimi].ang[1] += in.dmousy*+.01;
							//if (gautorotateax < 0) ganim[ganimi].ang[1] = min(max(ganim[ganimi].ang[1],PI*-.5),PI*.5);
							if (!(gautorotatespd[0]|gautorotatespd[1]|gautorotatespd[2])) ganim[ganimi].ang[1] = min(max(ganim[ganimi].ang[1],PI*-.5),PI*.5);
						}
						ganim[ganimi].ang[0] += vx[0].tx1*dtim*.0001f;
						ganim[ganimi].ang[1] += vx[0].ty1*dtim*.0001f;
						fr.x = 1.f; fr.y = 0.f; fr.z = 0.f;
						fd.x = 0.f; fd.y = 1.f; fd.z = 0.f;
						ff.x = 0.f; ff.y = 0.f; ff.z = 1.f;
						rotate_vex(ganim[ganimi].ang[0],&fr,&fd);
						rotate_vex(ganim[ganimi].ang[1],&fd,&ff);
						rotate_vex(ganim[ganimi].ang[2],&ff,&fr);

							//Hacks for nice matrix movement with Space Navigator
						f = fr.y; fr.y = fd.x; fd.x = f;
						f = fr.z; fr.z = ff.x; ff.x = f;
						f = fd.z; fd.z = ff.y; ff.y = f;

							//fr.x*fx + fd.x*fy + ff.x*fz = -ganim[ganimi].p.x
							//fr.y*fx + fd.y*fy + ff.y*fz = -ganim[ganimi].p.y
							//fr.z*fx + fd.z*fy + ff.z*fz = -ganim[ganimi].p.z
						fx = (ganim[ganimi].p.x*fr.x + ganim[ganimi].p.y*fd.x + ganim[ganimi].p.z*ff.x); //rotate around center of volume (backup)
						fy = (ganim[ganimi].p.x*fr.y + ganim[ganimi].p.y*fd.y + ganim[ganimi].p.z*ff.y);
						fz = (ganim[ganimi].p.x*fr.z + ganim[ganimi].p.y*fd.z + ganim[ganimi].p.z*ff.z);
						//rotate_vex(nav[0].az*dtim*-.005f + in.dmousx*+.01*(float)((bstatus&3)>=2) + (gautorotateax==2)*(gautorotatespd!=0)*pow(2.0,gautorotatespd-7.0)*dtim,&fr,&fd);
						//rotate_vex(nav[0].ay*dtim*-.005f - in.dmousy*+.01*(float)((bstatus&3)==1) + (gautorotateax==0)*(gautorotatespd!=0)*pow(2.0,gautorotatespd-7.0)*dtim,&fd,&ff);
						//rotate_vex(nav[0].ax*dtim*+.005f + in.dmousx*+.01*(float)((bstatus&3)==1) + (gautorotateax==1)*(gautorotatespd!=0)*pow(2.0,gautorotatespd-7.0)*dtim,&ff,&fr);
						rotate_vex(nav[0].az*dtim*-.005f + in.dmousx*+.01*(float)((bstatus&3)>=2) + (gautorotatespd[2]!=0)*pow(2.0,gautorotatespd[2]-7.0)*dtim,&fr,&fd);
						rotate_vex(nav[0].ay*dtim*-.005f - in.dmousy*+.01*(float)((bstatus&3)==1) + (gautorotatespd[0]!=0)*pow(2.0,gautorotatespd[0]-7.0)*dtim,&fd,&ff);
						rotate_vex(nav[0].ax*dtim*+.005f + in.dmousx*+.01*(float)((bstatus&3)==1) + (gautorotatespd[1]!=0)*pow(2.0,gautorotatespd[1]-7.0)*dtim,&ff,&fr);
						ganim[ganimi].p.x = (fr.x*fx + fr.y*fy + fr.z*fz); //rotate around center of volume instead of center of object
						ganim[ganimi].p.y = (fd.x*fx + fd.y*fy + fd.z*fz);
						ganim[ganimi].p.z = (ff.x*fx + ff.y*fy + ff.z*fz);

						f = fr.y; fr.y = fd.x; fd.x = f;
						f = fr.z; fr.z = ff.x; ff.x = f;
						f = fd.z; fd.z = ff.y; ff.y = f;
							//See MAT2ANG.KC for derivation
						ganim[ganimi].ang[0] = atan2(-fd.x,fd.y);
						ganim[ganimi].ang[1] = atan2(fd.z,sqrt(fd.x*fd.x + fd.y*fd.y));
						ganim[ganimi].ang[2] = atan2(-fr.z,ff.z);

						fr.x *= ganim[ganimi].sc; fr.y *= ganim[ganimi].sc; fr.z *= ganim[ganimi].sc;
						fd.x *= ganim[ganimi].sc; fd.y *= ganim[ganimi].sc; fd.z *= ganim[ganimi].sc;
						ff.x *= ganim[ganimi].sc; ff.y *= ganim[ganimi].sc; ff.z *= ganim[ganimi].sc;

#if (USELEAP)
						frame = leap_getframe();
						for(i=min(frame->nHands,1)-1;i>=0;i--)
						{
							hand = &frame->pHands[i];
							//if (hand->type == eLeapHandType_Left) ..

							palm = &hand->palm;
							memcpy(&palmp,&palm->position ,sizeof(point3d));
							memcpy(&palmd,&palm->normal   ,sizeof(point3d));
							memcpy(&palmf,&palm->direction,sizeof(point3d));
							palmr.x = palmd.y*palmf.z - palmd.z*palmf.y;
							palmr.y = palmd.z*palmf.x - palmd.x*palmf.z;
							palmr.z = palmd.x*palmf.y - palmd.y*palmf.x;
							//use palmp/r/d/f

							ganim[ganimi].p.x = palmp.x*+.01f;
							ganim[ganimi].p.y = palmp.z*+.01f;
							ganim[ganimi].sc = exp(palmp.y*.01f)*.125f;

							fr.x =-palmr.x*ganim[ganimi].sc; fr.y =-palmr.z*ganim[ganimi].sc; fr.z =+palmr.y*ganim[ganimi].sc;
							fd.x =-palmd.x*ganim[ganimi].sc; fd.y =-palmd.z*ganim[ganimi].sc; fd.z =+palmd.y*ganim[ganimi].sc;
							ff.x =-palmf.x*ganim[ganimi].sc; ff.y =-palmf.z*ganim[ganimi].sc; ff.z =+palmf.y*ganim[ganimi].sc;
							rotate_vex(PI*.5f,&fd,&ff);
						}
#endif

						if ((voxie_keystat(0x35)) || (ghitkey == 0x0e)) // /
						{
							ghitkey = 0;
							ganim[ganimi].ang[0] = ganim[ganimi].ang_init[0];
							ganim[ganimi].ang[1] = ganim[ganimi].ang_init[1];
							ganim[ganimi].ang[2] = ganim[ganimi].ang_init[2];
							ganim[ganimi].sc = ganim[ganimi].defscale;
							ganim[ganimi].p.x = ganim[ganimi].p_init.x;
							ganim[ganimi].p.y = ganim[ganimi].p_init.y;
							ganim[ganimi].p.z = ganim[ganimi].p_init.z;
						}
					}
					else
					{
						if (voxie_keystat(keyremap[1][4]) || voxie_keystat(keyremap[1][5]) || (vx[0].lt|vx[0].rt) || (in.dmousz != 0.0) || (nav[0].but) || ((bstatus&3) == 3)) //ButA, ButB, XBox, dmousz, nav
						{
							f = 1.f;
							if (vx[0].lt) f *= pow(1.0+vx[0].lt/256.0,(double)dtim);
							if (vx[0].rt) f /= pow(1.0+vx[0].rt/256.0,(double)dtim);
							if (in.dmousz != 0.0) f *= pow(0.5,(double)in.dmousz*.0005);
							if ((bstatus&3) == 3) f *= pow(0.5,(double)in.dmousy*-.01);
							if (voxie_keystat(keyremap[1][5])|(nav[0].but&1)) f *= pow(2.0,(double)dtim); //ButA
							if (voxie_keystat(keyremap[1][4])|(nav[0].but&2)) f *= pow(0.5,(double)dtim); //ButB

							gslicer.x *= f; gslicer.y *= f; gslicer.z *= f;
							gsliced.x *= f; gsliced.y *= f; gsliced.z *= f;
							gslicef.x *= f; gslicef.y *= f; gslicef.z *= f;
						}

						f = dtim*.5f;
						gslicep.x += vx[0].tx0*dtim*+.00002f + nav[0].dx*dtim*.005f;
						gslicep.y += vx[0].ty0*dtim*-.00002f + nav[0].dy*dtim*.005f;
						gslicep.z += (((vx[0].but>>0)&1)-((vx[0].but>>1)&1))*dtim*-.5f + nav[0].dz*dtim*.0025f;
						//if (voxie_keystat(0x2a)) { f *= 1.f/4.f; }
						//if (voxie_keystat(0x36)) { f *= 4.f/1.f; }
						if (voxie_keystat(keyremap[0][0])) { gslicep.x -= f; } //Left
						if (voxie_keystat(keyremap[0][1])) { gslicep.x += f; } //Right
						if (voxie_keystat(keyremap[0][2])) { gslicep.y -= f; } //Up
						if (voxie_keystat(keyremap[0][3])) { gslicep.y += f; } //Down
						if (voxie_keystat(keyremap[0][4])) { gslicep.z += f; } //ButA
						if (voxie_keystat(keyremap[0][5])) { gslicep.z -= f; } //ButB

						f = gslicer.y; gslicer.y = gsliced.x; gsliced.x = f;
						f = gslicer.z; gslicer.z = gslicef.x; gslicef.x = f;
						f = gsliced.z; gsliced.z = gslicef.y; gslicef.y = f;
						rotate_vex(nav[0].az*dtim*-.005f + in.dmousx*+.01*(float)((bstatus&3)>=2),&gslicer,&gsliced);
						rotate_vex(nav[0].ay*dtim*-.005f - in.dmousy*+.01*(float)((bstatus&3)==1),&gsliced,&gslicef);
						rotate_vex(nav[0].ax*dtim*+.005f + in.dmousx*+.01*(float)((bstatus&3)==1),&gslicef,&gslicer);
						f = gslicer.y; gslicer.y = gsliced.x; gsliced.x = f;
						f = gslicer.z; gslicer.z = gslicef.x; gslicef.x = f;
						f = gsliced.z; gsliced.z = gslicef.y; gslicef.y = f;

							//Keep slice near origin by subtracting dot of unused axes - works great!
						f = (gslicer.x*gslicep.x + gslicer.y*gslicep.y + gslicer.z*gslicep.z)
						  / (gslicer.x*gslicer.x + gslicer.y*gslicer.y + gslicer.z*gslicer.z); gslicep.x -= gslicer.x*f; gslicep.y -= gslicer.y*f; gslicep.z -= gslicer.z*f;
						f = (gslicef.x*gslicep.x + gslicef.y*gslicep.y + gslicef.z*gslicep.z)
						  / (gslicef.x*gslicef.x + gslicef.y*gslicef.y + gslicef.z*gslicef.z); gslicep.x -= gslicef.x*f; gslicep.y -= gslicef.y*f; gslicep.z -= gslicef.z*f;

						if ((voxie_keystat(0x35)) || (ghitkey == 0x0e)) // /
						{
							ghitkey = 0;
							gslicep.x = 0.f; gslicer.x = .05f; gsliced.x = 0.f; gslicef.x = 0.f;
							gslicep.y = 0.f; gslicer.y = 0.f; gsliced.y = .05f; gslicef.y = 0.f;
							gslicep.z = 0.f; gslicer.z = 0.f; gsliced.z = 0.f; gslicef.z = .05f;
						}
					}

					f = 1.f/min(min(vw.aspx,vw.aspy),vw.aspz/.2f);
					voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f);

					if (gslicemode) { voxie_setmaskplane(&vf, gslicep.x,gslicep.y,gslicep.z,gsliced.x,gsliced.y,gsliced.z); }

					if (!voxie_drawspr(&vf,(const char *)ganim[ganimi].tt[ganim[ganimi].cnt].f,&ganim[ganimi].p,&fr,&fd,&ff,colval))
					{
						point3d pp, rr, dd;
						pp.x = 0.0; rr.x = 20.0; dd.x =   0.0;
						pp.y = 0.0; rr.y =  0.0; dd.y =  60.0;
						pp.z = 0.0; rr.z =  0.0; dd.z =   0.0;
						voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords
						voxie_printalph(&vf,&pp,&rr,&dd,0xffffff,"File not found");
						voxie_setview(&vf,-vw.aspx*f,-vw.aspy*f,-vw.aspz*f,+vw.aspx*f,+vw.aspy*f,+vw.aspz*f);
					}

					if ((bstatus&4) || (gdrawstats))
					{
						fp.x = ganim[ganimi].p.x - (fr.x + fd.x + ff.x)*.5f;
						fp.y = ganim[ganimi].p.y - (fr.y + fd.y + ff.y)*.5f;
						fp.z = ganim[ganimi].p.z - (fr.z + fd.z + ff.z)*.5f;
						voxie_drawcube(&vf,&fp,&fr,&fd,&ff,1,0xffffff);
					}

						//Don't allow model to fall out of view too far (bad for Dejarik animation, which is not centered well)
					//g = (-vw.aspx*f) - (ganim[ganimi].p.x + (fabs(fr.x) + fabs(fd.x) + fabs(ff.x))*.5f); if (g > 0.f) ganim[ganimi].p.x += g; //(xmin view) - (xmax model)
					//g = (+vw.aspx*f) - (ganim[ganimi].p.x - (fabs(fr.x) + fabs(fd.x) + fabs(ff.x))*.5f); if (g < 0.f) ganim[ganimi].p.x += g; //(xmax view) - (xmin model)
					//g = (-vw.aspy*f) - (ganim[ganimi].p.y + (fabs(fr.y) + fabs(fd.y) + fabs(ff.y))*.5f); if (g > 0.f) ganim[ganimi].p.y += g; //(ymin view) - (ymax model)
					//g = (+vw.aspy*f) - (ganim[ganimi].p.y - (fabs(fr.y) + fabs(fd.y) + fabs(ff.y))*.5f); if (g < 0.f) ganim[ganimi].p.y += g; //(ymax view) - (ymin model)
					//g = (-vw.aspz*f) - (ganim[ganimi].p.z + (fabs(fr.z) + fabs(fd.z) + fabs(ff.z))*.5f); if (g > 0.f) ganim[ganimi].p.z += g; //(zmin view) - (zmax model)
					//g = (+vw.aspz*f) - (ganim[ganimi].p.z - (fabs(fr.z) + fabs(fd.z) + fabs(ff.z))*.5f); if (g < 0.f) ganim[ganimi].p.z += g; //(zmax view) - (zmin model)
				}
				//else
				//{
				//   INT_PTR qwptr, qrptr;
				//   if ((vf.x >  gxres) || (vf.y > gyres)) { xx =  vf.x; yy =  vf.y; qwptr = vf.f; }
				//                                     else { xx = gxres; yy = gyres; qwptr = vf.f; }
				//   qrptr = ganim[ganimi].tt[ganim[ganimi].cnt].f;
				//
				//   j = vw.framepervol;
				//   xsiz = ganim[ganimi].tt[ganim[ganimi].cnt].x/j;
				//   for(y=0;y<yy;y++,qwptr+=vf.p,qrptr+=ganim[ganimi].tt[ganim[ganimi].cnt].p) //z_arccos to z_arccos (direct copy)
				//      for(i=0;i<j;i++)
				//         memcpy((void *)(vf.fp*i + qwptr),(void *)(xsiz*i*4 + qrptr),xx<<2);
				//}

				if (((voxie_keystat(0xb8)) && (ganim[ganimi].n > 0)) || (gdrawstats)) //R.Alt
				{
					char tbuf[MAX_PATH];

					point3d pp, rr, dd;
					voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);

					rr.x = 0.125f; dd.x = 0.00f; pp.x = vw.aspx*.1f;
					rr.y = 0.00f;  dd.y = 0.25f; pp.y = vw.aspy*-.99f;
					rr.z = 0.00f;  dd.z = 0.00f; pp.z = -vw.aspz+0.01f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%3d/%3d",ganim[ganimi].cnt+1,ganim[ganimi].n);

					sprintf(tbuf,ganim[ganimi].file,ganim[ganimi].cnt);
					i = strlen(tbuf);
					rr.x = vw.aspx*2.f/((float)(max(i,16)+2)); rr.y = 0.0f; rr.z = 0.0f;
					dd.x = 0.0f; dd.y = rr.x*1.5f; dd.z = 0.0f;
					pp.x = -vw.aspx + rr.x*(float)(max(i,16)+2 - i)*.5f; pp.y = vw.aspy*+.99-dd.y; pp.z = vw.aspz*-.99f;
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%s",tbuf);
				}

				}
				break;
		}

#if (USEMAG6D)
		if (gusemag6d > 0)
		{
				//Draw mag6d axes
			voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
			f = .25f;
			voxie_drawcone(&vf,m6p.x,m6p.y,m6p.z,.02f,m6p.x+m6r.x*f,m6p.y+m6r.y*f,m6p.z+m6r.z*f,.02f,1,0xff0000);
			voxie_drawcone(&vf,m6p.x,m6p.y,m6p.z,.02f,m6p.x+m6d.x*f,m6p.y+m6d.y*f,m6p.z+m6d.z*f,.02f,1,0x00ff00);
			voxie_drawcone(&vf,m6p.x,m6p.y,m6p.z,.02f,m6p.x+m6f.x*f,m6p.y+m6f.y*f,m6p.z+m6f.z*f,.02f,1,0x0000ff);
			i = 0xffffff;
			if (m6but) i = (m6but&1)*0xff0000 + ((m6but>>1)&1)*0x00ff00 + ((m6but>>2)&1)*0x0000ff;
			voxie_drawsph(&vf,m6p.x,m6p.y,m6p.z,.03f,1,i);
		}
#endif

		voxie_setview(&vf,0.0,0.0,-256,gxres,gyres,+256); //old coords

		if ((gshowborder) && ((grendmode == RENDMODE_PLATONICSOLIDS) || (grendmode == RENDMODE_FLYINGSTUFF) || (grendmode == RENDMODE_DOTMUNCH) || (grendmode == RENDMODE_SNAKETRON) || (grendmode == RENDMODE_MODELANIM)))
		{
			voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
			if (!vw.clipshape)
			{
					//draw wireframe box
				voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
			}
			else
			{
				n = 64;
				for(j=-64;j<=64;j++)
				{
					if (j == -62) j = 63;
					for(i=0;i<n;i++)
					{
						voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f,
												cos((float)(i+1)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+1)*PI*2.0/(float)n)*vw.aspr, (float)j*vw.aspz/64.f, 0xffffff);
					}
				}

				n = 32;
				for(i=0;i<n;i++)
				{
					voxie_drawlin(&vf,cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, -vw.aspz,
											cos((float)(i+0)*PI*2.0/(float)n)*vw.aspr, sin((float)(i+0)*PI*2.0/(float)n)*vw.aspr, +vw.aspz, 0xffffff);
				}
			}
		}

		avgdtim += (dtim-avgdtim)*.1;
		if (voxie_keystat(0xb8) || (showphase)) //R.Alt
		{
			point3d pp, rr, dd;
			voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
			rr.x = 0.125f; dd.x = 0.00f; pp.x = vw.aspx*-.99f;
			rr.y = 0.00f;  dd.y = 0.25f; pp.y = vw.aspy*-.99f;
			rr.z = 0.00f;  dd.z = 0.00f; pp.z = vw.aspz-0.01f;
								voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%8.5f  %.4f",vw.freq,vw.phase);
			pp.z *= -1.f;  voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%.1f",1.0/avgdtim);
		}

		voxie_frame_end(); voxie_getvw(&vw);
		numframes++;
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
#if (USEMAG6D)
	if (gusemag6d >= 0) mag6d_uninit();
#endif
#if (USELEAP)
	leap_uninit();
#endif

	return(0);
}

#if 0
!endif
#endif
