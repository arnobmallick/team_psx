/*
 *	Active sub-divide simplest sample
 *
 *	"tuto0.c" ******** for texture gouror polygon
 *
 *      ADVANCED DEVELOPMENT SEMINAR VERSION
 *			Nov,  10, 1995
 *
 *		Copyright (C) 1995  Sony Computer Entertainment
 *		All rights Reserved
 */

#include <sys/types.h>

#include <libetc.h>		/* for Control Pad :
				   PAD���g�����߂ɃC���N���[�h����K�v���� */
#include <libgte.h>		/* LIBGS uses libgte :
				   LIGS���g�����߂ɃC���N���[�h����K�v����*/
#include <libgpu.h>		/* LIBGS uses libgpu :
				   LIGS���g�����߂ɃC���N���[�h����K�v���� */
#include <libgs.h>		/* for LIBGS :
				   �O���t�B�b�N���C�u���� ���g�����߂�
				   �\���̂Ȃǂ���`����Ă��� */

#define PACKETMAX 10000		/* Max GPU packets */

#define OBJECTMAX 100		/* Max Objects :
				   �RD�̃��f���͘_���I�ȃI�u�W�F�N�g��
                                   �������邱�̍ő吔 ���`���� */

#define PACKETMAX2 (PACKETMAX*24) /* size of PACKETMAX (byte)
                                     paket size may be 24 byte(6 word) */

#define TEX_ADDR   0x80100000	/* Top Address of texture data1 (TIM FORMAT)
                                /* �e�N�X�`���f�[�^�iTIM�t�H�[�}�b�g�j
				   ���������A�h���X */

#define OT_LENGTH  8		/* bit length of OT :
				   �I�[�_�����O�e�[�u���̉𑜓x */


GsOT            Wot[2];		/* Handler of OT
                                   Uses 2 Hander for Dowble buffer
				   : �I�[�_�����O�e�[�u���n���h��
				   �_�u���o�b�t�@�̂��߂Q�K�v */

GsOT_TAG	zsorttable[2][1<<OT_LENGTH]; /* Area of OT
                                             /* �I�[�_�����O�e�[�u������ */

GsDOBJ2		object[OBJECTMAX]; /* Array of Object Handler
                                   /* �I�u�W�F�N�g�n���h��
				      �I�u�W�F�N�g�̐������K�v */

u_long          Objnum;		/* valibable of number of Objects
                                /* ���f�����O�f�[�^�̃I�u�W�F�N�g�̐���
				   �ێ����� */


GsCOORDINATE2   DWorld;  /* Coordinate for GsDOBJ2
                         /* �I�u�W�F�N�g���Ƃ̍��W�n */

SVECTOR         PWorld; /* work short vector for making Coordinate parmeter
                        /* ���W�n����邽�߂̃��[�e�[�V�����x�N�^�[ */

GsRVIEW2  view;			/* View Point Handler
                                /* ���_��ݒ肷�邽�߂̍\���� */
GsF_LIGHT pslt[3];		/* Flat Lighting Handler
                                /* ���s������ݒ肷�邽�߂̍\���� */
u_long padd,pad_prev,PadTrig;	/* Controler data :
				   �R���g���[���̃f�[�^��ێ����� */

#define square(a)               ((a)*(a))

PACKET		out_packet[2][PACKETMAX2];  /* GPU PACKETS AREA */
extern int NEARZ;

static u_char dummy_scratch[1024];

char menu[4][128] = {
  "������ ON/OFF",
  "�����I�� 1 4 16 64 256 1024 4096",
  "�X�N���b�`�p�b�h �I�� ON/OFF",
  "������ ON/OFF"
  };

#define MAX_MENU 4
#define DIVIDE_SELECT 0
#define DIVNUMBER_SELECT 1
#define SCRATCH_SELECT 2
#define SUKIMA_SELECT 3

int MenuMode   = 0;
int DivideMode = 0;
int DivideNumber = 0;
int ScratchMode = 0;
int SukimaMode = 0;

/************* MAIN START ******************************************/
main()
{
  int     i;
  GsDOBJ2 *op;			/* pointer of Object Handler
                                /* �I�u�W�F�N�g�n���h���ւ̃|�C���^ */
  int     outbuf_idx;		/* double buffer index */
  MATRIX  tmpls;
  int vcount,load,num;		/* for DEBUG */
  
  ResetCallback();
  GsInitVcount();
  
  init_all();

  KanjiFntOpen(-320+16, -120+32, 640, 200, 704, 256, 768, 256, 0, 512);
  FntLoad(960, 256);
  
  while(1)
    {
      obj_interactive();	/* interactive parameter get :
				   �p�b�h�f�[�^���瓮���̃p�����[�^������ */
      GsSetRefView2(&view);	/* Calculate World-Screen Matrix :
				   ���[���h�X�N���[���}�g���b�N�X�v�Z */
      outbuf_idx=GsGetActiveBuff();/* Get double buffer index
	                           /* �_�u���o�b�t�@�̂ǂ��炩�𓾂� */

      /* Set top address of Packet Area for output of GPU PACKETS */
      GsSetWorkBase((PACKET*)out_packet[outbuf_idx]);
      
      GsClearOt(0,0,&Wot[outbuf_idx]); /* Clear OT for using buffer
	                               /* �I�[�_�����O�e�[�u�����N���A���� */
	
	/* Calculate Local-World Matrix :
	   ���[���h�^���[�J���}�g���b�N�X���v�Z���� */
	GsGetLw(&DWorld,&tmpls);

      /* Set LWMATRIX to GTE Lighting Registers
	 /* ���C�g�}�g���b�N�X��GTE�ɃZ�b�g���� */
	GsSetLightMatrix(&tmpls);
      
      /* Calculate Local-Screen Matrix :
	 �X�N���[���^���[�J���}�g���b�N�X���v�Z���� */
      GsGetLs(&DWorld,&tmpls);
      
      /* Set LSAMTRIX to GTE Registers :
	 �X�N���[���^���[�J���}�g���b�N�X��GTE�ɃZ�b�g���� */
      GsSetLsMatrix(&tmpls);
      
      /* Perspective Translate Object and Set OT :
	 �I�u�W�F�N�g�𓧎��ϊ����I�[�_�����O�e�[�u���ɓo�^���� */
      vcount = VSync(1);
      
      if(ScratchMode==0)
	num = sort_tg_plane(out_packet[outbuf_idx],14-OT_LENGTH,
		      &Wot[outbuf_idx],getScratchAddr(0));
      else
	num = sort_tg_plane(out_packet[outbuf_idx],14-OT_LENGTH,
		      &Wot[outbuf_idx],dummy_scratch);
      
      load = VSync(1)-vcount;
      fuka_meter(&Wot[outbuf_idx],outbuf_idx,load);
      KanjiFntPrint("%s\n������ = %4d/%4d  CACHE = %s ������ = %s\n",
		    menu[MenuMode],num,square(1<<DivideNumber),
		    (ScratchMode==0?"ON":"OFF"),
		    (SukimaMode==1?"ON":"OFF"));
/*      DrawSync(0);*/
      VSync(0);			/* Wait VSYNC : V�u�����N��҂� */
      ResetGraph(1);
      padd=PadRead(1);		/* Readint Control Pad data :
				   �p�b�h�̃f�[�^��ǂݍ��� */
      
      PadTrig = (padd != pad_prev?1:0);
      pad_prev = padd;
      
      GsSwapDispBuff();		/* Swap double buffer
				   : �_�u���o�b�t�@��ؑւ��� */
      
      /* Set SCREEN CLESR PACKET to top of OT
	 : ��ʂ̃N���A���I�[�_�����O�e�[�u���̍ŏ��ɓo�^���� */
      GsSortClear(0x0,0xa0,0x80,&Wot[outbuf_idx]);
      /* Drawing OT
      /* �I�[�_�����O�e�[�u���ɓo�^����Ă���p�P�b�g�̕`����J�n���� */
      GsDrawOt(&Wot[outbuf_idx]);
      KanjiFntFlush(-1);
    }
}


obj_interactive()
{
  SVECTOR v1;
  MATRIX  tmp1;

  
  /* Rotate X
  /* �I�u�W�F�N�g��X����]������ */
  if((padd & PADRup)>0) PWorld.vx+=1*ONE/360;

  /* Rotate X
  /* �I�u�W�F�N�g��X����]������ */
  if((padd & PADRdown)>0) PWorld.vx-=1*ONE/360;
  
  /* Rotate Z
  /* �I�u�W�F�N�g��Z����]������ */
  if((padd & PADRright)>0) PWorld.vz+=1*ONE/360;
  
  /* Rotate Z
  /* �I�u�W�F�N�g��Z����]������ */
  if((padd & PADRleft)>0) PWorld.vz-=1*ONE/360;
  
  /* Translate Z
  /* �I�u�W�F�N�g��Z���ɂ����ē����� */
  if((padd & PADm)>0) DWorld.coord.t[2]-=100;
  
  /* Translate Z
  /* �I�u�W�F�N�g��Z���ɂ����ē����� */
  if((padd & PADl)>0) DWorld.coord.t[2]+=100;

  /* Translate X :
     �I�u�W�F�N�g��X���ɂ����ē����� */
  if((padd & PADLleft)>0) DWorld.coord.t[0] +=10;

  /* Translate X :
     �I�u�W�F�N�g��X���ɂ����ē����� */
  if((padd & PADLright)>0) DWorld.coord.t[0] -=10;

  /* Translate Y :
     �I�u�W�F�N�g��Y���ɂ����ē����� */
  if((padd & PADLdown)>0) DWorld.coord.t[1]+=10;

  /* Translate Y :
     �I�u�W�F�N�g��Y���ɂ����ē����� */
  if((padd & PADLup)>0) DWorld.coord.t[1]-=10;

  /* Calculate Matrix from Object Parameter and Set Coordinate :
     �I�u�W�F�N�g�̃p�����[�^����}�g���b�N�X���v�Z�����W�n�ɃZ�b�g */
  set_coordinate(&PWorld,&DWorld);

  /* select menu */
  if((padd & PADk)>0 && PadTrig)
    {
      MenuMode++;
      MenuMode%=MAX_MENU;
    }
  if(MenuMode == DIVIDE_SELECT)
    {
      if((padd & PADo)>0) DivideMode = 0;
      if((padd & PADn)>0) DivideMode = 1;
    }
  if(MenuMode == DIVNUMBER_SELECT)
    {
      if(DivideNumber>0 && (padd & PADo)>0 && PadTrig) DivideNumber--;
      if(DivideNumber<6 && (padd & PADn)>0 && PadTrig) DivideNumber++;
    }
  if(MenuMode == SCRATCH_SELECT)
    {
      if((padd & PADo)>0 && PadTrig) ScratchMode = 0;
      if((padd & PADn)>0 && PadTrig) ScratchMode = 1;
    }
  if(MenuMode == SUKIMA_SELECT)
    {
      if((padd & PADo)>0 && PadTrig) SukimaMode = 0;
      if((padd & PADn)>0 && PadTrig) SukimaMode = 1;
    }
}


/* Initialize routine
/* ���������[�`���Q */
init_all()
{
  ResetGraph(0);		/* Reset GPU : GPU���Z�b�g */
  PadInit(0);			/* Reset Controller : �R���g���[�������� */
  padd=0;			/* : �R���g���[���l������ */

#if 0
  GsInitGraph(640,480,GsINTER|GsOFSGPU,1,0);
  /* rezolution set , interrace mode
  /* �𑜓x�ݒ�i�C���^�[���[�X���[�h�j */

  GsDefDispBuff(0,0,0,0);	/* Double buffer setting
                                /* �_�u���o�b�t�@�w�� */
#endif

  GsInitGraph(640,240,GsNONINTER|GsOFSGPU,0,0);
  /* rezolution set , non interrace mode
                                /* �𑜓x�ݒ�i�m���C���^�[���[�X���[�h�j */
  GsDefDispBuff(0,0,0,240);	/* Double buffer setting
                                /* �_�u���o�b�t�@�w�� */


  GsInit3D();			/* Init 3D system /* �RD�V�X�e�������� */
  
  Wot[0].length=OT_LENGTH;	/* Set bit length of OT handler
                                /* �I�[�_�����O�e�[�u���n���h���ɉ𑜓x�ݒ� */

  Wot[0].org=zsorttable[0];	/* Set Top address of OT Area to OT handler
                                /* �I�[�_�����O�e�[�u���n���h����
				   �I�[�_�����O�e�[�u���̎��̐ݒ� */
    
  /* same setting for anoter OT handler
  /*�_�u���o�b�t�@�̂��߂�������ɂ������ݒ� */
  Wot[1].length=OT_LENGTH;
  Wot[1].org=zsorttable[1];
  
  coord_init();			/* Init coordinate : ���W��` */
  view_init();			/* Setting view point : ���_�ݒ� */
  light_init();			/* Setting Flat Light : ���s�����ݒ� */
  texture_init(TEX_ADDR);
}


view_init()			/* Setting view point /* ���_�ݒ� */
{
  /*---- Set projection,view ----*/
  GsSetProjection(1000);	/* Set projection : �v���W�F�N�V�����ݒ� */
  NEARZ = 1000/2;		/* PROJECTION/2 */
    
  /* Setting view point location /* ���_�p�����[�^�ݒ� */
  view.vpx = 0; view.vpy = 0; view.vpz = 2000;
  
  /* Setting focus point location /* �����_�p�����[�^�ݒ� */
  view.vrx = 0; view.vry = 0; view.vrz = 0;
  
  /* Setting bank of SCREEN /* ���_�̔P��p�����[�^�ݒ� */
  view.rz=0;

  /* Setting parent of viewing coordinate /* ���_���W�p�����[�^�ݒ� */
  view.super = WORLD;
  
  /* Calculate World-Screen Matrix from viewing paramter
  /* ���_�p�����[�^���Q���王�_��ݒ肷��
     ���[���h�X�N���[���}�g���b�N�X���v�Z���� */
  GsSetRefView2(&view);
}


light_init()			/* init Flat light : ���s�����ݒ� */
{
  /* Setting Light ID 0 /* ���C�gID�O �ݒ� */
  /* Setting direction vector of Light0
  /* ���s���������p�����[�^�ݒ� */
  pslt[0].vx = 20; pslt[0].vy= -100; pslt[0].vz= -100;
  
  /* Setting color of Light0
  /* ���s�����F�p�����[�^�ݒ� */
  pslt[0].r=0xd0; pslt[0].g=0xd0; pslt[0].b=0xd0;
  
  /* Set Light0 from parameters
  /* �����p�����[�^��������ݒ� */
  GsSetFlatLight(0,&pslt[0]);

  
  /* Setting Light ID 1 /* ���C�gID�P �ݒ� */
  pslt[1].vx = 20; pslt[1].vy= -50; pslt[1].vz= 100;
  pslt[1].r=0x80; pslt[1].g=0x80; pslt[1].b=0x80;
  GsSetFlatLight(1,&pslt[1]);
  
  /* Setting Light ID 2 /* ���C�gID�Q �ݒ� */
  pslt[2].vx = -20; pslt[2].vy= 20; pslt[2].vz= -100;
  pslt[2].r=0x60; pslt[2].g=0x60; pslt[2].b=0x60;
  GsSetFlatLight(2,&pslt[2]);

  /* Setting Light ID 3 /* ���C�gID�Q �ݒ� */
  pslt[2].vx = 0; pslt[2].vy= 100; pslt[2].vz= 0;
  pslt[2].r=0xa0; pslt[2].g=0xa0; pslt[2].b=0xa0;
  GsSetFlatLight(2,&pslt[2]);
  
  /* Setting Ambient : �A���r�G���g�ݒ� */
  GsSetAmbient(0,0,0);

  /* Setting default light mode /* �����v�Z�̃f�t�H���g�̕����ݒ� */
  GsSetLightMode(0);
}

coord_init()			/* Setting coordinate /* ���W�n�ݒ� */
{
  VECTOR v;
  
  /* Setting hierarchy of Coordinate /* ���W�̒�` */
  GsInitCoordinate2(WORLD,&DWorld);
  
  /* Init work vector
  /* �}�g���b�N�X�v�Z���[�N�̃��[�e�[�V�����x�N�^�[������ */
  PWorld.vx=PWorld.vy=PWorld.vz=0;
  
  /* the org point of DWold is set to Z = -40000
  /* �I�u�W�F�N�g�̌��_�����[���h��Z = -4000�ɐݒ� */
  DWorld.coord.t[2] = -4000;
}


/* Set coordinte parameter from work vector
/* ���[�e�V�����x�N�^����}�g���b�N�X���쐬�����W�n�ɃZ�b�g���� */
set_coordinate(pos,coor)
SVECTOR *pos;			/* work vector /* ���[�e�V�����x�N�^ */
GsCOORDINATE2 *coor;		/* Coordinate /* ���W�n */
{
  MATRIX tmp1;
  VECTOR v1;
  
  /* Set translation /* ���s�ړ����Z�b�g���� */
  tmp1.t[0] = coor->coord.t[0];
  tmp1.t[1] = coor->coord.t[1];
  tmp1.t[2] = coor->coord.t[2];
  
  /* : �}�g���b�N�X���[�N�ɃZ�b�g����Ă��郍�[�e�[�V������
     ���[�N�̃x�N�^�[�ɃZ�b�g */
  
  /* Rotate Matrix
  /* �}�g���b�N�X�Ƀ��[�e�[�V�����x�N�^����p������ */
  RotMatrix(pos,&tmp1);
  
  /* Set Matrix to Coordinate
  /* ���߂��}�g���b�N�X�����W�n�ɃZ�b�g���� */
  coor->coord = tmp1;

  /* Clear flag becase of changing parameter
  /* �}�g���b�N�X�L���b�V�����t���b�V������ */
  coor->flg = 0;
}

u_short		clut, tpage;

/* Load texture to VRAM :
   �e�N�X�`���f�[�^��VRAM�Ƀ��[�h���� */
texture_init(u_long *addr)
{
  GsIMAGE tim1;
  
  /* Get texture information of TIM FORMAT
     : TIM�f�[�^�̃w�b�_����e�N�X�`���̃f�[�^�^�C�v�̏��𓾂� */
  GsGetTimInfo(addr+1,&tim1);
  
  /* Load texture to VRAM : VRAM�Ƀe�N�X�`�������[�h���� */
  tpage = LoadTPage(tim1.pixel,1,0,tim1.px,tim1.py,tim1.pw*2,tim1.ph);
  
  /* Exist Color Lookup Table : �J���[���b�N�A�b�v�e�[�u�������݂��� */  
  if((tim1.pmode>>3)&0x01)
    {
      clut = LoadClut(tim1.clut,tim1.cx,tim1.cy);
    }
}

POLY_F4 fuka_packet[2];

fuka_meter(ot,idx,load)
GsOT *ot;
int idx,load;
{
  setPolyF4(&fuka_packet[idx]);
  setRGB0(&fuka_packet[idx],0xc0,0x00,0x00);
  setXYWH(&fuka_packet[idx],-320+32,120-30,load*8,8);
  addPrim(ot->org,&fuka_packet[idx]);
}

