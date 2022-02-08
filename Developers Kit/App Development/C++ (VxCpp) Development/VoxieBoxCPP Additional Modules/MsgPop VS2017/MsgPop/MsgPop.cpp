#include "MsgPop.h"

MsgPop::MsgPop(IVoxieBox * voxiePtr)
{
	this->voxiePtr = voxiePtr;
	this->orientations2D[0].x = 30;
	this->orientations2D[0].y = 100;

	Orientation_t ori = { };

	ori.hang = 0;
	ori.vang = 0;
	ori.position.x = -.8;
	ori.position.y = .8;
	ori.position.z = 0;
	this->setOrientation3D(1, ori);

	ori.hang = 180;
	ori.vang = 0;
	ori.position.x = .8;
	ori.position.y = -.8;
	ori.position.z = 0;
	this->setOrientation3D(2, ori);

	ori.hang = 270;
	ori.vang = 0;
	ori.position.x = .8;
	ori.position.y = .8;
	ori.position.z = 0;
	this->setOrientation3D(3, ori);

	ori.hang = 90;
	ori.vang = 0;
	ori.position.x = -.8;
	ori.position.y = -.8;
	ori.position.z = 0;
	this->setOrientation3D(4, ori);
}

MsgPop::~MsgPop()
{
}

// add a message to the MsgPop service
int MsgPop::add(PopMessage_t newMessage, int orientation)
{
	if (voxiePtr == NULL) return 1;
	if (orientation > MAX_ORIENTATION || orientation < 0) orientation = 0;
	if (commonExpiry)	newMessage.displayTime = voxiePtr->getTime() + commonDisplayTime; 

	else  newMessage.displayTime += voxiePtr->getTime();
	messages.insert(messages.begin(), newMessage); // inserts the new message to the top of the vector
	return 0;
}


char* copyString(char s[])
{
	char* s2;
	s2 = (char*)malloc(20);

	strcpy(s2, s);
	return (char*)s2;
}

// This function allows variables to be added to the message 
int MsgPop::msgAmend(PopMessage_t * popMessagePtr, char * fmt, ...)
{
	va_list arglist;
	char buf[1024];
	int i = 0;

	if (!fmt) return 1;
	va_start(arglist, fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&buf, sizeof(buf) - 1, fmt, arglist)) buf[sizeof(buf) - 1] = 0;
#else
	if (vsprintf((char *)&buf, fmt, arglist)) buf[sizeof(buf) - 1] = 0; //NOTE:unsafe!
#endif
	va_end(arglist);

	popMessagePtr->message = copyString(buf);

	return 0;
}

// Clears all messages in the messages vector
void MsgPop::flush()
{
	this->messages.clear();
}

std::vector<PopMessage_t> MsgPop::getMessageVector()
{
	return std::vector<PopMessage_t>();
}

void MsgPop::setMessageVector(std::vector<PopMessage_t> newMessagesVector)
{
	this->messages = newMessagesVector;
}

void MsgPop::setOrientation3D(int orientationNo, Orientation_t newOri)
{
	setOrientation3D(orientationNo, newOri.position, newOri.hang, newOri.vang, newOri.twist);
}

void MsgPop::setOrientation3D(int orientationNo, point3d position, float hang, float vang, float twist)
{
	if (orientationNo > MAX_ORIENTATION || orientationNo < 0) return;

	this->orientations3D[orientationNo].position = position;
	this->orientations3D[orientationNo].hang = hang;
	this->orientations3D[orientationNo].vang = vang;
	this->orientations3D[orientationNo].twist = twist;
}

Orientation_t MsgPop::getOrientation3D(int orientationNo)
{
	Orientation_t orient = { 0 };

	if (orientationNo > MAX_ORIENTATION || orientationNo < 0) return orient;

	orient.hang = orientations3D[orientationNo].hang;
	orient.vang = orientations3D[orientationNo].vang;
	orient.position = orientations3D[orientationNo].position;
	orient.twist = orientations3D[orientationNo].twist;

	return orient;
}

void MsgPop::setOrientation2D(int orientationNo, point2d position)
{
	if (orientationNo > MAX_ORIENTATION || orientationNo < 0) return ;

	this->orientations2D[orientationNo].x = position.x;
	this->orientations2D[orientationNo].y = position.y;

}

point2d MsgPop::getOrientation2D(int orientationNo)
{
	point2d p2 = { 0 };

	if (orientationNo > MAX_ORIENTATION || orientationNo < 0) return p2;
	p2.x = orientations2D[orientationNo].x;
	p2.y = orientations2D[orientationNo].y;
		
	return p2;
}

void MsgPop::setAutoAlign(bool choice)
{
	this->autoAlign = choice;
}

bool MsgPop::getAutoAlign()
{
	return this->autoAlign;
}

// draws the messages
void MsgPop::draw()
{
	if (!drawable || voxiePtr == NULL) return;
	int i = 0;
	int align2D[MAX_ORIENTATION] = { 0 };
	int align3D[MAX_ORIENTATION] = { 0 };
	int const CLEAR_MAX = 20;
	int clearArray[CLEAR_MAX] = { 0 };
	int clearIndex = 0;
	point3d pp = { 0 }, rr = { 0 }, dd = { 0 };

	for (i = 0; i < CLEAR_MAX; i++) clearArray[i] = -1;
	i = 0;
	
	float vang = 0; 
	float hang = 0;
	point3d vx = { 0 } ;
	point3d vy = { 0 };
	float f = 0, ch = 0, sh = 0, cv = 0, sv = 0;

	for (auto msg = messages.begin(); msg != messages.end(); ++msg)
	{
		if (msg->displayTime < voxiePtr->getTime() && clearIndex < CLEAR_MAX) {
			clearArray[clearIndex] = i;
			clearIndex++;
		}
		switch (msg->destination) {
			case MSGPOP_2_TOUCH_SCREEN:

				if (msg->assignedOrientation < 0) { // free mode

					if (autoAlign) {
						voxiePtr->debugText(this->orientations2D[0].x, this->orientations2D[0].y + (align2D[msg->assignedOrientation] * 10), msg->color, -1, msg->message);
					}
					else {
						voxiePtr->debugText(msg->position.x, msg->position.y, msg->color, -1, msg->message);
					}
					break;

				}
				else { // orientation mode

					if (autoAlign) {
						voxiePtr->debugText(this->orientations2D[msg->assignedOrientation].x, this->orientations2D[msg->assignedOrientation].y + (align2D[msg->assignedOrientation] * 10), msg->color, -1, msg->message);
					}
					else {
						voxiePtr->debugText(this->orientations2D[msg->assignedOrientation].x, this->orientations2D[msg->assignedOrientation].y, msg->color, -1, msg->message);
					}
					break;

				}

			case MSGPOP_2_VOLUMETRIC:
				
				if (msg->assignedOrientation < 0) { // free mode 

					// voxiePtr->drawSphere(msg->position, 0.1, 0, 0xff0000);

					f = msg->hang * (PI / 180.f); ch = cos(f); sh = sin(f);
					f = msg->vang * (PI / 180.f); cv = cos(f); sv = sin(f);
					f = msg->twist * (PI / 180.f);

				}
				else {

					// voxiePtr->drawSphere(this->orientations3D[msg->assignedOrientation].position, 0.1, 0, 0xff0000); // debug 

					f = this->orientations3D[msg->assignedOrientation].hang * (PI / 180.f); ch = cos(f); sh = sin(f);
					f = this->orientations3D[msg->assignedOrientation].vang * (PI / 180.f); cv = cos(f); sv = sin(f);
					f = this->orientations3D[msg->assignedOrientation].twist * (PI / 180.f);

				}
				vx.x = ch;
				vx.y = sh;
				vx.z = 0.f;

				vy.x = -sh * cv;
				vy.y = ch * cv;
				vy.z = -sv;

				voxiePtr->rotVex(f, &vx, &vy);

				f = this->textWidth3d * .5f; vx.x *= f; vx.y *= f; vx.z *= f;
				f = this->textHeight3d * .5f; vy.x *= f; vy.y *= f; vy.z *= f;

				pp.x = this->orientations3D[msg->assignedOrientation].position.x;
				pp.y = this->orientations3D[msg->assignedOrientation].position.y;
				pp.z = this->orientations3D[msg->assignedOrientation].position.z;

				if (autoAlign) { //TODO - do this properly! 
					hang = this->orientations3D[msg->assignedOrientation].hang;
		
					voxiePtr->debugText(500, 300, 0xff0090, -1, "hanf %1.2f", hang);
					if (hang > -45 && hang < 45)	pp.y += (align3D[msg->assignedOrientation] * .1);
					if (hang > 45 && hang < 135)	pp.x -= (align3D[msg->assignedOrientation] * .1);
					if (hang > 135 && hang < 225)	pp.y -= (align3D[msg->assignedOrientation] * .1);
					if (hang > 225 && hang < 315)	pp.x += (align3D[msg->assignedOrientation] * .1);
									
				} 
				voxiePtr->drawText(&pp, &vx, &vy, msg->color, msg->message);
				
				break;
		}
		
		i++;

		if (msg->destination == MSGPOP_2_TOUCH_SCREEN_DEBUG || msg->destination == MSGPOP_2_TOUCH_SCREEN) align2D[msg->assignedOrientation]++;
		if (msg->destination == MSGPOP_2_VOLUMETRIC) align3D[msg->assignedOrientation]++;

	}
	
	if (clearIndex > 0) {

		for (i = clearIndex - 1; i >= 0; i--) {

			if (clearArray[i] == -1) continue;

			messages.at(clearArray[i]).color = voxiePtr->tweenCol(messages.at(clearArray[i]).color, this->fadeTime, 0x000000);

			if (messages.at(clearArray[i]).color == 0x000000) {
				messages.erase(messages.begin() + clearArray[i]);
			}
		}

	}
	
}

void MsgPop::report(int posX, int posY)
{
	if (voxiePtr == NULL) return;

	voxiePtr->debugText(posX, posY, 0xff0090, -1, "MsgPop    Compiled On : %s, %s ", __DATE__, __TIME__);
	voxiePtr->debugText(posX, posY,	voxiePtr->randomCol(), -1, "       --");
	posY += 10;
	voxiePtr->debugText(posX, posY, 0xff0090, -1, "AutoAlign    | fadeTime    | drawable    ");
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "          %d             %d             %d", this->autoAlign, this->fadeTime, this->drawable);
	posY += 10;
	voxiePtr->debugText(posX, posY, 0xff0090, -1, "Font W      ,H ");
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "       %1.3f    %1.3f", this->textWidth3d, this->textHeight3d);
	posY += 10;
	voxiePtr->debugText(posX, posY, 0xff0090, -1, "Message Size   | Runtime ");
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "             %d           %1.2f", messages.size(), voxiePtr->getTime());
	posY += 15;
	int i = 0;
	for (auto msg = messages.begin(); msg != messages.end(); ++msg)
	{

		voxiePtr->debugText(posX, posY + (i * 10), msg->color, -1, "%d Dest: %d Kill: %1.2f OriNo %d   %s ", i, msg->destination, 
			msg->displayTime, msg->assignedOrientation, msg->message);
		i++;
	}
	posX += 400; 
	for (i = 0; i < MAX_ORIENTATION; i++) {
		posY += 10;
		voxiePtr->debugText(posX , posY, 0xffffff, -1, "%d Ori 3D Pos X %1.2f Y %1.2f Z %1.2f| HA %1.2f, VA %1.2f, T %1.2f ", i, orientations3D[i].position.x, orientations3D[i].position.y, orientations3D[i].position.z, orientations3D[i].hang, orientations3D[i].vang, orientations3D[i].twist);
	}
	posY += 20;
	for (i = 0; i < MAX_ORIENTATION; i++) {

		voxiePtr->debugText(posX, posY + (i * 10), 0x00ffff, -1, "%d Ori 2D Pos X %1.2f Y %1.2f", i, orientations2D[i].x, orientations2D[i].y);
	}
}

void MsgPop::setFontSize3D(float textWidth, float textHeight)
{
	this->textHeight3d = textHeight;
	this->textWidth3d = textWidth;
}

// Sets the fade out time for messages 
void MsgPop::setFadeTime(int newFadeTime)
{
	if (newFadeTime > 30) newFadeTime = 30;
	else if (newFadeTime < 0) newFadeTime = 100;
	this->fadeTime = newFadeTime;
}

int MsgPop::getFadeTime()
{
	return this->fadeTime;
}

void MsgPop::enableCommonExpiry(bool choice)
{
	this->commonExpiry = choice;
}

void MsgPop::setCommonExpiryMessageTime(double newCommonDisplayTime)
{
	this->commonDisplayTime = newCommonDisplayTime;
}



