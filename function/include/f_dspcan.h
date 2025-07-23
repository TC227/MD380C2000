/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_dspcan.h
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/09/2010
* Description        : DSP CAN���ߵײ������⣬�����ļ�

********************************************************************************/
#ifndef		__f_dspcan__
#define		__f_dspcan__

// �Զ�Ӧ���������ݶ���
//#define		

// CAN�����ʺ궨��
#define		CAN_BAUD_SUM							7		// CAN����������
#define		CAN_BAUD_1M								6
#define		CAN_BAUD_500K							5
#define		CAN_BAUD_250K							4
#define		CAN_BAUD_125K							3
#define		CAN_BAUD_100K							2
#define		CAN_BAUD_50K							1
#define		CAN_BAUD_20K							0

#define		CAN_INIT_TIME							1		// ��ʼ��������
#define		CAN_INIT_SUCC							2		// ��ʼ���ɹ�
#define		CAN_INIT_TIMEOUT						3		// ��ʼ����ʱ��ʧ��
#define		CAN_INIT_BAUD_ERR						4		// ������ѡ�����

// �շ�״̬��־
#define		CAN_MBOX_TRAN_SUCC						0		// CAN��������ɹ�������ʾ���ͳɹ�
#define		CAN_MBOX_NUM_ERROR						1		// CAN����ų���
#define		CAN_MBOX_BUSY							2		// CAN����æ
#define		CAN_MBOX_EMPTY							3		// CAN��������գ�
#define		CAN_MBOX_REC_SUCC						4		// CAN����������ݳɹ�
#define		CAN_MBOX_REC_OVER						5		// CAN����������������

typedef struct 
{
	Uint16	BRPREG;
	Uint16	TSEG2REG;
	Uint16 	TSEG1REG;
}CAN_BAUD;



#define		ECANREGS			ECanaRegs					// ѡ��ʹ��CAN�ӿ�		
#define		ECANMBOXES			ECanaMboxes					// ����궨��
#define		ECANLAMS			ECanaLAMRegs				// �����˲��Ĵ�������


//typedef struct
//{
	

//}

// �ⲿ��������
extern Uint16 InitdspECan(Uint16 baud);						// DSP CANģ���ʼ��
extern void InitTranMbox(Uint16 mbox, Uint32 msgid, Uint32 *dataPi);
															// ��ʼ����������
extern void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam);
															// ��ʼ������Ϊ��������
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi);
															// ָ���������䷢������
extern Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi);		// ָ���������ݶ�ȡ
															














#endif


