/****************************************************************
�ļ����ܣ��͵��������صĹ�����������
�ļ��汾�� 
���¸��£� 

****************************************************************/

#include "MotorInclude.h"

// ����Ϊ������������
INV_STRUCT 				    gInvInfo;		//��Ƶ����Ϣ
MOTOR_STRUCT 			    gMotorInfo;	    //�����Ϣ
MOTOR_EXTERN_STRUCT		    gMotorExtInfo;	//�����չ��Ϣ��ʵ��ֵ��ʾ��
MOTOR_EXTERN_STRUCT		    gMotorExtPer;	//�����չ��Ϣ����ôֵ��ʾ��
RUN_STATUS_STRUCT 		    gMainStatus;	//������״̬
BASE_COMMAND_STRUCT		    gMainCmd;		//������
MAIN_COMMAND_EXTEND_UNION   gExtendCmd;     //����������չ
SUB_COMMAND_UNION           gSubCommand;	//�������ֽṹ

CONTROL_MOTOR_TYPE_ENUM     gCtrMotorType;  //������ͺͿ���ģʽ�����
MOTOR_POWER_TORQUE          gPowerTrq;      // ��Ƶ��������ʺ͵�����ת��


// ����Ϊ�͵����������趨��������
BASE_PAR_STRUCT			    gBasePar;	    //�������в���
COM_PAR_INFO_STRUCT		    gComPar;	    //��������

// ������������
CPU_TIME_STRUCT			    gCpuTime;

// �������ֵ����ñ���
MOTOR_DEBUG_DATA_RECEIVE_STRUCT     gTestDataReceive;//Ԥ���������������ֵ��Ե�����
MOTOR_DEBUG_DATA_DISPLAY_STRUCT     gTestDataDisplay;//Ԥ����������ʾ�������ֵ�������


