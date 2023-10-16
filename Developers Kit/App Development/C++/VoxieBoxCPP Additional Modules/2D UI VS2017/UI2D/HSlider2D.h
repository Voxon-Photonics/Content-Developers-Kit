#pragma once
#include "vxCpp.h"

#define HSLIDER_RADUS 20

class HSlider2D : public IActive, public IVoxiePtr, public IUI2D
{
public:
	HSlider2D(IVoxieBox * voxiePtr, int posX, int posY, int length, float maxValue, float minValue, float incrementAmount, int color);
	~HSlider2D();

	int isTouched();
	int update();
	void draw();

	void setMinValue(float newValue);
	void setMaxValue(float newValue);
	void setValue(float newValue);

	float getMinValue();
	float getMaxValue();
	float getValue();
	


private:
	IVoxieBox	*voxiePtr = 0;

	float		maxValue;
	float		minValue;
	float		incrementAmount;
	float		value;
	float		makerPos = 0;

	bool		isDraggable = true;
	bool		isLockable = false;

	int			lenght;
	int			lockedIndex = -1;
	int			posXOffset = 0;
	int			posYOffset = 0;
	int			style = 1;
	int			color = 0x303030;
	int			slideRadius = HSLIDER_RADUS;
	int			sliderBorder = 3;
};

