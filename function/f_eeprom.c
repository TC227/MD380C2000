//======================================================================
//
// EEPROM����24LC32, I2C����
//
// 
// Time-stamp: <2012-01-03 14:21:33  Shisheng.Zhi, >
//
//======================================================================
 

#include "f_eeprom.h"
#include "f_main.h"
#include "f_menu.h"
 

//--------------------------------------
#if F_DEBUG_RAM
#define DEBUG_F_TABLE_FC2EEPROM_CONST   0   // eeprom2Fcʹ��const������ռ�
#elif 1
#define DEBUG_F_TABLE_FC2EEPROM_CONST   1
#endif

#if DEBUG_F_TABLE_FC2EEPROM_CONST
// ��Ӧ��ϵ��
// y = eeprom2Fc[i]
// i, �����±꣬----�ù�������EEPROM��λ��
// y, �����ֵ��----����������
//
// �Զ�����
#include "f_table_eeprom2Fc.c"

#endif
//--------------------------------------


//======================================================================
// EEPROMоƬѡ��
#define EEPROM_24LC08   0
#define EEPROM_24LC16   1
#define EEPROM_24LC32   2

//#define EEPROM_TYPE     EEPROM_24LC08
//#define EEPROM_TYPE     EEPROM_24LC16
#define EEPROM_TYPE     EEPROM_24LC32
//======================================================================



// ������ʼ������������ȫ����ǰ�����뵽EEPROMʣ��Ŀռ�
// ĿǰEEPROM�ռ䲻������ʱȥ����
#define PARA_INIT_MODE_SAVE_ALL_CODE    1       // �����

#define DEBUG_F_EEPROM                  1       // ����ʹ�ã���ȫ��������д��__����

#define FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX     3       // 3��
#define FUNCCODE_READ_VERIFY_WRONG_TIME_MAX     4
#define FUNCCODE_READ_AFTER_WRITE_VERIFY_RIGHT_TIME_MAX     2   // 2��
#define FUNCCODE_READ_AFTER_WRITE_VERIFY_WRONG_TIME_MAX     3
#define FUNCCODE_READ_TIME_MAX                  5

enum FUNCCODE_WRITE_STATUS
{
    FUNCCODE_WRITE_STATUS_WRITE,                 
    FUNCCODE_WRITE_STATUS_WRITE_REMAINDER,       
    FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE  
};
//#define FUNCCODE_WRITE_TIME_MAX                     5

// ����Ϊ8�ı���������ᳬ��24LC08��һ��ҳ��
// I2cMsg.highAddr
// ��Ҫ����eeprom�ĵ�ַhighAddr(24lc08����ҪMemoryLowAddr)���ڸ�highAddr��ֵʱ��
// ����eeprom����ʼ��ַ���ɡ�
#define FUNCCODE_EEPROM_START_INDEX  16   // eeprom��ǰ��(_*2)��byte����д�Ŀ����ԱȽϴ����Բ�ʹ��

// ����Ϊ8�ı���������ᳬ��24LC08��һ��ҳ��
#define USER_PARA_START_INDEX       (2048 + FUNCCODE_EEPROM_START_INDEX)

#if DEBUG_F_POSITION_CTRL
#pragma DATA_SECTION(funcCodeOneWriteIndex, "data_ram");
#endif
// һ��һ������EEPROMд���ݡ�
// Ŀǰʹ��funcCodeOneWriteIndexд�����������У�
// 1. һ��ģ�menu3OnEnter����һ��������(���̡�ͨѶ)
// 2. ���޸����Ƶ��(�ȵ�)�������Ƶ��Ϊ��ֵ�Ĺ�����ҲҪ�޸ġ�������Ŀǰ���Ϊ14����
// 3. �������
// 4. ͣ������
// 5. �����¼����
#define FUNCCODE_ONE_WRITE_NUMBER_MAX       200     //+= 100��Ӧ���㹻�������Դ���Խ��Ŀ��ܡ�ʣ��ķ�ֹԽ��
Uint16 funcCodeOneWriteNum;
Uint16 funcCodeOneWriteIndex[FUNCCODE_ONE_WRITE_NUMBER_MAX];

// ��Ƭ�Ķ�дEEPROM���ݡ�
// Ŀǰʹ��paraInitModeд�����������У�
// 1. ��EEPROM��һ��ʹ�ã�ȫ��������͵��籣���������ֵд�뵽EEPROM�С�
// 2. ��EEPROM����ʹ�ù�����ȡȫ��������͵��籣�������RAM�С�
// 3. FP-01, ������ʼ�����ָ���������
Uint16 funcCodeRwMode;
Uint16 funcCodeRwModeTmp;
LOCALF struct I2C_MSG I2cMsg;

// ����д�ķ�Χ[startIndexWriteSeries, endIndexWriteSeries]
Uint16 startIndexWriteSeries;   // ����д����ʼindex������д�Ĺ��������ΪendIndexWriteSeries-startIndexWriteSeries+1
Uint16 endIndexWriteSeries;     // ����д�Ľ���index
Uint16 endIndexRwFuncCode;


enum FUNC_CODE_RW_STATUS
{
    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD,        // ׼����ȡ������
    FUNCCODE_RW_STATUS_READ_CURRENT_DATA,           // ��ȡ��������EEPROM�ĵ�ǰ����
    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE,      // ׼����Ҫд�������
    FUNCCODE_RW_STATUS_WRITE_DATA                   // д������
};
LOCALF enum FUNC_CODE_RW_STATUS funcCodeRwStatus;
#define FUNC_CODE_RW_STATUS_INIT    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD

FUNCCODE_RW funcCodeRead;           // ����(4/8��)����EEPROMд������
LOCALF FUNCCODE_RW funcCodeWrite;

#define FUNCCODE_RW_DELAY_TIME_MAX  100     // * 2ms
Uint16 rwFuncCodeRepeatDelayTicker; // ��д������ʱ����֤��һ�£���ʱһ��ʱ��֮���ٶ�ȡ����ʱ��ʱ��

#define EEPROM_READ_SINGLE_MAX      250     // _*2ms�����ζ�EEPROM�ĳ�ʱʱ��
#define EEPROM_WRITE_SINGLE_MAX     500     // _*2ms������дEEPROM�ĳ�ʱʱ��
#define EEPROM_RW_FUNCCODE_MAX      5000    // _*2ms��������д������ĳ�ʱʱ��

LOCALF Uint16 index4EepromDeal;     // eeprom��ַ
LOCALF Uint16 number4EepromDeal;

struct EEPROM_OPERATE_TIME eepromOperateTime;

#define RW_I2C_BUS_WRITE    0   // д
#define RW_I2C_BUS_READ     1   // ��
#define RW_I2C_BUS_ACK      2   // ��ѯ
LOCALD Uint16 RwI2cBus(Uint16 mode);

#define RW_EEPROM_WRITE     0   // д
#define RW_EEPROM_READ      1   // ��
LOCALD Uint16 RwEeprom(Uint16 mode);
LOCALD void I2cIntDeal(void);
LOCALD void UpdateIndexForEepromDeal(void);

void I2cDealBeforeInit(void);
void I2CNoACK(void);
void I2CStop(void);
void I2CRcvByte(void);


//=====================================================================
//
// ��дI2C����
//
//=====================================================================
LOCALF Uint16 RwI2cBus(Uint16 mode)
{
    int16 i;

   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
    if (I2caRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }
    
// Check if bus busy
    if ((I2caRegs.I2CSTR.bit.BB == 1) && (I2cMsg.status != I2C_MSG_STATUS_RESTART))
    {
        return I2C_BUS_BUSY_ERROR;
    }

// Setup slave address
#if (EEPROM_TYPE == EEPROM_24LC32)
    I2caRegs.I2CSAR = I2C_SLAVE_ADDR;
#elif 1
    I2caRegs.I2CSAR = (I2C_SLAVE_ADDR | (I2cMsg.highAddr >> 8)) & 0xff;
#endif    

    if (mode == RW_I2C_BUS_WRITE)
    {
        // Setup number of bytes to send
        // buffer + Address
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CCNT = I2cMsg.bytes + 2;
#elif 1
        I2caRegs.I2CCNT = I2cMsg.bytes + 1;
#endif

        I2caRegs.I2CDXR = I2cMsg.highAddr; // Setup data to send
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CDXR = I2cMsg.lowAddr;
#endif
        for (i = 0; i < I2cMsg.bytes; i++)
        {
            I2caRegs.I2CDXR = I2cMsg.buffer[i];
        }

        // Send start as master transmitter
        I2caRegs.I2CMDR.all = 0x6E20;   // S.A.D.P
    }
    else if ((mode == RW_I2C_BUS_READ) && (I2C_MSG_STATUS_RESTART == I2cMsg.status))
    {
        I2caRegs.I2CCNT = I2cMsg.bytes; // Setup how many bytes to expect
        
        I2caRegs.I2CMDR.all = 0x6C20;   // Send restart as master receiver
                                        // S.A.D.P
    }
    else                                // ACK, or start read
    {
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CCNT = 2;
        I2caRegs.I2CDXR = I2cMsg.highAddr;
        I2caRegs.I2CDXR = I2cMsg.lowAddr;
#elif 1
        I2caRegs.I2CCNT = 1;
        I2caRegs.I2CDXR = I2cMsg.highAddr;
#endif
        
        I2caRegs.I2CMDR.all = 0x6620;   // Send data to setup EEPROM address
                                        // S.A.D
    }

    return I2C_SUCCESS;
}


//=====================================================================
//
// I2C����״̬����
//
//=====================================================================
LOCALF void I2cIntDeal(void)
{
    Uint16 IntSource;

// Read interrupt source
    IntSource = I2caRegs.I2CISRC.all;

    if (IntSource == I2C_NO_ISRC)   // û���жϱ�־������
        return;

    if (IntSource == I2C_SCD_ISRC)   // Interrupt source = stop condition detected
    {
        if (I2cMsg.status == I2C_MSG_STATUS_WRITE_BUSY)
        {
            Uint16 tmp = 0;
            
            while (RwI2cBus(RW_I2C_BUS_ACK) != I2C_SUCCESS) // �����û����ɣ�������ѯI2C����״̬
            {
                // The EEPROM will send back a NACK while it is performing
                // a write operation. Even though the write communique is
                // complete at this point, the EEPROM could still be busy
                // programming the data. Therefore, multiple attempts are
                // necessary.
                if (++tmp >= 100)
                    break;
            }
        }
        // If a message receives a NACK during the address setup portion of the
        // EEPROM read, the code further below included in the register access ready
        // interrupt source code will generate a stop condition. After the stop
        // condition is received (here), set the message status to try again.
        // User may want to limit the number of retries before generating an error.
        else if (I2cMsg.status == I2C_MSG_STATUS_SEND_NOSTOP_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_IDLE;
        }
        // If completed message was reading EEPROM data, reset msg to inactive state
        // and read data from FIFO.
        else if (I2cMsg.status == I2C_MSG_STATUS_READ_BUSY)
        {
            int16 i;
            
            for (i = 0; i < I2cMsg.bytes; i++)
            {
                I2cMsg.buffer[i] = I2caRegs.I2CDRR;
            }

            I2cMsg.status = I2C_MSG_STATUS_RW_OK;
        }
    }  // end of stop condition detected
    // Interrupt source = Register Access Ready
    // This interrupt is used to determine when the EEPROM address setup portion of the
    // read data communication is complete. Since no stop bit is commanded, this flag
    // tells us when the message has been sent instead of the SCD flag. If a NACK is
    // received, clear the NACK bit and command a stop. Otherwise, move on to the read
    // data portion of the communication.
    else if (IntSource == I2C_ARDY_ISRC)    // ARDY�жϵķ��������ο�I2CSTR˵����
    {
        if (I2caRegs.I2CSTR.bit.NACK == 1)
        {
            I2caRegs.I2CMDR.bit.STP = 1;    // ֮��SCD=1������SCDINT
            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
        }
        else if (I2cMsg.status == I2C_MSG_STATUS_SEND_NOSTOP_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_RESTART;
        }
        else if (I2cMsg.status == I2C_MSG_STATUS_WRITE_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_RW_OK;

            InitSetI2ca();
            //I2caRegs.I2CMDR.all = 0x0000;    // reset I2C
            //I2caRegs.I2CMDR.all = 0x0020;
        }
    }
}


//=====================================================================
//
// Read/Write data from EEPROM section
// ����ֵ��0--��û�����
//         1--���
//
//=====================================================================
LOCALF Uint16 RwEeprom(Uint16 mode)
{
    Uint16 tmp = 0;
    
    if (I2C_MSG_STATUS_IDLE == I2cMsg.status)
    {
        if (RW_EEPROM_WRITE == mode)
        {
            if (RwI2cBus(RW_I2C_BUS_WRITE) == I2C_SUCCESS)
            {
                I2cMsg.status = I2C_MSG_STATUS_WRITE_BUSY;
            }
        }
        else //if (RW_EEPROM_READ == mode)
        {
            while (RwI2cBus(RW_I2C_BUS_READ) != I2C_SUCCESS)
            {
                if (++tmp >= 100)
                {
                    return 0;
                }
            }

            I2cMsg.status = I2C_MSG_STATUS_SEND_NOSTOP_BUSY;
        }
        return 0;
    }
    else if (I2C_MSG_STATUS_RESTART == I2cMsg.status)
    {
        while (RwI2cBus(RW_I2C_BUS_READ) != I2C_SUCCESS)
        {
            if (++tmp >= 100)
            {
                return 0;
            }
        }

        I2cMsg.status = I2C_MSG_STATUS_READ_BUSY;
        return 0;
    }
    else if (I2C_MSG_STATUS_RW_OK == I2cMsg.status) // ��д�������
    {
        I2cMsg.status = I2C_MSG_STATUS_IDLE;
        return 1;
    }
    else
        return 0;
}


//======================================================================================
//
// ��EEPROM(24LC08)�ж�ȡn��������
//
// ����:
//      funcCodeRw->index  -- ��ȡ���������ʼindex
//      funcCodeRw->number -- ��ȡ������ĸ���, (0, 8]
// �����
//      funcCodeRw->data[] -- �����ȡ������
// ���أ�
//      FUNCCODE_READ_RET_READING    -- ���ڶ�ȡ
//      FUNCCODE_READ_RET_OK         -- ��ȡ��ϣ���ȡ�����ݷ���data��ʼ�ĵ�ַ����
//      FUNCCODE_RW_RET_PARA_ERROR   -- ��������Ŀǰ����readNumber����
//      FUNCCODE_RW_RET_EEPROM_ERROR -- ��ȡEEPROM�������߶�ȡ֮ǰ���Ѿ���EEPROM����
//
// ������̣�
// 1. ��ʼ��ȡ����������ȡ(FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX)��ÿ������һ�£���ȡ��ϣ�
// 2. ���ڶ�ȡ�з����в�һ�£���ʱ(FUNCCODE_RW_DELAY_TIME_MAX * 1ms)��
//    ������������ȡ(FUNCCODE_READ_VERIFY_WRONG_TIME_MAX)�Σ�ÿ������һ�£���ȡ��ϣ�
// 3. �����ظ���2����
// 3. ����2���ظ������ﵽ(FUNCCODE_READ_TIME_MAX)�Σ�����
// 4. ��ǰ�ж�дEEPROM���󣬲���ȡ��ֱ���˳���
//
//=======================================================================================
Uint16 ReadFuncCode(FUNCCODE_RW *funcCodeRw)
{
    static Uint16 readTime;      // ��ȡ�������������ж��Ƿ��ǵ�һ�ζ�ȡ
    static Uint16 readRightTime; // ������ȡ������һ�µĴ���
    static Uint16 bVerifyWrong;  // ��֤��һ�±�־����ʱʹ��
    Uint16 dataRead[EEPROM_PAGE_NUM_FUNC_CODE];
    int16 readRightTimeMax;
    int16 i;
    Uint16 tmp;

    eepromOperateTime.readFlag = 1;

// ����������
    if ((0 == funcCodeRw->number) || (funcCodeRw->number > EEPROM_PAGE_NUM_FUNC_CODE))
        return FUNCCODE_RW_RET_PARA_ERROR;

// ��֤��һ�£���ʱһ��ʱ��֮���ٶ�ȡ
    if (bVerifyWrong && (rwFuncCodeRepeatDelayTicker < FUNCCODE_RW_DELAY_TIME_MAX))
        return FUNCCODE_READ_RET_READING; //FUNCCODE_READ_RET_WAIT_FOR_REPEAT;

    rwFuncCodeRepeatDelayTicker = 0;
    bVerifyWrong = 0;

// ��ȡEEPROM����
// allows the entire memory contents to be serially read during one operation.
#if (EEPROM_TYPE == EEPROM_24LC32)
    tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
    I2cMsg.highAddr = tmp >> 8;
    I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
    I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
#endif
    
    I2cMsg.bytes = funcCodeRw->number << 1;
    if (1 == RwEeprom(RW_EEPROM_READ)) // �����ǰI2Cû��׼���ã�����I2C����æ�����Զ��ض�
    {
        readTime++;
        
        for (i = funcCodeRw->number - 1; i >= 0 ; i--)
        {
            dataRead[i] = (I2cMsg.buffer[i<<1] << 8) + I2cMsg.buffer[(i<<1)+1];
            if (1 == readTime)
            {
                funcCodeRw->data[i] = dataRead[i];
            }
            else
            {
                if (funcCodeRw->data[i] != dataRead[i])
                    break;
            }
        }

        if (1 == readTime)
        {
            readRightTime = 1;  // ��֤��һ�£�����������ȡ_�Σ�ȫ��һ�£���ΪOK. 
            return FUNCCODE_READ_RET_READING;
        }

        if (i < 0)           // ���ϴ�(��һ��)��ȡ������һ��
        {
#if 0
            if (!repeatTime)
                readRightTimeMax = FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX;
            else
                readRightTimeMax = FUNCCODE_READ_VERIFY_WRONG_TIME_MAX;

            if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus) // д֮���ȡ��֤
                readRightTimeMax--;
#elif 1
            readRightTimeMax = FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX;
            if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus) // д֮���ȡ��֤
                readRightTimeMax = FUNCCODE_READ_AFTER_WRITE_VERIFY_RIGHT_TIME_MAX;
#endif

            // �ӿ�ʼ��ȡ��֤��������ȡ_�Σ�ȫ��һ�£�OK
            // ��֤��һ�£�����������ȡ_�Σ�ȫ��һ�£�OK
            if (++readRightTime >= readRightTimeMax)
            {
                readTime = 0;
                eepromOperateTime.readFlag = 0;     // �����
                eepromOperateTime.readTicker = 0;   // ticker����

                return FUNCCODE_READ_RET_OK;
            }
        }
        else                   // ���ϴζ�ȡ�����ݲ�һ��
        {
            readTime = 0;      // ��һ�εĶ�ȡ���п��ܲ���ȷ��Ҫ���¶�ȡ
            bVerifyWrong = 1;  // ��֤��һ�±�־����ʱʹ��
            
            return FUNCCODE_READ_RET_READING; // FUNCCODE_READ_RET_WAIT_FOR_REPEAT;
        }
    }

    return FUNCCODE_READ_RET_READING;   // ���ڶ�ȡ
}


//=====================================================================
//
// ��EEPROM(24LC08)д��n��������
//
// ���룺
//      funcCodeRw->index  -- д�빦�������ʼindex
//      funcCodeRw->data   -- д�빦�������ʼ��ַ
//      funcCodeRw->number -- д�빦����ĸ���, ��Χ��[0, 8]���Ҳ��ܳ���һҳ
// �����
//      �ޡ�
// ���أ�
//      FUNCCODE_WRITE_RET_WRITING   -- ����д��
//      FUNCCODE_WRITE_RET_OK        -- д�����
//      FUNCCODE_RW_RET_PARA_ERROR   -- ��������Ŀǰ����number����
//      FUNCCODE_RW_RET_EEPROM_ERROR -- д��EEPROM��������д��֮ǰ���Ѿ���EEPROM����
//
// ������̣�
// 1. д�롣
// 2. ��ȡ��֤��һ�£�д����ϡ�
// 3. ����֤��һ�£���ʱ(FUNCCODE_RW_DELAY_TIME_MAX * 1ms)֮��
//    ���ص�1��(����д�룬�ٶ�ȡ��֤)��
// 4. ���ظ�д������ﵽ(FUNCCODE_WRITE_TIME_MAX)������
// 5. ��ǰ�ж�дEEPROM���󣬲���ȡ��ֱ���˳���
//
//=====================================================================
Uint16 WriteFuncCode(FUNCCODE_RW *funcCodeRw)
{
    static enum FUNCCODE_WRITE_STATUS writeStatus;
    static Uint16 bVerifyWrong;  // ��֤��һ�±�־����ʱʹ��
    static Uint16 number;
    int16 i;
    Uint16 tmp;

    eepromOperateTime.writeFlag = 1;

// ����������
#if DSP_2803X    // 2803x����2808ƽ̨
    if ((funcCodeRw->index + funcCodeRw->number > (funcCodeRw->index & 0xFFFE) + 0x0002) // ���ܳ���һҳ,ע���� > , ���� >= !!
#elif 1
    if ((funcCodeRw->index + funcCodeRw->number > (funcCodeRw->index & 0xFFF8) + 0x0008) // ���ܳ���һҳ,ע���� > , ���� >= !!
#endif
        || (0 == funcCodeRw->number) || (funcCodeRw->number > EEPROM_PAGE_NUM_FUNC_CODE))
    {
        return FUNCCODE_RW_RET_PARA_ERROR;
    }

// ��֤��һ�£���ʱһ��ʱ��֮������д��
    if (bVerifyWrong && (rwFuncCodeRepeatDelayTicker < FUNCCODE_RW_DELAY_TIME_MAX))
        return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT;
        
    rwFuncCodeRepeatDelayTicker = 0;
    bVerifyWrong = 0;

// д��EEPROM����
    if (FUNCCODE_WRITE_STATUS_WRITE == writeStatus)
    {
        if (EEPROM_PAGE_NUM_FUNC_CODE == funcCodeRw->number) // �����8������д7����Ȼ��д���1��
            number = EEPROM_PAGE_NUM_FUNC_CODE - 1;
        else
            number = funcCodeRw->number;

        for (i = number - 1; i >= 0; i--)
        {
            I2cMsg.buffer[i << 1] = funcCodeRw->data[i] >> 8;
            I2cMsg.buffer[(i << 1) + 1] = funcCodeRw->data[i] & 0x00ff;
        }

#if (EEPROM_TYPE == EEPROM_24LC32)
        tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
        I2cMsg.highAddr = tmp >> 8;
        I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
        I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
#endif
        I2cMsg.bytes = number << 1;

        if (1 == RwEeprom(RW_EEPROM_WRITE))     // �����ǰI2Cû��׼���ã�����I2C����æ�����Զ���д
        {
            if (number == funcCodeRw->number)
                writeStatus = FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE;
            else
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE_REMAINDER;
        }
        else
            return FUNCCODE_WRITE_RET_WRITING;
    }

// дʣ���1��������
    if (FUNCCODE_WRITE_STATUS_WRITE_REMAINDER == writeStatus)
    {
        I2cMsg.buffer[0] = funcCodeRw->data[EEPROM_PAGE_NUM_FUNC_CODE - 1] >> 8;
        I2cMsg.buffer[1] = funcCodeRw->data[EEPROM_PAGE_NUM_FUNC_CODE - 1] & 0x00ff;
        
#if (EEPROM_TYPE == EEPROM_24LC32)
        tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX + EEPROM_PAGE_NUM_FUNC_CODE - 1) << 1;
        I2cMsg.highAddr = tmp >> 8;
        I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
        I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX + EEPROM_PAGE_NUM_FUNC_CODE - 1) << 1;
#endif
        I2cMsg.bytes = 1 << 1;

        if (1 == RwEeprom(RW_EEPROM_WRITE))     // �����ǰI2Cû��׼���ã�����I2C����æ�����Զ���д
            writeStatus = FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE;
        else
            return FUNCCODE_WRITE_RET_WRITING;
    }

// д�����֮�󣬶�ȡ������֤
    if (FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE == writeStatus)
    {
        funcCodeRead.index = funcCodeRw->index;
        funcCodeRead.number = funcCodeRw->number;

        if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
        {
            for (i = funcCodeRw->number - 1; i >= 0; i--)
            {
                if (funcCodeRead.data[i] != funcCodeRw->data[i]) // д�����֮���ȡ������֤
                    break;
            }

            if (i < 0)          // ��ȡ��������д�������һ��
            {
                eepromOperateTime.writeFlag = 0;            // д���
                eepromOperateTime.writeTicker = 0;          // ticker����
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE;  // д����ϣ�״̬��λ
                
                return FUNCCODE_WRITE_RET_OK;
            }
            else                    // ��ȡ��������д������ݲ�һ��
            {
                bVerifyWrong = 1;   // ��֤��һ�±�־����ʱʹ��
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE; // ��֤��һ�£���ʱһ��ʱ��֮������д��
                
                return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT;
            }
        }
        else
            return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_READING;
    }

    return FUNCCODE_WRITE_RET_WRITING; // ����ֵ��FUNCCODE_WRITE_RET_READINGʱ����ʵ����Ҫ
}


//=====================================================================
//
// EEPROM������������Ķ�ȡ������
// �������ݣ�
//     FUNCCODE_RW_MODE_WRITE_ONE                  ---- ����һ��������
//     FUNCCODE_RW_MODE_WRITE_ALL                  ---- ȫ��������ָ���������
//     FUNCCODE_RW_MODE_WRITE_SERIES               ---- ����д
//     FUNCCODE_RW_MODE_READ_ALL                   ---- ��ȡȫ��������
//     FUNCCODE_paraInitMode_CLEAR_RECORD          ---- �����¼��Ϣ
//     FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA  ---- �ָ������趨ֵ(�������������)
//     FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL  ---- �ָ������趨ֵ(�����������)
//     FUNCCODE_paraInitMode_SAVE_USER_PARA        ---- ���浱ǰ�û������뵽EEPROM��ʣ��ռ�
//     FUNCCODE_paraInitMode_RESTORE_USER_PARA     ---- �ָ�������û�������
//
// ������̣�
// 1. ��ȡ��������EEPROM�ĵ�ǰ���ݡ�
// 2. ��ȡ��Ҫд������ݡ�
// 3. ȷ���Ƿ�������Ҫд�룺ȫ���뵱ǰ������ȣ�������������д�롣
// 4. д������
//
//=====================================================================
void EepromDeal(void)
{
    int16 i;
    Uint16 fcIndex;
    
#if F_DEBUG_RAM     // �����Թ��ܣ���CCS��build option�ж���ĺ�
// ��ֹ�����볬��EEPROM������
    if (((FNUM_EEPROM + 0 + FUNCCODE_EEPROM_START_INDEX) << 1) > 4096)     // 4K bytes
    {    
        //errorEeprom = ERROR_EEPROM_WRITE_NUM_OVER;
        errorEeprom = FNUM_EEPROM + 1 + FUNCCODE_EEPROM_START_INDEX;
        errorCode = 01; // 01δʹ��
    }
#endif

    if (funcCodeOneWriteNum >= (FUNCCODE_ONE_WRITE_NUMBER_MAX >> 1))    // ��ֹfuncCodeOneWriteIndexԽ��
    {
        funcCodeOneWriteNum = FUNCCODE_ONE_WRITE_NUMBER_MAX >> 1;       // ��ֹ���

        if (POWER_ON_WAIT != powerOnStatus)  // ��ֹ�ϵ緢�ֹ����볬����ֵ��������������������ġ�
        {
            errorEeprom = ERROR_EEPROM_WRITE_NUM_OVER;
        }
    }

#if 0   // EEPROM����ʱ��Ҳ���Լ���ִ��
    if (errorEeprom)    // EEPROM���󣬲�ִ�б�����
        return;
#endif

//---------------------------------------------------------
// I2C����״̬����
    I2cIntDeal();
//---------------------------------------------------------


//---------------------------------------------------------
// �ж��Ƿ����ڲ���EEPROM����ǰû�в���EEPROM��������ӦEEPROM���������򣬵ȴ���ǰEEPROM������ɡ�
    if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode)
    {
        // ��(��Ƭ��/������)��������Ҫ����
        if (funcCodeRwModeTmp)
        {
            funcCodeRwMode = funcCodeRwModeTmp;
            if (FUNCCODE_RW_MODE_READ_ALL == funcCodeRwMode)
            {
                index4EepromDeal = 0;
            }
            else
            {
                index4EepromDeal = EEPROM_INDEX_USE_INDEX;      // ���ܲ���ʹ�õ�ַ��ʼ
            }
            endIndexRwFuncCode = EEPROM_INDEX_USE_LENGTH;   // EEPROM��ʹ�ó���
            if (FUNCCODE_RW_MODE_WRITE_SERIES == funcCodeRwMode) // ����д
            {
                // ����д����ʼ��ַ
                index4EepromDeal = GetEepromIndexFromFcIndex(startIndexWriteSeries);
                // Ҫ��1����endIndexRwFuncCode����
                endIndexRwFuncCode = GetEepromIndexFromFcIndex(endIndexWriteSeries) + 1;
            }
        }
        else if (funcCodeOneWriteNum)   // ����(һ��һ����)��������Ҫ����
        {
            funcCodeRwMode = FUNCCODE_RW_MODE_WRITE_ONE;

            funcCodeOneWriteNum--;      // ��Ҫ����Ĺ��������������1
            
            index4EepromDeal = GetEepromIndexFromFcIndex(funcCodeOneWriteIndex[funcCodeOneWriteNum]);
        }
        else                            // EEPROM��Ȼû�ж�д����(�� �ϵ�ʱ��ȡEEPROM_CHECK ֮��)
        {
            // �ϵ�ʱ��ȡEEPROM_CHECK��Ҳ�п��ܷ���EEPROM��д��ʱ����
            // ���ǣ�Ŀǰ��дһ��/ҳ��Ҳ��ʱ������
            return;
        }
    }

// ׼����ȡ�����롣׼��index(Ҫд�빦�������ʼindex)��number(Ҫд��Ĺ��������)
    if (FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD == funcCodeRwStatus)
    {
        if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode)
        {
            number4EepromDeal = 1;
        }
        else
        {
            Uint16 tmp;     // ��һ��ѭ������ʼindex
#if DSP_2803X     // 2803x����2808ƽ̨            
            tmp = (index4EepromDeal & 0xFFFE) + EEPROM_PAGE_NUM_FUNC_CODE;
#elif 1
            tmp = (index4EepromDeal & 0xFFF8) + EEPROM_PAGE_NUM_FUNC_CODE;
#endif
            if (tmp > endIndexRwFuncCode)
            {
                tmp = endIndexRwFuncCode;
            }
            number4EepromDeal = tmp - index4EepromDeal;
        }
        
        funcCodeRead.index = index4EepromDeal;
        funcCodeRead.number = number4EepromDeal;
        
        if (FUNCCODE_RW_MODE_WRITE_ALL == funcCodeRwMode) // �ָ�ȫ������������û�б�Ҫ�ȶ�ȡ
        {
            funcCodeRwStatus = FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE;
        }
        else
        {
#if PARA_INIT_MODE_SAVE_ALL_CODE
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // ���浱ǰ��ȫ��(�û�)������
                funcCodeRead.index = index4EepromDeal + USER_PARA_START_INDEX;    // USER_PARA_START_INDEX����Ϊ8�ı���!
#endif

            for (i = 0; i < number4EepromDeal; i++)
            {
                fcIndex = GetFcIndexFromEepromIndex(index4EepromDeal + i);  // ��ȡ������index
                if (FUNCCODE_RSVD4ALL_INDEX != fcIndex)     // ��EEPROM��ַ���Ǳ�����
                    break;
            }
            
            if (i == number4EepromDeal)     // ����EEPROM��ַȫ���Ǳ�����
            {
                UpdateIndexForEepromDeal();
            }
            else    // ������Ҫ����
            {
                funcCodeRwStatus = FUNCCODE_RW_STATUS_READ_CURRENT_DATA;
            }
        }
    }

// ��ȡ��������EEPROM�ĵ�ǰ���ݡ�
    if (FUNCCODE_RW_STATUS_READ_CURRENT_DATA == funcCodeRwStatus)
    {
        if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
        {
            funcCodeRwStatus = FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE;
        }
    }

// ׼��Ҫд�������
    if (FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE == funcCodeRwStatus)
    {
#if PARA_INIT_MODE_SAVE_ALL_CODE
        if ((FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // �ָ��û�����Ĺ�����
            || (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode)) // ���浱ǰ��ȫ��(�û�)������
        {
            if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode) // �ָ�
                funcCodeWrite.index = index4EepromDeal + USER_PARA_START_INDEX;
            else
                funcCodeWrite.index = index4EepromDeal;

            funcCodeWrite.number = number4EepromDeal;

            if (FUNCCODE_READ_RET_OK != ReadFuncCode(&funcCodeWrite))
            {
                return;
            }
        }
#endif
        
        for (i = 0; i < number4EepromDeal; i++)
        {
            fcIndex = GetFcIndexFromEepromIndex(index4EepromDeal + i);  // ��ȡ������index
            
            if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode)   // �ָ�(����)��������
            {
                funcCode.all[fcIndex] = GetFuncCodeInit(fcIndex, 0);
            }
            else if(FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode)
            {
                funcCode.all[fcIndex] = GetFuncCodeInit(fcIndex, 1); // �ָ�ȫ������ֵ
            }
            else if (FUNCCODE_RW_MODE_READ_ALL == funcCodeRwMode) // ��ȡȫ��������
            {
                funcCode.all[fcIndex] = funcCodeRead.data[i];
            }
            else if (FUNCCODE_RW_MODE_WRITE_ALL == funcCodeRwMode) // ���й�����ָ���������
            {
                // aiaoChckReadOKΪ1ʱ��AI1\AI2\AO1�Ѿ�У��������Ҫ��ȡ
                if ((((fcIndex >= AI1_CALB_START) && (fcIndex <= AI2_CALB_STOP))
                    || ((fcIndex >= AO1_CALB_START) && (fcIndex <= AO1_CALB_STOP)))
                    && (aiaoChckReadOK == AIAO_CHK_READ_OK)
					)
                {
                    ; 
                }
                else if ((fcIndex == SAVE_USER_PARA_PARA1) 
                    || (fcIndex == SAVE_USER_PARA_PARA2))
                {
                    ;
                }
                else
                {
                    funcCode.all[fcIndex] = GetFuncCodeInitOriginal(fcIndex); 
                }
                // ����û�п����������صĹ�����
                // �����ϵ磬����EEPROM���µģ����ֶ����Ļ���
                
                //-=funcCode.all[fcIndex] = 99;// ����

                // ���й�����ָ�����������һ����Ҫд��
                funcCodeRwStatus = FUNCCODE_RW_STATUS_WRITE_DATA;
            }
            else if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode)  // дһ��������
            {
                ;   // ʲô������Ҫ���������Ѿ�׼����
            }
            else if (FUNCCODE_RW_MODE_WRITE_SERIES == funcCodeRwMode) // ����д������
            {
                ;   // ʲô������Ҫ���������Ѿ�׼����
            }
#if PARA_INIT_MODE_SAVE_ALL_CODE
            else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode) // �ָ�
            {
                funcCode.all[fcIndex] = funcCodeWrite.data[i];
            }
#endif

            funcCodeWrite.data[i] = funcCode.all[fcIndex];  // ׼��Ҫд�������

            // ȷ���Ƿ�������Ҫд�룺ȫ���뵱ǰEEPROM������ȣ���������������Ҫд�롣
            // ��ȡȫ�������룬������д����
            if (funcCodeWrite.data[i] != funcCodeRead.data[i])
            {
                funcCodeRwStatus = FUNCCODE_RW_STATUS_WRITE_DATA;
            }
        }

        // EEPROM��ǰֵ����Ҫд��ֵ��ȫһ�£�����Ҫ������дEEPROM
        // ��ȡȫ�������룬����д��
        if (FUNCCODE_RW_STATUS_WRITE_DATA != funcCodeRwStatus)
        {
            UpdateIndexForEepromDeal();
        }
        else    // ��Ҫд
        {
            funcCodeWrite.index = index4EepromDeal;
            funcCodeWrite.number = number4EepromDeal;

#if PARA_INIT_MODE_SAVE_ALL_CODE
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // ���浱ǰ��ȫ��(�û�)������
                funcCodeWrite.index = index4EepromDeal + USER_PARA_START_INDEX;
#endif
        }
    }

// д������
    if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus)
    {
        if (FUNCCODE_WRITE_RET_OK == WriteFuncCode(&funcCodeWrite))
        {
            UpdateIndexForEepromDeal();
        }
    }

#if (F_DEBUG_RAM && (DEBUG_F_EEPROM))
// ����ʹ�ã���ȫ��������д��__����
    if (FUNCCODE_RW_STATUS_DEBUG == funcCodeRwStatus)
    {
        Uint16 all = FNUM_EEPROM; // 1024
        
        funcCodeRwMode = FUNCCODE_DEBUG;
        funcCodeWrite.index = index4EepromDeal;
        funcCodeWrite.number = all - funcCodeWrite.index;
        if (funcCodeWrite.number > EEPROM_PAGE_NUM_FUNC_CODE)
            funcCodeWrite.number = EEPROM_PAGE_NUM_FUNC_CODE;

        for (i = funcCodeWrite.number - 1; i >= 0; i--)
            funcCodeWrite.data[i] = ((index4EepromDeal+i) << 1) + 0x0aec;//0x345d; //0xffff;   // ��ȫ��������д��__����
        if (FUNCCODE_WRITE_RET_OK == WriteFuncCode(&funcCodeWrite))
        {
            index4EepromDeal += funcCodeWrite.number;        // ��һ��׼��д�빦�����indexForEepromDeal
            if (index4EepromDeal >= all)
                funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION;
        }
    }
#endif
}


//=====================================================================
//
// ����indexForEepromDeal
//
//=====================================================================
LOCALF void UpdateIndexForEepromDeal(void)
{
    funcCodeRwStatus = FUNC_CODE_RW_STATUS_INIT;    
    // ���ܷ���if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode){}��
    // ��Ϊ����������ʱ��Ҫ����ѭ��

    if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode) // ���漸��������
    {
        funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION;
    }
    else
    {
        index4EepromDeal += number4EepromDeal;  // ��һ��׼��д�빦�����indexForEepromDeal
        if (index4EepromDeal >= endIndexRwFuncCode)  // ȫ��д�뵽EEPROM��
        {
            if ((FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode) ||     // �ָ�(����)��������
                (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode))   // �ָ�(ȫ��)��������
            {
                RestoreCompanyParaOtherDeal();  // ȫ���ָ�֮����Ҫ����
            }

            // �洢�ѽ����û��洢EEPROM����
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode)
            {
                funcCode.code.saveUserParaFlag1 = USER_PARA_SAVE_FLAG1;
                funcCode.code.saveUserParaFlag2 = USER_PARA_SAVE_FLAG2;
            }
            
            funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION; // ҲҪ���㡣����FUNCCODE_RW_MODE_WRITE_ONE�Ĵ���ʽ��
            funcCodeRwModeTmp = FUNCCODE_RW_MODE_NO_OPERATION;
        }
    }
}



//=====================================================================
//
// EEPROMʱ�䴦���ж��Ƿ�ʱ. 2ms����1��
//
//=====================================================================
void EepromOperateTimeDeal(void)
{
    rwFuncCodeRepeatDelayTicker++;

    if (eepromOperateTime.readFlag)   // ���ڽ���EEPROM�Ķ�����
    {
        if (++eepromOperateTime.readTicker > EEPROM_READ_SINGLE_MAX)   // ���ζ���ʱ������
        {
            eepromOperateTime.readTicker = 0;
            errorEeprom = ERROR_EEPROM_READ;
        }
    }

    if (eepromOperateTime.writeFlag)   // ���ڽ���EEPROM��д����
    {
        if (++eepromOperateTime.writeTicker > EEPROM_WRITE_SINGLE_MAX)   // ����д��ʱ������
        {
            eepromOperateTime.writeTicker = 0;
            errorEeprom = ERROR_EEPROM_WRITE;
        }
    }

    if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode)    // EEPROM���ڽ��ж�д
    {// �ж��Ƿ���EEPROM��д��ʱ������һ��ʱ���ڹ������дû����ɣ�����
        if (++eepromOperateTime.rwTicker >= EEPROM_RW_FUNCCODE_MAX)
        {
            eepromOperateTime.rwTicker = 0;
            errorEeprom = ERROR_EEPROM_RW_OVER_TIME;
        }
    }
    else
    {
        eepromOperateTime.rwTicker = 0;
    }
}




void InitI2CGpio(void)
{
   EALLOW;
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled disabled by the user.
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO32 (SDAA)
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    // Enable pull-up for GPIO33 (SCLA)

/* Set qualification for selected pins to asynch only */
// This will select asynch (no qualification) for the selected pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO32 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO33 (SCLA)

/* Configure SCI pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be I2C functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO32 for SDAA operation
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO33 for SCLA operation

    EDIS;
}


void InitSetI2ca(void)
{
    // Initialize I2C
    I2caRegs.I2CMDR.all = 0x4000;    // reset I2C

#if (DSP_CLOCK == 100)      // DSP����Ƶ��100MHz
    I2caRegs.I2CPSC.all = 9;        // Prescaler - need 7-12 Mhz on module clk, I2C module clock = 10MHz
#elif (DSP_CLOCK == 60)      // DSP����Ƶ��60MHz
    I2caRegs.I2CPSC.all = 5;        // Prescaler - need 7-12 Mhz on module clk, I2C module clock = 10MHz
#endif

#if 0
    I2caRegs.I2CCLKL = 55;          // NOTE: must be non zero, clk�͵�ƽ����Ϊ (55+5)*0.1us = 6us
    I2caRegs.I2CCLKH = 55;          // NOTE: must be non zero, clk�ߵ�ƽ����Ϊ (55+5)*0.1us = 6us
#elif 1
    I2caRegs.I2CCLKL = 25;          // clk�͵�ƽ����Ϊ (25+5)*0.1us = 3us
    I2caRegs.I2CCLKH = 25;
#elif 1     // 200KHz
    I2caRegs.I2CCLKL = 20;          // clk�͵�ƽ����Ϊ (20+5)*0.1us = 2.5us
    I2caRegs.I2CCLKH = 20;
#elif 1
    I2caRegs.I2CCLKL = 10;           // ����ok
    I2caRegs.I2CCLKH = 10;
#elif 1
    I2caRegs.I2CCLKL = 4;           // ����ok
    I2caRegs.I2CCLKH = 9;
#endif

    I2caRegs.I2CIER.all = 0x0024;   // SCD & ARDY interrupts

    I2caRegs.I2CMDR.all = 0x4020;   // Take I2C out of reset
                                    // Stop I2C when suspended

    I2caRegs.I2CFFTX.all = 0x6040;  // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,
}


//--------------------------------------------------------------
#define SDA (GpioDataRegs.GPBDAT.bit.GPIO32)
#define SCL (GpioDataRegs.GPBDAT.bit.GPIO33)
#define SdaIoAsInput()  {                               \
        EALLOW;                                         \
        GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0;             \
        EDIS;                                           \
    }
#define SdaIoAsOutput() {                               \
        EALLOW;                                         \
        GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;             \
        EDIS;                                           \
    }

#define I2cClkHigh()    (GpioDataRegs.GPBSET.bit.GPIO33 = 1)
#define I2cClkLow()     (GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1)

#define I2cDataHigh()   (GpioDataRegs.GPBSET.bit.GPIO32 = 1)
#define I2cDataLow()    (GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1)
// ��EEPROM��ʼ��֮ǰ��EEPROM�п���ռ��I2C���ߣ�ֱ�ӳ�ʼ������ɹ���
// ִ�б�����֮��I2C������DSP���ơ�
void I2cDealBeforeInit(void)
{
    Uint16 a = 0;

// ��ʼ��ΪIO�ڡ�
    EALLOW;
	GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;     // Enable pull-up for GPIO32 (SDAA)
	GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;	    // Enable pull-up for GPIO33 (SCLA)

	GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;   // Asynch input GPIO32 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;   // Asynch input GPIO33 (SCLA)

	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;    // Configure GPIO32 for SDAA operation
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;    // Configure GPIO33 for SCLA operation

    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;     // output
    GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1;     // output

    GpioDataRegs.GPBDAT.bit.GPIO32 = 0;	    // low
    GpioDataRegs.GPBDAT.bit.GPIO33 = 0;	    // low
    EDIS;

// ��λʱEEPROM�������ڽ��в�д��������ʱ�ȴ���
    DELAY_US(20000);    // �ӳ�20ms���ȴ�EEPROMд���
    I2CStop();

    SdaIoAsOutput();
    I2cDataHigh();
    SdaIoAsInput();
    if (!SDA)
    {
        a = 1;
    }
    else
    {
        SdaIoAsOutput();
        I2cDataLow();
        SdaIoAsInput();
        if (SDA)
        {
            a = 1;
        }
    }
    
    if (a == 1)     // 
    {
        I2CRcvByte();
        I2CNoACK();
        I2CStop();
    }
}


void I2CNoACK(void)
{
    DELAY_US(5);
    
    I2cDataHigh();
    DELAY_US(5);
    
    I2cClkHigh();
    DELAY_US(5);
    
    I2cClkLow();
    DELAY_US(5);
}


void I2CStop(void)
{
    I2cDataLow();
    DELAY_US(5);
    
    I2cClkHigh();
    DELAY_US(5);
    
    I2cDataHigh();
    DELAY_US(5);
}


void I2CRcvByte(void)
{
    int i;
    
// SDAΪ����    
    SdaIoAsInput();
    for (i = 0; i < 800; i++)
    {
        I2cClkLow();
        DELAY_US(5);
        I2cClkHigh();
        DELAY_US(5);
    }

    I2cClkLow();
    DELAY_US(5);
    
// SDA�ָ�Ϊ���
    SdaIoAsOutput();
    DELAY_US(5);
}
//--------------------------------------------------------------



// �� EepromIndex(�����ַ) ��ö�Ӧ������� CodeIndex(�߼���ַ)
Uint16 GetFcIndexFromEepromIndex(Uint16 a)
{
    int16 i;

#if DEBUG_F_TABLE_FC2EEPROM_CONST
    i = eeprom2Fc[a];
#elif 1
    for (i = 0; i < EEPROM_INDEX_USE_LENGTH; i++)
    {
        if (GetEepromIndexFromFcIndex(i) == a)
            break;
    }
    if (i == EEPROM_INDEX_USE_LENGTH)   // ��������û�У�Ԥ��
    {
        i = FUNCCODE_RSVD4ALL_INDEX;
    }
#endif

    return i;
}



extern const Uint16 fcNoAttri2Eeprom[];
// �� CodeIndex(�߼���ַ) ��ö�Ӧ EEPROM index(�����ַ)
Uint16 GetEepromIndexFromFcIndex(Uint16 index)
{
    if (index >= GetCodeIndex(funcCode.group.fChk[0]))
    {
        return fcNoAttri2Eeprom[index - GetCodeIndex(funcCode.group.fChk[0])];
    }
    else
    {
        return funcCodeAttribute[index].eepromIndex;
    }
}







