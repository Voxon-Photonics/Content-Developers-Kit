#include "PinchBox2D.h"

PinchBox2D::PinchBox2D(IVoxieBox * voxiePtr, int posX, int posY, int width, int length, int col)
{
	this->voxiePtr = voxiePtr;

	this->posX = posX;
	this->posY = posY;
	this->length = length;
	this->width = width;

	this->color = color;
	activeCol = voxiePtr->brightenCol(color, 20);
}


PinchBox2D::~PinchBox2D()
{
}

int PinchBox2D::isTouched()
{
	point2d tl = { posX, posY };
	point2d br = { posX + length, posY + width };
	pinchesInside = 0;
	gDeltaX = 0;
	gDeltaY = 0;
	int i = 0;

	int result = 0;
	this->pinchDistanceDelta = 0; 
	this->pinchRotationDelta = 0; 


	// workout single touch

	if (voxiePtr->getTouchPressState(tl, br) >= 1) {
		
		lockedIndex = voxiePtr->getTouchPressIndex(tl, br);
		gSTouchX = voxiePtr->getTouchPosX(lockedIndex); 
		gSTouchY = voxiePtr->getTouchPosY(lockedIndex);

		dSTouchX = voxiePtr->getTouchDeltaX(lockedIndex);
		dSTouchY = voxiePtr->getTouchDeltaY(lockedIndex);

		rSTouchX = voxiePtr->getTouchPosX(lockedIndex) - posX;
		rSTouchY = voxiePtr->getTouchPosY(lockedIndex) - posY;


		result = 1;


	}
	else {

		dSTouchX = 0;
		dSTouchY = 0;
		gSTouchX = -1;
		gSTouchY = -1;
		rSTouchX = -1;
		rSTouchY = -1;
		lockedIndex = -1;

	}



	//if (voxiePtr->getTouchPressState(tl, br) > 0 &&  voxiePtr->getTouchInputStruct()->pinchActive == true) {

	if  (voxiePtr->getTouchInputStruct()->pinchActive) {
	
		// check if pinch point 1 is within the box
		if (voxiePtr->getTouchInputStruct()->opinch0.x >= tl.x &&
			voxiePtr->getTouchInputStruct()->opinch0.y >= tl.y &&
			voxiePtr->getTouchInputStruct()->opinch0.x <= br.x &&
			voxiePtr->getTouchInputStruct()->opinch0.y <= br.y) {
			result = 1;
	
			pinchPosRelX = voxiePtr->getTouchInputStruct()->opinch0.x - tl.x;
			pinchPosRelY = voxiePtr->getTouchInputStruct()->opinch0.y - tl.y;
			i = voxiePtr->getTouchInputStruct()->pinch0Index;

			gDeltaX += voxiePtr->getTouchDeltaX(i);
			gDeltaY += voxiePtr->getTouchDeltaY(i);
			pinchesInside++;
		}



		// check if pinch point 2 is within the box
		if	(voxiePtr->getTouchInputStruct()->opinch1.x >= tl.x &&
			voxiePtr->getTouchInputStruct()->opinch1.y >= tl.y &&
			voxiePtr->getTouchInputStruct()->opinch1.x <= br.x &&
			voxiePtr->getTouchInputStruct()->opinch1.y <= br.y
			) {

			if (result == 0) {
				pinchPosRelX = voxiePtr->getTouchInputStruct()->opinch1.x - tl.x;
				pinchPosRelY = voxiePtr->getTouchInputStruct()->opinch1.y - tl.y;

			}

			i = voxiePtr->getTouchInputStruct()->pinch1Index;

			gDeltaX += voxiePtr->getTouchDeltaX(i);
			gDeltaY += voxiePtr->getTouchDeltaY(i);

			pinchesInside++;
			result = 2;
		}

	}

	if (result >= minPinchResponse) {

		pinchActive = true;

		this->pinchDistanceDelta = voxiePtr->getTouchInputStruct()->pinchDistanceDelta;
		this->pinchRotationDelta = voxiePtr->getTouchInputStruct()->pinchRotationDelta;

	}
	else {
		pinchPosRelX = 0;
		pinchPosRelY = 0;
		gDeltaY = 0;
		gDeltaX = 0;
	}

	return result;
}

bool PinchBox2D::isPinchActive()
{
	return this->pinchActive;
}

int PinchBox2D::update()
{
	isTouched();
	hatPosUpdate();

	return 0;
}

void PinchBox2D::draw()
{
	int x = posX;
	int y = posY;
	int w = posX + width;
	int l = posY + length;

	int col = color;

	if (lockedIndex  >= 0 && brightenOnTouch) col = activeCol;
	if (this->pinchesInside > 0 && brightenOnTouch) col =  voxiePtr->brightenCol(col, 10);
	voxiePtr->debugDrawBoxFill(x, y, w, l, col);



	if (debug) {

		voxiePtr->debugText(x, y, 0x00ff00, 0x111111, "SR Touch: X:%d Y:%d\nG Touch X:%d Y:%d\nDelta ST X:%d Y:%d \nRel Pinch Pos X:%d Y:%d\nGlo Pinch Pos X:%d Y:%d\nPinch Rot %1.2f Dis %1.2f\nHat %d MinPinch %d", 
		getRSingleTouchX(), getRSingleTouchY(), getGSingleTouchX(), 
		getGSingleTouchY(), getDeltaSingleTouchX(), getDeltaSingleTouchY(),
		getRelPositionX(), getRelPositionY(), getGlobalDeltaX(), getGlobalDeltaY(), 
		getPinchRotationDelta(), getPinchDistanceDelta(), getHatPos(), getMinPinchResponse());

	}

}

void PinchBox2D::setMinPinchResponse(int newValue)
{
	this->minPinchResponse = newValue;
}

void PinchBox2D::setBrightOnTouch(bool option)
{
	this->brightenOnTouch = option;
}

float PinchBox2D::getPinchRotationDelta()
{
	return this->pinchRotationDelta;
}

float PinchBox2D::getPinchDistanceDelta()
{
	return this->pinchDistanceDelta;
}

int PinchBox2D::getRelPositionX()
{
	return this->pinchPosRelX;
}

int PinchBox2D::getRelPositionY()
{
	return this->pinchPosRelY;
}

int PinchBox2D::getGlobalDeltaX()
{
	return this->gDeltaX;
}

int PinchBox2D::getGlobalDeltaY()
{
	return this->gDeltaY;
}

int PinchBox2D::getMinPinchResponse()
{
	return this->minPinchResponse;
}

int PinchBox2D::getHatPos()
{
	return hatPos;
}

int PinchBox2D::getRSingleTouchX()
{
	return rSTouchX;
}

int PinchBox2D::getRSingleTouchY()
{
	return rSTouchY;
}

int PinchBox2D::getGSingleTouchX()
{
	return gSTouchX;
}

int PinchBox2D::getGSingleTouchY()
{
	return gSTouchY;
}

int PinchBox2D::getDeltaSingleTouchX()
{
	return dSTouchX;
}

int PinchBox2D::getDeltaSingleTouchY()
{
	return dSTouchY;
}

int PinchBox2D::hatPosUpdate()
{
	/*if (lockedIndex <= 0) {
		hatPos = NONE;
		return;
	}*/
	
	int tx = gSTouchX;
	int ty = gSTouchY;
	
	int topVLine =		posY + ( length / 3);
	int leftHLine =		posX + ( width / 3);

	int botVLine =		(posY + length) - (length / 3);
	int rightHLine =	(posX + width) - (width / 3);


	hatPos = NONE;

	// left top
	if (tx < leftHLine && ty < topVLine && tx > posX && ty > posY) {
		hatPos = TOP_LEFT;
	} 
	else if (tx < rightHLine && ty < topVLine && tx > leftHLine && ty > posY) {
		hatPos = TOP_UP;
	} else if (tx < posX + length && ty < topVLine && tx > rightHLine && ty > posY) {
		hatPos = TOP_RIGHT;
	}

	// middles
	if (tx < leftHLine && ty < botVLine && tx > posX && ty > topVLine) {
		hatPos = MID_LEFT;
	}
	else if (tx < rightHLine && ty < botVLine && tx > leftHLine && ty > topVLine) {
		hatPos = MID_CENTRE;
	} 
	else if (tx < posX + length && ty < botVLine && tx > rightHLine && ty > topVLine) {
		hatPos = MID_RIGHT;
	}

	// right 
	if (tx < leftHLine && ty < posY + length && tx > posX && ty > botVLine) {
		hatPos = BOT_LEFT;
	}
	else if (tx < rightHLine && ty < posY + length && tx > leftHLine && ty > botVLine) {
		hatPos = BOT_CENTRE;
	}
	else if (tx < posX + length && ty < posY + length && tx > rightHLine && ty > botVLine) {
		hatPos = BOT_RIGHT;
	}

	if (debug) {

		// top H
		voxiePtr->debugDrawLine(posX, topVLine, posX + width, topVLine, 0xff00ff);

		// bot H
		voxiePtr->debugDrawLine(posX, botVLine, posX + width, botVLine, 0x00ffff);

		// left V
		voxiePtr->debugDrawLine(leftHLine, posY, leftHLine, posY + length, 0xff00ff);

		// right V
		voxiePtr->debugDrawLine(rightHLine, posY, rightHLine, posY + length, 0x00ffff);

	}


	return hatPos;

}


