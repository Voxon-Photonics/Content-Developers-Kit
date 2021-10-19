#if 0
!if 1
#Visual C makefile:
touch2d.exe: touch2d.c voxiebox.h; cl /TP touch2d.c /Ox /MT /link user32.lib
	del touch2d.obj

!else

#GNU C makefile:
touch2d.exe: touch2d.c; gcc touch2d.c -o touch2d.exe -pipe -O3 -s -m64

!endif
!if 0
#endif
/** 
 *  Touch screen API or example code that can be used to help develop a custom touch solution for a VX application
 * 	By Matthew Vecchio for Voxon - 19/10/21
 *  Requires Runtime 18/10/21 or newer to work.
 * 
 */


#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#define PI 3.14159265358979323

static voxie_wind_t vw;
static voxie_frame_t vf; 

#define DEBUG 1

#define OBJECT_SCALE 0.75
#define OBJECT_MAX 20
typedef struct { point3d pos, xrot, yrot, zrot; float scale, angle, oldAngle; int col; } object_t;
static object_t objects[OBJECT_MAX];
static int objectNo = 0;


// some touch settings 
#define TOUCH_MAX 11 					// how many touches the API can process 
#define TIME_OUT 3						// how long before a dead touch sense dies out -- this is to prevent glitches
#define HELD_TIME 2						// how long a touch is recieved before the 'isHeld' boolean becomes true. 
#define ANGLE_DEAD_ZONE 0				// the dead zone for angle rotation delta 
#define DISTANCE_DEAD_ZONE 1			// the dead zone for the pinch distance delta

// touch point is for a single finger / touch point 
typedef struct touchPoint_t {int posx, posy, oposx, oposy, deltax, deltay, state = -1, ostate; bool isHeld, isDown, justPressed, onUp, active, inPinch; double startTime, lastUpdate; } touchPoint_t;

// touch input is the bigger struct that manages a few more lower level settings
typedef struct touchInput_t {point2d opinch0, opinch1; float sensitivity = 1, opinchDistance, pinchDistance, pinchDistanceDelta, pinchAngle, pinchRotation, opinchRotation, pinchRotationDelta, currentTouchNo; bool initPinch, active, pinchActive; touchPoint_t tPoint[TOUCH_MAX]; double heldTime = HELD_TIME; int gDeltaX, gDeltaY, ogDeltaX, ogDeltaY, pinchPriority; double pinchLastUpdate; bool focusPinch; } touchInput_t;

static touchInput_t touch;

static double gTim;

void touchPinchClear(touchInput_t * touchInputPtr) {

	if (touchInputPtr->pinchLastUpdate + 0.1 < gTim) {

		touchInputPtr->opinch0.x = 0;
		touchInputPtr->opinch0.y = 0;
		touchInputPtr->opinch1.x = 0;
		touchInputPtr->opinch1.y = 0;
		touchInputPtr->opinchDistance = 0;
		touchInputPtr->pinchDistance = 0;
		touchInputPtr->pinchDistanceDelta = 0;
		touchInputPtr->pinchRotationDelta = 0;
		touchInputPtr->initPinch = true;
		touchInputPtr->pinchRotation = 0;
		touchInputPtr->opinchRotation = 0;
		touchInputPtr->pinchActive = false;
		touchInputPtr->pinchPriority = -1;
	}

}

void touchClear(touchInput_t * touchInputPtr, int i) {

	touchInputPtr->tPoint[i].oposx = NULL;
	touchInputPtr->tPoint[i].oposy = NULL;
	touchInputPtr->tPoint[i].deltax = NULL;
	touchInputPtr->tPoint[i].deltay = NULL;
	touchInputPtr->tPoint[i].posx = NULL;
	touchInputPtr->tPoint[i].posy = NULL;
	touchInputPtr->tPoint[i].state = -1;
	touchInputPtr->tPoint[i].justPressed = false;
	touchInputPtr->tPoint[i].onUp = false;
	touchInputPtr->tPoint[i].active = false;
	touchInputPtr->tPoint[i].isDown = false;
	touchInputPtr->tPoint[i].inPinch = false;
	touchInputPtr->tPoint[i].startTime = 0;
	touchInputPtr->tPoint[i].lastUpdate = 0;

}

//Rotate vectors a & b around their common plane, by ang
void rotvex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}


// calculates the between 2D vector 
float calcDist2D(point2d *distanceFrom, point2d *distanceTo) {

	point2d pp;
	float f;

	pp.x = (distanceTo->x - distanceFrom->x);
	pp.y = (distanceTo->y - distanceFrom->y);

	f = sqrt( (pp.x * pp.x) + (pp.y * pp.y));

	return f;
}

// calculates the angle to move plug in the position of both objects 
float calcAng2D(point2d * distanceFrom,  point2d * distanceTo ) {
	return atan2((distanceTo->y  - distanceFrom->y ), ( distanceTo->x  - distanceFrom->x  )  ); // *(PI/180.f) add to end to turn into degrees  otherwise raidans
}

// 3d rectangle collision check boxTLU = Top, Left,  boxBRD = bottom, right,  Collision to check 2D bounds
bool boxColChk2d(point2d boxTL, point2d boxBR, point2d pos ) {

	if (pos.x < boxBR.x &&
		pos.x > boxTL.x 
		&&
		pos.y < boxBR.y &&
		pos.y > boxTL.y

		) {
		return true;
	} 

	return false;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	point2d distFrom, distTo;
	point2d p2, r2, d2, f2;
	int i,x,y,j;
	int inited = 0;
	float f;
	int touchTotal = 0;
	int pinchDistance = -1;
	float sensitivity = 1;
	bool showLogo = true;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}
// !! IMPORTANT !! 
// -- to get access to the touch screen you need to enable this hack to the voxie_wind struct. 
	vw.hacks += 64; //set this to disable menu touch
	voxie_init(&vw);		
// !! IMPORTANT !! 

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gTim = tim;

		if (voxie_keystat(0x1)) voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		if (voxie_keystat(0x0c)) touch.sensitivity -= 0.1;
		if (voxie_keystat(0x0d)) touch.sensitivity += 0.1;

			if (voxie_keystat(0x02) == 1) {  // KB 1 turn on touch for menu
				showLogo = !showLogo;
			}

			if (voxie_keystat(0x03) == 1) {  // KB 2 enable focus pinch turn on touch for menu
				touch.focusPinch = !touch.focusPinch;
			}

		/////////////////////////////////////////////////////////////////// INIT

		if (inited == 0) {


			for (i = 0; i < 1; i++) {

				objects[i].col = 0x404040; // rainbow color change
			
				objects[i].scale = OBJECT_SCALE;
				
					// create scale for 3d player
				objects[i].xrot.x = objects[i].scale; 	
				objects[i].xrot.y = 0;
				objects[i].xrot.z = 0;

				objects[i].yrot.x = 0;
				objects[i].yrot.y = objects[i].scale; 
				objects[i].yrot.z = 0;

				objects[i].zrot.x = 0;
				objects[i].zrot.y = 0;
				objects[i].zrot.z = objects[i].scale; 

				objects[i].pos.z = 0;
								
				objectNo++;
			}
			inited = 1;

		}


	/////////////// START OF TOUCH UPDATE ////////////////////


			touch.ogDeltaX = touch.gDeltaX;
			touch.ogDeltaY = touch.gDeltaY;

			touch.gDeltaX = 0;
			touch.gDeltaY = 0;
			
			if (touch.pinchActive) {

					distFrom.x = touch.tPoint[y].posx;
					distFrom.y = touch.tPoint[y].posy;

					distTo.x = touch.tPoint[x].posx;
					distTo.y = touch.tPoint[x].posy;

					touch.opinchDistance = touch.pinchDistance;
					touch.pinchDistance = calcDist2D(&distFrom, &distTo);

					touch.opinchRotation = touch.pinchRotation;
					touch.pinchRotation = calcAng2D(&distFrom, &distTo);
		
					touch.opinch0.x = touch.tPoint[x].posx;
					touch.opinch0.y = touch.tPoint[x].posy;
					touch.opinch1.x = touch.tPoint[y].posx;
					touch.opinch1.y = touch.tPoint[y].posy;

					touch.pinchDistanceDelta = (touch.pinchDistance - touch.opinchDistance) ;
					
					// way 1 using fmod()   touch.pinchRotationDelta = fmod(touch.pinchRotation - touch.opinchRotation + PI*3,PI*2) - PI;
					
					// this is a faster way then using the fmod approach 
					touch.pinchRotationDelta = (touch.pinchRotation - touch.opinchRotation) ;
					if (touch.pinchRotationDelta < -PI) touch.pinchRotationDelta += PI*2;
					if (touch.pinchRotationDelta > +PI) touch.pinchRotationDelta -= PI*2;

					if (fabs(touch.pinchDistanceDelta) * .01 > fabs(touch.pinchRotationDelta) ) touch.pinchPriority = 1;
					if (fabs(touch.pinchDistanceDelta) * .01 < fabs(touch.pinchRotationDelta) ) touch.pinchPriority = 2;
		

					if (touch.initPinch) {

							touch.pinchDistanceDelta = 0;
							touch.pinchRotationDelta = 0;
							touch.initPinch = false;
					}

			} else {
				touchPinchClear(&touch);
			}

			//Touch controls (lo level):
			//
			//  i: index of finger in this packet
			//x/y: current screen coords of this finger
			//  j: if (j&1): this is 1st  packet in path
			//     if (j&2): this is last packet in path ('i' will not exist in later calls)
			//returns: 0:no more data left (i,x,y,j not written)
			//         1:got more data (i,x,y,j written)
			//
			//This function should be called in a while loop until 0 is returned, ex:
			//   while (voxie_touch_read(&i,&x,&y,&j)) { /*process i,x,y,j*/ }
			
			// Get Touch inputs from voxiebox.dll
			touch.currentTouchNo = 0;

			// pinch update
			while (voxie_touch_read(&i, &x, &y, &j)) {

				touch.tPoint[i].oposx = touch.tPoint[i].posx;
				touch.tPoint[i].oposy = touch.tPoint[i].posy;
	
				touch.tPoint[i].posx = x;
				touch.tPoint[i].posy = y;

				if (j == 1) {
					touch.tPoint[i].oposx = -1;
					touch.tPoint[i].oposy = -1;
					touch.tPoint[i].startTime = tim;
				}

				touch.tPoint[i].lastUpdate = tim;
				touch.tPoint[i].state = j;
				touch.tPoint[i].active = true;

				touch.currentTouchNo++;
			}

			if (touch.currentTouchNo > TOUCH_MAX) touch.currentTouchNo = TOUCH_MAX;

				// update loop for touch controls
			for (i = 0; i < TOUCH_MAX; i++) {

				// add clear if  last update is long -- this releaves unexpected errors
				if (touch.tPoint[i].lastUpdate + TIME_OUT < tim)  touchClear(&touch, i);

				touch.tPoint[i].justPressed = false;
				touch.tPoint[i].onUp = false;
				touch.tPoint[i].isDown = false;
				touch.tPoint[i].isHeld = false;
				touch.tPoint[i].inPinch = false;

				// state reminder; 1 == just pressed, 2 == off, 0 == down
				if(touch.tPoint[i].state == 1) touch.tPoint[i].justPressed = true;
				if(touch.tPoint[i].state == 2 && touch.tPoint[i].ostate <= 1) touch.tPoint[i].onUp = true;
				if(touch.tPoint[i].state == 0) touch.tPoint[i].isDown = true;

				// hold / held check
				if(touch.tPoint[i].state == 0 && tim - touch.tPoint[i].startTime > touch.heldTime ) touch.tPoint[i].isHeld = true;

				if(touch.tPoint[i].state == 0 && touch.tPoint[i].justPressed == false) {

					touch.tPoint[i].deltax = touch.tPoint[i].posx - touch.tPoint[i].oposx;
					touch.tPoint[i].deltay = touch.tPoint[i].posy - touch.tPoint[i].oposy;

					touch.gDeltaX += touch.tPoint[i].deltax;
					touch.gDeltaY += touch.tPoint[i].deltay;

				}

		
				touch.tPoint[i].ostate = touch.tPoint[i].state;

				if(touch.tPoint[i].state == 2) { // 2 is off
					touchClear(&touch, i);
					continue;
				}


			}

		/////////////// END OF TOUCH UPDATE ////////////////////

			voxie_frame_start(&vf); 

		/////////////// UI CHECK //////////////
		

		// 2D UI Collision check...  
		p2.x = 0;
		p2.y = 0;
		r2.x = 650;
		r2.y = 75;
		j = 0x330000;
		
	
		for(i = 0; i < TOUCH_MAX; i++) {
			if (touch.tPoint[i].active == false) continue;

			d2.x = touch.tPoint[i].posx;
			d2.y = touch.tPoint[i].posy;

			if (boxColChk2d(p2, r2, d2)) {
					j = 0x660000;

					// put some collision stuff in here
		
			}

		}
	voxie_debug_drawrectfill (p2.x, p2.y, r2.x, r2.y, j);

		////////////// START OF TOUCH DRAW //////////////////	
	
			x = -1; y = -1;
			//if (touch.pinchLastUpdate + 1 < tim) touchPinchClear(&touch);
			touch.pinchActive = false;
			for (i = 0; i < touch.currentTouchNo; i++) {

				if (touch.tPoint[i].state == -1) continue;
				
				// work out radius of touch
				j = 5 + ((tim - touch.tPoint[i].startTime) * 75) ;
				if (j > 30) j = 25;

				if (touch.tPoint[i].justPressed == true || touch.tPoint[i].isHeld ) voxie_debug_drawcircfill(touch.tPoint[i].posx, touch.tPoint[i].posy, j, 0x9d9d9d);
			
				else voxie_debug_drawcirc(touch.tPoint[i].posx, touch.tPoint[i].posy, j, 0x9d9d9d);
			
				// limit pinch when there is only two contact points
				//if (touch.focusPinch && touch.currentTouchNo == 2 || touch.focusPinch == false ) {
					if (touch.pinchActive == false) { //  remove this if statement if you want the pinch to search through the finger index
					
						// draw pinch
						if (touch.currentTouchNo > 1 && x == -1 || touch.currentTouchNo > 1 && y == -1 ) {
							if (touch.tPoint[i].active == true && x == -1) x = i;
							else if (touch.tPoint[i].active == true && y == -1) y = i;
							if (x != -1 && y != -1) {
							
								touch.pinchActive = true;	
								touch.pinchLastUpdate = tim;
								touch.tPoint[x].inPinch = true;
								touch.tPoint[y].inPinch = true;

								voxie_debug_drawline(touch.tPoint[x].posx, touch.tPoint[x].posy, touch.tPoint[y].posx, touch.tPoint[y].posy, 0x00ffff);

								if (touch.focusPinch) {
									for(j = 0; j < touch.currentTouchNo; j++) {
										
										if (j == x || j == y) continue;

										touchClear(&touch, j);
									}
								}
							}
						}
					}

				// centre draw pix and overlay (draw at the end so it is on top)
				voxie_debug_drawpix(touch.tPoint[i].posx, touch.tPoint[i].posy, 0x00ff00);
				voxie_debug_drawline(touch.tPoint[i].posx , touch.tPoint[i].posy, touch.tPoint[i].posx + touch.tPoint[i].deltax , touch.tPoint[i].posy + touch.tPoint[i].deltay, 0xff0000);
						
			}


		////////////// END OF TOUCH DRAW //////////////////	

		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

		// input checking
		i = 0;

		if (fabs(touch.pinchDistanceDelta) > DISTANCE_DEAD_ZONE  /* && touch.pinchPriority == 1 */ ) {

			objects[i].scale += touch.pinchDistanceDelta * .001;
		} 

		if (fabs(touch.pinchRotationDelta) > ANGLE_DEAD_ZONE /* && touch.pinchPriority == 2 */ ) {

				rotvex(touch.pinchRotationDelta,&objects[i].xrot, &objects[i].yrot);  // send delta straight to rotation
	
		}


		// using global delta for movements this method means if a pinch is active then don't allow movements
		/*
		if (touch.pinchActive == false ) {
			if (touch.gDeltaX != 0) {
				objects[i].pos.x += (touch.gDeltaX * .001) * touch.sensitivity;
			}

			if (touch.gDeltaY != 0) {
				objects[i].pos.z += (touch.gDeltaY * .001) * touch.sensitivity;
			}
		}
		*/
		// can still negate this if pinch is active by enable touch.focusPinch = true
		for (j = 0; j < touch.currentTouchNo; j++) {

			if (touch.tPoint[j].inPinch == false ) {
				if (touch.tPoint[j].deltax != 0) {
					objects[i].pos.x += (touch.tPoint[j].deltax * .001) * touch.sensitivity;
				}

				if (touch.tPoint[j].deltay != 0) {
					objects[i].pos.z += (touch.tPoint[j].deltay * .001) * touch.sensitivity;
				}
			}

		}

		// clipping the object so it stays on the display
		if (objects[i].pos.z > vw.aspz) objects[i].pos.z = vw.aspz;
		if (objects[i].pos.z < -vw.aspz) objects[i].pos.z = -vw.aspz;

		if (objects[i].pos.y > vw.aspy) objects[i].pos.y = vw.aspy;
		if (objects[i].pos.y < -vw.aspy) objects[i].pos.y = -vw.aspy;

		if (objects[i].pos.x > vw.aspy) objects[i].pos.x = vw.aspy;
		if (objects[i].pos.x < -vw.aspy) objects[i].pos.x = -vw.aspy;


		// apply transforms...
		pp.x = objects[i].pos.x; pp.y = objects[i].pos.y; pp.z = objects[i].pos.z;
		
		rr.x = objects[i].xrot.x * objects[i].scale;
		rr.y = objects[i].xrot.y * objects[i].scale;
		rr.z = objects[i].xrot.z * objects[i].scale;
		// HAK reversing the down vector as the voxon.obj logo is upside down :S
		dd.x = -(objects[i].yrot.x * objects[i].scale);
		dd.y = -(objects[i].yrot.y * objects[i].scale);
		dd.z = -(objects[i].yrot.z * objects[i].scale);
		
		ff.x = objects[i].zrot.x * objects[i].scale;
		ff.y = objects[i].zrot.y * objects[i].scale;
		ff.z = objects[i].zrot.z * objects[i].scale;

		if (showLogo)	voxie_drawspr(&vf, "voxon.obj", &pp, &rr, &dd, &ff, objects[i].col);
		else            voxie_drawspr(&vf, "crab_anim0.obj", &pp, &rr, &dd, &ff, objects[i].col);


		////////////////////////////////////////////////////////////////// DEBUG
#if DEBUG	
		//draw wireframe box around the edge of the screen
		voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

		//display VPS
		avgdtim += (dtim - avgdtim) * .1;
		i = 0xffffff;
		voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f TOUCH TOTAL %d HACKS %d, Sense %1.2f Ppriority %d Focus Pinch %d", 1.0 / avgdtim, touch.currentTouchNo, vw.hacks, touch.sensitivity, touch.pinchPriority, touch.focusPinch ); 	
			voxie_debug_print6x8_(30, 76, 0xffc080, -1, "pinch A:%d 0X %1.0f 0Y %1.0f 1X %1.0f 1Y %1.0f\nDist %1.2f ODist %1.2f Delta %1.2f\nAng %1.2f OAng %1.2f AngD %1.2f Angt %1.2f", touch.pinchActive, touch.opinch0.x, touch.opinch0.y, touch.opinch1.x, touch.opinch1.y, touch.pinchDistance, touch.opinchDistance, touch.pinchDistanceDelta, touch.pinchRotation, touch.opinchRotation, touch.pinchRotationDelta, touch.pinchAngle); 	
		if (touch.pinchRotationDelta > ANGLE_DEAD_ZONE) i = 0x00ff00;
		else if (touch.pinchRotationDelta < -ANGLE_DEAD_ZONE) i = 0xff0000;
		
		voxie_debug_print6x8(30, 100, i, -1, "Angle");

		i = 0xffffff;
		if (touch.pinchDistanceDelta > DISTANCE_DEAD_ZONE) i = 0x00ff00;
		else if (touch.pinchDistanceDelta < -DISTANCE_DEAD_ZONE) i = 0xff0000;
		
		voxie_debug_print6x8(30, 100, i, -1, "             Distance");
		
		for (i = 0; i < TOUCH_MAX; i++) {

			voxie_debug_print6x8_(30, 110 + (i * 10), 0xff30f0, -1, "touch X %d Y %d Delta X %d Y %d state %d utim %1.2f stim %1.2f  ", touch.tPoint[i].posx, touch.tPoint[i].posy,  touch.tPoint[i].deltax, touch.tPoint[i].deltay, touch.tPoint[i].state, touch.tPoint[i].lastUpdate, touch.tPoint[i].startTime); 	

			if (touch.tPoint[i].active)		voxie_debug_print6x8_(30, 110 + (i * 10), 0xff0000, -1, "								   	                                    A  "); 	
			if (touch.tPoint[i].isDown)		voxie_debug_print6x8_(30, 110 + (i * 10), 0x00ffff, -1, "								   	                                     D %1.2f  ", tim - touch.tPoint[i].startTime); 	
			if (touch.tPoint[i].justPressed) 	voxie_debug_print6x8_(30, 110 + (i * 10), 0x00ff00, -1, "									                                             P"); 	
			if (touch.tPoint[i].onUp) 			voxie_debug_print6x8_(30, 110 + (i * 10), 0xffff00, -1, "									                                              U");
			if (touch.tPoint[i].inPinch) 		voxie_debug_print6x8_(30, 110 + (i * 10), 0xff00ff, -1, "									                                                I");

		}

		voxie_debug_print6x8_(30, 400, 0x00ff00, -1, "Touch Input Demo -- touch the screen! or get a touch screen\n1 = switch gfx\n2 = enable / disable focus pinch (exclusive pinch mode)\n- = reduce sensitivity\n+ = increase sensitivity ", 1.0 / avgdtim, touch.currentTouchNo, vw.hacks, touch.sensitivity, touch.pinchPriority ); 	
	
#endif
		voxie_frame_end(); 
		voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
