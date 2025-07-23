

#ifndef __F_ERROR_H__
#define __F_ERROR_H__

#include "f_funcCode.h"

// ������Ϣ
#define ERROR_NONE                      0       // 0  -- ��
#define ERROR_INVERTER_UNIT             1       // 1  -- ��䵥Ԫ����
#define ERROR_OC_ACC_SPEED              2       // 2  -- ���ٹ�����
#define ERROR_OC_DEC_SPEED              3       // 3  -- ���ٹ���·
#define ERROR_OC_CONST_SPEED            4       // 4  -- ���ٹ�����
#define ERROR_OV_ACC_SPEED              5       // 5  -- ���ٹ���ѹ
#define ERROR_OV_DEC_SPEED              6       // 6  -- ���ٹ���ѹ
#define ERROR_OV_CONST_SPEED            7       // 7  -- ���ٹ���ѹ
#define ERROR_BUFFER_RES                8       // 8  -- ���������ع���
#define ERROR_UV                        9       // 9  -- Ƿѹ����
#define ERROR_OL_INVERTER               10      // 10 -- ��Ƶ������
#define ERROR_OL_MOTOR                  11      // 11 -- �������
#define ERROR_LOSE_PHASE_INPUT          12      // 12 -- ����ȱ��
#define ERROR_LOSE_PHASE_OUTPUT         13      // 13 -- ���ȱ��
#define ERROR_OT_IGBT                   14      // 14 -- ɢ��������
#define ERROR_EXTERNAL                  15      // 15 -- �ⲿ����
#define ERROR_COMM                      16      // 16 -- ͨѶ(��ʱ)����
#define ERROR_CONTACTOR                 17      // 17 -- �Ӵ�������
#define ERROR_CURRENT_SAMPLE            18      // 18 -- ����������
#define ERROR_TUNE                      19      // 19 -- �����г����
#define ERROR_ENCODER                   20      // 20 -- ���̹���
#define ERROR_EEPROM                    21      // 21 -- EEPORM��д����
#define ERROR_22                        22      // 22 -- ��Ƶ��Ӳ������
#define ERROR_MOTOR_SHORT_TO_GND        23      // 23 -- ����Եض�·����
#define ERROR_24                        24      // 24 -- ����
#define ERROR_25                        25      // 25 -- �������
#define ERROR_RUN_TIME_OVER             26      // 26 -- ����ʱ�䵽��

#define ERROR_USER_1                    27      // 27 -- �û��Զ������1
#define ERROR_USER_2                    28      // 28 -- �û��Զ������2
#define ERROR_POWER_UP_TIME_OVER        29      // 29 -- �ϵ�ʱ�䵽��
#define ERROR_LOSE_LOAD                 30      // 30 -- ����
#define ERROR_FDB_LOSE                  31      // 31 -- ����ʱPID������ʧ
#define ERROR_CBC                       40      // 40 -- ����������
#define ERROR_SWITCH_MOTOR_WHEN_RUN     41      // 41 -- ����ʱ�л����
#define ERROR_DEV                       42      // 42 -- �ٶ�ƫ�����
#define ERROR_OS                        43      // 43 -- ������ٶ�
#define ERROR_MOTOR_OT                  45      // 45 -- ������¹���
#define ERROR_INIT_POSITION             51      // 51 -- �ż�λ�ü��ʧ��
//#define ERROR_SPEED_DETECT              52      // 52 -- ���λ�ñ�ʶʧ��
#define ERROR_UVW_FDB                   53      // 53 -- uvw�źŷ�������
//#define ERROR_PG_PARA_ERROR             90      // 90 -- �����������趨���� 
//#define ERROR_PG_LOST                   91      // 91 -- δ�ӱ�����
//#define ERROR_PROGRAM_LOGIC             99      // 99 -- ����ִ���߼�����

#define ERROR_RSVD                      9999    // ����

#define PLC_DEFINE_ERROR_START          80
#define PLC_DEFINE_ERROR_END            89

//--------------------------------------------------
#define ERROR_LEVEL_NO_ERROR    0   // �޹���
#define ERROR_LEVEL_FREE_STOP   1   // ���ϣ�����ͣ��
#define ERROR_LEVEL_STOP        2   // ���ϣ�����ͣ��(��������ͣ��)
#define ERROR_LEVEL_RUN         3   // ���ϣ���������

#define COMM_ERROR_MODBUS     1
#define COMM_ERROR_CANLINK    2
#define COMM_ERROR_CANOPEN    3
#define COMM_ERROR_PROFIBUS   4
#define COMM_ERROR_P2P        5
#define COMM_ERROR_PLC        6

struct ERROR_ATTRIBUTE_BITS
{
    Uint16 reset:1;     // ��λ��ʽ��0-�����ֶ���λ��1-�Զ���λ
    Uint16 level:3;     // 000-�޹��ϣ�001-����ͣ����010-��ͣ����ʽͣ����011-�������У�
};
union ERROR_ATTRIBUTE
{
    Uint16 all;
    struct ERROR_ATTRIBUTE_BITS bit;
};

/****************************************************
 * ����ʱʱ�����
 *    �ϵ�ʱ�䡢����ʱ��
****************************************************/
struct CUR_TIME
{
    Uint16 runTimeM;        // ����ʱ��-��
    Uint16 runTimeSec;      // ����ʱ��-��

    Uint16 powerOnTimeM;    // �ϵ�ʱ��-��
    Uint16 powerOnTimeSec;  // �ϵ�ʱ��-��

    Uint16 runTime;
    Uint16 powerOnTime;
    
};

// ���ϴ�������  1: ����ͣ��  2:����ͣ��  3:��������
struct ERROR_ATTRIBUTE_LEVEL_BITS
{
    Uint16 Err1:3;
    Uint16 Err2:3;
    Uint16 Err3:3;
    Uint16 Err4:3;
    Uint16 Err5:3;
};
union ERROR_ATTRIBUTE_LEVEL
{
    Uint16 all;
    struct ERROR_ATTRIBUTE_LEVEL_BITS bit;
};

extern union ERROR_ATTRIBUTE errorAttribute;

extern Uint16 motorErrInfor[5];

extern struct CUR_TIME curTime; 

extern Uint16 errorCode;
extern Uint16 errorCodeOld;
extern Uint16 errAutoRstNum;
extern Uint16 bUv;
extern Uint16 errorOther;
extern Uint16 errorsCodeFromMotor[2];
extern Uint16 errorInfo;

enum ERROR_DEAL_STATUS
{
    ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM,    // ׼�����ϼ�¼����
    ERROR_DEAL_WAIT_FOR_WRITE_EEPROM,       // ���ϴ����ȴ�������ϼ�¼���
    ERROR_DEAL_WRITE_EEPROM_OK,             // ����˹��ϼ�¼���档�ȴ��������
    ERROR_DEAL_OK                           // 
};
extern enum ERROR_DEAL_STATUS errorDealStatus;

#define ERROR_DEALING (dspSubCmd.bit.errorDealing)

void ErrorDeal(void);
void ErrorReset(void);
Uint16 GetErrorAttribute(Uint16 errCode);

#endif  // __F_ERROR_H__














