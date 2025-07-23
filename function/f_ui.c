//======================================================================
//
// Time-stamp: <2012-12-20 14:01:46  Shisheng.Zhi, 0354>
//
// UI, user interface
// ĿǰΪ ��ʾ����, ���̴���
//
//======================================================================


#include "f_ui.h"
#include "f_menu.h"
#include "f_frqSrc.h"
#include "f_comm.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_error.h"

Uint16 flashMaxSDSD = 10;

#if F_DEBUG_RAM

#define DEBUG_F_KEY_ANIMATE                     0   // ģ�ⰴ������
#define DEBUG_F_NEWMENU_USE_DOT_REVERSE_DIR     0   // ���ұ�����ܵ�С���㣬��ʾ�趨Ƶ�ʷ���
                                                    // ���������ܵ�С���㣬��ʾ˲ʱƵ�ʷ���
#define DEBUG_F_ERROR_INDICATION                1   // ����ָʾ�ƣ�Ŀǰʹ��tune/tc������˸
#elif 1

#define DEBUG_F_KEY_ANIMATE                     0   // ģ�ⰴ������
#define DEBUG_F_NEWMENU_USE_DOT_REVERSE_DIR     0

//+=M
#define DEBUG_F_ERROR_INDICATION                1   // ����ָʾ�ƣ�Ŀǰʹ��tune/tc������˸
#endif

//=============================================================================
//
// ����/���ü����ж�
//
//=============================================================================
#if DSP_2803X
#define KEY_SEL_TEST_TIME  750   // 350*2ms
Uint16 keySelStatus;
Uint16 keySelType;
Uint16 keySelTypeBak;
Uint16 keyChecking;
#endif

struct SPI_IN_OUT spiData;

const Uint16 dispCodeMenuMode[6][4] = 
{
    {DISPLAY_F,     DISPLAY_u,      DISPLAY_n,      DISPLAY_C},     // Func base
    {DISPLAY_U,     DISPLAY_5,      DISPLAY_E,      DISPLAY_t},     // Uset user
    {DISPLAY_U,     DISPLAY_LINE,   DISPLAY_LINE,   DISPLAY_C},     // U--C --C--
};

Uint16 onQuickTicker;               // ����QUICK
Uint16 onQuickTickerMax = 1000;     // *2ms

Uint16 onShiftPassViewTicker;
Uint16 onShiftPassViewTickerMax = 1000;

Uint16 factoryPwd = 0;

typedef struct
{
    Uint16 ctrlCnt;     // �ϵ����ĸ���
    Uint16 ctrlVal;     // ���ɨ�赽���ϵ�����ֵ
    Uint16 funcCnt;     // ���ܼ��ĸ���
    Uint16 funcVal;     // ���ɨ�赽�Ĺ��ܼ���ֵ
} keyRet;

// ������ʾ�ַ�����
const Uint16 DISPLAY_CODE[DISPLAY_8LED_CODE_NUM]=
{   0xc0,   0xf9,   0xa4,   0xb0,   0x99,   // 0-4
//  0,      1,      2,      3,      4,  
    0x92,   0x82,   0xf8,   0x80,   0x90,   // 5-9
//  5,      6,      7,      8,      9,  
    0x88,   0x83,   0xc6,   0xa1,   0x86,   // 10-14
//  A,      b,      C,      d,      E,
    0x8e,   0x8c,   0x89,   0xf1,   0xc7,   // 15-19
//  F,      P,      H,      J       L,             
    0xAB,   0xaf,   0xc1,   0x91,   0xc2,   // 20-24 
//  n,      r,      U,      y,      G, 
    0x8b,   0xcf,   0xc8,   0xa3,   0x98,   // 25-29   
//  h,      I,      N,      o,      q       
    0xce,   0x87,   0xe3,   0xff,   0xbf,   // 30-34
//  T,      t,      u/v     ȫ��,   - 
    0x7f,   0x00,   0xa7                    // 35-37
//  С����, ȫ��8.  Сдc
};

// LED
const Uint16 LED_CODE[DISPLAY_LED_CODE_NUM] =
{0xBF,   0xDF,  0xEF, 0xF7, 0xFB, 0xFD, 0xFE, 0xFC, 0xF9, 0x80, 0xFF};
//Run,  Local,  Dir,  TUNE,    V,    A,   Hz,  RPM,    %, ȫ��, ȫ��


// �����й���8LED(�����)��λ����
// Ӧ��ͳһ��!
//
// �����:           �ڿڿڿڿ�
// displayBuffer:    0 1 2 3 4
// digit:            4 3 2 1 0
// operateDigit:     4 3 2 1 0
// scanStep:         0 1 2 3 4  5
// scanStepΪ0��ɨ����ʾ����ߵ������
// scanStepΪ5��ɨ����ʾled��
// winkFlag:    bit7,�������ߵ�1��(�����); bit3, �������ߵ�5��(���ұ�)
// winkFlagLed: bit0-Led0;bit1-Led1,...(LED_RUN, ...)
//



Uint16 displayBuffer[DISPLAY_8LED_NUM + 1] = {0xbf, 0x89, 0xBF, 0xC6, 0xbf, 0x80};    // -H-C-, LEDȫ��

 
Uint16 scanStep;        //  ��ǰɨ��,��ʾ��scanStep(0-4)������ܺ�LED
// 8LED:        �ڿڿڿڿ� led
// scanStep:    0 1 2 3 4  5
// scanStepΪ0��ɨ����ʾ����ߵ������
// scanStepΪ5��ɨ����ʾled��

Uint16 keyBordTestFlag;     // ���̲��Ա�־  0-��������������  1-������������
Uint16 keyBordValue;        // ��¼�Ѱ�����Ϣ

LOCALF Uint16 keyPreFunc;       // ��ǰ���ܼ�ֵ
Uint16 keyPreCtrl;              // ��ǰ�ϵ���ֵ
Uint16 keyFunc;                 // 

LOCALF Uint16 keyAge;           // �������䣬������������ʱ��
LOCALF Uint16 bKeyEsc;          // �����ͷű�־��0--���ͷ�
LOCALF Uint16 bKeyProc;         // ������Ч��־��1--����ִ��

enum KEY_STATUS
{
    KEY_STATUS_JITTER_DEAL,     // ȥ���׶�
    KEY_STATUS_WAIT_REPEAT,     // �ظ�ǰ���ӻ��׶�
    KEY_STATUS_REPEAT,          // �����ظ��Ľ׶�
    KEY_STATUS_WAIT_CARRY       // ������λ����ͣ�׶�
};
LOCALF enum KEY_STATUS keyStatus;   // ����״̬

union DI_HW_STATUS diHwStatus;              // DI��Ӳ��״̬��˲ʱ��DI״̬
LOCALF Uint16 keyValue;         // ��������

enum EN_FLASH
{
    FLASH_YES,
    FLASH_NO
};
LOCALF enum EN_FLASH bEnflash;         // ������˸�ܱ�־��Ĭ��Ϊ��˸
LOCALF Uint16 flashTicker[DISPLAY_8LED_NUM];
LOCALF Uint16 bLight[DISPLAY_8LED_NUM];     // ��˸�������־��1-����0-��
LOCALF Uint16 flashTickerLed[DISPLAY_LED_NUM];

#if 0
LOCALF const Uint16 flashTickerMax[DISPLAY_8LED_NUM] = {40, 40, 40, 40, 40}; // ��˸ʱ�� _ * 6 * 2ms
LOCALF const Uint16 flashTickerLedMax[DISPLAY_LED_NUM] = {45, 45, 45, 45, 45};
#elif 1       // ʹ��define���ɡ����ﲻ��Ҫ��ͬ����˸ʱ��
#define flashTickerMax      42      // ��˸(һ��һ��)ʱ�䣬42*6*2*2ms = 1008ms
#define flashTickerLedMax   45      // ��˸(һ��һ��)ʱ�䣬45*6*2*2ms = 1080ms
#endif

#define keyAgeMax   20          // ������ȥ�������ӳ�ʱ��, *2ms
// 2008-12-26������ȥ�������ӳ�ʱ���Ϊ50ms

#define maxRate     100         // ���������İ����ظ�ǰ���ӳ�ʱ��, *2ms
#define continueKeyFlag ((1 << KEY_UP) | (1 << KEY_DOWN))   // ��������������ӦλΪ1����������
//#define continueKeyFlag ((1 << KEY_UP) | (1 << KEY_DOWN) | (1 << KEY_QUICK))   // ��������������ӦλΪ1����������

#define ctrlKeyFlag     0           // �����ϵ�������ӦλΪ1�����ϵ���


#define minRatePause    100         //400; // ������ͣ
//LOCALF const Uint16 minRatePause = 250; //400; // ������ͣ
LOCALF const Uint16 minRate[5] = {50, 80, 130, 180, 200};// {80, 90, 120, 100, 110};
LOCALF const Uint16 repeatNumMax[5] = {9, 9, 9, 9, 9};
LOCALF Uint16 repeatNum;    // ������ʱ���ظ�����
LOCALF Uint16 repeatStep;
LOCALF Uint16 bOnShift;     // ����shift
Uint16 upDownDelta;         // ����UP/DOWN������

#if DEBUG_F_KEY_ANIMATE
Uint16 keyAnimateTime = 0;
Uint16 keyAnimateTime1 = 50;
Uint16 keyAnimateTimeAll = 500;
#endif

void UpdateMenuModeDisplayBuffer();


//=====================================================================
//
// ������ʾ���ݻ���
//
// 1. RUN, LOCAL/REMOTE�Ƶĸ���
// 2. �������ݻ���(��ʾ����,��λ,��˸������)
//    ��һ����ʾɨ�����(��6*2ms����һ��)���Ÿ�������
//
//=====================================================================
extern void Menu1OnPrg(void);
extern Uint16 checkMenuModePara;
extern Uint16 checkMenuModeCmd;
extern void DealCheckMenuModeGroupGrade(Uint16 flag);
void UpdateDisplayBuffer(void)
{
// RUN�Ƶĸ���
    displayBuffer[5] |= ~LED_CODE[LED_RUN]; // ��
    if (runFlag.bit.run)    // Ӧ���Դ�Ϊ׼���ڼ���ֱ���ƶ��ȴ�ʱ�䣬dspStatus.bit.run = 0
    {
        displayBuffer[5] &= LED_CODE[LED_RUN];  // ��
    }

// LOCAL/REMOTE��
    menuAttri[menuLevel].winkFlagLed &= ~(0x1U << LED_LOCAL);  // local����˸
    if (FUNCCODE_runSrc_DI == runSrc)
    {
        displayBuffer[5] &= LED_CODE[LED_LOCAL];  // ��
    }
    else if (FUNCCODE_runSrc_COMM == runSrc)
    {
        menuAttri[menuLevel].winkFlagLed |= (0x1U << LED_LOCAL); // local��˸
    }
    else //if (FUNCCODE_runSrc_PANEL == runSrc) // �����Զ�����
    {
        displayBuffer[5] |= ~LED_CODE[LED_LOCAL]; // ��
    }

// FWD/REV��
    if (runFlag.bit.dirReversing)  // ����ת�л�ʱ��˸
    {
        menuAttri[menuLevel].winkFlagLed |= (0x1U << LED_DIR); // tune��˸
    }
    else
    {
        displayBuffer[5] |= ~LED_CODE[LED_DIR]; // ��

        if (REVERSE_DIR == runFlag.bit.dir)
        { 
        displayBuffer[5] &= LED_CODE[LED_DIR];  // �� 
        }
    }
    
// TUNE/TC��
    menuAttri[menuLevel].winkFlagLed &= ~(0x1U << LED_TUNE);  // local����˸
    if ((tuneCmd)   // ��гʱһֱ��˸
#if DEBUG_F_ERROR_INDICATION
        || errorCode
#endif
        )
    {
        menuAttri[menuLevel].winkFlagLed |= (0x1U << LED_TUNE); // tune��˸
    }
    else if (runMode == RUN_MODE_TORQUE_CTRL)
    {
        displayBuffer[5] &= LED_CODE[LED_TUNE];  // ��
    }
    else
    {
        displayBuffer[5] |= ~LED_CODE[LED_TUNE]; // ��
    }

    DispDataDeal();         // ÿ�Ķ�ɨ��

    if (checkMenuModeCmd)   // ��check�˵�ģʽ����������
    {
        DealCheckMenuModeGroupGrade(checkMenuModePara);
    }

    //if (errorCodeOld != errorCode)
    if ((!errorCodeOld) 
        && (errorCode)     // ��0���˵�����������ʱ����0���˵���ʾ����
        && ((errAutoRstNum == 0) 
            || (errAutoRstNum == funcCode.code.errAutoRstNumMax) 
            || (funcCode.code.errAutoRstNumMax == 0))  // �����Զ���λ����Ϊ0��ǰ��λ����Ϊ0
        )
    {
        Menu1OnPrg();
    }

// һ��ɨ����ɣ��Ÿ������ݻ���
    if (!scanStep)      // Ŀǰ12ms����һ��
    {
// ��ʾ���ݣ���λ�ĸ���
        displayBuffer[5] |= ~LED_CODE[LED_HZ];  // Ĭ�ϵ�λ��ȫ��
        displayBuffer[5] |= ~LED_CODE[LED_A];
        displayBuffer[5] |= ~LED_CODE[LED_V];

        menuAttri[menuLevel].winkFlag = 0;    // �����Ĭ�ϲ���˸

        if (MENU_MODE_NONE == menuModeStatus)
        {
            menu[menuLevel].UpdateDisplayBuffer();
        }
        else    // ��ʾ�˵�ģʽ
        {
            UpdateMenuModeDisplayBuffer();
        }
    }
}


//=====================================================================
//
// LED������ܵ���˸����
//
// 1. ����winkFlag����������ܵ���˸
// 2. ����winkFlagLed������LED����˸
//
//=====================================================================
void WinkDeal(void)
{
    static Uint16 enflashOld;
    static Uint16 winkFlagOld;
    static Uint16 menuLevelOld;
    static Uint16 winkFlagLedOld;
    Uint16 winkFlag1 = menuAttri[menuLevel].winkFlag;
    Uint16 winkFlagLed1 = menuAttri[menuLevel].winkFlagLed;
    int16 i;

#if DSP_2803X
    if (keyChecking == 1)
    {
        return;
    }
#endif
    
// SPIҪ���͵����ݣ�����DO,��ʾ
#if DSP_2803X        // 2803x����2808ƽ̨
    spiData.out.a.bit.dispCode = displayBuffer[scanStep]; // ��ʾλ�����
#else
    spiData.out.a.bit.doData = (doHwStatus.all);   // ��8λ��DO����������̵���
    spiData.out.a.bit.dispCode = displayBuffer[scanStep]; // ��ʾλ�����
#endif
    

// ��˸����
    // �����˸��־(bEnflash,λ���ƼĴ���)�ı��ˣ����߲˵�����ı��ˣ���˸ʱ�����ʾ״̬Ҫ���³�ʼ��
    if ((winkFlagOld != winkFlag1)
        || (menuLevelOld != menuLevel)
        || (enflashOld != bEnflash)
        )
    {
        for (i = DISPLAY_8LED_NUM - 1; i >= 0; i--)
        {
            // ��˸��ʼΪ��1/4����ʱ��
            if ((bOnShift)       // ����shift
//                || (!(winkFlag1 & 0x01))
                )
            {
                bLight[i] = 0;
//                flashTicker[i] = flashTickerMax[i] >> 2;
                flashTicker[i] = flashTickerMax >> 2;
            }
            else
            {
                bLight[i] = !0;
//                flashTicker[i] = flashTickerMax[i] >> 2;
                flashTicker[i] = flashTickerMax >> 2;
            }
        }
    }
    
// ĳ���Ƶ���˸�ı��ˣ��õ�״̬�÷�
// ���õ�ȡ��������Ʋ���
    if (winkFlagLedOld != winkFlagLed1)
    {
        for (i = DISPLAY_LED_NUM - 1; i >= 0; i--)
        {
            if ((winkFlagLed1 ^ winkFlagLedOld) & (0x01U << i))
            {
                flashTickerLed[i] = 0;
                //displayBuffer[5] &= LED_CODE[i];  // ��
                displayBuffer[5] ^= ~LED_CODE[i];   // ȡ��
            }
        }
        
        winkFlagLedOld = winkFlagLed1;
    }

    winkFlagOld = winkFlag1;
    winkFlagLedOld = winkFlagLed1;
    menuLevelOld = menuLevel;
    enflashOld = bEnflash;

    if (scanStep < DISPLAY_8LED_NUM)    // ����ܵ���˸
    {
        if ((FLASH_YES == bEnflash) && (winkFlag1 & (0x01U << (7 - scanStep))))
        {
//            if (++flashTicker[scanStep] >= flashTickerMax[scanStep])
            if (++flashTicker[scanStep] >= flashTickerMax)
            {
                flashTicker[scanStep] = 0;
                bLight[scanStep] = !bLight[scanStep];
            }

            if (!bLight[scanStep])
            {
                spiData.out.a.bit.dispCode = DISPLAY_CODE[DISPLAY_NULL];    // ��
            }
        }
    }
    else                                // LED�Ƶ���˸
    {
        Uint16 flashMax;
        
        for (i = DISPLAY_LED_NUM - 1; i >= 0; i--)  // ����һ������Ҫ��˸���������ﲻ��Ҫ��������Ҫ�����Ƶ���˸ͬ����
        {
#if DEBUG_F_ERROR_INDICATION    // ������ʾ����˸
            flashMax = flashTickerLedMax;
            
            if (((i == LED_TUNE)
                && (errorCode))
#if 0   // ����Դ�Զ�����
                || ((i == LED_LOCAL)
                    && (FUNCCODE_runSrc_AUTO_RUN == runSrc))
#endif
                )
            {
                flashMax = flashMaxSDSD;
                
                if (++flashTickerLed[i] >= flashMax)
                {
                    flashTickerLed[i] = 0;
                    displayBuffer[5] ^= ~LED_CODE[i];   //
                }
            }
            else
#endif

            if (winkFlagLed1 & (0x01U << i))
            {
//                if (++flashTickerLed[i] >= flashTickerLedMax[i])
                if (++flashTickerLed[i] >= flashMax)
                {
                    flashTickerLed[i] = 0;
                    displayBuffer[5] ^= ~LED_CODE[i];   //
                }
            }
        }
    }

#if DEBUG_F_NEWMENU_USE_DOT_REVERSE_DIR // ���ұ�����ܵ�С���㣬��ʾ�趨Ƶ�ʷ������������ܵ�С���㣬��ʾ˲ʱƵ�ʷ���
    if (((0 == scanStep) && runFlag.bit.curDir)     // frq, ˲ʱƵ��
        || ((4 == scanStep) && runFlag.bit.dir))    // frqAim, �趨(Ŀ��)Ƶ��
    {
        spiData.out.a.bit.dispCode &= (DISPLAY_CODE[DISPLAY_DOT]);
    }
#endif
}



//==========================================================================================================
//
// ��ʾɨ��׼������ʵ��SPI�Ľ��գ��Լ�������ʾ����
//
//              XXXXX led
// scanStep:    01234 5
// scanStepΪ0��ɨ����ʾ����ߵ������
// scanStepΪ5��ɨ����ʾled��
//
// RCK�����ã�
// 0 -- input(74HC165):Parallel Load; output(74HC594):Storage register state is not changed
//      input:  DSP�����������(74HC165���ݴ��з������)��74HC165��DI���������̡���������
//              �����ݲ���load���ڲ���Q0-Q7���ȴ�/PL(RCK)���1�������ݴ��з��ͳ�ȥ��
//      output: DSP���͵��������(74HC594�ڲ���λ���)��74HC594��Storage register���ݲ��ٸı䡣
// 1 -- input(74HC165):Serial Shift; output(74HC594):Contents of Shift Register transferred to output latches
//      input:  DSP��ʼ����74HC165���з��͵����ݡ�3*8 bits
//      output: DSP��ʼ��74HC594�������ݣ�74HC594��ʼ��λ��
//              74HC594��Shift Register���͵�Storage register��������һ�δ��н��յ������ݲ��������Q0-Q7��
//
//==========================================================================================================
void DisplayScanPrepare(void)
{
    Uint16 RXFFST;
    int16 i;
    static int16 j;
    static Uint16 tcnt;
    
    RXFFST = SpiaRegs.SPIFFRX.bit.RXFFST;
// RCK���л���Ҫ��ʱһ��ʱ��(1usʵ��OK)�������ﵽSPI����֮��ĳ���Ҳ������ʱ���á�

#if DSP_2803X         // 2803x����2808ƽ̨
    RCK = 0;        // ������ɣ�RCK����
#else
    if (RXFFST >= SPI_LENGTH)    // Ӧ��ʹ���ж�
    {
        RCK = 0;        // ������ɣ�RCK����
    }
#endif

// �����ϴ�SPIͨѶ�����ݡ�
    for (i = 0; i < RXFFST; i++)    // ���յ��������ݣ���ȡ��������
    {
        spiData.in.all[j] = SpiaRegs.SPIRXBUF;
#if DSP_2803X         // 2803x����2808ƽ̨
            j = 0;
#else
        if (++j >= SPI_LENGTH)
            j = 0;
#endif
    }


#if DSP_2803X         // 2803x����2808ƽ̨
    if (!keySelStatus)
    {
         keyValue = spiData.in.key >> 7;      // ��������
    }
    // 2. ������ʾ���� 
    spiData.out.a.bit.dispSect = 0x7F & (~(1U << (5 - scanStep)));
#else
    diHwStatus.all = ((spiData.in.c.di & 0x00FF) << 8) + (spiData.in.c.di >> 8); // DI���룬˲ʱֵ
    keyValue = spiData.in.c.key >> 7;      // ��������
    // 2. ������ʾ����
    spiData.out.b.bit.dispSect = 0x7F & (~(1U << (5 - scanStep)));
#endif
  // ���룬��ʾ��i�������, JPXY = 1, FAN = 0;


// ����/���ü����ж�
#if DSP_2803X         // 2803x����2808ƽ̨
    keySelTypeBak = keySelType;
    if (!tcnt)
    {
        keySelType = 1;
        keySelStatus = 1;
    }
    else
    {
        if (keySelStatus)
        {
            (!(spiData.in.key >> 7)) ? (keySelType = 0) : (keySelType = 1);
            keySelStatus = 0;
        }
    }
	(tcnt >= KEY_SEL_TEST_TIME) ? (tcnt = 0) : (tcnt++);

    // ��ǰΪ���ü���ʱ�л��ж��Ƿ�����������
    // keyCheckingΪ1ʱ,������ʾ״̬ά�ֲ���(��ֹ���ü�����ʾ��˸)
    if ((keySelType == 1) && (keySelTypeBak == 0))
    {
        keyChecking = 1;
    }
    else
    {
        keyChecking = 0;
    }
    
#endif

// 3. ����scanStep
#if DSP_2803X
	if (!keyChecking)
#endif
	{
	    if (++scanStep >= DISPLAY_8LED_NUM + 1) // һ����_������ܣ��Լ�led��
	    {
	        scanStep = 0;
	    }
	}
}



//=====================================================================
//
// ������ʾɨ�����ݣ���SPI����
//
//=====================================================================
void DisplayScan(void)
{
// SPI����, SPI���ַ�����������Ϊ16��ע��SPI�ǴӸ�λ��ʼ����
    RCK = 1;

#if DSP_2803X               // 2803x����2808ƽ̨
    KEYSEL = keySelType;    // ����ѡ��(0-����  1-����)
    SpiaRegs.SPITXBUF = ~spiData.out.a.all; //stamp MD380

#else
    SpiaRegs.SPITXBUF = spiData.out.b.all;   // SPI�ǴӸ�λ��ʼ���ͣ���λΪ��Ч����
    SpiaRegs.SPITXBUF = spiData.out.a.all;
#endif    
}



//=====================================================================
//
// ���ݽ��յ��ļ������ݣ��ж��Ƿ��а������£��ĸ�/��Щ����������
// ���ж��Ƿ���Ҫ������Ӧ����
//
// ���룺keyVal -- ����ֵ
// �����bKeyProc   -- 0������Ҫ������Ӧ������1����Ҫ������Ӧ����
//       keyPreFunc -- ���µĹ��ܼ���ֵ
//       keyPreCtrl -- ���µ��ϵ�����ֵ
//
//=====================================================================
void KeyScan(void)
{
    keyRet keyTemp = {0, 0, 0, 0};
    int16 j = 0;
    Uint16 keyValueTmp;
//     static Uint16 repeatNum = 0;
//     static Uint16 repeatStep = 0;
#if DEBUG_F_KEY_ANIMATE
    keyValue = (~((0x1U << (KEY_UP - 1)))) & ((1 << KEY_NUM) - 1);
#endif

    keyValueTmp = ((~keyValue) & ((1 << KEY_NUM) - 1)) << 1;
    if (keyValueTmp)     // �м�����
    {
        for (j = KEY_NUM; j >= 1; j--)
        {
            if ((keyValueTmp & (0x01U << j)))     // ��jλ
            {
                if (((keyValueTmp) & ctrlKeyFlag) && (ctrlKeyFlag == (0x01 << j)))  // �ü����ϵ���
                {
                    keyTemp.ctrlCnt++;      // �ϵ���������1
                    keyTemp.ctrlVal = j;    // ����ֵ�����ϵ�����ֵ����
                }
                else                        // �ü��ǹ��ܼ�
                {
                    keyTemp.funcCnt++;      // ���ܼ�������1
                    keyTemp.funcVal = j;    // ����ֵ���빦�ܼ���ֵ����
                }
            }
        }
    }

    if ((!keyTemp.funcCnt) || (keyTemp.ctrlCnt > 1)) // �޹��ܼ�����,����,ͬʱ���µ��ϵ�����������1��
    {
        keyPreFunc = 0;
        keyPreCtrl = 0;             // �ϴι��ܼ����ϴ��ϵ�����������
        keyFunc = 0;                // ������Ҫ����㶯����
        keyAge = 0;                 // �������������
        keyStatus = KEY_STATUS_JITTER_DEAL;   // ����״̬
        bKeyEsc = 0;                // �����ͷű�־
        bEnflash = FLASH_YES;       // ������˸

        accDecFrqPrcFlag = ACC_DEC_FRQ_NONE;
        
        upDownDelta = 1;
    }
    else if (!bKeyEsc)                  // �����Ѿ��ͷ�
    {
        if (keyTemp.funcCnt > 1)        // �ж���һ�����ܼ����£���λ�����ͷű�־
            bKeyEsc = 1;
        else if ((keyTemp.funcVal != keyPreFunc)
                 || ((keyTemp.ctrlVal != keyPreCtrl) && keyPreCtrl)
                 )
                // ���ι��ܼ�ֵ���ϴβ�ͬ
                // ���ߣ������ϵ���ֵ���ϴβ�ͬ���ϴ����ϵ���
        {
            keyPreFunc = keyTemp.funcVal;   // �ñ��μ�ֵ�����ϴμ�ֵ
            keyPreCtrl = keyTemp.ctrlVal;
            keyAge = 0;                     // ������������
        }
        else if ((!keyPreCtrl) && keyTemp.ctrlVal)// 2�ι��ܼ���ͬ���ϴ����ϵ���������
        {
            bKeyEsc = 1;    // ����: �Ȱ��¹��ܼ����ٰ��ϵ������ְ������
        }
        else
        {
            keyAge++;                           // 2�ι��ܼ����ϵ�������ͬ
            switch (keyStatus)
            {
                case KEY_STATUS_JITTER_DEAL:    // ȥ���׶�
                    if (keyAge >= keyAgeMax)    // ��ɼ���ȥ������
                    {
                        bKeyProc = 1;           // ���м�ֵ����
                        if ((((continueKeyFlag) & (0x01U << keyTemp.funcVal)) == 0)  // ���ߣ��ù��ܼ�����������
                         // || (keyTemp.ctrlCnt == 1)  // ��һ�Ը��ϼ���
                            )
                            bKeyEsc = 1;
                        else
                        {
                            keyStatus = KEY_STATUS_WAIT_REPEAT;   // ����״̬�����ظ�ǰ���ӻ��׶�
                            keyAge = 0;         // ������������
                        }
                    }
                    break;

                case KEY_STATUS_WAIT_REPEAT:    // �ظ�ǰ���ӻ��׶�
                    if (keyAge >= maxRate)      // ��������ﵽ�����ظ���ʱ��
                    {
                        bKeyProc = 1;           // ���м�ֵ����
                        keyAge = 0;             // ������������
                        keyStatus = KEY_STATUS_REPEAT;   // ����״̬�����ظ��׶�
                        repeatNum = 0;
                        repeatStep = 0;
                    }
                    break;

                case KEY_STATUS_REPEAT:         // �ظ��׶�
                    if (keyAge >= minRate[repeatStep])      // �������䵽
                    {
                        keyAge = 0;             // ������������
                        bKeyProc = 1;           // ���м�ֵ����
                        repeatNum++;

                        if (bFrqDigital)
                        {
                            // �����������ﵽrepeatNumMax[]֮����ͣһ��ʱ��
                            if (repeatNum >= repeatNumMax[repeatStep]) 
                            {
                                if (repeatStep < 2)
                                {
                                    bKeyProc = 0;           // �����м�ֵ����
                                    keyStatus = KEY_STATUS_WAIT_CARRY;   // ����״̬����������λ����ͣ�׶�
                                }
                            }
                        }
                    }
                    break;

                case KEY_STATUS_WAIT_CARRY:     // ������λ����ͣ�׶�
                    if (keyAge >= minRatePause) // �������䵽
                    {
                        keyAge = 0;             // ������������
                        bKeyProc = 1;           // ���м�ֵ����
                        keyStatus = KEY_STATUS_REPEAT;   // ����״̬�����ظ��׶�
                        upDownDelta *= 10;
                        repeatStep++;
                        repeatNum = 0;
                    }
                    break;

                default:
                    break;
            }
        }
    }

#if DEBUG_F_KEY_ANIMATE
    {
        static Uint16 i;
        if (++i >= keyAnimateTime)     // __*2ms
        {
            bKeyProc = 1;
            keyPreFunc = KEY_UP;
            i = 0;
        }
    }
#endif
}


//=====================================================================
//
// ���̲���
// ��ͨѶ��ָ��͹���ʱ����ʼ����ÿ�������Ƿ���Ѱ���
//
//=====================================================================
void KeyBordTest(void)
{
    // ��֤�Ƿ������еİ���
    if (keyBordTestFlag)
    {
        // ÿ���а���ʱ����¼�ð����Ѱ��±�־
        if (keyPreFunc)
        {
            keyBordValue |= (1<<(keyPreFunc - 1));
        }
    }
    else
    {
        keyBordValue = 0;
    }
}

//=====================================================================
//
// ���̴�����
// ������һ�����������£�������Ӧ����
// ���룺bKeyProc   -- �Ƿ��а�����Ҫ����1���У�0���ޡ����У�����֮��Ҫ����
//       keyPreFunc -- ����
// �������
//
//=====================================================================
void KeyProcess(void)
{
    KeyBordTest();
    
    if ((KEY_REV_JOG != keyFunc) && (KEY_FWD_JOG != keyFunc))
    {
        keyFunc = 0;        // ÿ2ms�����㡣2ms��û�д����������µİ���������������
    }

    if (!bKeyProc)          // ���û�а�����Ҫ����
    {

        // �˵�ģʽ�л�QUICK������
        if (MENU_MODE_ON_QUICK == menuModeStatus)
        {
            if (++onQuickTicker >= onQuickTickerMax)
            {
                onQuickTicker = 0;

                menuModeStatus = MENU_MODE_NONE;
                menuModeTmp = menuMode;  // �ָ�menuMode
            }
        }

#if DEBUG_RANDOM_FACPASS  
        // ����������ʾ
        if (facPassViewStatus == FAC_PASS_VIEW)
        {
            if (++onShiftPassViewTicker >= onShiftPassViewTickerMax)
            {
                onShiftPassViewTicker = 0;
                facPassViewStatus = FAC_PASS_NONE;
            }
        }
#endif
        
        return;
    }
    
    bKeyProc = 0;
    bOnShift = 0;           // ���µķ�shift����ʱ�ñ�־������
    keyFunc = keyPreFunc;

    if ((MENU_MODE_ON_QUICK == menuModeStatus) &&
        (KEY_QUICK != keyPreFunc) && 
        ((KEY_ENTER != keyPreFunc) 
//        || ((KEY_ENTER == keyPreFunc) && (MENU_LEVEL_0 != menuLevel))
        )
        )
    {
        onQuickTicker = 0;

        menuModeStatus = MENU_MODE_NONE;
        menuModeTmp = menuMode;  // �ָ�menuMode

        return;
    }

    switch (keyPreFunc)
    {
        case KEY_PRG:                           // ����PRG��
            menu[menuLevel].onPrgFunc();
            break;
            
        case KEY_UP:                            // ����UP��
            bEnflash = FLASH_NO;
            menu[menuLevel].onUpFunc();
            break;
            
        case KEY_ENTER:                         // ����ENTER��
            menu[menuLevel].onEnterFunc();
            break;
            
        case KEY_MFK:                           // ����MF.K��
            menu[menuLevel].onMfkFunc();
            break;
            
        case KEY_DOWN:                          // ����DOWN��
            bEnflash = FLASH_NO;
            menu[menuLevel].onDownFunc();
            break;
            
        case KEY_SHIFT:                         // ����SHIFT��
            bOnShift = 1;
            menu[menuLevel].onShiftFunc();
            break;
            
        case KEY_RUN:                           // ����RUN��
            menu[menuLevel].onRunFunc();
            break;
            
        case KEY_STOP:                          // ����STOP��
            menu[menuLevel].onStopFunc();
            break;
            
        case KEY_QUICK:                         // ����QUICK��
            onQuickTicker = 0;
            menu[menuLevel].onQuickFunc();
            break;

        default:
            break;
    }
}




void InitSpiaGpio(void)
{
   EALLOW;
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled by the user.
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;   // Enable pull-up on GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;   // Enable pull-up on GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;   // Enable pull-up on GPIO18 (SPICLKA)
//    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;   // Enable pull-up on GPIO19 (SPISTEA)


/* Set qualification for selected pins to asynch only */
// This will select asynch (no qualification) for the selected pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 (SPICLKA)
//    GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; // Asynch input GPIO19 (SPISTEA)

/* Configure SPI-A pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be SPI functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1; // Configure GPIO16 as SPISIMOA
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1; // Configure GPIO17 as SPISOMIA
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1; // Configure GPIO18 as SPICLKA
//    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1; // Configure GPIO19 as SPISTEA

#if DSP_2803X         // 2803x����2808ƽ̨
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;    // Configure GPIO22, RCK
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;     // output
    //GpioDataRegs.GPADAT.bit.GPIO22 = 1;     // RCK = 1
    GpioDataRegs.GPADAT.bit.GPIO22 = 0;     // RCK = 0

    // KEYSEL
    GpioCtrlRegs.AIOMUX1.bit.AIO4 = 0;     // Dig.IO funct. applies to AIO2,4,6,10,12,14
    GpioCtrlRegs.AIODIR.bit.AIO4 = 1;      // AIO2,4,6,19,12,14 are digital inputs
    GpioDataRegs.AIODAT.bit.AIO4 = 1; 

#else
// 2808 DSP��壬ʹ��GPIO19��Ϊ���̵�ѡ���
// �����ŷ�
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;    // Configure GPIO19, RCK
    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;     // output
    //GpioDataRegs.GPADAT.bit.GPIO19 = 1;     // RCK = 1
    GpioDataRegs.GPADAT.bit.GPIO19 = 0;     // RCK = 0
#endif    
    EDIS;
}



//=====================================================================
//
// SPIA��ʼ������ʾ��DI��DOʹ��
//
//=====================================================================
void InitSetSpiForFunc(void)
{
// Initialize SPI FIFO registers
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x405F;  // Receive FIFO reset
    SpiaRegs.SPIFFRX.all = 0x205F;  // Re-enable transmit FIFO operation
    SpiaRegs.SPIFFCT.all = 0x0000;

// Initialize  SPI
    SpiaRegs.SPICCR.all = 0x000F;   // Reset on, rising edge, 16-bit char bits
    SpiaRegs.SPICTL.all = 0x000E;   // Enable master mode, SPICLK signal delayed by one half-cycle
                                    // enable talk, and SPI int disabled.
                                    // 74HC594: SCK���ʱ����ʼ��λ��Ҫ����λ֮ǰ�����ݴ��͵�74HC594��
                                    // ���ԣ�CLOCK PHASE = 1

// SPI������Ϊ195000bpsʱ������3��byte��ʱ���ԼΪ0.13ms��
// 320��280F����ʹ��IO��ģ��SPI��
// 320����ʾû��ʹ��SPI������ʹ����SPI��������ԼΪ118KHz
// 280F�Ĳ�����ԼΪ143KHz
#if (DSP_CLOCK == 100)              // DSP����Ƶ��100MHz
    SpiaRegs.SPIBRR = 0x007F;       // 100/4 * 10^6 / (127+1) = 195312.5
#elif (DSP_CLOCK == 60)             // DSP����Ƶ��60MHz
    SpiaRegs.SPIBRR = 0x004C;       // 60/4 * 10^6 / (76+1) = 194805.2
#endif

    SpiaRegs.SPICCR.bit.SPISWRESET = 1; // Relinquish SPI from Reset
    SpiaRegs.SPIPRI.bit.FREE = 1;       // Set so breakpoints don't disturb xmission

// �������õ����ݣ���ʼ��������Ҫ��
}





void UpdateMenuModeDisplayBuffer()
{
    //displayBuffer[0] = DISPLAY_CODE[DISPLAY_q] & DISPLAY_CODE[DISPLAY_DOT];
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_LINE];
    displayBuffer[1] = DISPLAY_CODE[dispCodeMenuMode[menuModeTmp-1][0]];
    displayBuffer[2] = DISPLAY_CODE[dispCodeMenuMode[menuModeTmp-1][1]];
    displayBuffer[3] = DISPLAY_CODE[dispCodeMenuMode[menuModeTmp-1][2]];
    displayBuffer[4] = DISPLAY_CODE[dispCodeMenuMode[menuModeTmp-1][3]];
}





