#if 0
!if 1
#Visual C makefile:
tally.exe: tally.c voxiebox.h; cl /TP tally.c /Ox /MT /link user32.lib
	del tally.obj

!else

#GNU C makefile:
tally.exe: tally.c; gcc tally.c -o tally.exe -pipe -O3 -s -m64

!endif
!if 0
#endif
/*
	Tally is a short program to create a tallied list 





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




void tally( int tallyTotal, point3d pos, point2d boxSize, float scale, int ori) {
poltex_t vtext[4]; int mesh[5]; 
float halfScale = scale / 2;
int i = 0;
float offSetX = 0;
float offSetY = 0;
float offSetZ = 0;
float blankTallySpace = scale / 10;
float tilt = 0.1; // how much to tilt the graphic 
int debug = 1; // switch this to zero for no box
float f = 0;

	vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
	vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
	vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

	mesh[0] = 0; mesh[1] = 1; mesh[2] = 2; mesh[3] = 3; mesh[4] = -1;
	
	offSetX = blankTallySpace;
	offSetY = blankTallySpace;

	for ( i = 0; i < tallyTotal; i++ ) {

		switch(ori) {
			case 0:
				vtext[0].x = -halfScale + pos.x + offSetX;	vtext[0].y = -halfScale + pos.y + offSetY;	vtext[0].z = pos.z + offSetZ;       
				vtext[1].x = halfScale 	+ pos.x + offSetX; 	vtext[1].y = -halfScale + pos.y + offSetY; 	vtext[1].z = pos.z + offSetZ;
				vtext[2].x = halfScale 	+ pos.x + offSetX;	vtext[2].y =  halfScale + pos.y + offSetY;	vtext[2].z = pos.z + offSetZ + tilt;
				vtext[3].x = -halfScale + pos.x + offSetX;	vtext[3].y =  halfScale + pos.y + offSetY; 	vtext[3].z = pos.z + offSetZ + tilt;

				offSetX += scale + blankTallySpace;
				if ((offSetX + scale + blankTallySpace) > boxSize.x) {offSetY += scale + blankTallySpace; offSetX = blankTallySpace;}
				if ((offSetY + scale + blankTallySpace) > boxSize.y) {offSetZ += (scale / 2 ); offSetX = blankTallySpace; offSetY = blankTallySpace;}
				if (i == 0 && debug == 1) 	voxie_drawbox(&vf, pos.x, pos.y, pos.z, pos.x + boxSize.x - scale, pos.y + boxSize.y - scale, pos.z + 0.1, 1, 0xffff00);
			break;
			case 1:
				vtext[3].x = -halfScale + pos.x - offSetX;	vtext[3].y = -halfScale + pos.y - offSetY;	vtext[3].z = pos.z + offSetZ + tilt;;       
				vtext[2].x = halfScale 	+ pos.x - offSetX; 	vtext[2].y = -halfScale + pos.y - offSetY; 	vtext[2].z = pos.z + offSetZ + tilt;;
				vtext[1].x = halfScale 	+ pos.x - offSetX;	vtext[1].y =  halfScale + pos.y - offSetY;	vtext[1].z = pos.z + offSetZ;
				vtext[0].x = -halfScale + pos.x - offSetX;	vtext[0].y =  halfScale + pos.y - offSetY; 	vtext[0].z = pos.z + offSetZ;

				offSetX += scale + blankTallySpace;
				if ((offSetX + scale + blankTallySpace) > boxSize.x) {offSetY += scale + blankTallySpace; offSetX = blankTallySpace;}
				if ((offSetY + scale + blankTallySpace) > boxSize.y) {offSetZ += (scale / 2 ); offSetX = blankTallySpace; offSetY = blankTallySpace;}
				if (i == 0 && debug == 1) 	voxie_drawbox(&vf, pos.x, pos.y, pos.z, pos.x - boxSize.x + scale, pos.y - boxSize.y + scale, pos.z + 0.1, 1, 0x00ffff);
			break;
			case 2:
				vtext[0].y = -halfScale + pos.x - offSetX;	vtext[0].x = -halfScale + pos.y - offSetY;	vtext[0].z = pos.z + offSetZ;       
				vtext[1].y = halfScale 	+ pos.x - offSetX; 	vtext[1].x = -halfScale + pos.y - offSetY; 	vtext[1].z = pos.z + offSetZ;
				vtext[2].y = halfScale 	+ pos.x - offSetX;	vtext[2].x =  halfScale + pos.y - offSetY;	vtext[2].z = pos.z + offSetZ + tilt;
				vtext[3].y = -halfScale + pos.x - offSetX;	vtext[3].x =  halfScale + pos.y - offSetY; 	vtext[3].z = pos.z + offSetZ + tilt;

				offSetX += scale + blankTallySpace;
				if ((offSetX + scale + blankTallySpace) > boxSize.x) {offSetY -= scale + blankTallySpace; offSetX = blankTallySpace;}
				if ((-offSetY + scale + blankTallySpace) > boxSize.y) {offSetZ += (scale / 2 ); offSetX = blankTallySpace; offSetY = blankTallySpace;}
				if (i == 0 && debug == 1) 	voxie_drawbox(&vf, pos.y, pos.x, pos.z, pos.y + boxSize.y - scale, pos.x - boxSize.x + scale, pos.z + 0.1, 1, 0xff0000);
				break;	
			case 3:
				vtext[3].y = -halfScale + pos.x + offSetX;	vtext[3].x = -halfScale + pos.y + offSetY;	vtext[3].z = pos.z + offSetZ + tilt;       
				vtext[2].y = halfScale 	+ pos.x + offSetX; 	vtext[2].x = -halfScale + pos.y + offSetY; 	vtext[2].z = pos.z + offSetZ + tilt;
				vtext[1].y = halfScale 	+ pos.x + offSetX;	vtext[1].x =  halfScale + pos.y + offSetY;	vtext[1].z = pos.z + offSetZ;
				vtext[0].y = -halfScale + pos.x + offSetX;	vtext[0].x =  halfScale + pos.y + offSetY; 	vtext[0].z = pos.z + offSetZ;

				offSetX += scale + blankTallySpace;
				if ((offSetX + scale + blankTallySpace) > boxSize.x) {offSetY -= scale + blankTallySpace; offSetX = blankTallySpace;}
				if ((-offSetY + scale + blankTallySpace) > boxSize.y) {offSetZ += (scale / 2 ); offSetX = blankTallySpace; offSetY = blankTallySpace;}
				if (i == 0 && debug == 1) 	voxie_drawbox(&vf, pos.y, pos.x, pos.z, pos.y - boxSize.y + scale, pos.x + boxSize.x - scale, pos.z + 0.1, 1, 0x00ff00);
			break;
		}

		voxie_drawmeshtex(&vf,"Tally.png",vtext,5,mesh,6,2,0xFFFFFF);

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
	int noOfTally = 10;
	point3d pos = {0,0,0};
	point2d boxSize = {1,1};
	float scale = 0.2;
	int ori = 0;
	poltex_t vtext[4]; int mesh[4]; 


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
		}


	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*  INPUT                  *
	*                         *
	**************************/

	// keyboard 1
	if (voxie_keystat(0x02) == 1) noOfTally--; 
	// keyboard 2
	if (voxie_keystat(0x03) == 1) noOfTally++;

	// hold left CTRL
	if (voxie_keystat(0x1d)) {

		// arrow up
		if (voxie_keystat(0xc8) == 1) boxSize.y -= 0.025;
		// arrow down
		if (voxie_keystat(0xd0) == 1) boxSize.y += 0.025;
		// arrow left
		if (voxie_keystat(0xcb) == 1) boxSize.x -= 0.025;
		// arrow right
		if (voxie_keystat(0xcd) == 1) boxSize.x += 0.025;




	} else {

		// arrow up
		if (voxie_keystat(0xc8) == 1) pos.y -= 0.025;
		// arrow down
		if (voxie_keystat(0xd0) == 1) pos.y += 0.025;
		// arrow left
		if (voxie_keystat(0xcb) == 1) pos.x -= 0.025;
		// arrow right
		if (voxie_keystat(0xcd) == 1) pos.x += 0.025;

	}

	// Right Shift
	if (voxie_keystat(0x36) == 1) pos.z -= 0.0125;
	// Right CTRL
	if (voxie_keystat(0x9d) == 1) pos.z += 0.0125;



	if (voxie_keystat(0x18) == 1) {
	ori++;
	if (ori > 3) ori = 0;	
	}

	// A
	if (voxie_keystat(0x1e) == 1) scale += 0.0125;
	// z
	if (voxie_keystat(0x2c) == 1) scale -= 0.0125;

	/**************************
	*   LOGIC                 *
	*                         *
	**************************/


	/**************************
	*   DRAW                  *
	*                         *
	**************************/

	tally(noOfTally,pos,boxSize,scale,ori);


	/* WACKY GFX */
/*
	vtext[0].col = 0; vtext[0].u = cos(tim); 	vtext[0].v = sin(tim);           
	vtext[1].col = 0; vtext[1].u = sin(tim); 	vtext[1].v = sin(tim);
	vtext[2].col = 0; vtext[2].u = sin(tim); 	vtext[2].v = cos(tim);
	vtext[3].col = 0; vtext[3].u = cos(tim); 	vtext[3].v = cos(tim);
	

	vtext[0].col = 0; vtext[0].u = 0; 	vtext[0].v = 0;           
	vtext[1].col = 0; vtext[1].u = 1; 	vtext[1].v = 0;
	vtext[2].col = 0; vtext[2].u = 1; 	vtext[2].v = 1;
	vtext[3].col = 0; vtext[3].u = 0; 	vtext[3].v = 1;


	mesh[0] = 0; mesh[1] = 1; mesh[2] = 2; mesh[3] = 3; mesh[4] = -1; 
*/
/*

// p1 ori
	vtext[0].x = -scale;	vtext[0].y = -scale;	vtext[0].z = 0;       
	vtext[1].x = scale; 	vtext[1].y = -scale; 	vtext[1].z = 0;
	vtext[2].x = scale;		vtext[2].y =  scale;	vtext[2].z = 0;
	vtext[3].x = -scale;	vtext[3].y =  scale; 	vtext[3].z = 0;


// p2 ori
	vtext[3].x = -scale;	vtext[3].y = -scale;	vtext[0].z = 0;       
	vtext[2].x = scale; 	vtext[2].y = -scale; 	vtext[1].z = 0;
	vtext[1].x = scale;		vtext[1].y =  scale;	vtext[2].z = 0;
	vtext[0].x = -scale;	vtext[0].y =  scale; 	vtext[3].z = 0;

// p3 ori
	vtext[0].y = -scale;	vtext[0].x = -scale;	vtext[0].z = 0;       
	vtext[1].y = scale; 	vtext[1].x = -scale; 	vtext[1].z = 0;
	vtext[2].y = scale;		vtext[2].x =  scale;	vtext[2].z = 0;
	vtext[3].y = -scale;	vtext[3].x =  scale; 	vtext[3].z = 0;

// p4 ori
	vtext[3].y = -scale;	vtext[3].x = -scale;	vtext[0].z = 0;       
	vtext[2].y = scale; 	vtext[2].x = -scale; 	vtext[1].z = 0;
	vtext[1].y = scale;		vtext[1].x =  scale;	vtext[2].z = 0;
	vtext[0].y = -scale;	vtext[0].x =  scale; 	vtext[3].z = 0;


	
	voxie_drawmeshtex(&vf,"Tally.png",vtext,5,mesh,6,2,0xFFFFFF);
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
			voxie_debug_print6x8_(30, 90, 0xffff00, -1, "Press 1 or 2 to adjust number to tally || Tally : %d", noOfTally); 
			voxie_debug_print6x8_(30, 110, 0xffff00, -1, "Arrow Keys, right CTRL and right SHIFT moves tally position POS.x %2.3f POS.y %2.3f POS.z %2.3f", pos.x, pos.y, pos.z); 
			voxie_debug_print6x8_(30, 130, 0xffff00, -1, "A and Z change Scale || Scale %2.3f", scale ); 
			voxie_debug_print6x8_(30, 150, 0xffff00, -1, "HOLD left CTRL and arrows to adjust boxsize || BoxSize X: %2.3f Y: %2.3f", boxSize.x, boxSize.y ); 
			voxie_debug_print6x8_(30, 170, 0xffff00, -1, "O to change orientation || Orientation %d", ori ); 
	
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
