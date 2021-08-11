#include "pch.h"
#include "PlotPoint.h"


PlotPoint::PlotPoint()
{
}

PlotPoint::PlotPoint(Ivoxiebox * voxiePTR, point3d pos, float radius, int fillMode, int col)
{

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

void PlotPoint::setIvoxiebox(Ivoxiebox * voxiePtr)
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

bool PlotPoint::isActive()
{
	return active;
}

void PlotPoint::draw()
{
	if (!active) return;
	if (voxie == NULL) return;

	voxie->drawSphere(this->pos, this->radius, this->fill, this->colour);


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

void PlotPoint::setActive(bool option)
{
	this->active = option;
}
