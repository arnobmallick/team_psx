// ----------------------------------------------------------------------------
// 
// 							2D ���̨�������: FUNC.C
// 
// 				Copyright (C) 1995 Sony Computer Entertainment Inc.
// 							   All Rights Reserved
// ----------------------------------------------------------------------------

// ============================================================================
// �T�u���[�`��
// ============================================================================
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include "orghead.h"

#define TEX_ADR			0x80100000			// �e�N�X�`���f�[�^ ���[�h�A�h���X
#define TP_X			768					// TPAGE �ʒu
#define TP_Y			0
#define CLUT_X			0					// CLUT �ʒu
#define CLUT_Y			480

#define MAP_ADR			0x80300000			// �}�b�v�f�[�^ ���[�h�A�h���X

MATRIX	ZMatrix;
SVECTOR RotS[4];

// VecR, AngR ���牼�z��ʏ�̋�`�ʒu�i4 �_�j�����߂�
// ----------------------------------------------------------------------------
void getvar(long spd,
		   SVECTOR ror0, SVECTOR ror1, SVECTOR ror2, SVECTOR ror3,
		   SVECTOR *angr, VECTOR *vecr,
		   VECTOR *v0, VECTOR *v1, VECTOR *v2, VECTOR *v3){
	MATRIX m;
	long flg;
	rotzmatrix(angr->vz); SetRotMatrix(&ZMatrix);			// RTM�ATRV �̐ݒ�
	TransMatrix(&m, vecr); SetTransMatrix(&m);
	
	ror0.vy -= spd; ror1.vy -= spd; ror2.vy -= spd; ror3.vy -= spd;
															// �I�t�Z�b�g���l��
	RotTrans(&ror0, v0, &flg); RotTrans(&ror1, v1, &flg);	// ���W�ϊ�
	RotTrans(&ror2, v2, &flg); RotTrans(&ror3, v3, &flg);
}

// offset �������x�N�g�������߂�
// ----------------------------------------------------------------------------
void getvecoff(long offx, long offy, SVECTOR *angr, VECTOR *vecr, VECTOR *vecl){
	SVECTOR svec;
	
	setVector(&svec, offx, offy, 0);	// �x�N�g���̐ݒ�
	
	rotzmatrix(angr->vz);				// �����ł� AngR �����X���Ă���
	
	ApplyMatrix(&ZMatrix, &svec, vecl);	// ���Z
	addVector(vecl, vecr);				// ��_�x�N�g���𑫂�
}

// �\���p�� VecS �����߂�
// ----------------------------------------------------------------------------
void getvecs(long scr_z, VECTOR *vecr, VECTOR *vecs){
	SVECTOR svec;
	
	rotzmatrix(-BaseAngR.vz);			// Base �̌X�����t��]
	
	svec.vx = vecr->vx-BaseVecL.vx;	svec.vy = vecr->vy-BaseVecL.vy; svec.vz = 0;
										// Base ���猩���G�̊�_�x�N�g�������߂�
	ApplyMatrix(&ZMatrix, &svec, vecs);	// break RTM
	vecs->vz = scr_z;
}

// VecR�AAngR ���瓧���ϊ��܂ł���Ă��܂��BVecS�AAngS �̍X�V�������Ȃ�
// ----------------------------------------------------------------------------
void getvas(long spd,
			SVECTOR *angr, SVECTOR *angs, VECTOR *vecr, VECTOR *vecs,
			SVECTOR rot0, SVECTOR rot1, SVECTOR rot2, SVECTOR rot3,
			POLY_FT4 *ft4){
	MATRIX m;
	long dmy, flg;
	
	getvecs(vecs->vz, vecr, vecs);				//	VecS �����߂�
	
	angs->vz = (angr->vz)-(BaseAngR.vz);		//	AngS �����߂�
												// ��ʏ�̌X���� VecR-BaseVecR
	rotzmatrix(angs->vz); SetRotMatrix(&ZMatrix);
	TransMatrix(&m, vecs); SetTransMatrix(&m);	//	VecS, AngS �Œ萔�s��̐ݒ�
	
	rot0.vy -= spd; rot1.vy -= spd; rot2.vy -= spd; rot3.vy -= spd;
												// ��_����̋����i�I�t�Z�b�g�j�̍X�V
	RotTransPers4(&rot0, &rot1, &rot2, &rot3,
		(long *)&ft4->x0, (long *)&ft4->x1,
		(long *)&ft4->x2, (long *)&ft4->x3, &dmy, &flg);	// �����ϊ�
}

// �y����]�݂̂̃}�g���N�X�����i�����c�Ӂj
// ----------------------------------------------------------------------------
void initzmatrix(void){
	u_short i, j;
	
	for(i = 0; i < 2; i ++) for(j = 0; j < 2; j ++) ZMatrix.m[i][j] = 0;
	ZMatrix.m[2][2] = 1;
}

void rotzmatrix(short vz){
	short i;
	
	ZMatrix.m[0][0] = ZMatrix.m[1][1] = rcos(vz);
	ZMatrix.m[1][0] =  i = rsin(vz);
	ZMatrix.m[0][1] = -i;
}

// BG ����
// ----------------------------------------------------------------------------
//	VecR�AAngR ���牼�z��̋�`�ʒu�����߁A4�_���ꂼ��ɕǂ̔���
short walljudge(long spd,
			  SVECTOR ror0, SVECTOR ror1, SVECTOR ror2, SVECTOR ror3,
			  SVECTOR *angr, VECTOR *vecr){
	VECTOR v[4];
	
	getvar(spd, ror0, ror1, ror2, ror3, angr, vecr, &v[0], &v[1], &v[2], &v[3]);
												// ���z��ʏ�� 4 �_�����߂�
	return (getwall(v[0].vx, v[0].vy)  +getwall(v[1].vx, v[1].vy)*2+
			getwall(v[2].vx, v[2].vy)*4+getwall(v[3].vx, v[3].vy)*8);
												// �e�_�̔���󋵂��r�b�g�Ŏ���
}
//	�����D��̓G�p�ǔ��胋�[�`���i�P�_����j
short walljudge_2(long spd, SVECTOR *angr, VECTOR *vecr, VECTOR *vecl){
	getvecoff(0, -spd, angr, vecr, vecl);		// ���z��ʏ�� 1 �_�����߂�
	return getwall(vecl->vx, vecl->vy);
}
//	�ǂɂԂ������ꍇ�̈ړ����������߂�
void getxy(long spd, short ang, long *x, long *y){
	*x = *y = spd/2;							// �����͂P�^�Q
	switch(ang/1024){							// �e�ی����Ƃɏꍇ����
		case 0: *y *= -1; break;
		case 2: *x *= -1; break;
		case 3: *x *= -1; *y *= -1; break;
	}
}
//	�O�i�A��ގ��̓��蔻��̉�͂Ƃ��̌�̋���������i���@�p�j
void dirjudgeud(long spd, long p, long ang, VECTOR *vecl, long *x, long *y){
	short i[4], j, k;
	
	i[0] = p&0x01; i[1] = p&0x02; i[2] = p&0x04; i[3] = p&0x08;
											// �e�_�̕Ǐ������
	getxy(spd, ang, x, y);					// �p�x�ɂ���đ���������
	
	ang /= 1024; ang %= 2;
	
	if(spd >0 ){ j = 0; k = 1; }			// �� 0, 2 �ی��A�� 1, 3 �ی��͓�������
	else   	   { j = 3; k = 2; }
	
	if(i[j]){ if(ang) *x = 0;	else *y = 0; }
	if(i[k]){ if(ang) *y = 0;	else *x = 0; }
	
	if(getwall(vecl->vx+*x, vecl->vy+*y)) *x = *y = 0;
											// �����悤�Ƃ����Ƃ����܂��ǂȂ�~�܂�
}

// ============================================================================
// �f�[�^�t�@�C�����
// ============================================================================
// TIM ���[�h
// ----------------------------------------------------------------------------
//	CLUT �̈ʒu�����߃��[�h�A�e�N�X�`���̈ʒu��������
static long *clutload(long *clutadr, u_short *clut, u_short cx, u_short cy){
	if(*clutadr & (long)0x08){
		*clut = LoadClut(clutadr+4, cx, cy);
		return (long *)((char *)clutadr+4+*(clutadr+1));
	}
	else
		return clutadr+1;
}
//	�e�N�X�`���f�[�^�̃��[�h
static void pixload(long *pixadr, u_short *pix, u_short px, u_short py){
	*pix = LoadTPage(pixadr+3, 1, 0, px, py, 256, 256);
}
//	TIM �t�H�[�}�b�g�̉�͂ƃ��[�h
void load_texture(long *texadr, u_short *clut, u_short *pix,
				  u_short px, u_short py, u_short cx, u_short cy){
	texadr = clutload(texadr+1, clut, cx ,cy);
	pixload(texadr, pix, px, py);
}
//	�e�N�X�`���̓ǂݍ���
void init_texture(void){
	load_texture((u_long *)TEX_ADR, &Clut, &Tpage, TP_X, TP_Y, CLUT_X, CLUT_Y);
}

// MAP ���[�h
// ----------------------------------------------------------------------------
void mapload(void){
	u_char *pt;
	
	pcfileread("seminar.bgd",  (u_char *)MAP_ADR);			// �t�@�C���ǂݍ���
	pt = (u_char *)MAP_ADR;
	GrdWx = (u_short)*(pt+5); GrdWy = (u_short)*(pt+4);		// ���
	GrdData = (u_short *)(pt+8);
}