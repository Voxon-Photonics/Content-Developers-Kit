#if 0

!if 0

demsimp.exe: demsimp.obj demview.obj demview_64.obj kcpulib.obj
	link      demsimp.obj demview.obj demview_64.obj kcpulib.obj user32.lib gdi32.lib
	del demsimp.obj
demsimp.obj   : demsimp.c voxiebox.h; cl /c /TP demsimp.c /Ox /GFy /MT
demview.obj   : demview.c voxiebox.h; cl /c /TP demview.c /Ox /GFy /MT /DVOXIEBOX
demview_64.obj: demview_64.asm      ; ml64 /c demview_64.asm
kcpulib.obj   : kcpulib.c           ; cl /c /TP kcpulib.c /Ox /GFy /MT

!else

demsimp.exe: demsimp.c voxiebox.h; cl /TP demsimp.c /Ox /GFy /MT /DUSEDEMVIEWDLL=1 /link user32.lib gdi32.lib
	del demsimp.obj

!endif

!if 0
#endif

#if (USEDEMVIEWDLL == 0)
#define EXTRN extern //hack for voxiebox.h to prevent duplicate voxie definitions
#endif
#include "voxiebox.h"
#include <intrin.h>
#include <stdlib.h>
#include <math.h>
#include "demview.h"
#define PI 3.14159265358979323

static void rotvex (double ang, dpoint3d *a, dpoint3d *b) //Rotate vectors a&b around common plane, by ang
{
	double c, s, f;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

static void drawgrid (voxie_frame_t *vf, float lon, float lat, float alt_m, float altradii, float fexagz, int col, int showlonlat)
{
	#define GRDSIZ 12
	static point3d grd[GRDSIZ][GRDSIZ];
	point3d pp, pr, pd;
	float f, g, rf, fcx, fcy;
	int i, x, y;
	char tbuf[256];

	f = altradii*32.f;
	i = *(int *)&f; i &= 0x7f800000; f = *(float *)&i; //quantize to power of 2
	if (f > 16.f) f = 16.f;
	rf = 1.f/f;
	fcx = ceil(lon*rf)*f;
	fcy = ceil(lat*rf)*f;
	if (((float)     0-(float)GRDSIZ*.5f)*f+fcy < -90.f) fcy = -90.f - ((float)     0-(float)GRDSIZ*.5f)*f;
	if (((float)GRDSIZ-(float)GRDSIZ*.5f)*f+fcy > +90.f) fcy = +90.f - ((float)GRDSIZ-(float)GRDSIZ*.5f)*f;

	for(y=0;y<GRDSIZ;y++)
		for(x=0;x<GRDSIZ;x++)
		{
			demview_sph2voxie(((float)x-(float)GRDSIZ*.5f)*f+fcx,
									((float)y-(float)GRDSIZ*.5f)*f+fcy,alt_m*fexagz*(1.0/EARTHRADMET),&grd[y][x]);
		}
	for(y=0;y<GRDSIZ;y++)
	{
		for(x=0;x<GRDSIZ-1;x++) voxie_drawlin(vf,grd[y][x].x,grd[y][x].y,grd[y][x].z,grd[y][x+1].x,grd[y][x+1].y,grd[y][x+1].z,col);
		if (!showlonlat) continue;
		sprintf(tbuf,"%g",((float)y-(float)GRDSIZ*.5f)*f+fcy); g = (float)strlen(tbuf)*.5f;
		pr.x = 0.10f; pd.x = 0.00f; pp.x = grd[y][GRDSIZ>>1].x - g*pr.x;
		pr.y = 0.00f; pd.y = 0.00f; pp.y = grd[y][GRDSIZ>>1].y - g*pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[y][GRDSIZ>>1].z - g*pr.z;
		voxie_printalph(vf,&pp,&pr,&pd,col,tbuf);
	}
	for(x=0;x<GRDSIZ;x++)
	{
		for(y=0;y<GRDSIZ-1;y++) voxie_drawlin(vf,grd[y][x].x,grd[y][x].y,grd[y][x].z,grd[y+1][x].x,grd[y+1][x].y,grd[y+1][x].z,col);
		if (!showlonlat) continue;
		sprintf(tbuf,"%g",((float)x-(float)GRDSIZ*.5f)*f+fcx); g = (float)strlen(tbuf)*.5f;
		pr.x = 0.00f; pd.x = 0.00f; pp.x = grd[GRDSIZ>>1][x].x - g*pr.x;
		pr.y = 0.10f; pd.y = 0.00f; pp.y = grd[GRDSIZ>>1][x].y - g*pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[GRDSIZ>>1][x].z - g*pr.z;
		voxie_printalph(vf,&pp,&pr,&pd,col,tbuf);
	}
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_wind_t vw;
	voxie_frame_t vf;
	voxie_inputs_t in;
	drawglobestats_t dgs = {0};      //Heightmap statistics from previous frame: used to keep mountains in view!
	dpoint3d dpos, drig, ddow, dfor; //<-Cartesian coordinates (relative to center of globe, where globe has radius of 1.0), and unit orthonormal orientation
	point3d pp, pr, pd, pf;
	double lon, lat, altradii;       //<-Spherical coordinates; recommended for informational purposes only
	double tim = 0.0, otim, dtim;
	float f, g, fx, fy, fz, fsc, fexagz = 8.f, fhakup = 0.f, resthresh = 256.f, yaw_deg = 150.f, pit_deg = 0.f, rol_deg = 0.f;
	int i, x, y;
	char tbuf[256];

	if (voxie_load(&vw) < 0) { MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) { MessageBox(0,"Error: voxie_init() failed","",MB_OK); return(-1); }

	demview_init(hvoxie);

	tiletype hp; if (kpzload("heipal.png",&hp.f,&hp.p,&hp.x,&hp.y)) { demview_setheipal((int *)hp.f,hp.x,-10000.f,+10000.f,.5f); }

		//Fields copied from DEMVIEW.INI in same order:
	if (demview_setserv(0/*height server*/, 0,15,8,1,1,0,1,0,"cache/ter%d_%dx%dx%d.png","s3.amazonaws.com"                ,"/elevation-tiles-prod/terrarium/%d/%d/%d.png",1.f,1.f,1.f)) return(1);
	if (demview_setserv(1/*color  server*/, 0,20,8,1,1,1,8,1,"cache/vi2%d_%dx%dx%d.jpg","t0.ssl.ak.tiles.virtualearth.net","/tiles/a%s.jpeg?g=5290"                      ,1.f,1.f,1.f)) return(1);

		//Convert location in degrees and altitude in meters to Cartesian coordinates
	demview_sph2xyz(-71.410129/*lon_deg*/, +41.825446/*lat_deg*/, (0.0/*alt_m*/ * fexagz + 3000.0)*(1.0/EARTHRADMET),&dpos,&drig,&ddow,&dfor); //Providence, RI

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		if (voxie_keystat(0x1)) voxie_quitloop(); //ESC

		if ((vw.useemu) && (in.dmousx|in.dmousy)) //let mouse rotate emulated view
		{
			vw.emuvang = min(max(vw.emuvang-in.dmousy*.01,-PI*.5),0.1268);
			vw.emuhang -= (float)in.dmousx*.01;
			voxie_init(&vw);
		}

		if (voxie_keystat(0x2) == 1) { if (demview_setserv(1,0,20,8,1,1,1,8,1,"cache/vi2%d_%dx%dx%d.jpg","t0.ssl.ak.tiles.virtualearth.net","/tiles/a%s.jpeg?g=5290",1.f,1.f,1.f)) return(1); }
		if (voxie_keystat(0x3) == 1) { if (demview_setserv(1,0,20,8,1,0,1,8,5,"cache/vir%d_%dx%dx%d.png",   "ecn.t0.tiles.virtualearth.net","/tiles/r%s.png?g=5273" ,1.f,1.f,1.f)) return(1); }
		if (voxie_keystat(0x4) == 1) { demview_setheipal(0,-1,-10000.f,+10000.f,.25f); }
		if (voxie_keystat(0x5) == 1) { demview_setheipal(0,-1,-10000.f,+10000.f,.50f); }

			//Rotate sprite
		if (voxie_keystat(0x10)) { yaw_deg -= dtim*90.0; } //Q
		if (voxie_keystat(0x11)) { yaw_deg += dtim*90.0; } //W
		if (voxie_keystat(0x12)) { pit_deg -= dtim*90.0; } //E
		if (voxie_keystat(0x13)) { pit_deg += dtim*90.0; } //R
		if (voxie_keystat(0x14)) { rol_deg -= dtim*90.0; } //T
		if (voxie_keystat(0x15)) { rol_deg += dtim*90.0; } //Y
		if (voxie_keystat(0x16)) { yaw_deg = 0.0; pit_deg = 0.0; rol_deg = 0.0; } //U

			//Control exaggerated height
		if (voxie_keystat(0x1f)) { fexagz *= pow(2.0,dtim); } //S
		if (voxie_keystat(0x2d)) { fexagz /= pow(2.0,dtim); } //X

			//Up/down
		if (voxie_keystat(0x1e)) { fhakup += dtim*1.0; } //A
		if (voxie_keystat(0x2c)) { fhakup -= dtim*1.0; } //Z

		voxie_frame_start(&vf);
		vf.flags |= VOXIEFRAME_FLAGS_IMMEDIATE; //<-Mandatory for internal multithreading!
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

			//NOTE: it is MUCH easier to use Cartesian coordinates (dpos,drig,ddow,dfor)
			//for consistent movement - for handling zoom and travelling near poles.
			//Double precision is necessary for the range of zoom allowed.
		demview_xyz2sph(&dpos,&lon,&lat,&altradii); f = altradii*dtim;
		fx = (voxie_keystat(0xcd)!=0)-(voxie_keystat(0xcb)!=0); //Rt-Lt
		fy = (voxie_keystat(0xd0)!=0)-(voxie_keystat(0xc8)!=0); //Dn-Up
		fz = (voxie_keystat(0x52)!=0)-(voxie_keystat(0x9d)!=0); //KP0-RCtrl
		dpos.x += (drig.x*fx + ddow.x*fy + dfor.x*fz)*f;
		dpos.y += (drig.y*fx + ddow.y*fy + dfor.y*fz)*f;
		dpos.z += (drig.z*fx + ddow.z*fy + dfor.z*fz)*f;

			//Handle rotation (optional)
		rotvex((float)((voxie_keystat(0x33)!=0)-(voxie_keystat(0x34)!=0))*dtim,&drig,&ddow); //,-.

			//Always keep earth at bottom (i.e. force right & down vectors to be perpendicular to position vector (relative to center of earth))
		rotvex(atan2(dpos.x*drig.x + dpos.y*drig.y + dpos.z*drig.z,dpos.x*dpos.x + dpos.y*dpos.y + dpos.z*dpos.z),&drig,&dfor);
		rotvex(atan2(dpos.x*ddow.x + dpos.y*ddow.y + dpos.z*ddow.z,dpos.x*dpos.x + dpos.y*dpos.y + dpos.z*dpos.z),&ddow,&dfor);

#if 0
			//Assign the most pleasing height (yes - this is ugly but unavoidable)
		dgs.fhakzadd = -vw.aspz-dgs.fzmin;
		if (dgs.fzcnt) dgs.fhakzadd = max(dgs.fhakzadd,-dgs.fzsum/(float)dgs.fzcnt);
		dgs.fhakzadd += fhakup;
#else
		
	 //double dtopm, dbotm;
	 //demview_gettopbotaltm(&dpos,&vw,&dtopm,&dbotm,fexagz,dgs.fhakzadd); //get cur top&bot

	 //demview_settopbotaltm(&dpos,&vw,  0.0, dbotm,&fexagz,&dgs.fhakzadd); //force sea level to be at top?
	 //demview_settopbotaltm(&dpos,&vw,dtopm,   0.0,&fexagz,&dgs.fhakzadd); //force sea level to be at bot?
	 //demview_settopbotaltm(&dpos,&vw,174.0,   0.0,&fexagz,&dgs.fhakzadd);
	 //demview_settopbotaltm(&dpos,&vw, 87.0,   0.0,&fexagz,&dgs.fhakzadd); //sea level to Scituate Reservoir
		demview_settopbotaltm(&dpos,&vw,100.0,-100.0,&fexagz,&dgs.fhakzadd); //set view range to: -100m to +100m altitude
		fexagz = 8.f;
#endif

		fsc = demview_drawglobe(&vw,&vf,&dpos,&drig,&ddow,&dfor,fexagz,resthresh,1,&dgs);

#if 1
			//Scituate reservoir //FUKNEW
		demview_sph2voxie(-71.587/*lon_deg*/,+41.758/*lat_deg*/,87/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pp);
		demview_sph2voxie(-71.596/*lon_deg*/,+41.760/*lat_deg*/,87/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pr);
		demview_sph2voxie(-71.589/*lon_deg*/,+41.764/*lat_deg*/,87/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pd);
		voxie_drawsph(&vf,pp.x,pp.y,pp.z,fsc*0.00001f,1,0xffffff);
		voxie_drawsph(&vf,pr.x,pr.y,pr.z,fsc*0.00001f,1,0xffffff);
		voxie_drawsph(&vf,pd.x,pd.y,pd.z,fsc*0.00001f,1,0xffffff);
		voxie_drawlin(&vf,pp.x,pp.y,pp.z,pr.x,pr.y,pr.z,0xffffff);
		voxie_drawlin(&vf,pr.x,pr.y,pr.z,pd.x,pd.y,pd.z,0xffffff);
		voxie_drawlin(&vf,pd.x,pd.y,pd.z,pp.x,pp.y,pp.z,0xffffff);
			//Show view volume
		for(i=0;i<64;i++)
		{
			voxie_drawsph(&vf,cos((double)i*PI*2.0/64.0)*1.41,sin((double)i*PI*2.0/64.0)*1.41,-vw.aspz,0.02,0,0xffffff);
			voxie_drawsph(&vf,cos((double)i*PI*2.0/64.0)*1.41,sin((double)i*PI*2.0/64.0)*1.41,     0.0,0.02,0,0xffffff);
			voxie_drawsph(&vf,cos((double)i*PI*2.0/64.0)*1.41,sin((double)i*PI*2.0/64.0)*1.41,+vw.aspz,0.02,0,0xffffff);
		}
#endif

		//------------------------------------------------------------------------
			//Draw sphere / line / text by projecting multiple key points
		demview_sph2voxie(-71.45/*lon_deg*/,+41.83/*lat_deg*/,120/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pp);
		demview_sph2voxie(-71.39/*lon_deg*/,+41.85/*lat_deg*/,120/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pr);
		demview_sph2voxie(-71.45/*lon_deg*/,+41.83/*lat_deg*/, 20/*alt_m*/ *fexagz*(1.0/EARTHRADMET),&pd);
		voxie_drawsph(&vf,pp.x,pp.y,pp.z,fsc*0.00001f,1,0xffffff);
		voxie_drawsph(&vf,pr.x,pr.y,pr.z,fsc*0.00001f,1,0xffffff);
		voxie_drawsph(&vf,pd.x,pd.y,pd.z,fsc*0.00001f,1,0xffffff);
		voxie_drawlin(&vf,pp.x,pp.y,pp.z,pr.x,pr.y,pr.z,0xffffff);
		voxie_drawlin(&vf,pp.x,pp.y,pp.z,pd.x,pd.y,pd.z,0xffffff);
		sprintf(tbuf,"VOXIEWOOD"); f = 1.f/(float)strlen(tbuf);
		pr.x = (pr.x-pp.x)*f; pd.x = (pd.x-pp.x);
		pr.y = (pr.y-pp.y)*f; pd.y = (pd.y-pp.y);
		pr.z = (pr.z-pp.z)*f; pd.z = (pd.z-pp.z); //fsc*fexagz*.00001f;
		voxie_printalph(&vf,&pp,&pr,&pd,0xffffff,tbuf);

			//Use fancy demview_sph2voxie() to return full pos&ori and draw a primitive (text) with it.
		demview_sph2voxie2(-71.4,+41.81,70.0,yaw_deg,pit_deg,rol_deg,.0003f/*scale*/,fexagz,1.f,&pp,&pr,&pd,&pf);
		pp.x -= pr.x*(float)strlen(tbuf)*.5f;
		pp.y -= pr.y*(float)strlen(tbuf)*.5f;
		pp.z -= pr.z*(float)strlen(tbuf)*.5f;
		voxie_printalph(&vf,&pp,&pr,&pf,0xffffff,tbuf);

			//Simple sprite function. Specify pos&ori using: center (lon,lat,alt_m), yaw,pit,rol, scale, Z exag (pos), Z exag (scale), col
		demview_drawspr(&vf,"caco.kv6",-71.38,+41.82,70.0, yaw_deg,pit_deg,rol_deg, .0003f/*scale*/, fexagz,       1.f,0x808000); //preserve cubic aspect
		demview_drawspr(&vf,"caco.kv6",-71.42,+41.80,70.0, yaw_deg,pit_deg,rol_deg, .0003f/*scale*/, fexagz,fexagz/8.f,0x008080); //scale height with fexagz (fexagz/8.f makes it cubic when fexagz == 8.f)

			//draw lat/lon grid at some height;
		drawgrid(&vf,lon,lat,0.0/*alt_m*/,altradii,fexagz,0xc0c0ff,voxie_keystat(0x2a));

		//------------------------------------------------------------------------

		voxie_debug_print6x8_(0,64,0xffffff,-1," Longitude:%9.5f deg %c",fabs(lon),lon>0?'E':'W');
		voxie_debug_print6x8_(0,72,0xffffff,-1," Latituude:%9.5f deg %c",fabs(lat),lat>0?'N':'S');
		voxie_debug_print6x8_(0,80,0xffffff,-1,"  Altitude: ~%.1f km",altradii*EARTHRADMET*.001f);
		voxie_debug_print6x8_(0,88,0xffffff,-1,"HeightExag: %.2fx",fexagz);
		voxie_debug_print6x8_(0,96,0xffffff,-1,"Yaw:%f Pit:%f Rol:%f",yaw_deg,pit_deg,rol_deg);
		//voxie_debug_print6x8_(0,112,0xffffff,-1,"zmin: %f\nzmax: %f\nzavg: %f\nznum: %i\n",dgs.fzmin,dgs.fzmax,dgs.fzavg/(float)dgs.fzavgn,dgs.fzavgn);
		if (tim < 5.0)
		{
			voxie_debug_print6x8_(0,128,0xffffff,-1,"Mapzen terrain tiles contain 3DEP, SRTM, and GMTED2010 content courtesy of");
			voxie_debug_print6x8_(0,136,0xffffff,-1,"the U.S. Geological Survey and ETOPO1 content courtesy of U.S. National");
			voxie_debug_print6x8_(0,144,0xffffff,-1,"Oceanic and Atmospheric Administration.");
		}

		voxie_debug_print6x8_(0,200,0xffffff,-1,"zmin: %f\nzmax: %f\nzavg: %f\nzstd: %f\nznum: %i\n",dgs.fzmin,dgs.fzmax,dgs.fzsum/(float)dgs.fzcnt,sqrt(fabs(dgs.fzsum2 - dgs.fzsum*dgs.fzsum/(double)dgs.fzcnt)/(double)(dgs.fzcnt-1)),dgs.fzcnt);
		voxie_debug_print6x8_(0,248,0xffffff,-1,"hakzadd: %f\n\n",dgs.fhakzadd);

		voxie_frame_end(); voxie_getvw(&vw);
	}

	demview_uninit();

	voxie_uninit(0);
	return(0);
}

#if 0
!endif
#endif
