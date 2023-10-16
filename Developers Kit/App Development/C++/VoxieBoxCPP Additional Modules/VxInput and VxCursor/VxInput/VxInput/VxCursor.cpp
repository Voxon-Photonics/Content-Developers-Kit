#include "VxCursor.h"

VxCursor::VxCursor(IVoxieBox* voxiePtr)
{
	this->voxiePtr = voxiePtr;
}


VxCursor::~VxCursor()
{
}

void VxCursor::setPos(point3d newPos)
{
	this->pos = newPos;
}

point3d VxCursor::getPos()
{
	return this->pos;
}

void VxCursor::pingDraw() {

	if (ping < PING_MAX) {

		voxiePtr->drawSphere(pos, ping, 0, this->pingCol);
		ping += (pingSpeed * voxiePtr->getDeltaTime());

	}

}

void VxCursor::draw()
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	pingDraw();

	if (this->isFlashing && this->flashAmountNo != 0) {

		if (this->flashTimeDelay  - .125 > voxiePtr->getTime()) {
			return;
		}

		if (this->flashTimeDelay < voxiePtr->getTime() ) {
			flashTimeDelay = voxiePtr->getTime() + .25;

			if (flashAmountNo > 0) flashAmountNo--;
			if (flashAmountNo == 0) isFlashing = false;
			return;
		}

	}

	// draw centre
	voxiePtr->drawVox(pos, this->centreCol);



	float ll = cursorRadius *.5; // ll = line length 

	switch (drawState) {
		case 0: // 0
		default:

			// draw sphere
			voxiePtr->drawSphere(pos, this->cursorRadius, 0, this->cursorCol);

			// draw throb around cursor
				voxiePtr->drawSphere(pos, this->cursorRadius + (abs(cos(voxiePtr->getTime() )) * 0.025), 0, this->cursorCol);

			// draw lines
			voxiePtr->drawLine(pos.x + ll, pos.y, pos.z, pos.x - ll, pos.y, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x , pos.y + ll, pos.z, pos.x, pos.y - ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x, pos.y, pos.z + ll, pos.x, pos.y, pos.z - ll, this->lineCol);


			break;
		case 1: // 1 clicked // pressed
			// draw sphere
			voxiePtr->drawSphere(pos, this->cursorRadius, 1, this->cursorCol);

			// draw throb around cursor
			voxiePtr->drawSphere(pos, this->cursorRadius + (abs(cos(voxiePtr->getTime())) * 0.025), 0, this->cursorCol);


			// draw lines
			voxiePtr->drawLine(pos.x + ll, pos.y, pos.z, pos.x - ll, pos.y, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x, pos.y + ll, pos.z, pos.x, pos.y - ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x, pos.y, pos.z + ll, pos.x, pos.y, pos.z - ll, this->lineCol);

			break;
		case 2:
			// draw sphere
			voxiePtr->drawSphere(pos, this->cursorRadius, 0, this->hiLightCol);

			// draw throb around cursor
			voxiePtr->drawSphere(pos, this->cursorRadius + (cos(voxiePtr->getTime() * 15) * 0.05), 0, this->hiLightCol);

			// draw lines
			voxiePtr->drawLine(pos.x + ll, pos.y, pos.z, pos.x - ll, pos.y, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x, pos.y + ll, pos.z, pos.x, pos.y - ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x - ll, pos.y - ll, pos.z, pos.x + ll, pos.y + ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x + ll, pos.y - ll, pos.z, pos.x - ll, pos.y + ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x, pos.y, pos.z + ll, pos.x, pos.y, pos.z - ll, this->lineCol);
			break;
		case 3: // no no
			// draw sphere
	//		voxiePtr->drawSphere(pos, this->cursorRadius, 1, this->cursorCol);

			// draw throb around cursor
		//	voxiePtr->drawSphere(pos, this->cursorRadius + (cos(voxiePtr->getTime() * 5) * 0.1), 0, this->cursorCol);

			// draw lines

			ll *= 2;



			voxiePtr->drawLine(pos.x - ll, pos.y - ll, pos.z, pos.x + ll, pos.y + ll, pos.z, this->lineCol);
			voxiePtr->drawLine(pos.x + ll, pos.y - ll, pos.z, pos.x - ll, pos.y + ll, pos.z, this->lineCol);
			break;


	}
	





}

point3d VxCursor::movePos(point3d directionXYZ)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return pos;
	double dtim = voxiePtr->getDeltaTime();

	point3d movement;

	movement.x = (directionXYZ.x * cursorMovementSpeedXY) * dtim;
	movement.y = (directionXYZ.y * cursorMovementSpeedXY) * dtim;
	movement.z = (directionXYZ.z * cursorMovementSpeedZ) * dtim;

	pos.x += movement.x;
	pos.y += movement.y;
	pos.z += movement.z;

	if (this->enableClipping) voxiePtr->clipInsideVolume(&pos);

	return pos;
}

point3d VxCursor::movePos(float X, float Y, float Z)
{
	point3d newMovement{ X,Y,Z };
	return movePos(newMovement);
}

bool VxCursor::isClipEnabled()
{
	return this->enableClipping;
}

void VxCursor::setEnabledClip(bool choice)
{
	this->enableClipping = choice;
}

bool VxCursor::colCheck(point3d colPos, float radius)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return false;

	return voxiePtr->sphereCollideChk(&pos, this->cursorRadius, &colPos, radius, false);
}

float VxCursor::getMovementSpeed()
{
	return this->cursorMovementSpeedXY;
}

// 1 is default; 0 is no speed. 0.1 is slowest.... max float is highest (WAY to fast) 
void VxCursor::setMovementSpeed(float newSpeedValueXY, float newSpaceValueZ)
{
	this->cursorMovementSpeedXY = newSpeedValueXY;
	this->cursorMovementSpeedZ = newSpaceValueZ;
}



void VxCursor::startPing()
{

	this->ping = 0;
	
}

void VxCursor::setFlashing(bool flashOn, int amount)
{
	this->isFlashing = flashOn;
	this->flashAmountNo = amount;
}

void VxCursor::setDrawable(bool option)
{
	this->isDrawable = option;
}

bool VxCursor::isDrawableEnabled()
{
	return this->isDrawable;
}

void VxCursor::setDrawState(int newState)
{
	this->drawState = newState;
}

int VxCursor::getDrawState()
{
	return this->drawState;
}

void VxCursor::report(int posX, int posY)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return ;

	voxiePtr->debugText(posX, posY, 0x3090ff, -1, "VxCursor V1 * Report :");
	posY += 8;
	voxiePtr->debugText(posX, posY, cursorCol, -1, "Cursor Pos:  X %1.3f Y %1.3F Z %1.3f ", pos.x, pos.y, pos.z);
	posY += 8;
	voxiePtr->debugText(posX, posY, hiLightCol, -1, "Is flashing:  %d  amount %d time: %1.2f", this->isFlashing, this->flashAmountNo, this->flashTimeDelay);
	posY += 8;
	voxiePtr->debugText(posX, posY, centreCol, -1, "DrawType : $d isClipEnabled %d", this->drawState, this->isClipEnabled());
	posY += 8;
	voxiePtr->debugText(posX, posY, lineCol, -1, "Radius %1.2f, MovementSpeed XY: %1.3f Z: $1.3f", this->cursorRadius, this->cursorMovementSpeedXY, this->cursorMovementSpeedZ);



}
