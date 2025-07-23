//======================================================================
//
// ��������ĸ���
// ������
// �������Ĳ���,
// ���Ʋ���
//
// Time-stamp: <2012-12-20 14:01:46  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_invPara.h"


#if F_DEBUG_RAM
#define DEBUG_F_MANY_MOTOR     0    // ������
#elif 1
#define DEBUG_F_MANY_MOTOR     1
#endif


Uint16 polePair;


#if DEBUG_F_MANY_MOTOR

//-----------------------------------------------------------------
#define MOTOR3_INDEX_N_INC_TO_MOTOR2                        \
(GetCodeIndex(funcCode.code.motorFcM3.motorCtrlMode) -      \
GetCodeIndex(funcCode.code.motorFcM2.motorCtrlMode))        \

#define MOTOR4_INDEX_N_INC_TO_MOTOR2                        \
(GetCodeIndex(funcCode.code.motorFcM4.motorCtrlMode) -      \
GetCodeIndex(funcCode.code.motorFcM2.motorCtrlMode))        \

// ��2/3/4����ķǵ��������������2����ķǵ��������CodeIndex����
const Uint16 motorSnIndexNIncToMotor2[3] = 
{
    0,                              // ��2���
    MOTOR3_INDEX_N_INC_TO_MOTOR2,   // ��3���
    MOTOR4_INDEX_N_INC_TO_MOTOR2,   // ��4���    
};

//=====================================================================
//
// ���µ�1/2/3/4�����
// ����������صĵ�����������Ʋ�����
// ������صĲ���
//
//=====================================================================

//Uint16 polePair;        // ������

extern const int16 decNumber[];
void UpdateMotorPara(void)
{
    Uint16 tmp; 
    
    // ������
    if (diSelectFunc.f2.bit.motorSnDi)  
    {
        motorSn = (enum MOTOR_SN)diFunc.f2.bit.motorSnDi;   // ����ȷ��
    }
    else
    {
        motorSn = funcCode.code.motorSn;                    // ������ȷ��
    }
    


    // ���µ������
    if (MOTOR_SN_1 == motorSn)  // ��1���
    {
        memcpy(&motorFc.motorPara, &funcCode.code.motorParaM1, sizeof(struct MOTOR_PARA_STRUCT));  // �������
        memcpy(&motorFc.pgPara,    &funcCode.code.pgParaM1, sizeof(struct PG_PARA_STRUCT));        // PG������  
        memcpy(&motorFc.vcPara, &funcCode.code.vcParaM1, sizeof(struct VC_PARA));                  // ʸ�����Ʋ���
        motorFc.motorCtrlMode = funcCode.code.motorCtrlMode;             // ������Ʒ�ʽ
        motorFc.accDecTimeMotor = 0;                                     // ����Ӽ���ʱ��(��1����ļӼ���ʱ���ɶ��Ӹı�)
        motorFc.torqueBoost = funcCode.code.torqueBoost;                 // ת������               
//        motorFc.antiVibrateGainMode = funcCode.code.antiVibrateGainMode; // ����������ģʽ
        motorFc.antiVibrateGain = funcCode.code.antiVibrateGain;         // ����������
    }
    else    // ��2/3/4���
    {
		 memcpy(&motorFc, &funcCode.all[GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.motorType) + motorSnIndexNIncToMotor2[motorSn - MOTOR_SN_2]], sizeof(struct MOTOR_FC));  
    }

    // ǿ��VF����
    if (errorCode == ERROR_ENCODER)   // Ϊ����������
    {
        motorFc.motorCtrlMode = FUNCCODE_motorCtrlMode_VF;
    }


#if 1
    // ���㼫����
    tmp = motorFc.motorPara.elem.ratingSpeed;
    polePair = (60UL * motorFc.motorPara.elem.ratingFrq / decNumber[funcCode.code.frqPoint] + tmp / 2) / tmp;
#endif    
}

#elif 1

void UpdateMotorPara(void)
{
    motorFc.motorCtrlMode = funcCode.code.motorCtrlMode;
}

#endif




