#if 0
!if 1
#Visual C makefile:
lala.exe: lala.c voxiebox.h; cl /TP lala.c /Ox /MT /link user32.lib
	del lala.obj

!else

#GNU C makefile:
lala.exe: lala.c; gcc lala.c -o lala.exe -pipe -O3 -s -m64

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
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

static voxie_wind_t vw;
static voxie_frame_t vf;

/**********************************************************************************************
*
* SA LALA Land demo 28/7/2020 By Matthew Vecchio / ReadyWolf.
* A demo inspired by demoscene demos.
* A showcase of various strange effects i've worked out on the VX1.
* I hope this demo inspires people to make more impressive demos or neat ideas on Voxon Hardware
*
***********************************************************************************************/



//you can iterate through these colours for a full colour scroll || or choose a random colour in this array to get some nice colours for the VX1
int rainbowCol[90] = {
0xFF00EE, 0xFF00DD, 0xFF00CC, 0xFF00BB, 0xFF00AA, 0xFF0099, 0xFF0088, 0xFF0077, 0xFF0066, 0xFF0055, 0xFF0044, 0xFF0033, 0xFF0022, 0xFF0011, 0xFF0000, // RED
0xFF1100, 0xFF2200, 0xFF3300, 0xFF4400, 0xFF5500, 0xFF6600, 0xFF7700, 0xFF8800, 0xFF9900, 0xFFAA00, 0xFFBB00, 0xFFCC00, 0xFFDD00, 0xFFEE00, 0xFFFF00, // YELLOW
0xEEFF00, 0xDDFF00, 0xCCFF00, 0xBBFF00, 0xAAFF00, 0x99FF00, 0x88FF00, 0x77FF00, 0x66FF00, 0x55FF00, 0x44FF00, 0x33FF00, 0x22FF00, 0x11FF00, 0x00FF00, // GREEN
0x00FF11, 0x00FF22, 0x00FF33, 0x00FF44, 0x00FF55, 0x00FF66, 0x00FF77, 0x00FF88, 0x00FF99, 0x00FFAA, 0x00FFBB, 0x00FFCC, 0x00FFDD, 0x00FFEE, 0x00FFFF, // CYAN
0x00EEFF, 0x00DDFF, 0x00CCFF, 0x00BBFF, 0x00AAFF, 0x0099FF, 0x0088FF, 0x0077FF, 0x0066FF, 0x0055FF, 0x0044FF, 0x0033FF, 0x0022FF, 0x0011FF, 0x0000FF, // BLUE
0x1100FF, 0x2200FF, 0x3300FF, 0x4400FF, 0x5500FF, 0x6600FF, 0x7700FF, 0x8800FF, 0x9900FF, 0xAA00FF, 0xBB00FF, 0xCC00FF, 0xDD00FF, 0xEE00FF, 0xFF00FF // MAGENTA
};
int const COL_SCROLL_MAX = 90; 


// i use these types to manage various objects in the demo.

// simple primitive used with drawing voxie primitives (used as transitions in this demo)
typedef struct { point3d pos, vel, size; int col, fillmode, type; float radius;  } prim_t;
#define PRIMITIVE_MAX 500
int primNo = 0;
static prim_t prims[PRIMITIVE_MAX];

// simple 3d Object xRot, yRot, zRot used to remember rotation data 
typedef struct { point3d vel, pos, xRot, yRot, zRot, size; int col; char* filename;  } obj3d_t;
#define OBJECT3D_MAX 100
int obj3dNo = 0;
static obj3d_t obj3ds[OBJECT3D_MAX];

// simple 2d Object //tl tr lb br stands for top left, top right, bottom left, bottom right
typedef struct { point3d pos, size; point2d tl, tr, bl, br; int col, flip; char* filename; float speed, angle; } obj2d_t;
#define OBJECT2D_MAX 40
int obj2dNo = 0;
static obj2d_t obj2ds[OBJECT2D_MAX];


poltex_t vtext[4]; int mesh[5]; 

// divide a hex colour value by a division number (use this as when using voxie_drawspr the colour values are a ~4th of the range)
int colorHexDivide(int _colour, float _divideAmount) {

	int b, g, r;

	b = (_colour & 0xFF);
	g = (_colour >> 8) & 0xFF;
	r = (_colour >> 16) & 0xFF;

	if (_divideAmount == 0) return 0;

	b /= _divideAmount;
	g /= _divideAmount;
	r /= _divideAmount;
	
	return (r << 16) | (g << 8) | (b);

}


// tweens a colour to a destination colour... good for fade outs or tweens 
int tweenCol(int colour, int speed, int destColour) {

	int b, g, r;
	int bd, gd, rd;

	b = (colour & 0xFF);
	g = (colour >> 8) & 0xFF;
	r = (colour >> 16) & 0xFF;
	bd = (destColour & 0xFF);
	gd = (destColour >> 8) & 0xFF;
	rd = (destColour >> 16) & 0xFF;

	if (b > bd)	b -= speed;
	else if (b < bd) b += speed;
	if (r > rd)	r -= speed;
	else if (r < rd) r += speed;
	if (g > gd)	g -= speed;
	else if (g < gd) g += speed;

	if (r < 0x00) r = 0x00;
	if (r > 0xFF) r = 0xFF;
	if (g < 0x00) g = 0x00;
	if (g > 0xFF) g = 0xFF;
	if (b < 0x00) b = 0x00;
	if (b > 0xFF) b = 0xFF;
	
	return (r << 16) | (g << 8) | (b);

}

// create a 3d Object
void objCreate(char* filename, point3d pos, point3d size, int col) {

	if (obj3dNo < OBJECT3D_MAX) {

		obj3ds[obj3dNo].pos.x = pos.x;
		obj3ds[obj3dNo].pos.y = pos.y;
		obj3ds[obj3dNo].pos.z = pos.z;

		obj3ds[obj3dNo].vel.x = 0;
		obj3ds[obj3dNo].vel.y = 0;
		obj3ds[obj3dNo].vel.z = 0;


		obj3ds[obj3dNo].xRot.x = size.x;
		obj3ds[obj3dNo].xRot.y = 0;
		obj3ds[obj3dNo].xRot.z = 0;

		obj3ds[obj3dNo].yRot.x = 0;
		obj3ds[obj3dNo].yRot.y = size.y;
		obj3ds[obj3dNo].yRot.z = 0;

		obj3ds[obj3dNo].zRot.x = 0;
		obj3ds[obj3dNo].zRot.y = 0;
		obj3ds[obj3dNo].zRot.z = size.z;

		obj3ds[obj3dNo].col = col;
	
		obj3ds[obj3dNo].size.x = size.x;
		obj3ds[obj3dNo].size.y = size.y;
		obj3ds[obj3dNo].size.z = size.z;

		obj3ds[obj3dNo].filename = filename; 
		
		obj3dNo++;
	}


}

void objDebug(float x, float y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0xffffff, -1, "obj3dNo = %d", obj3dNo); 

	for (i = 0; i < obj3dNo; i++) {

		voxie_debug_print6x8_(x, y + 12 + (i * 12), obj3ds[i].col, -1, "%s | Pos %1.2f %1.2f %1.2f | Vel %1.2f %1.2f %1.2f | XRot %1.2f %1.2f %1.2f | YRot %1.2f %1.2f %1.2f | ZRot %1.2f %1.2f %1.2f",
		obj3ds[i].filename,
		obj3ds[i].pos.x, obj3ds[i].pos.y, obj3ds[i].pos.z,
		obj3ds[i].vel.x, obj3ds[i].vel.y, obj3ds[i].vel.z,
		obj3ds[i].xRot.x, obj3ds[i].xRot.y, obj3ds[i].xRot.z,
		obj3ds[i].yRot.x, obj3ds[i].yRot.y, obj3ds[i].yRot.z,
		obj3ds[i].zRot.x, obj3ds[i].zRot.y, obj3ds[i].zRot.z ); 

	} 

}

void objDebug2D(float x, float y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0xffffff, -1, "obj2dNo = %d", obj2dNo); 

	for (i = 0; i < obj2dNo; i++) {

		voxie_debug_print6x8_(x, y + 12 + (i * 12), 0xffffff, -1, "%s |	Pos %1.2f %1.2f %1.2f |	TL %1.2f %1.2f | TR %1.2f %1.2f | BL %1.2f %1.2f | BR %1.2f %1.2f",
		obj2ds[i].filename,
		obj2ds[i].pos.x,  obj2ds[i].pos.y,  obj2ds[i].pos.z,
		obj2ds[i].tl.x, obj2ds[i].tl.y, 
		obj2ds[i].tr.x, obj2ds[i].tr.y,
		obj2ds[i].bl.x, obj2ds[i].bl.y, 
		obj2ds[i].br.x, obj2ds[i].br.y ); 

	} 

}




void objDraw2d() {
	int i = 0;
	point3d pp,rr,dd,ff;
	
	for(i = 0; i < obj2dNo; i++) {

		pp.z = obj2ds[i].pos.z;
		
		pp.x = obj2ds[i].pos.x + obj2ds[i].tl.x;
		pp.y = obj2ds[i].pos.y + obj2ds[i].tl.y;
		rr.x = obj2ds[i].pos.x + obj2ds[i].tr.x;
		rr.y = obj2ds[i].pos.y + obj2ds[i].tr.y;
		dd.x = obj2ds[i].pos.x + obj2ds[i].bl.x;
		dd.y = obj2ds[i].pos.y + obj2ds[i].bl.y;
		ff.x = obj2ds[i].pos.x + obj2ds[i].br.x;
		ff.y = obj2ds[i].pos.y + obj2ds[i].br.y;

		if (obj2ds[i].flip == 0) {
			vtext[0].x = pp.x; 	vtext[0].y = pp.y;	vtext[0].z = pp.z;
			vtext[1].x = rr.x;	vtext[1].y = rr.y;	vtext[1].z = pp.z;
			vtext[2].x = ff.x;	vtext[2].y = ff.y;	vtext[2].z = pp.z;
			vtext[3].x = dd.x; 	vtext[3].y = dd.y;	vtext[3].z = pp.z;
		} else {
			vtext[0].x = pp.x; 	vtext[0].z = pp.y;	vtext[0].y = pp.z;
			vtext[1].x = rr.x;	vtext[1].z = rr.y;	vtext[1].y = pp.z;
			vtext[2].x = ff.x;	vtext[2].z = ff.y;	vtext[2].y = pp.z;
			vtext[3].x = dd.x; 	vtext[3].z = dd.y;	vtext[3].y = pp.z;
		}


		voxie_drawmeshtex(&vf,obj2ds[i].filename,vtext,5,mesh,6,2,obj2ds[i].col);
	}


}


// returns a random point within the volume
void randomPointInVolume(point3d *pp) {
	pp->x = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspx;
	pp->y = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspy;
	pp->z = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspz;

}

// used for cube demo to find a random point to spawn
void randomFallPoint(point3d *pp) {

	pp->x = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspx * .9;
	pp->y = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspy * .9;
	pp->z = -vw.aspz - fabs((float)((rand() & 32767) - 16384) / 16384.f * 2);

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

//Rotate vectors a & b around their common plane, by ang
static void rotvex(float ang, point2d * a, point2d * b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f * c + b->x * s; b->x = b->x * c - f * s;
	f = a->y; a->y = f * c + b->y * s; b->y = b->y * c - f * s;

}

// draw 3d object
void objDraw() {
	int i = 0;
	for(i = 0; i < obj3dNo; i++) {
		voxie_drawspr(&vf, (char*)obj3ds[i].filename, &obj3ds[i].pos, &obj3ds[i].xRot, &obj3ds[i].yRot, &obj3ds[i].zRot, obj3ds[i].col);
	}

}


//box collision check for 3d space 
static bool boxColChk3d(point3d objectAPos, point3d objectBPos, point3d objectBScale) {

point3d halfScale = { float(objectBScale.x * .5), float(objectBScale.y * .5), float(objectBScale.z * .5),};


	if (objectAPos.y > objectBPos.y - halfScale.y &&
		objectAPos.y < objectBPos.y + halfScale.y &&
		objectAPos.x > objectBPos.x - halfScale.x &&
		objectAPos.x < objectBPos.x + halfScale.x &&
		objectAPos.z > objectBPos.z - halfScale.z &&
		objectAPos.z < objectBPos.z + halfScale.z) return true;
	else	return false;

}


// checks falling cubes for demo scene 6
void checkFallingCubeCollision() {

// if collision above the viewing field reset the position
// if collision with a cube bounce them

	int i = 0;
	int j = 0;
	for(i = 0; i < obj3dNo; i++) {
		for(j = 0; j < obj3dNo; j++) {

			if (i == j) continue;
	
			if (boxColChk3d(obj3ds[i].pos,obj3ds[j].pos,obj3ds[j].size) == true) {

				if (obj3ds[i].pos.z < -vw.aspz) { // if collision is above the playfield reset cube if they collide 

					randomFallPoint(&obj3ds[i].pos);
				} else { // if its below bounce them out

					obj3ds[i].vel.x = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspx * 2;
					obj3ds[i].vel.y = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspy * 2;
					obj3ds[i].vel.z = -2.2; 

				}

			}

		}
	}




}

void objCreate2d(char* filename, point3d pos, point3d size, int col, int flip, float angle) {

	if (obj2dNo < OBJECT2D_MAX) {

		obj2ds[obj2dNo].pos.x = pos.x;
		obj2ds[obj2dNo].pos.y = pos.y;
		obj2ds[obj2dNo].pos.z = pos.z;


		obj2ds[obj2dNo].col = col;
		obj2ds[obj2dNo].flip = flip;
		
		obj2ds[obj2dNo].size.x = size.x;
		obj2ds[obj2dNo].size.y = size.y;
		obj2ds[obj2dNo].size.z = size.z;

		obj2ds[obj2dNo].tl.x = -size.x;
		obj2ds[obj2dNo].tl.y = -size.y;

		obj2ds[obj2dNo].tr.x = size.x;
		obj2ds[obj2dNo].tr.y = -size.y;

		obj2ds[obj2dNo].bl.x = -size.x;
		obj2ds[obj2dNo].bl.y = size.y;

		obj2ds[obj2dNo].br.x = size.x;
		obj2ds[obj2dNo].br.y = size.y;

				// setup the right angle
		rotvex(obj2ds[obj2dNo].angle, &obj2ds[obj2dNo].br, &obj2ds[obj2dNo].bl);
		rotvex(obj2ds[obj2dNo].angle, &obj2ds[obj2dNo].tl, &obj2ds[obj2dNo].tr);


		obj2ds[obj2dNo].filename = filename; 
		
		obj2dNo++;
	}


}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff, ss, xSetA = {0,0,0}, ySetA = {0,0,0}; 
	voxie_xbox_t vx[4]; 
	int i,j,k;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 0;
	int scene = 0;
	int mxLength = 96;
	double mxTimer = 0;
	bool isTransitioning = false;
	int colScroll = 0;
	double colScrollTim = 0, sceneTim = 0, transTim = 0;
	float g,f,h, g2, f2;
	int mx = 0;
	int keyFrame = 0;

	const int SCENE_MAX = 7;

	vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
	vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
	vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

	mesh[0] = 0; mesh[1] = 1; mesh[2] = 2; mesh[3] = 3; mesh[4] = -1;


	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	
	// force aspect ratios so demo always looks nice
	
	vw.aspz = 0.45;
	vw.aspx = 1;
	vw.aspy = 1;
  	vw.usecol = 1; // put in full colour
	vw.ilacemode = 0; 
	voxie_init(&vw);


	
	
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	
	voxie_init(&vw);

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 
		for (vxnplays = 0; vxnplays < 4; vxnplays++) 
		{
			ovxbut[vxnplays] = vx[vxnplays].but; 

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
			break; //but, lt, rt, tx0, ty0, tx1, ty1
		}


		if (voxie_keystat(0x1)) // esc key closes app
		{
			voxie_playsound_update(mx,-1,-1,-1,1); //make sure the music is unloaded (as this can crash some boxes on exit)
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}
	
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1); // keys '[' and ']'
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

	

 /****************************************************************************
 * 
 * INPUT
 * 
 * ***************************************************************************/


	// to learn keyboard scan codes use 'keyview.exe' located in the tools folder == 1 is for just pressed while == 3 is for pressed

		if (voxie_keystat(0x02) == 1) {scene = 1; inited = 0; } // kb 1 
		if (voxie_keystat(0x03) == 1) {scene = 2; inited = 0; }	// kb 2	
		if (voxie_keystat(0x04) == 1) {scene = 3; inited = 0; } // kb 3
		if (voxie_keystat(0x05) == 1) {scene = 4; inited = 0; } // kb 4
		if (voxie_keystat(0x06) == 1) {scene = 5; inited = 0; } // kb 5
		if (voxie_keystat(0x07) == 1) {scene = 6; inited = 0; } // kb 6
		if (voxie_keystat(0x08) == 1) {scene = 7; inited = 0; } // kb 7
		if (voxie_keystat(0x09) == 1) {isTransitioning = true; transTim = tim + 3; inited = 0;}	// kb 8
		if (voxie_keystat(0x20) == 1) {debug = !debug; }		 // kb d toggle debug mode
		if (voxie_keystat(0x0b) == 1) {scene = 0; inited = 0; } // kb 0 credit


		// xSet and ySet are for developing use it to easily place objects in 3d Space
		if (voxie_keystat(0x38)) {
			if (voxie_keystat(0xcb) == 3) ySetA.x -= 0.005;
			if (voxie_keystat(0xcd) == 3) ySetA.x += 0.005;
			if (voxie_keystat(0xc8) == 3) ySetA.y -= 0.005;
			if (voxie_keystat(0xd0) == 3) ySetA.y += 0.005;
			if (voxie_keystat(0x2a) == 3) ySetA.z -= 0.005;
			if (voxie_keystat(0x1d) == 3) ySetA.z += 0.005;
		} else {
			if (voxie_keystat(0xcb) == 3) xSetA.x -= 0.005;
			if (voxie_keystat(0xcd) == 3) xSetA.x += 0.005;
			if (voxie_keystat(0xc8) == 3) xSetA.y -= 0.005;
			if (voxie_keystat(0xd0) == 3) xSetA.y += 0.005;
			if (voxie_keystat(0x2a) == 3) xSetA.z -= 0.005;
			if (voxie_keystat(0x1d) == 3) xSetA.z += 0.005;
		}



 /****************************************************************************
 * 
 * MISC THINGS
 * 
 * ***************************************************************************/


		// rainbow colour scroller
		keyFrame = 0;

		if (colScrollTim < tim) {
			colScrollTim = tim + 0.1; // speed of colScroll
			colScroll++;
			keyFrame = 1;
		
			if (colScroll >= COL_SCROLL_MAX) colScroll = 0;


		}

		// scene timer when its up make a transition and load up a new scene

		if (sceneTim < tim ) {

			isTransitioning = true;
			transTim = tim + 3;
			inited = 0;
			sceneTim = transTim + 1; 

		}

		// music player - set to loop music
		if (mxTimer < tim) {

			mxTimer = tim + mxLength;
			mx = voxie_playsound("mx/M@quadSong.wav",-1,100,100,1);

		}





 /****************************************************************************
 * 
 * SCENES - INITIALIZATION
 * 
 * ***************************************************************************/


		if (inited == 0) // scene initialization - setup for various scenes
		{
			inited = 1;
			obj3dNo = 0;
			obj2dNo = 0;
			g2 = 1;
			f2 = 0;

			vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
			vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
			vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
			vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

			if (isTransitioning == true) {

				primNo = 0;
				j = rand() % 3; // randomise type of transition
				for (i = 0; i < 100; i++) {

					randomPointInVolume(&prims[i].pos);
					randomPointInVolume(&prims[i].vel);
					prims[i].col = rainbowCol[rand() % COL_SCROLL_MAX ];
					prims[i].type = j;
					prims[i].radius = 0;
					prims[i].fillmode = 0;

					primNo++;
				}




			} else {

				switch(scene) {
					default:
					break;
					case 3:
					k = rand() % 5;
					f2 = 1;

					break;
					case 1:
					
						ss.x = .6;
						ss.y = .6;
						ss.z = .6;

						// create obj cubes
						pp.x = -.35;
						pp.y = 0;
						pp.z = 0;
							
						objCreate((char*)"gfx/orangeCube.obj",pp,ss, 0x404040);
						rotvex(-.25,&obj3ds[0].yRot,&obj3ds[0].zRot);
						rotvex(.25,&obj3ds[0].zRot,&obj3ds[0].xRot);
						pp.x = .35;
						pp.z = 0.1;
						
						objCreate((char*)"gfx/greenCube.obj",pp,ss, 0x404040);	
						rotvex(-.25,&obj3ds[1].yRot,&obj3ds[1].zRot);
						rotvex(-.25,&obj3ds[1].zRot,&obj3ds[1].xRot);
				
						ss.x *= 3;
						ss.y *= 3;
						ss.z *= 3;
						
						// La 
						pp.x = 0.34;
						pp.y = 0.11;
						pp.z = 0.44;
						objCreate((char*)"gfx/la.obj",pp,ss, 0xFFFFFF);
						rotvex(PI/2,&obj3ds[2].yRot,&obj3ds[2].zRot); // rotate to face up
				
						rotvex(-.35,&obj3ds[2].yRot,&obj3ds[2].zRot); // rotate to face to read
						rotvex(-.35,&obj3ds[2].xRot,&obj3ds[2].yRot); // to right angle
										

						pp.x = 0.61;
						pp.y = .3;
						pp.z = .89;
						objCreate((char*)"gfx/la.obj",pp,ss, 0xFFFFFF);
						rotvex(PI/2,&obj3ds[3].yRot,&obj3ds[3].zRot);
						rotvex(-.35,&obj3ds[3].yRot,&obj3ds[3].zRot); // rotate to face to read
						rotvex(.35,&obj3ds[3].xRot,&obj3ds[3].yRot); // to right angle
		
						pp.x = .60;
						pp.y = -0.20;
						pp.z = 0.06;

						objCreate((char*)"gfx/sa.obj",pp,ss, 0xFFFFFF);
						rotvex(PI/2,&obj3ds[4].yRot,&obj3ds[4].zRot);
						rotvex(-1,&obj3ds[4].yRot,&obj3ds[4].zRot);

						pp.x = .73;
						pp.y = 1.30;
						pp.z = 0.60;

						ss.x = 2.5;
						ss.y = 2.5;
						ss.z = 2.5;

						objCreate((char*)"gfx/land.obj",pp,ss, 0xFFFFFF);
						rotvex(PI/2,&obj3ds[5].yRot,&obj3ds[5].zRot);
						rotvex(-1,&obj3ds[5].yRot,&obj3ds[5].zRot);
					break;

					case 4: // 2d objs falling;
						ss.x = .5;
						ss.y = .5;
						ss.z = 0;
						for (i = 0; i < 8; i++) {
							// create obj La images
							pp.x = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspx;
							pp.z = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspy;
							pp.y = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspz;

								
							objCreate2d((char*)"2d/laNeon.png",pp,ss, colorHexDivide(rainbowCol[rand() % COL_SCROLL_MAX], 4), 0, 1 );
							obj2ds[i].speed =  (float)((rand() & 32767) - 16384) / 16384.f * 0.6;
							obj2ds[i].speed = fabs(obj2ds[i].speed) + 0.2;

						}

					break;
					case 6: // 3d cubes falling
						ss.x = .2;
						ss.y = .2;
						ss.z = .2;

						// create obj cubes

						for(i = 0; i < 8; i++) {

						objCreate((char*)"gfx/orangeCube.obj",pp,ss, 0x303030);
						randomFallPoint(&obj3ds[i].pos);
											
						objCreate((char*)"gfx/greenCube.obj",pp,ss, 0x303030);
						randomFallPoint(&obj3ds[i + 1].pos);
							
						objCreate((char*)"gfx/purpleCube.obj",pp,ss, 0x303030);
						randomFallPoint(&obj3ds[i + 2].pos);

						objCreate((char*)"gfx/yellowCube.obj",pp,ss, 0x303030);
						randomFallPoint(&obj3ds[i + 3].pos);

						objCreate((char*)"gfx/redCube.obj",pp,ss, 0x303030);
						randomFallPoint(&obj3ds[i + 4].pos);
						}

						checkFallingCubeCollision(); 

					break;
					case 7: // Big LA Cubes
						// the pivoit points in my models went centred so had to get creative to make them interesting
				
						ss.x = 4.1;
						ss.y = 4.1;
						ss.z = 4.1;

						pp.x = 1.35 + xSetA.x;
						
						pp.y = 1.25 + xSetA.y;
						pp.z = 0;
						
						objCreate((char*)"gfx/lagreenCube.obj",pp,ss, 0x404040);

						pp.x = 1.35 + ySetA.x;
						
						pp.y = 1.25 + ySetA.y;
						pp.z = 0;
						
						objCreate((char*)"gfx/laorangeCube.obj",pp,ss, 0x404040);

					break;
				}
			}

		}

 /****************************************************************************
 * 
 * TRANSITION SCREEN 
 * 
 * ***************************************************************************/

		if (isTransitioning == true) {

			g = .3 * dtim;
			for(i = 0; i < primNo; i++) {

				if (prims[i].type == 0) { // transition type 0 is with spheres
					prims[i].radius += 1 * dtim;
					voxie_drawsph(&vf, prims[i].pos.x, prims[i].pos.y, prims[i].pos.z, prims[i].radius, prims[i].fillmode, prims[i].col);
				}

				if (prims[i].type == 1) { // transition type 1 is with lines
					if (keyFrame == 1) randomPointInVolume(&prims[i].vel);
					randomPointInVolume(&prims[i].pos);
					voxie_drawlin(&vf,prims[i].pos.x,prims[i].pos.y,prims[i].pos.z,prims[i].vel.x,prims[i].vel.y,prims[i].vel.z,prims[i].col);
				}

				if (prims[i].type == 2) { // // transition type 2 is with cubes

					if (i >= 15) break;
								
					if (prims[i].pos.x < 0) {
						prims[i].vel.x -= g;
						prims[i].pos.x -= g;
					} else {
						prims[i].vel.x += g;
						prims[i].pos.x += g;
					}

					if (prims[i].pos.y < 0) {
						prims[i].vel.y -= g;
						prims[i].pos.y -= g;
					} else {
						prims[i].vel.y += g;
						prims[i].pos.y += g;
					}

					if (prims[i].pos.z < 0) {
						prims[i].vel.z -= g;
						prims[i].pos.z -= g;
					} else {
						prims[i].vel.z += g;
						prims[i].pos.z += g;
					}
				
					voxie_drawbox(&vf, prims[i].pos.x, prims[i].pos.y, prims[i].pos.z,
					prims[i].vel.x, prims[i].vel.y, prims[i].vel.z, 2, prims[i].col );
					
				}

			}


		
			// transition timer loads the next scene
			if (transTim < tim)  { 
				isTransitioning = false;
				sceneTim = tim + 15;
				inited = 0;
				scene++;
				if (scene > 7) scene = 0;
			}
			

		} else {

 /****************************************************************************
 * 
 * SCENES - LOOP
 * 
 * ***************************************************************************/


			switch(scene) {
				case -1: // Black and White version of Breathing - unused

					g = -.1;
					f = .025;
					h = .4;

					vtext[0].x = -vw.aspx;	 		vtext[0].y = -vw.aspy;		vtext[0].z =  g;      
					vtext[1].x = vw.aspx;			vtext[1].y = -vw.aspy;		vtext[1].z =  g;
					vtext[2].x = vw.aspx;			vtext[2].y = vw.aspy;		vtext[2].z =  g + h; 
					vtext[3].x = -vw.aspx;	 		vtext[3].y = vw.aspy;		vtext[3].z =  g + h; 

					voxie_drawmeshtex(&vf,"2d/border.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;
					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h; 
					voxie_drawmeshtex(&vf,"2d/SALANDTEXT.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;
					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h;  

					voxie_drawmeshtex(&vf,"2d/LaLaBox2.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;

				break;
				case 0: // 2D credit scene 
				
					//g = cos(tim) * .4; 
					//f = sin(tim) * .4;

					vtext[0].x = -vw.aspx;	 		vtext[0].y = -vw.aspy;		vtext[0].z =  -vw.aspz;     
					vtext[1].x = vw.aspx;			vtext[1].y = -vw.aspy;		vtext[1].z =  -vw.aspz;
					vtext[2].x = vw.aspx;			vtext[2].y = vw.aspy;		vtext[2].z =  vw.aspz; 
					vtext[3].x = -vw.aspx;	 		vtext[3].y = vw.aspy;		vtext[3].z =  vw.aspz; 

					voxie_drawmeshtex(&vf,"2d/credit.png",vtext,5,mesh,6,2,0x404040);

				break;
				case 1: 	// Scene 3D creation of the logos bopping up and down LA LAs

					if (voxie_keystat(0x39) == 3) {

						obj3ds[0].col = 0x000000;
						obj3ds[1].col = 0x000000;
						

					}

					for(i = 0; i < obj3dNo; i++) {

						if (i == 0) rotvex(.25 * dtim, &obj3ds[i].yRot, &obj3ds[i].zRot );
						if (i == 1) rotvex(-.25 * dtim, &obj3ds[i].yRot, &obj3ds[i].zRot );
						if (i == 2 || i == 3) obj3ds[i].pos.y += (cos(tim) * 0.001);

					}

					objDraw();

				break;
				case 2: // 2D breathe scene SA LALA land logo

					g = -.1;
					f = sin(tim * 5) * .045;
					h = .4;

					vtext[0].x = -vw.aspx;	 		vtext[0].y = -vw.aspy;		vtext[0].z =  g;      
					vtext[1].x = vw.aspx;			vtext[1].y = -vw.aspy;		vtext[1].z =  g;
					vtext[2].x = vw.aspx;			vtext[2].y = vw.aspy;		vtext[2].z =  g + h; 
					vtext[3].x = -vw.aspx;	 		vtext[3].y = vw.aspy;		vtext[3].z =  g + h; 

					voxie_drawmeshtex(&vf,"2d/border.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;

					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h; 
					voxie_drawmeshtex(&vf,"2d/SALANDTEXT.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;

					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h;  

					voxie_drawmeshtex(&vf,"2d/LaLaBox.png",vtext,5,mesh,6,2,0x404040);

					g -= 0.1 + f;

					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h; 

					voxie_drawmeshtex(&vf,"2d/LaLaText.png",vtext,5,mesh,6,2,0x404040);

				break;
				case 3: // Demo Scene Walls

					// centre piece
					g = 0.2f;

					// movement
					h += 0.5 * dtim;
					if (h > 6) h = -.2;
					f = (sin(tim * xSetA.y) / 2 );


					vtext[0].x = vw.aspx;	 		vtext[0].y = -vw.aspy + g;		vtext[0].z =  0 + (cos(tim) * .25);     
					vtext[1].x = -vw.aspx;			vtext[1].y = -vw.aspy + g;		vtext[1].z =  0 + (cos(tim) * .25);
					vtext[2].x = -vw.aspx;			vtext[2].y = vw.aspy - g;		vtext[2].z =  0 + (sin(tim) * .25);
					vtext[3].x = vw.aspx;	 		vtext[3].y = vw.aspy - g;		vtext[3].z =  0 + (sin(tim) * .25);
					
					switch(k) {
						case 0:
						default:
							vtext[0].u = h;	 		    vtext[0].v = f;           
							vtext[1].u = f; 			vtext[1].v = f;
							vtext[2].u = f; 			vtext[2].v = h;
							vtext[3].u = h; 			vtext[3].v = h;

							if (keyFrame == 1) {
								j = tweenCol(j,2,0x606060);
							if (j >= 0x404040) j = 0x000000;
							}
	
							voxie_drawmeshtex(&vf,"2d/LaLaFull.png",vtext,5,mesh,6,2,j);
							voxie_drawmeshtex(&vf,"2d/LaLaFull2.png",vtext,5,mesh,6,2,0x404040);

						break;
						case 1:

							// crazy spiral texture wrap
							g = (sin(tim *.5) * .5 );
							f = (cos(tim *.5 ) * .5);

							vtext[0].u = 0; vtext[0].v = 0;           
							vtext[1].u = g; vtext[1].v = 0;
							vtext[2].u = f; vtext[2].v = g;
							vtext[3].u = 0; vtext[3].v = f;
				
							voxie_drawmeshtex(&vf,"2d/pattern1.png",vtext,5,mesh,6,2,0x404040);
						break;
						case 2:

							// moving vertical down texture wrap
							g2 += 0.1 * dtim;
							if (g2 > 1) g2 = 0;
							
							vtext[0].u = 0; vtext[0].v = g2;           
							vtext[1].u = 1; vtext[1].v = g2;
							vtext[2].u = 1; vtext[2].v = 0;
							vtext[3].u = 0; vtext[3].v = 0;
							
							j = colorHexDivide(rainbowCol[colScroll], 4);

							voxie_drawmeshtex(&vf,"2d/pattern2.png",vtext,5,mesh,6,2,j);
						break;
						case 3:
						
							if (keyFrame == 1) f2 = (rand() % 10  - 5 * .05) * dtim;
					
							
							vtext[0].u = 0; 	vtext[0].v = f2;           
							vtext[1].u = f2;	vtext[1].v = f2;
							vtext[2].u = f2; 	vtext[2].v = 0;
							vtext[3].u = 0; 	vtext[3].v = 0;


							voxie_drawmeshtex(&vf,"2d/pattern3.png",vtext,5,mesh,6,2,0x404040);
						break;
						case 4:

						
							// moving vertical down texture wrap
							g2 += 0.5 * dtim;
							if (g2 > 3) g2 = 0;

							vtext[0].u = 0; 		vtext[0].v = 1;           
							vtext[1].u = g2;		vtext[1].v = 1;
							vtext[2].u = g2; 		vtext[2].v = 0;
							vtext[3].u = 0; 		vtext[3].v = 0;


							voxie_drawmeshtex(&vf,"2d/pattern4.png",vtext,5,mesh,6,2,0x404040);
						
						break;

					}








				
				break;
				case 4: // LALA 2D movement


					for(i = 0; i < obj2dNo; i++) {

						obj2ds[i].pos.y += obj2ds[i].speed * dtim;

						if (obj2ds[i].pos.y - (obj2ds[i].size.y *.5) > vw.aspy) {

							obj2ds[i].pos.y = -vw.aspy - (obj2ds[i].size.y *.5);
							obj2ds[i].pos.x = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspx;
							obj2ds[i].pos.z = (float)((rand() & 32767) - 16384) / 16384.f * vw.aspz;
							obj2ds[i].col = colorHexDivide(rainbowCol[rand() % COL_SCROLL_MAX], 4);
							obj2ds[i].speed =  (float)((rand() & 32767) - 16384) / 16384.f * 1;
							obj2ds[i].speed = fabs(obj2ds[i].speed) + 0.2;
						}			
					
					}

					objDraw2d();

				break;
				case 5: // colour scroll

					g = -.1;
					f = sin(tim) * .025;
					h = .4;
					f2 -= 0.3 * dtim;
					if (f2 + h < -vw.aspz * 3)  f2 = vw.aspz;
					i = colorHexDivide(rainbowCol[colScroll], 4);

					vtext[0].x = -vw.aspx;	 		vtext[0].y = -vw.aspy;		vtext[0].z =  g;      
					vtext[1].x = vw.aspx;			vtext[1].y = -vw.aspy;		vtext[1].z =  g;
					vtext[2].x = vw.aspx;			vtext[2].y = vw.aspy;		vtext[2].z =  g + h; 
					vtext[3].x = -vw.aspx;	 		vtext[3].y = vw.aspy;		vtext[3].z =  g + h; 


					voxie_drawmeshtex(&vf,"2d/border.png",vtext,5,mesh,6,2,i);

					g -= 0.1 + f;
					vtext[0].z =  g;      
					vtext[1].z =  g;
					vtext[2].z =  g + h; 
					vtext[3].z =  g + h; 
					voxie_drawmeshtex(&vf,"2d/SALANDTEXT.png",vtext,5,mesh,6,2,i);


					g2 = 0;
					for (j = 0; j < 5; j++) {
						g2++;	
						vtext[0].z =  f2 + (g2 * 0.2);      
						vtext[1].z =  f2 + (g2 * 0.2);
						vtext[2].z =  f2 + h + (g2 * 0.2 ); 
						vtext[3].z =  f2 + h + (g2 * 0.2);  
						voxie_drawmeshtex(&vf,"2d/LaLaBox2.png",vtext,5,mesh,6,2,i = tweenCol(i,10,0x000000));
					}

					g -= 0.1 + f;


				break;
				case 6: // Cubes Falling

					g = 2 * dtim; // gravity;

					for(i = 0; i < obj3dNo; i++) {

		

						// velocity and movement programming for cubes
						// this scene is code is a bit ungracefull but it works 
				
						if (obj3ds[i].vel.x > 0.01 * dtim) {
							obj3ds[i].pos.x += obj3ds[i].vel.x * dtim;
							obj3ds[i].vel.x -= 0.01 * dtim;
						} else if (obj3ds[i].vel.x < -0.01 * dtim) {
							obj3ds[i].pos.x += obj3ds[i].vel.x * dtim;
							obj3ds[i].vel.x += 0.01 * dtim;
						} else {
							obj3ds[i].vel.x = 0;
						}

							
						if (obj3ds[i].vel.y > 0.01 * dtim) {
							obj3ds[i].pos.y += obj3ds[i].vel.y * dtim;
							obj3ds[i].vel.y -= 0.01 * dtim;
						} else if (obj3ds[i].vel.y < -0.01 * dtim) {
							obj3ds[i].pos.y += obj3ds[i].vel.y * dtim;
							obj3ds[i].vel.y += 0.01 * dtim;
						} else {
							obj3ds[i].vel.y = 0;
						}
					
						if (obj3ds[i].pos.z  > -vw.aspz + 0.1)  {

							if (obj3ds[i].vel.z > 0.01 * dtim) {
								obj3ds[i].pos.z += obj3ds[i].vel.z * dtim;
								obj3ds[i].vel.z -= 0.01 * dtim;
							} else if (obj3ds[i].vel.z < -0.01 * dtim) {
								obj3ds[i].pos.z += obj3ds[i].vel.z * dtim;
								obj3ds[i].vel.z += 0.01 * dtim;
							} else {
								obj3ds[i].vel.z = 0;
							}
						} else {
							obj3ds[i].vel.z = 0;
						}
				

											
						if (obj3ds[i].pos.z + obj3ds[i].size.z / 2> vw.aspz && obj3ds[i].vel.z >= 0) {

						
							obj3ds[i].col = tweenCol(obj3ds[i].col, 1, 0x000000);
							obj3ds[i].vel.x = 0;
							obj3ds[i].vel.y = 0;
							

							if ( obj3ds[i].col == 0x000000) {

								obj3ds[i].col = 0x404040;
								randomFallPoint(&obj3ds[i].pos);
								obj3ds[i].vel.x = 0;
								obj3ds[i].vel.y = 0;
								obj3ds[i].vel.z = 0;

							}



						} else {
							
							rotvex(obj3ds[i].vel.x * .1, &obj3ds[i].xRot, &obj3ds[i].zRot );
							rotvex(obj3ds[i].vel.y * .1, &obj3ds[i].yRot, &obj3ds[i].zRot );
							//rotvex(obj3ds[i].vel.z * .2, &obj3ds[i].zRot, &obj3ds[i].xRot );
							obj3ds[i].pos.z += g;
							

						}
						
						checkFallingCubeCollision(); 

						
						
				

					}

					objDraw();

				break;
				case 7: // Big LA Cubes 
			

					for(i = 0; i < obj3dNo; i++) {

					
						if (i == 0) rotvex(-1 * dtim, &obj3ds[i].zRot, &obj3ds[i].yRot );
						if (i == 1) rotvex(1 * dtim, &obj3ds[i].zRot, &obj3ds[i].xRot );

					}

					objDraw();

				break;


			}
		}


	/**************************
	*   DEBUG                 *
	*                         *
	**************************/

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			// debug 3d objects
			objDebug(10,150);

			// debug my 2d objects
			objDebug2D(10,350);

			// see my Xset ad Yset points for development
			voxie_debug_print6x8_(100,68, 0x00ff00, -1, "Xset X %1.2f Y %1.2f Z %1.2f I %#08x", xSetA.x, xSetA.y, xSetA.z, i );
			voxie_debug_print6x8_(400,68, 0xff0000, -1, "Yset X %1.2f Y %1.2f Z %1.2f Scene %d", ySetA.x, ySetA.y, ySetA.z, scene);

			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
	
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
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
