#ifndef __F_FRQSRC_H__
#define __F_FRQSRC_H__


#include "f_funcCode.h"

#define ACC_DEC_FRQ_NONE    0
#define ACC_DEC_FRQ_WAIT    1
#define ACC_DEC_FRQ_DONE    2



typedef struct
{
// input
    Uint16 frq;     // ��ԾƵ��
    Uint16 range;   // ��ԾƵ�ʷ���

// output
    Uint32 low;     // ��ԾƵ�ʷ�Χ��low
    Uint32 high;    // ��ԾƵ�ʷ�Χ��high
} JUMP_FRQ;


// ���ܲ���ʹ�õ�PID
typedef struct
{
    void  (*calc)(void *);  // Pointer to calculation function

    int32  ref;             // PID������Q15
    int32  fdb;             // PID������Q15

    int32  Kp;              // ��������
    int32  Ki;              // ��������
    int32  Kd;              // ΢������

    int32  Kp2;             // ��2������P
    int32  Ki2;             // ��2����
    int32  Kd2;             // ��2����

    int16  Qp;              // Kp��Q��ʽ
    int16  Qi;              // Ki��Q��ʽ
    int16  Qd;              // Kd��Q��ʽ

    int32  deltaMax;        // Q15, �������֮��ƫ������ֵ
    int32  deltaMin;        // Q15, �������֮��ƫ������ֵ
    int32  outMax;          // Q15��PID������ֵ
    int32  outMin;          // Q15��PID�����Сֵ
    int32  pidDLimit;       // Q15, PID΢���޷�
    int32  errorDead;       // С��ƫ��ޣ���PID������, Q15
    int32  errorSmall;      // �����ʹ��
    int32  errorBig;        // �����ʹ��

    int32  error;           // PIDƫ�Q15, ע����ܳ���0xffff
    int32  error1;          // 
    int32  error2;

    int32  delta;           // �������֮��Ĳ�ֵ

    int32  deltaPRem;       // P�����Remainder
    int64  deltaIRem;       // I�����Remainder
    int32  deltaDRem;       // D�����Remainder
    int32  deltaRemainder;  // �������֮���޷�֮�����ֵ�������´μ���ʹ��

    int32  out;             // Output: PID output, Q15
    Uint16 sampleTime;      // PID��������
    Uint16 sampleTcnt;      // PID�������ڼ�ʱ
} PID_FUNC;


#define PID_FUNC_DEFAULTS         \
{                                 \
    (void (*)(void *))PidFuncCalc \
}



extern PID_FUNC pidFunc;

extern int32 frq;
extern int32 frqTmp;
extern int32 frqFrac;
extern int32 frqTmpFrac;
extern int32 frqCurAimFrac;
extern int32 frqDroop; 
extern int32 frqAim;
extern int32 frqAiPu;
extern int32 frqAimTmp;
extern int32 frqAimTmp0;
extern int32 frqCurAim;
extern Uint16 frqKeyUpDownDelta;
extern Uint16 upperFrq;
extern Uint16 lowerFrq;
extern Uint16 maxFrq;
extern Uint16 benchFrq;
extern Uint16 frqPuQ15;
extern int32 uPDownFrqMax;
extern int32 uPDownFrqMin;
extern Uint16 upDownFrqInit;
extern Uint16 plcStep;
extern Uint32 plcTime;
extern Uint16 bPlcEndOneLoop;
extern Uint16 plcStepRemOld;
extern Uint32 plcTimeRemOld;

extern Uint16 timeBench;

extern int32 frqDigitalTmp;
extern Uint16 bFrqDigital;

extern Uint16 frqCalcSrcOld;

extern Uint16 bStopPlc;
extern Uint16 bAntiReverseRun;

extern int32 frqAimOld4Dir;

#define RUN_MODE_SPEED_CTRL     0   // �ٶȿ���
#define RUN_MODE_TORQUE_CTRL    1   // ת�ؿ���
#define RUN_MODE_POSITION_CTRL  2   // λ�ÿ���
extern Uint16 runMode; // ����ģʽ

#define UP_DN_OPERATION_ON          1
#define UP_DN_OPERATION_OFF         0
struct FRQ_FLAG_BITS
{
    Uint16 comp:1;          // 0:   ����
    Uint16 upDown:1;        // �����趨��UP/DOWN���޸�
    Uint16 x:1;             // 1:   X
    Uint16 y:1;             // 2:   Y
    
    Uint16 fcPosLimit:1;    // 3:   �������趨���������޷�
    Uint16 fcNegLimit:1;    // 4:   �������趨���������޷�
    
    Uint16 upDownoperationStatus:1;        // 5:   ������UP/DN������־(��ֹ����Ƶ�ʱ���,UP/DN��Ϊ���޷����޷����ű仯)

    Uint16 frqSetLimit:1;                    // �趨Ƶ�ʱ�����
};
union FRQ_FLAG
{
    Uint16 all;
    struct FRQ_FLAG_BITS bit;
};

extern union FRQ_FLAG frqFlag;

struct FRQ_XY
{
    int32 x;    // ��Ƶ��X
    int32 y;    // ��Ƶ��Y
    int32 z;    // ��Ƶ��Yƫ��
};

extern struct FRQ_XY frqXy; 

void FrqSrcDeal(void);
void UpdateFrqAim(void);
void PidFuncCalc(PID_FUNC *);

extern Uint16 dpFrqAim;
extern Uint16 frqCalcSrc;
extern Uint16 plcStepOld;
int32 FrqPidSetDeal(void);
int32 UpdateMultiSetFrq(Uint16 step);
void ResetUpDownFrq(void);

extern int32 upDownFrq;
extern int32 upDownFrqTmp;

#endif // __F_FRQSRC_H__





