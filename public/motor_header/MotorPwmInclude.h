/***************************************************************
�ļ����ܣ�PWM�������ز��������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTOR_PWM_INCLUDE_H
#define MOTOR_PWM_INCLUDE_H

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
typedef struct FC_CAL_STRUCT_DEF{
	Uint	Cnt;
	Uint	Time;
	Uint	FcBak;
    Uint    FcLimitOvLoad;
}FC_CAL_STRUCT;//�����ز�Ƶ�ʳ���ʹ�õ����ݽṹ
typedef struct ANGLE_STRUCT_DEF {
	long 	StepPhase;			//�����Ƕȣ����������
	long 	StepPhaseApply;		//�����Ƕȣ�ʵ��ʹ�ã�
	long 	IMPhase; 			//M��Ƕ�
	int 	OutPhase; 			//PWM�Ƕ�
	int  	CompPhase;			//��λ�ӳٲ����Ƕ�
	int  	RotorPhase;			//ת�ӽǶ�
}ANGLE_STRUCT;
typedef struct OUT_VOLT_STRUCT_DEF {
	int  	Volt;				//Q12�м������̵������ѹ����λ
	int     MaxOutVolt;         //�Ե����ѹ�ı�ôֵ
	int     vfSplit;            //si, 1V, VF����ʱ�������ѹ
	int   	VoltPhase;

	int  	VoltApply;			//Q12�������ϵ��ʹ�õ������ѹ����λ
	int     VoltDisplay;        //��ʾ�����ѹ����ֵΪ��Ƶ�����ѹ
	int   	VoltPhaseApply;
	Uint	VoltSVCCalSignal;
	Uint	VoltSVCCalSignalB;

    Uint    antiVolt;
    int16   detVfVoltUp;
}OUT_VOLT_STRUCT;
typedef enum ZERO_LENGTH_PHASE_SELECT_ENUM_DEF{
    ZERO_VECTOR_U,        //DPWM����ʱ��U�෢ȫ����
    ZERO_VECTOR_V,        //DPWM����ʱ��V�෢ȫ����  
    ZERO_VECTOR_W,        //DPWM����ʱ��W�෢ȫ����
    ZERO_VECTOR_NONE=100  //û�з�ȫ�������
}ZERO_LENGTH_PHASE_SELECT_ENUM;//PWM�����ָ����һ�෢����ȫ����
typedef struct PWM_OUT_STRUCT_DEF {
	long	U;
	long	V;
	long	W;				//�ýṹǰ��Ĳ�����Ҫ�ı�
    ZERO_LENGTH_PHASE_SELECT_ENUM  gZeroLengthPhase; 

	Uint	gPWMPrd;		//����õ��ز�����
	Uint	gPWMPrdApply;	//�ж���ʵ��ʹ���ز�����

	Uint	AsynModle;		//�첽����ģʽ/ͬ������ģʽѡ��
	Uint	PWMModle;		//��������ģʽ/��ɢ����ģʽѡ��
	Uint    SoftPWMTune;
	int    	SoftPWMCoff;

    int     PwmModeSwitchHF;
    int     PwmModeSwitchLF;
}PWM_OUT_STRUCT;	//��ΪPWM����Ľṹ

typedef struct DEAD_BAND_STRUCT_DEF{
   int		DeadBand;			//����ʱ��
   Uint     DeadTimeSet;       //1140V����ʱ�����ϵ��
   int		Comp;				//����ʱ��
   Uint     CompCoff;           //����������У��ϵ��
   int     	CompU;
   int     	CompV;
   int     	CompW;
   int		MTPhase;
   long     InvCurFilter;
}DEAD_BAND_STRUCT; 	//����/����������ر���
typedef struct SYN_PWM_STRUCT_DEF {
	Ulong   FcApply;
	Uint	ModuleApply;	//ʵ��ʹ�õĵ��Ʒ�ʽ
	Uint 	Index;			//ͬ�����Ƶ��ز���
	Uint	AbsFreq;
	Uint	Flag;
}SYN_PWM_STRUCT;	//


/************************************************************/
/*******************���ⲿ���ñ�������***********************/
extern FC_CAL_STRUCT			gFcCal;
extern ANGLE_STRUCT			    gPhase;		//�ǶȽṹ
extern OUT_VOLT_STRUCT			gOutVolt;
extern Uint					    gRatio;			//����ϵ��
extern PWM_OUT_STRUCT			gPWM;
extern SYN_PWM_STRUCT			gSynPWM;
extern DEAD_BAND_STRUCT		    gDeadBand;

/************************************************************/
/*******************���ⲿ���ú�������***********************/
void DeadBandComp(void);
void SendPWM(void);
void CalCarrierWaveFreq(void);
void SoftPWMProcess(void);
void CalOutputPhase(void);
void CalDeadBandComp(void);
void InitSetPWM(void);
void OutPutPWMVC(void);
void OutPutPWMVF(void);

void ImSvcDeadBandComp(void);
void HVfDeadBandComp(void);
void ImFvcDeadBandComp(void);
void PmFvcDeadBandComp(void);


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


