/*************** (C) COPYRIGHT 2012 y Co., Ltd****************
* File Name          : f_canlink.h
* Author             : 	
* Version            : 
* Date               : 08/25/2012
* Description        : CAN_LINK������
					  ����31�������Զ�Ӧ������
					  ����30~16��������������
					  15~0��������������


********************************************************************************/

#ifndef	__f_canlink__
#define	__f_canlink__

#define		CAN_REMOTE_EN   		0						// CANԶ��֡ʹ��

// ֡���Ͷ���
#define		CAN_REMOTE_FRAME		0xd						// Զ��֡		1101
#define		CAN_CONTROL_FRAME		0x8						// �������֡	1000
#define		CAN_CONFIG_FRAME		0xA						// ����֡		1010
#define		CAN_DATA_FRAME			0xB						// ����֡		1011

#define     CANlink_ASK             1                       // CAN��֡֡
#define     CANlink_ACK             0                       // Ӧ��֡

// �շ�״̬��־
#define		CANLINK_RT_SUCC			0						// ���ݷ��ͳɹ�
#define		CANLINK_RT_BUSY			1						// ����æ
#define		CANLINK_RT_TIMEOUT		2						// ��ʱ
#define		CANLINK_R_EMPTY			3						// δ���յ�����
#define		CANLINK_R_OVER			4						// �������������

// ID�����α�ʶ
#define		CANLINK_ID_MASK			0x3f					// ID���α�ʶ

// ����������
#define		TRAN_MBOX_NUM			4						// ����������
#define		REC_MBOX_NUM			4						// ����������


// ���������
#define		AAM_MBOX_N				31						// �Զ�Ӧ������ţ��Զ�Ӧ������������ȼ����
#define		REC_BOX_N				30						// ���������
#define		TRAN_BOX_N				15						// ���������

// Э���������
#define		CAN_LINK_DEL			1						// ɾ��CAN_LINK������Ϣ
#define		CAN_LINK_INC			2						// �����豸CAN_LINK������Ϣ
#define		CAN_LINK_R_CFG			3						// ������
#define		CAN_LINK_R_REG			4						// ���Ĵ���
#define		CAN_LINK_W_REG			5						// д�Ĵ���
#define		CAN_LINK_R_INFO			6						// ��վ���豸��Ϣ
#define		CAN_LINK_R_WAR			7						// ���澯��Ϣ
#define		CAN_LINK_W_EEP			0x0A					// дEEPROM

#define		CAN_TRAN_TAB_CFG		0x0B					// ���ͱ�����
#define		CAN_REC_TAB_CFG			0x0C					// ���ձ�����
#define		CAN_FUN_U3_CFG			0x0D					// U3�Զ��幦��������
#define		CAN_FUN_C0_CFG			0x0E					// C0�Զ��幦��������
#define		CAN_READ_PLC_INFO		0x0F					// ��PLC���豸��Ϣ


#define		CAN_LINK_S_WAR			0x10					// ���͸澯��Ϣ
#define		CAN_LINK_Q_CFG			0x20					// ������������


// �����붨��
#define		CAN_LINK_S_ADDR			(funcCode.code.commSlaveAddress & CANLINK_ID_MASK) // ��վ��ַ
#define		CAN_LINK_BAUD_SEL		(funcCode.code.commBaudRate/1000)    // ����������

#define     P2P_COMM_HOST           1         // ��Ե�ͨѶ����
#define     P2P_COMM_SLAVE          2         // ��Ե�ͨѶ�ӻ�



// CANLINK��ϢIDλ����
struct	CANLINK_MsgID_BITS	
{
	Uint16	srcSta:8;									// Դվ��ID
	Uint16	destSta:8;									// Ŀ��վID		8
	Uint16	code:8;										// �������		16
	Uint16	aq:1;										// �ʴ��־		24
	Uint16	framf:4;									// ֡���ͱ�ʶ	25
	Uint16	aam:1;										// �Զ�Ӧ��λ	29
	Uint16	ame:1;										// ����ʹ��λ	30
	Uint16	ide:1;										// 				31
};




// CAN_LINK ���ݽṹ
union CANLINK_MsgID
{
	Uint32	all;
	struct	CANLINK_MsgID_BITS bit;
};

struct CANLINK_DATAHL
{
	Uint16 datah;
	Uint16 datal;
};

// ���ݶ���
union CANLINK_DATA
{
	Uint32 all;
	struct CANLINK_DATAHL data;	
};

// CAN_LINK������������
struct CANLINK_DATA_BUF
{
	union CANLINK_MsgID msgid;
	union CANLINK_DATA mdl;
	union CANLINK_DATA mdh;	
	Uint32 len;                                             // �������ݳ���
};

// CAN_LINK���ջ������ݽṹ����
struct CANLINK_REC_BUF
{
	Uint16 bufFull;											// ������Ч��ʶλ��bit0 "1" buf[0]������Ч
	struct CANLINK_DATA_BUF buf[REC_MBOX_NUM];
};


extern Uint16 CanRxTxCon;
extern Uint32 canLinkTicker;

extern void CanlinkFun(void);
extern Uint16 CanControlWriter(Uint16 addr, Uint16 data, Uint16 eeprom);
extern Uint16 CanControlRead(Uint16 addr, Uint16* result);
extern Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut);


#define CAN_LINK_TIME_DEBUG                 0




#endif


