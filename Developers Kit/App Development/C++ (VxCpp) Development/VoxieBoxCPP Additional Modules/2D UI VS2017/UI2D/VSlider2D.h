#pragma once
#include "vxCpp.h"


#define VSLIDER_RADUS 20

class VSlider2D : public IActive, public IVoxiePtr, public IUI2D
{
public:
	VSlider2D(IVoxieBox * voxiePtr, int posX, int posY, int length, double maxValue, double minValue, double incrementAmount, int color);
	~VSlider2D();

	int isTouched();
	int update();
	void draw();

	void setMinValue(double newValue);
	void setMaxValue(double newValue);
	void setValue(double newValue);

	float getMinValue();
	float getMaxValue();
	float getValue();


private:
	IVoxieBox	*voxiePtr = 0;

	double		maxValue;
	double		minValue;
	double		incrementAmount;
	float		value;
	float		makerPos = 0;

	bool		isDraggable = true;
	bool		isLockable = true;

	int			lenght;
	int			lockedIndex = -1;

	int			posXOffset = 0;
	int			posYOffset = 0;
	int			style = 1;
	int			color = 0x303030;
	int			slideRadius = VSLIDER_RADUS;
	int			sliderBorder = 3;
};

