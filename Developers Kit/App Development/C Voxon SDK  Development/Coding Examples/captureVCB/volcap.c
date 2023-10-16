#if 0
volcap.exe: volcap.c voxiebox.h; cl /TP volcap.c /Ox /MT /link user32.lib
   del volcap.obj
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>

   //09/07/2023: test voxie_volcap(). Originally by Matthew Vecchio
int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
   voxie_wind_t vw; voxie_frame_t vf; voxie_inputs_t in;
   double tim = 0.0, otim, dtim, avgdtim = 0.0;
   int i, recording = 0;

   if (voxie_load(&vw) < 0) { MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); return (-1); }
   if (voxie_init(&vw) < 0) { return (-1); }
   while (!voxie_breath(&in))
   {
      otim = tim; tim = voxie_klock(); dtim = tim - otim;
      if (voxie_keystat(0x1)) { voxie_quitloop(); }
      i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1); //']'-'['
      if (i)
      {
              if (voxie_keystat(0x2a)|voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang+(float)i*dtim*   2.0,-3.14159*.5),0.1268); //Shift+[,]
         else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist =     max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
         else vw.emuhang += (float)i*dtim*2.0; //[,]
         voxie_init(&vw);
      }

      if (voxie_keystat(0x02) == 1) {                                  voxie_volcap("mycap.ply",VOLCAP_FRAME_PLY   , 0);   } //1   :)
      if (voxie_keystat(0x03) == 1) {                                  voxie_volcap("mycap.rec",VOLCAP_FRAME_REC   , 0);   } //2   :)
      if (voxie_keystat(0x04) == 1) {                                  voxie_volcap("mycap.png",VOLCAP_FRAME_PNG   , 0);   } //3   :) (NOTE: must be in UPDN mode, else ignored!)
      if (voxie_keystat(0x05) == 1) {                                  voxie_volcap("mycap.vcb",VOLCAP_FRAME_VCB   , 0);   } //4   :) (NOTE: must be in UPDN mode, else ignored!)

      if (voxie_keystat(0x06) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.rec",VOLCAP_VIDEO_REC   ,15); } } //5   :)
      if (voxie_keystat(0x07) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.zip",VOLCAP_VIDEO_PLY   , 5); } } //6   Broken - probably never worked. Creates 0by mycap.zip and single voxie0000.ply
      if (voxie_keystat(0x08) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.zip",VOLCAP_VIDEO_VCBZIP, 5); } } //7   :) (NOTE: must be in UPDN mode, else ignored!)
      if (voxie_keystat(0x39) == 1) { recording = 0;                   voxie_volcap(          0,VOLCAP_OFF         , 0);   } //Spc :)

      voxie_frame_start(&vf);
      voxie_setview(&vf,-vw.aspx     ,-vw.aspy     ,-vw.aspz,vw.aspx     ,vw.aspy     ,vw.aspz);
      voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,vw.aspx-1e-3,vw.aspy-1e-3,vw.aspz,1,0xffffff);

      voxie_drawsph(&vf,0,0,cos(tim)*0.05,.25,1,0xff0000);
      voxie_drawsph(&vf,cos(tim*4)*0.3-0.4,sin(tim*4)*0.25,0,.3,1,0x00ff00);
      voxie_drawsph(&vf,sin(tim*4)*0.3+0.4,cos(tim*4)*0.25,0,.3,1,0x0000ff);

      avgdtim += (dtim-avgdtim)*.1;
      { point3d tp, tr = {.08,0,0}, td = {0,.08,0}; tp.x = -0.8; tp.y = 0.7; tp.z = -vw.aspz+1e-3; voxie_printalph_(&vf,&tp,&tr,&td,0xffffff,"tim:%1.2f",tim); }
      voxie_debug_print6x8_(30,68,0xffc080,-1,"VPS %5.1f Recording %d\n",1.0/avgdtim,recording);

      voxie_frame_end(); voxie_getvw(&vw);
   }
   voxie_uninit(0);
   return 0;
}

#if 0
!endif
#endif
