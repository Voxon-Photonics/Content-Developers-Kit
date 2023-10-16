#pragma pack(push,1)
typedef struct { double x, y, z; } dpoint3d;
typedef struct { float fzmin, fzmax, fhakzadd; double fzsum, fzsum2; int fzcnt; } drawglobestats_t; //units in voxie z coordinates (typically: -.5 .. +.5)
#pragma pack(pop)
#define EARTHRADMET 6371e3 //avg

#if (USEDEMVIEWDLL == 0)

extern void demview_init (HINSTANCE hvoxie);
extern void demview_uninit (void);

	//Parameters to support most web-based tile servers. Parameters may be copied from the examples near the top of DEMVIEW.INI.
	//        sind: 0=heightmap, 1=color
	//    tilezmin: outermost tile level: almost always 0. (0: 1 tile covers entire globe (except for a small section near the poles), 1: 2x2 grid, 2: 4x4 grid, ..)
	//    tilezmax: innermost tile level: typically:{15..21}. (ex: 15 means 32768x32768 grid is finest level)
	// log2tilesiz: tile resolution: usually 8 for 256x256 tiles; have seen 9 (512x512)
	//   iswebmerc: 1 if uses Web Mercator projection, 0 if not. Almost always 1. If height & color don't match, try the other!
	//     ishttps: tile server prefix: 0=http://, 1=https://
	//  isquadtree: controls how tile names are generated. 0=separate z,x,y, 1=quadtree (single base 4 number; used by VirtualEarth)
	//     servnum: number of port connections to server. typical range: 1..8. Higher may load faster - please check whether server supports it!
	//servcharoffs: offset to char in url_st[] to select other servers (used when servnum > 1).
	//                 Example: "serv-a.place.com", "serv-b.place.com" would use: servnum=2, servcharoffs=5 (where the 'a' is)
	//    cache_st: printf-formatted string of how to name cached tiles. Must contain exactly 4 %d's for: tilesiz,z,x,y
	//                 Other chars can be whatever, but should be unique to identify which server, or height vs color.
	//      url_st: base server address (i.e. somesite.com)
	//     tile_st: printf-formatted string appended to server name to generate tile names (quadtree==0: must have 3 %d's for z,x,y; quadtree==1: must have 1 %d)
	//  {r/g/b}mul: color scaling. No scale: 1.f (for sind==1 only)
extern int demview_setserv (int sind, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char *cache_st, char *url_st, char *tile_st, float rmul, float gmul, float bmul);

	//     dpos: cartesian coordinate of camera (0,0,0) is center of globe, radius always 1 (assumes perfect sphere).
	//drig,ddow,dfor: unit orthonormal rotation matrix of camera
	//   fexagz: height exaggeration (1.0 is real life)
	//resthresh: for internal use; pass 256
	//    flags: |1:crop ceiling to top, |2:crop floor to bottom
	//      dgs: gives statistics on height during rendering. Use this to automatically adjust height for the next frame.
	//              This is necessary for keeping the data centered in view. Unfortunately, there is no perfect solution. :/
extern float demview_drawglobe (voxie_wind_t *vw, voxie_frame_t *vf, dpoint3d *dpos, dpoint3d *drig, dpoint3d *ddow, dpoint3d *dfor, float fexagz, float resthresh, int flags, drawglobestats_t *dgs);

	//filnam: sprite filename
	//lon_deg,lat_deg: yup
	//          alt_m: altitude above sea level in meters
	//        yaw_deg: rotation of sprite
	//      pitch_deg: rotation of sprite
	//       roll_deg: rotation of sprite
	//         fscale: scale in earth radii? start with small numbers like .00005f
	//     fexagz_pos: height exaggeration for position
	//     fexagz_sca: height exaggeration for scale
	//            col: color scale
extern void demview_drawspr (voxie_frame_t *vf, const char *filnam, double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, int col);

	//Load a height palette (color scale based on height)
	//pal: 1D array of 24-bit colors. Set to 0 to keep old palette (handy for modifying other vars: altitudes / transition ratio).
	//paln: number of palette entries (1024 typical). Set to 0 to disable. Set to -1 to keep old palette size (handy for modifying other vars: altitudes / transition ratio).
	//met0: altitude in meters at pal[0]
	//met1: altitude in meters at pal[paln-1]
	//trans: transition ratio from original tile server color to height palette. Range:{0.f..1.f}
extern void demview_setheipal (int *pal, int paln, float met0, float met1, float trans);

	//lon,lat: longitude & latitude in degrees
	//altradii: approximate altitude above sea level in globe (usually Earth) radii. (In reality this is a scale factor; there is no actual exact altitude for various reasons)
	//   Why not meters? Answer: to support other planets without library needing to know object's size.
	//   Examples: sea level:0, 1000m above sea level: 1000/EARTHRADMET = 1.57e-4, ISS (LEO):340e3/EARTHRADMET = 0.0534, Geostationary orbit:35786/EARTHRADMET = 5.617
	//dpos: cartesian coordinate
	//drig,ddow,dfor: unit orthonormal (bonus)
extern void demview_sph2xyz (double lon_deg, double lat_deg, double altradii, dpoint3d *dpos, dpoint3d *drig, dpoint3d *ddow, dpoint3d *dfor);
extern void demview_xyz2sph (dpoint3d *dpos, double *lon_deg, double *lat_deg, double *altradii);
extern void demview_xyz2voxie (dpoint3d *dpos, point3d *voxiepos);
extern void demview_sph2voxie (double lon_deg, double lat_deg, double altradii, point3d *voxiepos);
extern void demview_sph2voxie2 (double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, point3d *voxiepos, point3d *pr, point3d *pd, point3d *pf);
extern void demview_voxie2xyz (point3d *voxie, dpoint3d *dpos);
extern void demview_voxie2sph (point3d *voxie, double *lon_deg, double *lat_deg, double *altradii);

#else //if (USEDEMVIEWDLL != 0)

static HINSTANCE hdemview = (HINSTANCE)INVALID_HANDLE_VALUE;

#ifdef  __cplusplus
extern "C" {
#endif

void  (__cdecl *demview_init_int  )(HINSTANCE hvoxie);
void  (__cdecl *demview_uninit_int)(void);
int   (__cdecl *demview_setserv   )(int sind, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char *cache_st, char *url_st, char *tile_st, float rmul, float gmul, float bmul);
float (__cdecl *demview_drawglobe )(voxie_wind_t *vw, voxie_frame_t *vf, dpoint3d *dpos, dpoint3d *drig, dpoint3d *ddow, dpoint3d *dfor, float fexagz, float resthresh, int flags, drawglobestats_t *dgs);
void  (__cdecl *demview_drawspr   )(voxie_frame_t *vf, const char *filnam, double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, int col);
void  (__cdecl *demview_setheipal )(int *pal, int paln, float met0, float met1, float trans);
void  (__cdecl *demview_sph2xyz   )(double lon_deg, double lat_deg, double altradii, dpoint3d *dpos, dpoint3d *drig, dpoint3d *ddow, dpoint3d *dfor);
void  (__cdecl *demview_xyz2sph   )(dpoint3d *dpos, double *lon_deg, double *lat_deg, double *altradii);
void  (__cdecl *demview_xyz2voxie )(dpoint3d *dpos, point3d *voxiepos);
void  (__cdecl *demview_sph2voxie )(double lon_deg, double lat_deg, double altradii, point3d *voxiepos);
void  (__cdecl *demview_sph2voxie2)(double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, point3d *voxiepos, point3d *pr, point3d *pd, point3d *pf);
void  (__cdecl *demview_voxie2xyz )(point3d *voxie, dpoint3d *dpos);
void  (__cdecl *demview_voxie2sph )(point3d *voxie, double *lon_deg, double *lat_deg, double *altradii);

#ifdef __cplusplus
}
#endif

static void demview_init (HINSTANCE hvoxie)
{
	hdemview = LoadLibrary("demview.dll");
	if (!hdemview) { MessageBox(0,"Fail to load DEMVIEW.DLL","DEMVIEW.H",MB_OK); ExitProcess(0); }

	demview_init_int   = (void  (__cdecl *)(HINSTANCE hvoxie))GetProcAddress(hdemview,"demview_init");
	demview_uninit_int = (void  (__cdecl *)(void))GetProcAddress(hdemview,"demview_uninit");
	demview_setserv    = (int   (__cdecl *)(int, int, int, int, int, int, int, int, int, char *, char *, char *, float, float, float))GetProcAddress(hdemview,"demview_setserv");
	demview_drawglobe  = (float (__cdecl *)(voxie_wind_t *, voxie_frame_t *, dpoint3d *, dpoint3d *, dpoint3d *, dpoint3d *, float, float, int, drawglobestats_t *))GetProcAddress(hdemview,"demview_drawglobe");
	demview_drawspr    = (void  (__cdecl *)(voxie_frame_t *, const char *, double, double, double, double, double, double, double, double, double, int))GetProcAddress(hdemview,"demview_drawspr");
	demview_setheipal  = (void  (__cdecl *)(int *, int, float, float, float))GetProcAddress(hdemview,"demview_setheipal");
	demview_sph2xyz    = (void  (__cdecl *)(double, double, double, dpoint3d *, dpoint3d *, dpoint3d *, dpoint3d *))GetProcAddress(hdemview,"demview_sph2xyz");
	demview_xyz2sph    = (void  (__cdecl *)(dpoint3d *, double *, double *, double *))GetProcAddress(hdemview,"demview_xyz2sph");
	demview_xyz2voxie  = (void  (__cdecl *)(dpoint3d *, point3d *))GetProcAddress(hdemview,"demview_xyz2voxie");
	demview_sph2voxie  = (void  (__cdecl *)(double, double, double, point3d *))GetProcAddress(hdemview,"demview_sph2voxie");
	demview_sph2voxie2 = (void  (__cdecl *)(double, double, double, double, double, double, double, double, double, point3d *, point3d *, point3d *, point3d *))GetProcAddress(hdemview,"demview_sph2voxie2");
	demview_voxie2xyz  = (void  (__cdecl *)(point3d *, dpoint3d *))GetProcAddress(hdemview,"demview_voxie2xyz");
	demview_voxie2sph  = (void  (__cdecl *)(point3d *, double *, double *, double *))GetProcAddress(hdemview,"demview_voxie2sph");

	demview_init_int(hvoxie);
}

static void demview_uninit (void)
{
	demview_uninit_int();
	if (hdemview != (HINSTANCE)INVALID_HANDLE_VALUE) { FreeLibrary(hdemview); hdemview = 0; }
}

#endif

static void demview_settopbotaltm (dpoint3d *dpos, voxie_wind_t *vw, double dtopm, double dbotm, float *fexagz, float *fhakzadd)
{
	double d, daltm;
	daltm = (sqrt(dpos->x*dpos->x + dpos->y*dpos->y + dpos->z*dpos->z) - 1.0)*EARTHRADMET;
	d = vw->aspz*2.0 / (dtopm-dbotm);
	(*fexagz) = daltm*d;
	(*fhakzadd) = dbotm*d + vw->aspz;
	if (vw->nblades >= 2) { (*fexagz) *= 0.9325; (*fhakzadd) -= 1.0625f; } //these are hack constants for spinner!
						  else { (*fexagz) *= 1.15; (*fhakzadd) -= 0.87f; } //these are these hack constants for VX1!
}
static void demview_gettopbotaltm (dpoint3d *dpos, voxie_wind_t *vw, double *dtopm, double *dbotm, float fexagz, float fhakzadd)
{
	double d, daltm;
	daltm = (sqrt(dpos->x*dpos->x + dpos->y*dpos->y + dpos->z*dpos->z) - 1.0)*EARTHRADMET;
	if (vw->nblades >= 2) { fexagz /= 0.9325; fhakzadd += 1.0625f; } // these hack constants for spinner
						  else { fexagz /= 1.15; fhakzadd += 0.87f; } // these are these hack constants? for VX1!
	d = daltm/fexagz;
	(*dbotm) = (fhakzadd - vw->aspz)*d;
	(*dtopm) = vw->aspz*d*2.0 + (*dbotm);
}
