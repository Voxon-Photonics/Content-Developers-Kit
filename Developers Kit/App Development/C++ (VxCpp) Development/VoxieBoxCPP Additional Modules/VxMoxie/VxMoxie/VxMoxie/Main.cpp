#include "vxCpp.h"
#include "VxMoxie.h"


void drawMoxieObjects(VxMoxie* moxie, IVoxieBox * voxie) {

		int i, j;
		double tim = voxie->getTime();

		for (i = -1; i <= 1; i += 2)
		{
			point3d p = { (float)i * 0.5,0.0,0.0 }, r = { 1.f,0.f,0.f }, d = { 0.f,1.f,0.f }, f = { 0.f,0.f,1.f }; moxie->drawModel((char*)"gfx/caco.kv6", &p, &r, &d, &f, 0x404040);
		}

		moxie->drawSphere( 0.f, -.5f, 0.f, 0.1f, 1, 0x808080);
		moxie->drawCone( .0f, -.5f, +.0f, 0.08f, +.4f, -.4f - .5f, +.4f, 0.f, 1, 0x404040);
		moxie->drawLine(.0f, -.5f, +.0f, -.4f, -.4f - .5f, -.4f, 0xffff00);
		
		{
			poltex_t vt[4]; int mesh[256]; i = 0;
			vt[0].x = -0.4; vt[0].y = -0.4 - .5f; vt[0].z = -0.4; vt[0].col = 0xffffff;
			vt[1].x = -0.4; vt[1].y = +0.4 - .5f; vt[1].z = +0.4; vt[1].col = 0xffffff;
			vt[2].x = +0.4; vt[2].y = -0.4 - .5f; vt[2].z = +0.4; vt[2].col = 0xffffff;
			vt[3].x = +0.4; vt[3].y = +0.4 - .5f; vt[3].z = -0.4; vt[3].col = 0xffffff;
			mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = -1; mesh[i++] = 1; mesh[i++] = 3;
			moxie->drawMesh( 0, vt, 4, mesh, i, 1, 0xffffff);
		}

		for (i = 1; i <= 7; i++)
		{
			float fx, fy, fz;
			int col;
			fx = ((float)i - 4.f) * .25f; fy = .7f; fz = sin(tim + (float)i) * (voxie->getAspectZ() - .1f);
			col = ((i & 1) + (((i & 2) >> 1) << 8) + (((i & 4) >> 2) << 16)) * 255;
			moxie->drawBox( fx - .1, fy - .1, fz - .1,
				fx + .1, fy + .1, fz + .1, 1, col);
			if (i != 3) continue;
			for (j = 1024; j > 0; j--)
			{
				moxie->drawVox(fx + (float)((rand() & 32767) - 16384) / 16383.f * .1f,
					fy + (float)((rand() & 32767) - 16384) / 16383.f * .1f,
					fz + (float)((rand() & 32767) - 16384) / 16383.f * .1f,
					col);
			}
		}

		{
			point3d p, r, d, f;
			r.x = cos(tim) * .5f; d.x = sin(tim) * .5f; f.x = 0.f; p.x = .7f - (r.x + d.x + f.x) * .5f;
			r.y = sin(tim) * -.5f; d.y = cos(tim) * .5f; f.y = 0.f; p.y = -.6f - (r.y + d.y + f.y) * .5f;
			r.z = 0.f; d.z = 0.f; f.z = .5f; p.z = +.0f - (r.z + d.z + f.z) * .5f;
			moxie->drawCube(&p, &r, &d, &f, 1, 0x808080);
		}

		{
			point3d p, r, d, f;
			r.x = cos(tim) * .05f; d.x = sin(tim) * .15f; p.x = .7f - r.x * 4.f - d.x * .5f;
			r.y = sin(tim) * -.05f; d.y = cos(tim) * .15f; p.y = -.6f - r.y * 4.f - d.y * .5f;
			r.z = 0.f; d.z = 0.f; p.z = +.0f - (r.z + d.z) * .5f;
			moxie->drawTextExt(&p, &r, &d, 0xc0c0c0, "hello %i", rand() & 63);
		}

		point3d gcamr = moxie->getCamRVec();
		point3d gcamd = moxie->getCamDVec();


		if ((fabs(gcamr.z) == 0.f) && (fabs(gcamd.z) == 0.f)) //NOTE: heightmaps don't currently support rotation off the x-y plane ;P
		{
			point3d p, r, d, f;
			r.x = .5f; d.x = 0.f; f.x = 0.f; p.x = -.7f - (r.x + d.x + f.x) * .5f;
			r.y = 0.f; d.y = .5f; f.y = 0.f; p.y = -.6f - (r.y + d.y + f.y) * .5f;
			r.z = 0.f; d.z = 0.f; f.z = -.1f; p.z = +.0f - (r.z + d.z + f.z) * .5f;
			moxie->drawHeightMap((char*)"gfx/canyon.png", &p, &r, &d, &f, -1, 0, (1 << 1) + (1 << 2));
		}

}

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();

	VxMoxie moxie(voxie);
	voxie->setBorder(true);

	while (voxie->breath())
	{
		voxie->startFrame();

		if (voxie->getKeyOnDown(KB_Equals)) moxie.setMovementSpeed(moxie.getMovementSpeed() + 0.1);
		if (voxie->getKeyOnDown(KB_Minus)) moxie.setMovementSpeed(moxie.getMovementSpeed() - 0.1);


		if (voxie->getKeyIsDown(KB_Q)) moxie.zoomCamIn();
		if (voxie->getKeyIsDown(KB_E)) moxie.zoomCamOut();

		if (voxie->getKeyIsDown(KB_A)) moxie.rotCam(1,0,0);
		if (voxie->getKeyIsDown(KB_D)) moxie.rotCam(-1,0,0);
		if (voxie->getKeyIsDown(KB_W)) moxie.rotCam(0,1,0);
		if (voxie->getKeyIsDown(KB_S)) moxie.rotCam(0,-1,0);
		if (voxie->getKeyIsDown(KB_Z)) moxie.rotCam(0,0,1);
		if (voxie->getKeyIsDown(KB_C)) moxie.rotCam(0,0,-1);

		if (voxie->getKeyOnDown(KB_N)) moxie.setRotation(90, 0, 0, true);
		if (voxie->getKeyOnDown(KB_M)) moxie.setRotation(-90, 0, 0, true);



		if (voxie->getKeyIsDown(KB_Shift_Left) || voxie->getKeyIsDown(KB_Shift_Right)) {
			if (voxie->getKeyIsDown(KB_Arrow_Left)) moxie.movePivot(1, 0, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Right)) moxie.movePivot(-1, 0, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Up)) moxie.movePivot(0, 1, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Down)) moxie.movePivot(0, -1, 0);
			if (voxie->getKeyIsDown(KB_NUMPAD_Minus)) moxie.movePivot(0, 0, 1);
			if (voxie->getKeyIsDown(KB_NUMPAD_Plus)) moxie.movePivot(0, 0, -1);


		}
		else {
			if (voxie->getKeyIsDown(KB_Arrow_Left)) moxie.moveCam(1, 0, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Right)) moxie.moveCam(-1, 0, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Up)) moxie.moveCam(0, 1, 0);
			if (voxie->getKeyIsDown(KB_Arrow_Down)) moxie.moveCam(0, -1, 0);
			if (voxie->getKeyIsDown(KB_NUMPAD_Minus)) moxie.moveCam(0, 0, 1);
			if (voxie->getKeyIsDown(KB_NUMPAD_Plus)) moxie.moveCam(0, 0, -1);
		}

		voxie->debugText(33, 80, 0xffffff, -1, "VxMoxie Test Keyboard Layout\nArrows + Numpad - and + = Move camera\n W,S,A,D,Z,C = Rotate camera\n hold SHIFT + Arrows = Move pivot point");


		drawMoxieObjects(&moxie, voxie);
		moxie.drawPivot(0.1, 0x00ff00, 0);

		moxie.report(50, 300);
		voxie->showVPS();
		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
