#ifndef __F_IO_H__
#define __F_IO_H__


#include "f_funcCode.h"



//=====================================================================
// DI���Ӷ���
//
// DI��������
#define DI_TERMINAL_NUMBER          (DI_NUMBER_PHSIC +     \
                                     DI_NUMBER_V +         \
                                     DI_NUMBER_AI_AS_DI)  
                                     
#if !DEBUG_F_POSITION_CTRL
#define DI_FUNC_NUMBER              60      // DI�Ĺ���ѡ������
#elif 1
#define DI_FUNC_NUMBER              100     // DI�Ĺ���ѡ������
#endif

#define DI_FUNC_NO_FUNC             0   // �޹���
#define DI_FUNC_ACC_DEC_TIME_SRC1   16  // �Ӽ���ʱ��ѡ�����
#define DI_FUNC_ACC_DEC_TIME_SRC2   17  // �Ӽ���ʱ��ѡ�����
#define DI_FUNC_COUNTER_TICKER_IN   25  // ����������(DI5)
#define DI_FUNC_LENGTH_TICKER_IN    27  // ���ȼ���������(DI5)
#define DI_FUNC_FVC                 31  // ǿ���л�ΪFVC
#define DI_FUNC_APTP_ZERO           76  // aptp�������(DI5)

struct DI_FUNC1_BITS
{
    Uint16 noFunc:1;                // 0: �޹���
    Uint16 fwd:1;                   // 1: ��ת����FWD
    Uint16 rev:1;                   // 2: ��ת����REV
    Uint16 tripleLineCtrl:1;        // 3: ����ʽ���п���
    Uint16 fwdJog:1;                // 4: ����㶯
    Uint16 revJog:1;                // 5: ����㶯
    Uint16 up:1;                    // 6: ����UP
    Uint16 down:1;                  // 7: ����DOWN

    Uint16 closePwm:1;              // 8:    ����ͣ����������PWM���
    Uint16 errorReset:1;            // 9:    ���ϸ�λ
    Uint16 runPause:1;              // 10:   ������ͣ
    Uint16 externalErrOpenIn:1;     // 11:   �ⲿ��������(����)
    Uint16 multiSet:4;              // 15:12 ���ָ�����4��3��2��1

    Uint16 accDecTimeSrc:2;         // 17:16 �Ӽ���ʱ��ѡ�����2��1
    Uint16 frqSrcSwitch:1;          // 18    Ƶ��Դ�л�
    Uint16 clearUpDownFrq:1;        // 19    UP/DOWN�趨����
    Uint16 localOrRemote:1;         // 20    ���������л����ӣ�DI/comm<->panel
    Uint16 forbidAccDecSpd:1;       // 21    �Ӽ��ٽ�ֹ
    Uint16 pidPause:1;              // 22    PID��ͣ
    Uint16 resetPLC:1;              // 23    PLC״̬��λ
    
    Uint16 swingPause:1;            // 24    ��Ƶ��ͣ
    Uint16 counterTickerIn:1;       // 25    ����������(DI5)
    Uint16 resetCounter:1;          // 26    ��������λ
    Uint16 lengthTickerIn:1;        // 27    ���ȼ���������(DI5)
    Uint16 resetLengthCounter:1;    // 28    ���ȼ�������λ
    Uint16 forbidTorqueCtrl:1;      // 29    ת�ؿ��ƽ�ֹ
    Uint16 pulseIn:1;               // 30    ��������
    Uint16 motorCtrlMode2Fvc:1;     // 31    ǿ���л�ΪFVC(����)
};
union DI_FUNC1
{
    Uint32                  all;
    struct DI_FUNC1_BITS    bit;
};

struct DI_FUNC2_BITS
{
    Uint16 brake:1;                 // 32+0: ֱ���ƶ�
    Uint16 externalErrCloseIn:1;    // 32+1: �ⲿ���ϳ�������
    Uint16 frqOk:1;                 // 32+2: Ƶ���趨��Ч����
    Uint16 pidDirRev:1;             // 32+3: PID���÷���ȡ������
    Uint16 stopPanel:1;             // 32+4: �ⲿͣ�����ӣ�������������Ч
    Uint16 diOrComm:1;              // 32+5: ����Դ�л�����2��DI<-->comm
    Uint16 pidPauseI:1;             // 32+6: PID������ͣ����
    Uint16 frqXSrc2Preset:1;        // 32+7: ��Ƶ��ԴX��Ԥ��Ƶ���л�    
    Uint16 frqYSrc2Preset:1;        // 32+8: ��Ƶ��ԴY��Ԥ��Ƶ���л�
    Uint16 motorSnDi:2;             // 41,42: ���ѡ����ӡ�ѡ���˸ö��ӣ����ѡ��������Ч
    Uint16 pidChg:1;                // 43:  PID�����л����ӡ�0-PID1, 1-PID2
    Uint16 userError1:1;            // 44:  �û��Զ������1
    Uint16 userError2:1;            // 45:  �û��Զ������2
    Uint16 SpdTorqSwitch:1;         // 46:  �ٶȿ���/ת�ؿ����л�
    Uint16 emergencyStop:1;         // 47:  ����ͣ��
    Uint16 stop4dec:1;              // 48: �ⲿ����ͣ��(������ʱ��4,�κ�ʱ����Ч)
    Uint16 decBrake:1;              // 49: ����ֱ���ƶ�
    Uint16 clearSetRunTime:1;       // 50: ��������ʱ������
    Uint16 rsvd2:13;
};
union DI_FUNC2
{
    Uint32                  all;
    struct DI_FUNC2_BITS    bit;
};

struct DI_FUNC
{
    union DI_FUNC1  f1;
    union DI_FUNC2  f2;
};

struct DI_STATUS_BITS
{
    Uint16 di1:1;       // 0
    Uint16 di2:1;       // 1
    Uint16 di3:1;       // 2
    Uint16 di4:1;       // 3 
    
    Uint16 di5:1;       // 4
    Uint16 di6:1;       // 5
    Uint16 di7:1;       // 6
    Uint16 di8:1;       // 7
    
    Uint16 di9:1;       // 8
    Uint16 di10:1;      // 9
    Uint16 vdi1:1;      // 10
    Uint16 vdi2:1;      // 11
     
    Uint16 vdi3:1;      // 12
    Uint16 vdi4:1;      // 13
    Uint16 vdi5:1;      // 14
    Uint16 ai1:1;       // 15
    
    Uint16 ai2:1;
    Uint16 ai3:1;
    Uint16 rsvd:14;
};

union DI_STATUS
{
    Uint32 all;
    struct DI_STATUS_BITS bit;
};

struct DI_STATUS_ALL
{
    union DI_STATUS a;          // ��ǰ��DI����״̬����ʱǰ
    union DI_STATUS b;          // ��ǰ��DI����״̬����ʱ�������߼�ǰ
    union DI_STATUS c;          // ��ǰ��DI����״̬����ʱ�������߼���
};


//======================================================================
// DO ���ѡ��
#define DO_TERMINAL_NUMBER           (DO_NUMBER_PHSIC + DO_NUMBER_V)  // DO��������

#define DO_FUNC_COMM_CTRL            20
#define DO_FUNC_NUMBER               42  // DO�Ĺ���ѡ������

#if DSP_2803X
#define FUNCCODE_FM_OUT_SELECT_MAX   1
#else
#define FUNCCODE_FM_OUT_SELECT_MAX   2
#endif

struct DO_FUNC1_BITS
{
    Uint16 noFunc:1;                // 0: �޹���
    Uint16 run:1;                   // 1: ��Ƶ��������
    Uint16 error:1;                 // 2: �������
    Uint16 frqFdtArrive:1;          // 3: Ƶ��ˮƽ���FDT����
    Uint16 frqArrive:1;             // 4: Ƶ�ʵ���
    Uint16 zeroSpeedRun:1;          // 5: ����������
    Uint16 motorPreOl:1;            // 6: �������Ԥ����
    Uint16 inverterPreOl:1;         // 7: ��Ƶ������Ԥ����
    
    Uint16 counterSetArrive:1;      // 8:    �趨��������ֵ����
    Uint16 counterPointArrive:1;    // 9:    ָ����������ֵ����
    Uint16 lengthArrive:1;          // 10:   ���ȵ���
    Uint16 plcEndLoop:1;            // 11    PLCѭ�����
    Uint16 runTimeArrive:1;         // 12    ����ʱ�䵽��
    Uint16 frqLimit:1;              // 13    Ƶ���޶���
    Uint16 torqueLimit:1;           // 14    ת���޶���
    Uint16 runReadyOk:1;            // 15    ����׼������
    
    Uint16 ai1GreaterThanAi2:1;     // 16    AI1 > AI2
    Uint16 upperFrqArrive:1;        // 17    ����Ƶ�ʵ���
    Uint16 lowerFrqArrive:1;        // 18    ����Ƶ�ʵ���
    Uint16 uv:1;                    // 19    Ƿѹ״̬���
    Uint16 commCtrl:1;              // 20    ͨѶ����
    Uint16 pcOk:1;                  // 21    ��λ���
    Uint16 pcNear:1;                // 22    ��λ�ӽ�
    Uint16 zeroSpeedRun1:1;         // 23    ����������(ͣ����Ч)
    Uint16 powerUpTimeArrive:1;     // 24    �ϵ�ʱ�䵽��
    Uint16 frqFdtArrive1:1;         // 25:   Ƶ��ˮƽ���FDT1����
    Uint16 frqArrive1:1;            // 26:   Ƶ�ʵ���1
    Uint16 frqArrive2:1;            // 27:   Ƶ�ʵ���2
    Uint16 currentArrive1:1;        // 28:   ��������1
    Uint16 currentArrive2:1;        // 29:   ��������2
    Uint16 setTimeArrive:1;         // 30:   ��ʱ����
    Uint16 ai1limit:1;              // 31:   AI1���볬��������
};

union DO_FUNC1
{
    Uint32                  all;
    struct DO_FUNC1_BITS    bit;
};

struct DO_FUNC2_BITS
{
    Uint16 loseLoad:1;                // 32+0: ������
    Uint16 speedDir:1;                // 32+1: ת�ٷ���
    Uint16 oCurrent:1;                // 32+2: ������⵽�����
    Uint16 tempArrive:1;              // 32+3: ģ���¶ȵ���    
    Uint16 softOc:1;                  // 32+4: ����������
    Uint16 lowerFrqArrive:1;          // 32+5: ����Ƶ�ʵ���(�������й�)
    Uint16 errorOnStop:1;             // 32+6: �������(����ͣ�������)
    Uint16 motorForeOT:1;             // 32+7: �������Ԥ����
    Uint16 setRunTimeArrive:1;        // 32+8: ��ǰ����ʱ�䵽��
    Uint16 errorOnNoUV;               // 32+9: �������(Ƿѹ�����)
    Uint16 rsvd0:6;                   // 
	Uint16 rsvd1:16;                  // 32+31
};

union DO_FUNC2
{
    Uint32                  all;
    struct DO_FUNC2_BITS    bit;
};

struct DO_FUNC
{
    union DO_FUNC1  f1;
    union DO_FUNC2  f2;
};

// DO�����������״̬��
struct DO_STATUS_BITS
{
// �빦����˳��һ��
    Uint16 do3:1;       // bit0-DO3
    Uint16 relay1:1;    // bit1-relay1
    Uint16 relay2:1;    // bit2-relay2
    Uint16 do1:1;       // bit3-DO1
    
    Uint16 do2:1;       // bit4-DO2
    Uint16 vdo1:1;      // bit5-VDO1
    Uint16 vdo2:1;      // bit6-VDO2
    Uint16 vdo3:1;      // bit7-VDO3
    
    Uint16 vdo4:1;      // bit8-VDO4
    Uint16 vdo5:1;      // bit9-VDO5
    
    Uint16 rsvd2:6;     // ����
};

union DO_STATUS
{
    Uint16 all;
    struct DO_STATUS_BITS bit;
};

struct DO_STATUS_ALL
{
    union DO_STATUS a;          // ��ǰ��DO����״̬����ʱǰ
    union DO_STATUS b;          // ��ǰ��DO����״̬����ʱ�������߼�ǰ
    union DO_STATUS c;          // ��ǰ��DO����״̬����ʱ�������߼���
};

// DO�����������״̬��(Ӳ��)
struct DO_HW_STATUS_BITS
{
// ��Ӳ��һ��
    Uint16 relay1:1;    // relay1
    Uint16 do2:1;       // DO2
    Uint16 do1:1;       // DO1
    Uint16 relay2:1;    // relay2

    Uint16 error:1;     // ����ָʾ��
    Uint16 run:1;       // ����ָʾ��
    Uint16 rsvd:1;      // ����Ϊ��
    Uint16 fan:1;       // ��������
};
union DO_HW_STATUS
{
    Uint16 all;
    struct DO_HW_STATUS_BITS bit;
};
extern union DO_HW_STATUS doHwStatus;
//======================================================================

struct DIDO_DELAY_STRUCT  // ����֮�󣬿���1->0, 0->1���п���
{
    Uint32 high;    // 0->1����ʱ
    Uint32 low;     // 1->0����ʱ
};
union DI_DELAY_TICKER_UNION
{
    struct DIDO_DELAY_STRUCT all[3];

    struct      // Ҫ�빦�����˳��һ��
    {
        struct DIDO_DELAY_STRUCT di1;
        struct DIDO_DELAY_STRUCT di2;
        struct DIDO_DELAY_STRUCT di3;
    } single;
};
union DO_DELAY_TICKER_UNION
{
    struct DIDO_DELAY_STRUCT all[DO_TERMINAL_NUMBER];

    struct      // Ҫ�빦�����˳��һ��
    {
        struct DIDO_DELAY_STRUCT do3;
        struct DIDO_DELAY_STRUCT relay1;
        struct DIDO_DELAY_STRUCT relay2;
        struct DIDO_DELAY_STRUCT do1;
        struct DIDO_DELAY_STRUCT do2;
    } single;
};

struct DI_HW_STATUS_BITS
{
    Uint16 di1:1;       // 0
    Uint16 di2:1;       // 1
    Uint16 di3:1;       // 2
    Uint16 di4:1;       // 3 
    
    Uint16 di5:1;       // 4
    Uint16 di6:1;       // 5
    Uint16 di7:1;       // 6
    Uint16 di8:1;       // 7
    
    Uint16 di9:1;       // 8
    Uint16 di10:1;      // 9
};

union DI_HW_STATUS
{
    Uint32 all;
    struct DI_HW_STATUS_BITS bit;
};

extern union DI_HW_STATUS diHwStatus;

//======================================================================
// FMP, AO
#define AOFMP_FMP   0
#define AOFMP_AO1   1
#define AOFMP_AO2   2
//======================================================================


//======================================================================
// AO,FMP���ѡ��
#define AO_FMP_FUNC_NUMBER          13  // AO�趨�ĸ���
#define AO_FMP_FUNC_FRQ_SET         0   // ����Ƶ��
#define AO_FMP_FUNC_FRQ_AIM         1   // �趨Ƶ��
#define AO_FMP_FUNC_OUT_CURRENT     2   // �������
#define AO_FMP_FUNC_OUT_TORQUE      3   // ���ת��(����ֵ)
#define AO_FMP_FUNC_OUT_POWER       4   // �������
#define AO_FMP_FUNC_OUT_VOLTAGE     5   // �����ѹ
#define AO_FMP_FUNC_PULSE_IN        6   // PULSE��������
#define AO_FMP_FUNC_AI1             7   // AI1
#define AO_FMP_FUNC_AI2             8   // AI2
#define AO_FMP_FUNC_AI3             9   // AI3(��չ��)
#define AO_FMP_FUNC_LENGTH          10   // ����
#define AO_FMP_FUNC_COUNTER         11   // ����ֵ
#define AO_FMP_FUNC_COMM            12   // ͨѶ�������
#define AO_FMP_FUNC_SPEED           13   // ���ת��
#define AO_FMP_FUNC_OUT_CURRENT_1   14   // �������  ��Ӧ0~1000A
#define AO_FMP_FUNC_OUT_VOLTAGE_1   15   // �����ѹ  ��Ӧ0~1000V
#define AO_FMP_FUNC_OUT_TORQUE_DIR  16   // ���ת��(������)
//======================================================================


// ����(x1,y1), (x2,y2), �������(x, y)��y
// ��ȷ��: x1 < x2
typedef struct
{
    void (*calc)(void *);     // Pointer to calculation functon
    
    int16 mode;               // 1����ʾx��y���޷���0���޷�

    int32 x1;
    int32 y1;                 // (x1,y1)
    int32 x2;
    int32 y2;                 // (x2,y2)

    int32 x;                  // ��Ҫ����(x,y)��x, ����
    int32 y;                  // ��Ҫ����(x,y)��y, ���
} LINE_STRUCT;


void LineCalc(LINE_STRUCT *p);

#define LINE_STRTUCT_DEFALUTS       \
{                                   \
    (void (*)(void *))LineCalc      \
}

typedef struct
{
    void (*calc)(void *);         // Pointer to calculation functon

    int32 t;                      // �˲�ʱ��

    int32 in;                     // ����
    int32 out;                    // ���

    int32 outOld;                 // ��һ�ε����
    int32 remainder;              // ��������е�����
} LowPassFilter;

void LpfCalc(LowPassFilter *p);

#define LPF_DEFALUTS            \
{                               \
    (void (*)(void *))LpfCalc   \
}


//=====================================================================
// ECAP
// ECCTL1 ( ECAP Control Reg 1)
//==========================
// CAPxPOL bits
#define   EC_RISING       0x0
#define   EC_FALLING      0x1
// CTRRSTx bits
#define   EC_ABS_MODE     0x0
#define   EC_DELTA_MODE   0x1
// PRESCALE bits
#define   EC_BYPASS       0x0
#define   EC_DIV1         0x0
#define   EC_DIV2         0x1
#define   EC_DIV4         0x2
#define   EC_DIV6         0x3
#define   EC_DIV8         0x4
#define   EC_DIV10        0x5
// ECCTL2 ( ECAP Control Reg 2)
//==========================
// CONT/ONESHOT bit
#define   EC_CONTINUOUS   0x0
#define   EC_ONESHOT      0x1
// STOPVALUE bit
#define   EC_EVENT1       0x0
#define   EC_EVENT2       0x1
#define   EC_EVENT3       0x2
#define   EC_EVENT4       0x3
// RE-ARM bit
#define   EC_ARM          0x1
// TSCTRSTOP bit
#define   EC_FREEZE       0x0
#define   EC_RUN          0x1
// SYNCO_SEL bit
#define   EC_SYNCIN       0x0
#define   EC_CTR_PRD      0x1
#define   EC_SYNCO_DIS    0x2
// CAP/APWM mode bit
#define   EC_CAP_MODE     0x0
#define   EC_APWM_MODE    0x1
// APWMPOL bit
#define   EC_ACTV_HI      0x0
#define   EC_ACTV_LO      0x1
// Generic
#define   EC_DISABLE      0x0
#define   EC_ENABLE       0x1
#define   EC_FORCE        0x1
//=====================================================================

extern struct DI_FUNC diFunc;
extern struct DI_FUNC diSelectFunc;
extern struct DI_STATUS_ALL diStatus;


extern struct DO_FUNC doFunc;
extern struct DO_STATUS_ALL doStatus;


extern Uint32 pulseInFrq;
extern int16 pulseInSet;
extern Uint16 lineSpeed;

//extern Uint16 fanControl;

struct AI_DEAL
{
    Uint16 sample;          // ����ֵ��Q16
    
    int16 voltageOrigin;    // У��ǰ��ѹ
    int16 voltage;          // У�����ѹ
    
    int16 set;              // AI�趨
};
extern struct AI_DEAL aiDeal[];


void InitSetEcap4(void);
void InitSetEcap2(void);
void InitSetEcap1();

void DiCalc(void);
void DoCalc(void);
void PulseInCalc(void);
void PulseInSample(void);
void FMPDeal(void);
void AiCalc(void);
void AoCalcChannel(Uint16);

void InitDIGpio(void);
void InitDOGpio(void);
void InitECap3Gpio(void);
void InitSetEcap3(void);
void InitECap4Gpio(void);

void InitSetEPWM4(void);
void InitSetEPWM5(void);
void InitSetEPWM6(void);

void InitEPwm4Gpio(void);
void InitEPwm5Gpio(void);
void InitEPwm6Gpio(void);



#endif // __F_IO_H__


