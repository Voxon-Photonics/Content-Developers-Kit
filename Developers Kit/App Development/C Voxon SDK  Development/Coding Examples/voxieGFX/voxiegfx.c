// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1

	#Visual C makefile:
voxieGfx.exe: voxieGfx.c voxiebox.h; cl /TP voxieGfx.c /Ox /MT /link user32.lib
	del voxieGfx.obj

!else

	#GNU C makefile:
voxieGfx.exe: voxieGfx.c; gcc voxieGfx.c -o voxieGfx.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

	//VoxieGfx
	//Crudely written program i've used over time to learn various graphic and function calls for the Voxon hardware. 
	//You can use this as a development tool also - Matthew Vecchio 
	
#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif



static voxie_wind_t vw;

static int gcnti[2], gbstat = 0;
static void mymix (int *ibuf, int nsamps)
{
	static int cnt[2]; int i, c;
	for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)
			{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&gbstat; cnt[c] += gcnti[c]; }
}


static void axisrotate (point3d *p, point3d *ax, float w) //10/26/2011:optimized algo :)
{
	double f, c, s, ox, oy, oz;

	c = cos(w); s = sin(w);

		//P = cross(AX,P)*s + dot(AX,P)*(1-c)*AX + P*c;
	ox = p->x; oy = p->y; oz = p->z;
	f = (ox*ax->x + oy*ax->y + oz*ax->z)*(1.0-c);
	p->x = (ax->y*oz - ax->z*oy)*s + ax->x*f + ox*c;
	p->y = (ax->z*ox - ax->x*oz)*s + ax->y*f + oy*c;
	p->z = (ax->x*oy - ax->y*ox)*s + ax->z*f + oz*c;
}

// fade down or up colour
int tweenCol(int colour, float speed, int destColour) {

int b,g,r;
int bd,gd,rd;

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



if (r < 0x00) {
	r = 0x00;
}
if (r > 0xFF) {
	r = 0xFF;
}
if (g < 0x00) {
	g = 0x00;
}
if (g > 0xFF) {
	g = 0xFF;
}
if (b < 0x00) {
	b = 0x00;
}
if (b > 0xFF) {
	b = 0xFF;
}

return (r << 16) | (g << 8) | (b);

}

	//Rotate vectors by 90' WIP
static void rot90vex (int direction, point3d *rr, point3d *dd)
{

	point3d pp;

	if (direction == 1) {

		if (rr->x != 0) {

			pp.x = rr->x;
			pp.y = rr->y;
			pp.z = rr->z;
			
			rr->x = dd->x;
			rr->y = dd->y;
			rr->z = dd->z;

			dd->x = pp.x;
			dd->y = pp.y;
			dd->z = pp.z;

		return;
		}

		if (dd->x != 0) {

			pp.x = -dd->x;
			pp.y = -dd->y;
			pp.z = -dd->z;
			
			dd->x = -rr->x;
			dd->y = -rr->y;
			dd->z = -rr->z;

			rr->x = pp.x;
			rr->y = pp.y;
			rr->z = pp.z;

		return;
		}
	}


	return;
	

}

// flip Vertical or Horzional 
static void flipvex (point3d *a, point3d *b) {

point3d t;

t.x = a->x; a->x = b->x; b->x = t.x; 
t.y = a->y; a->y = b->y; b->y = t.y;
t.z = a->z; a->z = b->z; b->z = t.z;

}



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




#define SNAP_MAX 4
typedef struct { point3d pp, rr, dd, ff; int fillmode, r, g, b, fade; float rad, rad2; } snap_t;
static snap_t snap[SNAP_MAX];
static int snapNo = 0;

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim, avgdtim = 0.0, dlyTim, titleWaitTime;
	int i, mousx = 256, mousy = 256, mousz = 0, x, y, r = 0xFF, g = 0xFF, b = 0xFF, col;
	point3d ss, pp ={-0.5f,-0.5,-0.2f}, rr = {-0.1f,-0.1f,-0.1f}, dd = {0.5f,0.5f,0.2f}, ff = {0,0,0}, pos = {0.0f,0.0f,1.0f}, kk, inc = {0.3,0.2,0.1};
	point3d nn ={0,0,0};
	voxie_xbox_t vx[4];
	int ovxbut[4], vxnplays, numframes = 0;

	// define variables
	int buttonDelay = 0;
	static tiletype img = {0};
	static tiletype img2 = {0};
	int type = 2; // the type of object to draw
	int fillmode = 1; // the type of fill for object
	int rnd;
	int speed = 100;
	float rad = 0.1f, rad2 = 0.1f;
	bool markers = true;
	poltex_t vtext[20]; int mesh[20]; 
	char * charArray1 = "voxon.png";
	char * charArray2 = "usflag.png";
	float length = 50;
	float angle = 0.0;
	float angle_stepsize = 0.1;
	float f,hf,gf;




	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }

	if (vw.playsamprate)
	{
		gcnti[0] = (262<<21)/vw.playsamprate;
		gcnti[1] = (392<<21)/vw.playsamprate;
		voxie_setaudplaycb(mymix);
	}

	while (!voxie_breath(&in)) // Breath must mean as its updating loop
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz; // mouse variables
		gbstat = -(in.bstat != 0); // global button state
				for(vxnplays=0;vxnplays<4;vxnplays++) // set up xbox controllers
		{
			ovxbut[vxnplays] = vx[vxnplays].but;
		}
		// put controls here keys here
		if (voxie_keystat(0x1)) { voxie_quitloop(); }

if (buttonDelay < tim) {
	 buttonDelay = tim + 0.01;
			if (voxie_keystat(0x02) == 1) { type = 1; } // kb 1
			if (voxie_keystat(0x03) == 1) { type = 2; } // kb 2
			if (voxie_keystat(0x04) == 1) { type = 3; } // kb 3
			if (voxie_keystat(0x05) == 1) { type = 4; } // kb 4
			if (voxie_keystat(0x06) == 1) { type = 5; } // kb 5
			if (voxie_keystat(0x07) == 1) { type = 6; } // kb 6
			if (voxie_keystat(0x08) == 1) { type = 7; } // kb 7
			if (voxie_keystat(0x09) == 1) { type = 8; } // kb 8
 			if (voxie_keystat(0x0a) == 1) { type = 9; } // kb 9
			if (voxie_keystat(0x0b) == 1) { type = 10; ; 
			 
			 	rr.x = vw.aspx*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
	rr.y =         0.f; dd.y =vw.aspz*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
	rr.z =         0.f; dd.z =         0.f; ff.z =vw.aspz*-2.f; pp.z = (ff.z*-.5f) - 0.3;
			 } // kb 0
 			
		
 			if (voxie_keystat(0x0c) == 1) { speed -= 10; } // kb -
 			if (voxie_keystat(0x0d) == 1) { speed += 10; } // kb -
 			

 			if (voxie_keystat(0x10) == 1) { fillmode += 1;  } // kb q
 			if (voxie_keystat(0x1e) == 1) { fillmode -= 1;  } // kb a


 			// move color up red
 			if (voxie_keystat(0x11) && r <= 0xFE) { r += 1; } // kb w
 			if (voxie_keystat(0x1f) && r >= 0x01) { r -= 1; } // kb s

 			// move color up green
 			if (voxie_keystat(0x12) && g <= 0xFE) { g += 1; } // kb e
 			if (voxie_keystat(0x20) && g >= 0x01) { g -= 1;  } // kb d

 			// move color up blue
 			if (voxie_keystat(0x13) && b <= 0xFE) { b += 1;  } // kb r
 			if (voxie_keystat(0x21) && b >= 0x01) { b -= 1;  } // kb f

 			if (r > 0xff) r = 0xff; 		if (r < 0x00) r = 0x00;
 			if (g > 0xff) g = 0xff;  		if (g < 0x00) g = 0x00;
 			if (b > 0xff) b = 0xff;  		if (b < 0x00) b = 0x00;
			if (fillmode < 0) fillmode = 0; 			

 			// increase radius
 			if (voxie_keystat(0x17) ) { rad += 0.1f * dtim; } // kb i
 			if (voxie_keystat(0x25) ) { rad -= 0.1f * dtim; } // kb k

 			 			// increase radius
 			if (voxie_keystat(0x18) ) { rad2 += 0.1f * dtim;  } // kb o
 			if (voxie_keystat(0x26) ) { rad2 -= 0.1f * dtim;  } // kb l

 			if (voxie_keystat(0x2d) ) { // ky x // setup for text view
 										// dd.y = text height
 										// rr.x = text seperation
						r = 0xff; g = 0xff; b = 0xff;

 						pp.x = -0.780f; rr.x = 0.1f; dd.x = 0.020f; 
						pp.y = -0.910f;	rr.y = 0.0f; dd.y = 0.200f;
						pp.z = 0.270f; 	rr.z = 0.0f; dd.z = 0.0f;
						type = 7;

 			}
 			if (voxie_keystat(0x2e)) { // ky c setup for sprite view
					  fillmode = 1;
 						pp.x = 0.f; 	rr.x = -1.0f; 	dd.x = 0.000f;	ff.x = 0.0f;
						pp.y = 0.f;		rr.y = 0.0f; 	dd.y = 1.000f;	ff.y = 0.0f;
						pp.z = 0.100f; 	rr.z = 0.0f; 	dd.z = 0.000f;	ff.z = 1.0f;
						r = 40; g = 40; b = 40;
						type = 9;
 			}

 			if (voxie_keystat(0x2f)) { // ky v setup for default view

 						pp.x = -0.5f; 	rr.x = -0.1f; 	dd.x = 0.5f;	ff.x = 0.0f;
						pp.y = -0.5f;	rr.y = -0.1f; 	dd.y = 0.5f;	ff.y = 0.0f;
						pp.z = -0.2f; 	rr.z = -0.1f; 	dd.z = 0.2f;	ff.z = 1.0f;
						
 			}

				if (voxie_keystat(0x30)) { // ky b setup for bitmap view
				type = 1;
				fillmode = 6;
				col = 0x404040;

 						pp.x = -0.4f; 	rr.x = 0.4f; 	  dd.x = -0.4f;	ff.x = 0.4f;
						pp.y = -0.4f;	  rr.y = -0.4f; 	dd.y = 0.4f;	ff.y = 0.4f;
						pp.z = 0; 		rr.z = 0; 		dd.z = 0;	  	ff.z = 0;
						
 			}
 			if (voxie_keystat(0x32) == 1) {// toggle markers M 

 				if (markers == true) {
 					markers = false;
 				
 				} else {
 					markers = true;
 			
 				}
				 }

				if (voxie_keystat(0x19) == 1) {// create snap
			
					
						snap[snapNo].pp.x = pp.x;
						snap[snapNo].pp.y = pp.y;
						snap[snapNo].pp.z = pp.z;

						snap[snapNo].rr.x = rr.x;
						snap[snapNo].rr.y = rr.y;
						snap[snapNo].rr.z = rr.z;

						snap[snapNo].dd.x = dd.x;
						snap[snapNo].dd.y = dd.y;
						snap[snapNo].dd.z = dd.z;

						snap[snapNo].ff.x = ff.x;
						snap[snapNo].ff.y = ff.y;
						snap[snapNo].ff.z = ff.z;
						 
						snap[snapNo].fillmode = fillmode;
						snap[snapNo].r = r;
						snap[snapNo].g = g;
						snap[snapNo].b = b;
						snap[snapNo].rad = rad;
						snap[snapNo].rad2 = rad2;
						
						
						snapNo++;
						if (snapNo > 3) {
							snapNo = 0;
						}
					}

						if (voxie_keystat(0x27) == 1) {// load snap

				

					
						pp.x = snap[snapNo].pp.x;
						pp.y = snap[snapNo].pp.y;
						pp.z = snap[snapNo].pp.z;


						rr.x = snap[snapNo].rr.x;
						rr.y = snap[snapNo].rr.y;
						rr.z = snap[snapNo].rr.z;

						dd.x = snap[snapNo].dd.x; 
						dd.y = snap[snapNo].dd.y; 
						dd.z = snap[snapNo].dd.z;

						ff.x = snap[snapNo].ff.x; 
						ff.y = snap[snapNo].ff.y; 
						ff.z = snap[snapNo].ff.z; 
						 
					 // fillmode = 	snap[snapNo].fillmode;
						r = snap[snapNo].r;
						g = snap[snapNo].g;
						b = snap[snapNo].b;
						rad   = snap[snapNo].rad;
						rad2 = snap[snapNo].rad2;
						
						
						snapNo++;
						if (snapNo > 3) {
							snapNo = 0;
						}
					}




if (voxie_keystat(0x38)) { // if ALT is down move dd

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { dd.x += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { dd.x -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb g

 			if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { dd.y += 0.005f * speed * dtim;  buttonDelay = tim + 0.2; } // kb y
			if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { dd.y -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb h

			if (voxie_keystat(0x16) || voxie_keystat(0x4e) || voxie_keystat(0x36)  ) { dd.z -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb u
			if (voxie_keystat(0x24) || voxie_keystat(0x4a) || voxie_keystat(0x9d) ) { dd.z += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb j

			if (voxie_keystat(0x2c) ) { dd.x = 0.f; dd.y = 0.0f; dd.z = 0.0f; } // ky z reset to zero


} else if (voxie_keystat(0x0f)) { // if tab is down move rr

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { kk.x += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { kk.x -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb g

 			if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { kk.y += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb y
			if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { kk.y -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb h

			if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { kk.z -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb u
			if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { kk.z += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb j

			if (voxie_keystat(0x2c) ) { kk.x = 0.f; kk.y = 0.0f; kk.z = 0.0f; } // kb z reset to zero

} else if (voxie_keystat(0x39)) { // if space is down rotate
																	// RR and DD are X axis

if (type != 1 && fillmode != 6) {

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { rotvex(rad,&rr,&dd);   } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { rotvex(-rad,&rr,&dd); } // kb g
																	// DD and FF are Y Axis
 		  	if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { rotvex(rad,&dd,&ff);  } // kb y
     		if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { rotvex(-rad,&dd,&ff); } // kb h
																	// FF and RR are diagonial Axis
     		if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { rotvex(rad,&ff,&rr);  } // kb u
     		if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { rotvex(-rad,&ff,&rr); } // kb j

				//TODO :finish this function
      	if (voxie_keystat(0x33) == 1) { rot90vex(1,&rr, &dd);   }
				if (voxie_keystat(0x34) == 1) { rot90vex(-1,&rr, &dd);   }

} 



}else if (voxie_keystat(0xb8)) { // if LEFT alt is down axis rotate
																	// RR and DD are X axis

if (type != 1 && fillmode != 6) {

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { axisrotate(&rr,&nn,rad);  } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { axisrotate(&pp,&nn,rad); } // kb g
																	// DD and FF are Y Axis
 		/*if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { axisrotate(rad,&dd,&ff);  } // kb y
     		if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { axisrotate(-rad,&dd,&ff); } // kb h
																	// FF and RR are diagonial Axis
     		if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { axisrotate(rad,&ff,&rr);  } // kb u
     		if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { axisrotate(-rad,&ff,&rr); } // kb j */
}
     	

}else if (voxie_keystat(0x2a)) { // if L Shift  is down move rr

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { rr.x += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { rr.x -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb g

 			if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { rr.y += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb y
     		if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { rr.y -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb h

     		if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { rr.z -= 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb u
     		if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { rr.z += 0.005f * speed * dtim; buttonDelay = tim + 0.2; } // kb j

     			if (voxie_keystat(0x2c) == 1) { rr.x = 0.f; rr.y = 0.0f; rr.z = 0.0f; } // ky z reset to zero

} else if (voxie_keystat(0x1d)) { // if L ctrl  is down move ff

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { ff.x += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { ff.x -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb g

 			if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { ff.y += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb y
     	if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { ff.y -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb h

     		if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { ff.z -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb u
     		if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { ff.z += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb j

     			if (voxie_keystat(0x2c) == 1) { ff.x = 0.f; ff.y = 0.0f; ff.z = 0.0f; } // ky z reset to zero

} else {
 			// move pp.x (top-ley-up corner)
 			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { pp.x += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb t
 			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { pp.x -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb g

 			if (voxie_keystat(0x15) || voxie_keystat(0xd0) ) { pp.y += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb y
     		if (voxie_keystat(0x23) || voxie_keystat(0xc8) ) { pp.y -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb h

     		if (voxie_keystat(0x16) || voxie_keystat(0x4e) ) { pp.z -= 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb u
     		if (voxie_keystat(0x24) || voxie_keystat(0x4a) ) { pp.z += 0.005f * speed * dtim; buttonDelay = tim + 0.02; } // kb j

     			if (voxie_keystat(0x2c) == 1 ) { pp.x = 0.f; pp.y = 0.0f; pp.z = 0.0f; } // ky z reset to zero
}
 			}


 
 





		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);
		if (i)
		{
				  if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

#if 0

		




#endif

			//draw wireframe box
		col=(r<<16)|(g<<8)|(b);
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		

#if 1
		// put stuff here


	switch (type) {

		case 10: // heightmap

	


//void voxie_drawheimap (voxie_frame_t *vf, char *filnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags)
//voxie_drawheimap(&vf, "map.png", pp, rr, dd, ff,0x12345678,-1,100 );
		voxie_drawheimap(&vf,"map.png", &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2));
		break;

		case 1:
	//poltex_t vtext[20]; int mesh[20]; 



		switch (fillmode) {
			case 0:
i = 0;




	  vtext[0].x = pp.x; vtext[0].y = pp.y; vtext[0].z = pp.z; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
      vtext[1].x = rr.x; vtext[1].y = rr.y; vtext[1].z = rr.z; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
      vtext[2].x = ff.x; vtext[2].y = ff.y; vtext[2].z = ff.z; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].x = dd.x; vtext[3].y = dd.y; vtext[3].z = dd.z; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
      vtext[4].x = kk.x; vtext[4].y = kk.y; vtext[4].z = kk.z; vtext[4].col = 0; vtext[4].u = 0; vtext[4].v = 0;
	  
	  mesh[0] = 4; mesh[1] = 3; mesh[2] = 2; mesh[3] =1 ; mesh[4] = 0; mesh[5] = -1;

	

	   // voxie_drawmeshtex(&vf,0,vtext,4,mesh,i,2,0x404040); // for custom colour

		voxie_drawmeshtex(&vf,"voxon.png",vtext,5,mesh,6,2,col); // for texture 0x404040 for default
		break;


		
			case 1:
      pol_t pt2[6];
      pt2[0].x = -0.2; pt2[0].y = 0.2; pt2[0].z = 0.0f; pt2[0].p2 = 1;
      pt2[1].x = 0.5; pt2[1].y = 0.0; pt2[1].z = 0.0f; pt2[1].p2 = 2;
      pt2[2].x = 0.5; pt2[2].y = 0.8; pt2[2].z = 0.0f; pt2[2].p2 = 3;
      pt2[3].x = 0.0; pt2[3].y = 0.8; pt2[3].z = 0.0f; pt2[3].p2 = 4;
      pt2[4].x = 0.0; pt2[4].y = 0.8; pt2[4].z = 0.5f; pt2[4].p2 = 5;
      pt2[5].x = 0.5; pt2[5].y = -0.8; pt2[5].z = 0.0f; pt2[5].p2 = 0;
      	  
    
     voxie_drawpol(&vf,pt2,6, col); 

			break;
			case 2:
		//For fillmode 0, mesh and meshn are ignored - pass 0's in their place.
	   poltex_t vt[4]; i = 0;
	   vt[0].x =-0.8; vt[0].y =-0.8; vt[0].z = 0.0; vt[0].col = col;
	   vt[1].x =+0.8; vt[1].y =+0.8; vt[1].z = 0.0; vt[1].col = col;
	   vt[2].x =-0.8; vt[2].y =+0.8; vt[2].z = 0.0; vt[2].col = col;
	   vt[3].x =+0.8; vt[3].y =-0.8; vt[3].z = 0.0; vt[3].col = col;
	   voxie_drawmeshtex(&vf,0,vt,4,0,0,0,col);
			break;
					case 3:
      pol_t pt1[4];
      pt1[0].x = 0.0; pt1[0].y = 0.0; pt1[0].z = 0.0f; pt1[0].p2 = 1;
      pt1[1].x = 0.5; pt1[1].y = 0.0; pt1[1].z = 0.0f; pt1[1].p2 = 2;
      pt1[2].x = 0.5; pt1[2].y = 0.8; pt1[2].z = 0.0f; pt1[2].p2 = 3;
      pt1[3].x = 0.0; pt1[3].y = 0.8; pt1[3].z = 0.0f; pt1[3].p2 = 0;
     voxie_drawpol(&vf,pt1,4, col); 

			break;
		case 4:
		// draw texture map shape

		//texture-mapped tetrahedron..
		//poltex_t vtext[20]; int mesh[20]; 

	  i = 0;


		// flat flag
	  vtext[0].x = -0.8; vtext[0].y = -0.5; vtext[0].z = 0.0f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
      vtext[1].x = 0.8; vtext[1].y = -0.5; vtext[1].z = 0.0f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
      vtext[2].x = 0.8; vtext[2].y = 0.5; vtext[2].z = 0.0f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
      vtext[3].x = -0.8; vtext[3].y = 0.5; vtext[3].z = 0.0f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

	

	   // voxie_drawmeshtex(&vf,0,vtext,4,mesh,i,2,0x404040); // for custom colour

		voxie_drawmeshtex(&vf,"voxon.png",vtext,4,mesh,5,2,col); // for texture

		break;
			case 5:
		// draw texture map shape

		i = 0;
	  // how tall
	  vtext[0].x = 0.0; vtext[0].y = 0.5; vtext[0].z = -0.35f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
    vtext[1].x = 0.8; vtext[1].y = 0.5; vtext[1].z = -0.35f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
    vtext[2].x = 0.8; vtext[2].y = 0.5; vtext[2].z = 0.35f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
    vtext[3].x = 0.0; vtext[3].y = 0.5; vtext[3].z = 0.35f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;


	
		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

	   // voxie_drawmeshtex(&vf,0,vtext,4,mesh,i,2,0x404040); // for custom colour
		voxie_drawmeshtex(&vf,"voxon.png",vtext,4,mesh,5,2,0x404040); // for texture

		break;
		case 6:
		// draw texture map shape
		i = 0;

		// marker rotate

	if (voxie_keystat(0x39)) { // if space is down rotate
		if (type == 1 && fillmode == 6) {

			if (voxie_keystat(0x14) || voxie_keystat(0xcb) ) { rotvex(rad,&dd,&ff); rotvex(rad,&rr,&pp);   } // kb t
			if (voxie_keystat(0x22) || voxie_keystat(0xcd) ) { rotvex(-rad,&dd,&ff); rotvex(-rad,&rr,&pp); } // kb g

			if (voxie_keystat(0x15) == 1 || voxie_keystat(0xd0) == 1 ) { flipvex(&pp,&ff);  flipvex(&dd,&rr);  } // flip up and down
     	if (voxie_keystat(0x23) == 1 || voxie_keystat(0xc8) == 1 ) { flipvex(&pp,&rr);  flipvex(&dd,&ff);  } // flip
		
		/* TODO finish this function
			if (voxie_keystat(0x16) == 1 || voxie_keystat(0x4e) == 1 ) { rot90vex(1,&dd,&ff); rot90vex(1,&rr,&pp);   } // kb t
			if (voxie_keystat(0x24) == 1 || voxie_keystat(0x4a) == 1 ) {  } // kb g
		*/

	}


}


		vtext[0].x = pp.x; vtext[0].y = pp.y; vtext[0].z = pp.z; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
		vtext[1].x = rr.x; vtext[1].y = rr.y; vtext[1].z = rr.z; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
		vtext[2].x = ff.x; vtext[2].y = ff.y; vtext[2].z = ff.z; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
		vtext[3].x = dd.x; vtext[3].y = dd.y; vtext[3].z = dd.z; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		voxie_drawmeshtex(&vf,"voxon.png",vtext,4,mesh,5,2,col); // for texture

		break;
		case 7:
		// draw texture map shape

		i = 0;

	  vtext[0].x = pp.x; vtext[0].y = pp.y; vtext[0].z = pp.z; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
		vtext[1].x = rr.x; vtext[1].y = rr.y; vtext[1].z = rr.z; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
		vtext[2].x = ff.x; vtext[2].y = ff.y; vtext[2].z = ff.z; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
		vtext[3].x = dd.x; vtext[3].y = dd.y; vtext[3].z = dd.z; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
	
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

		voxie_drawmeshtex(&vf,"usflag.png",vtext,4,mesh,5,2,col); // for texture 0x404040 for default
		break;
case 8:
		// draw texture map shape
		i = 0;

		vtext[0].x = -0.8; vtext[0].y = -0.5; vtext[0].z = 0.0f; vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
		vtext[1].x = 0.8; vtext[1].y = -0.5; vtext[1].z = 0.0f; vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
		vtext[2].x = 0.8; vtext[2].y = 0.5; vtext[2].z = 0.0f; vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
		vtext[3].x = -0.8; vtext[3].y = 0.5; vtext[3].z = 0.0f; vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
		
	  mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;
		voxie_drawmeshtex(&vf,charArray1,vtext,4,mesh,5,2,col); // for texture 0x404040 for default

if (voxie_keystat(0x0f)) { charArray1 = "usflag.png"; } 
else { charArray1 = "voxon.png"; }
		break;
				}
	
	


		break;
		case 2:
		//Draw filled rectangle.
		//  x0,y0,z0: top/left/up corner
		//  x1,y1,z1: bot/right/down corner
		//  fillmode: 0:dots, 1:lines, 2:surfaces, 3:solid
		//       col: 24-bit color
		//void voxie_drawbox (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1,
		//				  int fillmode, int col);
		
		voxie_drawbox(&vf, pp.x, pp.y, pp.z, dd.x, dd.y,dd.z, fillmode, col);		


		break;
		case 3:
	//Draw single voxel at specified location.
		switch(fillmode) {
			default:
			case 0: // draw a single vox
				voxie_drawvox (&vf, pp.x, pp.y, pp.z, col);
			break;
			case 1: // draw a cricle
				length = rad;
				angle = 0.0;
				// rad2 is resolution use max and min to clip data
				rad2 = max(rad2,.0125);
				rad2 = min(rad2,1);

				// go through all angles from 0 to 2 * PI radians
				while (angle < 2 * PI)
				{
					// calculate x, y from a vector with known length and angle
					pos.x = pp.x + length * cos (angle);
					pos.y = pp.y + length * sin (angle);
					pos.z = pp.z;
					voxie_drawvox (&vf, pos.x, pos.y, pos.z, col);
					angle += (rad2) ;
				}
			break;
			case 2: // draw a cricle tall
				length = rad;
				angle = 0.0;
				// rad2 is resolution use max and min to clip data
				rad2 = max(rad2,.0125);	
				rad2 = min(rad2,1);
				// go through all angles from 0 to 2 * PI radians
				while (angle < 2 * PI)
				{
					// calculate x, y from a vector with known length and angle
					pos.x = pp.x + length * cos (angle);
					pos.z = pp.z + length * sin (angle);
					pos.y = pp.y;
					voxie_drawvox (&vf, pos.x, pos.y, pos.z, col);
					angle += rad2;
				}
			break;
			case 3: // draw grid of voxels
				// rad2 = res
				pos.x = -vw.aspx;
				pos.y = -vw.aspz;
				pos.z = 0;  

				// rad2 is resolution;
				rad2 = max(rad2,vw.aspx / 100);	
				rad2 = min(rad2,vw.aspx / 2);

				for (pos.x = -vw.aspx; pos.x < vw.aspx; pos.x += rad2 ) 
					for (pos.y = -vw.aspy; pos.y < vw.aspy; pos.y += rad2 )  
						for (pos.z = -vw.aspz; pos.z < vw.aspz; pos.z += rad2  ) {
					voxie_drawvox(&vf, pos.x, pos.y, pos.z, col);
				}
				break;
		case 4: // with dither
				
				pos.x = -vw.aspx;
				pos.y = -vw.aspz;
				pos.z = 0;  

				//		col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change	

				f = fabs((float)(((rand() & 32767) - 16384) / 16384.f) *.01);
				gf = fabs((float)(((rand() & 32767) - 16384) / 16384.f) *.01);
				hf = fabs((float)(((rand() & 32767) - 16384) / 16384.f) *.01);
				// rad2 is resolution;
				rad2 = max(rad2,vw.aspx / 100);	
				rad2 = min(rad2,vw.aspx / 2);
				
				for (pos.x = -vw.aspx; pos.x < vw.aspx; pos.x += rad2 ) {
					 
					for (pos.y = -vw.aspy; pos.y < vw.aspy; pos.y += rad2 ) {
				
						for (pos.z = -vw.aspz; pos.z < vw.aspz; pos.z += rad2  ) {
						f = fabs((float)(((rand() & 32767) - 16384) / 16384.f) *.01);	
			
					voxie_drawvox(&vf, pos.x + f, pos.y + gf, pos.z + hf, col);
				}
					}
				}
		break;
		case 5: // dome example from kens
		if (!img.f) { img.x = 512; img.y = 512; img.p = (INT_PTR)img.x*4; img.f = (INT_PTR)malloc(img.p*img.y); }

		for(y=0;y<img.y;y++)
			for(x=0;x<img.x;x++)
			{
				int rgb24 = (128<<16)+(y<<8)+x, hgt8 = sqrt(256.f*256.f - (x-256)*(x-256) - (y-256)*(y-256));
				*(int *)(img.p*y + (x<<2) + img.f) = rgb24 + (hgt8<<24);
			}


		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
		voxie_drawheimap(&vf,(char *)&img, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);

		break;
		case 6: // dome example from kens
		if (!img2.f) { img2.x = 256; img2.y = 256; img2.p = (INT_PTR)img2.x*4; img2.f = (INT_PTR)malloc(img2.p*img2.y); }

		for(y=0;y<img2.y;y++)
			for(x=0;x<img2.x;x++)
			{
				int rgb24 = (128<<16)+(y<<8)+x; // the colour
				int hgt8 = x *y / vw.aspz ; // the height map
				*(int *)(img2.p*y + (x<<2) + img2.f) = rgb24 + (hgt8<<24);
			}

	
		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
	
		voxie_drawheimap(&vf,(char *)&img2, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);

		break;
		case 7: // dome example from kens
		if (!img2.f) { img2.x = 256; img2.y = 256; img2.p = (INT_PTR)img2.x*4; img2.f = (INT_PTR)malloc(img2.p*img2.y); }

		for(y=0;y<img2.y;y++)
			for(x=0;x<img2.x;x++)
			{
				int rgb24 = (128<<16)+(x<<8)+x; // the colour
				int hgt8 = rad2 * x * y ; // the height map
				*(int *)(img2.p*y + (x<<2) + img2.f) = rgb24 + (hgt8<<24);
			}

	
		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
	
		voxie_drawheimap(&vf,(char *)&img2, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);

		break;
		case 8: // dome example from kens
		if (!img2.f) { img2.x = 256; img2.y = 256; img2.p = (INT_PTR)img2.x*4; img2.f = (INT_PTR)malloc(img2.p*img2.y); }

		for(y=0;y<img2.y;y++)
			for(x=0;x<img2.x;x++)
			{
				int rgb24 = (col + x << 16) + (col + x<<8)+(x); // the colour
				int hgt8 = 0; // the height map
				*(int *)(img2.p*y + (x<<2) + img2.f) = rgb24;
			}

	
		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
	
		voxie_drawheimap(&vf,(char *)&img2, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/ + (1<<3)/*filnam is tiletype*/);

		break;
	}
		break;
		case 4:
	//Draw line
	//  x0,y0,z0: 1st point
	//  x1,y1,z1: 2nd point
	//       col: 24-bit color
		// void voxie_drawlin (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1,
		//				  int col);
		
voxie_drawlin (&vf, pp.x, pp.y, pp.z, dd.x, dd.y, dd.z,
						  col);
		break;
		case 5:
	//Draw sphere
	//  fx,fy,fz: center location
	//       rad: radius
	//     issol: 0=shell, 1=solid filled
	//       col: 24-bit color
//void voxie_drawsph (voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col);
		voxie_drawsph(&vf, pp.x, pp.y, pp.z, rad, fillmode, col);
		break;
		case 6:
	//Draw cone with rounded ends (also draws cylinder/sphere)
	//  x0,y0,z0,r0: sphere 0 center&radius
	//  x1,y1,z1,r1: sphere 1 center&radius
	//  fillmode: 0=shell, 1=solid filled (always solid filled currently)
	//       col: 24-bit color
//void voxie_drawcone (voxie_frame_t *vf, float x0, float y0, float z0, float r0,
//													 float x1, float y1, float z1, float r1,
//													 int fillmode, int col);

		voxie_drawcone(&vf, pp.x, pp.y, pp.z, rad, dd.x, dd.y, dd.z, rad2, fillmode, col);
		break;
		case 7:
	//Draw line-based 'skeleton' text. Supports any orientation.
	//   p: top-left-up corner or first character
	//   r: right vector - length is size of single character
	//   d: down vector - length is height of character
	// col: 24-bit color
	//  st: null-terminated ASCII byte string.
	//void voxie_printalph (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col,
	//						 const char *st);
	voxie_printalph_(&vf,&pp,&rr,&dd,col,"text %d", rad );
		break;
		case 8:
	//Draw cube using specified vectors. Parallelepipeds supported - vectors need not be orthogonal.
	//       p: top-left-up corner
	//       r: right vector
	//       d: down vector
	//       f: forward vector
	//fillmode: 0:dots, 1:edges/wireframe, 2:surfaces, 3:solid filled
	//     col: 24-bit color
	//void voxie_drawcube (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f,
	//							int fillmode, int col);
		voxie_drawcube(&vf,&pp, &rr, &dd, &ff, fillmode, col);		

		break;
		case 9:
			//Draw sprite (voxel model).
	//      fnam: filename. Cached internally. Currently supported:KV6,STL. May be supported: KVX,KVS
	//         p: position of center of model (pivot)
	//     r,d,f: right, down, forward vectors (respectively)
	//forcescale: for STL files, forces scale factor (for alignment inside ZIP animations) Leave this
	//            0.f typically.
	//       col: color multiplier. 24-bit color, each 8 bits scales intensity of respective
	//            component. 64=1.0 or no scale. Use 0x404040 for no change; 0x808080 to draw as
	//            double brightness, etc.. (currently ignored)
	//   returns: 1=found file&valid, 0=bad file
//int voxie_drawspr (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d,
//						 point3d *f, float forcescale, int col);

		switch(fillmode) {
			case 0:
			default:
			voxie_drawspr(&vf,"zedlogoobj.obj",&pp,&rr,&dd,&ff,col);
			break;
			case 1:
			voxie_drawspr(&vf,"invader1_anim0.obj",&pp,&rr,&dd,&ff,col);
			break;
			case 2:
			voxie_drawspr(&vf,"chopper.kv6",&pp,&rr,&dd,&ff,col);
			break;
			case 3:
			voxie_drawspr(&vf,"caco.kv6",&pp,&rr,&dd,&ff,col);

		}
		break;
	}
	


#else


#endif
		// final update loop for frame
			avgdtim += (dtim-avgdtim)*.1;
		if (markers == true )
		{
			point3d pp2, rr2, dd2;
			voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
			rr2.x = 0.1f; dd2.x = 0.00f; pp2.x = +vw.aspx*-0.19f;
			rr2.y = 0.00f;  dd2.y = 0.150f; pp2.y = vw.aspy*+.86f;
			rr2.z = 0.00f;  dd2.z = 0.00f; pp2.z = vw.aspz-0.01f;
			voxie_printalph_(&vf,&pp2,&rr2,&dd2,0xffffff,"BOTTOM RIGHT",1.0/avgdtim);

			pp2.x = vw.aspx*-.99f;
			pp2.y = vw.aspy*-.99f;
			pp2.z *= -1.f;
			voxie_printalph_(&vf,&pp2,&rr2,&dd2,0xffffff,"TOP LEFT %.1f",1.0/avgdtim);


			dd2.y = 0.08f;
			rr2.x = 0.04f;
			voxie_printalph_(&vf,&pp,&rr2,&dd2,0x00ff00,"  P");
			voxie_printalph_(&vf,&rr,&rr2,&dd2,0xff0000,"  R");
			voxie_printalph_(&vf,&dd,&rr2,&dd2,0x0000ff,"  D");
			voxie_printalph_(&vf,&ff,&rr2,&dd2,0xff00ff,"  F");
			//voxie_printalph_(&vf,&kk,&rr2,&dd2,0xffff00,"  K");
			// draw markers pointers for points
			voxie_drawsph(&vf, pp.x, pp.y, pp.z, 0.05, 1, 0x00FF00);
			voxie_drawsph(&vf, rr.x, rr.y, rr.z, 0.05, 1, 0xFF0000);
			voxie_drawsph(&vf, dd.x, dd.y, dd.z, 0.05, 1, 0x0000FF);
			voxie_drawsph(&vf, ff.x, ff.y, ff.z, 0.05, 1, 0xFF00FF); 
			//voxie_drawsph(&vf, kk.x, kk.y, kk.z, 0.05, 1, 0xFFFF00); 
			

			
		}
		// debug info
		x = 64; y = 128;
		voxie_debug_print6x8_(x-30,y-70,0xff0000,-1,"Voxie Rot : a tool to understand drawing graphics in VoxieBox 16th of Oct 2019 - ReadyWolf / Matthew Vecchio VPS :%5.1f",1.0/avgdtim);
		voxie_debug_print6x8_(x-30,y-60,0xffc080,-1,"1 = draw poly, 2 = draw box, 3 = draw vox, 4 = draw line, 5 draw sphere, 6 draw cone, 7 draw text, 8 draw cube 9 draw sprite 0 heightmap");	
		voxie_debug_print6x8_(x-30,y-50,0xffc080,-1,"Q = fill up, A = fill down  Fill mode = %d",fillmode);
		voxie_debug_print6x8_(x-30,y-40,0xff4040,-1,"W = more Red, S = less Red %x",r);
		voxie_debug_print6x8_(x-30,y-30,0x40ff40,-1,"E = more Green, D = less Green %x",g);
		voxie_debug_print6x8_(x-30,y-20,0x4040ff,-1,"R = more Blue, F = less Blue %x",b);

				voxie_debug_print6x8_(x+250,y,col,-1,"*** Selected Colour %x ***",col);

		if (voxie_keystat(0x38)) { // Left Alt
		voxie_debug_print6x8_(x-30,y-10,0x0000ff,-1,"T = dd.x up, G = dd.x down %3.3f",dd.x);
		voxie_debug_print6x8_(x-30,y,		0x0000ff,-1,"Y = dd.y up, H = dd.y down %3.3f",dd.y);
		voxie_debug_print6x8_(x-30,y+10,0x0000ff,-1,"U = dd.z down, J = dd.z up %3.3f",dd.z);
		} else  if (voxie_keystat(0x2a)) {  // Left Shift 
		voxie_debug_print6x8_(x-30,y-10,0xff0000,-1,"T = rr.x up, G = rr.x down %3.3f",rr.x);
		voxie_debug_print6x8_(x-30,y,		0xff0000,-1,"Y = rr.y up, H = rr.y down %3.3f",rr.y);
		voxie_debug_print6x8_(x-30,y+10,0xff0000,-1,"U = rr.z down, J = rr.z up %3.3f",rr.z);

		} else  if (voxie_keystat(0x0f)) {  // TAB
		voxie_debug_print6x8_(x-30,y-10,	0xffff00,-1,"T = kk.x up, G = kk.x down %3.3f",kk.x);
		voxie_debug_print6x8_(x-30,y,			0xffff00,-1,"Y = kk.y up, H = kk.y down %3.3f",kk.y);
		voxie_debug_print6x8_(x-30,y+10,	0xffff00,-1,"U = kk.z down, J = kk.z up %3.3f",kk.z);

		}else if (voxie_keystat(0x1d)) { // if L ctrl  is down move ff
		voxie_debug_print6x8_(x-30,y-10,0xff00ff,-1,"T = ff.x up, G = ff.x down %3.3f",ff.x);
		voxie_debug_print6x8_(x-30,y,   0xff00ff,-1,"Y = ff.y up, H = ff.y down %3.3f",ff.y);
		voxie_debug_print6x8_(x-30,y+10,0xff00ff,-1,"U = ff.z down, J = ff.z up %3.3f",ff.z);

		} else  {
		voxie_debug_print6x8_(x-30,y-10,0x00ff00,-1,"T = pp.x up, G = pp.x down %3.3f",pp.x);
		voxie_debug_print6x8_(x-30,y,		0x00ff00,-1,"Y = pp.y up, H = pp.y down %3.3f",pp.y);
		voxie_debug_print6x8_(x-30,y+10,0x00ff00,-1,"U = pp.z down, J = pp.z up %3.3f",pp.z);

		}

		voxie_debug_print6x8_(x-30,y+20,0xffc080,-1,"I = radius 1 up, K = radius 1 down %3.3f", rad);
		voxie_debug_print6x8_(x-30,y+30,0xffc080,-1,"O = radius 2 up, L = radius 2 down %3.3f", rad2);
			
		voxie_debug_print6x8_(x-30,y+40,0xffc080,-1,"- = speed down, = = speed up %d", speed);
		voxie_debug_print6x8_(x-30,y+50,0xffc080,-1,"Hold L Alt = Dd point3d, L Shift = rr point3d, L Ctrl = ff point3d");
		voxie_debug_print6x8_(x-30,y+60,0xffffff,-1,"Reset Point3der to.... Z = zero, X = Text mode, C = Sprite Mode V = Default B = Bitmap");
		voxie_debug_print6x8_(x-30,y+70,0xffc080,-1,"M = toggle Markers P = Take Snap Shot : = Load Snap Shot");
		
		voxie_debug_print6x8_(x-30,y+80,0xffc080,-1,"Hold Spacebar for Rotation change speed with radius 1");


		voxie_debug_print6x8_(x	- 30,y + 95, 0xffffff,-1,"Current Dimentions Hold L Alt, L Shift or L Ctrl to change 3dPoint");
		voxie_debug_print6x8_(x - 30,y + 110,0xffffff,-1,"pp.x: %5.3f, pp.y: %5.3f, pp.z: %5.3f", pp.x, pp.y, pp.z);
		voxie_debug_print6x8_(x - 30,y + 120,0xffffff,-1,"dd.x: %5.3f, dd.y: %5.3f, dd.z: %5.3f", dd.x, dd.y, dd.z);
		voxie_debug_print6x8_(x - 30,y + 130,0xffffff,-1,"rr.x: %5.3f, rr.y: %5.3f, rr.z: %5.3f", rr.x, rr.y, rr.z);
		voxie_debug_print6x8_(x - 30,y + 140,0xffffff,-1,"ff.x: %5.3f, ff.y: %5.3f, ff.z: %5.3f", ff.x, ff.y, ff.z);
		
	
		voxie_debug_print6x8_(x - 30,y + 200,0x00ff00,-1,"pp.x: %5.3f, pp.y: %5.3f, pp.z: %5.3f", snap[0].pp.x, snap[0].pp.y, snap[0].pp.z);
		voxie_debug_print6x8_(x - 30,y + 210,0x00ff00,-1,"dd.x: %5.3f, dd.y: %5.3f, dd.z: %5.3f", snap[0].dd.x, snap[0].dd.y, snap[0].dd.z);
		voxie_debug_print6x8_(x - 30,y + 220,0x00ff00,-1,"rr.x: %5.3f, rr.y: %5.3f, rr.z: %5.3f", snap[0].rr.x, snap[0].rr.y, snap[0].rr.z);
		voxie_debug_print6x8_(x - 30,y + 230,0x00ff00,-1,"ff.x: %5.3f, ff.y: %5.3f, ff.z: %5.3f", snap[0].ff.x, snap[0].ff.y, snap[0].ff.z);
		voxie_debug_print6x8_(x - 30,y + 240,0x00ff00,-1,"col: %x%x%x, rad1: %5.3f, rad2: %5.3f", snap[0].r, snap[0].g, snap[0].b, snap[0].rad, snap[0].rad2);
		voxie_debug_print6x8_(x - 30,y + 250,0x00ff00,-1,"fillmode: %d", snap[0].fillmode);

		voxie_debug_print6x8_(x - 30,y + 300,0xff0000,-1,"pp.x: %5.3f, pp.y: %5.3f, pp.z: %5.3f", snap[1].pp.x, snap[1].pp.y, snap[1].pp.z);
		voxie_debug_print6x8_(x - 30,y + 310,0xff0000,-1,"dd.x: %5.3f, dd.y: %5.3f, dd.z: %5.3f", snap[1].dd.x, snap[1].dd.y, snap[1].dd.z);
		voxie_debug_print6x8_(x - 30,y + 320,0xff0000,-1,"rr.x: %5.3f, rr.y: %5.3f, rr.z: %5.3f", snap[1].rr.x, snap[1].rr.y, snap[1].rr.z);
		voxie_debug_print6x8_(x - 30,y + 330,0xff0000,-1,"ff.x: %5.3f, ff.y: %5.3f, ff.z: %5.3f", snap[1].ff.x, snap[1].ff.y, snap[1].ff.z);
		voxie_debug_print6x8_(x - 30,y + 340,0xff0000,-1,"col: %x%x%x, rad1: %5.3f, rad2: %5.3f", snap[1].r, 		snap[1].g, 		snap[1].b, snap[1].rad, snap[1].rad2);
		voxie_debug_print6x8_(x - 30,y + 350,0xff0000,-1,"fillmode: %d", snap[1].fillmode);	

		voxie_debug_print6x8_(x - 30,y + 400,0x0000ff,-1,"pp.x: %5.3f, pp.y: %5.3f, pp.z: %5.3f", snap[2].pp.x, snap[2].pp.y, snap[2].pp.z);
		voxie_debug_print6x8_(x - 30,y + 410,0x0000ff,-1,"dd.x: %5.3f, dd.y: %5.3f, dd.z: %5.3f", snap[2].dd.x, snap[2].dd.y, snap[2].dd.z);
		voxie_debug_print6x8_(x - 30,y + 420,0x0000ff,-1,"rr.x: %5.3f, rr.y: %5.3f, rr.z: %5.3f", snap[2].rr.x, snap[2].rr.y, snap[2].rr.z);
		voxie_debug_print6x8_(x - 30,y + 430,0x0000ff,-1,"ff.x: %5.3f, ff.y: %5.3f, ff.z: %5.3f", snap[2].ff.x, snap[2].ff.y, snap[2].ff.z);
		voxie_debug_print6x8_(x - 30,y + 440,0x0000ff,-1,"col: %x%x%x, rad1: %5.3f, rad2: %5.3f", snap[2].r, 		snap[2].g, 		snap[2].b, snap[2].rad, snap[2].rad2);
		voxie_debug_print6x8_(x - 30,y + 450,0x0000ff,-1,"fillmode: %d", snap[2].fillmode);	

		voxie_debug_print6x8_(x - 30,y + 500,0xff00ff,-1,"pp.x: %5.3f, pp.y: %5.3f, pp.z: %5.3f", snap[3].pp.x, snap[3].pp.y, snap[3].pp.z);
		voxie_debug_print6x8_(x - 30,y + 510,0xff00ff,-1,"dd.x: %5.3f, dd.y: %5.3f, dd.z: %5.3f", snap[3].dd.x, snap[3].dd.y, snap[3].dd.z);
		voxie_debug_print6x8_(x - 30,y + 520,0xff00ff,-1,"rr.x: %5.3f, rr.y: %5.3f, rr.z: %5.3f", snap[3].rr.x, snap[3].rr.y, snap[3].rr.z);
		voxie_debug_print6x8_(x - 30,y + 530,0xff00ff,-1,"ff.x: %5.3f, ff.y: %5.3f, ff.z: %5.3f", snap[3].ff.x, snap[3].ff.y, snap[3].ff.z);
		voxie_debug_print6x8_(x - 30,y + 540,0xff00ff,-1,"col: %x%x%x, rad1: %5.3f, rad2: %5.3f", snap[3].r, 		snap[3].g, 		snap[3].b, snap[3].rad, snap[3].rad2);
		voxie_debug_print6x8_(x - 30,y + 550,0xff00ff,-1,"fillmode: %d", snap[3].fillmode);	

   

		switch(snapNo) {
			case 0:
			voxie_debug_print6x8_(x + 350,y + 200,0xffff00,-1,"<=- ***");
						break;
			case 1:
	voxie_debug_print6x8_(x + 350,y + 300,0xffff00,-1,"<=- ***");
			break;
						case 2:
	voxie_debug_print6x8_(x + 350,y + 400,0xffff00,-1,"<=- ***");
			break;
						case 3:
	voxie_debug_print6x8_(x + 350,y + 500,0xffff00,-1,"<=- ***");
			break;

		}

		voxie_frame_end(); voxie_getvw(&vw);
		numframes++;
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
