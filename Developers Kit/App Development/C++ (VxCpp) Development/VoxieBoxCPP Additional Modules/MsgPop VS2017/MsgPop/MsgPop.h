#pragma once
#include "vxCpp.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>

/***
 * Message Popper - a pop up message service for VX apps.
 * 
 * 
 */

enum MsgPopDestinations {
	MSGPOP_2_VOLUMETRIC = 0,
	MSGPOP_2_TOUCH_SCREEN = 1,
	MSGPOP_2_TOUCH_SCREEN_DEBUG = 2
};

#define MAX_ORIENTATION 10

typedef struct { point3d position; float hang, vang, twist; } Orientation_t;

typedef struct { char * message; int color, destination, assignedOrientation; double displayTime; float vang, hang, twist; point3d position; } PopMessage_t;

class MsgPop : public IDrawable
{
public:
	MsgPop(IVoxieBox * voxiePtr);
	~MsgPop();
	
	int							add(PopMessage_t newMessage, int orientation = 0);
	int							msgAmend(PopMessage_t * message,char *fmt, ...);
	   
	void						flush(); 
	std::vector<PopMessage_t>	getMessageVector();
	void						setMessageVector(std::vector<PopMessage_t> newMessagesVector);
	void						setOrientation3D(int orientationNo, Orientation_t newOri);  
	void						setOrientation3D(int orientationNo, point3d position, float hang, float vang, float twist);
	Orientation_t				getOrientation3D(int orientationNo);

	void						setOrientation2D(int orientationNo, point2d position);
	point2d						getOrientation2D(int orientationNo);

	void						setAutoAlign(bool choice);
	bool						getAutoAlign();
	void						setFadeTime(int newFadeTime);
	int							getFadeTime();
	void						enableCommonExpiry(bool choice);
	void						setCommonExpiryMessageTime(double newCommonDisplayTime);

	void						draw();
	void						report(int posX, int posY);

	void						setFontSize3D(float textWidth, float textHeight);

private:

	double commonDisplayTime = 3;
	int fadeTime = 7;
	bool commonExpiry = false;
	bool autoAlign = false;
	Orientation_t orientations3D[MAX_ORIENTATION] = { 0 };
	point2d orientations2D[MAX_ORIENTATION] = { 0 };
	float textWidth3d = .1;  
	float textHeight3d = .15; 

	IVoxieBox * voxiePtr = 0;
	std::vector<PopMessage_t> messages;

};

