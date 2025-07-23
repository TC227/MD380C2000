/***************************************************************************
�ļ�˵����
�ļ����ܣ�
�ļ��汾��
���¸��£�
������־��
***************************************************************************/
#ifndef MOTOR_ENCODER_INCLUDE_H
#define MOTOR_ENCODER_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"


//***************************************************************************
#define C_MAX_C_ZERO            200             //�����ұ�����Z�źŵ���ʱ��C�ź�����ƽ
#define C_MIN_D_ZERO            19000           //�����ұ�����Z�źŵ���ʱ��D�ź���С��ƽ
#define C_UVW_FOR_ZERO          5
#define C_UVW_BACK_ZERO         6

#define DIR_FORWARD   1
#define DIR_BACKWARD  2
#define DIR_ERROR     0

//#define	RCK	(GpioDataRegs.GPBDAT.bit.GPIO34) // ��74HC594/165�����
#define GetQepCnt()			(*EQepRegs).QPOSCNT
#define SetQepCnt(X)		    (*EQepRegs).QPOSCNT = X;

#define ACTIVE_HARDWARE_LOGICAL_U   1           
#define ACTIVE_HARDWARE_LOGICAL_V   1
#define ACTIVE_HARDWARE_LOGICAL_W   1           // UVW PG ��Ӳ���߼���һ�෴���ˣ�������������

/***********************�ṹ�嶨��***************************/
typedef enum PG_TYPE_ENUM_STRUCT_DEF{  
    PG_TYPE_ABZ,                //��ͨABZ������
    PG_TYPE_UVW,                //��UVW�źŵ�ABZ��ֱ�����
    PG_TYPE_RESOLVER,           //��ת��ѹ��
    PG_TYPE_SC,                 //���������źŵ�ABZ��ֱ����� 
    PG_TYPE_SPECIAL_UVW,        //ʡ�߷�ʽUVW������
    PG_TYPE_NULL=100,           //û�нӱ�����
    PG_TYPE_RT                  // ??
}PG_TYPE_ENUM_STRUCT;       //����������

typedef enum QEP_INDEX_ENUM_STRUCT_DEF{
    QEP_SELECT_1,               //ѡ������QEP1���ڲ���
    QEP_SELECT_2,               //ѡ������QEP2���ڲ���
    QEP_SELECT_PULSEIN,         //ʹ��PULSE�������
    QEP_SELECT_NONE=100         //δѡ���������ٵ�QEPģ��
}QEP_INDEX_ENUM_STRUCT;

typedef struct PG_DATA_STRUCT_DEF {
	Uint 	PGType;
    QEP_INDEX_ENUM_STRUCT   QEPIndex;   //��ǰ����ʹ�õ�QEPģ�飬Ҫ����280xDSP������QEPģ��֮���л�

    Uint    PGMode;                     //��������ʽ�������ͷ�����ʽ��0Ϊ����ʽ
    Uint    PGTypeGetFromFun;           //���ܴ��ݵ�PG������
	Uint 	PulseNum;				    //����������
    Uint    SpeedDir;                   //ABZ,UVW����������AB�źţ�������������ź�
    Uint    SpeedDirLast;               //
    Uint    PGDir;                      //��ʶ�õ��ı������ٶ�������
    Uint    PGErrorFlag;                //��ʶ�õ��ı�����������Ϣ 0-����;1-δ��⵽������;2-�����������趨���� 

    Uint    imPgEstTick;                // im ��������ʶ������
    int     imDirAdder;
    long    imFreqErr;                  // ���ֱ������������
    long    imFrqEncoder;               // 
}PG_DATA_STRUCT;	            //��������ز���

typedef struct ROTOR_SPEED_SMOOTH_DEF {
    int     LastSpeed;
    int     SpeedMaxErr;
}ROTOR_SPEED_SMOOTH;

typedef struct FVC_SPEED_STRUCT_DEF {
//	int 	SpeedApply;			//ʵ��ʹ��ת���ٶ�
    //FVC
	int		Flag;
    Uint    TransRatio;         //������ٴ�����
    Uint    MTZeroCnt;
	int 	SpeedEncoder;		// ͨ����������⵽��ת��Ƶ��, �����������
	int     SpeedTemp;          // ����ǰ����������ֵ
	//M�����ٱ���
	long 	MLastPos;			//ǰһ��λ��
	int 	MDetaPos;			//λ��ƫ��
	int     MDetaPosBak;
	int 	MSpeed;				//M������ֵ
	//T�����ٱ���
	Uint 	MTPulseNum;			//������N
	Ulong 	MTTime;				//N�����ʱ��
	Uint	MTCnt;
	int 	MTSpeed;			//MT������ֵ
	Uint	MTLimitTime;		//
    Uint    IntCnt;
    Uint    IntNum;
    
    ROTOR_SPEED_SMOOTH  MSpeedSmooth;
    ROTOR_SPEED_SMOOTH  MTSpeedSmooth;
}FVC_SPEED_STRUCT;	//�ٶȷ����������ݽṹ

typedef struct IPM_UVW_PG_STRUCT_DEF {
	Uint	UVWData;
	Uint	UVWAngle;       // UVW�źž���λ�ýǶ�, ���ڱ������͵����������ȣ���˱�ʾ��Ƕ�
	
	Uint    U_Value;
	Uint    V_Value;
	Uint    W_Value;
	Uint	ZeroCnt;

    Uint    LogicU;         // U�ź��߼�
    Uint    LogicV;         // V�ź��߼�
    Uint    LogicW;         // W�ź��߼�

    Uint	UvwDir;                 // UVW�ź�������

    Uint    debugdeta1;
    Uint    debugdeta2;
    
    Uint    lastAgl;
    Uint    NewAgl;
    Uint    ErrAgl;
    Uint    TuneFlag;
    
    llong   TotalErr;
    Ulong   UvwCnt;

    int     UvwZIntErr;      // Z�ж�uvw���
    int     UvwZIntErr_deg;
    
    Uint    UvwZeroPos;     // uvw�����λ�ýǶ�
    Uint    UvwZeroPos_deg;

    Uint    UvwEstStep;
    int     UvwCnt2;        // ��е���ڼ�����
}IPM_UVW_PG_STRUCT;//����ͬ�������UVW�����������ݽṹ

typedef struct IPM_PG_DIR_STRUCT_DEF {
	int	    ABAngleBak;
    int     ABDirCnt;
    int     ABDir;
	int	    UVWAngleBak;
    int     UVWDirCnt;
    int     UVWDir;
	int	    CDAngleBak;
    int     CDDirCnt;
    int     CDDir;
    int     CDErr;
    int     RtPhaseBak;
    int     RtDirCnt;
}IPM_PG_DIR_STRUCT; //����ͬ�����������ʶ����������߷�������ݽṹ

typedef struct ROTOR_TRANS_STRUCT_DEF{
    Uint    RTPos;              // ͨ�������ȡ�ĵ�Ƕ�(Totor Transformer)
    Uint    RtorBuffer;         // ��ȡspi��buffer
    int     RealTimeSpeed;
    Ulong 	TimeBak;				//��һ�β��ٵĻ�׼ʱ��
    Ulong	DetaTime;
	Uint    PosBak;
	int		DetaPos;
	int		FreqFeed;
	Uint	Flag;
    Uint    Poles;  
    Uint    PolesRatio;         // ���伫��������
    int     PosComp;
	int     ConFlag;
    Uint    SimuZBack;
    Uint    SimuZBack2;

    Uint    IntNum;
    Uint    IntCnt;

    Uint    RtRealPos;          // �����ʱ��Ƕȣ����˲����Ƕ�֮��

    Uint    AbsRotPos;          // �������λ�ã�0-4096
}ROTOR_TRANS_STRUCT; 	// �������ݽṹ


//***************************************************************************
extern struct EQEP_REGS        *EQepRegs;
extern IPM_UVW_PG_STRUCT        gUVWPG;
extern PG_DATA_STRUCT			gPGData;
extern IPM_PG_DIR_STRUCT        gPGDir;
extern BURR_FILTER_STRUCT		gSpeedFilter;
extern CUR_LINE_STRUCT_DEF		gSpeedLine;
extern ROTOR_TRANS_STRUCT		gRotorTrans;
extern FVC_SPEED_STRUCT		gFVCSpeed;

//*******************���ⲿ���ú�������***********************
extern void GetUvwPhase(void);
extern Uint JudgeABDir(void);
extern Uint JudgeUVWDir(void);
extern Uint JudgeRTDir(void);
extern void GetMTTimeNum(void);
extern void GetMDetaPos(void);
extern void RotorTransCalVel(void);
extern void GetRotorTransPos(void);
extern void RotorTransSamplePos(void);
extern void VCGetFeedBackSpeed(void);
extern void ReInitForPG(void);
extern void InitSetQEP(void);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/

