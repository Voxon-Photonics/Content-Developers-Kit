#if 0
!if 1

	#Visual C makefile:
obj2d3d.exe: obj2d3d.c voxiebox.h; cl /TP obj2d3d.c /Ox /MT /link user32.lib
	del obj2d3d.obj

!else

	#GNU C makefile:
obj2d3d.exe: obj2d3d.c; gcc obj2d3d.c -o obj2d3d.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

enum {
METHOD_DEGREE, METHOD_VECTOR,
};

enum {
	BEH_SEEK, BEH_IDLE, BEH_RANDOM,
};

/*
This demo program is for moving an object from 2d to 3d and copying over its rotations
*/


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

//Rotate vectors a & b around their common plane, by ang
void rotvex (float ang, point2d *a, point2d *b)
{
	float f, c, s;
	int i;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;

}

static voxie_wind_t vw;
voxie_frame_t vf;
#define OBJECT_MAX 20
typedef struct { point3d pos, vel, xrot, yrot, zrot; point2d tl, tr, bl, br; float speed, scale, scale2dOf, angle, oldAngle; int beh, col; bool bullet, view2d; double life; } object_t;
static object_t objects[OBJECT_MAX];
static int objectNo = 0;

static int gcnti[2], gbstat = 0, disp = 1, buttonDelay = 0;
poltex_t vtext[5]; int mesh[5]; 




// generates a random colour value
int randomCol() {

    return ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23);
}

//returns a random point
point3d randomPos(voxie_wind_t *vw) {

   point3d pos;

    pos.x = (float)((rand()&32767)-16384)/16384.f*vw->aspx;
    pos.y = (float)((rand()&32767)-16384)/16384.f*vw->aspy;
    pos.z = (float)((rand()&32767)-16384)/16384.f*vw->aspz;

return pos;
}


// Draw a circle
void drawCir( voxie_frame_t *vf, point3d pos, float rad, float resolution, int colour) {
    float angle = 0;
    point2d pp; 

    while (angle < 2 * PI) {
        // calculate x, y from a vector with known length and angle
        pp.x = pos.x + rad * cos (angle);
        pp.y = pos.y + rad * sin (angle);
        voxie_drawvox (vf, pp.x, pp.y, pos.z, colour);
        angle += resolution;
    }

}

// 3d rectangle collision check boxTLU = Top, Left, Up, boxBRD = bottom, right, down collision to check
bool boxColChk3d(point3d boxTLU, point3d boxBRD, point3d pos ) {

	if (pos.x < boxBRD.x &&
		pos.x > boxTLU.x 
		&&
		pos.y < boxBRD.y &&
		pos.y > boxTLU.y
		&&
		pos.z < boxBRD.z &&
		pos.z > boxTLU.z
		) {
		return true;
	} 

	return false;
}

// Sphere collision check - checks weather a sphere is inside another sphere
bool sphereColChk(point3d cir1, float cir1Rad,  point3d cir2, float cir2Rad) {

    float sidea = fabs(cir1.x - cir2.x);
    float sideb = fabs(cir1.y - cir2.y);
    sidea = sidea * sidea;
    sideb = sideb * sideb;
    float distance = (float) sqrt(sidea+sideb);
    if(distance < cir1Rad + cir2Rad && cir1.z - cir1Rad < cir2.z + cir2Rad && cir1.z + cir1Rad > cir2.z - cir2Rad ) {
	
		return true;
    }  
    return false;
}

// 2d circle collision check for a 3d sphere use sphereColChk
bool circleColChk2d(point3d cir1, float cir1Rad,  point3d cir2, float cir2Rad){

    float sidea = fabs(cir1.x - cir2.x);
    float sideb = fabs(cir1.y - cir2.y);
    sidea = sidea * sidea;
    sideb = sideb * sideb;
    float distance = (float) sqrt(sidea+sideb);
    if(distance < cir1Rad + cir2Rad){
	
	    return true;
    }  
    return false;
}

// convert radians to degrees
float rad2Deg (float rad) {
	
    return rad * (180/PI);
}
// convert degrees to radians
float deg2Rad (float deg) {

	return deg * (PI/180);
}

// fixes radians that are outside of the radius of a cricle
float radFix (float rad ) {

	if (rad > 6.2831) return  rad -= 6.2831;
	else if (rad < 0) return rad += 6.2831;
	return rad;
}





// reset Ang - resets the rotation of an object
void resetAng(point3d *rr, point3d *dd, point3d *ff, float scale) 
{
	rr->x = scale;	rr->y = 0; 			rr->z = 0; 	
	dd->x = 0;			dd->y = scale; 	dd->z = 0;
	ff->x = 0;			ff->y = 0;			ff->z = scale; 
}

// reset Ang - resets the rotation of an object
void resetAng2d(point2d *tl, point2d *tr, point2d *bl, point2d *br, float scale) 
{
		tl->x = -scale;
		tl->y = -scale;

		tr->x = scale; 
		tr->y = -scale;
	
		bl->x = -scale; 
		bl->y = scale;
	
		br->x = scale; 
		br->y = scale;	
}

// flip Vertical or Horzional 
void flipvex (point3d *a, point3d *b) {

point3d t;

t.x = a->x; a->x = b->x; b->x = t.x; 
t.y = a->y; a->y = b->y; b->y = t.y;
t.z = a->z; a->z = b->z; b->z = t.z;

}

// calculates the angle to move plug in the position of both objects and their velocity 
float calcAng(point3d seekFromPos, point3d seekFromVel, point3d seekToPos, point3d seekToVel) {
return atan2(((seekToPos.y + seekToVel.y ) -seekFromPos.y + seekFromVel.y), (( seekToPos.x + seekToVel.x ) - seekFromPos.x + seekFromVel.x )  );
}

// calculates the angle to move plug in the position of both objects 
float calcAng(point3d seekFromPos,  point3d seekToPos ) {
return atan2((seekToPos.y  - seekFromPos.y), ( seekToPos.x - seekFromPos.x  ) );
}

// calculates the angle to move plug in the position of both objects 
float calcAng(point2d seekFromPos,  point2d seekToPos ) {
return atan2((seekToPos.y  - seekFromPos.y), ( seekToPos.x - seekFromPos.x  ) );
}

// using vector math move a point3d towards another point3d returns true once collisin happens
bool seekTo(point3d *distanceFrom, point3d *distanceTo, float speed, double dtim, float colSize) {

// pp is position pv is position velocity.
point3d pp, pv; 
float f, g;
if (colSize == 0) g = 0.002; // accuracy
else g = colSize / 2;

    pp.x = distanceTo->x - distanceFrom->x;
    pp.y = distanceTo->y - distanceFrom->y;
    pp.z = distanceTo->z - distanceFrom->z;

    // vector normalisation without this the speed will ramp and and ramp down as it draws closer
    f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z);

    // work out the velocity.
    pv.x = 	(speed ) * (pp.x / f) * dtim; 
    pv.y = 	(speed ) * (pp.y / f) * dtim;
    pv.z = 	(speed ) * (pp.z / f) * dtim;

    // check weather the object is aleady at the position.
    if ( distanceFrom->x < distanceTo->x + g  && distanceFrom->x > distanceTo->x - g 
	  && distanceFrom->y < distanceTo->y + g  && distanceFrom->y > distanceTo->y - g 
      && distanceFrom->z < distanceTo->z + g  && distanceFrom->z > distanceTo->z - g 
        ) {
        return true; 
    
    } else {		
    // apply the velocity
    distanceFrom->x +=  pv.x;
    distanceFrom->y +=  pv.y;
    distanceFrom->z +=  pv.z;
    return false;
    }
}





int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	int testState = 1;

	vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
	vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
	vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
	mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;
	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim, btnDly, avgdtim = 0.0;
	int i, mousx = 0, mousy = 0, mousz = 0, x,z, choice1, choice2, choice3, choice4;
	point3d ss, pp, spp, rr, ppt, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int ovxbut[4], vxnplays, col[10] = {0xffff00,0x00ffff,0xff00ff,0xffffff,0xff0000,0x0000ff,0xff00ff,0x00ff00,0xffff00,0x00ffff};
    int inited = 0;
		float rad = 1,f, g, g2;
	float angle = 0;
	float resolution = 2;
	float angleOFFSET = 1.5707963268f; // 90' offset due to where the objs face (this is a quick hak to fix my .objs rotations)
	bool isShotLocked = false; 
	float enemyShotAccuracy = 2; 
	float TL, TR, BL, BR;
	float scale = 0.2f;



	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }

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

		if (voxie_keystat(0x0d) == 1) { scale += 0.01; inited = 0; } // kb  -
		if (voxie_keystat(0x0c) == 1) { scale -= 0.01; inited = 0; } // kb + 
		if (voxie_keystat(0x02) == 1) { for(i = 0; i < objectNo; i++) objects[i].beh = BEH_SEEK; }
		if (voxie_keystat(0x03) == 1) { for(i = 0; i < objectNo; i++) objects[i].beh = BEH_IDLE; }
		if (voxie_keystat(0x04) == 1) { for(i = 0; i < objectNo; i++) objects[i].beh = BEH_RANDOM; }
		if (voxie_keystat(0x05) == 1) { for(i = 0; i < objectNo; i++) { objects[i].view2d = !objects[i].view2d;}
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
		// put stuff here
#endif
	if (inited == 0) {
	objectNo = 1;
	inited = 1;



		for (i = 0; i < OBJECT_MAX; i++) {

		objects[i].col = 0xFFFF00 + (i * 5); // rainbow colour change
	
		objects[i].scale = scale;
		objects[i].scale2dOf = objects[i].scale * .7 ;

		objects[i].beh = BEH_IDLE;
		

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

		objects[i].pos.z = .3;

		objects[i].tl.x = -objects[i].scale2dOf;
		objects[i].tl.y = -objects[i].scale2dOf;

		objects[i].tr.x = objects[i].scale2dOf ; 
		objects[i].tr.y = -objects[i].scale2dOf ;
	
		objects[i].bl.x = -objects[i].scale2dOf ; 
		objects[i].bl.y = objects[i].scale2dOf ;
	
		objects[i].br.x = objects[i].scale2dOf ; 
		objects[i].br.y = objects[i].scale2dOf ;	
			

			
			objectNo++;
		}

		// create a cirle of invaders



		resolution = 0.4;
		rad = 0.7f;
		angle = 0;
		i = 1;
		while (angle < 2 * PI  ) {
		// calculate x, y from a vector with known length and angle
		objects[i].pos.x = rad * cos (angle);
		objects[i].pos.y = rad * sin (angle);

		objects[i].angle = angle - angleOFFSET;
		// for 3d
		rotvex(objects[i].angle,&objects[i].xrot, &objects[i].yrot);  
		
		// for 2d
		rotvex(objects[i].angle,&objects[i].br,&objects[i].bl);  	rotvex(objects[i].angle,&objects[i].tl,&objects[i].tr);
		i++;
		angle += resolution;
			objectNo = i;
		}

	}


		//draw wireframe box
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		//voxie_drawvox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);




	if (in.bstat == 1 && btnDly < tim || voxie_keystat(0x39) == 1) { // right click
		for(i = 0; i < objectNo; i++) objects[i].view2d = !objects[i].view2d;
		btnDly = tim + 0.5;
	}




		// final update loop for frame
	drawCir(&vf, objects[0].pos, 0.2, 	.02f, 0xFFFFFF);

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
		rotvex(objects[0].angle -	objects[0].oldAngle,&objects[i].br,&objects[i].bl);  	rotvex(objects[0].angle -	objects[0].oldAngle,&objects[i].tl,&objects[i].tr);
	// draw objects

		rr.x = 0.05f; dd.x = 0; 
		rr.y = 0.0f; dd.y = 0.100f;
		rr.z = 0.0f; dd.z = 0;


	avgdtim += (dtim-avgdtim)*.1;
voxie_debug_print6x8_(32,65,0x00ff00,-1,"View 2d to 3d || press Right Mouse to switch View || View 2D : %d || VPS :%5.1f", objects[0].view2d, 1.0/avgdtim);
voxie_debug_print6x8_(32,85,0x00ff00,-1,"Arrows left and right rotate 2d bitmap || Press 1 - 5 for behaviours || - / + Change scale");

pp.x = 0; pp.y = 0; pp.z = 0;
drawCir(&vf,  pp, 0.1, 	.02f, 0xFF0000);



for (i = 0; i < objectNo; i++) {


		switch (objects[i].beh)
			{
			default:	
			case BEH_IDLE:
			break;
			case BEH_RANDOM:
			objects[i].pos = randomPos(&vw);
			objects[i].beh = BEH_IDLE;
			break;
			case BEH_SEEK:
			if (i != 0) {
				if (seekTo(&objects[i].pos, &objects[0].pos , .2, dtim, objects[0].scale) == true) objects[i].beh = BEH_IDLE;
			
			resetAng2d(&objects[i].tl,&objects[i].tr, &objects[i].bl, &objects[i].br, objects[i].scale2dOf ); 

			objects[i].angle = calcAng(objects[i].pos, objects[0].pos ) - angleOFFSET;
			rotvex(objects[i].angle,&objects[i].br,&objects[i].bl);  	rotvex(objects[i].angle,&objects[i].tl,&objects[i].tr);
			}	
			break;
			}

	spp.x = objects[i].pos.x;
	spp.y = objects[i].pos.y;
	spp.z = -0.3;

	rr.x = 0.05f; dd.x = 0; 
	rr.y = 0.0f; dd.y = 0.100f;
	rr.z = 0.0f; dd.z = 0;

  voxie_drawlin(&vf, 0,0,0,objects[i].pos.x, objects[i].pos.y, objects[i].pos.z, 0xff0000 );
voxie_printalph_(&vf, &spp, &rr,&dd,0x00ff00,"%d", i);



	
if (!objects[i].view2d ) voxie_drawspr(&vf, "crab_anim0.obj", &objects[i].pos, &objects[i].xrot,&objects[i].yrot, &objects[i].zrot, objects[i].col);
else {  

// draw in 2d
if (voxie_keystat(0xcb)) { // move left

rotvex(0.1,&objects[i].br,&objects[i].bl);  	rotvex(0.1,&objects[i].tl,&objects[i].tr);
}

if (voxie_keystat(0xcd)) { // move right

 rotvex(-0.1,&objects[i].br,&objects[i].bl);  	rotvex(-0.1,&objects[i].tl,&objects[i].tr);
}


pp.x =  objects[i].pos.x + objects[i].tl.x;
pp.y =  objects[i].pos.y + objects[i].tl.y;
rr.x =  objects[i].pos.x + objects[i].tr.x;
rr.y =  objects[i].pos.y + objects[i].tr.y;
dd.x =  objects[i].pos.x + objects[i].bl.x;
dd.y =  objects[i].pos.y + objects[i].bl.y;
ff.x =  objects[i].pos.x + objects[i].br.x;
ff.y =  objects[i].pos.y + objects[i].br.y;

// draw markers
if ( i == 0 ) {
			voxie_drawsph(&vf, pp.x, pp.y, objects[0].pos.z, 0.05, 1, 0x00FF00);
			voxie_drawsph(&vf, rr.x, rr.y, objects[0].pos.z, 0.05, 1, 0xFF0000);
			voxie_drawsph(&vf, dd.x, dd.y, objects[0].pos.z, 0.05, 1, 0x0000FF);
			voxie_drawsph(&vf, ff.x, ff.y, objects[0].pos.z, 0.05, 1, 0xFF00FF); 
}


		voxie_debug_print6x8_(50,110,0x00ff00,-1,"pp.x: %5.3f, pp.y: %5.3f", objects[0].tl.x, objects[0].tl.y);
		voxie_debug_print6x8_(50,120,0x0000ff,-1,"dd.x: %5.3f, dd.y: %5.3f", objects[0].tr.x, objects[0].tr.y);
		voxie_debug_print6x8_(50,130,0xff0000,-1,"rr.x: %5.3f, rr.y: %5.3f", objects[0].bl.x, objects[0].bl.y);
		voxie_debug_print6x8_(50,140,0xff00ff,-1,"ff.x: %5.3f, ff.y: %5.3f", objects[0].br.x, objects[0].br.y);






			vtext[0].x = pp.x; 	vtext[0].y = pp.y;		vtext[0].z =  objects[i].pos.z;        
			vtext[1].x = rr.x;	vtext[1].y = rr.y;		vtext[1].z =  objects[i].pos.z; 
			vtext[2].x = ff.x;	vtext[2].y = ff.y;		vtext[2].z =  objects[i].pos.z; 
			vtext[3].x = dd.x; 	vtext[3].y = dd.y;		vtext[3].z =  objects[i].pos.z; 

	mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;

voxie_drawmeshtex(&vf,"2dCrab0.png",vtext,5,mesh,6,2,objects[i].col);

		 }

		}



	

		voxie_frame_end(); voxie_getvw(&vw);
	
	}
	
	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}


#if 0
!endif
#endif
