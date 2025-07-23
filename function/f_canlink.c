/*************** (C) COPYRIGHT 2012   Co., Ltd****************
* File Name          : f_canlink.c
* Author             : 
* Version            : V0
* Date               : 08/25/2012
* Description        : CAN_LINK������

********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// ����ͷ�ļ�

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"
#include "f_comm.h"
#include "f_plc.h"
#include "f_p2p.h"



#define DEBUG_F_CANLINK              1



#if DEBUG_F_CANLINK

Uint16 CanRxTxCon = 0;
// ����CAN_LINK���ջ���
struct CANLINK_REC_BUF	CanlinkRecBuf;						// ���ջ��� 4����


// �ڲ�ʹ�ú�������
#if (0 == CAN_REMOTE_EN)
void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi);    // CANʹ��Ӳ��Զ��֡
#endif
void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi);		// CAN_LINK�������ݴ���
void CanLinkConDeal(struct CANLINK_DATA_BUF *dataPi);		// ����֡����

void CanHostTx(Uint16 addr, Uint16 data);

Uint32	CanLinkChara[2] = {									// ��Ƶ����Ʒ��ʶ
							0x00000000,
							0x00002774,                     // �㴨��Ʒ����Ƶ��
							};
							
static Uint16 sBaud = 0, sSourID = 0;						// ��������Դ��ַ							

/*******************************************************************************
* ��������          : void InitCanlinkTran(Uint16 num)
* ��ڲ���			: 	����ID
						�Զ�Ӧ������
* ����				��
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: ��ʼ��CAN_LINK����
********************************************************************************/
void InitCanlinkTran(Uint16	addr, Uint32 *dataPi)
{
	union CANLINK_MsgID msgId;
	Uint16	i;
    
	addr &= CANLINK_ID_MASK;
	// 			Դվ     ����		�ʴ�	   �Զ�Ӧ��		����ʹ��   ��չλ
	msgId.all = 0xff | (0xfful<<16) | (1ul<<24) | (1ul<<29) | (0ul<<30) | (1ul<<31);

//    msgId.bit.srcSta = 0xff;								// Դ��ַ
    msgId.bit.destSta = addr;								// ������ַ
//    msgId.bit.code = 0xff;									// �������
//    msgId.bit.aq = 1;										// �ʴ��־
    msgId.bit.framf = CAN_REMOTE_FRAME;						// ֡���ͱ�־��Զ��֡
//    msgId.bit.aam = 1;										// �Զ�Ӧ���־
//    msgId.bit.ame = 0;										// ��������ʹ�ܣ�����������Ч
//    msgId.bit.ide = 1;										// ��չ֡��־

#if CAN_REMOTE_EN	
	InitTranMbox(AAM_MBOX_N, msgId.all, dataPi);			// ��ʼ���Զ�Ӧ������
#endif	
    msgId.bit.aam = 0;										// �Զ�Ӧ���־����ͨ�������䲻�Զ�Ӧ��

	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		InitTranMbox(TRAN_BOX_N-i, msgId.all, dataPi);		// ��ʼ����������
	}
}

/*******************************************************************************
* ��������          : Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
* ��ڲ���			: 	��ϢID
						��������
* ����				��	CANLINK_RT_SUCC		���ݷ��ͳɹ�
*						CANLINK_RT_BUSY		����æ
*						CANLINK_RT_TIMEOUT	���ͳ�ʱ
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: CAN_LINK����
********************************************************************************/
Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
{
	Uint16 stat, i;
	Uint32 msgid;
	static Uint16 count = 0;
	
	msgid = *dataPi++;
	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		stat = eCanDataTran(TRAN_BOX_N-i, len, msgid, dataPi);
		if (CAN_MBOX_TRAN_SUCC == stat)
		{
			count = 0;
			return (CANLINK_RT_SUCC);						// ���ͳɹ�		
		}
	}
	if (++count >= timeOut)
	{
		count = 0;
		return (CANLINK_RT_TIMEOUT);						// ���ͳ�ʱ
	}
	else
	{
		return (CANLINK_RT_BUSY);							// ��������æ
	}
}

/*******************************************************************************
* ��������          : void InitCanlinkRec(Uint16 addr)
* ��ڲ���			: ����ID
* ����				��
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: ��ʼ��CAN_LINK����
********************************************************************************/
void InitCanlinkRec(Uint16 addr)
{
	union CANLINK_MsgID msgId;
	Uint32 lam;
	Uint16	i;
    
	addr &= CANLINK_ID_MASK;
	// 			  ����ʹ��   ��չλ		��֡		
	msgId.all =  (1ul<<30) | (1ul<<31);// | (1ul<<24);

//    msgId.bit.srcSta = 0xff;								// Դ��ַ
    msgId.bit.destSta = addr;								// ����Ŀ���ַ�� ��վ��ַ
//    msgId.bit.code = 0xff;									// �������
//    msgId.bit.aq = 1;										// �ʴ��־
//    msgId.bit.framf = CAN_REMOTE_FRAME;						// ֡���ͱ�־��Զ��֡
//    msgId.bit.aam = 1;										// �Զ�Ӧ���־
//    msgId.bit.ame = 0;										// ��������ʹ�ܣ�����������Ч
//    msgId.bit.ide = 1;										// ��չ֡��־
	lam = (~(0xfful<<8)) & ( ~(7ul<<29));// & (~(1ul<<24));		// ֻ�ܽ�����չ֡ "0"�˲�ƥ��
// 			Ŀ���ַ	  ֻ������չ֡		��֡			// �������ݲ����ʡ���֡������PLC������֡����
	for (i=0; i<REC_MBOX_NUM-1; i++)
	{
		InitRecMbox((REC_BOX_N-i) | 0x40, msgId.all, lam);	// ��ʼ����������
	}
	InitRecMbox((REC_BOX_N-i), msgId.all, lam);				// ����������������	
	
//	CanlinkRecBuf.num = 0;                                  // �������ݼ���ֵΪ��
}

/*******************************************************************************
* ��������          : Uint16 CanlinkDataRec(Uint32 msgid, Uint32 *dataPi, Uint16 timeOut)
* ��ڲ���			: 	��ϢID
						��������
* ����				��CANLINK_R_EMPTY	���������
*					  CANLINK_R_OVER    ���ճɹ������ݻ��������
*					  CANLINK_RT_SUCC	���ճɹ�
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: ��ʼ��CAN_LINK����
********************************************************************************/
Uint16 CanlinkDataRec(struct CANLINK_REC_BUF *dataPi)
{
	Uint16 i, stat;

	dataPi->bufFull = 0;
	for (i=0; i<REC_MBOX_NUM; i++)
	{
		stat = eCanDataRec(REC_BOX_N-i, (Uint32 *)(&(dataPi->buf[i])) );
		if (CAN_MBOX_EMPTY != stat)//(CAN_MBOX_REC_SUCC == stat) || (CAN_MBOX_REC_OVER == stat) )	// ���յ�����
			dataPi->bufFull |= 1<<i;						// ���ջ�����Ч
	}
	if ( 0 == dataPi->bufFull)								// δ������ 
		return (CANLINK_R_EMPTY);							// ��������գ�����
	if (CAN_MBOX_REC_OVER == stat)
		return (CANLINK_R_OVER);
	else
		return (CANLINK_RT_SUCC);
}


/*******************************************************************************
* ��������          : Uint16 InitCanlink(Uint16 addr, Uint16 baud, Uint32 *dataPi)
* ��ڲ���			: 	����ID
*						������ѡ�� 
*						�Զ�Ӧ�����ݻ���
* ����				��CAN_INIT_TIME	 ��ʼ��������
*					  CAN_INIT_SUCC  ��ʼ���ɹ�
*					  CAN_INIT_TIMEOUT ��ʼ����ʱ
*					  CAN_INIT_BAUD_ERR �����ʳ���
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: ��ʼ��CAN_LINK����ģ��
********************************************************************************/
Uint16 InitCanlink(Uint16 addr, Uint16 baud, Uint32 *dataPi)
{
	Uint16 stat;
	addr &= CANLINK_ID_MASK;								// ���˲���
//	baud &= 0x7;
	
	stat = InitdspECan(baud);								// eCANģ���ʼ��
	if (stat != CAN_INIT_SUCC)
		return (stat);
		
	InitCanlinkTran(addr, dataPi);							// ��ʼ��CAN_LINK���ͣ�
	InitCanlinkRec(addr);									// ����	
	return (CAN_INIT_SUCC);
}


/*******************************************************************************
* ��������          : void CanlinkFun(void)
* ��ڲ���			: 
* ����				��
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: CAN_LINK����ģ�飬ʹ��2ms������ò���
*					  ��Ҫ������   վID  1~63
*					  ����������	     1~2������0�ر�CAN����
********************************************************************************/
#if (1 == CAN_LINK_TIME_DEBUG)
Uint32	CanTime1, CanTime2, CanTime;
#endif
void CanlinkFun(void)
{
	Uint16 stat;
	static Uint16 timeOutErr = 0, initFlag = 0;                             // ��ʼ����ɱ�־
	

#if (1 == CAN_LINK_TIME_DEBUG)
	CanTime1 = ECANREGS.CANTSC;
#endif

#if DEBUG_F_P2P_CTRL
    // ��Ե�ͨѶ��Ч
    if (p2pData.p2pEnable)
    {
        // 1:����    2:�ӻ�
        CanRxTxCon = p2pData.p2pEnable + funcCode.code.p2pTypeSel;
    }
    else
    {
        CanRxTxCon = 0;
    }

#endif
/*                                                          // CANlinkЭ����Զִ�У����ر�
    if ((funcCode.code.commProtocolSec != PROFIBUS)         // PROFIBUS����CAN�ӿ�
        && (funcCode.code.commProtocolSec != CANLINK)       // CAN-LINK����CAN�ӿ�
        && (funcCode.code.plcEnable != 1))                  // PLC����CAN�ӿ�
    {
        initFlag = 0;                                       // ��ִ��CANģ��
        return;
    }
*/                                                           // �޸Ĳ������³�ʼ��
    if (funcCode.code.plcEnable)                            // ʹ��PLC��
    {
        if ( (sBaud != CAN_BAUD_1M) || (sSourID != INV_PLC_ID) )
		{
			sBaud = CAN_BAUD_1M;
			sSourID = INV_PLC_ID;		
			initFlag = 0;			
		}
    }
    // ��Ե�ͨѶ
    else if (funcCode.code.p2pEnable)
    {
        Uint16 p2pID;
        Uint16 p2pBaud;
        
        p2pBaud = CAN_LINK_BAUD_SEL;   // ��Ե�ͨѶ������
        
        // �ӻ� ��ַ1
        if (funcCode.code.p2pTypeSel)        {
            p2pID = COMM_P2P_SLAVE_ADDRESS;   // �ӻ���ַ
        }
        // ���� ��ַ2
        else
        {
            p2pID = COMM_P2P_MASTER_ADDRESS;
        }

        if ( (sBaud != p2pBaud) || (sSourID != p2pID) )
        {
            sBaud = p2pBaud;
            sSourID = p2pID;
            initFlag = 0;
        }
    }
    else if ( (sBaud != CAN_LINK_BAUD_SEL) || (sSourID != CAN_LINK_S_ADDR) )
    {														// ʹ��CAN_LINKЭ��	
        sBaud = CAN_LINK_BAUD_SEL;
        sSourID = CAN_LINK_S_ADDR;
        initFlag = 0;
    }

	if (initFlag == 0)										// ��ʼ��CAN_LINK
	{
        funcCode.code.u3[10] = 0;
        funcCode.code.u3[11] = 0;
        if (CAN_INIT_SUCC != InitCanlink(sSourID, sBaud, CanLinkChara) )
			return ;
		else
			initFlag = 0xcc;                                // ��ʼ���ɹ�
			
		return;		
	}

#if DEBUG_F_P2P_CTRL
    // Ϊ��Ե�ͨѶ������
    if (CanRxTxCon == P2P_COMM_HOST)                        // ֻ���͹̶�����
    {
        // ��Ե�ͨѶ��������
        p2pData.p2pSendTcnt++;
        if (p2pData.p2pSendTcnt >= p2pData.p2pSendPeriod)
        {
            // para(�ӻ��������ݵ�ַ,������������)
            CanHostTx(COMM_P2P_COMM_ADDRESS_DATA, p2pData.P2PSendData);   // ��������
            p2pData.p2pSendTcnt = 0;
        }        
        return;
    }
#endif

	stat = CanlinkDataRec(&CanlinkRecBuf);					// ����CAN����

	if ((CANLINK_RT_SUCC == stat) || (CANLINK_R_OVER == stat) )	
	{														// ��������ݳɹ�
		for (stat=0; stat<REC_MBOX_NUM; stat++)				// �����������
		{
			if ( (CanlinkRecBuf.bufFull & (1u<<stat) )== (1<<stat))
			{
//				CanlinkRecBuf.buf[stat].msgid.all -= 1;
															// ����CAN_LINK���ݴ�����
				CanLlinkDataDeal((struct CANLINK_DATA_BUF *)(&CanlinkRecBuf.buf[stat]) );
			}	
		}
        timeOutErr = 0;
        funcCode.code.u3[10] = 0;
	}
    else                                                    // δ���յ�����
    {
        if (++timeOutErr > 4)                               // 0~4������10ms
        {
            if (funcCode.code.u3[10] < 65535)
                funcCode.code.u3[10]++;                     // ��ʱ����

            // �ɱ�̹�����Чʱ����
            if ((funcCode.code.plcEnable)
                && ((curTime.powerOnTimeSec > 10) // �ϵ�ʱ�䳬��10����ж�
                || (curTime.powerOnTimeM > 0))  
                )
            {
                errorOther = ERROR_COMM;
                errorInfo = COMM_ERROR_PLC;
            }
                
            timeOutErr = 0;
        }
    }
#if (1 == CAN_LINK_TIME_DEBUG)	
	CanTime2 = ECANREGS.CANTSC;
	CanTime = CanTime2 - CanTime1;
#endif    
}

/*******************************************************************************
* ��������          : void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: 
* ����				��
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: CAN_LINKЭ��������ݴ���
********************************************************************************/
void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
{
	switch (dataPi->msgid.bit.framf)						// ֡���ʹ���
	{
#if (0 == CAN_REMOTE_EN)
        case CAN_REMOTE_FRAME:								// CAN_LINKԶ��֡�������
			CanLinkRemoteDeal(dataPi);
			break;
#endif
        case CAN_CONTROL_FRAME:								// ����֡����
			CanLinkConDeal(dataPi);
			break;
		case CAN_CONFIG_FRAME:								// ����֡����

			break;
		case CAN_DATA_FRAME:								// ����֡����
			PlcDataFramDeal(dataPi);
		
			break;
			
		default:
            if (funcCode.code.u3[11] < 65535)
                funcCode.code.u3[11]++;                     // ���ݳ������
			break;				
			
	}		
}


/*******************************************************************************
* ��������          : void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: ���ݻ���ָ��
* ����				��
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: CAN_LINKԶ��֡�������
********************************************************************************/
#if (0 == CAN_REMOTE_EN)
void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi)
{
	if (dataPi->msgid.bit.aq == CANlink_ACK)                // �յ�Ӧ��֡������
        return;
//	dataPi->msgid.bit.framf = CAN_REMOTE_FRAME;			    //
    dataPi->msgid.bit.destSta = sSourID;					// ������ַ
//	dataPi->msgid.bit.srcSta = 0xff	;						// 
    dataPi->msgid.bit.aq = CANlink_ACK;						// Զ��֡ʹ�ø�λ��Ӧ
//	dataPi->msgid.bit.code = 0xff;							//
	dataPi->mdl.all= CanLinkChara[0];
	dataPi->mdh.all= CanLinkChara[1];
    
    funcCode.code.u3[10] = 0;
    funcCode.code.u3[11] = 0;
    
	CanlinkDataTran((Uint32*)(dataPi), 8, 1000);			// ����Զ��֡��Ӧ

    

}
#endif

//===================================================================
// ��������: ��̨д����
// ����    ��addr  ��ַ
//			 dat   д����
//           eeprom дEEPROM ʹ��
// return  ��ִ��״̬(Ϊ0ִ�гɹ�)
// ����    : Yanyi	
//===================================================================
Uint16 CanControlWriter(Uint16 addr, Uint16 data, Uint16 eeprom)
{
    Uint16 oscReturn;
	// ����д����
    if (eeprom)
        commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
    sciFlag.all = 0;                                        // ����ͨѶ״̬��־
    oscReturn = CommWrite(addr, data);
    commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;    // ���ؽ�����RAM

    return oscReturn;
}


//===================================================================
// ��������: Uint16 CanControlRead(Uint16 addr, Uint16* result)
// ��ڲ�����addr   ��ַ   
//			 dat	��ȡ����ָ��
// return	 ��ִ��״̬(Ϊ0ִ�гɹ�)
// ����	   : 	
// ˵��	   ��CAN_LINK������
//===================================================================
Uint16 CanControlRead(Uint16 addr, Uint16* result)
{
    Uint16 oscReturn;
    // ���ݶ�����
    sciFlag.all = 0;                                        // ����ͨѶ״̬��־
    oscReturn = CommRead(addr, 1);
    *result = commReadData[0];
    return oscReturn;
}

/*******************************************************************************
* ��������          : void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: ���ݻ���ָ��
* ����				��
* ����	            : 	
* �汾		        : 1
* ʱ��              : 
* ˵��				: CAN_LINKЭ������֡����
********************************************************************************/
void CanLinkConDeal(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err, len = 4;
//	struct CANLINK_DATA_BUF	txBuf;							// ���ͻ���	1����	

	if (dataPi->msgid.bit.aq == CANlink_ACK)                // �յ�Ӧ��֡������
        return;
    
	switch (dataPi->msgid.bit.code)							// �������
	{
		case CAN_LINK_R_CFG:								// ������

			break;
		case CAN_LINK_R_REG:								// ���Ĵ���
			err = CanControlRead(dataPi->mdl.data.datah, (Uint16*) (&dataPi->mdl.data.datal) );
//			len = 4;
			break;
		case CAN_LINK_W_REG:								// д�Ĵ���
			err = CanControlWriter(dataPi->mdl.data.datah, dataPi->mdl.data.datal, 0);
//			len = 4;
			break;
		case CAN_LINK_R_INFO:								// ��վ���豸��Ϣ

			break;
		case CAN_LINK_R_WAR:								// ���澯��Ϣ

			break;
		case CAN_LINK_W_EEP:								// дEEPROM
			err = CanControlWriter(dataPi->mdl.data.datah, dataPi->mdl.data.datal, 1);
//			len = 4;
			break;
// PLC��CAN�������

		case CAN_TRAN_TAB_CFG:								// ���ͱ�����
			err = InvTranTabCfg(dataPi);
//			len = 4;
			break;
		case CAN_REC_TAB_CFG:								// ���ձ�����
			err = InvRecTabCfg(dataPi);
//			len = 4;
			break;
		
		default:
			err = COMM_ERR_CMD;								// ��д������Ч
			break;				
			
	}

#if DEBUG_F_P2P_CTRL
    // Ϊ��Ե�ͨѶ�Ĵӻ�
    if (CanRxTxCon == P2P_COMM_SLAVE)                        // == 2ֻ���ղ�����
	{
        // ��Ե�ͨѶ�쳣ʱ����0
        p2pData.p2pCommErrTcnt = 0;
        return;
	}
#endif
    
	dataPi->msgid.bit.destSta = dataPi->msgid.bit.srcSta;	// дĿ���ַ
	dataPi->msgid.bit.srcSta = sSourID;						// дԴ��ַ
	dataPi->msgid.bit.aq = CANlink_ACK;						// ���־


    if (err)
    {
        dataPi->msgid.bit.code = 0xff;                      // �������������뷵��0xFF
        dataPi->mdl.data.datah = 0x8001;
	    dataPi->mdl.data.datal = err;
    }
    else if ( (sciFlag.bit.pwdPass)                         // ����ͨ�������� 0x8888
//    && (0x1f00 == dataPi->mdl.data.datah) 
    )
    {
        dataPi->mdl.data.datal = 0x8888;
	}
/*    else if (sciFlag.bit.write)                             // д���ݲ����������д����򱨴���������д
    {
	}
    else if (sciFlag.bit.read)                              // ͨѶ������ȡ����������Ҫ��ȡ
    {

    }

	if (err == COMM_ERR_CMD)								// ������Ч
	{
		dataPi->msgid.bit.code = 0xff;						// ���س�������
		len = 0;
	}else if (err == COMM_ERR_ADDR)							// ��ַ��Ч
	{
		dataPi->mdl.data.datah += 1;
	}else if (err == COMM_ERR_PARA)							// ������Ч
	{
		dataPi->mdl.data.datal += 1;	
	}
*/
    if (err)
    {
        if (funcCode.code.u3[11] < 65535)
            funcCode.code.u3[11]++;                         // ���ݳ������
    }
	
	CanlinkDataTran((Uint32*)(dataPi), len, 1000);			// ������Ӧ֡
}



/*******************************************************************************
* ��������          : void CanHostTx(void)
* ��ڲ���			: addr CAN_LINKĿ��Ĵ�����ַ
*                     data дĿ��Ĵ�������
* ����				����
* ����	            : 	
* �汾		        : 
* ʱ��              : 
* ˵��				: ʹ��CAN_LINKЭ�鷢��ָ������
********************************************************************************/
void CanHostTx(Uint16 addr, Uint16 data)
{
    struct CANLINK_DATA_BUF databuf;
    Uint16 sendDataStatus;
    
    databuf.msgid.bit.destSta = COMM_P2P_SLAVE_ADDRESS;	    // �ӻ���ַ�̶�Ϊ2
	databuf.msgid.bit.srcSta = sSourID;						// дԴ��ַ
	databuf.msgid.bit.code = CAN_LINK_W_REG;                // д�Ĵ�������
	databuf.msgid.bit.framf = CAN_CONTROL_FRAME;            // ����֡
	databuf.msgid.bit.aq = 1;								// ��֡
    databuf.mdl.data.datah = addr;                          // ָ����ַ
    databuf.mdl.data.datal = data;                          // ����
	
	sendDataStatus = CanlinkDataTran((Uint32*)(&databuf), 4, 1000);			// ������Ӧ֡

    // ���������쳣
    if (sendDataStatus)
    {
        // ����δ���ͳɹ�
        p2pData.p2pCommErrTcnt++;
    }
}
#elif 1

void CanlinkFun(void){}

#endif











