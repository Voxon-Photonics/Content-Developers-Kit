#include "pch.h"
#include "PlotPoint.h"


PlotPoint::PlotPoint()
{
}

PlotPoint::PlotPoint(IVoxieBox * voxiePtr, point3d pos, float radius, int fillMode, int col)
{
	this->update(pos, radius, fillMode, col);
	this->setIVoxieBox(voxiePtr);
	this->IActive::setActive(true);

}


PlotPoint::~PlotPoint()
{
}

void PlotPoint::update(point3d pos, float radius, int fillMode, int col)
{
	this->pos = pos;
	this->radius = radius;
	this->fill = fillMode;
	this->colour = col;
}

void PlotPoint::setIVoxieBox(IVoxieBox * voxiePtr)
{
	this->voxie = voxiePtr;
}

void PlotPoint::setHeldValue(double newValue)
{
	this->heldValue = newValue;
}

double PlotPoint::getHeldValue()
{
	return this->heldValue;
}


void PlotPoint::draw() 
{
	if (!IActive::isActive()) return;
	if (voxie == NULL) return;

	voxie->drawSphere(this->pos, this->radius, this->fill, this->colour);

	point3d textPos	= { pos.x - (fontSize * 1.5), pos.y + fontSize, pos.z };
	point3d dVec	= { 0, fontSize, 0 };
	point3d rVec	= { fontSize / 2, 0, 0 };
	
	if (this->showPos) {

		voxie->drawText(&textPos, &rVec, &dVec, 0xffffff, "X %+1.2f\nY %+1.2f\nZ %+1.2f", pos.x, pos.y, pos.z);

	}

}

void PlotPoint::setColour(int colour)
{
	this->colour = colour;
}

int PlotPoint::getColour()
{
	return this->colour;
}

void PlotPoint::setPos(point3d pos)
{
	this->pos = pos;
}

void PlotPoint::setXPos(float x)
{
	this->pos.x = x;
}

void PlotPoint::setYPos(float y)
{
	this->pos.y = y;
}

void PlotPoint::setZPos(float z)
{
	this->pos.z = z;
}

point3d PlotPoint::getPos()
{
	return this->pos;
}

float PlotPoint::getXPos()
{
	return this->pos.x;
}

float PlotPoint::getYPos()
{
	return this->pos.y;
}

float PlotPoint::getZPos()
{
	return this->pos.z;
}

void PlotPoint::setRadius(float radius)
{
	this->radius = radius;
}

float PlotPoint::getRadius()
{
	return this->radius;
}

void PlotPoint::setFillMode(int newFillMode)
{
	this->fill = newFillMode;
}

int PlotPoint::getFillMode()
{
	return this->fill;
}

void PlotPoint::setFontSize(float newFontSize)
{
	this->fontSize = newFontSize;
}

void PlotPoint::setShowValue(bool option)
{
	this->showValue = option;
}

void PlotPoint::setShowPos(bool option)
{
	this->showPos = option;
}

