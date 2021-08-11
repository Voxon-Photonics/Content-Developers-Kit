// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1

#Visual C makefile:
bricks.exe: bricks.c voxiebox.h; cl /TP bricks.c /Ox /MT /link user32.lib
	del bricks.obj

!else

#GNU C makefile:
bricks.exe: bricks.c; gcc bricks.c -o bricks.exe -pipe -O3 -s -m64

!endif
!if 0
/* Bricks program for drawing bricks unto the screen and only draws the outside


*/

#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define COLOUR_GREY 			0x303030
#define COLOUR_BLUE  			0x0000FF
#define COLOUR_GREEN 		 	0x00FF00
#define COLOUR_ORANGE		 	0x996000
#define COLOUR_YELLOW		 	0xFFFF00
#define COLOUR_MAGENTA 		 	0xFF0060
#define COLOUR_WHITE 			0x404040
#define COLOUR_RED 				0xFF0000
#define COLOUR_CYAN			 	0x00FFFF
#define COLOUR_RED_DIM 			0x800000
#define COLOUR_GREEN_DIM 		0x008000
#define COLOUR_BLUE_DIM 		0x000080
#define COLOUR_YELLOW_DIM 		0x808000
#define COLOUR_MAGENTA_DIM 	 	0x800080
#define COLOUR_CYAN_DIM 		0x008080
#define COLOUR_YELLOW_ULTRA 	0x606000
#define COLOUR_YELLOW_ULTRA_DIM 0x404000
#define COLOUR_RED_ULTRA 		0x600000
#define COLOUR_RED_ULTRA_DIM 	0x400000
#define COLOUR_BLUE_ULTRA 		0x000060
#define COLOUR_BLUE_ULTRA_DIM 	0x000040
#define COLOUR_GREEN_ULTRA 		0x006000
#define COLOUR_GREEN_ULTRA_DIM 	0x004000
#define COLOUR_MAGENTA_ULTRA 	0x600060
#define COLOUR_MAGENTA_ULTRA_DIM 0x400040
#define COLOUR_CYAN_ULTRA 		0x006060
#define COLOUR_CYAN_ULTRA_DIM 	0x004040


#define BRICK_MAX 1850
typedef struct { point3d pos, vel, movTo; float sHeight, scale; int fill, id, type, col, col2, col3, zHeight; bool unbreakable, destroy, flying, drawShadow, drawAll, drawTop, drawBottom, drawLeft, drawRight, drawDown, drawUp, drawWire; } brick_t;

static brick_t bricks[BRICK_MAX];
static int brickNo = 0;
static int brickID = 0; 


//dimentions of the grid to import
static int mapX = 15, mapY = 15, mapZ = 9;
static int map[50][50][25]; 


// scaling variables
static float minX, maxX, minY, maxY, midX, midY;
static float doubleScale, scale, halfScale, qtrScale, eighthScale, sixteethScale, zFloorTrue, zFloor;

// offsets for drawing grid
static float X_OFFSET = -1.02;
static float Y_OFFSET = -1.03;
static float Z_OFFSET = -1.01;



static voxie_wind_t vw;
static voxie_frame_t vf;
static int gcnti[2], gbstat = 0;



/* LEVELS */

// level 1 single units
int lv1MapA[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 };

static int lv1MapB[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,55, 55,55,55,55,55, 55,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, }, 
 
};

static int lv1MapC[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,22, 22,22,22,22,22, 22,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
};

static int lv1MapD[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,33,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
};

static int lv1MapE[15][15] =  {
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,44,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,44,0, 0,0,66,0,0, 0,0,0,0,0, },
 { 0,0,0,44,0, 0,0,66,0,0, 0,0,0,0,0, },
 { 0,0,0,44,0, 0,0,66,0,0, 0,0,0,0,0, },
 { 0,0,0,44,0, 0,0,66,0,0, 0,0,0,0,0, },
 { 0,0,0,44,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,44,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
};


static int lv1MapF[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 77,77,77,77,77, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,1,0, 0,0,0,0,0, 0,1,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
};

static int lv1MapG[15][15] =  {

 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 { 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, },
 
 
};

// Space Invaders UFO ship
static int lv2MapA[3][16] =  {

 { 0,0,0,4,0, 0,0, 0,0, 0,0, 0,4,0,0,0, },
 { 0,0,0,4,0, 0,0, 0,0, 0,0, 0,4,0,0,0, },
 { 0,0,0,4,0, 0,0, 0,0, 0,0, 0,4,0,0,0, },

};

static int lv2MapB[3][16] =  {

 { 0,0,4,4,4, 0,0, 4,4, 0,0, 4,4,4,0,0, },
 { 0,0,4,4,4, 0,0, 4,4, 0,0, 4,4,4,0,0, },
 { 0,0,4,4,4, 0,0, 4,4, 0,0, 4,4,4,0,0, },

 
 
};

static int lv2MapC[3][16] =  {

 { 4,4,4,4,4, 4,4, 4,4, 4,4, 4,4,4,4,4, },
 { 4,4,4,4,4, 4,4, 4,4, 4,4, 4,4,4,4,4, },
 { 4,4,4,4,4, 4,4, 4,4, 4,4, 4,4,4,4,4, },

 
};

static int lv2MapD[3][16] =  {

 { 0,4,4,0,4, 4,0, 4,4, 0,4, 4,0,4,4,0, },
 { 0,4,4,0,4, 4,0, 4,4, 0,4, 4,0,4,4,0, },
 { 0,4,4,0,4, 4,0, 4,4, 0,4, 4,0,4,4,0, },

 
 
};

static int lv2MapE[3][16] =  {
 { 0,0,4,4,4, 4,4, 4,4, 4,4, 4,4,4,0,0, },
 { 0,0,4,4,4, 4,4, 4,4, 4,4, 4,4,4,0,0, },
 { 0,0,4,4,4, 4,4, 4,4, 4,4, 4,4,4,0,0, },

};


static int lv2MapF[3][16] =  {

 { 0,0,0,4,4, 4,4, 4,4, 4,4, 4,4,0,0,0, },
 { 0,0,0,4,4, 4,4, 4,4, 4,4, 4,4,0,0,0, },
 { 0,0,0,4,4, 4,4, 4,4, 4,4, 4,4,0,0,0, },

 
 
};

static int lv2MapG[3][16] =  {

 { 0,0,0,0,0, 4,4, 4,4, 4,4, 0,0,0,0,0, },
 { 0,0,0,0,0, 4,4, 4,4, 4,4, 0,0,0,0,0, },
 { 0,0,0,0,0, 4,4, 4,4, 4,4, 0,0,0,0,0, },

 
 
};


// map 4 octo invader

static int lv4MapA[15][15] =  {

	{ 5,5,0, 0,0, 0, 0, 0,0, 0,5,5, },
	{ 5,5,0, 0,0, 0, 0, 0,0, 0,5,5, },
 
};

static int lv4MapB[15][15] =  {

	{ 0,0,5, 5,0, 5, 5, 0,5, 5,0,0, },
	{ 0,0,5, 5,0, 5, 5, 0,5, 5,0,0, },
 
};

static int lv4MapC[15][15] =  {

	{ 0,0,0, 5,5, 0, 0, 5,5, 0,0,0, },
	{ 0,0,0, 5,5, 0, 0, 5,5, 0,0,0, },
 
};

static int lv4MapD[15][15] =  {

	{ 5,5,5, 5,5, 5, 5, 5,5, 5,5,5, },
	{ 5,5,5, 5,5, 5, 5, 5,5, 5,5,5, },

};

static int lv4MapE[15][15] =  {

	{ 5,5,5, 0,0, 5, 5, 0,0, 5,5,5, },
	{ 5,5,5, 0,0, 5, 5, 0,0, 5,5,5, },
 
};


static int lv4MapF[15][15] =  {

	{ 5,5,5, 5,5, 5, 5, 5,5, 5,5,5, },
	{ 5,5,5, 5,5, 5, 5, 5,5, 5,5,5, },

};

static int lv4MapG[15][15] =  {

	{ 0,5,5, 5,5, 5, 5, 5,5, 5,5,0, },
	{ 0,5,5, 5,5, 5, 5, 5,5, 5,5,0, },
 
};

static int lv4MapH[15][15] =  {

	{ 0,0,0, 0,5, 5, 5, 5,0, 0,0,0, },
	{ 0,0,0, 0,5, 5, 5, 5,0, 0,0,0, },
 
};


// map 3 crab invader
static int lv3MapA[2][15] =  {
	{ 0,0,0, 3,3, 0, 0, 3,3, 0,0,0, },
	{ 0,0,0, 3,3, 0, 0, 3,3, 0,0,0, },
 };

static int lv3MapB[2][15] =  {
	{ 3,0,3, 0,0, 0, 0, 0,0, 3,0,3, },
	{ 3,0,3, 0,0, 0, 0, 0,0, 3,0,3, },
};

static int lv3MapC[2][15] =  {
	{ 3,0,3, 3,3, 3, 3, 3,3, 3,0,3, },
	{ 3,0,3, 3,3, 3, 3, 3,3, 3,0,3, },
};

static int lv3MapD[2][15] =  {
	{ 3,3,3, 3,3, 3, 3, 3,3, 3,3,3, },
	{ 3,3,3, 3,3, 3, 3, 3,3, 3,3,3, },
};

static int lv3MapE[2][15] =  {
	{ 0,3,3, 0,3, 3, 3, 3,0, 3,3,0, },
	{ 0,3,3, 0,3, 3, 3, 3,0, 3,3,0, },
 };

static int lv3MapF[2][15] =  {
	{ 0,0,3, 3,3, 3, 3, 3,3, 3,0,0, },
	{ 0,0,3, 3,3, 3, 3, 3,3, 3,0,0, },
};

static int lv3MapG[2][15] =  {
	{ 0,0,0, 3,0, 0, 0, 0,3, 0,0,0, },
	{ 0,0,0, 3,0, 0, 0, 0,3, 0,0,0, },
 };

static int lv3MapH[2][15] =  {
	{ 0,0,3, 0,0, 0, 0, 0,0, 3,0,0, },
	{ 0,0,3, 0,0, 0, 0, 0,0, 3,0,0, },
 };


// map 5 squid invader

static int lv5MapA[15][15] =  {

	{ 0,1,0, 0,0, 0, 1, 0,0, 0,0,0, },
	{ 0,1,0, 0,0, 0, 1, 0,0, 0,0,0, },
 
};

static int lv5MapB[15][15] =  {

	{ 1,0,0, 0,0, 0, 0, 1,0, 0,0,0, },
	{ 1,0,0, 0,0, 0, 0, 1,0, 0,0,0, },
 
};

static int lv5MapC[15][15] =  {

	{ 0,1,0, 1,1, 0, 1, 0,0, 0,0,0, },
	{ 0,1,0, 1,1, 0, 1, 0,0, 0,0,0, },
 
};

static int lv5MapD[15][15] =  {

	{ 1,1,1, 1,1, 1, 1, 1,0, 0,0,0, },
	{ 1,1,1, 1,1, 1, 1, 1,0, 0,0,0, },

};

static int lv5MapE[15][15] =  {

	{ 1,1,0, 1,1, 0, 1, 1,0, 0,0,0, },
	{ 1,1,0, 1,1, 0, 1, 1,0, 0,0,0, },
 
};


static int lv5MapF[15][15] =  {

	{ 0,1,1, 1,1, 1, 1, 0,0, 0,0,0, },
	{ 0,1,1, 1,1, 1, 1, 0,0, 0,0,0, },

};

static int lv5MapG[15][15] =  {

	{ 0,0,1, 1,1, 1, 0, 0,0, 0,0,0, },
	{ 0,0,1, 1,1, 1, 0, 0,0, 0,0,0, },
 
};

static int lv5MapH[15][15] =  {

	{ 0,0,0, 1,1, 0, 0, 0,0, 0,0,0, },
	{ 0,0,0, 1,1, 0, 0, 0,0, 0,0,0, },
 
};



// map 6 bunker

static int lv6MapA[4][22] =  {

	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
 
};

static int lv6MapB[4][22] =  {

	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  0,0,0,0,0,0,0,0,0,0, 4,4,4,4,4,4, },


};

static int lv6MapC[4][22] =  {

	{ 4,4,4,4,4,4,  4,0,0,0,0,0,0,0,0,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,0,0,0,0,0,0,0,0,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,0,0,0,0,0,0,0,0,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,0,0,0,0,0,0,0,0,4, 4,4,4,4,4,4, },
 
};

static int lv6MapD[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,0,0,0,0,0,0,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,0,0,0,0,0,0,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,0,0,0,0,0,0,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,0,0,0,0,0,0,4,4, 4,4,4,4,4,4, }, 
};

static int lv6MapE[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },


};

static int lv6MapF[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, }, 
};

static int lv6MapG[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, }, 
};

static int lv6MapH[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
 	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
};

static int lv6MapI[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },

};

static int lv6MapJ[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, }, 
};

static int lv6MapK[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
 	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
};

static int lv6MapL[4][22] =  {

	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, },
	{ 4,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4, }, 
};

static int lv6MapM[4][22] =  {

	{ 0,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,0, },
	{ 0,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,0, },
	{ 0,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,0, },
	{ 0,4,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,0, }, 
};

static int lv6MapN[4][22] =  {

	{ 0,0,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,0,0, },
	{ 0,0,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,0,0, },
	{ 0,0,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,0,0, },
	{ 0,0,4,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,4,0,0, }, 
};

static int lv6MapO[4][22] =  {

	{ 0,0,0,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,0,0,0, },
	{ 0,0,0,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,0,0,0, },
	{ 0,0,0,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,0,0,0, },
	{ 0,0,0,4,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,4,0,0,0, }, 
};

static int lv6MapP[4][22] =  {

	{ 0,0,0,0,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,0,0,0,0, },
	{ 0,0,0,0,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,0,0,0,0, },
	{ 0,0,0,0,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,0,0,0,0, },
	{ 0,0,0,0,4,4,  4,4,4,4,4,4,4,4,4,4, 4,4,0,0,0,0, }, 
};

// bunker optimised


// map 7 bunker optimised

static int lv7MapA[2][11] =  {

	{ 4,4,4, 0,0,0,0,0, 4,4,4, },
	{ 4,4,4, 0,0,0,0,0, 4,4,4, },
 
};

static int lv7MapB[2][11] =  {

	{ 4,4,4, 4,0,0,0,4, 4,4,4, },
	{ 4,4,4, 4,0,0,0,4, 4,4,4, },


};

static int lv7MapC[2][11] =  {

	{ 4,4,4, 4,4,4,4,4, 4,4,4, },
	{ 4,4,4, 4,4,4,4,4, 4,4,4, },


};

static int lv7MapD[2][11] =  {

	{ 4,4,4, 4,4,4,4,4, 4,4,4, },
	{ 4,4,4, 4,4,4,4,4, 4,4,4, },


};

static int lv7MapE[2][11] =  {

	{ 4,4,4, 4,4,4,4,4, 4,4,4, },
	{ 4,4,4, 4,4,4,4,4, 4,4,4, },


};

static int lv7MapF[2][11] =  {

	{ 4,4,4, 4,4,4,4,4, 4,4,4, },
	{ 4,4,4, 4,4,4,4,4, 4,4,4, },


};

static int lv7MapG[2][11] =  {

	{ 0,0,4, 4,4,4,4,4, 4,0,0, },
	{ 0,0,4, 4,4,4,4,4, 4,0,0, },


};

static int lv7MapH[2][11] =  {

	{ 0,0,0, 0,4,4,4,0, 0,0,0, },
	{ 0,0,0, 0,4,4,4,0, 0,0,0, },


};



// updates the scale to new settings
static void scaleUpdate() {

scale = 2 / float(mapY) * 1;
doubleScale = scale * 2;
halfScale = scale /2;
qtrScale = scale /4;
eighthScale = scale / 8;
sixteethScale = scale / 16;
maxX = X_OFFSET + ( mapX   * scale   ) ;
minX = Y_OFFSET ;
maxY = Y_OFFSET + ( mapY   * scale   ) 	;
minY = Y_OFFSET ;
midX = X_OFFSET + ((mapX / 2) * scale);
midY = Y_OFFSET + ((mapY / 2) * scale);
zFloor = vw.aspz + 1;
zFloorTrue = zFloor + Z_OFFSET;



}

static float calZHeight(int z, float distance) {

return zFloor - (distance * z) + -1.01;

}

//Rotate vectors a & b around their common plane, by ang
static void rotvex(float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang);
	s = sin(ang);
	f = a->x;
	a->x = f * c + b->x * s;
	b->x = b->x * c - f * s;
	f = a->y;
	a->y = f * c + b->y * s;
	b->y = b->y * c - f * s;
	f = a->z;
	a->z = f * c + b->z * s;
	b->z = b->z * c - f * s;
}

/****************
*NON MAIN FUNCTIONS *
*               *
*****************/

// a brick move to function returns turn when in position
bool bricksMovTo (int brickId, point3d moveToPos, float speed, double dtim) {

	point3d pp, vel;
	float f;

	if ( bricks[brickId].pos.x < moveToPos.x + 0.1 + (speed * dtim) && bricks[brickId].pos.x > moveToPos.x - 0.1 - (speed * dtim)&&
		   bricks[brickId].pos.y < moveToPos.y + 0.1 + (speed * dtim) && bricks[brickId].pos.y > moveToPos.y - 0.1 - (speed * dtim)&&
		   bricks[brickId].pos.z < moveToPos.z + 0.1 + (speed * dtim) && bricks[brickId].pos.z > moveToPos.z - 0.1 - (speed * dtim)
	 ) { 
		  bricks[brickId].pos.x = moveToPos.x;
	  	bricks[brickId].pos.y = moveToPos.y;
		  bricks[brickId].pos.z = moveToPos.z;
		 return true;

	 }
	
	else {
		pp.x = moveToPos.x - bricks[brickId].pos.x;
		pp.y = moveToPos.y - bricks[brickId].pos.y;
		pp.z = moveToPos.z - bricks[brickId].pos.z;

		f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z); // normilize vectors
		
		vel.x = 	(speed ) * (pp.x / f) * dtim;
		vel.y = 	(speed ) * (pp.y / f) * dtim;
		vel.z = 	(speed ) * (pp.z / f) * dtim;

		bricks[brickId].pos.x += vel.x;
		bricks[brickId].pos.y += vel.y;
		bricks[brickId].pos.z += vel.z;

		return false;
	}

}


// processes how the bricks will look (only do once bricks change)
static void bricksProcess() {

float bScale, bHalf, bQtr, bEigth;
int j,k;

bScale = bricks[0].scale;
bHalf = bScale * .5;
bQtr = bHalf * .5;
bEigth = bQtr * .5;

for (j = brickNo - 1; j >= 0; j--) {

	bricks[j].drawTop = true;
	bricks[j].drawBottom = true;
	bricks[j].drawLeft = true;
	bricks[j].drawRight = true;
	bricks[j].drawUp = true;
	bricks[j].drawDown = true;
	bricks[j].drawShadow = false;

	if (bricks[j].destroy == true) {
		brickNo--; bricks[j] = bricks[brickNo]; continue;
	}



		// check if there is a brick below
		for (k = brickNo - 1; k >= 0; k--) {
			if (bricks[j].id == bricks[k].id) continue; // cull bricks that are the same brick

			// work out wether to draw the top
			if (bricks[j].pos.x == bricks[k].pos.x 	 &&
				bricks[j].pos.y == bricks[k].pos.y 	 &&
				bricks[j].zHeight + 1 == bricks[k].zHeight
				) {
	
			 bricks[j].drawTop = false; 			 
			}

						// work out wether to draw the bottom
			if (bricks[j].pos.x == bricks[k].pos.x 	 &&
				bricks[j].pos.y == bricks[k].pos.y 	 &&
				bricks[j].zHeight - 1 == bricks[k].zHeight
				) {
	
			 bricks[j].drawBottom = false; 			 
			}

			// work out wether to draw the right
			if (bricks[j].pos.x + bHalf < bricks[k].pos.x 	     			&&
			bricks[j].pos.x + bScale + bHalf > bricks[k].pos.x    &&
			bricks[j].pos.y == bricks[k].pos.y 									&&
			bricks[j].zHeight == bricks[k].zHeight ) {
			
			 bricks[j].drawRight = false; 
			} 

			// work out wether to draw the left
			if (bricks[j].pos.x - bHalf  > bricks[k].pos.x 	  &&
			bricks[j].pos.x - bScale - bHalf < bricks[k].pos.x  &&
			bricks[j].pos.y == bricks[k].pos.y 					  &&
			bricks[j].zHeight == bricks[k].zHeight
			) {
			
				bricks[j].drawLeft = false;
			}

			// work out wether to draw the up
			if (bricks[j].pos.y - bHalf > bricks[k].pos.y 	  		&&
				bricks[j].pos.y - bScale - bHalf < bricks[k].pos.y  	&&
				bricks[j].pos.x == bricks[k].pos.x 					 	&&
				bricks[j].zHeight == bricks[k].zHeight
			) {
				
				 bricks[j].drawUp = false; 
			} 

			// work out wether to draw the down
			if (bricks[j].pos.y + bHalf < bricks[k].pos.y 	     			 &&
				bricks[j].pos.y + bScale + bHalf  > bricks[k].pos.y &&
				bricks[j].pos.x == bricks[k].pos.x 								 &&
				bricks[j].zHeight == bricks[k].zHeight
			) {
		
			 bricks[j].drawDown = false; 	 
			}
			
		

			// check wether to draw a shadow
			if (bricks[j].flying == true && bricks[j].zHeight > 0 && bricks[j].drawShadow == false) {

				if (bricks[j].pos.x == bricks[k].pos.x 	 &&
				bricks[j].pos.y == bricks[k].pos.y 		   &&
				bricks[j].zHeight  > bricks[k].zHeight 	 &&
				bricks[j].drawShadow == false ) {
				bricks[j].sHeight = bricks[k].pos.z;
				bricks[j].drawShadow = true;
				} 
				
				if (bricks[j].pos.x == bricks[k].pos.x 	   		 &&
				bricks[j].pos.y == bricks[k].pos.y 				 &&
				bricks[j].zHeight - 1 == bricks[k].zHeight 				
				) bricks[j].drawShadow = false;
			
			}
		
		}

	// finalise some drawing methods 

	if ( bricks[j].drawUp == true && bricks[j].drawDown == true &&
	     bricks[j].drawLeft == true && bricks[j].drawTop == true && 
	     bricks[j].drawRight == true ) bricks[j].drawAll = true;
	else bricks[j].drawAll = false;


	if (bricks[j].flying == true && bricks[j].zHeight > 0 && bricks[j].drawShadow == false) {
	bricks[j].drawShadow = true;
	bricks[j].sHeight = vw.aspz - bEigth; 
	}




	}

}


// the draw call for the bricks
static void bricksDraw() {

float bScale, bHalf, bQtr, bEigth;
point3d rr,ff,dd,pp,sff,spp;
int i;

bScale = bricks[0].scale;
bHalf = bScale * .5;
bQtr = bHalf * .5;
bEigth = bQtr * .5;
	
	for ( i = brickNo - 1; i >= 0; i--) {

	rr.x = bScale; rr.y = 0.0f; rr.z = 0.0f; // brick size X
	dd.x = 0.0f; dd.y = bScale; dd.z = 0.0f; // brick size Y
	ff.x = 0.0; ff.y = 0.0f; ff.z = bScale; // brick height

	// special ffx for shadow & Panel height
	sff.x = 0.0; sff.y = 0.0f; sff.z = bEigth; 

	pp.x = bricks[i].pos.x; pp.y = bricks[i].pos.y; pp.z = bricks[i].pos.z;

	if (bricks[i].destroy == true) {
		brickNo--; bricks[i] = bricks[brickNo]; continue;
	}

	// draw shadow
	if (bricks[i].drawShadow == true) {
	spp.x = bricks[i].pos.x; spp.y = bricks[i].pos.y; spp.z =  bricks[i].sHeight - bEigth; 
	voxie_drawcube(&vf,&spp, &rr, &dd, &sff, 2, COLOUR_GREY);	
	}

	/*	// draw all
		if (bricks[i].drawAll == true) {
		voxie_drawcube(&vf,&pp, &rr, &dd, &ff, bricks[i].fill , bricks[i].col);	
		continue;
		}
	*/
		// draw top
		if (bricks[i].drawTop == true) {
		spp.x = bricks[i].pos.x; spp.y = bricks[i].pos.y; spp.z = bricks[i].pos.z;
		voxie_drawcube(&vf,&spp, &rr, &dd, &sff, bricks[i].fill, bricks[i].col);	
		}


		// draw bottom
		if (bricks[i].drawBottom == true) {
		spp.x = bricks[i].pos.x; spp.y = bricks[i].pos.y; spp.z = bricks[i].pos.z + bScale - bEigth ;
		voxie_drawcube(&vf,&spp, &rr, &dd, &sff, bricks[i].fill, bricks[i].col3);	
		}


		if (bricks[i].drawLeft == true) {
		// draw left
		spp.x = bEigth; spp.y = rr.y; spp.z = rr.z;
		dd.y = bScale;
		voxie_drawcube(&vf,&pp, &spp, &dd, &ff, bricks[i].fill, bricks[i].col);	
		}


		// draw up
		if (bricks[i].drawUp == true) {
			spp.x = bScale; spp.y = rr.y; spp.z = rr.z;
			dd.y = bEigth;
			voxie_drawcube(&vf,&pp, &spp, &dd, &ff, bricks[i].fill, bricks[i].col2);	
		}
		// draw right
		if (bricks[i].drawRight == true) {
			pp.x = bricks[i].pos.x + bScale - bEigth; pp.y = bricks[i].pos.y;
			spp.x = bEigth; spp.y = rr.y; spp.z = rr.z;
			dd.y = bScale;
			voxie_drawcube(&vf,&pp, &spp, &dd, &ff, bricks[i].fill, bricks[i].col3);	
		}
		// draw down	
		if (bricks[i].drawDown == true) {
			pp.x = bricks[i].pos.x; pp.y = bricks[i].pos.y + bScale - bEigth;
			spp.x = bScale; spp.y = rr.y; spp.z = rr.z;
			dd.y = bEigth;
			voxie_drawcube(&vf,&pp, &spp, &dd, &ff, bricks[i].fill, bricks[i].col2);	
		}

		if (bricks[i].drawWire == true) {
		voxie_drawbox(&vf, bricks[i].pos.x, bricks[i].pos.y, bricks[i].pos.z, bricks[i].pos.x + bScale - bEigth, bricks[i].pos.y + bScale - bEigth, bricks[i].pos.z + bScale - bEigth, 1, bricks[i].col );

		}




	}

}

static void bricksClear() {
	int y;
	brickNo = 0;
	for (y=0; y < BRICK_MAX; y++)  {
	bricks[y].destroy = true;
	bricks[y].col = 0x000000;
	bricks[y].col2 = 0x000000;
	bricks[y].col3 = 0x000000;
	} 


}

// build brick map set position and the scale
static void buildBrickMap(point3d _mapPos, float bScale, int ori) {
float bHalf, bQtr, bEigth;
bHalf = bScale * .5;
bQtr = bHalf * .5;
bEigth = bEigth * .5;
int y,x,z;
point3d pp;
float f;








	for(y=0;y<mapY;y++) {
		for(x=0;x<mapX;x++) {
			for(z=0;z<mapZ;z++) {

			if (!map[y][x][z]) continue;
			if (map[y][x][z] > 0) { // 0 means draw nothing
				pp.y = (bScale * y + -1.03 + bQtr) + _mapPos.y;
				pp.x = (bScale * x + -1.01 + bQtr) + _mapPos.x;
				pp.z = (((vw.aspz + 1) - (bScale * z ) + -1.01) - bScale) + _mapPos.z;
				bricks[brickNo].zHeight = z;

			
				switch(ori) {
					case 0:
					case 1:
					bricks[brickNo].pos.x = pp.x; bricks[brickNo].pos.y = pp.y; bricks[brickNo].pos.z = pp.z;
					break;
					case 2:
					case 3:
					bricks[brickNo].pos.y = pp.x; bricks[brickNo].pos.x = pp.y; bricks[brickNo].pos.z = pp.z;
					break;
					case 4:
					bricks[brickNo].pos.z = pp.x; bricks[brickNo].pos.x = pp.y; bricks[brickNo].pos.y = pp.z;
					break;
					
				}
	
				bricks[brickNo].scale = bScale;
				bricks[brickNo].destroy = false;
				bricks[brickNo].flying = false;
				bricks[brickNo].drawWire = false;
				bricks[brickNo].id = brickID;
				bricks[brickNo].fill = 2;
				bricks[brickNo].drawWire = false;

				switch(map[y][x][z]) {
					case 1111:
					case 111:
					bricks[brickNo].drawWire = false;
					case 11:
					case 1:
					bricks[brickNo].type = 1;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_RED;
					bricks[brickNo].col2 = COLOUR_RED_DIM;
					bricks[brickNo].col3 = COLOUR_RED_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;															
					break;
					case 2222:
					case 222:
					bricks[brickNo].drawWire = false;
					case 22:
					case 2: // create BLUE brick
					bricks[brickNo].type = 2;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_BLUE;
					bricks[brickNo].col2 = COLOUR_BLUE_DIM;
					bricks[brickNo].col3 = COLOUR_BLUE_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;			
					break;
					case 3333:
					case 333:
					bricks[brickNo].drawWire = false;
					case 33:
					case 3: // create GREEN brick
					bricks[brickNo].type = 3;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_GREEN;
					bricks[brickNo].col2 = COLOUR_GREEN_DIM;
					bricks[brickNo].col3 = COLOUR_GREEN_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;			
					break;
					case 4444:
					case 444:
					bricks[brickNo].drawWire = false;
					case 44:
					case 4: // create YELLOW brick
					bricks[brickNo].type = 4;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_YELLOW;
					bricks[brickNo].col2 = COLOUR_YELLOW_DIM;
					bricks[brickNo].col3 = COLOUR_YELLOW_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;			
					break;
					case 5555:
					case 555:
					bricks[brickNo].drawWire = false;
					case 55:
					case 5: // create MAGENTA brick
					bricks[brickNo].type = 5;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_MAGENTA;
					bricks[brickNo].col2 = COLOUR_MAGENTA_DIM;
					bricks[brickNo].col3 = COLOUR_MAGENTA_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;			
					break;
					case 6666:
					case 666:
					bricks[brickNo].drawWire = false;
					case 66:
					case 6: // create CYAN brick
					bricks[brickNo].type = 6;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = COLOUR_CYAN;
					bricks[brickNo].col2 = COLOUR_CYAN_DIM;
					bricks[brickNo].col3 = COLOUR_CYAN_ULTRA_DIM;
					bricks[brickNo].unbreakable = false;	
				
					break;
					case 7777:
					case 777:
					bricks[brickNo].drawWire = false;
					case 77:
					case 7: // create RANDOM brick brick
					bricks[brickNo].type = 7;
					bricks[brickNo].flying = false;
					bricks[brickNo].col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
					bricks[brickNo].col2 = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
					bricks[brickNo].col3 = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
					bricks[brickNo].unbreakable = false;	
					break;	
				}


				if (map[y][x][z] == 11 || map[y][x][z] == 111) bricks[brickNo].flying = true;
				if (map[y][x][z] == 22 || map[y][x][z] == 222) bricks[brickNo].flying = true;
				if (map[y][x][z] == 33 || map[y][x][z] == 333) bricks[brickNo].flying = true;
				if (map[y][x][z] == 44 || map[y][x][z] == 444) bricks[brickNo].flying = true;
				if (map[y][x][z] == 55 || map[y][x][z] == 555) bricks[brickNo].flying = true;
				if (map[y][x][z] == 66 || map[y][x][z] == 666) bricks[brickNo].flying = true;
				if (map[y][x][z] == 77 || map[y][x][z] == 777) bricks[brickNo].flying = true;
				
				if (map[y][x][z] == 1111 || map[y][x][z] == 111) { bricks[brickNo].col = COLOUR_RED_ULTRA; bricks[brickNo].col2 = COLOUR_RED_ULTRA_DIM; }
				if (map[y][x][z] == 2222 || map[y][x][z] == 222) { bricks[brickNo].col = COLOUR_BLUE_ULTRA; bricks[brickNo].col2 = COLOUR_BLUE_ULTRA_DIM; }
				if (map[y][x][z] == 3333 || map[y][x][z] == 333) { bricks[brickNo].col = COLOUR_GREEN_ULTRA; bricks[brickNo].col2 = COLOUR_GREEN_ULTRA_DIM; }
				if (map[y][x][z] == 4444 || map[y][x][z] == 444) { bricks[brickNo].col = COLOUR_YELLOW_ULTRA; bricks[brickNo].col2 = COLOUR_YELLOW_ULTRA_DIM; }
				if (map[y][x][z] == 5555 || map[y][x][z] == 555) { bricks[brickNo].col = COLOUR_MAGENTA_ULTRA; bricks[brickNo].col2 = COLOUR_MAGENTA_ULTRA_DIM; }
				if (map[y][x][z] == 6666 || map[y][x][z] == 666) { bricks[brickNo].col = COLOUR_CYAN_ULTRA; bricks[brickNo].col2 = COLOUR_CYAN_ULTRA_DIM; }
				brickNo++;
				brickID++;

				}
			}

		}

	}
}

static void loadBrickMap( int loadMapNo) {

int x,y,z;

// clear the map no
for(y=0;y<mapY;y++) for(x=0;x<mapX;x++)	for(z=0;z<mapZ;z++)



switch(loadMapNo) {
	case 0:						
	break;
	case 1:
	mapY = 15; mapX = 15;	mapZ = 7;
	for(y=0;y<mapY;y++) for(x=0;x<mapX;x++)	for(z=0;z<mapZ;z++)
	{switch (z){
	case 0: map[y][x][z] =  lv1MapA[y][x]; break;
	case 1: map[y][x][z] =  lv1MapB[y][x]; break;
	case 2: map[y][x][z] =  lv1MapC[y][x]; break;
	case 3: map[y][x][z] =  lv1MapD[y][x]; break;
	case 4: map[y][x][z] =  lv1MapE[y][x]; break;
	case 5: map[y][x][z] =  lv1MapF[y][x]; break;
	case 6: map[y][x][z] =  lv1MapG[y][x]; break; 
	default: map[y][x][z] = 0; break;
	} } 
	break;
	case 2: // UFO
	mapY = 2; mapX = 16; mapZ = 7;
	for(y=0;y<mapY;y++) for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z){
	case 0: map[y][x][z] =  lv2MapA[y][x]; break;
	case 1: map[y][x][z] =  lv2MapB[y][x]; break;
	case 2: map[y][x][z] =  lv2MapC[y][x]; break;
	case 3: map[y][x][z] =  lv2MapD[y][x]; break;
	case 4: map[y][x][z] =  lv2MapE[y][x]; break;
	case 5: map[y][x][z] =  lv2MapF[y][x]; break;
	case 6: map[y][x][z] =  lv2MapG[y][x]; break;
	default: map[y][x][z] = 0; break;  } } 
	break;
	case 3: // crab invader
	mapY = 15; mapX = 15;	mapZ = 8;
	for(y=0;y<mapY;y++)  for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z) {
	case 0: map[y][x][z] =  lv3MapA[y][x]; break;
	case 1: map[y][x][z] =  lv3MapB[y][x]; break;
	case 2: map[y][x][z] =  lv3MapC[y][x]; break;
	case 3: map[y][x][z] =  lv3MapD[y][x]; break;
	case 4: map[y][x][z] =  lv3MapE[y][x]; break;
	case 5: map[y][x][z] =  lv3MapF[y][x]; break;
	case 6: map[y][x][z] =  lv3MapG[y][x]; break;
	case 7: map[y][x][z] =  lv3MapH[y][x]; break;
	default: map[y][x][z] = 0; break;
	} } 
	break;
	case 4: // octopus invader
	mapY = 15; mapX = 15;	mapZ = 8;
	for(y=0;y<mapY;y++)  for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z) {
	case 0: map[y][x][z] =  lv4MapA[y][x]; break;
	case 1: map[y][x][z] =  lv4MapB[y][x]; break;
	case 2: map[y][x][z] =  lv4MapC[y][x]; break;
	case 3: map[y][x][z] =  lv4MapD[y][x]; break;
	case 4: map[y][x][z] =  lv4MapE[y][x]; break;
	case 5: map[y][x][z] =  lv4MapF[y][x]; break;
	case 6: map[y][x][z] =  lv4MapG[y][x]; break;
	case 7: map[y][x][z] =  lv4MapH[y][x]; break;
	default: map[y][x][z] = 0; break;
	} } 
	break;
	case 5: // squid map
	mapY = 2; mapX = 8;	mapZ = 8;
	for(y=0;y<mapY;y++)  for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z) {
	case 0: map[y][x][z] =  lv5MapA[y][x]; break;
	case 1: map[y][x][z] =  lv5MapB[y][x]; break;
	case 2: map[y][x][z] =  lv5MapC[y][x]; break;
	case 3: map[y][x][z] =  lv5MapD[y][x]; break;
	case 4: map[y][x][z] =  lv5MapE[y][x]; break;
	case 5: map[y][x][z] =  lv5MapF[y][x]; break;
	case 6: map[y][x][z] =  lv5MapG[y][x]; break;
	case 7: map[y][x][z] =  lv5MapH[y][x]; break;
	default: map[y][x][z] = 0; break;
	} } 
	break;
	case 6: // bunker full
	mapY = 4; mapX = 22; mapZ= 16;		
	for(y=0;y<mapY;y++)  for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z) {
	case 0: map[y][x][z] =   lv6MapA[y][x]; break;
	case 1: map[y][x][z] =   lv6MapB[y][x]; break;
	case 2: map[y][x][z] =   lv6MapC[y][x]; break;
	case 3: map[y][x][z] =   lv6MapD[y][x]; break;
	case 4: map[y][x][z] =   lv6MapE[y][x]; break;
	case 5: map[y][x][z] =   lv6MapF[y][x]; break;
	case 6: map[y][x][z] =   lv6MapG[y][x]; break;
	case 7: map[y][x][z] =   lv6MapH[y][x]; break;
	case 8: map[y][x][z] =   lv6MapI[y][x]; break;
	case 9: map[y][x][z] =   lv6MapJ[y][x]; break;
	case 10: map[y][x][z] =  lv6MapK[y][x]; break;
	case 11: map[y][x][z] =  lv6MapL[y][x]; break;
	case 12: map[y][x][z] =  lv6MapM[y][x]; break;
	case 13: map[y][x][z] =  lv6MapN[y][x]; break;
	case 14: map[y][x][z] =  lv6MapO[y][x]; break;
	case 15: map[y][x][z] =  lv6MapP[y][x]; break;		
	default: map[y][x][z] = 0; break;
	} } 
	break;
		case 7: // bunker small
	mapY = 2; mapX = 11; mapZ= 8;		
	for(y=0;y<mapY;y++)  for(x=0;x<mapX;x++) for(z=0;z<mapZ;z++) {
	switch (z) {
	case 0:  map[y][x][z] =  lv7MapA[y][x]; break;
	case 1:  map[y][x][z] =  lv7MapB[y][x]; break;
	case 2:  map[y][x][z] =  lv7MapC[y][x]; break;
	case 3:  map[y][x][z] =  lv7MapD[y][x]; break;
	case 4:  map[y][x][z] =  lv7MapE[y][x]; break;
	case 5:  map[y][x][z] =  lv7MapF[y][x]; break;
	case 6:  map[y][x][z] =  lv7MapG[y][x]; break;
	case 7:  map[y][x][z] =  lv7MapH[y][x]; break;
	
	default: map[y][x][z] =  0; break;
	} } 
	break;
	}
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i, j, k, mousx = 256, mousy = 256, mousz = 0; 
	point3d ss, tp, pp, rr, dd, ff, pos = {0.0, 0.0, 0.0}, inc = {0.3, 0.2, 0.1}, sff,spp, mapPos = {0.0,0.0,0.0};
	voxie_xbox_t vx[4];
	int x, y, xx, yy, inited = 0, debug = 1, ovxbut[4], vxnplays; //ovxbut and vxnplays sets up the controllers
	float f, g, scaleSize = .15;
	int mapLoad = 1;
	bool isBrickFallOn = false;
	bool isBrickExplodeOn = false;
	bool loadNextMap = false;
	int brickFallInt = 0;
	float loadNextMapDly = 0;
	int ori = 0;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK);
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						 /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/
		return (-1);
	}

	while (!voxie_breath(&in)) // Breath is the update loop
	{
		otim = tim;
		tim = voxie_klock();
		dtim = tim - otim; // the timer
		mousx += in.dmousx;
		mousy += in.dmousy;
		mousz += in.dmousz;
		gbstat = -(in.bstat != 0);
		for (vxnplays = 0; vxnplays < 4; vxnplays++) // seting up game controllers
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
				break; //but, lt, rt, tx0, ty0, tx1, ty1
		}


		// keyboard controls

		// put controls global keys here
		if (voxie_keystat(0x1)) 	voxie_quitloop(); // esc key closes ap
			
	  // change zooms
		if (voxie_keystat(0x0d)) {	scaleSize += 0.005;	inited = 0;} // press +
		if (voxie_keystat(0x0c)) {	scaleSize -= 0.005;	inited = 0;} // press -
	
		if (voxie_keystat(0x2e) == 1) bricksClear(); // press C to  clear bricks
		if (voxie_keystat(0x21) == 1) { isBrickFallOn = !isBrickFallOn; inited = 0; isBrickExplodeOn = false; } // press F to make bricks fall
		if (voxie_keystat(0x2d) == 1 ) { isBrickExplodeOn = !isBrickExplodeOn;  inited = 0; } // press X

		// move brick map
		if (voxie_keystat(0xc8)) { mapPos.y += 0.01; inited = 0; } // press up
		if (voxie_keystat(0xd0)) { mapPos.y -= 0.01; inited = 0; } // press down
		if (voxie_keystat(0xcb)) { mapPos.x += 0.01; inited = 0; } // press left
		if (voxie_keystat(0xcd)) { mapPos.x -= 0.01; inited = 0; } // press right

		// cycle through maps
		if (voxie_keystat(0x10) == 1 ) { mapLoad++; inited = 0; } // press Q
		if (voxie_keystat(0x1e) == 1 ) { mapLoad--; inited = 0; } // press A
		if (voxie_keystat(0x18) == 1 ) { mapLoad--; if (ori == 0) ori = 3; else ori = 0; inited = 0; } // press O
	
		// effects


		// rotation keys // this adds using [ ] keys to rotate the image in emulation mode
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1);
		if (i)
		{
			if (voxie_keystat(0x2a) | voxie_keystat(0x36))
				vw.emuvang = min(max(vw.emuvang + (float)i * dtim * 2.0, -PI * .5), 0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d) | voxie_keystat(0x9d))
				vw.emudist = max(vw.emudist - (float)i * dtim * 2048.0, 400.0); //Ctrl+[,]
			else
				vw.emuhang += (float)i * dtim * 2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);

		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz);

	/**************
	*   START OF  *
	* Innit Phase *
	**************/

		if (inited == 0)
		{
			if (loadNextMap) {
				mapLoad++;
				loadNextMap = false;
			}
			

			inited = 1;
			bricksClear(); // clears out all the bricks
			loadBrickMap(mapLoad); // loads a brick object
			scaleUpdate(); // updates the scale 
			buildBrickMap(mapPos, scaleSize, ori); // builds it 
			bricksProcess(); // processes the data

			if (isBrickExplodeOn) {

				for (i = 0; i < brickNo; i++) {
					bricks[i].vel.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
					bricks[i].vel.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
					bricks[i].vel.z = -fabs((float)((rand()&32767)-16384)/16384.f*vw.aspz) * 2;
					bricks[i].drawShadow = false;
				}


			}
			else if  (isBrickFallOn) {
		
				for (i = 0; i < brickNo; i++) {
				bricks[i].movTo.x = bricks[i].pos.x;
				bricks[i].movTo.y = bricks[i].pos.y;
				bricks[i].movTo.z = bricks[i].pos.z;
				bricks[i].pos.z += -vw.aspz * 2;
				bricks[i].drawShadow = false;



				}
				brickFallInt = brickNo;
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

	/*************************
	*  INPUT CHECK           *
	*                        *
	*************************/

	/**************************
	*   LOGIC CHECK           *
	*                         *
	**************************/




	if (isBrickFallOn == false && loadNextMapDly < tim && loadNextMapDly != 0) {
		brickFallInt = 0;
		loadNextMapDly = 0;
		inited = 0;
	  isBrickFallOn = true;
		loadNextMap = true;
		
	}

	if (isBrickExplodeOn) {

			for (i = 0; i < brickNo; i++) {

				bricks[i].pos.x += bricks[i].vel.x *dtim;
				bricks[i].pos.y += bricks[i].vel.y *dtim;
				bricks[i].pos.z += bricks[i].vel.z *dtim;

				bricks[i].vel.z += 2 * dtim; // gravity

			  if (bricks[i].pos.z  > vw.aspz + 0.01f) { 

				bricks[i].vel.x = 0;
				bricks[i].vel.y = 0;
				bricks[i].vel.z = 0;								


				}
				

			}


	}

	if (isBrickFallOn) {

		if (bricksMovTo(brickFallInt,bricks[brickFallInt].movTo,5,dtim)) {
			brickFallInt--;
			if (brickFallInt < 0) { isBrickFallOn = false; bricksProcess(); loadNextMapDly = tim + 2; }
			
		}


	}


	/*********************
    **	 DRAW GFX PHASE **
	*********************/
	// draw the bricks
	bricksDraw();



	/*******************
	* Debug / Render   *
	*******************/

		// final update loop for frame
		if (debug == 1)
		{
			//draw wireframe box
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);



			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f brickNo %d mapPos.x %1.5f mapPos.y %1.5f mapPos.z %1.5f ori %d", 1.0 / avgdtim, brickNo, mapPos.x, mapPos.y, mapPos.z, ori);
			voxie_debug_print6x8_(30, 85, 0xffc080, -1, "Arrow Keys = move map, + / - = Zoom In / Out Q / A = cycle through brick maps ", 1.0 / avgdtim, brickNo);
			voxie_debug_print6x8_(30, 105, 0x00ffff, -1, "Effects: C = clear bricks, F = make bricks fall, O = ori switch, X = explode bricks", 1.0 / avgdtim, brickNo);



						for (i = 0; i < brickNo; i++) {
				voxie_debug_print6x8_(30, 150 + ( i * 10), 0x00ff00, -1, "brick.pos.x %1.5f, brick.pos.y %1.5f brick.pos.z %1.5f", bricks[i].pos.x, bricks[i].pos.y, bricks[i].pos.z);
			}
		}

		voxie_frame_end();
		voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
