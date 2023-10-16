#include "vxCPP.h"

#include "DemClass.h"
#include "ColorRamps.h"
/* DemClass Demo 
 * Example program of a VX++ app using the DemClass 
 *
 * 
 * NOTE on Caching - by default the DemClass will cache tiles under a cache\ sub directory relative to the EXE file so make sure that a cache folder / directory exists.  
 *
 */

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (!hVoxieDLL) return 1;
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
	if (pEntryFunction == 0) return 1;
	IVoxieBox* voxie = pEntryFunction();

	DemClass demClass(voxie);
	demClass.setServer(DEM_SERVER_TYPE::DEM_HEIGHT_SERVER/*height server*/, 0, 15, 8, 1, 1, 0, 1, 0, (char*)"cache/ter%d_%dx%dx%d.png", (char*)"s3.amazonaws.com", (char*)"/elevation-tiles-prod/terrarium/%d/%d/%d.png", 1.f, 1.f, 1.f);
	demClass.setServer(DEM_SERVER_TYPE::DEM_COLOR_SERVER/*color  server*/, 0, 20, 8, 1, 1, 1, 8, 1, (char*)"cache/vi2%d_%dx%dx%d.jpg", (char*)"t0.ssl.ak.tiles.virtualearth.net", (char*)"/tiles/a%s.jpeg?g=5290", 1.f, 1.f, 1.f);
	float fexagz = 8.f;
	// set starting pos
	demClass.setPosbyLonLatAlt(-71.410129/*lon_deg*/, +41.825446/*lat_deg*/, (0.0/*alt_m*/ * fexagz + 3000.0) * (1.0 / EARTHRADMET));



	// DEMO variables
	point3d mov = { 0 };
	float movSpeed = 1;
	point3d pp = { 0 }, pr = { 0 }, pd = { 0 }, pf = { 0 };
	float globalFSc = 1; // a simpler way to hold the globalForce Scale of the dem
	char tbuf[256]; // text buffer
	float f; 
	double dbotm, dtopm;
	int modelsFound = 0;
	while (voxie->breath())
	{
		/****
		* Input Keys
		*/ 
		

		// Move camera / DEM
		mov = { 0,0,0 };

		if (voxie->getKeyIsDown(KB_Arrow_Left))			mov.x =		-movSpeed;
		else if (voxie->getKeyIsDown(KB_Arrow_Right))	mov.x =		movSpeed;
		if (voxie->getKeyIsDown(KB_Arrow_Up))			mov.y =		-movSpeed;
		else if (voxie->getKeyIsDown(KB_Arrow_Down))	mov.y =		movSpeed;
		if (voxie->getKeyIsDown(KB_NUMPAD_0))			mov.z =		-movSpeed;
		else if (voxie->getKeyIsDown(KB_Control_Right)) mov.z =		movSpeed;

		if (mov.x != 0 || mov.y != 0 || mov.z != 0) {
			demClass.moveCamera(mov.x, mov.y, mov.z);
		}

		// Jup to Lat Lon
		if (voxie->getKeyIsDown(KB_1))	demClass.setPosbyLonLatAlt(-71.410129/*lon_deg*/, +41.825446/*lat_deg*/, (0.0/*alt_m*/ * fexagz + 3000.0) * (1.0 / EARTHRADMET));
		if (voxie->getKeyIsDown(KB_2))  demClass.setPosbyLonLat(-23.882738/*lon_deg*/, +16.191860/*lat_deg*/);


		// Rotate camera / DEM
		if (voxie->getKeyIsDown(KB_Comma))		demClass.rotateCamera(-1);
		if (voxie->getKeyIsDown(KB_Full_Stop))  demClass.rotateCamera(1);
	
		if (voxie->getKeyOnDown(KB_L)) demClass.setDemRotatedAngle(demClass.getDemRotatedAngle() - 1.57081); // flip angle 90 degrees <--
		if (voxie->getKeyOnDown(KB_Semicolon)) demClass.setDemRotatedAngle(demClass.getDemRotatedAngle() + 1.57081); // flip angle 90 degrees -->

		// Rotate Sprite 
		if (voxie->getKeyIsDown(KB_Q)) { demClass.g_yaw_deg -= voxie->getDeltaTime() * 90.0; } //Q
		if (voxie->getKeyIsDown(KB_W)) { demClass.g_yaw_deg += voxie->getDeltaTime() * 90.0; } //W
		if (voxie->getKeyIsDown(KB_E)) { demClass.g_pitch_deg -= voxie->getDeltaTime() * 90.0; } //E
		if (voxie->getKeyIsDown(KB_R)) { demClass.g_pitch_deg += voxie->getDeltaTime() * 90.0; } //R
		if (voxie->getKeyIsDown(KB_T)) { demClass.g_roll_deg -= voxie->getDeltaTime() * 90.0; } //T
		if (voxie->getKeyIsDown(KB_Y)) { demClass.g_roll_deg += voxie->getDeltaTime() * 90.0; } //Y
		if (voxie->getKeyIsDown(KB_U)) { demClass.g_yaw_deg = 0.0; demClass.g_pitch_deg = 0.0; demClass.g_roll_deg = 0.0; } //U

		// Move Camera Z
		if (voxie->getKeyIsDown(KB_K))  demClass.adjustCameraZ(-1);
		if (voxie->getKeyIsDown(KB_M))  demClass.adjustCameraZ(1);
		if (voxie->getKeyOnDown(KB_J)) { demClass.getTopBotAltm(&dtopm, &dbotm); demClass.setTopBotAltm(dtopm, 0); }  //force sea level to be at top of view
		if (voxie->getKeyOnDown(KB_N)) { demClass.getTopBotAltm(&dtopm, &dbotm); demClass.setTopBotAltm(0, dbotm); } // force sea level to be at bottom of view


		// Toggle Sea Lock
		if (voxie->getKeyOnDown(KB_S))  demClass.setSeaLock(!demClass.isSeaLockEnabled());

		// Draw Extra Stuff
		if (voxie->getKeyIsDown(KB_G))  demClass.drawGrid(0x00ffff);
		if (voxie->getKeyIsDown(KB_F))  demClass.drawCompass();


		// Adjust Col Multiplier
		if (voxie->getKeyOnDown(KB_C)) demClass.setColMultiplier(3, 2, 2);

		// Load Height Map Pal - functions 
		if (voxie->getKeyOnDown(KB_P)) demClass.setHeighPalIMG("assets/heipal.png", 0.5f);
		if (voxie->getKeyOnDown(KB_I)) demClass.setHeightPalColArray(beach_ramp, 1024, -10000, 10000, 0.75);
		if (voxie->getKeyOnDown(KB_O)) demClass.clearHeightPal();

		// Adjust Height Exaggeration 
		if (voxie->getKeyIsDown(KB_A)) demClass.adjustHeightEx(1);
		if (voxie->getKeyIsDown(KB_Z)) demClass.adjustHeightEx(-1);


		// Start the Voxie Drawing Engine... 
		voxie->startFrame();

		// render the DEM onto the display -- do this at the start of the frame to avoid some graphical glitches
		demClass.drawGlobe();

		/* Drawing on the DEM examples
		 *
		 */

		// use the various converters to convert the dem's locations to Voxie Space		 
		demClass.sph2voxie(-71.45/*lon_deg*/, +41.83/*lat_deg*/, demClass.alt2altradii(120, true)/*alt_m add in height exaggeration to have it appear in the correct spot*/, &pp);
		demClass.sph2voxie(-71.39/*lon_deg*/, +41.85/*lat_deg*/, demClass.alt2altradii(120, true)/*alt_m*/, &pr);
		demClass.sph2voxie(-71.45/*lon_deg*/, +41.83/*lat_deg*/, demClass.alt2altradii(20, true)/*alt_m*/, &pd);
		
		globalFSc = demClass.getGlobalFloatScale();

		voxie->drawSphere(pp.x, pp.y, pp.z, globalFSc * 0.00001f, 1, 0xffffff);
		voxie->drawSphere(pr.x, pr.y, pr.z, globalFSc * 0.00001f, 1, 0xffffff);
		voxie->drawSphere(pd.x, pd.y, pd.z, globalFSc * 0.00001f, 1, 0xffffff);
		voxie->drawLine(pp.x, pp.y, pp.z, pr.x, pr.y, pr.z, 0xffffff);
		voxie->drawLine(pp.x, pp.y, pp.z, pd.x, pd.y, pd.z, 0xffffff);

		sprintf_s(tbuf, "VOXIEWOOD"); // using a char buffer as
		f = 1.f / (float)strlen(tbuf); // need to know how many chars in the string to get the sizing right
		pr.x = (pr.x - pp.x) * f; pd.x = (pd.x - pp.x);
		pr.y = (pr.y - pp.y) * f; pd.y = (pd.y - pp.y);
		pr.z = (pr.z - pp.z) * f; pd.z = (pd.z - pp.z); //fsc*fexagz*.00001f;
		voxie->drawText(&pp, &pr, &pd, 0xffffff, tbuf);

		//Use fancy demview_sph2voxie() to return full pos&ori and draw a primitive (text) with it.
		demClass.sph2voxie2(-71.4, +41.81, 70.0, demClass.g_yaw_deg, demClass.g_pitch_deg, demClass.g_roll_deg, .0003f/*scale*/, demClass.getHeightEx(), 1.f, &pp, &pr, &pd, &pf);
		pp.x -= pr.x * (float)strlen(tbuf) * .5f;
		pp.y -= pr.y * (float)strlen(tbuf) * .5f;
		pp.z -= pr.z * (float)strlen(tbuf) * .5f;
		voxie->drawText(&pp, &pr, &pf, 0xffffff, tbuf);

		//Simple sprite function. Specify pos&ori using: center (lon,lat,alt_m), yaw,pit,rol, scale, Z exag (pos), Z exag (scale), col
		demClass.drawSprite("assets/caco.kv6", -71.38, +41.82, 120, demClass.g_yaw_deg, demClass.g_pitch_deg, demClass.g_roll_deg, .0003f/*scale*/, demClass.getHeightEx(), 1.f, 0x808000); //preserve cubic aspect
		
		// Draw Sprite but have it scale according to height exaggeration
		demClass.drawSprite("assets/caco.kv6", -71.42, +41.80, 70.0,  demClass.g_yaw_deg, demClass.g_pitch_deg, demClass.g_roll_deg, .0003f/*scale*/, demClass.getHeightEx(), demClass.getHeightEx() / 8.f, 0x008080); //scale height with fexagz (fexagz/8.f makes it cubic when fexagz == 8.f)
		


		demClass.report(100, 100);
		
		voxie->showVPS();
		voxie->endFrame();
	}

	voxie->quitLoop();
	//delete voxie;
	return 0;
}
