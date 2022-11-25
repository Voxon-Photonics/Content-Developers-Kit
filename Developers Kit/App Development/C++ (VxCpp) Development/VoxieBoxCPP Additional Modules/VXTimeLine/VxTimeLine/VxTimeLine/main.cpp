#include "vxCPP.h"
#include "VxTimeLine.h"


int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();


	VxTimeLine timeLine;
	point3d pp = { -1, -.9, 0 };
	point3d rr = { 0.1, 0, 0 };
	point3d dd = { 0, 0.1, 0 };
	timeLine.setStartTime("2020-06-10|10:00:00");
	timeLine.setStopTime("2020-06-11|10:00:00");
	timeLine.setCurrentTime("2020-06-10|10:00:20");


	while (voxie->breath())
	{
		voxie->startFrame();



		timeLine.update(voxie);



		voxie->debugText(100, 120, 0x00fffff, -1, "date 1 %d", timeLine.isWithinTimeLine("2020-06-10|09:59:00", "2019-06-10|10:00:00", "2022-06-20|10:00:00"));
		voxie->debugText(100, 140, 0x00fffff, -1, "date 2 %d", timeLine.isWithinTimeLine("2021-06-10|11:00:00", "2020-06-10|10:00:00", "2020-06-20|10:00:00"));


		if (voxie->getKeyOnDown(KB_Space_Bar)) {





			timeLine.setPlaying(!timeLine.isPlaying());


		}

		if (voxie->getKeyOnDown(KB_A)) {

			timeLine.amendTime(timeLine.getCurrentTime(), -60);



		}

		if (voxie->getKeyOnDown(KB_B)) {

			timeLine.amendTime(timeLine.getCurrentTime(), -20);



		}


		voxie->showVPS();
		timeLine.report(100, 100, voxie);
		
		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
