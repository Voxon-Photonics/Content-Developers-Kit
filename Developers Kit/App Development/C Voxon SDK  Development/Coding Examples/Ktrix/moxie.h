	//Library of cover-up functions to allow translation, rotation, and scale of all voxie() objects in scene together
	//Step 1: call moxie_setcam() with the appropriate camera parameters
	//Step 2: replace any voxie() calls with moxie() to apply the above translation

	//Init w/identity matrix
static point3d gcamp = {0.f, 0.f, 0.f};
static point3d gcamr = {1.f, 0.f, 0.f};
static point3d gcamd = {0.f, 1.f, 0.f};
static point3d gcamf = {0.f, 0.f, 1.f};
static float gscale = 1.f;

//--------------------------------------------------------------------------------------------------
void moxie_setcam (point3d *p, point3d *r, point3d *d, point3d *f)
{
	gcamp = *p; gcamr = *r; gcamd = *d; gcamf = *f;
	gscale = sqrt(r->x*r->x + r->y*r->y + r->z*r->z); //precalc for spheres & cones - only works if axes are orthogonal ;P
}
//--------------------------------------------------------------------------------------------------
void moxie_xform_pos (point3d *p)
{
	point3d t; t.x = p->x-gcamp.x; t.y = p->y-gcamp.y; t.z = p->z-gcamp.z;
	p->x = t.x*gcamr.x + t.y*gcamd.x + t.z*gcamf.x;
	p->y = t.x*gcamr.y + t.y*gcamd.y + t.z*gcamf.y;
	p->z = t.x*gcamr.z + t.y*gcamd.z + t.z*gcamf.z;
}
void moxie_invxform_pos (point3d *p)
{
	point3d t; t = *p;
	p->x = t.x*gcamr.x + t.y*gcamr.y + t.z*gcamr.z + gcamp.x;
	p->y = t.x*gcamd.x + t.y*gcamd.y + t.z*gcamd.z + gcamp.y;
	p->z = t.x*gcamf.x + t.y*gcamf.y + t.z*gcamf.z + gcamp.z;
}

void moxie_xform_ori (point3d *r, point3d *d, point3d *f, point3d *nr, point3d *nd, point3d *nf)
{
	nr->x = r->x*gcamr.x + r->y*gcamd.x + r->z*gcamf.x;
	nr->y = r->x*gcamr.y + r->y*gcamd.y + r->z*gcamf.y;
	nr->z = r->x*gcamr.z + r->y*gcamd.z + r->z*gcamf.z;
	nd->x = d->x*gcamr.x + d->y*gcamd.x + d->z*gcamf.x;
	nd->y = d->x*gcamr.y + d->y*gcamd.y + d->z*gcamf.y;
	nd->z = d->x*gcamr.z + d->y*gcamd.z + d->z*gcamf.z;
	nf->x = f->x*gcamr.x + f->y*gcamd.x + f->z*gcamf.x;
	nf->y = f->x*gcamr.y + f->y*gcamd.y + f->z*gcamf.y;
	nf->z = f->x*gcamr.z + f->y*gcamd.z + f->z*gcamf.z;
}
//--------------------------------------------------------------------------------------------------
void moxie_drawvox (voxie_frame_t *vf, float fx, float fy, float fz, int col)
{
	point3d t; t.x = fx; t.y = fy; t.z = fz; moxie_xform_pos(&t);
	voxie_drawvox(vf,t.x,t.y,t.z,col);
}
void moxie_drawbox (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col)
{
	point3d p[4];
	p[0].x = x0; p[0].y = y0; p[0].z = z0; moxie_xform_pos(&p[0]);
	p[1].x = x1; p[1].y = y0; p[1].z = z0; moxie_xform_pos(&p[1]); p[1].x -= p[0].x; p[1].y -= p[0].y; p[1].z -= p[0].z;
	p[2].x = x0; p[2].y = y1; p[2].z = z0; moxie_xform_pos(&p[2]); p[2].x -= p[0].x; p[2].y -= p[0].y; p[2].z -= p[0].z;
	p[3].x = x0; p[3].y = y0; p[3].z = z1; moxie_xform_pos(&p[3]); p[3].x -= p[0].x; p[3].y -= p[0].y; p[3].z -= p[0].z;
	voxie_drawcube(vf,&p[0],&p[1],&p[2],&p[3],fillmode,col);
}
void moxie_drawlin (voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int col)
{
	point3d p0, p1;
	p0.x = x0; p0.y = y0; p0.z = z0; moxie_xform_pos(&p0);
	p1.x = x1; p1.y = y1; p1.z = z1; moxie_xform_pos(&p1);
	voxie_drawlin(vf,p0.x,p0.y,p0.z,p1.x,p1.y,p1.z,col);
}
void moxie_drawpol (voxie_frame_t *vf, pol_t *pt, int n, int col)
{
	pol_t *npt;
	if (n <= 4096) { npt = (pol_t *)_alloca(n*sizeof(pol_t)); }
				 else { npt = (pol_t *)malloc (n*sizeof(pol_t)); }
	memcpy(npt,pt,n*sizeof(pol_t));
	for(int i=n-1;i>=0;i--)
	{
		point3d t; t.x = npt[i].x; t.y = npt[i].y; t.z = npt[i].z; moxie_xform_pos(&t);
		npt[i].x = t.x; npt[i].y = t.y; npt[i].z = t.z;
	}
	voxie_drawpol(vf,npt,n,col);
	if (n > 4096) free(npt);
}
void moxie_drawmeshtex (voxie_frame_t *vf, char *fnam, poltex_t *vt, int vtn, int *mesh, int meshn, int flags, int col)
{
	poltex_t *nvt;
	if (vtn <= 4096) { nvt = (poltex_t *)_alloca(vtn*sizeof(poltex_t)); }
					else { nvt = (poltex_t *)malloc (vtn*sizeof(poltex_t)); }
	memcpy(nvt,vt,vtn*sizeof(poltex_t));
	for(int i=vtn-1;i>=0;i--)
	{
		point3d t; t.x = nvt[i].x; t.y = nvt[i].y; t.z = nvt[i].z; moxie_xform_pos(&t);
		nvt[i].x = t.x; nvt[i].y = t.y; nvt[i].z = t.z;
	}
	voxie_drawmeshtex(vf,fnam,nvt,vtn,mesh,meshn,flags,col);
	if (vtn > 4096) free(nvt);
}
void moxie_drawsph (voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col)
{
	point3d t; t.x = fx; t.y = fy; t.z = fz; moxie_xform_pos(&t);
	voxie_drawsph(vf,t.x,t.y,t.z,rad*gscale,issol,col);
}
void moxie_drawcone (voxie_frame_t *vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col)
{
	point3d p0, p1;
	p0.x = x0; p0.y = y0; p0.z = z0; moxie_xform_pos(&p0);
	p1.x = x1; p1.y = y1; p1.z = z1; moxie_xform_pos(&p1);
	voxie_drawcone(vf,p0.x,p0.y,p0.z,r0*gscale,p1.x,p1.y,p1.z,r1*gscale,fillmode,col);
}
int moxie_drawspr (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col)
{
	point3d np = *p, nr, nd, nf; moxie_xform_pos(&np); moxie_xform_ori(r,d,f,&nr,&nd,&nf);
	return voxie_drawspr(vf,fnam,&np,&nr,&nd,&nf,col);
}
int moxie_drawspr_ext (voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col, float forcescale, float fdrawratio, int flags)
{
	point3d np = *p, nr, nd, nf; moxie_xform_pos(&np); moxie_xform_ori(r,d,f,&nr,&nd,&nf);
	return voxie_drawspr_ext(vf,fnam,&np,&nr,&nd,&nf,col,forcescale,fdrawratio,flags);
}
//--------------------------------------------------------------------------------------------------
void moxie_printalph (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *st)
{
	point3d np, nr, nd;
	np.x = p->x     ; np.y = p->y     ; np.z = p->z     ; moxie_xform_pos(&np);
	nr.x = p->x+r->x; nr.y = p->y+r->y; nr.z = p->z+r->z; moxie_xform_pos(&nr); nr.x -= np.x; nr.y -= np.y; nr.z -= np.z;
	nd.x = p->x+d->x; nd.y = p->y+d->y; nd.z = p->z+d->z; moxie_xform_pos(&nd); nd.x -= np.x; nd.y -= np.y; nd.z -= np.z;
	voxie_printalph(vf,&np,&nr,&nd,col,st);
}
void moxie_drawcube (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f, int fillmode, int col)
{
	point3d np, nr, nd, nf;
	np.x = p->x     ; np.y = p->y     ; np.z = p->z     ; moxie_xform_pos(&np);
	nr.x = p->x+r->x; nr.y = p->y+r->y; nr.z = p->z+r->z; moxie_xform_pos(&nr); nr.x -= np.x; nr.y -= np.y; nr.z -= np.z;
	nd.x = p->x+d->x; nd.y = p->y+d->y; nd.z = p->z+d->z; moxie_xform_pos(&nd); nd.x -= np.x; nd.y -= np.y; nd.z -= np.z;
	nf.x = p->x+f->x; nf.y = p->y+f->y; nf.z = p->z+f->z; moxie_xform_pos(&nf); nf.x -= np.x; nf.y -= np.y; nf.z -= np.z;
	voxie_drawcube(vf,&np,&nr,&nd,&nf,fillmode,col);
}
float moxie_drawheimap (voxie_frame_t *vf, char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags)
{
	point3d np = *p, nr, nd, nf; moxie_xform_pos(&np); moxie_xform_ori(r,d,f,&nr,&nd,&nf);
	return voxie_drawheimap(vf,fnam,&np,&nr,&nd,&nf,colorkey,reserved,flags);
}
void moxie_drawdicom (voxie_frame_t *vf, voxie_dicom_t *vd, const char *gfilnam, point3d *p, point3d *r, point3d *d, point3d *f, int *animn, int *loaddone)
{
	point3d np = *p, nr, nd, nf; moxie_xform_pos(&np); moxie_xform_ori(r,d,f,&nr,&nd,&nf);
	voxie_drawdicom(vf,vd,gfilnam,&np,&nr,&nd,&nf,animn,loaddone);
}
//--------------------------------------------------------------------------------------------------
	//Extension for C/C++ allowing use of this function printf-style
void moxie_printalph_ (voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist,fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
#else
	if (vsprintf((char *)&st,fmt,arglist)) st[sizeof(st)-1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	moxie_printalph(vf,p,r,d,col,st);
}
