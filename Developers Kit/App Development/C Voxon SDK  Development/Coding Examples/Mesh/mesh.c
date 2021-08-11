// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#define WINDOWS_IGNORE_PACKING_MISMATCH
#if 0
!if 1
#Visual C makefile:
mesh.exe: mesh.c voxiebox.h; cl /TP mesh.c /Ox /MT /link user32.lib
	del mesh.obj

!else

#GNU C makefile:
mesh.exe: mesh.c; gcc mesh.c -o mesh.exe -pipe -O3 -s -m64

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



int gMeshNo = 0;
int gVertNo = 0;
int gNextShape = 0;
#define POLTEX_MAX 200
#define MESH_MAX 3000
#define CURSOR_RADIUS 0.03
poltex_t vt[POLTEX_MAX]; 
int mesh[MESH_MAX];

/*
Have a master VT anad load it in
put a center point to make all the map square

*/


// cursors
typedef struct { point3d pos, lineP1; int lineState, lineValue, mode; float radius; } curs_t;
static curs_t curs;


void newMesh(int meshValue) {

	mesh[gMeshNo] = meshValue;
	gMeshNo++;

}


void newVert(point3d pos, int col, int noMesh) {


	if (gVertNo < POLTEX_MAX) {
		int v = gVertNo;
		vt[v].x = pos.x; vt[v].y = pos.y;  vt[v].z = pos.z;
		vt[v].col = 0;          
		gVertNo++;
		gNextShape++;
	
	
		if (gNextShape % 3 == 0 && gNextShape != 0 && curs.mode == 0 && noMesh == 0) {
			// make a triangle
			vt[gVertNo - 3].u = 1; vt[gVertNo - 3].v = 0;
			vt[gVertNo - 2].u = 1; vt[gVertNo - 2].v = 1;
			vt[gVertNo - 1].u = 0; vt[gVertNo - 1].v = 1;
			newMesh(gVertNo - 1);
			newMesh(gVertNo - 2);
			newMesh(gVertNo - 3);
			newMesh(-1);
			gNextShape = 0;

		}

		if (gNextShape % 4 == 0  && curs.mode == 1 && noMesh == 0) {
			// make a square
			vt[gVertNo - 4].u = 0; vt[gVertNo - 4].v = 0;           
			vt[gVertNo - 3].u = 1; vt[gVertNo - 3].v = 0;
			vt[gVertNo - 2].u = 1; vt[gVertNo - 2].v = 1;
			vt[gVertNo - 1].u = 0; vt[gVertNo - 1].v = 1;
			newMesh(gVertNo - 1);
			newMesh(gVertNo - 2);
			newMesh(gVertNo - 3);
			newMesh(gVertNo - 4);
			newMesh(-1);
			gNextShape = 0;


		}

	
	}

	
  



}



void meshDebug() {



}

static void cursDraw() {

	switch(curs.mode) {
		case 0:
		voxie_drawsph(&vf,curs.pos.x, curs.pos.y, curs.pos.z, curs.radius, 0, 0x00ff00);
		break;
		case 1:
		voxie_drawsph(&vf,curs.pos.x, curs.pos.y, curs.pos.z,  curs.radius, 0, 0xff0000);
		break;

	}

	if (curs.lineState == 1) {
		voxie_drawlin(&vf, curs.pos.x, curs.pos.y, curs.pos.z, curs.lineP1.x, curs.lineP1.y, curs.lineP1.z,0xff0000);
	}

}

void meshDebug(int x, int y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0x00ff00, -1, "Vertices %d Mesh %d", gVertNo, gMeshNo);	
	y += 10;
	point3d dd = { 0, .05, 0 };
	point3d rr = { .05, 0, 0 };
	point3d pp = {0,0,0};
	
	for (i = 0 ; i < gVertNo; i++ ) {
		voxie_debug_print6x8_(x , y + (i * 10 ), 0x00ffff, -1, "Vertice %d x %1.3f y %1.3f z %1.3f u %1.2f v %1.2f c %x", i, vt[i].x, vt[i].y, vt[i].z, vt[i].u, vt[i].v, vt[i].col);	
		voxie_drawsph(&vf,vt[i].x, vt[i].y, vt[i].z, 0.01, 0, vt[i].col);

		pp.x = vt[i].x; pp.y = vt[i].y; pp.z = vt[i].z;
		voxie_printalph_(&vf,&pp,&rr,&dd,vt[i].col, "%d", i );
	}
	y += (gVertNo * 10);
	for (i = 0 ; i < gMeshNo; i++ ) {
		voxie_debug_print6x8_(x + ( i * 80), y, 0xff00ff, -1, "Mesh[%d] = %d |", i, mesh[i]);
	
	}
	


}

void meshDraw() {

	if (gVertNo > 2) {
	  	voxie_drawmeshtex(&vf,"texture.png",vt,gVertNo,mesh,gMeshNo,2,0x404040);
	}

}

// clips the mouse so the pointer won't go out of the bounds....
static void cursClip() {
	float f = 1;

	if (vw.clipshape > 0)   f = 1.5;

	float	CURS_MAX_X = vw.aspx * f;
	float	CURS_MAX_Y = vw.aspy * f;
	float 	CURS_MAX_Z = vw.aspz;

		if (curs.pos.x >  CURS_MAX_X) curs.pos.x = CURS_MAX_X;
		if (curs.pos.x < -CURS_MAX_X) curs.pos.x = -CURS_MAX_X;
		if (curs.pos.y >  CURS_MAX_Y) curs.pos.y = CURS_MAX_Y;
		if (curs.pos.y < -CURS_MAX_Y) curs.pos.y = -CURS_MAX_Y;
		if (curs.pos.z >  CURS_MAX_Z) curs.pos.z = CURS_MAX_Z;
		if (curs.pos.z < -CURS_MAX_Z) curs.pos.z = -CURS_MAX_Z;


}

int circleColChk3d(point3d cir1, double cir1Rad, point3d cir2, double cir2Rad) {
	float sidea = fabs(cir1.x - cir2.x);
	float sideb = fabs(cir1.y - cir2.y);
	sidea = sidea * sidea;
	sideb = sideb * sideb;
	float distance = (double)sqrt(sidea + sideb);
	if (distance < cir1Rad + cir2Rad && cir1.z - cir1Rad < cir2.z + cir2Rad && cir1.z + cir1Rad > cir2.z - cir2Rad) {

		return 1;
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	int i,j,k;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;
	float x = 0 ,y = 0,z = 0;
	float f = 0,g = 0;
	int xRows, yRows;
	int mapMode = 0;





	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a single breath renders the whole volume
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 


		/**************************
		*  INPUT                  *
		*                         *
		**************************/
		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 

		f = .5 * dtim;
		curs.pos.x += in.dmousx * ( f * vw.aspx);
		curs.pos.y += in.dmousy * ( f * vw.aspy);
		curs.pos.z += in.dmousz * ( f / 5 * vw.aspz);
		cursClip();

		for(i = 0; i < gVertNo; i++) {
			vt[i].col = 0xffff00;
			pp.x  = vt[i].x; pp.y = vt[i].y; pp.z = vt[i].z;		
			if (circleColChk3d(curs.pos, curs.radius, pp, curs.radius))  	vt[i].col = 0xffffff;

		}


		if (curs.lineState == 1) {

			if (mapMode == 0) {
				vt[curs.lineValue].x = curs.pos.x;
				vt[curs.lineValue].y = curs.pos.y;
			}
			vt[curs.lineValue].z = curs.pos.z;
		

		}

		if (in.bstat == 2 && in.obstat != 2) {

			if (curs.lineState == 0) newVert(curs.pos,0xFFFFFF,0);
		

		}

		if (in.bstat == 1 && in.obstat != 1) {

			if (curs.lineState == 1) curs.lineState = 0;
			else {

				for(i = 0; i < gVertNo; i++) {
						pp.x  = vt[i].x; pp.y = vt[i].y; pp.z = vt[i].z;
						
						if (circleColChk3d(curs.pos, curs.radius, pp, curs.radius))  {

							if (curs.lineState == 0) {	
								curs.lineP1.x = vt[i].x;
								curs.lineP1.y = vt[i].y;
								curs.lineP1.z = vt[i].z;
								curs.lineValue = i;
								curs.lineState = 1;
								break;
								
							}

						}

				}
			}

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


		// draw 4 
		if (voxie_keystat(0x2a)) curs.mode = 1;
		else curs.mode = 0;


		// 
		if (voxie_keystat(0x02) == 1) {
			i = 0;
	        j = 0;
			mapMode = 1;
			float xres = 0.2;
			float yres = xres;
			xRows = 0;
			yRows = 0;
			x = -vw.aspx;
			y = -vw.aspy;
			z = 0;
			for (x = -vw.aspx; x <= vw.aspx; x += xres) {
				for (y = -vw.aspy; y <= vw.aspy; y += yres) {
					pp.x = x; pp.y = y; pp.z = z;	
					newVert(pp,0xffffff,1);		
				yRows++;
				}
				xRows++;
			}
			
			for (i = 0; i < 109; i++) {

							
			if (i == xRows - 1) continue;
			if (i == (xRows * 2) - 1) continue;
			if (i == (xRows * 3) - 1) continue;
			if (i == (xRows * 4) - 1) continue;
			if (i == (xRows * 5) - 1) continue;
			if (i == (xRows * 6) - 1) continue;
			if (i == (xRows * 7) - 1) continue;
			if (i == (xRows * 8) - 1) continue;
			if (i == (xRows * 9) - 1) continue;
			if (i == (xRows * 10) - 1) continue;
			//	if (i % xRows) continue;
				//if (i / (xRows - 1) != 0) continue;
					newMesh(i);
					newMesh(xRows + i);
					newMesh(i + 1);
					newMesh(-1);

					newMesh(xRows + i + 1);
					newMesh(xRows + i);
					newMesh(i + 1);
					newMesh(-1);
		

			}

			/*
			for(i = 0; i < 200; i++ ) {
				pp.x = x; pp.y = y; pp.z = z;			
				newVert(pp,0xffffff,1);
				pp.x = x; pp.y = y + yres; pp.z = z;			
				newVert(pp,0xffffff,1);
				pp.x = x + xres; pp.y = y + yres; pp.z = z;			
				newVert(pp,0xffffff,1);
				pp.x = x + xres; pp.y = y; pp.z = z;			
				newVert(pp,0xffffff,1);
				

				// create square for inner circle always
				/*
				newMesh(gVertNo - 1);
				newMesh(gVertNo - 2);
				newMesh(gVertNo - 3);
				newMesh(-1);
			
				newMesh(gVertNo - 4);
				newMesh(gVertNo - 1);
				newMesh(gVertNo - 3);
				newMesh(-1);
		
				if (i % 2 && c == 0) { // if X odd - go between
			
				newMesh(gVertNo - 5);
				newMesh(gVertNo - 4);
				newMesh(gVertNo - 3);
				newMesh(-1);

				newMesh(gVertNo - 6);
				newMesh(gVertNo - 5);
				newMesh(gVertNo - 3);
				newMesh(-1);	
			
				}

				if (j % 2) { // if j is odd 

				newMesh(gVertNo - 14);
				newMesh(gVertNo - 4);
				newMesh(gVertNo - 1);
				newMesh(-1);

				newMesh(gVertNo - 15);
				newMesh(gVertNo - 4);
				newMesh(gVertNo - 14);
				newMesh(-1);


				}
			

			
			
				c = 0;
				

						
				x += xres * 2;
				if (x > vw.aspx) { x = -vw.aspx; y += yres * 2; j++;} 
				if (y > vw.aspy) break;



			}
			*/
	
			


		
		}




	/**************************
	*  START OF INNIT PHASE   *
	*                         *
	**************************/

		if (inited == 0)
		{
			curs.radius = CURSOR_RADIUS;
			inited = 1;
		}

	/**************************
	*   DRAW                  *
	*                         *
	**************************/
		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

		cursDraw();
		meshDraw();
	

	/**************************
	*   DEBUG                 *
	*                         *
	**************************/

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

			meshDebug(20,200);

			if (gVertNo % 10) {
				voxie_debug_print6x8_(30, 368, 0xff0000, -1, "4");
				
			}

			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f mous X %2.2f, Y %2.2f Z %2.2f Mode %d lineState %d lineValue %d xRows %d yRows %d", 1.0 / avgdtim, curs.pos.x, curs.pos.y, curs.pos.z, curs.mode, curs.lineState, curs.lineValue, xRows, yRows);	
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
