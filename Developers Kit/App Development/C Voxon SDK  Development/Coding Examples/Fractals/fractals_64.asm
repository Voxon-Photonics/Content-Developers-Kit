data segment align(64)

ALIGN 32
qq131072i   dd 8 dup (131072)
qq01234567i dd 0,1,2,3,4,5,6,7
qq12345670i dd 1,2,3,4,5,6,7,0
qq65535i    dd 8 dup (65535)
qq65536i    dd 8 dup (65536)
qq40000000i dd 8 dup (40000000h)
qqn28f      dd 8 dup (-28.f)
qqn20f      dd 8 dup (-20.f)
qqn16f      dd 8 dup (-16.f)
qqn8f       dd 8 dup ( -8.f)
qqn4f       dd 8 dup ( -4.f)
qq6f        dd 8 dup (  6.f)
qq64f       dd 8 dup ( 64.f)
qq68f       dd 8 dup ( 68.f)

extrn compactlut:near
extrn palcur2:near

extrn qqxoff:ymmword
extrn qqyoff:ymmword
extrn qqzoff:ymmword
extrn qqxadd8:ymmword
extrn qqyadd8:ymmword
extrn qqzadd8:ymmword
extrn qqxmul:ymmword
extrn qqymul:ymmword
extrn qqzmul:ymmword
extrn qqpxs:ymmword
extrn qqpys:ymmword
extrn qqpzs:ymmword
extrn qqrxs:ymmword
extrn qqrys:ymmword
extrn qqrzs:ymmword
extrn qqdxs:ymmword
extrn qqdys:ymmword
extrn qqdzs:ymmword
extrn qqfxs:ymmword
extrn qqfys:ymmword
extrn qqfzs:ymmword

.CODE

	;VCx64 inputs: rcx,rdx,r8,r9,[rsp+40],[rsp+48],...
	;VCx64 output: rax
	;     scratch: rax,    rcx,rdx,            r8-r11,        xmm0-xmm5
	;      backup:     rbx,        rsi,rdi,rbp,       r12-r15,         xmm6-xmm15

																;     rcx        rdx         r8         r9      ->r10      ->r11      ->r12         ->r13    ymm0    ymm1    ymm2
PUBLIC getvox_xform_avx2 ;void getvox_xform_avx2 (int vtn, float *wx, float *wy, float *wz, float *px, float *py, float *pz, poltex_t *vt, int ix, int iy, int iz);
	ALIGN 16
getvox_xform_avx2:
	push r12
	push r13
	vmovdqa xmmword ptr [rsp-24], xmm6
	vmovdqa xmmword ptr [rsp-40], xmm7
	vmovdqa xmmword ptr [rsp-56], xmm8

		;for(j=0;j<vtn;j+=8)
		;{
		;   *(int *)&f = (ix>>9)|0x40000000; ix += ixadd; vt[j].x = (f-3.f)*vw.aspx;
		;   *(int *)&f = (iy>>9)|0x40000000; iy += iyadd; vt[j].y = (f-3.f)*vw.aspy;
		;   *(int *)&f = (iz>>9)|0x40000000; iz += izadd; vt[j].z = (f-3.f)*vw.aspz;
		;   px[j] = wx[j] = vt[j].x*args->pr.x + vt[j].y*args->pd.x + vt[j].z*args->pf.x + args->pp.x;
		;   py[j] = wx[j] = vt[j].x*args->pr.y + vt[j].y*args->pd.y + vt[j].z*args->pf.y + args->pp.y;
		;   pz[j] = wx[j] = vt[j].x*args->pr.z + vt[j].y*args->pd.z + vt[j].z*args->pf.z + args->pp.z;
		;}

	mov r10, [rsp+16+40]
	mov r11, [rsp+16+48]
	mov r12, [rsp+16+56]
	mov r13, [rsp+16+64]

	vpbroadcastd ymm0, dword ptr [rsp+16+72]
	vpbroadcastd ymm1, dword ptr [rsp+16+80]
	vpbroadcastd ymm2, dword ptr [rsp+16+88]
	vpaddd ymm0, ymm0, qqxoff
	vpaddd ymm1, ymm1, qqyoff
	vpaddd ymm2, ymm2, qqzoff

	xor eax, eax
gv_beg:
		vcvtdq2ps ymm3, ymm0
		vcvtdq2ps ymm4, ymm1
		vcvtdq2ps ymm5, ymm2
		vmulps ymm3, ymm3, ymmword ptr qqxmul ;vw.aspx/2^31
		vmulps ymm4, ymm4, ymmword ptr qqymul ;vw.aspy/2^31
		vmulps ymm5, ymm5, ymmword ptr qqzmul ;vw.aspz/2^31
		vpaddd ymm0, ymm0, ymmword ptr qqxadd8
		vpaddd ymm1, ymm1, ymmword ptr qqyadd8
		vpaddd ymm2, ymm2, ymmword ptr qqzadd8

			;Write poltex_t vt[] which is in 'array of struct' order :/
			;ymm3:[x7 x6 x5 x4 x3 x2 x1 x0]
			;ymm4:[y7 y6 y5 y4 y3 y2 y1 y0]
			;ymm5:[z7 z6 z5 z4 z3 z2 z1 z0]
		vpunpckldq ymm6, ymm3, ymm5 ;ymm6:[z5 x5 z4 x4 z1 x1 z0 x0]
		vpunpckldq ymm7, ymm4, ymm4 ;ymm7:[y5 y5 y4 y4 y1 y1 y0 y0]
		vpunpckldq ymm8, ymm6, ymm7 ;ymm8:[-- z4 y4 x4 -- z0 y0 x0]
		vpunpckhdq ymm6, ymm6, ymm7 ;ymm6:[-- z5 y5 x5 -- z1 y1 x1]
		vmovaps xmmword ptr [r13+24*0], xmm8
		vmovups xmmword ptr [r13+24*1], xmm6
		vextractf128 xmm8, ymm8, 1
		vextractf128 xmm6, ymm6, 1
		vmovaps xmmword ptr [r13+24*4], xmm8
		vmovups xmmword ptr [r13+24*5], xmm6
		vpunpckhdq ymm6, ymm3, ymm5 ;ymm6:[z7 x7 z6 x6 z3 x3 z2 x2]
		vpunpckhdq ymm7, ymm4, ymm4 ;ymm7:[y7 y7 y6 y6 y3 y3 y2 y2]
		vpunpckldq ymm8, ymm6, ymm7 ;ymm8:[-- z6 y6 x6 -- z2 y2 x2]
		vpunpckhdq ymm6, ymm6, ymm7 ;ymm6:[-- z7 y7 x7 -- z3 y3 x3]
		vmovaps xmmword ptr [r13+24*2], xmm8
		vmovups xmmword ptr [r13+24*3], xmm6
		vextractf128 xmm8, ymm8, 1
		vextractf128 xmm6, ymm6, 1
		vmovaps xmmword ptr [r13+24*6], xmm8
		vmovups xmmword ptr [r13+24*7], xmm6

		vmovaps ymm6, ymmword ptr qqpxs
		vmovaps ymm7, ymmword ptr qqpys
		vmovaps ymm8, ymmword ptr qqpzs
		vfmadd231ps ymm6, ymm3, ymmword ptr qqrxs
		vfmadd231ps ymm7, ymm3, ymmword ptr qqrys
		vfmadd231ps ymm8, ymm3, ymmword ptr qqrzs
		vfmadd231ps ymm6, ymm4, ymmword ptr qqdxs
		vfmadd231ps ymm7, ymm4, ymmword ptr qqdys
		vfmadd231ps ymm8, ymm4, ymmword ptr qqdzs
		vfmadd231ps ymm6, ymm5, ymmword ptr qqfxs
		vfmadd231ps ymm7, ymm5, ymmword ptr qqfys
		vfmadd231ps ymm8, ymm5, ymmword ptr qqfzs
		vmovaps ymmword ptr [rdx+rax*4], ymm6 ;wx
		vmovaps ymmword ptr [r8 +rax*4], ymm7 ;wy
		vmovaps ymmword ptr [r9 +rax*4], ymm8 ;wz
		vmovaps ymmword ptr [r10+rax*4], ymm6 ;px
		vmovaps ymmword ptr [r11+rax*4], ymm7 ;py
		vmovaps ymmword ptr [r12+rax*4], ymm8 ;pz
		add r13, 24*8
		add eax, 8
		cmp eax, ecx ;vtn
		jl gv_beg

	vmovdqa xmm6, xmmword ptr [rsp-24]
	vmovdqa xmm7, xmmword ptr [rsp-40]
	vmovdqa xmm8, xmmword ptr [rsp-56]
	pop r13
	pop r12
	vzeroupper
	ret

																			  ;     rcx        rdx         r8         r9      ->r10      ->r11      ->r12        ->r13        ->r14
PUBLIC getvox_mandelbulb_avx2 ;void getvox_mandelbulb_avx2 (int wptr, float *wx, float *wy, float *wz, float *px, float *py, float *pz, int *cntind, int *outcol);
	ALIGN 16
getvox_mandelbulb_avx2:
	push rbx
	push rsi
	push rdi
	push rbp
	push r12
	push r13
	push r14
	push r15
	vmovdqa xmmword ptr [rsp-24], xmm6
	vmovdqa xmmword ptr [rsp-40], xmm7
	vmovdqa xmmword ptr [rsp-56], xmm8
	vmovdqa xmmword ptr [rsp-72], xmm9
	vmovdqa xmmword ptr [rsp-88], xmm10
	vmovdqa xmmword ptr [rsp-104], xmm11
	vmovdqa xmmword ptr [rsp-120], xmm12
	vmovdqa xmmword ptr [rsp-136], xmm13
	vmovdqa xmmword ptr [rsp-152], xmm14
	vmovdqa xmmword ptr [rsp-168], xmm15

	mov r10, [rsp+40+8*8]
	mov r11, [rsp+48+8*8]
	mov r12, [rsp+56+8*8]
	mov r13, [rsp+64+8*8]
	mov r14, [rsp+72+8*8]

	xor rbx, rbx

			;rax: temp   (mask:0-255)
			;rcx: var:   wptr
			;rdx: const: wx
			;rbx: var:   rptr
			;rsp: -
			;rbp: temp
			;rsi: temp
			;rdi: temp
			;r8:  const: wy
			;r9:  const: wz
			;r10: const: px
			;r11: const: py
			;r12: const: pz
			;r13: const: cntind
			;r14: const: outcol
			;r15: temp

		;pick off next (up to) 8 from: wx/wy/wz/px/py/pz/cntind
	vmovups ymm0, ymmword ptr [rbx*4 + rdx] ;x = wx;
	vmovups ymm1, ymmword ptr [rbx*4 + r8 ] ;y = wy;
	vmovups ymm2, ymmword ptr [rbx*4 + r9 ] ;z = wz;
	vmovups ymm15, ymmword ptr [rbx*4 + r13] ;ymm15 = cntind;
	jmp short getvox_beg

getvox_again:
		vpsubd ymm15, ymm15, ymmword ptr qq65536i
getvox_beg:
		vmulps ymm3, ymm0, ymm0 ;x2 = x*x;
		vmulps ymm4, ymm1, ymm1 ;y2 = y*y;
		vmulps ymm5, ymm2, ymm2 ;z2 = z*z;
		vmulps ymm6, ymm3, ymm3 ;x4 = x2*x2;
	  ;vmulps ymm7, ymm4, ymm4 ;y4 = y2*y2;
		vmulps ymm8, ymm5, ymm5 ;z4 = z2*z2;

			;k3 = x2 + z2;
		vaddps ymm10, ymm3, ymm5 ;k3
			;k5 = x4 + z4;
		vaddps ymm13, ymm6, ymm8
			;k4 = k3 - y2;
		vsubps ymm11, ymm10, ymm4 ;k4

			;k1 = k4*k4 - k3*y2*4.f;
		vmulps ymm9, ymm10, ymm4       ;ymm9 = k3*y2;
		vmulps ymm12, ymm11, ymm11     ;ymm12 = k4*k4;
		vfmadd132ps ymm9, ymm12, qqn4f ;ymm9 = ymm9*-4.f + ymm12;

			;k2 = k3*k3;
		vmulps ymm12, ymm10, ymm10 ;ymm12 = k3^2;
			;k2 = rsqrtss(k2*k2*k2*k3) * k1*k4*y;
		vmulps ymm14, ymm10, ymm12 ;ymm14 = k3^3;
		vmulps ymm12, ymm12, ymm12 ;ymm12 = k3^4;
		vmulps ymm12, ymm12, ymm14 ;ymm12 = k3^7;
		vrsqrtps ymm12, ymm12
		vmulps ymm12, ymm12, ymm1
		vmulps ymm1, ymm9, ymm11
		vmulps ymm12, ymm12, ymm1

			;k6 = x2*z2;
		vmulps ymm14, ymm3, ymm5

			;wx = (k6*6.f - k5)*(z2 - x2)*x*z*k2*64.f; wx += px;
		vmovaps ymm1, ymm14
		vfmsub132ps ymm1, ymm13, ymmword ptr qq6f ;ymm1 = k6*6.f - k5;
		vmulps ymm0, ymm0, ymm1
		vsubps ymm1, ymm5, ymm3 ;ymm1 = z2 - x2;
		vmulps ymm0, ymm0, ymm1
		vmulps ymm0, ymm0, ymm2
		vmulps ymm0, ymm0, ymmword ptr qq64f
		vfmadd213ps ymm0, ymm12, ymmword ptr [rbx*4 + r10] ;FMA:ymm0 = ymm0*k2 + px;

			;wy = k1*k1 - y2*k3*k4*k4*16.f; wy += py;
		vfmadd213ps ymm9, ymm9, ymmword ptr [rbx*4 + r11] ;FMA:k1 = k1*k1 + py;
		vmulps ymm1, ymm4, ymm10 ;y2 *= k3;
		vmulps ymm11, ymm11, ymm11 ;k4 *= k4;
		vmulps ymm1, ymm1, ymm11 ;y2 *= k4;
		vfmadd132ps ymm1, ymm9, ymmword ptr qqn16f ;wy += y2*-16.f; FMA:ymm1 = ymm1*-16.f + ymm9;

			;wz = ((k6*-28.f + k5)*k5 + x4*z4*68.f)*k2*-8.f; wz += pz;
		vfmadd132ps ymm14, ymm13, ymmword ptr qqn28f ;ymm13 = k6*-28.f + k5;
		vmulps ymm3, ymm6, ymm8 ;ymm3 = x4*z4;
		vmulps ymm3, ymm3, ymmword ptr qq68f ;ymm3 *= 68.f;
		vfmadd213ps ymm14, ymm13, ymm3 ;ymm2 = ymm2*k5 + ymm3;
		vmulps ymm2, ymm14, ymmword ptr qqn8f ;ymm2 *= -8.f;
		vfmadd213ps ymm2, ymm12, ymmword ptr [rbx*4 + r12] ;FMA:ymm2 = ymm2*k2 + pz;

			;if (wx*wx + wy*wy + wz*wz > 20.f) break;
		vmovaps ymm3, ymmword ptr qqn20f
		vfmadd231ps ymm3, ymm0, ymm0 ;ymm3 += ymm0*ymm0;
		vfmadd231ps ymm3, ymm1, ymm1 ;ymm3 += ymm1*ymm1;
		vfmadd231ps ymm3, ymm2, ymm2 ;ymm3 += ymm2*ymm2;
		vpsubd ymm4, ymm15, ymmword ptr qq131072i ;ymm4 = cntind-131072
		vpandn ymm3, ymm4, ymm3
		vmovmskps eax, ymm3

			;enabling this is negligibly slower :/
		;cmp eax, 255 ;if all 8 still going, skip compaction & memory stuff
		;je getvox_again

		popcnt r15d, eax ;r15: popcnt

			;rcx:wptr
			;rbx:rptr
		lea edi, [ebx+8]  ;edi: ie = min(wptr-rptr,8) = min(wptr,rptr+8)-rptr;
		cmp ecx, edi
		cmovl edi, ecx
		sub edi, ebx

		mov rsi, compactlut
		vpmovzxbd ymm4, dword ptr [rax*8 + rsi] ;ML64 bug:should be qword ptr!
		vpermd ymm0, ymm4, ymm0
		vpermd ymm1, ymm4, ymm1
		vpermd ymm2, ymm4, ymm2
		vmovups ymmword ptr [rcx*4 + rdx], ymm0      ;write wx
		vmovups ymmword ptr [rcx*4 + r8 ], ymm1      ;write wy
		vmovups ymmword ptr [rcx*4 + r9 ], ymm2      ;write wz

		vpermd ymm0, ymm4, ymmword ptr [rbx*4 + r10] ;copy&compact px
		vpermd ymm1, ymm4, ymmword ptr [rbx*4 + r11] ;copy&compact py
		vpermd ymm2, ymm4, ymmword ptr [rbx*4 + r12] ;copy&compact pz
		vpermd ymm3, ymm4, ymm15                     ;copy&compact cntind
		vpsubd ymm3, ymm3, ymmword ptr qq65536i
		vmovups ymmword ptr [rcx*4 + r10], ymm0      ;write px
		vmovups ymmword ptr [rcx*4 + r11], ymm1      ;write py
		vmovups ymmword ptr [rcx*4 + r12], ymm2      ;write pz
		vmovups ymmword ptr [rcx*4 + r13], ymm3      ;write cntind

		lea rsi, [rcx*4 + r13]  ;&cntind[wptr]

		add ebx, edi     ;rptr += min(wptr-rptr,8);
		add ecx, r15d    ;wptr += popcnt(mask);

if 1
			;for(?=popcnt;?<ie;?++)
		lea rax, [rsi + rdi*4]              ;rax: &cntind[wptr + ie*4]
		sub r15, rdi                        ;r15: popcnt - ie
		jge short endwr
		mov rbp, palcur2
	begwr:mov esi, dword ptr [rax + r15*4] ;compactlut[mask][r15 + ie]    r15: -3, -2, -1
			movzx edi, si                    ;compactlut[mask][popcnt < i < ie]
			shr esi, 16
			mov esi, [rbp + rsi*4]           ;palcur2[0..7]
			mov dword ptr [rdi*4 + r14], esi
			add r15, 1
			jl short begwr
elseif 0
			;slightly slower than ^ :/
		cmp r15d, edi ;popcnt, ie
		jge short endwr
		vpsrld ymm2, ymm3, 16
		vpermd ymm2, ymm2, ymmword ptr palcur2
	begwr:vmovd xmm1, r15d
			vpermd ymm0, ymm1, ymm3
			vpextrw esi, xmm0, 0
			vpermd ymm0, ymm1, ymm2
			vmovd dword ptr [rsi*4 + r14], xmm0
			add r15d, 1
			cmp r15d, edi ;ie
			jl short begwr
else
			;works but even slower :/
		sub edi, r15d ;ie, popcnt
		jle short endwr
		vmovd xmm7, r15d
		vpbroadcastd ymm7, xmm7
		vpaddd ymm7, ymm7, ymmword ptr qq01234567i
		vpermd ymm3, ymm7, ymm3
		vmovdqa ymm6, ymmword ptr qq12345670i
		vpsrld ymm2, ymm3, 16
		vpermd ymm2, ymm2, ymmword ptr palcur2
		vpand ymm3, ymm3, ymmword ptr qq65535i
	begwr:vmovd esi, xmm3
			vmovd dword ptr [rsi*4 + r14], xmm2
			vpermd ymm3, ymm6, ymm3
			vpermd ymm2, ymm6, ymm2
			sub edi, 1
			jnz short begwr
endif

endwr:cmp ebx, ecx
		jge short getvox_end

			;pick off next (up to) 8 from: wx/wy/wz/px/py/pz/cntind
		vmovups ymm0, ymmword ptr [rbx*4 + rdx] ;x = wx;
		vmovups ymm1, ymmword ptr [rbx*4 + r8 ] ;y = wy;
		vmovups ymm2, ymmword ptr [rbx*4 + r9 ] ;z = wz;
		vmovups ymm15, ymmword ptr [rbx*4 + r13] ;ymm15 = cntind;
		jmp getvox_beg

getvox_end:
	;mov eax, ecx
	vmovdqa xmm6, xmmword ptr [rsp-24]
	vmovdqa xmm7, xmmword ptr [rsp-40]
	vmovdqa xmm8, xmmword ptr [rsp-56]
	vmovdqa xmm9, xmmword ptr [rsp-72]
	vmovdqa xmm10, xmmword ptr [rsp-88]
	vmovdqa xmm11, xmmword ptr [rsp-104]
	vmovdqa xmm12, xmmword ptr [rsp-120]
	vmovdqa xmm13, xmmword ptr [rsp-136]
	vmovdqa xmm14, xmmword ptr [rsp-152]
	vmovdqa xmm15, xmmword ptr [rsp-168]
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rdi
	pop rsi
	pop rbx
	vzeroupper
	ret

_TEXT ENDS
END
