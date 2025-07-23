//======================================================================
//
// IO��������DI, DO, AI, AO, PulseIn, PulseOut.
//
// Time-stamp: <2012-10-22 21:06:51  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_io.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_comm.h"
#include "f_menu.h"
#include "f_frqSrc.h"
#include "f_posCtrl.h"
#include "f_error.h"

#if F_DEBUG_RAM        // �����Թ��ܣ���CCS��build option�ж���ĺ�

#define DEBUG_F_DI              0
#define DEBUG_F_DO              0
#define DEBUG_F_AI              0
#define DEBUG_F_AO              0
#define DEBUG_F_HDI             0
#define DEBUG_F_HDO             0
#define DEBUG_F_TEMPERATURE     0

#elif 1

#define DEBUG_F_DI              1
#define DEBUG_F_DO              1
#define DEBUG_F_AI              1
#define DEBUG_F_AO              1
#define DEBUG_F_HDI             1
#define DEBUG_F_HDO             1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_TEMPERATURE     1
#elif 1
#define DEBUG_F_TEMPERATURE     0
#endif

#endif

//======================================================================
struct DI_FUNC diFunc;          // ��ǰ��DI�������ܵ�״̬
struct DI_FUNC diSelectFunc;    // �Ƿ���DI����ѡ������Ӧ�Ĺ���
struct DI_STATUS_ALL diStatus;  // ��ǰ��DI����״̬
LOCALF union DI_DELAY_TICKER_UNION diDelayTicker;
//======================================================================


//======================================================================
struct DO_FUNC doFunc;
Uint16  oCurrentCheckFlag;
struct DO_STATUS_ALL doStatus;  // ��ǰ��DO����״̬
union DO_HW_STATUS doHwStatus;  // DOӲ��״̬
LOCALF union DO_DELAY_TICKER_UNION doDelayTicker;

#define DO_STATUS_VDO   5      // doStatus�У�VDO1��bitλ��

//#define PULSE_OUT_CTR_PERIOD       (MAX_UINT32 - 2)    // DO3ʹ��ECAP2��APWMʱ������ֵ
#define PULSE_OUT_CTR_PERIOD       (MAX_UINT16 - 2)
//======================================================================


//======================================================================
#define AI_VOLTAGE_CALC_SHIFT   5                   // AI��������еĵ�ѹ����λ������LineCalc()�����ƣ�Ŀǰ���Ϊ5��
#define AI_SAMPLE_Q             16                  // ����ʱʹ�ò���ֵ��Q��ʽ��ĿǰΪ16
#define AI_MAX_SAMPLE1          ((0x0FFFUL-0)<<4) 
#define AI_MAX_SAMPLE           ((0x0FFFUL-3)<<4)   // ������ֵ(Q16)����_ʱ����Ϊ��ѹΪAI_VOLTAGE_INPUT_MAX
//#define AI_MAX_SAMPLE           ((0x0FFFUL-0)<<4)

#define AI_MAX_VOLTAGE_HW       1057    // Ӳ����AI������������ѹ�������ٵ�ѹ��ӦDSP�������̡���λ��10mv
#define AI_MIN_VOLTAGE_HW       (int16)-1057
//#define AI_MAX_VOLTAGE_IDEA     10000   // �����AI������������ѹ����λ��1mv

LowPassFilter aiLpf[AI_NUMBER] = {LPF_DEFALUTS, LPF_DEFALUTS, LPF_DEFALUTS};
struct AI_DEAL aiDeal[AI_NUMBER];
//======================================================================

//======================================================================
#define AO_PWM_FRQ          10000   // AO��PWM��Ƶ, _Hz
#define AO_PWM_PERIOD       (DSP_CLOCK * 1000000UL / AO_PWM_FRQ)

#define AO_MAX_VOLTAGE_IDEA     10000   // �����AO��������ѹ����λ��1mv

#define FAN_CONTROL_STOP_DELAY  2500  // ɢ��������ͣת�ӳ�ʱ��(5000ms)
#define FAN_CONTROL_START_TEMP  50    // ɢ�����������¶�
#define FAN_CONTROL_STOP_TEMP   45    // ɢ��������ͣת�¶�

int32 aoFmpValue;            // FMP,AO�����ֵ
int32 aoFmpMax;              // FMP,AO��������ֵ

//======================================================================

//======================================================================
#define PULSE_IN_ZERO                           1000    // _ms��û����������(����)����Ϊ����Ƶ��Ϊ0
#define COUNTER_IN_USE_CAPTURE_TICKER_LIMIT     21000   // ������ʹ��CAP���������Ƶ����ֵ,_Hz

int16 pulseInSet;               // Q15, 100.0% - 1*2^15
Uint32 pulseInFrq;              // �������������Ƶ�ʣ���λ: 1Hz
Uint32 pulseInFrq1;
Uint16 lineSpeed;

//Uint16 fanControl;

//LOCALF LINE_STRUCT pulseInLine = LINE_STRTUCT_DEFALUTS;
LOCALF LowPassFilter pulseInLpf = LPF_DEFALUTS;
LOCALF Uint32 capturePeriodSum;     // ����captureTicker��ʱ�䣬CAP��ʱ���Ļ���ΪSYSCLK
LOCALF Uint16 captureTicker;        // capturePeriodSumʱ���ڣ����񵽵Ĵ���
LOCALF Uint16 noCaptureTicker;      // PULSE_IN_CALC_PERIODʱ����û�в���һֱ���ۼ�ticker

LOCALF Uint16 bOverFlowCounterTicker;   // ����ֵ �����־
LOCALF Uint16 bOverFlowLengthCurrent;   // ��ǰ���� �����־

LOCALF Uint32 oCurrentChkTicker;         //  ��������ʱ
Uint16 softOcDoFlag = 0;
//======================================================================



void TemperatureDeal(void);
void GetTemperature(void);
Uint16 GetTemperatureCalc(Uint16 voltage, const Uint16 *p, Uint16 len);
void TemperatureAfterDeal(void);
void TemperatureSensorDeal(void);
Uint16 motorForeOT;                         // ����Ԥ������־
Uint16 motorOT;                             // ���±�����־
Uint16 temperature;                         // ��⵽���¶�ֵ
Uint16 tickerTempDeal;
Uint32 tempSampleSum;
Uint16 temperatureVoltage;                  // �¶ȼ��ĵ�ѹ, 0-3V
Uint16 temperatureVoltageOrigin;            // �¶ȼ��ĵ�ѹ, У��ǰ��0-3V
#define TEMPERATURE_CALC_PERIOD     100     // �¶ȴ������ڣ�_ms
#define TEMPERATURE_CALL_PERIOD     2       // �¶ȴ������������ڣ�_ms
//======================================================================
//
// Time-stamp: <2007-12-20 14:01:46  Shisheng.Zhi, 0354>
// 
// �¶ȼ��Ĳ���
// DSP������ѹ���¶ȵĶ�Ӧ��ϵ��
//
// PT100, PT1000, NTC
//
//======================================================================


#if DEBUG_F_TEMPERATURE
// PT100�Ĳ�����DSP������ѹ���¶ȵĶ�Ӧ��ϵ��
const Uint16 voltageTempPT100[] = 
{// ��ѹmV      �¶ȡ�
    1650 	,   //	0
    1675 	,   //	4
    1700 	,   //	8
    1724 	,   //	12
    1749 	,   //	16
    1774 	,   //	20
    1798 	,   //	24
    1823 	,   //	28
    1847 	,   //	32
    1872 	,   //	36
    1896 	,   //	40
    1920 	,   //	44
    1945 	,   //	48
    1969 	,   //	52
    1993 	,   //	56
    2017 	,   //	60
    2041 	,   //	64
    2066 	,   //	68
    2090 	,   //	72
    2114 	,   //	76
    2138 	,   //	80
    2161 	,   //	84
    2185 	,   //	88
    2209 	,   //	92
    2233 	,   //	96
    2257 	,   //	100
    2280 	,   //	104
    2304 	,   //	108
    2328 	,   //	112
    2351 	,   //	116
    2375 	,   //	120
    2398 	,   //	124
    2422 	,   //	128
    2445 	,   //	132
    2469 	,   //	136
    2492 	,   //	140
    2515 	,   //	144
    2538 	,   //	148
    2562 	,   //	152
    2585 	,   //	156
    2608 	,   //	160
    2631 	,   //	164
    2654 	,   //	168
    2677 	,   //	172
    2700 	,   //	176
    2723 	,   //	180
    2746 	,   //	184
    2769 	,   //	188
    2791 	,   //	192
    2814 	,   //	196
    2837 	,   //	200

}; 



// PT1000�Ĳ�����DSP������ѹ���¶ȵĶ�Ӧ��ϵ��
const Uint16 voltageTempPT1000[] = 
{// ��ѹmV      �¶ȡ�
    1648	,   //	0
    1673	,   //	4
    1698	,   //	8
    1722	,   //	12
    1748	,   //	16
    1772	,   //	20
    1795	,   //	24
    1820	,   //	28
    1844	,   //	32
    1868	,   //	36
    1891	,   //	40
    1916	,   //	44
    1941	,   //	48
    1964	,   //	52
    1988	,   //	56
    2012	,   //	60
    2036	,   //	64
    2059	,   //	68
    2083	,   //	72
    2108	,   //	76
    2130	,   //	80
    2155	,   //	84
    2177	,   //	88
    2202	,   //	92
    2225	,   //	96
    2249	,   //	100
    2272	,   //	104
    2295	,   //	108
    2317	,   //	112
    2341	,   //	116
    2364	,   //	120
    2387	,   //	124
    2411	,   //	128
    2433	,   //	132
    2456	,   //	136
    2478	,   //	140
    2502	,   //	144
    2524	,   //	148
    2547	,   //	152
    2570	,   //	156
    2592	,   //	160
    2616	,   //	164
    2638	,   //	168
    2661	,   //	172
    2683	,   //	176
    2706	,   //	180
    2729	,   //	184
    2750	,   //	188
    2772	,   //	192
    2795	,   //	196
    2818	,   //	200 
};
#endif


LOCALD void UpdateDoFunc(void);

void  newUpdateDoFunc(Uint16 index);

LOCALD void UpdateFmpAoValue(Uint16 func, Uint16 aoOrFmp);
void PulseOutDeal(void);
void PulseOutCalc(void);
LOCALD void Di2Bin(void);
LOCALD void Do2Bin(void); 
LOCALD void getDiHwStatus(void);
LOCALD void setDOStatus(void);

//=====================================================================
//
// DI ������
// ���룺diHwStatus -- DI��˲ʱ״̬
// ������funcCode.code.diFilterTime -- �˲�ʱ��
// �����diFunc -- DI���ӹ���״̬
//
// ���̣�
// 1. �ⲿ���ϳ������ӡ�ͣ��ֱ���ƶ����Ӹ�ֵΪ1������diFunc��ֵΪ0��
// 2. ����DI��˲ʱ״̬���˲�ʱ�䣬�õ�DI���ȶ�״̬ diStatus.a��
// 3. ����diStatus����diFunc��Ӧλ��1��
//
//    
//=====================================================================
Uint32 diLogic;     // �����߼���VDI�޴˴���
Uint16 vdiSrc;
Uint16 vdiFcSet;
void DiCalc(void)
{
    static Uint16 diXFilterCounter[DI_NUMBER_PHSIC];     // ÿ�����ӵ��˲���ʱ��
    int16 i;
    Uint32 tmp;
    Uint32 tmpFc;

    Uint16 fc1[DI_TERMINAL_NUMBER];     // DI���ӵĹ���ѡ��
    Uint32 *pDiSelectFunc = &diSelectFunc.f1.all;
    Uint32 *pDiFunc = &diFunc.f1.all;

    diSelectFunc.f1.all = 0;
    diSelectFunc.f2.all = 0;
    diFunc.f1.all = 0;                   // ����ɾ��
    diFunc.f2.all = 0;
#if DEBUG_F_POSITION_CTRL   //
    diSelectFunc.f3.all = 0;
    diFunc.f3.all = 0;
#endif

#if DEBUG_F_DI
    Di2Bin();

#if DSP_2803X   // 2803xƽ̨
    getDiHwStatus();
#endif
    asm(" nop");


// ��� diStatus.a.all���˲�ʱ��
    // ����DI���ӣ�DI1 -- DI10
    for (i = DI_NUMBER_PHSIC - 1; i >= 0; i--)//382
    {
        tmp = 0x01UL << i;
        
        if ((diStatus.a.all ^ (~diHwStatus.all)) & tmp) // ��DI���ӵ�״̬�ı���
        {                                           // ע��Ӳ���ϣ�DI�Ѿ�ȡ��
            // ȷ�ϸ�λDI��״̬�Ѿ��ı�(�˲�)
            if (++diXFilterCounter[i] >= funcCode.code.diFilterTime / DI_CALC_PERIOD)
            {
                diStatus.a.all ^= tmp;              // ��λDI��״̬ȡ��
                diXFilterCounter[i] = 0;
            }
        }
        else    // û�иı䣬�˲���ʱ�����㡣����ɾ��������: �ڴﵽ�˲�ʱ��֮ǰ���������
        {
            diXFilterCounter[i] = 0;
        }

        fc1[i] = funcCode.code.diFunc[i];
    }
    // ����DI���ӣ�VDI1 -- VDI5//359
    for (i = DI_NUMBER_V - 1; i >= 0; i--)
    {
        Uint16 vdiTmp = (0x01U << i);
        Uint16 vdi;
        
        tmp = 0x01UL << (i + DI_NUMBER_PHSIC);

        if (!(vdiSrc & vdiTmp))     // 0��������DOx�ڲ�����
        {
            vdi = doStatus.c.all >> DO_STATUS_VDO;  // doStatus.c.all��bit6-bit10��ʾVDO
        }
        else                        // 1: �������趨
        {
            vdi = vdiFcSet;
        }

        if (vdi & vdiTmp)
        {
            diStatus.a.all |= tmp;
        }
        else
        {
            diStatus.a.all &= ~tmp;
        }

        fc1[i + DI_NUMBER_PHSIC] = funcCode.code.vdiFunc[i];
    }
    // AI��ΪDIʹ��//208
    for (i = DI_NUMBER_AI_AS_DI - 1; i >= 0; i--)
    {
        tmp = 0x01UL << (i + DI_NUMBER_PHSIC + DI_NUMBER_V);

#define AI_AS_DI_LOW    300     // <= 3V
#define AI_AS_DI_HIGH   700     // >= 7V
        if (aiDeal[i].voltage >= AI_AS_DI_HIGH)
        {
            diStatus.a.all |= tmp;
        }
        else if (aiDeal[i].voltage <= AI_AS_DI_LOW)
        {
            diStatus.a.all &= ~tmp;
        }

        fc1[i + DI_NUMBER_PHSIC + DI_NUMBER_V] = funcCode.code.aiAsDiFunc[i];
    }

// ���diStatus.b.all��DI�ӳ�ʱ�䴦��//193
    for (i = 3-1; i >= 0; i--)
    {
        int32 delayTickerMax;

        tmp = 0x01UL << i;
        
        // F4-35 DI�ӳ�ʱ�书������ʼ
        delayTickerMax = (Uint32)funcCode.code.diDelayTime[i] * (TIME_UNIT_DI_DELAY / DI_CALC_PERIOD);

        if (diStatus.a.all & tmp)   // ��DO���1(��ʱǰ)
        {
            diDelayTicker.all[i].low = 0;
            if (++diDelayTicker.all[i].high >= delayTickerMax)
                diStatus.b.all |= tmp;     // ��ʱ��ɺ�һֱ���1
        }
        else
        {
            diDelayTicker.all[i].high = 0;
            if (++diDelayTicker.all[i].low >= delayTickerMax)
                diStatus.b.all &= ~tmp;    // ��ʱ��ɺ�һֱ���0
        }
    }
    diStatus.b.all = (diStatus.a.all & 0xFFFFFFF8UL) |  (diStatus.b.all & 0x00000007UL);
    
// ���diStatus.c.all�������߼�����
    diStatus.c.all = diStatus.b.all ^ diLogic;
    
// ����diStatus.c.all�����д���//1253
    for (i = DI_TERMINAL_NUMBER - 1; i >= 0; i--)
    {
        tmp = 0x01UL << i;
        
        tmpFc = 0x01UL << (fc1[i] % 32);            // fc1[]�п��ܴ���32
// code generation 4.13���룬��ʹfc >= 32��Ҳû�����⡣���Ϊ LSLL

// ���diSelectFunc
        *(pDiSelectFunc + fc1[i] / 32) |= tmpFc;    // �Ƿ���DI����ѡ������Ӧ�Ĺ��ܡ�

// ���diFunc
        if (diStatus.c.all & tmp)                   // ����DI���ȶ��ĵ�ǰ״̬����1 or ����
        {
            *(pDiFunc + fc1[i] / 32) |= tmpFc;
        }
        else
        {
            *(pDiFunc + fc1[i] / 32) &= ~tmpFc;
        }
    }

// ʹ��pulseInʱ��DI5���ܲ����㡣
// ���ǣ�ʹ��pulseInʱ��DI5����ͬʱ��Ϊ�������ͳ������롣
    if ((DI_FUNC_COUNTER_TICKER_IN != funcCode.code.diFunc[4]) && 
        (DI_FUNC_LENGTH_TICKER_IN != funcCode.code.diFunc[4]) && 
        (   // ʹ����Pulse In��DI5���ܲ����㡣
         (FUNCCODE_frqXySrc_PULSE == funcCode.code.frqXSrc) || 
         (FUNCCODE_frqXySrc_PULSE == funcCode.code.frqYSrc) || 
         (FUNCCODE_upperFrqSrc_PULSE  == funcCode.code.upperFrqSrc) || 
         (FUNCCODE_pidSetSrc_PULSE == funcCode.code.pidSetSrc) || 
         (FUNCCODE_pidFdbSrc_PULSE == funcCode.code.pidFdbSrc) || 
         (FUNCCODE_plcFrq0Src_PULSE == funcCode.code.plcFrq0Src)
        )
       )
    {
        *(pDiFunc) &= ~(0x01UL << funcCode.code.diFunc[4]);
    }
#endif
} // DiCalc()


//==================================================================
//
// ��Ƶ��������(1)
//==================================================================
void Dout1(void)
{
    if (runFlag.bit.run)
    {
        doFunc.f1.bit.run = 1;
    }
}

//==================================================================
//
// �������(����ͣ��)
//
//==================================================================
void Dout2(void)
{
    if //((ERROR_EXTERNAL == errorCode)      // �ⲿ����(���Զ���λ)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // ����ʱ�䵽��(���Զ���λ)
       //|| (ERROR_USER_1 == errorCode)        // �û�����1(���Զ���λ)
       //|| (ERROR_USER_2 == errorCode)        // �û�����2(���Զ���λ)
       //|| 
       (errorCode                              // �����ҹ����Զ���λ����   
       && ( errorDealStatus != ERROR_DEAL_OK)  // �����Զ���λ����һ��
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))
       )
    {
        // ������Ӧ��Ϊ�Զ�����
        if (ERROR_LEVEL_RUN != errorAttribute.bit.level)
        {
            doFunc.f1.bit.error = 1;  // 2:�������(����ͣ��)
        }
    }   
}

//==================================================================
//
// �������(�������)
//
//==================================================================
void Dout38(void)
{
    if //(  (ERROR_EXTERNAL == errorCode)      // �ⲿ����(���Զ���λ)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // ����ʱ�䵽��(���Զ���λ)
       //|| (ERROR_USER_1 == errorCode)        // �û�����1(���Զ���λ)
       //|| (ERROR_USER_2 == errorCode)        // �û�����2(���Զ���λ)
       //|| 
       (errorCode                              // �����ҹ����Զ���λ����   
       && ( errorDealStatus != ERROR_DEAL_OK)  // �����Զ���λ����һ��
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))      
       )
    {
        doFunc.f2.bit.errorOnStop = 1;      // 38:�������
    }   
}

//==================================================================
//
// �������
//
//==================================================================
void Dout41(void)
{
    if //((ERROR_EXTERNAL == errorCode)      // �ⲿ����(���Զ���λ)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // ����ʱ�䵽��(���Զ���λ)
       //|| (ERROR_USER_1 == errorCode)        // �û�����1(���Զ���λ)
       //|| (ERROR_USER_2 == errorCode)        // �û�����2(���Զ���λ)
       //|| 
       (errorCode                              // �����ҹ����Զ���λ����   
       && ( errorDealStatus != ERROR_DEAL_OK)  // �����Զ���λ����һ��
       && (errorCode != ERROR_UV)
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))
       )
    {
        // ������Ӧ��Ϊ�Զ�����
        if (ERROR_LEVEL_RUN != errorAttribute.bit.level)
        {
            doFunc.f2.bit.errorOnNoUV = 1;  // 2:�������(����ͣ��)
        }
    }   
}

//==================================================================
//
// 3:Ƶ��ˮƽ���FDT����
//
//==================================================================
void Dout3(void)
{
    static Uint16 frqFdtArriveOld;      // FDTƵ�ʵ���

    if (ABS_INT32(frq) >= funcCode.code.frqFdtValue)
    {
        frqFdtArriveOld = 1;
    }
    else if (ABS_INT32(frq) < ((Uint32)funcCode.code.frqFdtValue * (1000 - funcCode.code.frqFdtLag) / 1000))
    {
        frqFdtArriveOld = 0;
    }
    
    if (!runFlag.bit.run)       // ͣ��ʱ��Ӧ��һֱ�����Ч
    {
        frqFdtArriveOld = 0;
    }
    
    if (frqFdtArriveOld)
    {
        doFunc.f1.bit.frqFdtArrive = 1;
    }
}


//==================================================================
//
// 4:Ƶ�ʵ���
//
//==================================================================
void Dout4(void)
{
    if ((ABS_INT32(frq) <= (ABS_INT32(frqAim) + (int32)maxFrq * funcCode.code.frqArriveRange / 1000))
        && (ABS_INT32(frq) >= (ABS_INT32(frqAim) - (int32)maxFrq * funcCode.code.frqArriveRange / 1000))  
        && (runFlag.bit.run)
        && ((int64)frq * frqAim >= 0)
        && (!tuneCmd)  // ��гʱ
        )
    {
        doFunc.f1.bit.frqArrive = 1;
    }
}

//==================================================================
//
// ����������
//
//==================================================================
void Dout5(void)
{
    if (!frq)
    {
        if(runFlag.bit.run)  // ����������
        {
            doFunc.f1.bit.zeroSpeedRun = 1; // 5:����������(ͣ����Ч)
        }
    }
}

//==================================================================
//
// ����������
//
//==================================================================
void Dout23(void)
{
    if (!frq)
    {
        doFunc.f1.bit.zeroSpeedRun1 = 1;    // 23:����������2(ͣ����Ч)
    }
}

//==================================================================
//
// 6:�������Ԥ����
//
//==================================================================
void Dout6(void)
{
    if (dspStatus.bit.motorPreOl)
    {
        doFunc.f1.bit.motorPreOl = 1;
    }
}

//==================================================================
//
// 7:��Ƶ������Ԥ����
//
//==================================================================
void Dout7(void)
{
    if (dspStatus.bit.inverterPreOl)
    {
        doFunc.f1.bit.inverterPreOl = 1;
    }
}

//==================================================================
//
// 8:�趨��������ֵ����
//
//==================================================================
void Dout8(void)
{
    if (funcCode.code.counterTicker >= funcCode.code.counterSet)
    {
        doFunc.f1.bit.counterSetArrive = 1;
    }

    if (bOverFlowCounterTicker)    // counterTicker���
    {
        doFunc.f1.bit.counterSetArrive = 1;
    }
}

//==================================================================
//
// 9:ָ����������ֵ����
//
//==================================================================
void Dout9(void)
{
    if (funcCode.code.counterTicker >= funcCode.code.counterPoint)
    {
        doFunc.f1.bit.counterPointArrive = 1;
    }

    // �����������
    if (bOverFlowCounterTicker)    // counterTicker���
    {
        doFunc.f1.bit.counterPointArrive = 1;
    }
}

//==================================================================
//
// 10:���ȵ���
//
//==================================================================
void Dout10(void)
{
    if ((funcCode.code.lengthCurrent >= funcCode.code.lengthSet)
        || bOverFlowLengthCurrent)  // ���
    {
        doFunc.f1.bit.lengthArrive = 1;
    }
}

//==================================================================
//
// 11:PLCѭ�����
//
//==================================================================
void Dout11(void)
{
    if (bPlcEndOneLoop)
    {
        doFunc.f1.bit.plcEndLoop = 1;
    }
}

//==================================================================
//
// 12:����ʱ�䵽��
//
//==================================================================
void Dout12(void)
{
    if ((funcCode.code.runTimeAddup >= funcCode.code.runTimeArriveSet)
        && funcCode.code.runTimeArriveSet)  // �趨���е���ʱ��ʱ��Ϊ0����Ч
    {
        doFunc.f1.bit.runTimeArrive = 1;
    }
}

//==================================================================
//
// 13:Ƶ���޶���
//
//==================================================================
void Dout13(void)
{
	if (swingFrqLimit)
	{
		doFunc.f1.bit.frqLimit = 1;  // ��Ƶ�޶���
	}
}

//==================================================================
//
// 14:ת���޶���
//
//==================================================================
void Dout14(void)
{
    if (TorqueLimitCalc())
    {
        doFunc.f1.bit.torqueLimit = 1;  // ת���޶���
    }
}

//==================================================================
//
// ����׼������(15)
//
//==================================================================
void Dout15(void)
{
    if ((!errorCode)                    // û�й���
        && (!bUv)                       // ĸ�ߵ�ѹ��Ƿѹ
        && (!diFunc.f1.bit.closePwm)    // ����ͣ��(���н�ֹ)������Ч
        )
    {
        doFunc.f1.bit.runReadyOk = 1;
    }
}

//==================================================================
//
// AI1 > AI2(16)
//
//==================================================================
void Dout16(void)
{
    if (ABS_INT16(aiDeal[0].set) > ABS_INT16(aiDeal[1].set))
    {
        doFunc.f1.bit.ai1GreaterThanAi2 = 1;
    }
}

//==================================================================
//
// ����Ƶ�ʵ���(17)
//
//==================================================================
void Dout17(void)
{
	if ((ABS_INT32(frqAim) >= upperFrq) 
      && (ABS_INT32(frq) >= upperFrq))  // ע�⣬�����Ƿ������޹�(��ˮ)
	{
		doFunc.f1.bit.upperFrqArrive = 1;
	}
}

//==================================================================
//
// ����Ƶ�ʵ���(18)
//
//==================================================================
void Dout18(void)
{
	if ((ABS_INT32(frqAim) <= lowerFrq)
		&& (ABS_INT32(frq) <= lowerFrq)
		&& runFlag.bit.run)
	{
		doFunc.f1.bit.lowerFrqArrive = 1;
	}
}

//==================================================================
//
// Ƿѹ״̬���(19)
//
//==================================================================
void Dout19(void)
{
    if (!dspStatus.bit.uv)
    {
        doFunc.f1.bit.uv = 1;
    }
}

void DoutRSVD(void)
{
}

//==================================================================
//
// ��λ���(21)    
//
//==================================================================
void Dout21(void)
{
#if DEBUG_F_POSITION_CTRL
    if (bPcErrorOk)
    {
        doFunc.f1.bit.pcOk = 1; // DO ��� ��λ����ź�
    }
#endif
}

//==================================================================
//
// ��λ�ӽ�(22)
//
//==================================================================
void Dout22(void)
{
#if DEBUG_F_POSITION_CTRL
    if (bPcErrorNear)
    {
        doFunc.f1.bit.pcNear = 1; // DO ��� ��λ�ӽ��ź�
    }
#endif
}

//==================================================================
//
// �ϵ�ʱ�䵽��(24)
//
//==================================================================
void Dout24(void)
{
    if ((funcCode.code.powerUpTimeAddup >= funcCode.code.powerUpTimeArriveSet)
        && funcCode.code.powerUpTimeArriveSet)  // �趨���е���ʱ��ʱ��Ϊ0����Ч
    {
        doFunc.f1.bit.powerUpTimeArrive = 1;
    }
}

//==================================================================
//
// Ƶ��ˮƽ���FDT1����(25)
//
//==================================================================
void Dout25(void)
{
    static Uint16 frqFdt1ArriveOld;     // FDT1Ƶ�ʵ���

    if (ABS_INT32(frq) >= funcCode.code.frqFdt1Value)
    {
        frqFdt1ArriveOld = 1;
    }
    else if (ABS_INT32(frq) < ((Uint32)funcCode.code.frqFdt1Value * (1000 - funcCode.code.frqFdt1Lag) / 1000))
    {
        frqFdt1ArriveOld = 0;
    }
    if (!runFlag.bit.run)       // ͣ��ʱ��Ӧ��һֱ�����Ч
    {
        frqFdt1ArriveOld = 0;
    }
    if (frqFdt1ArriveOld)
    {
        doFunc.f1.bit.frqFdtArrive1 = 1;
    }	
}

//==================================================================
//
// Ƶ�ʵ���1(26)
//
//==================================================================
void Dout26(void)
{
    if ((ABS_INT32(frq) <= (funcCode.code.frqArriveValue1 + (int32)maxFrq * funcCode.code.frqArriveRange1 / 1000))
        && (ABS_INT32(frq) >= (funcCode.code.frqArriveValue1 - (int32)maxFrq * funcCode.code.frqArriveRange1 / 1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.frqArrive1 = 1;
    }
}

//==================================================================
//
// Ƶ�ʵ���2(27)
//
//==================================================================
void Dout27(void)
{
    if ((ABS_INT32(frq) <= (funcCode.code.frqArriveValue2 + (int32)maxFrq * funcCode.code.frqArriveRange2 / 1000))
        && (ABS_INT32(frq) >= (funcCode.code.frqArriveValue2 - (int32)maxFrq * funcCode.code.frqArriveRange2 / 1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.frqArrive2 = 1;
    }
}

//==================================================================
//
// �����������1(28)
//
//==================================================================
void Dout28(void)
{
    if ((outCurrentDisp >= (((int32)funcCode.code.currentArriveValue1 - funcCode.code.currentArriveRange1)*motorFc.motorPara.elem.ratingCurrent/1000))
        && (outCurrentDisp <= ((funcCode.code.currentArriveValue1 + funcCode.code.currentArriveRange1)*motorFc.motorPara.elem.ratingCurrent/1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.currentArrive1 = 1;
    }
}

//==================================================================
//
// �����������2(29)
//
//==================================================================
void Dout29(void)
{
    if ((outCurrentDisp >= (((int32)funcCode.code.currentArriveValue2 - funcCode.code.currentArriveRange2)*motorFc.motorPara.elem.ratingCurrent/1000))
        && (outCurrentDisp <= ((Uint32)(funcCode.code.currentArriveValue2 + funcCode.code.currentArriveRange2)*motorFc.motorPara.elem.ratingCurrent/1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.currentArrive2 = 1;
    }	
}

//==================================================================
//
// ��ʱ����(30)
//
//==================================================================
#define SET_TIME_ARRIVE_TCNT_MAX   250  //500ms
void Dout30(void)
{
    static Uint16 arriveTcnt;    
    if ((!setRunLostTime) && setRunTimeAim)
    {
        arriveTcnt = SET_TIME_ARRIVE_TCNT_MAX;
    }

    if (arriveTcnt > 0)
    {
        doFunc.f1.bit.setTimeArrive = 1;   // DO ���  ��ʱ�����ź�
        arriveTcnt--;
    }
}

//==================================================================
//
// AI1����������(31)
//
//==================================================================
void Dout31(void)
{
    if ((ABS_INT16(aiDeal[0].voltage) > funcCode.code.ai1VoltageUpper) || 
        (ABS_INT16(aiDeal[0].voltage) < funcCode.code.ai1VoltageLimit))
    {
        doFunc.f1.bit.ai1limit = 1;
    }
}

//==================================================================
//
// ����(32)
//
//==================================================================
void Dout32(void)
{
    if (dspStatus.bit.outAirSwitchOff)
    {
        doFunc.f2.bit.loseLoad = 1;
    }
}

//==================================================================
//
// ת�ٷ���(33)
//
//==================================================================
void Dout33(void)
{
    if ( runFlag.bit.run
    	&& ((frqRun < 0) 
        	|| ((frqRun == 0) && (frqAim < 0)))
		)
    {
        doFunc.f2.bit.speedDir = 1;  // ���з���Ϊ��
    }
}

//==================================================================
//
// ��������(34)
//
//==================================================================
void Dout34(void)
{
    if (!oCurrentCheckFlag)
    {
        if ((runFlag.bit.run) && 
            (outCurrentDisp <= ((Uint32)funcCode.code.oCurrentChkValue*motorFc.motorPara.elem.ratingCurrent/1000))
            )
        {
            // ��������ʱ��
            if (oCurrentChkTicker < ((Uint32)funcCode.code.oCurrentChkTime*TIME_UNIT_CURRENT_CHK / DO_CALC_PERIOD))
            {
                oCurrentChkTicker++;
            }
        }
        else
        {
            oCurrentChkTicker = 0;
        }

        if (oCurrentChkTicker >= ((Uint32)funcCode.code.oCurrentChkTime*TIME_UNIT_CURRENT_CHK / DO_CALC_PERIOD))
        {
            doFunc.f2.bit.oCurrent = 1;
        }

        // ���ж�
        oCurrentCheckFlag = 1;
    }
}

//==================================================================
//
// ģ���¶�Ԥ����(35)
//
//==================================================================
void Dout35(void)
{
    if (funcCode.code.radiatorTemp >= funcCode.code.temperatureArrive)
    {
        doFunc.f2.bit.tempArrive= 1;
    }
}

//==================================================================
//
// �������DO���(36)
//
//==================================================================
void Dout36(void)
{
    if (softOcDoFlag == 1)
    {
        doFunc.f2.bit.softOc= 1;
    }
    else
    {
        doFunc.f2.bit.softOc= 0;
    }
}

//==================================================================
//
// ����Ƶ�ʵ���(37)
//
//==================================================================
void Dout37(void)
{
	if ((ABS_INT32(frqAim) <= lowerFrq)
		&& (ABS_INT32(frq) <= lowerFrq)) // ע�⣬�����Ƿ������й�	
	{
		doFunc.f2.bit.lowerFrqArrive = 1;
	}
}

void Dout39(void)
{
    // �������Ԥ����
    if (motorForeOT)
    {
        doFunc.f2.bit.motorForeOT = 1;
    }
}

void Dout40(void)
{
    // ��ǰ����ʱ�䵽��
    if (funcCode.code.setTimeArrive 
        && (curTime.runTime >= funcCode.code.setTimeArrive))
    {
        doFunc.f2.bit.setRunTimeArrive = 1;
    }
}

typedef struct
{
    void (*calc)();       // ����ָ��
} DO_FUNC_CAL;

const DO_FUNC_CAL doFuncCal[DO_FUNC_NUMBER] = {
    DoutRSVD,  Dout1,  Dout2,  Dout3,  Dout4,  
    Dout5,     Dout6,  Dout7,  Dout8,  Dout9,  
    Dout10,    Dout11, Dout12, Dout13, Dout14, 
    Dout15,    Dout16, Dout17, Dout18, Dout19, 
    DoutRSVD,  Dout21, Dout22, Dout23, Dout24, 
    Dout25,    Dout26, Dout27, Dout28, Dout29, 
    Dout30,    Dout31, Dout32, Dout33, Dout34, 
    Dout35,    Dout36, Dout37, Dout38, Dout39,
    Dout40,    Dout41,
};

//=====================================================================
//
// DO ������
//
// ���ݵ�ǰ��Ƶ���ĸ���״̬/ͨѶ��DO���ƣ�
// ���DO���ӵ�doStatus��
//
// doStatus��λ���壬��ͨѶ���Ƶ�DO������Ƶ�λ����Ĺ�ϵ��
// doStatus.a.bit(i)         -- doComm Index[i]
// doStatus.bit0 --   do3  -- doComm.bit4
// doStatus.bit1 -- relay1 -- doComm.bit2
// doStatus.bit2 -- relay2 -- doComm.bit3
// doStatus.bit3 --   do1  -- doComm.bit0
// doStatus.bit4 --   do2  -- doComm.bit1
//
// doHwIndex, ������DO˳����Ӳ��DO˳��Ĺ�ϵ
//
//=====================================================================
Uint16 doLogic;
void DoCalc(void)
{
#if DEBUG_F_DO
    Uint32 delayTickerMax;
    Uint16 doCommIndex[DO_TERMINAL_NUMBER] = {4, 2, 3, 0, 1, 5, 6, 7, 8, 9};
    int16 i;

// ����doFunc    
    doFunc.f1.all = 0;
    doFunc.f2.all = 0;
    oCurrentCheckFlag = 0;

// ����doFunc������doStatus
    doStatus.a.all = 0;
    for (i = DO_TERMINAL_NUMBER - 1; i >= 0; i--)
    {
        Uint16 fc;      // ��DO�Ĺ���ѡ��
        Uint16 tmpSci; 
        Uint16 tmp;
        Uint16 flag = 0;

        if (i < DO_NUMBER_PHSIC)   // ����DO
        {
            fc = funcCode.code.doFunc[i];
    		tmp = 1U << i;
            delayTickerMax = (Uint32)funcCode.code.doDelayTime[i] * (TIME_UNIT_DO_DELAY / DO_CALC_PERIOD);
        }
        else        // ����DO
        {
            Uint16 vdoTmp = i - DO_NUMBER_PHSIC;
            fc = funcCode.code.vdoFunc[vdoTmp];
    		tmp = 1U << (vdoTmp + DO_STATUS_VDO);
            delayTickerMax = (Uint32)funcCode.code.vdoDelayTime[vdoTmp] * (TIME_UNIT_DO_DELAY / DO_CALC_PERIOD);
        }
        
        // ͨѶ����
        tmpSci = 1U << doCommIndex[i];

        // ����DO��������ԴΪDI
        if ((i >= DO_NUMBER_PHSIC) 
            && (fc == 0))
        {
            flag = (diStatus.c.all>>(i - DO_NUMBER_PHSIC)) & 0x0001;
        }
        else if (DO_FUNC_COMM_CTRL != fc) 
        {
            doComm.all &= ~tmpSci;              // ��DO��doSci��Ӧ��λ���㡣
#if 0
            flag = doFuncCal[fc].calc();
#else
            doFuncCal[fc].calc();
            // ��DO��doSci��Ӧ��λ���㡣
            if(fc > 31)
            {
                if (doFunc.f2.all & (0x1UL << (fc-32)))  // ��doFunc����
                    flag = 1;   // ��DO���1
            }
            else
            {
                if (doFunc.f1.all & (0x1UL << fc))  // ��doFunc����
                    flag = 1;   // ��DO���1
            }
#endif
        }
        else
        {
            if (doComm.all & tmpSci)            // ͨѶ���Ƶĸ�λDOΪ1
            {
                flag = 1;    
            }
        }

        // DO�����ʱ����
        if (flag)   // ��DO���1(��ʱǰ)
        {
            doStatus.a.all |= tmp;
            
            doDelayTicker.all[i].low = 0;
            if (++doDelayTicker.all[i].high >= delayTickerMax)
                doStatus.b.all |= tmp;     // ��ʱ��ɺ�һֱ���1
        }
        else
        {
            //doStatus.a.all &= ~tmp;
            
            doDelayTicker.all[i].high = 0;
            if (++doDelayTicker.all[i].low >= delayTickerMax)
                doStatus.b.all &= ~tmp;    // ��ʱ��ɺ�һֱ���0
        }
    }

// �����߼�
    Do2Bin();   // ����doLogic
    doStatus.c.all = doStatus.b.all ^ doLogic;

// ת����Ӳ��DO��Ӧ˳��
// DO3(FMR)�Ŀ�����FMPDeal()��
    doHwStatus.bit.relay1 = doStatus.c.bit.relay1;      // ���ư�̵���
    doHwStatus.bit.relay2 = doStatus.c.bit.relay2;      // ��չ��̵���
    doHwStatus.bit.do1    = doStatus.c.bit.do1;         // DO1
    doHwStatus.bit.do2    = doStatus.c.bit.do2;         // ��չ��DO2
    doHwStatus.bit.error  = doFunc.f1.bit.error;        // ����ָʾ��
    doHwStatus.bit.run    = doFunc.f1.bit.run;          // ����ָʾ��
    doHwStatus.bit.fan    = (dspStatus.bit.fan | funcCode.code.fanControl);
    //doHwStatus.bit.fan    = dspStatus.bit.fan;          // ���ȿ���

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.outPortControl && funcCode.code.plcEnable)
    {
        Uint16 digit1[5];
        Uint16 digit2[5];
        // ��ȡ������ӿ�����Դ(PLC�����ƻ��Ǳ�Ƶ������)
        GetNumberDigit1(digit1, funcCode.code.outPortControl);
        // ��ȡPLC�����ƶ�������ź�
        GetNumberDigit1(digit2, funcCode.code.inPortOut);
        
        // FMPΪPLC������
        if (digit1[0])
        {
            doStatus.c.bit.do3 = digit2[0];
        }
        
        // �̵���1ΪPLC������
        if (digit1[1])
        {
            doHwStatus.bit.relay1 = digit2[1];
        }
        
        // DO1ΪPLC������
        if (digit1[2])
        {
            doHwStatus.bit.do1 = digit2[2];
        }
    }
#endif

#if DSP_2803X
    setDOStatus();
#endif
    
    
#endif
} // DoCalc()


#if DEBUG_F_TEMPERATURE

//========================================================================
//
// �¶ȼ�⴦��
//
//========================================================================
void TemperatureDeal(void)
{
    // û���¶ȴ�����
    if (!funcCode.code.motorOtMode)
    {
        motorForeOT = 0;   // ����Ԥ������0
        motorOT = 0;       // ���±�����0
        // �¶ȼ��ֵ��0
        tickerTempDeal = 0;
        tempSampleSum = 0;
        temperature = 0;
        
        if (ERROR_MOTOR_OT == errorOther)
        {
            errorOther = ERROR_NONE;
        }
        return;
    }
    // �¶ȴ�����
    TemperatureSensorDeal();   
    // ��ȡ�¶�֮��Ĵ���
    TemperatureAfterDeal();
}


//========================================================================
//
// �¶ȴ���������
//
//========================================================================
void TemperatureSensorDeal()
{
    // �����¶ȴ�����
    if (FUNCCODE_tempSenorType_NONE == funcCode.code.motorOtMode)
    {
        return;
    }

    // AI2 �¶Ȳ���ֵ
    tempSampleSum += aiDeal[2].sample;

    tickerTempDeal++;
    if (tickerTempDeal >= TEMPERATURE_CALC_PERIOD / TEMPERATURE_CALL_PERIOD) // ʱ���
    {
        Uint32 k;
        LINE_STRUCT aiLine = LINE_STRTUCT_DEFALUTS;
        
        Uint16 *p = &funcCode.code.aiCalibrateCurve[2].before1;  // ʵ���ѹ      
#define MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA  3300        // 3300mv

        // ��ȡ�¶ȼ��Ĳ�����ѹ
        k = (4095L << 4) * tickerTempDeal / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;
        temperatureVoltageOrigin = tempSampleSum / k;

        // ����¶ȴ�����AI2����У������
        aiLine.mode = 1;    // ���޷�
        aiLine.y1 = ((int32)(int16)(*(p + 0)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // (����)�����ѹ1����������������ѹ
        aiLine.x1 = ((int32)(int16)(*(p + 1)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // ��Ƶ��������ѹ1(У��ǰ)
        aiLine.y2 = ((int32)(int16)(*(p + 2)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // ���������ѹ2
        aiLine.x2 = ((int32)(int16)(*(p + 3)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // ��Ƶ��������ѹ2(У��ǰ)
        aiLine.x = ((int32)temperatureVoltageOrigin * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;
        aiLine.calc(&aiLine);
        if (aiLine.y < 0)    // �������ܳ��ָ�ֵ�����¶ȴ�������ѹ����Ϊ��
        {
            aiLine.y =0;
        }

        temperatureVoltage = aiLine.y * MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA / 0x7FFF;

        // ���ݲ�����ѹ����ѹ���¶ȵĶ�Ӧ��ϵ������¶�
        GetTemperature();

        tickerTempDeal = 0;  // ����
        tempSampleSum = 0;

        // AI3��ѹ��ʾ
        aiDeal[2].voltageOrigin = temperatureVoltageOrigin;
        aiDeal[2].voltage = temperatureVoltage/10;
    }
}


//========================================================================
//
// ����¶�ֵ
//
//========================================================================
void GetTemperature()
{
    const Uint16 *p;
    Uint16 size;

// �¶ȴ���������
    if (FUNCCODE_tempSenorType_PTC100 == funcCode.code.motorOtMode)
    {
        p = voltageTempPT100;
        size = sizeof(voltageTempPT100);
    }
    else if (FUNCCODE_tempSenorType_PTC1000 == funcCode.code.motorOtMode)
    {
        p = voltageTempPT1000;
        size = sizeof(voltageTempPT1000);
    }

    temperature = GetTemperatureCalc(temperatureVoltage, p, size);
}



// ���ݲ�����ѹ����ѹ���¶ȵĶ�Ӧ��ϵ������¶�
Uint16 GetTemperatureCalc(Uint16 voltage, const Uint16 *p, Uint16 len)
{
    int16 i;
    LINE_STRUCT temperatureLine = LINE_STRTUCT_DEFALUTS;

    for (i = 1; i < len; i++)
    {
        if (voltage < *(p + i))
            break;
    }
    if (i >= len)  // ��������
    {
        i = len - 1;
    }

    temperatureLine.mode = 0;
    temperatureLine.x1 = *(p + i - 1);
    temperatureLine.y1 = (i - 1) * 4;
    temperatureLine.x2 = *(p + i);
    temperatureLine.y2 = i * 4;
    temperatureLine.x = voltage;
    temperatureLine.calc(&temperatureLine);
    
    return (temperatureLine.y);
}


//========================================================================
//
// �¶ȴ���
//
//========================================================================
void TemperatureAfterDeal(void)
{
    // ����Ԥ����
    if (temperature >= funcCode.code.motorOtCoef)
    {
        motorForeOT = 1;
    }
    else
    {
        motorForeOT = 0;
    }

    // ������±���
    if (temperature >= funcCode.code.motorOtProtect)
    {
        motorOT = 1;
    }
    else
    {
        motorOT = 0;
    }

    if (motorOT)
    {
        if (!errorOther)
        {
            errorOther = ERROR_MOTOR_OT;    // ������¹���
        }
    }
}
#else
void TemperatureDeal(void)
{
}
#endif

//=====================================================================
//
// AI������
// AI1 -- A2
// AI2 -- B2
//
//=====================================================================
void AiCalc(void)
{
#if DEBUG_F_AI
    LINE_STRUCT aiLine = LINE_STRTUCT_DEFALUTS;
    Uint16 * pAiSetCurve[5] =               // AI�趨����
    {
        &funcCode.code.curveSet2P1[0],
        &funcCode.code.curveSet2P2[0],
        &funcCode.code.curveSet2P3[0],
        &funcCode.code.curveSet4P1[0],
        &funcCode.code.curveSet4P2[0],
    };
    Uint16 aiFilterTime[AI_NUMBER];
    Uint16 aiNum;
    int32 point;    // AI JUMP�ĵ�
    int32 range;    // AI JUMP�ķ�Χ
    Uint16 *p = &funcCode.code.curveSet2P1[0];  // ��ʱָ��
    int16 i;
    int16 j;
    int32 aiMin;

    if (1 == mainLoopTicker)        // 
    {
        aiLpf[0].out = aiDeal[0].sample;
        aiLpf[1].out = aiDeal[1].sample;
        aiLpf[2].out = aiDeal[2].sample;

        mainLoopTicker = 2;         // �����ܴ���AI�ĵ�1�ģ���˴���֮�󣬿�����ʹ��AI
    }

#if F_DEBUG_RAM
#if 0
#define AI1_NUMBER  256
    static Uint16 ai1Array[AI1_NUMBER];
    Uint32 ai1Sum;
#endif

//#define AI1_RESULT  (AdcRegs.ADCRESULT4)    // AI1��Ӧ��ADCRESULT
//#define AI2_RESULT  (AdcRegs.ADCRESULT5)    // AI2��Ӧ��ADCRESULT
#if 0
#define AI1_RESULT  (AdcRegs.ADCRESULT1)    // AI1��Ӧ��ADCRESULT
#define AI2_RESULT  (AdcRegs.ADCRESULT3)    // AI2��Ӧ��ADCRESULT
#define AI3_RESULT  (AdcRegs.ADCRESULT5)    // AI2��Ӧ��ADCRESULT
    aiDeal[0].sample = AI1_RESULT;                 // AI1�Ĳ���ֵ, Q16
    aiDeal[1].sample = AI2_RESULT;                 // AI2�Ĳ���ֵ, Q16
    aiDeal[2].sample = AI3_RESULT;                 // AI2�Ĳ���ֵ, Q16
#endif    
#if 0
    ai1Sum = 0;
    for (i = AI1_NUMBER-1; i > 0; i--)
    {
        ai1Array[i] = ai1Array[i - 1];
        ai1Sum += ai1Array[i];
    }
    ai1Array[0] = aiDeal[0].sample;
    ai1Sum += ai1Array[0];
    aiDeal[0].sample = ai1Sum / AI1_NUMBER;
#endif
#endif

    aiFilterTime[0] = funcCode.code.ai1FilterTime;
    aiFilterTime[1] = funcCode.code.ai2FilterTime;
    aiFilterTime[2] = funcCode.code.ai3FilterTime;
    
    aiMin = (int32)AI_MIN_VOLTAGE_HW*10;
    for (j = AI_NUMBER - 1; j >= 0; j--)
    //j = 0;
    {
        // AI3Ϊ�¶ȴ�����ʱ���������´���
        if ((j == AI_NUMBER - 1) 
            && (FUNCCODE_tempSenorType_NONE != funcCode.code.motorOtMode)
            )
        {
            aiMin = 0;
            continue;
        }
// AI�˲�
        aiLpf[j].t = aiFilterTime[j] * (TIME_UNIT_AI_PULSE_IN_FILTER / AI_CALC_PERIOD); // �˲�
        aiLpf[j].in = aiDeal[j].sample;      // Q16
        aiLpf[j].calc(&aiLpf[j]);

        aiLine.mode = 0;    // �޷�

        aiLine.x1 = 0;
        aiLine.y1 = aiMin;
        aiLine.x2 = (4095L << 4);
        aiLine.y2 = (int32)AI_MAX_VOLTAGE_HW*10;
        aiLine.x = aiLpf[j].out;
        aiLine.calc(&aiLine);
        aiDeal[j].voltageOrigin = aiLine.y;
        aiMin = 0;
        
// AIУ������
#define SIZE_AI_CALIBRATE   sizeof(struct ANALOG_CALIBRATE_CURVE)   // AIУ�����ߵ�size
        p = (&funcCode.code.aiCalibrateCurve[0].before1 + SIZE_AI_CALIBRATE * j);
#undef SIZE_AI_CALIBRATE
        aiLine.mode = 1;    // ���޷�
        aiLine.y1 = (int16)(*(p + 0));       // ��������������ѹ(У����)
        aiLine.x1 = (int16)(*(p + 1));       // ��Ƶ��������ѹ(У��ǰ) 
        aiLine.y2 = (int16)(*(p + 2));       // ��������������ѹ(У����)
        aiLine.x2 = (int16)(*(p + 3));       // ��Ƶ��������ѹ(У��ǰ)
        aiLine.x = aiDeal[j].voltageOrigin;  // ������ѹ
        aiLine.calc(&aiLine);

        if (aiDeal[j].voltageOrigin == 0)
        {
            aiDeal[j].voltage = 0;   // ������ѹΪ0ʱʵ�ʵ�ѹҲΪ0
        }
        else
        {
            aiDeal[j].voltage = aiLine.y / 10;   // ʵ�ʵ�ѹ
        }
        
// ��AI�趨����
        aiNum = (funcCode.code.aiCurveSrc >> (j * 4)) & 0x000F;
        p = pAiSetCurve[aiNum - 1];
        if (aiNum >= 4)     // 4������ߡ�����4������5Ϊ4������
        {
            // �趨���ߵ��������Ѿ���֤��С����
            for (i = 2; i >= 0; i--) // 4�㣬��3�Ρ�ȷ����ǰAI��ѹ��3���е���һ�Ρ�
            {
                if (aiDeal[j].voltage  >= (int16)(*(p + i*2))) 
                {
                    break;
                }
            }
            if (i < 0)
            {
                i = 0;
            }
            
            p += i * 2;
        }

// AI�趨
        if((aiNum >= 4) && (i > 0))   // Ϊ4�������ҵ�ǰλ�ò��ڵ�һ��
        {
            aiLine.mode = 0;    // �޷�
        }
        else
        {
            if((funcCode.code.aiLimitSrc >> (j * 4)) & 0x000F)
            {
                aiLine.mode = 2;    // �޷��ҵ�������Ϊ0
            }
            else
            {
                aiLine.mode = 0;    // �޷�
            }
        }
        //aiLine.mode = 0;        // �޷�
        aiLine.x1 = ((int32)(int16)(*(p + 0)) * 33553)>>10;  // �Ż�
        aiLine.y1 = ((int32)(int16)(*(p + 1)) * 33553)>>10;  // �Ż�
        aiLine.x2 = ((int32)(int16)(*(p + 2)) * 33553)>>10;  // �Ż�
        aiLine.y2 = ((int32)(int16)(*(p + 3)) * 33553)>>10;  // �Ż�
        //aiLine.x = aiLine.y;
        aiLine.x = aiDeal[j].voltage * 33553>>10;  // Q10 32.767
        aiLine.calc(&aiLine);
        aiDeal[j].set = aiLine.y;

        //aiDeal[j].set = (int32)aiDeal[j].set * 2 - (32767);

// AI��Ծ��
#define SIZE_AI_JUMP sizeof(struct AI_JUMP)     // AI JUMP��size
        p = (&funcCode.code.aiJumpSet[0].point + SIZE_AI_JUMP * j);
        point = (int32)(int16)(*(p + 0)) * 33553 >> 10;   // �趨��Ծ��
        range = (int32)(int16)(*(p + 1)) * 33553 >> 10;   // �趨��Ծ����
        if ((point - range < aiLine.y) && 
            (aiLine.y < point + range)      // ������Ծ��Χ֮��
            ) 
        {
            aiDeal[j].set = point;
        }
#undef SIZE_AI_JUMP
    }
    
    //aiDeal[2].voltageOrigin = aiLpf[2].out * 20000 / (4095L << 4) - 10000; // У׼ǰ��ѹ
#endif
} // AiCalc()


//=====================================================================
//
// AO������
// AO1, AO2(��FMP��ѡһ)
//
//=====================================================================
void AoCalcChannel(Uint16 channel)
{
#if DEBUG_F_AO
    int32 outCoeff;
    Uint32 tmpAo;
    
#if DSP_2803X
	volatile struct EPWM_REGS *EPwmRegs  = &EPwm6Regs;
#else
    volatile struct ECAP_REGS *pECapRegs = &ECap3Regs;
#endif
       
    LINE_STRUCT aoLine = LINE_STRTUCT_DEFALUTS;
    struct ANALOG_CALIBRATE_CURVE *pAo = &funcCode.code.aoCalibrateCurve[0];

    if (AOFMP_AO1 == channel)           // AO1
    {
        ;
    }
    else //if (AOFMP_AO2 == channel)    // AO2
    {
#if DSP_2803X
        EPwmRegs =  &EPwm4Regs;
#else
        pECapRegs = &ECap2Regs;
#endif
    }

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable
        && (AOFMP_AO1 == channel)
        && (funcCode.code.outPortControl >= 10000) 
        )
    {
        aoFmpValue = funcCode.code.ao1Value;
        aoFmpMax = 1000;
    }
    else
#endif        
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[channel], channel);
    }
    outCoeff = (aoFmpValue << 15) / aoFmpMax;   // Q15

    outCoeff = (outCoeff * 1000) / 1047;   // У��1046V��ѹ
// ���������ƫ������
    outCoeff = ((outCoeff * (int32)(int16)funcCode.code.aoPara[channel-1].gain) * 327 >> 15)
        + (((int32)(int16)funcCode.code.aoPara[channel-1].offset * 2097) >> 6);
    
// AOУ������
    // ע�Ᵽ֤��Ҫ���
    // У��Ӧ�÷��ڹ���������֮��
    if (outCoeff < 0)
    {
        outCoeff = 0;
    }
    else if (outCoeff > (Uint32)(1.5*32768))
    {
        outCoeff = (Uint32)(1.5*32768);
    }

    pAo = &funcCode.code.aoCalibrateCurve[channel-1];
    aoLine.mode = 1;    // ���޷�
    aoLine.x1 = ((int32)(int16)pAo->after1  * 13421 ) >> 12;  // ���ñ������ѹ
    aoLine.y1 = ((int32)(int16)pAo->before1 * 13421 ) >> 12;  // ���������ѹ
    aoLine.x2 = ((int32)(int16)pAo->after2  * 13421 ) >> 12;  // ���ñ������ѹ
    aoLine.y2 = ((int32)(int16)pAo->before2 * 13421 ) >> 12;  // ���������ѹ
    aoLine.x = outCoeff;
    aoLine.calc(&aoLine);
    outCoeff = aoLine.y;

    if (outCoeff < 0)
    {
        outCoeff = 0;
    }
    else if (outCoeff >= (1U << 15))
    {
        outCoeff = (1U << 15) + 100;
    }
    // outCoeff > 1�����100%
    // outCoeff=1ʱ����1sysclk�ĵ�����
    
    tmpAo = (outCoeff * (AO_PWM_PERIOD >> 3)) >> (15 - 3);    // 2^15*11*1250 < 2^31

#if DSP_2803X
    EPwmRegs->TBPRD = AO_PWM_PERIOD;    // ��ǰ������Ч
    EPwmRegs->CMPA.half.CMPA  = tmpAo;            // ��һ������Ч
#else
    pECapRegs->CAP1 = AO_PWM_PERIOD;    // ��ǰ������Ч
    pECapRegs->CAP4 = tmpAo;            // ��һ������Ч
#endif
      
#endif
} // AoCalcChannel()


//=====================================================================
//
// pulse in ������
// ʹ��ECAP1��
//
// 1. ����deltaģʽ����ʼ����InitSetEcap4()�С�
// 2. Ŀǰ�ľ���(PulseInSample()��������100us�£������ж�CAPx��ʽ)��
//     < 8KHz������С����0%%������ʾƵ����������
//     < 40KHz����� < 1%%��
//     50-40KHz, ��Լ�� 0.5%%-1%%��������1.5%%����������2%%��
//     ����DSP��PULSE IN ֱ�ӽӵ�DSP��PULSE OUT�����ǳ�С����ʹƵ�ʴﵽ500K��Ҳ�ܺܺõĲ����������ֵ����û�б仯��
// 3. �������в������ò���ֵ���м��㡣
// 4. �������޲��񣬽��й��㡣
//    ��һ��ʱ����û����������(����)����Ϊ����Ƶ��Ϊ0���ɺ�PULSE_IN_ZERO���ã�Ŀǰ����Ϊ1000ms��
// 5. PulseInSample()�ĵ������ڡ�100us��500us���ܱȽϺõ����㾫�ȣ���Ƶʱ500us�����Դ�
// 6. ����CAP����PulseInSample()�н��м�¼��������PulseInCalc()�м�¼��PulseInCalc()�н����м��㡣
//
//=====================================================================
//#define HDI_CAP (ECap4Regs)
void PulseInCalc(void)
{
#if DEBUG_F_HDI
    LINE_STRUCT pulseInLine = LINE_STRTUCT_DEFALUTS;

    static Uint16 pulseCounterRemainder;
#if 1
    static Uint32 lineSpeedCounter;
    static Uint16 lineSpeedCounterRemainder;
    static Uint32 lineSpeedTicker;
#endif
    static Uint16 lengthTickerInOld;
    static Uint16 counterTickerInOld;
    Uint16 pulseCounter;
    Uint16 frq1;
    Uint16 captureTickerTmp = captureTicker;

    Uint16 counterTickerOld;    // counterTicker����
    Uint16 counterTickerDelta;
    Uint16 lengthCurrentOld;    // lengthCurrent����
    Uint16 lengthTickerDelta;

// ���ʱ��(PULSE_IN_CALC_PERIOD)��û�в���noCaptureTicker��1
    if (!captureTicker)
    {
        ++noCaptureTicker;
    }
    
// ǰ�漸�����ںͱ������޲��񣬽��й���
    if (noCaptureTicker)
    {
        frq1 = ((TIME_UNIT_MS_PER_SEC << 1) / (noCaptureTicker * PULSE_IN_CALC_PERIOD) + 1) >> 1;

        if (pulseInFrq1 > frq1)  // ����֮ǰƵ�ʴ��ڹ���Ƶ��ʱ����Ƶ���ڼ�Сʱ���Ž��й��㡣
        {
            pulseInFrq1 = frq1;
        }
    }
    
// �������в���
    if (captureTicker)
    {
#if (DSP_CLOCK == 100)
        pulseInFrq1 = ((((100UL * 1000000UL) << 5) / capturePeriodSum) * captureTicker + (1 << 4)) >> 5;
#elif (DSP_CLOCK == 60)
        pulseInFrq1 = ((((60UL * 1000000UL) << 5) / capturePeriodSum) * captureTicker + (1 << 4)) >> 5;
#endif
        
        capturePeriodSum = 0;   // �����ʱ������
        captureTicker = 0;      // ���������������
        
        noCaptureTicker = 0;    // �в�����noCaptureTicker����
    }

#if (TIME_UNIT_MS_PER_SEC / PULSE_IN_ZERO > 1) // �ܲ�������СƵ�� <= 1Hz������Ҫ����Ƚ�˳��
    if (pulseInFrq1 < TIME_UNIT_MS_PER_SEC / PULSE_IN_ZERO)
    {
        pulseInFrq1 = 0;
    }
#endif

#if 1   // counter�ļ������LPF֮ǰ�������׼ȷ��
    pulseCounter = ((Uint32)pulseInFrq1 * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) / TIME_UNIT_MS_PER_SEC;
    pulseCounterRemainder = ((Uint32)pulseInFrq1 * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) % TIME_UNIT_MS_PER_SEC;

    if (pulseInFrq1 > COUNTER_IN_USE_CAPTURE_TICKER_LIMIT)
    {
        counterTickerDelta = pulseCounter;
        lengthTickerDelta = pulseCounter;
    }
    else    // ���ò��������ʹ�����ȷ
    {
        counterTickerDelta = captureTickerTmp;
        lengthTickerDelta = captureTickerTmp;
    }
#endif

// pulseInFrq�˲�
    pulseInLpf.t = (int32)funcCode.code.pulseInFilterTime * (TIME_UNIT_AI_PULSE_IN_FILTER / PULSE_IN_CALC_PERIOD); // �˲�
    pulseInLpf.in = pulseInFrq1;
    ///
    pulseInLpf.calc(&pulseInLpf);
    pulseInFrq = pulseInLpf.out;

// ����pulseInSet
    pulseInLine.x1 = (int32)(int16)funcCode.code.curveSet2P4[0] * 10 / 2; // pulseInFrq�ĵ�λΪ1Hz����������Ҫ *10
    pulseInLine.x2 = (int32)(int16)funcCode.code.curveSet2P4[2] * 10 / 2;
    pulseInLine.y1 = ((int32)(int16)funcCode.code.curveSet2P4[1] * 33553) >> 10;   //�Ż�����
    pulseInLine.y2 = ((int32)(int16)funcCode.code.curveSet2P4[3] * 33553) >> 10;   //�Ż�����
    
    pulseInLine.x = pulseInFrq / 2;
    pulseInLine.calc(&pulseInLine);
    pulseInSet = pulseInLine.y;

#if 0   // counter�ļ������LPF֮�󣬿��ܲ�׼ȷ��
    pulseCounter = ((Uint32)pulseInFrq * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) / TIME_UNIT_MS_PER_SEC;
    pulseCounterRemainder = ((Uint32)pulseInFrq * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) % TIME_UNIT_MS_PER_SEC;

    if (pulseInFrq > COUNTER_IN_USE_CAPTURE_TICKER_LIMIT)
    {
        counterTickerDelta = pulseCounter;
        lengthTickerDelta = pulseCounter;
    }
    else
    {
        counterTickerDelta = captureTickerTmp;
        lengthTickerDelta = captureTickerTmp;
    }
#endif

#if 1   //+= ���ٶȼ���
#define LINE_SPEED_CALC_PERIOD  100     // _ms����һ��lineSpeed
    lineSpeedCounter += pulseCounter;
    if (++lineSpeedTicker >= LINE_SPEED_CALC_PERIOD / PULSE_IN_CALC_PERIOD) // _ms����һ��
    {
        lineSpeed = (lineSpeedCounter * 10 * 60 * 10 + lineSpeedCounterRemainder) / funcCode.code.lengthPulsePerMeter;
        lineSpeedCounterRemainder = (lineSpeedCounter * 10 * 60 * 10 + lineSpeedCounterRemainder)
            % funcCode.code.lengthPulsePerMeter;
        
        lineSpeedCounter = 0;
        lineSpeedTicker = 0;
    }
#endif

// ����������
    if (DI_FUNC_COUNTER_TICKER_IN != funcCode.code.diFunc[4]) // DI5û��ѡ�����������
    {
        if (counterTickerInOld != diFunc.f1.bit.counterTickerIn) // ����DIҲ��������Ϊ ����������
        {
            counterTickerDelta = diFunc.f1.bit.counterTickerIn; // 1 or 0
            
            counterTickerInOld = diFunc.f1.bit.counterTickerIn;
        }
        else                    // ��������������Ϊ0
        {
            counterTickerDelta = 0;
        }
    }

    counterTickerOld = funcCode.code.counterTicker;
    funcCode.code.counterTicker += counterTickerDelta; // ���������
    bOverFlowCounterTicker = 0;
    if (diFunc.f1.bit.resetCounter)         // ��������λ
    {
        funcCode.code.counterTicker = 0;
    }
    else if (funcCode.code.counterTicker < counterTickerOld)    // ���
    {
        bOverFlowCounterTicker = 1;
    }

// ���ȼ�������
    if (DI_FUNC_LENGTH_TICKER_IN != funcCode.code.diFunc[4])  // ����DIҲ��������Ϊ ���ȼ�������
    {
        if (lengthTickerInOld != diFunc.f1.bit.lengthTickerIn)
        {
            lengthTickerDelta = diFunc.f1.bit.lengthTickerIn;
            
            lengthTickerInOld = diFunc.f1.bit.lengthTickerIn;
        }
        else
        {
            lengthTickerDelta = 0;
        }
    }

    lengthCurrentOld = funcCode.code.lengthCurrent;
    funcCode.code.lengthCurrent += ((Uint32)lengthTickerDelta * 10 + funcCode.code.lengthTickerRemainder) 
        / funcCode.code.lengthPulsePerMeter; // ����ʵ�ʳ��ȡ�10-lengthPulsePerMeter�ĵ�λ
    funcCode.code.lengthTickerRemainder = ((Uint32)lengthTickerDelta * 10 + funcCode.code.lengthTickerRemainder) 
        % funcCode.code.lengthPulsePerMeter;
    bOverFlowLengthCurrent = 0;
    if (diFunc.f1.bit.resetLengthCounter)   // ���ȸ�λ
    {
        funcCode.code.lengthCurrent = 0;
        funcCode.code.lengthTickerRemainder = 0;
    }
    else if (funcCode.code.lengthCurrent < lengthCurrentOld)    // ���
    {
        bOverFlowLengthCurrent = 1;
    }
#endif
} // PulseInCalc()


//=====================================================================
//
// Pulse In ��������
// ʹ��ECAP1��
//
// 1. �����ֵECap1Regs.CAPx������β������ϴβ���֮���delta��
// ��ECap1Regs.ECCTL1.bit.CTRRST1 = EC_DELTA_MODE;
// 
// 2. ���� PULSE_IN_CALC_PERIOD ʱ���ڻ���ÿ��pulse in����capturePeriodSum, captureTicker���㣬
// ����capturePeriodSum, captureTicker���������
//
// 3. �в���ʱ����CAP1/CAP2/CAP3/CAP4���㡣ͨ���ж�CAPx�Ƿ�Ϊ0ȷ���Ƿ����µĲ���
//
// 4. ���ǣ����������Է���0.5ms�����У���ʹ����100us����Ҳ���ԡ�����100us�����о�����ߡ�
//
//=====================================================================
void PulseInSample(void)
{
#if DEBUG_F_HDI
    Uint32 ts1, ts2, ts3, ts4;
#if DSP_2803X
    volatile struct ECAP_REGS *pECapRegs = &ECap1Regs;
#else
    volatile struct ECAP_REGS *pECapRegs = &ECap4Regs;
#endif
    ts1 = pECapRegs->CAP1;
    if (ts1)
    {
        pECapRegs->CAP1 = 0;
        captureTicker++;
    }

    ts2 = pECapRegs->CAP2;
    if (ts2)
    {
        pECapRegs->CAP2 = 0;
        captureTicker++;
    }

    ts3 = pECapRegs->CAP3;
    if (ts3)
    {
        pECapRegs->CAP3 = 0;
        captureTicker++;
    }

    ts4 = pECapRegs->CAP4;
    if (ts4)
    {
        pECapRegs->CAP4 = 0;
        captureTicker++;
    }

    capturePeriodSum += ts1 + ts2 + ts3 + ts4;
#endif
}

#if DSP_2803X
#define HDO_PRD     (EPwm5Regs.TBPRD)
#define HDO_CMP     (EPwm5Regs.CMPB)
#define HDO_CTR     (EPwm5Regs.TBCTR)
#endif
//=====================================================================
//
// FMP������
// 
//=====================================================================
void FMPDeal(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)

#if DSP_2803X
    static Uint16 fmOutSelectOld;

    if (fmOutSelectOld != funcCode.code.fmOutSelect) // F5-00  �๦�ܶ������ѡ��
    {
        fmOutSelectOld = funcCode.code.fmOutSelect;
        EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
        EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
        HDO_CTR = 0;
    }

    if (FUNCCODE_fmOutSelect_PULSE == funcCode.code.fmOutSelect)    // HDO
    {
        PulseOutCalc();
    }
    else if (FUNCCODE_fmOutSelect_DO == funcCode.code.fmOutSelect)  // DO
    {
        HDO_CTR = 0;        // ��counter���ó�period���¡�
        HDO_PRD = PULSE_OUT_CTR_PERIOD;     // period

        if (doStatus.c.bit.do3)                 // ����Ϊ0
        {
            HDO_CMP = PULSE_OUT_CTR_PERIOD + 1;  // CMP >= PERIOD + 1, ǿ�����1
        }
        else                        // Ƶ��Ϊ0��Ƶ��С��һ��ֵ
        {
            HDO_CMP = 0;            // CMP = 0, ǿ�����0
        }
    }
#if 0  // 2803xƽ̨��AO2��FMR�Ƿ����
    else //if (FUNCCODE_fmOutSelect_AO == funcCode.code.fmOutSelect)   // AO2
    {
        AoCalcChannel(AOFMP_AO2);
    }
#endif
#else
    static Uint16 fmOutSelectOld;

    if (fmOutSelectOld != funcCode.code.fmOutSelect) // F5-00  �๦�ܶ������ѡ��
    {
        ECap2Regs.TSCTR = 0;
        fmOutSelectOld = funcCode.code.fmOutSelect;
    }

    if (FUNCCODE_fmOutSelect_PULSE == funcCode.code.fmOutSelect)    // HDO
    {
        PulseOutDeal();
    }
    else if (FUNCCODE_fmOutSelect_DO == funcCode.code.fmOutSelect)  // DO
    {
        // DO3(FM)���������ʹ��APWM�����
        ECap2Regs.TSCTR = 0;                        // ��counter���ó�period���¡�
        ECap2Regs.CAP1 = PULSE_OUT_CTR_PERIOD;      // period
        
        if (doStatus.c.bit.do3)                     // FM(DO3)���ѡ��
        {
            ECap2Regs.CAP2 = PULSE_OUT_CTR_PERIOD + 1;    // CMP >= PERIOD + 1, ǿ�����1
        }
        else
        {
            ECap2Regs.CAP2 = 0;                     // CMP = 0, ǿ�����0
        }
    }
    else //if (FUNCCODE_fmOutSelect_AO == funcCode.code.fmOutSelect)   // AO2
    {
        AoCalcChannel(AOFMP_AO2);
    }
#endif
#endif
}

      
void PulseOutCalc(void)
{
#if DEBUG_F_HDO
#if DSP_2803X
    static Uint16 periodOld = MAX_UINT16;
    static Uint16 beforeFrq = 200;
    static Uint16 CLOCK_BENCH = 1;
    Uint16 period;
    Uint64 tmp;
    Uint16 newCompare;
    Uint16 oldCompare;
    Uint16 HDOCTR;

    Uint16 curFrq;

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable 
        && ((funcCode.code.outPortControl >= 11000) 
            || ((funcCode.code.outPortControl < 10000) && (funcCode.code.outPortControl >= 1000)))
        )
    {
        aoFmpMax = 1000;
        aoFmpValue = funcCode.code.fmpValue;
    }
    else
#endif
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[AOFMP_FMP], AOFMP_FMP);
    }
    curFrq = aoFmpValue * funcCode.code.fmpOutMaxFrq / aoFmpMax;
    HDOCTR = HDO_CTR;
    
    // �ߵ�Ƶ����л�(2KHZΪ�л���)
    if ((curFrq >= 200) && (beforeFrq < 200))
    {
        // ��Ƶ�л�����Ƶ
        if (HDOCTR > 292)
        {
            HDOCTR = 0xFFFF;
        }
        else
        {
            HDOCTR = HDOCTR * 224;
        }
        
        if (periodOld > 292)
        {
            periodOld = 0xFFFF;
        }
        else
        {
            periodOld = periodOld * 224;
        }
        
        //EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
        //EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
        CLOCK_BENCH = 1;
    }
    else if ((curFrq < 200) && (beforeFrq >= 200))
    {
        // ��Ƶ�л�����Ƶ
        HDOCTR = HDOCTR / 224;
        periodOld = periodOld / 224;

        //EPwm4Regs.TBCTL.bit.HSPCLKDIV = 7;          // TBCLK = SYSCLK/28
        //EPwm4Regs.TBCTL.bit.CLKDIV = 4;
        CLOCK_BENCH = 224;
    }

    tmp  = (Uint64)(DSP_CLOCK/10) * 1000000UL * aoFmpMax / (aoFmpValue * funcCode.code.fmpOutMaxFrq);
    tmp /= CLOCK_BENCH;
    if ((tmp > MAX_UINT16) || (!aoFmpValue)) // ����MAX_UINT16������outValue=0��һֱ���0Ƶ��
    {
        tmp = MAX_UINT16;
    }
    period = tmp;
 
    if (period)
        period--;

    oldCompare = periodOld >> 1;
    newCompare = period >> 1;              // Ĭ��ռ�ձ�50%

    if (((MAX_UINT16 - 1) != period) && period)   // ����Ϊ0�����ߴﵽ2^16-1
    {
        // HDOCTR = HDO_CTR;
        // Ƶ�ʸı�
        if (period != HDO_PRD)
        {
            // Ƶ�ʴӸ߱�ɵ�
            if (periodOld < period)     
            { 
                if (HDOCTR > oldCompare)
                {
                    HDOCTR = HDOCTR - oldCompare + newCompare;  // ע���޷�����������
                }		
            }
            // Ƶ�ʴӵͱ�ɸ�(�����ɴ��С)
            else                        
            {   
                // ��ʱ���ں�������ҳ����µİ�����
                if (HDOCTR > newCompare + oldCompare)
                {
                    // ֹͣ��ǰ�������¿�ʼ����
                    HDOCTR = 0;
                }
                // ��ʱ���ں��������δ�����µİ�����
                else if (HDOCTR > oldCompare)
                {
                    // ֱ���Ե�ǰ��ʱֵ��ʼ�µĺ�����ڼ���
                    HDOCTR = HDOCTR - oldCompare + newCompare;
                }
                // ��ʱ����ǰ�������Ҽ�ʱֵ�����µİ�����
                else if (HDOCTR > newCompare)
                {
                    // ��������������
                    HDOCTR = newCompare;
                }
                // ��ʱ����ǰ�������Ҽ�ʱֵС���µİ�����, ֱ���Ե�ǰ��ʱֵ��ʼ�µ�ǰ�����ڼ���
            } 
            
            if (HDOCTR > period)
	        {
	            HDOCTR = 0;
	        }

	        DINT;   
            EALLOW;
			if (CLOCK_BENCH == 1)
            {
                EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
                EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
            }
            else
            {
                EPwm5Regs.TBCTL.bit.HSPCLKDIV = 7;          // TBCLK = SYSCLK/28
                EPwm5Regs.TBCTL.bit.CLKDIV = 4;
            }
            HDO_PRD = period;            // �ٸ�����ֵperiod
			HDO_CMP = newCompare;        // �ȸ��Ƚ�ֵcompare
			HDO_CTR = HDOCTR;
     
            EDIS;
	        EINT;
        }
        
    }
    // ����Ϊ0�����ߴﵽ2^16-1
    else  
    {
        HDO_CTR = 0;        // ��counter���ó�period���¡�
        HDO_PRD = PULSE_OUT_CTR_PERIOD;     // period

        if (!period)                // ����Ϊ0
        {
            HDO_CMP = PULSE_OUT_CTR_PERIOD + 1;  // CMP >= PERIOD + 1, ǿ�����1
        }
        else                        // Ƶ��Ϊ0��Ƶ��С��һ��ֵ
        {
            HDO_CMP = 0;            // CMP = 0, ǿ�����0
        }
    }

    periodOld = period;
    beforeFrq = curFrq;
#endif
#endif
}



// pulse out
// ʹ��ECAP2
//
// ���۸ı�Ƶ�ʵ�ʱ����ǰ������ڣ������ں������ڣ���������ʼ�µ�Ƶ�ʡ�
//#define HDO_PRD     (ECap2Regs.CAP1)
//#define HDO_CMP     (ECap2Regs.CAP2)
//#define HDO_CTR     (ECap2Regs.TSCTR)
#if !DSP_2803X
void PulseOutDeal(void)
{
#if DEBUG_F_HDO
#if !DSP_2803X
    static Uint32 periodOld = MAX_UINT32;
    Uint32 period;
    Uint64 tmp;

    Uint32 newCompare;
    Uint32 oldCompare;

// ����ĳЩ�����������fmpOutMaxFrq = 1��aoFmpMax = 63000��aoFmpValue = 10

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable 
        && ((funcCode.code.outPortControl >= 11000) 
            || ((funcCode.code.outPortControl < 10000) && (funcCode.code.outPortControl >= 1000)))
        )
    {
        aoFmpMax = 1000;
        aoFmpValue = funcCode.code.fmpValue;
    }
    else
#endif
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[AOFMP_FMP], AOFMP_FMP);
    }

// fmpOutMaxFrq��λ��0.01kHz
    tmp  = (Uint64)(DSP_CLOCK/10) * 1000000UL * aoFmpMax / (aoFmpValue * funcCode.code.fmpOutMaxFrq);
    
    if ((tmp > MAX_UINT32) || (!aoFmpValue)) // ����MAX_UINT32������outValue=0��һֱ���0Ƶ��
    {
        tmp = MAX_UINT32;
    }
    
    period = tmp;
    if (period)
        period--;

    oldCompare = periodOld >> 1;
    newCompare = period >> 1;              // Ĭ��ռ�ձ�50%
//    newCompare = (period * 3277) >> 15;   // ռ�ձ�10%
//    newCompare = (period * 1966) >> 16;   // ռ�ձ�3%

    if (((MAX_UINT32-1) != period) && period)   // ����Ϊ0�����ߴﵽ2^32-1
    {
        ECap2Regs.CAP2 = newCompare;    // �ȸ��Ƚ�ֵcompare
        ECap2Regs.CAP1 = period;        // �ٸ�����ֵperiod

        if (periodOld < period)     // Ƶ�ʴӸ߱�ɵͣ����ڴӶ̱䳤
        {
            DINT;                   // ��ʱӦ�ùر�ȫ���жϣ�Ҳ����ͣ������
            
            /*
            * ���۸ı�Ƶ�ʵ�ʱ����ǰ�������
            * �����ں�������
            * ��������ʼ�µ�Ƶ�ʡ�
            */
            if (ECap2Regs.TSCTR > oldCompare)
                ECap2Regs.TSCTR = ECap2Regs.TSCTR - oldCompare + newCompare; // ע���޷�����������
                
            EINT;                   // ���¿���ȫ���ж�
        }
        else if (periodOld > period)// Ƶ�ʴӵͱ�ɸߣ����ڴӳ����
        {
            DINT;                   // ��ʱӦ�ùر�ȫ���жϣ�Ҳ����ͣ������
            
            /* 
            * �ı�Ƶ�ʵ�ʱ���ں�������
            */
            // ���ں������ڵ�ʱ�䳬���µİ�����ʱ�䣬
            if (ECap2Regs.TSCTR > newCompare + oldCompare)
            {
                // ���������µ�ǰ�����ڡ�
                ECap2Regs.TSCTR = 0;
            }
            // ���ں������ڵ�ʱ��û�г����µİ�����ʱ�䣬
            else if (ECap2Regs.TSCTR > oldCompare)
            {
                // ������ʼ�µ�Ƶ��
                ECap2Regs.TSCTR = ECap2Regs.TSCTR - oldCompare + newCompare;
            }
            
            /*
            * �ı�Ƶ�ʵ�ʱ����ǰ�������
            */
            // ����ǰ������ڵ�ʱ�䳬���µİ�����ʱ�䣬
            else if (ECap2Regs.TSCTR > newCompare)
            {
                // ���������µĺ������
                ECap2Regs.TSCTR = newCompare;
            }
            // ����ǰ������ڵ�ʱ��û�г����µİ�����ʱ�䣬������ʼ�µ�Ƶ�ʡ�

            // ���¿���ȫ���ж�
            EINT;                   
        }
        // ����ֵ(�������Ƶ��)δ�ı䣬����������ֵ��

        if (ECap2Regs.TSCTR > period)
        {
            ECap2Regs.TSCTR = 0;
        }
    }
    else
    {
        ECap2Regs.TSCTR = 0;        // ��counter���ó�period���¡�
        ECap2Regs.CAP1 = PULSE_OUT_CTR_PERIOD;     // period

        if (!period)                // ����Ϊ0
        {
            ECap2Regs.CAP2 = PULSE_OUT_CTR_PERIOD + 1; // CMP >= PERIOD + 1, ǿ�����1
        }
        else                        // Ƶ��Ϊ0��Ƶ��С��һ��ֵ
        {
            ECap2Regs.CAP2 = 0;     // CMP = 0, ǿ�����0
        }
    }

    periodOld = period;
#endif
#endif
} // FMPDeal()
#endif

//=====================================================================
//
// ����FMP(Pulse Out)��AO�����ֵ��������ֵ
//
// ���:
//      aoFmpValue      FMP,AO�����ֵ
//      aoFmpMax        FMP,AO��������ֵ
//
// ��ע��
// 1�����������ͣ��ʱ������outCurrent���ܲ�Ϊ0�����Ի��������������:
// ��ʾ����Ϊ0����PULSE OUT���в��ȵ����������
// ������ϵĻ������������������
// 
// 2����F3-00(�๦�ܶ������ѡ��)�ı�ʱ����Ҫ��APWM������ֵ��ֵ֮��
// ��������TSCTR���㡣
//
//=====================================================================
LOCALF void UpdateFmpAoValue(Uint16 func, Uint16 aoOrFmp)
{
#if (DEBUG_F_AO || DEBUG_F_HDO)
    switch (func)
    {
        case AO_FMP_FUNC_FRQ_SET:       // ����Ƶ��
            aoFmpValue = frqDisp;
            //aoFmpValue = frq;
            //+= aoFmpValue = frqRun;
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_FRQ_AIM:            // �趨Ƶ��
            aoFmpValue = frqAim; 
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_OUT_CURRENT:           // �������
            aoFmpValue = outCurrentDisp;        // ʵ�ʵ���
            aoFmpMax = motorFc.motorPara.elem.ratingCurrent << 1;   // 2����������
            break;

        // 0~200.0%��Ӧ0~10V
        case AO_FMP_FUNC_OUT_TORQUE:    // ���ת��
            aoFmpValue = ABS_INT16((int16)itDisp);
            aoFmpMax = 2000;            // 2������ת��
            break;    

        case AO_FMP_FUNC_OUT_POWER:     // �������
            aoFmpValue = outPower >> 1;
            aoFmpMax = motorFc.motorPara.elem.ratingPower;
            break;

        case AO_FMP_FUNC_OUT_VOLTAGE:       // �����ѹ
            aoFmpValue = outVoltage;        // Q12
            aoFmpMax = (int32)(1.2*4096);   // 1.2����Ƶ�����ѹ
            break;

        case AO_FMP_FUNC_PULSE_IN:          // PULSE��������
            aoFmpValue = pulseInFrq / 2;    // 1Hz
            aoFmpMax = 50000;               // 50kHz
            break;
            
        case AO_FMP_FUNC_AI1:           // AI1
        case AO_FMP_FUNC_AI2:           // AI2
        case AO_FMP_FUNC_AI3:           // AI3
            aoFmpValue = aiDeal[func - AO_FMP_FUNC_AI1].voltage;    // 0.01V
            aoFmpMax = 1000;                                        // ���10.00V
            break;
            
        case AO_FMP_FUNC_LENGTH:        // ����
            aoFmpValue = funcCode.code.lengthCurrent;
            aoFmpMax = funcCode.code.lengthSet;
            break;

        case AO_FMP_FUNC_COUNTER:       // ����ֵ
            aoFmpValue = funcCode.code.counterTicker;
            aoFmpMax = funcCode.code.counterSet;
            break;

        case AO_FMP_FUNC_COMM:          // ͨѶ�������
            aoFmpValue = aoComm[aoOrFmp];
            aoFmpMax = 0x7FFF;
            break;
            
        case AO_FMP_FUNC_SPEED:         // �������ת��
            aoFmpValue = frqRun;
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_OUT_CURRENT_1:
            aoFmpValue = outCurrentDisp;        // ʵ�ʵ���
            aoFmpMax = 10000;                   // 1000A 2����������
             break;

        case AO_FMP_FUNC_OUT_VOLTAGE_1:
            aoFmpValue = generatrixVoltage;
            aoFmpMax = 10000;                   // ������Ϊ1000.0V
            break;

        // ���ת��(������) -200.0%~200.0%��Ӧ0~10V
        case AO_FMP_FUNC_OUT_TORQUE_DIR:
            aoFmpValue = (int16)(itDisp) + 2000;
            aoFmpMax = 4000;                    // 2������ת��
            break;
            
        default:
            break;
    }

    aoFmpValue = ABS_INT32(aoFmpValue);
    aoFmpMax = ABS_INT32(aoFmpMax);
    
    if (aoFmpValue > aoFmpMax)      // �޷�
    {
        aoFmpValue = aoFmpMax;
    }
#endif
}


//=====================================================================
//
// �൱��һ���߶Σ���֪���˵�(x1,y1), (x2,y2), �������(x, y)��y
// ��ȷ��: x1 < x2
// �����Q4��ʽ
//
// 1000 * 2^6 = 64000 < 65536�����������
// pulseInFrqĿǰ���Ϊ50000Hz��Ҳ���������
//
//=====================================================================
void LineCalc(LINE_STRUCT *p)
{
#if 0   // ��x1 < x2. ��x > x1, y = y2; ��x <= x1, y = y1.
    if (p->x <= p->x1)
        p->y = p->y1;
    else if (p->x >= p->x2)
        p->y = p->y2;
    else
        p->y = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
#elif 1
    int32 tmp;
    tmp = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
    
    // ��x1 < x2. ��x > x1, y = y2; ��x <= x1, y = y1.
    if((p->mode == 2) && (p->x < p->x1))    // �޷��ҵ�������ʱΪ0
        p->y = 0;
    else if (p->x <= p->x1)
        p->y = p->y1;
    else if (p->x >= p->x2)
        p->y = p->y2;
    else
        p->y = tmp;
    
    if (p->mode == 1)    // 1����ʾ���޷�
        p->y = tmp;
#elif 1   //+e �ɴ���ѧ�Ϸ�Ӧx1 < x2����������ݲ�����
    int32 max, min;

    max = MAX(p->x1, p->x2);
    min = MIN(p->x1, p->x2);

    if (p->x <= min)
        p->y = p->y1;
    else if (p->x >= max)
        p->y = p->y2;
    else
        p->y = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
#endif
}


//=====================================================================
//
// ��ͨ�˲�������
// ע��:
//
//=====================================================================
void LpfCalc(LowPassFilter *p)
{
#if 0   // ��һ��Ч�������һ�㣬ԭ���Ժ��ٲ���. ������ܳ���32767, 65535����bug
    int32 tmp;
    int16 t1 = p->t * TIME_UNIT_AI_PULSE_IN_FILTER;

    p->outOld = p->out;
    if (!p->t)
    {
        p->out = p->in;
    }
    else
    {
        tmp = ((int32)p->in - p->outOld + p->remainder) * AI_CALC_PERIOD;
        p->out = tmp / t1 + p->outOld;
        p->remainder = tmp % t1;
    }
/* #elif 0                                         */
/*     int32 tmp;                                  */
/*                                                 */
/*     p->outOld = p->out;                         */
/*     if (!p->t)                                  */
/*     {                                           */
/*         p->out = p->in;                         */
/*     }                                           */
/*     else                                        */
/*     {                                           */
/*         tmp = p->in + p->remainder - p->outOld; */
/*         p->out = tmp / p->t + p->outOld;        */
/*         p->remainder = tmp % p->t;              */
/*     }                                           */
#elif 1
    int32 tmp;
    int32 out;

    p->outOld = p->out;
    if (!p->t)
    {
        p->out = p->in;
    }
    else
    {
        tmp = ((int32)p->in - p->outOld + p->remainder) * AI_CALC_PERIOD;
        out = tmp / (p->t * 1) + p->outOld;
 
            
            
        p->out = tmp / (p->t * 1) + p->outOld;
        p->remainder = tmp % (p->t * 1);
    }
#endif
}

////////////////////////////////////////////////////////////////
//
//  2803x AO2
//
////////////////////////////////////////////////////////////////
void InitSetEPWM4(void)
{
#if (DEBUG_F_AO)  
#if DSP_2803X
    EALLOW;
    EPwm4Regs.TBPRD = 0;                    // Period = 601 TBCLK counts
    EPwm4Regs.CMPA.half.CMPA = 0;           // Compare A = 350 TBCLK counts
    EPwm4Regs.TBPHS.all = 0;                // Set Phase register to zero
    EPwm4Regs.TBCTR = 0;                    // clear TB counter
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    //EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;
    //EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EDIS;
#endif
#endif
}

///////////////////////////////////////////////
//
// 2803xƽ̨HDO1
//
///////////////////////////////////////////////
void InitSetEPWM5(void)
{
 #if ( DEBUG_F_HDO || DEBUG_F_DO)
 #if DSP_2803X
    EALLOW;
    EPwm5Regs.TBPRD = 0;                   // Period = 601 TBCLK counts
    EPwm5Regs.CMPB = 0;                    // Compare B = 200 TBCLK counts
    EPwm5Regs.TBPHS.all = 0;               // Set Phase register to zero
    EPwm5Regs.TBCTR = 0;                   // clear TB counter
    
    EPwm5Regs.TBCTL.bit.FREE_SOFT = 2;
    
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm5Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_IMMEDIATE;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_IMMEDIATE;
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    //EPwm5Regs.AQCTLA.bit.ZRO = AQ_SET;
    //EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm5Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm5Regs.AQCTLB.bit.CBU = AQ_SET;

    EDIS;
#endif
#endif
}

////////////////////////////////////////////////////////////////
//
//  2803x AO1
//
////////////////////////////////////////////////////////////////
void InitSetEPWM6(void)
{
#if (DEBUG_F_AO)  
#if DSP_2803X
    EALLOW;
    EPwm6Regs.TBPRD = 0;                // Period = 601 TBCLK count
    EPwm6Regs.CMPA.half.CMPA = 0;          // Compare A = 350 TBCLK counts
    EPwm6Regs.TBPHS.all = 0;               // Set Phase register to zero
    EPwm6Regs.TBCTR = 0;                   // clear TB counter
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm6Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm6Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    //EPwm6Regs.AQCTLB.bit.ZRO = AQ_SET;
    //EPwm6Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EDIS;
#endif
#endif
}

// PULSE OUT, AO��DO3(FM)
#if !DSP_2803X
void InitSetEcap2(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)
#if !DSP_2803X
// Setup APWM mode on CAP2, set period and compare registers
    ECap2Regs.ECCTL1.bit.FREE_SOFT = 2;  // halt DSPʱ���ü�������������
    ECap2Regs.ECCLR.all = 0xFFFF;        // Clear pending interrupts
    ECap2Regs.ECCTL2.all = 0x0216;

    //ECap2Regs.CAP1 = 0xffffffff;         // Set Period value
    ECap2Regs.CAP1 = 0;
    ECap2Regs.CAP2 = 0;                  // Set Compare value����ʼ�����0
#endif
#endif
}
#endif

////////////////////////////////////////////////////////////////
//
//  2808 AO1
//
////////////////////////////////////////////////////////////////
#if !DSP_2803X
void InitSetEcap3(void)
{
#if DEBUG_F_AO
#if !DSP_2803X
    ECap3Regs.ECCTL1.bit.FREE_SOFT = 2;  // halt DSPʱ���ü�������������
    
    ECap3Regs.ECCLR.all = 0xFFFF;        // Clear pending interrupts
    ECap3Regs.ECCTL2.all = 0x0216;

    ECap3Regs.CAP1 = 0;                  // Set Period value
    ECap3Regs.CAP2 = 0;                  // Set Compare value����ʼ�����0
#endif
#endif
}
#endif

/////////////////////////////////////////////
//
// PULSE IN(2803x)
//
/////////////////////////////////////////////
void InitSetEcap1(void)
{
#if DEBUG_F_HDI
#if DSP_2803X
    ECap1Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags   
    ECap1Regs.ECCTL1.all = 0x8000;             // Disable CAP1-CAP4 register loads, halt DSPʱ���ü�������������
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = EC_FREEZE;// Make sure the counter is stopped

    ECap1Regs.ECCTL1.all = 0x81AA;
    ECap1Regs.ECCTL2.all = 0x0096;
#endif
#endif
}

/////////////////////////////////////////////
//
// PULSE IN(2808)
//
/////////////////////////////////////////////
#if !DSP_2803X
void InitSetEcap4(void)
{
#if DEBUG_F_HDI || DEBUG_F_POSITION_CTRL
    ECap4Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
    ECap4Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags   
    ECap4Regs.ECCTL1.all = 0x8000;             // Disable CAP1-CAP4 register loads, halt DSPʱ���ü�������������
    ECap4Regs.ECCTL2.bit.TSCTRSTOP = EC_FREEZE;// Make sure the counter is stopped

    ECap4Regs.ECCTL1.all = 0x81AA;
    ECap4Regs.ECCTL2.all = 0x0096;
#endif
}
#endif



void InitDIGpio(void)
{
#if DSP_2803X     // 2803x����2808ƽ̨
#if DEBUG_F_DI
    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0;    // Configure GPIO42, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO42 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 0;    // Configure GPIO43, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO43 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO43 = 0;     // 

    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;    // Configure GPIO27, DI
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 0;     // input
    GpioDataRegs.GPADAT.bit.GPIO27 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;    // Configure GPIO40, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO40 = 0;     // 

    //GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;    // Configure GPIO19, DI
    //GpioCtrlRegs.GPADIR.bit.GPIO19 = 0;     // input
    //GpioDataRegs.GPADAT.bit.GPIO19 = 0;     // 

    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;    // Configure GPIO25, DI
    GpioCtrlRegs.GPADIR.bit.GPIO25 = 0;     // input
    GpioDataRegs.GPADAT.bit.GPIO25 = 0;     // 

    // DI7
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0;    // Configure GPIO44, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO44 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO35 = 0;    // Configure GPIO35, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO35 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO35 = 0;     //

    GpioCtrlRegs.GPBMUX1.bit.GPIO36 = 0;    // Configure GPIO36, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO36 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO36 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO38 = 0;    // Configure GPIO38, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO38 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO38 = 0;     //
    EDIS;
#endif 
#endif
}

void InitDOGpio(void)
{
#if DSP_2803X     // 2803x����2808ƽ̨
#if DEBUG_F_DO
    EALLOW;
    // D01
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;    // Configure GPIO34, DO
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO34 = 1;     // 

    // DO2
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;    // Configure GPIO24, DO
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;     // output
    GpioDataRegs.GPADAT.bit.GPIO24 = 1;     // 
    
    // relay1
    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;    // Configure GPIO41, DO
    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO41 = 1;     // Ϊ1��ʾ��Ч

    // relay2
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;    // Configure GPIO26, DO
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;     // output
    GpioDataRegs.GPADAT.bit.GPIO26 = 1;     // 

    // FAN
    GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
    GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 

    //HDO1
    //GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;    // Configure GPIO09, DO
    //GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;     // output
    //GpioDataRegs.GPADAT.bit.GPIO9 = 0;     // 
    EDIS;
#endif    
#endif
}


void InitECap2Gpio(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)
#if !DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;      // Disable pull-up on GPIO7 (CAP2)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;    // Synch to SYSCLKOUT GPIO7 (CAP2)
   GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 3;     // Configure GPIO7 as CAP2
EDIS;
#endif
#endif
}


//=============================================
//
// 2803xƽ̨HDO1
//
//=============================================
void InitEPwm5Gpio(void)
{
#if (DEBUG_F_HDO || DEBUG_F_DO)
#if DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;      // Disable pull-up on GPIO9 (ePWM5)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;    // Synch to SYSCLKOUT GPIO9 (ePWM5)
   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;     // 00:GPIO9   01:EPWM5B  10:LINTXA  11:Reserved
   GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
   
EDIS;
#endif
#endif
}

//=============================================
//
// 2803xƽ̨AO2
//
//=============================================
void InitEPwm4Gpio(void)
{
#if (DEBUG_F_AO)
#if DSP_2803X
   EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;     // Disable pull-up on GPIO6 (ePWM4)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;   // Synch to SYSCLKOUT GPIO6 (ePWM4)
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;    // 00:GPIO6   01:EPWM4A  10:SPIDTED  11:ECAP2
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
    EDIS;
#endif
#endif
}

//=============================================
//
// 2803xƽ̨AO1
//
//=============================================
void InitEPwm6Gpio(void)
{
#if (DEBUG_F_AO)
#if DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;     // Disable pull-up on GPIO10 (ePWM6)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 0;   // Synch to SYSCLKOUT GPIO10 (ePWM6)
   GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;    // 00:GPIO10   01:EPWM6A  10:SPIDTED  11:ECAP2
   GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;
EDIS;
#endif
#endif
}


//=============================================
//
// PULSE IN(2808)
//
//=============================================
void InitECap3Gpio(void)
{
#if DEBUG_F_AO
#if !DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;      // Enable pull-up on GPIO9 (CAP3)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;    // Synch to SYSCLKOUT GPIO9 (CAP3)
   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 3;     // Configure GPIO9 as CAP3
EDIS;
#endif
#endif
}


//=============================================
//
// PULSE IN(2803x)
//
//=============================================
void InitECap1Gpio(void)
{
#if (DEBUG_F_HDI || DEBUG_F_DI)
#if DSP_2803X
   EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;   // Enable pull-up on GPIO19 (CAP1)
   GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 0; // Synch to SYSCLKOUT GPIO19 (CAP1)
   GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 3;  // Configure GPIO19 as CAP1 
   EDIS;
#endif
#endif
}   

//=============================================
//
// PULSE IN(2808)
//
//=============================================
void InitECap4Gpio(void)
{
#if DEBUG_F_HDI || DEBUG_F_POSITION_CTRL
#if !DSP_2803X
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;   // Enable pull-up on GPIO11 (CAP4)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 0; // Synch to SYSCLKOUT GPIO11 (CAP4)
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 3;  // Configure GPIO11 as CAP4
    EDIS;
#endif
#endif
}   

// Ϊ�˼�Сʱ��
// ���ڹ�����ı�ʱ����ת����bin
LOCALF void Di2Bin(void)
{
#if DEBUG_F_DI
    static Uint16 diLogicOld[2];
    static Uint16 diLogicAiAsDiOld;
    static Uint16 vdiSrcOld;
    static Uint16 vdiFcSetOld;
    
    if ((diLogicOld[0] != funcCode.code.diLogic[0]) ||
        (diLogicOld[1] != funcCode.code.diLogic[1]) ||
        (diLogicAiAsDiOld != funcCode.code.diLogicAiAsDi)
        )
    {
        diLogicOld[0] = funcCode.code.diLogic[0];
        diLogicOld[1] = funcCode.code.diLogic[1];
        diLogicAiAsDiOld = funcCode.code.diLogicAiAsDi;
        
        diLogic = FcDigit2Bin(funcCode.code.diLogic[0]) + 
        ((FcDigit2Bin(funcCode.code.diLogic[1])) << 5) + 
        ((Uint32)(FcDigit2Bin(funcCode.code.diLogicAiAsDi)) << (DI_NUMBER_PHSIC + DI_NUMBER_V));
    }

    if (vdiSrcOld != funcCode.code.vdiSrc)
    {
        vdiSrcOld = funcCode.code.vdiSrc;
        
        vdiSrc = FcDigit2Bin(funcCode.code.vdiSrc);
    }

    if (vdiFcSetOld != funcCode.code.vdiFcSet)
    {
        vdiFcSetOld = funcCode.code.vdiFcSet;
        
        vdiFcSet = FcDigit2Bin(funcCode.code.vdiFcSet);
    }
#endif
}

//=====================================================================
//
// ���DI����״̬(2803xƽ̨)
//
//=====================================================================
LOCALF void getDiHwStatus()
{
#if DEBUG_F_DI
#if DSP_2803X
	diHwStatus.all = 0;
    diHwStatus.bit.di1 |= GpioDataRegs.GPBDAT.bit.GPIO42;
    diHwStatus.bit.di2 |= GpioDataRegs.GPBDAT.bit.GPIO43;
    diHwStatus.bit.di3 |= GpioDataRegs.GPADAT.bit.GPIO27;
    diHwStatus.bit.di4 |= GpioDataRegs.GPBDAT.bit.GPIO40;
    diHwStatus.bit.di5 |= GpioDataRegs.GPADAT.bit.GPIO19;
    diHwStatus.bit.di6 |= GpioDataRegs.GPADAT.bit.GPIO25;
    diHwStatus.bit.di7 |= GpioDataRegs.GPBDAT.bit.GPIO44;
    diHwStatus.bit.di8 |= GpioDataRegs.GPBDAT.bit.GPIO35;
    diHwStatus.bit.di9 |= GpioDataRegs.GPBDAT.bit.GPIO36;
    diHwStatus.bit.di10 |= GpioDataRegs.GPBDAT.bit.GPIO38;   
#endif    
#endif
}

//=====================================================================
//
// ����DO����״̬(2803xƽ̨)
//
//=====================================================================
LOCALF void setDOStatus()
{
#if DEBUG_F_DO
#if DSP_2803X
EALLOW;
    //GpioDataRegs.GPBDAT.bit.GPIO34 = doHwStatus.bit.do1;
    //GpioDataRegs.GPADAT.bit.GPIO24 = doHwStatus.bit.do2;    
	//GpioDataRegs.GPBDAT.bit.GPIO37 = !doHwStatus.bit.fan; 		
    //GpioDataRegs.GPADAT.bit.GPIO26 = !doHwStatus.bit.relay2;  
	//GpioDataRegs.GPBDAT.bit.GPIO41 = !doHwStatus.bit.relay1; 
	
	doHwStatus.bit.do1 ? (GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1): (GpioDataRegs.GPBSET.bit.GPIO34 = 1);
	doHwStatus.bit.do2 ? (GpioDataRegs.GPACLEAR.bit.GPIO24 = 1): (GpioDataRegs.GPASET.bit.GPIO24 = 1);

	doHwStatus.bit.fan ? (GpioDataRegs.GPBCLEAR.bit.GPIO37 = 1): (GpioDataRegs.GPBSET.bit.GPIO37 = 1); //MD380 ���ȿ���  

	doHwStatus.bit.relay1 ? (GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1): (GpioDataRegs.GPBSET.bit.GPIO41 = 1);
	doHwStatus.bit.relay2 ? (GpioDataRegs.GPACLEAR.bit.GPIO26 = 1): (GpioDataRegs.GPASET.bit.GPIO26 = 1);
EDIS;
#endif
#endif
}

// Ϊ�˼�Сʱ��
// ���ڹ�����ı�ʱ����ת����bin
LOCALF void Do2Bin(void)
{
#if DEBUG_F_DO
    static Uint16 doLogicOld;
    static Uint16 vdoLogicOld;

    if ((doLogicOld != funcCode.code.doLogic) ||
        (vdoLogicOld != funcCode.code.vdoLogic)
        )
    {
        doLogicOld = funcCode.code.doLogic;
        vdoLogicOld = funcCode.code.vdoLogic;
        
        doLogic = FcDigit2Bin(funcCode.code.doLogic) + 
            ((FcDigit2Bin(funcCode.code.vdoLogic)) << 5);
    }
#endif
}








