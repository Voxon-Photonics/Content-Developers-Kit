#if 0
leaptest.exe: leaptest.obj cwinmain.obj
	link       leaptest.obj cwinmain.obj user32.lib gdi32.lib c:\LeapSDK\lib\x64\LeapC.lib
leaptest.obj: leaptest.c; cl /c /TP /MT leaptest.c /Ox /GFy /Ic:\LeapSDK\include
cwinmain.obj: cwinmain.c; cl /c /TP /MT cwinmain.c /Ox /GFy
!if 0
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include "cwinmain.h"
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------
#include "LeapC.h"

unsigned char leap_image[640*480+640/*hack*/];

static LEAP_CONNECTION leap_con;
static LEAP_IMAGE_FRAME_REQUEST_TOKEN *leap_imagetok = 0;
static LEAP_TRACKING_EVENT leap_frame[2] = {0,0}; //FIFO for thread safety
static int leap_framecnt = 0, leap_iscon = 0, leap_isrun = 0, leap_useimage;

static void leap_thread (void *_)
{
	LEAP_CONNECTION_MESSAGE msg;
	eLeapRS res;

	while (leap_isrun)
	{
		res = LeapPollConnection(leap_con,1000,&msg); if (res != eLeapRS_Success) { /*printf("LeapPollConnection() = %d\n",res);*/ continue; }
		switch (msg.type)
		{
			case eLeapEventType_Connection: leap_iscon = 1; break;
			case eLeapEventType_ConnectionLost: leap_iscon = 0; break;
			case eLeapEventType_Device: break; //code removed; assume 1 valid device
			case eLeapEventType_DeviceLost: break;
			case eLeapEventType_DeviceFailure: break; //use msg.device_failure_event->status, msg.device_failure_event->hDevice
			case eLeapEventType_Tracking:
				memcpy(&leap_frame[leap_framecnt&1],msg.tracking_event,sizeof(LEAP_TRACKING_EVENT)); //support polling tracking data from different thread
				leap_framecnt++;
				if ((leap_useimage) && (!leap_imagetok))
				{
					LEAP_IMAGE_FRAME_DESCRIPTION frameDescription;
					frameDescription.type       = eLeapImageType_Default;
					frameDescription.frame_id   = msg.tracking_event->info.frame_id;
					frameDescription.buffer_len = sizeof(leap_image);
					frameDescription.pBuffer    = leap_image;
					leap_imagetok = (LEAP_IMAGE_FRAME_REQUEST_TOKEN *)malloc(sizeof(LEAP_IMAGE_FRAME_REQUEST_TOKEN));
					LeapRequestImages(leap_con,&frameDescription,leap_imagetok);
				}
				break;
			case eLeapEventType_ImageComplete:
				//use msg.image_complete_event->info.frame_id //frame number
				//use msg.image_complete_event->data_written //frame size
				free(leap_imagetok); leap_imagetok = 0;
				break;
			case eLeapEventType_ImageRequestError:
				free(leap_imagetok); leap_imagetok = 0;
				break; //use msg.image_request_error_event
			case eLeapEventType_LogEvent:          break; //use msg.log_event->Severity, msg.log_event->Timestamp, msg.log_event->Message
			case eLeapEventType_Policy:            break; //use msg.policy_event->current_policy
			case eLeapEventType_ConfigChange:      break; //use msg.config_change_event->requestID, msg.config_change_event->status
			case eLeapEventType_ConfigResponse:    break; //use msg.config_response_event->requestID, msg.config_response_event->value
			default: break; //msg.type bad
		}
	}
}

void leap_uninit (void) { LeapDestroyConnection(leap_con); leap_isrun = 0; }
LEAP_CONNECTION *leap_init (int useimage)
{
	eLeapRS res;
	res = LeapCreateConnection(0,&leap_con); if (res != eLeapRS_Success) return(&leap_con);
	res = LeapOpenConnection(leap_con); if (res != eLeapRS_Success) return(&leap_con);
	leap_isrun = 1; leap_useimage = useimage;
	_beginthread(leap_thread,0,0);
	while (!leap_iscon) Sleep(15);
	return(&leap_con);
}

LEAP_TRACKING_EVENT *leap_getframe (void) { return(&leap_frame[leap_framecnt&1]); }

//--------------------------------------------------------------------------------------------------
#if 0
	//Ken's LeapC.h summary:
typedef struct _LEAP_FRAME_HEADER { void reserved; int64_t frame_id, timestamp/*us*/; } LEAP_FRAME_HEADER;
typedef struct _LEAP_VECTOR       { union { float v[3]; struct { float x, y, z; }; }; } LEAP_VECTOR;
typedef struct _LEAP_MATRIX_3x3   { LEAP_VECTOR m[3]; } LEAP_MATRIX_3x3;
typedef struct _LEAP_QUATERNION   { union { float v[4]; struct { float x, y, z, w; }; }; } LEAP_QUATERNION;
typedef struct _LEAP_BONE
{
	LEAP_VECTOR prev_joint/*closer to heart*/, next_joint/*farther to heart*/;
	float width/*avgwidthofflesharoundboneinmm;
	LEAP_QUATERNION rotation/*rotfromforwarddir*/;
} LEAP_BONE;
typedef struct _LEAP_DIGIT
{
	int32_t finger_id;
	union { LEAP_BONE bones[4]; struct { LEAP_BONE metacarpal/*0forThumb*/, proximal/*knuckle*/, intermediate, distal/*FingerTip*/; }; };
	LEAP_VECTOR tip_velocity/*Instant.speed@tip;mm/sec*/, stabilized_tip_position/*lpf*/;
	uint32_t is_extended; //is straight-ish?
} LEAP_DIGIT;
typedef struct _LEAP_PALM
{
	LEAP_VECTOR position/*cent_mm*/, stabilized_position, velocity/*mm/sec*/, normal/*points down/front of palm*/;
	float width; //width of palm when flat
	LEAP_VECTOR direction; //palm toward fingers
	LEAP_QUATERNION orientation; //{normal x direction, -normal, -direction}
} LEAP_PALM;
typedef enum _eLeapHandType { eLeapHandType_Left, eLeapHandType_Right } eLeapHandType;
typedef struct _LEAP_HAND
{
	uint32_t id, flags;
	eLeapHandType type;
	float confidence;
	uint64_t visible_time; /*life of tracked hand in us*/
	float pinch_distance/*finger 1-2 dist*/, grab_angle/*fingers to palm*/, pinch_strength/*0=not pinch, 1=full pinch*/, grab_strength/*0=no grab, 1=full grab*/;
	LEAP_PALM palm;
	union { struct { LEAP_DIGIT thumb, index, middle, ring, pinky; }; LEAP_DIGIT digits[5]; };
	LEAP_BONE arm;
} LEAP_HAND;
typedef struct _LEAP_TRACKING_EVENT
{
	LEAP_FRAME_HEADER info;
	int64_t tracking_frame_id;
	LEAP_VECTOR interaction_box_size, interaction_box_center;
	uint32_t nHands;
	LEAP_HAND *pHands;
	float framerate;
} LEAP_TRACKING_EVENT;
#endif
//--------------------------------------------------------------------------------------------------

typedef struct { float x, y, z; } point3d;

static void project (tiletype *dd, float fx, float fy, float fz, float *fsx, float *fsy)
{
	(*fsx) = (      fx)/(384.f-fz)*dd->x*.5f + dd->x*.5f;
	(*fsy) = (384.f-fy)/(384.f-fz)*dd->x*.5f + dd->y*.5f;
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	LEAP_TRACKING_EVENT *frame;
	LEAP_HAND *hand;
	LEAP_PALM *palm;
	LEAP_DIGIT *digit;
	LEAP_BONE *bone;
	LEAP_VECTOR *vec;
	tiletype dd;
	point3d palmp, palmr, palmd, palmf;
	double tim = 0.0, otim, dtim, avgdtim = 0.02;
	float f, fx, fy, fz, fsx, fsy, fsx2, fsy2;
	int i, j, k, x, y, *wptr, col, obstatus = 0;
	unsigned char *rptr0, *rptr1;

	xres = 1280; yres = 960; prognam = "Ken's Leap test";
	if (!initapp(hinst)) return(-1);

	leap_init(1);

	while (!breath())
	{
		otim = tim; tim = klock(); dtim = tim-otim;

		if (startdirectdraw(&dd.f,&dd.p,&dd.x,&dd.y))
		{
			//drawrectfill(&dd,0,0,dd.x,dd.y,0x000000);

				//Draw stereo IR webcam images (640x240x2)
			for(y=min(960,dd.y)-1;y>=0;y--)
			{
				wptr = (int *)(dd.p*y + dd.f);
				rptr0 = &leap_image[((y>>2) + 240)*640     ];
				rptr1 = &leap_image[((y>>2)      )*640+32*0];
				for(x=min(640,(dd.x-1)>>1)-1;x>=0;x--)
				{
					i = (int)rptr0[x]; i += (((int)rptr1[x])<<16) + (i<<8);
					wptr[x*2+0] = i;
					wptr[x*2+1] = i;
				}
			}

			frame = leap_getframe();
			if (frame)
			{
				for(i=frame->nHands-1;i>=0;i--)
				{
					hand = &frame->pHands[i];
					if (hand->type == eLeapHandType_Left) col = 0xfc0000; else col = 0x00fc00;

						//Draw arm vector:
					bone = &hand->arm; vec = &bone->prev_joint;
					if (384.f-vec->z >= 0)
					{
						project(&dd,vec->x,vec->y,vec->z,&fsx ,&fsy ); vec = &bone->next_joint;
						project(&dd,vec->x,vec->y,vec->z,&fsx2,&fsy2);
						drawline(&dd,fsx,fsy,fsx2,fsy2,col);
					}

						//Draw fingers:
					for(j=0;j<5;j++)
					{
						digit = &hand->digits[j];
						//vec = &digit->stabilized_tip_position;
						vec = &digit->bones[3].next_joint; //unstabilized tip position
						project(&dd,vec->x,vec->y,vec->z,&fsx,&fsy);
						drawcirc(&dd,fsx,fsy,4096.f/(384.f-vec->z),col>>((!digit->is_extended)*2));
						print6x8(&dd,fsx-3,fsy-4,0xffffff,-1,"%d",j+1);
						for(k=0;k<4;k++)
						{
							bone = &digit->bones[k];
							vec = &bone->prev_joint; project(&dd,vec->x,vec->y,vec->z,&fsx ,&fsy );
							vec = &bone->next_joint; project(&dd,vec->x,vec->y,vec->z,&fsx2,&fsy2);
							drawline(&dd,fsx,fsy,fsx2,fsy2,col>>((!digit->is_extended)*2));
						}
					}

						//Draw palm:
					palm = &hand->palm;
					memcpy(&palmp,&palm->position,sizeof(point3d));
					memcpy(&palmd,&palm->normal,sizeof(point3d));
					memcpy(&palmf,&palm->direction,sizeof(point3d));
					palmr.x = palmd.y*palmf.z - palmd.z*palmf.y;
					palmr.y = palmd.z*palmf.x - palmd.x*palmf.z;
					palmr.z = palmd.x*palmf.y - palmd.y*palmf.x;
					f = 64.f;
					project(&dd,palmp.x          ,palmp.y          ,palmp.z          ,&fsx ,&fsy ); drawcirc(&dd,fsx,fsy,4096.f/(384.f-palmp.z),0xffffff);
					project(&dd,palmp.x+palmr.x*f,palmp.y+palmr.y*f,palmp.z+palmr.z*f,&fsx2,&fsy2); drawline(&dd,fsx,fsy,fsx2,fsy2,0xff4040);
					project(&dd,palmp.x+palmd.x*f,palmp.y+palmd.y*f,palmp.z+palmd.z*f,&fsx2,&fsy2); drawline(&dd,fsx,fsy,fsx2,fsy2,0x40ff40);
					project(&dd,palmp.x+palmf.x*f,palmp.y+palmf.y*f,palmp.z+palmf.z*f,&fsx2,&fsy2); drawline(&dd,fsx,fsy,fsx2,fsy2,0x4080ff);
					print6x8(&dd,0,i*8,0xffffff,0,"palm:%+7.2f %+7.2f %+7.2f",palmp.x,palmp.y,palmp.z);
				}

				print6x8(&dd,dd.x-256,0,0xffffff,0,"Frame %d",frame->tracking_frame_id);

				avgdtim += (dtim - avgdtim)*.02;
				print6x8(&dd,dd.x-256,dd.y-16,0xffffff,0,"%.2f fps",1.0/avgdtim);
				print6x8(&dd,dd.x-256,dd.y- 8,0xffffff,0,"%.2f rps",(double)leap_framecnt/tim);
			}

			stopdirectdraw(); nextpage();
		}

		obstatus = bstatus;
		if (keystatus[0x1]) { keystatus[0x1] = 0; quitloop(); } //Pressed 'ESC'
		Sleep(5);
	}

	leap_uninit();
	uninitapp();
	return(0);
}

#if 0
!endif
#endif
