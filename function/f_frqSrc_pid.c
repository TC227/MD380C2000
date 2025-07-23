//======================================================================
//
// Ƶ��Դ����PID���� 
//
// Time-stamp: <2012-03-20 10:10:04  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_frqSrc.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"

#if F_DEBUG_RAM                         // �����Թ��ܣ���CCS��build option�ж���ĺ�
#define DEBUG_F_PID             0       // PID
#elif 1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_PID             1
#elif 1
#define DEBUG_F_PID             0
#endif

#endif


#define PID_ERROR_DEAD          20      // PID��С������Q15



PID_FUNC pidFunc = PID_FUNC_DEFAULTS;


#if DEBUG_F_PID
LINE_CHANGE_STRUCT pidFuncRef = LINE_CHANGE_STRTUCT_DEFALUTS;

#if 0
// ����ƫ��ı�PID����
struct PID_PARA_LINE
{
    LINE_STRUCT pLine;
    LINE_STRUCT iLine;
    LINE_STRUCT dLine;
};
struct PID_PARA_LINE pidParaLine = {LINE_STRTUCT_DEFALUTS, LINE_STRTUCT_DEFALUTS, LINE_STRTUCT_DEFALUTS};
#else
    LINE_STRUCT pidParaLine = LINE_STRTUCT_DEFALUTS;
#endif


#define PID_LPF_FDB 0   // PID�����˲�
#define PID_LPF_OUT 1   // PID����˲�
LowPassFilter pidLpf[2] = {LPF_DEFALUTS, LPF_DEFALUTS};


Uint32 pidInitTicker;
enum PID_CALC_FLAG
{
    PID_CALC_FLAG_NO_CALC,          // 0-��ֵ��������PID
    PID_CALC_FLAG_CALC,             // 1-����PID
    PID_CALC_FLAG_PID_INIT_HOLD,    // 2-PID��ֵ
    PID_CALC_FLAG_CALC_TRUE         // 3-��������PID
};
enum PID_CALC_FLAG pidCalcFlag;


Uint32 pidFdbLoseTicker;    // PID������ʧ����ticker
int32 pidError;             // PID ��ǰƫ��
Uint16 bPidFdbLose;


void UpdatePidFuncPara(PID_FUNC *p);
void UpdatePidFuncRefFdb(PID_FUNC *p);



//=====================================================================
//
// Ƶ��ԴΪPID
//
//=====================================================================
int32 FrqPidSetDeal(void)
{
    static Uint16 pidCalcFlagOld;
    int32 frqPid;
    int32 pidOut = pidFunc.out;

    runFlag.bit.pid = 1;

    UpdatePidFuncRefFdb(&pidFunc);  // pid����������ͣ��ʱҲҪ����
    UpdatePidFuncPara(&pidFunc);

    if ((FUNCCODE_pidCalcMode_YES == funcCode.code.pidCalcMode) // ͣ��ʱ����
            || (runCmd.bit.common))   // ʹ������������Ч
         // || (runFlag.bit.common)) 
    {
        if (PID_CALC_FLAG_NO_CALC == pidCalcFlag)
        {
            pidCalcFlag = PID_CALC_FLAG_CALC;
        }
    }
    else
    {
        pidCalcFlag = PID_CALC_FLAG_NO_CALC;
    }

// �Ƿ�Ϊ��1������PID
    if ((PID_CALC_FLAG_CALC == pidCalcFlag)
        && (!pidCalcFlagOld)
        )
    {
        pidCalcFlag = PID_CALC_FLAG_PID_INIT_HOLD;
        
        pidInitTicker = 0;
    }

    // PID��ֵ
    if (PID_CALC_FLAG_PID_INIT_HOLD == pidCalcFlag)
    {
        if (++pidInitTicker >= (int32)funcCode.code.pidInitTime * (TIME_UNIT_PID_INIT / PID_CALC_PERIOD))
        {
            pidCalcFlag = PID_CALC_FLAG_CALC_TRUE;
        }
        
        pidFunc.out = (int32)funcCode.code.pidInit * 0x7fff / 1000;
        pidOut = pidFunc.out;
    }

    // ����PIDƫ��
    pidError = pidFunc.ref - pidFunc.fdb;    
    if (funcCode.code.pidDir^diFunc.f2.bit.pidDirRev)   // PID���÷���
    {
        pidError = -pidError;
    }

    // PID����
    if ((PID_CALC_FLAG_CALC_TRUE == pidCalcFlag)
        && (!diFunc.f1.bit.pidPause)    // PID��ͣ������Ч
        && (!runFlag.bit.jog)           //+= �����е㶯����ʱ�����㡣?
        && ((ABS_INT32(pidError) > pidFunc.errorDead) )    // ����ƫ��޲Ž���PID���ڼ���
        )
    {
     
        pidFunc.calc(&pidFunc);         // PID����

        // PID����˲�
        pidLpf[PID_LPF_OUT].t = (int32)funcCode.code.pidOutLpfTime * (TIME_UNIT_PID_FILTER / PID_CALC_PERIOD); // �˲�
        pidLpf[PID_LPF_OUT].in = pidFunc.out;
        pidLpf[PID_LPF_OUT].calc(&pidLpf[PID_LPF_OUT]);
        pidOut = pidLpf[PID_LPF_OUT].out;
    }

    // ����PIDƫ���¼
    pidFunc.error2 = pidFunc.error1;
    pidFunc.error1 = pidFunc.error;
    pidFunc.error = pidError;

    
    if ((!pidCalcFlag)                      // ͣ��ʱ�����㣬ͣ��ʱ����
        || (frqCalcSrcOld != frqCalcSrc)    // ������Ƶ��Դ�л���PID��PID����
        || ((plcStepOld) && (!plcStep))     // PLC�ĵ�0��ʹ��PID�����½���plc��0��
        )
    {
        pidFunc.deltaPRem = 0;
        pidFunc.deltaIRem = 0;
        pidFunc.deltaDRem = 0;
        pidFunc.deltaRemainder = 0;
        pidFunc.out = 0;
		pidLpf[PID_LPF_OUT].out = 0;
		pidLpf[PID_LPF_OUT].remainder = 0;
        pidOut = 0;
    }
    
    frqPid = (pidOut * maxFrq + (1 << 14)) >> 15;

    pidCalcFlagOld = pidCalcFlag;
    
    return frqPid;
}



//=====================================================================
//
// PID����
//
//=====================================================================

void PidFuncCalc(PID_FUNC *p)
{
    static Uint16 pidChgOld;
    int32 Kp, Ki, Kd;
    int32 deltaP, deltaI, deltaD;   // P,I,D�������ֵ�delt

// Kp,Ki,Kd��ֵ
    Kp = p->Kp;
    Ki = p->Ki;
    Kd = p->Kd;
    
// PID�����л�
    if (FUNCCODE_pidParaChgCondition_DI  == funcCode.code.pidParaChgCondition)   // DI����
    { 
        if (diFunc.f2.bit.pidChg)   // DI-PID�����л� ������Ч
        {
            Kp = p->Kp2;
            Kd = p->Kd2;
            Ki = p->Ki2;
        }

        if(diFunc.f2.bit.pidChg != pidChgOld)
        {
            int32 tmp1,tmp2;

            tmp1 = (Kp * (pidError - p->error));
            tmp2 = (Kd * (pidError - p->error2 + 3 * (p->error - p->error1)));
                
            if(p->Qi >= p->Qp)
            {
                tmp1 = tmp1<<(p->Qi - p->Qp);
            }
            else
            {
                tmp1 = tmp1>>(p->Qp - p->Qi);
            }

            if(p->Qi >= p->Qd)
            {
                tmp2 = tmp2<<(p->Qd - p->Qi);
            }
            else
            {
                tmp2 = tmp2>>(p->Qd - p->Qi);
            }
            
            Ki = (p->delta<<p->Qi - tmp1 - tmp2 )/((pidError + p->error) >> 1);
        }
        
        pidChgOld = diFunc.f2.bit.pidChg;
    }
    else if (FUNCCODE_pidParaChgCondition_PID_ERROR == funcCode.code.pidParaChgCondition)   // ����ƫ���Զ��л�
    {
        pidParaLine.x1 = p->errorSmall;
        pidParaLine.x2 = p->errorBig;
        
        pidParaLine.y1 = p->Kp;
        pidParaLine.y2 = p->Kp2;
        pidParaLine.x = ABS_INT32(pidError); // p->error���ᳬ��0xffff
        pidParaLine.calc(&pidParaLine);
        Kp = pidParaLine.y;

        pidParaLine.y1 = p->Ki;
        pidParaLine.y2 = p->Ki2;
        pidParaLine.x = ABS_INT32(pidError); // p->error���ᳬ��0xffff
        pidParaLine.calc(&pidParaLine);
        Ki = pidParaLine.y;

        pidParaLine.y1 = p->Kd;
        pidParaLine.y2 = p->Kd2;
        pidParaLine.x = ABS_INT32(pidError); // p->error���ᳬ��0xffff
        pidParaLine.calc(&pidParaLine);
        Kd = pidParaLine.y;
    }
   

// ����deltaP
    p->deltaPRem += (int64)Kp * (pidError - p->error);     // Qp + 15
    deltaP = p->deltaPRem >> p->Qp;                     // Q15
    p->deltaPRem = p->deltaPRem - (deltaP << p->Qp);    // ������ֵ���´μ���ʹ��
    
// ����deltaI
    if (( (((p->out >= p->outMax) && (pidError > 0)) 
        || ((p->out < p->outMin) && (pidError < 0))
        || (frqFlag.bit.frqSetLimit))             // �Ѿ��ﵽ���ֵ����I��������
         && (funcCode.code.pidIAttribute/10))     // PID�������������ֵʱֹͣ����
        || ((diFunc.f2.bit.pidPauseI) && (funcCode.code.pidIAttribute%10))) // PID������ͣ
    {
        deltaI = 0;
        p->deltaIRem = 0;
    }
    else
    {
        p->deltaIRem += ((int64)Ki * ((pidError + p->error) >> 1));    // Qi + 15
        deltaI = p->deltaIRem >> p->Qi;   // Q15
        p->deltaIRem = p->deltaIRem - (deltaI << p->Qi);
    }

// ����deltaD
// �ĵ����Ĳ�ַ� ��ֱ�Ӳ��õ�ǰ���e(k)�������ù�ȥ�������ĸ�����ʱ�̵�����ƽ��ֵ��Ϊ΢�ֻ�׼
// e(k) = [e(k) + e(k-1) + e(k-2) + e(k-3)]/4
    p->deltaDRem += (int64)Kd * (pidError - p->error2 + 3 * (p->error - p->error1));
    deltaD = p->deltaDRem >> p->Qd;
    // ΢���޷�
    if((deltaD > 0) && (deltaD > p->pidDLimit))
    {
        deltaD = p->pidDLimit;
    }
    else if ((deltaD < 0) && (deltaD < (-p->pidDLimit)))
    {
        deltaD = (-p->pidDLimit);
    }
    p->deltaDRem = p->deltaDRem - (deltaD << p->Qd);

// ����delta
    p->delta = deltaP + deltaI + deltaD; // Q15

    p->delta += p->deltaRemainder;      // ��һ�ε�delta�޷���ֵ
    if (p->delta > p->deltaMax)
    {
        p->deltaRemainder = p->delta - p->deltaMax;  // ����delta�޷���ֵ���´μ���ʹ��
        p->delta = p->deltaMax;
    }
    else if (p->delta < p->deltaMin)
    {
        p->deltaRemainder = p->delta - p->deltaMin;
        p->delta = p->deltaMin;
    }
    else
    {
        p->deltaRemainder = 0;
    }

// ����out, out�޷�
    if (frqFlag.bit.frqSetLimit)
    {
        if (((p->out > 0) && (p->delta < 0))
            || ((p->out < 0) && (p->delta > 0))
            )
        {
            p->out += p->delta;
        }
        else
        {
            p->deltaRemainder = 0;   // ���������ƫ���޷���ֵ
            p->deltaPRem = 0;            // �������ֵ
            p->deltaDRem = 0;            // ��΢���޷���ֵ
        }
    }
    else
    {
        p->out += p->delta;
    }

    if (p->out > p->outMax)
    {
        p->out = p->outMax;
        p->deltaRemainder = 0;   // ���������ƫ���޷���ֵ
        p->deltaPRem = 0;            // �������ֵ
        p->deltaDRem = 0;            // ��΢���޷���ֵ
    }
    else if (p->out < p->outMin)
    {
        p->out = p->outMin;
        p->deltaRemainder = 0;   // ���������ƫ���޷���ֵ
        p->deltaPRem = 0;            // �������ֵ
        p->deltaDRem = 0;            // ��΢���޷���ֵ
    }
	
}


//=====================================================================
//
// PID��������
//
// ��֤:
//      2^9  < p->Kp <= 2^14�� 1000 - 100.0% - 1 - 1*2^Qp
//      2^11 < p->Ki <= 2^15,
//
//=====================================================================
LOCALF void UpdatePidFuncPara(PID_FUNC *p)
{
    Uint32 Kp = (((Uint32)funcCode.code.pidKp << 16) / 1000) << 8; // Q24����ʱҪ��֤������2^29����pidKp<=2^5*1000
    Uint32 Ki = (((((((Uint32)funcCode.code.pidKp * PID_CALC_PERIOD) << 14) / 100) << 6) / 100) << 6)
            / funcCode.code.pidTi; // Q26����ʱҪ��֤С��2^30
//    Uint32 Kd;
    Uint16 Qp, Qi, Qd;
    int32 outMax;
    int32 outMin;

// Kp��Qp
    Qp = 24;
    while (Kp >= 1L << 15)
    {
        Kp >>= 1;
        Qp--;
    }

    if (p->Qp > Qp)             // ����µ�Qp��С�ˣ���֮ǰ����ֵ����
    {
        p->deltaPRem = 0;
    }
    p->Qp = Qp;
    p->Kp = Kp;

// Ki��Qi
    Qi = 26;
    while (Ki >= 1L << 16)
    {
        Ki >>= 1;
        Qi--;
    }

    if (p->Qi > Qi)
    {
        p->deltaIRem = 0;
    }
    p->Qi = Qi;
    p->Ki = Ki;

// Kd, Qd
    Qd = 12;
    p->Kd = (((((Uint32)funcCode.code.pidKp * funcCode.code.pidTd) << 6) / 5) << 3) / (25*PID_CALC_PERIOD); // Q12
    p->Qd = Qd;

    // errorDead��������С����Ϊ PID_ERROR_DEAD
    p->errorDead = ((Uint32)funcCode.code.pidErrMin << 15) / 1000;// + PID_ERROR_DEAD; // Q15

    // 1. ������ΪAIʱ��499.5mv-500.4999mv֮�䶼����ʾΪ500mv��
    // Ϊ������ʾ500mvʱ��PID������������/�仯��������˴���
    // 2. ����ref = 50%��pidErrMin = 20%��Ҫ��AI����Ϊ300mv/700mvʱ��PID���û�б仯��������˴���
    if (funcCode.code.pidFdbSrc <= FUNCCODE_pidFdbSrc_AI1_SUB_AI2)
    {
        p->errorDead += PID_ERROR_DEAD;  // ������AIʱ����PID����
    }


// PID���������
// PID�����ã��������ڸ�����Ƶ�ʵ�����Ƶ�ʣ��ٷ������ڸ���������������Ƶ���������С�
    outMax = 0x7FFF;            // 100.0%
    if (upperFrq < maxFrq)      // ����Ƶ��
    {
        outMax = ((int32)upperFrq << 15) / maxFrq;
    }
    
    // PIDΪ����Ƶ��������Ƶ�ʳ�������Ƶ�ʷ�Χ
    if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
       && (upperFrq > frqAiPu))   
    {
        outMax = ((int32)frqAiPu << 15) / maxFrq;
    }
    
    p->outMax = outMax;

// PID���������
// PID�����ã��������ڸ�����Ƶ�ʵ�����Ƶ�ʣ��ٷ������ڸ���������������Ƶ���������С�
    outMin = lowerFrq;
    if (0 == funcCode.code.reverseCutOffFrq)    // ��ֹ��ת���ֹƵ��Ϊ0��PID����Ϊ����Ƶ��
    {
        ;
    }
    // ��ֹƵ�ʲ�Ϊ0��PID����: ����Ƶ�ʺͷ�ת��ֹƵ�ʵ�max����ȡ����
    else
    {
        if (outMin < funcCode.code.reverseCutOffFrq)
        {
            outMin = funcCode.code.reverseCutOffFrq;
        }

        // PIDΪ����Ƶ��������Ƶ�ʳ�������Ƶ�ʷ�Χ
        if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
            && (outMin > frqAiPu))
        {
            outMin = frqAiPu;
        }
        
        outMin = -outMin;
    }
    p->outMin = (outMin << 15) / maxFrq;


// PID�������Сֵ
//    p->outMin = -(int32)funcCode.code.reverseCutOffFrq * 0x7FFF / maxFrq; // ��ת��ֹƵ��

// ΢���޷�
    p->pidDLimit =  (Uint32)funcCode.code.pidDLimit * 53687>>14;  //0x7FFF / 10000;

// PID���ƫ������ֵ����Сֵ
    p->deltaMax = (Uint32)funcCode.code.pidOutDeltaMax * 53687>>14;  //0x7FFF / 10000;
    p->deltaMin = -(int32)((Uint32)funcCode.code.pidOutDeltaMin * 53687>>14);  //0x7FFF / 10000;

    // Kp2
    if (Qp >= 16)
    {
        p->Kp2 = (((Uint32)funcCode.code.pidKp2 << 16) / 1000) << (Qp - 16);
    }
    else
    {
         p->Kp2 = (((Uint32)funcCode.code.pidKp2 << Qp) / 1000);
    }

    // Ki2
    if (Qi >= 20)
    {
        p->Ki2 = (((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) / 100) << 6) / 100) << (Qi - 20)) / funcCode.code.pidTi2; // Q26����ʱҪ��֤С��2^30
    }
    else if(Qi >= 14)
    {
        p->Ki2 = ((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) / 100) << (Qi - 14)) / 100) / funcCode.code.pidTi2; // Q26����ʱҪ��֤С��2^30
    }
    else
    {
       p->Ki2 = ((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << Qi) / 10000) / funcCode.code.pidTi2; // Q26����ʱҪ��֤С��2^30 
    }

    // Kd2
    p->Kd2 = (((((Uint32)funcCode.code.pidKp2 * funcCode.code.pidTd2) << 6) / 5) << 3) / (25*PID_CALC_PERIOD); // Q12

    // PID�����л�ƫ��
	p->errorSmall = (int32)funcCode.code.pidParaChgDelta1 * 0x7FFF / 1000;
    p->errorBig   = (int32)funcCode.code.pidParaChgDelta2 * 0x7FFF / 1000;
   

}



//=====================================================================
//
// PID��������������
//
// �����p->ref, Q15, 100.0% -- 2^15
//       p->fdb, Q15, 100.0% -- 2^15
//
//=====================================================================
LOCALF void UpdatePidFuncRefFdb(PID_FUNC *p)
{
    int32 aimValue;
    int32 fdb;
    int16 i;

// PID����
    switch (funcCode.code.pidSetSrc) // PID����Դ
    {
        // �������趨
        case FUNCCODE_pidSetSrc_FC:
            aimValue = (int32)(int16)funcCode.code.pidSet * 2097 >> 6;  // 0x7fff/1000 (Q15)
            break;

        // AI1\AI2\AI3
        // AI�趨����ΪQ15
        case FUNCCODE_pidSetSrc_AI1:
        case FUNCCODE_pidSetSrc_AI2:
        case FUNCCODE_pidSetSrc_AI3:
            i = funcCode.code.pidSetSrc - FUNCCODE_pidSetSrc_AI1;
            aimValue = aiDeal[i].set;
            break;

        // ��������
        // ����ΪQ15
        case FUNCCODE_pidSetSrc_PULSE:
            aimValue = pulseInSet;
            break;

        // ͨѶ
        case FUNCCODE_pidSetSrc_COMM:
            aimValue = (int32)(int16)funcCode.code.frqComm * 13421 >> 12;  // 0x7FFF/10000
            break;

        // ������趨
        case FUNCCODE_pidSetSrc_MULTI_SET:
            aimValue = UpdateMultiSetFrq(diFunc.f1.bit.multiSet);
            aimValue = aimValue * 0x7fff / (int32)maxFrq;               // PID��100%��Ӧ���Ƶ��
            break;
            
        default:
            break;
    }


#if 0  // ��������С��0
    // ��������С��0
    if (aimValue < 0)
    {
        aimValue = 0;
    }
#endif

// PID�����仯ʱ��
    pidFuncRef.aimValue = aimValue;
    pidFuncRef.tickerAll = (Uint32)funcCode.code.pidSetChangeTime * (TIME_UNIT_PID_SET_CHANGE / PID_CALC_PERIOD);
    pidFuncRef.maxValue = 0x7fff;
    pidFuncRef.curValue = p->ref;
    pidFuncRef.calc(&pidFuncRef);
    p->ref = pidFuncRef.curValue;

    // PID����Դ
    switch (funcCode.code.pidFdbSrc) // PID����ֵ
    {
        // AI1\AI2\AI3
        case FUNCCODE_pidFdbSrc_AI1:
        case FUNCCODE_pidFdbSrc_AI2:
        case FUNCCODE_pidFdbSrc_AI3:
            fdb = aiDeal[funcCode.code.pidFdbSrc - FUNCCODE_pidFdbSrc_AI1].set;
            break;

        // AI1-AI2
        case FUNCCODE_pidFdbSrc_AI1_SUB_AI2:    // ��ʹ |aiDeal[0].set - aiDeal[1].set| > 1Ҳû�����⡣
            fdb = (int32)aiDeal[0].set - aiDeal[1].set;
            break;

        // ��������
        case FUNCCODE_pidFdbSrc_PULSE:
            fdb = pulseInSet;
            break;

        // ͨѶ
        case FUNCCODE_pidFdbSrc_COMM:
            fdb = (int32)(int16)funcCode.code.frqComm * 13421 >> 12;  // 0x7FFF/10000
            break;

        // AI1+AI2
        case FUNCCODE_pidFdbSrc_AI1_ADD_AI2:
            fdb = (int32)aiDeal[0].set + aiDeal[1].set;
            break;

        // MAX(AI1,AI2)
        case FUNCCODE_pidFdbSrc_MAX_AI:
            fdb = GetMax(ABS_INT32(aiDeal[0].set), ABS_INT32(aiDeal[1].set));
            break;

        // MIN(AI1,AI2)
        case FUNCCODE_pidFdbSrc_MIN_AI:
            fdb = GetMin(ABS_INT32(aiDeal[0].set), ABS_INT32(aiDeal[1].set));
            break;

        default:
            break;
    }

#if 0   // ��������С��0   
    // ��������С��0
    if (fdb < 0)
    {
        fdb = 0;
    }
#endif    
    // �������ܴ���100%
    if (fdb > 0x7fff)
    {
        fdb = 0x7fff;
    }

    // PID������ʧ���
    if (PID_CALC_FLAG_CALC_TRUE == pidCalcFlag)
    {
        // ������ʧ���ֵ��Ϊ0�ҷ���С�ڼ��ֵ�Ҵ�������״̬
        if ((funcCode.code.pidFdbLoseDetect) && (runFlag.bit.run) && (fdb < (int32)funcCode.code.pidFdbLoseDetect * 0x7fff / 1000))
        {
            if (pidFdbLoseTicker < (int32)funcCode.code.pidFdbLoseDetectTime * (TIME_UNIT_PID_FDB_LOSE / PID_CALC_PERIOD))
            {
                pidFdbLoseTicker++;
            }
            else
            {
                bPidFdbLose = 1;

                // ����ʱPID������ʧ
                errorOther = ERROR_FDB_LOSE;
            }
        }
        else
        {
            bPidFdbLose = 0;
            pidFdbLoseTicker = 0;
        }
    }
    else
    {
        pidFdbLoseTicker = 0;
    }
        

// PID�����˲�
    pidLpf[PID_LPF_FDB].t = (int32)funcCode.code.pidFdbLpfTime * (TIME_UNIT_PID_FILTER / PID_CALC_PERIOD); // �˲�
    pidLpf[PID_LPF_FDB].in = fdb;
    pidLpf[PID_LPF_FDB].calc(&pidLpf[PID_LPF_FDB]);
    p->fdb = pidLpf[PID_LPF_FDB].out;
}

#elif 1

int32 FrqPidSetDeal(void){}
void PidFuncCalc(PID_FUNC *p){}

#endif






