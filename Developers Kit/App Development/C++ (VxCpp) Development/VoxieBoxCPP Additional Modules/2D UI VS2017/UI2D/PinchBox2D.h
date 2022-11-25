#pragma once
#include "vxCpp.h"

// HAT POS

enum HAT_POSITIONS {

	NONE        = 0,
	TOP_LEFT	= 1,
	TOP_UP		= 2,
	TOP_RIGHT	= 3,
	MID_LEFT	= 4,
	MID_CENTRE  = 5,
	MID_RIGHT	= 6,
	BOT_LEFT	= 7,
	BOT_CENTRE  = 8,
	BOT_RIGHT	= 9,

};


class PinchBox2D : public IActive, public IVoxiePtr, public IUI2D
{

public:
	
	PinchBox2D(IVoxieBox * voxiePtr, int posX, int posY, int width, int length, int col);
	~PinchBox2D();

	int		isTouched();
	int		getTouchState();
	bool	hasDoubleTouched();
	bool	isPinchActive();
	int		update();
	void	draw();

	// !!IMPORTANT this is how many touch point need to be within the pinchbox for a change to be registered. (0 - any pinch will react, 1 - only 1 point needs to be inside, 2 both need to be inside)
	void	setMinPinchResponse(int newValue);
	void	setBrightOnTouch(bool option);

	float	getPinchRotationDelta();
	float	getPinchDistanceDelta(); 
	int		getRelPositionX();
	int		getRelPositionY();
	int		getGlobalDeltaX();
	int		getGlobalDeltaY();
	int		getMinPinchResponse();
	int		getHatPos();
	int     getRSingleTouchX();
	int     getRSingleTouchY();
	int     getGSingleTouchX();
	int     getGSingleTouchY();
	int     getDeltaSingleTouchX();
	int     getDeltaSingleTouchY();
	int		hatPosUpdate();

private:
	   
	int     rSTouchX = 0,		rSTouchY = 0;
	int     gSTouchX = 0,		gSTouchY = 0;
	int		dSTouchX = 0,		dSTouchY = 0;
	int		pinchPosRelX = 0,	pinchPosRelY = 0; 
	int		hatPos				= -1;
	int		lockedIndex			= -1;
	int		gDeltaX		= 0,	gDeltaY = 0;
	int		width		= 0,	length = 0;
	int		color				= 0x303030;
	bool	pinchActive			= false;
	bool	debug				= true;

	float	pinchDistanceDelta	= 0;
	float	pinchRotationDelta	= 0;
	bool	brightenOnTouch		= true;
	int		pinchesInside		= 0;
	int		activeCol			= 0x303030;
	int		minPinchResponse	= 2; 
	double  doubleTouchThreshold = .25;
	double  lastTouchTime = 0;
	bool	doubleTouch = false;
	int	touchState = 0;

};
