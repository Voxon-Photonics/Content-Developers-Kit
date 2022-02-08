#pragma once
#include "vxCpp.h"
#include "Element2D.h"
#include <vector>

class Window2D : public IActive, public IVoxiePtr, public IUI2D
{
public:
	
	Window2D(IVoxieBox * voxiePtr, int posX, int widthX, int posY, int heightY, int color);
	~Window2D();

	int isTouched();
	int isScrollBarTouched();
	void setDragable(bool choice);

	void setEnableTitle(bool choice);
	int setTitle(char * title, ...);
	int setTitleColor(int newColor);

	void setShowElements(bool choice);
	void setEnableScrollBar(bool choice);
	void setScrollBar(int width, int trackcol, int barcol);
	void autoArrangeElementsStyle(int option);
	void setElementSpacing(int xSpace, int ySpace);

	void addElement(Element2D * newElement2DPtr);
	void flushElements();
	void removeElement(int elementIndex);
	void draw();
	void updateElementsPos();
	int update();

private:

	char *		copyString(char c[]);

	int			posX		= 0, posY			= 0;
	int			posXOffset	= 0, posYOffset		= 0;
	int			widthX		= 0, heightY		= 0;

	int			elementXSpacing = 110;
	int			elementYSpacing = 110;


	int			color					= 0x303030;
	IVoxieBox	*voxiePtr				= nullptr;
	int			lockedIndex				= -1;
	int			lockedIndexScrollBar    = -1;
	int			titleColor				= 0xffffff;
	char *		title					= (char*)" ";
	bool		isDraggable				= true;
	bool		showTitle				= false;
	int			style					= 1;
	bool		showElements			= false;
	bool		enableScrollBar			= false;
	int			autoArrangeElements		= 0;
	int			scrollBarTicks			= 0;
	int			scrollBarPosX			= 0;
	int			scrollBarElementsPerRow = 0;
	int			scrollBarTotalElements  = 0;
	int			scrollBarPosY			= 0;
	int			scrollBarMarkerPos		= 0;
	int         scrollBarMarkerRelPos   = 0;
	int			scrollBarTrackLength	= 0;
	int			scrollBarHeight			= 50;

	int			scrollBarWidth			= 50;
	int			scrollBarColor			= 0x006000;
	int			scrollBarTrackColor		= 0x303000;
	int			scrollBarHilightColor	= 0x606000;
	int			scrollBarBorderPadding  = 10;
	int			scrollBarBorderThickness = 3;
	bool		scrollBarDebug			= true;

	void		drawScrollBar();
	void		scrollBarCalc();

	std::vector<Element2D*> elements;

};

