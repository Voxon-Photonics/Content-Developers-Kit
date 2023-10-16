#pragma once
#ifndef VxMoxieClass
#define VxMoxieClass
#include "../../../../Headers and DLLs/vxCpp.h"

/*
 * VxMoxie based on Ken Silverman's 'Moxie.h' allows translation, rotation, and scale of all 'moxie'  objects in scene together
 * Basically all sorts of models / Vxprimiatives  can be drawn using the same world space which can be rotated around.
 *
 * Ken's notes on integrating the system
 * //Step 1: create a VxMoxie class and pass in a IVoxieBox Ptr
 * //Step 2: call setcam() with the appropriate camera parameters
 * //Step 3: replace any voxie() calls with moxie() to apply the translation
 *
 * Implementation by Matthew Vecchio for Voxon 6/6/2022
 *
 *
 * TODO: zoomCamIn and zoomCamOut functions are at a fixed speed would be better to allow the user to define the zoom amount
 *
 *
 */
class VxMoxie : public IVoxiePtr {

public:

	// constructors

	VxMoxie();
	VxMoxie(IVoxieBox* voxiePtr);
	~VxMoxie();

	// core functions 

	void setIVoxieBoxPtr(IVoxieBox* voxiePtr);
	IVoxieBox* getIVoxieBoxPtr();

	void setCam(point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector);

	void xformPos(point3d* p);
	void xformOri(point3d* r, point3d* d, point3d* f, point3d* nr, point3d* nd, point3d* nf);

	void setCamPos(float x, float y, float z);
	void setCamPos(point3d* pos);

	void zoomCamIn(float amount = 2);
	void zoomCamOut(float amount = 0.5);

	void rotCam(float xAmount, float yAmount, float zAmount);
	void setRotation(float xRad, float yRad, float zRad, bool isDegrees);
	void moveCam(float xAmount, float yAmount, float zAmount);
	void movePivot(float xAmount, float yAmount, float zAmount);

	point3d getCamPos();
	float getCamPosX();
	float getCamPosY();
	float getCamPosZ();
	point3d getCamRVec();
	point3d getCamDVec();
	point3d getCamFVec();

	float getScale();
	void setZoom(float amount);

	void	setPivot(point3d pos);
	point3d getPivot();
	
	void    resetRotation();


	// useful functions

	void drawPivot(float rad, int fillMode, int col);

	// converters  
	// give a voxie position and get back a position within the moxie world cam
	point3d voxie2MoxiePos(point3d pos);



	// vxCpp drawing calls

	void drawVox(float x, float y, float z, int col);
	void drawVox(point3d pos, int col);

	void drawSphere(float x, float y, float z, float rad, int type, int col);
	void drawSphere(point3d pos, float rad, int type, int col);

	void drawLine(float x0, float y0, float z0, float x1, float y1, float z1, int col);
	void drawLine(point3d pos0, point3d pos1, int col);

	void drawCube(point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int fillmode, int col);
	void drawBox(point3d LTU, point3d RBD, int fillmode, int col);
	void drawBox(float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
	float drawHeightMap(char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int colorkey, int reserved, int flags);
	void drawDicom(voxie_dicom_t* vd, const char* gfilnam, point3d* p, point3d* r, point3d* d, point3d* f, int* animn, int* loaddone);

	void drawCone(float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
	void drawCone(point3d pos0, float r0, point3d pos1, float r1, int fillmode, int col);

	void drawPoly(pol_t* pt, int n, int col);

	void drawMesh(char* fileName, poltex_t* verticeList, int verticeNum, int* meshList, int meshNum, int flags, int col = 0xffffff);

	int drawModel(const char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int col);
	int drawModelExt(const char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int col, float forcescale, float fdrawratio, int flags);
	void drawText(point3d* p, point3d* r, point3d* d, int col, const char* st);
	void drawTextExt(point3d* p, point3d* r, point3d* d, int col, const char* fmt, ...);


	// debug
	void report(int posx, int posy, bool showPivot = true);

	// settings
	void	setEnableScrollPivot(bool newValue);
	void	setMovementSpeed(float newValue);
	float	getMovementSpeed();
	void	setMovementSpeedPiv(float newValue);
	float	getMovementSpeedPiv();

private:

	float normalise();
	void scaleUpdate();

	// behavior settings
	bool isScrollPivotEnabled = false; // if true 
	void transposeMovCam(float x, float y, float z);

	point3d cPos = { 0,0,0 };
	point3d cRVec = { 1,0,0 };
	point3d cDVec = { 0,1,0 };
	point3d cFVec = { 0,0,1 };
	point3d piv = { 0,0,0 };

	float movementSpeedPiv = 1;
	float movementSpeed = 1;
	float scale = 1; // used for spheres and cones  
	bool debug = true;


};

#endif


