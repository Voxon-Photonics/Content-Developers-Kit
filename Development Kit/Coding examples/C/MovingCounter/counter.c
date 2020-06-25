#if 0
!if 1
#Visual C makefile:
counter.exe: counter.c voxiebox.h; cl /TP counter.c /Ox /MT /link user32.lib
	del counter.obj

!else

#GNU C makefile:
counter.exe: counter.c; gcc counter.c -o counter.exe -pipe -O3 -s -m64

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



// particle Char - these can be used to make numbers and letters
#define PARTCHAR_MAX 20
typedef struct { point3d pos, vel, anchorPos, xRot, yRot, zRot, movTo; float speed, scale, angle, oAngle; int col, col2; } partCharNo_t;
static int partCharNo = 0;
static partCharNo_t partChars[PARTCHAR_MAX];
int colors[6] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF};


static float gScale = .15;
static float gSpeed = 1;

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


static int partCharMap0[15] = {
	0,1,0,
	1,0,1,
	1,0,1,
	1,0,1,
	0,1,0,

};

static int partCharMap1[15] = {
	0,1,0,
	1,1,0,
	0,1,0,
	0,1,0,
	1,1,1,

};

static int partCharMap2[15] = {
	1,1,1,
	0,0,1,
	1,1,1,
	1,0,0,
	1,1,1,

};

static int partCharMap3[15] = {
	1,1,1,
	0,0,1,
	0,1,1,
	0,0,1,
	1,1,1,

};

static int partCharMap4[15] = {
	1,0,1,
	1,0,1,
	1,1,1,
	0,0,1,
	0,0,1,

};

static int partCharMap5[15] = {
	1,1,1,
	1,0,0,
	1,1,1,
	0,0,1,
	1,1,0,

};

static int partCharMap6[15] = {
	1,1,1,
	1,0,0,
	1,1,1,
	1,0,1,
	1,1,1,

};

static int partCharMap7[15] = {
	1,1,1,
	0,0,1,
	0,0,1,
	0,0,1,
	0,0,1,

};

static int partCharMap8[15] = {
	1,1,1,
	1,0,1,
	1,1,1,
	1,0,1,
	1,1,1,

};

static int partCharMap9[15] = {
	1,1,1,
	1,0,1,
	1,1,1,
	0,0,1,
	0,0,1,

};


//Create the shape of a number
void particleMakeNum(int number) {
	int totalParts, i;
	float xOffset, yOffset;
	point3d	pp;
	int const MAX_ROWS = 5;
	int const MAX_COLS = 3;
	int const MAP_MAX = 15;
	int map[MAP_MAX];
	int x, y, newPart = 0;

	pp.x = partChars[0].anchorPos.x ;
	pp.y = partChars[0].anchorPos.y ;
	pp.z = partChars[0].anchorPos.z ;
	
	// clear the partChars
	for (i = 0; i < PARTCHAR_MAX; i++) { partChars[i].col = 0x000000; }

	switch(number) {
		case 0:
			totalParts = 8;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap0[i]; }
		break;
		case 1:
			totalParts = 8;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap1[i]; }
		break;
		case 2:
			totalParts = 11;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap2[i]; }
		break;
		case 3:
			totalParts = 10;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap3[i]; }
		break;
		case 4:
			totalParts = 9;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap4[i]; }
		break;
		case 5:
			totalParts = 10;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap5[i]; }
		break;
		case 6:
			totalParts = 12;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap6[i]; }
		break;
		case 7:
			totalParts = 7;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap7[i]; }
		break;
		case 8:
			totalParts = 13;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap8[i]; }
		break;
		case 9:
			totalParts = 10;
			for( i = 0; i < MAP_MAX; i++) { map[i] = partCharMap9[i]; }
		break;
	}

	// nominate movTos
	x = 0; y = 0; newPart = 0;
	for (i = 0; i < MAP_MAX; i++) {
		
		if (x > MAX_COLS - 1) { y++; x = 0; }
		x++;

		if (map[i] == 0) continue; 
	
		if (newPart <= totalParts) {
	
			partChars[newPart].col = partChars[newPart].col2;
			partChars[newPart].movTo.x = pp.x + (partChars[newPart].scale * x );
			partChars[newPart].movTo.y = pp.y + (partChars[newPart].scale * y );
			partChars[newPart].movTo.z = pp.z + (partChars[newPart].scale * y / 2);
			newPart++;
		} 
		else break;
	}
	
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	voxie_xbox_t vx[4]; 
	int countDown = 0;
	int i;
	float f;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;
	double countDownTim;

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
		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 
		for (vxnplays = 0; vxnplays < 4; vxnplays++) 
		{
			ovxbut[vxnplays] = vx[vxnplays].but; 

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
			break; //but, lt, rt, tx0, ty0, tx1, ty1
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
			for (i = 0; i < PARTCHAR_MAX; i++) {
				partChars[i].col = 0x000000;
				partChars[i].col2 = colors[rand() % 6];
				partChars[i].scale = gScale;
				partChars[i].speed = gSpeed;

				partChars[i].anchorPos.x = 0 - gScale * 2;
				partChars[i].anchorPos.y = 0 - gScale;
				partChars[i].anchorPos.z = -vw.aspz / 3;

				partChars[i].pos.x = 0;
				partChars[i].pos.y = 0;
				partChars[i].pos.z = 0;
				

				partChars[i].xRot.x = partChars[i].scale;
				partChars[i].xRot.y = 0;
				partChars[i].xRot.z = 0;
				
				partChars[i].yRot.x = 0;
				partChars[i].yRot.y = partChars[i].scale;
				partChars[i].yRot.z = 0;

				partChars[i].zRot.x = 0;
				partChars[i].zRot.y = 0;
				partChars[i].zRot.z = partChars[i].scale;

				partCharNo++;
			}
			particleMakeNum(0);
		}

	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*  INPUT                  *
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

	if  (voxie_keystat(0x02) == 1)  particleMakeNum(1);	// press 1
	if  (voxie_keystat(0x03) == 1)  particleMakeNum(2);	// press 1
	if  (voxie_keystat(0x04) == 1)  particleMakeNum(3);	// press 1
	if  (voxie_keystat(0x05) == 1)  particleMakeNum(4);	// press 1
	if  (voxie_keystat(0x06) == 1)  particleMakeNum(5);	// press 1
	if  (voxie_keystat(0x07) == 1)  particleMakeNum(6);	// press 1
	if  (voxie_keystat(0x08) == 1)  particleMakeNum(7);	// press 1
	if  (voxie_keystat(0x09) == 1)  particleMakeNum(8);	// press 1
	if  (voxie_keystat(0x0A) == 1)  particleMakeNum(9);	// press 1
	if  (voxie_keystat(0x0B) == 1)  particleMakeNum(0);	// press 1
	if  (voxie_keystat(0x0C) == 1)  countDown = 10;	// press 1
	
	if (countDown > 0 && countDownTim < tim)	{

		countDown--;
		countDownTim = tim + 2.5;
		particleMakeNum(countDown);


	}
	
		

	f = 0.01; // amount of leeway of calculation
	// update the partChar
	for (i = 0; i < partCharNo; i++) {

		
		// debug
		//voxie_drawsph(&vf, partChars[i].pos.x , partChars[i].pos.y, partChars[i].pos.z , .02 , 1, partChars[i].col);
		//voxie_debug_print6x8_(30, 100 + (i * 20), 0xffc080, -1, "No %d, movTo x %1.2f, y %1.2f, z %1.2f", i, partChars[i].movTo.x, partChars[i].movTo.y, partChars[i].movTo.z); 

		if (partChars[i].col == 0x000000) continue;

		// draw partChar
		voxie_drawspr(&vf, "crab_anim0.obj", &partChars[i].pos, &partChars[i].xRot, &partChars[i].yRot, &partChars[i].zRot, partChars[i].col);
		//rotvex(1 * dtim, &partChars[i].xRot, &partChars[i].zRot);
		
		if ( // check movto amount for partChar
		partChars[i].pos.x < partChars[i].movTo.x + f && partChars[i].pos.x > partChars[i].movTo.x - f &&
		partChars[i].pos.y < partChars[i].movTo.y + f && partChars[i].pos.y > partChars[i].movTo.y - f &&
		partChars[i].pos.z < partChars[i].movTo.z + f && partChars[i].pos.z > partChars[i].movTo.z - f
		) continue;
		else {

			pp.x = partChars[i].movTo.x - partChars[i].pos.x;
			pp.y = partChars[i].movTo.y - partChars[i].pos.y;
			pp.z = partChars[i].movTo.z - partChars[i].pos.z;

			f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z);

			partChars[i].vel.x = 	(partChars[i].speed ) * (pp.x / f) * dtim;
			partChars[i].vel.y = 	(partChars[i].speed ) * (pp.y / f) * dtim;
			partChars[i].vel.z = 	(partChars[i].speed ) * (pp.z / f) * dtim;

			partChars[i].pos.x += partChars[i].vel.x;
			partChars[i].pos.y += partChars[i].vel.y;
			partChars[i].pos.z += partChars[i].vel.z;
		}



	}


	/**************************
	*   DEBUG                 *
	*                         *
	**************************/
	pp.x = 0; pp.y = -.7; pp.z = 0 + cos(tim ) / 6;;
	rr.x = 1; rr.y = 0; rr.z = 0;
	dd.x = 0; dd.y = 1; dd.z = 0;
	ff.x = 0; ff.y = 0; ff.z = 1 / 2; 

	voxie_drawspr(&vf, "gameoverdude2.obj", &pp, &rr, &dd, &ff, 0x00FFFF);

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
			
			
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
		
			voxie_debug_print6x8_(30, 100 , 0xffff00, -1, "Press 1 - 0 to create some numbers" ); 

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
