/***************************************************************
�ļ����ܣ�����һЩ�������ܼ���:�Եض�·���,�ϵ�����ж�,������,Ӳ��������ѹ����,
          ֱ���ƶ�,�ƶ��������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTOR_PUBLIC_CAL_INCLUDE_H
#define  MOTOR_PUBLIC_CAL_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"

/************************************************************/
/************************�궨��******************************/

/************************************************************/
/***********************�ṹ�嶨��***************************/
typedef struct SHORT_GND_STRUCT_DEF {
	Uint 	Comper;
	Uint 	BaseUDC;
	Uint	ocFlag;
	int		ShortCur;
}SHORT_GND_STRUCT;	//�ϵ�Եض�·������ݽṹ
typedef struct BRAKE_CONTROL_STRUCT_DEF{
	Uint	Flag;				//��ǰ��ͨ/�ض�״̬: 
	                            // bit0 = 0 : ��ʾ��δ���п��ƣ���һ�ĺ���1
	                            // bit1 = 1 : ���ڿ�ͨ�׶Σ�bit1 = 0: ���ڹضϽ׶�
	Uint	Cnt;				//��ǰ��ͨ����
	Uint	OnCnt;				//�ܹ�������ͨ����
	Uint	OffCnt;				//�ܹ������ضϴ���
	Uint    VoltageLim;         //�ƶ����迪ʼ�����ĵ�ѹֵ
}BRAKE_CONTROL_STRUCT;//�ƶ��������ģ��ʹ�õ����ݽṹ
typedef struct DC_BRAKE_STRUCT_DEF{
	int			Time;			//������
	PID_STRUCT	PID;			//�ƶ���ѹ
}DC_BRAKE_STRUCT;//ֱ���ƶ�ģ��ʹ�õ����ݽṹ
typedef struct JUDGE_POWER_LOW_DEF {
	Uint 	PowerUpFlag;
	Uint 	WaiteTime;
	Uint 	UDCOld;
}JUDGE_POWER_LOW;	//�ϵ绺���ж�ʹ�����ݽṹ
/************************************************************/
/*******************���ⲿ���ñ�������***********************/
extern DC_BRAKE_STRUCT			gDCBrake;	//ֱ���ƶ��ñ���
extern BRAKE_CONTROL_STRUCT	    gBrake;		//�ƶ���������ñ���
extern SHORT_GND_STRUCT		    gShortGnd;
extern JUDGE_POWER_LOW			gLowPower;	//�ϵ绺���ж�ʹ�����ݽṹ

/************************************************************/
/*******************���ⲿ���ú�������***********************/
extern void CBCLimitCurPrepare(void);
extern void HardWareOverUDCDeal(void);	
extern void BrakeResControl(void);
extern void RunCaseDcBrake(void);
extern void RunCaseLowPower(void);
//extern void ParameterChange2Ms(void);	
extern void RunCaseShortGnd(void);
extern void ChangeCurrent(void);
extern void ChangeMotorPar(void);
extern void SystemParChg2Ms();
extern void RunStateParChg2Ms();
extern void SystemParChg05Ms();
extern void RunStateParChg05Ms();
extern void InvCalcPower();


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


