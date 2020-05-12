#if 0
!if 1
#Visual C makefile:
voxiePie.exe: voxiePie.c voxiebox.h; cl /TP voxiePie.c /Ox /MT /link user32.lib
	del voxiePie.obj

!else

#GNU C makefile:
voxiePie.exe: voxiePie.c; gcc voxiePie.c -o voxiePie.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

/*
VoxiePie - a demo written to work out how to draw a pie like graph. This was written for a stats screen for a Video game 
	but could be modified to be used for graphing or something cool.

04/10/2019 - Matthew Vecchio for Voxon 
*/

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


#define PIE_MAX 4
typedef struct { point3d centre; float angleMin, angleMax, unitsPercent, finalHeight; int col; } pie_t;
static pie_t pies[PIE_MAX];

float Z_RATIO = 0;

static float dotMovementUnitAmount = -vw.aspz / 100;
static float dotMovementSpeed = .25;

static const float PLAYER3_CENTRE_POS = 0;
static const float PLAYER1_CENTRE_POS = 1.570795;
static const float PLAYER4_CENTRE_POS = 3.14159;
static const float PLAYER2_CENTRE_POS = 4.712385;
static const float PIE_ANGLE_DIFF = 0.7853975;

static const float PIE_ORI0_MIN = PLAYER1_CENTRE_POS - PIE_ANGLE_DIFF;
static const float PIE_ORI0_MAX = PLAYER1_CENTRE_POS + PIE_ANGLE_DIFF;
static const float PIE_ORI1_MIN = PLAYER2_CENTRE_POS - PIE_ANGLE_DIFF;
static const float PIE_ORI1_MAX = PLAYER2_CENTRE_POS + PIE_ANGLE_DIFF;
static const float PIE_ORI2_MIN = PLAYER3_CENTRE_POS - PIE_ANGLE_DIFF;
static const float PIE_ORI2_MAX = PLAYER3_CENTRE_POS + PIE_ANGLE_DIFF;
static const float PIE_ORI3_MIN = PLAYER4_CENTRE_POS - PIE_ANGLE_DIFF;
static const float PIE_ORI3_MAX = PLAYER4_CENTRE_POS + PIE_ANGLE_DIFF;


// draw function pies
static void drawPie( point3d pos, float rad, float resolution, int colour, float angleStart, float angleStop) {
	float angle = angleStart;
	int j; 

	// centre pole
	voxie_drawlin(&vf, 0, 0, pos.z, 0,0, Z_RATIO, colour );

	// lines from sides to bottom
	for (j = 0; j < 400; j++) {
	if (pos.z + ( j * resolution) > vw.aspz) continue;
		voxie_drawlin(&vf, rad * cos (angleStart), rad * sin (angleStart), pos.z + ( j * resolution), 0,0,pos.z + ( j * resolution), colour );
		voxie_drawlin(&vf, rad * cos (angleStop),  rad * sin (angleStop) , pos.z + ( j * resolution), 0,0,pos.z + ( j * resolution), colour );
	}

	// draw the semi circle part
	while (angle < angleStop) {
		// calculate x, y from a vector with known length and angle
		pos.x = rad * cos (angle);
		pos.y = rad * sin (angle);
		
		//voxie_drawvox (&vf, pos.x, pos.y, pos.z, colour);
		voxie_drawlin (&vf, pos.x, pos.y, pos.z, pos.x, pos.y, Z_RATIO , colour);
		voxie_drawlin (&vf, pos.x, pos.y, pos.z, 0, 0, pos.z , colour);
		angle += resolution;
	}

}

// draw a circle
static void drawCir( point3d pos, float rad, float resolution, int colour) {
	float angle = 0;

	while (angle < 2 * PI) {
		// calculate x, y from a vector with known length and angle
		pos.x = rad * cos (angle);
		pos.y = rad * sin (angle);
		voxie_drawvox (&vf, pos.x, pos.y, pos.z, colour);
		angle += resolution;
	}

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

static void pieRandomPoints() {
int p1Score, p2Score, p3Score, p4Score, totalScore;

		p1Score = rand() % 5000;
		p2Score = rand() % 5000;
		p3Score = rand() % 5000;
		p4Score = rand() % 5000;

		totalScore = p1Score + p2Score + p3Score + p4Score;

		pies[0].unitsPercent = float(p1Score) / float(totalScore) * 100;
		pies[1].unitsPercent = float(p2Score) / float(totalScore) * 100;
		pies[2].unitsPercent = float(p3Score) / float(totalScore) * 100;
		pies[3].unitsPercent = float(p4Score) / float(totalScore) * 100;




}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
		double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	voxie_xbox_t vx[4]; 
	int i, j;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;
	float VORTEX_SCALE = 0.18,	WORLD_SCALE = 1;
	float dotRes = 0.012;


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
			for (i = 0; i < PIE_MAX; i++) {

				switch(i) {
					case 0:
						pies[i].angleMin = PIE_ORI0_MIN;
						pies[i].angleMax = PIE_ORI0_MAX;
						pies[i].col = 0xFFFF00;
					break;
					case 1:
						pies[i].angleMin = PIE_ORI1_MIN;
						pies[i].angleMax = PIE_ORI1_MAX;
						pies[i].col = 0x00FFFF;
					break;
					case 2:
						pies[i].angleMin = PIE_ORI2_MIN;
						pies[i].angleMax = PIE_ORI2_MAX;
						pies[i].col = 0xFF0000;
					break;
					case 3:
						pies[i].angleMin = PIE_ORI3_MIN;
						pies[i].angleMax = PIE_ORI3_MAX;
						pies[i].col = 0x00FF00;
					break;


				}
				


			}
			inited = 1;
		}


	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*  INPUT                  *
	*                         *
	**************************/

	voxie_debug_print6x8_(30, 168, 0xffc080, -1, "VoxiePie - Pie Chart Demo\n\n Press '1' to randomize height Press '2' or '3' to Change Resolution %1.5f \n Press '4' for random data\n Press '5' for 100 points data\n Press '6' or '7' to Change Speed %1.5f", dotRes, dotMovementSpeed ); 
 	
	// press keyboard '1'
    if (voxie_keystat(0x02) == 1) { for (i = 0; i < PIE_MAX; i++) { pies[i].centre.z = ((float)((rand()&32767)-16384)/16384.f*vw.aspz); pies[i].finalHeight = pies[i].centre.z; } }
	
	// press keyboard '2'
	if (voxie_keystat(0x03) == 1) { dotRes += 0.005f; }
	
	// press keyboard '3'
	if (voxie_keystat(0x04) == 1) { dotRes -= 0.005f; }
	
	// press keyboard '4'
	if (voxie_keystat(0x05) == 1) { 
		pieRandomPoints();
		for (i = 0; i < PIE_MAX; i++)  { 
			pies[i].centre.z = Z_RATIO;
			pies[i].finalHeight = vw.aspz + ( (pies[i].unitsPercent * dotMovementUnitAmount) * 2 ) - scale ;
		} 
	}

	// press keyboard '5'
	if (voxie_keystat(0x06) == 1) { 
		pies[0].unitsPercent = 100;
		pies[1].unitsPercent = 0;
		pies[2].unitsPercent = 50;
		pies[3].unitsPercent = 25;
		
	for (i = 0; i < PIE_MAX; i++)  { 
		pies[i].centre.z = Z_RATIO;
		pies[i].finalHeight = vw.aspz + ( (pies[i].unitsPercent * dotMovementUnitAmount) * 2 ) - scale;
	
		} 
	}

	// press keyboard '6'
	if (voxie_keystat(0x07) == 1) { dotMovementSpeed += 0.01f;  }

	// press keyboard '7'
	if (voxie_keystat(0x08) == 1) { dotMovementSpeed -= 0.01f;  }
	
	voxie_debug_print6x8_(30, 300, 0x00ff00, -1, "Pies Percent | 1: %1.0f | 2: %1.0f | 3: %1.0f | 4: %1.0f |", pies[0].unitsPercent, pies[1].unitsPercent, pies[2].unitsPercent, pies[3].unitsPercent ); 

	/**************************
	*   DRAW                  *
	*                         *
	**************************/
	// draw lines
	Z_RATIO = vw.aspz; // was 0.30;
	VORTEX_SCALE = 0.18;
	WORLD_SCALE = 1;
	dotMovementUnitAmount = -Z_RATIO / 100;
	pp.z = 0;

	//draw pies
	for (i = 0; i < PIE_MAX; i++) {
		// work out pie height movement
		if (pies[i].centre.z == pies[i].finalHeight) { }
		else if (pies[i].centre.z + 0.01 > pies[i].finalHeight && pies[i].centre.z - 0.01 < pies[i].finalHeight ) pies[i].centre.z = pies[i].finalHeight;
		else if (pies[i].centre.z < pies[i].finalHeight)  pies[i].centre.z += dotMovementSpeed * dtim;
		else if (pies[i].centre.z > pies[i].finalHeight ) pies[i].centre.z -= dotMovementSpeed * dtim;
	
		// draw the pie
		drawPie ( pies[i].centre,WORLD_SCALE, dotRes, pies[i].col, pies[i].angleMin, pies[i].angleMax );
	}

	/* Draw some lines to help with sizing
	// draw top
	pp.z = -Z_RATIO;
	drawCir( pp, WORLD_SCALE, 	.02f, 0xFFFFFF);
	drawCir( pp, VORTEX_SCALE, 	.05f, 0xFF0000);
	// draw bottom
	pp.z = Z_RATIO;
	drawCir( pp, WORLD_SCALE, 	.02f, 0xFFFFFF);
	drawCir( pp, VORTEX_SCALE, 	.05f, 0xFF0000);


	// drawing borders
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, 0, 1, Z_RATIO, 0xFFFF00);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, 0, -1, Z_RATIO, 0x00FFFF);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, 1, 0, Z_RATIO, 0xFF0000);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, -1, 0, Z_RATIO, 0x00FF00);

	// sides for players
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, 1, 1, Z_RATIO, 0xFFFFFF);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, 1, -1, Z_RATIO, 0xFFFFFF);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, -1, 1, Z_RATIO, 0xFFFFFF);
	voxie_drawlin (&vf, 0.f, 0.f, Z_RATIO, -1, -1, Z_RATIO, 0xFFFFFF);
	*/

	/**************************
	*   DEBUG                 *
	*                         *
	**************************/


		if (debug == 1) // if debug is set to 1 display these debug messages
		{
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
