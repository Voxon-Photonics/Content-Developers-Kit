#include "HSlider2D.h"



HSlider2D::HSlider2D(IVoxieBox * voxiePtr, int posX, int posY, int length, float maxValue, float minValue, float incrementAmount, int color)
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

HSlider2D::~HSlider2D()
{
}

int HSlider2D::isTouched()
{
	if (!isDraggable) return -1;

	point2d markerPos = { makerPos, (float)this->posY,  };

	if (voxiePtr->getTouchPressStateCir(markerPos, slideRadius) >= 1) {

		lockedIndex = voxiePtr->getTouchPressIndexCir(markerPos, 35);
		posXOffset = voxiePtr->getTouchPosX(lockedIndex) - posX;
		posYOffset = voxiePtr->getTouchPosY(lockedIndex) - posY;

	}
	else if (voxiePtr->getTouchState(lockedIndex) != 2 || isLockable == false) { // if the touch falls out of the slider forget about it!
		lockedIndex = -1;
	}

	return voxiePtr->getTouchPressStateCir(markerPos, 35);
}

void HSlider2D::draw()
{
	float l = lenght * .5;
	float pix = lenght / (maxValue - minValue);
	//makerPos = posX + (l)-((value - minValue) * pix);

	makerPos = posX - (l)+((value - minValue) * pix);


	int i = 0;
	int markerCol = voxiePtr->brightenCol(color, 20);
	
	
	voxiePtr->debugDrawLine(posX - (l), posY, posX + (l), posY, color);

	voxiePtr->debugDrawLine(posX + (l), posY - 10, posX + (l), posY + 10, 0x00ffff);				// top marker
	voxiePtr->debugText(posX + (l), posY + 25, 0x9d9d9d, -1, "%1.1f", maxValue);

	voxiePtr->debugDrawLine(posX , posY - 5, posX, posY + 6, color);								// center
	voxiePtr->debugText(posX , posY + 25, 0x9d9d9d, -1, "%1.1f", (minValue + maxValue) * .5);

	voxiePtr->debugDrawLine(posX - (l), posY - 10 , posX - (l), posY + 10 , 0xff0000);				// bottom marker
	voxiePtr->debugText(posX - (l), posY + 25, 0x9d9d9d, -1, "%1.1f", minValue);


	// debug increment about to pix
	//voxiePtr->debugText(500, 100, 0x00ffff, -1, "pix %1.2f, value %1.2f", pix, value);

	voxiePtr->debugDrawCircFill(makerPos, posY, slideRadius, 0xffffff);
	voxiePtr->debugDrawCircFill(makerPos, posY, slideRadius - sliderBorder, color);

	voxiePtr->debugText(makerPos, posY + 25, markerCol, -1, "%1.1f", value);

	//	voxiePtr->debugDrawCircFill()
}

int HSlider2D::update()
{

	isTouched();

	float l = lenght * .5;
	float pix = lenght / (maxValue - minValue);


	if (lockedIndex != -1) {

		if (voxiePtr->getTouchState(lockedIndex) == 0) lockedIndex = -1;

		if (voxiePtr->getTouchState(lockedIndex) >= 1) {

			//	this->posX = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;

			if (voxiePtr->getTouchPosX(lockedIndex) > posX - (l) && voxiePtr->getTouchPosX(lockedIndex) < posX + (l)) {

				value = (posX + (l) - (voxiePtr->getTouchPosX(lockedIndex)) * incrementAmount);
				value /= pix;
				value = maxValue - value;

			}

		}
	}

	if (minValue > maxValue) {

		l = maxValue;
		maxValue = minValue;
		minValue = l;

	}

	//if (value > maxValue)	value = maxValue;
	//if (value < minValue)	value = minValue;

	return 0;
}

void HSlider2D::setMinValue(float newValue)
{
	minValue = newValue;
}

void HSlider2D::setMaxValue(float newValue)
{
	maxValue = newValue;
}

void HSlider2D::setValue(float newValue)
{
	value = newValue;
}

float HSlider2D::getMinValue()
{
	return minValue;
}

float HSlider2D::getMaxValue()
{
	return maxValue;
}

float HSlider2D::getValue()
{
	return value;
}
