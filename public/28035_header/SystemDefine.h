/************************************************************
 28035�汾����ϵͳ��ʹ�õĺ�ͳ���
            
************************************************************/
#ifndef MAIN_DEFINE_28035_H
#define MAIN_DEFINE_28035_H

#ifdef __cplusplus
extern "C" {
#endif
#include "DSP2803x_Device.h"



/************************************************************
*************************************************************
	28035�汾����������汾�ŵ�ά��
***********************************************************/
#define SOFT_VERSION		72		// 28035ver
// 0.02 update on xxxx
// 0.04 update on 2010��4��26 (�Բ�ڶ��汾)
// 0.06 update on 2010��5��17 (380���������ύ���԰汾)
// 0.08 update on 2010, 6, 21 (380������һ�ֲ������޸ĺ�İ汾)
// 0.10 update on 2010, 6, 30 (��һ����֤����2)
// 0.16 update on 2010��8��25 (����֮��)
// 0.18 sizeD��֤

// 0.20 update 2010��11��15 ����SVC �Ż�����
// 0.24 update 2010��12��22 �޸�VF������
                         //�޸��˲�������

// �Ǳ��¼
// 60001�Ǳ꣬��55kw/220V ���ƻ��Ͷ�Ӧ��132kw/380V (ͨ����110kw/380V)
// 60002�Ǳ꣬��55kw/220V ���ƻ��Ͷ�Ӧ��132kw/380V (ͨ����110kw/380V)
// 60003�Ǳ꣬��3.7kw/220V���ƻ��Ͷ�Ӧ��7.5kw/380V (ͨ����5.5kw/380V) 2010/11/11
        

/************************************************************
************************************************************/

// // ����DSPоƬ��ʱ��
//#define    TMS320F2801		1		                            //2801оƬ
//#define    TMS320F2802		2		                            //2802оƬ
//#define      TMS320F2808		8		                            //2808оƬ
#define      TMS320F28035   3                               //28035оƬ

//#define    	 DSP_CLOCK100			100		                        //100MHzʱ��
//#define      DSP_CLOCK80        80                                  // 80MHz
#define    DSP_CLOCK60			60		                     //60MHzʱ�� 

#ifdef		 DSP_CLOCK100
	#define    	DSP_CLOCK			100		                //100MHzʱ��
	#define		PWM_CLK_DIV			0
	#define     PWM_CLOCK           100                     //PWMģ��ʱ������ 
	#define     DBTIME_1140V         50                      //1140V����ʱ��
    #define     DCTIME_1140V         14                      //1140V��������
#endif
#ifdef      DSP_CLOCK80
	#define    	DSP_CLOCK			80		                //80MHzʱ��
	#define		PWM_CLK_DIV			0
	#define     PWM_CLOCK           80                      //PWMģ��ʱ������   
#endif
#ifdef      DSP_CLOCK60
	#define    	DSP_CLOCK			60		                //60MHzʱ��
	#define		PWM_CLK_DIV			0
	#define     PWM_CLOCK           60                      //PWMģ��ʱ������   
    #define     DBTIME_1140V         30                     //1140V����ʱ��
    #define     DCTIME_1140V         8                     //1140V��������
#endif

#define     PWM_CLOCK_HALF      (PWM_CLOCK/2)               //PWMʱ�ӳ���2
#define  	C_TIME_05MS         (DSP_CLOCK*500L)	        //0.5ms��Ӧ�Ķ�ʱ��1����ֵ
#define  	C_TIME_20MS         (DSP_CLOCK*2000L)
#define     C_TIME_045MS        (DSP_CLOCK*450L)            //0.45ms��Ӧ�Ķ�ʱ��1����ֵ

#define  READ_RAND_FLASH_WAITE		3		                //Flash�����еĵȴ�ʱ��
#define  READ_PAGE_FLASH_WAITE		3		                //
#define  READ_OTP_WAITE				8		                //OTP�ж����ݵĵȴ�ʱ��

/************************************************************
��оƬ��صļĴ�������
************************************************************/
#define ADC_GND              (AdcResult.ADCRESULT0<<4)          //28035��������Ҷ��룬Ϊ�˸�2808���ݣ�����4λ
#define	ADC_IU				 (AdcResult.ADCRESULT1<<4)
#define ADC_IW               (AdcResult.ADCRESULT2<<4)
#define	ADC_UDC				 (AdcResult.ADCRESULT3<<4)

#define	ADC_TEMP			 (AdcResult.ADCRESULT4<<4)
#define	ADC_IV               (AdcResult.ADCRESULT5<<4)          // ����ط�������Ӳ�������嵽DSP��ʱ�ӿڷ�����
#define ADC_AI1              (AdcResult.ADCRESULT6<<4)
#define ADC_VREFLO           (AdcResult.ADCRESULT6<<4)          //ѡ��ADCINB5��ο������ӣ�������Ʈ
#define	ADC_AI2				 (AdcResult.ADCRESULT7<<4)

#define ADC_AI3              (AdcResult.ADCRESULT8<<4)
#define	ADC_UU4              (AdcResult.ADCRESULT9<<4)          //PG-SinCos/UVW���ö���
#define ADC_UU5              (AdcResult.ADCRESULT10<<4)         //PG-SinCos/UVW���ö���
#define ADC_UU6              (AdcResult.ADCRESULT11<<4)         //PG-SinCos/UVW���ö���
#define ADC_UU7              (AdcResult.ADCRESULT12<<4)         //PG-SinCos�ö���
#define	PL_VOE_PROTECT		 (AdcResult.ADCRESULT13<<4)

#define UVW_PG_U                (AdcResult.ADCRESULT9 <<4)         //PG-SinCos/UVW���ö���
#define UVW_PG_V                (AdcResult.ADCRESULT10<<4)         //PG-SinCos/UVW���ö���
#define UVW_PG_W                (AdcResult.ADCRESULT11<<4)         //PG-SinCos/UVW���ö���

// // ��оƬ��صļĴ�������
#define PIE_VECTTABLE_ADCINT    PieVectTable.ADCINT1                    //ADC�ж�����
#define ADC_CLEAR_INT_FLAG      AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1    //���ADCģ����жϱ�־
#define ADC_RESET_SEQUENCE      AdcRegs.SOCPRICTL.bit.RRPOINTER = 0x20  //reset the sequence
#define ADC_START_CONVERSION   	AdcRegs.ADCSOCFRC1.all = 0xFFFF 		//�������AD
#define ADC_END_CONVERSIN       AdcRegs.ADCINTFLG.bit.ADCINT1           //ADת����ɱ�־λ

#define  ADC_VOLTAGE_08         ( 8L*65535/33)  // AD����0.8V��Ӧ�Ĳ���ֵ   28035AD��Χ��0-3.3V
#define  ADC_VOLTAGE_10         (10L*65535/33)  // AD����1.0V��Ӧ�Ĳ���ֵ
#define  ADC_VOLTAGE_20         (20L*65535/33)  // AD����2.0V��Ӧ�Ĳ���ֵ
#define  ADC_VOLTAGE_25         (25L*65535/33)  // AD����2.5V��Ӧ�Ĳ���ֵ

#define PL_INPUT_HIGH           (PL_VOE_PROTECT < ADC_VOLTAGE_08)       //����ȱ���źŵ�ƽ�ж�

#define DisConnectRelay()   	GpioDataRegs.GPASET.bit.GPIO11   = 1
#define ConnectRelay()    	    GpioDataRegs.GPACLEAR.bit.GPIO11 = 1    //�ϵ绺��̵�������

#define EnableDrive()    	    GpioDataRegs.GPACLEAR.bit.GPIO7  = 1						  
#define DisableDrive()     	GpioDataRegs.GPASET.bit.GPIO7  = 1	    //PWM����������
#define GetOverUdcFlag()      GpioDataRegs.AIODAT.bit.AIO2 == 0       // 28035ͨ��io����Ӳ����ѹ

// //��ת��ѹ���õ��ĺ궨��
#define RT_SAMPLE_START         (GpioDataRegs.AIOCLEAR.bit.AIO10 = 1)      // �����źſ�ʼ����
#define RT_SAMPLE_END           (GpioDataRegs.AIOSET.bit.AIO10 = 1)        // �����źŲ������

#define ROTOR_TRANS_RDVEL	    GpioDataRegs.GPADAT.bit.GPIO20
#define ROTOR_TRANS_SO		    GpioDataRegs.GPADAT.bit.GPIO21
#define ROTOR_TRANS_RD	        GpioDataRegs.GPADAT.bit.GPIO23
#define ROTOR_TRANS_SCLK	    GpioDataRegs.AIODAT.bit.AIO12

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

