#if 0
!if 1
#Visual C makefile:
ringChk.exe: ringChk.c voxiebox.h; cl /TP ringChk.c /Ox /MT /link user32.lib
	del ringChk.obj

!else

#GNU C makefile:
ringChk.exe: ringChk.c; gcc ringChk.c -o ringChk.exe -pipe -O3 -s -m64

/*
Demo of working out the angle between 2 points mapped onto a circle
Matthew Vecchio 2nd of May 2022
*/


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

static double gtim;

// cursor we use to move around the screen 
typedef struct { point3d pos; int col1, focusOn; float radius; bool execute; } cursor_t;
static cursor_t curs; 

// returns a distance value between the two points
double dist2Point3D(point3d pos1, point3d pos2) {

return sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) +   pow(pos1.z - pos2.z, 2));

}
// returns a distance value between the two points
double dist2Point2D(point3d pos1, point3d pos2) {

return sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2));

}
// returns a distance value between the two points
double dist2Point2D(point2d pos1, point2d pos2) {

return sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2));

}

void cursorDraw() {

	voxie_drawsph(&vf, curs.pos.x, curs.pos.y, curs.pos.z, curs.radius, 0, curs.col1);
	voxie_drawsph(&vf, curs.pos.x, curs.pos.y, curs.pos.z, curs.radius + (cos(gtim * 2) * .02 ), 0, curs.col1);


}


// clip the cursor so it doesn't go out of the view the edge
void cursorClip() {


	if (curs.pos.x > vw.aspx)	{ curs.pos.x = vw.aspx;  }
	if (curs.pos.x < -vw.aspx)	{ curs.pos.x = -vw.aspx; }
	if (curs.pos.y > vw.aspy)	{ curs.pos.y = vw.aspy;  }
	if (curs.pos.y < -vw.aspy)   { curs.pos.y = -vw.aspy; }
	if (curs.pos.z > vw.aspz)	{ curs.pos.z = vw.aspz;  }
	if (curs.pos.z < -vw.aspz)   { curs.pos.z = -vw.aspz; }

}

double gDeltaTime;


int sphereCollideChk(point3d * sphereAPos, double sphereARadius, point3d * sphereBPos, double sphereBRadius) {
	float sidea = fabs(sphereAPos->x - sphereBPos->x);
	float sideb = fabs(sphereAPos->y - sphereBPos->y);
	sidea = sidea * sidea;
	sideb = sideb * sideb;
	float distance = (double)sqrt(sidea + sideb);

	int result = 0;
	int collisionCol = 0xff0000;
	
	if (distance < sphereARadius + sphereBRadius && sphereAPos->z - sphereARadius < sphereBPos->z + sphereBRadius && sphereAPos->z + sphereARadius > sphereBPos->z - sphereBRadius) {
		result =  1;
	}

	return result;
}


// moved to Pos used to cursorclipping
int moveToPos(point3d * currentPos, point3d destinationPos, float speed, float accuracy)
{
	float acc = accuracy; // accuracy

	if (!(accuracy > 0)) {
		acc = 0.005;
	}

	point3d pp, vel;
	float f;
	double dtim = gDeltaTime;

	if (currentPos->x < destinationPos.x + acc + (speed * dtim) && currentPos->x > destinationPos.x - acc - (speed * dtim) &&
		currentPos->y < destinationPos.y + acc + (speed * dtim) && currentPos->y > destinationPos.y - acc - (speed * dtim) &&
		currentPos->z < destinationPos.z + acc + (speed * dtim) && currentPos->z > destinationPos.z - acc - (speed * dtim)
		) {
		currentPos->x = destinationPos.x;
		currentPos->y = destinationPos.y;
		currentPos->z = destinationPos.z;
		return 1;

	}
	else {
		pp.x = destinationPos.x - currentPos->x;
		pp.y = destinationPos.y - currentPos->y;
		pp.z = destinationPos.z - currentPos->z;

		f = sqrt(double(pp.x) * double(pp.x) + double(pp.y) * double(pp.y) + double(pp.z) * double(pp.z)); // normalise vectors

		vel.x = (speed) * (pp.x / f) * dtim;
		vel.y = (speed) * (pp.y / f) * dtim;
		vel.z = (speed) * (pp.z / f) * dtim;

		currentPos->x += vel.x;
		currentPos->y += vel.y;
		currentPos->z += vel.z;

		return 0;
	}

}

int clipInsideVolume(voxie_wind_t * vw,  point3d * pos, float radius)
{
	int response = 0;

	if (vw->clipshape == 0) { // if up/down display
		
		if (pos->x > vw->aspx - radius)	{ response += 1; pos->x = vw->aspx - radius;  }
		if (pos->x < -vw->aspx + radius)	{ response += 1; pos->x = -vw->aspx + radius; }
		if (pos->y > vw->aspy - radius)	{ response += 2; pos->y = vw->aspy - radius;	}
		if (pos->y < -vw->aspy + radius) { response += 2; pos->y = -vw->aspy + radius;	}
		if (pos->z > vw->aspz - radius)  { response += 4; pos->z = vw->aspz - radius;  }
		if (pos->z < -vw->aspz + radius) { response += 4; pos->z = -vw->aspz + radius; }
	}
	else { // if spinner
		point3d centrePoint = { 0,0,0 };
		point3d pp = { 0 };

		while (sphereCollideChk(&centrePoint, radius, pos, 0.01) == 0) {
			response = 1;
			centrePoint.z = pos->z;

			pp.z = pos->z;

			moveToPos(pos, pp, 1, 0.0001);
		}
		

		if (pos->z > vw->aspz)	{ response += 2; pos->z = vw->aspz - 0.01; }
		if (pos->z < -vw->aspz)	{ response += 2; pos->z = -vw->aspz + 0.01; }
	}

	return response;

}


// renders a sphere onto a circle - worked out from looking at the centre of the circle versus where the point is. returns the radian angle difference 
double drawSphAtAngle(point3d circlePos, point3d pointPos, float radius, float zHeight, int col, float size ) {

	double radian = atan2(circlePos.y - pointPos.y, circlePos.x - pointPos.x );

	voxie_drawsph(&vf, circlePos.x + (radius * -cos(radian)), circlePos.y + (radius * -sin(radian)), circlePos.z + zHeight, size, 0, col );
	
	return radian; 

}

// draws a point coming out of the circle at that angle
double drawAngleLine(point3d circlePos, float radians, float length, float zHeight, int col, bool useDegrees ) {

	if (useDegrees) radians = radians * PI / 180 ;
	
	voxie_drawlin(&vf, circlePos.x + (length * -cos(radians)), circlePos.y + (length * -sin(radians)), circlePos.z + zHeight, circlePos.x, circlePos.y, circlePos.z, col );
	
	return radians; 

}


void drawBorder() {


	// draw circle 
	float	r = vw.aspr * .99;
	float	rmin = vw.asprmin * 1.03;
	int i = 0, n = 0, j = 0;

	int borderCol = 0; 

	// draw borders 
	if (vw.clipshape == 0) 	voxie_drawbox(&vf, vw.aspx + 1e-3f, -vw.aspy + 1e-3f, -vw.aspz, +vw.aspx - 1e-3f, +vw.aspy - 1e-3f, +vw.aspz, 1, 0xffffff);
	else { // if spinner draw around the outside of the shape
		n = 64;
		for (j = -64; j <= 64; j++)
		{
			if (j == -62) j = 62;
			for (i = 0; i < n; i++)
			{
				// draw outter circle 
				voxie_drawlin(&vf, cos((float)(i + 0) * PI * 2.0 / (float)n) * (r), sin((float)(i + 0) * PI * 2.0 / (float)n) * (r), (float)j * vw.aspz / 64.f,
					cos((float)(i + 1) * PI * 2.0 / (float)n) * (r), sin((float)(i + 1) * PI * 2.0 / (float)n) * (r), (float)j * vw.aspz / 64.f, 0xffffff);

				// draw inner circle 
				voxie_drawlin(&vf, cos((float)(i + 0) * PI * 2.0 / (float)n) * (rmin), sin((float)(i + 0) * PI * 2.0 / (float)n) * (rmin), (float)j * vw.aspz / 64.f,
					cos((float)(i + 1) * PI * 2.0 / (float)n) * (rmin), sin((float)(i + 1) * PI * 2.0 / (float)n) * (rmin), (float)j * vw.aspz / 64.f, 0xffffff);
			}
		}

	}



}

#define DEFAULT_RADIUS 0.05

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
		double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 

	point3d pp, rr, dd, ff; 
	int inited = 0; 
	int debug = 1;
	float f,g,h;
	int i,j,k,n;
	float r = 0; 
	float aout = 0;
	float ain = 0;
	float rmin = 0;
	float xOffset = 0;
	float yOffset = 0;
	

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	vw.clipshape = 1;
	voxie_init(&vw);

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		gtim = tim;
		gDeltaTime = dtim;



	/**************************
	*  INPUT                  *
	*                         *
	**************************/

		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 


		// mouse 
		f = 0.001; // mouse sentivity
		curs.pos.x = mousx * ( f * vw.aspx);
		curs.pos.y = mousy * ( f * vw.aspy);
		curs.pos.z = mousz * ( f * vw.aspz);

		clipInsideVolume(&vw, &curs.pos, vw.aspr);

		if (voxie_keystat(0x1)) // esc key closes ap
		{
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

		if (voxie_keystat(0xc8)) { // arrow up
			yOffset += 0.01;
			if (yOffset > 3) yOffset = 3;
		}

		if (voxie_keystat(0xd0)) { // arrow down
			yOffset -= 0.01;
			if (yOffset < -3) yOffset = -3;
		}

		if (voxie_keystat(0xcb)) { // arrow left
			xOffset -= 0.01;
			if (xOffset < -3) xOffset = -3;
		}

		if (voxie_keystat(0xcd)) { // arrow right
			xOffset += 0.01;
			if (xOffset > 3) xOffset = 3;
		}

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

	/**************************
	*  START OF INNIT PHASE   *
	*                         *
	**************************/

		if (inited == 0)
		{
			inited = 1;
			curs.col1 = 0x00FFFF;
			curs.radius = DEFAULT_RADIUS;
			curs.focusOn = -1;
		}


	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*   LOGIC                 *
	*                         *
	**************************/

	pp.x = 0; pp.y = 0; pp.z = 0;
	r = vw.aspr; // the radius of the outter circle (the size of the display)
	rmin = vw.asprmin; // the radius of the inner circle


	if (sphereCollideChk(&pp, r, &curs.pos, curs.radius )) drawSphAtAngle(pp,curs.pos,r, curs.pos.z, 0x00ff00, 0.1f);

	// draw the sphere on the inner radius (sphereCollideChk ensures it doesn't draw while within the inner radius)
	if (!sphereCollideChk(&pp, rmin, &curs.pos, curs.radius )) drawSphAtAngle(pp,curs.pos,rmin, curs.pos.z, 0x00ff00, 0.1f);



	/**************************
	*   DRAW                  *
	*                         *
	**************************/


	cursorDraw();
	drawBorder();

	// draw your own circle at a custom radius
	r = 0.7;
	n = 64;
	// custom offset
	pp.x = xOffset; pp.y = yOffset; pp.z = 0;
		for (j = -64; j <= 64; j++)
		{
			if (j == -62) j = 62;
			for (i = 0; i < n; i++)
			{
				// draw circle at custom offset 
				voxie_drawlin(&vf, pp.x + cos((float)(i + 0) * PI * 2.0 / (float)n) * (r), pp.y + sin((float)(i + 0) * PI * 2.0 / (float)n) * (r), (float)j * vw.aspz / 64.f,
					pp.x + cos((float)(i + 1) * PI * 2.0 / (float)n) * (r), pp.y + sin((float)(i + 1) * PI * 2.0 / (float)n) * (r), (float)j * vw.aspz / 64.f, 0xff0000);
			}
		}


		// example of ways to use sphereCollideChk
		if (sphereCollideChk(&pp, r, &curs.pos, curs.radius )) i = 0xff0000;
		if (!sphereCollideChk(&pp, r, &curs.pos, curs.radius )) i = 0x0000ff;
		

		aout = drawSphAtAngle(pp,curs.pos,r, curs.pos.z, i, 0.1f);

		drawAngleLine(pp, aout, 1, pp.z, 0x00ff00, false);
		drawAngleLine(pp,  aout * 180 / PI, 1, pp.z, 0x00ff00, true);


	/**************************
	*   DEBUG                 *
	*                         *
	**************************/

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 


			voxie_debug_print6x8_(30, 80, 0x00ff00, -1, "*** Ringcheck - Demo of working out the angle between 2 points mapped onto a circle ***\nby Matthew Vecchio 2/5/2022\n\n\nUse the mouse to move around the cursor... press arrow keys to change the offset\nXoffset %1.2f Yoffset %1.2f ", xOffset, yOffset);
			
			voxie_debug_print6x8_(30, 200, 0xff0000, -1, "ANGLE : in Radians %5.1f in Degrees %3.0f ", aout, aout * 180 / PI); 

			//curs debug
			voxie_debug_print6x8_(30, 185, 0xffc080, -1, "curs X %1.2f Y  %1.2f Z  %1.2f Focus %d Exe %d", curs.pos.x, curs.pos.y, curs.pos.z, curs.focusOn, curs.execute); 


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
