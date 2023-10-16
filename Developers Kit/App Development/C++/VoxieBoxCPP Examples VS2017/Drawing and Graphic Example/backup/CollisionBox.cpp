#include "pch.h"
#include "collisionBox.h"




collisionBox::collisionBox(Ivoxiebox * voxiePtr, point3d LUTpos, point3d RDBpos, char* setReturnMessage, float setReturnValue)
{
	this->update(voxiePtr, LUTpos, RDBpos, setReturnMessage, setReturnValue);

}

collisionBox::collisionBox() {

	this->update(NULL, point3d{ 0,0,0 }, point3d{ 0,0,0 }, (char*)"newBox", 0);
	active = false;
}


collisionBox::~collisionBox()
{
}

char * collisionBox::getMessage()
{
	return returnMessage;
}

void collisionBox::setMessage(char * newMessage)
{
	returnMessage = newMessage;
}

void collisionBox::setValue(float newValue)
{
	returnValue = newValue;
}

float collisionBox::getValue()
{
	return returnValue;
}

void collisionBox::setLUTPos(point3d LUTpos)
{
	this->LUTpoint = LUTpos;
}

void collisionBox::setRDBPos(point3d RDBpos)
{
	this->RDBpoint = RDBpos;
}

void collisionBox::update(point3d LUTpos, point3d RDBpos, char * setReturnMessage, float setReturnValue)
{
	this->LUTpoint = LUTpos;
	this->RDBpoint = RDBpos;
	this->returnMessage = setReturnMessage;
	this->returnValue = setReturnValue;
	this->active = true;
}


void collisionBox::update(Ivoxiebox * voxiePtr, point3d LUTpos, point3d RDBpos, char * setReturnMessage, float setReturnValue)
{
	this->voxie = voxiePtr;
	this->LUTpoint = LUTpos;
	this->RDBpoint = RDBpos;
	this->returnMessage = setReturnMessage;
	this->returnValue = setReturnValue;
	this->active = true;
}

void collisionBox::setPos(point3d LUTpos, point3d RDBpos)
{
	this->setLUTPos(LUTpos);
	this->setRDBPos(RDBpos);
}

void collisionBox::setIvoxiebox(Ivoxiebox * voxiePtr)
{
	this->voxie = voxiePtr;
}

void collisionBox::setActive(bool option)
{
	active = option;
}

bool collisionBox::isActive()
{
	return active;
}

void collisionBox::setColour(int colour)
{
	this->colour = colour;
}

int collisionBox::checkCollision(point3d cursor)
{
	if (voxie != NULL) {
		return voxie->boxColChk3d(this->LUTpoint, this->RDBpoint, cursor);
	}
	else return NULL;

}

void collisionBox::draw()
{

	voxie->drawBox(this->LUTpoint, this->RDBpoint, 1, this->colour);

}
