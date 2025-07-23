/***************************************************************
�ļ����ܣ�����ʸ������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTOR_SVC_INCLUDE_H
#define MOTOR_SVC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
/************************************************************/
/***********************�ṹ�嶨��***************************/
typedef struct SVC_FLUX_CAL_STRUCT_DEF {
	long 	FilterTime;
	Uint 	SampleTime;
}SVC_FLUX_CAL_STRUCT;	//�����ͨ�õı�������

typedef struct FLUX_STRUCT_DEF {
	Uint	Amp;
	Uint	Theta;
}FLUX_STRUCT;	//��ͨ�۲����õı�������
typedef struct SVC_SPEED_STRUCT_DEF {
	int		SvcSynSpeed;
	int		SvcWs;
	int		SvcRotorSpeed;

	//SVCD0 ר�ñ���
	int		SvcLastFluxPos;
	Uint	SvcSignal;           //����SVC0�������ȶ��Բ���
    int     DetaTimer;         //������SVC0
	Ulong	Timer;             //������SVC0ͬ���ټ���
    int     DetaPhase;         //������SVC0ת�Ӵų�ͬ���ټ���
    
}SVC_SPEED_STRUCT;	//SVCת�ٹ������ݽṹ

/************************************************************/
/*******************���ⲿ���ñ�������***********************/
                                                    
extern FLUX_STRUCT              gFluxR;
extern FLUX_STRUCT              gFluxS;
extern MT_STRUCT_Q24           gIMTQ24_obs;  //��ת�ٹ����⣬�蹩�������ƺ���ʹ��
extern MT_STRUCT_Q24           gIMTQ12_obs;  //��ת�ٹ����⣬�蹩�������ƺ���ʹ��

/************************************************************/
/*******************���ⲿ���ú�������***********************/
void ResetSVC(void);
void SVCCalRotorSpeed(void);
void SvcCalOutVolt(void);
void SVCCalFlux_380(void);        
void SvcCalOutVolt_380(void);
void SVCCalRotorSpeed_380(void);



#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

