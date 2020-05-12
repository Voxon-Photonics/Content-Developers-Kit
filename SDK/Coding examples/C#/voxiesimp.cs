#if false
voxiesimp.exe: voxiesimp.cs; csc voxiesimp.cs /o /nologo /platform:x64
!if 0
#endif

	//32-bit voxiebox.dll: /platform:x86
	//64-bit voxiebox.dll: /platform:x64

	//VOXIEBOX library example for C#. Requires voxiebox.dll to be in path.
	//To compile, type "nmake voxiesimp.cs" at command prompt with csc.exe in path.
	//Code by Ken Silverman

#define FIX_AMD64_HACK

using System;
using System.Runtime.InteropServices;
//using System.Numerics; //http://www.drdobbs.com/windows/64-bit-simd-code-from-c/240168851

class myprog
{
	struct point2d { public float x, y; }
	struct point3d { public float x, y, z; }
#if !FIX_AMD64_HACK
	struct pol_t { public float x, y, z; public int p2; }
#endif
	struct poltex_t { public float x, y, z, u, v; public int col; }
	struct tiletype { public IntPtr f, p, x, y; }
	struct voxie_inputs_t { public int bstat, obstat, dmousx, dmousy, dmousz; }
	//static const int MAXDISP=3;
	struct voxie_disp_t
	{
		public point2d keyst0, keyst1, keyst2, keyst3, keyst4, keyst5, keyst6, keyst7;
		public int colo_r, colo_g, colo_b;
		public int mono_r, mono_g, mono_b;
		public int mirrorx, mirrory;
	}
	struct voxie_wind_t
	{
		public int useemu; public float emuhang, emuvang, emudist;                                                                        //Emulation
		public int xdim, ydim, projrate, framepervol, usecol, dispnum, HighLumenDangerProtect; public voxie_disp_t disp0, disp1, disp2;   //Display
		public int hwsync_frame0, hwsync_phase, hwsync_amp0, hwsync_amp1, hwsync_amp2, hwsync_amp3, hwsync_pha0, hwsync_pha1, hwsync_pha2, hwsync_pha3, hwsync_levthresh, voxie_vol; //Actuator
		public int ilacemode, drawstroke, dither, smear, usekeystone, flip, menu_on_voxie; public float aspx, aspy, aspz, gamma, density; //Render
		public int sndfx_vol, voxie_aud, excl_audio, sndfx_aud0, sndfx_aud1, playsamprate, playnchans, recsamprate, recnchans;            //Audio
		public int isrecording, hacks, dispcur;                                                                                           //Misc.
		public double freq, phase;                                                                                                        //Obsolete
	}
	struct voxie_frame_t
	{
		public IntPtr f, p, fp;
		public int x, y, usecol, drawplanes, x0, y0, x1, y1;
		public float xmul, ymul, zmul, xadd, yadd, zadd;
		public tiletype f2d;
	}

	struct tri_t
	{
		public float x0, y0, z0; public int n0;
		public float x1, y1, z1; public int n1;
		public float x2, y2, z2; public int n2;
	}

	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_loadini_int(ref voxie_wind_t vw);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_getvw      (ref voxie_wind_t vw);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_init       (ref voxie_wind_t vw);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_uninit_int (int i);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_breath     (ref voxie_inputs_t ins);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_quitloop   ();
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static double voxie_klock      ();
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_keystat    (int i);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_keyread    ();
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_doscreencap();
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_setview    (ref voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_frame_start(ref voxie_frame_t vf);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_frame_end  ();
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_setleds    (int id, int r, int g, int b);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawvox    (ref voxie_frame_t vf, float fx, float fy, float fz, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawbox    (ref voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawlin    (ref voxie_frame_t vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
#if !FIX_AMD64_HACK
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawpol    (ref voxie_frame_t vf, ref pol_t pt, int n, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawmeshtex(ref voxie_frame_t vf, [MarshalAs(UnmanagedType.LPStr)] string st, ref poltex_t pt, int ptn, ref int mesh, int meshn, int fillmode, int col);
#else
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawpol    (ref voxie_frame_t vf, ref tri_t tri, int n, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawmeshtex(ref voxie_frame_t vf, [MarshalAs(UnmanagedType.LPStr)] string st, ref poltex_t pt, int ptn, ref int mesh, int meshn, int fillmode, int col);
#endif
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawsph    (ref voxie_frame_t vf, float fx, float fy, float fz, float rad, int issol, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawcone   (ref voxie_frame_t vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int issol, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_drawspr    (ref voxie_frame_t vf, [MarshalAs(UnmanagedType.LPStr)] string st, ref point3d p, ref point3d r, ref point3d d, ref point3d f, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_printalph  (ref voxie_frame_t vf, ref point3d p, ref point3d r, ref point3d d, int col, [MarshalAs(UnmanagedType.LPStr)] string st);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_drawcube   (ref voxie_frame_t vf, ref point3d p, ref point3d r, ref point3d d, ref point3d f, int fillmode, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static float  voxie_drawheimap (ref voxie_frame_t vf, [MarshalAs(UnmanagedType.LPStr)] string st, ref point3d p, ref point3d r, ref point3d d, ref point3d f, int colorkey, int heimin, int flags);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static int    voxie_playsound  ([MarshalAs(UnmanagedType.LPStr)] string st, int chan, int volperc0, int volperc1, float frqmul);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_playsound_update (int handle, int chan, int volperc0, int volperc1, float frqmul);

	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_print6x8    (int x, int y, int fcol, int bcol, [MarshalAs(UnmanagedType.LPStr)] string st);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawpix     (int x, int y, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawhlin    (int x0, int x1, int y, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawline    (float x0, float y0, float x1, float y1, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawcirc    (int xc, int yc, int r, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawrectfill(int x0, int y0, int x1, int y1, int col);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_debug_drawcircfill(int x, int y, int r, int col);


/*
		//To use this code, must put unsafe in front of Main and compile with /unsafe :/
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void mycbtype(int *ibuf, int nsamps);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_setaudplaycb([MarshalAs(UnmanagedType.FunctionPtr)] mycbtype mymix);
	[DllImport("voxiebox.dll",CallingConvention=CallingConvention.Cdecl)] extern static void   voxie_setaudreccb([MarshalAs(UnmanagedType.FunctionPtr)] mycbtype mymix);
	static int[] gcnt = {0,0}, gcnti = new int[2];
	static public unsafe void mymix (int *ibuf, int nsamps)
	{
		int i;
		for(i=0;i<nsamps;i++)
		{
			ibuf[i*2+0] = (gcnt[0]&(1<<20))-(1<<19); gcnt[0] += gcnti[0];
			ibuf[i*2+1] = (gcnt[1]&(1<<20))-(1<<19); gcnt[1] += gcnti[1];
		}
	}
*/

	static public void Main ()
	{
		voxie_wind_t vw = new voxie_wind_t();
		voxie_frame_t vf = new voxie_frame_t();
		voxie_inputs_t ins = new voxie_inputs_t();
#if !FIX_AMD64_HACK
		pol_t[] pt = new pol_t[3];
#else
		tri_t tri = new tri_t();
#endif
		//tiletype tt = new tiletype();
		point3d pp = new point3d(), rr = new point3d(), dd = new point3d(), ff = new point3d();
		point3d pos = new point3d(), inc = new point3d();
		double tim = 0.0, otim; float dtim;
		int i = 0, mousx = 256, mousy = 256, mousz = 0;

		pos.x = 0.0f; pos.y = 0.0f; pos.z = 0.0f;
		inc.x = 0.3f; inc.y = 0.2f; inc.z = 0.1f;

		voxie_loadini_int(ref vw); //get settings from KENTEST.INI. May override settings in vw structure here if desired.
		voxie_init(ref vw); //Start video and audio.

/*
		if (vw.samprate != 0)
		{
			gcnti[0] = (262<<21)/vw.samprate;
			gcnti[1] = (330<<21)/vw.samprate;
			voxie_setaudplaycb(mymix);
		}
*/

		//Console.Write("Hi {0} {1,9:##0.00000}",17,vw.emuvang);
		while (voxie_breath(ref ins) == 0)
		{
			otim = tim; tim = voxie_klock(); dtim = (float)(tim-otim);
			mousx += ins.dmousx; mousy += ins.dmousy; mousz += ins.dmousz;

			if (voxie_keystat(0x1) != 0) { voxie_quitloop(); }
			//i = voxie_keyread(); if ((i&255) == 27) voxie_quitloop();

			if (voxie_keystat(0x39) == 1) { voxie_playsound("c:/windows/media/tada.wav",0,100,100,1.0f); }

			i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1);
			if (i != 0)
			{
					  if (voxie_keystat(0x2a)+voxie_keystat(0x36) != 0) vw.emuvang = (float)Math.Min(Math.Max(vw.emuvang+(float)i*dtim*2.0,-Math.PI*0.5),0.1268); //Shift+[,]
				else if (voxie_keystat(0x1d)+voxie_keystat(0x9d) != 0) vw.emudist = (float)Math.Max(vw.emudist-(float)i*dtim*2048.0,2000.0); //Ctrl+[,]
				else                                                   vw.emuhang += (float)i*dtim*(float)2.0; //[,]
				voxie_init(ref vw);
			}

			voxie_frame_start(ref vf);
			voxie_setview(ref vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

				//draw wireframe box
			voxie_drawbox(ref vf,-vw.aspx+1e-3f,-vw.aspy+1e-3f,-vw.aspz,+vw.aspx-1e-3f,+vw.aspy-1e-3f,+vw.aspz,1,0xffffff);

				//draw various primitives
			voxie_drawvox(ref vf,-0.5f,0.0f,0.0f,0xffffff);
			voxie_drawlin(ref vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz,0xffffff);
			voxie_drawsph(ref vf,pos.x,pos.y,pos.z,0.1f,0,0xffffff);
#if !FIX_AMD64_HACK
			pt[0].x =-0.833f; pt[0].y =-0.178f; pt[0].z =-vw.aspz; pt[0].p2 = 1;
			pt[1].x =-0.298f; pt[1].y =-0.178f; pt[1].z =    0.0f; pt[1].p2 = 2;
			pt[2].x =-0.536f; pt[2].y = 0.129f; pt[2].z =+vw.aspz; pt[2].p2 = 0;
			voxie_drawpol(ref vf,ref pt[0],3,0xffffff); //NOTE:corrupt in 64-bit mode because C# does not hold array indices in consecutive memory :/
#else
			tri.x0 =-0.833f; tri.y0 =-0.178f; tri.z0 =-vw.aspz; tri.n0 = 1;
			tri.x1 =-0.298f; tri.y1 =-0.178f; tri.z1 =    0.0f; tri.n1 = 2;
			tri.x2 =-0.536f; tri.y2 = 0.129f; tri.z2 =+vw.aspz; tri.n2 = 0;
			voxie_drawpol(ref vf,ref tri,3,0xffffff);
#endif

			rr.x = 0.2f; dd.x = 0.0f;                      pp.x = -0.7f;
			rr.y = 0.0f; dd.y = (float)Math.Cos(tim)*0.4f; pp.y = +0.7f + dd.y*-.5f;
			rr.z = 0.0f; dd.z = (float)Math.Sin(tim)*0.4f; pp.z = dd.z*-.5f;
			voxie_printalph(ref vf,ref pp,ref rr,ref dd,0xffffff,"HI * " + (int)tim);

			rr.x = 0.39f; dd.x = 0.00f; ff.x = 0.00f; pp.x = ((float)(mousx&511))/256.0f-1.0f;
			rr.y = 0.00f; dd.y = 0.39f; ff.y = 0.00f; pp.y = ((float)(mousy&511))/256.0f-1.0f;
			rr.z = 0.00f; dd.z = 0.00f; ff.z = 0.39f; pp.z = 0.0f;
			voxie_drawspr(ref vf,"caco.kv6",ref pp,ref rr,ref dd,ref ff,0x808080);
			pp.x += (rr.x + dd.x + ff.x)*-0.5f;
			pp.y += (rr.y + dd.y + ff.y)*-0.5f;
			pp.z += (rr.z + dd.z + ff.z)*-0.5f;
			voxie_drawcube(ref vf,ref pp,ref rr,ref dd,ref ff,1,0xffffff);

				//make pos bounce around
			pos.x += inc.x*dtim; if (Math.Abs(pos.x) > vw.aspx-0.1) inc.x = Math.Abs(inc.x)*-Math.Sign(pos.x);
			pos.y += inc.y*dtim; if (Math.Abs(pos.y) > vw.aspy-0.1) inc.y = Math.Abs(inc.y)*-Math.Sign(pos.y);
			pos.z += inc.z*dtim; if (Math.Abs(pos.z) > vw.aspz-0.1) inc.z = Math.Abs(inc.z)*-Math.Sign(pos.z);

#if false
//Vector2f, Vector3f, or Vector4f use 128-bit xmm's
var vector1 = new Vector3f(x: 5f, y: 15f, z: 25f);
var vector2 = new Vector3f(x: 3f, y: 5f, z: 8f);
var array = new float[3];
vector1.CopyTo(array); //movaps/movups
var vector3 = vector1 + vector2; //addps
var vector3 = vector2 - vector1; //subps
var vector3 = vector1 * vector2; //mulps
var vector3 = vector1 / vector2; //divps
var areEqual = (vector1 == vector2); //cmpeqps
var areNotEqual = (vector1 != vector2); //cmpeqps
var vector4 = Vector3f.Add(vector1, vector2); //addps
var vector4 = Vector3f.Subtract(vector2, vector1); //subps
var vector4 = Vector3f.Multiply(vector1, vector2); //mulps
var vector4 = Vector3f.Divide(vector1, vector2); //divps
var vector3 = VectorMath.Min(vector1, vector2); //minps
var vector3 = VectorMath.Max(vector1, vector2); //maxps
var vector3 = VectorMath.SquareRoot(vector1); //sqrtps
var vector3 = VectorMath.Abs(vector1); //movss/shufps/movaps/andps
var dotProduct = VectorMath.DotProduct(vector1, vector2); //mulps/movaps/addps

var valuesIn = new float[] {4f, 16f, 36f, 64f, 9f, 81f, 49f, 25f, 100f, 121f, 144f, 16f, 36f, 4f, 9f, 81f};
var valuesOut = new float[valuesIn.Length];
for (int i=0;i<valuesIn.Length;i+=Vector<int>.Length) //Vector<int>.Length is 4 for SSE2
{
	 // Each vector works with 4 int values from i to i + 4 when RyuJIT produces SSE2 instructions
	 var vectorIn = new Vector<float>(valuesIn, i);
	 var vectorOut = VectorMath.SquareRoot(vectorIn);
	 vectorOut.CopyTo(valuesOut, i);
}
#endif

			double d0 = voxie_klock();
			//var v1 = new Vector3f(5f, 15f, 25f);
			//var v2 = new Vector3f(3f, 5f, 8f);
			//var v3 = v1*v2; //mulps
			int val = 0;
			for(i=0;i<16777216;i++) { val += i*i; }
			double d1 = voxie_klock();
			voxie_debug_print6x8(0,64,0xffffff,-1,"Val: " + (int)val);
			voxie_debug_print6x8(0,72,0xffffff,-1,String.Format("Time: {0:N2} ms",(double)(d1-d0)*1e3));
			//voxie_debug_print6x8(0,64,0xffffff,-1,String.Format("Time: {0} {1} {2}",v3.x,v3.y,v3.z));

			voxie_frame_end(); voxie_getvw(ref vw);
		}
		voxie_uninit_int(0);
	}
}

#if false
!endif
#endif
