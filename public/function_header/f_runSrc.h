#ifndef __F_COMMAND_H__
#define __F_COMMAND_H__

#include "f_funcCode.h"



// �Ӽ���
#define CONST_SPEED     0       // ����
#define ACC_SPEED       1       // ����
#define DEC_SPEED       2       // ����


//=====================================================================
// runCmd������������
//
#define FORWARD_DIR         0   // ������
#define REVERSE_DIR         1   // ������

#define RUN_CMD_NO_JOG      0   // �޵㶯����
#define RUN_CMD_FWD_JOG     1   // ����㶯
#define RUN_CMD_REV_JOG     2   // ����㶯
#define RUN_CMD_FWD_REV_JOG 3   // ��������㶯������з���㶯����
struct RUN_CMD_BITS
{                               // bits  description
    Uint16 common0:1;           // 0     +, ��ͨ��������(�ǵ㶯����г)���м�ֵ���ұ����´�ʹ��
    Uint16 common:1;            // 1     -, �߼�����֮�����ͨ��������(�ǵ㶯)
    Uint16 jog:2;               // 3:2   -, 0-no jog, 1-jog
    Uint16 rsvd1:2;             // 5:4   

    Uint16 dir:1;               // 6     +, 0-fwd, 1-rev. ��ʾ���з��򣬲������㶯����

    Uint16 pause:1;             // 7     -, ������ͣ
    
    Uint16 freeStop:1;          // 8     -,
    Uint16 hurryStop:1;         // 9     ?, Ŀǰ��δʹ��

    Uint16 otherStop:1;         // 10    +, ���������ͣ��/��Ĭ��ͣ����ǿ�Ʊ���
    Uint16 startProtect:1;      // 11    +, ��������

    Uint16 errorReset:1;        // 12    -, ���ϸ�λ

    Uint16 rsvd:3;              // 14:13
};

union RUN_CMD
{
    Uint16              all;
    struct RUN_CMD_BITS bit;
};

extern union RUN_CMD runCmd;

//=====================================================================
// ��Ƶ������״̬��Ϣ
//=====================================================================
struct INVT_STATUS_BITS
{
    Uint16 run:2;           // 0;ͣ��  1;��ת   2;��ת
    Uint16 accDecStatus:2;  // 0;����  1;����   2;����
    Uint16 uv:1;            // 0;����  1;Ƿѹ
};

union INVT_STATUS
{
    Uint16               all;
    struct INVT_STATUS_BITS bit;
};
extern union INVT_STATUS invtStatus;

//=====================================================================
// runFlag, ��Ƶ�����й����е�״̬��
//
struct RUN_FLAG_BITS
{                               // bits  description
    Uint16 run:1;               // 0    (�ܵ�)���б�־
    
    Uint16 common:1;            // 1    ��ͨ����(�ǵ㶯���ǵ�г)
    Uint16 jog:1;               // 2    �㶯����
    Uint16 tune:1;              // 3    ��г����

    Uint16 jogWhenRun:1;        // 4    �����е㶯
    
    Uint16 accDecStatus:2;      // 6:5  0 ���٣� 1 ���٣� 2 ����


// ֮�µ�bitλ��shutdownʱ��Ҫ���
    Uint16 plc:1;               // 7     PLC����
    Uint16 pid:1;               // 8     PID����
    Uint16 torque:1;            // 9     ת�ؿ���
    
    Uint16 dir:1;               // 10    �趨Ƶ�ʷ���(������F0-12���з���֮ǰ), 0-fwd, 1-rev
    Uint16 curDir:1;            // 11    ��ǰ����Ƶ�ʷ���, 0-fwd, 1-rev
    Uint16 dirReversing:1;      // 12    ���ڷ����־, 0-��ǰû�з���, 1-���ڷ���
    Uint16 dirFinal:1;          // 13    �趨Ƶ�ʷ���(������F0-12���з���֮��), 0-fwd, 1-rev

    Uint16 servo:1;             // 14
    Uint16 rsvd:1;              // 15
};


union RUN_FLAG
{
    Uint16               all;
    struct RUN_FLAG_BITS bit;
};
extern union RUN_FLAG runFlag;

struct RUN_STATUS_FIRST_STEP_BITS
{
    Uint16 accStep:1;          // �������е�һ��
    Uint16 invarianceStep:1;   // �������е�һ��
    Uint16 decStep:1;          // �������е�һ��
};

union RUN_STATUS_FIRST_STEP
{
    Uint16 all;
    struct RUN_STATUS_FIRST_STEP_BITS bit;
};

extern union RUN_STATUS_FIRST_STEP runStatus1Step;

//=====================================================================


//=====================================================================
// dspMainCmd, ת�ݸ����ܵ���������
//
struct DSP_MAIN_COMMAND_BITS
{                               // bits  description
    Uint16 run:1;               // 0,    0:stop, 1:run
    Uint16 startBrake:1;        // 1,    start brake
    Uint16 stopBrake:1;         // 2,    stop brake
    Uint16 motorCtrlMode:2;     // 4:3   00-SVC, 01-VC, 10-VF
    Uint16 startFlux:1;         // 5:    Ԥ����
    Uint16 torqueCtrl:1;        // 6     ת�ؿ���
    Uint16 accDecStatus:2;      // 8:7   0 ���٣� 1 ���٣� 2 ����. //! Ŀǰת���ʹ���˸ñ�־
    Uint16 spdLoopI1:1;         // 9     �ٶȻ����ַ���
};

union DSP_MAIN_COMMAND
{
    Uint16                       all;
    struct DSP_MAIN_COMMAND_BITS bit;
};

extern union DSP_MAIN_COMMAND dspMainCmd;
//=====================================================================


//=====================================================================
// dspMainCmd1, ת�ݸ����ܵ���������1�������в��ɸ���
//
struct DSP_MAIN_COMMAND1_BITS
{                               // bits  description
    Uint16 pgLocation:2;        // 1:0,  �ٶȷ���PGѡ��. 0������PG��1����չPG��2��PULSE�������루DI5��
    Uint16 fvcPgLogic:1;        // 2,    �ٶȷ���PG����AB����. 1-B��ǰA.
    Uint16 deadCompMode:3;      // 5:3,  ��������ģʽѡ��. 0����������ģʽ0��1����������ģʽ1
    Uint16 modulationMode:1;    // 6     ���Ʒ�ʽ��0-�첽���ƣ�1-ͬ������
    Uint16 rsvd:1;              // 7     ����
    Uint16 rsvd1:1;             // 8     ����
    Uint16 frqPoint:2;          // 10:9  Ƶ��ָ��С���㡣0: 1Hz��1��0.1Hz��2��0.01Hz
    Uint16 speedTrack:2;        // 12:11 0��ת�ٸ�����Ч��1��ת�ٸ���ģʽ1��2��ת�ٸ���ģʽ2
    Uint16 shortGnd:1;          // 13    �ϵ�Եض�·����־
};

union DSP_MAIN_COMMAND1
{
    Uint16                       all;
    struct DSP_MAIN_COMMAND1_BITS bit;
};

extern union DSP_MAIN_COMMAND1 dspMainCmd1;
//=====================================================================


//=====================================================================
// dspSubCmd, ת�ݸ����ܵĸ�������
//
struct DSP_SUB_COMMAND_BITS
{                                       // bits  description
    Uint16 errorDealing:1;              // 0,    1:ERROR TALK, �������ڽ��й��ϴ���
    Uint16 outPhaseLossProtect:1;       // 1,    ���ȱ�ౣ��
    Uint16 inPhaseLossProtect:1;        // 2,    ����ȱ�ౣ��
    Uint16 overloadMode:1;              // 3,    ������ر���ʹ��
    Uint16 loseLoadProtectMode:1;       // 4,    ������ر���ʹ�ܱ�־
    Uint16 poffTransitoryNoStop:1;      // 5,    ˲ͣ��ͣʹ��
    Uint16 cbc:1;                       // 6,    ����������ʹ�ܱ�־
    Uint16 varFcByTem:1;                // 7    �ز�Ƶ�����¶ȵ���
    Uint16 fanRunWhenWaitStopBrake:1;   // 8    ͣ��ֱ���ƶ��ȴ�ʱ���ڷ������б�־
    Uint16 contactorMode:1;             // 9    �Ӵ������ϱ���
};

union DSP_SUB_COMMAND
{
    Uint16                      all;
    struct DSP_SUB_COMMAND_BITS bit;
};

extern union DSP_SUB_COMMAND dspSubCmd;
//=====================================================================



//=====================================================================
// ���ܴ��ݸ����ܵ�״̬��
//
struct DSP_STATUS_BITS
{                                   // bits  description
    Uint16 runEnable:1;		        // 0     1-��ʼ����ɣ��������б�־
    Uint16 uv:1;                    // 1     ĸ�ߵ�ѹǷѹ���ϱ�־  0-Ƿѹ  1-��Ƿѹ
    Uint16 run:1;                   // 2     ����/ͣ��״̬��־
    Uint16 rsvd:1;                  // 3
    Uint16 speedTrackEnd:1;         // 4     ת�ٸ��ٽ�����־
    Uint16 inverterPreOl:1;         // 5     ��Ƶ������Ԥ������־
    Uint16 motorPreOl:1;            // 6     �������Ԥ������־
    Uint16 fan:1;                   // 7     �������б�־�������ŷ�ʹ�ã���������
    Uint16 outAirSwitchOff:1;       // 8     ��Ƶ������տ��Ͽ���־�������ر�־
};

union DSP_STATUS 
{
   Uint16                   all;
   struct DSP_STATUS_BITS   bit;
};

extern union DSP_STATUS dspStatus;
//-----------------------------------------------------


//=====================================================================
// ���ܴ��ݸ����ܵĸ���״̬��
//
struct DSP_SUB_STATUS_BITS
{                                   // bits  description
    Uint16 accDecStatus:2;          // 1:0   0 ���٣� 1 ���٣� 2 ����
    Uint16 rsvd:14;                 // 15:2  ����
};

union DSP_SUB_STATUS 
{
   Uint16                       all;
   struct DSP_SUB_STATUS_BITS   bit;
};

extern union DSP_SUB_STATUS dspSubStatus;
//-----------------------------------------------------




// ����ʱ��������Ƶ��ѡ��
#define ERR_RUN_FRQ_RUN       0  // �Ե�ǰ����Ƶ������
#define ERR_RUN_FRQ_AIM       1  // ���趨Ƶ������
#define ERR_RUN_FRQ_UPPER     2  // ������Ƶ������
#define ERR_RUN_FRQ_LOWER     3  // ������Ƶ������
#define ERR_RUN_FRQ_SECOND    4  // ���쳣ʱ����Ƶ������

//-----------------------------------------------------
// runStatus����ǰ����״̬/����
//
enum RUN_STATUS
{
    RUN_STATUS_WAIT,        // �ȴ�����
    RUN_STATUS_ZERO,        // ��Ƶ����
    RUN_STATUS_START,       // ����
    RUN_STATUS_NORMAL,      // (����)����
    RUN_STATUS_STOP,        // ͣ��
    RUN_STATUS_JOG,         // �㶯����

    RUN_STATUS_POS_CTRL,    // λ�ÿ���
    
    RUN_STATUS_TUNE,        // ��г����
    RUN_STATUS_DI_BRAKE_DEC,  // DI����ֱ���ƶ�Ƶ�ʼ���
    RUN_STATUS_DI_BRAKE,      // DI���ӵ�ֱ���ƶ�(������ֱ���ƶ���ͣ��ֱ���ƶ�)
    RUN_STATUS_LOSE_LOAD,     // ��������
    RUN_STATUS_SHUT_DOWN      // shut down, �ض�
};
extern enum RUN_STATUS runStatus;
//-----------------------------------------------------

//-----------------------------------------------------
enum START_RUN_STATUS
{
    START_RUN_STATUS_SPEED_TRACK,       // ת�ٸ���
    START_RUN_STATUS_BRAKE,             // ����ֱ���ƶ�
    START_RUN_STATUS_PRE_FLUX,          // Ԥ��������
    START_RUN_STATUS_HOLD_START_FRQ     // ����Ƶ�ʱ���
};
#define START_RUN_STATUS_INIT           START_RUN_STATUS_SPEED_TRACK
extern enum START_RUN_STATUS startRunStatus;
//-----------------------------------------------------

//-----------------------------------------------------
enum STOP_RUN_STATUS
{
    STOP_RUN_STATUS_DEC_STOP,           // ����ͣ��
    STOP_RUN_STATUS_WAIT_BRAKE,         // ͣ��ֱ���ƶ��ȴ�
    STOP_RUN_STATUS_BRAKE               // ͣ��ֱ���ƶ�
};
#define STOP_RUN_STATUS_INIT            STOP_RUN_STATUS_DEC_STOP
extern enum STOP_RUN_STATUS stopRunStatus;
//-----------------------------------------------------

//-----------------------------------------------------
// ֱ�߱仯�ļ��㣺�Ӽ��٣�PID�����ļ���
// ��֪�� 0 �� ���ֵmaxValue �ı仯ʱ��Ϊ tickerAll��
// ÿ���µļ��㣬remainderӦ�����㣬����Ӱ���С��
//
typedef struct
{
    void (*calc)(void *);       // ����ָ��

    int32 maxValue;             // ���ֵ
    int32 aimValue;             // Ŀ��ֵ
    int32 curValue;             // ��ǰֵ

    Uint32 tickerAll;           // ��0�����ֵ��ticker
    int32 remainder;            // ����delta����ֵ
} LINE_CHANGE_STRUCT;

#define LINE_CHANGE_STRTUCT_DEFALUTS       \
{                                          \
    (void (*)(void *))LineChangeCalc       \
}

//-----------------------------------------------------

extern Uint32 accFrqTime;
extern Uint32 decFrqTime;
extern Uint16 startProtectSrc;
extern Uint16 otherStopSrc;

extern Uint16 runSrc;

extern Uint16 bDiAccDecTime;
extern Uint16 accDecTimeSrcPlc;

extern Uint16 runDirPanelOld;
extern Uint16 shuntFlag;
extern Uint16 setRunLostTime;
extern Uint16 setRunTimeAim;  // �趨��ʱ����ʱ��;
extern Uint16 swingFrqLimit;  // ��Ƶ�޶���

enum SWING_STATUS
{
    SWING_NONE,             // ��Ƶ�ȴ�����û�а�Ƶ
    SWING_UP,               // ��Ƶ�������׶�
    SWING_DOWN              // ��Ƶ���½��׶�
};
extern enum SWING_STATUS swingStatus;

extern Uint16 tuneCmd;
extern int32 accel;
extern Uint16 accelDisp;


extern Uint16 pcFdbDisp;


extern int32 frqCurAimOld;
extern int32 accTimeOld;
extern int32 decTimeOld;

extern LINE_CHANGE_STRUCT frqLine;


// function
void UpdateRunCmd(void);
void RunSrcDeal(void);
void AccDecTimeCalc(void);
void LineChangeCalc(LINE_CHANGE_STRUCT *p);
void LineFrqChangeCalc(LINE_CHANGE_STRUCT *p);

void AccDecFrqCalc(int32 accTime, int32 decTime, Uint16 mode);

void setTimeRun(void);
void runTimeCal(void);

extern int16 upperTorque;
extern int16 torqueAim;
void TorqueCalc(void);
Uint16 TorqueLimitCalc(void);

#endif  // __F_COMMAND_H__








