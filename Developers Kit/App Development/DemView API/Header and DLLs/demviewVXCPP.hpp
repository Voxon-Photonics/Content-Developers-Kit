#include "vxCpp.h"
#pragma pack(push,1)
typedef struct { double x, y, z; } dpoint3d;
typedef struct { float fzmin, fzmax, fhakzadd; double fzsum, fzsum2; int fzcnt; } drawglobestats_t; //units in voxie z coordinates (typically: -.5 .. +.5)
#pragma pack(pop)

#define EARTHRADMET 6371e3 //avg radius of the planet earth change this if you are ever working with another planet

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
//#endif

static void rotDPvex(double ang, dpoint3d* a, dpoint3d* b) //Rotate dpoint vectors a&b around common plane, by ang
{
	double c, s, f;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f * c + b->x * s; b->x = b->x * c - f * s;
	f = a->y; a->y = f * c + b->y * s; b->y = b->y * c - f * s;
	f = a->z; a->z = f * c + b->z * s; b->z = b->z * c - f * s;
}



static void demview_init (HINSTANCE hvoxie)
{
	hdemview = LoadLibrary("demview.dll");
	if (!hdemview) { MessageBox(0,"Fail to load DEMVIEW.DLL","DEMVIEWVXCPP.HPP",MB_OK); ExitProcess(0); }

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
	if (hdemview != (HINSTANCE)INVALID_HANDLE_VALUE) { 
			FreeLibrary(hdemview);
			hdemview = 0;
	}
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
