#pragma once
/** \file vxDataTypes.h
 *
 * VX++ data types header contains definitions of all Voxon related structs and types.
 * This header is essential to developing VX++ applications using the VxCpp.DLL.
 * This file is kept separate to allow easy integration...
 *
 */


#ifndef VX_DATA_TYPES
#define VX_DATA_TYPES

#include <windows.h>
#include <memory>

#pragma pack(push,1)



//! 2 floats (x & y) usually to describe a point in 2D
typedef struct point2d {

	float x, //!< x position
		  y; //!< y position
		//function scale (multiply) values
	void scale(float value) {
		x *= value; y *= value;
	}

} point2d;

//! 3 floats (x, y & z) usually to describe a point in 3D
typedef struct point3d {

	float x, //!< x position
		y, //!< y position
		z; //!< z position
		//! scale (multiply) values
	void scale(float value) {
		x *= value; y *= value; z *= value;
	}
	//! amend all values (add a value to each)
	void amend(float addX, float addY, float addZ) {
		x += addX; y += addY; z += addZ;
	}
	//! amend all values with the same value
	void amendAll(float value) {
		x += value; y += value; z += value;
	}

} point3d;

//! point3d with index to next point in loop (starting at 0). Similar to poltex_t but with no texture data. Used by VoxieBox::drawPoly()
typedef struct pol_t { 
	
	float	x,			//!< x position
			y,			//!< y position
			z;			//!< z position
	int		p2;			//!< next index in the pol_t array jump to @see VoxieBox::drawPoly()

} pol_t;

//!  3D point with texture coordinate and color (ARGB32). Used by VoxieBox::drawMesh()
typedef struct poltex_t { 
	
	float	x,			//!< x position
			y,			//!< y position
			z,			//!< z position
			u,			//!< u texture  
			v;			//!< v texture 
	
	int		col;		//!< color value ARGB32 

} poltex_t;

//! 2D array of pixels can be used with VoxieBox::drawMesh, VoxieBox::drawHeightMap and the 2D display (via voxie_frame_t)
typedef struct tiletype { 

	INT_PTR		f; 		//!< pointer to the top-left pixel ( (INT_PTR)malloc(tile.p*tile.y) - good formula for working it out )	
	INT_PTR		p;		//!< pitch - number of bytes per horizontal line (usually 32 bit color (ARGB32) x*4 but may be higher or negative) ( tile.p = (tile.x<<2) - good formula))
	INT_PTR		x;		//!< x (horizontal) image dimension  (look at the file's details)
	INT_PTR		y;		//!< y (vertical) image dimension (look at the file's details)

} tiletype;

#define MAXDISP 3			//the maximum amount of displays which can render a volumetric volume at any point in time.
#define VOXIE_DICOM_MIPS 4	//NOTE:limited to 6 by dicom_gotz

//! Voxon Display Struct - Configuration for a display suitable to render a volumetric image (usually projector)
/**
* This struct manages a projector's display settings which includes keystone (the position coordinates for the display), LED levels
* and mirror settings. Each display struct (voxie_disp_t) is loaded into the voxie window (voxie_wind_t) during the voxie_load() and voxie_init() or when a new 
* VoxieBox is constructed 
* @see VoxieBox::VoxieBox() 
* @see VoxieBox::init()
*/
typedef struct voxie_disp_t {

	point2d keyst[8];	//!< Keystone settings for display. a collection of 2D points giving coordinates for top and bottom corners of the display
	int colo_r,			//!<LED initial start up values for the red channel in RGB color mode. 0..255
		colo_g,			//!<LED initial start up values for the green channel in RGB color mode. 0..255
		colo_b,			//!<LED initial start up values for the blue channel in RGB color mode. 0..255
		mono_r,			//!<LED initial start up values for the red channel in monochrome (single color) mode. 0..255
		mono_g,			//!<LED initial start up values for the red channel in monochrome (single color) mode. 0..255
		mono_b,			//!<LED initial start up values for the red channel in monochrome (single color) mode. 0..255
		mirrorx,		//!<Horizontal mirror setting value. Uses projector hardware flipping. 0 is for non-mirrored image. 1 is for mirrored image	
		mirrory;		//!<Vertical mirror setting value. Uses projector hardware flipping. 0 is for non-mirrored image. 1 is for mirrored image
	
} voxie_disp_t;

//! Voxie Window Struct - Configuration for the Voxon Window. This struct manages all the settings to do with the volume.
/** Most settings to do with creating a volume are contained here. This struct is passed in when VoxieBox::init() is called. 
 *  Most of these settings can be updated while the volume is rendering. Pass the voxie_wind_t struct through VoxieBox::update() 
 *  for the settings to be applied to the current volume.
 *  @see VoxieBox::update()
 *  @see VoxieBox::voxie_disp_t()
 */
typedef struct voxie_wind_t {

	// Emulator and display settings 
	int		useemu;					//!< value determines to render the Voxie Window on hardware or via software emulation or simulation. 
									//!< set values are: 
									//!< 0 to run on Voxon hardware,
									//!< 1 = emulator perspective view (looks like authentic hardware),
									//!< 2 = simulator perspective view, 
									//!< 3 = emulator orthogonal view, 
									//!< 4 = emulator orthogonal view using older, slower code. 
			
	float	emuhang,				//!< emulator view horizontal angle expressed as radians
			emuvang,				//!< emulator view vertical angle expressed as radians 
			emudist;				//!< distance for the emulator's viewpoint range is {0..2000.0} 0 = close, 2000 = furthest  
	
	int		xdim,					//!< projector's x resolution dimensions currently needs to be 912 
			ydim;					//!< projector's y resolution dimensions currently needs to be 1140 		

	int		projrate;				//!< projector's refresh rate in Hz range is {60..107} 
	int		framepervol;			//!< number of projector's frames per volume {1..16}. example with framepervol set to 3 at projrate set to 60 render a volume at 60/3 = 20 Hz volume rate. This setting should always be at '8' on a typical VX1 setup. 60/8 =  
	int		usecol;					//!< display color setting. 0 = mono white, 1 = RGB full color time multiplexed. various negative values are for full color mono -1 = red, -2 = green, -3 yellow,  -4 blue, -5 magenta, -6 cyan, 
	int		dispnum;				//!< number of displays to search for and use for volumetric rendering. a VX1 only uses 1 display but it is possible to create a volume using multiple displays. 
	int		bitspervol;				//!< bit planes per volume written by the VoxieBox.dll. Can be useful for custom volume renderer 


	voxie_disp_t disp[MAXDISP];		//!< An array of voxie_disp_t. These are the different display settings for each display targeted for volumetric rendering. These 'displays' are typically projectors. See voxie_disp_t for infomation of what is within this struct. 
    

	// Actuator and motor settings 

	int		hwsync_frame0,			//!< first frame (frame is a 2D slice, a volume is created by a stack of 2D frames / slices) offset (-1 to disable sync hw). For hardware experts only let the software manage this	
			hwsync_phase,			//!< high precision phase offset. For hardware experts only let the software manage this
			hwsync_amp[4],			//!< amplitude {0..65536} for each channel.  For hardware experts only let the software manage this
			hwsync_pha[4],			//!< phase {0..65536} for each channel. For hardware experts only let the software manage this
			hwsync_levthresh,		//!< threshold ADC value for peak detection {0..1024, but typically in range: 256..512} use Host.exe to test the right level
			voxie_vol;				//!< amplitude scale when using sine wave audio output {0..100}. The higher the value the more max amplitude can be sent to the motors               


	// Rendering 

	int		ilacemode,				//!< interlace mode: 0:default, 6:2/3 color mode;uses outcol[]) {0-7?}
			drawstroke,				//!< determines the drawing / rending type. 1=draw on up stroke, 2=draw on down stroke, 3=draw on both up&down
			dither,					//!< enable / disable dither effects 0=no dither, 1=height dither, 2=time dither, 3=height&time dither
			smear,					//!< enable / disable smear 1+=increase brightness in x-y at post processing (slower render), 0=not
			usekeystone,			//!< 0=no keystone compensation (for testing only), 1=keystone quadrilateral compensation (default) @see voxie_disp_t
			flip,					//!< flip coordinates in software (via voxiebox.dll) 0 = no flip, 1 = flip 90 degrees, 2 = flip 180 degrees, 3 = flip 270 degrees 
			menu_on_voxie;			//!< 1 = display menu on voxiebox view
	
	float	aspx,					//!< aspect ratio of x dimension, loaded from voxiebox.ini, used by application. Values typically around 1.f
			aspy,					//!< aspect ratio of y dimension, loaded from voxiebox.ini, used by application. Values typically around 1.f
			aspz,					//!< aspect ratio of z dimension, loaded from voxiebox.ini, used by application. Values typically around .4f
			gamma,					//!< gamma value for interpolating many graphical draw calls
			density;				//!< scale factor controlling dot density of graphical draw calls (default:1.f)
	

	//Audio Settings 

	int		sndfx_vol,				//!< amplitude scale of sound effects {0..100}
			voxie_aud,				//!< audio channel index of motor. (Playback devices..Speakers..Configure..Audio Channels)
			excl_audio,				//!< enable / disable exclusive audio mode 1=exclusive audio mode (much faster & more stable sync - recommended!
			sndfx_aud[2],			//!< audio channel indices of sound effects, [0]=left, [1]=right
			playsamprate,			//!< sample rate used by audio driver (written by voxie_init()). 0 is written if no audio channels are enabled in voxiebox.ini.
			playnchans,				//!< number of audio channels expected to be rendered by user audio mixer
			recsamprate,			//!< recording sample rate - to use, must write before voxie_init()
			recnchans;              //!< number of audio channels in recording callback
	

	// Misc 1/2 

	int		isrecording,			//!< 0=normally, 1 when .REC file recorder is in progress (written by voxiebox.DLL)
			hacks,					//!< bit0!=0:exclusive mouse; bit1!=0:disable REC/TCP options under File menu (for Voxieplay)
			dispcur,				//!< current display selected in menus {0..dispnum-1}
			sndfx_nspk,				//!< variable to hold the number of physical speakers {1 = VX1 default, 2 = stereo system}
			reserved;				//!< reserved variable for future features
	// Obsolete 

	float	freq,					//!< starting value in Hz (must be set before first call to voxie_init()); obsolete - not used by current hardware
			phase;					//!< phase lock {0.0..1.0} (can be updated on later calls to voxie_init()); obsolete - not used by current hardware


	// Misc 2/2 

	int		thread_override_hack;	//!< 0:default thread behavior, 1..n:force n threads for voxie_drawspr()/voxie_drawheimap(); bound to: {1 .. #CPU cores (1 less on hw)}
	int		motortyp;				//!< 0=DCBrush+A*, 1=CP+FreqIn+A*, 2=BL_Airplane+A*, 3=VSpin1.0+CP+FreqIn, 4=VSpin1.0+BL+A4915, 5=VSpin1.0+BL+WS2408
	int		clipshape;				//!< don't render voxels outside of shape. 0 for a rectangle (for VX1) and 1 for a circle (for Spinner) 0=rectangle (vw.aspx,vw.aspy), 1=circle (vw.aspr)

	int		goalrpm,				//!< for Spinner only. The goalrpm rate for the motor to spin up to. 
			cpmaxrpm,				//!< for Spinner only. Maximum rotation rate the voxiebox.dll will allow the motor to spin
			ianghak,				//!< for Spinner only. Phase offset for rotation (0-65535)
			ldotnum,				//!< dot number the size of a single voxel. affects brightness. Range:{0..3}, 0=default
			normhax;				//!< Used by global normal shading stores the vertical and horizontal angle and amplitude : (vang/*-90..90*/<<20) + ((hang/*-180..180*/&4095)<<8) + amp/*0..255*/. 0=disable.
	
	int		upndow;					//!< screen shape: 0=sawtooth, 1=triangle
	int		nblades;				//!< screen shape: 0=VX1 (not spinner), 1=/|, 2=/|/|, ..
	int		usejoy;					//!< usb gamepad input API -1=none, 0=joyInfoEx (directInput), 1=XInput
	int		dimcaps;				//!< dim top & bottom by sine - up/dn (VX1) mode only. 0 is default.
	float	emugam;					//!< gamma applied to emulator rendering mode #1. Typical range: 0.25-4.0
	float	asprmin;				//!< minimum radius; typically 0.10
	float	sync_usb_offset;		//!< offset to USB timing (see voxiebox menu Stats..On:PDF)

	int		sensemask[3],			//!< for multicolor/multiprojector modes: which data goes to projector. Ex (R/B mode): [0]=0xff0000, [1]=0x00ff00
			outcol[3];				//!< for multicolor/multiprojector modes: LED color of projector        Ex (R/B mode): [0]=0xff0000, [1]=0x00ff00

	float	aspr,					//!< size of radius; typically 1.41 or sqrt(2) to circumscribe square
			sawtoothrat;			//!< //for /|/| or /| screens; typically: 0.875, range: ~0.5-1.0


} voxie_wind_t;

//! \def VOXIEFRAME_FLAGS_BUFFERED
#define VOXIEFRAME_FLAGS_BUFFERED 0 //!< default (buffers voxie gfx commands internally for potential speedup)
//! \def VOXIEFRAME_FLAGS_IMMEDIATE
#define VOXIEFRAME_FLAGS_IMMEDIATE 1 //!< use this if calling voxie gfx from multiple threads (ex: custom user multithread)


//! Struct which holds all the frame data (frame is a 2D slice of the volumetric image) which gets loaded unto the projector. 
/** It also holds the tiletype data which gets rendered to the 2D secondary screen (touch screen) 
*/
typedef struct voxie_frame_t {

	INT_PTR		f,				//!< Pointer to top-left-up of current frame to draw
				p,				//!< Number of bytes per horizontal line (x)
				fp;				//!< Number of bytes per 24-plane frame (1/3 of screen)
	
	int			x,				//!< Width of viewport
				y,				//!< Height of viewport
				flags,			//!< @see VOXIEFRAME_FLAGS_BUFFERED @see VOXIEFRAME_FLAGS_IMMEDIATE
				drawplanes,		//!< Number of bit planes used for drawing
				x0,				//!< Viewport extents
				y0,				//!< Viewport extents
				x1,				//!< Viewport extents
				y1;				//!< Viewport extents

	float		xmul,			//!< Transform for medium and high level graphics functions..
				ymul,			//!< Transform for medium and high level graphics functions..
				zmul,			//!< Transform for medium and high level graphics functions..
				xadd,			//!< Transform is: actual_x = passed_x * xmul + xadd
				yadd,			//!< Transform is: actual_y = passed_y * ymul + yadd
				zadd;			//!< Transform is: actual_z = passed_z * zmul + zadd

	tiletype	 f2d;			//!< image for 2D debug window. (update between VoxieBox::startFrame() & VoxieBox::endFrame() )

} voxie_frame_t;

// Input Devices

//! Struct to manage mouse inputs. Mouse state is updated when VoxieBox::breath() is called.  
/**
*	mouse and keyboard input are managed by VoxieBox class there are many function calls which 
*	can be used to interact with the mouse (and keyboard) @see VoxieBox to see them all.  	
*	for convenience VoxieBox privately tracks the mousePosition @see  VoxieBox::getMousePosition() 
*   as the voxie_input_t just tracks movements
*   use @see VoxieBox::reportMouse to see a live report
*/
typedef struct voxie_input_t { 

	int			bstat,			//!< buttonstate for mouse buttons 0 = no buttons pressed, 1 = left mouse button pressed, 
								//!< 2 = right mouse button pressed, 3 = both left and right button pressed. 
								//!< @see VoxieBox::getMouseButtonState @see VoxieBox::getMouseClickOnDown
				obstat,			//!< the previous button state compare this value to the current bstat to do your own 'On Down' or 'On Up' calls or use VoxieBox::getMouseClickOnDown & VoxieBox::getMouseClickOnUp 
				dmousx,			//!< delta X movement from mouse 
				dmousy,			//!< delta Y movement from mouse
				dmousz;			//!< delta Z movement from mouse (scroll wheel)

} voxie_inputs_t;

//! USB game controller input state struct. Used to manage USB game controller input
typedef struct voxie_xbox_t {

	short	but,	//!< XBox controller buttons (same layout as XInput) @see JoyButtonCodes 
			lt,		//!< XBox controller left analog trigger (0..255)
			rt,		//!< XBox controller right analog trigger (0..255)
			tx0,	//!< XBox controller left analog stick x axis (-32768..32767)
			ty0,	//!< XBox controller left analog stick y axis (-32768..32767)
			tx1,	//!< XBox controller right analog stick x axis (-32768..32767)
			ty1,	//!< XBox controller right analog stick y axis (-32768..32767)
			hat;	//!< Hat (some directional pads use this) input state (for supporting DirectInput controllers)

} voxie_xbox_t;


//! 3D SpaceMouse / Space Navigator by 3DConnexion input state struct. Used to manage SpaceNav input
typedef struct voxie_nav_t {
	
	float	dx,		//!< Space Navigator's translation delta movement on X axis 
			dy,		//!< Space Navigator's translation delta movement on Y axis
			dz,		//!< Space Navigator's translation delta movement on Z axis
			ax,		//!< Space Navigator's rotation (angle) delta on X axis
			ay,		//!< Space Navigator's rotation (angle) delta on Y axis
			az;		//!< Space Navigator's rotation (angle) delta on Z axis

	int		but;	//!< Space Navigator button status similar to a traditional mouse 0 = no buttons, 1 = left button, 2 = right button, 3 = both buttons 

} voxie_nav_t;


//! Manages custom touch keys displayed on the 2D secondary (touch) screen 
/**
* To make use of this struct in Voxon development see VoxieBox::touchAddKey
*/
typedef struct touchkey_t {

	char	*st;		//!<  string to display. Arrows use special string code: 0xcb,0xcd,0xc8,0xd0 (up,down,left,right respectively) following by null terminator.
	
	int		x0,			//!< left most pixel position of button's rectangle
			y0,			//!< top most pixel position of button's rectangle
			xs,			//!< size (how wide) in pixels for the button's rectangle
			ys,			//!< size (how tall) in pixels for the button's rectangle
			fcol,		//!< foreground color (24-bit hex 0xRBG value 0xff0000 = Red) 
			bcol,		//!< background color (24-bit hex 0xRBG value 0xff0000 = Red). Use -1 for transparent background.
			keycode;	//!< keyboard scan code of button. use extended scancodes for visual feedback @see inputTypes.h::Keys for list of scancodes. Special keycodes for mouse: Mouse:0x0000, LBut:0x0001, RBut:0x0002, MBut:0x0003

} touchkey_t;

#define VOXIE_DICOM_MIPS 4 //NOTE:limited to 6 by voxie_dicom_t:gotz bit mask trick

//! struct for reading DICOM (media data) - used internally; not for developers
typedef struct voxie_dicom_t {

	signed short	*mip[VOXIE_DICOM_MIPS];			
	int				*gotz;							
	point3d			rulerpos[3];
	
	point3d			slicep,
					slicer,
					sliced,
					slicef;
	
	float			reserved, 
					timsincelastmove,
					detail,
					level[2];
	
	int				xsiz, 
					ysiz, 
					zsiz;           
	
	int				zloaded;                   
	
	int				color[2],
					autodetail,
					usedmip,
					slicemode,
					drawstats,
					ruler,
					flags;
	
	int				defer_load_posori, 
					gotzip;
	
	int				forcemip, 
					lockmip; 
	
	int				saveasstl; 
	int				nfiles; 
	
	int				n, 
					cnt, 
					nummin, 
					nummax; 
	
	char			filespec[MAX_PATH];   

	float			xsc, 
					ysc,
					zsc;

} voxie_dicom_t;



//! internal struct used by voxiebox.dll privately to manage zip files (similar to FILE structure)
struct kzfile_t;
//! internal struct used by voxiebox.dll privately to manage searching through zip files (similar to FILE structure)
struct kzfind_t;
// enum to manage file attributes from kzfileinfo_t
enum { ATTRIB_ISDIR = 1, ATTRIB_INZIP = 2, ATTRIB_RDONLY = 4, ATTRIB_HIDDEN = 8, ATTRIB_ISDRIVE = 16 };
//! internal struct used by voxiebox.dll to store ziped files information.
typedef struct { __int64 size; int year; char month, day, dayofweek, hour, minute, second; short attrib; char name[MAX_PATH]; } kzfileinfo_t;
// enum to manage different image decoding functions used
enum { KPLIB_NONE = 0, KPLIB_PNG, KPLIB_JPG, KPLIB_GIF, KPLIB_CEL, KPLIB_BMP, KPLIB_PCX, KPLIB_DDS, KPLIB_TGA }; //kpgetdim() returns this

//! struct to manage Voxon Volumetric Recordings .REC files - used internally; not for developers
typedef struct voxie_rec_t {

	kzfile_t			*fil; 			//!< Warning: do not use this from user code - for internal use only.
	double			timleft;
	float			*frametim;
	
	int				*frameseek,
					framemal,
					kztableoffs,
					error;
	
	int				playmode,		
					framecur,
					framenum;

	int				currep,
					animmode;		//!< 0=forward, 1=ping-pong, 2=reverse
	
	int				playercontrol[4];

} voxie_rec_t;


//! Not for developers - Legacy code - This is for an experimental laser controller developed by Voxon which is not used by the public
typedef struct voxie_laser_t {

	point3d		pt,
				vec;

	int			navx,
				navy,
				but;

} voxie_laser_t;


static const int KEY_HISTORY_LENGTH = 15; //default 15 change this value to have a longer history buffer of keystrokes if your program requires it
static const int XBOX_HISTORY_LENGTH = 16;
static const int NAV_HISTORY_LENGTH = 3;

//! used internally by VoxieBox class to log various button inputs this struct allows you to view the history of various inputs
typedef struct voxie_input_state_t {

	int inputCodeRaw;
	int inputCode;
	double startTime;
	double duration;
	double startLastPressed;
	bool isHeld; 
	bool onUp;
	int state;
} voxie_input_state_t;

//! used internally by VoxieBox to log various keyboard button inputs this struct allows you to view the history
typedef struct voxie_keyboard_history_t {

	voxie_input_state_t history[KEY_HISTORY_LENGTH];

} voxie_keyboard_history_t;


//! used internally by VoxieBox to log various USB game controller button inputs this struct allows you to view the history
typedef struct voxie_xbox_history_t {

	voxie_input_state_t history[XBOX_HISTORY_LENGTH];

} voxie_xbox_history_t;

//! used internally by VoxieBox to log various Space Nav inputs this struct allows you to view the history of various inputs
typedef struct voxie_nav_history_t {

	voxie_input_state_t history[NAV_HISTORY_LENGTH];

} voxie_nav_history_t;

static int const COL_SCROLL_MAX = 90;
static int const PALETTE_COLOR_MAX = 90;


//! collection bold colors chosen for a VX1. This is what VoxieBox::randomCol() draws from. Use with VoxieBox::randomCol()
static int const RANDOM_COLOR[90] = {
0xFF00EE, 0xFF00DD, 0xFF00CC, 0xFF00BB, 0xFF00AA, 0xFF0099, 0xFF0088, 0xFF0077, 0xFF0066, 0xFF0055, 0xFF0044, 0xFF0033, 0xFF0022, 0xFF0011, 0xFF0000, // RED
0xFF1100, 0xFF2200, 0xFF3300, 0xFF4400, 0xFF5500, 0xFF6600, 0xFF7700, 0xFF8800, 0xFF9900, 0xFFAA00, 0xFFBB00, 0xFFCC00, 0xFFDD00, 0xFFEE00, 0xFFFF00, // YELLOW
0xEEFF00, 0xDDFF00, 0xCCFF00, 0xBBFF00, 0xAAFF00, 0x99FF00, 0x88FF00, 0x77FF00, 0x66FF00, 0x55FF00, 0x44FF00, 0x33FF00, 0x22FF00, 0x11FF00, 0x00FF00, // GREEN
0x00FF11, 0x00FF22, 0x00FF33, 0x00FF44, 0x00FF55, 0x00FF66, 0x00FF77, 0x00FF88, 0x00FF99, 0x00FFAA, 0x00FFBB, 0x00FFCC, 0x00FFDD, 0x00FFEE, 0x00FFFF, // CYAN
0x00EEFF, 0x00DDFF, 0x00CCFF, 0x00BBFF, 0x00AAFF, 0x0099FF, 0x0088FF, 0x0077FF, 0x0066FF, 0x0055FF, 0x0044FF, 0x0033FF, 0x0022FF, 0x0011FF, 0x0000FF, // BLUE
0x1100FF, 0x2200FF, 0x3300FF, 0x4400FF, 0x5500FF, 0x6600FF, 0x7700FF, 0x8800FF, 0x9900FF, 0xAA00FF, 0xBB00FF, 0xCC00FF, 0xDD00FF, 0xEE00FF, 0xFF00FF // MAGENTA
};


// collection bold colors chosen for a VX1 display. This is what VoxieBox::scrollCol() draws from. Use with VoxieBox::scrollCol()
static int const PALETTE_COLOR[PALETTE_COLOR_MAX] = {
0xFF00EE, 0xFF00DD, 0xFF00CC, 0xFF00BB, 0xFF00AA, 0xFF0099, 0xFF0088, 0xFF0077, 0xFF0066, 0xFF0055, 0xFF0044, 0xFF0033, 0xFF0022, 0xFF0011, 0xFF0000, // RED
0xFF1100, 0xFF2200, 0xFF3300, 0xFF4400, 0xFF5500, 0xFF6600, 0xFF7700, 0xFF8800, 0xFF9900, 0xFFAA00, 0xFFBB00, 0xFFCC00, 0xFFDD00, 0xFFEE00, 0xFFFF00, // YELLOW
0xEEFF00, 0xDDFF00, 0xCCFF00, 0xBBFF00, 0xAAFF00, 0x99FF00, 0x88FF00, 0x77FF00, 0x66FF00, 0x55FF00, 0x44FF00, 0x33FF00, 0x22FF00, 0x11FF00, 0x00FF00, // GREEN
0x00FF11, 0x00FF22, 0x00FF33, 0x00FF44, 0x00FF55, 0x00FF66, 0x00FF77, 0x00FF88, 0x00FF99, 0x00FFAA, 0x00FFBB, 0x00FFCC, 0x00FFDD, 0x00FFEE, 0x00FFFF, // CYAN
0x00EEFF, 0x00DDFF, 0x00CCFF, 0x00BBFF, 0x00AAFF, 0x0099FF, 0x0088FF, 0x0077FF, 0x0066FF, 0x0055FF, 0x0044FF, 0x0033FF, 0x0022FF, 0x0011FF, 0x0000FF, // BLUE
0x1100FF, 0x2200FF, 0x3300FF, 0x4400FF, 0x5500FF, 0x6600FF, 0x7700FF, 0x8800FF, 0x9900FF, 0xAA00FF, 0xBB00FF, 0xCC00FF, 0xDD00FF, 0xEE00FF, 0xFF00FF // MAGENTA
};

// some touch settings 
#define TOUCH_MAX					11	// how many touches the API can process 
#define TOUCH_TIME_OUT				3	// how long before a dead touch sense dies out -- this is to prevent glitches
#define TOUCH_HELD_TIME				2	// how long a touch is received before the 'isHeld' boolean becomes true. 
#define TOUCH_ANGLE_DEAD_ZONE		0	// the dead zone for angle rotation delta 
#define TOUCH_DISTANCE_DEAD_ZONE	1	// the dead zone for the pinch distance delta

//! touch point struct used for holding data associated for a single finger / touch point. Used for advanced touchscreen use. 
typedef struct touchPoint_t { int posx, posy, oposx, oposy, deltax, deltay, state = -1, ostate; bool isHeld, isDown, justPressed, onUp, active, inPinch; double startTime, lastUpdate; } touchPoint_t;

//! touch input struct holds all the touch input data. Used for advanced touchscreen use. 
typedef struct touchInput_t { point2d opinch0, opinch1; float sensitivity = 1, opinchDistance, pinchDistance, pinchDistanceDelta, pinchAngle, pinchRotation, opinchRotation, pinchRotationDelta, currentTouchNo; bool initPinch, active, pinchActive; touchPoint_t tPoint[TOUCH_MAX]; double heldTime = TOUCH_HELD_TIME; int pinch0Index, pinch1Index, gDeltaX, gDeltaY, ogDeltaX, ogDeltaY, pinchPriority; double pinchLastUpdate; bool focusPinch; } touchInput_t;

// enum to manage file VoxieBox::getTouchPressState return values
enum { TOUCH_STATE_NOT_PRESSED = 0, TOUCH_STATE_IS_DOWN = 1, TOUCH_STATE_IS_HELD = 2, TOUCH_STATE_JUST_PRESSED = 3, TOUCH_STATE_ON_UP = 4 };



static touchInput_t touch;

#pragma pack(pop)
#endif