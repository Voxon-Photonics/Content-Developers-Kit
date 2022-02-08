#include "Window2D.h"

Window2D::Window2D(IVoxieBox * voxiePtr, int posX, int widthX, int posY, int heightY, int color)
{
	this->voxiePtr = voxiePtr;
	this->posX = posX;
	this->widthX = widthX;
	this->posY = posY;
	this->heightY = heightY;
	this->color = color;
}

Window2D::~Window2D()
{
}

void Window2D::draw()
{

	if (voxiePtr == nullptr) return;


	int x = this->posX, y = this->posY, w = this->posX + this->widthX, h = this->posY + this->heightY;
	int bgcol = 0, fgcol = 0;

	switch (style)
	{

	default:
	case 0: // style 0 just a box 
		voxiePtr->debugDrawBoxFill(this->posX, this->posY, this->posX + this->widthX, this->posY + this->heightY, this->color);
	
		break;

	case 1: // style 1 box with shadow
		x = this->posX;
		y = this->posY;

		bgcol = voxiePtr->tweenCol(this->color, 50, 0x000000);
		fgcol = voxiePtr->brightenCol(this->color, 50);

		voxiePtr->debugDrawBoxFill(x, y + 5, w + 5, h + 5, bgcol); // draws a rectangle
		voxiePtr->debugDrawBoxFill(x, y, w, h, color);

		voxiePtr->debugDrawLine(x, y, w, y, fgcol); // draws a line
		voxiePtr->debugDrawLine(x, y, x, h, fgcol);

		break;

	case 2: // style 2 box with brighter outline
		x = this->posX;
		y = this->posY;

		bgcol = voxiePtr->tweenCol(this->color, 50, 0x000000);
		fgcol = voxiePtr->brightenCol(this->color, 50);

		voxiePtr->debugDrawBoxFill(x, y, w, h, color);

		voxiePtr->debugDrawLine(x, y, w, y, fgcol);
		voxiePtr->debugDrawLine(x, y, x, h + 2, fgcol);
		voxiePtr->debugDrawLine(w, y, w, h, fgcol);
		voxiePtr->debugDrawLine(x, h, w + 2, h, fgcol);

		break;

	}

	// work out center position for button how many characters in label's 
	int centX = (x + (this->widthX * 0.5)) - (std::string(this->title).length() * 3);
	int centY = this->posY + 5; //y + (this->heightY * 0.5);

	if (this->showTitle) {
		voxiePtr->debugText(centX, centY, this->titleColor, -1, "%s", this->title);
	}


	// draw elements
	int i = 0;

	if (showElements) {

		for (i = 0; i < elements.size(); i++) {

			
				elements[i]->draw();
			

		}

	

	}

	// draw scroll bars
	if (enableScrollBar) drawScrollBar();
	

}



int Window2D::isScrollBarTouched()
{
	point2d tl = { (float)this->scrollBarPosX, (float)this->scrollBarMarkerPos };
	point2d rb = { (float)this->scrollBarPosX + (float)this->scrollBarWidth, (float)this->scrollBarMarkerPos + (float)this->scrollBarHeight };

	if (voxiePtr->getTouchPressState(tl, rb) >= 1) {


		lockedIndexScrollBar = voxiePtr->getTouchPressIndex(tl, rb);
		if (isDraggable) {
			posXOffset = voxiePtr->getTouchPosX(lockedIndexScrollBar) - posX;
			posYOffset = voxiePtr->getTouchPosY(lockedIndexScrollBar) - posY;
		}
	
	}
	else {
		
	}

	

	return voxiePtr->getTouchPressState(tl, rb);

}





int Window2D::isTouched()
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
	
	return voxiePtr->getTouchPressState(tl, rb);

}

void Window2D::setDragable(bool choice)
{
	isDraggable = choice;
}

char* Window2D::copyString(char s[])
{
	char* s2;
	s2 = (char*)malloc(20);

	strcpy(s2, s);
	return (char*)s2;
}

void Window2D::drawScrollBar()
{

	int bgcol = voxiePtr->tweenCol(scrollBarTrackColor, 50, 0x000000);
	int fgcol = voxiePtr->brightenCol(scrollBarTrackColor, 50);
	int mbgcol = voxiePtr->tweenCol(scrollBarColor, 50, 0x000000);
	int mfgcol = voxiePtr->brightenCol(scrollBarColor, 50);
	int x = scrollBarPosX;
	int y = scrollBarPosY;
	int w = scrollBarPosX + scrollBarWidth;
	int h = scrollBarPosY + heightY - scrollBarBorderPadding - 5;
	int my = scrollBarMarkerPos;
	int mh = scrollBarMarkerPos + scrollBarHeight;

	// draw scroll bar track
	voxiePtr->debugDrawBoxFill(x, y, w, h, scrollBarTrackColor);

	int i = 0;
	 
	for (i = 0; i < scrollBarBorderThickness; i++) {
		voxiePtr->debugDrawLine(x, y + i, w, y + i, fgcol);			// top
		voxiePtr->debugDrawLine(x + i, y, x + i, h + 2, fgcol);		// left
		voxiePtr->debugDrawLine(w - i, y, w - i, h, fgcol);			// right
		voxiePtr->debugDrawLine(x, h - i, w + 2, h - i , fgcol);	// bottom
	}

	// draw scroll bar
	if (this->lockedIndexScrollBar != -1) {
		voxiePtr->debugDrawBoxFill(x, my, w, mh, scrollBarHilightColor);
		mbgcol = voxiePtr->tweenCol(scrollBarHilightColor, 50, 0x000000);
		mfgcol = voxiePtr->brightenCol(scrollBarHilightColor, 50);
	}
	else voxiePtr->debugDrawBoxFill(x, my, w, mh, scrollBarColor);

	for (i = 0; i < scrollBarBorderThickness; i++) {
		voxiePtr->debugDrawLine(x, my + i, w, my + i, mfgcol);			// top
		voxiePtr->debugDrawLine(x + i , my, x + i, mh + 2, mfgcol);		// left
		voxiePtr->debugDrawLine(w - i , my, w - i, mh, mfgcol);			// right
		voxiePtr->debugDrawLine(x, mh - i, w + 2, mh - i, mfgcol);		// bottom
	}

	scrollBarMarkerRelPos = scrollBarMarkerPos - posY - scrollBarBorderPadding;

	if (scrollBarDebug) {
		
		voxiePtr->debugText(scrollBarPosX + 50, scrollBarPosY, 0xffffff, -1, "sb pos X %d Y %d, ticks %d, ele  %d / %d, markerPos %d / %d markerPos / Offset %d Starting Row %d", scrollBarPosX, scrollBarPosY, scrollBarTicks, scrollBarTotalElements, scrollBarElementsPerRow,  scrollBarMarkerRelPos, scrollBarTrackLength, scrollBarMarkerRelPos % scrollBarTotalElements, (scrollBarMarkerRelPos) / scrollBarTicks);

	}


}

void Window2D::scrollBarCalc()
{
	scrollBarPosX = (this->posX + this->widthX) - scrollBarBorderPadding - scrollBarWidth;
	scrollBarPosY = (this->posY) + scrollBarBorderPadding;
	scrollBarMarkerPos = scrollBarPosY;

	int i = 0;
	int x = 0;
	int y = 0;

	scrollBarTotalElements = elements.size();
	scrollBarElementsPerRow = 1;
	
	for (i = 0; i < elements.size(); i++) {
			if ((x * elementXSpacing) + (this->posX) + 10 + this->scrollBarWidth + elements[i]->getWidth() > this->posX + this->widthX) {
				y++;
				scrollBarElementsPerRow = x;
				x = 0;		
			}
			if ((y * elementYSpacing) > this->posY + this->heightY) {
				enableScrollBar = true;
			}
			x++;
	}

	scrollBarTrackLength = (scrollBarPosY + heightY) - (scrollBarPosY + scrollBarHeight + scrollBarBorderPadding + 5);
	scrollBarTicks = scrollBarTrackLength / (scrollBarTotalElements / scrollBarElementsPerRow) ;

}

void Window2D::setEnableTitle(bool choice)
{
	this->showTitle = choice;
}

int Window2D::setTitle(char * title, ...)
{
	va_list arglist;
	char buf[1024];
	int i = 0;

	if (!title) return 1;
	va_start(arglist, title);
#if defined(_WIN32)
	if (_vsnprintf((char *)&buf, sizeof(buf) - 1, title, arglist)) buf[sizeof(buf) - 1] = 0;
#else
	if (vsprintf((char *)&buf, title, arglist)) buf[sizeof(buf) - 1] = 0; //NOTE:unsafe!
#endif
	va_end(arglist);

	this->title = copyString(buf);

	return 0;
}

int Window2D::setTitleColor(int newColor)
{
	return this->titleColor = newColor;
}

void Window2D::setShowElements(bool choice)
{
	this->showElements = choice;
}

void Window2D::setEnableScrollBar(bool choice)
{
	this->enableScrollBar = choice;
}

void Window2D::setScrollBar(int width, int trackcol, int barcol)
{
	scrollBarColor			= trackcol;
	scrollBarTrackColor		= barcol;
	scrollBarWidth			= width;

}

void Window2D::autoArrangeElementsStyle(int option)
{
	this->autoArrangeElements = option;
}

void Window2D::setElementSpacing(int xSpace, int ySpace)
{
	this->elementXSpacing = xSpace;
	this->elementYSpacing = ySpace;

}


void Window2D::updateElementsPos()
{
	int i = 0;
	int y = 0;
	int x = 0;
	int titlesPerRow = 0;
	if (autoArrangeElements) enableScrollBar = false;
	for (i = 0; i < elements.size(); i++) {

		if (this->autoArrangeElements == 1) {

			if ((x * elementXSpacing) + (this->posX) + 10 + this->scrollBarWidth + elements[i]->getWidth() > this->posX + this->widthX) {
				y++;
				x = 0;
				titlesPerRow = x;
			}

			elements[i]->setRelPosX( (x  * elementXSpacing) + 10 + this->posX);
			elements[i]->setRelPosY( (y  * elementXSpacing) + 20 + this->posY);

			if ((y * elementYSpacing) > this->posY + this->heightY) {
				enableScrollBar = true;
				
			}

			x++;

		}
		else {
			elements[i]->setRelPosX(elements[i]->getRelPosXOffset() + this->posX);
			elements[i]->setRelPosY(elements[i]->getRelPosYOffset() + this->posY);
		}

	}


	i = 0;
	y = 0;
	x = 0;
	int offset = 0;
	if (scrollBarMarkerRelPos != 0 || scrollBarTotalElements != 0) {
		offset = (scrollBarMarkerRelPos % scrollBarTotalElements) / (this->scrollBarTicks * .5);
	}

	//sort them in the right spot 
	for (i = 0; i < elements.size(); i++) {

		if (this->autoArrangeElements == 1) {

			// cull from starting row 
			if (scrollBarElementsPerRow != 0 || scrollBarTicks != 0) {
				if (i < (scrollBarMarkerRelPos / scrollBarTicks) *  scrollBarElementsPerRow) {
					elements[i]->setRelPosY(-100);
					continue;
				}
			}
//			if (this->scrollBarMarkerPos > elements[i]->getRelPosY()) { elements[i]->setRelPosY(-100);  continue; }

			if ((x * elementXSpacing) + (this->posX) + 10 + this->scrollBarWidth + elements[i]->getWidth() > this->posX + this->widthX) {
				y++;
				x = 0;
			}

			elements[i]->setRelPosX((x  * elementXSpacing) + 10 + this->posX);
			elements[i]->setRelPosY((y  * elementYSpacing) + 20 + this->posY - offset);

			if (elements[i]->getRelPosY() + elementYSpacing  > this->posY + this->heightY) {
				
				elements[i]->setRelPosY(-100);  continue;
			}


			x++;

		}
		

	}


	if (enableScrollBar) {

		scrollBarCalc();

	}


}


void Window2D::addElement(Element2D * newElement2DPtr)
{
	if (voxiePtr == nullptr) return;

	newElement2DPtr->setIsChild(true);
	
	elements.push_back(newElement2DPtr);

	updateElementsPos();
}

void Window2D::flushElements()
{
	this->elements.clear();
}

void Window2D::removeElement(int elementIndex)
{
	auto elem_to_remove = elements.begin() + elementIndex;
	if (elem_to_remove != elements.end()) {
		elements.erase(elem_to_remove);
	}

}

int Window2D::update()
{
	if (voxiePtr == nullptr) return 1;

	isTouched();

	if (lockedIndex != -1) {

		if (voxiePtr->getTouchState(lockedIndex) == 0) lockedIndex = -1;

		if (voxiePtr->getTouchState(lockedIndex) == 2) {

			this->posX = voxiePtr->getTouchPosX(lockedIndex) - posXOffset;
			this->posY = voxiePtr->getTouchPosY(lockedIndex) - posYOffset;
			if (showElements) this->updateElementsPos();

		}
	}
	int i = 0;

	if (showElements) {

		for (i = 0; i < elements.size(); i++) {

			elements[i]->update();

		}
		
	}

	if (enableScrollBar) {

		isScrollBarTouched();

	}

	if (lockedIndexScrollBar != -1) {

		if (voxiePtr->getTouchState(lockedIndexScrollBar) == 0) lockedIndexScrollBar = -1;

		if (voxiePtr->getTouchState(lockedIndexScrollBar) >= 1) {

			
			if (voxiePtr->getTouchPosY(lockedIndexScrollBar) > scrollBarPosY && this->scrollBarMarkerPos + this->scrollBarHeight < (scrollBarPosY + heightY) - scrollBarBorderPadding - 5 - scrollBarHeight ) {
				
				this->scrollBarMarkerPos = voxiePtr->getTouchPosY(lockedIndexScrollBar); 		
				if (this->scrollBarMarkerPos + scrollBarHeight > (scrollBarPosY + heightY) - scrollBarBorderPadding - 5) scrollBarMarkerPos = (scrollBarPosY + heightY) - scrollBarBorderPadding - 5 - scrollBarHeight;
			}

		}
	}

	return 0;
}
