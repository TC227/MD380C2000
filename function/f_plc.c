/*************** (C) COPYRIGHT 2012 Co., Ltd****************
* File Name          : f_plc.c
* Author             : 	
* Version            : V0.0.1
* Date               : 09/25/2012
* Description        : PLC�������ļ�



********************************************************************************/
#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"
#include "f_comm.h"
#include "f_plc.h"


#define DEBUG_F_PLC              1


#if DEBUG_F_PLC

Uint16 InvTrFastIndex = 1,									// ���ͱ�����
	   InvTrSlowIndex = 1;

Uint16 InvTrFastTab[INV_TRAN_FAST_NUM+1] = {0, 0x703d};		// ��Ƶ�����Ϳ��
Uint16 InvTrSlowTab[INV_TRAN_SLOW_NUM+1] = {0, 0};			// ��������
Uint16 InvRecFastTab[INV_REC_FAST_NUM+1] = {0, 0};			// ���տ��
Uint16 InvRecSlowTab[INV_REC_SLOW_NUM+1] = {0, 0};  		// ��������

// �ڲ���������
Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode);


// �����ļ����õĺ�
// ĳЩ�����룬ͨѶ���ܽ���W
#define COMM_NO_W_FC_0  GetCodeIndex(funcCode.code.tuneCmd) // ��г
#define COMM_NO_W_FC_1  GetCodeIndex(funcCode.code.menuMode)// �˵�ģʽ
#define COMM_NO_W_FC_2  GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)      // ��г
#define COMM_NO_W_FC_3  GetCodeIndex(funcCode.code.motorFcM3.tuneCmd)      // ��г
#define COMM_NO_W_FC_4  GetCodeIndex(funcCode.code.motorFcM4.tuneCmd)      // ��г
#define COMM_NO_W_FC_5  GetCodeIndex(funcCode.code.funcParaView)           // ���ܲ˵�ģʽ����
// ĳЩ�����룬ͨѶ���ܽ���R
#define COMM_NO_R_FC_0  GetCodeIndex(funcCode.code.userPassword)            // �û�����
// ĳЩ�����룬ͨѶ���ܽ���RW
#define COMM_NO_RW_FC_0 GetCodeIndex(funcCode.code.userPasswordReadOnly)    // ֻ���û�����
#define COMM_NO_RW_FC_2  GetCodeIndex(funcCode.code.plcEnable)               // PLC����
#define COMM_READ_CURRENT_FC GetCodeIndex(funcCode.group.u0[4])              // ͨѶ��ȡ����

#define COMM_READ_FC    0       							// ͨѶ��������
#define COMM_WRITE_FC   1       							// ͨѶд������



/*******************************************************************************
* ��������          : Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: ���ݻ���ָ��
* ����				��"0" 		�����ɹ�
*					: "����"	�μ�������
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 09/25/2010
* ˵��				: ��Ƶ�����ͱ�����
********************************************************************************/
Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err;
	err = FuncCodeRwAtrrib(dataPi->mdl.data.datal, COMM_READ_FC);
	if (err)
		return (COMM_ERR_PARA); 							// �������ݴ� ��Ч����

	err = COMM_ERR_ADDR;		                            // ���ɹ���Ч��ַ
	if (dataPi->mdl.data.datah >= INV_SLOW_TAB_ADDR)		// ���÷�������
	{
		if ( (dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR) <= INV_TRAN_SLOW_NUM )
		{
			InvTrSlowTab[dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// �����ɹ�
		}
	}
	else if (dataPi->mdl.data.datah != 1)					// ���Ϳ����1����ַ�̶�������д����
	{
		if (dataPi->mdl.data.datah <= INV_TRAN_FAST_NUM)
		{
			InvTrFastTab[dataPi->mdl.data.datah] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// �����ɹ�
		}
	}
	
	return (err);
}

/*******************************************************************************
* ��������          : Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: ���ݻ���ָ��
* ����				��"0" �����ɹ�
*					: "����"	�μ�������
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 09/25/2010
* ˵��				: ��Ƶ�����ձ�����
********************************************************************************/
Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err;
	err = FuncCodeRwAtrrib(dataPi->mdl.data.datal, COMM_WRITE_FC);
	if (err)
		return (err);										// �������ݴ�

	err = COMM_ERR_PARA;									// Ԥ�ò�������		
	if (dataPi->mdl.data.datah >= INV_SLOW_TAB_ADDR)		// ���ý�������
	{
		if ( (dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR) <= INV_TRAN_SLOW_NUM )
		{
			InvRecSlowTab[dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// �����ɹ�
		}
	}
	else                                                    // ���տ��
	{
		if (dataPi->mdl.data.datah <= INV_TRAN_FAST_NUM)
		{
			InvRecFastTab[dataPi->mdl.data.datah] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// �����ɹ�
		}
	}
	return (err);
}


/*******************************************************************************
* ��������          : Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode)
* ��ڲ���			: addr 		���ʵ�ַ
*					: reMode	��дģʽ
* ����				��"0"		�ɲ���
*					����!=0��		���ɲ���
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 09/25/2010
* ˵��				: �������д�����ж�
********************************************************************************/
Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode)
{
	Uint16 index, group, grade, highH;
    Uint16 funcCodeGradeComm[FUNCCODE_GROUP_NUM];
//    Uint16 errType = COMM_ERR_NONE;
	if (addr == 0)
		return COMM_ERR_NONE;								// ����д"0"ȡ������
		
    highH = (addr & 0xF000);
// ��֤�������ַ��	
	if (
//			(highH == 0x0000)||      							// Fx-RAM
        (highH == 0xF000) ||  								// Fx
        (highH == 0xA000) ||   								// Ax
        (highH == 0xB000) ||  								// Bx
        (highH == 0xC000) ||  								// Cx
		((highH == 0x7000) && (rwMode == COMM_READ_FC) ) ||	// Ux	ֻ��
		( ((addr & 0xFF00) == 0x7300) && (rwMode == COMM_WRITE_FC) )
															// U3�����д
//        || ((addr & 0xFF00) == 0x1F00)          			// FP��1Fxx �û�������������������
		) 
    {
															// ��ַ��Ч
    }
    else
    {
        return COMM_ERR_ADDR;								// ���ص�ַ��Чs
    }
// ��ȡgroup, grade
    group = (addr >> 8) & 0x0F;								// ȡ����
    grade = addr & 0xFF;
	
    if (0xA000 == highH)                 					// Ax
    {
        group += FUNCCODE_GROUP_A0;
    }
    else if (0xB000 == highH)            					// Bx
    {
        group += FUNCCODE_GROUP_B0;
    }
    else if (0xC000 == highH)            					// Cx
    {
        group += FUNCCODE_GROUP_C0;
    }
/*    else if ((addr & 0xFF00) == 0x1F00)  					// FP ����������
    {
        group = FUNCCODE_GROUP_FP;
    }
*/
    else if (0x7000 == highH)            					// Ux
    {
        group += FUNCCODE_GROUP_U0;
    }
	
	if (group == FC_GROUP_FACTORY)							// FF�� ϵͳ(���ҹ�����)����
		return COMM_ERR_ADDR;//COMM_ERR_SYSTEM_LOCKED;
		
// ����ͨѶ����£���ÿһgroup���û����Բ�����grade����
    UpdataFuncCodeGrade(funcCodeGradeComm);
        
    if (grade >= funcCodeGradeComm[group]) 					// ��������
    {
        return COMM_ERR_ADDR;
    }
// ��֤�ض������룬����д����
    index = GetGradeIndex(group, grade);    				// ���㹦�������
    if (
			(COMM_NO_RW_FC_0 == index) ||       			// ĳЩ�����룬ͨѶ���ܽ���RW
			(COMM_NO_RW_FC_2  == index)||
			(	(COMM_WRITE_FC == rwMode) &&       			// ĳЩ�����룬ͨѶ���ܽ���W
				(	(COMM_NO_W_FC_0 == index) ||
					(COMM_NO_W_FC_1 == index) ||
					(COMM_NO_W_FC_2 == index) ||
					(COMM_NO_W_FC_3 == index) ||
					(COMM_NO_W_FC_4 == index) ||
					(COMM_NO_W_FC_5 == index)
				)
			) ||
			((COMM_READ_FC == rwMode) && (COMM_NO_R_FC_0 == index))
															// ĳЩ�����룬ͨѶ���ܽ���R
		)
    {
        return COMM_ERR_ADDR;      // ��Ч��ַ
    }

	return COMM_ERR_NONE;
}

/*******************************************************************************
* ��������          : void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
* ��ڲ���			: dataPi ֡����ָ��
* ����				��
* ����	            : 	
* �汾		        : V0.0.1
* ʱ��              : 09/25/2010
* ˵��				: ����PLC������֡
********************************************************************************/
void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 addr, *InvRecTab, *InvTrTab;					    // �շ���ָ��
	Uint16 result, invTrIndex, indexReset = 1;              // "1"ʱ��λ����
	Uint16 len = 0;
	
	addr = dataPi->mdl.data.datal;							// ���ݵ�ַ����
	
	if ( (addr > 0) && (addr <= INV_REC_FAST_NUM) )			// д���տ��
	{
		InvRecTab = InvRecFastTab;                          // ���տ���ַ
		
		InvTrTab = InvTrFastTab;							// ���ͱ����
		invTrIndex = InvTrFastIndex;
	}
	else if ( (addr > INV_SLOW_TAB_ADDR) && (addr <= (INV_SLOW_TAB_ADDR + INV_REC_SLOW_NUM) ) )
	{														// ������
        addr -= INV_SLOW_TAB_ADDR;                          // ��������ַ
        InvRecTab = InvRecSlowTab;                          // ��������
		
		InvTrTab = InvTrSlowTab;
		invTrIndex = InvTrSlowIndex;
	}
	else
	{
        if (funcCode.code.u3[11] < 65535)
            funcCode.code.u3[11]++;                         // ���ݳ������

        return;												// ��ַ����ֱ�ӷ���
	}
// ���ձ����	
	if (InvRecTab[addr])									// д�����
	{														// ���б���������дRAM
		CanControlWriter(InvRecTab[addr], dataPi->mdl.data.datah, 0);
		if (InvRecTab[addr+1])
		{
			CanControlWriter(InvRecTab[addr+1], dataPi->mdh.data.datal, 0);		
			if (InvRecTab[addr+2])	
				CanControlWriter(InvRecTab[addr+2], dataPi->mdh.data.datah, 0);			
		}
	}
	
// ���ͱ�񷵻�
	if (InvTrTab[invTrIndex])
	{
        if (InvTrTab == InvTrFastTab)
            dataPi->mdl.data.datal = invTrIndex;				// �������ݱ��ַ����
        else
            dataPi->mdl.data.datal = invTrIndex + INV_SLOW_TAB_ADDR;
        
        CanControlRead(InvTrTab[invTrIndex], &result);
		dataPi->mdl.data.datah = result;
		len += 4;
		if (InvTrTab[invTrIndex+1])
		{
			CanControlRead(InvTrTab[invTrIndex+1], &result);
			dataPi->mdh.data.datal = result;
			len += 2;
			if (InvTrTab[invTrIndex+2])
			{
				CanControlRead(InvTrTab[invTrIndex+2], &result);
				dataPi->mdh.data.datah = result;
				len += 2;
				if (InvTrTab[invTrIndex+3])					// ��һ����Ԫ����0���´β�����ͷ��ʼ
					indexReset = 0;
			}
		}
	}
	dataPi->msgid.bit.destSta = PLC_CARD_ID;				// дĿ���ַ
	dataPi->msgid.bit.srcSta = INV_PLC_ID;					// дԴ��ַ
	CanlinkDataTran((Uint32*)(dataPi), len, 1000);			// ������Ӧ֡
	
// ���Ϳ졢����λ�ô���
	if (InvTrTab == InvTrFastTab)							// ���
	{
		if ( (InvTrFastIndex > (INV_TRAN_FAST_NUM - 3) ) 
			|| (0 != indexReset) )							// ������0 ��λ�����Ĵ���
			InvTrFastIndex = 1;
		else
			InvTrFastIndex += 3;		
	}
	else													// ����
	{
		if ( (InvTrSlowIndex > (INV_TRAN_SLOW_NUM - 3) ) 
			|| (0 != indexReset) )
			InvTrSlowIndex = 1;
		else
			InvTrSlowIndex += 3;	
	}
}


#else

Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
    return 0;
}
Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
    return 0;
}
void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
}

#endif





