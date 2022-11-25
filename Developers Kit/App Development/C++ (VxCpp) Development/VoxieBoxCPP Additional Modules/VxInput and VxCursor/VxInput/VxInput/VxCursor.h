#pragma once

#include "vxCpp.h"

/*** A simple VX class to handle a draw cursor 
 *  10/06/2022 by Matthew Vecchio for Voxon Photonics 
 */

class VxCursor : IDrawable
{
public:

	VxCursor(IVoxieBox* voxiePtr);
	~VxCursor();

	void	setPos(point3d newPos);
	point3d	getPos();

	void	draw();

	point3d movePos(point3d directionXYZ);
	point3d movePos(float X, float Y, float Z);

	bool	isClipEnabled();
	void	setEnabledClip(bool choice);
	bool	colCheck(point3d colPos, float radius);

	float	getMovementSpeed();
	void	setMovementSpeed(float newSpeedValueXY, float newSpaceValueZ);

	void	startPing();
	void	setFlashing(bool flashOn, int amount = -1);

	void	setDrawable(bool option);
	bool    isDrawableEnabled();

	void	setDrawState(int newState);
	int		getDrawState();

	void	report(int posX, int posY);

private:

	
	// user settings

	// set your color values here
	int centreCol = 0xff0000;
	int cursorCol = 0xffffff;
	int hiLightCol = 0x00ff00;
	int lineCol = 0xff0000;
	int pingCol = 0xffffff;

	float cursorMovementSpeedXY = 1;
	float cursorMovementSpeedZ = 0.6;


	float cursorRadius = 0.033; // the radius / size of the cursor

	float pingSpeed = 5;

	bool enableClipping = true;

	bool isFlashing = false;
	bool isDrawable = true;
	double flashTimeDelay = 0;
	double flashTimeDuration = 0;

	int flashAmountNo = 0;

	int drawState = 0; //0 normal //1 pressed //2 hi-light // 3 NO  

	float const PING_MAX = 50;
	float ping = PING_MAX;
	void	pingDraw();

	point3d pos = { 0,0,0 };

};

