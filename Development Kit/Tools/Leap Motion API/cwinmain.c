#if 0
#cwinmain.exe: cwinmain.c; cl cwinmain.c /Ox /G6Fy /MD /link /opt:nowin98 user32.lib gdi32.lib
#   del cwinmain.obj
cwinmain.exe: cwinmain.c; \Dev-Cpp\bin\gcc cwinmain.c -o cwinmain.exe -pipe -Os -s -lgdi32 -lcomdlg32 -mwindows
#NOTE: relies on nmake.exe, no matter which compiler (VC vs. GCC) is selected. To use gcc`s make util:
#      change: `!if 0` to: `ifdef dontdefineme`; and at end, change: `!endif` to: `endif`
!if 0
#endif

#define _CRT_SECURE_NO_WARNINGS //Tell later versions of VC headers (>6) to STFU IMO
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct { INT_PTR f, p, x, y; } tiletype;

#if !defined(max)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//--------------------------------------------------------------------------------------------------
HWND ghwnd = 0;
int xres = 0, yres = 0, mousx = 0, mousy = 0, bstatus = 0, dmousx = 0, dmousy = 0, dmousz = 0, ActiveApp = 1;
char keystatus[256] = {0}, *prognam = (char *)"CWinMain by Ken Silverman";
int shkeystatus = 0, showwindmode = SW_NORMAL;
int progwndflags = (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME);
HCURSOR gcursordef = (HCURSOR)-1;
#define KEYBUFSIZ 256
int keybuf[KEYBUFSIZ], keybufr = 0, keybufw = 0, keybufw2 = 0;
static __int64 gq0, gqper;
typedef struct
{
	HBITMAP hBitmap;
	BITMAPINFO *pbmi;
	INT_PTR ptr, bpl, xsiz, ysiz;
} bitmap_t;
static bitmap_t gscr;

static HMODULE huser32lib;
typedef struct { USHORT usUsagePage, usUsage; DWORD dwFlags; HWND hwndTarget; } KRAWINPUTDEVICE;
typedef struct { DWORD dwType, dwSize; HANDLE hDevice; WPARAM wParam; } KRAWINPUTHEADER;
static BOOL (__stdcall *KRegisterRawInputDevices)(KRAWINPUTDEVICE *pRawInputDevices, UINT uiNumDevices, UINT cbSize);
static UINT (__stdcall *KGetRawInputData)(HANDLE/*HRAWINPUT*/ hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
static void wminput_uninit (void) { if (huser32lib) { FreeLibrary(huser32lib); huser32lib = 0; } }
static int wminput_init (HWND hwnd)
{
	KRAWINPUTDEVICE rid;
	char tbuf[MAX_PATH];

	GetSystemDirectory(tbuf,sizeof(tbuf)-12); strcat(tbuf,"\\user32.dll");
	huser32lib = LoadLibrary(tbuf); if (!huser32lib) { MessageBox(0,"LoadLibrary failed",":/",MB_OK); return(-1); }
	KRegisterRawInputDevices = (BOOL (_stdcall *)(KRAWINPUTDEVICE*,UINT,UINT))GetProcAddress(huser32lib,"RegisterRawInputDevices");
	if (!KRegisterRawInputDevices) return(-1);
	KGetRawInputData = (UINT (_stdcall *)(HANDLE/*HRAWINPUT*/,UINT,LPVOID,PUINT,UINT))GetProcAddress(huser32lib,"GetRawInputData");
	if (!KGetRawInputData) return(-1);

	rid.usUsagePage = 1; //HID_USAGE_PAGE_GENERIC;
	rid.usUsage = 2; //HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = 0x00000100; //RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;
	KRegisterRawInputDevices(&rid,1,sizeof(KRAWINPUTDEVICE));
	return(0);
}

int startdirectdraw (INT_PTR *f, INT_PTR *p, INT_PTR *x, INT_PTR *y)
{
	HDC hdc; static int gcxres, gcyres = -1;

	if ((xres != gcxres) || (yres != gcyres) || (gscr.hBitmap == INVALID_HANDLE_VALUE))
	{
		hdc = GetDC(ghwnd);
		if (gscr.hBitmap != INVALID_HANDLE_VALUE) { DeleteObject(gscr.hBitmap); }
		gcxres = xres; gcyres = yres;
		gscr.xsiz = xres; gscr.ysiz = yres;
		gscr.pbmi = (BITMAPINFO *)LocalAlloc(LMEM_FIXED,sizeof(BITMAPINFO));
		gscr.pbmi->bmiHeader.biSize = sizeof(BITMAPINFO);
		gscr.pbmi->bmiHeader.biWidth = xres;
		gscr.pbmi->bmiHeader.biHeight = -yres;
		gscr.pbmi->bmiHeader.biPlanes = 1;
		gscr.pbmi->bmiHeader.biBitCount = 32;
		gscr.pbmi->bmiHeader.biCompression = BI_RGB;
		gscr.pbmi->bmiHeader.biSizeImage = 0;
		gscr.pbmi->bmiHeader.biXPelsPerMeter = xres;
		gscr.pbmi->bmiHeader.biYPelsPerMeter = yres;
		gscr.pbmi->bmiHeader.biClrUsed = 0;
		gscr.pbmi->bmiHeader.biClrImportant = 0;
		gscr.hBitmap = CreateDIBSection(hdc,gscr.pbmi,DIB_RGB_COLORS,(void **)&gscr.ptr,0,0);
		gscr.bpl = xres*4;
		ReleaseDC(0,hdc);
	}
	(*f) = gscr.ptr; (*p) = gscr.bpl; (*x) = gscr.xsiz; (*y) = gscr.ysiz;
	return(1);
}
void stopdirectdraw (void) {}
void nextpage (void)
{
	HDC hdc; hdc = GetDC(ghwnd);
	StretchDIBits(hdc,0,0,gscr.xsiz,gscr.ysiz,0,0,gscr.xsiz,gscr.ysiz,(void *)gscr.ptr,gscr.pbmi,DIB_RGB_COLORS,SRCCOPY);
	ReleaseDC(0,hdc);
}

long (CALLBACK *peekwindowproc[8])(HWND,UINT,WPARAM,LPARAM) = {0,0,0,0,0,0,0,0};
static LRESULT CALLBACK WindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	for(i=0;peekwindowproc[i];i++) { j = peekwindowproc[i](hwnd,msg,wParam,lParam); if (j >= 0) return(j); }
	switch (msg)
	{
		case WM_SIZE:       if (hwnd == ghwnd) { xres = (lParam&65535); yres = (lParam>>16); } break;
		case WM_MOUSEMOVE:  mousx = (lParam&65535); mousy = (lParam>>16); break;
		case 0x020a:        dmousz = min(max(dmousz+((signed short)HIWORD(wParam)),(signed)0x80010000),(signed)0x7fff0000); break; //WM_MOUSEWHEEL
		case 0x00ff: //WM_INPUT
			{
			unsigned char tbuf[64]; unsigned int u = sizeof(tbuf);
			KGetRawInputData((HANDLE/*HRAWINPUT*/)lParam,0x10000003/*RID_INPUT*/,tbuf,&u,sizeof(KRAWINPUTHEADER));
			if (*(int *)&tbuf/*raw->header.dwType*/ != 0/*RIM_TYPEMOUSE*/) break;
#ifndef _WIN64
			u = 20;
#else
			u = 28;
#endif
			dmousx += *(int *)&tbuf[u+ 8]; //raw->data.mouse.lLastX;
			dmousy += *(int *)&tbuf[u+12]; //raw->data.mouse.lLastY;
			if (tbuf[u]& 1) bstatus |=  1;
			if (tbuf[u]& 2) bstatus &= ~1;
			if (tbuf[u]& 4) bstatus |=  2;
			if (tbuf[u]& 8) bstatus &= ~2;
			if (tbuf[u]&16) bstatus |=  4;
			if (tbuf[u]&32) bstatus &= ~4;
			}
			break;

		case WM_KEYDOWN: case WM_SYSKEYDOWN:
			keystatus[((lParam>>16)&127)+((lParam>>17)&128)] |= 1;
			if ((wParam&0xff) == 0xff) break; //Fixes SHIFT+[ext key] on XP
			switch (lParam&0x17f0000)
			{
				case 0x02a0000: shkeystatus |= (1<<16); break; //0x2a
				case 0x0360000: shkeystatus |= (1<<17); break; //0x36
				case 0x01d0000: shkeystatus |= (1<<18); break; //0x1d
				case 0x11d0000: shkeystatus |= (1<<19); break; //0x9d
				case 0x0380000: shkeystatus |= (1<<20); break; //0x38
				case 0x1380000: shkeystatus |= (1<<21); break; //0xb8
				default:
					{
					int i = ((keybufw2+1)&(KEYBUFSIZ-1));
					keybuf[keybufw2&(KEYBUFSIZ-1)] = (((lParam>>8)&0x7f00)+((lParam>>9)&0x8000))+shkeystatus;
					if (i == keybufr) keybufr = ((keybufr+1)&(KEYBUFSIZ-1)); //lose old keystrokes to prevent fifo overlap
					keybufw2 = i;
					}
			}
			return(0);
		case WM_KEYUP: case WM_SYSKEYUP:
			keystatus[((lParam>>16)&127)+((lParam>>17)&128)] &=~1;
			if ((wParam&0xff) == 0xff) break; //Fixes SHIFT+[ext key] on XP
			switch (lParam&0x17f0000)
			{
				case 0x02a0000: shkeystatus &= ~(3<<16); break; //0x2a
				case 0x0360000: shkeystatus &= ~(3<<16); break; //0x36
				case 0x01d0000: shkeystatus &= ~(1<<18); break; //0x1d
				case 0x11d0000: shkeystatus &= ~(1<<19); break; //0x9d
				case 0x0380000: shkeystatus &= ~(1<<20); break; //0x38
				case 0x1380000: shkeystatus &= ~(1<<21); break; //0xb8
			}
			return(0);
		case WM_SYSCHAR: case WM_CHAR:
			if (keybufw2 != keybufr) //stick ASCII code in last FIFO value
				keybuf[(keybufw2-1)&(KEYBUFSIZ-1)] |= (wParam&255);
			return(0);

		case WM_ACTIVATEAPP: ActiveApp = (BOOL)wParam; shkeystatus = 0; break;
		case WM_DESTROY: PostQuitMessage(0); break;
	}
	return(DefWindowProc(hwnd,msg,wParam,lParam));
}

int cmdline2arg (char *cmdline, char **argv)
{
	int i, j, k, inquote, argc;

		//Convert Windows command line into ANSI 'C' command line...
	argv[0] = (char *)"exe"; argc = 1; j = inquote = 0;
	for(i=0;cmdline[i];i++)
	{
		k = (((cmdline[i] != ' ') && (cmdline[i] != '\t')) || (inquote));
		if (cmdline[i] == '\"') inquote ^= 1;
		if (j < k) { argv[argc++] = &cmdline[i+inquote]; j = inquote+1; continue; }
		if ((j) && (!k))
		{
			if ((j == 2) && (cmdline[i-1] == '\"')) cmdline[i-1] = 0;
			cmdline[i] = 0; j = 0;
		}
	}
	if ((j == 2) && (cmdline[i-1] == '\"')) cmdline[i-1] = 0;
	argv[argc] = 0;
	return(argc);
}

static WNDCLASS wc;
int initapp (HINSTANCE hinst)
{
	RECT rw;
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hInstance = hinst;
	wc.hIcon = LoadIcon(0,IDI_APPLICATION);
	if (gcursordef == (HCURSOR)-1) gcursordef = LoadCursor(0,IDC_CROSS);
	wc.hCursor = gcursordef;
#ifndef __cplusplus
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
#else
	wc.hbrBackground = (HBRUSH__ *)GetStockObject(BLACK_BRUSH);
#endif
	wc.lpszMenuName = wc.lpszClassName = prognam;
	if (!RegisterClass(&wc)) return(0);

	SystemParametersInfo(SPI_GETWORKAREA,0,&rw,0);
	ghwnd = CreateWindowEx(0,prognam,prognam,progwndflags,
		 ((rw.right -rw.left-(xres+GetSystemMetrics(SM_CXSIZEFRAME)*2))>>1) + rw.left,
		 ((rw.bottom-rw.top -(yres+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYSIZEFRAME)*2))>>1) + rw.top,
		 xres+GetSystemMetrics(SM_CXSIZEFRAME)*2,
		 yres+GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYSIZEFRAME)*2,0,0,hinst,0);
	if (!ghwnd) return(0);

	ShowWindow(ghwnd,showwindmode); //SW_NORMAL/SW_MAXIMIZE/etc..
	UpdateWindow(ghwnd);

	memset(keystatus,0,sizeof(keystatus));

	QueryPerformanceFrequency((LARGE_INTEGER *)&gqper);
	QueryPerformanceCounter((LARGE_INTEGER *)&gq0);

	if (wminput_init(ghwnd) < 0) { /*bad*/ return(0); }

	return(1);
}
void uninitapp (void)
{
	DeleteObject(gscr.hBitmap); gscr.hBitmap = (HBITMAP)INVALID_HANDLE_VALUE;
	wminput_uninit();
	DestroyWindow(ghwnd);
	UnregisterClass(wc.lpszClassName,wc.hInstance);
}
int breath (void)
{
	MSG msg;
	while (PeekMessage(&msg,0,0,0,PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return(-1);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	keybufw = keybufw2; //to be safe with multithreads
	return(0);
}
void quitloop (void) { PostMessage(ghwnd,WM_CLOSE,0,0); }
double klock (void)
{
	__int64 q1;
	QueryPerformanceCounter((LARGE_INTEGER *)&q1);
	return(((double)(q1-gq0))/((double)gqper));
}
int keyread (void)
{
	int i;
	if (keybufr == keybufw) return(0);
	i = keybuf[keybufr]; keybufr = ((keybufr+1)&(KEYBUFSIZ-1));
	return(i);
}
//--------------------------------------------------------------------------------------------------
	//Select file..
#pragma comment(lib,"comdlg32.lib")
#include <commdlg.h>
static char exepath[MAX_PATH] = "";
char picselectnam[MAX_PATH] = {0};
char *loadfileselect (char *mess, char *spec, char *defext, char *deffil)
{
	OPENFILENAME ofn =
	{
		sizeof(OPENFILENAME),ghwnd,0,spec,0,0,1,deffil,MAX_PATH,0,0,exepath,mess,
		OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,0,0,defext,0,0,0
	};
	if (!GetOpenFileName(&ofn)) return(0); else return(deffil);
}
//--------------------------------------------------------------------------------------------------

	//NOTE: font is stored vertically first!
static const int font6x8[] = //256 DOS chars, from: DOSAPP.FON (tab blank)
{
	0x00000000,0x3E000000,0x3E455145,0x6F6B3E00,0x1C003E6B,0x1C3E7C3E,0x7E3C1800,0x3000183C,0x30367F36,0x7E5C1800,0x0000185C,0x00001818,0xE7E7FFFF,0x0000FFFF,0x00000000,0xDBDBC3FF,
	0x3000FFC3,0x0E364A48,0x79290600,0x60000629,0x04023F70,0x0A7E6000,0x2A003F35,0x2A1C361C,0x3E7F0000,0x0800081C,0x007F3E1C,0x7F361400,0x00001436,0x005F005F,0x7F090600,0x22007F01,
	0x2259554D,0x60606000,0x14000060,0x14B6FFB6,0x7F060400,0x10000406,0x10307F30,0x3E080800,0x0800081C,0x08083E1C,0x40407800,0x08004040,0x083E083E,0x3F3C3000,0x0300303C,0x030F3F0F,
	0x00000000,0x00000000,0x00065F06,0x00030700,0x24000307,0x247E247E,0x6A2B2400,0x63000012,0x63640813,0x56493600,0x00005020,0x00000307,0x413E0000,0x00000000,0x00003E41,0x1C3E0800,
	0x0800083E,0x08083E08,0x60E00000,0x08000000,0x08080808,0x60600000,0x20000000,0x02040810,0x49513E00,0x00003E45,0x00407F42,0x49516200,0x22004649,0x36494949,0x12141800,0x2F00107F,
	0x31494949,0x494A3C00,0x01003049,0x03050971,0x49493600,0x06003649,0x1E294949,0x6C6C0000,0x00000000,0x00006CEC,0x22140800,0x24000041,0x24242424,0x22410000,0x02000814,0x06095901,
	0x5D413E00,0x7E001E55,0x7E111111,0x49497F00,0x3E003649,0x22414141,0x41417F00,0x7F003E41,0x41494949,0x09097F00,0x3E000109,0x7A494941,0x08087F00,0x00007F08,0x00417F41,0x40403000,
	0x7F003F40,0x41221408,0x40407F00,0x7F004040,0x7F020402,0x04027F00,0x3E007F08,0x3E414141,0x09097F00,0x3E000609,0x5E215141,0x09097F00,0x26006619,0x32494949,0x7F010100,0x3F000101,
	0x3F404040,0x40201F00,0x3F001F20,0x3F403C40,0x08146300,0x07006314,0x07087008,0x45497100,0x00000043,0x0041417F,0x08040200,0x00002010,0x007F4141,0x01020400,0x80800402,0x80808080,
	0x07030000,0x20000000,0x78545454,0x44447F00,0x38003844,0x28444444,0x44443800,0x38007F44,0x08545454,0x097E0800,0x18000009,0x7CA4A4A4,0x04047F00,0x00000078,0x00407D00,0x84804000,
	0x7F00007D,0x00442810,0x7F000000,0x7C000040,0x78041804,0x04047C00,0x38000078,0x38444444,0x4444FC00,0x38003844,0xFC444444,0x44784400,0x08000804,0x20545454,0x443E0400,0x3C000024,
	0x007C2040,0x40201C00,0x3C001C20,0x3C603060,0x10106C00,0x9C00006C,0x003C60A0,0x54546400,0x0800004C,0x0041413E,0x77000000,0x00000000,0x083E4141,0x02010200,0x3C000001,0x3C262326,
	0xE1A11E00,0x3D001221,0x007D2040,0x54543800,0x20000955,0x78555555,0x54552000,0x20007855,0x78545555,0x55572000,0x1C007857,0x1422E2A2,0x55553800,0x38000855,0x08555455,0x55553800,
	0x00000854,0x00417C01,0x79020000,0x00000042,0x00407C01,0x24297000,0x78007029,0x782F252F,0x54547C00,0x34004555,0x58547C54,0x7F097E00,0x38004949,0x00394545,0x44453800,0x39000039,
	0x00384445,0x21413C00,0x3D00007D,0x007C2041,0x60A19C00,0x3D00003D,0x003D4242,0x40413C00,0x1800003D,0x00246624,0x493E4800,0x29006249,0x292A7C2A,0x16097F00,0x40001078,0x02097E88,
	0x55542000,0x00007855,0x00417D00,0x45443800,0x3C000039,0x007D2140,0x0A097A00,0x7A000071,0x00792211,0x55550800,0x4E005E55,0x004E5151,0x4D483000,0x3C002040,0x04040404,0x04040404,
	0x17001C04,0x506A4C08,0x34081700,0x0000782A,0x00307D30,0x00140800,0x14001408,0x08140008,0x11441144,0x55AA1144,0x55AA55AA,0xEEBBEEBB,0x0000EEBB,0x0000FF00,0xFF080808,0x0A0A0000,
	0x0000FF0A,0xFF00FF08,0xF8080000,0x0000F808,0xFE0A0A0A,0xFB0A0000,0x0000FF00,0xFF00FF00,0xFA0A0000,0x0000FE02,0x0F080B0A,0x0F080000,0x00000F08,0x0F0A0A0A,0x08080000,0x0000F808,
	0x0F000000,0x08080808,0x08080F08,0xF8080808,0x00000808,0x0808FF00,0x08080808,0x08080808,0x0808FF08,0xFF000000,0xFF000A0A,0x0808FF00,0x0B080F00,0xFE000A0A,0x0A0AFA02,0x0B080B0A,
	0xFA0A0A0A,0x0A0AFA02,0xFB00FF00,0x0A0A0A0A,0x0A0A0A0A,0xFB00FB0A,0x0A0A0A0A,0x0A0A0B0A,0x0F080F08,0x0A0A0808,0x0A0AFA0A,0xF808F808,0x0F000808,0x08080F08,0x0F000000,0x00000A0A,
	0x0A0AFE00,0xF808F800,0xFF080808,0x0808FF00,0xFB0A0A0A,0x08080A0A,0x00000F08,0xF8000000,0xFFFF0808,0xFFFFFFFF,0xF0F0F0F0,0xFFFFF0F0,0x000000FF,0xFF000000,0x0F0FFFFF,0x0F0F0F0F,
	0x24241800,0xFE002418,0x00344A4A,0x01017F00,0x02000003,0x027E027E,0x49556300,0x18000063,0x041C2424,0x2020FC00,0x0800001C,0x00047804,0x77550800,0x3E000855,0x003E4949,0x02724C00,
	0x22004C72,0x00305955,0x18241800,0x18001824,0x18247E24,0x2A2A1C00,0x3C00002A,0x003C0202,0x2A2A2A00,0x0000002A,0x00242E24,0x4A4A5100,0x44000044,0x00514A4A,0xFC000000,0x20000402,
	0x00003F40,0x2A080800,0x24000808,0x00122412,0x09090600,0x00000006,0x00001818,0x00080000,0x30000000,0x02023E40,0x010E0100,0x0900000E,0x00000A0D,0x3C3C3C00,0x0000003C,0x00000000,
};
void print6x8 (tiletype *dd, int ox, int y, int fcol, int bcol, const char *fmt, ...)
{
	va_list arglist;
	char st[1024], *c, *v;
	int i, j, ie, x;
	int *lp, *lpx;

	if (!fmt) return;
	va_start(arglist,fmt);
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
	va_end(arglist);

	lp = (int *)(y*dd->p+dd->f);
	for(j=1;j<256;y++,lp=(int *)(((INT_PTR)lp)+dd->p),j+=j)
		if ((unsigned)y < (unsigned)dd->y)
			for(c=st,x=ox;*c;c++,x+=6)
			{
				v = ((int)(*c))*6 + ((char *)font6x8); lpx = &lp[x];
				for(i=max(-x,0),ie=min(dd->x-x,6);i<ie;i++) { if (v[i]&j) lpx[i] = fcol; else if (bcol >= 0) lpx[i] = bcol; }
				if ((*c) == 9) { if (bcol >= 0) { for(i=max(-x,6),ie=min(dd->x-x,18);i<ie;i++) lpx[i] = bcol; } x += 2*6; }
			}
}

void drawpix (tiletype *dd, int x, int y, int col)
{
	if (((unsigned)x < (unsigned)dd->x) && ((unsigned)y < (unsigned)dd->y))
		*(int *)(dd->p*y + (x<<2) + dd->f) = col;
}

void drawhlin (tiletype *dd, int x0, int x1, int y, int col)
{
	int x, *iptr;

	if ((unsigned)y >= (unsigned)dd->y) return;
	x0 = max(x0,0); x1 = min(x1,dd->x); if (x0 >= x1) return;
	iptr = (int *)(dd->p*y + dd->f);
	for(x=x0;x<x1;x++) iptr[x] = col;
}

void drawrectfill (tiletype *dd, int x0, int y0, int x1, int y1, int col)
{
	int x, y, *iptr;

	x0 = max(x0,0); x1 = min(x1,dd->x); if (x0 >= x1) return;
	y0 = max(y0,0); y1 = min(y1,dd->y); if (y0 >= y1) return;
	iptr = (int *)(dd->p*y0 + dd->f);
	for(y=y0;y<y1;y++,iptr=(int *)(((INT_PTR)iptr)+dd->p))
		for(x=x0;x<x1;x++) iptr[x] = col;
}

void drawline (tiletype *dd, float x0, float y0, float x1, float y1, int col)
{
	float f;
	int i, x, y, ipx[2], ipy[2];

		  if (x0 <   0.f) { if (x1 <   0.f) return; y0 = (         0.f-x0)*(y1-y0)/(x1-x0)+y0; x0 =          0.f; }
	else if (x0 > dd->x) { if (x1 > dd->x) return; y0 = ((float)dd->x-x0)*(y1-y0)/(x1-x0)+y0; x0 = (float)dd->x; }
		  if (y0 <   0.f) { if (y1 <   0.f) return; x0 = (         0.f-y0)*(x1-x0)/(y1-y0)+x0; y0 =          0.f; }
	else if (y0 > dd->y) { if (y1 > dd->y) return; x0 = ((float)dd->y-y0)*(x1-x0)/(y1-y0)+x0; y0 = (float)dd->y; }
		  if (x1 <   0.f) {                         y1 = (         0.f-x1)*(y1-y0)/(x1-x0)+y1; x1 =          0.f; }
	else if (x1 > dd->x) {                         y1 = ((float)dd->x-x1)*(y1-y0)/(x1-x0)+y1; x1 = (float)dd->x; }
		  if (y1 <   0.f) {                         x1 = (         0.f-y1)*(x1-x0)/(y1-y0)+x1; y1 =          0.f; }
	else if (y1 > dd->y) {                         x1 = ((float)dd->y-y1)*(x1-x0)/(y1-y0)+x1; y1 = (float)dd->y; }

	x1 -= x0; y1 -= y0;
	i = (int)max(fabs(x1),fabs(y1)); if (!(i&0x7fffffff)) return;/*FUK:32-bit!*/
	f = 65536.f/(float)i;
	ipx[0] = (int)(x0*65536.0); ipx[1] = (int)(x1*f);
	ipy[0] = (int)(y0*65536.0); ipy[1] = (int)(y1*f);
	for(;i>0;i--)
	{
		x = (ipx[0]>>16); y = (ipy[0]>>16);
		if (((unsigned)x < (unsigned)dd->x) && ((unsigned)y < (unsigned)dd->y))
			*(int *)(dd->p*y + (x<<2) + dd->f) = col;
		ipx[0] += ipx[1]; ipy[0] += ipy[1];
	}
}

void drawcirc (tiletype *dd, int xc, int yc, int r, int col)
{
	int x, y, d;
	for(x=0,y=r,d=1-r;1;x++)
	{
		drawpix(dd,xc+x,yc+y,col); drawpix(dd,xc+y,yc+x,col);
		drawpix(dd,xc+y,yc-x,col); drawpix(dd,xc+x,yc-y,col);
		drawpix(dd,xc-x,yc+y,col); drawpix(dd,xc-y,yc+x,col);
		drawpix(dd,xc-y,yc-x,col); drawpix(dd,xc-x,yc-y,col);
		if (x >= y) break;
		if (d < 0) { d += ( x   <<1)+3; }
				else { d += ((x-y)<<1)+5; y--; }
	}
}

void drawcircfill (tiletype *dd, int xc, int yc, int r, int col)
{
	int i, y;
	for(y=yc-r;y<=yc+r;y++)
	{
		i = (int)sqrt((double)(r*r - (yc-y)*(yc-y)));
		drawhlin(dd,xc-i,xc+i,y,col);
	}
}

//--------------------------------------------------------------------------------------------------

#if 0
!endif
#endif
