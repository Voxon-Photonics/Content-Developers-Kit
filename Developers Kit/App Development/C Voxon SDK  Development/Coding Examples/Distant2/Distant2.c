#if 0
!if 1
#Visual C makefile:
Distant2.exe: Distant2.c voxiebox.h; cl /TP Distant2.c /Ox /MT /link user32.lib
	del Distant2.obj

!else

#GNU C makefile:
Distant2.exe: Distant2.c; gcc Distant2.c -o Distant2.exe -pipe -O3 -s -m64

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

// traces are points we can use to calcuate 
typedef struct { point3d pos; int col; float radius; double distance; } trace_t;
#define TRACE_MAX 50
int traceNo = 0;
static trace_t traces[TRACE_MAX];


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


// works out the closest between all the points
void  distanceUpdate() {
	int i = 0;

	float closestDistance = vw.aspx + vw.aspy + vw.aspz;
	float furthestDistance = 0;

	for (i = 0; i < traceNo; i++) {
		voxie_drawlin(&vf,curs.pos.x, curs.pos.y, curs.pos.z, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, 0xffff00);
		traces[i].distance = dist2Point3D(curs.pos, traces[i].pos);
	}

	for (i = 0; i < traceNo; i++) {

		traces[i].col = 0xffffff;
		if (fabs(traces[i].distance) > fabs(furthestDistance)	)   furthestDistance = traces[i].distance;  
		if (fabs(traces[i].distance) < fabs(closestDistance)	)  closestDistance = traces[i].distance; 



	}

	for (i = 0; i < traceNo; i++) { 

		if (traces[i].distance == furthestDistance) 	traces[i].col = 0xff0000;
		if (traces[i].distance == closestDistance) 		traces[i].col = 0x00ff00;



	}

	





}





// creates a trace - at this stage just a sphere
void traceCreate(point3d pos, float radius, int col) {

	if (traceNo < TRACE_MAX) {

		traces[traceNo].pos.x = pos.x;
		traces[traceNo].pos.y = pos.y;
		traces[traceNo].pos.z = pos.z;
		
		traces[traceNo].col = col;
		traces[traceNo].radius = radius;

		traces[traceNo].distance = 0;

	
		traceNo++;
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
		thisPoint.x = traces[i].pos.x;
		thisPoint.y = traces[i].pos.y - .1;
		thisPoint.z = traces[i].pos.z;
		
		voxie_drawsph(&vf, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, traces[i].radius, 0, col);	
		voxie_drawvox(&vf, traces[i].pos.x, traces[i].pos.y, traces[i].pos.z, 0xFFFFFF);
		voxie_printalph_(&vf, &thisPoint,&fontW, &fontH, col, "%1.2f", traces[i].distance );
	}

}

void traceDebug(float x, float y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0xffffff, -1, "TraceNo = %d", traceNo);

	for (i = 0; i < traceNo; i++) {

		voxie_debug_print6x8_(x, y + 12 + (i * 12), traces[i].col, -1, " %d | Pos %1.2f %1.2f %1.2f | Distance %1.2f ",
			i,
			traces[i].pos.x, traces[i].pos.y, traces[i].pos.z,
			traces[i].distance			
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

		if (in.bstat == 2 && in.obstat != 2 || in.bstat == 1 && in.obstat != 1   ) { //create point

		
 				traceCreate(curs.pos, DEFAULT_RADIUS, 0xFF0000);

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

	/**************************
	*   DRAW                  *
	*                         *
	**************************/
	distanceUpdate();
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


			voxie_debug_print6x8_(30, 80, 0x00ff00, -1, "*** Distance2 - Demo of working out the distance between 2 points ***\nby Matthew Vecchio 11/3/2022\n\n\nUse the mouse to add points in the 3D space\nRight Click - Create new trace");
			
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
