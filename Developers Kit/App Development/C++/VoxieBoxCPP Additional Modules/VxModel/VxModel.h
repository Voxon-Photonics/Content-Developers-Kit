#pragma once
#include "vxCpp.h"


// A class which can manage a model 
/**
 * By Matthew Vecchio for Voxon 12/05/2012
 *
 *
 **/
class VxModel : public IDrawable
{
public:

	VxModel();
	VxModel(IVoxieBox* voxiePtr, point3d pos,const char filename[256] );

	void setVoxiePtr(IVoxieBox* voxiePtr);
	IVoxieBox* getVoxiePtr();

	void draw();
	void draw(const char filename[256], bool initExtendsChoice);

	void setFilename(const char filename[256]);
	const char* getFilename();

	void setScale(point3d scale);
	point3d getScale();

	void rotatePitch	(float rotateAmountInRadian, bool useDegrees = false);
	void rotateRoll		(float rotateAmountInRadian, bool useDegrees = false);
	void rotateYaw		(float rotateAmountInRadian, bool useDegrees = false);

	void setColour(int newColour);
	int getColour();

	void setPos(point3d newPos);
	point3d getPos();
	
	void	setRightVec(point3d newRightVec);
	point3d getRightVec();

	void	setDownVec(point3d newDownVec);
	point3d getDownVec();

	void	setForwardVec(point3d newForwardVec);
	point3d getForwardVec();

	float   getScalarValue();

	void setForceScale(int forceScaleValue);
	int getForceScale();

	void setFlagValue(int wireFrameValue);
	int getFlagValue();

	void report(int posX, int posY);


private:


	bool extentsInited = false;
	extents_t modelExtents = { 0 };

	point3d	scale		=	{ 1, 1, 1 };
	point3d pos			=	{ 0, 0, 0 };
	point3d rightVec	=	{ 1, 0, 0 };
	point3d downVec		=	{ 0, 1, 0 };
	point3d forwardVec	=	{ 0, 0, 1 };
	int colour = 0x404040;
	IVoxieBox* voxiePtr = nullptr;
	int flags = 0;
	int forceScale = 0;
	float forceScaleAmount = 0;
	char fileName[256] = { 0 };
	bool isModelFound = false;







};

