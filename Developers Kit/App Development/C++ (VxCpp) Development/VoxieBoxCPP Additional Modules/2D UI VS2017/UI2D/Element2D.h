#pragma once
#include "vxCpp.h"
//#include "Window2D.h"

// Element
// can be a text label
// can be a image
// can press liked be a button
// part of a groupbox which can stack all the elements together

class Element2D : public IActive, public IVoxiePtr, public IUI2D
{
public:
	Element2D(IVoxieBox * voxiePtr, int posX, int posY, int width, int length, int bgCol, int textCol);
	Element2D();
	~Element2D();
	   	 
	int		isTouched();
	int		update();
	void	draw();
	void	setSettings(int posX, int posY, int width, int length, int bgCol, int textCol);
	void	setTextColor(int newColor);
	void    setBgCol(int newColor);
	void	setPressedTextColor(int newColor);
	int     setMessage(char * message,  ...);
	void	setImage(char * imgPath, int imgXPixel, int imgYPixel, int imgXOffset, int imgYOffset);

	void	setShowImage(bool choice);
	void	setShowText(bool choice);
	void    setEnableAbsolutePos(bool choice);

	void    setRelPosX(int relPosX);
	void    setRelPosY(int relPosY);
	void    setRelPosXOffset(int relPosX);
	void    setRelPosYOffset(int relPosY);
	int     getRelPosX();
	int     getRelPosY();
	int     getRelPosXOffset();
	int     getRelPosYOffset();
	void	setWidth(int newWidth);
	void    setHeight(int newLength);
	int	    getWidth();
	int	    getHeight();


	bool	getShowImage();
	bool	getAbsolutePos();
	bool    getShowText();
	void	setIsChild(bool choice);
	bool	getIsChild();

private:

	char * copyString(char c[]);
//	Window2D * windowParentPtr = nullptr;
	tiletype				image;
	int relPosX				= 0;
	int relPosY				= 0;
	int relPosXOffset		= 0;
	int relPosYOffset		= 0;
	int posX				= 0; // posX 
	int posY				= 0;
	int posXOffset			= 0;
	int posYOffset			= 0;
	int imgXOffset			= 0;
	int	imgYOffset			= 0;
	int imgXPixel			= 0;
	int imgYPixel			= 0;
	int heightY				= 0;
	int widthX				= 0;
	int style				= 1; // 0 3d button,, 1 outline box
	int lockedIndex			= -1;
	int borderThickness		= 3; // how thick the border is
	int color				= 0x003000;
	int textColor			= 0xffffff;
	int pressedTextColor	= 0x00ffff;
	int buttonState			= 0;
	bool isChild			= false;
	bool isDraggable		= false;
	bool absolutePos		= false;
	bool pressable			= false;
	bool showImage			= false;
	bool showText			= true;
	bool hilightOnTouch		= false;
	char * imgPath			= NULL;
	char * textMessage		= (char*)"text";
	
};

