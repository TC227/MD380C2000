/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_plc.h
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 09/25/2010
* Description        : PLC��������������ļ�



********************************************************************************/

#ifndef __f_plc__
#define	__f_plc__

#define		PLC_CARD_ID      			1					// PLCʹ��ID
#define		INV_PLC_ID      			2					// ��Ƶ��PLCʹ��ID

#define		INV_TRAN_FAST_NUM			6					// ��Ƶ�����Ϳ��
#define		INV_TRAN_SLOW_NUM			30					// ��Ƶ����������
#define		INV_REC_FAST_NUM			6					// ��Ƶ�����տ��
#define		INV_REC_SLOW_NUM			30					// ��Ƶ����������


#define		INV_FAST_TAB_ADDR			0					// ��Ƶ��������ַ
#define		INV_SLOW_TAB_ADDR			0x100				// ��Ƶ������

#define		PLC_DISP_U3NUM				10					// PLC���ӹ����볤��
#define		PLC_FUNC_C0NUM				15					// PLCʹ�òμ��������볤��


// �Զ��幦�������Զ���
typedef struct C0_ATTRIBUTE_STRUCT
{
    Uint16                  lower;          // ����
    Uint16                  upper;          // ����
    Uint16                  init;           // ����ֵ
    union FUNC_ATTRIBUTE    attribute;      // ����
} C0_FUNCCODE_ATTRIBUTE;


// �ⲿʹ�ú�������
extern Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi);
extern Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi);
extern void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi);


#endif



