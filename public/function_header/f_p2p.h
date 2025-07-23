//======================================================================
//
// Time-stamp: 
//
// P2P comm
// ��Ե�ͨѶ���ݴ���
//
//======================================================================

#ifndef __F_P2P_H__
#define __F_P2P_H__

#define  COMM_P2P_COMM_ADDRESS_DATA     0x1001  // ��Ե�ͨѶ���ݵ�ַ
#define  COMM_P2P_COMM_ADDRESS_COMMAND          // ��Ե�ͨѶ���ݵ�ַ
#define  COMM_P2P_MASTER_ADDRESS      1         // ��Ե�ͨѶ�����̶���ַ
#define  COMM_P2P_SLAVE_ADDRESS       2         // ��Ե�ͨѶ�ӻ��̶���ַ

#define P2P_OUT_TORQUE      0   // ���ת��
#define P2P_FRQ_SET         1   // ����Ƶ��
#define P2P_FRQ_AIM         2   // �趨Ƶ��
#define P2P_FRQ_FDB         3   // ����Ƶ��

#define P2P_REV_TORQUE_SET  0   // ת�ظ���
#define P2P_REV_FRQ_SET     1   // Ƶ�ʸ���

#if DEBUG_F_P2P_CTRL
typedef struct
{
    Uint16 p2pEnable;     // P2P��ǰ�Ƿ���Ч
    Uint16 P2PSendData;   // ͨѶ��������
    Uint16 P2PRevData;    // ͨѶ��������
    Uint16 processValue;  // �������ݽ������Դ����
    Uint16 p2pSendPeriod; // ͨѶ��������
    Uint16 p2pSendTcnt;   // ��Ե㷢�ͼ�ʱ
    Uint16 p2pCommErrTcnt;// ��Ե�ͨѶ�쳣��ʱ
} P2P_DATA_STRUCT;

extern P2P_DATA_STRUCT p2pData;

void P2PDataDeal(void);
#elif
#endif
#endif



