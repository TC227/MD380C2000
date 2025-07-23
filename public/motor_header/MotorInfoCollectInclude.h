/***************************************************************
�ļ����ܣ�ģ��������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTORINFO_COLLECT_INCLUDE_H
#define  MOTORINFO_COLLECT_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"

// // �ṹ�嶨�� 
typedef struct CUR_EXCURSION_STRUCT_DEF{
	long	TotalIu;
	long	TotalIv;
	int		Iu;                             // ȥ��Ưǰ
	int		Iv;
    long    IuValue;                        //���ڲ�����ʶ
    long    IvValue;                        //���ڲ�����ʶ
	int		ErrIu;				            //U����Ư��С
	int		ErrIv;				            //V����Ư��С
	int  	Count;
	int  	EnableCount;
	int  	ErrCnt;
}CUR_EXCURSION_STRUCT;                  //�����Ưʹ�õĽṹ

typedef struct TEMPLETURE_STRUCT_DEF{
	Uint	TempAD;				            //AD��ȡֵ�������¶Ȳ��
	Uint	Temp;				            //�öȱ�ʾ��ʵ���¶�ֵ
	Uint	TempBak;			            //�öȱ�ʾ��ʵ���¶�ֵ
	Uint	ErrCnt;
}TEMPLETURE_STRUCT;                     //�ͱ�Ƶ���¶���ص����ݽṹ

typedef struct AI_STRUCT_DEF {
	Uint 	gAI1;
	Uint 	gAI2;
    Uint    gAI3;

    Ulong   ai1Total;
    Ulong   ai2Total;
    Ulong   ai3Total;
    int     aiCounter;
}AI_STRUCT;	//

// // ���ⲿ���ñ������� 
extern I_STRUCT				    ISamp;
extern ADC_STRUCT				gADC;		//ADC���ݲɼ��ṹ
extern UDC_STRUCT				gUDC;		//ĸ�ߵ�ѹ����
extern IUVW_SAMPLING_STRUCT	    gCurSamp;
extern UVW_STRUCT				gIUVWQ12;	//�����������������
extern UVW_STRUCT_Q24           gIUVWQ24;   //Q24��ʽ�����ඨ�ӵ���
extern ALPHABETA_STRUCT		    gIAlphBeta;	//�����������������
extern ALPHABETA_STRUCT		    gIAlphBetaQ12;	//�����������������
extern MT_STRUCT				gIMTQ12;    //MT��ϵ�µĵ���
extern MT_STRUCT                gIMTSetQ12;
extern MT_STRUCT_Q24            gIMTQ24;
extern AMPTHETA_STRUCT			gIAmpTheta;	//�������ʾ�ĵ���
extern LINE_CURRENT_STRUCT		gLineCur;	
extern CUR_EXCURSION_STRUCT	    gExcursionInfo;//�����Ưʹ�õĽṹ
extern TEMPLETURE_STRUCT		gTemperature;
extern AI_STRUCT				gAI;

extern PID_STRUCT               gWspid;

// // ���ⲿ���ú������� 
void GetCurExcursion(void);
void GetUDCInfo(void);
void ADCProcess(void);
void GetCurrentInfo(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


