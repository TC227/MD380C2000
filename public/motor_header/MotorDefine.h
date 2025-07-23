/************************************************************
------------------���ļ����������ͷ�ļ�---------------------
�������ܲ���ʹ�õĺ�ͳ���(2908 ��28035 �ǹ��õ�)
************************************************************/
#ifndef MAIN_INCLUDE_H
#define MAIN_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"

/*----------------------------------------------------------------------
---------------------------Data typr show-------------------------------
Type 				Size 		Representation 		Minimum 		Maximum

char, signed char 	16 bits 	ASCII 				-32768 			32767
unsigned char 		16 bits		ASCII 				0 				65535
short 				16 bits 	2s complement 		-32768 			32767
unsigned short 		16 bits 	Binary 				0 				65535
int, signed int 	16 bits 	2s complement 		-32768 			32767
unsigned int 		16 bits 	Binary 				0 				65535
long, signed long 	32 bits 	2s complement 		-2147483648 	214783647
unsigned long 		32 bits 	Binary 				0 				4294967295
long long, 
signed long long	64 bits 	2s complement 		-9223372036854775808 
																	9223372036854775807
unsigned long long 	64 bits 	Binary 				0 				18446744073709551615
enum 				16 bits 	2s complement 		-32768 			32767
pointers 			16 bits 	Binary 				0 				0xFFFF
far pointers 		22 bits 	Binary 				0 				0x3FFFFF
-----------------------------------------------------------------------*/
/************************************************************
	����ʹ�õ��¶����������
************************************************************/
typedef	long long 				llong;
typedef	unsigned int			Uint;
typedef	unsigned long			Ulong;
typedef	unsigned long long 		Ullong;

typedef struct BIT32_REG_DEF {
   Uint16  LSW;
   Uint16  MSW;
}BIT32_REG;

typedef union BIT32_GROUP_DEF {
   Uint32     all;
   BIT32_REG  half;
}BIT32_GROUP;

/************************************************************
	����Ϊ���Դ�����ʱ����ı���
************************************************************/
//#define  C_CMD_OK			   0
//#define  C_CMD_CHECKFALSE	   1
//#define  C_CMD_TIMEOUT	   2
#define  C_CMD_WRITE	0		//��PC����ͨѶ����
#define  C_CMD_READ		1
#define  C_CMD_DEBUG	2
#define  C_COM_OVER_TIME	   100      //��PC��ͨѶ��ʱʱ�� 2ms��λ


/************************************************************
	�����������������
************************************************************/
#define  GetTime() 	(CpuTimer1.RegsAddr->TIM.all)

/************************************************************
	��������
************************************************************/
//�������
#define MOTOR_TYPE_IM			0		//��Ӧ���
#define MOTOR_TYPE_VARFREQ      1       //��Ƶ�첽���
#define MOTOR_TYPE_PM			2		//����ͬ�����
#define MOTOR_NONE              100
//���Ʒ�ʽ
#define IDC_SVC_CTL				0		//SVC
#define IDC_FVC_CTL				1		//FVC
#define IDC_VF_CTL				2		//VF
//��Ƶ����ѹ�ȼ�
#define INV_VOLTAGE_220V        0   
#define INV_VOLTAGE_380V        1
#define INV_VOLTAGE_480V        2
#define INV_VOLTAGE_690V        3
#define INV_VOLTAGE_1140V       5
//��Ƶ����״̬
#define STATUS_LOW_POWER		1		//Ƿѹ״̬
#define STATUS_GET_PAR			2		//��̬������ʶ�׶�
#define STATUS_STOP				3		//ͣ��״̬
#define STATUS_SPEED_CHECK		4		//ת�ٸ��ٽ׶�
#define STATUS_RUN				5		//����״̬, (����ֱ���ƶ��׶�)
#define STATUS_SHORT_GND		6		//�Եض�·���׶�
#define STATUS_IPM_INIT_POS		7		//ͬ����ʶ��ż���ʼλ�ýǽ׶�

//�Ӽ��ٱ�־
#define C_SPEED_FLAG_CON 		0		//���ٱ�־
#define C_SPEED_FLAG_ACC 		1		//���ٱ�־
#define C_SPEED_FLAG_DEC 		2		//���ٱ�־
// ������
#define speed_ACC (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_ACC)
#define speed_DEC (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_DEC)
#define speed_CON (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_CON)

//����/��ɢ����
#define  MODLE_CPWM				0		//��������
#define  MODLE_DPWM				1		//��ɢ����
//ͬ������/�첽����
#define  MODLE_SYN				0		//ͬ������
#define  MODLE_ASYN				1		//�첽����

#define  CANCEL_DB_COMP_NO      0       // ������
#define  DEADBAND_COMP_280      1       //��ǰ280���õ�����������ʽ, AD�ж��м���
#define  DEADBAND_COMP_380      2      //�Ż���380����������ʽ


#define  IDENTIFY_PROGRESS_LENGTH 5     //��ǰ������ʶ���4��
//���ز�Ƶ�����ʱ�䶨��    
#ifdef	DSP_CLOCK100
	#define C_INIT_PRD			10000	//��ʼ(5KHz)��PWM���ڣ�PWM��ʱ��ʱ��Ϊ10ns��
	#define C_MAX_DB			320		//��ʼ������С3.2us
	#define SHORT_GND_PERIOD 	12500	//�ϵ�Եض�·���ʱ����ز�����
    #define TUNE_Rs_PRD         25000  //380V��ѹ�ǵȼ����ӵ��������ʶ��Ƶ2K
    #define SHORT_GND_CMPR_INC	100     // 1us��Ӧ������ֵ
	#define SHORT_GND_PERIOD_1140 	50000
    #define SHORT_GND_PERIOD_690    33000
#else
	#define C_INIT_PRD			6000	//��ʼ(5KHz)��PWM���ڣ�PWM��ʱ��ʱ��Ϊ16.7ns��
	#define C_MAX_DB			192		//��ʼ������С3.2us
	#define SHORT_GND_PERIOD 	7500
    #define TUNE_Rs_PRD         15000  //380V��ѹ�ǵȼ����ӵ��������ʶ��Ƶ2K
	#define SHORT_GND_CMPR_INC	60
    #define SHORT_GND_PERIOD_1140 	30000
    #define SHORT_GND_PERIOD_690    20000
#endif
#define C_MAX_LONG_VALUE		(0x7FFFFFFF)	//���������ݵ����ֵ
#define C_MAX_PER               (1073741824L)   //Q30

#define  COM_PHASE_DEADTIME		600	   //���������жϵĳ�ǰ�Ƕ�

/************************************************************
	 ����DSP��ADC����ڡ�GPIO������ڡ�GPIO�����
************************************************************/
#define TurnOnFan()    		gMainStatus.StatusWord.bit.FanControl = 1						  
#define TurnOffFan()     	    gMainStatus.StatusWord.bit.FanControl = 0	//���ȿ���

#define TurnOnBrake()    		GpioDataRegs.GPACLEAR.bit.GPIO8 = 1					  
#define TurnOffBrake()     	GpioDataRegs.GPASET.bit.GPIO8 = 1	        //�ƶ��������


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
