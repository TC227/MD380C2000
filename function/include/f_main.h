#ifndef __F_MAIN_H__
#define __F_MAIN_H__


#include "f_funcCode.h"
#include "f_error.h"
#include "f_common.h"
#include "f_canlink.h"


#define ABS_INT16(a) (((a) >= 0) ? (a) : (-(a)))
#define ABS_INT32(a) (((a) >= 0L) ? (a) : (-(a)))
#define ABS_INT64(a) (((a) >= (int64)0) ? (a) : (-(a)))

#define  GetMax( x, y ) ( ((x) > (y)) ? (x) : (y) )
#define  GetMin( x, y ) ( ((x) < (y)) ? (x) : (y) )

#define DoNothing()                 // �մ���

#define  GetTime() 	(CpuTimer1.RegsAddr->TIM.all)

#define MAX_UINT32  0xFFFFFFFF      // 32bit���޷������ֵ
#define MAX_UINT16  0xFFFF          // 16bit���޷������ֵ

// F2808�İ���
#define OpenDrive()     (GpioDataRegs.GPACLEAR.bit.GPIO6 = 1)       // ����PWM
#define CloseDrive()    (GpioDataRegs.GPASET  .bit.GPIO6 = 1)       // �ر�PWM


#define Nop()   asm(" nop")     // nopָ��


// ������һЩģ��ĵ�������
#define FUNC_DEAL_PERIOD        2       // ���ܳ��������ڣ�_ms
#define RUN_CTRL_PERIOD         2       // ����Դ���� RunSrcDeal() �������ڣ�_ms
//#define RUN_CTRL_PERIOD         0.5     // ����Դ���� RunSrcDeal() �������ڣ�_ms
#define TORQUE_CTRL_PERIOD      2       // ת�ؿ��Ƶ�����
#define COMM_DEAL_PERIOD        2       // ͨѶ���������
#define ERROR_DEAL_PERIOD       2       // ���ϴ���errorDeal()�������ڣ�_ms
#define FRQ_SRC_PERIOD          2       // Ƶ��Դ����frqSrc()�������ڣ�_ms
#define PULSE_IN_CALC_PERIOD    2       // ��������ļ�������
#define AI_CALC_PERIOD          2       // AI�ļ�������
#define DI_CALC_PERIOD          2       // DI�ļ�������
#define DO_CALC_PERIOD          2       // DO�ļ�������
#define PID_CALC_PERIOD         2       // PID�ļ�������
#define SERVO_CALC_PERIOD       RUN_CTRL_PERIOD       // λ�û�������
#define VF_CALC_PERIOD          2       // VF����ļ�������
#define CORE_FUNC_PERIOD        2       // ���ܵ��������ݸ�������
#define DISP_SCAN_PERIOD        2       // ����ɨ������
#define RUN_TIME_CAL_PERIOD     2       // ����ʱ��ͳ������

enum POWER_ON_STATUS
{
    POWER_ON_WAIT,              // �ȴ��ϵ�׼��OK��
    POWER_ON_CORE_OK,           // (����)�ϵ�׼��OK��ĸ�ߵ�ѹ������ϣ��ϵ�Եض�·������
    POWER_ON_FUNC_WAIT_OT       // ���ܵȴ�ʱ�䳬ʱ�����ܵĵȴ�ʱ�䳬��_ʱ�䣬�����ϵ�׼����û����ϡ�
};
extern enum POWER_ON_STATUS powerOnStatus;




extern Uint16 limitedByOtherCodeIndex[];
extern Uint16 limitedByOtherCodeIndexNum;




#define FRQ_REF_I   0
#define FRQ_FDB_I   1
#define PC_REF_I    2
#define PC_FDB_I    3
extern Uint16 driveCoeff[2][4];


#define DECIMAL     0
#define HEX         1
Uint16 GetNumberDigit(Uint16 digit[5], Uint16 number, Uint16 mode);
void GetNumberDigit1(Uint16 digit[5], Uint16 number);
void GetNumberDigit2(Uint16 digit[5], Uint16 number);

extern struct MOTOR_FC motorFc;
extern enum MOTOR_SN motorSn;
extern Uint16 polePair;

extern Uint16 torqueCurrent;

extern Uint16 memoryValue;

#define AIAO_CHK_CHECK    0     // ����Ƿ���Ҫ����AIAOУ����ʼ��
#define AIAO_CHK_START    1     // ��ʼAIAOУ��ֵ��ȡ
#define AIAO_CHK_END      2     // ����AIAOУ��
#define AIAO_CHK_WORD    0xA5   // AIAOУ����־��
#define AIAO_CHK_READ_NULL   0
#define AIAO_CHK_READ_OK     1
extern Uint16 aiaoChckReadOK;          // ��ȡ��AIAOУ������ֵ����


void InitForFunctionApp(void);          // �������ʼ��

void Main0msFunction(void);             // ���ܴ�����, 0ms
void Main05msFunction(void);            // ���ܴ�����, 0.5ms
void Main2msFunction(void);             // ���ܴ�����, 2ms



#endif // __F_MAIN_H__








