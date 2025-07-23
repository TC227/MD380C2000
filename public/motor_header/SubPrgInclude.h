/************************************************************
------------���ļ���ͨ���ӳ���ģ������ͷ�ļ�---------------
************************************************************/
#ifndef SUBPRG_INCLUDE_H
#define SUBPRG_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"

//#define FilterN(x,y) (y + (x-y)/N)   һ���˲������� �о���
#define	Filter1(x,total)    (x)
#define	Filter2(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<15) - (((Ulong)total)<<15) )>>16)
#define	Filter4(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<14) - (((Ulong)total)<<14) )>>16)
#define	Filter8(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<13) - (((Ulong)total)<<13) )>>16)
#define	Filter16(x,total)  (( (((Ulong)total)<<16) + (((Ulong)x)<<12) - (((Ulong)total)<<12) )>>16)
#define	Filter32(x,total)  (( (((Ulong)total)<<16) + (((Ulong)x)<<11) - (((Ulong)total)<<11) )>>16)
#define	Filter64(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<10) - (((Ulong)total)<<10) )>>16)
#define Filter128(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<9 ) - (((Ulong)total)<<9 ) )>>16)
#define Filter256(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<8 ) - (((Ulong)total)<<8 ) )>>16)

/************************************************************
	�ṹ����
************************************************************/
typedef struct PID_STRUCT_DEF {
	long 	Total;			//�����ۼ�ֵ
	long 	Out;			//���ֵ
	int  	Max;			//���ֵ����
	int  	Min;			//��Сֵ����
	int  	Deta;			//ƫ��ֵ
	int  	KP;				//KP����
	int  	KI;				//KI����
	int  	KD;				//KD����

    Uint    QP;             // KP�ķŴ��� KP = KP << QP
    Uint    QI;             // KI�ķŴ���
    Uint    QD;             // KD�ķŴ���
}PID_STRUCT;//PID�����õ����ݽṹ(���������ݽṹ)
typedef struct PID_STRUCT_LONG_DEF {
	llong 	Total;			//�����ۼ�ֵ
	llong 	Out;			//���ֵ
	long  	Max;			//���ֵ����
	long  	Min;			//��Сֵ����
	long  	Deta;			//ƫ��ֵ
	Uint  	KP;				//KP����
	Uint  	KI;				//KI����
	Uint  	KD;				//KD����

    int     sVoltGain;
}PID_STRUCT_LONG;//PID�����õ����ݽṹ(���������ݽṹ)

typedef struct PID32_STRUCT_DEF{
	long  	Deta;			//ƫ��ֵ(Q24.7,���Ϊ2.0)
	long 	Out;			//���ֵ(Q24.7)
	long 	Total;			//�����ۼ�ֵ(Q24.7)
	long  	Max;			//���ֵ����(Q24.7,��Ҫ����64)
	long  	Min;			//��Сֵ����(Q24.7,��Ҫ����-64)
	long  	KP;				//KP����(Q12.19) �൱����Q12��ʽ
	long  	KI;				//KI����(Q16.15) �൱����Q16��ʽ
}PID32_STRUCT;

typedef struct BURR_FILTER_STRUCT_DEF{					
	long 	Input;			//���β�������
	long	Output;			//��ǰʹ������
	long 	Err;			//ƫ����ֵ
	long  	Max;			//���ƫ��ֵ
	long  	Min;			//��Сƫ��ֵ
}BURR_FILTER_STRUCT;//ȥ��ë���˲���������ݽṹ

typedef struct CUR_LINE_STRUCT_DEF{					
	int		FilterTime;		//�˲�����
	int 	Input;			//������������
	int		Output;			//����Ԥ������
	int		Data[16];		//������ʷ���ݣ����8ms���ݣ�
}CUR_LINE_STRUCT_DEF;//���������ص����ݽṹ

/************************************************************
	�����������������
************************************************************/
extern Uint swap(Uint);				//
extern int 	abs(int);				//
extern int 	absl(long int);			//r ���������������
extern int 	qsin(int);				//���Һ���
extern int 	qatan(long int);		//�����к���
extern int  atan(int x, int y);		//�����޷����к���
extern Uint qsqrt(Ulong);			//��������

extern Uint GetInvCurrent(Uint);	//���ݻ��Ͳ�ѯ��Ƶ�����������
extern void PID(PID_STRUCT * pid);
extern void BurrFilter(BURR_FILTER_STRUCT * );
extern int Filter(int , int , int );

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
