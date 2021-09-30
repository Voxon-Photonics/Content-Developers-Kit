#include "VxTouch.h"

VxTouch::VxTouch(IVoxieBox * voxiePtr)
{
	this->voxiePtr = voxiePtr;
}

VxTouch::VxTouch(IVoxieBox * voxiePtr, const touchkey_t * touchKeyPtr, int touchArraySize)
{
	this->voxiePtr = voxiePtr;
	this->setTouchLayout(touchKeyPtr, touchArraySize);
}

void VxTouch::setTouchLayout(const touchkey_t * touchKeyPtr, int touchArraySize)
{
	if (voxiePtr == 0) return;
	voxiePtr->touchAddCustomLayout(touchKeyPtr, touchArraySize);
	touchKeyArrayNum = touchArraySize;
	
}

const touchkey_t * VxTouch::getTouchLayout()
{
	if (voxiePtr == 0) return nullptr;
	return this->currentTouchLayout;
}

void VxTouch::loadDefaultTouchLayout()
{
	if (voxiePtr == 0) return;
	voxiePtr->touchAddCustomLayout(NULL, NULL);
	touchKeyArrayNum = 72;
}

int VxTouch::getNumButtons()
{
	return touchKeyArrayNum;
}

VxTouch::~VxTouch()
{
}
