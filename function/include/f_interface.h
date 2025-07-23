//=====================================================================
//
// �����빦�ܵĽӿ�
//
//=====================================================================

#ifndef __F_INTERFACE_H__
#define __F_INTERFACE_H__

//-----------------------------------------------
// �����빦�ܵĽ���
#define FUNC_TO_MOTOR_05MS_DATA_NUM     14      // ����==>���ܵĲ���������0.5ms
extern int16 gSendToMotor05MsDataBuff[];        // ����==>���ܵĲ�����0.5ms

#define FUNC_TO_MOTOR_2MS_DATA_NUM      75     // ����==>���ܵĲ���������2ms
#define FUNC_TO_CORE_DEBUG_DATA_NUM     40      // ����ʹ�ã�func2motor
extern int16 gSendToMotor2MsDataBuff[];         // ����==>���ܵĲ�����2ms

// ���FUNC_TO_CORE_DEBUG_DATA_NUM��������CF�鹦���룬ÿ�Ķ��ᴫ�ݸ����ܣ����ܵ���ʹ��

#define MOTOR_TO_Func_2MS_DATA_NUM      30      // ����==>���ܵĲ���������2ms
#define CORE_TO_FUNC_DISP_DATA_NUM	    30      // ����ʹ�ã�motor2func
extern int16 gSendToFunctionDataBuff[];         // ����==>���ܵĲ�����2ms
// ���CORE_TO_FUNC_DISP_DATA_NUM��������UF�飬������ʾʹ�á�ǰ��(REM_P_OFF_MOTOR)���������

#define MOTOR_TO_Func_05MS_DATA_NUM       0
extern int16 gRealTimeToFunctionDataBuff[];     // ����==>���ܵĲ�����ʵʱ

#define TUNE_DATA_NUM                  20       // ��г�Ĳ�������
extern int16 gParaIdToFunctionDataBuff[];       // ��г
//-----------------------------------------------




// ���ܴ��ݸ����ܵ�����
extern Uint16 frq2Core;
extern Uint16 frqCurAim2Core;

// ���ܴ��ݸ����ܵ�����
extern Uint16 errorCodeFromMotor;
extern Uint16 currentOc;
extern Uint16 generatrixVoltage;
extern Uint16 outVoltage;
extern Uint16 outCurrent;
extern int16 outPower;
extern Uint16 currentPu;
extern Uint16 rsvdData;

extern int32 frqRun;
extern int32 frqVFRun;
extern int32 frqVFRunRemainder;
extern Uint16 frqFdb;
extern Uint16 frqFdbDisp;
extern int32  frqFdbTmp;
extern Uint16 frqFdbFlag;
extern Uint16 motorRun;     // �����������ٶ�
//extern Uint16 PGErrorFlag;  // ������ʶPG�����ϱ�־

extern Uint16 outCurrentDispOld;

extern Uint16 enCoderPosition;
extern Uint16 ABZPos;
// ���ܴ��ݸ����ܼ�ʱ
extern int16 gPhiRtDisp;     // 2    ���������Ƕ�

extern Uint16 pmsmRotorPos;  // ͬ����ת��λ��(����ʵʱ����)

#endif // __F_INTERFACE_H__



