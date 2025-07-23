//======================================================================
//
// S����
//
// Time-stamp: <2012-02-28 20:46:14  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_main.h"
#include "f_menu.h"
#include "f_debug.h"

#if F_DEBUG_RAM                         // �����Թ��ܣ���CCS��build option�ж���ĺ�

#define DEBUG_F_S_CURVE_A         0       // S����A
#define DEBUG_F_S_CURVE_B         0       // S����B

#elif 1

#define DEBUG_F_S_CURVE_A         1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_S_CURVE_B         1
#elif 1
#define DEBUG_F_S_CURVE_B         1
#endif

#endif

#define ACC_DEC_DEBUG 1         // debug acc-dec 
enum S_CURVE_A_STATUS
{
    SA_STEP_START_S,
    SA_STEP_LINE,
    SA_STEP_END_S,
    SA_STEP_OK
};
enum S_CURVE_B_STATUS
{
    SB_STEP_START_S,
    SB_STEP_LINE,
    SB_STEP_END_S,
    SB_STEP_OK
};

                               
#define LINE_FRQ_CHANGE_STRTUCT_DEFALUTS   \
{                                          \
	(void (*)(void *))LineChangeCalc       \
}

LINE_CHANGE_STRUCT frqLine = LINE_FRQ_CHANGE_STRTUCT_DEFALUTS;

int32 frqCurAimOld;
int32 accTimeOld;
int32 decTimeOld;
// Ŀ��Ƶ�ʷ����ı��־��Ӧ����S���ߵ�ʱ����Ч��־
// ����ʱ��ҲӦ��Ϊ1
Uint16 bFrqCurAimChg;

LOCALD int32 SCurveOridinaryCalc(int32 frq0, int32 accDecTime);
LOCALD int32 SCurveSpecialCalc(int32 frq0, int32 accDecTime);

//=====================================================================
//
// �Ӽ��ټ���
// ������accTime -- ����ʱ�䣬��λ: ͬ������
//       decTime -- ����ʱ�䣬��λ: ͬ������
//       mode    -- �Ӽ��ٷ�ʽ��ֱ�߼Ӽ���/S���߼Ӽ���
// ���룺
//       frqTmp -- ��ǰ˲ʱƵ��
//       frqCurAim -- Ŀ��Ƶ��
// �����
//       frqTmp -- ��ǰ˲ʱƵ��
//
// ע�⣺frqCurAimһ��Ҫ��ȷ������
//
//=====================================================================
void AccDecFrqCalc(int32 accTime, int32 decTime, Uint16 mode)
{
    int32 accDecTime; 
    int32 frq0;
   
    if (((funcCode.code.ovGain) || (funcCode.code.ocGain))
        && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)   // VF����
        && (motorFc.motorPara.elem.motorType != MOTOR_TYPE_PMSM)  // ��ͬ����
        && (!funcCode.code.droopCtrl)                             // �´�������Ч
        && (runStatus != RUN_STATUS_TUNE)                         // �����ڵ�г״̬
        && (!((frqCurAim == 0) && (ABS_INT32(frqRun) <= 50)))       // ��ΪĿ��Ƶ��Ϊ0������Ƶ��С��0.5Hz���
    ) // ��ѹʧ�٣�����ʧ�����涼Ϊ0
	{
		frq0 = frqRun;
    }
    else
    {
        frq0 = frqTmp;
		frqVFRunRemainder = 0;
    }

    
    if (frqCurAimOld != frqCurAim)  // Ŀ��Ƶ�ʷ����ı�
    {
        frqCurAimOld = frqCurAim;
        bFrqCurAimChg = 1;
    }

    if (frqCurAim == frq0)                      // ����
    {
        runFlag.bit.accDecStatus = CONST_SPEED;
        frqTmpFrac = frqCurAimFrac;             // ˲ʱƵ�ʵ�С����   
    }
    else
    {
        // ����
        if (((frq0 >= 0) && (frqCurAim > frq0))
            || ((frq0 <= 0) && (frqCurAim < frq0))) 
        {
            runFlag.bit.accDecStatus = ACC_SPEED;
            accDecTime = accTime;
         	benchFrq = ABS_INT32(frqCurAim);
            if (accTimeOld != accTime)
            {
                bFrqCurAimChg = 1;
            }
        }
         // ����
        else                   
        {
            runFlag.bit.accDecStatus = DEC_SPEED;
            accDecTime = decTime;
            if (decTimeOld != decTime)
            {
                bFrqCurAimChg = 1;
            }
        }


        // ��׼Ƶ��
        if (funcCode.code.accDecBenchmark == 0)
        {    
            benchFrq = maxFrq;    // ���Ƶ��
        }
        else if (funcCode.code.accDecBenchmark == 2)
        {
            benchFrq = 100 * decNumber[funcCode.code.frqPoint]; // 100Hz
        }
        
#if DEBUG_F_S_CURVE_A || DEBUG_F_S_CURVE_A
        // ����S���ߣ��Ӽ���ʱ�䲻��̫������ͨ���η�S���ߣ�û�д�����
        if ((FUNCCODE_accDecSpdCurve_S_CURVE_B == mode) && 
            ((accDecTime > (int32)100 * TIME_UNIT_MS_PER_SEC / timeBench)    // �Ӽ���ʱ�����_s
            || (ABS_INT32(frqCurAim) > 6 * motorFc.motorPara.elem.ratingFrq))) // �趨Ƶ�ʴӴ���_�Ƶ��
        {
            mode = FUNCCODE_accDecSpdCurve_LINE;
        }
#endif
	    accDecTime = accDecTime * (timeBench / RUN_CTRL_PERIOD);// ת����ticker
        //accDecTime = accDecTime * (timeBench * (Uint16)(1 / RUN_CTRL_PERIOD));// ת����ticker
        
        if (FUNCCODE_accDecSpdCurve_LINE == mode) // ֱ�߼Ӽ���
        {
            frqLine.aimValue = frqCurAim;
            frqLine.tickerAll = accDecTime;
            frqLine.maxValue = benchFrq;
            frqLine.curValue = frq0;
            frqLine.calc(&frqLine);
            frq0 = frqLine.curValue;
            //frqTmpFrac = ((int64)frqLine.remainder << 15) / frqLine.tickerAll;
        }
#if DEBUG_F_S_CURVE_A 
        else if (FUNCCODE_accDecSpdCurve_S_CURVE_A == mode)     // ��ͨ���η�S����
        {
            frq0 = SCurveOridinaryCalc(frq0, accDecTime);
        }
#endif
#if DEBUG_F_S_CURVE_B        
        else    // �����S����
        {
            frq0 = SCurveSpecialCalc(frq0, accDecTime);
        }
#endif

    }

    bFrqCurAimChg = 0;
    decTimeOld = decTime;
    accTimeOld = accTime;
    frqTmp = frq0;
}




//=====================================================================
//
// ��ͨ���η�S���߼Ӽ��ټ���
// ������accDecTime -- �Ӽ���ʱ�䣬��λ: ticker
//
// ���룺
//       frq0 -- ��һ��˲ʱƵ��
//       frqCurAim -- Ŀ��Ƶ�ʣ�ȫ�ֱ���
// �����
//       frq0 -- ��ǰ��˲ʱƵ��
//
// ע�⣺frqCurAimһ��Ҫ��ȷ������
//
//       S���߼���ʱ�� = ֱ�߼Ӽ���ʱ��
//
//=====================================================================
LOCALF int32 SCurveOridinaryCalc(int32 frq0, int32 accDecTime)
{
#if DEBUG_F_S_CURVE_A
    static enum S_CURVE_A_STATUS sCurveAStatus;
    static int32 ticker;
    static int32 t[3];      // S������ʼ�Ρ�ֱ�߶Ρ�S���߽����ε�ʱ��, ticker
    static int32 a[3];      // S������ʼ�Ρ�ֱ�߶Ρ�S���߽����ε�ϵ��
    static Uint16 q[3];     // a[]��Ӧ��Q��ʽ
    static int64 remainder;
    int32 delta;
    int64 tmp;
    int32 tmp2;

    if (!accDecTime)    // �Ӽ���ʱ��Ϊ0
    {
        frq0 = frqCurAim;
        frqTmpFrac = 0;
        return frq0;
    }

    if (bFrqCurAimChg)
    {
        int32 tickerAll;  // �����׶���Ҫ��ʱ�䣬���ô���Ϊ��λ
        Uint16 startTime;
        Uint16 lineTime;   // ֱ�߶�ʱ���2��
        Uint16 endTime;
        int32 frqAimDelta;
        int64 tmp1;
        int64 tmp3;
        
        sCurveAStatus = SA_STEP_START_S;
        ticker = 0;
        remainder = 0;

        startTime = funcCode.code.sCurveStartPhaseTime;
        endTime = funcCode.code.sCurveEndPhaseTime;
        lineTime = 2000 - startTime - endTime;
        frqAimDelta = frqCurAim - frq0;
#define Q   16
        tmp3 = ((int64)accDecTime * ABS_INT32(frqAimDelta) << Q) / (int32)benchFrq;
        tickerAll = tmp3 >> Q;
        t[0] = ((int64)tickerAll * startTime + 0) / 1000;  //186.
        t[2] = ((int64)tickerAll * endTime + 0) / 1000;    //407.
        t[1] = tickerAll - (t[0] + t[2]);

        q[0] = 20;  // ��ʼ��a[0], a[1]��Ӧ��Q��ʽ
        q[1] = 35;

        // ����a[1], ֱ�߶ε�һ����ϵ����ͬʱ���¶�Ӧ��Q��ʽ
        tmp = ((int64)benchFrq * 2000 << q[1]) / ((int64)accDecTime * lineTime); //310.
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[1]--;
        }
        a[1] = tmp;
        
        // ����a[0]��S������ʼ�εĶ�����ϵ��
        tmp1 = ((int64)a[1] * 500 << (q[0] + Q)) / tmp3;
#undef Q
        q[0] += q[1];
        q[2] = q[0];
        tmp = tmp1 / startTime;
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[0]--;
        }
        a[0] = tmp;
        
        // ����a[2]��S���߽����εĶ�����ϵ��
        tmp = tmp1 / endTime;
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[2]--;
        }
        a[2] = tmp;
    }
    
    if (sCurveAStatus <= SA_STEP_END_S)
    {
        if (ticker >= t[sCurveAStatus])     // �ö��Ѿ����
        {
            ticker = 0; // ��ʼ����0
            sCurveAStatus++;
            
            // ��һ��ʱ��Ϊ0
            while ((t[sCurveAStatus] == 0) && (SA_STEP_OK != sCurveAStatus))
            {
                sCurveAStatus++;
            }
            
            remainder = 0;
        }
        
        ticker++;
        
        if (SA_STEP_START_S == sCurveAStatus)
        {
            tmp2 = 2 * ticker - 1;
        }
        else if (SA_STEP_LINE == sCurveAStatus)
        {
            tmp2 = 1;
        }
        else //if (SA_STEP_END_S == sCurveAStatus)
        {
            tmp2 = 2 * t[2] - 2 * ticker + 1;     // ��ϸ�Ƶ�������Ϊ+1, ������-1/0
        }
        
        tmp = (int64)tmp2 * a[sCurveAStatus] + remainder;
        delta = tmp >> q[sCurveAStatus];
        remainder = tmp - ((int64)delta << q[sCurveAStatus]);

        if (q[sCurveAStatus] >= 15)
        {
            frqTmpFrac = remainder >> (q[sCurveAStatus] - 15);
        }
        else
        {
            frqTmpFrac = remainder << (15 - q[sCurveAStatus]);
        }
        
        if (frq0 < frqCurAim)
        {
            frq0 += delta;
            if (frq0 > frqCurAim)   // ��ֹ����Ŀ��Ƶ��
            {
                frq0 = frqCurAim;
            }
        }
        else
        {
            frq0 -= delta;
            frqTmpFrac = -frqTmpFrac;
            
            if (frq0 < frqCurAim)   // ��ֹ����Ŀ��Ƶ��
            {
                frq0 = frqCurAim;
            }
        }
    }
    
    if (SA_STEP_OK == sCurveAStatus)
    {
        frq0 = frqCurAim;
    }
#endif
    return frq0;
}


//=====================================================================
//
// �����S���ߣ��ο�����S����B
// 
//=====================================================================
LOCALF int32 SCurveSpecialCalc(int32 frq0, int32 accDecTime)
{
#if DEBUG_F_S_CURVE_B
    static enum S_CURVE_B_STATUS sCurveBStatus;
    const int16 m1 = 13848;        // Q15, m = 1/(16*0.7)+1/3, Ҳ������5/12
    const int16 fm = 11483;        // Q15, (9*m-1)/8
    //int32 A = 688;
    //int32 B = 530;
    //int16 Sden = 1000;
    const int16 A = 21;
    const int16 B = 16;
    const int16 Sden = 30;
    //  A = 750;
    //  B = 500;
    static int32 ticker;
    static int32 tickerAll1;  // �յ�ǰ��S���ߵ�tickerAll
    static int32 tickerAll2;
    static int32 tickerAll3;
    static int32 inflexionFrq;            // �յ�
    static int32 pointFrq;
    static int64 rem1;
    static int64 remainder;
    static int32 delta;
    static int32 deltaFrqAim; // Ƶ�ʵı仯ֵ
    Uint16 tmp0;
    int16 flag = 0;
    int16 s;
    int32 tmp;
    int32 frqTmpOld = frq0;
    int32 a;
    int32 b;

    if (!accDecTime)    // �Ӽ���ʱ��Ϊ0
    {
        frq0 = frqCurAim;
        return frq0;
    }
    
    if (bFrqCurAimChg)
    {
        if (frq0 > 0)
        {
            inflexionFrq = motorFc.motorPara.elem.ratingFrq;
        }
        else if (frq0 < 0)
        {
            inflexionFrq = -(int32)motorFc.motorPara.elem.ratingFrq;
        }
        else if (frqCurAim > 0)
        {
            inflexionFrq = motorFc.motorPara.elem.ratingFrq;
        }
        else if (frqCurAim < 0)
        {
            inflexionFrq = -(int32)motorFc.motorPara.elem.ratingFrq;
        }
        
        pointFrq = ((inflexionFrq * fm) >> 15); // ����������ֱ�ߵĽ���

        tickerAll2 = (int32)accDecTime * ABS_INT32(inflexionFrq) / (int32)benchFrq;
        tickerAll1 = (tickerAll2 * m1) >> 15; // m = 1/(16*0.7)+1/3

        tmp = 0;
        if (runFlag.bit.accDecStatus == ACC_SPEED) // ����
        {
            if (ABS_INT32(frqCurAim) > ABS_INT32(inflexionFrq))
            {
                tmp = frqCurAim;
            }
        }
        else
        {
            if (ABS_INT32(frq0) > ABS_INT32(inflexionFrq))
            {
                tmp = frq0;
            }
        }

        if (tmp)
        {
            // (4/9*(f/fb)^2 + 5/9) * T
#define QQ1 13                  // Ƶ�ʴﵽ 8*inflexionFrq, �����
            tmp = ((tmp << QQ1) / inflexionFrq);
            tickerAll3 = ((((tmp * tmp) >> (QQ1-2)) + (5UL << QQ1)) * tickerAll2 / 9 + (1UL << (QQ1-1))) >> QQ1;
#undef QQ1
        }
        else
        {
            tickerAll3 = tickerAll2;
        }

        if (ABS_INT32(frq0) <= ABS_INT32(pointFrq)) // λ�ڶ�������
        {
            Uint32 tmp;
#define QQ1 15
            // ���ʱ�䣬һԪ���η������
            // ((B^2 + 4*A*f)^0.5 - B)/(2*A)
            tmp = (((((Uint64)B*B) << (2*QQ1)) / (Sden*Sden))
                   + (4*((((Uint64)A) << (2*QQ1)) * frq0) / (Sden * inflexionFrq)));
            tmp0 = qsqrt(tmp);
            ticker = (((((int32)tmp0 + 3)*Sden - ((int32)B<<QQ1)) * tickerAll2 / A + (1UL << QQ1)) >> (QQ1+1));
#undef QQ1

            remainder = 0;
            deltaFrqAim = inflexionFrq;
            sCurveBStatus = SB_STEP_START_S;
        }
        else if (ABS_INT32(frq0) <= ABS_INT32(inflexionFrq)) // λ��ֱ��
        {
            //(f/fb - fm) * 8/9 * t2 + t1 = t2* (f/fb - fm) * 8/9 * (1+m1)
            ticker = ((((((frq0 << 15) / inflexionFrq - fm) * 8 + (int32)m1 * 9)) * tickerAll2 / 9
                      + (1UL<<(15-1))) >> (15));
            
            deltaFrqAim = inflexionFrq - pointFrq;

            sCurveBStatus = SB_STEP_LINE;
        }
        else                    // ���ڻ�׼Ƶ��
        {
            ticker = tickerAll3;
            sCurveBStatus = SB_STEP_END_S;
        }
    }

    switch (sCurveBStatus)
    {
        case SB_STEP_START_S:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                s = 1;
                if (ticker < tickerAll1)
                {
                    flag = 1;
                }
            }
            else
            {
                ticker--;
                s = -1;
                if (ticker > 0)
                {
                    flag = 1;
                }
            }
            
            if (flag)           // ��������
            {
                a = 2 * ticker - 1;
                b = 1;
                
                delta = (((int64)A * a + tickerAll2 * B * b) * deltaFrqAim + remainder)
                    / ((int64)tickerAll2 * tickerAll2 * Sden);
                remainder = (((int64)A * a + tickerAll2 * B * b) * deltaFrqAim + remainder)
                    % ((int64)tickerAll2 * tickerAll2 * Sden);
                frq0 += s*delta;
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    frq0 += s*delta;
                    deltaFrqAim = inflexionFrq - frq0;
                    rem1 = 0;
                    sCurveBStatus = SB_STEP_LINE;
                }
                else
                {
                    sCurveBStatus = SB_STEP_OK;
                }
            }
            
            break;
            
        case SB_STEP_LINE:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                s = 1;
                if (ticker < tickerAll2)
                    flag = 1;
            }
            else
            {
                ticker--;
                s = -1;
                if (ticker > tickerAll1 + 1)
                    flag = 1;
            }
            
            if (flag)           // ֱ��
            {
                delta = (deltaFrqAim + rem1) / (tickerAll2 - tickerAll1);
                rem1 = (deltaFrqAim + rem1) % (tickerAll2 - tickerAll1);

                frq0 += s*delta;
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    frq0 += s*delta;
                    sCurveBStatus = SB_STEP_END_S;
                }
                else
                {
                    frq0 += s*delta;
                    remainder = 0;
                    deltaFrqAim = inflexionFrq;
                    sCurveBStatus = SB_STEP_START_S;

                    ticker = tickerAll1 + 1;
                }
            }

            break;

        case SB_STEP_END_S:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                if (ticker < tickerAll3)
                    flag = 1;
            }
            else
            {
                ticker--;
                if (ticker > tickerAll2 + 0)
                    flag = 1;
            }
            
            if (flag)
            {
                Uint32 tmp;
#define QQ2 28
                // 3/2*inflexionFrq * ((t/T)-5/9)^0.5
                tmp = ((((int64)9 * ticker - 5 * tickerAll2) << QQ2) / (9 * tickerAll2));
                tmp0 = qsqrt(tmp);
                frq0 = (((int32)tmp0 + 3) * inflexionFrq * 3 ) >> (QQ2/2 + 1);
#undef QQ2
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    sCurveBStatus = SB_STEP_OK;
                }
                else
                {
                    frq0 = inflexionFrq;
                    deltaFrqAim = inflexionFrq - pointFrq;
                    rem1 = 0;
                    sCurveBStatus = SB_STEP_LINE;

                    ticker = tickerAll2 + 1;
                }
            }
            break;
            
        default:
            break;
    }

// ��ֹ����Ƶ��
    if (((frqCurAim > frqTmpOld) && (frq0 > frqCurAim))
        || ((frqCurAim <= frqTmpOld) && (frq0 < frqCurAim)))
    {
        frq0 = frqCurAim;
    }

    if (sCurveBStatus == SB_STEP_OK)
    {
        frq0 = frqCurAim;
    }
#endif
    return frq0;
}


//=====================================================================
//
// �Ӽ���ֱ�߼���
//
//=====================================================================
void LineChangeCalc(LINE_CHANGE_STRUCT *p)
{
    int32 delta;

    if (p->curValue != p->aimValue)
    {
        if (!p->tickerAll)
        {
            p->curValue = p->aimValue;
        }
        else
        {
            delta = ((int32)p->maxValue + p->remainder) / p->tickerAll;
            p->remainder = ((int32)p->maxValue + p->remainder) % p->tickerAll;

            if (p->aimValue > p->curValue)
            {
                p->curValue += delta;
                if (p->curValue > p->aimValue)
                    p->curValue = p->aimValue;
            }
            else
            {
                p->curValue -= delta;
                if (p->curValue < p->aimValue)
                    p->curValue = p->aimValue;
            }
        }
    }
}
