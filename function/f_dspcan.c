/*************** (C) COPYRIGHT 2012 Co., Ltd****************
* File Name          : f_dspcan.c
* Author             : 	
* Version            : V0.0.1
* Date               : 08/09/2012
* Description        : DSP CAN���ߵײ�������

********************************************************************************/
#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// ����ͷ�ļ�

#include "f_funcCode.h"
#include "f_dspcan.h"



#define DEBUG_F_CAN              1



#if DEBUG_F_CAN

#if (DSP_CLOCK == 100)
	#define		DSPCAN_CLK		100000
#else
	#define		DSPCAN_CLK		30000
#endif

const	CAN_BAUD	eCanBaud[CAN_BAUD_SUM] = {
									{(DSPCAN_CLK/20/20)-1, 3, 14},	// 20Kbps	
									{(DSPCAN_CLK/20/50)-1, 3, 14},	// 50Kbps		
									{(DSPCAN_CLK/20/100)-1, 3, 14},	// 100Kbps	
									{(DSPCAN_CLK/20/125)-1, 3, 14},	// 125Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/250)-1, 3, 14},	// 250Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/500)-1, 3, 14},	// 500Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/10/1000)-1, 1, 6} //  1Mbps		1+6 + 2 +1 = 10
								};

Uint32 eCanTranEnFlag;// = 0;
Uint32 eCanReEnFlag;// = 0;
	
/*******************************************************************************
* ��������          : Uint16 InitdspECan(Uint16 baud)
* ��ڲ���			: CAN�ӿڲ����ʣ�
* ����				��CAN_INIT_TIME	 ��ʼ��������
*					  CAN_INIT_SUCC  ��ʼ���ɹ�
*					  CAN_INIT_TIMEOUT ��ʼ����ʱ
*					  CAN_INIT_BAUD_ERR �����ʳ���
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 07/29/2012
* ˵��				: ��ʼ��DSP Ecan�ӿ�
********************************************************************************/
#define		IINIT_CAN_TIME				3
Uint16 InitdspECan(Uint16 baud)		// Initialize eCAN-A module
{
	struct ECAN_REGS ECanaShadow;							// ����һ��Ӱ�ӼĴ�����ĳЩ�Ĵ���ֻ��ʹ��32λ����
	Uint32 *MsgCtrlPi;										// ��ʼ������ָ��
	Uint16	i;												// ѭ������
	static	Uint16 con = 0;
	static	Uint16 count = 0;								// ��ʱ������
	
	if (baud >= CAN_BAUD_SUM)
		return CAN_INIT_BAUD_ERR;							// �����ʳ���
	if (count > IINIT_CAN_TIME)								// ��ʼ����ʱ����
		return  CAN_INIT_TIMEOUT;
	
	EALLOW;
	if (con == 0)
	{
		GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	    // Enable pull-up for GPIO30 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	    // Enable pull-up for GPIO31 (CANTXA)

	/* Set qualification for selected CAN pins to asynch only */
	// Inputs are synchronized to SYSCLKOUT by default.  
	// This will select asynch (no qualification) for the selected pins.

		GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)   

	/* Configure eCAN-A pins using GPIO regs*/
	// This specifies which of the possible GPIO pins will be eCAN functional pins.

		GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;	// Configure GPIO30 for CANTXA operation
		GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;	// Configure GPIO31 for CANRXA operation
	
	/* Configure eCAN RX and TX pins for eCAN transmissions using eCAN regs*/  
		ECANREGS.CANTIOC.bit.TXFUNC = 1;
		ECANREGS.CANRIOC.bit.RXFUNC = 1;  

	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
										// HECC mode also enables time-stamping feature
		ECanaShadow.CANMC.all = 0;
		ECanaShadow.CANMC.bit.SRES = 1;
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// �����λCANģ��
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			// ��ȡCAN�����ƼĴ���
		ECanaShadow.CANMC.bit.SCB = 1;						// eCANģʽ				
		ECanaShadow.CANMC.bit.SUSP = 1;						// ���費�ܵ���Ӱ��
//		ECanaShadow.CANMC.bit.DBO = 1;						// ���� �����Чλ ���ֽ���ǰ
		ECanaShadow.CANMC.bit.CCR = 1;						// CPU�����޸Ĳ����ʻ�ȫ�����μĴ���
		ECanaShadow.CANMC.bit.ABO = 1;						// �Զ��ָ�����ʹ��
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// ��д���ƼĴ���
		
	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero
		MsgCtrlPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGCTRL);	// ��Ϣ������ָ��
		for (i=0; i<32; i++)
		{
			MsgCtrlPi[i<<2] = 0x00000000;					// ����������Ϣ���ƼĴ���
		}
		MsgCtrlPi = (Uint32 *)(&ECANLAMS.LAM0);				// Ϣ������ָ��
		for (i=0; i<32; i++)								// ����������μĴ���
		{
			MsgCtrlPi[i] = 0x00000000;						// 
		}
		
	/* 
		ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
		..........
		ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;
	*/    
	// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
	//	as a matter of precaution. 
		ECANREGS.CANTRR.all	= 0xFFFFFFFF;					// ��λ��������ȡ�����ڽ��еķ���
		ECANREGS.CANTA.all	= 0xFFFFFFFF;					// ���㷢����Ӧ�Ĵ���/* Clear all TAn bits */      
		ECANREGS.CANRMP.all = 0xFFFFFFFF;					// ������Ϣ����Ĵ���/* Clear all RMPn bits */      
		ECANREGS.CANGIF0.all = 0xFFFFFFFF;					// ȫ���жϱ�־/* Clear all interrupt flag bits */ 
		ECANREGS.CANGIF1.all = 0xFFFFFFFF;
		ECANREGS.CANOPC.all = 0;							// ��������ɱ�����
	/* Configure bit timing parameters for eCANA
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 1 ;            			// CPU�����޸Ĳ����ʻ�ȫ�����μĴ���
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
	*/	
		con = 1;											// ��һ�׶����
	}
    if (con == 1)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE != 1 ) 				// Wait for CCE bit to be set..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;							// ��ʼ��������
		}
		else
			con = 2;
	}
	
    if (con == 2)
	{
		ECanaShadow.CANBTC.all = 0;                         // ��ʼ��������
		ECanaShadow.CANBTC.bit.BRPREG = eCanBaud[baud].BRPREG;
		ECanaShadow.CANBTC.bit.TSEG2REG = eCanBaud[baud].TSEG2REG;
		ECanaShadow.CANBTC.bit.TSEG1REG = eCanBaud[baud].TSEG1REG; 
		ECanaShadow.CANBTC.bit.SAM = 0;
		ECANREGS.CANBTC.all = ECanaShadow.CANBTC.all;
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// ������������� Set CCR = 0
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
		con = 3;
    }
	if (con == 3)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE != 0 ) 				// Wait for CCE bit to be  cleared..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;		
		}
	}
/* Disable all Mailboxes  */

	con = 0;
	count = 0;
 	ECANREGS.CANME.all = 0;									// Required before writing the MSGIDs

    EDIS;
	
	eCanTranEnFlag = 0;                                     // ��������ʼ����־
	eCanReEnFlag = 0;
	return CAN_INIT_SUCC;									// ��ʼ���ɹ� 
}	


/*******************************************************************************
* ��������          : void InitTranMbox(Uint16 mbox)
* ��ڲ���			: mbox ������ 0~31�� 
*					  ID	��Ϣ��ʶID
* ����				����
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 07/29/2012
* ˵��				: ����CAN�������䣬�ɳ�ʼ��Ϊ�Զ�Ӧ������
********************************************************************************/
void InitTranMbox(Uint16 mbox, Uint32 msgid, Uint32 *dataPi)
{
	Uint16 id;
	Uint32 ECanaShadow, *msgIdPi;	                        //ָ�븳ֵ��ϢID��ַ

	id = mbox & 0x1f;
	eCanTranEnFlag |= 1ul <<mbox;							// �����ʼ�����ͱ�־

	msgIdPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	msgIdPi[id<<2] = msgid;								    // д��Ϣ��־��ȷ���Ƿ�Ϊ�Զ�Ӧ������
	msgIdPi[(id<<2) +1] = 8;
	
	ECanaShadow = ECANREGS.CANMD.all;
	ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANMD.all = ECanaShadow;						// ��������Ϊ��������

	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// ʹ�ܶ�Ӧ����

	msgIdPi[(id<<2) + 2] = *dataPi++;						// д�Զ�Ӧ����Ϣ��
	msgIdPi[(id<<2) + 3] = *dataPi;	
	
}

/*******************************************************************************
* ��������          : void InitReMbox(Uint16 mbox, union CANMSGID_REG msgid, union CANLAM_REG lam)
* ��ڲ���			: mbox ������ 0~31��bit7 ��1�� ����Զ��֡ ��0�����?bit6 "1"���Ǳ���
*					  msgid	��Ϣ��ʶID
*					  lam	�������μĴ���
* ����				����
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 07/29/2012
* ˵��				: ����CAN��������
********************************************************************************/
void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam)
{
	Uint16 id;
	Uint32 ECanaShadow,  *pi;								// = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	
	id = mbox & 0x1f;
	eCanReEnFlag |= 1ul << id;
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	pi[id<<2] = msgid;										// ��Ϣ��ʶ�Ĵ���
	if ((mbox & 0x80) == 0x80)								// ����Զ��֡�����ʼ��
		pi[(id<<2) +1] = 1<<4 | 8;							// ��Ϣ���ƼĴ���
	else
		pi[(id<<2) +1] = 8;
		
	ECanaShadow = ECANREGS.CANOPC.all;
	if ( (mbox & 0x40) == 0x40 )							// ʹ�ܸ��Ǳ�����飬����ʼ�����Ѿ������������ֹ���Ǳ���
		ECanaShadow |= 1ul<<id;
	else
		ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANOPC.all = ECanaShadow;
		
	ECanaShadow = ECANREGS.CANMD.all;						// �á�1������Ϊ��������
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANMD.all = ECanaShadow;						// 
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// ʹ�ܶ�Ӧ����
	
	pi = (Uint32 *)(&ECANLAMS.LAM0);						// ���ý������μĴ���
	pi[id] = lam;
}


/*******************************************************************************
* ��������          : Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
* ��ڲ���			: mbox ������ 0~31��
*					  ID	��Ϣ��ʶID			ֻ������ЧIDλ
* ����				��CAN_MBOX_NUM_ERROR		����ų���������δ����ʼ��Ϊ��������
*					  CAN_MBOX_BUSY				����æ
*					  CAN_MBOX_TRAN_SUCC		���ͳɹ�
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 08/25/2012
* ˵��				: ָ�����䷢�����ݣ�������뱻��ʼ��Ϊ��������
********************************************************************************/
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
{
	Uint32 ECanaShadow, *pi;
	mbox &= 0x1f;
	if ( (eCanTranEnFlag & (1ul << mbox)) != (1ul << mbox) )
	{
		return (CAN_MBOX_NUM_ERROR);						// CAN����ų�������δ��ʼ��
	}
	
	if (ECANREGS.CANTRS.all & (1ul << mbox))				// ����ϴη����Ƿ���ɣ����������־��λ
	{
		return (CAN_MBOX_BUSY);								// CAN����æ
	}
	
	ECANREGS.CANTA.all = 1ul << mbox;						// ��շ�����Ӧ��־
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);				// дID��д����
	
	msgid &= ~(0x7ul<<29);									// �������λ
	msgid |= pi[mbox<<2] & (0x7ul << 29);					// ���޸�ID����λ
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow &= ~(1ul<<mbox);
	ECANREGS.CANME.all = ECanaShadow;						// ��ֹ��Ӧ����
	
	pi[mbox<<2] = msgid;									// ��дID
	pi[(mbox<<2) + 1] = len;
	pi[(mbox<<2) + 2] = *dataPi++;							// д����
	pi[(mbox<<2) + 3] = *dataPi;
	
	ECanaShadow |= 1ul<<mbox;
	ECANREGS.CANME.all = ECanaShadow;						// ʹ�ܶ�Ӧ����	

	ECANREGS.CANTRS.all = 1ul << mbox;						// ʹ�ܷ���
	return (CAN_MBOX_TRAN_SUCC);
}

/*******************************************************************************
* ��������          : Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
* ��ڲ���			: mbox ������ 0~31��
*					  *	dataPi ���ջ���
* ����				��CAN_MBOX_NUM_ERROR		����ų���������δ����ʼ��Ϊ��������
*					  CAN_MBOX_EMPTY			���������
*					  CAN_MBOX_REC_SUCC			�������ݳɹ�
*					  CAN_MBOX_REC_OVER			�����������
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 08/25/2012
* ˵��				: �������ݽ��ջ�����
********************************************************************************/
Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
{
	Uint32 *pi;
	
	mbox &= 0x1f;
//	if ( (eCanReEnFlag & (1ul << mbox)) != (1ul << mbox))
//	{
//		return (CAN_MBOX_NUM_ERROR);						// CAN����ų�������δ��ʼ��
//	}
	if (ECANREGS.CANRMP.all & (1ul << mbox) )				// ����Ƿ��н�����Ϣ����
	{
		pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
		*dataPi++ = pi[mbox<<2];							//  ��ID��������
		*dataPi++ = pi[(mbox<<2) + 2];	
		*dataPi++ = pi[(mbox<<2) + 3];
        *dataPi   = pi[(mbox<<2) + 1] & 0xf;                // ��ȡ�������ݳ���

//		ECanaShadow = 1ul<<mbox;
		
		if (ECANREGS.CANRML.all & (1ul << mbox))			// ��������Ƿ񱻸��ǹ�
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// �����Ϣ����Ĵ���
			return (CAN_MBOX_REC_OVER);
		}	
		else
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// �����Ϣ����Ĵ���
			return (CAN_MBOX_REC_SUCC);		
		}
	}
	else
	{
		return (CAN_MBOX_EMPTY);							// CAN����գ��޿ɶ�ȡ����		
	}
}




#elif 1



#endif




