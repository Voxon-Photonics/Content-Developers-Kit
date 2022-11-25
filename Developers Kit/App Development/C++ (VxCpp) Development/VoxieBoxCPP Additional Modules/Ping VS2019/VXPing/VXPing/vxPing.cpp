#include "vxPing.h"

vxPing::vxPing(IVoxieBox* voxiePtr) {

	this->voxiePtr = voxiePtr;
	this->pingMaxRadius = (voxiePtr->getAspectX() + voxiePtr->getAspectY() + voxiePtr->getAspectZ()) * 1.5;


}

vxPing::~vxPing()
{

}



void vxPing::draw()
{

	int i = 0;
	int col = 0;
	if (voxiePtr == nullptr) return;
	if (!drawable) return; 

	for (i = 0; i < pingNo; i++) {

		col = ping[i].col;

		if (col == PING_RAINBOW_COLOUR) {

			col = voxiePtr->scrollCol(pingNo);

		}

		switch (ping[i].style) {
			default:
			case PING_DELETED:
				continue;
			break;
			case PING_FORWARDS:

				ping[i].rad += (radisUnit * (ping[i].speed * voxiePtr->getDeltaTime()));
				if (fabs(ping[i].rad) > pingMaxRadius) ping[i].style = PING_DELETED;

				voxiePtr->drawSphere(ping[i].pos, ping[i].rad, 0, col);

				break;
			case PING_REVERSE:
				ping[i].rad += (radisUnit * (ping[i].speed * voxiePtr->getDeltaTime()));
				if (fabs(ping[i].rad) > pingMaxRadius) ping[i].style = PING_DELETED;

				voxiePtr->drawSphere(ping[i].pos, pingMaxRadius - ping[i].rad, 0, col);
				break;
			case PING_PULSE:
				if (ping[i].rad < voxiePtr->getTime()) ping[i].style = PING_DELETED;
			

				voxiePtr->drawSphere(ping[i].pos, cos(voxiePtr->getTime() * 2) * .1, 0, col);
				break;


		}

	}



	if (autoCleanUp) this->cleanUp();

}

/* Adds a ping to the stack returns an id number of that ping
	
	@param point3d		pos		the point3d position of where to centre the pink
	@param int			color	the hexadecimal color value use 'PING_RAINBOW_COLOUR' for a rainbow swirl
	@param float		speed	the movement speed for the ping
	@param int (enum)	style	the style / type of the

 */
int vxPing::addPing(point3d pos, int color, float speed, int style)
{
	if (pingNo > MAX_PINGS) {
		pingNo = 0;
	}

	if (pingNo < MAX_PINGS) {

		ping[pingNo].pos.x = pos.x;
		ping[pingNo].pos.y = pos.y;
		ping[pingNo].pos.z = pos.z;
		ping[pingNo].col = color;
		ping[pingNo].rad = 0;
		ping[pingNo].style = style;
		ping[pingNo].speed = speed;

		if (style == PING_PULSE) {

			ping[pingNo].rad = voxiePtr->getTime() + speed;
		}

		pingNo++;
		
	} else {
		return -1;
	}

	return pingNo - 1;

}

int vxPing::addPing(float xpos, float ypos, float zpos, int color, float speed, int style)
{
	point3d pos = { xpos, ypos, zpos };

	return addPing(pos, color, speed, style);
}

void vxPing::clearPing(int pingID)
{
	if (pingID < 0 || pingID > pingNo) return;

	ping[pingID].style = PING_DELETED;
}

void vxPing::clearAll()
{
	int i = 0;
	pingNo = 0;
	for (i = 0; i < MAX_PINGS; i++) {

		ping[i].col = 0x000000;
		ping[i].pos.x = 0;
		ping[i].pos.y = 0;
		ping[i].pos.z = 0;
		ping[i].rad = pingMaxRadius;
		ping[i].speed = 0;
		ping[i].style = PING_UNINITIED;


	}
}

void vxPing::cleanUp()
{
	int i = 0;

	for (i = 0; i < pingNo; i++) {
		if (ping[i].style == PING_DELETED) {
			pingNo--; 
			ping[i] = ping[pingNo]; 
			continue;
		}
	}

}

void vxPing::setisDrawable(bool newValue)
{
	drawable = newValue;
}

bool vxPing::getisDrawable()
{
	return drawable;
}

void vxPing::report(int posX, int posY)
{
	int i = 0;
	voxiePtr->debugText(posX, posY, 0xff0000, 0x9d9d9d, "vxPING report");

	for (i = 0; i < pingNo; i++) {
		voxiePtr->debugText(posX, posY + 8 +  (8 * i), ping[i].col, -1, "%d x: %1.2f y: %1.2f z: %1.2f rad: %1.1f, speed %1.1f, style %d ", i, ping[i].pos.x, ping[i].pos.y, ping[i].pos.z, ping[i].rad, ping[i].speed, ping[i].style);



	}


}


