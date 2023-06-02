#if 0
!if 1
#Visual C makefile:
HeightMap.exe: HeightMap.c voxiebox.h; cl /TP HeightMap.c /Ox /MT /link user32.lib
	del HeightMap.obj

!else

#GNU C makefile:
HeightMap.exe: HeightMap.c; gcc HeightMap.c -o HeightMap.exe -pipe -O3 -s -m64

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

/* 
	Example on for using and understanding heightmap data types 
	by Matthew Vecchio for Voxon - 1/6/2023


	A series of helper functions to help working with HeightMaps
	rotation and scale.

	TODO:
	add menu to pic


*/




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



// based on Ken's solution -- rotates 
void rotateHeightMap( float angleInRads, point3d * pos, point3d *rVec, point3d * dVec, point3d * fVec, bool useDegrees = false ) 
{
	float fx, fy, g, c, s, gx, gy, gz, u, v, rd;
	
	if (useDegrees) angleInRads = angleInRads * (PI/180);

	fx = rVec->x; gx = dVec->x;
	fy = rVec->y; gy = dVec->y;
	gz = fVec->z;

	c = cos(angleInRads); 
	s = sin(angleInRads);
	g = rVec->x;
	rVec->x = rVec->x*c + rVec->y*s;
	rVec->y = rVec->y*c -    g*s;
	g = dVec->x;
	dVec->x = dVec->x*c + dVec->y*s;
	dVec->y = dVec->y*c -    g*s;

	rd = 1.f/(fx*gy - fy*gx);

	u = (pos->x*gy - pos->y*gx)*rd;
	v = (pos->y*fx - pos->x*fy)*rd;
	pos->x = rVec->x*u + dVec->x*v;
	pos->y = rVec->y*u + dVec->y*v;

}


// scales a Height Map (Zooms in / out )
// updates the pos, rVec, dVec and fVec for a Heightmap - based on Ken's solution
void scaleHeightMap( float amount, float dtim, point3d * pos, point3d *rVec, point3d * dVec, point3d * fVec ) 
{
	float fx, fy, gx, gy, gz, u, v, rd, fval;

	fx = rVec->x; gx = dVec->x;
	fy = rVec->y; gy = dVec->y;
	gz = fVec->z;

	if (amount > 0) fval = pow((float)2.0,(float)dtim); 
	else			fval = pow((float)0.5,(float)dtim);		

	rVec->x *= fval; dVec->x *= fval;
	rVec->y *= fval; dVec->y *= fval;
	fVec->z *= fval;

		//0 = pos.x + fx*u + gx*v
		//0 = pos.y + fy*u + gy*v
		//0 = pos.x'+ gr.x*u + gd.x*v
		//0 = pos.y'+ gr.y*u + gd.y*v

		//fx*u + fy*v = gp.x
		//gx*u + gy*v = gp.y
	rd = 1.f/(fx*gy - fy*gx);
	u = (pos->x*gy - pos->y*gx)*rd;
	v = (pos->y*fx - pos->x*fy)*rd;
	pos->x = rVec->x*u + dVec->x*v;
	pos->y = rVec->y*u + dVec->y*v;

		//gp.z_old +   gz*f/256.0 = 0.0
		//gp.z     + gf.z*f/256.0 = 0.0
	pos->z *= fVec->z/gz;


}


// returns the middle point between the two points
// simple similar to working out the average - add them then divide by 2 
point3d CalcMidPoint( point3d p1, point3d p2) 
{
	point3d result;
	result.x = (p1.x + p2.x ) * 0.5;
	result.y = (p1.y + p2.y ) * 0.5;
	result.z = (p1.z + p2.z ) * 0.5;
	return result;

}

// get Centre Pos of HeightMap or any 3D model with rVec, dVec and fVec
point3d getCentrePos(point3d * pos, point3d *rVec, point3d * dVec, point3d * fVec ) 
{
	point3d p1 = { pos->x, pos->y, pos->z };
	point3d p2 = { rVec->x + dVec->x + fVec->x, rVec->y + dVec->y + fVec->y, rVec->z + dVec->z + fVec->z };
	point3d result = {(p1.x + p2.x ) * 0.5f,  (p1.y + p2.y ) * 0.5f, (p1.z + p2.z ) * 0.5 };

	return result;
}


// draw Guides 
void reportHeightMap(voxie_frame_t * vf, point3d * pos, point3d *rVec, point3d * dVec, point3d * fVec)
{
	// work out the left, upper, top point 
	point3d lutPoint = { pos->x, pos->y, pos->z};

	// work out the right down bottom point
	point3d rdbPoint = { pos->x + rVec->x + dVec->x + fVec->x, pos->y + rVec->y + dVec->y + fVec->y, pos->z + rVec->z + dVec->z + fVec->z };

	// work out the centre point
	point3d cPoint = CalcMidPoint(lutPoint, rdbPoint);

	point3d textPos = { 0,0,0};
	point3d textWidth = { 0.1, 0, 0}; 
	point3d textHeight = { 0, 0.1, 0}; 

	float r = 0.05; // sphere  size
	int f = 0; // fillmode

	// draw a cube around the heightmap
	voxie_drawcube(vf, pos, rVec, dVec, fVec, 1, 0xff00ff);

	voxie_drawsph(vf, lutPoint.x, lutPoint.y, lutPoint.z, r, 1, 0x00ff00);
	voxie_drawsph(vf, rdbPoint.x, rdbPoint.y, rdbPoint.z, r, 1, 0xff00ff);
	voxie_drawsph(vf, cPoint.x, cPoint.y, cPoint.z, r, 1, 0xffff00);

	voxie_drawsph(vf, pos->x, pos->y, pos->z, r, f, 0x00ff00);
	textPos.x = pos->x; 	textPos.y = pos->y + 0.1; 	textPos.z = pos->z - 0.1;
	voxie_printalph(vf,&textPos,&textWidth, &textHeight, 0x00ff00,"pos");

	voxie_drawsph(vf, rVec->x, rVec->y, rVec->z, r, f, 0xff0000);
	textPos.x = rVec->x; 	textPos.y = rVec->y + 0.1; 	textPos.z = rVec->z - 0.1;
	voxie_printalph(vf,&textPos,&textWidth, &textHeight, 0xff0000,"rVec");

	voxie_drawsph(vf, dVec->x, dVec->y, dVec->z, r, f, 0x0000ff);
	textPos.x = dVec->x; 	textPos.y = dVec->y + 0.1; 	textPos.z = dVec->z - 0.1;
	voxie_printalph(vf,&textPos,&textWidth, &textHeight, 0x0000ff,"dVec");

	voxie_drawsph(vf, fVec->x, fVec->y, fVec->z, r, f, 0x00ffff);
	textPos.x = fVec->x; 	textPos.y = fVec->y + 0.1; 	textPos.z = fVec->z - 0.1;
	voxie_printalph(vf,&textPos,&textWidth, &textHeight, 0x00ffff,"fVec");

}

char gHeiFilePath[256]; 

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i;

	int inited = 0; 
	int debug = 1;

	point3d pos = { 0 }, rVec = { 0 }, dVec = { 0 }, fVec = { 0 }; 
	point3d pp = { 0 }, rr = { 0 }, dd = { 0 }, ff = { 0 }; 
	point3d scale = { 1, 1, 1 };

	float hmHeight = 0;
	float movSpeed =  0.5;
	float rotSpeed = 1;
	int editMode = 0;

	voxie_wind_t vw;
	voxie_frame_t vf; 

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

		/**************************
		*  INPUT                  *
		*                         *
		**************************/	
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

		/**************************
		*  INIT                   *
		*                         *
		**************************/	

		if (inited == 0)
		{
			// load in the default testing file
			strcpy(gHeiFilePath, "canyon.png");

			// the pos is the starting position for the heightmap always Upper, Top, Left
			pos.x = -vw.aspx;
			pos.y = -vw.aspy;
			pos.z = 0;

			// right vector 
			rVec.x = vw.aspx; 

			// down vector
			dVec.y = vw.aspy;

			// forward vector (the ff.z holds the height scale pos.z + fVec = max height distance)
			fVec.z = vw.aspz;
			inited = 1;

		}


		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

   //Draw heightmap
   //  filnam: filename or pointer to 2d array containing image; heightmap stored alpha channel
   // p/r/d/f: position of top-left corner and orientation (restrictions: r.z = d.z = f.x = f.y = 0)
   //          Also: p.z is height=0, p.z+f.z is height=255
   //colorkey: ARGB 32-bit color to be transparent (supported in nearest mode only)
   //   flags: (1<<0): reserved (height dither now controlled by vw.dither)
   //          (1<<1): 0=nearest filter   , 1=bilinear filter  (recommended)
   //          (1<<2): 0=color dither off , 1=color dither on  (recommended)
   //          (1<<3): 0=filnam is filename string, 1=filnam is tiletype * or pointer to 2d array.
   //          (1<<4): 0=texture clamp    , 1=texture wrap
   //          (1<<5): 0=8-bit height     , 1=mapzen.com terrarium style height
   //returns average height in middle region, range:{0..255}
   //void voxie_drawheimap (voxie_frame_t *vf, char *filnam,
   //                     point3d *p, point3d *r, point3d *d, point3d *f,
   //                   int colorkey, int reserved, int flags)

	// fVec.x and fVec.y aren't used as the fVec.z stores the height distance.



	// Move Pos controls
	if (voxie_keystat(0x1d) || voxie_keystat(0x9d) ){ // scale mode -- CTRL
		editMode = 1;
//		 These all work well... especially if you want to distort the heightmap 

		if (voxie_keystat(0xc8)) scale.y -= movSpeed * dtim; // up arrow
		if (voxie_keystat(0xd0)) scale.y += movSpeed * dtim; // down arrow
		if (voxie_keystat(0xcb)) scale.x -= movSpeed * dtim; // left arrow
		if (voxie_keystat(0xcd)) scale.x += movSpeed * dtim; // right arrow
		if (voxie_keystat(0x28)) scale.z -= movSpeed * dtim; // ' or " for moving Z up
		if (voxie_keystat(0x35)) scale.z += movSpeed * dtim; // / or ? for moving Z down



	} else if (voxie_keystat(0x2a) || voxie_keystat(0x36) ){ // rotate -- SHIFT
	
		editMode = 2;

		if (voxie_keystat(0xcb)) rotateHeightMap(rotSpeed * dtim, &pos, &rVec, &dVec, &fVec, false);  // left arrow
		if (voxie_keystat(0xcd)) rotateHeightMap(-rotSpeed * dtim,  &pos, &rVec, &dVec, &fVec, false); // right arrow


// uncomment these if you are interested in seeing this behaviour

//		if (voxie_keystat(0xc8)) rotvex(rotSpeed * dtim, &dVec, &fVec);  // up arrow -- THIS DOESN'T REALLY WORK for HEIGHTMAPS BUT its an example
//		if (voxie_keystat(0xd0)) rotvex(-rotSpeed * dtim, &dVec, &fVec); // up down  -- THIS DOESN'T REALLY WORK for HEIGHTMAPS BUT its an example
//		if (voxie_keystat(0xcb)) rotvex(rotSpeed * dtim, &rVec, &dVec);  // left arrow
//		if (voxie_keystat(0xcd)) rotvex(-rotSpeed * dtim, &rVec, &dVec); // right arrow
//		if (voxie_keystat(0x28)) rotvex(rotSpeed * dtim, &rVec, &fVec);  // ' or " for moving Z up  -- THIS DOESN'T REALLY WORK for HEIGHTMAPS BUT its an example
//		if (voxie_keystat(0x35)) rotvex(-rotSpeed * dtim, &rVec, &fVec); // / or ? for moving Z down  -- THIS DOESN'T REALLY WORK for HEIGHTMAPS BUT its an example
	
	}	 else if (voxie_keystat(0x38) || voxie_keystat(0xb8) ){ // zoom -- ALT
		
		editMode = 3;

		if (voxie_keystat(0xc8)) scaleHeightMap(movSpeed, dtim, &pos, &rVec, &dVec, &fVec); 	// up arrow
		if (voxie_keystat(0xd0)) scaleHeightMap(-movSpeed, dtim, &pos, &rVec, &dVec, &fVec); 	// down arrow
		if (voxie_keystat(0xcb)) scaleHeightMap(movSpeed, dtim, &pos, &rVec, &dVec, &fVec); 	// left arrow
		if (voxie_keystat(0xcd)) scaleHeightMap(-movSpeed , dtim, &pos, &rVec, &dVec, &fVec); 	// right arrow
		if (voxie_keystat(0x28)) scaleHeightMap(movSpeed , dtim, &pos, &rVec, &dVec, &fVec); 	// ' or " for moving Z up
		if (voxie_keystat(0x35)) scaleHeightMap(-movSpeed , dtim, &pos, &rVec, &dVec, &fVec); 	// / or ? for moving Z down

		
	} 
	else { // no SHIFT, CTRL, or ALT being pressed - just straight button press 
		editMode = 0;

		if (voxie_keystat(0xc8)) pos.y -= (movSpeed * dtim); // up arrow
		if (voxie_keystat(0xd0)) pos.y += (movSpeed * dtim); // down arrow
		if (voxie_keystat(0xcb)) pos.x -= (movSpeed * dtim); // left arrow
		if (voxie_keystat(0xcd)) pos.x += (movSpeed * dtim); // right arrow
		if (voxie_keystat(0x28)) pos.z -= (movSpeed * dtim); // ' or " for moving Z up
		if (voxie_keystat(0x35)) pos.z += (movSpeed * dtim); // / or ? for moving Z down

	 	if (voxie_keystat(0x33) == 1) rotateHeightMap(-90,  &pos, &rVec, &dVec, &fVec, true); // '<' to flip -90 degrees
		if (voxie_keystat(0x34) == 1) rotateHeightMap(90,  &pos, &rVec, &dVec, &fVec, true); // right arrow; // '>' to flip 90 degrees

	}


	// apply position	
	pp.x = pos.x; pp.y = pos.y; pp.z = pos.z;

	// apply scale
	rr.x = rVec.x * scale.x; rr.y = rVec.y * scale.y; rr.z = rVec.z * scale.z;
	dd.x = dVec.x * scale.x; dd.y = dVec.y * scale.y; dd.z = dVec.z * scale.z;
	ff.x = fVec.x * scale.x; ff.y = fVec.y * scale.y; ff.z = fVec.z * scale.z;

	hmHeight = ff.z - pp.z ;

	voxie_drawheimap(&vf, gHeiFilePath, &pp, &rr, &dd, &ff,0x00ffff,0,0 );

	reportHeightMap(&vf, &pp, &rr, &dd, &ff);

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

			voxie_debug_print6x8_(30, 78, 0xffffff, -1, "HeightMap Test - Press CTRL, ALT and SHIFT + Arrow Keys and ' / to addjust heightmap!");

			voxie_debug_print6x8_(30, 118, 0x00ff00, -1, "Pos.x %2.1f, Pos.y %2.1f, Pos.z %2.1f",  pos.x,pos.y,pos.z);
			voxie_debug_print6x8_(30, 128, 0xff0000, -1, "RVec.x %2.1f, RVec.y %2.1f, RVec.z %2.1f",  rVec.x,rVec.y,rVec.z);
			voxie_debug_print6x8_(30, 138, 0x0000ff, -1, "DVec.x %2.1f, DVec.y %2.1f, DVec.z %2.1f",  dVec.x,dVec.y,dVec.z);
			voxie_debug_print6x8_(30, 148, 0x00ffff, -1, "FVec.x %2.1f, FVec.y %2.1f, FVec.z %2.1f", fVec.x,fVec.y,fVec.z);
			voxie_debug_print6x8_(30, 158, 0xffff00, -1, "scale.x %2.1f, scale.y %2.1f, scale.z %2.1f, hmHeight = %2.2f",  scale.x,scale.y,scale.z, hmHeight);

			voxie_debug_print6x8_(30, 188, 0xffff00, -1, "After Translations..." );

			voxie_debug_print6x8_(30, 198, 0x00ff00, -1, "Pp.x %2.1f, Pp.y %2.1f, pp.z %2.1f",  pp.x,pp.y,pp.z);
			voxie_debug_print6x8_(30, 208, 0xff0000, -1, "rr.x %2.1f, rr.y %2.1f, rr.z %2.1f",  rr.x,rr.y,rr.z);
			voxie_debug_print6x8_(30, 218, 0x0000ff, -1, "dd.x %2.1f, dd.y %2.1f, dd.z %2.1f",  dd.x,dd.y,dd.z);
			voxie_debug_print6x8_(30, 228, 0x00ffff, -1, "ff.x %2.1f, ff.y %2.1f, ff.z %2.1f",  ff.x,ff.y,ff.z);

			switch(editMode) {
				case 0:
				default:
					voxie_debug_print6x8_(30, 88, 0x00ff00, -1, "** edit mode: %d - Pos **", editMode);
				break;
				case 1:
					voxie_debug_print6x8_(30, 88, 0x00ffff, -1, "** edit mode: %d - Scale **", editMode);
				break;
				case 2:
					voxie_debug_print6x8_(30, 88, 0xff00ff, -1, "** edit mode: %d - Rotate **", editMode);
				break;
				case 3:
					voxie_debug_print6x8_(30, 88, 0xff0000, -1, "** edit mode: %d - Zoom **", editMode);
				break;
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
