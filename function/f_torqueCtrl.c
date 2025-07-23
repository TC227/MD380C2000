//======================================================================
//
// ת�ؿ�����ش���
//
// Time-stamp: <2012-06-14 16:04:40  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_main.h"
#include "f_p2p.h"

#if F_DEBUG_RAM                     // �����Թ��ܣ���CCS��build option�ж���ĺ�
#define DEBUG_F_TORQUE      0       // ת�ؿ���
#elif 1
#define DEBUG_F_TORQUE      1       // ת�ؿ���
#endif

int16 upperTorque;          // �ٶȿ���ʱΪת�����ޣ�ת�ؿ���ʱΪ�趨ת��
int16 torqueAim;            // ת��Ŀ���趨

Uint16 torqueSetEthDp;      // PLCͨѶ�������趨ת�أ�ת�ؿ���ʱʹ��
Uint16 torqueUpperEthDp;    // PLCͨѶ������ת�����ޣ��ٶȿ���ʱʹ��

#define TORQUE_LIMIT_TIME   800     // ת���޶��У��ж�ʱ�䣬_ms
extern Uint16 itDisp;

#if DEBUG_F_TORQUE

LINE_CHANGE_STRUCT torqurLine = LINE_CHANGE_STRTUCT_DEFALUTS;

// ����ת�ؿ���Ŀ��ֵ
void UpdateTorqueAim(void);

// ת�����Լ���
void AccDecTorqueCalc(void);

//=====================================================================
// 
// ת������
// 
//=====================================================================
void TorqueCalc(void)
{
    UpdateTorqueAim();       // ����ת��Ŀ���趨

    AccDecTorqueCalc();      // ת�ؼӼ��ټ���
}

// ����ת��Ŀ���趨
void UpdateTorqueAim(void)
{
    int16 tmp,tmp1;
    Uint16 torqueSrc;   // ת��Դ
    int16 torquePu;     // ת�ص�PU

    // ��ת�ؿ���
    if (RUN_MODE_TORQUE_CTRL != runMode)    
    {
        // �ٶȿ���ת��Դ��ת������
        torqueSrc = motorFc.vcPara.spdCtrlDriveTorqueLimitSrc;
        torquePu = motorFc.vcPara.spdCtrlDriveTorqueLimit;
    }
    // ת�ؿ���
    else    
    {
        // ��������ת��Դ��ת������
        torqueSrc = funcCode.code.driveUpperTorqueSrc;  // ����ת������Դ
        torquePu = funcCode.code.driveUpperTorque;      // ����ת�����������趨
    }

    // ����ת������Դ
    switch (torqueSrc)
    {     
        case FUNCCODE_upperTorqueSrc_FC:  // �������趨
            tmp = torquePu;
            break;

        case FUNCCODE_upperTorqueSrc_AI1:  // AI1
        case FUNCCODE_upperTorqueSrc_AI2:  // AI2
        case FUNCCODE_upperTorqueSrc_AI3:  // AI3
            tmp = torqueSrc - FUNCCODE_upperTorqueSrc_AI1;
            tmp = ((int32)aiDeal[tmp].set * torquePu) >> 15;
            break;
    
        case FUNCCODE_upperTorqueSrc_PULSE:  // PULSE
            tmp = ((int32)pulseInSet * torquePu) >> 15;
            break;

        case FUNCCODE_upperTorqueSrc_COMM:   // ͨѶ
            // funcCode.code.frqComm���ܳ���32767��Ŀǰ [-10000, +10000]
#if DEBUG_F_PLC_CTRL
            if (funcCode.code.plcEnable)
            {
                tmp = (int16)funcCode.code.plcTorqueSet;
            }
            else
#endif      
#if DEBUG_F_P2P_CTRL
            if ((CanRxTxCon == P2P_COMM_SLAVE) // ��Ե�ͨѶ��Ч�ҽ���������Ϊת�ظ���
                && (funcCode.code.p2pRevDataSel == P2P_REV_TORQUE_SET)
                )
            { 
                tmp = ((int32)(int16)p2pData.processValue * 2000) / 10000;
            }
            else
#endif
            {
                tmp = ((int32)(int16)funcCode.code.frqComm * torquePu) / 10000;
            }
            break;

        case FUNCCODE_upperTorqueSrc_MIN_AI1_AI2:  // MIN(AI1,AI2)
            tmp = ((int32)aiDeal[0].set * torquePu) >> 15;
            tmp1 = ((int32)aiDeal[1].set * torquePu) >> 15;
            
            if (tmp > tmp1)
            {
                tmp = tmp1;
            }
            break;
            
        case FUNCCODE_upperTorqueSrc_MAX_AI1_AI2:  // MAX(AI1,AI2)
            tmp = ((int32)aiDeal[0].set * torquePu) >> 15;
            tmp1 = ((int32)aiDeal[1].set * torquePu) >> 15;
            
            if (tmp < tmp1)
            {
                tmp = tmp1;
            }
            break;
            
        default:
            break;
    }

    torqueAim = tmp;

#if 0
#define RATING_TORQUE      2000                              // �ת��, 200.0%
#define TORQUE_LINE_FILTER funcCode.code.torqueFilter*5      // ת�ظ��������˲�
    // ת�ظ��������˲�
    torqurLine.aimValue = ABS_INT16(torqueAim);
    torqurLine.tickerAll = TORQUE_LINE_FILTER;
    torqurLine.maxValue = RATING_TORQUE; 
    torqurLine.curValue = upperTorque;
    torqurLine.calc(&torqurLine);
    upperTorque = torqurLine.curValue;


    AccDecTorqueCalc();
#endif
}


// ���㵱ǰת���趨
void AccDecTorqueCalc(void)
{
    int32 accDecTime;

    // ��ת�ؿ���
    if (RUN_MODE_TORQUE_CTRL != runMode)    // ��ת�ؿ���
    {
        upperTorque = torqueAim;
        return;
    }

    if (!runFlag.bit.run)       // ͣ��
    {
        upperTorque = 0;
        return;
    }

    if (torqueAim == upperTorque)  // �Ѿ��ﵽĿ��ת��
    {
        return;
    }

    if (((upperTorque >= 0) && (torqueAim > upperTorque))
        || ((upperTorque <= 0) && (torqueAim < upperTorque)))   // ת�ؼ���
    {
        accDecTime = (int32)funcCode.code.torqueCtrlAccTime;
    }
    else
    {
        accDecTime = (int32)funcCode.code.torqueCtrlDecTime;
    }
    
    accDecTime = accDecTime * (Uint16)(TIME_UNIT_TORQUE_CTRL_ACC_DEC / TORQUE_CTRL_PERIOD); // ת����ticker

#define RATING_TORQUE   2000    // �ת��, 100.0%
    // ֱ�߼Ӽ���
    torqurLine.aimValue = torqueAim;
    torqurLine.tickerAll = accDecTime;
    torqurLine.maxValue = RATING_TORQUE; 
    torqurLine.curValue = upperTorque;
    torqurLine.calc(&torqurLine);
    upperTorque = torqurLine.curValue;
}



/************************************************************
 * ��������: �ж��Ƿ���ת���޶���
 * ����: 
 *       ret :  0-δ����ת���޶���   1-����ת���޶���
 *
************************************************************/
Uint16 TorqueLimitCalc(void)
{
    static Uint16 torqueLimitCurTicker;
    static Uint16 torqueLimitSpdTicker;
    Uint16 ret = 0;

// ת�س���85%������ʱ�䳬��_
    if (itDisp > (int32)(ABS_INT32(upperTorque) * 85) / 100)
    {
        if (++torqueLimitCurTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        {
            torqueLimitCurTicker = TORQUE_LIMIT_TIME / DO_CALC_PERIOD + 1;
        }
    }
    else
    {
        torqueLimitCurTicker = 0;
    }

// �ٶ�ƫ���2Hz������ʱ�䳬��_
    if ( (ABS_INT32(frq - frqRun)) > 200 )
    {
        if (++torqueLimitSpdTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        {
            torqueLimitSpdTicker = TORQUE_LIMIT_TIME / DO_CALC_PERIOD + 1;
        }
    }
    else
    {
        torqueLimitSpdTicker = 0;
    }

// �ж��Ƿ��� ת���޶���
    if ((torqueLimitCurTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
// 2009.05.12��������ۺ�ȥ��������
// �������Ӧ�ñ���
        && (torqueLimitSpdTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        )
    {
        ret = 1;
    }

    return (ret);
}


#elif 1

void TorqueCalc(void)
{
    upperTorque = motorFc.spdCtrlTorqueLimit;
}

void UpdateTorqueAim(void)
{
    torqueAim = motorFc.spdCtrlTorqueLimit;
}

Uint16 TorqueLimitCalc(void)
{
    return 0;
}
#endif







