/***************************************************************
�ļ����ܣ���VF������ص����ݽṹ���壬��������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTOR_VF_INCLUDE_H
#define MOTOR_VF_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorCurrentTransform.h"

/************************************************************/
/***********************�ṹ�嶨��***************************/
typedef struct VF_INFO_STRUCT_DEF {
	Uint 	VFLineType;				//VF����ѡ��
	Uint 	VFTorqueUp;				//VFת����������
	Uint 	VFTorqueUpLim;			//VFת����������Ƶ��
	Uint 	VFOverExc;				//VF����������
	Uint 	VFWsComp;				//VFת�������
	Uint 	VFOvShock;				//VF����������
	Uint 	VFLineFreq1;			//���VFƵ��1
	Uint 	VFLineVolt1;			//���VF��ѹ1
	Uint 	VFLineFreq2;			//���VFƵ��2
	Uint 	VFLineVolt2;			//���VF��ѹ2
	Uint 	VFLineFreq3;			//���VFƵ��3
	Uint 	VFLineVolt3;			//���VF��ѹ3
	Uint 	SVCTorqueUp;			//svcת����������
	Uint 	SVCTorqueUpLim;			//svcת����������Ƶ��

    Uint    vfResComp;      // ��Ƶ����ѹ���Ƿ����·ֽ����
    Uint    ovGain;     // ��ѹʧ������
    Uint    ovPoint;    // ��ѹʧ�����Ƶ�
    Uint    ovPointCoff;
    Uint    ocGain;     // ����ʧ������
    Uint    ocPoint;    // ����ʧ�ٵ�
    int     FreqApply;  // vf ʧ�ٿ��Ʋ�����Ƶ��

    int     vfMode;     // Vfʧ�ٿ���ģʽ��0:280ģʽ��1:��ģʽ
    int     tpLst;
}VF_INFO_STRUCT;	//VF�����������ݽṹ

typedef struct  VF_AUTO_TORQUEBOOST_VAR_DEF{
    Uint        VfAutoTorqueEnable;     //  1- Execute auto torque boost
    int         DestinationVolt;        //  �Զ�ת�������ջ���Ŀ���ѹ
    int         VfCurrentIs;
    int         VfReverseAngle;
    int         VfRIsSinFai;
    int         VfRVCosFai;
    int         VfAngleSin;
    int         VfReverseVolt;
    int         VfTorqueEnableTime;
    PID_STRUCT  AutoTorquePID;
}VF_AUTO_TORQUEBOOST_VAR;	//�Զ�ת�������ñ����ṹ

typedef struct OVER_SHOCK_STRUCT_DEF{
	int				IMFilter;
    int             IO;             //�����񵴵����ŵ�������ֵӦ�������ӣ�������ʻ�������ʱ����һ���������
	PID_STRUCT		pid;
    Uint            TimesNub;       //������4s���ڣ�������Ŀ�����ʹ�ÿ��ص����趨ֵ
    Uint            oscMode;        //������ģʽ
    int             ShockDecrease;  //�����������������������DPWM����ʱ��ȡ��������

    int             OscVolt;
    //int             OscPhase;
}OVER_SHOCK_STRUCT;

typedef struct VF_WS_COMP_STRUCT_DEF {
	Uint 	   Coff;
	long       CompFreq;
    int        DelayTime;
    int        FilterCoff; //�˲�ϵ��
    MT_STRUCT  WsCompMT;
    MT_STRUCT  WsCompMTApply;
}VF_WS_COMP_STRUCT;	

typedef struct VAR_AVR_STRUCT_DEF{
	int		UDCFilter;
	int		CoffApply;
	int		Cnt;
}VAR_AVR_STRUCT;//������ģ��(�ɵ�AVR����)ʹ�õ����ݽṹ

#if 0
typedef struct VF_CURRENT_CONTROL_DEF{
    Uint      disVfCsr;

    Uint      ocPoint;
    Uint      currentLimit;
    Uint      active;      // �����������ñ�־
    Uint      vfCsrKP;
    Uint      vfCsrKI;
    Uint      vfCsrKD;
    
    int16       detaVolt;
    PID_STRUCT	pid;	
}VF_CURRENT_CONTROL;
#endif

typedef struct THREE_ORDER_FILTER_DEF{ 
    long    OutK1;
    long    OutK2;
    long    OutK3;
    
    long    InK1;
    long    InK2;
    long    InK3;
    
    long    OutData1;
    long    OutData2;
    long    OutData3;
    
    long    InData1;
    long    Indata2;
    long    Indata3;    
}THREE_ORDER_FILTER; //�����˲�����ʹ�õ�ϵ��

typedef struct VF_VAR_CALC_DEF{
    int     vfTq;       // Q12
    int     vfPt;       // Q12
}VF_VAR_CALC;

typedef struct VF_OVER_UDC_DAMP_DEF
{
    //int16 mStepUdc;   // ����Ƶ�ʱ仯����(��ѹ)
    int16 uDcBrakePt;
    int16 vfOvUdcPt;
    
    PID_STRUCT pidUdc;
    PID_STRUCT pidPower;

    int16       powerFdb;
    int16       powerSet;
    Uint    maxPower;
} VF_OVER_UDC_DAMP;

typedef struct VF_FREQ_DEAL_DEF
{
    int    stepCur;
    int    stepUdc;
    int    stepSet;
    int    stepEnd;
    Uint    freqDir;
    Uint    freqSet;
    Uint    freqAim;
    Uint    freqApply;
    Uint    preSpdFlag; 
    Uint    spedChg;
}VF_FREQ_DEAL;

typedef struct OVER_CURRENT_DAMP_DEF2{
	int		LowFreq;			//��Ƶת�۵� (SI)

	int		stepApply;			//ʵ��ʹ�õĲ���
	int		stepLAmp;			//����С��0.88������Ĳ���
	int		stepLAmpLim;			//����С��0.88������Ĳ�������
	int		stepHAmp;			//��������0.88������Ĳ���
	int		CurBak;				//��һ�ĵ���ֵ

	int		maxStepLF;			//���ڵ�Ƶת�۵����󲽳�
	int		maxStepHF;		//���ڵ�Ƶת�۵����󲽳�
	int		ocPointQ12;
	int		subStep;			//��������0.88������ʱ������С�ٶ�
	int     addStep;
	int		Flag;				//��־
								//BIT0	=1 ��ʾ�ٶ��Ѿ�׷���趨�ٶ�
								//BIT15 =1 ��ʾ�����Ѿ������������0.88��
} OVER_CURRENT_DAMP2;//��������ģ��ʹ�õ����ݽṹ

typedef struct OVER_CURRENT_DAMP_DEF{
	int		StepApply;			//ʵ��ʹ�õĲ���
	int		StepLow;			//����С��0.88������Ĳ���
	int		StepLowLim;			//����С��0.88������Ĳ�������
	int		StepHigh;			//��������0.88������Ĳ���
	int		CurBak;				//��һ�ĵ���ֵ
	int		LowFreq;			//��Ƶת�۵�
	int		MaxStepLow;			//���ڵ�Ƶת�۵����󲽳�
	int		MaxStepHigh;		//���ڵ�Ƶת�۵����󲽳�
	int		CurLim;
	int		SubStep;			//��������0.88������ʱ������С�ٶ�
	int		Flag;				//��־
								//BIT0	=1 ��ʾ�ٶ��Ѿ�׷���趨�ٶ�
								//BIT15 =1 ��ʾ�����Ѿ������������0.88��
}OVER_CURRENT_DAMP;//��������ģ��ʹ�õ����ݽṹ

typedef struct OVER_UDC_CTL_STRUCT_DEF{
	int		CoffApply;
	int		CoffAdd;
	int		Limit;              // ��ѹ���Ƶ�
	int		StepApply;
    int     LastStepApply;
	int		StepBak;
	int		ExeCnt;
	int		UdcBak;
	int		Flag;
	int		FreqMax;			//���ٵ�һ�ĵ�Ƶ��
	int     AccTimes;           //��ѹ���Ƶ��µ�Ƶ�����Ӵ���
	int		OvUdcLimitTime;

    //int     PreStepApply;
}OVER_UDC_CTL_STRUCT;//��������ģ��ʹ�õ����ݽṹ

typedef struct HVF_OSC_DAMP_STRUCT_DEF
{
#if 0
    int FreqSet;                // �����趨ͬ��Ƶ��
    int FreqSynApply;           // ʵ��ͬ��Ƶ��
    int FreqSpliEst;            // ����ת��
    int VoltOsc;
    int CompLeakageLs;          // �Ƿ񲹳�©�е�ѹ
#endif

    int OscDampGain;            // ����������
    int VoltSmSet;              // m���ѹ
    int VoltEmf;                // vvvf�����ƣ�vers freq
    
    int VoltAmp;                // �����ѹ��ֵ
    int VoltPhase;              // ��ѹ��λ

#if 0
//debug
    int detVoltT;
    int detVoltM;
    // gama ģ��
    int Rs;
    int Lt;             //Q format
    int Lg;             // ©��
    int LgPerSet;       // ©��ϵ���趨
    int CurM;
    int CurMDealy;      // �ӳ��˲�
    int taoCurMDealy;
    int CurTDealy;
#endif

    int Rs;
    int CurMagSet;          // ���ŵ����趨
    int VoltAmpAdjActive;    // ��ѹ��ֵ�Ƿ������ С���ʵ���������ת������
    
}HVF_OSC_DAMP_STRUCT ;

typedef struct HVF_OSC_JUDGE_INDEX_DEF
{
    int AnglePowerFactor;      // �������ؽ�
    
    int wCntUse;
    int wCntRltm;
    int maxAngle;
    int minAngle;
    int oscIndex;
    
}HVF_OSC_JUDGE_INDEX;

typedef struct HVF_DB_COMP_OPT_DEF
{
    int     HVfDbCompOptActive;     // :=1 �����Ż�
    int     PhasPredictGain;        // ��λԤ������
    
    int     CurPhaseFeed;
    int     CurPhaseFeed_pre;  // ��һ��
    int     CurPhasePredict;   // Ԥ����λ
    int     StepPhaseSet;       // ÿ���жϵ���λstep(Q15)
    int     CurPhaseStepFed;    // ���õ��ĵ�����λ�仯
    int     CurPhaseStepPredict;// Ԥ����λ�仯��
    int     PhaseFwdFedCoeff;   // ǰ��ϵ��

    int     DbOptActHFreq;      // �Ż���Ч����Ƶ�ʵ�
    int     DbOptActLFreq;      // �Ż���Ч����Ƶ�ʵ�

    int     DbCompCpwmWidth;    // 
}HVF_DB_COMP_OPT;

/************************************************************/
/*******************���ⲿ���ñ�������***********************/
extern VF_INFO_STRUCT			gVFPar;		//VF����
extern VF_AUTO_TORQUEBOOST_VAR  gVFAutoVar;
extern OVER_SHOCK_STRUCT		gVfOsc;	//�������ýṹ����
extern VF_WS_COMP_STRUCT		gWsComp;
extern VAR_AVR_STRUCT			gVarAvr;

//extern VF_CURRENT_CONTROL       gVfCsr;
extern MT_STRUCT				gHVfCur;

extern THREE_ORDER_FILTER      gOscAmp;
extern OVER_UDC_CTL_STRUCT		gOvUdc;
extern Uint const gExponentVf[][129];

extern HVF_OSC_DAMP_STRUCT      gHVfOscDamp;
extern HVF_OSC_JUDGE_INDEX      gHVfOscIndex;
extern HVF_DB_COMP_OPT          gHVfDeadBandCompOpt;

/*******************���ⲿ���ú�������***********************/

int  CalOutVotInVFStatus(int);
void OverShockControl(void);
void VFOverMagneticControl(void);
void VfVarInitiate(void);
void VFWsTorqueBoostComm( void );
void VFWSCompControl(void);		
void VfFreqDeal2(void);
void VFSpeedControl(void);
void VFAutoTorqueBoost(void);

void VfFreqDeal(void);
void VfOverCurDeal();
void VfOverUdcDeal();

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

