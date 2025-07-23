//======================================================================
//
// VF����
//
// Time-stamp: <2012-10-22 21:06:51  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"



#if F_DEBUG_RAM        // �����Թ��ܣ���CCS��build option�ж���ĺ�

#define DEBUG_F_VF          0       // VF����

#elif 1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_VF          1       // VF����
#elif 1
#define DEBUG_F_VF          0       // VF����
#endif

#endif



Uint16 vfSeprateVolAim;        // VF����ĵ�ѹ��Ŀ��ֵ
Uint16 vfSeparateVol;           // VF���������ѹ


#if DEBUG_F_VF
LINE_CHANGE_STRUCT vfSeprateVolAccLine = LINE_CHANGE_STRTUCT_DEFALUTS;
void vfSeparateDeal(void);
extern int32 FrqPlcSetDeal(void);
extern Uint16 outVoltageDisp;

//-----------------------------------------------------------
//
// VF����
//
//-----------------------------------------------------------
void vfSeparateDeal(void)
{
    int16 tmp;
    
    // ����VF���룬������
    if (((FUNCCODE_vfCurve_ALL_SEPARATE != funcCode.code.vfCurve)     // ��ΪVF����
       && (FUNCCODE_vfCurve_HALF_SEPARATE != funcCode.code.vfCurve))
	   || (motorFc.motorCtrlMode != FUNCCODE_motorCtrlMode_VF)       // ��ΪVF����
	   )
    {
        vfSeparateVol = 0;
        vfSeprateVolAim = 0;
        return;
    }

    switch (funcCode.code.vfVoltageSrc)     // VF����ĵ�ѹԴ
    {
        case FUNCCODE_vfVoltageSrc_FC:      // �������趨
            vfSeprateVolAim = funcCode.code.vfVoltageDigtalSet;
            break;

        case FUNCCODE_vfVoltageSrc_AI1:     // AI1
        case FUNCCODE_vfVoltageSrc_AI2:     // AI2
        case FUNCCODE_vfVoltageSrc_AI3:     // AI3
            tmp = aiDeal[funcCode.code.vfVoltageSrc - FUNCCODE_vfVoltageSrc_AI1].set;
            vfSeprateVolAim = (int32)tmp * funcCode.code.motorParaM1.elem.ratingVoltage >> 15;
            break;

        case FUNCCODE_vfVoltageSrc_PULSE:  // PULSE�����趨
            vfSeprateVolAim = (int32)pulseInSet * funcCode.code.motorParaM1.elem.ratingVoltage >> 15;
            break;

        case FUNCCODE_vfVoltageSrc_MULTI_SET: // �����
            vfSeprateVolAim = (int32)UpdateMultiSetFrq(diFunc.f1.bit.multiSet) 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_PLC:  // PLC
            vfSeprateVolAim = (int32)FrqPlcSetDeal() 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_PID:  // PID
            vfSeprateVolAim = (int32)FrqPidSetDeal() 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_COMM: // ͨѶ
            vfSeprateVolAim = (int32)(int16)funcCode.code.frqComm 
                * funcCode.code.motorParaM1.elem.ratingVoltage / 10000;
            break;

        default:
            break;
    }

    vfSeprateVolAim = ABS_INT16((int16)vfSeprateVolAim);

    // �����ģʽ
    if (FUNCCODE_vfCurve_HALF_SEPARATE == funcCode.code.vfCurve)
    {
        // V/F=2 * X * ��������ѹ��/������Ƶ�ʣ�
        // V = 2 * F * X * (������ѹ) / (����Ƶ��)
        // V = 2 * F * vfSeprateVolAim / ����Ƶ��
        vfSeprateVolAim = ((Uint32)2 * ABS_INT32(frq) * vfSeprateVolAim) / motorFc.motorPara.elem.ratingFrq;
    }

     // ͣ��״̬�����ѹΪ0
    if (!runFlag.bit.run)
    {
        vfSeparateVol = 0;
        return;
    }

    // ������������ѹΪ������ѹ
    if (vfSeprateVolAim > motorFc.motorPara.elem.ratingVoltage)
    {
        vfSeprateVolAim = motorFc.motorPara.elem.ratingVoltage;
    }
     
// VF����ĵ�ѹ����ʱ��
    vfSeprateVolAccLine.aimValue = vfSeprateVolAim;
    vfSeprateVolAccLine.tickerAll = (Uint32)funcCode.code.vfVoltageAccTime * (TIME_UNIT_VF_VOL_ACC_TIME / VF_CALC_PERIOD);
    vfSeprateVolAccLine.maxValue = motorFc.motorPara.elem.ratingVoltage;
    vfSeprateVolAccLine.curValue = vfSeparateVol;    // �ڵ�ǰ�����ѹ�Ļ����ϱ仯
    vfSeprateVolAccLine.calc(&vfSeprateVolAccLine);
    vfSeparateVol = vfSeprateVolAccLine.curValue;
}


#elif 1

void vfSeparateDeal(void){}




#endif



