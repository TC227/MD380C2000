/****************************************************************

****************************************************************/

#ifndef MOTOR_STRUCT_DEFINE_H
#define  MOTOR_STRUCT_DEFINE_H
#ifdef __cplusplus
extern "C"[
#endif

// // ���ýṹ��Ķ���
struct MAIN_COMMAND_STRUCT_DEF{
   Uint    Start:1;					// 1 �𶯣� 0 ͣ��
   Uint    StartDC:1;				// 1 ��ֱ���ƶ�
   Uint    StopDC:1;				// 1 ͣ��ֱ���ƶ�����   
   Uint    ControlMode:2;           // 0��SVC��1��FVC��2��VF
   Uint    PreExcFlux:1;            // 1 Ԥ��������
   Uint	   TorqueCtl:1;				// 1 ת�ؿ��Ʊ�־   
   Uint    SpeedFlag:2;             // 0�����٣�1�����٣�2������
   Uint    IntegralDiscrete:1;      // 1�����ַ���
};

union MAIN_COMMAND_UNION_DEF {
   Uint   all;
   struct MAIN_COMMAND_STRUCT_DEF  	bit;
};

struct MAIN_COMMAND_EXTEND_STRUCT_DEF{
   Uint    QepIndex:2;              // 0��ʹ��EQP2����,1��ʹ��QEP1���٣�2��PULSE��������
   Uint    SpeedRev:1;				// 1 ���̷������־(//rt �δ�����ʱ����)
   Uint    DeadCompMode:3;           //0��������������1��AD�������� 2���Ż��������� 2011.5.7 L1082
   Uint    ModulateType:1;          //0���첽���ƣ�1 ��ͬ������
   Uint    Reserved:1;             //0������
   Uint    Reserved1:1;             //����
   Uint    FreqUint:2;              //Ƶ��ָ�λ��0��1Hz��1��0.1Hz��2��0.01Hz
   Uint    SpeedSearch:2;			// 0��ת�ٸ�����Ч��1��ת�ٸ���ģʽ1��2��ת�ٸ���ģʽ2
   Uint    ShortGnd:1;				// 1���ϵ�Եض�·����־   
};

typedef union MAIN_COMMAND_EXTEND_UNION_DEF{
    Uint all;
    struct MAIN_COMMAND_EXTEND_STRUCT_DEF bit;
}MAIN_COMMAND_EXTEND_UNION;

struct SUB_COMMAND_STRUCT_DEF{
   Uint    ErrorOK:1;				// 1 ���ϴ�����ϱ�־
   Uint    OutputLost:1;			// 1 ���ȱ����ʹ��
   Uint    InputLost:1;				// 1 ����ȱ�ౣ��ʹ��
   Uint    MotorOvLoad:1;			// 1 ������ر���ʹ��
   Uint	   LoadLose:1;				// 1 ������ر���ʹ�ܱ�־
   Uint    NoStop:1;				// 1 ˲ͣ��ͣʹ��
   Uint	   CBCEnable:1;				// 1 ����������ʹ�ܱ�־
   Uint    VarFcByTem:1;			// 1 �ز�Ƶ�����¶ȵ���(�̶�Ϊ1)
   Uint    FanNoStop:1;				// 1 ͣ��ֱ���ƶ��ȴ�ʱ���ڷ������б�־  
}; 
typedef union SUB_COMMAND_UNION_DEF {
   Uint   all;
   struct SUB_COMMAND_STRUCT_DEF  	bit;
}SUB_COMMAND_UNION;

struct SEND_STATUS_STRUCT_DEF{
   Uint    RunEnable:1;				// 1 ��ʼ����ɣ��������б�־
   Uint    LowUDC:1;				// 0 ĸ�ߵ�ѹǷѹ���ϱ�־
   Uint    StartStop:1;				// 1 ����/ͣ��״̬��־ (���ڸñ�־û����)//rt
   Uint    ShortGndOver:1;			// 1 �Եض�·�����ϱ�־    
   Uint    SpeedSearchOver:1;		// 1 ת�ٸ��ٽ�����־
   Uint    PerOvLoadInv:1;			// 1 ��Ƶ������Ԥ������־
   Uint    PerOvLoadMotor:1;		// 1 �������Ԥ������־
   Uint    FanControl:1;			// 1 ��������
   Uint    OutOff:1;				// 1 ��Ƶ������տ��Ͽ���־
}; 

typedef union SEND_STATUS_UNION_DEF {
   Uint   all;
   struct SEND_STATUS_STRUCT_DEF  	bit;
}SEND_STATUS_UNION;

// // ����Ϊ������Ϣ���ݽṹ 
typedef struct INV_STRUCT_DEF {
	Uint 	InvTypeApply;			    //ʵ��ʹ�õĻ��ͣ�P�ͻ� = G�ͻ�-1��
	Uint    InvTypeSet;                 //���ܴ��ݵĻ��ͣ�������ѹ�ȼ���Ϣ
	Uint    InvVoltageType;             //��Ƶ����ѹ�ȼ���Ϣ
	Uint 	InvType;				    //�û����õĻ��ͣ�ȥ����ѹ�ȼ���Ϣ����Χ��00��99
	Uint    GpTypeSet;                  // �������趨��GP
	Uint    GPType;                     // 1 G�ͻ���2 P�ͻ�
	Uint 	InvCurrent;				    //���õ��ı�Ƶ��������	��λ�ɻ���ȷ��
	Uint    InvCurrentOvload;           //��Ƶ�����ص�����	��λ0.01A
	Uint    InvCurrForP;                //P�ͻ�ʹ�õĶ����       ���ر����õ���
	Uint 	InvVolt;				    //���õ��ı�Ƶ����ѹ��	��λ1V
	Uint 	CurrentCoff;			    //��Ƶ����������ϵ��  ��	��λ0.1%
	Uint 	UDCCoff;				    //��Ƶ��ĸ�ߵ�ѹ����ϵ����	��λ0.1%
	Uint 	TempType;				    //��Ƶ���¶�����ѡ��
	Uint 	InvUpUDC;				    //ĸ�߹�ѹ��				��λ0.1V
	Uint    InvLowUdcStad;              // δУ����Ƿѹ��
	Uint 	InvLowUDC;				    //Ƿѹ��
	Uint    LowUdcCoff;                 //Ƿѹ��У��ϵ��
	Uint 	BaseUdc;				    //ĸ�ߵ�ѹ��׼ 380V����Ϊ537.4V
}INV_STRUCT;                         //��Ƶ��Ӳ����Ϣ�ṹ

// // ʹ��ö�ٱ������ֵ�����ͺͿ���ģʽ�����
typedef enum CONTROL_MOTOR_TYPE_ENUM_DEF{
    ASYNC_SVC,                          // 
    ASYNC_FVC,                          // 
    ASYNC_VF,                           // �첽��SVC����Ϊ0��FVCΪ1��VFΪ2
    
    SYNC_SVC = 10,                      // 
    SYNC_FVC,
    SYNC_VF,                            // ͬ����SVCΪ0��FVCΪ1��VFΪ2 
    
    DC_CONTROL = 20,                    // ֱ���ƶ�Ϊ20
    RUN_SYNC_TUNE                       // ������ʶʱ��Ҫ������е�״̬����Ҫ��ͬ����ʹ��
}CONTROL_MOTOR_TYPE_ENUM;  

typedef struct MOTOR_STRUCT_DEF {
    Uint    MotorType;              //0����ͨ�첽�����1����Ƶ�첽�����2������ͬ�����
    Uint    LastMotorType;          //�洢��һ�Ĺ��ܴ��ݵĵ�����ͣ�����ĳЩ�ڵ�����͸ı�ʱ����Ҫ�����ı���
	Uint 	Power;					//�������					��λ0.1KW
	Uint 	Votage;					//�����ѹ					��λ1V
	Uint 	CurrentGet;				//���ܴ��ݵĵ������			��λ�ɻ���ȷ��
	Uint 	Frequency;				//���Ƶ��					
	Uint 	FreqPer;				//��ôֵ���Ƶ��
	Uint 	Current;				//��������ѡ�õĵ�����ֵ(���ܺ�ʵ�ʵ����������)					
	Uint	CurBaseCoff;			//������ֵ�ķŴ���
	Uint    Motor_HFreq;            // ����Ƶ�ʵ�60%
    Uint    Motor_LFreq;            // ����Ƶ�ʵ�40%
}MOTOR_STRUCT;                   //���������Ϣ�ṹ

typedef struct MOTOR_EXTERN_STRUCT_DEF {
    Uint    UnitCoff;                           //�������ת��ϵ�� ����С��22��Ϊ1������22��Ϊ10
	Uint 	R1;						            //���������		 ����С��22����λ0.001ŷķ������22��0.0001ŷķ
	Uint 	R2;						            //ת�������		 ����С��22����λ0.001ŷķ������22��0.0001ŷķ
	Uint 	L0;						            //©��			 ����С��22����λ0.01mH������22����λ0.001mH
	Uint 	LM;						            //����			 ����С��22����λ0.1mH������22����λ0.01mH
	Uint 	I0;						            //���ص���		 ����С��22����λ0.01A������22����λ0.1A
	Uint    IoVsFreq;                           //�����������ٶȱ仯�Ŀ��ص�����ֻ�������ı�ôֵ
	Uint 	Rpm;					            //���ת��		 ��λ1rpm
    Uint    RatedComp;	                        //�ת����       0.01Hz, pu
	Uint 	Poles;					            //�������
	Uint 	L1;						            //��������		 ����С��22����λ0.1mH������22����λ0.01mH
	Uint 	L2;						            //ת������		 ����С��22����λ0.1mH������22����λ0.01mH
    Uint    RsPm;                               // ͬ�������ӵ���
	Uint 	LD;						            //ͬ����D����     ���� <22����λ0.01mH������ >22����λ0.001mH
	Uint 	LQ;						            //ͬ����Q����     ���� <22����λ0.01mH������ >22����λ0.001mH
	Uint    BemfVolt;                               // ͬ�������綯�Ƶ�ѹ�� ���Լ����ת�Ӵ���
	Uint    FluxRotor;                              // ͬ����ת�Ӵ���  Q12
    Uint    FluxRotor1;
    Uint    ItRated;                                // ����ص�����pm im����
    Uint    FluxLeakCurveGain;
}MOTOR_EXTERN_STRUCT;   //�����չ��Ϣ�ṹ

struct ERROR_FLAG_SIGNAL {
    Uint16  OvCurFlag:1;                            // bit0=1��ʾ�����˹����ж�
    Uint16  OvUdcFlag:1;                            // bit1=1��ʾ�����˹�ѹ�ж�
	Uint16  Res:14;                                 // ����    
};

union ERROR_FLAG_SIGNAL_DEF {
   Uint16                	all;
   struct ERROR_FLAG_SIGNAL  bit;
};

typedef struct PRG_STATUS_STRUCT_DEF{
   Uint    PWMDisable:1;			                        //BIT0=1 ��ʾ������PWM����
   Uint    ACRDisable:1;			                        //BIT1=1 ��ʾ�������������
   Uint    ASRDisable:1;			                        //BIT2=1 ��ʾ�������ٶȻ���
}PRG_STATUS_STRUCT;                                     //�������п���״̬�ֵ�bit����

typedef union PRG_STATUS_UNION_DEF {                // ����״̬��
   Uint   all;
   PRG_STATUS_STRUCT bit;
}PRG_STATUS_UNION;

typedef struct RUN_STATUS_STRUCT_DEF {
	Uint 	RunStep;				                        //������
	Uint 	SubStep;				                        //������
	Uint    ParaCalTimes;                                   //���ڿ����ϵ��ֻ����һ�εĲ���ת����
	PRG_STATUS_UNION 	PrgStatus;				            //�������״̬��	
	union   ERROR_FLAG_SIGNAL_DEF	ErrFlag;									
    SEND_STATUS_UNION		StatusWord;	
}RUN_STATUS_STRUCT;                                     //��Ƶ������״̬�ṹ


typedef struct BASE_COMMAND_STRUCT_DEF {
	union MAIN_COMMAND_UNION_DEF Command;	            //�������ֽṹ
	int 	FreqSet;				                    // ���ܴ��ݵ��趨Ƶ��
	int     FreqSetApply;                               // ����ʵ��ʹ�õ��趨Ƶ��
	int		FreqSetBak;				                    //���µķ�0�����ٶ�
	int 	FreqSyn;			                        //ʵ���ٶȣ�ͬ���ٶȣ�
    int     FreqSynFilter;                              //ʵ�����ͬ��Ƶ�ʵ��˲�ֵ����������������
	int     FreqWs;                                   // ʸ��ʱ�����ת��
	int 	FreqDesired;			                    //Ŀ���ٶ�
	int 	VCTorqueLim;			                    //VCת���޶�
	int     FreqToFunc;                                 //����������ģ����ٶȣ���ôֵ��ʾ
	                                                    // ���ڷ����ٶȷ�Ϊ��Ƶ������Ƶ�ʺͱ���������
    int     FreqFeed;
	long 	FreqReal;				                    // ʵ���趨Ƶ��(�Ǳ�ôֵ��ʾ),��λ0.01Hz
	long    FreqDesiredReal;                            // ʵ��Ŀ��Ƶ��(�Ǳ�ôֵ��ʾ),��λ0.01Hz	
    long    FreqSetReal;                                // ʵ���趨Ƶ��(�Ǳ�ôֵ��ʾ),��λ0.01Hz	
                                                        // ���ܴ��ݵ�Ƶ��ֵת��Ϊ0.01Hzʱ��ϵ��
    long    FreqRealFilt;
                                                        
	Uint    pu2siCoeff;                          //��ôֵƵ�ʻ�ֵ��λ�������ʹ�õ�ʵ��Ƶ�ʵ�λ��ͬ������һ��ת��ϵ��
	                                                    // 1Hz ת��Ϊ����С�����ϵ��              
	Uint    si2puCoeff;                          //�����е�ʵ��Ƶ��ת��Ϊ��ôֵƵ�ʣ���Ҫ��У��ϵ����    
	Uint	FirstCnt;                            //��������ʱ�����⴦���ӳټ���ֵ

    //Uint	SpeedFalg;				//�Ӽ��ٱ�־
}BASE_COMMAND_STRUCT;                              //ʵʱ�޸ĵ�����ṹ

// // ����Ϊ�͵����������趨�����������ݽṹ 
typedef struct BASE_PAR_STRUCT_DEF {
	Ulong 	FullFreq01;				                    // Full freq, SI, 0.01Hz;
	Uint    FullFreq;                                   // SI, С�����빦�ܻ�ֵһ�£����ڽ����ܴ��ݵ�ʵ��ֵת��Ϊ��ôֵ
	Uint 	MaxFreq;				                    //���Ƶ�� format: point
	Uint 	FcSet;					                    //�趨�ز�Ƶ��		
	Uint 	FcSetApply;				                    //ʵ���ز�Ƶ��		
}BASE_PAR_STRUCT;	                                //����������Ϣ�ṹ

typedef struct COM_PAR_INFO_STRUCT_DEF {
	Uint 	StartDCBrakeCur;		                    //��ֱ���ƶ�����
	Uint 	StopDCBrakeCur;			                    //ͣ��ֱ���ƶ�����
	Uint 	BrakeCoff;				                    //�ƶ�����ʹ����
	Uint 	MotorOvLoad;			                    //������ر�������
	Uint 	PerMotorOvLoad;			                    //������ر���Ԥ��ϵ��
	Uint 	SpdSearchMethod;		                    //�ٶ�������ʽ
	Uint 	SpdSearchTimeSet;                           //���ܴ����ٶ���������
	Uint 	SpdSearchTime;			                    //ʵ���ٶ���������
}COM_PAR_INFO_STRUCT;	                            //�����з�ʽ�޹صĲ������ýṹ

typedef struct MOTOR_POWER_TORQUE_DEF{
    long    InvPowerPU;                             // ��Ƶ��������ʱ�ôֵ
    int     InvPower_si;                            // ��Ƶ���������
    int     TrqOut_pu;                              // ���ת�� 0.1%

    Uint    rpItRated;                              // ����ص����ĵ��� Q12    
    int     anglePF;
    int     Cur_Ft4;
}MOTOR_POWER_TORQUE;

// // ����ģ��ʹ�õ����ݽṹ
typedef struct CPU_TIME_STRUCT_DEF {
	Ulong 	MFA2msBase;
    Ulong 	MFB2msBase;
    Ulong 	MFC2msBase;
    Ulong 	MFD2msBase;
    Ulong   Motor05MsBase;
	Ulong	ADCIntBase;
	Ulong	PWMIntBase;
	Uint 	MFA2ms;
    Uint 	MFB2ms;
    Uint 	MFC2ms;
    Uint 	MFD2ms;
	Uint    Motor05Ms;
	Uint	ADCInt;				                    //ADC�ж�ִ��ʱ��
	Uint	PWMInt;				                    //PWM�ж�ִ��ʱ��
    Uint    tmpBase;
    Uint    tmpTime;
    Uint    CpuCoff0Ms;
    Uint    tmp0Ms;
    Uint    Det05msClk;
    Uint    CpuBusyCoff;                            // cpuʱ��Ƭִ�ж�ϵ��
}CPU_TIME_STRUCT;	    //ͳ��ģ��ִ��ʱ������ݽṹ

// // �������ֵ��������ݽṹ
typedef struct MOTOR_DEBUG_DATA_RECEIVE_STRUCT_DEF{
    int     TestData0;
    int     TestData1;
    int     TestData2;
    int     TestData3;
    int     TestData4;
    int     TestData5;
    int     TestData6;
    int     TestData7;
    int     TestData8;
    int     TestData9;
    int     TestData10;
    int     TestData11;
    int     TestData12;
    int     TestData13;
    int     TestData14;
    int     TestData15;
    int     TestData16;
    int     TestData17;
    int     TestData18;
    int     TestData19;
    int     TestData20;
    int     TestData21;
    int     TestData22;
    int     TestData23;
    int     TestData24;
    int     TestData25;
    int     TestData26;
    int     TestData27;
    int     TestData28;
    int     TestData29;
    int     TestData30;
    int     TestData31;
    int     TestData32;
    int     TestData33;
    int     TestData34;
    int     TestData35;
    int     TestData36;
    int     TestData37;
    int     TestData38;
    int     TestData39;
}MOTOR_DEBUG_DATA_RECEIVE_STRUCT;   //���ܵ������ݽ���

typedef struct MOTOR_DEBUG_DATA_DISPLAY_STRUCT_DEF{
    int     DisplayData1;
    int     DisplayData2;    
    int     DisplayData3;
    int     DisplayData4;
    int     DisplayData5;    
    int     DisplayData6;
    int     DisplayData7;
    int     DisplayData8;    
    int     DisplayData9;
    int     DisplayData10;
    int     DisplayData11;
    int     DisplayData12;    
    int     DisplayData13;
    int     DisplayData14;
    int     DisplayData15;    
    int     DisplayData16;
    int     DisplayData17;
    int     DisplayData18;    
    int     DisplayData19;
    int     DisplayData20; 
    int     DisplayData21;
    int     DisplayData22;    
    int     DisplayData23;
    int     DisplayData24;
    int     DisplayData25;    
    int     DisplayData26;
    int     DisplayData27;
    int     DisplayData28;    
    int     DisplayData29;
    int     DisplayData30;     
}MOTOR_DEBUG_DATA_DISPLAY_STRUCT;   //���ܵ���������ʾ

// // �ṹ�嶨�����

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif

