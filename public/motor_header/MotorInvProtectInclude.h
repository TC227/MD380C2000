/***************************************************************
�ļ����ܣ��͵��,��Ƶ��������ص����ݽṹ���壬��������
�ļ��汾��
���¸��£�
************************************************************/
#ifndef MOTORINV_PROTECT_INCLUDE_H
#define MOTORINV_PROTECT_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"

/******************���ϴ��붨��*************************///
//���ϴ���
#define  ERROR_OVER_CURRENT         (1L<< (1-1))      // 1 ����-2
#define  ERROR_OVER_UDC             (1L<< (2-1))      // 2 ��ѹ-5
#define  ERROR_RESISTER_HOT         (1L<< (3-1))      // 3 ����������-8
#define  ERROR_LOW_UDC              (1L<< (4-1))      // 4 Ƿѹ-9
#define  ERROR_INV_OVER_LAOD        (1L<< (5-1))      // 5 ��Ƶ������-10

#define  ERROR_MOTOR_OVER_LOAD      (1L<< (6-1))      // 6 �������-11
#define  ERROR_INPUT_LACK_PHASE     (1L<< (7-1))      // 7 ����ȱ��-12
#define  ERROR_OUTPUT_LACK_PHASE    (1L<< (8-1))      // 8 ���ȱ��-13
#define  ERROR_INV_TEMPERTURE       (1L<< (9-1))      // 9 ��Ƶ������-14
#define  ERROR_RESISTANCE_CONTACK   (1L<< (10-1))     // 10 ����������Ϲ���-17

#define  ERROR_CURRENT_CHECK        (1L<< (11-1))     // 11 ������Ʈ������-18
#define  ERROR_SHORT_EARTH          (1L<< (12-1))     // 12 �Եض�·������-23
#define  ERROR_LOAD_LOST            (1L<< (13-1))     // 13 ������أ�30
#define  ERROR_TRIP_ZONE            (1L<< (14-1))     // 14 ����������-40
#define  ERROR_INIT_POS			    (1L<< (15-1)) 	  // 15 ��ʼλ�ò��ǺϹ���-92
                                                      //    ͬ�����ż�λ���ж�ʧ��
#define  ERROR_SPEED_LOSE           (1L<< (16-1))     // 16 ͬ�������������λ�ýǱ�ʶʧ��94(ȡ��)
#define  ERROR_PROGRAM_LOGIC        (1L<< (17-1))     // 17 �����ڲ��߼���λ-99
#define  ERROR_TUNE_FAIL            (1L<< (18-1))     // 18 ��гʧ�� - ??(19)

#define  ERROR_ENCODER              (1L<< (19-1))     // 19 �źű���������

/*
ERROR_ENCODER ������Ϣ˵�� 
infor = :
1	// �����ر�ʶ�� ab�źŷ����ʶ����
2	// �����ر�ʶ�� uvw�źŷ����ʶ����
3	// �����ر�ʶ�� �����źŷ����ʶ����
4	// �����ر�ʶ�� û����⵽Z�źţ����������λ�ã���

5	// �����ر�ʶ�� ab�źŷ����ʶ����
6	// �����ر�ʶ�� uvw�źŷ����ʶ����
7	// �����ر�ʶ�� �����źŷ����ʶ����

9	�첽����ʶ������δ�ӣ�
10	�첽����ʶ��������������

11      uvw�źŷ����ߣ�
12	uvw�߼����󣨱���������pg���𻵣���
*/

/************************************************************/
/***********************�ṹ�嶨��***************************/
struct FAULT_INFOR_MODE_STRUCT_DEF{
    Uint    Fault1:4;                       //������Ϣֻ���趨Ϊ0��15֮�����
    Uint    Fault2:4;
    Uint    Fault3:4;
    Uint    Fault4:4;
}; //������ֹ��ϵ�ͣ������ʽ
typedef union FAULT_INFOR_MODE_UNION_DEF{
    Uint    all;
    struct  FAULT_INFOR_MODE_STRUCT_DEF bit;
}FAULT_STOP_MODE_UNION;
struct INV_ERROR_CODE_STRUCT{
    Uint    ErrorCode1;
    Uint    ErrorCode2;
};
union FAULT_CODE_UNION_DEF{
    Uint32  all;
    struct INV_ERROR_CODE_STRUCT  ErrorCodeStruct;
};
struct FAULT_CODE_INFOR_STRUCT_DEF{
    union FAULT_CODE_UNION_DEF ErrorCode;       //���ϴ���.������Ϣ��ŷ�ʽ�μ����϶��壬��bitλ���
    FAULT_STOP_MODE_UNION       ErrorInfo[5];    //������Ϣ.���ڰ�����λ����λ��.ÿһ���ϵĹ�����Ϣ��4��bitλ���.���˳��ͬ
                                                 //���ϴ����еĹ�������˳��һ�¡�
	Uint32                      LastErrorCode;   //�ϴι��ϴ��룬���������ѹ��λ�л�    
};
typedef struct OVER_LOAD_PROTECT_DEF{
   int		Cnt;
   long		FilterRealFreq;
   int		FilterInvCur;
   int		FilterMotorCur;
   BIT32_GROUP		InvTotal;
   BIT32_GROUP		MotorTotal;
}OVER_LOAD_PROTECT; 	//���ر������ݽṹ
typedef struct PHASE_LOSE_STRUCT_DEF{
   Ulong	Time;
   Ulong	TotalU;
   Ulong	TotalV;
   Ulong	TotalW;
   Uint		Cnt;
   Uint     errMaxCur;
   Uint     errMinCur;
}PHASE_LOSE_STRUCT; 	//���ȱ���жϳ���
typedef struct INPUT_LOSE_STRUCT_DEF{
   Uint		Cnt;
   Uint		UpCnt;          // PL�ߵ�ƽ������
   Uint		ErrCnt;
   Uint		CntRes;         // �䵱ʱ�������
   Uint		UpCntRes;
   Uint     ForeInvType;    //��ʼȱ�ౣ���ĳ�ʼ����
}INPUT_LOSE_STRUCT; 	//����ȱ���жϳ���
typedef struct LOAD_LOSE_STRUCT_DEF {
	Uint 	ErrCnt;
    Uint    ChkLevel;       // ���ؼ��ˮƽ
    Uint    ChkTime;        // ���ؼ��ʱ��
}LOAD_LOSE_STRUCT;	//������ؼ�����ݽṹ
typedef struct FAN_CTRL_STRUCT_DEF{
   Uint		EnableCnt;	//�ж��ϵ������1����ܹ���ʼ����
   Uint		RunCnt;		//�ж�����ʱ������10��
}FAN_CTRL_STRUCT; 	//���ȿ��Ƴ���ʹ�ýṹ����
struct CBC_FLAG_STRUCT{
    Uint16  CBC_U:1;
    Uint16  CBC_V:1;
    Uint16  CBC_W:1;
    Uint16  RESV:13;
};
typedef union CBC_FLAG_UNION_DEF {
   Uint16                  all;
   struct CBC_FLAG_STRUCT  bit;
}CBC_FLAG_UNION;
typedef struct CBC_PROTECT_STRUCT_DEF{
   CBC_FLAG_UNION	Flag;	//BIT0/1/2Ϊ1�ֱ��ʾUVW���ദ��������״̬
   int		TotalU;
   int		TotalV;
   int     	TotalW;
   int     	Total;
   int		EnableFlag;
   int		CntU;		//��������1.6����ֵ��������ۼ�
   int		CntV;
   int     	CntW;

   Uint     maxCBCTime;     // 60deg�¶ȶ�Ӧ��CBCʱ��(����)
   Uint     minCBCTime;     // 40deg�¶ȶ�Ӧ��CBCʱ��(����)
}CBC_PROTECT_STRUCT; 	//�������������ݽṹ

/************************************************************/
/*******************���ⲿ���ñ�������***********************/
extern OVER_LOAD_PROTECT		gOverLoad;
extern PHASE_LOSE_STRUCT		gPhaseLose;
extern INPUT_LOSE_STRUCT		gInLose;
extern LOAD_LOSE_STRUCT		    gLoadLose;
extern FAN_CTRL_STRUCT			gFanCtrl;
extern Ulong					gBuffResCnt;	//������豣������
extern CBC_PROTECT_STRUCT		gCBCProtect;
/************************************************************/
/*******************���ⲿ���ú�������***********************/
void InvDeviceControl(void);
void OutputLoseAdd(void);
extern struct FAULT_CODE_INFOR_STRUCT_DEF  gError;
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================



