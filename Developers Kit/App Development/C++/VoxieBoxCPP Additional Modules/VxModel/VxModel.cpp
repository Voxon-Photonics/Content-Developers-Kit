#include "VxModel.h"

VxModel::VxModel()
{




}

VxModel::VxModel(IVoxieBox* voxiePtr, point3d pos, const char filename[256])
{

	this->voxiePtr = voxiePtr;
	this->pos = pos;
	strcpy(this->fileName, filename);
	point3d pp{ -voxiePtr->getAspectX() - 100, 0, 0 }, rr = { 0,0,0 }, ff = { 0,0,0 }, dd = { 0,0,0 };

	isModelFound = voxiePtr->drawModel(this->fileName, &pp, &rr, &dd, &ff, 0x404040);
	
}

void VxModel::setVoxiePtr(IVoxieBox* voxiePtr)
{
	this->voxiePtr = voxiePtr;

}

IVoxieBox* VxModel::getVoxiePtr()
{
	return voxiePtr;
}


void VxModel::draw(const char filename[256], bool initExtendsChoice) {


	if (voxiePtr == nullptr) return;
	if (isModelFound == false) return;


	point3d rVec = { 0 };
	point3d dVec = { 0 };
	point3d fVec = { 0 };

	// copy in saved rotation vectors to local version 
	memcpy(&rVec, &rightVec, sizeof(point3d));
	memcpy(&dVec, &downVec, sizeof(point3d));
	memcpy(&fVec, &forwardVec, sizeof(point3d));

	//apply scaling
	rVec.scale(scale.x);
	dVec.scale(scale.y);
	fVec.scale(scale.z);

	//voxiePtr->drawModel(this->fileName, &pos, &rVec, &dVec, &fVec, colour);

	if (!extentsInited || initExtendsChoice) {


		voxiePtr->drawModelGetExtents(this->fileName, &this->modelExtents, this->flags);
		forceScaleAmount = modelExtents.sc;
		

	}

	float f = 1; // forcescale amount set to 1 if forceScale isn't being used

	if (!forceScale) forceScaleAmount = 0;

	if (forceScale && extentsInited) {

		f = sqrt(double(rVec.x) * double(rVec.x) + double(rVec.y) * double(rVec.y) + double(rVec.z) * double(rVec.z)) * modelExtents.sc * .5f/*for avg below*/;
		pos.x -= (modelExtents.x0 + modelExtents.x1) * f;
		pos.y -= (modelExtents.z0 + modelExtents.z1) * f;
		pos.z += (modelExtents.y0 + modelExtents.y1) * f;

	}

	/*
	Be sure to call voxie_drawspr_getextents() only on the first frame.
	Then remember (i.e. declare global or static) and apply the
	detected ext.sc and offset (the stuff added to gp.x/y/z) to all subsequent frames.
	When you call voxie_drawspr_ext(), be sure to include the forcescale term (ext.sc) and OR (1<<1) to the flags in the last term.

	*/
	//voxiePtr->drawSphere(pos, 0.05, 0, 0xff0000);
	voxiePtr->drawModelExt(filename, &pos, &rVec, &dVec, &fVec, colour, forceScaleAmount, f, flags);


}

void VxModel::draw()
{

	draw(this->fileName, extentsInited);

	/*

	if (voxiePtr == nullptr) return;

	point3d rVec = { 0 };
	point3d dVec = { 0 };
	point3d fVec = { 0 };

	// copy in saved rotation vectors to local version 
	memcpy(&rVec, &rightVec, sizeof(point3d));
	memcpy(&dVec, &downVec, sizeof(point3d));
	memcpy(&fVec, &forwardVec, sizeof(point3d));

	//apply scaling
	rVec.scale(scale.x);
	dVec.scale(scale.y);
	fVec.scale(scale.z);

	//voxiePtr->drawModel(this->fileName, &pos, &rVec, &dVec, &fVec, colour);

	if (!extentsInited) {


		voxiePtr->drawModelGetExtents(this->fileName, &this->modelExtents, this->flags);

		forceScaleAmount = modelExtents.sc;
	}

	float f = 1; // forcescale amount set to 1 if forceScale isn't being used
	
	if (!forceScale) forceScaleAmount = 0;

	if (forceScale && extentsInited) {

		f =		sqrt(double(rVec.x) * double(rVec.x) + double(rVec.y) * double(rVec.y) + double(rVec.z) * double(rVec.z)) * modelExtents.sc * .5f; //
		pos.x -=	(modelExtents.x0 + modelExtents.x1) * f;
		pos.y -=	(modelExtents.z0 + modelExtents.z1) * f;
		pos.z +=	(modelExtents.y0 + modelExtents.y1) * f;

	}

	
	voxiePtr->drawSphere(pos, 0.05, 0, 0xff0000);
	voxiePtr->drawModelExt(this->fileName, &pos, &rVec, &dVec, &fVec, colour, forceScaleAmount, f, flags);

	*/
}

void VxModel::setFilename(const char filename[256])
{
	strcpy(this->fileName, filename);
	this->extentsInited = false;
	point3d pp{ -voxiePtr->getAspectX() - 100, 0, 0 }, rr = { 0,0,0 }, ff = { 0,0,0 }, dd = { 0,0,0 };

	isModelFound = voxiePtr->drawModel(this->fileName, &pp, &rr, &dd, &ff, 0x404040);
}

const char* VxModel::getFilename()
{
	return fileName;
}


void VxModel::setScale(point3d scale)
{
	this->scale = scale;
}

point3d VxModel::getScale()
{
	return this->scale;
}

void VxModel::rotatePitch(float rotateAmountInRadian, bool useDegrees)
{
	if (useDegrees) rotateAmountInRadian *= (PI / 180);
	voxiePtr->rotVex(rotateAmountInRadian, &this->downVec, &this->forwardVec);
}

void VxModel::rotateRoll(float rotateAmountInRadian, bool useDegrees)
{
	if (useDegrees) rotateAmountInRadian *= (PI / 180);
	
	voxiePtr->rotVex(rotateAmountInRadian, &this->rightVec, &this->forwardVec);
}

void VxModel::rotateYaw(float rotateAmountInRadian, bool useDegrees)
{
	if (useDegrees) rotateAmountInRadian *= (PI / 180);

	voxiePtr->rotVex(rotateAmountInRadian, &this->rightVec, &this->downVec);
}

void VxModel::setColour(int newColour)
{
	this->colour = newColour;
}

int VxModel::getColour()
{
	return colour;
}

void VxModel::setPos(point3d newPos)
{
	this->pos = newPos;
}

point3d VxModel::getPos()
{
	return this->pos;
}

void VxModel::setRightVec(point3d newRightVec)
{
	this->rightVec = newRightVec;
}

point3d VxModel::getRightVec()
{
	return this->rightVec;
}

void VxModel::setDownVec(point3d newDownVec)
{
	this->downVec = newDownVec;
}

point3d VxModel::getDownVec()
{
	return this->downVec;
}

void VxModel::setForwardVec(point3d newForwardVec)
{
	this->forwardVec = newForwardVec;
}

point3d VxModel::getForwardVec()
{
	return this->forwardVec;
}

float VxModel::getScalarValue()
{
	return this->modelExtents.sc;
}

void VxModel::setForceScale(int forceScaleValue)
{
	forceScale = forceScaleValue;
}

int VxModel::getForceScale()
{
	return this->forceScale;
}

// Bit 0 :: 0 normal, 1 wireframe
void VxModel::setFlagValue(int flagValues)
{
	this->flags = flagValues;
}

int VxModel::getFlagValue()
{
	return this->flags;
}

void VxModel::report(int posX, int posY)
{
	voxiePtr->debugText(posX, posY, 0xff0000, -1, "Model Report");
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "fileName %d %s", this->isModelFound, this->fileName);
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "pos %1.2f, %1.2f, %1.2f", pos.x, pos.y, pos.z);
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "rVec %1.2f, %1.2f, %1.2f", rightVec.x, rightVec.y, rightVec.z);
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "dVec %1.2f, %1.2f, %1.2f", downVec.x, downVec.y, downVec.z);
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "fVec %1.2f, %1.2f, %1.2f", forwardVec.x, forwardVec.y, forwardVec.z);
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "extends %d, scale %1.2f", this->extentsInited, this->scale);
	posY += 8;

}
