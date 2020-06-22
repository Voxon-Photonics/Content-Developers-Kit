#if 0
!if 1

	#Visual C makefile:
objRot.exe: objRot.c voxiebox.h; cl /TP objRot.c /Ox /MT /link user32.lib
	del objRot.obj

!else

	#GNU C makefile:
objRot.exe: objRot.c; gcc objRot.c -o objRot.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

enum {

METHOD_DEGREE, METHOD_VECTOR,

};

/*
This demo program contains obj rotation along a circle and two algro fo seeking objects using vector and degrees


*/



static float BULLET_SPEED = 0.5;
static float CIRCLE_RAD = 0.05;

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



static int seekMethod;

static voxie_wind_t vw;
	voxie_frame_t vf;
#define OBJECT_MAX 100
typedef struct { point3d pos, vel, xrot, yrot, zrot; float speed, angle, oldAngle; int col; bool bullet; double life; } object_t;
static object_t objects[OBJECT_MAX];
static int objectNo = 0;

#define BULLET_MAX 100
typedef struct { point3d pos, vel, xrot, yrot, zrot, dest; float speed, angle, faceAngle, oldFaceAngle; int col; bool bullet; double life; } bullet_t;
static bullet_t bullets[BULLET_MAX];
static int bulletNo = 0;



static int gcnti[2], gbstat = 0, disp = 1, tics = 0, buttonDelay = 0;
static void mymix (int *ibuf, int nsamps)
{
	static int cnt[2]; int i, c;
	for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
		for(c=min(vw.playnchans,2)-1;c>=0;c--)
			{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&gbstat; cnt[c] += gcnti[c]; }
}

// reset Ang - resets the rotation of an object
static void resetAng(point3d *rr, point3d *dd, point3d *ff, float scale) 
{

	rr->x = scale; 	
	rr->y = 0; 	
	rr->z = 0; 	

	dd->x = 0;
	dd->y = scale; 
	dd->z = 0;

	ff->x = 0;
	ff->y = 0;
	ff->z = scale; 
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



static void facevex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}




// 3d circle colision check
static bool circleColChk3d(point3d cir1, float cir1Rad,  point3d cir2, float cir2Rad){
    float sidea = abs(cir1.x - cir2.x);
    float sideb = abs(cir1.y - cir2.y);
    sidea = sidea * sidea;
    sideb = sideb * sideb;
    float distance = (float) sqrt(sidea+sideb);
    if(distance < cir1Rad + cir2Rad && cir1.z - cir1Rad < cir2.z + cir2Rad && cir1.z + cir1Rad > cir2.z - cir2Rad ){
	
		return true;
    }  
    return false;
}




// calculates the angle to move plug in the position of both objects and their velocity 
static float calcSeek(point3d seekFromPos, point3d seekFromVel, point3d seekToPos, point3d seekToVel) {

return atan2(((seekToPos.y + seekToVel.y ) -seekFromPos.y + seekFromVel.y), (( seekToPos.x + seekToVel.x ) - seekFromPos.x + seekFromVel.x )  );

}

// calculates the angle to move plug in the position of both objects 
static float calcSeek(point3d seekFromPos,  point3d seekToPos ) {

return atan2((seekToPos.y  - seekFromPos.y), ( seekToPos.x - seekFromPos.x  )  );

}


// draw a circle
static void drawCir( point3d pos, float rad, float resolution, int colour) {

float angle = 0;
float x,y;

while (angle < 2 * PI) {
    // calculate x, y from a vector with known length and angle
    x = rad * cos (angle);
    y = rad * sin (angle);
    voxie_drawvox (&vf,x + pos.x,y + pos.y, pos.z, colour);
    angle += resolution;
	}

}

// calculates the Distance in a Vector
void calcDistV(point3d *distanceFrom, point3d *distanceTo, float speed, double dtim) {

point3d pp, pv;
float f;

		pp.x = distanceTo->x - distanceFrom->x;
		pp.y = distanceTo->y - distanceFrom->y;
		pp.z = distanceTo->z - distanceFrom->z;

  	f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z);

		pv.x = 	(speed ) * (pp.x / f) * dtim;
		pv.y = 	(speed ) * (pp.y / f) * dtim;
		pv.z = 	(speed ) * (pp.z / f) * dtim;

		pp.x +=  pv.x;
		pp.y +=  pv.y;
		pp.z +=  pv.z;

		distanceFrom->x = pp.x;
		distanceFrom->y = pp.y;
		distanceFrom->z = pp.z;


}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	int testState = 1;

	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim, btnDly;
	int i, mousx = 0, mousy = 0, mousz = 0, x,z, y;
	point3d ss, pp, spp, rr, ppt, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int ovxbut[4], vxnplays, col[10] = {0xffff00,0x00ffff,0xff00ff,0xffffff,0xff0000,0x0000ff,0xff00ff,0x00ff00,0xffff00,0x00ffff};
  int inited = 0;
	float rad = 1,f, g, g2;
	float angle = 0;
	float resolution = 2;
	float angleOFFSET = 1.5707963268f; // 90' offset due to where the objs face 
	
	bool isShotLocked = false; // false for a shot CLOSE to invadership (for enemy fire) 
	float enemyShotAccuracy = 2; 



	if (inited == 0) {
	objectNo = 1;
	inited = 1;
	objects[0].bullet = false;
	objects[1].bullet = false;
	objects[0].col = 0xFFFF00;
	objects[1].col = 0x00FFFF;



		for (i = 0; i < OBJECT_MAX; i++) {
			// create rotations for player
		objects[i].xrot.x = 0.2; 	
		objects[i].xrot.y = 0;
		objects[i].xrot.z = 0;

		objects[i].yrot.x = 0;
			objects[i].yrot.y = 0.2; 
		objects[i].yrot.z = 0;

		objects[i].zrot.x = 0;
		objects[i].zrot.y = 0;
		objects[i].zrot.z = 0.2; 

		objects[i].pos.z = .3;


	
		
			

			
			objectNo++;
		}

		// create a cirle of invaders

				resolution = 0.4;
		rad = 0.7f;
		
		i = 1;
		while (angle < 2 * PI  ) {
		// calculate x, y from a vector with known length and angle
		objects[i].pos.x = rad * cos (angle);
		objects[i].pos.y = rad * sin (angle);
		objects[i].col = 0x0000FF + i;
		objects[i].angle = angle - angleOFFSET;
		rotvex(objects[i].angle ,&objects[i].xrot, &objects[i].yrot);
		i++;
		angle += resolution;
			objectNo = i;
		}

	}
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
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);
				for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1

		}
		// put controls here keys here
		if (voxie_keystat(0x1)) { voxie_quitloop(); }
	
	






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
		// put stuff here
#endif

			//draw wireframe box
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		//voxie_drawvox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

#if 1
		// put stuff here
	if (in.bstat == 1) { // click mouse

	// to create a heat seeking bullet
	if (isShotLocked) {
			// fire bullet
		if (bulletNo + 1 < BULLET_MAX) {
		voxie_debug_print6x8_(32,120 ,0xffc080,-1,"Clicked");
		bulletNo++;
		bullets[bulletNo].bullet = true;
		bullets[bulletNo].pos.x = objects[rand() % objectNo ].pos.x;
		bullets[bulletNo].pos.y = objects[rand() % objectNo ].pos.y;
		bullets[bulletNo].pos.z = objects[rand() % objectNo ].pos.z;
		bullets[bulletNo].vel.x = 0;
		bullets[bulletNo].vel.y = 0;
		bullets[bulletNo].vel.z = 0;
		bullets[bulletNo].life = tim + 10;
		bullets[bulletNo].col = 0xFF0000;

		bullets[bulletNo].xrot.x = 0.2; 	
		bullets[bulletNo].xrot.y = 0;
		bullets[bulletNo].xrot.z = 0;

		bullets[bulletNo].yrot.x = 0;
		bullets[bulletNo].yrot.y = 0.2; 
		bullets[bulletNo].yrot.z = 0;

		bullets[bulletNo].zrot.x = 0;
		bullets[bulletNo].zrot.y = 0;
		bullets[bulletNo].zrot.z = 0.2; 

		bullets[bulletNo].faceAngle = 0;
		
	//	rotvex(angleOFFSET,&bullets[bulletNo].yrot,&bullets[bulletNo].zrot);
		}
	// to create a bullet near the destination
	} else {


		if (bulletNo + 1 < BULLET_MAX) {
		voxie_debug_print6x8_(32,120 ,0xffc080,-1,"Clicked");
		bulletNo++;
		bullets[bulletNo].bullet = true;
		bullets[bulletNo].pos.x = objects[0].pos.x;
		bullets[bulletNo].pos.y = objects[0].pos.y;
		bullets[bulletNo].pos.z = objects[0].pos.z;
		bullets[bulletNo].vel.x = 0;
		bullets[bulletNo].vel.y = 0;
		bullets[bulletNo].vel.z = 0;
		bullets[bulletNo].life = tim + 10;
		bullets[bulletNo].col = 0xFF0000;

		bullets[bulletNo].xrot.x = 0.2; 	
		bullets[bulletNo].xrot.y = 0;
		bullets[bulletNo].xrot.z = 0;

		bullets[bulletNo].yrot.x = 0;
		bullets[bulletNo].yrot.y = 0.2; 
		bullets[bulletNo].yrot.z = 0;

		bullets[bulletNo].zrot.x = 0;
		bullets[bulletNo].zrot.y = 0;
		bullets[bulletNo].zrot.z = 0.2; 

		bullets[bulletNo].faceAngle = 0;

		// where the bullet is going 
		// work out the angle between + 3 and - 3
		f = (float)((rand() & 32767) - 16384) / 16384.f * 3; // circle
		f =  (float)((rand() & 32767) - 16384) / 16384.f * .7;  // accuracy
		bullets[bulletNo].dest.z = vw.aspz;
		bullets[bulletNo].dest.x = cos( f) * ( .8); // .8 is the distance out
		bullets[bulletNo].dest.y = sin(f) * ( .8);

		
	//	rotvex(angleOFFSET,&bullets[bulletNo].yrot,&bullets[bulletNo].zrot);
		}



	}

		}
		

	


	if (in.bstat == 2 && btnDly < tim) { // right click
	

 if (seekMethod == METHOD_DEGREE && !isShotLocked) {
		isShotLocked = true;	
		 seekMethod = METHOD_DEGREE;
	}

  else if (seekMethod == METHOD_DEGREE && isShotLocked) {
		isShotLocked = false;	
		 seekMethod = METHOD_VECTOR;
	}
	else if (seekMethod == METHOD_VECTOR && !isShotLocked)  {
		
			isShotLocked = true;	
		seekMethod = METHOD_VECTOR;
	}	else if (seekMethod == METHOD_VECTOR && isShotLocked)  {
		
			isShotLocked = false;	
		seekMethod = METHOD_DEGREE;
	}
	btnDly = tim + 0.5;
	}




#else


#endif



	drawCir( objects[0].pos, 0.2, 	.02f, 0xFFFFFF);



		for (i=bulletNo ;i >0;i--) {

	bullets[i].speed = BULLET_SPEED;

	switch(seekMethod) {
		case METHOD_DEGREE:
		// seek towards object 0 (which is the yellow mouse control)
		bullets[i].angle = calcSeek(bullets[i].pos, bullets[i].vel, objects[0].pos, objects[0].vel);
	
			
		bullets[i].vel.x = bullets[i].speed * cos(bullets[i].angle)  * dtim; // angle
		bullets[i].vel.y = bullets[i].speed * sin(bullets[i].angle)  * dtim; // angle 
		bullets[i].vel.z = (bullets[i].speed *2) * atan((objects[0].pos.z + objects[0].vel.z ) - bullets[i].pos.z + bullets[i].vel.z  )  * dtim ;
		//objects[i].vel.z =  atan2((objects[0].pos.z + objects[0].vel.z ) -objects[i].pos.z + objects[i].vel.z, objects[i].angle  )  * dtim ;
		bullets[i].pos.x +=  bullets[i].vel.x;
		bullets[i].pos.y +=  bullets[i].vel.y;
		bullets[i].pos.z +=  bullets[i].vel.z;



		break;
		case METHOD_VECTOR:



 if (isShotLocked) { // seek the player
		pp.x = objects[0].pos.x - bullets[i].pos.x;
		pp.y = objects[0].pos.y - bullets[i].pos.y;
		pp.z = objects[0].pos.z - bullets[i].pos.z;
 } else {				// seek the destination that has been pre determined
		pp.x = bullets[i].dest.x - bullets[i].pos.x;
		pp.y = bullets[i].dest.y - bullets[i].pos.y;
		pp.z = bullets[i].dest.z - bullets[i].pos.z;



 }
		// vector normalisation (this makes it so the velocity won't speed up when its far or slow down when its close)
		f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z);
		
		bullets[i].vel.x = 	(bullets[i].speed ) * (pp.x / f) * dtim;
		bullets[i].vel.y = 	(bullets[i].speed ) * (pp.y / f) * dtim;
		bullets[i].vel.z = 	(bullets[i].speed ) * (pp.z / f) * dtim;

		bullets[i].pos.x +=  bullets[i].vel.x;
		bullets[i].pos.y +=  bullets[i].vel.y;
		bullets[i].pos.z +=  bullets[i].vel.z;




		break;

	}


	// how to work out angle of bullet


	// first reset vectors to netural;
		resetAng( &bullets[i].xrot, &bullets[i].yrot, &bullets[i].zrot, 0.2);
		// WORK OUT ANGLE OF X;
  	angle = calcSeek(objects[0].pos, bullets[i].pos );
		rotvex(angle +angleOFFSET, &bullets[i].xrot, &bullets[i].yrot);
		// WORK OUT ANGLE OF Z /// add the offset to get the bullet right perspective
		angle = atan((objects[0].pos.z  - bullets[i].pos.z));
		rotvex(angle -angleOFFSET , &bullets[i].zrot, &bullets[i].yrot);




	voxie_drawspr(&vf, "beamALT1.obj", &bullets[i].pos, &bullets[i].xrot,&bullets[i].yrot, &bullets[i].zrot, 0xff0000);


	if (circleColChk3d(bullets[i].pos, CIRCLE_RAD, objects[0].pos, CIRCLE_RAD )&& isShotLocked ) bullets[i].life = -1;
	if (circleColChk3d(bullets[i].pos, CIRCLE_RAD, bullets[i].dest, CIRCLE_RAD ) && !isShotLocked ) bullets[i].life = -1;


		if (bullets[i].life < tim) { bulletNo--; bullets[i] = bullets[bulletNo]; }
		
		}


 		pp.x = -0.950f; rr.x = 0.04f; dd.x = 0.015f; 
		pp.y = -0.910f;	rr.y = 0.0f; dd.y = 0.080f;
		pp.z = 0.270f; 	rr.z = 0.0f; dd.z = 0.0f;

		voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Pos X%d, Y%d, Z%d Del X%d, Y%d, Z%d", mousx, mousy, mousz, in.dmousx, in.dmousy, in.dmousz);
		pp.y -= -0.120f;

		voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"But %d OBut %d ONo : %d", in.bstat, in.obstat, objectNo);
		pp.y -= -0.120f;

		objects[0].pos.x = mousx * .001;
		objects[0].pos.y = mousy * .001;
		objects[0].pos.z = mousz * .0004;
		objects[0].oldAngle = objects[0].angle;
		objects[0].angle = atan2( (( 0 ) - objects[0].pos.y + objects[0].vel.y), (( 0 ) - objects[0].pos.x + objects[0].vel.x )  ) + angleOFFSET;
		rotvex(objects[0].angle -	objects[0].oldAngle, &objects[i].xrot,&objects[i].yrot);

	// draw objects

		rr.x = 0.05f; dd.x = 0; 
		rr.y = 0.0f; dd.y = 0.100f;
		rr.z = 0.0f; dd.z = 0;


if (seekMethod == METHOD_DEGREE) {
voxie_debug_print6x8_(32,65,0x00ff00,-1,"Method : DEGREE || press Right Mouse to switch algo || Player Shoot : %d", isShotLocked);
} else {
voxie_debug_print6x8_(32,65,0x00ff00,-1,"Method : VECTOR || press Right Mouse to switch algo ||  Player Shoot : %d", isShotLocked);
}

if (!isShotLocked) {

	for (i=bulletNo ;i >0;i--) {
voxie_drawsph(&vf, bullets[i].dest.x, bullets[i].dest.y, bullets[i].dest.z, 0.05, 1, 0x00ff00);
	}


}

		pp.x = 0; pp.y = 0; pp.z = 0;
	drawCir( pp, 0.1, 	.02f, 0xFF0000);



		for (i = 0; i < objectNo; i++) {

pp.x = objects[i].pos.x;
pp.y = objects[i].pos.y;
pp.z = -0.3;



  voxie_drawlin(&vf, 0,0,0,objects[i].pos.x, objects[i].pos.y, objects[i].pos.z, 0xff0000 );





	voxie_printalph_(&vf, &pp, &rr,&dd,0x00ff00,"%d", i);

	voxie_debug_print6x8_(32,100 + (i * 20),0xffc080,-1,"Object %d Pos: X %1.3f, Y %1.3f, Z %1.3f Angle %1.3f ", i, objects[i].pos.x,
	objects[i].pos.y,objects[i].pos.z, objects[i].angle);

	voxie_drawspr(&vf, "crab_anim0.obj", &objects[i].pos, &objects[i].xrot,&objects[i].yrot, &objects[i].zrot, objects[i].col);
	
		}




//longest
f = 1000;

//shortest
f = -1000;
z = -1;
				for (i = 0; i < bulletNo; i++) {


pp.x = bullets[i].pos.x;
pp.y = bullets[i].pos.y;
pp.z = bullets[i].pos.z;

ppt.x = objects[0].pos.x - bullets[i].pos.x;
ppt.y = objects[0].pos.y - bullets[i].pos.y;
ppt.z = objects[0].pos.z - bullets[i].pos.z;
g = sqrt(ppt.x*ppt.x + ppt.y*ppt.y + ppt.z*ppt.z);



// work out shortest distance
// this works by measuring the distance of all points and appling them
// make sure the f value is high before the loop
if (  g     < f) {

	f = g;
  z = i; // i is the new king

}                    
/* longest
if (  g     > f) {

	f = g;
  x = i; // i is the new king

} 
*/ 


		// final update loop for frame
		x = 20;
	
		pp.x = 0; //sin(objects[0].angle);
		pp.y = 0; // cos(objects[0].angle);
		pp.z = 0;

		rr.x = x; 	
		rr.y = 0;
		rr.z = 0;

		dd.x = 0;
		dd.y = x; 
		dd.z = 0;

		ff.x = 0;
		ff.y = 0;
		ff.z = x; 
		voxie_drawspr(&vf, "crab_anim0.obj", &pp, &rr,&dd, &ff, 0xffffff);
				
	
  voxie_drawlin(&vf, bullets[i].pos.x,bullets[i].pos.y,bullets[i].pos.z,objects[0].pos.x, objects[0].pos.y, objects[0].pos.z, 0x0000ff );
	voxie_printalph_(&vf, &pp, &rr,&dd,0x00ff00,"%d", i);

	voxie_debug_print6x8_(32,500 + (i * 20),0xff0000,-1,"Bul %d Pos: X %1.3f, Y %1.3f, Z %1.3f Angle %1.3f ", i, bullets[i].pos.x,
	bullets[i].pos.y,bullets[i].pos.z, bullets[i].faceAngle);
		}

		if (z != -1)  voxie_drawlin(&vf, bullets[x].pos.x,bullets[x].pos.y,bullets[x].pos.z,objects[0].pos.x, objects[0].pos.y, objects[0].pos.z, 0xffffff );


		voxie_frame_end(); voxie_getvw(&vw);
		tics ++;
	}
	
	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}


#if 0
!endif
#endif
