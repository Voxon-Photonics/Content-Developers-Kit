#include "vxCPP.h"
#include "demviewVXCPP.hpp"


/* DemView API working in VX++ Frame Work. 
 * This example uses the demviewVXCPP.hpp header to interface with the demview.DLL
 *
 */

static void drawgrid(IVoxieBox * voxie, float lon, float lat, float alt_m, float altradii, float fexagz, int col, int showlonlat)
{
#define GRDSIZ 12
	static point3d grd[GRDSIZ][GRDSIZ];
	point3d pp, pr, pd;
	float f, g, rf, fcx, fcy;
	int i, x, y;
	char tbuf[256];

	f = altradii * 32.f;
	i = *(int*)&f; i &= 0x7f800000; f = *(float*)&i; //quantize to power of 2
	if (f > 16.f) f = 16.f;
	rf = 1.f / f;
	fcx = ceil(lon * rf) * f;
	fcy = ceil(lat * rf) * f;
	if (((float)0 - (float)GRDSIZ * .5f) * f + fcy < -90.f) fcy = -90.f - ((float)0 - (float)GRDSIZ * .5f) * f;
	if (((float)GRDSIZ - (float)GRDSIZ * .5f) * f + fcy > +90.f) fcy = +90.f - ((float)GRDSIZ - (float)GRDSIZ * .5f) * f;

	for (y = 0; y < GRDSIZ; y++)
		for (x = 0; x < GRDSIZ; x++)
		{
			demview_sph2voxie(((float)x - (float)GRDSIZ * .5f) * f + fcx,
				((float)y - (float)GRDSIZ * .5f) * f + fcy, alt_m * fexagz * (1.0 / EARTHRADMET), &grd[y][x]);
		}
	for (y = 0; y < GRDSIZ; y++)
	{
		for (x = 0; x < GRDSIZ - 1; x++) voxie->drawLine(grd[y][x].x, grd[y][x].y, grd[y][x].z, grd[y][x + 1].x, grd[y][x + 1].y, grd[y][x + 1].z, col);
		if (!showlonlat) continue;
		sprintf_s(tbuf, "%g", ((float)y - (float)GRDSIZ * .5f) * f + fcy); g = (float)strlen(tbuf) * .5f;
		pr.x = 0.10f; pd.x = 0.00f; pp.x = grd[y][GRDSIZ >> 1].x - g * pr.x;
		pr.y = 0.00f; pd.y = 0.00f; pp.y = grd[y][GRDSIZ >> 1].y - g * pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[y][GRDSIZ >> 1].z - g * pr.z;
		voxie->drawText(&pp, &pr, &pd, col, tbuf);
	}
	for (x = 0; x < GRDSIZ; x++)
	{
		for (y = 0; y < GRDSIZ - 1; y++) voxie->drawLine(grd[y][x].x, grd[y][x].y, grd[y][x].z, grd[y + 1][x].x, grd[y + 1][x].y, grd[y + 1][x].z, col);
		if (!showlonlat) continue;
		sprintf_s(tbuf, "%g", ((float)x - (float)GRDSIZ * .5f) * f + fcx); g = (float)strlen(tbuf) * .5f;
		pr.x = 0.00f; pd.x = 0.00f; pp.x = grd[GRDSIZ >> 1][x].x - g * pr.x;
		pr.y = 0.10f; pd.y = 0.00f; pp.y = grd[GRDSIZ >> 1][x].y - g * pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[GRDSIZ >> 1][x].z - g * pr.z;
		voxie->drawText(&pp, &pr, &pd, col, tbuf);
	}
}

/* Port of DemSimp.c as for VXCPP 
 *
 *
 */
int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (!hVoxieDLL) return 1;
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
	if (pEntryFunction == 0) return 1;
	IVoxieBox* voxie = pEntryFunction();

	// Define DemView Variables
	drawglobestats_t dgs = { 0 };		//Heightmap statistics from previous frame: used to keep mountains in view!
	dpoint3d dpos, drig, ddow, dfor;	//<-Cartesian coordinates (relative to center of globe, where globe has radius of 1.0), and unit orthonormal orientation
	point3d pp, pr, pd, pf;
	double lon, lat, altradii;			//<-Spherical coordinates; recommended for informational purposes only
	float f, g, fx, fy, fz, fsc, fexagz = 8.f, fhakup = 0.f, resthresh = 256.f, yaw_deg = 150.f, pit_deg = 0.f, rol_deg = 0.f;
	int i, x, y;
	char tbuf[256];

	// initialise Dem view
	demview_init(voxie->getVoxieBoxDLLHandle()); // Give Dem a Handle of the voxiebox.dll instance. this is a direct voxiebox.DLL handle which is nested within the Voxiebox class (not VxCPP.dll)


	// Load Color override - if heipal.png exists
	tiletype hp; if (voxie->_kpzload("heipal.png", &hp.f, &hp.p, &hp.x, &hp.y)) { demview_setheipal((int*)hp.f, hp.x, -10000.f, +10000.f, .5f); }

	// set servers
	//Fields copied from DEMVIEW.INI in same order:
	if (demview_setserv(0/*height server*/, 0, 15, 8, 1, 1, 0, 1, 0, (char*)"cache/ter%d_%dx%dx%d.png", (char*)"s3.amazonaws.com", (char*)"/elevation-tiles-prod/terrarium/%d/%d/%d.png", 1.f, 1.f, 1.f)) return(1);
	if (demview_setserv(1/*color  server*/, 0, 20, 8, 1, 1, 1, 8, 1, (char*)"cache/vi2%d_%dx%dx%d.jpg", (char*)"t0.ssl.ak.tiles.virtualearth.net", (char*)"/tiles/a%s.jpeg?g=5290", 1.f, 1.f, 1.f)) return(1);

	//Convert location in degrees and altitude in meters to Cartesian coordinates
	// load starting view
	demview_sph2xyz(-71.410129/*lon_deg*/, +41.825446/*lat_deg*/, (0.0/*alt_m*/ * fexagz + 3000.0) * (1.0 / EARTHRADMET), &dpos, &drig, &ddow, &dfor); //Providence, RI
	  

	while (voxie->breath())
	{
		/************
		* INPUT
		*/

		if (voxie->getKeyIsDown(KB_1) == 1) { if (demview_setserv(1, 0, 20, 8, 1, 1, 1, 8, 1, (char*)"cache/vi2%d_%dx%dx%d.jpg", (char*)"t0.ssl.ak.tiles.virtualearth.net", (char*)"/tiles/a%s.jpeg?g=5290", 1.f, 1.f, 1.f)) return(1); }
		if (voxie->getKeyIsDown(KB_2) == 1) { if (demview_setserv(1, 0, 20, 8, 1, 0, 1, 8, 5, (char*)"cache/vir%d_%dx%dx%d.png", (char*)"ecn.t0.tiles.virtualearth.net", (char*)"/tiles/r%s.png?g=5273", 1.f, 1.f, 1.f)) return(1); }
		if (voxie->getKeyIsDown(KB_3) == 1) { demview_setheipal(0, -1, -10000.f, +10000.f, .25f); }
		if (voxie->getKeyIsDown(KB_4) == 1) { demview_setheipal(0, -1, -10000.f, +10000.f, .50f); }

		//Rotate sprite
		if (voxie->getKeyIsDown(KB_Q)) { yaw_deg -= voxie->getDeltaTime() * 90.0; } //Q
		if (voxie->getKeyIsDown(KB_W)) { yaw_deg += voxie->getDeltaTime() * 90.0; } //W
		if (voxie->getKeyIsDown(KB_E)) { pit_deg -= voxie->getDeltaTime() * 90.0; } //E
		if (voxie->getKeyIsDown(KB_R)) { pit_deg += voxie->getDeltaTime() * 90.0; } //R
		if (voxie->getKeyIsDown(KB_T)) { rol_deg -= voxie->getDeltaTime() * 90.0; } //T
		if (voxie->getKeyIsDown(KB_Y)) { rol_deg += voxie->getDeltaTime() * 90.0; } //Y
		if (voxie->getKeyIsDown(KB_U)) { yaw_deg = 0.0; pit_deg = 0.0; rol_deg = 0.0; } //U

			//Control exaggerated height
		if (voxie->getKeyIsDown(KB_S) == 1) { fexagz *= pow(2.0, voxie->getDeltaTime()); } //S
		if (voxie->getKeyIsDown(KB_X) == 1 ) { fexagz /= pow(2.0, voxie->getDeltaTime()); } //X

			//Up/down
		if (voxie->getKeyIsDown(KB_A)) { fhakup += voxie->getDeltaTime() * 1.0; } //A
		if (voxie->getKeyIsDown(KB_Z)) { fhakup -= voxie->getDeltaTime() * 1.0; } //Z



		/**********
		 * Draw
	     */

		voxie->startFrame();
		// set this flag in the voxie_frame buffer for each frame to enable multi threading which is necessary for Demview API
		voxie->getVoxieFrame()->flags |= VOXIEFRAME_FLAGS_IMMEDIATE; //<-Mandatory for internal multi threading!

		//NOTE: it is MUCH easier to use Cartesian coordinates (dpos,drig,ddow,dfor)
		//for consistent movement - for handling zoom and travelling near poles.
		//Double precision is necessary for the range of zoom allowed.
		demview_xyz2sph(&dpos, &lon, &lat, &altradii); f = altradii * voxie->getDeltaTime();
		fx = (voxie->getKeyIsDown(KB_Arrow_Right) != 0) - (voxie->getKeyIsDown(KB_Arrow_Left) != 0); //Rt-Lt
		fy = (voxie->getKeyIsDown(KB_Arrow_Down) != 0)  - (voxie->getKeyIsDown(KB_Arrow_Up) != 0);   //Dn-Up
		fz = (voxie->getKeyIsDown(KB_NUMPAD_0) != 0)    - (voxie->getKeyIsDown(KB_Control_Right) != 0);  //KP0-RCtrl
		dpos.x += (drig.x * fx + ddow.x * fy + dfor.x * fz) * f;
		dpos.y += (drig.y * fx + ddow.y * fy + dfor.y * fz) * f;
		dpos.z += (drig.z * fx + ddow.z * fy + dfor.z * fz) * f;

		//Handle rotation (optional)
		rotDPvex((float)((voxie->getKeyIsDown(KB_Comma) != 0) - (voxie->getKeyIsDown(KB_Full_Stop) != 0)) * voxie->getDeltaTime(), &drig, &ddow); //,-.
		

			//Always keep earth at bottom (i.e. force right & down vectors to be perpendicular to position vector (relative to center of earth))
		rotDPvex(atan2(dpos.x * drig.x + dpos.y * drig.y + dpos.z * drig.z, dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z), &drig, &dfor);
		rotDPvex(atan2(dpos.x * ddow.x + dpos.y * ddow.y + dpos.z * ddow.z, dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z), &ddow, &dfor);

		//Assign the most pleasing height (yes - this is ugly but unavoidable)
		dgs.fhakzadd = -voxie->getAspectZ() - dgs.fzmin;
		if (dgs.fzcnt) dgs.fhakzadd = max(dgs.fhakzadd, -dgs.fzsum / (float)dgs.fzcnt);
		dgs.fhakzadd += fhakup;
	
		
		/* - uncomment this to seet view range
		demview_settopbotaltm(&dpos, voxie->getVoxieWindow(), 100.0, -100.0, &fexagz, &dgs.fhakzadd); //set view range to: -100m to +100m altitude
		fexagz = 8.f;
		*/


		// draw globe
		fsc = demview_drawglobe(voxie->getVoxieWindow(), voxie->getVoxieFrame(), &dpos, &drig, &ddow, &dfor, fexagz, resthresh, 1, &dgs);


		// Draw onto the Dem

		//Scituate reservoir 
		demview_sph2voxie(-71.587/*lon_deg*/, +41.758/*lat_deg*/, 87/*alt_m*/ * fexagz * (1.0 / EARTHRADMET), &pp);
		demview_sph2voxie(-71.596/*lon_deg*/, +41.760/*lat_deg*/, 87/*alt_m*/ * fexagz * (1.0 / EARTHRADMET), &pr);
		demview_sph2voxie(-71.589/*lon_deg*/, +41.764/*lat_deg*/, 87/*alt_m*/ * fexagz * (1.0 / EARTHRADMET), &pd);
		voxie->drawSphere(pp, fsc * 0.00001f, 1, 0xffffff);
		voxie->drawSphere(pr, fsc * 0.00001f, 1, 0xffffff);
		voxie->drawSphere(pd, fsc * 0.00001f, 1, 0xffffff);
		voxie->drawLine(pp.x, pp.y, pp.z, pr.x, pr.y, pr.z, 0xffffff);
		voxie->drawLine(pr.x, pr.y, pr.z, pd.x, pd.y, pd.z, 0xffffff);
		voxie->drawLine(pd.x, pd.y, pd.z, pp.x, pp.y, pp.z, 0xffffff);
		//Show view volume
		for (i = 0; i < 64; i++)
		{
			voxie->drawSphere( cos((double)i * PI * 2.0 / 64.0) * 1.41, sin((double)i * PI * 2.0 / 64.0) * 1.41, -voxie->getAspectZ(), 0.02, 0, 0xffffff);
			voxie->drawSphere( cos((double)i * PI * 2.0 / 64.0) * 1.41, sin((double)i * PI * 2.0 / 64.0) * 1.41, 0.0, 0.02, 0, 0xffffff);
			voxie->drawSphere(cos((double)i * PI * 2.0 / 64.0) * 1.41, sin((double)i * PI * 2.0 / 64.0) * 1.41, +voxie->getAspectZ(), 0.02, 0, 0xffffff);
		}

		//draw lat/lon grid at some height;
		drawgrid(voxie, lon, lat, 0.0/*alt_m*/, altradii, fexagz, 0xc0c0ff, voxie->getKeyIsDown(KB_Shift_Left));

		sprintf_s(tbuf, "VOXIEWOOD"); f = 1.f / (float)strlen(tbuf);
		pr.x = (pr.x - pp.x) * f; pd.x = (pd.x - pp.x);
		pr.y = (pr.y - pp.y) * f; pd.y = (pd.y - pp.y);
		pr.z = (pr.z - pp.z) * f; pd.z = (pd.z - pp.z); //fsc*fexagz*.00001f;
		voxie->drawText(&pp, &pr, &pd, 0xffffff, tbuf);

		//Use fancy demview_sph2voxie() to return full pos&ori and draw a primitive (text) with it.
		demview_sph2voxie2(-71.4, +41.81, 70.0, yaw_deg, pit_deg, rol_deg, .0003f/*scale*/, fexagz, 1.f, &pp, &pr, &pd, &pf);
		pp.x -= pr.x * (float)strlen(tbuf) * .5f;
		pp.y -= pr.y * (float)strlen(tbuf) * .5f;
		pp.z -= pr.z * (float)strlen(tbuf) * .5f;
		voxie->drawText(&pp, &pr, &pf, 0xffffff, tbuf);

		//Simple sprite function. Specify pos&ori using: center (lon,lat,alt_m), yaw,pit,rol, scale, Z exag (pos), Z exag (scale), col
		demview_drawspr(voxie->getVoxieFrame(), "caco.kv6", -71.38, +41.82, 70.0, yaw_deg, pit_deg, rol_deg, .0003f/*scale*/, fexagz, 1.f, 0x808000); //preserve cubic aspect
		demview_drawspr(voxie->getVoxieFrame(), "caco.kv6", -71.42, +41.80, 70.0, yaw_deg, pit_deg, rol_deg, .0003f/*scale*/, fexagz, fexagz / 8.f, 0x008080); //scale height with fexagz (fexagz/8.f makes it cubic when fexagz == 8.f)


		//------------------------------------------------------------------------

		voxie->debugText(0, 64, 0xffffff, -1, " Longitude:%9.5f deg %c", fabs(lon), lon > 0 ? 'E' : 'W');
		voxie->debugText(0, 72, 0xffffff, -1, " Latituude:%9.5f deg %c", fabs(lat), lat > 0 ? 'N' : 'S');
		voxie->debugText(0, 80, 0xffffff, -1, "  Altitude: ~%.1f km", altradii * EARTHRADMET * .001f);
		voxie->debugText(0, 88, 0xffffff, -1, "HeightExag: %.2fx", fexagz);
		voxie->debugText(0, 96, 0xffffff, -1, "Yaw:%f Pit:%f Rol:%f", yaw_deg, pit_deg, rol_deg);
		//voxie->debugText(0,112,0xffffff,-1,"zmin: %f\nzmax: %f\nzavg: %f\nznum: %i\n",dgs.fzmin,dgs.fzmax,dgs.fzavg/(float)dgs.fzavgn,dgs.fzavgn);
		if (voxie->getTime() < 5.0)
		{
			voxie->debugText(0, 128, 0xffffff, -1, "Mapzen terrain tiles contain 3DEP, SRTM, and GMTED2010 content courtesy of");
			voxie->debugText(0, 136, 0xffffff, -1, "the U.S. Geological Survey and ETOPO1 content courtesy of U.S. National");
			voxie->debugText(0, 144, 0xffffff, -1, "Oceanic and Atmospheric Administration.");
		}

		voxie->debugText(0, 200, 0xffffff, -1, "zmin: %f\nzmax: %f\nzavg: %f\nzstd: %f\nznum: %i\n", dgs.fzmin, dgs.fzmax, dgs.fzsum / (float)dgs.fzcnt, sqrt(fabs(dgs.fzsum2 - dgs.fzsum * dgs.fzsum / (double)dgs.fzcnt) / (double)(dgs.fzcnt - 1)), dgs.fzcnt);
		voxie->debugText(0, 248, 0xffffff, -1, "hakzadd: %f\n\n", dgs.fhakzadd);

		voxie->showVPS();
		voxie->endFrame();
	}

	demview_uninit();

	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;

	return 0;
}
