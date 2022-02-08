#include "Button2D.h"


Button2D::Button2D(IVoxieBox * voxiePtr, char * label, int posX, int posY, int widthX, int heightY, int color)
{
	this->voxiePtr = voxiePtr;
	this->label = label;
	this->posX = posX;
	this->widthX = widthX;
	this->posY = posY;
	this->heightY = heightY;
	this->color = color;
}

Button2D::~Button2D()
{
}

int Button2D::isTouched()
{


	point2d tl = { (float)this->posX, (float)this->posY };
	point2d rb = { (float)this->posX + (float)this->widthX, (float)this->posY + (float)this->heightY };

	if (voxiePtr->getTouchPressState(tl, rb) == 2) {

		lockedIndex = voxiePtr->getTouchPressIndex(tl, rb);
		if (isDraggable) {
			posXOffset = voxiePtr->getTouchPosX(lockedIndex) - posX;
			posYOffset = voxiePtr->getTouchPosY(lockedIndex) - posY;
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

int Button2D::getButtonState()
{
	return buttonState;
}

void Button2D::setButtonState(int newButtonValue)
{
	buttonState = newButtonValue;
}

int Button2D::update()
{
	isTouched();

	if (lockedIndex != -1) {

		if (voxiePtr->getTouchState(lockedIndex) == 0) lockedIndex = -1;

		if (voxiePtr->getTouchState(lockedIndex) == 2) {

			this->posX = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;
			this->posY = voxiePtr->getTouchPosY(lockedIndex) - posYOffset;

		}

	}

	return 0;
}

void Button2D::setDragable(bool choice)
{
	isDraggable = choice;
}

void Button2D::draw()
{
	int x = this->posX, y = this->posY, w = this->posX + this->widthX, h = this->posY + this->heightY;
	int bgcol = 0, fgcol = 0, hocol = 0, precol = 0;
	int i = 0;

	// work out center position for button how many characters in label's 
	int centX = (x + (this->widthX * 0.5)) - (std::string(this->label).length() * 3);
	int centY = y + (this->heightY * 0.5);


	switch (style)
	{

		default:
		case 0: // style 0 just a box 

			x = this->posX;
			y = this->posY;

			bgcol = voxiePtr->tweenCol(this->color, 50, 0x000000);
			fgcol = voxiePtr->brightenCol(this->color, 50);
			hocol=  voxiePtr->brightenCol(this->color, 25);
			precol = voxiePtr->tweenCol(this->color, 25, 0x000000);

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
					voxiePtr->debugDrawTile(&image, this->posX + imgXOffset, this->posY + imgYOffset);
				}

				if (this->showLabel) {
					voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->label);
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
					voxiePtr->debugDrawTile(&image, this->posX + imgXOffset, this->posY + imgYOffset);
				}

				if (this->showLabel) {
					voxiePtr->debugText(centX, centY, this->textColor, -1, "%s", this->label);
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
					voxiePtr->debugDrawTile(&image, this->posX + imgXOffset, this->posY + imgYOffset);
				}


				if (this->showLabel) {
					voxiePtr->debugText(centX, centY, this->pressedTextColor, -1, "%s", this->label);
				}
				break;



			}
			
			break;

			
	}
}



void Button2D::setLabel(char * newLabel)
{
	this->label = newLabel;
}

char * Button2D::getLabel()
{
	return this->label;
}

void Button2D::setShowLabel(bool choice)
{
	showLabel = choice;
}

void Button2D::setShowImage(bool choice)
{
	showImage = choice;
}

void Button2D::setImage(char * imgPath, int imgXPixel, int imgYPixel, int imgXOffset, int imgYOffset)
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

void Button2D::setTextColor(int newColor)
{
	textColor = newColor;
}

void Button2D::setPressedTextColor(int newColor)
{
	pressedTextColor = newColor;
}
