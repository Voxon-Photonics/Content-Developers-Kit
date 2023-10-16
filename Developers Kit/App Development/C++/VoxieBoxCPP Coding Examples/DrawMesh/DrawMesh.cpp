/* VXCPP drawMesh and drawMeshExt examples
 * How to setup and draw a mesh from scratch. 
 * By Ken S and Mat V for Voxon.
 * 4 / 10 / 2023 
*/

#include "pch.h"
#include "VxCpp.h"


int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	// load in the VxCpp.dll so we can create the VoxieBox object...
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("VxCpp.dll");
	if (hVoxieDLL == NULL) return 1;
	// Access and call "CreateVoxieBoxObject" function from VxCpp.dll. The CreateVoxieBoxObject function creates a new VoxieBox object.
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	// Set a new IVoxieBox pointer to point to the entry function (the voxiebox object) now 'voxie' get access to the VoxieBox class
	IVoxieBox* voxie = pEntryFunction();

	voxie->setBorder(true); // if true draws a border around the perimeter of the display

	int i, drawMode = 0/*0=voxie_drawmeshtex(), 1=voxie_drawmeshtex_ext()*/, vertexShadingFlag = 16;

	//-----------------------------------
		//voxie_drawmeshtex() setup:
	poltex_t verticeArray[8]; int vertCount;
	int meshArray[40], meshCount;

	//voxie_drawmeshtex() -- draw a diamond 

	// define vertices
	vertCount = 0;
	verticeArray[vertCount].x = -0.3; verticeArray[vertCount].y = -0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //LTU
	verticeArray[vertCount].x = +0.3; verticeArray[vertCount].y = -0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 0; vertCount++; //RTU
	verticeArray[vertCount].x = +0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //RBU
	verticeArray[vertCount].x = -0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 1; vertCount++; //LBU
	verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = -0.3; verticeArray[vertCount].col = 0xff00ff; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //top mid
	verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = -0.3; verticeArray[vertCount].col = 0xff00ff; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //top mid
	verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = 0.3; verticeArray[vertCount].col = 0x00ff00; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //bot mid
	verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = 0.3; verticeArray[vertCount].col = 0x00ff00; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //bot mid

		// draw mesh use a -1 to start a new face
	meshCount = 0;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 5; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1; //draw top spike
	meshArray[meshCount++] = 1; meshArray[meshCount++] = 4; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 4; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 3; meshArray[meshCount++] = 5; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 1; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1; //draw base
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 3; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 7; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1; //draw bot spike
	meshArray[meshCount++] = 1; meshArray[meshCount++] = 6; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 6; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 3; meshArray[meshCount++] = 7; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	//-----------------------------------
		//voxie_drawmeshtex_ext() setup:
	point3d verticeXYZ[8]; int vXYZCount;
	uvcol_t verticeUVC[8]; int vUVCCount;
	inds_t indices[40]; int indCount;

	//voxie_drawmeshtex_ext() -- draw a diamond

	// define vertice positions and vertice U,V and color values spread over 2 different arrays.
	vXYZCount = 0; vUVCCount = 0;
	verticeXYZ[vXYZCount].x = -0.3; verticeXYZ[vXYZCount].y = -0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //LTU
	verticeXYZ[vXYZCount].x = +0.3; verticeXYZ[vXYZCount].y = -0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 0; vUVCCount++; //RTU
	verticeXYZ[vXYZCount].x = +0.3; verticeXYZ[vXYZCount].y = 0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //RBU
	verticeXYZ[vXYZCount].x = -0.3; verticeXYZ[vXYZCount].y = 0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 1; vUVCCount++; //LBU
	verticeXYZ[vXYZCount].x = 0; verticeXYZ[vXYZCount].y = 0; verticeXYZ[vXYZCount].z = -0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0xff00ff; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //top mid
	verticeXYZ[vXYZCount].x = 0; verticeXYZ[vXYZCount].y = 0; verticeXYZ[vXYZCount].z = -0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0xff00ff; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //top mid
	verticeXYZ[vXYZCount].x = 0; verticeXYZ[vXYZCount].y = 0; verticeXYZ[vXYZCount].z = 0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0x00ff00; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //bot mid
	verticeXYZ[vXYZCount].x = 0; verticeXYZ[vXYZCount].y = 0; verticeXYZ[vXYZCount].z = 0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0x00ff00; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //bot mid

		// use the inds_t struct to set define the faces / indices the xyz and col values
		// the difference with using drawmeshtex_ext is that instead of using a -1 at the end of the face you add a 1 to the last bit of the 32 bit interger ( +(1<<31) ) to signitfy the start of a new face / polygon index loop.
		// this could be assigned to a variable aka EXT_END_LOOP = (1<<31) (as defined in vxDataTypes.h);

	// you can use EXT_START_LOOP or +(1<<31) to signify start of polygon loops aka indices[indCount++].xyzi = 0 + (1<<31);
	indCount = 0;
	indices[indCount++].xyzi = 0 + EXT_START_LOOP; indices[indCount++].xyzi = 5; indices[indCount++].xyzi = 1; //draw top spike
	indices[indCount++].xyzi = 1 + EXT_START_LOOP; indices[indCount++].xyzi = 4; indices[indCount++].xyzi = 2;
	indices[indCount++].xyzi = 2 + EXT_START_LOOP; indices[indCount++].xyzi = 4; indices[indCount++].xyzi = 3;
	indices[indCount++].xyzi = 3 + EXT_START_LOOP; indices[indCount++].xyzi = 5; indices[indCount++].xyzi = 0;
	indices[indCount++].xyzi = 0 + EXT_START_LOOP; indices[indCount++].xyzi = 1; indices[indCount++].xyzi = 2; //draw base
	indices[indCount++].xyzi = 2 + EXT_START_LOOP; indices[indCount++].xyzi = 3; indices[indCount++].xyzi = 0;
	indices[indCount++].xyzi = 0 + EXT_START_LOOP; indices[indCount++].xyzi = 7; indices[indCount++].xyzi = 1; //draw bot spike
	indices[indCount++].xyzi = 1 + EXT_START_LOOP; indices[indCount++].xyzi = 6; indices[indCount++].xyzi = 2;
	indices[indCount++].xyzi = 2 + EXT_START_LOOP; indices[indCount++].xyzi = 6; indices[indCount++].xyzi = 3;
	indices[indCount++].xyzi = 3 + EXT_START_LOOP; indices[indCount++].xyzi = 7; indices[indCount++].xyzi = 0;

	// this simple for loop just copies the indices indexes to the uvci values and set the 32nd bit to be corrected 
	for (i = 0; i < indCount; i++) { indices[i].uvci = indices[i].xyzi & ~EXT_START_LOOP; }
	//-----------------------------------


	// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume.
	while (voxie->breath())
	{
		// Input
		if (voxie->getKeyOnDown(KB_1) == 1) drawMode = 0;
		if (voxie->getKeyOnDown(KB_2) == 1) drawMode = 1;
		if (voxie->getKeyOnDown(KB_3) == 1) vertexShadingFlag = 16;
		if (voxie->getKeyOnDown(KB_4) == 1) vertexShadingFlag = 0;

		voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)

		if (!drawMode) voxie->drawMesh((char*)"pattern.png", verticeArray, vertCount, meshArray, meshCount, 2 + vertexShadingFlag, 0x404040);
		else voxie->drawMeshExt((char*)"pattern.png", verticeXYZ, vXYZCount, verticeUVC, vUVCCount,	indices, indCount, 2 + vertexShadingFlag, 0x404040);



		float xf, yf, zf = 0, xSize = 0.5f, ySize = 0.5f;
		inds_t quadInd[5];
		uvcol_t quadUVC[4];
		point3d quadPos[4];

		xf = -0.9f; xSize = 0.5f;
		yf = -0.9f; ySize = 0.5f; zf = 0.f;
		quadPos[0].x = xf;           quadPos[0].y = yf;                   quadPos[0].z = zf;
		quadPos[1].x = xf + xSize;   quadPos[1].y = yf;                   quadPos[1].z = zf;
		quadPos[2].x = xf + xSize;   quadPos[2].y = yf + (ySize * 1.53);  quadPos[2].z = zf;
		quadPos[3].x = xf;           quadPos[3].y = yf + (ySize * 1.53);  quadPos[3].z = zf;
		quadUVC[0].u = 0.f;          quadUVC[0].v = 0.f;                  quadUVC[0].col = 0xff0000; //LEF-TOP
		quadUVC[1].u = 1.f;          quadUVC[1].v = 0.f;                  quadUVC[1].col = 0x00ff00; //RIG-TOP
		quadUVC[2].u = 1.f;          quadUVC[2].v = 1.f;                  quadUVC[2].col = 0x00ffff; //RIG-BOT
		quadUVC[3].u = 0.f;          quadUVC[3].v = 1.f;                  quadUVC[3].col = 0xffff00; //LEF-BOT

		quadInd[0].xyzi = 0 + (1 << 31); quadInd[1].xyzi = 1;                 quadInd[2].xyzi = 2;          quadInd[3].xyzi = 3;
		quadInd[0].uvci = 0;			 quadInd[1].uvci = 1;                 quadInd[2].uvci = 2;          quadInd[3].uvci = 3;

		voxie->drawMeshExt((char*)"plane.png", quadPos, 4, quadUVC, 4, quadInd, 4, 2 + vertexShadingFlag, 0x404040);

/*
			// draw a QUAD mesh using make a tile type as a texture
		static tiletype img = { 0 };
		int y = 0, x = 0;
		if (!img.f) { img.x = 512; img.y = 512; img.p = (INT_PTR)img.x * 4; img.f = (INT_PTR)malloc(img.p * img.y); }

		for (y = 0; y < img.y; y++)
			for (x = 0; x < img.x; x++)
			{
				int rgb24 = (128 << 16) + (y << 8) + x, hgt8 = sqrt(256.f * 256.f - (x - 256) * (x - 256) - (y - 256) * (y - 256));
				*(int*)(img.p * y + (x << 2) + img.f) = rgb24 + (hgt8 << 24);
			}

		float xf = -0.8;
		float xSize = 1.3;
		float yf = -0.99;
		float  ySize = 1.3;
		float z = 0;

		poltex_t quadPol[4];
		int quadMesh[5];

		quadPol[0].x = xf;				quadPol[0].y = yf;						quadPol[0].z = z;
		quadPol[1].x = xf + xSize; 		quadPol[1].y = yf; 						quadPol[1].z = z;
		quadPol[2].x = xf + xSize;	 	quadPol[2].y = yf + (ySize * 1.53);		quadPol[2].z = z;
		quadPol[3].x = xf;	 			quadPol[3].y = yf + (ySize * 1.53); 	quadPol[3].z = z;

		quadPol[0].u = 0;				quadPol[0].v = 0;				quadPol[0].col = 0x404040; // LEFT - TOP
		quadPol[1].u = 1;				quadPol[1].v = 0;				quadPol[1].col = 0x404040; // RIGHT - TOP
		quadPol[2].u = 1;				quadPol[2].v = 1;				quadPol[2].col = 0x404040; // RIGHT - BOT
		quadPol[3].u = 0;				quadPol[3].v = 1;				quadPol[3].col = 0x404040; // LEFT - BOT

		// map the mesh 
		quadMesh[0] = 0; 			   quadMesh[1] = 1; 			quadMesh[2] = 2; 			quadMesh[3] = 3; 			quadMesh[4] = -1;

		voxie->drawMesh((char*)&img, quadPol, 4, quadMesh, 5, 2 + 8, 0x404040);
*/








		voxie->showVPS(); //show VPS data and VxCpp.DLL version unto the touch screen.

		voxie->debugText(30, 80, 0xffc080, -1, "ext: %d", drawMode); // display stats
		voxie->debugText(30, 95, 0x00ff00, -1, "DrawmeshTex and DrawmeshTex_ext examples\n\nPress 1 & 2 to switch between draw types. \nPress 3 & 4 to toggle vertex shading"); //instructions 


		voxie->endFrame(); // the end of drawing to the volumetric display.
	}

	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;
	return 0;   // After the program quits the de-constructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}
