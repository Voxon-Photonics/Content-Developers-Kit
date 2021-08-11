#include "pch.h"
#include "CollisionBox.h"


CollisionBox::CollisionBox(IVoxieBox * voxiePtr, point3d LUTpos, point3d RDBpos, char* setReturnMessage, float setReturnValue)
{
	this->update(voxiePtr, LUTpos, RDBpos, setReturnMessage, setReturnValue);

}

CollisionBox::CollisionBox() {

	this->update(NULL, point3d{ 0,0,0 }, point3d{ 0,0,0 }, (char*)"newBox", 0);
	active = false;
}

CollisionBox::~CollisionBox()
{
}

char * CollisionBox::getMessage()
{
	return returnMessage;
}

void CollisionBox::setMessage(char * newMessage)
{
	returnMessage = newMessage;
}

void CollisionBox::setValue(float newValue)
{
	returnValue = newValue;
}

float CollisionBox::getValue()
{
	return returnValue;
}

void CollisionBox::setLUTPos(point3d LUTpos)
{
	this->LUTpoint = LUTpos;
}

void CollisionBox::setRDBPos(point3d RDBpos)
{
	this->RDBpoint = RDBpos;
}

void CollisionBox::update(point3d LUTpos, point3d RDBpos, char * setReturnMessage, float setReturnValue)
{
	this->LUTpoint = LUTpos;
	this->RDBpoint = RDBpos;
	this->returnMessage = setReturnMessage;
	this->returnValue = setReturnValue;
	this->active = true;
}


void CollisionBox::update(IVoxieBox * voxiePtr, point3d LUTpos, point3d RDBpos, char * setReturnMessage, float setReturnValue)
{
	this->voxie = voxiePtr;
	this->LUTpoint = LUTpos;
	this->RDBpoint = RDBpos;
	this->returnMessage = setReturnMessage;
	this->returnValue = setReturnValue;
	this->active = true;
}

void CollisionBox::setPos(point3d LUTpos, point3d RDBpos)
{
	this->setLUTPos(LUTpos);
	this->setRDBPos(RDBpos);
}

void CollisionBox::setIVoxieBox(IVoxieBox * voxiePtr)
{
	this->voxie = voxiePtr;
}


void CollisionBox::setColour(int colour)
{
	this->colour = colour;
}

int CollisionBox::checkCollision(point3d cursor)
{
	if (voxie != NULL) {
		return voxie->boxInsideCollideChk(&this->LUTpoint, &this->RDBpoint, cursor);
	}
	else return NULL;

}

void CollisionBox::draw()
{

	voxie->drawBox(this->LUTpoint, this->RDBpoint, 1, this->colour);

}

void CollisionBox::setActive(bool option)
{
	
	if (option == false) setDrawable(false);
	// this->setCollidable(option);
	
}
