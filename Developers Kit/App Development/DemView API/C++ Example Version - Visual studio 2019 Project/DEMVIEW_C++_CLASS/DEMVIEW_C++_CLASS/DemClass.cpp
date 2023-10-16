#include "DemClass.h"

DemClass::DemClass()
{
}

DemClass::DemClass(IVoxieBox* voxiePtr)
{
	Init(voxiePtr);
}

DemClass::~DemClass()
{
	demview_uninit_int();
	if (hdemview != (HINSTANCE)INVALID_HANDLE_VALUE) {
		FreeLibrary(hdemview);
		hdemview = 0;
	}
}

// Load DemView.DLL into memory and map DLL functions to class
void DemClass::Init(IVoxieBox* voxiePtr)
{
	hdemview = LoadLibrary("demview.dll");
	if (!hdemview) { MessageBox(0, "Fail to load DEMVIEW.DLL", "DEMVIEWVXCPP.HPP", MB_OK); ExitProcess(0); }

	demview_init_int = (void(__cdecl*)(HINSTANCE hvoxie))GetProcAddress(hdemview, "demview_init");
	demview_uninit_int = (void(__cdecl*)(void))GetProcAddress(hdemview, "demview_uninit");
	demview_setserv = (int(__cdecl*)(int, int, int, int, int, int, int, int, int, char*, char*, char*, float, float, float))GetProcAddress(hdemview, "demview_setserv");
	demview_drawglobe = (float(__cdecl*)(voxie_wind_t*, voxie_frame_t*, dpoint3d*, dpoint3d*, dpoint3d*, dpoint3d*, float, float, int, drawglobestats_t*))GetProcAddress(hdemview, "demview_drawglobe");
	demview_drawspr = (void(__cdecl*)(voxie_frame_t*, const char*, double, double, double, double, double, double, double, double, double, int))GetProcAddress(hdemview, "demview_drawspr");
	demview_setheipal = (void(__cdecl*)(int*, int, float, float, float))GetProcAddress(hdemview, "demview_setheipal");
	demview_sph2xyz = (void(__cdecl*)(double, double, double, dpoint3d*, dpoint3d*, dpoint3d*, dpoint3d*))GetProcAddress(hdemview, "demview_sph2xyz");
	demview_xyz2sph = (void(__cdecl*)(dpoint3d*, double*, double*, double*))GetProcAddress(hdemview, "demview_xyz2sph");
	demview_xyz2voxie = (void(__cdecl*)(dpoint3d*, point3d*))GetProcAddress(hdemview, "demview_xyz2voxie");
	demview_sph2voxie = (void(__cdecl*)(double, double, double, point3d*))GetProcAddress(hdemview, "demview_sph2voxie");
	demview_sph2voxie2 = (void(__cdecl*)(double, double, double, double, double, double, double, double, double, point3d*, point3d*, point3d*, point3d*))GetProcAddress(hdemview, "demview_sph2voxie2");
	demview_voxie2xyz = (void(__cdecl*)(point3d*, dpoint3d*))GetProcAddress(hdemview, "demview_voxie2xyz");
	demview_voxie2sph = (void(__cdecl*)(point3d*, double*, double*, double*))GetProcAddress(hdemview, "demview_voxie2sph");

	this->voxiePtr = voxiePtr;
	demview_init_int(voxiePtr->getVoxieBoxDLLHandle());
	isInitialized = true;
}
int DemClass::setServer(DEM_SERVER_TYPE serverType, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char* cache_st, char* url_st, char* tile_st, float rmul, float gmul, float bmul)
{

	if (serverType == DEM_SERVER_TYPE::DEM_COLOR_SERVER) {
		this->currentColorServer.serverType = serverType;
		this->currentColorServer.tilezmin = tilezmin;
		this->currentColorServer.tilezmax = tilezmax;
		this->currentColorServer.log2tilesiz = log2tilesiz;
		this->currentColorServer.iswebmerc = iswebmerc;
		this->currentColorServer.ishttps = ishttps;
		this->currentColorServer.isquadtree = isquadtree;
		this->currentColorServer.servnum = servnum;
		this->currentColorServer.servcharoffs = servcharoffs;
		strcpy_s(this->currentColorServer.cache_st, cache_st);
		strcpy_s(this->currentColorServer.url_st, url_st);
		strcpy_s(this->currentColorServer.tile_st, tile_st);
		this->currentColorServer.isSet = true;
	}

	if (serverType == DEM_SERVER_TYPE::DEM_HEIGHT_SERVER) {
		this->currentHeightServer.serverType = serverType;
		this->currentHeightServer.tilezmin = tilezmin;
		this->currentHeightServer.tilezmax = tilezmax;
		this->currentHeightServer.log2tilesiz = log2tilesiz;
		this->currentHeightServer.iswebmerc = iswebmerc;
		this->currentHeightServer.ishttps = ishttps;
		this->currentHeightServer.isquadtree = isquadtree;
		this->currentHeightServer.servnum = servnum;
		this->currentHeightServer.servcharoffs = servcharoffs;
		strcpy_s(this->currentHeightServer.cache_st, cache_st);
		strcpy_s(this->currentHeightServer.url_st, url_st);
		strcpy_s(this->currentHeightServer.tile_st, tile_st);
		this->currentColorServer.isSet = true;
	}

	colMultiplier.x = rmul;
	colMultiplier.y = gmul;
	colMultiplier.z = bmul;


	return demview_setserv((int)serverType, tilezmin, tilezmax, log2tilesiz, iswebmerc, ishttps, isquadtree, servnum, servcharoffs,  cache_st,  url_st,  tile_st,  rmul,  gmul,  bmul);
}

void DemClass::updateGlobe()
{

	//Assign the most pleasing height (yes - this is ugly but unavoidable)	
	dgs.fhakzadd = -voxiePtr->getAspectZ() - dgs.fzmin;
	if (dgs.fzcnt) dgs.fhakzadd = max(dgs.fhakzadd, -dgs.fzsum / (float)dgs.fzcnt);
	dgs.fhakzadd += this->fhakup;

	// update globe's log, lat and altradii
	demview_xyz2sph(&dpos, &lon, &lat, &altradii);

	// update globe's alt_m
	alt_m = this->altradii2alt(altradii);

	if (seaLock) {
		float ofexagz = this->fexagz; // hak to preserve the height exaggeration 
		this->setTopBotAltm(800, -800);
		fexagz = ofexagz; 
	}

	//Always keep earth at bottom (i.e. force right & down vectors to be perpendicular to position vector (relative to center of earth))
	/* -- this isn't used
	if (keepEarthAtBottom) {
		rotDPVex(atan2(dpos.x * drig.x + dpos.y * drig.y + dpos.z * drig.z, dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z), &drig, &dfor);
		rotDPVex(atan2(dpos.x * ddow.x + dpos.y * ddow.y + dpos.z * ddow.z, dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z), &ddow, &dfor);

	}
	*/

	
	if (alt_m > MAX_ALT_ZOOM) {
		this->setPosbyLonLatAlt(lon, lat, this->alt2altradii(MAX_ALT_ZOOM));
	}

	if (alt_m < MIN_ALT_ZOOM) {
		this->setPosbyLonLatAlt(lon, lat, this->alt2altradii(MIN_ALT_ZOOM));
	}
	
}


void DemClass::drawCompass() {

	if (voxiePtr == nullptr) return;

	int i = 0;
	int j = 0; int n = 0;

	voxie_wind_t* vw = voxiePtr->getVoxieWindow();
	float r = vw->aspr * .99;

	if (vw->clipshape == 0) r = vw->aspx;

	int col[4] = { 0xff0000, 0x00ffff, 0x00ff00, 0xffff00, };
	point3d pp = { 0,0,0 };
	point3d textPos = { 0, 0, 0 };
	point3d textWidth = { .15,0,0 };
	point3d textHeight = { 0,.15,0 };


	n = 4;
	for (i = 0; i < n; i++)
	{
	
		pp.x = cos(this->rotatedAngle + (float)(i + 0) * ((PI * 2.0)) / (float)n) * (r);
		pp.y = sin(this->rotatedAngle + (float)(i + 0) * ((PI * 2.0)) / (float)n) * (r);
		pp.z = 0;


		textPos.x = cos(this->rotatedAngle + (float)(i + 0) * ((PI * 2.0)) / (float)n) * (r * .9);
		textPos.y = sin(this->rotatedAngle + (float)(i + 0) * ((PI * 2.0)) / (float)n) * (r * .9);
		textPos.z = -vw->aspz + 0.1;


		voxiePtr->drawLine(pp.x, pp.y, -vw->aspz,
			pp.x, pp.y, +vw->aspz, col[i]);



		switch (i) {
		case 0:
			voxiePtr->drawText(&textPos, &textWidth, &textHeight, col[i], (char*)"E");

			break;
		case 1:

			voxiePtr->drawText(&textPos, &textWidth, &textHeight, col[i], (char*)"S");


			break;
		case 2:
			voxiePtr->drawText(&textPos, &textWidth, &textHeight, col[i], (char*)"W");

			break;
		case 3:
			voxiePtr->drawText(&textPos, &textWidth, &textHeight, col[i], (char*)"N");

			break;

		}

	}
	

}

float DemClass::getGlobalFloatScale()
{
	return globalFSc;
}

bool DemClass::isSeaLockEnabled()
{
	return this->seaLock;
}

void DemClass::setUpdateGlobeOnDraw(bool choice)
{
	this->updateGlobeOnDraw = choice;
}

bool DemClass::isUpdateGlobeOnDrawEnabled() {
	
	return this->updateGlobeOnDraw;
}



void DemClass::setSeaLock(bool choice)
{
	this->seaLock = choice;
}

int DemClass::setServer(int serverType, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char* cache_st, char* url_st, char* tile_st, float rmul, float gmul, float bmul)
{

	DEM_SERVER_TYPE serverTypeConverted = DEM_SERVER_TYPE::DEM_HEIGHT_SERVER;

	switch (serverType) {
	case 0:
	default:
		serverTypeConverted = DEM_SERVER_TYPE::DEM_HEIGHT_SERVER;
	case 1:
		serverTypeConverted = DEM_SERVER_TYPE::DEM_COLOR_SERVER;
	}

	return setServer(serverTypeConverted, tilezmin, tilezmax, log2tilesiz, iswebmerc, ishttps, isquadtree, servnum, servcharoffs, cache_st, url_st, tile_st, rmul, gmul, bmul);

}

float DemClass::drawGlobe()
{
	if (voxiePtr == nullptr) return 0.0f;


	voxiePtr->getVoxieFrame()->flags |= VOXIEFRAME_FLAGS_IMMEDIATE; //<-Mandatory for internal multi threading!
	
	updateGlobe();
	this->globalFSc = demview_drawglobe(voxiePtr->getVoxieWindow(), voxiePtr->getVoxieFrame(), &this->dpos, &this->drig, &this->ddow, &this->dfor, this->fexagz, this->resthresh, this->demRenderFlag, &dgs);
	return  globalFSc;
}

void DemClass::setPos(dpoint3d dpos, dpoint3d drig, dpoint3d ddow, dpoint3d dfor)
{
	this->dpos.x = dpos.x;
	this->dpos.y = dpos.y;
	this->dpos.z = dpos.z;

	this->drig.x = drig.x;
	this->drig.y = drig.y;
	this->drig.z = drig.z;

	this->ddow.x = ddow.x;
	this->ddow.y = ddow.y;
	this->ddow.z = ddow.z;

	this->dfor.x = dfor.x;
	this->dfor.y = dfor.y;
	this->dfor.z = dfor.z;

}
void DemClass::getPos(dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor)
{
	dpos->x = this->dpos.x;
	dpos->y = this->dpos.y;
	dpos->z = this->dpos.z;

	drig->x = this->drig.x;
	drig->y = this->drig.y;
	drig->z = this->drig.z;

	ddow->x = this->ddow.x;
	ddow->y = this->ddow.y;
	ddow->z = this->ddow.z;

	dfor->x = this->dfor.x;
	dfor->y = this->dfor.y;
	dfor->z = this->dfor.z;
}
point3d DemClass::getColMultiplier()
{
	return this->colMultiplier;
}
void DemClass::setColMultiplier(float rMulti, float gMulti, float bMulti)
{
	this->colMultiplier.x = rMulti;
	this->colMultiplier.y = gMulti;
	this->colMultiplier.z = bMulti;

	if (currentColorServer.isSet) {
		this->setServer(currentColorServer.serverType, currentColorServer.tilezmin, currentColorServer.tilezmax, currentColorServer.log2tilesiz, currentColorServer.iswebmerc, currentColorServer.ishttps, currentColorServer.isquadtree, currentColorServer.servnum, currentColorServer.servcharoffs, currentColorServer.cache_st, currentColorServer.url_st, currentColorServer.tile_st, colMultiplier.x, colMultiplier.y, colMultiplier.z);
	}
}

void DemClass::rotateCamera(float angleMovement) {

	angleMovement *= 0.025; 
	angleMovement* voxiePtr->getDeltaTime();

	this->rotatedAngle += (angleMovement);
	rotDPVex(angleMovement, &ddow, &drig);

}

void DemClass::setDemRotatedAngle(float radian )
{


	// remove current angle to zero it out....
	rotDPVex(-rotatedAngle, &ddow, &drig);

	// now that the rotation is zero'd out apply new angle...
	rotDPVex(radian, &ddow, &drig);
	this->rotatedAngle = radian;

}

// adjusts where the sea level of the display
void DemClass::adjustCameraZ(float amountZ)
{
	amountZ *= voxiePtr->getDeltaTime();
	fhakup += amountZ;
}

void DemClass::moveCamera(float amountX, float amountY, float amountZ)
{
	float f = altradii * voxiePtr->getDeltaTime();
	point3d am = { amountX, amountY, amountZ };

	dpos.x += (drig.x * am.x + ddow.x * am.y + dfor.x * am.z) * f;
	dpos.y += (drig.y * am.x + ddow.y * am.y + dfor.y * am.z) * f;
	dpos.z += (drig.z * am.x + ddow.z * am.y + dfor.z * am.z) * f;

}
void DemClass::sph2xyz(double lon_deg, double lat_deg, double altradii, dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor)
{
	demview_sph2xyz(lon_deg, lat_deg, altradii, dpos, drig, ddow, dfor);
}
void DemClass::drawSprite(const char* filnam, double lon_deg, double lat_deg, double alt_m, double spr_yaw_deg, double spr_pit_deg, double spr_rol_deg, double fscale, double fexagz_pos, double fexagz_sca, int col)
{

	// TODO. are fscale, fexagz_pos, fexagz_sca needed?
	demview_drawspr(voxiePtr->getVoxieFrame(), filnam, lon_deg, lat_deg, alt_m, spr_yaw_deg, spr_pit_deg, spr_rol_deg, fscale, fexagz_pos, fexagz_sca, col);
}

void DemClass::setHeighPalIMG(const char* filename, float transparency) {

	if (voxiePtr == nullptr) return;

	if (voxiePtr->_kpzload(filename, &HeightPal.f, &HeightPal.p, &HeightPal.x, &HeightPal.y)) { // if file name is found turn fill the contents of a tile type
		demview_setheipal((int*)HeightPal.f, HeightPal.x, -10000.f, +10000.f, transparency);
	}


}

void DemClass::setHeightPalColArray(int* colArray, int colArrayLength, float alt_mStartRange, float alt_mEndRange, float transparency)
{
	demview_setheipal(colArray, colArrayLength, alt_mStartRange, alt_mEndRange, transparency);
}

void DemClass::clearHeightPal() {

	demview_setheipal(0, -1, -10000.f, +10000.f, 0);
}

void DemClass::adjustHeightEx(float adjustmentAmount)
{
	int i = 0;


	// Always make this adjustment from a pow of two
	if (adjustmentAmount > 0) {
		(int)adjustmentAmount + 1;
		for (i = 0; i < adjustmentAmount; i++) {
			fexagz *= pow(2.0, voxiePtr->getDeltaTime());
		}
	}
	else if (adjustmentAmount < 0) {
		(int)adjustmentAmount + 1;
		for (i = 0; i < fabs(adjustmentAmount); i++) {
			fexagz /= pow(2.0, voxiePtr->getDeltaTime());
		}
	}

	if (fexagz > HEIGHT_EXAGRATION_MAX) fexagz = HEIGHT_EXAGRATION_MAX;
	if (fexagz < HEIGHT_EXAGRATION_MIN) fexagz = HEIGHT_EXAGRATION_MIN;
}
void DemClass::setHeightEx(float fexagz)
{
	if (fexagz > HEIGHT_EXAGRATION_MAX) fexagz = HEIGHT_EXAGRATION_MAX;
	if (fexagz < HEIGHT_EXAGRATION_MIN) fexagz = HEIGHT_EXAGRATION_MIN;

	this->fexagz = fexagz;
}

float DemClass::getHeightEx() {
	return this->fexagz;
}

void DemClass::setPosbyLonLatAlt(double lon_deg, double lat_deg, double altradii)
{
	demview_sph2xyz(lon_deg, lat_deg, altradii, &this->dpos, &this->drig, &this->ddow, &this->dfor);
}

// update DEM position from a Lon Lat
void DemClass::setPosbyLonLat(double lon_deg, double lat_deg) {

	demview_sph2xyz(lon_deg, lat_deg, this->altradii, &this->dpos, &this->drig, &this->ddow, &this->dfor);


}
void DemClass::setLon(double newLon)
{
	setPosbyLonLatAlt(newLon, this->lat, this->altradii);
}
void DemClass::setLat(double newLat)
{
	setPosbyLonLatAlt(this->lon, newLat, this->altradii);

}
void DemClass::setAltradii(double newAltRadii)
{
	setPosbyLonLatAlt(this->lon, this->lat, newAltRadii);
}
void DemClass::setAltM(double setAltM)
{
	setAltM = this->alt2altradii(setAltM);	
	setPosbyLonLatAlt(this->lon, this->lat, setAltM);
}
void DemClass::xyz2sph(dpoint3d* dpos, double* lon_deg, double* lat_deg, double* altradii)
{
	demview_xyz2sph(dpos, lon_deg, lat_deg, altradii);
}

void DemClass::getLonLatAlt(double* lon_deg, double* lat_deg, double* altradii) 
{
	demview_xyz2sph(&this->dpos, lon_deg, lat_deg, altradii);
}

double DemClass::getLon()
{
	return this->lon;
}

double DemClass::getLat()
{
	return this->lat;
}

double DemClass::getAltradii()
{
	return this->altradii;
}

double DemClass::getAltM()
{
	return this->alt_m;
}

void DemClass::drawGrid(int col, float height_m, bool showLonLat )
{

	double lon;
	double lat;
	double altradii;
	float alt_m = 0; // where to draw the grid, 0 = sea level
	getLonLatAlt(&lon, &lat, &altradii);

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
				((float)y - (float)GRDSIZ * .5f) * f + fcy, height_m * fexagz * (1.0 / EARTHRADMET), &grd[y][x]);
		}
	for (y = 0; y < GRDSIZ; y++)
	{
		for (x = 0; x < GRDSIZ - 1; x++) voxiePtr->drawLine(grd[y][x].x, grd[y][x].y, grd[y][x].z, grd[y][x + 1].x, grd[y][x + 1].y, grd[y][x + 1].z, col);
		if (!showLonLat) continue;
		sprintf_s(tbuf, "%g", ((float)y - (float)GRDSIZ * .5f) * f + fcy); g = (float)strlen(tbuf) * .5f;
		pr.x = 0.10f; pd.x = 0.00f; pp.x = grd[y][GRDSIZ >> 1].x - g * pr.x;
		pr.y = 0.00f; pd.y = 0.00f; pp.y = grd[y][GRDSIZ >> 1].y - g * pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[y][GRDSIZ >> 1].z - g * pr.z;
		voxiePtr->drawText(&pp, &pr, &pd, col, tbuf);
	}
	for (x = 0; x < GRDSIZ; x++)
	{
		for (y = 0; y < GRDSIZ - 1; y++) voxiePtr->drawLine(grd[y][x].x, grd[y][x].y, grd[y][x].z, grd[y + 1][x].x, grd[y + 1][x].y, grd[y + 1][x].z, col);
		if (!showLonLat) continue;
		sprintf_s(tbuf, "%g", ((float)x - (float)GRDSIZ * .5f) * f + fcx); g = (float)strlen(tbuf) * .5f;
		pr.x = 0.00f; pd.x = 0.00f; pp.x = grd[GRDSIZ >> 1][x].x - g * pr.x;
		pr.y = 0.10f; pd.y = 0.00f; pp.y = grd[GRDSIZ >> 1][x].y - g * pr.y;
		pr.z = 0.00f; pd.z = 0.15f; pp.z = grd[GRDSIZ >> 1][x].z - g * pr.z;
		voxiePtr->drawText(&pp, &pr, &pd, col, tbuf);
	}

}

// rotates a double point (dpoint3d) around a vector a&b common play, by angle
void DemClass::rotDPVex(double ang, dpoint3d* a, dpoint3d* b)
{
	double c, s, f;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f * c + b->x * s; b->x = b->x * c - f * s;
	f = a->y; a->y = f * c + b->y * s; b->y = b->y * c - f * s;
	f = a->z; a->z = f * c + b->z * s; b->z = b->z * c - f * s;
}



double DemClass::alt2altradii(double alt_m, bool addHeightEx)
{
	float heightEx = 1;
	if (addHeightEx) heightEx = fexagz;
	return  alt_m * heightEx * (1.0 / EARTHRADMET);
}

void DemClass::setDemRenderFlag(int newValue)
{
	this->demRenderFlag = newValue;
}

double DemClass::altradii2alt(double altradii)
{
	return altradii * EARTHRADMET * 1.f;
}

point3d DemClass::sph2voxie(double lon_deg, double lat_deg, double altradii, point3d* voxiepos)
{
	demview_sph2voxie(lon_deg, lat_deg, altradii, voxiepos);
	point3d returnValue = { voxiepos->x, voxiepos->y, voxiepos->z };
	return returnValue;
}

point3d DemClass::xyz2voxie(dpoint3d* dpos, point3d* voxiepos)
{
	demview_xyz2voxie(dpos, voxiepos);
	point3d returnValue = { voxiepos->x, voxiepos->y, voxiepos->z };
	return returnValue;
}

void DemClass::sph2voxie2(double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, point3d* voxiepos, point3d* pr, point3d* pd, point3d* pf) {
	
	demview_sph2voxie2(lon_deg, lat_deg, alt_m, yaw_deg, pit_deg, rol_deg, fscale, fexagz_pos, fexagz_sca, voxiepos, pr, pd, pf);

}

dpoint3d DemClass::voxie2xyz(point3d* voxiepos, dpoint3d* dpos)
{
	demview_voxie2xyz(voxiepos, dpos);
	dpoint3d returnValue = { dpos->x, dpos->y, dpos->z };
	return returnValue;
}

void DemClass::voxie2sph(point3d* voxie, double* lon_deg, double* lat_deg, double* altradii) {

	demview_voxie2sph(voxie, lon_deg, lat_deg, altradii);
}


void DemClass::setTopBotAltm(double dtopm, double dbotm)
{
	if (voxiePtr == nullptr) return;

	double d, daltm;

	daltm = (sqrt(dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z) - 1.0) * EARTHRADMET;
	d = voxiePtr->getAspectZ() * 2.0 / (dtopm - dbotm);
	(fexagz) = daltm * d;
	(this->dgs.fhakzadd) = dbotm * d + voxiePtr->getAspectZ();
	if (voxiePtr->getVoxieWindow()->nblades >= 2) {  (fexagz) *= 0.9325; (this->dgs.fhakzadd) -= 1.0625f; } //these are hack constants for spinner!
	else { (fexagz) *= 1.15; (this->dgs.fhakzadd) -= 0.87f; } //these are these hack constants for VX1!
}

void DemClass::getTopBotAltm(double* dtopm, double* dbotm)
{
	if (voxiePtr == nullptr) return;

	double d, daltm;

	daltm = (sqrt(dpos.x * dpos.x + dpos.y * dpos.y + dpos.z * dpos.z) - 1.0) * EARTHRADMET;

	if (voxiePtr->getVoxieWindow()->nblades >= 2) { fexagz /= 0.9325; this->dgs.fhakzadd += 1.0625f; } // these hack constants for spinner
	else { fexagz /= 1.15; this->dgs.fhakzadd += 0.87f; } // these are these hack constants? for VX1!
	d = daltm / fexagz;

	(*dbotm) = (this->dgs.fhakzadd - voxiePtr->getVoxieWindow()->aspz) * d;
	(*dtopm) = voxiePtr->getVoxieWindow()->aspz * d * 2.0 + (*dbotm);


}


int DemClass::getDemRenderFlag()
{
	return this->demRenderFlag;
}

void DemClass::report(int posX, int posY)
{
	if (voxiePtr == nullptr) return;
	voxiePtr->debugText(posX + 20, posY, 0xffff00, -1, "DemClass Report");
	voxiePtr->debugText(posX + 120, posY + 10, 0xffffff, -1, "Lon: %f\nLat: %f\nAlt_m: %f\n fsc %f", lon, lat, alt_m, this->globalFSc);
	voxiePtr->debugText(posX + 20, posY + 10, 0xffffff, -1, "zmin: %f\nzmax: %f\nzavg: %f\nzstd: %f\nznum: %i\n", dgs.fzmin, dgs.fzmax, dgs.fzsum / (float)dgs.fzcnt, sqrt(fabs(dgs.fzsum2 - dgs.fzsum * dgs.fzsum / (double)dgs.fzcnt) / (double)(dgs.fzcnt - 1)), dgs.fzcnt);
	voxiePtr->debugText(posX, posY + 58, 0xffffff, -1, "hakzadd: %f demRenderFlags: %d seaLock: %d \n\n", dgs.fhakzadd, demRenderFlag, seaLock);

	voxiePtr->debugText(posX, posY + 68, 0xffffff, -1, "alt_m %1.3f altradii %1.3f rotated Angle: %f\n\n", this->alt_m, this->altradii, this->rotatedAngle);
	voxiePtr->debugBar(posX, posY, (double)this->getHeightEx(), HEIGHT_EXAGRATION_MAX, HEIGHT_EXAGRATION_MIN, (char*)"Height EX", 2);


	// server info

	voxiePtr->debugText(posX, posY + 100, 0x00ffff, -1, "Server Height: Type:%d TileZMin:%d TileZMax:%d Log2Tile:%d isWebMerc:%d\nisHttps:%d isQuadTree: %d Servnum: %d servCharOffS: %d isSet: %d,\ncache_st: %s\nurl_st: %s\ntile_st: %s\n",
		(int)currentHeightServer.serverType, currentHeightServer.tilezmin, currentHeightServer.tilezmax, currentHeightServer.log2tilesiz, currentHeightServer.iswebmerc,
		currentHeightServer.ishttps, currentHeightServer.isquadtree, currentHeightServer.servnum, currentHeightServer.servcharoffs, currentHeightServer.isSet,
		currentHeightServer.cache_st, currentHeightServer.url_st, currentHeightServer.tile_st);

	voxiePtr->debugText(posX, posY + 150, 0xff00ff, -1, "Server Color: Type:%d TileZMin:%d TileZMax:%d Log2Tile:%d isWebMerc:%d\nisHttps:%d isQuadTree: %d Servnum: %d servCharOffS: %d isSet: %d,\ncache_st: %s\nurl_st: %s\ntile_st: %s\n",
		(int)currentColorServer.serverType, currentColorServer.tilezmin, currentColorServer.tilezmax, currentColorServer.log2tilesiz, currentColorServer.iswebmerc,
		currentColorServer.ishttps, currentColorServer.isquadtree, currentColorServer.servnum, currentColorServer.servcharoffs, currentColorServer.isSet,
		currentColorServer.cache_st, currentColorServer.url_st, currentColorServer.tile_st);

}	


double DemClass::getDemRotatedAngle()
{
	return this->rotatedAngle;
}
