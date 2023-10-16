#pragma once
#include "vxCpp.h"
#pragma pack(push,1)

/** \mainpage DemClass - a  Class version of Voxon's Demview DLL / API 
 * \section intro_sec Introduction
 * Demview is a Digital Elevation Map viewer for Voxon's Volumetric technology, written by Ken Silverman for Voxon 
 * Demview - is a stand alone application for PC and Voxon's Volumetric Display and can also be used as a .DLL / API (DemView.DLL)
 * 
 * 
 * The DemClass makes use of the DemView.DLL and wraps the functions of the DLL into its own class allowing developers
 * to easily include the class and thus be able to render their own DEM in their VX applications. 
 * 
 * It also contains quality of life features and helper functions to make development easier. 
 *
 * DemClass requires VXCPP.DLL (VX++) which is the C++ class based version of our VoxieBox API  
 *
 * DemClass is written by Matthew Vecchio for Voxon 
 * 
 * DemView streams tile data from online servers. It sources two types of data to build its tiles; elevation data and texture map  
 * Data, after a Latitude, Longitude and AltRadii (altradii = approximate altitude above sea level in globe (usually Earth) radii.).  
 * There is a function to convert altitude from meters above sea level to altradii within the class. 
 * 
 * NOTE on Caching - by default the DemClass will cache tiles under a cache\ sub directory relative to the EXE file so make sure 
 * that a cache folder / directory exists, otherwise tiles will not be cached and the system by crash if it runs out of memory to cache.  
 * 
 * 
 */
 /** \file DemClass.h
  *
  * Header file for DemClass outlines the VoxieBox class and the IVoxieBox interface.
  * main header file for VX++ development. Requires the VXCPP.DLL and associated files to work (VX++ Framework)
  */


// ** UNIQUE DATA TYPES FOR DEMVIEW **/

enum class DEM_SERVER_TYPE {
	DEM_HEIGHT_SERVER = 0,
	DEM_COLOR_SERVER = 1
}; //!< Since there are currently two types of servers used for rendering a DEM one is a height Server the other is the color (texture map) this enum is used to manage which is which. 

typedef struct { double x, y, z; } dpoint3d; //!< similar to voxiebox::point3d but uses doubles instead for more precision 

typedef struct { float fzmin, fzmax, fhakzadd; double fzsum, fzsum2; int fzcnt; } drawglobestats_t; //units in voxie z coordinates (typically: -.5 .. +.5)

//! the variables to define a demview server are stored within this struct
	 typedef struct {
	DEM_SERVER_TYPE serverType;  //!< the two server types 0=heightmap, 1=color
	int tilezmin,				 //!< outermost tile level: almost always 0. (0: 1 tile covers entire globe (except for a small section near the poles), 1: 2x2 grid, 2: 4x4 grid, ..)
		tilezmax,				 //!< innermost tile level: typically:{15..21}. (ex: 15 means 32768x32768 grid is finest level)
		log2tilesiz,			 //!< tile resolution: usually 8 for 256x256 tiles; have seen 9 (512x512)
		iswebmerc,				 //!< 1 if uses Web Mercator projection, 0 if not. Almost always 1. If height & color don't match, try the other!
		ishttps,				 //!< tile server prefix: 0=http://, 1=https:// 
		isquadtree,				 //!< controls how tile names are generated. 0=separate z,x,y, 1=quadtree (single base 4 number; used by VirtualEarth)
		servnum,				 //!< number of port connections to server. typical range: 1..8. Higher may load faster - please check whether server supports it!
		servcharoffs;			 //!< offset to char in url_st[] to select other servers (used when servnum > 1). Example: "serv-a.place.com", "serv-b.place.com" would use: servnum=2, servcharoffs=5 (where the 'a' is)
	char cache_st[256];			 //!< printf-formatted string of how to name cached tiles. Must contain exactly 4 %d's for: tilesiz,z,x,y ther chars can be whatever, but should be unique to identify which server, or height vs color. 
	char url_st[256];			 //!< base server address (i.e. somesite.com)
	char tile_st[256];			 //!< printf-formatted string appended to server name to generate tile names (quadtree==0: must have 3 %d's for z,x,y; quadtree==1: must have 1 %d)
	bool isSet;
} demview_server_t;
#pragma pack(pop)




// Sizing constants - these can be modified to suit your needs.

#define		EARTHRADMET 6371e3			// The avg radius of the planet earth change this if you are ever working with another planet! 
#define		MAX_ALT_ZOOM 500000			// The maximum altitude the DEM's camera can zoom out to 
#define		MIN_ALT_ZOOM -5000			// The minimum altitude the DEM'S camera can zoom into
#define		HEIGHT_EXAGRATION_MAX 100   // The maximum value for height exaggeration 
#define		HEIGHT_EXAGRATION_MIN 1		// The minimal value for height exaggeration (usually always 1)




// Class definition of DemClass
class DemClass : public IVoxiePtr
{
public:

	DemClass();							//!< Constructor for the DemClass. Before you can use the Class you'll need to initialise it by calling Init() and passing in a voxiePtr 
	DemClass(IVoxieBox* voxiePtr);		//!< Constructor will also initialise it by calling Init()

	~DemClass();						//!< Deconstructor frees the DemView.DLL from memory. 

	void Init(IVoxieBox* voxiePtr);		//!< Initializes the DemClass and loads the DemView.DLL into memory and maps the DemView.dll functions to the DemClass. If a valid voxiePtr is found sets the internal isInitialized to true

	/** Set server needs to be called before the globe has been drawn.This is where the data is streamed from.
	 *  There are two servers to set a height server which determines where the height elevation comes from
	 *   and a colour server which forms the tile's texture.
	 */
	int setServer(DEM_SERVER_TYPE serverType, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char* cache_st, char* url_st, char* tile_st, float rmul, float gmul, float bmul);
	
	//! Legacy function for setting a Server which uses and INT for serverType instead. 
	int setServer(int serverType, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char* cache_st, char* url_st, char* tile_st, float rmul, float gmul, float bmul);


	void updateGlobe();					//!< updateGlobe function performs all the updates to the coordinates systems ready for the DEM to be drawn. Called automatically if DemClass::updateGlobeOnDraw is set to true.
	float drawGlobe();					//!< Draws the globe onto the volumetric display. The function call must reside within a Voxie->StartFrame() and Voxie->EndFrame() function call to be seen by the Voxie Frame Buffer. 

	void setPos(dpoint3d dpos, dpoint3d drig, dpoint3d ddow, dpoint3d dfor);		//!< setter for the internal cartesian coordinates. These are used to move and draw the DEM
	void getPos(dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor);	//!< getter for the internal cartesian coordinates. Presented as 
	
	point3d getColMultiplier();											//!< Returns the color multiplier values as a point3d X = R, Y = G, Z = B
	void	setColMultiplier(float rMulti, float gMulti, float bMulti); //!< sets the colour channel multipler values for the texture image. 1.0f is default. Changing these settings 

	// camera movement functions
	void moveCamera(float amountX, float amountY, float amountZ); //!< Move the DEM's camera in a specific direction. The amount moving is not by pixel, the higher the value the faster the speed. Negative numbers move in the opposite direction 
	void rotateCamera(float angleMovement);						  //!< Rotates the DEM's yaw angle. The value passed in is not a uniform measurement. 
	double getDemRotatedAngle();								  //!< Get the internal rotated Angle value. This value will be expressed as an offset from 0' in radians. 
	void setDemRotatedAngle(float radian);						  //!< Set the rotated angle this rotation will be applied from the origin angle (0 / 0') and be a radian value. 
	void adjustCameraZ(float amountZ);							  //!< Adjusts the Z height of the DEM. To move the whole heightmap towards the top or the bottom of the view. Does not work when DemClass::setSeaLock() is enabled. 




	// Height exaggeration functions 
	void setHeightEx(float fexagz);								  //!< Set the height exaggeration for the DEM. The value is a multiplier; 1 is natural height, 8 is our default value is 8 times higher than natural, real world elevation.
	void adjustHeightEx(float adjustmentAmount);				  //!< Adjustment for height exaggeration this is used for adjusting the height exaggeration over time. a positive value increases, negative decreases. 
	float getHeightEx();										  //!< Return the current height exaggeration amount 

	//! Draws a Sprite / Model onto the DEM. 
	/**
	*	@param	filnam		Sprite / model filename to draw (supported types are .OBJ, .PLY, .STL, .KV6). Path can be relative of absolute. 
	*	@param	lon_deg		the longitude value of where to place the sprite.
	*	@param  lat_deg		the latitude value of where to place the sprite.
	*   @param  alt_m		Altitude above (or below) sea level in meters.
	*   @param  yaw_deg		The yaw's rotation of the sprite, expressed in degrees. 
	*   @param  pitch_deg:  The pitch's rotation of sprite, expressed in degrees.
	*   @param  roll_deg:	The roll's rotation of sprite, expressed in degrees.
	*   @param  fscale:		Scale of model in relation to Earth radii? start with small numbers like .00005f.
	*   @param  fexagz_pos: Height exaggeration for position, pass in straight DemClass::getHeightEx() for even values.
	*   @param  fexagz_sca: Height exaggeration for scale, pass in 1 for no scale changes based on height exaggeration. 
	*   @param  col:		Color scale -- 0x404040 is natural anything below is dimmer any color value above 0x404040 applies a tint. 
	*/
	void drawSprite(const char* filnam, double lon_deg, double lat_deg, double alt_m, double spr_yaw_deg, double spr_pit_deg, double spr_rol_deg, double fscale, double fexagz_pos, double fexagz_sca, int col);


	// Custom Height Color Palette can be mixed in with the DEM's texture.  

	//! Superimpose a custom colour palette based on the height - see heipal.png. A 1024 x 1 pixel image (.PNG or .JPG) is the requirement, from lowest to highest.
	void setHeighPalIMG(const char* filename, float transparency); 
	//! Set a height colour palette by passing in an int array of colour values, from lowest altitude to highest. Define the lowest height and the highest point in meters to be represented. 
	void setHeightPalColArray(int* colArray, int colArrayLength, float alt_mStartRange = -10000.f, float alt_mEndRange = +10000.f, float transparency = 0.75);
	//! Clears the height palette settings.  
	void clearHeightPal();

	// setters for DEM view.
	
	void setPosbyLonLatAlt(double lon_deg, double lat_deg, double altradii); //!< sets the position of the DEM by setting a Lon, Lat and an altradii (you can convert altitude in meters to altradii using the DemClass::alt2altradii() converstion function 
	void setPosbyLonLat(double lon_deg, double lat_deg);					 //!< sets the position of the DEM by longitude and latitude - using the previous altradii / altitude values. 
	void setLon(double newLon);												 //!< sets the position of the DEM's longitude value. 
	void setLat(double newLat);												 //!< sets the position of the DEM's latitude value.
	void setAltradii(double newAltRadii);									 //!< sets the position of the DEM's altRadii value (altradii = approximate altitude above sea level in globe (usually Earth) radii.) 
	void setAltM(double newAltM);											 //!< sets the position of the DEM's altitude in meters value (from sea level)

	// getters for the DEM view. 

	void getLonLatAlt(double* lon_deg, double* lat_deg, double* altradii);   //!< get the longitude, latitude and altradii values for the current DEM position
	double getLon();														 //!< returns the longitude value for the current DEM position
	double getLat();														 //!< returns the latitude value for the current  DEM position
	double getAltradii();													 //!< returns the altradii value for the current DEM position (altradii = approximate altitude above sea level in globe (usually Earth) radii.) 
	double getAltM();														 //!< returns the altitude in meters (Above sea level) of the current DEM position. 

	// various converters for working in various spaces.

	void		sph2xyz(double lon_deg, double lat_deg, double altradii, dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor);  	//!< Convert a spherical position to cartesian coordinates.
	void		xyz2sph(dpoint3d* dpos, double* lon_deg, double* lat_deg, double* altradii);		//!< Convert a cartesian to spherical position
	point3d		xyz2voxie(dpoint3d* dpos, point3d* voxiepos);										//!< Convert a cartesian position to 'Voxie' volumetric space. 
	point3d		sph2voxie(double lon_deg, double lat_deg, double altradii, point3d* voxiepos);	    //!< Convert a spherical postion  to 'Voxie' volumetric space.

																									//! Fancy sph2voxie converter which also returns full position and orientation values. 
	void		sph2voxie2(double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, point3d* voxiepos, point3d* pr, point3d* pd, point3d* pf);
	
	dpoint3d	voxie2xyz(point3d* voxiepos, dpoint3d* dpos);										//!< Converts 'Voxie' volumetric space to cartesian
	void		voxie2sph(point3d* voxie, double* lon_deg, double* lat_deg, double* altradii);		//!< Converts 'Voxie' volumetric space to spherical
	double		alt2altradii(double alt_m, bool addHeightEx = false);								//!< Converts altitude above sea level in meters to altradii. (altradii = approximate altitude above sea level in globe (usually Earth) radii.) 
	double		altradii2alt(double altradii);														//!< Converts altradii to altitude aove sea level.


	// Extra Helper Functions

	void rotDPVex(double ang, dpoint3d* a, dpoint3d* b);	//!< rotate a double point by radians 

	// Misc settings

	void setTopBotAltm(double dtopm, double dbotm);			//!< Set the DEM's Top and Bottom altitude in meters
	void getTopBotAltm(double* dtopm, double* dbotm);		//!< get the Top and Bottom altitude in meters for the current view.
	float getGlobalFloatScale();							//!< return the global float scale useful for sizing the things by a radius amount.


	// Drawing extra stuff on display
	void drawGrid(int col, float height_m = 0, bool showLonLat = false);	//!< Draws a grid on the DEM 
	void drawCompass();														//!< Draws a compass on the DEM

	void setDemRenderFlag(int newValue);									//!< Set the DemRender Flag ( 0 = none, 1 (0<<1) draw images below the volume on the bottom, 2 (0<<2) draw images above the volume on the top )
	int  getDemRenderFlag();												//!< Get the DemRender flag value

	// Processing options 

	void setSeaLock(bool choice);											//!< Toggle seaLock if enabled SeaLevel is drawn at the center of the display and the DEM can't be moved up or down.
	bool isSeaLockEnabled();												//!< Function to check if SeaLock is enabled or not

	void setUpdateGlobeOnDraw(bool choice);									//!< Toggle to enable or disabled calling the DemClass::UpdateGlobe() function on Draw() - saving a developer from writing UpdateGlobe() somewhere in their application. 
	bool isUpdateGlobeOnDrawEnabled();										//!< Function to check if UpdateGlobeOnDraw is enabled or not.
		
	// Reporting / Debug

	void report(int posX, int posY);										//!< Writes onto the VoxieBox secondary (touch) screen various information about the current DEM instance, usually for DEBUG purposes.  

	// Useful Public class variables

	double g_yaw_deg = 150;		//!< global yaw degrees used if you want to draw sprites and objects on the DEM and use the same rotations - not used by the DEM rendering process here for convenience. 
	double g_pitch_deg = 0;		//!< global pitch degrees used if you want to draw sprites and objects on the DEM and use the same rotations - not used by the DEM rendering process here for convenience.
	double g_roll_deg = 0;		//!< global roll degrees used if you want to draw sprites and objects on the DEM and use the same rotations - not used by the DEM rendering process here for convenience.

private:

	bool seaLock = false;					//!< locks the camera to show sea level at the centre of the display. 
	bool updateGlobeOnDraw = true;			//!< if enabled DemClass::updateGlobe() will get called on DemClass::drawGlobe()

	tiletype HeightPal;						//!< Internal variable to manage any height palette setting (an image that changes colour depending on the height of the map)

	drawglobestats_t dgs = { 0 };			//!< Height map statistics from previous frame: used to keep mountains in view!
	dpoint3d dpos, drig, ddow, dfor;		//!< current view's Cartesian coordinates (relative to center of globe, where globe has radius of 1.0), and unit orthonormal orientation
	
	
	float fexagz = 8.f,						//!< current view's height exaggeration true value is 1 but you'll notice that its not that exciting volumetrically so 8 is default
		  fhakup = 0.f,						//!< an offset used to raise or lower the position of the heightmap
		  resthresh = 256.f,				//!< Scale factor that determines what resolution to use at a particular altitude - like a detail control. Default is 256
		  globalFSc = 1;					//!< Global float scale an estimated scaling amount generated by the DrawGlobe() command which can be used for sizing radius primitives, or points that don't have a R,F,D vectors to scale. 
	int demRenderFlag = 3;					//!< Rendering the DEM flags drawing flags : 0 = draw as is, +1 (1<<0) = clip data lower than the volume to the bottom +2 (1<<1) clip data higher than the volume to the top
	double lon, lat, altradii;				//!< current views Spherical coordinates - natively demclass::drawGlobe() uses the dpos, drig, ddown and dfor for drawing the tiles but this is updated on DemClass::UpdateGlobe()
	double alt_m = 0;						//!< current view camera position alt in meters above sea level. 
	double rotatedAngle = 0;				//!< the rotated angle for the DEM - note that this in radians 0 would mean it natural.
	demview_server_t currentHeightServer;	//!<- remembers the current Height Server - makes it easier to adjust color multipliers 
	demview_server_t currentColorServer;	//!<- remembers the current Color Server - makes it easier to adjust color multipliers 
	point3d colMultiplier = { 1,1,1 };		//!< internal color R/G/B scalar multiplier stored as a point3d for ease ( X = R, Y = G, Z = B ) No scale: 1.f (for color server only only)

	bool  isInitialized = false;			//!< When a the class is initialized and  voxiePtr and the DemView.DLL this is set to true 
	IVoxieBox* voxiePtr = nullptr;			//!< The pointer to the VoxeBox classed used to interact with VxCPP.dll


	HINSTANCE hdemview = (HINSTANCE)INVALID_HANDLE_VALUE; //!< Demview.DLL handle

	// List of DLL Functions (these get updated and mapped when the Demview.DLL is loaded into memory. 
	void(__cdecl* demview_init_int)(HINSTANCE hvoxie);
	void(__cdecl* demview_uninit_int)(void);
	int(__cdecl* demview_setserv)(int sind, int tilezmin, int tilezmax, int log2tilesiz, int iswebmerc, int ishttps, int isquadtree, int servnum, int servcharoffs, char* cache_st, char* url_st, char* tile_st, float rmul, float gmul, float bmul);
	float(__cdecl* demview_drawglobe)(voxie_wind_t* vw, voxie_frame_t* vf, dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor, float fexagz, float resthresh, int flags, drawglobestats_t* dgs);
	void(__cdecl* demview_drawspr)(voxie_frame_t* vf, const char* filnam, double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, int col);
	void(__cdecl* demview_setheipal)(int* pal, int paln, float met0, float met1, float trans);
	void(__cdecl* demview_sph2xyz)(double lon_deg, double lat_deg, double altradii, dpoint3d* dpos, dpoint3d* drig, dpoint3d* ddow, dpoint3d* dfor);
	void(__cdecl* demview_xyz2sph)(dpoint3d* dpos, double* lon_deg, double* lat_deg, double* altradii);
	void(__cdecl* demview_xyz2voxie)(dpoint3d* dpos, point3d* voxiepos);
	void(__cdecl* demview_sph2voxie)(double lon_deg, double lat_deg, double altradii, point3d* voxiepos);
	void(__cdecl* demview_sph2voxie2)(double lon_deg, double lat_deg, double alt_m, double yaw_deg, double pit_deg, double rol_deg, double fscale, double fexagz_pos, double fexagz_sca, point3d* voxiepos, point3d* pr, point3d* pd, point3d* pf);
	void(__cdecl* demview_voxie2xyz)(point3d* voxie, dpoint3d* dpos);
	void(__cdecl* demview_voxie2sph)(point3d* voxie, double* lon_deg, double* lat_deg, double* altradii);

};

