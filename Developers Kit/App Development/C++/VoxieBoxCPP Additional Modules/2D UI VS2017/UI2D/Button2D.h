#pragma once
#include "vxCpp.h"

class Button2D : public IActive, public IVoxiePtr, public IUI2D
{
public:
	Button2D(IVoxieBox * voxiePtr, char * label, int posX, int posY, int widthX, int heightY, int color);
	~Button2D();

	int		isTouched();
	int		getButtonState();
	void	setButtonState(int newButtonValue);
	int		update();
	void	setDragable(bool choice);
	void	draw();

	void	setLabel(char * newLabel);
	char *  getLabel();

	void	setShowLabel(bool choice);
	void	setShowImage(bool choice);
	void	setImage(char * imgPath, int imgXPixel, int imgYPixel, int imgXOffset, int imgYOffset);
	void	setTextColor(int newColor);
	void	setPressedTextColor(int newColor);

private:

	tiletype	image;

	int imgXOffset, imgYOffset;
	int imgXPixel, imgYPixel;

	int posXOffset, posYOffset;
	int widthX, heightY;
	int color = 0x303030;
	int lockedIndex = -1;
	bool isDraggable = true;
	int style = 1;
	int buttonState = 0;
	char * label;
	char * imgPath = NULL;
	int textColor = 0xffffff;
	int pressedTextColor = 0x00ffff;
	bool showImage = false;
	bool showLabel = true;
	int borderThickness = 3;



};
