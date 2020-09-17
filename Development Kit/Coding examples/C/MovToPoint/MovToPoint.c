#if 0
!if 1
#Visual C makefile:
MovToPoint.exe: MovToPoint.c voxiebox.h; cl /TP MovToPoint.c /Ox /MT /link user32.lib
	del MovToPoint.obj

!else

#GNU C makefile:
MovToPoint.exe: MovToPoint.c; gcc MovToPoint.c -o MovToPoint.exe -pipe -O3 -s -m64

/*
Voxon Demo Program of moving an object to multiple scripted points
Matthew Vecchio 17th of September 2020
*/


!endif
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <queue>
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
static double gdtim;


const float DEFAULT_RADIUS = 0.01;
const float TRACE_SPEED = 0.5;



// cursor we use to move around the screen 
typedef struct { point3d pos; int col1, focusOn; float radius; bool execute; } cursor_t;
static cursor_t curs; 

// traces are moved from one area to another using their movTo array.
#define TRACE_MOV_TO_MAX 20
typedef struct { point3d pos; point3d movTo[TRACE_MOV_TO_MAX]; int col, movToNo; char* name; float radius; bool isSelected, isHovered, showName, execute; } trace_t;
#define TRACE_MAX 50
int traceNo = 0;
static trace_t traces[TRACE_MAX];



// returns true if location has been reached and updates points
bool moveToPos(point3d *currentPos, point3d movToPos, float speed, double dtim) {

	float acc = 0.05; // accuracy

	point3d pp, vel;
	float f;

	if (currentPos->x < movToPos.x + acc + (speed * dtim) && currentPos->x > movToPos.x - acc - (speed * dtim) &&
		currentPos->y < movToPos.y + acc + (speed * dtim) && currentPos->y > movToPos.y - acc - (speed * dtim) &&
		currentPos->z < movToPos.z + acc + (speed * dtim) && currentPos->z > movToPos.z - acc - (speed * dtim)
		) {
		currentPos->x = movToPos.x;
		currentPos->y = movToPos.y;
		currentPos->z = movToPos.z;
		return true;

	}

	else {
		pp.x = movToPos.x - currentPos->x;
		pp.y = movToPos.y - currentPos->y;
		pp.z = movToPos.z - currentPos->z;

		f = sqrt(double(pp.x) * double(pp.x) + double(pp.y) * double(pp.y) + double(pp.z) * double(pp.z)); // normilize vectors

		vel.x = (speed) * (pp.x / f) * dtim;
		vel.y = (speed) * (pp.y / f) * dtim;
		vel.z = (speed) * (pp.z / f) * dtim;

		currentPos->x += vel.x;
		currentPos->y += vel.y;
		currentPos->z += vel.z;

		return false;
	}

}






// creates a trace - at this stage just a sphere
void traceCreate(char* name, point3d pos, float radius, int col) {

	if (traceNo < TRACE_MAX) {

		traces[traceNo].pos.x = pos.x;
		traces[traceNo].pos.y = pos.y;
		traces[traceNo].pos.z = pos.z;
		
		traces[traceNo].col = col;
		traces[traceNo].radius = radius;

		traces[traceNo].name = name;

	
		traceNo++;
	}



}

void traceAddPoint(int traceID, point3d pos ) {
	int i = traceID;
	int j = 0;
  	
	if (traces[i].movToNo < TRACE_MOV_TO_MAX) {
		j = traces[i].movToNo;

		traces[i].movTo[j].x = pos.x;
		traces[i].movTo[j].y = pos.y;
		traces[i].movTo[j].z = pos.z;
		
		traces[i].movToNo++;
	}



}

void traceUpdate() {
	int i, j;


	for (i = 0; i < traceNo; i++)  {
		if (traces[i].execute == false || traces[i].movToNo == 0) continue;


		if (moveToPos(&traces[i].pos, traces[i].movTo[0], TRACE_SPEED, gdtim) == true) {

			for (j = 0; j < traces[i].movToNo; j++) {

				traces[i].movTo[j].x = traces[i].movTo[j + 1].x;
				traces[i].movTo[j].y = traces[i].movTo[j + 1].y;
				traces[i].movTo[j].z = traces[i].movTo[j + 1].z;

			}
		
			traces[i].movToNo--;



		}
	}



}

void traceDraw() {
	int i, j, col, nodeCol = 0x0000FF, lineCol = 0x00FFFF;
	point3d fontPos = {0,0,0}, fontH = {0,0,0}, fontW ={0,0,0};
	int lenghtOffSet = 0;
	point3d thisPoint = {0,0,0}, nextPoint = {0,0,0}; // this  


	// font size

	fontH.y = 0.1f; // height
	fontW.x = 0.050f; 


	for (i = 0; i < traceNo; i++) {

		col = traces[i].col;
		if (traces[i].isSelected == true) col = 0xFFFFFF;
		else if (traces[i].isHovered == true) col = 0xFFFF00;

		voxie_drawsph(&vf, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, traces[i].radius, 0, col);	
		voxie_drawvox(&vf, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, 0xFFFFFF);
		if (traces[i].isSelected == true || traces[i].isHovered == true) {
			voxie_drawsph(&vf, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, traces[i].radius + (cos(gtim * 4) * .02 ), 0, col);

			// show path for selected trace
			thisPoint.x = traces[i].pos.x; thisPoint.y = traces[i].pos.y; thisPoint.z = traces[i].pos.z;

			for (j = 0;  j < traces[i].movToNo; j++) {

				nextPoint.x = traces[i].movTo[j].x; nextPoint.y = traces[i].movTo[j].y; nextPoint.z = traces[i].movTo[j].z;

				voxie_drawlin(&vf, thisPoint.x, thisPoint.y, thisPoint.z, 
								   nextPoint.x, nextPoint.y, nextPoint.z, lineCol );	

				voxie_drawsph(&vf, nextPoint.x, nextPoint.y, nextPoint.z, traces[i].radius * .5, 0, nodeCol);				   

				thisPoint.x = nextPoint.x; thisPoint.y = nextPoint.y; thisPoint.z = nextPoint.z;
				   


			}




		}
		if (traces[i].isHovered == true || traces[i].showName == true || traces[i].isSelected == true) {

			lenghtOffSet = strlen(traces[i].name);

			fontPos.x = traces[i].pos.x + (-fontH.y * (lenghtOffSet +1) / 4); 	
			fontPos.y = traces[i].pos.y + 0.05;
			fontPos.z = traces[i].pos.z;
			
			voxie_printalph_(&vf,&fontPos,&fontW,&fontH,col,traces[i].name);
		}

	}

}

void traceDebug(float x, float y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0xffffff, -1, "TraceNo = %d", traceNo);

	for (i = 0; i < traceNo; i++) {

		voxie_debug_print6x8_(x, y + 12 + (i * 12), traces[i].col, -1, "%s | Pos %1.2f %1.2f %1.2f | MOVtoNo %d sel %d hov %d | Mov0 %1.2f %1.2f %1.2f | Mov1 %1.2f %1.2f %1.2f | Mov2 %1.2f %1.2f %1.2f  ",
			traces[i].name,
			traces[i].pos.x, traces[i].pos.y, traces[i].pos.z,
			traces[i].movToNo, traces[i].isSelected, traces[i].isHovered,
			traces[i].movTo[0].x, traces[i].movTo[0].y, traces[i].movTo[0].z,
			traces[i].movTo[1].x, traces[i].movTo[1].y, traces[i].movTo[1].z,
			traces[i].movTo[2].x, traces[i].movTo[2].y, traces[i].movTo[2].z			
			);
	
	}

}


void cursorDraw() {

	voxie_drawsph(&vf, curs.pos.x, curs.pos.y, curs.pos.z, curs.radius, 0, curs.col1);
	voxie_drawsph(&vf, curs.pos.x, curs.pos.y, curs.pos.z, curs.radius + (cos(gtim * 2) * .02 ), 0, curs.col1);


}


// clip the cursor so it doesn't go over the edge
void cursorClip() {


	if (curs.pos.x > vw.aspx)	{ curs.pos.x = vw.aspx;  }
	if (curs.pos.x < -vw.aspx)	{ curs.pos.x = -vw.aspx; }
	if (curs.pos.y > vw.aspy)	{ curs.pos.y = vw.aspy;  }
	if (curs.pos.y < -vw.aspy)   { curs.pos.y = -vw.aspy; }
	if (curs.pos.z > vw.aspz)	{ curs.pos.z = vw.aspz;  }
	if (curs.pos.z < -vw.aspz)   { curs.pos.z = -vw.aspz; }

}

char* randomName() {

	int i = rand() % 10;
	char* name;
		switch(i) {
			case 0:
			default: 
				name = "ALPHA";
			break;
			case 1:
				name = "BETA";
			break;
			case 2:
				name = "GAMMA";
			break;
			case 3:
				name = "DELTA";
			break;
			case 4:
				name = "EPSILON";
			break;
			case 5:
				name = "ZETA";
			break;
			case 6:
				name = "ETA";
			break;
			case 7:
				name = "THETA";
			break;
			case 8:
				name = "IOTA";
			break;
			case 9:
				name = "KAPPA";
			break;
		}

	return name;

}


/*
General Functions
*/

// circle col check
static bool circleColChk3d(point3d cir1, float cir1Rad, point3d cir2, float cir2Rad) {
	float sidea = fabs(double(cir1.x) - double(cir2.x));
	float sideb = fabs(double(cir1.y) - double(cir2.y));
	sidea = sidea * sidea;
	sideb = sideb * sideb;
	float distance = (float)sqrt(double(sidea) + double(sideb));
	if (distance < cir1Rad + cir2Rad && cir1.z - cir1Rad < cir2.z + cir2Rad && cir1.z + cir1Rad > cir2.z - cir2Rad) {

		return true;
	}
	return false;
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
	int i,j,k;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		gtim = tim;
		gdtim = dtim;


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

		cursorClip();

		if (in.bstat == 2 && in.obstat != 2 ) { //create point

			if (curs.focusOn == -1) {// if nothing is focused on create a point
				 traceCreate(randomName(),curs.pos, DEFAULT_RADIUS, 0xFF0000);
			} else {

				for (i = 0; i < traceNo; i++) 
				{
				if (traces[i].isSelected == false) continue;
				 	traceAddPoint(i, curs.pos); 
				
				}

			}


		}

		if (in.bstat == 1 && in.obstat != 1 && curs.focusOn != -1 && voxie_keystat(0x2a) == 0  && voxie_keystat(0x36) == 0) { // remove all points hold down shift to select multiple
			curs.focusOn = -1;
			for (i = 0; i < traceNo; i++) traces[i].isSelected = false;
		} 



		// Keyboatd

		if (voxie_keystat(0x39) == 1) { // press SPACE to execute commands

			curs.execute = !curs.execute;
			for (i = 0; i < traceNo; i++) traces[i].execute = curs.execute;

		}






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

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

	/**************************
	*  START OF INNIT PHASE   *
	*                         *
	**************************/

		if (inited == 0)
		{
			inited = 1;
			curs.col1 = 0x00FF00;
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

	for(i = 0; i < traceNo; i++) {

		traces[i].isHovered = false;
		if (circleColChk3d(curs.pos, curs.radius * 2, traces[i].pos, traces[i].radius ) && traces[i].isSelected == false) {
			traces[i].isHovered = true;

			if (in.bstat == 1 && in.obstat != 1) { 

				traces[i].isSelected = true;
				curs.focusOn = i;
			}
		}


	}

	if (curs.execute == true) {

		//draw wireframe box around the edge of the screen
		voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

		traceUpdate();

	}


	/**************************
	*   DRAW                  *
	*                         *
	**************************/

	cursorDraw();
	traceDraw();

	/**************************
	*   DEBUG                 *
	*                         *
	**************************/

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 


			voxie_debug_print6x8_(30, 80, 0x00ff00, -1, "*** MovToPoint - Demo of moving an object to multiple scripted points ***\nby Matthew Vecchio 17/9/2020\n\n\nUse the mouse to add tracers in the 3D space\nRight Click - Create new trace\nLeft Click - Select a trace\nWhile a trace is selected right click to assign a movement command\npress SPACE BAR to execute all movement commands\nhold SHIFT to select multiple points");
			
			//curs debug
			voxie_debug_print6x8_(30, 185, 0xffc080, -1, "curs X %1.2f Y  %1.2f Z  %1.2f Focus %d Exe %d", curs.pos.x, curs.pos.y, curs.pos.z, curs.focusOn, curs.execute); 

			traceDebug(30,200);

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
