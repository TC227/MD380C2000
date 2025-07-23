/***********************************Inovance***********************************
����������Function Description��:
����޸����ڣ�Date����
�޸���־��History��:�����¼�¼Ϊ��һ��ת���Ժ󣬿�ʼ��¼��
	���� 		ʱ�� 		����˵��
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance
***********************************/
#ifndef ZERO_POS_CTL_INCLUDE_H
#define ZERO_POS_CTL_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes 
------------------------------------------------------------------*/
#include "DataTypeDef.h"

/* Private typedef 
-----------------------------------------------------------*/
typedef struct ZERO_POS_STRUCT_DEF {
    u16     Flag;               /*Flag = 1 ��ʾ���ŷ�����*/

	s16  	PosInit;            /*���ŷ����λ��*/
	s16  	PosLast;            /*���ŷ�ʱ���ƶ���λ��,��һ��*/
	s16  	Pos;                /*���ŷ�ʱ���ƶ���λ��*/

    s16     KPPos;              /*���ŷ���λ��KP*/
    s16     KPSpeed;            /*���ŷ����ٶ�KP*/
	s16  	KPDecCoff;          /*���ŷ�KP�������ļ�С����*/

	s16  	ItSet;              /*���ŷ�����ʱ����������*/
	s16  	ItKpPos;            /*λ�õ��ڲ����ṩ������*/
	s16  	ItKpSpeed;          /*�ٶȿ��Ʋ����ṩ������*/

    s16     Speed;              /*������ٶ�*/
    u32     TimeLast;           /*��һ�ĵ�ʱ��*/
    u32     DetaTime;           /*ÿ�����ʱ��*/
    
}ZERO_POS_STRUCT;	//���ŷ����ƽṹ


/* Private define 
------------------------------------------------------------*/
//ȡ����ֵ�ĺ���(����������������)
#define abs(x)  	((x>0)?(x):-(x))
//ȡ��ֵ����(����������������)
#define Max(x,y)	((x>y)?(x):(y))
//ȡСֵ����(����������������)
#define Min(x,y)	((x<y)?(x):(y))

/* Private macro 
-------------------------------------------------------------*/


/* Private function prototypes 
-----------------------------------------------*/
extern ZERO_POS_STRUCT          gZeroPos;

extern s16 Filter(s16 LastOne, s16 Input, s16 Coff);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/******************************* END OF FILE
***********************************/


