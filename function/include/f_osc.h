/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : sci_osc.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 05/18/2010
* Description        : MD380ʾ����SCI��̨���ģ������ļ�
***************************************************************************************************
* �޸�				��
* �汾				��V1.00
* ʱ��				��
* ˵��				���޸���������У�飬�޸ķ���˫�������ģʽ��ֹ�������
***************************************************************************************************
* �޸�				��
* �汾				��V1.10
* ʱ��				��
* ˵��				����������SCI��������ֹ�����ʲ���ȷ���SCI�������״̬
*					  �޸��������ʼ����������19200���������ô���
**************************************************************************************************/

#ifndef	 _sci_osc_h_
#define	 _sci_osc_h_

#include "f_funcCode.h"
// �ڲ��궨��
// ����������
#define			OSC_SCI_SEL						1			// "1"ѡ��SCIA	��2��ѡ��SCIB
#define			OSC_CON_CHECK					1			// ����֡У��ʹ��
#define			OSC_DATA_CHECK					1			// ����֡У��ʹ�ܣ���1��ʹ�ü���żУ�飬��0���ر�
#define			FC_CODE_CONTROL_EN				1			// ��̨���ƹ��ܽ��ܣ���1��ʹ�ܣ���0���ر�
#define			OSC_TX_INT_EN					1			// OSC�����ж�ʹ�� "1"ʹ���ж�
#define			SCI_INT_LOAD_RAM				0			// SCI�жϼ��ص�RAM������

#if DSP_2803X
#define			PERIPHERAL_CLK					15E6		// ����ʱ��
#else
#define			PERIPHERAL_CLK					25E6		// ����ʱ��
#endif
// �����жϼ���ʹ�ܣ����������ݳ�����4����������ʹ�ܣ��������ʹ�ܽ������
#define			SCI_TX_INT_SPEEDUP				1			// ʹ�ܼ��� 

#if DSP_2803X
    #define			RS485_ENABLE			    0			// 28035������ʹ��485����
#else
    #define			RS485_ENABLE			    1			// ʹ��485����
#endif


// RS485�Ľ��շ����л�

#if RS485_ENABLE == 1
// RS485�Ľ��շ����л�
    #if DSP_2803X
    #define RS485_RTS_O (GpioDataRegs.GPBDAT.bit.GPIO39)
    #else
    #define RS485_RTS_O (GpioDataRegs.GPADAT.bit.GPIO27)
    #endif
    #define RS485_R_O     0
	#define RS485_T_O     1

#endif

// ���²���һ�㲻���޸�
/*********************************************************************************************************************/
#if 	(1 == OSC_SCI_SEL)
	#define			SCI_OSC_REGS					SciaRegs// ѡ��SCIA�ӿ�
#else
	#define			SCI_OSC_REGS					ScibRegs// ѡ��SCIB
#endif

#define			OSC_CON_FRAME_HEAD				0xAC		// ����֡֡ͷ
#define			OSC_DATA_FRAME_HEAD_A			0xA5		// ����֡֡ͷ
#define			OSC_DATA_FRAME_HEAD_B			0xCD		// 
		

// �ڲ�����������
#define			OSC_BUF_DATA_LEN				64			// ʾ�������ͻ������ݳ���
#define			FC_FRAME_LEN					8			// ����֡����
// 
#define			OSC_SCI_EN						0xCA		// ʾ����ģ��ʹ�ܣ�����ֵ�ر�

// ����״̬
#define			SCI_RT_BUSY						0x03		// SCIæ
#define			SCI_RS485_TX_BUSY				0x04		// SCIʹ��485�ӿڷ���æ	


// ʾ�������ڿ�������
#define         FC_COMM_TEST                    0x30        // ͨѶ��������
#define			FC_CHANNL_SEL					0x31		// ͨ��ѡ������
#define			FC_PARA_CFG						0x32		// ��������
//#define			FC_AQU_SPEED					0x32		// �����ٶ�
//#define			FC_BAUD_CFG						0x33		// �������޸�
//#define			FC_RUN_CONTINUE					0x34		// ��������
#define			FC_START_OSC					0x33		// ����
#define			FC_STOP_OSC						0x34		// ֹͣ
#define			FC_OSC_OFF						0x35		// �ر�ʾ����ģʽ	

// ���ܷ���
#define			FC_READ_FC_DATA					0x41		// ���ڲ����������
#define			FC_WRITE_FC_DATA				0x42		// д�ڲ����������

// ��̨���ƽṹ
#define			CONTROL_FRAME_HEAD				OscConFrameBuf[0]
#define			CONTROL_FRAME_FC				OscConFrameBuf[1]
#define			CONTROL_FRAME_P1				OscConFrameBuf[2]
#define			CONTROL_FRAME_P2				OscConFrameBuf[3]
#define			CONTROL_FRAME_P3				OscConFrameBuf[4]
#define			CONTROL_FRAME_P4				OscConFrameBuf[5]
#define			CONTROL_FRAME_CRCL				OscConFrameBuf[6]
#define			CONTROL_FRAME_CRCH				OscConFrameBuf[7]


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	�޷���8λ���ͱ���  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		�з���8λ���ͱ���  */
typedef unsigned int   uint16;                  /* defined for unsigned 16-bits integer variable 	�޷���16λ���ͱ��� */
//typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		�з���16λ���ͱ��� */
typedef unsigned  long uint32;                  /* defined for unsigned 32-bits integer variable 	�޷���32λ���ͱ��� */
//typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		�з���32λ���ͱ��� */
typedef float          fp32;                    /* single precision floating point variable (32bits) �����ȸ�������32λ���ȣ� */
typedef double         fp64;                    /* double precision floating point variable (64bits) ˫���ȸ�������64λ���ȣ� */


// ��ȡʾ�������ݺ궨��



// ʾ������̨ģ����ƿ�����
typedef struct
{
	uint8	status;            								// ����״̬(0-ֹͣ����  1-��ʼ����) 
    uint8	baudRate;										// ������ѡ�� 0��115'200	1��57'600	2: 19'200 
    uint8 	interval;          								// �������(*0.5ms) 	1~8								
    uint8	runContinue;           							// ��������(1-ͣ������  0-ͣ��������)
    uint8	ch1Addr;           								// ͨ��1��ַ(��������1��ַ), д�롰0xFF�����ø�ͨ��
    uint8	ch2Addr;           								// ͨ��2��ַ(��������2��ַ)
    uint8	ch3Addr;           								// ͨ��3��ַ(��������3��ַ)
    uint8	ch4Addr;           								// ͨ��4��ַ(��������4��ַ)   
	uint8   chSum;											// ͨ������
} DSP_OSC_CON_DATA;

// ʾ�����������ݽṹ
typedef	struct
{
	uint8	frameHead1;										// ����֡ͷ
	uint8 	frameHead2;
	uint8 	frameNum;										// ֡��
	uint8	oscDataBuf[OSC_BUF_DATA_LEN];					// ���ݻ�����
	uint8	check;											// У��
	uint8	rwPI;											// ���ݶ�д����
	uint8	full;											// ����־					
} OSC_DATA_TYPE;

// �����жϴ�����ƽṹ
typedef struct
{
	uint8 *buf;												// ������ָ��
	uint8 len;												// �ա�������
	uint8 busy;												// ����æ�źţ�"1" ������		"0"����״̬ 
	uint8 rxflag;											// ���ձ�־���յ�����rxflag�á�1��
//	uint8 err;												// ���ڳ���
} SCI_RT_CON_DATA;



// �ⲿ���ú�����ȫ�ֱ�������
extern void OscSciFunction(void);


#endif														// end sic_osc.h




