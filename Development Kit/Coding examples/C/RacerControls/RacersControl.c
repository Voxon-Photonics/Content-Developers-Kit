#if 0
!if 1
#Visual C makefile:
RacersControl.exe: RacersControl.c voxiebox.h; cl /TP RacersControl.c /Ox /MT /link user32.lib
	del RacersControl.obj

!else

#GNU C makefile:
RacersControl.exe: RacersControl.c; gcc RacersControl.c -o RacersControl.exe -pipe -O3 -s -m64

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


#define RACER_MAX 4
typedef struct { point3d pos, vel, xrot, yrot, zrot; float acceleration, speed, maxSpeed, minSpeed, angle, scale; int col; char * gfxModel; } racer_t;

static racer_t racers[RACER_MAX];
static int racersNo = 0;

static const float RACER_SCALE = 0.15f;
static const float RACER_SPEED = 10;
static const float RACER_ROTATION_SPEED = 2;
static const float JOY_DEADZONE = 10000; // how much dead zone of the analog joystick
static const float RACERS_MAX_SPEED = 1.5;
static const float RACERS_MIN_SPEED = 0.01f;
static const float FRICTION = 1.5;
static const float Z_FLOOR = .3f;

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
	voxie_xbox_t vx[4]; 
	int i;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;
	float carSize = 0.3;
	int noRacers = 1;
	float f,g;

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

			// create racer
			racersNo = 0;
			for (i = 0; i < noRacers; i++) {
				racers[racersNo].scale = RACER_SCALE;
				racers[racersNo].gfxModel = "Muscle_Coupe.obj";
				racers[racersNo].col = 0xFF4040;
				racers[racersNo].acceleration = 2;
				
				racers[racersNo].pos.x = 0;
				racers[racersNo].pos.y = 0;
				racers[racersNo].pos.z = -0.2;

				// work out rotations
				racers[racersNo].xrot.x = RACER_SCALE;
				racers[racersNo].xrot.y = 0;
				racers[racersNo].xrot.z = 0;

				racers[racersNo].yrot.x = 0;
				racers[racersNo].yrot.y = RACER_SCALE;
				racers[racersNo].yrot.z = 0;
				
				racers[racersNo].zrot.x = 0;
				racers[racersNo].zrot.y = 0;
				racers[racersNo].zrot.z = RACER_SCALE;

				// rotate OBJ so it faces the correct way
				rotvex(-PI / 2,&racers[racersNo].xrot,&racers[racersNo].yrot);

				racersNo++;
			}
			
			


		}


	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*  INPUT                  *
	*                         *
	**************************/
	for (i = 0; i < noRacers; i++) {

		if (  voxie_keystat(0xcb) ||(vx[i].but>>2)&1  || vx[i].tx0 > JOY_DEADZONE )  { // move left inputs for keyboard / digital Dpad / i analog stick 
		

		
		
		
			
			if (fabs(racers[i].speed) > 0.2) {
				if ( vx[i].tx0 > -JOY_DEADZONE && vx[i].tx0 < JOY_DEADZONE ) f = RACER_ROTATION_SPEED * (racers[i].acceleration / racers[i].speed) * dtim; // non analog boost
				else f = (RACER_ROTATION_SPEED *  fabs((vx[i].tx0 / JOY_DEADZONE))) * dtim;
				
				racers[i].angle += f;
				rotvex(f , &racers[i].xrot, &racers[i].yrot);	
			}
		} else if (  voxie_keystat(0xcd) ||(vx[i].but>>3)&1  ||  vx[i].tx0 < -JOY_DEADZONE )  { // move right inputs for keyboard / digital Dpad / i analog stick 
			
	


			
			//f = (RACER_ROTATION_SPEED * (abs(vx[i].tx0) / 32000)  ) * dtim;
			if (fabs(racers[i].speed) > 0.2) {
				if ( vx[i].tx0 > -JOY_DEADZONE && vx[i].tx0 < JOY_DEADZONE )  f = RACER_ROTATION_SPEED * (racers[i].acceleration / racers[i].speed ) * dtim;
				else f = (RACER_ROTATION_SPEED * fabs((vx[i].tx0 / JOY_DEADZONE))) * dtim;
			
				racers[i].angle += -f;
				rotvex(-f , &racers[i].xrot, &racers[i].yrot);
			}
		}




		if (  voxie_keystat(0xc8) ||(vx[i].but>>0)&1  || (vx[i].but>>12)&1 )  { // move up inputs for keyboard / digital Dpad / i analog stick 
		
	
			racers[i].speed += ( racers[i].acceleration ) * dtim;
			
			
		} else 	if (  voxie_keystat(0xd0) ||(vx[i].but>>1)&1  || (vx[i].but>>13)&1 )  { // move left inputs for keyboard / digital Dpad / i analog stick 
		
		
			racers[i].speed += ( -racers[i].acceleration ) * dtim;
			
		
			
		}

	}

	/**************************
	*   LOGIC                 *
	*                         *
	**************************/
	for (i = 0; i < noRacers; i++) { 


		if (fabs(racers[i].speed) > 0.01) {
			if 		 (racers[i].speed > 0) racers[i].speed -= FRICTION * dtim;
			else if  (racers[i].speed < 0) racers[i].speed += FRICTION * dtim;

		}

		if (fabs(racers[i].speed) > RACERS_MAX_SPEED ) {
			if	 	 (racers[i].speed > 0) racers[i].speed = RACERS_MAX_SPEED;
			else if  (racers[i].speed < 0) racers[i].speed = -RACERS_MAX_SPEED;

		} 


		racers[i].vel.x = racers[i].speed * cos(racers[i].angle)  * dtim; // angle
		racers[i].vel.y = racers[i].speed * sin(racers[i].angle)  * dtim; // speed
		
		if (racers[i].pos.z <= Z_FLOOR) racers[i].vel.z += 1 * dtim; // 0.008 is gravity

		racers[i].pos.x +=  racers[i].vel.x;
		racers[i].pos.y +=  racers[i].vel.y;
		racers[i].pos.z +=  racers[i].vel.z;

		racers[i].vel.z = 0;


	}
	/**************************
	*   DRAW                  *
	*                         *
	**************************/


	// draw racer
	for (i = 0; i < noRacers; i++) {
	
		voxie_drawspr(&vf,racers[i].gfxModel,&racers[i].pos,&racers[i].xrot,&racers[i].yrot,&racers[i].zrot,racers[i].col);

		voxie_drawcone (&vf, 0 + (cos(racers[i].angle ) / 2), .5 + (sin(racers[i].angle ) / 2) , 0, fabs(racers[i].speed / 20) + 0.05, 
						0,	.5, 0, fabs(racers[i].speed / 10) + 0.05, 1, 0x00ff00 );
		
		/* voxie_drawcone (&vf, sin(racers[i].speed * 2 / PI ) , cos(racers[i].speed / PI ) , 0,  0.05, 
							0,	0, 0,  0.001, 1, 0x00ff00 );
							*/
		

	}

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
			voxie_debug_print6x8_(30, 70, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim);
			voxie_debug_print6x8_(30, 80, 0xffc080, -1, "Use the arrow keys or a gamepad to drive the car!", 1.0 / avgdtim);
			for (i=0; i < noRacers; i++) 
			voxie_debug_print6x8_(30, 100 + (i * 30), racers[i].col, -1, "Car Stats: Speed %1.2f", racers[i].speed); 
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
