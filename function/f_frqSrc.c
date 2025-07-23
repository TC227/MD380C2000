//======================================================================
//
// Ƶ��Դ���� 
//
// Time-stamp: <2012-10-24 15:06:59  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_frqSrc.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_p2p.h"
#include "f_comm.h"

int32 upDownFrq;
Uint16 upDownFrqInit;
int32 upDownFrqTmp;

int32 uPDownFrqMax;
int32 uPDownFrqMin;
int32 uPDownFrqLower1;
int32 uPDownFrqLower2;

#if F_DEBUG_RAM    // �����Թ��ܣ���CCS��build option�ж���ĺ�

#define DEBUG_F_MULTI_SET_PLC       0   // ����PLC�������
#define DEBUG_F_JUMP_FRQ            0   // ��ԾƵ��
#define DEBUG_F_FRQ_SRC_COMPOSE     0   // Ƶ��Դ����

#elif 1

#define DEBUG_F_MULTI_SET_PLC       1
#define DEBUG_F_JUMP_FRQ            1
#define DEBUG_F_FRQ_SRC_COMPOSE     1

#endif


union FRQ_FLAG frqFlag;


#define PLC_END_LOOP_TIME_MAX   250 // ���һ��PLCѭ�������һ��_ms�������ź�
Uint16 plcStep;         // PLC�Ľ׶Σ�[0, PLC_STEP_MAX]
Uint32 plcTime;         // ��ǰPLC�׶ε�����ʱ�䣬Ŀǰ��λΪ10ms
Uint16 bPlcEndOneLoop;  // plc���һ��ѭ���ı�־���ñ�־����(PLC_END_LOOP_TIME_MAX)ms
Uint16 plcStepRemOld;   // PLC�����step
Uint32 plcTimeRemOld;   // PLC�����time

int32 frq;              // ˲ʱֵ, ���ܴ��ݸ����ܵ�����Ƶ��
int32 frqTmp;           // ����frq����ʱ����
int32 frqFrac;          // С������, Q15
int32 frqTmpFrac;       // ����frq����ʱ������С������, Q15
int32 frqDroop;         // �´�֮����ٶȣ�˲ʱֵ, ���ܴ��ݸ����ܵ�����Ƶ��
int32 frqCurAimFrac;

int32 frqAim;           // �趨(Ŀ��)Ƶ��
int32 frqAimTmp;        // ����frqAim����ʱ����, ��һ�μ��㲻��ʹ�á���ԾƵ��֮�󣬵㶯Ƶ��֮ǰ��
int32 frqAimTmp0;       // ����frqAim����ʱ����, ��һ�μ����ʹ�á�Ƶ��Դ�趨֮����ԾƵ��֮ǰ��

int32 frqCurAim;        // ��ǰĿ��Ƶ�ʣ�ע����ÿ�ε���AccDecFrqCalc()֮ǰҪ����frqCurAim

Uint16 timeBench;       // �Ӽ���ʱ���׼

int32 frqDigitalTmp;    // �����趨Ƶ����ʱֵ


struct FRQ_XY frqXy;       

LOCALF int32 frqDigitalPlc;
LOCALF Uint16 presetFrqOld;    // Ԥ��Ƶ�ʵ�oldֵ

Uint16 upperFrq;        // ����Ƶ��
Uint16 lowerFrq;        // ����Ƶ��
Uint16 maxFrq;          // ���Ƶ��
Uint16 benchFrq;        // ��׼Ƶ��
int32 frqAiPu;          // AI,pulse��100%��ӦƵ��

Uint16 frqPuQ15;        // Ƶ��ת��ʱ��Q15��1PU��Ƶ�ʣ���maxFrq+20Hz

Uint16 frqCalcSrc;
Uint16 frqCalcSrcOld;
Uint16 plcStepOld;          // ���һ��PLCѭ��ʹ��
Uint16 bStopPlc;            // PLC�����ж�ʱ��ȫΪ0�����ߵ�������ͣ��

Uint16 bFrqDigital;         // �����趨Ƶ�ʱ�־��������UP/DOWN���֮��һ��ʱ�����ʾ����ʱ��
Uint16 frqKeyUpDownDelta;   // ʹ�����UP/DOWN����Ƶ�ʵ�delata

LOCALF Uint32 reverseTicker;// ����תʱ���ʱ

Uint16 bAntiReverseRun;     // ���ڽ�ֹ��ת�С�������������Ϊ��ֹ��ת���Ҹ�Ƶ�ʻ��߷�ת���������Ϊ1.

LOCALF JUMP_FRQ jumpFrq1;   // ��ԾƵ��
LOCALF JUMP_FRQ jumpFrq2;   // ��ԾƵ��2

// ����ģʽ
Uint16 runMode; // ����ģʽ
int32 frqAimOld4Dir;

// ��������Ƶ��Դ
Uint16 runCmdBind;

LOCALD int32 FrqAimUpDownDeal(void);
LOCALD int32 FrqSrcOperate(void);
LOCALD int32 FrqXyCalc(Uint16 src);
LOCALD int32 FrqPlcSetDeal(void);
LOCALD void JumpFrqDeal(void);
LOCALD void JumpFrqEndCalc(JUMP_FRQ *p);
LOCALD void UpdateLimitFrq(void);
LOCALD void UpdateRunMode(void);


int32 GetFrqX(void);
int32 GetFrqY(void);
int32 GetFrqComp(void);
int32 GetFrq(void);

Uint16 frqPlcType;
//=====================================================================
// 
// ����Ƶ��Դ(280F�Ĺ�������F0-01)�������趨Ƶ��frqAimTmp0
//
// �������з��򡢵㶯������������Դ����ģ�
// �������з��򡢵㶯�Լ�֮��ķ���תѡ�����з����෴Ҳ��������Դ�д���
//
//=====================================================================
void FrqSrcDeal(void)
{
    static int32 frqAimTmpBak;

// ������Ƶ�ʼ���
    UpdateLimitFrq();

// ��������ģʽ
    UpdateRunMode();

    runFlag.bit.plc = 0;
    runFlag.bit.pid = 0;
    runFlag.bit.torque = 0;

    frqPlcType = 0;
    
    if (RUN_MODE_TORQUE_CTRL == runMode)
    {
        runFlag.bit.torque = 1;
    }

    frqCalcSrc = funcCode.code.frqCalcSrc;
    
    if (frqCalcSrcOld != frqCalcSrc)    // frqCalcSrc�ı���
    {
        frqCurAimFrac = 0;  //-= С����ı�
    }

    if (presetFrqOld != funcCode.code.presetFrq)   // ��Ԥ��Ƶ�ʸı���(F0-03,�ָ���������,ͨѶ�޸ĵ�)�������趨Ƶ��
    {
        presetFrqOld = funcCode.code.presetFrq;
    }

// Ƶ��Դ����
    if ((!runFlag.bit.jog)          // �㶯
// ����ʱ����Ƶ��Դ
//        && (!errorCode)             // ������
        && (RUN_MODE_TORQUE_CTRL != runMode)    // û��ת�ؿ���
        )
    {
        frqAimTmp0 = FrqSrcOperate();
    }
    else
    {
        accDecFrqPrcFlag = ACC_DEC_FRQ_NONE;      // ����UP/DOWN�޸�Ƶ�ʱ�־������
    }

    // PLC״̬��λ
    if (diFunc.f1.bit.resetPLC)
    {
        plcStep = 0;
        plcTime = 0;
    }

    frqCalcSrcOld = frqCalcSrc;     // ����frqCalcSrcOld
    plcStepOld = plcStep;   // ����plcStepOld
// �ж��Ƿ񳬹�����Ƶ��
    if (frqAimTmp0 > upperFrq)
    {
        frqAimTmp0 = upperFrq;
    }
    else if (frqAimTmp0 < -(int32)upperFrq)
    {
        frqAimTmp0 = -(int32)upperFrq;
    }

#if 0
// ת�ؿ���ʱ�ж��Ƿ񳬹�ת�ؿ������Ƶ��
    if (RUN_MODE_TORQUE_CTRL == runMode)    // ��ת�ؿ���
    {
        // �������Ƶ��
        if (frqAimTmp0 >= 0)
        {
            if (frqAimTmp0 > funcCode.code.torqueCtrlFwdMaxFrq)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;
            }
        }
        // �������Ƶ��
        else
        {
            if (frqAimTmp0 < -(int32)funcCode.code.torqueCtrlRevMaxFrq)
            {
                (frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq);
            }
        }
    }
    
#endif

     // Ƶ���趨��Ч����
    if (diSelectFunc.f2.bit.frqOk && (!diFunc.f2.bit.frqOk))
    {    
        frqAimTmp0 = frqAimTmpBak;
    }
    else
    {
        frqAimTmpBak = frqAimTmp0;
    }
        
// ��ֵ��frqAimTmp
    frqAimTmp = frqAimTmp0;

// �趨Ƶ����ʾֵ
    frqAimDisp = ABS_INT32(frqAimTmp);   // ��ԾƵ��֮ǰ���㶯֮���ֵ

// ��ԾƵ�ʴ����㶯Ƶ�ʲ�����ԾƵ��Ӱ�졣
// ��ԾƵ�ʼ���֮ǰ����������Ƶ�ʱȽϡ�
// ������������ԾƵ�ʵ�low��������Ƶ�ʣ�����������
// ��ԾƵ�ʼ���֮���ٺ�����Ƶ�ʱȽϣ��Ƿ�������
// �������趨Ƶ�ʸ�������Ƶ�ʣ�����������������ԾƵ�ʵ�low��������Ƶ�ʣ�����������ͣ�����̡�
    JumpFrqDeal();
}


//=====================================================================
// 
// 1. �������з���,���з�ʽ(�㶯������ͨ����)����ԾƵ��
// �����趨Ƶ��(Ŀ��Ƶ��frqAim)
// 2. �����趨Ƶ�ʺ�����Ƶ�ʵķ����жϵ�ǰ�Ƿ����ڷ���(�Ƿ�������ת������)��
//
// �������з��򡢵㶯������������Դ����ģ�
// ���Ըú���Ӧ��������Դ�е��á�
// 
//=====================================================================
void UpdateFrqAim(void)
{
    int32 frq4Dir;           // ˲ʱƵ��, �жϷ���ʹ��

// ��ֵ��frqAimTmp
    //frqAimTmp = frqAimTmp0;  // FrqSrcDealΪ2ms��UpdateFrqAimΪ0.5����,���ܵ������²���ȡ�����ִ��
    //JumpFrqDeal();
    
// ���з���
    if (FORWARD_DIR != runCmd.bit.dir)
    {
        frqAimTmp = -frqAimTmp;
    }

// �㶯���㶯������runCmd.bit.dir�޹ء�
    if (runFlag.bit.jog)            // ������Ч�ĵ㶯����
    {                               // �㶯����ȡ��֮��Ҳ���ܽ���öδ���
        if (RUN_CMD_FWD_JOG == runCmd.bit.jog)  // �㶯���򣬶�������ͨ���з���
        {
            frqAimTmp = funcCode.code.jogFrq;
        }
        else if (RUN_CMD_REV_JOG == runCmd.bit.jog) //
        {
            frqAimTmp = -(int16)funcCode.code.jogFrq;
        }

        frqAimDisp = funcCode.code.jogFrq; // �㶯ʱ���趨Ƶ����ʾΪ�㶯Ƶ��
    }

#if DEBUG_F_PLC_CTRL
    frqAimPLCDisp = (int16)(frqAimTmp*10000/maxFrq);
#endif
    
// �㶯Ƶ�ʲ��ܳ�������Ƶ��
// �ж��Ƿ񳬹�����Ƶ��

    frqFlag.bit.frqSetLimit = 0;
    if (frqAimTmp >= upperFrq)
    {
        frqAimTmp = upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }
    else if (frqAimTmp <= -(int32)upperFrq)
    {
        frqAimTmp = -(int32)upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }

// ��ֵ��frqAim
    frqAim = frqAimTmp;

// �ж��趨Ƶ�ʵķ���
    if (frqAim > 0)
    {
        runFlag.bit.dir = FORWARD_DIR;
    }
    else if (frqAim < 0)
    {
        runFlag.bit.dir = REVERSE_DIR;
    }
    else
    {
        runFlag.bit.dir = runCmd.bit.dir;
    }

// ��ת��ֹ
    bAntiReverseRun = 0;
    if (funcCode.code.antiReverseRun)
    {
        if (frqAimTmp < 0)  // ��ת��ֹ���趨Ƶ��Ϊ�����趨Ƶ����ʾΪ0��
        {
            frqAimDisp = 0;
            frqFlag.bit.frqSetLimit = 1;
        }
        
        if ((frqAimTmp < 0) || (frqAim < 0))
        {
            frqAim = 0;      // ��ת��ֹ�����븺Ƶ��ʱ����0Ƶ���С�
            frqFlag.bit.frqSetLimit = 1;
            bAntiReverseRun = 1;
        }
    }
    
// ���з����෴
    frq4Dir = frq;
    //frq4Dir = frqRun;  // ����ת����ʹ�÷�����Ƶ��
    runFlag.bit.dirFinal = runFlag.bit.dir;
    if (FUNCCODE_runDir_REVERSE == funcCode.code.runDir)
    {
        frqAim = -frqAim;
        frq4Dir = -frq4Dir;

        runFlag.bit.dirFinal = ~runFlag.bit.dirFinal;
    }

// �ж�����Ƶ�ʵķ���
    if (frq4Dir > 0)
    {
        runFlag.bit.curDir = FORWARD_DIR;
    }
    else if (frq4Dir < 0)
    {
        runFlag.bit.curDir = REVERSE_DIR;
    }
    else    // ����ת����
    {
        if (runFlag.bit.run
            && ((int64)frqAimOld4Dir * frqAim < 0)    // ������0֮�䣬���ܽ���
            )
        {
            if (++reverseTicker >= (Uint32)funcCode.code.zeroSpeedDeadTime
                * (Uint16)(TIME_UNIT_ZERO_SPEED_DEAD / RUN_CTRL_PERIOD))
            {
                runFlag.bit.curDir = runFlag.bit.dir;
                
                //if (frqAim)
                    frqAimOld4Dir = frqAim;
            }
        }
        else
        {
            runFlag.bit.curDir = runFlag.bit.dir;

            //if (frqAim)      // �趨Ƶ��Ϊ0ʱ��������frqAimOld4Dir
            {
                frqAimOld4Dir = frqAim;
            }
        }
    }

// �ж��Ƿ����ڷ���
    if ((runFlag.bit.curDir != runFlag.bit.dir) // ���ڷ���
//        && (funcCode.code.zeroSpeedDeadTime)    // û�� ����ʱ�䣬frqCurAimֱ�Ӹ�ֵΪ�趨Ƶ�ʣ������м丳ֵΪ0.����s����
        )
    {
        runFlag.bit.dirReversing = 1;
    }
    else        // û�з���
    {
        runFlag.bit.dirReversing = 0;
        reverseTicker = 0;      // ticker����
    }
}


//=====================================================================
//
// �����趨Ƶ��.
// Ƶ��ԴΪ�����趨����UP/DOWN���ڣ��������̺�DI
//
// ��ע��
//      PLC�Ͷ��ָ��Ҳ����ñ�����
//
//=====================================================================   
LOCALF int32 FrqAimUpDownDeal(void)
{
    int32 frqDeltaDi;                      // �����趨UP/DOWNƵ�ʵ�����
    Uint16 up = diFunc.f1.bit.up;
    Uint16 down = diFunc.f1.bit.down;
    int32 delta = 0;
    static Uint16 bFrqDigitalOld;
    static int16 frqDeltaDiRemainder;      // �����趨UP/DOWNƵ�ʵ�������ֵ
    //frqFlag.bit.upDown = 1;
    
// ��DI��UP������Ч��DI��DOWN������Ч��
// 0���˵��¼���û�е�������UP��0���˵��¼���û�е�������DOWN������Ϊû�н��������趨Ƶ��
    if ((up || down || accDecFrqPrcFlag)
        && (MENU0_DISP_STATUS_RUN_STOP == menu0DispStatus)  // ������/ͣ����ʾʱ��up/down�������ڹ���/��г��ʾʱ����up/down
        && (!runFlag.bit.dirReversing)  // δ��������ת�л���
        && ((diSelectFunc.f2.bit.frqOk&&diFunc.f2.bit.frqOk) || (!diSelectFunc.f2.bit.frqOk))    // Ƶ���趨��Ч���ӹ��ܱ�ѡ���Ҷ�����Ч �� Ƶ���趨����δѡ��
        && ((ERROR_LEVEL_RUN != errorAttribute.bit.level) 
           || ((ERROR_LEVEL_RUN == errorAttribute.bit.level) && (funcCode.code.errorRunFrqSrc == ERR_RUN_FRQ_AIM)))
        )
    {
        bFrqDigital = 1;
    }
    else
    {
        bFrqDigital = 0;
    }

    // ����״̬�£�UP/DOWN���������趨Ƶ�ʣ��ӵ�ǰ(��˲ʱ)Ƶ�ʿ�ʼ�޸ġ�
    if (bFrqDigitalOld != bFrqDigital)      // �����趨Ƶ�ʱ�־�ı���
    {
        if (bFrqDigital
            && runFlag.bit.common           // ֮ǰû�������趨Ƶ�ʣ��ҵ�ǰ�������
            && (SWING_NONE == swingStatus)  // û�н����Ƶ
            )
        {
            //frqDigitalTmp = ABS_INT32(frq); // �趨Ƶ�ʴӵ�ǰƵ�ʵľ���ֵ��ʼ�޸�
            if(funcCode.code.updnBenchmark //|| 
               // ((frqFlag.bit.comp) && 
               // (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
              )
            {
                // ���趨Ƶ��Ϊ��׼
            }
            else
            {
                // ��������Ƶ��������
    			if ((ABS_INT32(frqAim) < lowerFrq) && (ABS_INT32(frq) <= lowerFrq))
    			{
                    if (runFlag.bit.curDir^runDirPanelOld)
                    {
                        upDownFrq = uPDownFrqLower2;
                    }
                    else
                    {
                        upDownFrq = uPDownFrqLower1;
                    }
    			}
    			else
                {
        			// ������+����
                    if ((frqFlag.bit.comp)
						&& ((((frqXy.x + frqXy.z) < 0)
                        	&& (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
						 || (((frqXy.y + frqXy.z + funcCode.code.presetFrq) < 0)
                        	&& (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1))
							)
                        )
        			{
    					upDownFrq += ABS_INT32(frqAim) - ABS_INT32(frq);
    				}
    				// X(����)
    		        // Y(����)
    		        // X(����) + Y(����)��Ҳ�Ƿǵ���
    		        // X(����) + Y(������)
                    else
    				{
    					upDownFrq += ABS_INT32(frq) - ABS_INT32(frqAim);
    				}
                }
			}
        }

        if (!bFrqDigital)
        {
            bFrqDigitalDone4WaitDelay = 1;  // UP/DOWN������־
        }

        frqDeltaDiRemainder = 0;  // �����趨Ƶ�ʱ�־�ı�ʱ����ֵ����
    }

    if (bFrqDigital)    // ����UP/DOWN�趨Ƶ��
    {
        upDownFrqInit = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_ON;
        
        if (ACC_DEC_FRQ_WAIT == accDecFrqPrcFlag)  // ����UP/DOWN�޸�Ƶ��
        {
            if (KEY_UP == keyFunc)      // ���������趨Ƶ�ʣ���0���˵��°���UP
            {
                delta = (int32)frqKeyUpDownDelta;
            }
            else //if (KEY_DOWN == keyFunc) // ���̼�С�趨Ƶ�ʣ���0���˵��°���DOWN
            {
                delta = -(int32)frqKeyUpDownDelta;
            }

            accDecFrqPrcFlag = ACC_DEC_FRQ_DONE;      // ����UP/DOWN �Ѿ�����
        }
        else if ((up || down)       // DI���ӵ�UP/DOWN��Ч
                && (!(up && down) && (!frqFlag.bit.upDown))  // ͬʱ��Ч�����仯
                )
        {
            int32 diUpDownSlope = funcCode.code.diUpDownSlope;
            
            if (down)
            {
                diUpDownSlope = -diUpDownSlope;
            }
            
            frqDeltaDi = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) / (10000);
            frqDeltaDiRemainder = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) % (10000);
            
            delta = frqDeltaDi;
        }
    }

    frqFlag.bit.upDown = 1;
    upDownFrq += delta;

    if (frqFlag.bit.upDownoperationStatus == UP_DN_OPERATION_OFF)
    {
        upDownFrq = (int16)upDownFrqInit;  
    }
    
    if (upDownFrq > uPDownFrqMax)
    {
        upDownFrq = uPDownFrqMax;
    }
    else if (upDownFrq < uPDownFrqMin)
    {
        upDownFrq = uPDownFrqMin;
    }
    
#if 1
    if ((diFunc.f1.bit.clearUpDownFrq)           // DI������Ч��UP/DOWN�趨���㣨���ӡ����̣�
//        || (frqSrcOld != frqSrc)                    // �л�������Ƶ���趨�������л���PLC/�����
        )
    {
        upDownFrq = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    }
#endif

    bFrqDigitalOld = bFrqDigital;

    // ����Ƶʼ��㣬��YҲΪ�����趨��Ԥ��Ƶ��(F0-08)��������
    if ((frqFlag.bit.comp) && (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
    {
        // X(������) + Y(����)
        return upDownFrq;
    }
    else
    {
        // X(����)
        // Y(����)
        // X(����) + Y(����)��Ҳ�Ƿǵ���
        // X(����) + Y(������)
        return (upDownFrq + (int32)funcCode.code.presetFrq);
    }
}


//=====================================================================
//
// Ƶ��ԴΪPLC
//
//=====================================================================
LOCALF int32 FrqPlcSetDeal(void)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 plcEndLoopDelayTicker;
    static Uint16 plcTimeTicker;
    static Uint16 plcFlag;
    static Uint16 plcStepEnable;
    static Uint16 bPlcEndOneLoopEnable = 1;
    Uint32 plcTimeMax = 0;
    Uint16 loop = 0;
    Uint16 plcStep1;            // PLC�Ľ׶Σ�[0, PLC_STEP_MAX-1]
    int32 frqPlc;

    runFlag.bit.plc = 1;
    bStopPlc = 0;               // ֹͣPLC��־

#define PLC_TIME_TICKER     5   // 5��PLC�������ڣ�plcTime����1����ֹplcTime���
    // ת�ٸ��١�����ֱ���ƶ�ʱ�䲻����PLCʱ�䣬����Ƶ�ʱ���ʱ�����PLCʱ�䡣    
    if (((RUN_STATUS_START == runStatus) && (START_RUN_STATUS_HOLD_START_FRQ == startRunStatus))
        || (RUN_STATUS_NORMAL == runStatus)
        )
    {
        if ((!frqPlcType)  // ����2msѭ��δ����PLC���д���
            && (++plcTimeTicker >= PLC_TIME_TICKER))   // ��ֹplcTimeMax���
        {
            plcTimeTicker = 0;
            plcTime++;
        }
        
        plcFlag = 1;    // �ϵ�֮��ʹ�ù�PLC
    }
    else if (!runFlag.bit.run)  // ����(��������)ͣ����������plcStep��
    {
        
        // ��������䡣ͣ�������ݲ�����
        if (((FUNCCODE_plcPowerOffRemMode_REM != (funcCode.code.plcPowerOffRemMode%10))
             || (plcFlag))  // �ϵ�֮��ʹ�ù�PLC
             && (FUNCCODE_plcStopRemMode_REM != (funcCode.code.plcPowerOffRemMode/10)) // ��ͣ������
            && (!runCmd.bit.pause)
            )
        {
		    plcStep = 0;    // ��step0���¿�ʼ����
		    plcTime = 0;
        }       
        else
        {
            if (plcStep >= PLC_STEP_MAX)
            {
                plcStepRemOld = 0;
                plcTimeRemOld = 0;
            }
            plcStep = plcStepRemOld;    // ��plcStepRemOld���¿�ʼ����
            plcTime = plcTimeRemOld;    // �ָ�plcTime
        }       
    }

    if (!plcStep)
    {
        bPlcEndOneLoopEnable = 1;
    }
    
    if (plcStep < PLC_STEP_MAX)
    {
        plcTimeMax = (Uint32)funcCode.code.plcAttribute[plcStep].runTime
            * ((TIME_UNIT_PLC / PLC_TIME_TICKER) / FRQ_SRC_PERIOD);
        if (FUNCCODE_plcTimeUnit_H == funcCode.code.plcTimeUnit) // PLC����ʱ�䵥λ
            plcTimeMax *= TIME_UNIT_SEC_PER_HOUR;
    }
#undef PLC_TIME_TICKER

    if (plcTime >= plcTimeMax)
    {
        Uint16 bEndSearch = 0;
        
        plcTime = 0;
        plcTimeTicker = 0;

        for (;;)    // ������һ������ʱ�䲻Ϊ0��plcStep
        {
            if (++plcStep >= PLC_STEP_MAX)
            {
                plcStep = PLC_STEP_MAX;     // ��ֹplcStepһֱ����
                // plcRunMode�ı�ʱ��Ҳ�ܴ���
                if (FUNCCODE_plcRunMode_REPEAT == funcCode.code.plcRunMode) // һֱѭ��
                {
                    plcStep = 0;
                }

                if (FUNCCODE_plcRunMode_ONCE_RUN == funcCode.code.plcRunMode) 
                {
                    plcStep = plcStepEnable;
					bEndSearch = 1;
                }

                if ((plcStepOld < PLC_STEP_MAX) && (runFlag.bit.common) && (!runFlag.bit.jog))  // ���һ��PLCѭ��
                {
                    plcEndLoopDelayTicker = 0;
                    if (bPlcEndOneLoopEnable)
                    {
                        bPlcEndOneLoop = 1;
                        bPlcEndOneLoopEnable = 0;
                    }
                }
            }

            if (plcStep < PLC_STEP_MAX) // plcStep����Ч��Χ��
            {
                if (funcCode.code.plcAttribute[plcStep].runTime)    // plc����ʱ�䲻Ϊ0����ֹͣ�����˳�ѭ�����趨Ƶ��Ϊ��ǰplcStepƵ��
                {
                    bEndSearch = 1;
                }
            }
            else
            {
                bEndSearch = 1;  // PLC���в���һֱѭ������plcStep >= PLC_STEP_MAX, ���˳�ѭ��
            }
            
            if (++loop > PLC_STEP_MAX)      // ȫ��plc�׶ε�����ʱ�䶼Ϊ0������ѭ����������PLC
            {
                bStopPlc = 1;
                bEndSearch = 1;
            }

            if (bEndSearch)
            {
                break;
            }
        }
    }

    if (plcStep >= PLC_STEP_MAX)    // �������н���ͣ�������ߵ������н���������ֵ
    {        
        if (FUNCCODE_plcRunMode_ONCE_STOP == funcCode.code.plcRunMode) // �������н���ͣ��
        {
            bStopPlc = 1;

            if (!runFlag.bit.common)    // PLC�������н���ͣ�����Ӽ���ʱ��ʹ�����һ�μӼ���ʱ�䲻Ϊ0����һ��PLC�ļӼ���ʱ��
            {
                plcStep = 0;
            }
        }
        // �������н���������ֵ������Ϊ���һ�ε�frqAimTmp0
    }

    plcStep1 = plcStep;
    plcStepEnable = plcStep;
    if (plcStep1 >= PLC_STEP_MAX)
    {
        plcStep1 = PLC_STEP_MAX - 1;
    }

// PLC��_�μӼ���ʱ��ѡ��
    accDecTimeSrcPlc = funcCode.code.plcAttribute[plcStep1].accDecTimeSet;

// ����frqAimTmp0
    frqPlc = UpdateMultiSetFrq(plcStep1);

// ���һ��PLCѭ�������һ��_ms�������ź�
// Ƶ��Դѡ������ʱ�����޸ġ����Ը�_ms�Ĵ����������û�����⡣
// ҲҪ����PLC��������ͣ��/������ֵ
    if (bPlcEndOneLoop)
    {
        if (++plcEndLoopDelayTicker >= PLC_END_LOOP_TIME_MAX / FRQ_SRC_PERIOD)
        {
            //plcEndLoopDelayTicker = 0;
            bPlcEndOneLoop = 0;
        }
    }
    
    frqPlcType = 1;

    return frqPlc;
#else
    return 0;
#endif
}


//=====================================================================
//
// ���¶��ָ�PLC��Ƶ��
//
// ���룺
//      step -- ���ָ��step
// Ҫ��: 0 <= step <= PLC_STEP_MAX-1
//
//=====================================================================
LOCALF int32 UpdateMultiSetFrq(Uint16 step)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 stepOld;
    int32 frqPlc;
    int16 i;
    
    if (!step)               // plcStep == 0
    {
        switch (funcCode.code.plcFrq0Src) // F8-00 �����0������ʽ
        {
            case FUNCCODE_plcFrq0Src_FC:
                frqPlc = (int32)(int16)funcCode.code.plcFrq[0] * maxFrq / 1000;
                break;

            case FUNCCODE_plcFrq0Src_AI1:
            case FUNCCODE_plcFrq0Src_AI2:
            case FUNCCODE_plcFrq0Src_AI3:
                i = funcCode.code.plcFrq0Src - FUNCCODE_plcFrq0Src_AI1;
                frqPlc = ((int32)aiDeal[i].set * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PULSE:
                frqPlc = ((int32)pulseInSet * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PID:
                frqPlc = FrqPidSetDeal();
                break;
                
            case FUNCCODE_plcFrq0Src_PRESET_FRQ:
                if (stepOld)    // stepOld != 0
                {
                    frqDigitalTmp = frqDigitalPlc;  // �ָ�frqDigitalTmp
                }
                
                frqPlc = FrqAimUpDownDeal();
                break;
                
            default:
                break;
        }
    }
    else    // plcStep != 0
    {
        if (!stepOld)    // stepOld == 0
        {
            frqDigitalPlc = frqDigitalTmp;  // ����frqDigitalTmp
        }

        frqPlc = (int32)(int16)funcCode.code.plcFrq[step] * maxFrq / 1000;
    }

    stepOld = step;

    return frqPlc;
#endif
}



//=====================================================================
//
// ������Ƶ�ʣ����Ƶ�ʸ���
// ����Ƶ��Դ���ΪAIʱ��100.0%�Ӧ�������趨
//
//=====================================================================
LOCALF void UpdateLimitFrq(void)
{
    int32 tmp;
    Uint16 i;

// ���Ƶ��
    maxFrq = funcCode.code.maxFrq;
    frqPuQ15 = maxFrq + 20 * decNumber[funcCode.code.frqPoint];

// ����Ƶ��
    tmp = funcCode.code.upperFrq;
    switch (funcCode.code.upperFrqSrc)
    {
        case FUNCCODE_upperFrqSrc_FC:  // ��ֵ�趨
            break;

        case FUNCCODE_upperFrqSrc_AI1: // AI1
        case FUNCCODE_upperFrqSrc_AI2: // AI2
        case FUNCCODE_upperFrqSrc_AI3: // AI3
            i = funcCode.code.upperFrqSrc - FUNCCODE_upperFrqSrc_AI1;
            tmp = ((int32)aiDeal[i].set * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // ����Ƶ��ƫ��
            break;
            
        case FUNCCODE_upperFrqSrc_PULSE: // PULSE�����趨(DI5)
            tmp = ((int32)pulseInSet * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // ����Ƶ��ƫ��
            break;
            
        case FUNCCODE_upperFrqSrc_COMM:
            // funcCode.code.frqComm���ܳ���32767��Ŀǰ [-10000, +10000]
            tmp = ((int32)(int16)funcCode.code.frqComm * maxFrq + 5000) / 10000;   // ��������
            break;

        default:
            break;
    }

    

    if (tmp < 0L)        // AI��PulseIn����Ϊ����
        tmp = 0;
    if (tmp > (int32)maxFrq)
        tmp = maxFrq;
    
    upperFrq = tmp;

// ����Ƶ��
    lowerFrq = funcCode.code.lowerFrq;

// ����Ƶ�ʲ��ܵ�������Ƶ��
    if (upperFrq < lowerFrq)
    {
        upperFrq = lowerFrq;
    }
}


//=====================================================================
//
// ����  : ��ԾƵ�ʴ���
// ����  : 
//
//=====================================================================
LOCALF void JumpFrqDeal(void)
{
#if DEBUG_F_JUMP_FRQ
    int32 tmp = ABS_INT32(frqAimTmp);
    int32 tmp1 = ABS_INT32(frqTmp);

    jumpFrq1.frq = funcCode.code.jumpFrq1;
    jumpFrq1.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq1);

    jumpFrq2.frq = funcCode.code.jumpFrq2;
    jumpFrq2.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq2);

    if ((jumpFrq2.low <= jumpFrq1.low) && (jumpFrq1.low <= jumpFrq2.high))
    {
        jumpFrq1.low = jumpFrq2.low;
    }
    if ((jumpFrq1.low <= jumpFrq2.low) && (jumpFrq2.low <= jumpFrq1.high))
    {   
        jumpFrq2.low = jumpFrq1.low;
    }
    if ((jumpFrq2.low <= jumpFrq1.high) && (jumpFrq1.high <= jumpFrq2.high))
    {   
        jumpFrq1.high = jumpFrq2.high;
    }
    if ((jumpFrq1.low <= jumpFrq2.high) && (jumpFrq2.high <= jumpFrq1.high))
    {   
        jumpFrq2.high = jumpFrq1.high;
    }

    if ((jumpFrq1.low < tmp) && (tmp < jumpFrq1.high))  // ����ԾƵ������
    {
        // �趨Ƶ����Ծ
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim))) // Ҫ���ǣ���������Ƶ�ʣ�������Ƶ�����С�
        {
            tmp = jumpFrq1.high;
        }
        else
        {
            tmp = jumpFrq1.low;
        }
        
    }
    if ((jumpFrq2.low < tmp) && (tmp < jumpFrq2.high))  // ����ԾƵ������
    {
        // �趨Ƶ����Ծ
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim)))
        {
            tmp = jumpFrq2.high;
        }
        else
        {
            tmp = jumpFrq2.low;
        }
    }

    // �趨Ƶ����Ծ
    if (tmp > (int32)upperFrq)
    {
        tmp = (int32)upperFrq;
    }
    frqAimTmp = (frqAimTmp >= 0) ? (tmp) : (-tmp);
    
    
    if (funcCode.code.jumpFrqMode)
    {
        if ((jumpFrq1.low < tmp1) && (tmp1 < jumpFrq1.high))  // ����ԾƵ������
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                    && (runFlag.bit.run)
                    && (runCmd.bit.common)
                )
            {
                tmp1 = jumpFrq1.high;
            }
            else
            {
                tmp1 = jumpFrq1.low;
            }        
        }
        if ((jumpFrq2.low < tmp1) && (tmp1 < jumpFrq2.high))  // ����ԾƵ������
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                && (runFlag.bit.run)
                && (runCmd.bit.common)
            )
            {
                tmp1 = jumpFrq2.high;
            }
            else
            {
                tmp1 = jumpFrq2.low;
            }
        }

        if (tmp1 > (int32)upperFrq)
        {
            tmp1 = (int32)upperFrq;
        }

        frqTmp = (frqTmp >= 0) ? (tmp1) : (-tmp1);
    }
#endif
}

//=====================================================================
//
// ��ԾƵ�ʵ����˵����
//
// ���룺p->frq   -- ��ԾƵ��
//       p->range -- ��ԾƵ�ʷ���
// �����p->low   -- ��ԾƵ�ʷ�Χ��low
//       p->high  -- ��ԾƵ�ʷ�Χ��high
//
//=====================================================================
#define JUMP_FRQ_RANGE_ALL  0   // ��ԾƵ�ʵ�high, low֮��Ĳ�ֵΪjumpFrqRange��2����
#define JUMP_FRQ_RANGE_HALF 1   // ��ԾƵ�ʵ�high, low֮��Ĳ�ֵΪjumpFrqRange��
#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_ALL  // ��ԾƵ�ʵ�high, low֮��Ĳ�ֵΪjumpFrqRange��2����
//#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_HALF // ��ԾƵ�ʵ�high, low֮��Ĳ�ֵΪjumpFrqRange��1����
LOCALF void JumpFrqEndCalc(JUMP_FRQ *p)
{
#if DEBUG_F_JUMP_FRQ
    int32 low;

#if 0       // ��ԾƵ����Ϊ0ʱ���������á���֮ǰ����һ��
    if (0 == p->frq)    // ��ԾƵ����Ϊ0ʱ���������á�
    {
        p->low = 0;
        p->high = 0;
        return;
    }
#endif

    low = (int32)p->frq - ((int32)p->range >> JUMP_FRQ_RANGE);
    if (low < 0)
        low = 0;
    p->low = low;

    p->high = (int32)p->frq + ((int32)p->range >> JUMP_FRQ_RANGE);
#endif
}
#undef JUMP_FRQ_RANGE_ALL
#undef JUMP_FRQ_RANGE_HALF
#undef JUMP_FRQ_RANGE



//=====================================================================
//
// ��������ģʽ
// ת�ؿ��ƣ��ٶȿ��ƣ�λ�ÿ���
//
//=====================================================================
void UpdateRunMode(void)
{
    static Uint16 runModeOld;

#if (DEBUG_F_POSITION_CTRL)
    if ((FUNCCODE_posCtrl_POSITION_CTRL == funcCode.code.posCtrl)   // λ�ÿ���
        || ((FUNCCODE_posCtrl_SWITCH_TO_PC == funcCode.code.posCtrl)    // �ٶ�/ת�ؿ���<->λ�ÿ���
            && (diFunc.f3.bit.posCtrl))
        || ((FUNCCODE_posCtrl_SWITCH_FROM_PC == funcCode.code.posCtrl)  // λ�ÿ���<->�ٶ�/ת�ؿ���
            && (!diFunc.f3.bit.posCtrl))
        )
    {
        if (FUNCCODE_motorCtrlMode_FVC == motorFc.motorCtrlMode)
        {
            runMode = RUN_MODE_POSITION_CTRL;   // λ�ÿ���
        }
        else if (runFlag.bit.run)
        {
            errorOther = 98;    // ��ǰ����FVC���򱨴�98
        }
    }
    else
#endif
    {
        // �ٶȿ���/ת�ؿ����л�
        runMode = funcCode.code.torqueCtrl ^ diFunc.f2.bit.SpdTorqSwitch;
        if (FUNCCODE_motorCtrlMode_VF == motorFc.motorCtrlMode)     // VF����
        {
            runMode = RUN_MODE_SPEED_CTRL;                  // �ٶȿ���
        }

        // ת�ؿ��ƽ�ֹ
        runMode = runMode & (~diFunc.f1.bit.forbidTorqueCtrl);

        // ת�ؿ���
        if (RUN_MODE_TORQUE_CTRL == runMode)
        {
			// �����趨ת��
			TorqueCalc();

            if (torqueAim >= 0)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;   // ת�ؿ����������Ƶ��
            }
            else
            {
                frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq;   // ת�ؿ��Ʒ������Ƶ��
            }
        }
        // ת�ؿ��� --> ��ת�ؿ���
        else if (RUN_MODE_TORQUE_CTRL == runModeOld)    
        {
            frqTmp = frqRun;
        }
    }
 
    runModeOld = runMode;
}



int32 FrqSrcOperate(void)
{
    int32 frq;
    Uint16 digit[5];
    GetNumberDigit1(digit, funcCode.code.frqRunCmdBind);
    runCmdBind = digit[runSrc];

    // ����Դ�󶨵�Ƶ��Դ
    if (runCmdBind > 0)
    {
        frqAiPu = maxFrq;
        frqFlag.bit.comp = 0;
        frq = FrqXyCalc(runCmdBind);
        frqXDisp = 0; // ��ʾ
        frqYDisp = 0;
    }
    // �ް�
    else
    {
        frq = GetFrq();

        frqXDisp = ABS_INT16(frqXy.x); // ��ʾ
        frqYDisp = ABS_INT16(frqXy.y);
    }
    return frq;
}



#define UP_DOWN_LIMIT_X 0
#define UP_DOWN_LIMIT_Y 1
#define UP_DOWN_LIMIT_NO_COMP 0
#define UP_DOWN_LIMIT_COMP 1
int32 frqMax;
int32 frqMin;
void getUpDownLimit(Uint16 comp, Uint16 type)
{
    int32 noUpDownFrq;
    int32 min;
    int32 upDownFrqPu;
    int32 minFrq;
    minFrq = frqMin;
    // Ϊ���(����+������ /  ������+����)
    if (comp == UP_DOWN_LIMIT_COMP)
    {
        //������+����
        if (type == UP_DOWN_LIMIT_Y)  // YΪ����
        {
            // X(������) + Y(����)
            noUpDownFrq = frqXy.x + frqXy.z;
        }
        // ����+������
        else if (type == UP_DOWN_LIMIT_X)  // XΪ����
        {
            noUpDownFrq = frqXy.y + frqXy.z + (int32)funcCode.code.presetFrq;
            //if (frqXy.y >= 0)
            {
                min = 0 + frqXy.y + frqXy.z;
                if (min > minFrq)     // ȡ��ֵ
                {
                    minFrq = min;
                }
            }
            #if 0
            else
            {
                max = 0 + frqXy.y - frqXy.z;
                if (max < frqMax)     // ȡ��ֵ
                {
                    frqMax = max;
                }
            }
            #endif    
            
        }
    }
    else
    {
        noUpDownFrq = frqXy.z + (int32)funcCode.code.presetFrq;
        minFrq = frqXy.z;
    }

    // ��Сֵ�޷�
    if (minFrq > frqMax)
    {
        minFrq = frqMax;
    }
    
    uPDownFrqMax = frqMax - noUpDownFrq;
    uPDownFrqMin = minFrq - noUpDownFrq;
    uPDownFrqLower1 = lowerFrq - noUpDownFrq;
    uPDownFrqLower2 = (int16)(-lowerFrq) - noUpDownFrq;

     // ����ʱ���㸨��Ƶ��Y(����)�ķ�Χ
    if ((comp == UP_DOWN_LIMIT_COMP) && (type == UP_DOWN_LIMIT_Y)) 
    {
        // ����ʱ����Ƶ��ԴY�ķ�Χ
        if (funcCode.code.frqYRangeBase)
        {
            upDownFrqPu  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
        }
        else
        {
            upDownFrqPu = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
        }

        if (uPDownFrqMax > upDownFrqPu)
        {
            uPDownFrqMax = upDownFrqPu;
        }

        if (uPDownFrqMin < (-upDownFrqPu))
        {
            uPDownFrqMin = -upDownFrqPu;
        }
    }
}

void ResetUpDownFrq(void)
{
    frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    upDownFrq = 0;
    uPDownFrqMax = 0;
    uPDownFrqMin = 0;
    uPDownFrqLower1= 0;
    uPDownFrqLower2= 0;

}

// ��ȡ������������֮���Ƶ��
int32 GetFrq(void)
{
    int32 frq;
//    static int upperFrqBak;
//    static int lowerFrqBak;

    frqMax = upperFrq;         // ���Ĭ��Ϊ ����Ƶ��
    frqMin = -(int32)upperFrq; // ��С��Ĭ��Ϊ -����Ƶ��

    
#if 0
    if ((upperFrqBak != upperFrqBak)
        || (lowerFrqBak != lowerFrq)
        )
    {
        ResetUpDownFrq();
    }
    
	upperFrqBak = upperFrq;
	lowerFrqBak = lowerFrq;
#endif

    frqFlag.bit.comp = 0;           // ������
    frqFlag.bit.upDown = 0;
    frqFlag.bit.x = 0;
    frqFlag.bit.y = 0;
    frqXy.x = 0;
    frqXy.y = 0;
    frqXy.z = 0;                    // ��û�е��ӣ���ֵ��ȻΪ0
    
    //switch ((funcCode.code.frqCalcSrc >> 0) & 0x000F)
    switch (funcCode.code.frqCalcSrc%10)
    {
        case FUNCCODE_frqCalcSrc_X: // ��Ƶ��ԴX
            frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
            
            break;

        case FUNCCODE_frqCalcSrc_COMPOSE: // ����������
            frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
            frqFlag.bit.y = 1;  // ����Ƶ��ԴY��Ч
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_Y: // �� <--> ��
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
            }
            else
            {
                frqFlag.bit.y = 1;  // ����Ƶ��ԴY��Ч
            }
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_COMPOSE: // �� <--> ����������
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
            }
            else
            {
                frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
                frqFlag.bit.y = 1;  // ����Ƶ��ԴY��Ч
            }
            
            break;

        case FUNCCODE_frqCalcSrc_Y_OR_COMPOSE: // �� <--> ����������
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.y = 1;  // ����Ƶ��ԴY��Ч
            }
            else
            {
                frqFlag.bit.x = 1;  // ��Ƶ��ԴX��Ч
                frqFlag.bit.y = 1;  // ����Ƶ��ԴY��Ч
            }
            break;

        default:
            break;
    }   

    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frqXy.z = funcCode.code.frqYOffsetFc; // ����Ƶ��ƫ��
        
        //if (((funcCode.code.frqCalcSrc >> 4) & 0x000F) == 0)
        if ((funcCode.code.frqCalcSrc/10) == 0)
        {
            if ((funcCode.code.frqXSrc >= FUNCCODE_frqXySrc_AI1) ||
                (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
            {
                frqFlag.bit.comp = 1;                 // ��comp��־
            }
        }
    }

    // X+YΪ���Ƶ��
    if (frqFlag.bit.comp)
    {
        // ����+������
        if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqY();  // ���㸨��Ƶ��Y
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_X);
            GetFrqX();  // ������Ƶ��X
        }
        // ������+����
        else if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqX();  // ������Ƶ��X
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_Y); 
            GetFrqY();  // ���㸨��Ƶ��Y
        }
        // ������+������
        else
        {
            GetFrqX();  // ������Ƶ��X
            GetFrqY();  // ���㸨��Ƶ��Y
        }
    }
    // ��
    else 
    {
        // ��
        if (frqFlag.bit.x)
        {
            if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_X); 
            }
            GetFrqX();  // ������Ƶ��X
        }
        
        // ��
        if (frqFlag.bit.y)
        {
            // ����Ƶ��Y�ķ�ΧΪX
            if (funcCode.code.frqYRangeBase)
            {
                GetFrqX();
            }
            
            if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_Y); 
            }
            
            GetFrqY();  // ���㸨��Ƶ��Y
        }   
    }
    
    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frq = GetFrqComp();
    }
    else if (frqFlag.bit.x)
    {
        frq = frqXy.x;
    }
    else if (frqFlag.bit.y)
    {
        frq = frqXy.y;
    }

    if (frq > frqMax)
    {
        frq = frqMax;
    }
    else if (frq < frqMin)
    {
        frq = frqMin;
    }

    // ��ֹ����Ƶ���л�ʱһ��Ϊ����һ����Ϊ����
    // �л�Ϊ������ʱfrqFlag.bit.upDown�޷���1,��ʱupDownFrq�ᱻ���
    if (frqFlag.bit.upDown)
    {
        if(!((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) || 
            (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            ))
        {
            frqFlag.bit.fcPosLimit = 0;
            frqFlag.bit.fcNegLimit = 0;
            frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
            upDownFrqTmp = 0;
            upDownFrq = 0;
            
        }
    }
    return frq;
}


// ��������
int32 GetFrqComp(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    int32 x;
    int32 y;   
    int32 frqYCompMax;   // ����Ƶ��Y����ʱ����

    // ����ʱ����Ƶ��ԴY�ķ�Χ
    if (funcCode.code.frqYRangeBase)
    {
        frqYCompMax  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
    }
    else
    {
        frqYCompMax = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
    }

    // YƵ���޷�
    if (frqXy.y > frqYCompMax)
    {
        frqXy.y = frqYCompMax;
    }

    
    //switch ((funcCode.code.frqCalcSrc >> 4) & 0x000F)
    switch (funcCode.code.frqCalcSrc/10)
    {
        case FUNCCODE_frqCalcSrc_ADD:   // �� + ��
            x = frqXy.x;

            // X+Yʱ��������Ƶ��ԴΪ���ָ���ʱ��Ԥ��Ƶ�ʣ�F0-08���������ã�UP/DOWN������
            if ((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) &&
                (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
            {
                frqXy.y = 0;
            }
            y = frqXy.y;
            
            frq = x + y;
            //frqXy.z = zFc;

#if 0
            if (frq > upperFrq)
            {
                frq = upperFrq;
                frqXy.y = upperFrq - x - z;
            }
            else if (frq < -(int32)upperFrq)
            {
                frq = -(int32)upperFrq;
                frqXy.y = -(int32)upperFrq - x - z;
            }
#endif
            break;

        case FUNCCODE_frqCalcSrc_SUBTRATION: // �� - ��
            x = frqXy.x;
            y = frqXy.y; 
            
            frq = x - y;
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MAX: // MAX(��, ��)
            x = frqXy.x;
            y = frqXy.y; 

            //frq = GetMax(x, y);
            
            if (ABS_INT32(x) >= ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }
            
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MIN: // MIN(��, ��)
            x = frqXy.x;
            y = frqXy.y; 
            
            if (ABS_INT32(x) < ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }

            //frqXy.z = zFc;
            break;

        default:
            break;
    }
    
    frq += frqXy.z;   // ����Ƶ��ƫ��
    
#if 0
    if (frq > upperFrq)
    {
        frq = upperFrq;
    }
    else if (frq < -(int32)upperFrq)
    {
        frq = -(int32)upperFrq;
    }
#endif

    return frq;
#endif
}



//===========================================================
// ��ȡ��Ƶ��X
//===========================================================
int32 GetFrqX(void)
{
    int32 frq;
    
    frqAiPu = maxFrq;
    //frqFlag.bit.x = 1;

    // ��Ƶ��ԴX��Ԥ��Ƶ���л�
    if (diFunc.f2.bit.frqXSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqXSrc);
    }

    frqXy.x = frq;     // ��ֵ
    
    return frq;
}


// ��ȡ��Ƶ��Y
int32 GetFrqY(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    
    // Ϊ����
    if (frqFlag.bit.comp)    
    {
        // ����ʱY��ΧΪX
        if (FUNCCODE_frqYRangeBase_FRQ_X == funcCode.code.frqYRangeBase)
        {
            // X(����) + Y(������)ʱ��Ԥ��Ƶ����ΪY������
            if ((funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1) 
                && (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
                )
            {
                frqAiPu = funcCode.code.presetFrq;
            }
            else
            {
                frqAiPu = ABS_INT32(frqXy.x);
            }   
        }
        // ����ʱY��ΧΪ���Ƶ��
        else
        {
            frqAiPu = maxFrq;
        }

        frqAiPu = ((Uint32)frqAiPu * funcCode.code.frqYRange) / 100;
    }
    // ��������Ƶ��
    else    
    {
        frqAiPu = maxFrq;
    }

    // ��Ƶ��ԴY��Ԥ��Ƶ���л�
    if (diFunc.f2.bit.frqYSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqYSrc);
    }        

    // ����ʱ����Ƶ��Y�޷�
    if (frqFlag.bit.comp)
    {
        if (frq > frqAiPu)
        {
            frq = frqAiPu;
        }
        else if (frq < -frqAiPu)
        {
            frq = - frqAiPu;
        }
    }
    
    frqXy.y = frq;     // ��ֵ
    
    return frq;
#endif
}


//=====================================================================
//
//
//=====================================================================
LOCALF int32 FrqXyCalc(Uint16 src)
{
    int32 tmp = 0;
    
    switch (src)         // �ǵ㶯���Ž���Ƶ��Դ����
    {
        case FUNCCODE_frqXySrc_FC:              // �������趨�����粻����
        case FUNCCODE_frqXySrc_FC_P_OFF_REM:    // �������趨���������
            tmp = FrqAimUpDownDeal();
            break;

        case FUNCCODE_frqXySrc_AI1:             // AI1
        case FUNCCODE_frqXySrc_AI2:             // AI2
        case FUNCCODE_frqXySrc_AI3:             // AI3
            tmp = src - FUNCCODE_frqXySrc_AI1;
            tmp = ((int32)aiDeal[tmp].set * frqAiPu + (1 << 14)) >> 15;
            break;

        case FUNCCODE_frqXySrc_PULSE:           // PULSE�����趨(DI5)
            tmp = ((int32)pulseInSet * frqAiPu + (1 << 14)) >> 15;
            break;

        case FUNCCODE_frqXySrc_MULTI_SET:       // ���ָ��
            tmp = UpdateMultiSetFrq(diFunc.f1.bit.multiSet);
            break;

        case FUNCCODE_frqXySrc_PLC:             // PLC
            tmp = FrqPlcSetDeal();
            break;

        case FUNCCODE_frqXySrc_PID:             // PID
            tmp = FrqPidSetDeal();
            break;

        case FUNCCODE_frqXySrc_COMM:            // ͨѶ�趨
            // funcCode.code.frqComm���ܳ���32767��Ŀǰ [-10000, +10000]

#if DEBUG_F_PLC_CTRL
            if (funcCode.code.plcEnable)  // PLC������Ч
            {
                tmp = (int32)(int16)funcCode.code.plcFrqSet * maxFrq / 10000;
            }
            else
#endif
#if DEBUG_F_P2P_CTRL
            if ((CanRxTxCon == P2P_COMM_SLAVE)   // ��Ե�ͨѶ�ӻ���Ч�ҽ���������ΪƵ�ʸ���
                && (funcCode.code.p2pRevDataSel == P2P_REV_FRQ_SET)
                )
            { 
                tmp = ((int32)(int16)p2pData.processValue * maxFrq) / 10000;
            }
            else
#endif
            if (funcCode.code.commProtocolSec == PROFIBUS)   // PROFIBUS-DPƵ�ʸ���
            {
                tmp = (int32)(Uint16)dpFrqAim;                    // ֱ�Ӹ���Ƶ��
            }
            else
            {
                tmp = (int32)(int16)funcCode.code.frqComm * maxFrq / 10000;
            }
            break;

        default:
            break;
    }

    return tmp;
}





