/***************************************************************
�ļ����ܣ����������ʶ��صı����ͺ�������������ͬ�������첽��
�ļ��汾��
���¸��£�

************************************************************/
#ifndef MOTOR_PARA_ID_INCLUDE_H
#define MOTOR_PARA_ID_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"

#define TUNE_STEP_NOW (gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep])

// // �ṹ�嶨�� 
typedef enum TUNE_FLOW_ENUM_DEF{
    TUNE_NULL = 0,		         //û�е�г
    TUNE_IM_STATIC,              //�첽�����������̬��г
    TUNE_IM_ROTOR,		         //�첽�����������ת��г
    TUNE_PM_COMP_LOAD = 11,   //ͬ�����������λ��ʶ��
    TUNE_PM_COMP_NO_LOAD,  	     //ͬ�����������λ��ʶ��
    TUNE_PM_PARA_temp,           // 13 debug
    TUNE_INERTIA = 20            //����͸��ع�����г
}TUNE_FLOW_ENUM;             //�û��趨�ı�ʶ��ʽ���ǲ�ͬ��ʶ��Ԫ�����

typedef struct UV_AMP_COFF_STRUCT_DEF {
	Ulong	TotalU;
	Ulong	TotalV;
	Ulong	TotalVoltL;
	Ulong	TotalIL;
	Ulong	TotalVolt;
	Ulong	TotalI;
	Uint	Number;
	Uint	Comper;
	Uint	ComperL;
	Uint 	UDivVGet;
	Uint 	UDivV;
	Uint 	UDivVSave;
    Uint    IdRsCnt;            //���ӵ����ʶ����
    Uint    IdRsDelay;          //���ӵ����ظ���ʶ�ȴ�ʱ��
    Uint    IdRsBak;            //�ϴζ��ӵ����ʶֵ
    Uint    Rs_PRD;             //���ӵ�г��Ƶ
}UV_BIAS_COFF_STRUCT;        //�����������������ƫ������ݽṹ

typedef enum PAR_EST_MAIN_STEP_ENUM{
    IDENTIFY_RS,            // im and pm ���ӵ����ʶ
    IDENTIFY_RR_LO,         // im �첽��ת�ӵ����©�б�ʶ
    IDENTIFY_LM_IO,         // im �첽�����кͿ��ص�����ʶ

    PM_EST_POLSE_POS,       // pm �ż�λ�ñ�ʶ
    PM_EST_NO_LOAD,         // pm ���ر��������λ�ñ�ʶ
    PM_EST_WITH_LOAD,       // pm ���ر��������λ�ñ�ʶ
    PM_EST_BEMF,            // pm ���綯�Ʊ�ʶ
    
    IDENTIFY_END            //������ʶ���ָ�����������״̬
}PAR_EST_MAIN_STEP;     //���û��趨�ı�ʶ���̣��ֽ�Ϊ�������ģ�飬˳��ִ��

typedef enum TUNE_TO_FUNCTION_WORLD_ENUM_DEF{
    TUNE_INITIAL,                           //��ʼ״̬
    TUNE_ACC = 50,  			            //��ʼ����
    TUNE_DEC = 51,				            // ��ʼ����
    TUNE_SUCCESS = 100,                     //����������ɣ����Ա���
    TUNE_FINISH = 1000                      //������ʶ����ϻ��û������ֹͣ
}TUNE_TO_FUNCTION_WORLD_ENUM;        //������ʶ�����з���������ģ���״̬��

typedef struct MOTOR_PARAMETER_IDENTIFY_STRUCT{
    PAR_EST_MAIN_STEP    ParEstContent[IDENTIFY_PROGRESS_LENGTH];       //ʵ�ʲ�����ʶ����ѭ������
    Uint16                              ParEstMstep;                    //������ʶ��ǰ˳���
    Uint16                              IdSubStep;                          //���Ƹ���������ʶ���ڲ�ѭ��
    TUNE_TO_FUNCTION_WORLD_ENUM         StatusWord;
    TUNE_FLOW_ENUM                      TuneType;                           // ���ܴ��ݵĵ�г����ѡ��

    int  QtEstDelay;       // quit par-est delay counter
}MOTOR_PARA_EST;

typedef struct IDENTIFY_RRLO_VARIABLE_STRUCT{
int     WaitCnt;
int     IsSampleValue[20];
int     CurrentMax;
int     CurrentRatio;
int     RrL07PulseOverSig;
int     RrL0CycleCnt;
int     SampleTimes;
Uint    PwmCompareValue;
int     UdcVoltage;
long    RrAndRsMax;
long    RrAndRsMin;
long    LoMax;
long    LoMin;
long    RrAndRsAccValue;
long    LoAccValue;
} IDENTIFY_RRLO_VARIABLE;

typedef struct IDENTIFY_LMIO_VARIABLE_STRUCT{
int     WaitCnt;
int     DestinationFreq;
int     DataSavedNum;
Uint    VFOvShock;
long    lIsValue;
long    LmAccValue;
long    IoAccValue;
long    lImAccValue;
long    lIsAccValue;
}IDENTIFY_LMIO_VARIABLE;

// // ���ⲿ���ñ������� 
extern UV_BIAS_COFF_STRUCT		 gUVCoff;
extern MOTOR_PARA_EST            gGetParVarable;
extern MOTOR_EXTERN_STRUCT		 gMotorExtReg;
extern IDENTIFY_RRLO_VARIABLE    gRrLoIdentify;
extern IDENTIFY_LMIO_VARIABLE    gLmIoIdentify;

// // ���ⲿ���ú������� 
extern void RunCaseGetPar(void);
extern void PrepareParForTune(void);
extern void ChgParForEst(void);
extern void LmIoInPeriodInt(void);

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
