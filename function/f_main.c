//======================================================================
//
// �������ļ�������:
// 1���ϵ�ʱ���ܵĳ�ʼ������ҪΪ������Ķ�ȡ
// 2��2ms,0.5ms,0ms�Ĺ��ܺ���
// 3�����ϴ���
// 4�����ܡ����ܵĽ���
// 5����������ĸ���
//
// Time-stamp: <2012-03-14 14:08:23  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_eeprom.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_osc.h"
#include "f_canlink.h"
#include "f_p2p.h"


#define PRODUCT_VERSION      38000       // ��Ʒ�汾��
#define SOFTWARE_VERSION     73    // ����汾0.73


#if F_DEBUG_RAM

#define DEBUG_F_FC_INIT_OVER        0   // �������ȡok����

#define DEBUG_F_CHK_FUNC_CODE       0   // ��鹦�����Ƿ��д���ͬʱ�Զ���ñ��eeprom2Fc[], anybus2Fc[]

#elif 1

#define DEBUG_F_FC_INIT_OVER        1

//----����Ϊ����---------
#define DEBUG_F_CHK_FUNC_CODE       0  // ��鹦�����Ƿ��д���ͬʱ�Զ���ñ��eeprom2Fc[], anybus2Fc[]

#endif



// AIAO����У���Ƿ����
#define AIAO_CHK            (0x5A5A)



Uint16 memoryValue;
void getMomeryValue(void);
#define MEMORY_ADDRESS funcCode.code.memoryAddr  // �鿴�ڴ��ַ



#if 0
Uint16 dataDigit;  // Ĭ��Ϊ��5λ
Uint16 kk;
Uint16 jj;
Uint16 upper1;
Uint16 lower1;
Uint16 index;
#endif


#define START_DSP_DELAY_TIME_MS     300     // ������ʱʱ�䣬ms

#define WAIT_POWER_ON_TIME          10000       // ����������֮�󣬵ȴ�_*0.5ms��ʼ��ӦǷѹ�������������
enum POWER_ON_STATUS powerOnStatus;             // �ϵ�ʱ��״̬

enum FUNC_CODE_INIT_STATUS
{
    FUNCCODE_INIT_STATUS_VERIFY_EEPROM,
    FUNCCODE_INIT_STATUS_NEW_EEPROM,
    FUNCCODE_INIT_STATUS_OLD_EEPROM,
    FUNCCODE_INIT_STATUS_OVER
};
LOCALF enum FUNC_CODE_INIT_STATUS funcCodeInitStatus;  // ������ĳ�ʼ������


Uint16 driveCoeff[2][4];

extern Uint32 allTime;
extern Uint32 upTimes;

//-------------------------------------------
union MOTOR_PARA motorPara;
struct MOTOR_FC motorFc;            // ��1/2/3/4��������룬����������������Ʋ���
enum MOTOR_SN motorSn;              // ���ѡ��Ŀǰ��1/2/3/4���
extern void UpdateMotorPara(void);
//-------------------------------------------

Uint16 aiaoChckReadOK;          // ��ȡ��AIAOУ������ֵ����

Uint16 mainLoopTicker;
// �����жϹ��ܲ����Ƿ��Ѿ����յ����ܴ��ݵ�AI����ֵ��
// 0-δ������ѭ�������ߣ����ܲ��ֽ�����ѭ���ĵ�1�Ļ�û����ɡ�
// 1-���ܲ��ֽ�����ѭ���ĵ�1���Ѿ���ɡ�
// 2-���ܲ��֣��Ѿ����յ����ܴ��ݵ�AI����ֵ��֮�󣬿�������ʹ��AI��
// 
// ����:
//
// ��Ƶ��ԴΪAI������ԴΪ��������ʽ1������Ƶ�ʷ��㡣������������ϵ�ǰ����Ч��
// ���ϵ�����������������������������Ч����Ч������������
//
// ԭ��:
//
// ������ѭ��֮�����ܲŴ���AI�Ĳ���ֵ��ֱ�����ܵĵ�2�ĲŴ��ݡ�
// ������ѭ���ĵ�1�ģ���������û�д���AI����ֵ�������趨Ƶ��Ϊ0����������Ƶ�ʡ�
// ����������Ƶ��/����Ƶ��ʱ�����ڰ�ȫ���ǣ����������������Ͳ���������
//
// ����:
//
// �ڹ��ܽ��յ����ܴ��ݵ�AIʱ(������ѭ���ĵ�2��)��AI���˲�(��AI��ֱֵ�Ӹ�ֵ)��
// �����ɱ����ܵ�AI�˲�ʱ���Ӱ�죻AI�Ĳ���ֵҲ����������Ϊ�趨ֵ��
// ��ʾ��Ҳû�н�����̡�
// 


extern void InitSetFvcIndexIntEQep(void);

LOCALD void FuncVariableInit(void);


LOCALD void InitFuncCode(void);
void InitDspFunc(void);


// ����U0����ʾ����
extern void UpdateU0Data(void);

extern void SoftVersionDeal(void);


extern void UpdateDataCore2Func(void);
extern void UpdateDataFunc2Core0p5ms(void);
extern void UpdateDataFunc2Core2ms(void);
extern void TemperatureDeal(void);


Uint32 timeBase;
Uint32 timeBase05msOld;
Uint32 ticker05msFuncInternal;  // ����0.5ms����֮���ʱ��
Uint32 ticker05msFuncIn[4];
Uint16 ticker05msFuncInternalDisp[4];

Uint32 ticker05msFuncEach;      // ÿ��0.5ms���ú�����ִ��ʱ��
Uint32 ticker05msFuncEa[4];
Uint16 ticker05msFuncEachDisp[4];
void CodeRunMonitor(void);

void Main05msFunctionEachStart(void);
void Main05msFunctionA(void);
void Main05msFunctionB(void);
void Main05msFunctionC(void);
void Main05msFunctionD(void);
void Main05msFunctionEachEnd(void);



//=====================================================================
//
// ��Ƶ�����ܲ��ֳ�ʼ����������
// �����漰����Ƭ������
// �������ʼ��
// ������ʼ��
//
//=====================================================================
void InitForFunctionApp(void)
{
#if DEBUG_F_CHK_FUNC_CODE
    extern void ValidateFuncCode(void);

// �жϹ������ ���������Ը������Ƿ�һ�£�
// �жϹ������EEPROM��ַ��anybus��ַ�Ƿ��д���
// �Զ���ȡ eeprom2Fc[] , anybus2Fc[] ����.
// ������ʱʹ��
    ValidateFuncCode();
#endif

// ��ʼ�������漰����Ƭ������
    InitDspFunc();

// ��ʼ��������
    InitFuncCode();

// ĳЩ��ʼ��Ӧ���ڹ�����������ȡ֮�����

// ������ʼ��
    FuncVariableInit();
}


//=====================================================================
//
// ��ʼ�������漰����Ƭ������
//
//=====================================================================
extern void I2cDealBeforeInit(void);
void InitDspFunc(void)
{
    EALLOW;
#if DSP_2803X       // 2803x����2808ƽ̨
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;    // eCAN-A
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;    // ePWM4
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;    // ePWM5
    SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 1;    // ePWM5
    SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1;    // eCAP1
    SysCtrlRegs.PCLKCR0.bit.LINAENCLK = 1;     // LIN-A
#else
    SysCtrlRegs.PCLKCR1.bit.ECAP3ENCLK = 1;     // eCAP3
    SysCtrlRegs.PCLKCR1.bit.ECAP4ENCLK = 1;     // eCAP4
    SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;     // eQEP2
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;     // eCAN-A
#endif

   
    EDIS;
    
// SPI����ʾ�����̡�DIDO
    InitSpiaGpio();
    InitSetSpiForFunc();

// EEPROM��24LC08/24LC32
    I2cDealBeforeInit();    // ���ڲ���EEPROMʱDSP��λ��I2C��Ҫ����
    InitI2CGpio();
    InitSetI2ca();  

// SCI
#if 1
    InitSciaGpio();
    InitSetScia();
#elif 1
    InitScibGpioDp();
    InitSetSciDp(&sciM380DpData);
#endif

// AO1
#if DSP_2803X
    InitEPwm6Gpio(); 
    InitSetEPWM6();  
#else
    InitECap3Gpio();
    InitSetEcap3();
#endif


#if DSP_2803X
// DI/DO
    InitDIGpio();
    InitDOGpio();
#endif

// HDI
#if DSP_2803X
    InitECap1Gpio();
    InitSetEcap1();
#else
    InitECap4Gpio();
    InitSetEcap4();
#endif

// PULSE OUT, AO2��DO3(FM)
#if DSP_2803X
    InitEPwm4Gpio(); // AO2
    InitSetEPWM4();  // AO2
    InitEPwm5Gpio(); // HDO1/FM
    InitSetEPWM5();  // HDO1/FM
#else
    InitECap2Gpio();
    InitSetEcap2();
#endif
    
// CAN

}

//=====================================================================
//
// �������ʼ������AI��DI��PulseIn��ʼ����
//
// 1���տ�ʼ��EEPROM��д���ڽ��С�
// 2�����������(F2��)������Ķ�дû�����ʱ������DI��AI��PulseIn�ļ�����Ч��
// 3����������ӹ�����Ķ�д���ʱ��DI��AI��PulseIn�ļ������Ч��
// 4�����ڹ�����Ķ�д�Ǵ�0��ʼ�����ԣ���������ӹ�����Ķ�д��ɣ���
//    ȫ����������ɵ����ʱ�䣬�㹻DI��AI��PulseIn�ĸ��¡�
//
//=====================================================================
LOCALF void InitFuncCode(void)
{
    Uint32 baseTime;
    Uint16 readAIAOChck = AIAO_CHK_CHECK;
	Uint16 fcIndex;
    Uint16 readIndex;
    aiaoChckReadOK = AIAO_CHK_READ_NULL;
//    Uint16 initTicker = 0;

    baseTime = GetTime();    
    while ((!errorEeprom)   // ����EEPROM���ϣ������˳���ȡ������ѭ������ʾerr
           && ((FUNCCODE_INIT_STATUS_OVER != funcCodeInitStatus)
//               || (initTicker < START_DSP_DELAY_TIME_MS / 2)
               )
           )
    {
        switch (funcCodeInitStatus)
        {
            case FUNCCODE_INIT_STATUS_VERIFY_EEPROM: // �ж�EEPROM�Ƿ��һ���ϵ�ʹ�ã�EEPROMУ����1 �� EEPROMУ����2
                funcCodeRead.index = GetEepromIndexFromFcIndex(EEPROM_CHECK_INDEX1);
                funcCodeRead.number = 2;
                EepromDeal();   // ����ط�������I2C����״̬����
                if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                {
                    if ((EEPROM_CHECK1 == funcCodeRead.data[0])
                        && (EEPROM_CHECK2 == funcCodeRead.data[1]))
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_OLD_EEPROM; // EEPROM�Ѿ�ʹ��
                    }
                    else
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_NEW_EEPROM; // EEPROM��һ��ʹ��
                        readAIAOChck = AIAO_CHK_CHECK;
                    }
                }
                break;

            case FUNCCODE_INIT_STATUS_NEW_EEPROM:
                // ��ȡAIAO����У��ֵ
                if (readAIAOChck == AIAO_CHK_CHECK)
                {
                    // ��ȡAIAOУ����ֵ
                    readIndex = GetEepromIndexFromFcIndex(AI_AO_CHK_FLAG);
                    funcCodeRead.index = readIndex;
                    funcCodeRead.number = 1;
                    EepromDeal();
                    if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                    {
                        // Ϊ0XA5��ʾ�ѽ��й�װУ��
                        if (funcCodeRead.data[0] == AIAO_CHK_WORD)
                        {
                            // ��ȡ��װУ��ֵ
                            readAIAOChck = AIAO_CHK_START;
                            readIndex = GetEepromIndexFromFcIndex(AI_AO_CALIB_START);
                        }
                        else
                        {
                            // ֱ�ӿ�ʼ��ʼ��EEPROM
                            readAIAOChck = AIAO_CHK_END;
                        }
                    }
                }
                else if (readAIAOChck == AIAO_CHK_START)
                {
                    static Uint16 flag = 0;
                    funcCodeRead.index = readIndex;
                    funcCodeRead.number = 2;
                    EepromDeal();
                    if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                    {
						fcIndex = GetFcIndexFromEepromIndex(readIndex);
                        funcCode.all[fcIndex] = funcCodeRead.data[0];
                        funcCode.all[fcIndex+1] = funcCodeRead.data[1];
                        
                        readIndex += 2;
                        flag++;

                        // AI3����ҪУ��
                        if (flag == 4)
                        {
                            readIndex += 4;
                        }

                        // У����AO1ʱ����
                        if (readIndex > GetEepromIndexFromFcIndex(AI_AO_CALIB_STOP))
                        {
                            aiaoChckReadOK = AIAO_CHK_READ_OK;  // У��ֵ��ȷ
                            readAIAOChck = AIAO_CHK_END;        // ������ȡ
                        }
                    }
                }
                else //if (readAIAOChck == AIAO_CHK_END)
                {
                    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_ALL;
                    EepromDeal();
                    if (!funcCodeRwModeTmp)  // ȫ��������͵��籣���������ֵд�뵽EEPROM��
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_OVER;

                        // �洢EEPROMУ�顢AIAOУ��
                        funcCode.code.eepromCheckWord1 = EEPROM_CHECK1;
                        funcCode.code.eepromCheckWord2 = EEPROM_CHECK2;
                        funcCode.code.aiaoChkWord = FUNCCODE_READ_RET_OK;
                        SaveOneFuncCode(EEPROM_CHECK_INDEX1);
                        SaveOneFuncCode(EEPROM_CHECK_INDEX2);
                        SaveOneFuncCode(AI_AO_CHK_FLAG);
                    }
                }
                break;

            case FUNCCODE_INIT_STATUS_OLD_EEPROM:
                funcCodeRwModeTmp = FUNCCODE_RW_MODE_READ_ALL;
                EepromDeal();
                if (!funcCodeRwModeTmp)  // ȫ��������͵��籣������Ѿ�����RAM�С�
                {
                    funcCodeInitStatus = FUNCCODE_INIT_STATUS_OVER;
                }
                break;

            default:
                break;
        } // switch (funcCodeInitStatus)

        PulseInSample();

        if (baseTime - GetTime() >= (TIME_1MS << 1)) // 2ms
        {
            spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff); // ��ʾ��ʼ����ֵ, -H-C-
            DisplayScanPrepare();       // ��ʾɨ��׼��

//            initTicker++;
            baseTime = GetTime();
            //baseTime -= (TIME_1MS << 1);

            EepromOperateTimeDeal();
            
            UpdateDataCore2Func();      // ��Ҫ���ܴ���AI�Ĳ���ֵ
            DisplayScan();              // ��ʾɨ��
        }
    }

// ����汾����
    SoftVersionDeal();

#if DEBUG_F_FC_INIT_OVER
// �ж�EEPROM�е�ֵ�Ƿ���������֮��
    if (FUNCCODE_INIT_STATUS_OVER == funcCodeInitStatus) // ��ȡ���������
    {
        int16 i;
        Uint16 upper, lower;
        Uint16 tmp;
        int16 j;        // limitedByOtherCodeIndex[]���±�
		const FUNCCODE_ATTRIBUTE *maxFrqAttribute = &funcCodeAttribute[MAX_FRQ_INDEX];

        // ���� limitedByOtherCodeIndex ��
        j = 0;          // limitedByOtherCodeIndex[]���±�
        for (i = FNUM_PARA - 1; i >= 0; i--)    // û�а������籣��͹��ϼ�¼
        {
            const FUNCCODE_ATTRIBUTE *pAttribute = &funcCodeAttribute[i];

            if (pAttribute->attribute.bit.upperLimit)
            {
                limitedByOtherCodeIndex[j++] = i;

                upper = funcCode.all[pAttribute->upper];
            }
            else
            {
                upper = pAttribute->upper;
            }

            if (pAttribute->attribute.bit.lowerLimit)
            {
                // ���������������������ƣ�limitedByOtherCodeIndexǰ���Ѿ�����
                if (!pAttribute->attribute.bit.upperLimit)
                {
                    limitedByOtherCodeIndex[j++] = i;
                }

                lower = funcCode.all[pAttribute->lower];
            }
            else
            {
                lower = pAttribute->lower;
            }

            tmp = LimitDeal(pAttribute->attribute.bit.signal, funcCode.all[i], upper, lower, 0);
            if (funcCode.all[i] != tmp)
            {
                funcCode.all[i] = tmp;
                SaveOneFuncCode(i);
            }

            // ��ȡ���������֮�󣬸���IO�ڵ�����״̬
            DiCalc();
            PulseInCalc();
            AiCalc();
        }
        limitedByOtherCodeIndexNum = j; // �������������������������ƵĹ���������

		// ���Ƶ�ʵ�����ֵ���⴦��
        upper = maxFrqAttribute->upper;
        lower = 50 * decNumber[funcCode.code.frqPoint];
        tmp = LimitDeal(maxFrqAttribute->attribute.bit.signal, funcCode.all[MAX_FRQ_INDEX], upper, lower, 0);
        if (funcCode.all[MAX_FRQ_INDEX] != tmp)
        {
            funcCode.all[MAX_FRQ_INDEX] = tmp;
            SaveOneFuncCode(MAX_FRQ_INDEX);
        }

    }
    else
#endif
        if (errorEeprom)   // EEPROM����
    {
        errorCode = ERROR_EEPROM;
        UpdateErrorDisplayBuffer();

        for (;;)    // �ϵ�ʱEEPROM��д����һֱ��ʾerr����������ѭ��
        {
            if (baseTime - GetTime() >= (TIME_1MS << 1))
            {
                spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff);
                DisplayScanPrepare();

                baseTime = GetTime();
                PulseInCalc();  // ������ʱ����
                AiCalc();       // ��ʱ

                DisplayScan();
            }
        }
    }
}


//=====================================================================
//
// ���ܲ��ֵı�����ʼ��
//
//=====================================================================
LOCALF void FuncVariableInit(void)
{
#if DEBUG_F_MOTOR_FUNCCODE
    int16 i;
#endif

    funcCode.code.inverterGpTypeDisp = funcCode.code.inverterGpType; // F0-00, GP������ʾ

    UpdateInvType();    // ���ʵ�ʻ���(��Ƶ������)����Ƶ�����ѹ

    frqCalcSrcOld = funcCode.code.frqCalcSrc;


//=====================================================================
// ���籣������Ļָ�

// �����趨Ƶ��UP/DOWN�������
    // X����YΪ�������
    if ((funcCode.code.frqXSrc == FUNCCODE_frqXySrc_FC_P_OFF_REM) ||
        (funcCode.code.frqYSrc == FUNCCODE_frqXySrc_FC_P_OFF_REM))
    {
        upDownFrqInit = (int16)funcCode.code.upDownFrqRem;
    }
    else
    {
        upDownFrqInit = 0;
    }

// ͬ����ת��λ��
    pmsmRotorPos = funcCode.code.pmsmRotorPos;

    frqAimTmp0 = funcCode.code.presetFrq; // Ŀǰ����Ҫ��ʼ������ΪFrqSetAimSrcDeal()����presetFrqOld�����
//    presetFrqOld = funcCode.code.presetFrq;

// PLC�������
    if (FUNCCODE_plcPowerOffRemMode_REM == (funcCode.code.plcPowerOffRemMode%10)) // PLC�������
    {
        plcStep = funcCode.code.plcStepRem;
        plcStepRemOld = plcStep;

        plcTime = ((Uint32)funcCode.code.plcTimeHighRem << 16) + funcCode.code.plcTimeLowRem;
        plcTimeRemOld = plcTime;
    }

    //+= funcCode.code.extendType = ;
//=====================================================================


#if DEBUG_F_POSITION_CTRL
    InitPosCtrl();
#endif

#if 0
    displayBuffer[5] = LED_CODE[LED_NULL];          // LEDȫ��
    //displayBuffer[5] = LED_CODE[LED_ALL];          // LEDȫ��
    spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff); // ���¸�����ʾ���롣���������ڹ����������У�
                                                    // ��ʾ�������ʾɨ����Ⱥ�˳��scanStep����ʱ��
                                                    // curDisplayCode����û�и��£�������ʾ���롣
#elif 0
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[3] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[4] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[5] = LED_CODE[LED_NULL];
    scanStep = 5;
    spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff);
#elif 1
    scanStep = 5;
#endif

    dspMainCmd1.bit.shortGnd = funcCode.code.shortCheckMode;    // �ϵ�Եض�·��������
    dspMainCmd1.bit.pgLocation = motorFc.pgPara.elem.fvcPgSrc;

#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM

    // ���ܵĵ������
    for (i = REM_P_OFF_MOTOR - 1; i >= 0; i--)  // ������Ҫ�ĵ���������
    {
        gSendToFunctionDataBuff[i + MOTOR_TO_Func_2MS_DATA_NUM] = funcCode.code.remPOffMotorCtrl[i];
    }  
#endif
#endif

    UpdateDataFunc2Core0p5ms();           // �ڵ�����ѭ��֮ǰ���½�������
    UpdateDataFunc2Core2ms();

#if 0
//===========================
    //for (i = FNUM_PARA - 1; i >= 0; i--)    // û�а������籣��Ϳ�ݲ˵�
    for (i = 0; i < FNUM_PARA; i++)
    {
        kk = 0;
        index = i;
#if 0
        upper = funcCodeAttribute[index].upper;
        if (funcCodeAttribute[index].attribute.bit.upperLimit)
        {
            index = upper;
            upper = funcCodeAttribute[index].upper;
        }
#elif 1
        while (funcCodeAttribute[index].attribute.bit.upperLimit)
        {
            index = funcCodeAttribute[index].upper;
            if (++kk >= 5)
            {
                asm(" nop");
                break;
            }
        }
        upper1 = funcCodeAttribute[index].upper;
        asm(" nop");
#endif

        if (upper1 >= 10000)
            dataDigit = 5;
        else if (upper1 >= 1000)
            dataDigit = 4;
        else if (upper1 >= 100)
            dataDigit = 3;
        else if (upper1 >= 10)
            dataDigit = 2;
        else
            dataDigit = 1;

        if (funcCodeAttribute[index].attribute.bit.point >= dataDigit) // ������ʾ��С����λ��
            dataDigit = funcCodeAttribute[index].attribute.bit.point + 1;

        if (dataDigit != funcCodeAttribute[index].attribute.bit.displayBits)
        {
            jj++;
            asm(" nop");
        }
    }
    asm(" nop");
#endif

    //ECap3Regs.TSCTR= 0;

// �˵���ʼ��
    MenuInit();
}


//=====================================================================
//
// ��Ƶ�����ܴ�����
//
// ˵��:
// 1. ErrorDeal()����DO֮ǰ��DO����ErrorReset()֮ǰ��
// ���Ա��⣬��errorCodeһֱ���ڣ��ֲ�ͣ���ϸ�λʱ��
// DO��error���һ����һ��û�е������
//
// 2. ErrorReset()����UpdateDisplayBuffer()֮��
// ���Ա��⣬��errorCodeһֱ���ڣ��ֽ��й��ϸ�λʱ��
// ����Ҫ����ʾ��˸��
// 
// 3. ErrorDeal()����RunSrcDeal()֮�󣬴���Ƿѹʱ���б��������⡣
//
// 4. DisplayScanPrepare()��DisplayScan()֮�䣬��Լ��1800��SYSCLK��ͬʱ����
// SPI���պͷ���֮��(RCK��0��Ϊ1)����ʱ���á�
// RCK���л���Ҫ��ʱһ��ʱ��(1usʵ��OK)��
//
//=====================================================================
void Main2msFunction(void)
{
//    DINT; // ����ʹ��
    

//    EINT;
}


//=====================================================================
//
// ��Ƶ�����ܴ�����(0ms��������)
//
//=====================================================================
void Main0msFunction(void)
{
// PulseInSample()���Է���100us��ѭ����
	PulseInSample();            // PULSE IN�Ĳ�����������PulseInCalc().
	
//    SciDpDeal(&sciM380DpData);  //modefied by sjw 2009-12-24

#if DEBUG_F_POSITION_CTRL
    pulseInSamplePcQep();
#endif
}

#if 0
//=====================================================================
//
// ��Ƶ�����ܴ�����(0.5ms��������)
//
//=====================================================================
Uint32 allTime;
Uint32 upTimes;
Uint32 maxTime;
Uint32 minTime;
void Main05msFunction(void)
{
    static Uint16 ticker;
    Uint32 timeBase;

    timeBase = GetTime();
    ticker05msFuncInternal = timeBase05msOld - timeBase;
    timeBase05msOld = timeBase;

    //Main05msFunctionEachStart();    // ÿ��0.5ms������
    
    if (0 == ticker)
    {   
        Main05msFunctionA();        // 2ms�еĵ�0��0.5ms����0ms-0.5ms
    }
    else if (1 == ticker)
    {
        Main05msFunctionB();        // 2ms�еĵ�1��0.5ms����0.5ms-1.0ms
    }
    else if (2 == ticker)
    {
        Main05msFunctionC();        // 2ms�еĵ�2��0.5ms����1.0ms-1.5ms
    }
    else if (3 == ticker)
    {
        Main05msFunctionD();        // 2ms�еĵ�3��0.5ms����1.5ms-2.0ms     
    }
    Main05msFunctionEachEnd();      // ÿ��0.5ms������
    
    ticker05msFuncEach = timeBase - GetTime();

    if (3 == ticker)
    {
        allTime = 0;
        allTime += ticker05msFuncEa[0]; 
        allTime += ticker05msFuncEa[1]; 
        allTime += ticker05msFuncEa[2]; 
        allTime += ticker05msFuncEa[3];
		
		if (maxTime < allTime)
		{
			maxTime = allTime;
		}

		if (minTime > allTime)
		{
			minTime = allTime;
		}
    }
    

    ticker05msFuncEa[ticker] = ticker05msFuncEach;
    ticker05msFuncIn[ticker] = ticker05msFuncInternal;

    ticker++;
    if (ticker >= 4)
    {
        ticker = 0;
    }
}

#endif



// 0.5ms����
// 2ms�е�ÿ��0.5ms�����ã�������ǰ
void Main05msFunctionEachStart(void)
{
    static Uint16 waitPowerOnTicker;

    timeBase = GetTime();
    ticker05msFuncInternal = timeBase05msOld - timeBase;
    timeBase05msOld = timeBase;
    
    if ((dspStatus.bit.runEnable)                   // ���ܳ�ʼ�����
        && (dspStatus.bit.uv)                       // �Ҳ�Ƿѹ
        )
    {
        powerOnStatus = POWER_ON_CORE_OK;           // �ϵ�׼��OK
    }

    if (++waitPowerOnTicker >= WAIT_POWER_ON_TIME)  // ������ѭ����ʱ��ﵽ_ʱ��
    {
        if (POWER_ON_WAIT == powerOnStatus)
        {
            powerOnStatus = POWER_ON_FUNC_WAIT_OT;  // ���ܵĵȴ�ʱ�䳬ʱ
        }
    }
    
    commTicker++;                   // ���ڵ�ͣ��ʱ��

    canLinkTicker++;                // 

   	EepromDeal();                   // EEPROM������������Ķ�д
}



// 0.5ms���� 750
// 2ms�е�ÿ��0.5ms�����ã��������
void Main05msFunctionEachEnd(void)
{
	static Uint16 ticker = 0;
    
    ticker05msFuncEach = timeBase - GetTime();
    ticker05msFuncEa[ticker] = ticker05msFuncEach * 3413 >> 11;     // * 10 / 6
    ticker05msFuncIn[ticker] = ticker05msFuncInternal* 3413 >> 11;  // * 10 / 6
    
    ticker++;
    if (ticker >= 4)
    {
        ticker = 0;
    }
}


// 0.5ms����
// 2ms�еĵ�0��0.5ms����0ms-0.5ms
extern void vfSeparateDeal(void);
void Main05msFunctionA(void)
{
    asm(" nop");

    Main05msFunctionEachStart();    // ÿ��0.5ms������
    
    DisplayScanPrepare();       // 130.׼����ʾɨ��

    UpdateDataCore2Func();      // 680.�������ܴ��ݸ����ܵĽ�������(��ʱ����)
	
    DiCalc();                   // 2500
    PulseInCalc();              // 820

    Main05msFunctionEachEnd();
}



// 0.5ms����
// 2ms�еĵ�1��0.5ms����0.5ms-1.0ms
void Main05msFunctionB(void)
{
    Main05msFunctionEachStart();    // ÿ��0.5ms������
    
    AiCalc();                   // 2200

    UpdateMotorPara();          // ���µ������ (600)
    
#if DEBUG_F_POSITION_CTRL
    pulseInCalcPcEQep();        // ��չPG��������Ƶ�ʲ���
    PcMiscDeal();               // λ�ÿ��Ƶ�һЩ�����
#endif

    getMomeryValue();
    
    DisplayScan();              // ��ʾɨ�衣��DisplayScanPrepare()֮��ĳ���ͬʱ������ʱ�����á�

    KeyScan();                  // ����ɨ��(67)
    KeyProcess();               // ����(�˵�)����(47)
    P2PDataDeal();
#if DSP_2803X
    CanlinkFun();
    #if 0   // ���ڲ������ڴ�
    if ( (funcCode.code.commProtocolSec < CANLINK) && (funcCode.code.plcEnable != 1) )
    #endif
    {
    	SciDeal();                      // ����ͨѶ����(CANopen��PROFIBUS-DP��MODBUS)
    }
    OscSciFunction();                   // ��̨ͨѶ����(15)
#else

    CanlinkFun();
    if(funcCode.code.commMaster)
	{
		OscSciFunction();               // ��̨ͨѶ����(15)
	}
	else
	{

        if ( (funcCode.code.commProtocolSec < CANLINK) && (funcCode.code.plcEnable != 1) )
        {
        	SciDeal();                  // ͨѶ����---modefied by sjw 2009-12-24  306
        }
	}
#endif
    
    FrqSrcDeal();               // Ƶ��Դ 1100

    vfSeparateDeal();           // VF����Ĵ���(35)

    Main05msFunctionEachEnd();

}



// 0.5ms����
// 2ms�еĵ�2��0.5ms����1.0ms-1.5ms
void Main05msFunctionC(void)
{
    asm(" nop");

    Main05msFunctionEachStart();    // ÿ��0.5ms������

	AccDecTimeCalc();           // �Ӽ���ʱ����� 145
	
    runTimeCal();              // �ϵ�ʱ�䡢����ʱ��ͳ�� 100

    setTimeRun();              // ��ʱ����ʱ��ͳ�� 65

	RunSrcDeal();               // ����Դ   1300

	TorqueCalc();              // �����趨ת�� 80

	UpdateDataFunc2Core0p5ms(); // ���¹��ܴ��ݸ����ܵĽ�������  153

    EepromOperateTimeDeal();    // EEPROMʱ�䴦���ж��Ƿ�ʱ  40

    ErrorDeal();                // ����errorCode 527

    AoCalcChannel(AOFMP_AO1);   // AO1  510
#if DSP_2803X 
    AoCalcChannel(AOFMP_AO2);   // AO2  530
#endif
    FMPDeal();                  // FMP���� 710

    Main05msFunctionEachEnd();

}



// 0.5ms����
// 2ms�еĵ�3��0.5ms����1.5ms-2.0ms
void Main05msFunctionD(void)
{
    Main05msFunctionEachStart();    // ÿ��0.5ms������
    
    DoCalc();                   // DO  1590

    TemperatureDeal();          // ������´�����
    
    UpdateU0Data();              // 330

    UpdateDisplayBuffer();      // ����������ʾ����  440

    WinkDeal();                 // ��˸����   110

    ErrorReset();               // ���ϴ���    90

    UpdateDataFunc2Core2ms();   // 900

    CodeRunMonitor();           // 30

    if (0 == mainLoopTicker)    // ������ѭ���ĵ�1�ġ�����һ�Ŀ�ʼ�ſ�������Ƶ��/����Ƶ�ʵ���������
    {
        mainLoopTicker = 1;
    }

    Main05msFunctionEachEnd();

}



//#define CODE_RUN_DISPLAY_UPDATE_TIME        200     // _ms����������ʱ����ʾ����
#define CODE_MAX_RUN_DISPLAY_UPDATE_TIME    5000    // _ms���೤ʱ�����¸����ʱ��
Uint16 CODE_RUN_DISPLAY_UPDATE_TIME = 200;
// ����ִ��ʱ����
void CodeRunMonitor(void)
{
    static Uint16 ticker;

    if (++ticker >= CODE_RUN_DISPLAY_UPDATE_TIME / FUNC_DEAL_PERIOD)
    {
        ticker = 0;

        CODE_RUN_DISPLAY_UPDATE_TIME += 2;
        if (CODE_RUN_DISPLAY_UPDATE_TIME >= 300)
        {
            CODE_RUN_DISPLAY_UPDATE_TIME = 200;
        }

        ticker05msFuncEachDisp[0] = ticker05msFuncEa[0] * 3276 >> 15;
        ticker05msFuncEachDisp[1] = ticker05msFuncEa[1] * 3276 >> 15;
        ticker05msFuncEachDisp[2] = ticker05msFuncEa[2] * 3276 >> 15;
        ticker05msFuncEachDisp[3] = ticker05msFuncEa[3] * 3276 >> 15;

        ticker05msFuncInternalDisp[0] = ticker05msFuncIn[0] * 3276 >> 15;
        ticker05msFuncInternalDisp[1] = ticker05msFuncIn[1] * 3276 >> 15;
        ticker05msFuncInternalDisp[2] = ticker05msFuncIn[2] * 3276 >> 15;
        ticker05msFuncInternalDisp[3] = ticker05msFuncIn[3] * 3276 >> 15;

    }
}





// ��ȡ16λ����ÿһλ
// mode: 0--ʮ����, 1--ʮ������
// ������� digit[]��
// ���ظ����ֵ�λ��
Uint16 GetNumberDigit(Uint16 digit[5], Uint16 number, Uint16 mode)
{
    if (!mode)          // ʮ����
    {
        GetNumberDigit1(digit, number);
    }
    else                // ʮ������
    {
        GetNumberDigit2(digit, number);
    }

    if (digit[4])
            return 5;
    else if (digit[3])
        return 4;
    else if (digit[2])
        return 3;
    else if (digit[1])
        return 2;
    else //if (digit[0])
        return 1;
}

// ʮ����
void GetNumberDigit1(Uint16 digit[5], Uint16 number)
{
    Uint16 tmp;

    tmp = (Uint32)number * 52429 >> 19; // 52428.8 = 1/10*2^19
    digit[0] = number - tmp * 10;
    number = tmp;
    
    tmp = (Uint32)number * 52429 >> 19;
    digit[1] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[2] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[3] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[4] = number - tmp * 10;
    number = tmp;
}


// ʮ������
void GetNumberDigit2(Uint16 digit[5], Uint16 number)
{
    Uint16 tmp;

    tmp = number >> 4;
    digit[0] = number - (tmp << 4);
    number = tmp;
    
    tmp = number >> 4;
    digit[1] = number - (tmp << 4);
    number = tmp;

    tmp = number >> 4;
    digit[2] = number - (tmp << 4);
    number = tmp;

    tmp = number >> 4;
    digit[3] = number - (tmp << 4);
    number = tmp;

    // ��ֵ0,��ֹ�������ֵ
    digit[4] = 0;
}


// ����汾����
void SoftVersionDeal(void)
{
    funcCode.code.productVersion = PRODUCT_VERSION;    // ��Ʒ��
    funcCode.code.funcSoftVersion = SOFTWARE_VERSION;  // ���ܰ汾��

    // ��ʾ���������ܽϴ�İ汾��
    if (funcCode.code.funcSoftVersion < funcCode.code.motorSoftVersion)
    {
        funcCode.code.softVersion = funcCode.code.motorSoftVersion;
    }
    else
    {
        funcCode.code.softVersion = funcCode.code.funcSoftVersion;
    }
}


#if DEBUG_F_CHK_FUNC_CODE
// ��鹦�����Ƿ��д���
// ͬʱ�Զ���ñ��eeprom2Fc[], anybus2Fc[]
Uint16 sizeOfGroup;
Uint16 sizeOfGrade;
Uint16 sizeOfAll;
Uint16 sizeOfFuncCodeAttribute;
Uint16 sizeOfEeprom;
Uint16 sizeOfPara;
Uint16 errorFuncCode;
void ValidateFuncCode(void);


Uint16 eeprom2FcAutoGenerate[EEPROM_INDEX_USE_LENGTH];  // �Զ����ɵ�eeprom2Fc[]
extern const Uint16 fcNoAttri2Eeprom[];
Uint16 sizeOfTable_eeprom2Fc = EEPROM_INDEX_USE_LENGTH;
Uint16 maxEepromAddr;
Uint16 append;
void ValidateEepromAddr(void);
void GetTableEeprom2Fc(void);
void CheckEeprom2Fc(void);


// �жϹ������ ���������Ը������Ƿ�һ��
// ��Ӧ���ж�ÿ��ĸ����Ƿ�һ��
// funcCodeAttribute[((Uint16)((&(funcCode.group. bb [0] )) - (&(funcCode.all[0]))))]
// 
void ValidateFuncCode(void)
{
    // �ж� FUNCCODE_GROUP �� FUNCCODE_CODE �Ƿ�һ��
    sizeOfGroup = sizeof(struct FUNCCODE_GROUP);
    sizeOfGrade = sizeof(struct FUNCCODE_CODE);
    if (sizeOfGroup != sizeOfGrade)
    {
        asm(" ESTOP0");
    }

    // �ж� FUNCCODE_ALL �� FUNCCODE_CODE �Ƿ�һ��
    sizeOfAll = sizeof(FUNCCODE_ALL);
    if (sizeOfAll != sizeOfGrade)
    {
        asm(" ESTOP0");
    }

    // �ж����Ը����Ƿ�һ��
    //sizeOfFuncCodeAttribute = sizeof(funcCodeAttribute) / sizeof(FUNCCODE_ATTRIBUTE);
    sizeOfPara = FNUM_PARA;
    sizeOfEeprom = FNUM_EEPROM;
    if (funcCodeAttribute[FNUM_PARA-1].upper == 0)  // ����funcCodeAttribute[]���ܲ���
    {
        asm(" ESTOP0");
    }
    if (sizeOfFuncCodeAttribute != sizeOfPara)
    {
        asm(" nop");
        //asm(" ESTOP0");
    }
    
#if 0
    // �ж�U0����ʾ���Ը����Ƿ�һ��
    if (sizeof(dispAttributeU0) != U0NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }

    // �ж�U2����ʾ���Ը����Ƿ�һ��
    if (sizeof(dispAttributeU2) != U2NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }

    // �ж�U3����ʾ���Ը����Ƿ�һ��
    if (sizeof(dispAttributeU3) != U3NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }
#endif

    append = CHK_NUM + REM_NUM;

// �ж�EEPROM��ַ�Ƿ���ȷ
    ValidateEepromAddr();
// ��� eeprom2Fc[]
    GetTableEeprom2Fc();
// �������� eeprom2Fc[] �Ƿ���ȷ
    CheckEeprom2Fc();

    errorCode = errorFuncCode;
    if (errorFuncCode)
    {
        for (;;)
            ;
    }

// ��������������ѭ��������������������
    for (;;)
    {
        asm(" nop");
        asm(" ESTOP0");
    }
}



//--------------------------------------------------------
// �ж�EEPROM��ַ�Ƿ���ȷ
void ValidateEepromAddr(void)
{
    int16 i;
    int16 j;

// �Ƿ����ظ�
    for (i = 0; i < FNUM_PARA; i++)
    {
        // funcCodeAttribute[]�е�EEPROM��ַ�Ƿ����ظ�
        for (j = 0; j < FNUM_PARA; j++)
        {
            if ((i != j) 
                && (funcCodeAttribute[i].eepromIndex == funcCodeAttribute[j].eepromIndex)
                )
            {
                asm(" ESTOP0");
            }
        }
        
        // funcCodeAttribute[], ��fcNoAttri2Eeprom[]�еĵ�ַ�Ƿ����ظ�
        for (j = 0; j < append; j++)
        {
            if (funcCodeAttribute[i].eepromIndex == fcNoAttri2Eeprom[j]) 
            {
                asm(" ESTOP0");
            }
        }
    }

    // fcNoAttri2Eeprom[]�е�EEPROM��ַ�Ƿ����ظ�
    for (i = 0; i < append; i++)
    {
        for (j = 0; j < append; j++)
        {
            if ((i != j)
                && (fcNoAttri2Eeprom[i] == fcNoAttri2Eeprom[j])
                )
            {
                asm(" ESTOP0");
            }
        }
    }
    
// ���EEPROM��ַ�����ֵ
// �ж�max�Ƿ񳬹� EEPROM_INDEX_USE_LENGTH
    maxEepromAddr = 0;
    for (i = 0; i < append; i++)
    {
        if (maxEepromAddr < fcNoAttri2Eeprom[i])
            maxEepromAddr = fcNoAttri2Eeprom[i];
    }
    for (i = 0; i < FNUM_PARA; i++)
    {
        if (maxEepromAddr < funcCodeAttribute[i].eepromIndex)
            maxEepromAddr = funcCodeAttribute[i].eepromIndex;
    }
    // �ж�max�Ƿ񳬹� EEPROM_INDEX_USE_LENGTH
    // ���������� EEPROM_INDEX_USE_LENGTH Ҫ��Ӧ����!!
    if (maxEepromAddr >= EEPROM_INDEX_USE_LENGTH)
    {
        asm(" ESTOP0");
    }
}



// ���eeprom2Fc[]
#define EEPROM2FC_NONE      FUNCCODE_RSVD4ALL_INDEX     // ������ַ
// ��Ӧ��ϵ��
// y = eeprom2Fc[i]
// i, �����±꣬----�ù�������EEPROM��λ��
// y, �����ֵ��----����������
//
// �Զ�����
void GetTableEeprom2Fc(void)
{
    int16 i;
    int16 j;

    // ������� eeprom2fc[]
    for (i = 0; i <= EEPROM_INDEX_USE_LENGTH; i++)
    {
        for (j = 0; j < FNUM_PARA; j++)
        {
            if (i == funcCodeAttribute[j].eepromIndex)
            {
                eeprom2FcAutoGenerate[i] = j;
                break;
            }
        }
        
        if (j == FNUM_PARA) // funcCodeAttribute[].eepromIndexû��
        {
            for (j = 0; j < append; j++)
            {
                if (i == fcNoAttri2Eeprom[j])
                {
                    eeprom2FcAutoGenerate[i] = j + FNUM_PARA;
                    break;
                }
            }

            if (j == append)    // fcNoAttri2Eeprom[]Ҳû��
            {
                eeprom2FcAutoGenerate[i] = EEPROM2FC_NONE;
                //...
            }
        }
    }
}



// У������ eeprom2Fc[] �Ƿ���ȷ
void CheckEeprom2Fc(void)
{
    int16 i;
    
    for (i = 0; i < FNUM_PARA; i++) // ���ȼ�� A0-DF
    {
        if (eeprom2FcAutoGenerate[funcCodeAttribute[i].eepromIndex] != i)  // ����
        {
            asm(" ESTOP0");
        }
    }
    
    // CHK,REM,U1
    for (i = 0; i < append; i++)
    {
        if (eeprom2FcAutoGenerate[fcNoAttri2Eeprom[i]] != i + GetCodeIndex(funcCode.group.fChk[0]))  // ����
        {
            asm(" ESTOP0");
        }
    }
}
//--------------------------------------------------------

#endif

// ��������ڴ��ֵַ
void getMomeryValue(void)
{
	Uint16* a;
	a = (Uint16*)MEMORY_ADDRESS;
    memoryValue = *a;
}












