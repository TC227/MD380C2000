//======================================================================
//
// Time-stamp: <2012-02-23  Zhd, 0656>
//
// PROFIBUS Э��
//======================================================================


#include "f_comm.h"



#define DEBUG_F_DP1     1



#if DEBUG_F_DP1


// DP��-��Ƶ����������֡ͷ
#define PROFIBUS_START_HIGH    0xAA      // DP�����Ƶ��������������֡ͷ��λֵ 
#define PROFIBUS_START_LOW     0x55      // DP�����Ƶ��������������֡ͷ��λֵ
#define PROFIBUS_CHECK         0x5A      // DP�����Ƶ����ͨ�������֡ͷֵ

// ֡ͷ
#define DPHighStart       rcvFrame[0]    // DP����������֡ͷ��λ
#define DPlowStart        rcvFrame[1]    // DP����������֡ͷ��λ 
// PKE IND
#define DPFuncCmd         rcvFrame[2]>>4 // DP�����������������
#define DPhighFuncAddr    rcvFrame[3]    // DP�����������������ַ��λ
#define DPlowFuncAddr     rcvFrame[4]    // DP�����������������ַ��λ
// PKW
#define PZDHighCmd        rcvFrame[6]    // DP������PZD11~PZD12��Ӧ���������ֵ���� 0-�޲���  1-дPZDxֵ����Ӧ������(bit0~bit1��ӦPZD11~PZD12) 
#define PZDlowCmd         rcvFrame[7]    // DP������PZD3 ~PZD10��Ӧ���������ֵ���� 0-�޲���  1-дPZDxֵ����Ӧ������(bit0~bit7��ӦPZD3~PZD10) 
#define DPhighFuncData    rcvFrame[8]    // DP�����Ĺ��������ֵ���ݸ�λ(DPFuncCmdֵΪ2��14�����Ĺ�����) 
#define DPlowFuncData     rcvFrame[9]    // DP�����Ĺ��������ֵ���ݵ�λ

// DP���������������
#define DP_FUNC_READ          0x1        // ��������
#define DP_FUNC_WRITE_RAM     0x2        // д������ֵ
#define DP_FUNC_WRITE_EEPROM  0xE        // д������ֵ�ұ�����EEPROM

// PROFIBUS���ݸ�ʽ
#define PPO1                     0
#define PPO2                     1
#define PPO3                     2
#define PPO5                     3
#define DP_RCV_CRC_ERR_NUMBER   10             // CRCУ��������������ô�������Ϊ����

struct DP_CONTROL_CMD_BITS
{
    Uint16 stop:1;         // bit0-����ͣ��
    Uint16 freeStop:1;     // bit1-����ͣ��
    Uint16 fwdRun:1;       // bit2-��ת����
    Uint16 revRun:1;       // bit3-��ת����
    Uint16 fwdJog:1;       // bit4-��ת�㶯
    Uint16 revJog:1;       // bit5-��ת�㶯
    Uint16 reset:1;        // bit6-���ϸ�λ
#if 0    
    Uint16 DpCmdEnable:1;  // bit7-DP������Ч 
#endif 
    Uint16 rsvd1:9;
};
union DP_CONTROL_CMD
{
    Uint16 all;
    struct DP_CONTROL_CMD_BITS bit;
};


struct DP_STATUS_BITS
{
    Uint16 run:1;           // bit0-����/ͣ��
    Uint16 fwdRev:1;        // bit1-��ת/��ת
    Uint16 error:1;         // bit2-��Ƶ������
    Uint16 frqArrive:1;     // bit3-Ƶ�ʵ���
    Uint16 rsvd1:12;        // bit4-bit15
    
};


union DP_STATUS
{
    Uint16 all;
    struct DP_STATUS_BITS bit;
};


union DP_CONTROL_CMD dpControlCmd;             // DP����������
union DP_CONTROL_CMD dpControlCmdBak;          // DP���������ֱ���
union DP_STATUS dpStatus;                      // DP��Ƶ��״̬������               
//Uint16 dpStatus;
Uint16 dpFrqAim;                                // DPĿ��Ƶ�ʻ�ת�ظ���
const Uint16 DPDataNum[4] = {12, 20, 4, 32};   // PPO1��PPO2��PPO3��PPO5Э���Ӧ�����ݸ���
Uint16 rcvRigthFlag;                           // ֡ͷ�ж� Ϊ1��ʾ����0xAA
Uint16 DPRcvTest;                              // DP��ͨ���Ա�־(Ϊ1��ʾ��ǰΪ��ͨ������)

Uint16 saveEepromIndex = 0;
Uint16 saveEepromFlag = 0;
//====================================================================
//
// ���ݽ��պ���Ϣ����
//
//====================================================================
void ProfibusRcvDataDeal(void)
{
    commRcvData.commAddr = 0;
    commRcvData.commData = 0;
    commRcvData.commCmd = 0;
    saveEepromFlag = 0;
    // PPO3Э���޶�д���ܲ���
	// ��ͨ�����޶�д����
    if ((commProtocol != PPO3) && (!DPRcvTest))
    {
		//  �������ܲ�����ַ
		commRcvData.commAddr = (DPhighFuncAddr << 8) + DPlowFuncAddr;  
		//  ���Ĺ��ܲ���ֵ
        commRcvData.commData = (DPhighFuncData << 8) + DPlowFuncData;  
        // ��������
        if (DPFuncCmd == DP_FUNC_READ)
        {
            commRcvData.commCmd = SCI_CMD_READ;
			commRcvData.commData = 1;
        }
		// д������
        else if(DPFuncCmd == DP_FUNC_WRITE_RAM)
        {
            commRcvData.commCmd = SCI_CMD_WRITE;                  // д�������
            commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;  // ������RAM
        }
		// д�������ұ���EEPROM
        else if(DPFuncCmd == DP_FUNC_WRITE_EEPROM)
        {
            // дEEPROM˼·(��ֹѭ��д��EEPROM)
            // 1����¼��ҪдEEPROM��־
            // 2��дRAM
            // 3��дRAM�ɹ�����õ�ǰ���ܲ�������
            // 4�����֮ǰ�д洢���������ȱ���ǰһ������EEPROM����
            // 5�����浱ǰ���ܲ�������
            // 6������ʱ������洢�Ĳ���������ֵ�����EEPROM
            saveEepromFlag = 1;   // ��Ҫ�洢EEPROM����
            commRcvData.commCmd = SCI_CMD_WRITE;                  // д�������
            commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;  // 
        }

    }
#if 0    
    else
    {
		// ΪPPO3����ͨ����ʱ �޲������ܲ��������ַ������
        
    }
#endif    

    commRcvData.slaveAddr = 1;    // �ӻ���ַ(DP����)  // --? ����
	// ���յ���CRCУ��ֵ
	commRcvData.crcRcv = (rcvFrame[commRcvData.rcvNumMax - 1] << 8) + rcvFrame[commRcvData.rcvNumMax - 2];  
	 // CRCУ�鳤��
	commRcvData.crcSize = commRcvData.rcvNumMax - 2;                   
}


//====================================================================
//
// PROFIBUS֡ͷ�ж�
// ����: tmp-����֡����
// ����: 0-֡ͷ�жϹ�����
//       1-����Ҫ֡ͷ�жϣ�ֱ�Ӵ洢����
//
//===================================================================
Uint16 ProfibusStartDeal(Uint16 tmp)
{
    // ֡ͷ�жϱ�־��Ч
    if (!commRcvData.rcvDataJuageFlag)
    {
		// ���յ��µ�����ʱ����ͨ�����жϱ�־
        DPRcvTest = 0;
		// �������ݳ���
        commRcvData.rcvNumMax = DPDataNum[commProtocol] + 4;   // �������ݸ���
		// Ϊ��ͨ����֡ͷ
        if (tmp == PROFIBUS_CHECK)
        {
            rcvFrame[0] = tmp;
            commRcvData.rcvFlag = 0;         
            rcvRigthFlag = 2;         // ��2��ʼ����DP�������ַ       
        }
		// ����DP�������ַ
        else if(rcvRigthFlag == 2)
        {
            rcvFrame[1] = tmp;
            commRcvData.rcvFlag = 0;         
            rcvRigthFlag = 3;         // ��3��ʼ�жϵ���֡�����Ƿ�Ϊ0x55
        }
        else if(rcvRigthFlag == 3)
        {
			rcvRigthFlag = 0;
            // ��ͨ���Եĵ�һλΪ 0x5A ����λΪ0x55
            if(tmp == PROFIBUS_START_LOW)
            {
				DPRcvTest = 1;
                rcvFrame[2] = tmp;
                commRcvData.rcvDataJuageFlag = 1;  // PROFIBUS��֡ͷ�жϱ�־
                commRcvData.rcvFlag = 2;           // Ϊ2��ʾ�������ݴ���
                commRcvData.rcvNum = 3;            // �������ݼ���   
                commRcvData.rcvNumMax = 5;         // ��ͨ��������֡����3λ����λ2λУ��λ
            }
        }
        // �ж� 0xAA 0x55 ֡ͷ��Ϣ
        else if(tmp == PROFIBUS_START_HIGH)
        {
            rcvRigthFlag = 1;      // ����֡ͷ��һ������ȷ
            rcvFrame[0] = tmp;   
            commRcvData.rcvFlag = 0;
        }
        else if(rcvRigthFlag == 1) // ����֡ͷ��һ������ȷ
        {   
            rcvRigthFlag = 0;      // �����֡ͷ��һ������ȷ
            if(tmp == PROFIBUS_START_LOW)
            {
                rcvFrame[1] = tmp;
                commRcvData.rcvDataJuageFlag = 1;  // PROFIBUS��֡ͷ�жϱ�־
                commRcvData.rcvFlag = 2;           // ��ʼ���ݽ���
                commRcvData.rcvNum = 2;            // �������ݼ���   
            }              
        }
        else
        {
            rcvRigthFlag = 0;
            commRcvData.rcvCrcErrCounter  = 0;
            commRcvData.rcvFlag = 0;
        }
        return 0;
    }
    return 1;
}


//====================================================================
//
// ���DP����б�Ƶ���������?
//
//===================================================================
void getDPControlCmd(void)
{
#if 0    
    if (!dpControlCmd.bit.DpCmdEnable)
    {
        commRunCmd = SCI_RUN_CMD_NONE;
    }
    else 
    
    commRunCmd = SCI_RUN_CMD_NONE;

    if (dpControlCmd.bit.stop)
    {
        commRunCmd = SCI_RUN_CMD_STOP;
    }
    else if(dpControlCmd.bit.freeStop)
    {
        commRunCmd = SCI_RUN_CMD_FREE_STOP;
    }
    else if(dpControlCmd.bit.fwdRun)
    {
        commRunCmd = SCI_RUN_CMD_FWD_RUN;
    }
    else if(dpControlCmd.bit.revRun)
    {
        commRunCmd = SCI_RUN_CMD_REV_RUN;
    }
    else if(dpControlCmd.bit.fwdJog)
    {
        commRunCmd = SCI_RUN_CMD_FWD_JOG;
    }
    else if(dpControlCmd.bit.revJog)
    {
        commRunCmd = SCI_RUN_CMD_REV_JOG;
    }
    else if(dpControlCmd.bit.reset)
    {
        if (!dpControlCmdBak.bit.reset)
        {
            commRunCmd = SCI_RUN_CMD_RESET_ERROR;
        }
    }
    

    dpControlCmdBak = dpControlCmd;

    #endif 
}


//====================================================================
//
// ����DP��Ƶ��״̬��Ϣ
//
//===================================================================
void setDPStatusInfo(void)
{
    dpStatus.all = 0;
    
    // ͣ��/���� ״̬
    dpStatus.bit.run = runFlag.bit.run;     
    // ��ת/��ת ״̬
    dpStatus.bit.fwdRev = runFlag.bit.dir;
    
	// Ŀ��Ƶ�ʵ��� 
    if ((runFlag.bit.run) 
        && (frq == frqAim))
    {

        dpStatus.bit.frqArrive = 1;
    }

    // ����
    if (errorCode)
    {
        dpStatus.bit.error = 1;       
    }
   
   
}


//====================================================================
//
// ����ͨѶ����
// 1���������ݸ���
// 2����֡��ʼ�ж�ʱ��
// 3��Ӧ���ӳ�ʱ��
//
//====================================================================
void UpdateProfibusCommFormat(Uint16 baudRate)
{
	// Ϊ��ͨ����ʱ�������ݳ���Ϊ5
    if (DPRcvTest)
    {
        commRcvData.rcvNumMax = 5;
    }
    else
    {
        commRcvData.rcvNumMax = DPDataNum[commProtocol] + 4;   // �������ݸ���
    }
	// DP��û����ʱ���ӳ��ж�֡ͷ
    commRcvData.frameSpaceTime = 0;
	// DP��ͨѶ��Ӧ���ӳ�ʱ��
    commRcvData.delay = 0;
}


//====================================================================
//
// ׼����������
// ����: err-ͨѶ����������Ϣ,Ϊ0��ʾ�����ɹ�
// ����: 1���������ݳ���
//       2������������Ϣ
//
//====================================================================
void ProfibusSendDataDeal(Uint16 err)
{
    Uint16 PZD2ReturnData;
    Uint16 readDataStartIndex;
	Uint16 i, pzdNum, indexRead,indexWrite, group, grade;
    Uint16 pzdDataSaveCmd, pzdSaveData;
    commSendData.sendNumMax = commRcvData.rcvNumMax;

    // ��ͨ����
    if (DPRcvTest)
    {
        sendFrame[0] = PROFIBUS_CHECK;      // ��ͨ����֡ͷ   

        // DP��������ַ,Ӳ������Ϊ0ʱ�Ը�ֵΪ׼
        sendFrame[1] = funcCode.code.commSlaveAddress;
        
        // PROFIBUS-DP���ݸ�ʽ
        sendFrame[2] = commProtocol + 1;    // PPO���ݸ�ʽ
        if (sendFrame[2] > 3)
        {
            sendFrame[2] = 5;   // PPO5
        }
        return;
    }

	// ��������PKW
    sendFrame[0] = PROFIBUS_START_HIGH;    // ����֡ͷ��λ 0xAA
    sendFrame[1] = PROFIBUS_START_LOW;     // ����֡ͷ��λ 0x55  
    sendFrame[2] = 0x10;                   // ��4λΪ1��ʾ����������ɹ�
    sendFrame[3] = DPhighFuncAddr;         // �����������ַ��λ
	sendFrame[4] = DPlowFuncAddr;          // �����������ַ��λ
    sendFrame[5] = 0x00;                   // ����
    sendFrame[6] = 0x00;                   // ����(����Ϊ����PZD������ȡ/�޸�״̬��λ)
    sendFrame[7] = 0x00;                   // ����(����Ϊ����PZD������ȡ/�޸�״̬��λ)
    sendFrame[8] = DPhighFuncData;         // ����������ֵ��λ(��ȡ������ʱ����Ϊ��ȡ��ֵ)
    sendFrame[9] = DPlowFuncData;          // ����������ֵ��λ(��ȡ������ʱ����Ϊ��ȡ��ֵ)

	// �����������������ڹ���
	if (!(DPFuncCmd))
	{
        sendFrame[2] = 0x0;                // ��4λΪ1��ʾ����������ɹ�
        sendFrame[3] = 0x0;                // �����������ַ��λ
	    sendFrame[4] = 0x0;                // �����������ַ��λ
	}
    else if (err)
    {
        sendFrame[2] = 0x70; // ��4λ����Ϊ7��ʾ����ʧ��    
        sendFrame[8] = 0x00; // ���ϴ�����Ϣ��λ
        sendFrame[9] = err;  // ���ϴ�����Ϣ��λ
    }
	// ������������ɹ�
    else if (sciFlag.bit.read)              // ͨѶ������ȡ����������Ҫ��ȡ
    {
        // ���ض�ȡֵ
        sendFrame[8] = commReadData[0] >> 8;
        sendFrame[9] = commReadData[0] & 0x00ff;
    }
    else if (sciFlag.bit.write)
    {
        // ����洢EEPROM
        if (saveEepromFlag)
        {
            Uint16 highH,group,grade,index;
            // ��ù��ܲ����׵�ַ
            highH = (commRcvData.commAddr & 0xF000);

            // ��ȡgroup, grade
            group = (DPhighFuncAddr >> 8) & 0x0F;
            grade = DPlowFuncAddr & 0xFF;

            if (0xA000 == highH)       // Ax
            {
                group += FUNCCODE_GROUP_A0;
            }
            else if(0xB000 == highH)
            {
                group += FUNCCODE_GROUP_B0;
            }

            index = GetGradeIndex(group, grade);
            // ���֮ǰ������Ҫ���������
            if ((saveEepromIndex) 
                && (index != saveEepromIndex)
                )
            {
                // �洢EEPROM
                SaveOneFuncCode(saveEepromIndex);
            }
            saveEepromIndex = index;

            
        }
    }
    
    // ��PPO3���ݸ�ʽ PZD������ʼ��ַΪ10
    // PPO3 ���ݸ�ʽ  PZD������ʼ��ַΪ2
    if (commProtocol != PPO3)
    {
        readDataStartIndex = 10;
		pzdNum = (commSendData.sendNumMax - 16) >> 1;    // ������PZD��ݲ�������
    }
	else
	{
		readDataStartIndex = 2;
		pzdNum = 0;    // PPO3������PZD��ݲ�������Ϊ0               
	}

	// PZD1��PZD2����
    if (!sciFlag.bit.crcChkErr)   // CRCУ��ɹ�
    {
        Uint16 frq;
        // ������
        commRunCmd = rcvFrame[readDataStartIndex + 1];  
        // Ŀ��Ƶ��
        frq = (rcvFrame[readDataStartIndex + 2] << 8) + rcvFrame[readDataStartIndex + 3];
        // ���Ƶ������
        if (frq <= maxFrq)
        {
            dpFrqAim = frq;
        }
    }
  
	// PZD1��PZD2����
    setDPStatusInfo();     // ״̬��
	if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
	{
        PZD2ReturnData = frqFdb;       // �ջ�ʸ��ʱ����ʵ��ת��
	}
    else
    {
        PZD2ReturnData = frqRunDisp;   // ����ʸ��ʱ����ͬ��ת��
    }
	
	// PZD1��������(��Ƶ��״̬��Ϣ)
    sendFrame[readDataStartIndex++]     = (dpStatus.all >> 8);
    sendFrame[readDataStartIndex++] = (dpStatus.all & 0x00ff);
	// PZD2��������(����ת�ػ�Ƶ��)
	sendFrame[readDataStartIndex++] = (PZD2ReturnData >> 8);
    sendFrame[readDataStartIndex++] = (PZD2ReturnData & 0x00ff);


	// PZD������ݱ��������־
	pzdDataSaveCmd = (PZDHighCmd << 8) + PZDlowCmd;

	// �����������������(PZD3~PZD12)
	for (i = 0; i < pzdNum; i++)
	{
        // PZD�����ַ(��վ->��Ƶ��)
		group = funcCode.group.fe[i] / 100;
        grade = funcCode.group.fe[i] % 100;
		indexWrite = GetGradeIndex(group, grade);
        // PZD�����ַ(��Ƶ��->��վ)
	    group = funcCode.group.fe[i + 10] / 100;
        grade = funcCode.group.fe[i + 10] % 100;
		indexRead = GetGradeIndex(group, grade);
        
        // PZD���
        if (!sciFlag.bit.crcChkErr)
        {
            pzdSaveData = (rcvFrame[readDataStartIndex] << 8) + rcvFrame[readDataStartIndex + 1];
            if (ModifyFunccodeUpDown(indexWrite, &pzdSaveData, 0) == COMM_ERR_NONE)
            {
                ModifyFunccodeEnter(indexWrite, pzdSaveData);
            }
        }
        
		// PZD����
		sendFrame[readDataStartIndex++] = (funcCode.all[indexRead] >> 8);
    	sendFrame[readDataStartIndex++] = (funcCode.all[indexRead] & 0x00ff);  
	}          

}


//====================================================================
//
// ͨѶ�����ж�(�ж�ͨѶ�������SCIΪ���ݽ���״̬)
// ����: 0��ͨѶ����
//       1��ͨѶ����
//
//====================================================================
Uint16 ProfibusCommErrCheck(void)
{
#if 1

    if ((funcCode.code.commOverTime) && (commTicker >= (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime))
    {
        // MODBUSͨѶ��ʱ����ʱ
        commTicker = (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime;  
        return 1;
    }

    if (commRcvData.rcvCrcErrCounter > DP_RCV_CRC_ERR_NUMBER)
    {
       // PROFIBUS CRCУ��������
       commRcvData.rcvDataJuageFlag = 0;           // ���¿�ʼ�ж�֡ͷ     
       commRcvData.rcvCrcErrCounter = 0;           // ���¿�ʼ�ж��Ƿ�CRCУ�����
       return 1;
    }

    return 0;
#else
	return 0;
#endif
}



#elif 1


Uint16 ProfibusCommErrCheck(void){}
void ProfibusRcvDataDeal(void){}
void ProfibusSendDataDeal(Uint16 err){}
Uint16 ProfibusStartDeal(Uint16 tmp){}
void UpdateProfibusCommFormat(Uint16 baudRate){}
    


#endif






