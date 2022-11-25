#pragma once
#include "vxCpp.h"
#include <stdio.h>
#include <string.h>

/***
 *  vxPing -- is a class that can manage a 'ping' a little sphere that emits from a point and grows to get focus
 *  
 *  Feel free to edit / use this code by Matthew Vecchio for Voxon Photonics 7/4/2022 
 */
#define MAX_PINGS 100

typedef struct { point3d pos; float rad, speed; int col, style; } ping_t;

enum ping_types {

	PING_RAINBOW_COLOUR = -2,
	PING_DELETED = -1,
	PING_UNINITIED = 0, 
	PING_FORWARDS = 1,
	PING_REVERSE,
	PING_PULSE,
	};

class vxPing : public IDrawable
{
public:
	vxPing(IVoxieBox* voxiePtr);
	~vxPing();


	void	draw(); // draw also updates the ping values
	int		addPing(point3d pos, int color = 0xffffff, float speed = 1, int style = PING_FORWARDS);
	int		addPing(float xpos, float ypos, float zpos, int color = 0xffffff, float speed = 1, int style = PING_FORWARDS);
	void	clearPing(int pingID); // clears the ping by an ID only works if 'autoCleanUp' is disabled
	void	clearAll();
	void    cleanUp();

	void	setisDrawable(bool newValue);
	bool	getisDrawable();

	void	report(int posX, int posY); // use this to debug



private:

	bool autoCleanUp = true; // enable this if you want to clear out the ping pool automatically - WARNING ruins the ID update functionality
	float radisUnit = 0.1; // the base unit speed to increase / decrease
	bool drawable = true;
	int pingNo = 0;
	ping_t ping[MAX_PINGS];
	double pingMaxRadius = 3;
	IVoxieBox* voxiePtr = nullptr; 
};