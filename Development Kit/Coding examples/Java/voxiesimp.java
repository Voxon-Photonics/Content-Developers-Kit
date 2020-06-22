	//To set up environment to compile:
	//   set path="c:\Program Files\Java\jdk1.6.0_24\bin";%path%
	//   set classpath="c:\Program Files\Java\jdk1.6.0_24\lib"
	//
	//To compile: (.java to .class)
	//   javac -cp jna.jar voxiesimp.java
	//
	//To run, make sure 64-bit voxiebox.dll is in same directory, and run:
	//   java -cp .;jna.jar voxiesimp

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Library;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.Native;

public class voxiesimp
{
	public static class point2d extends Structure
	{
		public point2d() { setAlignType(Structure.ALIGN_NONE); }
		public point2d(float xx, float yy) { setAlignType(Structure.ALIGN_NONE); x = xx; y = yy; }
		public float x, y;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"x","y"}); }
	}
	public static class point3d extends Structure
	{
		public point3d() { setAlignType(Structure.ALIGN_NONE); }
		public point3d(float xx, float yy, float zz) { setAlignType(Structure.ALIGN_NONE); x = xx; y = yy; z = zz; }
		public float x, y, z;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"x","y","z"}); }
	}
	public static class poltex_t extends Structure
	{
		public poltex_t() { setAlignType(Structure.ALIGN_NONE); }
		public void set(float xx, float yy, float zz, float uu, float vv, int ccol) { x = xx; y = yy; z = zz; u = uu; v = vv; col = ccol; }
		public float x, y, z, u, v;
		public int col;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"x","y","z","u","v","col"}); }
	}
	public static class pol_t extends Structure
	{
		public pol_t() { setAlignType(Structure.ALIGN_NONE); }
		public void set(float xx, float yy, float zz, int pp2) { x = xx; y = yy; z = zz; p2 = pp2; }
		public float x, y, z;
		public int p2;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"x","y","z","p2"}); }
	}
	public static class tiletype extends Structure
	{
		public tiletype() { setAlignType(Structure.ALIGN_NONE); }
		public long f, p, x, y;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"f","p","x","y"}); }
	}
	public static class voxie_inputs_t extends Structure
	{
		public voxie_inputs_t() { setAlignType(Structure.ALIGN_NONE); }
		public int bstat, obstat, dmousx, dmousy, dmousz;
		protected List getFieldOrder() { return Arrays.asList(new String[]{"bstat","obstat","dmousx","dmousy","dmousz"}); }
	}
	public static class voxie_wind_t extends Structure
	{
		public voxie_wind_t() { setAlignType(Structure.ALIGN_NONE); }
		public double freq, phase; //initial freq set once at beginning
		public double emuhang, emuvang, emudist; //can be modified later
		public point2d proj0, proj1, proj2, proj3, proj4, proj5, proj6, proj7;
		public float aspx, aspy, aspz, gamma, density;
		public int useemu, hacks, voxie_vid, voxie_aud, sndfx_aud0, sndfx_aud1, projrate, framepervol, playsamprate, playnchans, recsamprate, recnchans;
		public int usecol, drawstroke, dither, smear, mono_r, mono_g, mono_b, xdim, ydim, voxie_vol, sndfx_vol;
		public int hwsync_frame0, hwsync_phase, hwsync_amp0, hwsync_amp1, hwsync_amp2, hwsync_amp3, hwsync_pha0, hwsync_pha1, hwsync_pha2, hwsync_pha3, projnum, hwsync_levthresh, usekeystone; //can be modified later
		public int colo_r, colo_g, colo_b, HighLumenDangerProtect, menu_on_voxie, excl_audio, isrecording, mirrorx, mirrory, flip, reserved;
		protected List getFieldOrder()
		{
			return Arrays.asList(new String[]
			{
				"freq", "phase",
				"emuhang", "emuvang", "emudist",
				"proj0", "proj1", "proj2", "proj3", "proj4", "proj5", "proj6", "proj7",
				"aspx", "aspy", "aspz", "gamma", "density",
				"useemu", "hacks", "voxie_vid", "voxie_aud", "sndfx_aud0", "sndfx_aud1", "projrate", "framepervol", "playsamprate", "playnchans", "recsamprate", recnchans",
				"usecol", "drawstroke", "dither", "smear", "mono_r", "mono_g", "mono_b", "xdim", "ydim", "voxie_vol", "sndfx_vol",
				"hwsync_frame0", "hwsync_phase", "hwsync_amp0", "hwsync_amp1", "hwsync_amp2", "hwsync_amp3", "hwsync_pha0", "hwsync_pha1", "hwsync_pha2", "hwsync_pha3", "projnum", "hwsync_levthresh", "usekeystone",
				"colo_r", "colo_g", "colo_b", "HighLumenDangerProtect", "menu_on_voxie", "excl_audio", "reserved3", "mirrorx", "mirrory", "flip", "reserved",
			});
		}
	}
	public static class voxie_frame_t extends Structure
	{
		public voxie_frame_t() { setAlignType(Structure.ALIGN_NONE); }
		public long f, p, fp;
		public int x, y, usecol, drawplanes, x0, y0, x1, y1;
		public float xmul, ymul, zmul, xadd, yadd, zadd;
		public tiletype f2d;
		protected List getFieldOrder()
		{
			return Arrays.asList(new String[]
			{
				"f", "p", "fp",
				"x", "y", "usecol", "drawplanes", "x0", "y0", "x1", "y1",
				"xmul", "ymul", "zmul", "xadd", "yadd", "zadd",
				"f2d",
			});
		}
	}

	private static Voxiebox voxie = (Voxiebox)Native.loadLibrary("voxiebox",Voxiebox.class);
	public interface Voxiebox extends Library
	{
		public int    voxie_breath      (voxie_inputs_t ins);
		public void   voxie_doscreencap ();
		public void   voxie_drawbox     (voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
		public void   voxie_drawcube    (voxie_frame_t vf, point3d p, point3d r, point3d d, point3d f, int fillmode, int col);
		public float  voxie_drawheimap  (voxie_frame_t vf, String st, point3d p, point3d r, point3d d, point3d f, int colorkey, int heimin, int flags);
		public void   voxie_drawlin     (voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
		public void   voxie_drawmeshtex (voxie_frame_t vf, String st, poltex_t[] vt, int vtn, int[] mesh, int meshn, int fillmode, int col);
		public void   voxie_drawpol     (voxie_frame_t vf, pol_t[] pt, int n, int col);
		public void   voxie_drawsph     (voxie_frame_t vf, float fx, float fy, float fz, float rad, int issol, int col);
		public void   voxie_drawcone    (voxie_frame_t vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int issol, int col);
		public int    voxie_drawspr     (voxie_frame_t vf, String st, point3d p, point3d r, point3d d, point3d f, int col);
		public void   voxie_drawvox     (voxie_frame_t vf, float fx, float fy, float fz, int col);
		public void   voxie_frame_end   ();
		public int    voxie_frame_start (voxie_frame_t vf);
		public int    voxie_keyread     ();
		public int    voxie_keystat     (int i);
		public double voxie_klock       ();
		public void   voxie_getvw       (voxie_wind_t vw);
		public int    voxie_init        (voxie_wind_t vw);
		public int    voxie_loadini_int (voxie_wind_t vw);
		public int    voxie_playsound   (String st, int chan, int volperc0, int volperc1, float frqmul);
		public void   voxie_printalph   (voxie_frame_t vf, point3d p, point3d r, point3d d, int col, String st);
		public void   voxie_quitloop    ();
	 //public void   voxie_setaudplaycb(void userplayfunc(int[] sampbuf, int nsamps)); ???
	 //public void   voxie_setaudreccb (void userrecfunc(int[] sampbuf, int nsamps)); ???
		public void   voxie_setleds     (int id, int r, int g, int b);
		public void   voxie_setview     (voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1);
		public void   voxie_uninit_int  (int i);
		public void   voxie_playsound_update (int handle, int chan, int volperc0, int volperc1, float frqmul);
	}

	public static void main (String[] args) throws Exception
	{
		voxie_wind_t vw = new voxie_wind_t();
		voxie_frame_t vf = new voxie_frame_t();
		voxie_inputs_t ins = new voxie_inputs_t();
		pol_t pt_addr = new pol_t(); pol_t pt[] = (pol_t[])pt_addr.toArray(3);
		point3d pp = new point3d(), rr = new point3d(), dd = new point3d(), ff = new point3d();
		point3d pos = new point3d(0.0f,0.0f,0.0f), inc = new point3d(0.3f,0.2f,0.1f);
		double tim = 0.0, otim, dtim;
		int i = 0, mousx = 0, mousy = 0, mousz = 0;

		voxie.voxie_loadini_int(vw); //get settings from KENTEST.INI. May override settings in vw structure here if desired.
		voxie.voxie_init(vw); //Start video and audio.

		while (voxie.voxie_breath(ins) == 0)
		{
			otim = tim; tim = voxie.voxie_klock(); dtim = tim-otim;
			mousx += ins.dmousx; mousy += ins.dmousy; mousz += ins.dmousz;

			if (voxie.voxie_keystat(0x1) != 0) { voxie.voxie_quitloop(); }
			//int i = voxie.voxie_keyread(); if ((i&255) == 27) voxie.voxie_quitloop();

			if (voxie.voxie_keystat(0x39) == 1) { voxie.voxie_playsound(String.format("c:/windows/media/tada.wav"),0,100,100,1.0f); }

			i = (voxie.voxie_keystat(0x1b)&1) - (voxie.voxie_keystat(0x1a)&1);
			if (i != 0)
			{
					  if (voxie.voxie_keystat(0x2a)+voxie.voxie_keystat(0x36) != 0) vw.emuvang = Math.min(Math.max(vw.emuvang+(float)i*dtim*2.0,-Math.PI*.5),0.1268); //Shift+[,]
				else if (voxie.voxie_keystat(0x1d)+voxie.voxie_keystat(0x9d) != 0) vw.emudist = Math.max(vw.emudist-(float)i*dtim*2048.0,2000.0); //Ctrl+[,]
				else                                                               vw.emuhang += (float)i*dtim*2.0; //[,]

				//if ((voxie.voxie_keystat(0x2a) == 0) && (voxie.voxie_keystat(0x36) == 0)) //no shifts down
				//   { vw.emuhang += (float)((float)i*dtim*2.0); } //[,]
				//else
				//{
				//   vw.emuvang += (float)((float)i*dtim*2.0); //Shift+[,]
				//   vw.emuvang = Math.min(Math.max(vw.emuvang,(float)Math.PI*-0.5f),0.1268f);
				//}

				voxie.voxie_init(vw);
			}

			voxie.voxie_frame_start(vf);
			voxie.voxie_setview(vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

				//draw wireframe box
			voxie.voxie_drawbox(vf,-vw.aspx+1e-3f,-vw.aspy+1e-3f,-vw.aspz,vw.aspx-1e-3f,vw.aspy-1e-3f,vw.aspz,1,0xffffff);

				//draw various primitives
			voxie.voxie_drawvox(vf,-0.5f,0.0f,0.0f,0xffffff);
			voxie.voxie_drawlin(vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz,0xffffff);
			voxie.voxie_drawsph(vf,pos.x,pos.y,pos.z,0.1f,0,0xffffff);

			pt[0].set(-0.833f,-0.178f,-vw.aspz,1);
			pt[1].set(-0.298f,-0.178f,+   0.0f,2);
			pt[2].set(-0.536f,+0.129f,+vw.aspz,0);
			voxie.voxie_drawpol(vf,pt,3,0xffffff);

			rr.x = 0.2f; dd.x = 0.0f;                      pp.x = -0.7f;
			rr.y = 0.0f; dd.y = (float)Math.cos(tim)*0.4f; pp.y = +0.7f + dd.y*-.5f;
			rr.z = 0.0f; dd.z = (float)Math.sin(tim)*0.4f; pp.z = dd.z*-.5f;
			voxie.voxie_printalph(vf,pp,rr,dd,0xffffff,String.format("HI * " + (int)tim));

			rr.x = 0.39f; dd.x = 0.00f; ff.x = 0.00f; pp.x = ((float)(mousx&511))/256.0f-1.0f;
			rr.y = 0.00f; dd.y = 0.39f; ff.y = 0.00f; pp.y = ((float)(mousy&511))/256.0f-1.0f;
			rr.z = 0.00f; dd.z = 0.00f; ff.z = 0.39f; pp.z = 0.0f;
			voxie.voxie_drawspr(vf,String.format("caco.kv6"),pp,rr,dd,ff,0x808080);
			pp.x += (rr.x + dd.x + ff.x)*-0.5f;
			pp.y += (rr.y + dd.y + ff.y)*-0.5f;
			pp.z += (rr.z + dd.z + ff.z)*-0.5f;
			voxie.voxie_drawcube(vf,pp,rr,dd,ff,1,0xffffff);

				//make pos bounce around
			pos.x += inc.x*(float)dtim; if (Math.abs(pos.x) > vw.aspx-0.1) { inc.x *= -1.0; }
			pos.y += inc.y*(float)dtim; if (Math.abs(pos.y) > vw.aspy-0.1) { inc.y *= -1.0; }
			pos.z += inc.z*(float)dtim; if (Math.abs(pos.z) > vw.aspz-0.1) { inc.z *= -1.0; }

			voxie.voxie_frame_end(); voxie.voxie_getvw(vw);
		}
		voxie.voxie_uninit_int(0);
	}
}
