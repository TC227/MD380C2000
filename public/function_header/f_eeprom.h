#ifndef __F_EEPROM_H__
#define __F_EEPROM_H__


#include "f_funcCode.h"

// ����1��������, ׼��д��EEPROM
#define SaveOneFuncCode(index)  (funcCodeOneWriteIndex[funcCodeOneWriteNum++] = (index))

#define I2C_SLAVE_ADDR          0x50

#if DSP_2803X     // 2803x����2808ƽ̨
#define EEPROM_PAGE_BYTE        4 // EEPROM(24LC08)��1ҳ��byte
#elif 1
#define EEPROM_PAGE_BYTE        16 // EEPROM(24LC08)��1ҳ��byte
#endif

#define FUNC_CODE_BYTE          2  // һ���������byte
#define EEPROM_PAGE_NUM_FUNC_CODE (EEPROM_PAGE_BYTE/FUNC_CODE_BYTE) // EEPROM(24LC08)��1ҳ����д��Ĺ��������

// ������ʼ��
#define FUNCCODE_paraInitMode_NO_OPERATION              0   // �޲���
#define FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA      1   // �ָ�(����)�����趨ֵ
#define FUNCCODE_paraInitMode_CLEAR_RECORD              2   // �����¼��Ϣ
#define FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL  3   // �ָ�(����)�����趨ֵ
#define FUNCCODE_paraInitMode_SAVE_USER_PARA            4   //+e ���浱ǰȫ���û�������
#define FUNCCODE_paraInitMode_RESTORE_USER_PARA         501   //+e �ָ�������û�������
// ���²�Ҫ��FUNCCODE_paraInitMode_xx�����ظ�
#define FUNCCODE_RW_MODE_WRITE_ALL                  10  // ȫ��������д�ɳ���ֵ
#define FUNCCODE_RW_MODE_WRITE_SERIES               11  // ����д��ʹ�����ʱ������EEPROM��FC��ַҪ����
#define FUNCCODE_RW_MODE_WRITE_ONE                  12  // дһ��������
#define FUNCCODE_RW_MODE_READ_ALL                   13  // ��ȡȫ��������
#define FUNCCODE_RW_MODE_READ_SERIES                14  // ������
#define FUNCCODE_RW_MODE_READ_ONE                   15  // ��һ�������룬��ֱ��ʹ��ReadFuncCode()����
#define FUNCCODE_RW_MODE_NO_OPERATION   FUNCCODE_paraInitMode_NO_OPERATION   // û��EEPROM����


// I2C  Message Commands for I2CMSG struct
#if DSP_2803X     // 2803x����2808ƽ̨
#define I2C_MSG_BUFFER_SIZE 4
#elif 1
#define I2C_MSG_BUFFER_SIZE 16
#endif
enum I2C_MSG_STATUS
{
    I2C_MSG_STATUS_IDLE,                   // ����
    I2C_MSG_STATUS_WRITE_BUSY,             // EEPROM�ڲ����ڲ�д����
    I2C_MSG_STATUS_SEND_NOSTOP_BUSY,       // DSP���ڷ���Ҫ��ȡ�����ݿ�����
    I2C_MSG_STATUS_RESTART,                // DSP����Ҫ��ȡ���������
    I2C_MSG_STATUS_READ_BUSY,              // DSP׼����������
    I2C_MSG_STATUS_RW_OK                   // ��д���
};
// I2C Message Structure
struct I2C_MSG 
{
  enum I2C_MSG_STATUS status;           // Word status
  Uint16 bytes;                         // Num of valid bytes in (or to be put in buffer)
  Uint16 highAddr;                      // EEPROM address of data associated with msg (high byte)
  Uint16 lowAddr;
  Uint16 buffer[I2C_MSG_BUFFER_SIZE];	// Array holding msg data - max that
  									    // MAX_BUFFER_SIZE can be is 16 due to
  									    // the FIFO's
};

typedef struct
{
    Uint16 index;   // Ҫ��д���������ʼindex, EEPROM��index
    Uint16 number;  // Ҫ��д������ĸ���
    
    Uint16 data[EEPROM_PAGE_NUM_FUNC_CODE];
} FUNCCODE_RW;
extern FUNCCODE_RW funcCodeRead;

struct EEPROM_OPERATE_TIME
{
    // ���� ReadFuncCode() ��������readFlag��־Ϊ1������ɣ�readFlag��־Ϊ0
    Uint16 readFlag;  // ����־
    Uint16 readTicker;

    // ���� WriteFuncCode() ����
    Uint16 writeFlag; // д��־
    Uint16 writeTicker;

    // funcCodeRwMode��Ϊ0
    Uint16 rwFlag;
    Uint16 rwTicker;
};
extern struct EEPROM_OPERATE_TIME eepromOperateTime;

#if F_DEBUG_RAM
#define FUNCCODE_DEBUG              5555
#define FUNCCODE_RW_STATUS_DEBUG    5555
#endif

#define FUNCCODE_READ_RET_READING           0   // ���ڶ�ȡ
#define FUNCCODE_READ_RET_OK                1   // ��ȡ��ϣ���ȡ�����ݷ���data��ʼ�ĵ�ַ����
//#define FUNCCODE_READ_RET_WAIT_FOR_REPEAT   2

#define FUNCCODE_WRITE_RET_WRITING          0   // ����д��
#define FUNCCODE_WRITE_RET_OK               1   // д�����
//#define FUNCCODE_WRITE_RET_READING          2
//#define FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT  3

#define FUNCCODE_RW_RET_PARA_ERROR          4   // ��������Ŀǰ����readNumber����
#define FUNCCODE_RW_RET_EEPROM_ERROR        5   // ��ȡEEPROM�������߶�ȡ֮ǰ���Ѿ���EEPROM����

extern Uint16 funcCodeOneWriteIndex[];
extern Uint16 funcCodeOneWriteNum;
extern Uint16 rwFuncCodeRepeatDelayTicker;
extern Uint16 funcCodeRwMode;
extern Uint16 funcCodeRwModeTmp;

extern Uint16 startIndexWriteSeries;
extern Uint16 endIndexWriteSeries;






//------------------------------------------------------
#define ERROR_EEPROM_NONE               0   // 
#define ERROR_EEPROM_WRITE              1   // д���������
#define ERROR_EEPROM_READ               2   // �����������
#define ERROR_EEPROM_RW_OVER_TIME       3   // ��д���������
#define ERROR_EEPROM_WRITE_NUM_OVER     4   // Ҫ����Ĺ��������Խ��, ����funcCodeOneWriteIndex��������
extern Uint16 errorEeprom;
//------------------------------------------------------




Uint16 ReadFuncCode(FUNCCODE_RW *funcCodeRw);
Uint16 WriteFuncCode(FUNCCODE_RW *funcCodeRw);
void EepromDeal(void);
void InitSetI2ca(void);
void EepromOperateTimeDeal(void);

Uint16 GetEepromIndexFromFcIndex(Uint16 index);
Uint16 GetFcIndexFromEepromIndex(Uint16 a);

#endif // __F_EEPROM_H__


