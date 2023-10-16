#include "vxCPP.h"

/*
  Coloring Models, Quads and Prim example and demonstration by Matthew Vecchio for Voxon 13-10-2023

  This demo shows how color works differently on coloring models, prims and quads. Press the Q,W,E,A,S,D keys to adjust colors
  
  Basically models (or sprites) mesh quads color values have a higher range to allow their texture to 
  be tinted or overloaded. The range for these types is x4 more So 0x404040 = 0xffffff 
  
  The monkey model in the bottom right is being rendered using a forth of the color value thus looking the same as a prim.

*/


// Clips INT values could use MAX,MIN but this is easier to read....
int clipIntValue(int value, int maxValue, int minValue) {
	if (value > maxValue) value = maxValue;
	if (value < minValue) value = minValue;
	return value;
}
// Merge RGB hex colors into a single value
int mergeRGBCol(int rValue, int gValue, int bValue) {

	return (rValue << 16) | (gValue << 8) | (bValue);
}

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (!hVoxieDLL) return 1;
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
	if (pEntryFunction == 0) return 1;
	IVoxieBox* voxie = pEntryFunction();

	point3d pp = { 0 }, modelRVec = { 0 }, modelDVec = { 0 }, modelFVec = { 0 };
	point3d pp2 = { 0 }, rr = { 1,0,0 }, dd = { 0,1,0 }, ff = { 0,0,1 };
	point3d textPos = { -0.9,0.80,-0.3 };
	point3d textHeight = { .1,0,0 };
	point3d textWidth = { 0,.1,0 };
	int rValue = 0x40;
	int gValue = 0x40;
	int bValue = 0x40;
	int colorValue = 0x404040;
	int increaseAmount = 1;
	float rotationChange = 0;
	float objScalar = 0.40;

	// define vectors for 3D models
	modelRVec = { 1 * objScalar , 0, 0 };
	modelDVec = { 0, 1 * objScalar , 0 };
	modelFVec = { 0, 0, 1 * objScalar };
	float modelSpace = 0.50;


	while (voxie->breath())
	{
		// ** INPUT
		if (voxie->getKeyIsDown(KB_Q)) rValue += increaseAmount;
		if (voxie->getKeyIsDown(KB_A)) rValue -= increaseAmount;

		if (voxie->getKeyIsDown(KB_W)) gValue += increaseAmount;
		if (voxie->getKeyIsDown(KB_S)) gValue -= increaseAmount;

		if (voxie->getKeyIsDown(KB_E)) bValue += increaseAmount;
		if (voxie->getKeyIsDown(KB_D)) bValue -= increaseAmount;

		// return to defaults
		if (voxie->getKeyIsDown(KB_Space_Bar)) {
			rValue = 0x40;
			gValue = 0x40;
			bValue = 0x40;
		}

		rotationChange = 0;
		if (voxie->getKeyIsDown(KB_Comma))		rotationChange = (-1 * voxie->getDeltaTime());
		if (voxie->getKeyIsDown(KB_Full_Stop))  rotationChange = ( 1 * voxie->getDeltaTime());


		// ** UPDATE

		// update rotation
		if (rotationChange != 0) {
			voxie->rotVex(rotationChange, &modelRVec, &modelDVec);
		}


		// clip the color values to fit within limits
		rValue = clipIntValue(rValue, 0xff, 0x00);
		gValue = clipIntValue(gValue, 0xff, 0x00);
		bValue = clipIntValue(bValue, 0xff, 0x00);
		// merge the three colour values together...
		colorValue = mergeRGBCol(rValue, gValue, bValue);

		
		// ** DRAW

		voxie->startFrame();

		// Draw Models
		textPos.y = -0.9;
		voxie->drawText(&textPos, &textHeight, &textWidth, 0xffffff, "Draw Model / Sprite");
		pp.x = -.70; pp.y = -.60;
		voxie->drawModel("assets/cube.obj", &pp, &modelRVec, &modelDVec, &modelFVec, colorValue);
		pp.x += modelSpace;
		voxie->drawModel("assets/sphere.obj", &pp, &modelRVec, &modelDVec, &modelFVec, colorValue);
		pp.x += modelSpace;
		voxie->drawModel("assets/plane.obj", &pp, &modelRVec, &modelDVec, &modelFVec, colorValue);
		pp.x += modelSpace;
		voxie->drawModel("assets/suzanne.obj", &pp, &modelRVec, &modelDVec, &modelFVec, colorValue);

		
		// Draw some prims

		pp.y += modelSpace;
		textPos.y = pp.y - 0.3;
		voxie->drawText(&textPos, &textHeight, &textWidth, 0xffffff, "Draw Prims");

		pp.x = -.70;

		voxie->drawSphere(pp, 0.2, 0, colorValue);
		pp.x += modelSpace;
		voxie->drawSphere(pp, 0.2, 1, colorValue);
		pp.x += modelSpace;
		// Draw cube draws from the TOPUPLEFT
		pp2.x = pp.x - ((modelRVec.x + modelDVec.x + modelFVec.x) * .5);
		pp2.y = pp.y - ((modelRVec.y + modelDVec.y + modelFVec.y) * .5);
		pp2.z = pp.z - ((modelRVec.z + modelDVec.z + modelFVec.z) * .5);
		voxie->drawCube(&pp2, &modelRVec, &modelDVec, &modelFVec, 2, colorValue);
		
		pp.x += modelSpace;
		// Hack to draw a cone as a sphere  
		pp2 = pp;
		pp2.z = pp.z - 0.2;
		voxie->drawCone(pp,0.2,pp2,0,1, colorValue);

		
		// Draw some Quads
		pp.y += modelSpace;
		textPos.y = pp.y - 0.3;
		voxie->drawText(&textPos, &textHeight, &textWidth, 0xffffff, "Draw Quads");
		pp.x = -.70;

		voxie->drawQuad((char*)"assets/ReadyBW.png", &pp, objScalar * 0.5, objScalar * 0.5, 0,0,0, colorValue, 1,1);
		pp.x += modelSpace;
		voxie->drawQuad((char*)"assets/ReadyColBg.png", &pp, objScalar * 0.5, objScalar * 0.5, 0, 0, 0, colorValue, 1, 1);
		pp.x += modelSpace;
		voxie->drawQuad((char*)"assets/BBricks.jpg", &pp, objScalar * 0.5, objScalar * 0.75, 0, 0, 0, colorValue, 1, 1);
		
		pp.x += modelSpace;
		voxie->drawModel("assets/suzanne.obj", &pp, &modelRVec, &modelDVec, &modelFVec, voxie->colorHexDivide(colorValue,4)); // if you divide a hex colour by 4 it will be scaled apporatelty to be the same as regular prims 

		// Write out colour value
		textPos.y = 0.8;
		voxie->drawText(&textPos, &textHeight, &textWidth, colorValue, "Sample Text");
		voxie->drawText(&textPos, &textHeight, &textWidth, 0xffffff, "\nCol Value %#08x",colorValue);

		// Draw some Textures for Example
		voxie->showVPS();
		voxie->debugText(20, 70, 0xffffff, -1, "This demo shows how color works differently on coloring Models,\nPrims and Quads.\nBasically models (or sprites) mesh and quads color values\nhave a higher range to allow their texture to be\ntinted or overloaded. The range for these types is x4 more\nSo, 0x404040 = 0xffffff\nThe monkey model in the bottom right is being rendered using\na forth of the color value thus looking the same as a prim.");
		voxie->debugText(20, 140, 0xffff00, -1, "Press Q,W,E keys to increase Red, Green or Blue Color Channels");
		voxie->debugText(20, 150, 0xffff00, -1, "Press A,S,D keys to decrease Red, Green or Blue Color Channels");

		voxie->debugText(20, 170, 0xff0000, -1,   "Red   Value  : %02x", rValue);
		voxie->debugText(20, 180, 0x00ff00, -1,   "Green Value  : %02x", gValue);
		voxie->debugText(20, 190, 0x0000ff, -1,   "Blue  Value  : %02x", bValue);
		voxie->debugText(20, 200, 0xffffff, -1, "Color Value : %#08x", colorValue);
		voxie->debugText(170, 200, colorValue, -1, "######" );

		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
