#if 0 //To compile, type "nmake playwav.c"
playwav.exe: playwav.c; cl playwav.c /O1 /MD /nologo /link winmm.lib /opt:nowin98
	del playwav.obj
!if 0
#endif

	//Last touch before adding mciSendString stuff:
	//11/29/2005  11:22p  680By  playwav.c

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define USEWINMAIN 0

#if (USEWINMAIN == 0)
int main (int argc, char **argv)
#else
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hprevinst, LPSTR cmdline, int cmdshow)
#endif
{
	char buf[512];
	int i;

#if (USEWINMAIN == 0)
	char *cmdline = argv[1]; if (argc < 2) return 0; for(i=0;cmdline[i];i++);
#endif
	if (cmdline[0] == '\"') { cmdline++; for(i=0;cmdline[i];i++); cmdline[i-1] = 0; }
							 else {            for(i=0;cmdline[i];i++); }

#if 0
	sndPlaySound(cmdline,SND_SYNC); //Supports WAV only!
	return 0;
#else
		//https://stackoverflow.com/questions/22253074/how-to-play-or-open-mp3-or-wav-sound-file-in-c-program
		//Supoprts: WAV, MP3, MP2, WMA, MID, ..
	memcpy(&buf[0],"open \"",6);
	memcpy(&buf[6],cmdline,i);
	memcpy(&buf[6+i],"\" type mpegvideo alias mp3",27);

	if (mciSendString(buf,0,0,0)) { printf("Fail!\n"); return 1; }
	mciSendString("play mp3 wait",0,0,0);
	mciSendString("close mp3",0,0,0);
	printf("Done.\n"); return 0;
#endif
}

#if 0
!endif
#endif
