// ----------------------------------------------------------------------------
// 
// 							2D ���̨�������: MAIN.C
// 
// 				Copyright (C) 1995 Sony Computer Entertainment Inc.
// 							   All Rights Reserved
// ----------------------------------------------------------------------------

// ============================================================================
// ���C�����[�`��
// ============================================================================
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsn.h>
#include "orghead.h"

struct DB Db[2];

// ������
// ----------------------------------------------------------------------------
static void initsystem(void){
	srand(138);									// �����̏�����
	initzmatrix();								// �O���[�o���}�g���N�X�̏�����
	initsysdebug();								// �f�o�b�O�p�p�����[�^������
}
//	�_�u���o�b�t�@���Ƃ̏�����
static void init_prim(struct DB *db){
	ClearOTag(db->ot, OT_DEPTH);				// ���C�� OT ������
	ClearOTag(db->ot_noclr, OT_NOCLR_DEPTH);	// �T�u OT ������
	
	base_init(db);
	foe_init(db);
	bg_init(db);
	scrn_init(db);
}
//	�e���[�`���̏�����
static void init_prim_common(void){
	base_init_common();
	foe_init_common();
	bg_init_common();
}

// ���C��
// ----------------------------------------------------------------------------
int main(void){
	struct DB *cdb;
	
	PadInit(0);								// �R���g���[��������
	ResetGraph(0);							// GPU ������
	SetGraphDebug(0);						// �f�o�b�O���x�� 0
	
	InitGeom();								// GTE �̏�����
	SetGeomOffset(GEOM_X, GEOM_Y); SetGeomScreen(SCR_Z);	// ��]���S�̐ݒ�
	
	SetDefDrawEnv(&Db[0].draw, 0,   0, 320, 240);		// �_�u���o�b�t�@�ݒ�
	SetDefDrawEnv(&Db[1].draw, 0, 240, 320, 240);
	SetDefDispEnv(&Db[0].disp, 0, 240, 320, 240);
	SetDefDispEnv(&Db[1].disp, 0,   0, 320, 240);
	
	Db[0].draw.tpage = Db[1].draw.tpage = Tpage;			// �f�t�H���g TPAGE
	Db[0].draw.isbg = Db[1].draw.isbg = 1;	// �`����ύX���̉�ʃN���A�ݒ�
	setRGB0(&Db[0].draw, 0, 0, 0); setRGB0(&Db[1].draw, 0, 0, 0);
	
	initsystem();
	
	init_texture();							// �e�N�X�`���f�[�^�̃��[�h
	mapload();								// �}�b�v�f�[�^�̃��[�h
	
	init_prim_common();						// �e���[�`���A�|���S���̏�����
	init_prim(&Db[0]);
	init_prim(&Db[1]);
	
	base_born();							// ���L�����o��
	
	SetDispMask(1);							// ��ʕ\���J�n
	
	while (1) {
		cdb = (cdb == &Db[0])? &Db[1]: &Db[0];	// GPU ��`��o�b�t�@��I��
  		
		ClearOTag(cdb->ot, OT_DEPTH);		// ���C�� OT ������
		cdb->ot[OT_SCREEN] = cdb->ot_noclr[OT_NOCLR_SCREEN];
		cdb->ot[OT_BASE]   = cdb->ot_noclr[OT_NOCLR_BASE];
		cdb->ot[OT_BG]     = cdb->ot_noclr[OT_NOCLR_BG];
											// ���C�� OT �ɃT�u OT �ڑ�
		if(padopr()) break;					// �R���g���[���p�b�h�f�[�^�ǂݍ���
		
		base_move(cdb);						// �ړ����[�`���Ăяo��
		foe_move(cdb);						// �iGPU ��`��o�b�t�@�̍X�V�j
		bg_move(cdb);
		scrn_move(cdb);
		
		DrawSync(0);						// �`��I���҂�
		VSync(0);							// �������A�҂�
		
		PutDrawEnv(&cdb->draw); PutDispEnv(&cdb->disp);
											// �t���[���_�u���o�b�t�@�؂�ւ�
		DrawOTag(cdb->ot);					// OT �`��
	}
}