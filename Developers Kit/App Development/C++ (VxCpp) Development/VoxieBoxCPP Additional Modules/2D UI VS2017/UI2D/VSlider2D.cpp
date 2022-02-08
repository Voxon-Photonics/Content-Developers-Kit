#include "VSlider2D.h"


VSlider2D::VSlider2D(IVoxieBox * voxiePtr, int posX, int posY, int length, double maxValue, double minValue, double incrementAmount, int color)
{
	this->voxiePtr = voxiePtr;

	this->posX = posX;
	this->posY = posY;
	this->lenght = length;

	this->maxValue = maxValue;
	this->minValue = minValue;
	this->value = (minValue + maxValue) * .5;

	this->incrementAmount = incrementAmount;	   
	
	this->color = color;
}

VSlider2D::~VSlider2D()
{
}

int VSlider2D::isTouched()
{
	if (!isDraggable) return -1;

	point2d markerPos = { (double)this->posX, makerPos };

	if (voxiePtr->getTouchPressStateCir(markerPos, slideRadius) >= 1) {

		lockedIndex = voxiePtr->getTouchPressIndexCir(markerPos, 35);
		posXOffset = voxiePtr->getTouchPosX(lockedIndex) - posX;
		posYOffset = voxiePtr->getTouchPosY(lockedIndex) - posY;

	}
	else if (voxiePtr->getTouchState(lockedIndex) == 0 || isLockable == false) { // if the touch falls out of the slider forget about it!
		lockedIndex = -1; 
	}

	return voxiePtr->getTouchPressStateCir(markerPos, 35);
}

void VSlider2D::draw()
{
	double l = lenght * .5;
	double pix = lenght / (maxValue - minValue);
	makerPos = posY + (l)-((value - minValue) * pix);
	int i = 0;
	int markerCol = voxiePtr->brightenCol(color, 20);
	voxiePtr->debugDrawLine(posX, posY - (l), posX, posY + (l), color);

	
	voxiePtr->debugDrawLine		(posX - 10, posY - (l), posX + 11, posY - (l), color);			// top marker
	voxiePtr->debugText			(posX + 25, posY - (l), 0x9d9d9d, -1, "%1.1f", maxValue);

	voxiePtr->debugDrawLine		(posX - 5, posY, posX + 6, posY, color);						// center
	voxiePtr->debugText			(posX + 25, posY, 0x9d9d9d, -1, "%1.1f", (minValue + maxValue) * .5);
	   	
	voxiePtr->debugDrawLine		(posX - 10, posY + (l), posX + 11, posY + (l), color);			// bottom marker
	voxiePtr->debugText			(posX + 25, posY + (l), 0x9d9d9d, -1, "%1.1f", minValue);


	// debug increment about to pix
	//voxiePtr->debugText(500, 200, 0xff00ff, -1, "pix %1.2f, value %1.2f", pix, value);

	voxiePtr->debugDrawCircFill(posX, makerPos, slideRadius, 0xffffff);
	voxiePtr->debugDrawCircFill(posX, makerPos, slideRadius - sliderBorder, color);
		
	voxiePtr->debugText(posX + 25, makerPos, markerCol, -1, "%1.1f", value);

//	voxiePtr->debugDrawCircFill()
}

int VSlider2D::update()
{
	double l = lenght * .5;
	double pix = lenght / (maxValue - minValue);

	isTouched();

	if (lockedIndex != -1) {

		if (voxiePtr->getTouchState(lockedIndex) == 0) lockedIndex = -1;

		if (voxiePtr->getTouchState(lockedIndex) >= 1) {

		//	this->posX = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;

			if (voxiePtr->getTouchPosY(lockedIndex) > posY - (l) && voxiePtr->getTouchPosY(lockedIndex) < posY + (l)) {

				
				value = (posY + (l) - (voxiePtr->getTouchPosY(lockedIndex)) * incrementAmount);
				value /= pix;
				value += minValue;
			
			}

		}
	}

	if (minValue > maxValue) {

		l = maxValue;
		maxValue = minValue;
		minValue = l;

	}

	if (value > maxValue)	value = maxValue;
	if (value < minValue)	value = minValue;

	return 0;
}

void VSlider2D::setMinValue(double newValue)
{
	minValue = newValue;
}

void VSlider2D::setMaxValue(double newValue)
{
	maxValue = newValue;
}

void VSlider2D::setValue(double newValue)
{
	value = newValue;
}

float VSlider2D::getMinValue()
{
	return minValue;
}

float VSlider2D::getMaxValue()
{
	return maxValue;
}

float VSlider2D::getValue()
{
	return value;
}
