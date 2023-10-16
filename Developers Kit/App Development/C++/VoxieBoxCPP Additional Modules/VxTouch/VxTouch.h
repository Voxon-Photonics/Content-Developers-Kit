#pragma once
#include "vxCpp.h"


/**
 *  VxTouch is a class to use with the Voxon touchscreen to setup your own custom keyboard layouts
 */

class VxTouch : public IVoxiePtr
{
public:
	VxTouch(IVoxieBox * voxiePtr);
	VxTouch(IVoxieBox * voxiePtr, const touchkey_t * touchKeyPtr, int touchArraySize);
	void setTouchLayout(const touchkey_t * touchKeyPtr, int touchArraySize);
	const touchkey_t * getTouchLayout();
	void loadDefaultTouchLayout();
	int getNumButtons();

	~VxTouch();

private: 
	int touchKeyArrayNum = -1;
	const  touchkey_t * currentTouchLayout;
};

