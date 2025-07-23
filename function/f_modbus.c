//======================================================================
//
// Time-stamp: <2012-06-23  LeiMin, 0656>
//
// MODBUS Э��
//======================================================================

#include "f_comm.h"

#define RTU_MASTER_FRAME_NUM_MAX  8     // ��������֡���ַ�����

// MODBUSЭ��
#define RTUslaveAddress rcvFrame[0]      // RTU֡�Ĵӻ���ַ
#define RTUcmd          rcvFrame[1]      // RTU֡��������
#define RTUhighAddr     rcvFrame[2]      // RTU֡�ĵ�ַ���ֽ�
#define RTUlowAddr      rcvFrame[3]      // RTU֡�ĵ�ַ���ֽ�
#define RTUhighData     rcvFrame[4]      // RTU֡�����ݸ��ֽ�
#define RTUlowData      rcvFrame[5]      // RTU֡�����ݵ��ֽ�
#define RTUlowCrc       rcvFrame[6]      // RTU֡��CRCУ����ֽ�
#define RTUhighCrc      rcvFrame[7]      // RTU֡��CRCУ����ֽ�

//====================================================================
//
// ���ݽ��պ���Ϣ����
//
//====================================================================
void ModbusRcvDataDeal(void)
{    
    commRcvData.slaveAddr = RTUslaveAddress;                // �ӻ���ַ
    commRcvData.commCmd = RTUcmd;                           // ͨѶ����
    commRcvData.commAddr = (RTUhighAddr << 8) + RTUlowAddr; // ������ַ
    commRcvData.commData = (RTUhighData << 8) + RTUlowData; // ��������
    commRcvData.crcRcv = (RTUhighCrc << 8) + RTUlowCrc;     // CRCУ��ֵ    
	commRcvData.crcSize = 6;                                // CRCУ�鳤��
	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // �洢EEPROM����
}


//====================================================================
//
// MODBUS֡ͷ�ж�
// ����: tmp-����֡����
// ����: 0-֡ͷ�жϹ�����
//       1-����Ҫ֡ͷ�жϣ�ֱ�Ӵ洢��������
//
//===================================================================
Uint16 ModbusStartDeal(Uint16 tmp)
{
    if ((commTicker > commRcvData.frameSpaceTime))
    {
        RTUslaveAddress = tmp;          // ����3.5���ַ�ʱ�䣬�µ�һ֡�Ŀ�ʼ
        // �㲥ģʽ    
        if (RTUslaveAddress == 0)       
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 1;    // ���յ�֡�ĵ�һ���ֽڣ����ǹ㲥ģʽ
        }
        else if ( (funcCode.code.commProtocolSec == CAN_OPEN) && (RTUslaveAddress == 1) )
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;                        // CANOPENʹ�� ������ַ 1
        }
		// ������ַ
        else if (RTUslaveAddress == funcCode.code.commSlaveAddress) 
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;                        // CANOPENʹ�� ������ַ 1
        }
		// ������ַ
        else
        {
            commRcvData.rcvFlag = 0;    // ��ַ����Ӧ�����ݲ����� 
        }
        
        return 0;
    }

    return 1;
}


//====================================================================
//
// ����ͨѶ����
// 1���������ݸ���
// 2����֡��ʼ�ж�ʱ��
// 3��Ӧ���ӳ�ʱ��
//
//====================================================================
void UpdateModbusCommFormat(Uint16 baudRate)
{
    commRcvData.rcvNumMax= RTU_MASTER_FRAME_NUM_MAX;      // �������ݸ���
    commRcvData.frameSpaceTime = 385 * 2 / baudRate+ 1;   // 3.5 char time=3.5*(1+8+2)/baud
    commRcvData.delay = funcCode.code.commDelay * 2;      // Ӧ���ӳ�ʱ��, _*0.5ms
}


//====================================================================
//
// ׼����������
// ����: err-ͨѶ����������Ϣ,Ϊ0��ʾ�����ɹ�
// ����: 1���������ݳ���
//       2������������Ϣ
//
//====================================================================
void ModbusSendDataDeal(Uint16 err)
{
	int16 i;
    sendFrame[0] = rcvFrame[0];   // �ظ�����֡ͷǰ2λ
    sendFrame[1] = rcvFrame[1];   // �ظ�����֡ͷǰ2λ
    commSendData.sendNumMax = 8;  // �������ݳ���
    if (err)
    {
        sendFrame[2] = 0x80;
	    sendFrame[3] = 0x01;
	    sendFrame[4] = 0x00;
        sendFrame[5] = err;
    }
    else if (sciFlag.bit.pwdPass)           // ����ͨ��������0x8888
    {
        sendFrame[2] = RTUhighAddr;
        sendFrame[3] = RTUlowAddr;
        sendFrame[4] = 0x88;
        sendFrame[5] = 0x88;
	}
    else if (sciFlag.bit.write)             // д���ݲ����������д����򱨴���������д
    {
        sendFrame[2] = RTUhighAddr;
        sendFrame[3] = RTUlowAddr;
        sendFrame[4] = RTUhighData;
        sendFrame[5] = RTUlowData;
	}
    else if (sciFlag.bit.read)              // ͨѶ������ȡ����������Ҫ��ȡ
    {
        Uint16 sendNum;
        Uint16 readDataStartIndex;
        sendNum = commRcvData.commData << 1;
        
        // ��׼��MODEBUSͨѶЭ��
        if (commProtocol)
        {
            sendFrame[2] = sendNum;                 // ���յ����ǹ��������*2
            commSendData.sendNumMax = sendNum + 5;  // ������ַ�����
            readDataStartIndex = 3;
        }
        // �Ǳ�׼MODBUSͨѶЭ��
        else if (commProtocol == 0)
        {
            sendFrame[2] = sendNum >> 8;     // �����������λ
            sendFrame[3] = sendNum & 0x00ff; // �����������λ
            commSendData.sendNumMax = sendNum + 6;    // ������ַ�����
            readDataStartIndex = 4;
        }

        // ��ȡ������ֵ
        for (i = commRcvData.commData - 1; i >= 0; i--)
        {
            sendFrame[(i << 1) + readDataStartIndex] = commReadData[i] >> 8;
            sendFrame[(i << 1) + readDataStartIndex + 1] = commReadData[i] & 0x00ff;
        }
    }
}


//====================================================================
//
// ͨѶ�����ж�
// ����: 0��ͨѶ����
//       1��ͨѶ����
//
//====================================================================
Uint16 ModbusCommErrCheck(void)
{
    if ((funcCode.code.commOverTime) && (commTicker >= (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime))
    {
        // MODBUSͨѶ��ʱ����ʱ
        commTicker = (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime;  
        return 1;
    }
    else
    {
        return 0;
    }
}












