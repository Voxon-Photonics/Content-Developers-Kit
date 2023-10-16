#include "Element2D.h"



Element2D::Element2D(IVoxieBox * voxiePtr, int posX, int posY, int width, int length, int bgCol, int textCol)
{
	this->voxiePtr	= voxiePtr;
	this->posX		= posX;
	this->posY		= posY;
	this->relPosXOffset = posX;
	this->relPosYOffset = posY;

	this->widthX	= width;
	this->heightY	= length;
	this->color		= bgCol;
	this->textColor	= textCol;
}

Element2D::Element2D()
{
}


Element2D::~Element2D()
{
}

int Element2D::isTouched()
{

	int x = this->posX;
	int y = this->posY;

	if (isChild && absolutePos == false) {
		x = this->relPosX;
		y = this->relPosY;
	}



	point2d tl = { (float)x, (float)y };
	point2d rb = { (float)x + (float)this->widthX, (float)y + (float)this->heightY };

	if (voxiePtr->getTouchPressState(tl, rb) == 2) {
		if (isDraggable) {
			lockedIndex = voxiePtr->getTouchPressIndex(tl, rb);
			posXOffset = voxiePtr->getTouchPosX(lockedIndex) - x;
			posYOffset = voxiePtr->getTouchPosY(lockedIndex) - y;
		}
	}

	switch (voxiePtr->getTouchPressState(tl, rb)) {
	default:
		buttonState = TOUCH_STATE_NOT_PRESSED;
		break;
	case TOUCH_STATE_IS_DOWN:
		buttonState = TOUCH_STATE_IS_DOWN;
		break;
	case TOUCH_STATE_IS_HELD:
		buttonState = TOUCH_STATE_IS_HELD;
		break;
	case TOUCH_STATE_JUST_PRESSED:
		buttonState = TOUCH_STATE_JUST_PRESSED;
		break;
	case TOUCH_STATE_ON_UP:
		buttonState = TOUCH_STATE_ON_UP;
		break;
	}

	return voxiePtr->getTouchPressState(tl, rb);
}

int Element2D::update()
{

	int x = this->posX, y = this->posY;

	if (isChild && absolutePos == false) {
		x = this->relPosX;
		y = this->relPosY;

	}
	
	isTouched();

	if (lockedIndex != -1) {

		if (voxiePtr->getTouchState(lockedIndex) == 0) lockedIndex = -1;

		if (voxiePtr->getTouchState(lockedIndex) == 2) {

			if (isChild && absolutePos == false) {
				this->relPosXOffset = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;
				this->relPosYOffset = voxiePtr->getTouchPosY(lockedIndex) - posYOffset;
			}
			else {

				this->posX = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;
				this->posY = voxiePtr->getTouchPosY(lockedIndex) - posYOffset;
			}
		}

	}

	return 0;
}

void Element2D::draw()
{
	if (voxiePtr == nullptr) return;

	//update();

	int x = this->posX, y = this->posY, w = this->posX + this->widthX, h = this->posY + this->heightY;
	int bgcol = 0, fgcol = 0, hocol = 0, precol = 0;
	int i = 0;

	if (isChild && absolutePos == false) {
		x = this->relPosX;
		y = this->relPosY;
		w = this->relPosX + this->widthX;
		h = this->relPosY + this->heightY;
	}

	// work out center position for button how many characters in label's 
	int centX = (x + (this->widthX * 0.5)) - (std::string(this->textMessage).length() * 3);
	int centY = y + (this->heightY * 0.5);

	bgcol = voxiePtr->tweenCol(this->color, 50, 0x000000);
	fgcol = voxiePtr->brightenCol(this->color, 50);
	hocol = voxiePtr->brightenCol(this->color, 25);
	precol = voxiePtr->tweenCol(this->color, 25, 0x000000);

	switch (style)
	{

	default:
	case 0: // style 0 just a box 

	///	x = this->posX;
	//	y = this->posY;



		switch (buttonState) {
		default:
		case 0:
			voxiePtr->debugDrawBoxFill(x, y, w, h, color);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, bgcol); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, bgcol); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, fgcol); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, fgcol); // left
			}

			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}

			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->textMessage);
			}

			break;
		case 2:
			voxiePtr->debugDrawBoxFill(x, y, w, h, hocol);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, bgcol); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, bgcol); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, fgcol); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, fgcol); // left
			}

			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}

			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->textMessage);
			}
			break;
		case 1:
		case 3:
		case 4:

			voxiePtr->debugDrawBoxFill(x, y, w, h, precol);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, fgcol); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, fgcol); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, bgcol); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, bgcol); // left
			}


			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}


			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->pressedTextColor, -1, "%s", this->textMessage);
			}
			break;



		}

		break;
	case 1: 	// style 1 outline 



		switch (buttonState) {
		default:
		case 0: // not pressed 
			//voxiePtr->debugDrawBoxFill(x, y, w, h, color);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, color); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, color); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, color); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, color); // left
			}

			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}

			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->textMessage);
			}

			break;
		case 2: // held down
			voxiePtr->debugDrawBoxFill(x, y, w, h, hocol);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, color); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, color); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, color); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, color); // left
			}

			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}

			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->textMessage);
			}
			break;
		case 1:
		case 3:
		case 4: // pressed 

			voxiePtr->debugDrawBoxFill(x, y, w, h, precol);

			for (i = 0; i < borderThickness; i++) {
				voxiePtr->debugDrawLine(w - i, y, w - i, h, fgcol); // right
				voxiePtr->debugDrawLine(x, h - i, w, h - i, fgcol); // bottom
				voxiePtr->debugDrawLine(x, y + i, w, y + i, fgcol); // top
				voxiePtr->debugDrawLine(x + i, y, x + i, h, fgcol); // left
			}


			if (this->showImage) {
				voxiePtr->debugDrawTile(&image, x + imgXOffset, y + imgYOffset);
			}


			if (this->showText) {
				voxiePtr->debugText(centX, centY, this->pressedTextColor, -1, "%s", this->textMessage);
			}
			break;



		}

		break;

	}

}




char* Element2D::copyString(char s[])
{
	char* s2;
	s2 = (char*)malloc(20);

	strcpy(s2, s);
	return (char*)s2;
}

void Element2D::setSettings(int posX, int posY, int width, int length, int bgCol, int textCol)
{
	this->posX = posX;
	this->posY = posY;
	this->widthX = width;
	this->heightY = length;
	this->color = bgCol;
	this->textColor = textCol;
}

void Element2D::setTextColor(int newColor)
{
	this->textColor = newColor;
}

void Element2D::setBgCol(int newColor)
{
	this->color = newColor;
}

void Element2D::setPressedTextColor(int newColor)
{
	this->pressedTextColor = newColor;
}

int Element2D::setMessage(char * message, ...)
{
		va_list arglist;
		char buf[1024];
		int i = 0;

		if (!message) return 1;
		va_start(arglist, message);
#if defined(_WIN32)
		if (_vsnprintf((char *)&buf, sizeof(buf) - 1, message, arglist)) buf[sizeof(buf) - 1] = 0;
#else
		if (vsprintf((char *)&buf, fmt, arglist)) buf[sizeof(buf) - 1] = 0; //NOTE:unsafe!
#endif
		va_end(arglist);

		this->textMessage = copyString(buf);

		return 0;
}

void Element2D::setImage(char * imgPath, int imgXPixel, int imgYPixel, int imgXOffset, int imgYOffset)
{
	
	this->imgPath = imgPath;
	this->imgXOffset = imgXOffset;
	this->imgYOffset = imgYOffset;

	// set the image as a tile type -> this can be tricky to learn 
	image.x = imgXPixel;													// get the image's x dimension (check the details in windows explorer)
	image.y = imgYPixel;													// get the image's y dimension (check the details in windows explorer)
	image.p = (image.x << 2);												// define the pitch number of bytes per horizontal line (usually x*4 but may be higher or negative)
	image.f = (INT_PTR)malloc(image.p*image.y);								// create pointer to 1st pixel
	voxiePtr->_kpzload(imgPath, &image.f, &image.p, &image.x, &image.y);	// load the image into file memory.	

	this->showImage = true;
	

}

void Element2D::setShowImage(bool choice)
{
	this->showImage = choice;
}

void Element2D::setShowText(bool choice)
{
	this->showText = choice;
}

void Element2D::setEnableAbsolutePos(bool choice)
{
	this->absolutePos = choice;
}

void Element2D::setRelPosX(int relPosX)
{
	this->relPosX = relPosX;
}

void Element2D::setRelPosY(int relPosY)
{
	this->relPosY = relPosY;
}

void Element2D::setRelPosXOffset(int relPosXOffset)
{
	this->relPosXOffset = relPosXOffset;
}

void Element2D::setRelPosYOffset(int relPosYOffset)
{
	this->relPosXOffset = relPosYOffset;
}

int Element2D::getRelPosX()
{
	return this->relPosX;
}

int Element2D::getRelPosY()
{
	return this->relPosY;
}

int Element2D::getRelPosXOffset()
{
	return this->relPosXOffset;
}

int Element2D::getRelPosYOffset()
{
	return this->relPosYOffset;
}

void Element2D::setWidth(int newWidth)
{
	this->widthX = newWidth;
}

void Element2D::setHeight(int newHeight)
{
	this->heightY = newHeight;
}

int Element2D::getWidth()
{
	return this->widthX;
}

int Element2D::getHeight()
{
	return this->heightY;
}

bool Element2D::getShowImage()
{
	return this->showImage;
}

bool Element2D::getAbsolutePos()
{
	return this->absolutePos;
}

bool Element2D::getShowText()
{
	return this->showText;
}

void Element2D::setIsChild(bool choice)
{
	this->isChild = choice;
}

bool Element2D::getIsChild()
{
	return this->isChild;
}

