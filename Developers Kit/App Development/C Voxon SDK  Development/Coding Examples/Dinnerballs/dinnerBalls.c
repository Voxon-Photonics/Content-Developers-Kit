// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1

	#Visual C makefile:
dinnerBalls.exe: dinnerBalls.c voxiebox.h; cl /TP dinnerBalls.c /Ox /MT /link user32.lib
	del dinnerBalls.obj

!else

	#GNU C makefile:
dinnerBalls.exe: dinnerBalls.c; gcc dinnerBalls.c -o dinnerBalls.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif


/*
Balls for Dinner Again 2019 for Global Game Jam
Code : Ready Wolf / Matthew Vecchio
Gfx : Luku Kuku
Music / Sfx : DJ Tr!p
*/

/*
TODO:

Done :
Added Vomiting and Spitting Frenzy Mechanic
Added Progress Bar

** Add AI for players that are inactive
** Add Menu 
** Add 'Speed Eat'n' 
** Add viberation
** Add win animation
** make sure RED and GREEN wins face the right ways

*/



// Memory Management
#define PLAYER_MAX 4
#define FLOOR_MAX 500
#define PARTICLE_MAX 500
#define BALL_MAX 120

enum {

	BALL_NORMAL, BALL_GROWTH, BALL_DOUBLE, BALL_QUAD,
	BALL_VOMIT, BALL_SPICY,


};

// define palete
/*
#define COLOUR_RED 				0x600000
#define COLOUR_BLUE  			0x000060
#define COLOUR_GREEN 		 	0x006000
#define COLOUR_ORANGE		 	0x603000
#define COLOUR_YELLOW		 	0x606000
#define COLOUR_PINK 		 	0x600030
#define COLOUR_MAGENTA 		 	0x600060
#define COLOUR_LIME			 	0x306010
#define COLOUR_CYAN			 	0x006060
#define COLOUR_AQUA 		 	0x006030
#define COLOUR_PURPLE 		 	0x300060
#define COLOUR_VIOLET		 	0x201060
#define COLOUR_LIGHT_RED 		0x501010
#define COLOUR_LIGHT_BLUE 		0x101050
#define COLOUR_LIGHT_GREEN 		0x105010
#define COLOUR_SHADOW 			0x101010
#define COLOUR_GREY 			0x202020
#define COLOUR_WHITE 			0x404040
*/

#define COLOUR_RED 				0xFF0000
#define COLOUR_RED_DIM 			0x100000
#define COLOUR_GREEN_DIM 		0x001000
#define COLOUR_BLUE_DIM 		0x000060
#define COLOUR_YELLOW_DIM 		0x101000

#define COLOUR_BLUE  			0x0000FF
#define COLOUR_GREEN 		 	0x00FF00
#define COLOUR_ORANGE		 	0x996000
#define COLOUR_YELLOW		 	0xFFFF00
#define COLOUR_PINK 		 	0x900060
#define COLOUR_MAGENTA 		 	0xFF0060
#define COLOUR_LIME			 	0x306010
#define COLOUR_CYAN			 	0x006060
#define COLOUR_AQUA 		 	0x009960
#define COLOUR_PURPLE 		 	0x600099
#define COLOUR_VIOLET		 	0x402099
#define COLOUR_LIGHT_RED 		0x501010
#define COLOUR_LIGHT_BLUE 		0x101050
#define COLOUR_LIGHT_GREEN 		0x105010
#define COLOUR_SHADOW 			0x101010
#define COLOUR_GREY 			0x202020
#define COLOUR_WHITE 			0x404040
#define FLOOR_COLOUR			0x808080
#define SPIT_COLOUR				0xFFFFFF

static int rainbowPal[5] = {COLOUR_RED, COLOUR_YELLOW, COLOUR_CYAN, COLOUR_GREEN, COLOUR_MAGENTA};

static int colourPal[17] = {COLOUR_LIGHT_RED,	COLOUR_RED, 		COLOUR_ORANGE, 		COLOUR_YELLOW, 
							COLOUR_LIME,		COLOUR_GREEN, 		COLOUR_LIGHT_GREEN, COLOUR_CYAN,	
							COLOUR_AQUA, 		COLOUR_BLUE, 		COLOUR_VIOLET, 		COLOUR_PURPLE, 
							COLOUR_MAGENTA,		COLOUR_PINK, 		COLOUR_SHADOW,		COLOUR_GREY, 		
							COLOUR_WHITE, };
static int playerCol1[4] = {COLOUR_YELLOW,		COLOUR_CYAN, 		COLOUR_RED, COLOUR_GREEN, };
static int playerCol2[4] = {COLOUR_ORANGE,		COLOUR_PURPLE,		COLOUR_GREY,	COLOUR_LIGHT_RED,};


/*
players
Positions X,y,Z
Velocitiy XV
Spin = applying spin to the vall
ButtonDly = 

*/
typedef struct { float x, y, z, xv, yv, zv, life, r; int col, fill, type, grace; } particle_t;
static particle_t particles[PARTICLE_MAX];
static int particleNo = 0;

typedef struct { float x, y, z, xv, yv, zv, buttonDly, mouthSize, mouthOpenTo; int vomitSnd, score, totalScore, col, col2, orient, size, wins, mouthState;  double mouthOpenTimeDly, mouthStateResetTim, frenzyTim, vomitTim; } player_t;
	
static player_t players[PLAYER_MAX];
static int playerNo = 0;


// floor tile
// type 00 - ground
// type 1 - 4 player  2nd digit is the Zheight

typedef struct { float x, y, z, xv, yv, zv, sHeight; int id, type, col, col2, value, zHeight; bool drawTop, drawLeft, drawRight, drawDown, drawUp, drawShadow, bMap, destroy;  } floor_t;

static floor_t floors[FLOOR_MAX];
static int floorNo = 0;


typedef struct { float x, y, z, xv, yv, zv, speed, sHeight, r, mass; double graceTime, bounceGrace; int type, col, col2, value, eatenBy; bool drawShadow, bounce, destroy, makeParticle; } ball_t;
	
static ball_t balls[BALL_MAX];
static int ballNo = 0;


// player consts
static const float PLAYER_MOVEMENT_X_SPEED = 0.75f; 
static const float PLAYER_MOVEMENT_Z_SPEED = 0.3f; 
static const float PLAYER_MOUTH_OPEN_SPEED = .7f; 
static const double PLAYER_MOUTH_OPEN_TIME_DELAY = .15f; 
static const float PLAYER_WALL_BOUNCE = 1.01f;
static const float PLAYER_MOVEMENT_X_FRICTION = 0.0001f; 
static const float PLAYER_MOVEMENT_Z_FRICTION = 0.0001f; 
static float PLAYER_MOUTH_CLOSED_SIZE = 0.01f; 
static float PLAYER_TEETH_SIZE_X = 0.1f; 
static float PLAYER_TEETH_SIZE_Z = 0.1f; 
static int EATS_TO_WIN = 50; // how many balls are needed to win
static const bool SPIT_REDUCE = false; // if you spit do you become smaller
static const int SPIT_COST_IN_SCORE = 3; // how much score is reduced when spitting
int const JOY_DEADZONE = 10000;


static const float BALL_MIN_SPEED = 0.015f; // setting for X Y mode
static const float BALL_MAX_SPEED = 0.4f;
static const float BALL_FRICTION = .01f;
static const float BALL_WALL_BOUNCE = 1.010;
static const float BALL_FLOOR_BOUNCE = 1.010;
static const float BALL_ZBOUNCE = 0.4;
static const int BALL_COLOUR = 0xFFFF00;
static const float BALL_GRACETIME = 0.01;

static bool DRAW_SHADOWS = false;
static const int SIZE_THRESHOLD = 5;
static const int SIZE_MIN_THRESHOLD = 2;
static int BALLS_TO_GROW = 10;
static const float SPIT_LEFT_XV = -3;
static const float SPIT_LEFT_YV = -3;
static const float SPIT_OPP_XV = 0;
static const float SPIT_OPP_YV = -1;
static const float SPIT_RIGHT_XV = 1;
static const float SPIT_RIGHT_YV = 0.7;
static const float SPIT_GRAVITY = 0.8;
static const float SPIT_SHOOT_DELAY_TIME = 0.75;
static const float SPIT_HIT_DELAY_TIME = 7;
static const float SPIT_MIN_THRESHOLD = 3;
static const float SPIT_FRENZY_TIME = 10;
static const float SPIT_VOMIT_TIME = 5;
static const float SPIT_VOMIT_COST = 15;
static const float SPIT_HIT_TRAVEL_TIME = 0.4;
static const float BOUNCE_GRACE_ADD = 0.0f;
static const int MOUTH_OPEN_TO_SIZE = 6;

static int mapX = 21, mapY = 21, mapZ = 3;
static float ballStartSpeed, scale, halfScale, qtrScale, eighthScale, shotScale;
static voxie_wind_t vw;
static int map[20][20][2]; 

static  float xoffSet = -1.f, yoffSet = -1.f, zoffSet = -1.f;
static  float zFloor =  1.37f;

// game settings
static bool XBOX_CONTROLS = false; // set to true for Xbox controls false for Z3D controls




static int menu_update (int id, char *st, double v, int how, void *userdata)
{
	/*
	switch(id)
	{

		case MENU_VSLIDERNAM:
		switch((int)v)
		{
			case 0: 
			vSlider = .0f;
			break;
			case 1:
			vSlider = .1f;
			break;
			case 2:
			vSlider = .2f;
			break;
			case 3:
			vSlider = .3f;
			break;
			default :
			vSlider = .5f;
			break;

		}
		break;
				case MENU_HSLIDERNAM:
	 	hSlider = (int)v;
	
		break;

		case MENU_UP:   yHeight -= 0.05f ; ghitkey = 0xc8; break; // you can put keycodes here to force actions in the program
		case MENU_ONE:  menuChoice = 1; break;
		case MENU_THREE: menuChoice = 3; break;
		case MENU_DOWN:  yHeight += 0.1f; ghitkey = 0xd0; break;
		case MENU_TWO: menuChoice = 2; break;
		case MENU_GO: goColour = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); break;

	}
	*/
	return(1);
	
}


// Level Setings 
// MAX BALLS,
// BALL RELEASE DELAY 
// noBallsReleasedAtTime
// ball start speed
// draw shadows = 0 
// balls to grow


static float gameSettings[6] = {

25, 1, 2, 0.07f, 0, 10

};

static int lv1MapA[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, }, 
 
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,3,3, 3,3,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,3,3, 3,3,2,2,2, 1,1,1,1,0, }, 

 { 0,1,1,1,1, 2,2,2,3,3, 3,3,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,3,3, 3,3,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, },
 { 0,1,1,1,1, 2,2,2,2,2, 2,2,2,2,2, 1,1,1,1,0, }, 

 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },



						};
static int lv1MapB[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

};


// level 2



static int lv2MapA[20][20] =  {

 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, }, 

 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, }, 

 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,0,0,0, 0,1,1,1,1, 1,1,1,1,0, 0,0,0,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },



};
static int lv2MapB[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,4,4, 4,4,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,4,4, 4,4,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,4,4, 4,4,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,4,4, 4,4,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 



						};


// level 3 high n low


static int lv3MapA[20][20] =  {


 { 0,0,0,0,0, 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,0,0, 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,0,0, 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, }, 

 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, }, 

 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0, },
 { 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0, },



						};
static int lv3MapB[20][20] =  {

 { 3,3,3,3,3, 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, },
 
 { 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 3,3,3,3,3, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, 3,3,3,3,3, },
 { 0,0,0,0,0, 0,0,0,0,0, 3,3,3,3,3, 3,3,3,3,3, },



						};			


// level 4 sky line



static int lv4MapA[20][20] =  {

 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, }, 
 
 { 2,2,2,2,2, 2,2,0,0,0, 0,0,0,2,2, 2,2,2,2,2, },
 { 2,2,2,2,2, 2,2,0,0,0, 0,0,0,2,2, 2,2,2,2,2, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 2,2,2,2,2, 2,2,0,0,0, 0,0,0,2,2, 2,2,2,2,2, },
 { 2,2,2,2,2, 2,2,0,0,0, 0,0,0,2,2, 2,2,2,2,2, }, 

 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, },
 { 1,1,1,1,1, 2,2,0,0,0, 0,0,0,2,2, 1,1,1,1,1, }, 



						};
static int lv4MapB[20][20] =  {

 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, },
 { 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, }, 

 { 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, },
 { 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, 5,5,5,5,5, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,5,5, 5,5,0,0,0, 0,0,0,0,0, }, 



						};								
			
// level 5 holes



static int lv5MapA[20][20] =  {

 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,3,3,3,3, 3,3,3,3,0, 0,0,0,0,0, },
 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, },
 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, },
 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, }, 

 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, },
 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, },
 { 1,1,1,1,0, 0,3,3,3,3, 3,3,3,3,0, 0,1,1,1,1, },
 { 0,0,0,0,0, 0,3,3,3,3, 3,3,3,3,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,1,1,1, 1,1,1,0,0, 0,0,0,0,0, }, 



						};
static int lv5MapB[20][20] =  {

 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, },
 { 2,2,2,2,2, 0,0,0,0,0, 0,0,0,0,0, 2,2,2,2,2, }, 



						};								
				
static int lv6MapA[20][20] =  {

 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, }, 
 
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, }, 

 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, },
 { 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, }, 
 
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, },
 { 1,1,1,1,1, 2,2,2,2,2, 1,1,1,1,1, 2,2,2,2,2, }, 



						};
static int lv6MapB[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 



						};


static int lv7MapA[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, }, 
 
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, },
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, },
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 

 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, },
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, },
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 
 { 0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, }, 

 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 0,0,0,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, },
 { 0,0,0,0,0, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 



						};
static int lv7MapB[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 



						};

static int lv8MapA[20][20] =  {

 { 0,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,0, },
 { 0,0,3,3,3, 3,3,3,3,3, 3,3,3,3,3, 3,3,3,0,0, },
 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, }, 
 
 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,1,1,1,0, 0,1,1,1,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,1,1,1,0, 0,1,1,1,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,1,1,0,0, 0,0,1,1,2, 2,2,3,0,0, },
 { 1,1,3,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,3,1,1, }, 

 { 1,1,3,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,3,1,1, },
 { 0,0,3,2,2, 2,1,1,0,0, 0,0,1,1,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,1,1,1,0, 0,1,1,1,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,1,1,1,0, 0,1,1,1,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, }, 

 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, },
 { 0,0,3,2,2, 2,2,2,2,0, 0,2,2,2,2, 2,2,3,0,0, },
 { 0,0,3,3,3, 3,3,3,3,3, 3,3,3,3,3, 3,3,3,0,0, },
 { 0,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,0,0, }, 



						};
static int lv8MapB[20][20] =  {

 { 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, },
 { 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, }, 
 
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,1,1, 1,1,0,0,0, 0,0,0,4,4, },
 { 4,4,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,4,4, }, 

 { 4,4,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,4,4, },
 { 4,4,0,0,0, 0,0,0,1,1, 1,1,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, }, 

 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,0,0,0, 0,0,0,0,1, 1,0,0,0,0, 0,0,0,4,4, },
 { 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, },
 { 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, 4,4,4,4,4, }, 



						};


static int gcnti[2], gbstat = 0;
					//Rotate vectors a & b around their common plane, by ang
static void rotvex (float ang, point3d *a, point3d *b)
					{
						float f, c, s;
						int i;

						c = cos(ang); s = sin(ang);
						f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
						f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
						f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
					}




/****************
*GAME FUNCTIONS *
*               *
*****************/


// PLAYER Movements

					static void playerMoveUp (int _player, double dtim ) {

					
						players[_player].zv += (-PLAYER_MOVEMENT_Z_SPEED - scale) * dtim;
					//	voxie_debug_print6x8_(200,80,0xffc080,-1,"PLAYER: Move Up"); 

					}

					static void playerMoveDown (int _player, double dtim ) {

				
						players[_player].zv += ( PLAYER_MOVEMENT_Z_SPEED + scale) * dtim;
					//	voxie_debug_print6x8_(200,80,0xffc080,-1,"PLAYER: Move Down"); 
					}

					static void playerMoveLeft (int _player , double dtim ) {
					
							players[_player].xv += ( -PLAYER_MOVEMENT_X_SPEED - scale) * dtim;
						
						
					//	voxie_debug_print6x8_(200,80,0xffc080,-1,"PLAYER: Move Left"); 
					}

					static void playerMoveRight (int _player, double dtim ) {

						players[_player].xv += (PLAYER_MOVEMENT_X_SPEED  + scale) *dtim;
					//	voxie_debug_print6x8_(200,80,0xffc080,-1,"PLAYER: Move Right"); 
					}

					static void playerOpenMouth (int _player ) {
						// SFX add sound effect here
					 
						players[_player].mouthState = 1;
						//players[_player].mouthOpenTo = (players[_player].size * halfScale);
						players[_player].mouthOpenTo = (MOUTH_OPEN_TO_SIZE * qtrScale);
					//	voxie_debug_print6x8_(200,80,0xffc080,-1,"PLAYER: Mouth Eat"); 

					}


// game Functions
// calZHeight works out where to play a floor
// Z -44 is the top .44 is the bottom

static int menu_dinnerBalls_update (int id, char *st, double v, int how, void *userdata)
{





	
}



static float calZHeight(int z, float distance) {

return zFloor - (distance * z) + zoffSet;

}


static void mxPlay (int track) {

	float volume = 100;
	float pitch = 1;

	switch (track) {

		case 0: // title
		voxie_playsound("mx/mxtitle.wav", 1, volume, volume,pitch); 
		break;
		case 1: // game
		voxie_playsound("mx/mx2.wav", 1, volume, volume,pitch); 
		break;
		case 2: // intro
		voxie_playsound("mx/mxintro.wav", 1, volume, volume,pitch); 
		break;





	}



}

	 static void fxPlay (int type, int player) {
//int voxie_playsound (const char *filnam, int chan, int volperc0, int volperc1, float frqmul);
		int volumeBuff = 50;
	 	int volume = 80;
		float pitch = 1;
		int randomNo = 0;
	 	switch (player) {
	 		case 0:
	 		pitch = 0.7;
	 		break;
	 		case 1:
			pitch = 1;
	 		break;
	 		case 2:
			pitch = 1.2;
	 		break;
	 		case 3:
	 		pitch = 0.85;
	 		break;
	




	 	}
	 	//  3 hit
	 	switch (type) {
	 		case 1: // test
	 		//voxie_playsound("test.wav", -1, 100, 100,pitch); 
	 		voxie_playsound("sfx/hit1.wav", -1, volume, volume,1); 
	 		break;
	 		case 2: // eat
	 		randomNo = rand() % 18 + 1;
	 		switch(randomNo) {
	 			case 1:
	 			voxie_playsound("sfx/eat4.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 2:
	 			voxie_playsound("sfx/eat2.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 3:
	 			voxie_playsound("sfx/eat17.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 4:
	 			voxie_playsound("sfx/eat4.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 5:
	 			voxie_playsound("sfx/eat5.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 6:
	 			voxie_playsound("sfx/eat6.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 7:
	 			voxie_playsound("sfx/eat7.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 8:
	 			voxie_playsound("sfx/eat8.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 9:
	 			voxie_playsound("sfx/eat9.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 10:
	 			voxie_playsound("sfx/eat10.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 11:
	 			voxie_playsound("sfx/eat11.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 12:
	 			voxie_playsound("sfx/eat12.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 13:
	 			voxie_playsound("sfx/eat13.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 14:
	 			voxie_playsound("sfx/eat14.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 15:
	 			voxie_playsound("sfx/eat15.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 16:
	 			voxie_playsound("sfx/eat16.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 17:
	 			voxie_playsound("sfx/eat17.wav", -1, volume, volume,pitch); 
	 			break;
	 			case 18:
	 			voxie_playsound("sfx/eat18.wav", -1, volume, volume,pitch); 
	 			break;


	 		}
	 		break;
	 		case 3: // hit
	 		randomNo = rand() % 6 + 1;
	 		switch(randomNo) {
	 			case 1:
	 			voxie_playsound("sfx/hit1.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 2:
	 			voxie_playsound("sfx/hit2.wav", -1, volume+ 50 , volume + 50,pitch); 
	 			break;
	 			case 3:
	 			voxie_playsound("sfx/hit3.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 4:
	 			voxie_playsound("sfx/hit4.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 5:
	 			voxie_playsound("sfx/hit5.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 6:
	 			voxie_playsound("sfx/hit6.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			
	 		

	 		}
	 		break;
			case 4: // spit
	 		randomNo = rand() % 5 + 1;
	 		switch(randomNo) {
	 			case 1:
	 			voxie_playsound("sfx/spit1.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 2:
	 			voxie_playsound("sfx/spit2.wav", -1, volume+ 50 , volume + 50,pitch); 
	 			break;
	 			case 3:
	 			voxie_playsound("sfx/spit3.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;
	 			case 4:
	 			voxie_playsound("sfx/spit4.wav", -1, volume+ 50 , volume + 50,pitch); 
	 			break;
	 			case 5:
	 			voxie_playsound("sfx/spit5.wav", -1, volume + 50, volume + 50,pitch); 
	 			break;





	 	}

	 	break;
	 	case 5: // start 
randomNo = rand() % 10 + 1;
	 		switch(randomNo) {
	 			case 1:
	 			voxie_playsound("sfx/start1.wav", -1, volume + volumeBuff , volume + volumeBuff,pitch); 
	 			break;
	 			case 2:
	 			voxie_playsound("sfx/start2.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 3:
	 			voxie_playsound("sfx/start3.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 4:
	 			voxie_playsound("sfx/start4.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 5:
	 			voxie_playsound("sfx/start5.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 6:
	 			voxie_playsound("sfx/start6.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 7:
	 			voxie_playsound("sfx/start7.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 8:
	 			voxie_playsound("sfx/start8.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 9:
	 			voxie_playsound("sfx/start9.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 10:
	 			voxie_playsound("sfx/start10.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;


	 }
	 break;
				 		case 6: // win
	 		randomNo = rand() % 16 + 1;
	 		switch(randomNo) {
	 			case 1:
	 			voxie_playsound("sfx/win1.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 2:
	 			voxie_playsound("sfx/win2.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 3:
	 			voxie_playsound("sfx/win3.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 4:
	 			voxie_playsound("sfx/win4.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 5:
	 			voxie_playsound("sfx/win5.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 6:
	 			voxie_playsound("sfx/win6.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 7:
	 			voxie_playsound("sfx/win7.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 8:
	 			voxie_playsound("sfx/win8.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 9:
	 			voxie_playsound("sfx/win9.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 10:
	 			voxie_playsound("sfx/win10.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 11:
	 			voxie_playsound("sfx/win11.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 12:
	 			voxie_playsound("sfx/win12.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 13:
	 			voxie_playsound("sfx/win13.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 14:
	 			voxie_playsound("sfx/win14.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 15:
	 			voxie_playsound("sfx/win15.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	 			case 16:
	 			voxie_playsound("sfx/win16.wav", -1, volume + volumeBuff, volume + volumeBuff,pitch); 
	 			break;
	

	

	}

}
}




static void spitBallCreate(int dir, int i) {


	balls[ballNo].x = players[i].x + (halfScale * players[i].size);
	balls[ballNo].z = players[i].z;
	balls[ballNo].y = players[i].y + (halfScale * players[i].size);
	balls[ballNo].col = 0xffffff;
	balls[ballNo].speed = 0.5f;
  balls[ballNo].mass = 1;
	switch (dir) {
		case 0:
		balls[ballNo].xv = .5f;
	break;



	}



}


					static void playerSpitLeft ( int p, double tim ) {
// SFX add spit sound effect here
	fxPlay(4,p); // spit sound
if (particleNo < PARTICLE_MAX) {


										

										switch (players[p].orient) {
											case 0:
											particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = SPIT_LEFT_XV;
											particles[particleNo].yv = SPIT_LEFT_YV;
											break;
											case 1:
											particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = -SPIT_LEFT_XV;
											particles[particleNo].yv = -SPIT_LEFT_YV;
											break;
											case 2:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = 0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = SPIT_LEFT_XV;
											particles[particleNo].yv = -SPIT_LEFT_YV;
											break;
											case 3:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = -0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = -SPIT_LEFT_XV;
											particles[particleNo].yv = SPIT_LEFT_YV;
											break;



										}




										if (players[p].score > SPIT_MIN_THRESHOLD || players[p].frenzyTim > tim ) {
										particles[particleNo].zv = -SPIT_GRAVITY;
										particles[particleNo].r = scale;
									} else {
											particles[particleNo].zv =SPIT_GRAVITY;
											particles[particleNo].r = eighthScale;
									}

										particles[particleNo].col = SPIT_COLOUR;



										particles[particleNo].life =  tim + 10.f;
										

										particles[particleNo].fill = 1;
										particles[particleNo].type = 5; // 5 is spitball
									

	float ggx = particles[particleNo].x ;
										float ggy = particles[particleNo].y ;
										float ggz = particles[particleNo].z ;

										particleNo++;






									
									

		int x = rand() % 4 ;
		int g;
	for(g = 0 ; g < x ; g++ ) { 	// create munch particles 


if (particleNo < PARTICLE_MAX) {





										particles[particleNo].x = ggx;
										particles[particleNo].y = ggy;
										particles[particleNo].z = ggz;
										particles[particleNo].xv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].yv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].zv = -0.8;


										particles[particleNo].col = SPIT_COLOUR;

										particles[particleNo].life =  tim + .1f;
										particles[particleNo].r = eighthScale;
										particles[particleNo].fill = 1;
										particles[particleNo].type = 2;
										particleNo++;




}
}



}


players[p].mouthState = 4;
players[p].mouthStateResetTim = tim + SPIT_SHOOT_DELAY_TIME;

if (players[p].score > 0 || players[p].frenzyTim > tim ) {

if (players[p].totalScore > SPIT_COST_IN_SCORE &&  players[p].frenzyTim < tim) { players[p].score = 0; players[p].totalScore -= SPIT_COST_IN_SCORE; }
//players[p].score = 0;


// work out how to hurt

	switch (players[p].orient) {
											case 0:
							players[3].mouthState = 6;
							players[3].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[3].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[3].size--;
							}   
											break;
											case 1:
							players[2].mouthState = 6;
							players[2].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[2].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[2].size--;
							}   
											break;
											case 2:
							players[0].mouthState = 6;
							players[0].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[0].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[0].size--;
							}  
											break;
											case 3:
							players[1].mouthState = 6;
							players[1].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[1].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[1].size--;
							}  
											break;



										}



					}
}

					static void playerSpitRight ( int p, double tim ) {
// SFX add spit sound effect here
	fxPlay(4,p); // spit sound
if (particleNo < PARTICLE_MAX) {


										

										switch (players[p].orient) {
											case 0:
												particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = -SPIT_LEFT_XV;
											particles[particleNo].yv = SPIT_LEFT_YV;
											break;
											case 1:
											particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = SPIT_LEFT_XV;
											particles[particleNo].yv = -SPIT_LEFT_YV;
											break;
											case 2:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = 0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = SPIT_LEFT_XV;
											particles[particleNo].yv = SPIT_LEFT_YV;
											break;
											case 3:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = -0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = -SPIT_LEFT_XV;
											particles[particleNo].yv = -SPIT_LEFT_YV;
											break;



										}




									if (players[p].score > SPIT_MIN_THRESHOLD || players[p].frenzyTim > tim) {
										particles[particleNo].zv = -SPIT_GRAVITY;
										particles[particleNo].r = scale;
									} else {
											particles[particleNo].zv =SPIT_GRAVITY;
											particles[particleNo].r = eighthScale;
									}

										particles[particleNo].col = SPIT_COLOUR;



										particles[particleNo].life =  tim + 10.f;
										

										particles[particleNo].fill = 1;
										particles[particleNo].type = 5; // 5 is spitball
									

	float ggx = particles[particleNo].x ;
										float ggy = particles[particleNo].y ;
										float ggz = particles[particleNo].z ;

										particleNo++;






									
									

		int x = rand() % 4 ;
		int g;
	for(g = 0 ; g < x ; g++ ) { 	// create munch particles 


if (particleNo < PARTICLE_MAX) {





										particles[particleNo].x = ggx;
										particles[particleNo].y = ggy;
										particles[particleNo].z = ggz;
										particles[particleNo].xv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].yv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].zv = -0.8;


										particles[particleNo].col = SPIT_COLOUR;

										particles[particleNo].life =  tim + .1f;
										particles[particleNo].r = eighthScale;
										particles[particleNo].fill = 1;
										particles[particleNo].type = 2;
										particleNo++;




}
}



}


players[p].mouthState = 4;
players[p].mouthStateResetTim = tim + SPIT_SHOOT_DELAY_TIME;

if (players[p].score > 0 || players[p].frenzyTim > tim ) {

if (players[p].totalScore > SPIT_COST_IN_SCORE &&  players[p].frenzyTim < tim) { players[p].score = 0; players[p].totalScore -= SPIT_COST_IN_SCORE; }


// work out how to hurt

	switch (players[p].orient) {
											case 0:
							players[2].mouthState = 6;
							players[2].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[2].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[2].size--;
							}   
											break;
											case 1:
							players[3].mouthState = 6;
							players[3].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[3].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[3].size--;
							}   
											break;
											case 2:
							players[1].mouthState = 6;
							players[1].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[1].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[1].size--;
							}  
											break;
											case 3:
							players[0].mouthState = 6;
							players[0].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[0].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[0].size--;
							}  
											break;



										}



					}
}




					static void playerSpitOpp ( int p, double tim ) {
// SFX add spit sound effect here
	fxPlay(4,p); // spit sound
if (particleNo < PARTICLE_MAX) {


										

										switch (players[p].orient) {
											case 0:
											particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = 0;
											particles[particleNo].yv = SPIT_LEFT_YV;
											break;
											case 1:
											particles[particleNo].x = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].y = players[p].y;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = 0;
											particles[particleNo].yv = -SPIT_LEFT_YV;
											break;
											case 2:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = 0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = SPIT_LEFT_XV;
											particles[particleNo].yv = 0;
											break;
											case 3:
											particles[particleNo].y = players[p].x  + (halfScale * players[p].size);
											particles[particleNo].x = -0.95;
											particles[particleNo].z = players[p].z;
											particles[particleNo].xv = -SPIT_LEFT_XV;
											particles[particleNo].yv = 0;
											break;



										}




									if (players[p].score > SPIT_MIN_THRESHOLD || players[p].frenzyTim > tim) {
										particles[particleNo].zv = -SPIT_GRAVITY;
										particles[particleNo].r = scale;
									} else {
											particles[particleNo].zv =SPIT_GRAVITY;
											particles[particleNo].r = eighthScale;
									}

										particles[particleNo].col = SPIT_COLOUR;



										particles[particleNo].life =  tim + 10.f;
										

										particles[particleNo].fill = 1;
										particles[particleNo].type = 5; // 5 is spitball
									

	float ggx = particles[particleNo].x ;
										float ggy = particles[particleNo].y ;
										float ggz = particles[particleNo].z ;

										particleNo++;






									
									

		int x = rand() % 4 ;
		int g;
	for(g = 0 ; g < x ; g++ ) { 	// create munch particles 


if (particleNo < PARTICLE_MAX) {





										particles[particleNo].x = ggx;
										particles[particleNo].y = ggy;
										particles[particleNo].z = ggz;
										particles[particleNo].xv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].yv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].zv = -0.8;


										particles[particleNo].col = SPIT_COLOUR;

										particles[particleNo].life =  tim + .1f;
										particles[particleNo].r = eighthScale;
										particles[particleNo].fill = 1;
										particles[particleNo].type = 2;
										particleNo++;




}
}



}


players[p].mouthState = 4;
players[p].mouthStateResetTim = tim + SPIT_SHOOT_DELAY_TIME;

if (players[p].score > 0 || players[p].frenzyTim > tim ) {

if (players[p].totalScore > SPIT_COST_IN_SCORE &&  players[p].frenzyTim < tim) { players[p].score = 0; players[p].totalScore -= SPIT_COST_IN_SCORE; }


// work out how to hurt

	switch (players[p].orient) {
											case 0:
							players[1].mouthState = 6;
							players[1].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[1].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[1].size--;
							}   
											break;
											case 1:
							players[0].mouthState = 6;
							players[0].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[0].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[0].size--;
							}   
											break;
											case 2:
							players[3].mouthState = 6;
							players[3].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[3].size > SIZE_MIN_THRESHOLD && SPIT_REDUCE == true) {
								players[3].size--;
							}  
											break;
											case 3:
							players[2].mouthState = 6;
							players[2].mouthStateResetTim = tim + SPIT_HIT_TRAVEL_TIME;
							if (players[2].size > SIZE_MIN_THRESHOLD  && SPIT_REDUCE == true) {
								players[2].size--;
							}  
											break;



										}



					}
}




// x y z, xv, yv, type, col
static void ballCreate() {

int x;

		balls[ballNo].makeParticle = true;
		balls[ballNo].col = BALL_COLOUR;
		balls[ballNo].speed = ballStartSpeed;
		balls[ballNo].type = BALL_NORMAL;
		balls[ballNo].r = eighthScale;
		  balls[ballNo].mass = 1;


		if (rand() % 30 == 1 ) { // colour scroll ball makes you grow

			balls[ballNo].type =  BALL_GROWTH;
					balls[ballNo].col = 0xFFFFFF;
		} else if (rand() % 20 == 1 ) {
			balls[ballNo].type = BALL_DOUBLE;
			balls[ballNo].r = qtrScale;
			balls[ballNo].mass = 0.8;
				
		} else if (rand() % 40 == 1 ) {
			balls[ballNo].type = BALL_QUAD;
			balls[ballNo].r = halfScale;
			balls[ballNo].mass = 0.6;
				
		} else if (rand() % 30 == 1 ) {
			balls[ballNo].type = BALL_VOMIT;
			balls[ballNo].r = eighthScale;
					balls[ballNo].col = 0x00FF00;
				
		} else if (rand() % 30 == 1 ) {
			balls[ballNo].type = BALL_SPICY;
			balls[ballNo].r = eighthScale;
			balls[ballNo].col = 0xFF0000;
				
		}


		ballNo++;	


}				



		


static void ballEaten(int i, int j, double *tim) {
int k;
	fxPlay(2,j);


				players[j].score++;
				players[j].totalScore++;




				switch (balls[i].type) {
					case 1:
					// this fixing the size growing clipping bug
 					if (players[j].x + ( (players[j].size + 2 ) * scale) + players[j].xv - scale >= vw.aspx) {
					players[j].x -= scale; };

					players[j].size++;
					break;
					case BALL_DOUBLE:
					players[j].score++;
				
			  	players[j].totalScore++;
					break;
					case BALL_QUAD:
					players[j].score++;
					players[j].score++;
					players[j].score++;
			  	players[j].totalScore++;
					players[j].totalScore++;
					players[j].totalScore++;
					break;
					case BALL_SPICY: // give wacky eyes
					players[j].frenzyTim = *tim + SPIT_FRENZY_TIME;
					
					// add programming for spicy ball effect here
							// (unlimited spits )
					break;
					case BALL_VOMIT:
							players[j].vomitTim = *tim + SPIT_VOMIT_TIME;
						for (k = 0; k < SPIT_VOMIT_COST; k++) {
						if (players[j].totalScore > 0)	players[j].totalScore -= 1;
						}
							fxPlay(3,j);
					// add programming for vomit ball effect here
					// (spew out current score)
			
					break;

				}
	




				
				if (players[j].score > BALLS_TO_GROW ) {
					players[j].score = 0;
						// this fixing the size growing clipping bug
 				if (players[j].x + ( (players[j].size + 2 ) * scale) + players[j].xv - scale >= vw.aspx) {
				players[j].x -= scale; };
					players[j].size++;
				}

				
				
				balls[i].destroy = true;
				// SFX add each sound here
			
				balls[i].z = players[j].z + players[j].mouthSize;
				balls[i].yv = 0;
				balls[i].xv = 0;
			

			switch(players[j].orient) {
				case 0:
				balls[i].y = +vw.aspy - qtrScale;
				break;
				case 1:
				balls[i].y = -vw.aspy + qtrScale;
				break;
				case 3:
				balls[i].x = -vw.aspx + qtrScale;
				break;
				case 2:
				balls[i].x = vw.aspx - qtrScale;
				break;

			}

			



}

static void ballReset(int ballNo) {

		//	balls[ballNo].x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
		//	balls[ballNo].y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
		//	balls[ballNo].z = (float)((rand()&32767)-16384)/16384.f*(vw.aspz - (vw.aspz / 2)) - (vw.aspz / 2);
		
			balls[ballNo].x = (float)rand()/RAND_MAX /10;
			balls[ballNo].y = (float)rand()/RAND_MAX /10;
			balls[ballNo].z = -vw.aspz + eighthScale;
			balls[ballNo].zv = 0.2;
			balls[ballNo].xv = (float)rand()/RAND_MAX /2;
			balls[ballNo].yv = (float)rand()/RAND_MAX /2;
			balls[ballNo].destroy = false;
		  //balls[ballNo].col = 0x0000ff;
			balls[ballNo].makeParticle = true;
			//balls[ballNo].type = -1;
			balls[ballNo].r = eighthScale;
			balls[ballNo].col = BALL_COLOUR;
			balls[ballNo].type = BALL_NORMAL;
				if (rand() % 30 == 1 ) { // colour scroll ball makes you grow

			balls[ballNo].type =  BALL_GROWTH;
					balls[ballNo].col = 0xFFFFFF;
		} else if (rand() % 20 == 1 ) {
			balls[ballNo].type = BALL_DOUBLE;
			balls[ballNo].r = qtrScale;
			balls[ballNo].mass = 0.8;
				
		} else if (rand() % 40 == 1 ) {
			balls[ballNo].type = BALL_QUAD;
			balls[ballNo].r = halfScale;
			balls[ballNo].mass = 0.6;
				
		} else if (rand() % 30 == 1 ) {
			balls[ballNo].type = BALL_VOMIT;
			balls[ballNo].r = eighthScale;
					balls[ballNo].col = 0x00FF00;
				
		} else if (rand() % 30 == 1 ) {
			balls[ballNo].type = BALL_SPICY;
			balls[ballNo].r = eighthScale;
			balls[ballNo].col = 0xFF0000;
				
		}



		
		if (rand() % 2 == 1) { balls[ballNo].x = -balls[ballNo].x;	}
		if (rand() % 2 == 1) { balls[ballNo].y = -balls[ballNo].y;	}
		if (rand() % 2 == 1) { balls[ballNo].xv = -balls[ballNo].xv;	}
		if (rand() % 2 == 1) { balls[ballNo].yv = -balls[ballNo].yv;	}

			//balls[ballNo].xv = 0;
			//balls[ballNo].yv = 1;



}

// 1


		





	int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
	{
		voxie_frame_t vf;
		voxie_inputs_t in;
		pol_t pt[3];
		double d, tim = 0.0, otim, dtim,  avgdtim = 0.0, titleWaitTime, floorFallTim, keyStoneDly, colourScrollDly, ballReleaseDly, startDinnerTim, ballReleaseDlyAmount, maxBallsInPlay;
		int i,j,k, mousx = 256, mousy = 256, mousz = 0, colourScroll = 0, floorID, noBallsReleasedAtTime;
		point3d ss, tp, pp, spp, rr, dd, ff, sff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
		voxie_xbox_t vx[4];
		bool floorFallCheck = false;
			poltex_t vtext[20]; int mesh[20]; 
		
// game variables setup
		int noPlayers = 0, x,  y, z, zz, xx, yy, inited = 0, debug = 0, ovxbut[4], vxnplays, col[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00}; 
		float gravity = 0, f, g, zFloorTrue, scaleOffSet = 1;
		static int loadMapNo = 1;
		int winPlayer = 0; // set this to 5
		bool eyeFlash, startSound = true;


		// music player settings
		static int mx = -1, mxVol = 100, mxLoop = 1, mxVolDefLevel = 100, mxFade = 0, curSong = 0;
		static double mxLoopTim = 0, timend = 0.0l,  mxTitleSampLength = 97.0, mxGameLength = 106.0;
		bool titleScreen = true;

	// game settings
		noPlayers = 4; 
		gravity = -0.8f;
		zFloorTrue = zFloor + zoffSet;
	
		mapY = 20; mapX = 20; mapZ = 2;
	
       





	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }

/*
	voxie_menu_reset(menu_update,0, 0); 
	voxie_menu_addtab("Balls",		450,0,500,350); // add a tab to the top of the menu be careful not to add to many as they will be hidden on the VX1
	voxie_menu_additem("Game Mode", 50,10, 64, 64,0   ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); // adding menu text
*/

	 voxie_mountzip("sfx/sfx.zip");

	while (!voxie_breath(&in)) // Breath must mean as its updating loop
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);
		for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1

		}




// GLOBAL KEYS

		// put controls here keys here
		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		//press 1 (load level 1 )
		if (voxie_keystat(0x02)) { inited = 0;loadMapNo = 1;	mapY = 20; mapX = 20;	}
		if (voxie_keystat(0x03)) { inited = 0;loadMapNo = 2;	mapY = 20; mapX = 20;	}
		if (voxie_keystat(0x04)) { inited = 0;loadMapNo = 3;	mapY = 20; mapX = 20;	} // 2
		if (voxie_keystat(0x05)) { inited = 0;loadMapNo = 4;	mapY = 20; mapX = 20;	}
		if (voxie_keystat(0x06)) { inited = 0;loadMapNo = 5;	mapY = 20; mapX = 20;	}
		if (voxie_keystat(0x07)) { inited = 0;loadMapNo = 6;	mapY = 20; mapX = 20;	}
		if (voxie_keystat(0x08)) { inited = 0;loadMapNo = 0;	mapY = 20; mapX = 20;	} // 2
		if (voxie_keystat(0x09)) { inited = 0;loadMapNo = 8;	mapY = 20; mapX = 20;	}
		// key stone
		// movements


		if (tim > keyStoneDly) {

			keyStoneDly = tim + 0.1f;

		if (voxie_keystat(0x4b)) { inited = 0;;	 xoffSet += .1; } 
		if (voxie_keystat(0x4d)) { inited = 0;	 xoffSet += -.1; }
		if (voxie_keystat(0x48)) { inited = 0;	 yoffSet += -.1; } 
		if (voxie_keystat(0x4c)) { inited = 0;	 yoffSet += .1; }

		// scale offset
		if (voxie_keystat(0x4a)) { inited = 0;	 scaleOffSet += -.05; } 
		if (voxie_keystat(0x4e)) { inited = 0;	 scaleOffSet += .05; }
}
		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
			if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

			//draw wireframe box
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		//voxie_drawvox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

#if 1
		/**************
		*   START OF  *
		* Innit Phase *
		**************/






		if (inited == 0) {

if (titleScreen == true) {

voxie_playsound_update(mx, 0, 0, 0, 1.0);
curSong = 0;
mx = voxie_playsound("mx/mxtitle.wav", 0, mxVol, mxVol,1.0); 
mxLoopTim = mxTitleSampLength + tim; // start music


}

// clear the map
for(y=0;y<mapY;y++) 
						for(x=0;x<mapX;x++)
							for(z=0;z<mapZ;z++)
							{
map[y][x][z] = 0;
}





			// create game
			inited = 1;
			playerNo = 0;
			floorNo = 0;
			floorID = 0;
			ballNo = 0;
			startDinnerTim = tim + rand() % 3;
			startSound = true;
			if (loadMapNo == 0) loadMapNo = rand() % 8 + 1;

			// load in settings
			maxBallsInPlay = gameSettings[0];
			ballReleaseDlyAmount = gameSettings[1];
			noBallsReleasedAtTime = gameSettings[2];
			ballStartSpeed = gameSettings[3];
			if (gameSettings[4] == 1 ) DRAW_SHADOWS = true;
			else DRAW_SHADOWS = false;
			BALLS_TO_GROW = gameSettings[5];



			// work out scale
			scale = 2 / float(mapY) * scaleOffSet;
			halfScale = scale /2;
			qtrScale = scale /4;
			eighthScale = scale / 8;
			shotScale = eighthScale / 1;
			PLAYER_MOUTH_CLOSED_SIZE = halfScale;
			PLAYER_TEETH_SIZE_Z = halfScale;
			PLAYER_TEETH_SIZE_X = halfScale * 3;
			 
			// TODO: add clear memory stuff here


//TODO: turn this into arrays


if(loadMapNo == 1) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv1MapA[y][x];
break;

case 1:
map[y][x][z] =  lv1MapB[y][x];
break;
} } }

if(loadMapNo == 2) {



for(y=0;y<mapY;y++) 
						for(x=0;x<mapX;x++)
							for(z=0;z<mapZ;z++)
							{
switch (z){
case 0:
map[y][x][z] =  lv2MapA[y][x];
break;

case 1:
map[y][x][z] =  lv2MapB[y][x];
break;
}

}
				}


if(loadMapNo == 3) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv3MapA[y][x];
break;

case 1:
map[y][x][z] =  lv3MapB[y][x];
break;
} } }


if(loadMapNo == 4) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv4MapA[y][x];
break;

case 1:
map[y][x][z] =  lv4MapB[y][x];
break;
} } }

if(loadMapNo == 5) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv5MapA[y][x];
break;

case 1:
map[y][x][z] =  lv5MapB[y][x];
break;
} } }

if(loadMapNo == 6) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv6MapA[y][x];
break;

case 1:
map[y][x][z] =  lv6MapB[y][x];
break;
} } }

if(loadMapNo == 7) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv7MapA[y][x];
break;

case 1:
map[y][x][z] =  lv7MapB[y][x];
break;
} } }


if(loadMapNo == 8) {


for(y=0;y<mapY;y++) 
for(x=0;x<mapX;x++)
for(z=0;z<mapZ;z++)
{
switch (z){
case 0:
map[y][x][z] =  lv8MapA[y][x];
break;

case 1:
map[y][x][z] =  lv8MapB[y][x];
break;
} } }


// turn the Map array in floor objects
	for(y=0;y<mapY;y++) {

		for(x=0;x<mapX;x++)
		{
				for(z=0;z<mapZ;z++) {

			if (!map[y][x][z]) continue;
			if (map[y][x][z] >= 0 && map[y][x][z] <= 9) {
				// create floors

				// player sections are if 10 >  < 19 player 1
				// player sections are if 10 >  < 19 player 1

			
				floors[floorNo].type = 0; // type zero is a normal floor
				pp.y = scale * y + yoffSet + qtrScale;
				pp.x = scale * x + xoffSet + qtrScale;
	   			pp.z = calZHeight( map[y][x][z], scale + eighthScale);
	   			floors[floorNo].zHeight = map[y][x][z];
	   			
	   				if (z == 1) {
					floors[floorNo].bMap = true;
					floors[floorNo].col = 0x600060;
				} else {
					floors[floorNo].bMap = false;
					floors[floorNo].col = 0x006000;
				}

		

				floors[floorNo].x = pp.x; floors[floorNo].y = pp.y; floors[floorNo].z = pp.z;
				floors[floorNo].xv = 0; floors[floorNo].yv = 0; floors[floorNo].zv = 0;
			
				floors[floorNo].id = floorID;
							

				} // end of switch



						floorNo++;
						floorID++;
			} // end of floor creation

			
		}
	}









	// create players create
				for (i = 0; i < noPlayers; i++) {

					switch(i) {
						case 0: // player 1 pos
						players[playerNo].x = xoffSet + (scale * ( mapX / 2) ) - scale ; players[playerNo].xv = 0.f;  // 0.f
						players[playerNo].y = yoffSet + (scale *  mapY) - scale ; players[playerNo].yv = 0.f;  // and the Y side
						players[playerNo].orient = 0;
						break;
						case 1: // player 2 pos
						players[playerNo].x = xoffSet + (scale * ( mapX / 2) ) - scale; players[playerNo].xv = 0.f; 
						players[playerNo].y =  yoffSet + scale; players[playerNo].yv = 0.f; 
						players[playerNo].orient = 1;
						break;
						case 2: // player 3 pos
						players[playerNo].x =  xoffSet +  (scale * ( mapY / 2) ) - scale ;					players[playerNo].xv = 0.f; 
						players[playerNo].y =  yoffSet + (scale * ( mapY / 2) )  - scale ; 	players[playerNo].yv = 0.f; 
						players[playerNo].orient = 2;
						break;
						case 3: // player 4 pos
						players[playerNo].y = yoffSet + (scale * ( mapY / 2) ) - scale; players[playerNo].xv = 0.f; 
						players[playerNo].x = xoffSet + (scale * ( mapX / 2) ) - scale; 	players[playerNo].yv = 0.f;
						players[playerNo].orient = 3; 
						break;
					
					}
					players[playerNo].size = 3;
					players[playerNo].mouthSize = PLAYER_MOUTH_CLOSED_SIZE;
					players[playerNo].mouthState = 0;
					
					players[playerNo].wins = 0;
					players[playerNo].frenzyTim = 0;
					players[playerNo].z = 0;
					players[playerNo].score = 0; 
					players[playerNo].totalScore = 0; 
					players[playerNo].col = playerCol1[playerNo];
					players[playerNo].col2 = playerCol2[playerNo];
					playerNo++;
				}

			






			}
		/**************
		*   END OF  *
		* Innit Phase *
		**************/






		/**************************
		*   START OF UPDATE LOOP  *
		*                         *
		**************************/

			/** MISC STUFF **/


			// colourScroll -- update to be once per frame
			if (colourScrollDly < tim) {
			colourScroll++;
			eyeFlash = false;
			if (colourScroll > 4) { colourScroll = 0; eyeFlash = true; } 
			colourScrollDly = tim + 0.2f;
			}

			// ball release
			if (ballNo <= maxBallsInPlay && tim > ballReleaseDly && startDinnerTim < tim) {

				if (startSound == true && titleScreen == false) {
					fxPlay(5,1);
					startSound = false;
				}

				ballReleaseDly = ballReleaseDlyAmount + tim;
				for ( i = 0; i < noBallsReleasedAtTime; i++) {
				ballCreate();
				ballReset(ballNo -1 );
				}
			
			}





if (  voxie_keystat(0xcb) && players[0].buttonDly < tim )  { // move left


			players[0].buttonDly = 0.1f + tim;
		 if (players[0].x + ( (players[0].size + 2 ) * scale) + players[0].xv - scale >= vw.aspx) {
				players[0].x -= scale; };
			players[0].size++;





	}

	else if (  voxie_keystat(0xcd)  && players[0].buttonDly < tim  )  { // move right

			players[0].buttonDly = 0.1f + tim;
			players[0].size--;

	}







// PLAYER / PLAYER UPDATE LOOP

// PLAYER Update


			for (i = 0; i < noPlayers; i++) {


if (titleScreen == true) {

if (titleWaitTime > tim ) continue;
if (titleWaitTime < tim ) winPlayer = 5; // set this back to 5 for finish

if ( vx[i].but&~ovxbut[i]&(1<<12) ||vx[i].but&~ovxbut[i]&(1<<13) || vx[i].but&~ovxbut[i]&(1<<14) || vx[i].but&~ovxbut[i]&(1<<15) || vx[i].but&~ovxbut[i]&(1<<4) ) {

	titleScreen = false;
	inited = 0;
	loadMapNo = 0;
	mxLoopTim = 0;
	voxie_playsound_update(mx, 0, 0, 0, 1.0);
	curSong = 1;
	mx = voxie_playsound("mx/mx2.wav", 0, mxVol, mxVol,1.0); 
	mxLoopTim = mxGameLength + tim; // start music



}





	continue;
}




if (players[i].totalScore > EATS_TO_WIN) {
fxPlay(6,i);
loadMapNo = 0;
inited = 0;
titleScreen = true;
curSong = 0;
voxie_playsound_update(mx, 0, 0, 0, 1.0);
mx = voxie_playsound("mx/mxtitle.wav", 0, mxVol, mxVol,1.0); 
mxLoopTim = mxTitleSampLength + tim; // start music
titleWaitTime = tim + 5;
winPlayer = i;


}

// PLAYER CONTROLS LOOP


// put in keyboard controls here


// Joystick controls

if (players[i].vomitTim < tim) {

				if (i <  vxnplays) { 

	if ( (vx[i].but>>1)&1  || vx[i].ty0 < -JOY_DEADZONE   ) { // move down

	playerMoveDown(i,  dtim);
		

	}

	else if ( (vx[i].but>>0)&1  || vx[i].ty0 > JOY_DEADZONE  )  { // move up

	playerMoveUp(i, dtim);
	}

	if ( (vx[i].but>>2)&1  || vx[i].tx0 < -JOY_DEADZONE  )  { // move left

		switch(players[i].orient) {
			case 0:
			case 3:

	playerMoveLeft(i,  dtim);
			break;

			case 1:
			case 2:
	playerMoveRight(i,  dtim);

			break;




		}

	}

	else if ( (vx[i].but>>3)&1  || vx[i].tx0 > JOY_DEADZONE  )  { // move right

	switch(players[i].orient) {
			case 0:
			case 3:
	playerMoveRight(i,  dtim);
			break;

			case 1:
			case 2:
playerMoveLeft(i,  dtim);
			break;




		}

	}
	
	if ( vx[i].but&~ovxbut[i]&(1<<14) && players[i].mouthState == 0) {


	playerOpenMouth(i);




	}


		if ( vx[i].but&~ovxbut[i]&(1<<12)  && players[i].score >  SPIT_MIN_THRESHOLD ||  vx[i].but&~ovxbut[i]&(1<<12)  && players[i].frenzyTim > tim ) {


		
			playerSpitLeft(i, tim);

		}


			if ( vx[i].but&~ovxbut[i]&(1<<13)  && players[i].score >  SPIT_MIN_THRESHOLD ||  vx[i].but&~ovxbut[i]&(1<<13)  && players[i].frenzyTim > tim ) {


			playerSpitRight(i, tim);

		}

			if ( vx[i].but&~ovxbut[i]&(1<<15)   && players[i].score >  SPIT_MIN_THRESHOLD ||  vx[i].but&~ovxbut[i]&(1<<15)  && players[i].frenzyTim > tim ) {


			
			playerSpitOpp(i, tim);

		}

	} // end of controls


	// mouth movement states
	switch (players[i].mouthState) {
		case 0: // closed

		break;
		case 1: // opening
		players[i].mouthSize =  players[i].mouthSize + (PLAYER_MOUTH_OPEN_SPEED * players[i].size) * dtim;

		if (players[i].mouthSize > players[i].mouthOpenTo) {
			players[i].mouthOpenTimeDly = tim + PLAYER_MOUTH_OPEN_TIME_DELAY;
			players[i].mouthState = 2;

		}
		break;
		case 2:
		if (players[i].mouthOpenTimeDly < tim) {
				players[i].mouthOpenTo = PLAYER_MOUTH_CLOSED_SIZE;
				players[i].mouthState = 3;
		}

		break;
		case 3:
			players[i].mouthSize =  players[i].mouthSize - (PLAYER_MOUTH_OPEN_SPEED * players[i].size) * dtim;
if (players[i].mouthSize < players[i].mouthOpenTo) {
			players[i].mouthSize = PLAYER_MOUTH_CLOSED_SIZE;
			players[i].mouthState = 0;

		}

		break;
case 4: // mouth spit
		
if (players[i].mouthStateResetTim < tim) {
			players[i].mouthSize = PLAYER_MOUTH_CLOSED_SIZE;
			players[i].mouthState = 0;

		}

		break;
case 5: // mouth hit 
		
if (players[i].mouthStateResetTim < tim) {
			players[i].mouthSize = PLAYER_MOUTH_CLOSED_SIZE;
			players[i].mouthState = 0;

		}
		
		break;
case 6: // mouth hit 
		
if (players[i].mouthStateResetTim < tim) {
			players[i].mouthStateResetTim =  tim + SPIT_HIT_DELAY_TIME;
			players[i].mouthState = 5;
			// SFX add hit sounds hit
			fxPlay(3, i);
		}
		
		break;

	}


} else { // if player is vomiting


if (colourScroll == 1 ) {


	players[i].mouthSize = players[i].mouthOpenTo;
	players[i].zv = (float)((rand() & 32767) - 16384) / 16384.f;
	players[i].zv *= dtim;

	if (players[i].vomitSnd == 0) {
fxPlay(3,i);
players[i].vomitSnd = 1;
	}	
			
	


	if (particleNo < PARTICLE_MAX) {


			switch(players[i].orient) {
				case 0: // player 1
				particles[particleNo].x = players[i].x + (halfScale * players[i].size);
				particles[particleNo].y = players[i].y - halfScale;
				break;
				case 1: // player 2
				particles[particleNo].x = players[i].x + (halfScale * players[i].size);
				particles[particleNo].y = players[i].y + halfScale;
				break;
				case 2: // player 3
				particles[particleNo].x = 0.95;
				particles[particleNo].y = players[i].x + (halfScale * players[i].size); 
				break;
				case 3: // player 4
				particles[particleNo].x = -0.95;
				particles[particleNo].y = players[i].x + (halfScale * players[i].size); 
				break;
							}

			

											particles[particleNo].z = players[i].z;
											particles[particleNo].xv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
											particles[particleNo].yv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
											particles[particleNo].zv = 0.5;


											particles[particleNo].col = 0x00ff00;

											particles[particleNo].life =  tim + .1f;
											particles[particleNo].r = scale;
											particles[particleNo].fill = 0;
											particles[particleNo].type = 6;
											particleNo++;

	}
	
} else {
players[i].vomitSnd = 0;
}

}

// PLAYER Colision 

	//	if (players[i].zv != 0 || players[i].xv != 0 ) {


			if (players[i].z + players[i].zv <= -vw.aspz   ) {
				players[i].zv = -players[i].zv * PLAYER_WALL_BOUNCE;
			
			}

			if ( players[i].z + players[i].mouthSize +  players[i].zv  >= vw.aspz && players[i].mouthState != 0 ) { // if mouth is growing add the offset 
			playerMoveUp(i, dtim);
				//players[i].zv -= players[i].mouthSize;
			} else if (players[i].z + players[i].mouthSize +  players[i].zv  >= vw.aspz) { 

				players[i].zv = -players[i].zv * PLAYER_WALL_BOUNCE;
			}


		
			if (players[i].x + players[i].xv <= -vw.aspx  || players[i].x + ( (players[i].size + 1 ) * scale) + players[i].xv - scale >= vw.aspx) {
				players[i].xv = -players[i].xv * PLAYER_WALL_BOUNCE;
				continue;

			}
		
			
	


			players[i].z +=  players[i].zv;
			if (i == 0 || 1) {
			players[i].x +=  players[i].xv;
			} else { // inverse the controlls for players 3 and 4
				players[i].y +=  players[i].xv;
			}

/* slide controls
			if (players[i].zv < 0.00001) {  players[i].zv = (players[i].zv - PLAYER_MOVEMENT_Z_FRICTION) * dtim;   }
			if (players[i].zv > 0.00001) {  players[i].zv = (players[i].zv + PLAYER_MOVEMENT_Z_FRICTION) * dtim;   }
			else {
				players[i].zv = 0;
			}	

			if (players[i].xv < 0.00001) {  players[i].xv = (players[i].xv + PLAYER_MOVEMENT_X_FRICTION) * dtim;   }
			if (players[i].xv > 0.00001) {  players[i].xv = (players[i].xv - PLAYER_MOVEMENT_X_FRICTION) * dtim;   }
			else {
				players[i].xv = 0;
			}	
	*/

			// non slide controls
			players[i].zv = 0;
			players[i].xv = 0;
			
	//	} // end of player collision dection of bounds
	


// HUD for players

if (players[i].frenzyTim > tim) players[i].col2 = 0xFF0000;
else if (players[i].vomitTim > tim) players[i].col2 = 0x00FF00;
else { players[i].col2 = 0x404040;}

// show eat meter
	switch (players[i].orient)
	{
	case 0: // player 1
	g = (   0.5 / EATS_TO_WIN ) * players[i].totalScore;
	voxie_drawcone (&vf,  0 - g,  vw.aspy - halfScale,  vw.aspz - scale,  0.02f,
                      0 + g,  vw.aspy - halfScale,  vw.aspz - scale,  0.02f,
                                         1,  players[i].col);
		break;
	case 1: // player 2
		g = (   0.5 / EATS_TO_WIN ) * players[i].totalScore;
	voxie_drawcone (&vf,  0 - g,  -vw.aspy + halfScale,  vw.aspz - scale,  0.02f,
                      0 + g,  -vw.aspy + halfScale,  vw.aspz - scale,  0.02f,
                                         1,  players[i].col);
	 break;	
	case 3: // player 4
		g = (   0.5 / EATS_TO_WIN ) * players[i].totalScore;
	voxie_drawcone (&vf, -vw.aspx + halfScale , 0 - g,  vw.aspz - scale,  0.02f,
                     -vw.aspx + halfScale  ,  0 + g,  vw.aspz - scale,  0.02f,
                                         1,  players[i].col);	
	break;
	case 2: // player 3
		g = (   0.5 / EATS_TO_WIN ) * players[i].totalScore;
	voxie_drawcone (&vf, vw.aspx - halfScale , 0 - g,  vw.aspz - scale,  0.02f,
                     vw.aspx - halfScale  ,  0 + g,  vw.aspz - scale,  0.02f,
                                         1,  players[i].col);	
	break;
	default:
		break;
	}

} // end of player update


	

	
// ball update

	for(i=ballNo-1;i>=0;i--) {






if (balls[i].type == BALL_GROWTH  && balls[i].destroy == false) balls[i].col = rainbowPal[colourScroll];


if (balls[i].speed < BALL_MIN_SPEED) balls[i].speed = BALL_MIN_SPEED;
if (balls[i].speed > BALL_MAX_SPEED) balls[i].speed = BALL_MAX_SPEED;

balls[i].speed -= ((BALL_FRICTION * balls[i].mass) * dtim);


		if (balls[i].xv < 0) balls[i].x += (balls[i].xv + -balls[i].speed) * dtim;
		if (balls[i].xv > 0) balls[i].x += (balls[i].xv + balls[i].speed) * dtim;
		if (balls[i].yv < 0) balls[i].y += (balls[i].yv + -balls[i].speed) * dtim;
		if (balls[i].yv > 0) balls[i].y += (balls[i].yv + balls[i].speed) * dtim;

		balls[i].z += balls[i].zv * dtim;





	
  			balls[i].zv -= gravity * dtim;
	 

	

        if (balls[i].drawShadow == true && DRAW_SHADOWS == true) {
        	  f = (shotScale + ((-balls[i].z + 1) * .20 ) * .10 )* scaleOffSet;
		  voxie_drawsph(&vf,balls[i].x,balls[i].y,balls[i].sHeight,f,1,COLOUR_BLUE);
        }



        	balls[i].drawShadow = false;


		if ( balls[i].bounce == true) {
	if (balls[i].zv > 0) { // ball bounce when it hits the ground
		balls[i].zv /= 2;
		balls[i].zv = -balls[i].zv;
	} else if (balls[i].bounceGrace < tim) {
		balls[i].zv = 0;
	}
		
balls[i].bounce = false;

		} 

		if (balls[i].z > vw.aspz) { // when a ball falls out of the level teleport it somewhere


			ballReset(i);



		}

			
	

		// the ball hits the outter walls


		if  (fabs(balls[i].x + (balls[i].xv * dtim )) >= vw.aspx || (fabs(balls[i].y + (balls[i].yv * dtim )))  >= vw.aspy ) {
		if  (fabs(balls[i].x + (balls[i].xv * dtim )) >= vw.aspx) {  balls[i].xv = -balls[i].xv; balls[i].speed *= BALL_WALL_BOUNCE;  }
		if  (fabs(balls[i].y + (balls[i].yv * dtim )) >= vw.aspy) {  balls[i].yv = -balls[i].yv; balls[i].speed *= BALL_WALL_BOUNCE;  }
		

		}

		if ( (balls[i].z + (balls[i].zv * dtim )) <= -vw.aspz) { balls[i].zv = -balls[i].zv / 2;}



	 	// if the ball hits a Floor
	 	if (balls[i].destroy == false) { 
		for (j = 0; j < floorNo; j++) {
	if (


				balls[i].x > floors[j].x 	     &&
				balls[i].x < floors[j].x + scale &&
				balls[i].y > floors[j].y  	     &&
				balls[i].y < floors[j].y + scale &&
				balls[i].z > floors[j].z - halfScale &&
				balls[i].z < floors[j].z + halfScale ) {



		if (balls[i].bounceGrace < tim ) {

		if (floors[j].bMap == false) {     // if ball is up the top it will just roll 

		k  = rand() % 10;

		switch (k) {
			case 0:
			case 1:
			case 2:
			case 4:
				balls[i].zv -= ( BALL_ZBOUNCE  );
				balls[i].bounceGrace = tim + 0.1f;
			break;
			case 5:
			balls[i].zv -= ( (BALL_ZBOUNCE )* 2 );
			balls[i].bounceGrace = tim + 0.3f;
			break;
			case 6:
			balls[i].zv -= ( (BALL_ZBOUNCE ) * 2.5 );
			balls[i].bounceGrace = tim + 0.4f;
			break;
			case 7:
		if (rand() % 2 == 1) { balls[ballNo].xv = -balls[ballNo].xv;	}
		if (rand() % 2 == 1) { balls[ballNo].yv = -balls[ballNo].yv;	}
		break;
			case 8:
			balls[i].bounceGrace = tim + 0.5f;
			break;

			case 9:
			balls[i].zv -= ( (BALL_ZBOUNCE ) * 2 );
			balls[i].bounceGrace = tim + 0.3f;
			break;
			case 10:
				balls[ballNo].xv = (float)rand()/RAND_MAX /2;
			balls[ballNo].yv = (float)rand()/RAND_MAX /2;

		if (rand() % 2 == 1) { balls[ballNo].xv = -balls[ballNo].xv;	}
		if (rand() % 2 == 1) { balls[ballNo].yv = -balls[ballNo].yv;	}
			break;




		}
	}


		balls[i].bounceGrace += BOUNCE_GRACE_ADD;
	
		balls[i].bounce = true;
	
	    }
	}



		if (


				balls[i].x > floors[j].x 	     &&
				balls[i].x < floors[j].x + scale &&
				balls[i].y > floors[j].y  	     &&
				balls[i].y < floors[j].y + scale &&
				balls[i].z < floors[j].z + halfScale ) {

			balls[i].drawShadow = true;
			balls[i].sHeight = floors[j].z;
		} 




}
}

		// if the ball hits a PLAYER

		for (j = 0; j < noPlayers; j++) {


			//TODO: fix .9 offset is a hack to make it work it won't scale correctly


			switch (players[j].orient) {

				case 0: // orient 1 ball collsision decection / player 1
			if (
				balls[i].x > players[j].x &&
				balls[i].x < players[j].x + (scale * players[j].size) &&
				balls[i].y > .9 &&
				balls[i].z > players[j].z - halfScale &&
				balls[i].z < players[j].z + players[j].mouthSize &&
				players[j].mouthState > 0 && players[j].mouthState < 4   && balls[i].destroy == false && players[j].vomitTim < tim
			 ) {

				ballEaten(i, j, &tim);




			}
				 
				break;
				case 1: // orient 2 ball collsision decection / player 2
			if (
				balls[i].x > players[j].x &&
				balls[i].x < players[j].x + (scale * players[j].size) &&
				balls[i].y < -.9 &&
				balls[i].z > players[j].z - halfScale &&
				balls[i].z < players[j].z + players[j].mouthSize &&
				players[j].mouthState > 0 && players[j].mouthState < 4  && balls[i].destroy == false && players[j].vomitTim < tim
			 ) {

				ballEaten(i, j,&tim);

			}
				 
				break;
					case 2: // orient 1 ball collsision decection / player 3

			if (
				balls[i].y > players[j].x  &&
				balls[i].y < players[j].x + (scale * players[j].size) &&
				balls[i].x > .9 &&
				balls[i].z > players[j].z - halfScale &&
				balls[i].z < players[j].z + players[j].mouthSize &&
				players[j].mouthState > 0 && players[j].mouthState < 4  && balls[i].destroy == false && players[j].vomitTim < tim
			 ) {



			
	
				ballEaten(i, j, &tim);


			}
				 
				break;
						case 3: // orient 2 ball collsision decection / player 2
			if (
				balls[i].y > players[j].x  &&
				balls[i].y < players[j].x + (scale * players[j].size) &&
				balls[i].x < -.9 &&
				balls[i].z > players[j].z - halfScale &&
				balls[i].z < players[j].z + players[j].mouthSize &&
				players[j].mouthState > 0 && players[j].mouthState < 4  && balls[i].destroy == false && players[j].vomitTim < tim
			 ) {

				ballEaten(i, j, &tim);

			}
				 
				break;


			} // end of switch for collission detection

	  	} // end of ball hit player


	if (balls[i].destroy == true  && balls[i].makeParticle == true) {

		balls[i].makeParticle = false;

		x = rand() % 30 ;
	for(k = 0 ; k < x ; k++ ) { 	// create munch particles 


if (particleNo < PARTICLE_MAX) {





										particles[particleNo].x = balls[i].x;
										particles[particleNo].y = balls[i].y;
										particles[particleNo].z = balls[i].z;
										particles[particleNo].xv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].yv = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
										particles[particleNo].zv = -0.8;


										particles[particleNo].col = balls[i].col;

										particles[particleNo].life =  tim + 1.f;
										particles[particleNo].r = eighthScale;
										particles[particleNo].fill = 1;
										particles[particleNo].type = 2;
										particleNo++;




}
}

		balls[i].speed = 0;
		balls[i].xv = 0;
		balls[i].yv = 0;
		balls[i].col = 0x000000; // get rid of the apperance of a ball

	


		ballReset(i);


		//ballNo--; balls[i] = balls[ballNo]; continue;
	}


	}	// end of update balls	


// floor update

	for (j = floorNo; j >= 0; j--) {

floors[j].drawTop = true;


// only draw from the top (looks better)

//floors[j].drawLeft = true;
//floors[j].drawRight = true;
//floors[j].drawUp = true;
//floors[j].drawDown = true;
floors[j].drawShadow = false;
// check if there is a floor below

if (floors[j].zHeight == 0) {

	continue;
}



			for (k = floorNo; k >= 0; k--) {

if (floors[j].id == floors[k].id) continue;



			// work out wether to draw the top
				if (floors[j].x == floors[k].x 	     &&
					floors[j].y == floors[k].y 		 &&
					floors[j].zHeight + 1 == floors[k].zHeight

		 	) {

				floors[j].drawTop = false;
			} 

			if (floors[j].bMap == true) {


	if (floors[j].x == floors[k].x 	     		       &&
		floors[j].y == floors[k].y 					   &&
		floors[j].zHeight > floors[k].zHeight          &&
		floors[k].bMap == false) {

	floors[j].drawShadow = true;
	floors[j].sHeight = floors[k].z;
		}





}





/*

			// work out wether to draw the right
			if (floors[j].x + halfScale < floors[k].x 	     					&&
				floors[j].x + scale + halfScale + qtrScale > floors[k].x        &&
				floors[j].y == floors[k].y 										&&
				floors[j].zHeight == floors[k].zHeight

		 	) {

				floors[j].drawRight = false;
			} 


				// work out wether to draw the left
			if (floors[j].x - halfScale   > floors[k].x 	  &&
				floors[j].x - scale - qtrScale < floors[k].x  	  &&
				floors[j].y == floors[k].y 					  &&
				floors[j].zHeight == floors[k].zHeight

		 	) {

				floors[j].drawLeft = false;
			} 

			// work out wether to draw the up
			if (floors[j].y - halfScale   > floors[k].y 	  &&
				floors[j].y - scale - qtrScale < floors[k].y  	  &&
				floors[j].x == floors[k].x 					  &&
				floors[j].zHeight == floors[k].zHeight

		 	) {

				floors[j].drawUp = false;
			} 

				// work out wether to draw the down
			if (floors[j].y + halfScale < floors[k].y 	     					&&
				floors[j].y + scale + halfScale + qtrScale > floors[k].y        &&
				floors[j].x == floors[k].x 										&&
				floors[j].zHeight == floors[k].zHeight

		 	) {

				floors[j].drawDown = false;
			} 
*/




} // end of floor draw check




		
				
		
} // end of floor update
			
	







#endif

	/*********************
    **	 DRAW GFX PHASE **
	*********************/

// draw particles
// particle update particle
  for (i = particleNo - 1; i >= 0; i--) {


 particles[i].zv -=  gravity*dtim * 2;
particles[i].x += particles[i].xv*dtim;
					particles[i].y += particles[i].yv*dtim;
					particles[i].z += particles[i].zv*dtim;
voxie_drawsph(&vf,particles[i].x,particles[i].y,particles[i].z,particles[i].r,particles[i].fill,particles[i].col);


if (particles[i].type == 5 &&  ((particleNo < PARTICLE_MAX))   ) { // draw ball trace if ball is not on ground
	   

	  				     
          particles[particleNo].x = particles[i].x;
          particles[particleNo].y = particles[i].y;
          particles[particleNo].z = particles[i].z;
          particles[particleNo].xv = 0.0;
          particles[particleNo].yv = 0.0;
          particles[particleNo].zv = 0.0;
          particles[particleNo].col = SPIT_COLOUR;
          particles[particleNo].life = tim + 1;
          particles[particleNo].type = 0;
          particles[particleNo].r = eighthScale;
          particles[particleNo].fill = 0;
          particleNo++;

}

if (particles[i].type == 6 &&  ((particleNo < PARTICLE_MAX))   ) { // draw ball trace if ball is not on ground
	   

	  				     
          particles[particleNo].x = particles[i].x;
          particles[particleNo].y = particles[i].y;
          particles[particleNo].z = particles[i].z;
          particles[particleNo].xv = 0.0;
          particles[particleNo].yv = 0.0;
          particles[particleNo].zv = 0.0;
          particles[particleNo].col = 0xffff00;
          particles[particleNo].life = tim + 1;
          particles[particleNo].type = 0;
          particles[particleNo].r = eighthScale;
          particles[particleNo].fill = 0;
          particleNo++;

}

/*



          if ((particleNo < PARTICLE_MAX)) { // draw ball trace

            particles[particleNo].x = particles[i].x + halfScale;
            particles[particleNo].y = particles[i].y + halfScale;
            particles[particleNo].z = particles[i].z;
            particles[particleNo].xv = 0.0;
            particles[particleNo].yv = 0.0;
            particles[particleNo].zv = -0.01f;
            particles[particleNo].col = 0xffffff;
            particles[particleNo].life = tim + 1;
            particles[particleNo].type = 0;
            particles[particleNo].r = qtrScale;
            particles[particleNo].fill = 0;
            particleNo++;

          }

if (particles[i].type == 0) {
        voxie_drawsph( & vf, particles[i].x, particles[i].y, particles[i].z, particles[i].r, particles[i].fill, particles[i].col);
}
*/
   //   if (fabs(particles[i].x) > vw.aspx || fabs(particles[i].y) > vw.aspy || fabs(particles[i].z) > vw.aspz ) particles[i].life = 0.0f;

      if (particles[i].life < tim) {

        particleNo--;
        particles[i] = particles[particleNo];
        continue;

      

  }
} // end of particles





// draw floors
/*
	rr.x = scale ; rr.y = 0.0f; rr.z = 0.0f; // floor size X
	dd.x = 0.0f; dd.y = scale; dd.z = 0.0f; // floor size Y
	ff.x = 0.0; ff.y = 0.0f; ff.z = halfScale + qtrScale  ; // floor height
*/	
/* old draw floors
rr.x = scale - eighthScale; rr.y = 0.0f; rr.z = 0.0f; // floor size X
	dd.x = 0.0f; dd.y = scale - eighthScale; dd.z = 0.0f; // floor size Y
	ff.x = 0.0; ff.y = 0.0f; ff.z = halfScale + qtrScale  ; // floor height
*/	



// special; ffx for 
	sff.x = 0.0; sff.y = 0.0f; sff.z = eighthScale ; // was qtrScale



for ( i = floorNo - 1; i >= 0; i--) {


	rr.x = scale ; rr.y = 0.0f; rr.z = 0.0f; // floor size X
	dd.x = 0.0f; dd.y = scale; dd.z = 0.0f; // floor size Y
	ff.x = 0.0; ff.y = 0.0f; ff.z = halfScale + qtrScale  ; // floor height

	if (floors[i].destroy == true) {


		floorNo--; floors[i] = floors[floorNo]; continue;
	}




	pp.x = floors[i].x; pp.y = floors[i].y; pp.z = floors[i].z;


// draw the floor
//	voxie_drawcube(&vf,&pp, &rr, &dd, &ff, 1, floors[i].col);	




	// draw the top
if (floors[i].drawTop == true) {


//	spp.x = floors[i].x; spp.y = floors[i].y; spp.z = floors[i].z - eighthScale; // old thickness
	spp.x = floors[i].x; spp.y = floors[i].y; spp.z = floors[i].z;
	


	if (floors[i].bMap == true) {
		voxie_drawcube(&vf,&pp, &rr, &dd, &ff, 1, 0x404040);	
		voxie_drawcube(&vf,&spp, &rr, &dd, &sff, 1, 0x808080);	
	} else {
			// single colour mode 
	voxie_drawcube(&vf,&spp, &rr, &dd, &sff, 1, FLOOR_COLOUR);	
	}
}

/*

if (floors[i].drawLeft == true) {
// draw left
	spp.x = eighthScale; spp.y = rr.y; spp.z = rr.z;
	dd.y = scale - eighthScale;
	voxie_drawcube(&vf,&pp, &spp, &dd, &ff, 2, floors[i].col);	
}


// draw up
if (floors[i].drawUp == true) {
	spp.x = scale - eighthScale; spp.y = rr.y; spp.z = rr.z;
	dd.y = eighthScale;
	voxie_drawcube(&vf,&pp, &spp, &dd, &ff, 2, floors[i].col);	
}
// draw right

if (floors[i].drawRight == true) {
	pp.x = floors[i].x + scale - qtrScale; pp.y = floors[i].y;
	spp.x = eighthScale; spp.y = rr.y; spp.z = rr.z;
	dd.y = scale - eighthScale;
	voxie_drawcube(&vf,&pp, &spp, &dd, &ff, 2, floors[i].col);	
}
// draw bottom	
if (floors[i].drawDown == true) {
	pp.x = floors[i].x; pp.y = floors[i].y + scale - eighthScale;
	spp.x = scale - eighthScale; spp.y = rr.y; spp.z = rr.z;
	dd.y = eighthScale;
	voxie_drawcube(&vf,&pp, &spp, &dd, &ff, 2, floors[i].col);	
}
*/


if (floors[i].drawShadow == true && DRAW_SHADOWS == true) {
	//TODO fix this
	rr.x = scale - eighthScale; rr.y = 0.0f; rr.z = 0.0f; // floor size X
	dd.x = 0.0f; dd.y = scale - eighthScale; dd.z = 0.0f; // floor size Y
	ff.x = 0.0; ff.y = 0.0f; ff.z = qtrScale  ; // floor height
	spp.x = floors[i].x; spp.y = floors[i].y; spp.z = floors[i].sHeight;

	voxie_drawcube(&vf,&spp, &rr, &dd, &ff, 2, COLOUR_BLUE_DIM);	


}


} // end of floor draw






	// Draw players


if (titleScreen == false) {

      dd.z = 0.0f;
      ff.x = 0.0;
      ff.y = 0.0f;
      rr.y = 0.0f;
      rr.z = 0.0f;
      dd.x = 0.0f;
      dd.y = scale; // PLAYER size

      ff.z = halfScale; // PLAYER height

	  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;



	for (i = 0; i < noPlayers; i++) {



	  pp.x = players[i].x; pp.y = players[i].y; pp.z = players[i].z;
      rr.x = scale * players[i].size;

      // scaled pp's are here
      spp.x = pp.x + (halfScale * players[i].size);
      spp.y = pp.y + (halfScale * players[i].size);

	
switch(players[i].orient) {
case 0:
	  spp.x = pp.x + (halfScale * players[i].size);
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;



	// draw mouth
if (players[i].mouthState == 4) { // spit


	 vtext[0].x = spp.x - (scale * 1.5) ; 	vtext[0].y = spp.y - 0.07; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = spp.x + (scale * 2); vtext[1].y = spp.y - 0.07 ; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x + (scale * 2);  vtext[2].y = spp.y - 0.07; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].x = spp.x - (scale * 1.5) ; vtext[3].y = spp.y - 0.07; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthSpit.png",vtext,5,mesh,6,2,players[i].col2);


	} else if (players[i].mouthState == 5) { // hurt



 	vtext[0].x = spp.x - (scale * 1.5) ;	vtext[0].y = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = spp.x + (scale * 2);  vtext[1].y = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x + (scale * 2);  vtext[2].y = spp.y; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].x = spp.x - (scale * 1.5) ; vtext[3].y = spp.y; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthOuch.png",vtext,5,mesh,6,2,players[i].col2);



	} else {

		 // top left PP
      vtext[0].x = players[i].x; 	vtext[0].y = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = sff.x; vtext[1].y = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = sff.x;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z + players[i].mouthSize ; 
      
     				 
        // bottom left /DD

      vtext[3].x = players[i].x; vtext[3].y = spp.y; vtext[3].z = players[i].z + players[i].mouthSize; 
      
	  voxie_drawmeshtex(&vf,"img/mouth.png",vtext,5,mesh,6,2,players[i].col2);
	

		// new titled way for teeth

	  for (j = 0; j < players[i].size; j++) {

	 // top left PP
      vtext[0].x = players[i].x + ( j * scale) ; vtext[0].y = spp.y - .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].x = players[i].x + ( j * scale) + scale ; vtext[1].y = spp.y - .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x + ( j * scale) + scale ;  vtext[2].y = spp.y - .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = players[i].x + ( j * scale) ; vtext[3].y = spp.y - .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeethTile.png",vtext,5,mesh,6,2,players[i].col2);

		 // top left PP
       vtext[0].y = spp.y - .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.y - .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
        vtext[2].y = spp.y - .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.y - .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeethTile.png",vtext,5,mesh,6,2,players[i].col2);


}

} // end of teeth


if (players[i].mouthState == 5 || players[i].vomitTim > tim) {

        // draw eyes left
	  
		 // top left PP
      vtext[0].x = players[i].x - (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);




} else {

        // draw eyes left
	  
		 // top left PP
      vtext[0].x = players[i].x - (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 

        // draw eyes right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 

}
          // nose
	  
		 // top left PP
      vtext[0].x = spp.x - scale  ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 2);
            
       // top right // RR
      
      vtext[1].x = spp.x + scale  ; vtext[1].y = spp.y ; vtext[1].z = players[i].z - (scale * 2);
     
       // bottom right // FF
      	 
      vtext[2].x = spp.x + scale  ;  vtext[2].y = spp.y - .05; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = spp.x - scale  ; vtext[3].y = spp.y - .05; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/nose.png",vtext,5,mesh,6,2,players[i].col2);


         // chin
	  
		 // top left PP
      vtext[0].x = spp.x - halfScale  ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 1);
            
       // top right // RR
      
      vtext[1].x = spp.x + halfScale  ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 1);
     
       // bottom right // FF
      	 
      vtext[2].x = spp.x + halfScale  ;  vtext[2].y = spp.y; 	vtext[2].z = sff.z + (scale * 1.5 );
      
        // bottom left /DD
      vtext[3].x = spp.x - halfScale  ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1.5 );

      voxie_drawmeshtex(&vf,"img/chin.png",vtext,5,mesh,6,2,players[i].col2);


         // draw cheek left
	  
		 // top left PP
      vtext[0].x = players[i].x - (scale * 2) ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 0) ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 0.5);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 0) ;  vtext[2].y = spp.y; 	vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 2) ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekLeft.png",vtext,5,mesh,6,2,players[i].col2);


              // draw cheek right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 2) ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 0) ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 0.5);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 0) ;  vtext[2].y = spp.y; vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 2) ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekRight.png",vtext,5,mesh,6,2,players[i].col2);

break;
case 1:
	  spp.x = pp.x + (halfScale * players[i].size);
      spp.y = yoffSet + 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;




if (players[i].mouthState == 4) { // spit


 vtext[0].x = spp.x - (scale * 1.5) ; 	vtext[0].y = spp.y + 0.07; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = spp.x + (scale * 2); vtext[1].y = spp.y + 0.07 ; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x + (scale * 2);  vtext[2].y = spp.y + 0.07; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].x = spp.x - (scale * 1.5) ; vtext[3].y = spp.y + 0.07; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthSpit.png",vtext,5,mesh,6,2,players[i].col2);


	} else if (players[i].mouthState == 5) { // hurt



 vtext[0].x = spp.x - (scale * 1.5) ; 	vtext[0].y = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = spp.x + (scale * 2); vtext[1].y = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x + (scale * 2);  vtext[2].y = spp.y; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].x = spp.x - (scale * 1.5) ; vtext[3].y = spp.y; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthOuch.png",vtext,5,mesh,6,2,players[i].col2);



	} else {


	// draw mouth

		 // top left PP
      vtext[0].x = players[i].x; 	vtext[0].y = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].x = sff.x; vtext[1].y = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].x = sff.x;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z + players[i].mouthSize ; 
      
     				 
        // bottom left /DD

      vtext[3].x = players[i].x; vtext[3].y = spp.y; vtext[3].z = players[i].z + players[i].mouthSize; 
      
	  voxie_drawmeshtex(&vf,"img/mouth.png",vtext,5,mesh,6,2,players[i].col2);



	  for (j = 0; j < players[i].size; j++) {

	 // top left PP
      vtext[0].x = players[i].x + ( j * scale) ; vtext[0].y = spp.y + .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].x = players[i].x + ( j * scale) + scale ; vtext[1].y = spp.y + .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x + ( j * scale) + scale ;  vtext[2].y = spp.y + .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = players[i].x + ( j * scale) ; vtext[3].y = spp.y + .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeethTile.png",vtext,5,mesh,6,2,players[i].col2);

		 // top left PP
       vtext[0].y = spp.y + .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.y + .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
        vtext[2].y = spp.y + .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.y + .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeethTile.png",vtext,5,mesh,6,2,players[i].col2);


}
}
	  // draw top teeth
	  /*
		 // top left PP
      vtext[0].x = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].y = spp.y + .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].x = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].y = spp.y + .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].y = spp.y + .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].y = spp.y + .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeeth.png",vtext,5,mesh,6,2,players[i].col2);


        // draw bottom teeth
	  
		 // top left PP
      vtext[0].x = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].y = spp.y + .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].x = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].y = spp.y + .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].x = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].y = spp.y + .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].y = spp.y + .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeeth.png",vtext,5,mesh,6,2,players[i].col2);
*/

        // draw eyes left

if (players[i].mouthState == 5 || players[i].vomitTim > tim)  {


		 // top left PP
      vtext[0].x = players[i].x - (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);

} else {
	  
		 // top left PP
      vtext[0].x = players[i].x - (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 3) ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 1) ; vtext[1].y = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 1) ;  vtext[2].y = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 3) ; vtext[3].y = spp.y; vtext[3].z = players[i].z - (scale );

  if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 

}
          // nose
	  
		 // top left PP
      vtext[0].x = spp.x - scale  ; vtext[0].y = spp.y; vtext[0].z = players[i].z - (scale * 2);
            
       // top right // RR
      
      vtext[1].x = spp.x + scale  ; vtext[1].y = spp.y ; vtext[1].z = players[i].z - (scale * 2);
     
       // bottom right // FF
      	 
      vtext[2].x = spp.x + scale  ;  vtext[2].y = spp.y + .05; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].x = spp.x - scale  ; vtext[3].y = spp.y + .05; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/nose.png",vtext,5,mesh,6,2,players[i].col2);


         // chin
	  
		 // top left PP
      vtext[0].x = spp.x - halfScale  ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 1);
            
       // top right // RR
      
      vtext[1].x = spp.x + halfScale  ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 1);
     
       // bottom right // FF
      	 
      vtext[2].x = spp.x + halfScale  ;  vtext[2].y = spp.y; 	vtext[2].z = sff.z + (scale * 1.5 );
      
        // bottom left /DD
      vtext[3].x = spp.x - halfScale  ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1.5 );

      voxie_drawmeshtex(&vf,"img/chin.png",vtext,5,mesh,6,2,players[i].col2);


         // draw cheek left
	  
		 // top left PP
      vtext[0].x = players[i].x - (scale * 2) ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].x = players[i].x - (scale * 0) ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 0.5);
      
       // bottom right // FF
      	 
      vtext[2].x = players[i].x - (scale * 0) ;  vtext[2].y = spp.y; 	vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].x = players[i].x - (scale * 2) ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekLeft.png",vtext,5,mesh,6,2,players[i].col2);


              // draw cheek right
	  
		 // top left PP
      vtext[0].x = sff.x + (scale * 2) ; vtext[0].y = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].x = sff.x + (scale * 0) ; vtext[1].y = spp.y; vtext[1].z = sff.z + (scale * 0.5);
     
       // bottom right // FF
      	 
      vtext[2].x = sff.x + (scale * 0) ;  vtext[2].y = spp.y; vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].x = sff.x + (scale * 2) ; vtext[3].y = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekRight.png",vtext,5,mesh,6,2,players[i].col2);


break;
case 2:
  spp.x = pp.x + (halfScale * players[i].size);
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;


if (players[i].mouthState == 4) { // spit


 vtext[0].y = spp.x - (scale * 1.5) ;	vtext[0].x = spp.y - 0.07; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y =  spp.x + (scale * 2);  vtext[1].x = spp.y - 0.07 ; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y =  spp.x + (scale * 2);   vtext[2].x = spp.y - 0.07; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].y =  spp.x - (scale * 1.5) ;  vtext[3].x = spp.y - 0.07; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthSpit.png",vtext,5,mesh,6,2,players[i].col2);


	} else if (players[i].mouthState == 5) { // hurt



 vtext[0].y =  spp.x - (scale * 1.5) ;  	vtext[0].x = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y = spp.x + (scale * 2);  vtext[1].x = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y =  spp.x + (scale * 2);   vtext[2].x = spp.y; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].y =  spp.x - (scale * 1.5) ; vtext[3].x = spp.y; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthOuch.png",vtext,5,mesh,6,2,players[i].col2);



	} else {
	// draw mouth

		 // top left PP
      vtext[0].y = players[i].x; 	vtext[0].x = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y = sff.x; vtext[1].x = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y = sff.x;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z + players[i].mouthSize ; 
      
     				 
        // bottom left /DD

      vtext[3].y = players[i].x; vtext[3].x = spp.y; vtext[3].z = players[i].z + players[i].mouthSize; 
      
	  voxie_drawmeshtex(&vf,"img/mouth.png",vtext,5,mesh,6,2,players[i].col2);



	  for (j = 0; j < players[i].size; j++) {

	 // top left PP
      vtext[0].y = players[i].x + ( j * scale) ; vtext[0].x = spp.y - .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].y = players[i].x + ( j * scale) + scale ; vtext[1].x = spp.y - .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x + ( j * scale) + scale ;  vtext[2].x = spp.y - .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = players[i].x + ( j * scale) ; vtext[3].x = spp.y - .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeethTile.png",vtext,5,mesh,6,2,players[i].col2);

		 // top left PP
       vtext[0].x = spp.y - .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].x = spp.y - .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
        vtext[2].x = spp.y - .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = spp.y - .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeethTile.png",vtext,5,mesh,6,2,players[i].col2);


}
}

/*
	  // draw top teeth
	  
		 // top left PP
      vtext[0].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].x = spp.y - .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].x = spp.y - .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].x = spp.y - .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].x = spp.y - .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeeth.png",vtext,5,mesh,6,2,players[i].col2);


        // draw bottom teeth
	  
		 // top left PP
      vtext[0].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].x = spp.y - .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].x = spp.y - .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].x = spp.y - .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].x = spp.y - .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeeth.png",vtext,5,mesh,6,2,players[i].col2);

*/
        // draw eyes left
	  if (players[i].mouthState == 5 || players[i].vomitTim > tim) {
		 // top left PP
	  
      vtext[0].y = players[i].x - (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);


	  } else {
		 // top left PP
	  
      vtext[0].y = players[i].x - (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 
}

          // nose
	  
		 // top left PP
      vtext[0].y = spp.x - scale  ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 2);
            
       // top right // RR
      
      vtext[1].y = spp.x + scale  ; vtext[1].x = spp.y ; vtext[1].z = players[i].z - (scale * 2);
     
       // bottom right // FF
      	 
      vtext[2].y = spp.x + scale  ;  vtext[2].x = spp.y - .05; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = spp.x - scale  ; vtext[3].x = spp.y - .05; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/nose.png",vtext,5,mesh,6,2,players[i].col2);


         // chin
	  
		 // top left PP
      vtext[0].y = spp.x - halfScale  ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 1);
            
       // top right // RR
      
      vtext[1].y = spp.x + halfScale  ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 1);
     
       // bottom right // FF
      	 
      vtext[2].y = spp.x + halfScale  ;  vtext[2].x = spp.y; 	vtext[2].z = sff.z + (scale * 1.5 );
      
        // bottom left /DD
      vtext[3].y = spp.x - halfScale  ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1.5 );

      voxie_drawmeshtex(&vf,"img/chin.png",vtext,5,mesh,6,2,players[i].col2);


         // draw cheek left
	  
		 // top left PP
      vtext[0].y = players[i].x - (scale * 2) ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 0) ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 0.5);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 0) ;  vtext[2].x = spp.y; 	vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 2) ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekLeft.png",vtext,5,mesh,6,2,players[i].col2);


              // draw cheek right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 2) ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 0) ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 0.5);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 0) ;  vtext[2].x = spp.y; vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 2) ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1 );


      voxie_drawmeshtex(&vf,"img/cheekRight.png",vtext,5,mesh,6,2,players[i].col2);

break;
case 3:
  spp.x = pp.x + (halfScale * players[i].size);
     spp.y = yoffSet + 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;


if (players[i].mouthState == 4) { // spit


 vtext[0].y = spp.x - (scale * 1.5) ;	vtext[0].x = spp.y + 0.07; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y = spp.x + (scale * 2); vtext[1].x = spp.y + 0.07 ; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y = spp.x + (scale * 2);  vtext[2].x = spp.y + 0.07; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].y =spp.x - (scale * 1.5) ; vtext[3].x = spp.y + 0.07; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthSpit.png",vtext,5,mesh,6,2,players[i].col2);


	} else if (players[i].mouthState == 5) { // hurt



 vtext[0].y = spp.x - (scale * 1.5) ; 	vtext[0].x = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y = spp.x + (scale * 2);  vtext[1].x = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y =  spp.x + (scale * 2);   vtext[2].x = spp.y; 	vtext[2].z = players[i].z + scale * 2; 
      
     				 
        // bottom left /DD

      vtext[3].y = spp.x - (scale * 1.5) ; vtext[3].x = spp.y; vtext[3].z = players[i].z + scale * 2; 
      
	  voxie_drawmeshtex(&vf,"img/mouthOuch.png",vtext,5,mesh,6,2,players[i].col2);



	} else {
	// draw mouth

		 // top left PP
      vtext[0].y = players[i].x; 	vtext[0].x = spp.y; vtext[0].z = players[i].z;
            
       // top right // RR
      
      vtext[1].y = sff.x; vtext[1].x = spp.y; vtext[1].z = players[i].z;
      
       // bottom right // FF
      	 
      vtext[2].y = sff.x;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z + players[i].mouthSize ; 
      
     				 
        // bottom left /DD

      vtext[3].y = players[i].x; vtext[3].x = spp.y; vtext[3].z = players[i].z + players[i].mouthSize; 
      
	  voxie_drawmeshtex(&vf,"img/mouth.png",vtext,5,mesh,6,2,players[i].col2);


	  for (j = 0; j < players[i].size; j++) {

	 // top left PP
      vtext[0].y = players[i].x + ( j * scale) ; vtext[0].x = spp.y + .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].y = players[i].x + ( j * scale) + scale ; vtext[1].x = spp.y + .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x + ( j * scale) + scale ;  vtext[2].x = spp.y + .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = players[i].x + ( j * scale) ; vtext[3].x = spp.y + .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeethTile.png",vtext,5,mesh,6,2,players[i].col2);

		 // top left PP
       vtext[0].x = spp.y + .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].x = spp.y + .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
        vtext[2].x = spp.y + .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].x = spp.y + .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeethTile.png",vtext,5,mesh,6,2,players[i].col2);


}
}
	  // draw top teeth
	  /*
		 // top left PP
      vtext[0].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].x = spp.y + .030f; vtext[0].z = players[i].z - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].x = spp.y + .030f; vtext[1].z = players[i].z - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].x = spp.y + .030f; 	vtext[2].z = players[i].z + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].x = spp.y + .030f; vtext[3].z = players[i].z + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/topTeeth.png",vtext,5,mesh,6,2,players[i].col2);


        // draw bottom teeth
	  
		 // top left PP
      vtext[0].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[0].x = spp.y + .025f; vtext[0].z = players[i].z + players[i].mouthSize - eighthScale;
            
       // top right // RR
      
      vtext[1].y = spp.x  + PLAYER_TEETH_SIZE_X ; vtext[1].x = spp.y + .025f; vtext[1].z = players[i].z + players[i].mouthSize - eighthScale;
      
       // bottom right // FF
      	 
      vtext[2].y = spp.x  + PLAYER_TEETH_SIZE_X ;  vtext[2].x = spp.y + .025f; 	vtext[2].z = players[i].z + players[i].mouthSize + PLAYER_TEETH_SIZE_Z ; 
      
        // bottom left /DD
      vtext[3].y = spp.x - PLAYER_TEETH_SIZE_X ; vtext[3].x = spp.y + .025f; vtext[3].z = players[i].z +players[i].mouthSize + PLAYER_TEETH_SIZE_Z; 

      voxie_drawmeshtex(&vf,"img/botTeeth.png",vtext,5,mesh,6,2,players[i].col2);
*/
if (players[i].mouthState == 5 || players[i].vomitTim > tim) {
              // draw eyes left
	  
		 // top left PP
      vtext[0].y = players[i].x - (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/eyeOuch.png",vtext,5,mesh,6,2,players[i].col2);

} else {
        // draw eyes left
	  
		 // top left PP
      vtext[0].y = players[i].x - (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 
      

        // draw eyes right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 3) ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 3);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 1) ; vtext[1].x = spp.y; vtext[1].z = players[i].z - (scale * 3);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 1) ;  vtext[2].x = spp.y; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 3) ; vtext[3].x = spp.y; vtext[3].z = players[i].z - (scale );

 if (players[i].score > SPIT_MIN_THRESHOLD || players[i].frenzyTim > tim) {



 		if (colourScroll < 2) {

 		voxie_drawmeshtex(&vf,"img/eyeCrazy2.png",vtext,5,mesh,6,2,players[i].col2);

 	  } else {

	voxie_drawmeshtex(&vf,"img/eyeCrazy1.png",vtext,5,mesh,6,2,players[i].col2);

 	  }


 	


 } else {

      voxie_drawmeshtex(&vf,"img/eyeLeft.png",vtext,5,mesh,6,2,players[i].col2);
    
   } 

}
          // nose
	  
		 // top left PP
      vtext[0].y = spp.x - scale  ; vtext[0].x = spp.y; vtext[0].z = players[i].z - (scale * 2);
            
       // top right // RR
      
      vtext[1].y = spp.x + scale  ; vtext[1].x = spp.y ; vtext[1].z = players[i].z - (scale * 2);
     
       // bottom right // FF
      	 
      vtext[2].y = spp.x + scale  ;  vtext[2].x = spp.y + .05; 	vtext[2].z = players[i].z - (scale  );
      
        // bottom left /DD
      vtext[3].y = spp.x - scale  ; vtext[3].x = spp.y + .05; vtext[3].z = players[i].z - (scale );

      voxie_drawmeshtex(&vf,"img/nose.png",vtext,5,mesh,6,2,players[i].col2);


         // chin
	  
		 // top left PP
      vtext[0].y = spp.x - halfScale  ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 1);
            
       // top right // RR
      
      vtext[1].y = spp.x + halfScale  ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 1);
     
       // bottom right // FF
      	 
      vtext[2].y = spp.x + halfScale  ;  vtext[2].x = spp.y; 	vtext[2].z = sff.z + (scale * 1.5 );
      
        // bottom left /DD
      vtext[3].y = spp.x - halfScale  ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1.5 );

      voxie_drawmeshtex(&vf,"img/chin.png",vtext,5,mesh,6,2,players[i].col2);


         // draw cheek left
	  
		 // top left PP
      vtext[0].y = players[i].x - (scale * 2) ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].y = players[i].x - (scale * 0) ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 0.5);
      
       // bottom right // FF
      	 
      vtext[2].y = players[i].x - (scale * 0) ;  vtext[2].x = spp.y; 	vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].y = players[i].x - (scale * 2) ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1 );

      voxie_drawmeshtex(&vf,"img/cheekLeft.png",vtext,5,mesh,6,2,players[i].col2);


              // draw cheek right
	  
		 // top left PP
      vtext[0].y = sff.x + (scale * 2) ; vtext[0].x = spp.y; vtext[0].z = sff.z + (scale * 0.5);
            
       // top right // RR
      
      vtext[1].y = sff.x + (scale * 0) ; vtext[1].x = spp.y; vtext[1].z = sff.z + (scale * 0.5);
     
       // bottom right // FF
      	 
      vtext[2].y = sff.x + (scale * 0) ;  vtext[2].x = spp.y; vtext[2].z = sff.z + (scale * 1 );
      
        // bottom left /DD
      vtext[3].y = sff.x + (scale * 2) ; vtext[3].x = spp.y; vtext[3].z = sff.z + (scale * 1 );


      voxie_drawmeshtex(&vf,"img/cheekRight.png",vtext,5,mesh,6,2,players[i].col2);

break;
}



// draw full look
      /*

	switch(players[i].orient) {
		case 0: // player 1

      vtext[0].x = -0.9; vtext[0].y = -1; vtext[0].z = -0.44f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
      vtext[1].x = -0.9; vtext[1].y = 1; vtext[1].z = -0.44f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
      vtext[2].x = -1; vtext[2].y = 1; vtext[2].z = 0.44f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].x = -1; vtext[3].y = -1; vtext[3].z = 0.445f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

voxie_drawmeshtex(&vf,"img/face1.jpg",vtext,5,mesh,6,2,players[i].col2);

	
		break;	
		case 1: // player 2


     vtext[0].x = 0.9;  vtext[0].y = -1; vtext[0].z = -0.44f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
     vtext[1].x = 0.9; 	vtext[1].y = 1; vtext[1].z = -0.44f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
     vtext[2].x = 1; 	vtext[2].y = 1; vtext[2].z = 0.44f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
     vtext[3].x = 1; 	vtext[3].y = -1; vtext[3].z = 0.445f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

voxie_drawmeshtex(&vf,"img/face2.jpg",vtext,5,mesh,6,2,players[i].col2);

		break;

		case 2: // player 3
     vtext[0].x = 1;  vtext[0].y = -0.9; vtext[0].z = -0.44f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
      vtext[1].x = -1; vtext[1].y = -0.9; vtext[1].z = -0.44f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
      vtext[2].x = -1; vtext[2].y = -1; vtext[2].z = 0.44f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].x = 1; vtext[3].y = -1; vtext[3].z = 0.445f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

voxie_drawmeshtex(&vf,"img/face3.jpg",vtext,5,mesh,6,2,players[i].col2);

break;

		case 3: // player 4 
	     vtext[0].x = -1;  vtext[0].y = 0.9; vtext[0].z = -0.44f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
      vtext[1].x = 1; vtext[1].y = 0.9; vtext[1].z = -0.44f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
      vtext[2].x = 1; vtext[2].y = 1; vtext[2].z = 0.44f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].x = -1; vtext[3].y = 1; vtext[3].z = 0.445f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

voxie_drawmeshtex(&vf,"img/face4.jpg",vtext,5,mesh,6,2,players[i].col2);
		break;	
	}
	*/
	







		
		
	}

}

// draw balls



	for (i = 0; i < ballNo; i++) {
	f = (balls[i].r + ((-balls[i].z + 1) * .20 ) * .10 )* scaleOffSet;
   
  if (balls[i].destroy == false) {

		
				voxie_drawsph(&vf,balls[i].x,balls[i].y,balls[i].z,f,1,balls[i].col);


		

	}
	else {
		voxie_drawsph(&vf,balls[i].x,balls[i].y,balls[i].z,f,0,balls[i].col);
	}
	}

// draw floor
    float Ytop = -.85;
      float Ybot = -.25;
	  float XLeft = -.8;
      float XRight = .8;

if (titleScreen == true) {

						rr.x = 0.1f; dd.x = 0.00f; pp.x = -0.5f;
						rr.y = 0.0f; dd.y = 0.345f; pp.y = 0.16f;
						rr.z = 0.00f; dd.z = 0.0f; pp.z = -0.20f;
					
						pp.y = cos(tim*4.0)*-0.15*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;
							pp.z = sin(tim*4.0)*-0.15*1; rr.z = 0.00; dd.z = sin(tim*4.0)*0.30*1;
		
	switch(winPlayer) {

		case 0:




	  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	  spp.x = 0;
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;

  		 Ytop = -.8; Ybot = .8; XLeft = -.8; XRight = .8;
  

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/yellowWin.png",vtext,5,mesh,6,2,0x404040);



		break;
		case 1:
  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	  spp.x = 0;
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;

  		 Ytop = -.8; Ybot = .8; XLeft = -.8; XRight = .8;
  

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/blueWin.png",vtext,5,mesh,6,2,0x404040);
		break;

		case 2:
  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	  spp.x = 0;
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;

  		 Ytop = -.8; Ybot = .8; XLeft = -.8; XRight = .8;
  

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	   voxie_drawmeshtex(&vf,"img/redWin.png",vtext,5,mesh,6,2,0x404040);
		break;

		case 3:
  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	  spp.x = 0;
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;

  		 Ytop = -.8; Ybot = .8; XLeft = -.8; XRight = .8;
  

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
       voxie_drawmeshtex(&vf,"img/greenWin.png",vtext,5,mesh,6,2,0x404040);
	
		break;

case 5:

      dd.z = 0.0f;
      ff.x = 0.0;
      ff.y = 0.0f;
      rr.y = 0.0f;
      rr.z = 0.0f;
      dd.x = 0.0f;
      dd.y = scale; // PLAYER size

      ff.z = halfScale; // PLAYER height

	  vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0; 
	  vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	  vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	  spp.x = 0;
      spp.y = -yoffSet - 0.05;
      sff.x = pp.x + (scale * players[i].size);
      sff.z = pp.z + players[i].mouthSize;


      Ytop = -.85;
       Ybot = -.25;
	   XLeft = -.8;
       XRight = .8;

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/titBall.png",vtext,5,mesh,6,2,0x404040);


       Ytop = -0.20;
       Ybot = 0.15;
	   XLeft = -.4;
      XRight = .4;

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/titFor.png",vtext,5,mesh,6,2,0x404040);

  Ytop = 0.2;
       Ybot = 0.7;
	   XLeft = -.8;
      XRight = .8;

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/titAgain.png",vtext,5,mesh,6,2,0x404040);


  Ytop = 0.75;
       Ybot = 0.95;
	   XLeft = -.8;
      XRight = .8;

      	pp.y = cos(tim*4.0)*-0.10*1; rr.y = 0.00; dd.y = cos(tim*4.0)*0.30*1;



	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		
	 vtext[0].x = XLeft ; 	vtext[0].y = Ytop; vtext[0].z = pp.z;
            
       // top right // RR
      
      vtext[1].x = XRight; vtext[1].y = Ytop ; vtext[1].z = pp.z;
      
       // bottom right // FF
      	 
      vtext[2].x = XRight;  vtext[2].y = Ybot; 	vtext[2].z = pp.y; 
      
     				 
        // bottom left /DD

      vtext[3].x = XLeft ; vtext[3].y = Ybot; vtext[3].z = pp.y; 
      
	  voxie_drawmeshtex(&vf,"img/titCredit.png",vtext,5,mesh,6,2,0x404040);

		break;
	}					


					



}

	/***************************
	** MUSIC PLAYER
	**/ 

				switch(mxFade) {
			case 0: 
			break;
			case 1: // fade out
			mxVol -= 2;
			
			if (mxVol <= 0) {
				mxVol = -1;
				mxFade = -1;
			}
			voxie_playsound_update(mx, 0, mxVol, mxVol, 1.0);
			break;
			case 2: // fade in
			mxVol += 2;
			voxie_playsound_update(mx, 0, mxVol, mxVol, 1.0);
			if (mxVol >= mxVolDefLevel) {
				mxVol = mxVolDefLevel;
				mxFade = -2;
				
			}
			break;
			case 3: // fade out 2 kill
			mxVol -= 2;
			
			if (mxVol <= 0) {
				mxVol = -1;
				mxFade = -3;
				//voxie_playsound_update(mx, 0, 0, 0, 1.0);
				mxLoopTim = 7.0 + tim; // start music
				curSong = rand() % 2;
				mxLoop = 1;
			}
			voxie_playsound_update(mx, 0, mxVol, mxVol, 1.0);

			break;

		}

			if (mxLoop == 1 && mxLoopTim < tim) {

				if (curSong == 0) {
					curSong = 0; // play song2
					mxFade = 2;
					
					mx = voxie_playsound("mx/mxtitle.wav", 0, mxVol, mxVol,1.0); 
					mxLoopTim = mxTitleSampLength + tim; // start music
					

				} else {
					curSong = 1; // play song2
					mxFade = 2;
				
					mx = voxie_playsound("mx/mx2.wav", 0, mxVol, mxVol,1.0); 
					mxLoopTim = mxGameLength + tim; // start music
			

				}
			


		}




	/***************************
    **	 DEBUG / RENDER PHASE **
	****************************/
		// final update loop for frame
	if (debug == 1) {
		avgdtim += (dtim-avgdtim)*.1;
		voxie_debug_print6x8_(30,68,0xffc080,-1,"FPS %5.1f tim %5.1f ",1.0/avgdtim, tim);	
		

		voxie_debug_print6x8_(30,128,0xffc080,-1,"SCALE: %1.4f H %1.4f Q %1.3f E %1.3f S %1.3f ZFLOOR: %1.4f f: %1.5f", 
				scale, halfScale, qtrScale, eighthScale, shotScale, zFloor,f);	

		voxie_debug_print6x8_(30,200,0xffc080,-1,"BALL: speed %1.4f  xv %1.4f yv %1.4f  zv %1.4f ballNo = %d", 
				balls[0].speed, balls[0].xv, balls[0].yv, balls[0].zv, ballNo);	
		
			voxie_debug_print6x8_(30,230,0xffc080,-1,"PLAYER: x %1.1f y %1.f z %1.1f  z  xv %1.4f  zv %1.4f mouthSize: %1.2f mouthState: %d score %d frenzyTim %1.5f", 
				players[2].x, players[2].y, players[0].z, players[2].xv, players[2].zv, players[2].mouthSize, players[2].mouthState, players[2].score, players[0].frenzyTim);	
	}



	voxie_frame_end(); voxie_getvw(&vw);

	
}
 

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif

