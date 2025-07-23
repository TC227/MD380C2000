//======================================================================
//
// ����Դ��
// �������frq
//
// Time-stamp: <2012-06-14 16:04:40  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_comm.h"
#include "f_ui.h"
#include "f_main.h"
#include "f_menu.h"
#include "f_eeprom.h"
#include "f_posCtrl.h"


extern int32 pcRef;
extern int32 pcFdb;

#if F_DEBUG_RAM                             // �����Թ��ܣ���CCS��build option�ж���ĺ�

#define DEBUG_F_RUN_SRC_DI          0       // ��������Դ
#define DEBUG_F_RUN_SRC_COMM        0       // ͨѶ����Դ
#define DEBUG_F_SWING               0       // ��Ƶ
#define DEBUG_F_TUNE                0       // ��г
#define DEBUG_F_POWER_OFF_NO_STOP   0       // ˲ͣ��ͣ
#define DEBUG_F_LOSE_LOAD           0       // ����
#define DEBUG_F_LOWER_FRQ           0       // ��������Ƶ��
#define DEBUG_F_JOG                 0       // jog
#define DEBUG_F_RUN_TIME            0
#define DEBUG_F_START_RUN           0
#define DEBUG_F_STOP_RUN            0
#define DEBUG_F_ACC_DEC_TIME        0
#define DEBUG_F_RUN_POWERUP_TIME    0
#define DEBUG_F_ERROR_RUN_FRQ       0
#define DEBUG_F_DORMANT_DEAL        0

#elif 1

#define DEBUG_F_RUN_SRC_DI          1
#define DEBUG_F_RUN_SRC_COMM        1
#define DEBUG_F_SWING               1
#define DEBUG_F_TUNE                1
#define DEBUG_F_POWER_OFF_NO_STOP   1
#define DEBUG_F_LOSE_LOAD           1
#define DEBUG_F_LOWER_FRQ           1
#define DEBUG_F_JOG                 1
#define DEBUG_F_RUN_TIME            1
#define DEBUG_F_START_RUN           1
#define DEBUG_F_STOP_RUN            1
#define DEBUG_F_ACC_DEC_TIME        1
#define DEBUG_F_RUN_POWERUP_TIME    1
#define DEBUG_F_ERROR_RUN_FRQ       1       // ����ʱ��������Ƶ��
#define DEBUG_F_DORMANT_DEAL        1       // ���߻���
#endif

union RUN_CMD runCmd;               // ����������
union RUN_FLAG runFlag;             // ���б�ʶ��
union RUN_STATUS_FIRST_STEP runStatus1Step;  // ����״̬��һ��
union DSP_MAIN_COMMAND dspMainCmd;  // ���ܴ��ݸ����ܵ���������
union DSP_MAIN_COMMAND1 dspMainCmd1;

union DSP_SUB_COMMAND dspSubCmd;    // ���ܴ��ݸ����ܵĸ�������
union DSP_STATUS dspStatus;         // ���ܴ��ݸ����ܵ�״̬��
union DSP_SUB_STATUS dspSubStatus;  // ����״̬��

union INVT_STATUS invtStatus;

Uint16 startProtectSrc;             // ��������Դ
Uint16 otherStopSrc;                // otherStopԴ

// otherStopԴ
// ����ͣ��
// plcʱ��ȫ��Ϊ0�����ߵ������н���ͣ��
Uint16 otherStopLowerSrc;

// otherStopLowerSrcԴ
// ����ʱ��������Ƶ��
// ����ʱ/����ʱ��������Ƶ�ʣ��ҵ�������Ƶ��ͣ��

enum RUN_STATUS runStatus;              // ����״̬
enum START_RUN_STATUS startRunStatus;   // ����ʱ������״̬
enum STOP_RUN_STATUS stopRunStatus;     // ͣ��ʱ������״̬
LOCALF enum RUN_STATUS runStatusOld4LoseLoad;
LOCALF enum RUN_STATUS runStatusOld4Jog;
LOCALF enum RUN_STATUS runStatusOld4POffNoStop;

Uint16 runSrc;          // ��������Դ
Uint32 accFrqTime;      // ����ʱ�䣬��λͬ������
Uint32 decFrqTime;      // ����ʱ��

enum SWING_STATUS swingStatus;  // ��Ƶ״̬
LOCALF Uint32 swingDoubleAw;    // ���ֵ���ڷ���2��
LOCALF int32 swingMaxFrq;       // ��Ƶ����Ƶ��
LOCALF int32 swingMinFrq;       // ��Ƶ����Ƶ��
LOCALF int32 swingJumpFrq;      // ��Ƶͻ��Ƶ��
LOCALF Uint32 swingAccTime;     // ��Ƶ����ʱ��
LOCALF Uint32 swingDecTime;     // ��Ƶ����ʱ��
Uint16 swingFrqLimit;           // ��Ƶ�޶���

Uint16 setRunLostTime;
Uint16 setRunTimeAim;               // �趨��ʱ����ʱ��
//LOCALF Uint32 setTimeTicker;        // ��ʱ���м�ʱ(S)
//LOCALF Uint32 setTimeSecTicker;     // ��ʱ���м�ʱ(S)
//LOCALF Uint32 setTimeMinTicker;     // ��ʱ���м�ʱ(M)
LOCALF Uint32 runTimeTicker;        // ����ʱ���ʱ
LOCALF Uint32 lowerDelayTicker;     // ��������Ƶ��ͣ��ʱ���ʱ
LOCALF Uint32 shuntTicker;          // ˲ͣ��ͣ�ĵ�ѹ����ʱ���ʱ
LOCALF Uint16 runTimeAddupTicker;   // �ۼ�����ʱ���ʱ
LOCALF Uint16 powerUpTimeAddupTicker;   // �ۼ�����ʱ���ʱ

Uint16 shuntFlag;    // ˲ͣ��ͣ��־

Uint16 accDecTimeSrcPlc;        // ��ǰPLC�εļӼ���ʱ��ѡ��

#define STOP_REM_NONE           0   // û��ͣ�����䣬����ͣ���������
#define STOP_REM_WAIT           1   // ��Ҫͣ������
#define STOP_REM_PAUSE_JUDGE    2   // ����֮����ͣ�������Ƿ�����ͣ���ж��Ƿ�ͣ������
LOCALF Uint16 stopRemFlag;
Uint16 runDirPanelOld;   // ��������old���з���

LOCALF Uint16 bRunJog;  // ͬʱ�����к͵㶯����������е㶯ʹ�ܣ���Ϊ1.

#define TUNE_STEP_OVER_TIME_MAX     60UL    // ��̬��г��ʱʱ�䣬_s
#define TUNE_RUN_OVER_TIME_MAX      180UL   // ����ʱ�ĵ�г��ʱʱ�䣬_s
enum TUNE_RUN_STATUS
{
    TUNE_RUN_STATUS_WAIT,           // �����ܷ��͵�г����ȴ�
    TUNE_RUN_STATUS_ACC,            // ��̬��г�����ٹ���
    TUNE_RUN_STATUS_DEC,            // ��̬��г��ɣ�ͣ��
    TUNE_RUN_STATUS_END             // ��г���
};
Uint16 tuneCmd;                 // ��г
Uint16 saveTuneDataFlag;        // ��г�����־��һ�ε�г����ֻ����һ��
//Uint16 tunePGflag;              // ������ʶPG����ʾ



//-----------------------------------------------------------
// ���߻���
// 1. �趨Ƶ�ʵ�������Ƶ��ʱ����������״̬���������������������ͣ��״̬��
// 2. �趨Ƶ�ʸ��ڻ���Ƶ��ʱ����Ӧ���������������������ʱ����������״̬��
// 3. ��һ�������������������Ƶ�ʣ�ҲҪ��Ӧ�������
// 4. �����뻽�ѵ��л�������ʱ���ɹ�����"�����ӳ�ʱ��"��"�����ӳ�ʱ��"ȷ����

// ���ߵ�״̬��Ϣ
enum DORMANT_STATUS
{
    DORMANT_RESPOND,            // ��Ӧ����������ڻ���Ƶ�ʣ����ߵ�һ������������ʱ��������Ƶ�ʡ�
    DORMANT_NO_RESPOND,         // ����Ӧ���������������Ƶ��
    DORMANT_2                   // δ��������״̬��������ʱ���ֵ�������Ƶ�ʡ�
};
enum DORMANT_STATUS dormantStatus;  // 0-û�����ߣ�1-��������״̬��2-δ��������״̬��������ʱ���ֵ�������Ƶ�ʡ�
// 
LOCALD void DormantDeal(void);

LOCALD void StartRunCtrl(void); // ����
LOCALD void NormalRunCtrl(void);
LOCALD void StopRunCtrl(void);
LOCALD void JogRunCtrl(void);   // �㶯
LOCALD void TuneRunCtrl(void);  // ��г
LOCALD void ShutDownRunCtrl(void);
LOCALD void LoseLoadRunCtrl(void);
LOCALD void LowerThanLowerFrqDeal(void);
LOCALD void PowerOffNoStopDeal(void);
LOCALD void SwingDeal(void);
LOCALD void UpdateSwingPara(void);
void SaveTuneData(void);

void AccCalc(int32 frq0);

void RunSrcUpdateFrq(void);

extern Uint16 spdLoopOut;

void setTimeRun(void);

//=====================================================================
//
// ��������������runCmd
//
// ����:
//
// 1. ������������Դ����ԴrunSrc��
//
// 2. �����������������ʽ������������runCmd��
//
// 3. ���ݱ�Ƶ���ڲ��߼�������������runCmd��
//
//=====================================================================
void UpdateRunCmd(void)
{
    static Uint16 bMfkSwitch;       // MFK�л��������Ч��־
    static Uint16 fwdOld, revOld, fJogOld, rJogOld;
    static Uint16 startProtectSrcOld;
    static Uint16 otherStopSrcOld;
    static Uint16 otherStopLowerSrcOld;
    static Uint16 bJog, bRun;
    static Uint16 jogCmdPulse;      // ������Ч��jog��Ŀǰ����ͨѶ�㶯
    static Uint16 runSrcBak;
    Uint16 jogCmdLevel;             // ��ƽ��Ч��jog
    Uint16 jogCmd;                  // �ܵ�jog����
    Uint16 protectCmd;              // ������־
    Uint16 fwd, rev, fJog, rJog;

    jogCmdLevel = 0;
    protectCmd = 0;

    fwd = diFunc.f1.bit.fwd;
    rev = diFunc.f1.bit.rev;
    fJog = diFunc.f1.bit.fwdJog;
    rJog = diFunc.f1.bit.revJog;

    if (POWER_ON_WAIT == powerOnStatus)  // �ϵ绹û��׼�����
    {
        fwdOld = fwd;
        revOld = rev;
        fJogOld = fJog;
        rJogOld = rJog;
    }

    // 1. ��������ԴrunSrc
    if (KEY_SWITCH == keyFunc)
    {
        bMfkSwitch = !bMfkSwitch;
        keyFunc = 0;
    }
    if (FUNCCODE_mfkKeyFunc_SWITCH != funcCode.code.mfkKeyFunc)
    {
        bMfkSwitch = 0;         // MF.K���Ĺ��ܲ����л�������������
    }

    runSrc = funcCode.code.runSrc;      // �������趨
    
    // MF.K���л����������
    if (bMfkSwitch)                     // MF.K�����л����������
    {
        runSrc = FUNCCODE_runSrc_PANEL;
    }
    
    // DI��comm������л�
    if (diFunc.f1.bit.localOrRemote)        
    {
        if (FUNCCODE_runSrc_PANEL != runSrc)
        {
            runSrc = FUNCCODE_runSrc_PANEL;
        }
#if 0
        else
        {
            runSrc = funcCode.code.runSrc;
        }
#endif
    }
    // DI��comm�л�
    else if (diFunc.f2.bit.diOrComm)        
    {
        if (FUNCCODE_runSrc_DI == funcCode.code.runSrc)
        {
            runSrc = FUNCCODE_runSrc_COMM;
        }
        else if (FUNCCODE_runSrc_COMM == funcCode.code.runSrc)
        {
            runSrc = FUNCCODE_runSrc_DI;
        }
    }

    // ���������л�ʱͣ��
    if (runSrcBak != runSrc)
    {
		runCmd.bit.common0 = 0;
        runCmd.bit.common = 0;
    }
    runSrcBak = runSrc;
    
    // 2. �����������������ʽ������������runCmd
    runCmd.bit.freeStop = 0;    // ����� freeStop, pause, tune
    runCmd.bit.errorReset = 0;
    runCmd.bit.pause = 0;
    
    switch (runSrc)                     // ��������ͨ��
    {
        case FUNCCODE_runSrc_PANEL:     // �������������ʽ: �������
            jogCmdPulse = 0;
            
            if ((KEY_STOP == keyFunc) || (diFunc.f2.bit.stopPanel))
            {
                runCmd.bit.common0 = 0;             // ͣ��
            }
            else if (KEY_REV == keyFunc)
            {
                runCmd.bit.dir = ~runCmd.bit.dir;   // ��ת
                runDirPanelOld = runCmd.bit.dir;    // �ı䷽��ʱ�Ÿ���runDirPanelOld
                keyFunc = 0;
            }
            else if (KEY_RUN == keyFunc)
            {
                runCmd.bit.common0 = 1;             // ����
                keyFunc = 0;
            }

            if (!runFlag.bit.run)                   // ͣ��ʱ�Żָ�runCmd.bit.dir
            {
                runCmd.bit.dir = runDirPanelOld;    // û���л�����Դʱ��û��Ӱ�졣
            }

            break;
            
#if DEBUG_F_RUN_SRC_DI
        case FUNCCODE_runSrc_DI:                        // �������������ʽ: ����
            jogCmdPulse = 0;
            
            switch (funcCode.code.diControlMode)        // �������ʽ
            {
                case FUNCCODE_diControlMode_2LINE1:     // ����ʽ1
                case FUNCCODE_diControlMode_2LINE2:     // ����ʽ2
                    if (FUNCCODE_diControlMode_2LINE1 == funcCode.code.diControlMode)   // ����ʽ1
                    {
                        if (fwd && (!rev))              // fwdRun
                        {
                            runCmd.bit.dir = FORWARD_DIR;
                            runCmd.bit.common0 = 1;
                        }
                        else if ((!fwd) && (rev))       // revRun
                        {
                            runCmd.bit.dir = REVERSE_DIR;
                            runCmd.bit.common0 = 1;
                        }
                        else
                        {
                            runCmd.bit.common0 = 0;     // ͣ��
                        }

                        protectCmd = 1; // ������ʽ2������ʽ1��Ҳ��ҪҪ����������
                        // ���磬fwd=0,rev=1,����ʽ2��Ȼ���Ϊ����ʽ1��Ӧ�ý�������������
                    }
                    else    // ����ʽ2
                    {
                        runCmd.bit.common0 = fwd;   // ��������
                    
                        if (rev)                // ����
                            runCmd.bit.dir = REVERSE_DIR;
                        else
                            runCmd.bit.dir = FORWARD_DIR;

                        protectCmd = 2;     // ������ʽ1������ʽ2��Ҳ��ҪҪ����������
                        // ���磬fwd=1,rev=1,����ʽ1��Ȼ���Ϊ����ʽ2��Ӧ�ý�������������
                    }
#if 0   //+e �ı�������ʽ��������Դ���Ƿ���Ҫ����? ��ʱ������
                    if ((protectCmdOld != protectCmd) // �ı�������ʽ��������Դ������0->1,0->2,1->2,2->1
                        && (((runFlag.bit.run) && (RUN_STATUS_STOP == runStatus))
                            || (!runFlag.bit.run)
                            )   // ����/���ӵ㶯����������Դ����Ч
                        )
                        startProtectSrc++;
#endif
                    break;

                case FUNCCODE_diControlMode_3LINE1:         // ����ʽ1
                case FUNCCODE_diControlMode_3LINE2:         // ����ʽ2
                    if (fwdOld != fwd)  // ����(������)��Ч
                    {
                        if (fwd)        // fwd
                        {
                            runCmd.bit.common0 = 1;
                            
                            if (FUNCCODE_diControlMode_3LINE1 == funcCode.code.diControlMode)
                            {
                                runCmd.bit.dir = FORWARD_DIR;
                            }
                        }
                    }

                    if (FUNCCODE_diControlMode_3LINE1 == funcCode.code.diControlMode) // ����ʽ1
                    {
                        if (revOld != rev)
                        {
                            if (rev)
                            {
                                runCmd.bit.common0 = 1;
                                runCmd.bit.dir = REVERSE_DIR;
                            }
                        }
                    }
                    else        // ����ʽ2
                    {
                        rev ? (runCmd.bit.dir = REVERSE_DIR) : (runCmd.bit.dir = FORWARD_DIR);
                        #if 0
                        if (rev)          // ����
                            runCmd.bit.dir = REVERSE_DIR;
                        else
                            runCmd.bit.dir = FORWARD_DIR;    
                        #endif
                    }

                    if (!diFunc.f1.bit.tripleLineCtrl)  // ��������
                    {
                        runCmd.bit.common0 = 0;
                    }

                    break;

                default:
                    break;
            }

            break;
#endif

#if DEBUG_F_RUN_SRC_COMM
        case FUNCCODE_runSrc_COMM:                 // �������������ʽ: ����ͨѶ
#if DEBUG_F_PLC_CTRL
            // PLC������Чʱ��ͨѶ������Դ��PLC����
            if (funcCode.code.plcEnable)
            {
               commRunCmd = funcCode.code.plcCmd;
            }
#endif            
            switch (commRunCmd)
            {
#if 1
                case SCI_RUN_CMD_FWD_RUN:
                    if (!runFlag.bit.jog)   // ��ǰû�е㶯/�����е㶯�����ܸ��ĵ㶯�ķ���
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = FORWARD_DIR;
                    }
                    break;
                    
                case SCI_RUN_CMD_REV_RUN:
                    if (!runFlag.bit.jog)
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = REVERSE_DIR;
                    }
                    break;
                    
#elif 1
                case SCI_RUN_CMD_FWD_RUN:
                    // ��ǰû�е㶯/�����е㶯�����ܸ��ĵ㶯�ķ���
                    if ((!runFlag.bit.jog) || (jogCmdPulse))    // ͨѶ����ĵ㶯��Ȼ������ͨѶ�������Ҫ��Ӧ��
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = FORWARD_DIR;
                        jogCmdPulse = 0;
                    }
                    break;
                case SCI_RUN_CMD_REV_RUN:
                    if ((!runFlag.bit.jog) || (jogCmdPulse))
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = REVERSE_DIR;
                        jogCmdPulse = 0;
                    }
                    break;
#endif
                case SCI_RUN_CMD_FWD_JOG:
                    jogCmdPulse = RUN_CMD_FWD_JOG;
                    break;
                    
                case SCI_RUN_CMD_REV_JOG:
                    jogCmdPulse = RUN_CMD_REV_JOG;
                    break;
                    
                case SCI_RUN_CMD_FREE_STOP:
                    otherStopSrc++;     // ����ͣ��
                    runCmd.bit.freeStop = 1;
                    //lint -fallthrough     //ͨѶ����ͣ��ҲҪ���jogCmdPulse��common0
                    
                case SCI_RUN_CMD_STOP:
                    jogCmdPulse = 0;    // ͨѶstop����ֹͣͨѶ�㶯��������ֹͣ�����㶯
                    runCmd.bit.common0 = 0;
                    break;
                    
                case SCI_RUN_CMD_RESET_ERROR:
                    runCmd.bit.errorReset = 1;
                    break;
                    
                default:
                    break;
            }

            break;
#endif

#if 0
        // �ϵ�����
        case FUNCCODE_runSrc_AUTO_RUN:
            runCmd.bit.common0 = 1;
            break;
#endif            
        default:
            break;
    }

//--------------------------------------------------------------------------------------
// 3. ���ݱ�Ƶ�����ڲ��߼�������runCmd
#if DEBUG_F_ACC_DEC_TIME
    // ����ʱ�������������������еĹ��ϣ���������������
    if (((errorCode && (errorAttribute.bit.level != ERROR_LEVEL_RUN))   // ����
          || (POWER_ON_WAIT == powerOnStatus)                           // �ϵ绹û��׼�����
          )
        )
    {
        startProtectSrc++;
    }

    if (bStopPlc)    // plcʱ��ȫ��Ϊ0�����ߵ������н���ͣ��
    {
        if (runCmd.bit.common0)      // ������
        {
            otherStopSrc++;
        }

        bStopPlc = 0;
    }

    if ((diFunc.f1.bit.closePwm)         // ��������ͣ����ǿ�Ʊ���
        ||(diFunc.f2.bit.emergencyStop)   // ���ӽ���ͣ��
		||(diFunc.f2.bit.stop4dec)        // ����ʱ��4ͣ��
        || ((FUNCCODE_stopKeyFunc_ALL == funcCode.code.stopKeyFunc) // ͣ�����ܸ��ֿ��Ʒ�ʽ����Ч
            && (KEY_STOP == keyFunc)    // �������Դ��stopҲ����ֹͣ���ӵ㶯��
            )
        )
    {
        otherStopSrc++;
        jogCmdPulse = 0;    // stopֹͣͨѶ�㶯
    }

    if (startProtectSrcOld != startProtectSrc)  // �������µ�startProtectSrc��bJog��bRun����
    {
        bJog = 0;
        bRun = 0;

        runCmd.bit.startProtect = 1;
    }

    if ((otherStopSrcOld != otherStopSrc)    // �������µ�otherStopSrc����bJog��bRun����
        || (otherStopLowerSrcOld != otherStopLowerSrc))
    {
        bJog = 0;
        bRun = 0;

        runCmd.bit.otherStop = 1;
    }

    if (!funcCode.code.startProtect) // ����������������Ч��û�б�Ҫ��bJog��bRun��Ϊ1
    {
        runCmd.bit.startProtect = 0;
    }

    if (runCmd.bit.startProtect || runCmd.bit.otherStop)
    {
        if ((fJogOld != fJog)       // ����㶯���ӷ����仯
            || (rJogOld != rJog)    // ����㶯���ӷ����仯
            || ((!fJog) && (!rJog)) // ����㶯������㶯���Ӷ�Ϊ��Ч
            )
        {
            bJog = 1;
        }

        if (!bJog)              // jog���ڱ���״̬����������/otherStop
        {
            fJog = 0;
            rJog = 0;
        }

        if (!runCmd.bit.common0)
        {
            bRun = 1;
        }

        if ((!runFlag.bit.common) // �Ѿ�ͣ��
            && (FUNCCODE_pidCalcMode_YES == funcCode.code.pidCalcMode)
            && (otherStopLowerSrcOld != otherStopLowerSrc) // ��ˮģʽ��������Ƶ��/����Ƶ�ʲ�����
            )
        {
            otherStopLowerSrcOld = otherStopLowerSrc;

            bRun = 1;
        }

        if (!bRun)              // ���ڱ���״̬
        {
            runCmd.bit.common0 = 0;
        }

        if (!protectCmd)        // ������ʽ��������Դ
        {
            if (bJog)
            {
                runCmd.bit.startProtect = 0;
                runCmd.bit.otherStop = 0;
                
                otherStopLowerSrcOld = otherStopLowerSrc;
            }
        }
        else                    // ����ʽ��������Դ
        {
            if (bRun && bJog)
            {
                runCmd.bit.startProtect = 0;
                runCmd.bit.otherStop = 0;

                otherStopLowerSrcOld = otherStopLowerSrc;
            }
        }
    }
    
    fwdOld = diFunc.f1.bit.fwd;
    revOld = diFunc.f1.bit.rev;
    fJogOld = diFunc.f1.bit.fwdJog;
    rJogOld = diFunc.f1.bit.revJog;
 // protectCmdOld = protectCmd;
    startProtectSrcOld = startProtectSrc;
    otherStopSrcOld = otherStopSrc;
#endif

    commRunCmd = 0;                 // ͨѶ����������

#if DEBUG_F_PLC_CTRL
    // PLC��������0
   funcCode.code.plcCmd = 0;
#endif

#if DEBUG_F_JOG
    // �������/���ӵ㶯���ͨѶ�㶯ʱ����ͨѶ�����������
    if ((KEY_FWD_JOG == keyFunc) && (runSrc == FUNCCODE_runSrc_PANEL))    // ���㶯��������㶯
    {
        jogCmdLevel = RUN_CMD_FWD_JOG;
    }
    else if ((KEY_REV_JOG == keyFunc) && (runSrc == FUNCCODE_runSrc_PANEL))   // ���㶯��������㶯
    {
        jogCmdLevel = RUN_CMD_REV_JOG;
    }
    else if ((fJog) && (runSrc == FUNCCODE_runSrc_DI))  // �������㶯
    {
        jogCmdLevel = RUN_CMD_FWD_JOG;
    }
    else if ((rJog) && (runSrc == FUNCCODE_runSrc_DI))  // ���ӷ��㶯
    {
        jogCmdLevel = RUN_CMD_REV_JOG;
    }
    // ���򣬵�ƽ�㶯����Ϊ��
#endif


//---------------------------------------------------------------
// ���Ϻ����з�ʽ
// ����ͣ��������ͣ������������
    if ((ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level)   // ����ͣ��
       	|| (((runStatus == RUN_STATUS_TUNE)                  // ��г �� ���ڹ���
       	    || (dspMainCmd1.bit.speedTrack)                  // ת��׷�� �Ҵ��ڹ���
       	    )
       	    &&  errorCode
       	   )
		)
    {
        if (runCmd.bit.common0)  // ����������
        {
            menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // �������/�澯��ʾ״̬
        }
        
        runCmd.bit.freeStop = 1;
    }
    else if (ERROR_LEVEL_STOP == errorAttribute.bit.level)  // ��ͣ����ʽͣ��
    {
        if (runCmd.bit.common0)  // ����������
        {
            menu0DispStatus = MENU0_DISP_STATUS_ERROR;      // �������/�澯��ʾ״̬
        }
        
        runCmd.bit.common0 = 0;
    }
    else if (ERROR_LEVEL_RUN == errorAttribute.bit.level)   // ��������
    {
        ;
    }
//---------------------------------------------------------------

// ���ߡ�����
    DormantDeal();

// common run
    runCmd.bit.common = 0;
    if ((runCmd.bit.common0)  // ����������
        && (DORMANT_RESPOND == dormantStatus)
        )
    {
        Uint16 common = 0;
        
        if ((runFlag.bit.common) || (runFlag.bit.tune)) // ��ǰ�������У���������tune
        {
            common = 1;
        }
        else if ((!runFlag.bit.jog) && (!runFlag.bit.tune)) // ׼������
        {
            if (((ABS_INT32(frqAimTmp0) >= funcCode.code.startFrq)  // ����ʱ����������Ƶ�ʲ���������ԾƵ�ʼ���֮ǰ��
                 || (runFlag.bit.pid)   // PID(ͣ��������)ʱ����ʹ������Ƶ�ʣ�Ҫ��֤������
                 )
                && ((FUNCCODE_lowerDeal_DELAY_STOP != funcCode.code.lowerDeal) // ������Ƶ������/���������У���ԾƵ�ʼ���֮��
                    || (ABS_INT32(frqAimTmp) >= lowerFrq) // �趨Ƶ�ʲ���������Ƶ��
                    )
                )
            {
                common = 1;
            }
            else
            {
                if (2 == mainLoopTicker)    // ������ѭ���ĵ�1�ģ����ش���otherStopLowerSrc(�̶�����������)
                {
                    otherStopLowerSrc++;
                }
                
                runCmd.bit.common0 = 0;     // ����������
            }
        }
        else
        {
            if (!runFlag.bit.jogWhenRun)    // ������ͣ�е������е㶯������Ҳ��ִ�е�����
            {
                runCmd.bit.common0 = 0;
            }
        }

        if (common && 
            (!diFunc.f1.bit.runPause)       // ������ͣ������Ч
            )
        {
            runCmd.bit.common = 1;          // ������Ч����������
        }
    }

// free stop
    if (diFunc.f1.bit.closePwm  // DI��������ͣ����common/jog/tune����Ч

//        || errorCode
        || (POWER_ON_WAIT == powerOnStatus)        // ���������ϵ紦�����ܷ�������/�㶯����
        || (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode)     // �ָ�(����)�����趨ֵ
        || (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode) // �ָ�(ȫ��)�����趨ֵ
//+e        || (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode)  // �ָ�������û�������
//        || ((RUN_MODE_TORQUE_CTRL == runMode) && (!runCmd.bit.common0)) // ת�ؿ���ģʽ�£�ͣ������
        )
    {
        runCmd.bit.freeStop = 1;          // ���ǻָ�����������������Ϊ�˲���Ӧ�������� 
    }

    // ����ͣ��
    if (diFunc.f2.bit.emergencyStop || diFunc.f2.bit.stop4dec)
    {
        runCmd.bit.common0 = 0;     // common run
        runCmd.bit.common = 0;
    }
    
    if ((diFunc.f1.bit.runPause) && (!runCmd.bit.freeStop))//+e || (extendCmd.bit.runPause))
    {
        runCmd.bit.pause = 1;       // ����ͣ��ʱ�����������ͣ����
    }

	// || (0) //+e ת�ؿ���, (funcCode.code.torqueControl && (!diFunc.f2.bit.forbid_torque_control))
    //+e (extendCmd.bit.freeStop)
    if ((runFlag.bit.common) 
    	&& (!runCmd.bit.common)
        && (!runFlag.bit.jogWhenRun)                              // ��ͨͣ������
        && (FUNCCODE_stopMode_FREESTOP == funcCode.code.stopMode) // ͣ����ʽΪ����ͣ��
        && (DORMANT_NO_RESPOND != dormantStatus)                  // ����������״̬
        )
    {
        runCmd.bit.freeStop = 1;
    }

    if ((runCmd.bit.freeStop)       // ���ϣ�����ͣ�����ӣ��ָ���������
        && (!runCmd.bit.pause)      // ������ͣ���������ͣ����������������
        )
    {
        runCmd.bit.common0 = 0;     // common run
        runCmd.bit.common = 0;
        jogCmdPulse = 0;            // ��jog����
        jogCmdLevel = 0;
    }

// jog
    jogCmd = jogCmdLevel | jogCmdPulse; // �����һ���㶯����

// jog when run, �ж��Ƿ������е㶯
    bRunJog = 0;
    if ((jogCmd)
        && (runFlag.bit.common                              // ��ǰ��������
            || ((runCmd.bit.pause) && (runCmd.bit.common0)) // ��ǰ��������֮���������ͣ״̬
            || runCmd.bit.common                            // ͬʱ�����к͵㶯����
            )
        )
    {
        if (funcCode.code.jogWhenRun)   // �����е㶯ʹ��
        {
            bRunJog = 1;                     // ͬʱ�����к͵㶯����������е㶯ʹ��
            
            if ((!dspMainCmd.bit.startBrake) // ֱ���ƶ������У������е㶯��Ч
                && (!dspMainCmd.bit.stopBrake)
                && (!dspMainCmd.bit.startFlux) // ��������
                )
            {
                runFlag.bit.jogWhenRun = 1;
            }
            else
            {
                jogCmd = 0;
            }
        }
        else
        {
            jogCmd = 0;         // ���������е㶯(����ʱ�������е㶯��Ч����㶯����)
        }
    }

#if DEBUG_F_JOG
    if (RUN_CMD_FWD_REV_JOG == jogCmd)  // ��������㶯������з���㶯�����Ϊ����㶯
    {
        jogCmd = RUN_CMD_FWD_JOG;
    }

    runCmd.bit.jog = jogCmd;

    if (jogCmdPulse != jogCmd)    // ���յĵ㶯���������㶯������������㶯����
    {
        jogCmdPulse = 0;
    }
#endif

// ������ͣ��ͣ������
    if (STOP_REM_PAUSE_JUDGE == stopRemFlag)
    {
        if (!runCmd.bit.pause)          // ������ͣ������Ч
        {
            stopRemFlag = STOP_REM_NONE;
        }
        else if (!runCmd.bit.common0)   // ������ͣ״̬��������ͣ��
        {
            stopRemFlag = STOP_REM_WAIT;
        }
    }
//--------------------------------------------------------------------------------------

}

//=====================================================================
// 
// ��ʱ����
//
//=====================================================================
void setTimeRun()
{
	Uint16 i;

    // ��ʱ������Ч
    if (!funcCode.code.setTimeMode)
    {
        // ʣ������ʱ��Ϊ0
        setRunLostTime = 0;
        setRunTimeAim = 0;
        return;
    }
    
    // ��ʱ�趨ʱ������
    setRunTimeAim = funcCode.code.setTimeValue;

    // ��ʱʱ���趨��Դ
    switch (funcCode.code.setTimeSource)
    {
        case FUNCCODE_plcFrq0Src_AI1:    // AI1\AI2\AI3
        case FUNCCODE_plcFrq0Src_AI2:
        case FUNCCODE_plcFrq0Src_AI3:
            i = funcCode.code.setTimeSource - FUNCCODE_plcFrq0Src_AI1;
            setRunTimeAim = ((int32)ABS_INT32(aiDeal[i].set) * setRunTimeAim + (1 << 14)) >> 15;
            break;

        default:
            break;
    }

    // �趨��ʱʱ�䲻Ϊ0
    if (setRunTimeAim > 0)
    {
        // ��ʱ����(���Զ�����)ͣ��
        if (curTime.runTime >= setRunTimeAim) 
        {
            // ��ʱʱ�䵽��ͣ��
            runCmd.bit.common0 = 0;
       		runCmd.bit.common = 0;
            otherStopSrc++;
            setRunLostTime = 0;    // ��ʱ����ʣ��ʱ��Ϊ0
        }
        else
        {
            setRunLostTime = setRunTimeAim - curTime.runTime;
        }
    }
    else
    {
        setRunLostTime = 0;
    }

}

//=====================================================================
// 
// �ϵ硢����ʱ��ͳ��
//
//=====================================================================
void runTimeCal(void)
{
#if DEBUG_F_RUN_POWERUP_TIME
    if(++powerUpTimeAddupTicker >= ((Uint16)(TIME_UNIT_MS_PER_SEC / RUN_TIME_CAL_PERIOD)))
    {
        powerUpTimeAddupTicker = 0;

        // ��ǰ�ϵ�ʱ��
        if(++curTime.powerOnTimeSec>= TIME_UNIT_SEC_PER_MIN)
        {
            curTime.powerOnTimeSec = 0;   // ��ǰ�ϵ�ʱ������0
            curTime.powerOnTimeM++;       // ��ǰ�ϵ�ʱ���++
        }
        
        if (++funcCode.code.powerUpTimeAddupSec >= TIME_UNIT_SEC_PER_HOUR)
        {
            funcCode.code.powerUpTimeAddupSec = 0;  // ����ʱ�ű��浽EEPROM
            funcCode.code.powerUpTimeAddup++;       // �ۼ��ϵ�ʱ��, ��λ: h��ͣ��ʱ���浽EEPROM
        }
    }

    // �ۼ��ϵ�ʱ�� �ﵽ �趨�ϵ�ʱ�䣬
    if (funcCode.code.powerUpTimeArriveSet // �趨�ϵ�ʱ��Ϊ0����ʱ������Ч
        && (funcCode.code.powerUpTimeAddup >= funcCode.code.powerUpTimeArriveSet)  // �ۼ�����ʱ�� �ﵽ �趨����ʱ�䣬
        )
    {
        errorOther = ERROR_POWER_UP_TIME_OVER;      // �ϵ�ʱ�䵽��
    }
#endif
    
#if DEBUG_F_RUN_TIME
    // ��������״̬�ұ�������ʱ�����������Ч
    // ���ʱ�䳬������ʱ�䵽���������ʱ��������ӿ�ʼ��Ч
    if ((runFlag.bit.run) 
        && ((!diFunc.f2.bit.clearSetRunTime) 
        || ((curTime.runTime >= funcCode.code.setTimeArrive)
            &&(funcCode.code.setTimeArrive)))
		)
    {
        if (++runTimeAddupTicker >= ((Uint16)(TIME_UNIT_MS_PER_SEC / RUN_TIME_CAL_PERIOD)))
        {
            runTimeAddupTicker = 0;

            if(++curTime.runTimeSec >= TIME_UNIT_SEC_PER_MIN)
            {
                curTime.runTimeSec = 0;   // ��ǰ����ʱ������0
                curTime.runTimeM++;       // ��ǰ����ʱ���++
            }
            
            if (++funcCode.code.runTimeAddupSec >= TIME_UNIT_SEC_PER_HOUR)
            {
                funcCode.code.runTimeAddupSec = 0;  // ����ʱ�ű��浽EEPROM
                funcCode.code.runTimeAddup++;       // �ۼ�����ʱ��, ��λ: h��ͣ��ʱ���浽EEPROM
            }
        } 

    }
    else
    {
        // �嵱ǰ����ʱ��
        curTime.runTimeSec = 0;   
        curTime.runTimeM = 0;       
    }

    // ��ǰ�ϵ�ʱ�䡢����ʱ��
    curTime.runTime = curTime.runTimeM*10 + curTime.runTimeSec/6;
    curTime.powerOnTime = curTime.powerOnTimeM;

    // �ۼ�����ʱ�� �ﵽ �趨����ʱ�䣬
    if (funcCode.code.runTimeArriveSet // �趨����ʱ��Ϊ0����ʱ������Ч
        && (funcCode.code.runTimeAddup >= funcCode.code.runTimeArriveSet)  // �ۼ�����ʱ�� �ﵽ �趨����ʱ�䣬
        )
    {
        errorOther = ERROR_RUN_TIME_OVER;      // ����ʱ�䵽��
    }
   
#endif
}

//=====================================================================
// 
// ����runCmd��
// (1) ���ǰ˲ʱ����Ƶ��frq
// (2) ���´��ݸ�dspMainCmd��run, dspSubCmd.
//
// ��ע��
// (1) ������������ʱ��Ҫ�������ʱ��֮�ڴ�PWM��
//     �磬����ֵĬ��(û������ֱ���ƶ���û������Ƶ�ʱ���)ʱ�����ڸ���
//     ��������ĵ�ǰ���ڣ����ܸ�����˲ʱƵ�ʺͿ���PWM�����
// (2) ����/�㶯ʱ������ͣ��/����ͣ��/ֹͣ�㶯������ʱ��Ҫ�������ʱ��
//     �ڹر�PWM��
//     �磬û��ͣ��ֱ���ƶ��ȴ���û��ͣ��ֱ���ƶ�ʱ�����ڸ���ͣ������
//     �ĵ�ǰ���ڣ��͸����ܼ�С��˲ʱƵ�ʵ������ͣ��ʱ��Ϊ0����Ҫ
//     �ڵ�ǰ���ڸ����ر�PWM�����
//
//=====================================================================
void RunSrcDeal(void)
{
// ������������
    UpdateRunCmd();

// ����˲ʱƵ��
    RunSrcUpdateFrq();

#if DEBUG_F_POSITION_CTRL
{
    UniversalDebug(0);
    UniversalDebug(1);
    UniversalDebug(2);
    UniversalDebug(3);
    UniversalDebug(4);
    UniversalDebug(5);
}
#endif
}



//=====================================================================
// 
// ���¼Ӽ���ʱ���׼
// 
//=====================================================================
void UpdateBenchTime(void)
{
    switch (funcCode.code.accDecTimeUnit)
    {
        case FUNCCODE_accDecTimeUnit_0POINT:
            timeBench = 1000;
            break;
            
        case FUNCCODE_accDecTimeUnit_1POINT:
            timeBench = 100;
            break;

        default:
            timeBench = 10;
            break;     
    }
}




void RunSrcUpdateFrq(void)
{
    static Uint16 errorLevelOld, runOldFrqFlag;
    static int32 frqOld;
    
#if (DEBUG_F_POSITION_CTRL)
    if (FUNCCODE_pcZeroSelect_ENCODER == funcCode.code.pcZeroSelect) // ������z�ź���Ϊ��λ�ź�
    {
        if ((FUNCCODE_aptpMode_INDEX == funcCode.code.aptpMode)
            && (1 == pEQepRegsFvc->QFLG.bit.IEL)
            )
        {
            aptpAbsZeroOk = 1;
            pcOrigin = pEQepRegsFvc->QPOSILAT; // ԭ��
        }
        else if ((!aptpAbsZeroOk)
                && (1 == pEQepRegsFvc->QFLG.bit.IEL)
                )
        {
            aptpAbsZeroOk = 1;
            pcOrigin = pEQepRegsFvc->QPOSILAT; // ԭ��
        }
    }
    else if (FUNCCODE_pcZeroSelect_DI == funcCode.code.pcZeroSelect) // DI������Ϊ���ź�
    {
#if 0
        if (diFunc.f3.bit.aptpZero)
        //if (1)
        {
            pcOrigin = GetCurPos(); // ԭ��
            aptpAbsZeroOk = 1;
        }
#endif
    }
    else if ((FUNCCODE_aptpMode_ABSOLUTE == funcCode.code.aptpMode)
        && (2 == funcCode.code.pcZeroSelect)
        )
    {
        pcOrigin = 0 - qposcntLastPOff; // ����ԭ��
        //pcOrigin = 0;
        aptpAbsZeroOk = 1;
    }
    
    if (aptpAbsZeroOk)
    {
        aptpCurPos = (int32)(GetCurPos() - pcOrigin);
        //pcCurrentPulse = (aptpCurPos / 4);
        pcCurrentPulse = (aptpCurPos / 1);
    }

    //UpdateRcMasterPos();
    RcCutPointDeal();       // ���е�Ĵ���
#endif
    
    if (runCmd.bit.freeStop)   // �����룬��������ͣ����ͨѶ����ͣ����ͣ����ʽΪ����ͣ�������ڻָ�����������
    {
        runStatus = RUN_STATUS_SHUT_DOWN;
    }
#if DEBUG_F_TUNE
    // ��гʱ������ͣ����Ч
    else if ((runCmd.bit.common) && (tuneCmd))
    {
        if (RUN_STATUS_WAIT == runStatus) // �ȴ�����
        {
            runStatus = RUN_STATUS_TUNE;
            saveTuneDataFlag = 0;           // ����
        }
    }
#endif
    else if (diFunc.f2.bit.brake)   // DI����ֱ���ƶ���Ч
    {                       // ֱ���ƶ�ʹ�ܶ��ӣ��Դ�û��Ӱ�졣
        runStatus = RUN_STATUS_DI_BRAKE;
    }
    else if (diFunc.f2.bit.decBrake)
    {
        if (runFlag.bit.run)
        {
            runStatus = RUN_STATUS_DI_BRAKE_DEC;
        }
        else
        {
            runStatus = RUN_STATUS_DI_BRAKE;
        }
    }
#if DEBUG_F_LOSE_LOAD
    // 320û�е��ر���
    else if (dspStatus.bit.outAirSwitchOff
             && runFlag.bit.run
             && funcCode.code.loseLoadProtectMode) // ���ر���
    {
        if (RUN_STATUS_LOSE_LOAD != runStatus)
        {
            runStatusOld4LoseLoad = runStatus;  // �������ʱ�̵�runStatus
        }
        
        runStatus = RUN_STATUS_LOSE_LOAD;
    }
#endif
#if DEBUG_F_JOG
    else if (runCmd.bit.jog) // �㶯�����������е㶯
    {
        if (RUN_STATUS_JOG != runStatus)
        {
            runStatusOld4Jog = runStatus;       // ���������е㶯ʱ�̵�����״̬
        }
            
        runStatus = RUN_STATUS_JOG;
        
        runFlag.bit.jog = 1;
    }
#endif
    else if (runCmd.bit.common)
    {
        if (RUN_STATUS_WAIT == runStatus) // �ȴ�����
        {
            runStatus = RUN_STATUS_START;
            startRunStatus = START_RUN_STATUS_INIT;
        }
    }
    
// �������з���,���з�ʽ(�㶯������ͨ����),��ԾƵ�ʣ������趨Ƶ��(Ŀ��Ƶ��frqAim).
// ����������.
    UpdateFrqAim();

// ˲ͣ��ͣ����
    PowerOffNoStopDeal();

#if DEBUG_F_ERROR_RUN_FRQ
    // ���й����в����������й��ϵĵ�һ�ļ��䵱ǰ����Ƶ��
    if (runFlag.bit.run && (ERROR_LEVEL_RUN == errorAttribute.bit.level) && (ERROR_LEVEL_RUN != errorLevelOld))
    {
        runOldFrqFlag = 1;
        frqOld = frq;        
    }
    
    if ((errorCode == ERROR_NONE)
        ||(ERROR_LEVEL_RUN != errorAttribute.bit.level)
        )
    {
        runOldFrqFlag = 0;
    }
    
    // ���ݹ��ϼ�����Ϣ
    errorLevelOld = errorAttribute.bit.level;
    
    // ���ڼ������еĹ���
    if ((ERROR_LEVEL_RUN == errorAttribute.bit.level)
        && (errorCode != ERROR_LOSE_LOAD)
        )
    {
        // ����ʱ��������Ƶ��ѡ��
        switch (funcCode.code.errorRunFrqSrc)
        {
            // ��ǰ����Ƶ��
            case ERR_RUN_FRQ_RUN:
                if (runOldFrqFlag)
                {
                    frqCurAim = frqOld;
                }
                else
                {
                    frqCurAim = frqAim; 
                }
                break;

            // �趨Ƶ��
            case ERR_RUN_FRQ_AIM:
                frqCurAim = frqAim; 
                break;

            // ����Ƶ��
            case ERR_RUN_FRQ_UPPER:
                frqCurAim = upperFrq;
                break;

            // ����Ƶ��
            case ERR_RUN_FRQ_LOWER:
                frqCurAim = lowerFrq;
                break;

            // ����Ƶ��
            case ERR_RUN_FRQ_SECOND:
				
                frqCurAim = ((Uint32)funcCode.code.errorSecondFrq * funcCode.code.maxFrq) / 1000;

				if (frqAim < 0)
				{
					frqCurAim = -frqCurAim;
				}
                break;

            default:
                break;
        }
    }
    else
#endif        
    {
        frqCurAim = frqAim;      // ֱ���ƶ�֮ǰ���͸����ܴ��� 
    }

    
#if (DEBUG_F_POSITION_CTRL)
    AccCalc(frqTmp);     // ����ٶ�
    //AccCalc(frqRun);
#endif

// runStatus����
    if (!shuntFlag)      // û�н���˲ͣ��ͣ���Ŵ���runStatus
    {
        switch (runStatus)
        {
            case RUN_STATUS_START:  // ����
                StartRunCtrl();
                break;

            case RUN_STATUS_JOG:    // �㶯����
                JogRunCtrl();
                break;

            case RUN_STATUS_TUNE:   // ��г����
                TuneRunCtrl();
                break;

            case RUN_STATUS_DI_BRAKE_DEC:  // �����ƶ�ǰ����
                // Ŀ��Ƶ��Ϊ0
                frqCurAim = 0;              
                AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
                if (!diFunc.f2.bit.decBrake)
                {
                    runStatus = RUN_STATUS_NORMAL;    
                }
                // С��ֱ���ƶ���ʼƵ��(����ֵ)
                else if (ABS_INT32(frqTmp) <= funcCode.code.stopBrakeFrq) 
                {
                    // �������ֱ���ƶ�
                    runStatus = RUN_STATUS_DI_BRAKE;          
                }
                break;
                
            case RUN_STATUS_DI_BRAKE:  // �ƶ�
                frqTmp = 0;
                //runCmd.bit.common0 = 1;
                runFlag.bit.run = 1;
                runFlag.bit.common = 1;
                dspMainCmd.bit.run = 1;
                dspMainCmd.bit.stopBrake = 1;
                if ((!diFunc.f2.bit.brake) // DI���ӵ�ֱ���ƶ������Ϊ��Ч֮�󣬹ض�
                    && (!diFunc.f2.bit.decBrake)
                    )
                {
                    // ����������ָ�Ϊ��������
                    if (runCmd.bit.common)
                    {
                        dspMainCmd.bit.stopBrake = 0;
                        runStatus = RUN_STATUS_NORMAL;
                    }
                    else
                    {
                        runCmd.bit.common0 = 0;
                        runStatus = RUN_STATUS_SHUT_DOWN;
                    }
                }
                break;

            case RUN_STATUS_LOSE_LOAD: // ����
                LoseLoadRunCtrl();
                break;
                
            case RUN_STATUS_WAIT:
                break;
                
            case RUN_STATUS_NORMAL:
                break;
                
            case RUN_STATUS_STOP:
                break;
                
            case RUN_STATUS_SHUT_DOWN:
                break;
                
#if DEBUG_F_POSITION_CTRL
            case RUN_STATUS_POS_CTRL:  // λ�ÿ���
                PcRunCtrl();
                break;
#endif
            default:
                break;
        }

        if (RUN_STATUS_NORMAL == runStatus) // normal run
        {
            NormalRunCtrl();
        }
    }

    if (RUN_STATUS_STOP == runStatus)       // ͣ��
    {
        StopRunCtrl();
    }

    if (RUN_STATUS_SHUT_DOWN == runStatus)  // shutdown, �ض�
    {
        ShutDownRunCtrl();
    }

#if 0
    // ������ʶ��PG������������Ϣ
    if (tunePGflag)
    {
        // Ϊ�ջ�����
        if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
        {
			// ����ʱ��ʾһ��
            if (tunePGflag == 1)
            {
                // δ�ӱ�����
                errorOther = ERROR_PG_LOST;
			   tunePGflag = 0;
            }
			// ��ʶ����ʾһ��
            else if(tunePGflag == 2)
            {
                // �����������趨���� 
                errorOther = ERROR_PG_PARA_ERROR;
			   tunePGflag = 0;
            }            
        }
        else
        {
            if(tunePGflag == 2)
            {
                tunePGflag = 0;
            }
        }
    }

#endif    
    
#if DEBUG_F_POSITION_CTRL
// ��������frq��ֵ
    if (RUN_STATUS_POS_CTRL == runStatus)
    {
        frq = frqPcOut;
        frqFrac = frqPcOutFrac;

        frqCurAim = frq;    // λ�ÿ���ʱ��Ŀ��Ƶ���뵱ǰƵ��һ��
        frqCurAimFrac = frqFrac;
        
#if 0   // λ�ÿ���ʱ����������Ƶ��+5Hz����
        if ((frq > (int32)upperFrq + 500) // �޷�����ֹ����
            || (frq < -(int32)upperFrq - 500)
            )
        {
            errorOther = 97;
        }
#endif
    }
    else
#endif
    {
        frq = frqTmp;
        frqFrac = frqTmpFrac;

        //pcRef = 0;      // ��λ�ÿ��ƣ�ǿ��Ϊ0
        //pcFdb = 0;
    }

#if 0
    if (RUN_STATUS_TUNE != runStatus)   // ��гʱ���ܴ�����
    {
        if (frq > upperFrq) // �޷�����ֹ����
        {
            frq = upperFrq;
            frqTmpFrac = 0;
        }
        else if (frq < -(int32)upperFrq)
        {
            frq = -(int32)upperFrq;
            frqTmpFrac = 0;
        }
    }
#endif    

// ͣ�����䣬���������ͣ��ҲҪ����
    if (STOP_REM_WAIT == stopRemFlag) // ����PLCĿǰͣ����plcStep����Ϊ0������Ҳ�ᵽ�����
    {
        if ((FUNCCODE_frqRemMode_STOP_NO == funcCode.code.frqRemMode)   // ͣ�������䣬��upDownFrq��0
            && (dormantStatus != DORMANT_NO_RESPOND)                    // δ��������״̬
            )
        {
            upDownFrq = 0;
            frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
        }
        stopRemFlag = STOP_REM_NONE;
    }
}



//=====================================================================
//
// ��������
//
//=====================================================================
extern Uint16 bFrqCurAimChg;
LOCALF void StartRunCtrl(void)
{
    runFlag.bit.run = 1;
    runFlag.bit.common = 1;
    dspMainCmd.bit.run = 1;

    bFrqCurAimChg = 1;  // ����

    if (!runCmd.bit.common)     // ����ʱ����ͣ������
    {
        if (START_RUN_STATUS_HOLD_START_FRQ == startRunStatus)
        {
            runStatus = RUN_STATUS_STOP;
            stopRunStatus = STOP_RUN_STATUS_INIT;
        }
        else
            runStatus = RUN_STATUS_SHUT_DOWN;

        runTimeTicker = 0;  // ticker����
        
        return;
    }
    
#if DEBUG_F_START_RUN
    if (bAntiReverseRun)    // ��ת��ֹ
    {
        runTimeTicker = 0;
        runStatus = RUN_STATUS_NORMAL;
        return;
    }

    switch (startRunStatus)
    {
        case START_RUN_STATUS_SPEED_TRACK:
            if (FUNCCODE_startMode_SPEED_TRACK == funcCode.code.startMode) // ת�ٸ�������
            {
                // Ϊͬ�������첽���ջ�ʸ��ʱֱ������
                if ((motorFc.motorPara.elem.motorType == MOTOR_TYPE_PMSM)
                    || (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
                    )
                {
                    startRunStatus = START_RUN_STATUS_BRAKE;   // ���ƶ�
                }
                else
                {
                    dspMainCmd1.bit.speedTrack = 1;

                    if (dspStatus.bit.speedTrackEnd)    // ת�ٸ������
                    {
                        dspMainCmd1.bit.speedTrack = 0;
                        frqAimOld4Dir = frqRun;         // ת�ٸ���ʱҲҪ��������ת����
                        runStatus = RUN_STATUS_NORMAL;
                    }
                    frqTmp = frqRun;                // ���µ�ǰ���ٵ�Ƶ��
                    break;
                } 
            }
            else  if (FUNCCODE_startMode_DIRECT_START == funcCode.code.startMode)
            {
                startRunStatus = START_RUN_STATUS_BRAKE;   // ���ƶ�
            }
            else   if (FUNCCODE_startMode_FORE_MAG == funcCode.code.startMode)
            {
                //  VF����ģʽ
                if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)
                {
                    startRunStatus = START_RUN_STATUS_BRAKE;   // ���ƶ�
                }
                else
                {
                    startRunStatus = START_RUN_STATUS_PRE_FLUX;  // Ԥ��������
                }
            }
            
        // ���ƶ�
        case START_RUN_STATUS_BRAKE:
            if(startRunStatus == START_RUN_STATUS_BRAKE)
            {
                if ((++runTimeTicker >= (Uint32)funcCode.code.startBrakeTime
                     * (Uint16)(TIME_UNIT_START_BRAKE / RUN_CTRL_PERIOD))
                    )
                {    
                    runTimeTicker = 0;
                    dspMainCmd.bit.startBrake = 0;  // ���ƶ�����
                    startRunStatus = START_RUN_STATUS_HOLD_START_FRQ;
                }
                else
                {
                    dspMainCmd.bit.startBrake = 1;  // ���ƶ�����
                    break;
                }
            }
            
        // Ԥ��������
        case START_RUN_STATUS_PRE_FLUX:
            if(startRunStatus == START_RUN_STATUS_PRE_FLUX)
            {
                if ((++runTimeTicker >= (Uint32)funcCode.code.startBrakeTime
                     * (Uint16)(TIME_UNIT_START_BRAKE / RUN_CTRL_PERIOD))
    				 )
                {
                    runTimeTicker = 0;
                    dspMainCmd.bit.startFlux = 0;  // ��Ԥ��������
                    startRunStatus = START_RUN_STATUS_HOLD_START_FRQ;
                }
                else
                {

                    dspMainCmd.bit.startFlux = 1;  // ��Ԥ��������
                    break;
                }
            }
            
        // ��Ƶ�ʱ���    
        case START_RUN_STATUS_HOLD_START_FRQ:
            if (bRunJog)   // ��ͬʱ�����к͵㶯����������е㶯ʹ�ܣ��˳�����Ƶ��
                break;
            
            // ע�⣬����Ƶ�ʱ���ʱ��ʱ��Ϊ0���������Ƶ�ʿ�ʼnormal run
            frqTmp = (FORWARD_DIR == runFlag.bit.dirFinal) ? (funcCode.code.startFrq) : (-(int32)funcCode.code.startFrq);
            if (++runTimeTicker >= (Uint32)funcCode.code.startFrqTime
                 * (Uint16)(TIME_UNIT_START_FRQ_WAIT / RUN_CTRL_PERIOD))
            {
                // FVC�������ӵ�ǰƵ�ʿ�ʼ
                if (FUNCCODE_motorCtrlMode_FVC == motorFc.motorCtrlMode)
                {
                    // �ӵ�ǰ��������ٶȿ�ʼ����
                    frqTmp = frqFdbTmp;  // frqRun;
                }
                
                runTimeTicker = 0;
                runStatus = RUN_STATUS_NORMAL;
            }
            else
            {
                frqCurAim = frqTmp;  // ����Ŀ��Ƶ��
                frqCurAimOld = frqCurAim;
            }

            break;

        default:
            break;
    }
#elif 1

    runTimeTicker = 0;
    runStatus = RUN_STATUS_NORMAL;

#endif
}


//=====================================================================
//
// normal���п���
//
//=====================================================================
LOCALF void NormalRunCtrl(void)
{
    if (!runCmd.bit.common) // ��������ͣ������
    {
        //+e if(function.f_code.torque_control&&(!(di_func[3]&0x20)))
        //     frq_set=(int)speed_run*(long)(int)(function.f_code.maxfrq+2000)/0x7fff;
        if (runMode == RUN_MODE_TORQUE_CTRL)
        {
            frq = frqRun;
        }
        
        swingStatus = SWING_NONE;    // ��Ƶ״̬��Ϊ�ް�Ƶ״̬
        runStatus = RUN_STATUS_STOP;
        stopRunStatus = STOP_RUN_STATUS_INIT;
        return;
    }

// �Ƿ�����ת
    if (runFlag.bit.dirReversing) // ���ڷ���
    {
        frqCurAim = 0;
    }
    else
    {
//+==        frqCurAim = frqAim;

#if DEBUG_F_LOWER_FRQ
        // �ж��Ƿ��������Ƶ��
        LowerThanLowerFrqDeal();
#endif

#if DEBUG_F_SWING
// �ж��Ƿ��Ƶ
        if (((frq == frqAim) // �ﵽ�趨Ƶ�ʣ�����SwingDeal()�ж��Ƿ���Ҫ��Ƶ
             || (SWING_NONE != swingStatus))      // �Ѿ������Ƶ
            && (!bAntiReverseRun)                 // û�д��ڷ�ת��ֹ״̬
            && (runMode != RUN_MODE_TORQUE_CTRL)  // ��Ϊת�ؿ���
            )
        {
            SwingDeal();// ��Ƶ
        }
#endif
    }

// �Ӽ���
    if (!diFunc.f1.bit.forbidAccDecSpd)
    {
#if DEBUG_F_SWING
        if (SWING_NONE != swingStatus) // ���ڰ�Ƶ
        {
            AccDecFrqCalc(swingAccTime, swingDecTime, ACC_DEC_LINE);
        }
        else
#endif
        {
            AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
        }
    }
}


//=====================================================================
//
// ͣ������
//
//=====================================================================
LOCALF void StopRunCtrl(void)
{
    int32 tmp;
    Uint16 enableStopBrake = 1;

    if (runCmd.bit.common)     // ͣ��ʱ������������
    {
        if (STOP_RUN_STATUS_WAIT_BRAKE == stopRunStatus)
        {
            //dspMainCmd.bit.run = 1; ����Ҫ
            runStatus = RUN_STATUS_START; //+= ����ͣ��ֱ���ƶ��ȴ�ʱ���ǹض�PWM���������¿�ʼ��������
            startRunStatus = START_RUN_STATUS_INIT;
        }
        else
        {
            dspMainCmd.bit.stopBrake = 0;
            runStatus = RUN_STATUS_NORMAL;
        }

        runTimeTicker = 0;  // ticker����

        return;
    }
    
#if DEBUG_F_STOP_RUN
    switch (stopRunStatus)
    {
        case STOP_RUN_STATUS_DEC_STOP:  // ����ͣ��
            frqCurAim = 0;              // Ŀ��Ƶ��Ϊ0
            AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);

            if ((!funcCode.code.ovGain)    // ��ѹʧ�٣�����ʧ�����涼Ϊ0
                && (!funcCode.code.ocGain) // ��ΪVF����
                && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)
                ) 
            {
                tmp = frqTmp;
            }
            else    // ��ѹʧ�٣�����ʧ��
            {
                tmp = frqRun;
            }
            
            if (((ABS_INT32(tmp) <= funcCode.code.stopBrakeFrq) // С��ֱ���ƶ���ʼƵ��(����ֵ)
			    || (ABS_INT32(tmp) < 50))
//                && funcCode.code.stopBrakeFrq                        // ��ͣ��ֱ���ƶ���ʼƵ�ʲ�Ϊ0
// ͣ��ֱ���ƶ���ʼƵ�ʿ���Ϊ0������λ���Ը��أ�������Ҫ��ͣ��ʱ����
//                && funcCode.code.stopBrakeCurrent                    // ͣ��ֱ���ƶ�������Ϊ0
// ��֮ǰ����һ��
//                && funcCode.code.stopBrakeTime                       // ��ͣ��ֱ���ƶ�ʱ�䲻Ϊ0
                && enableStopBrake)                    // DI���ӵ�ͣ��ֱ���ƶ�ʹ�ܶ�����Ч(Ĭ����Ч)
            {
                stopRunStatus = STOP_RUN_STATUS_WAIT_BRAKE;          // ����ֱ���ƶ��ȴ�
                // ֱ�ӽ���ֱ���ƶ��ȴ������õ���һ�����ڲŽ���
            }
            else
            {
                if ((!frqTmp)    // ���ܸ���Ƶ��Ϊ0�������ܸ�����ʵ��ת��С��1Hz���ض�
                    && (ABS_INT32(tmp) < 50)  // ���ܷ���С��_
                    )
                    runStatus = RUN_STATUS_SHUT_DOWN;
                
                break;
            }
            //lint -fallthrough

        case STOP_RUN_STATUS_WAIT_BRAKE:    // ͣ��ֱ���ƶ��ȴ�
            frqTmp = 0;
            if ((!enableStopBrake) // ͣ��ֱ���ƶ�ʹ�ܶ�����Ч�������˳�
                || (++runTimeTicker >= (Uint32)funcCode.code.stopBrakeWaitTime  // ͣ��ֱ���ƶ��ȴ�ʱ��
                    * (Uint16)(TIME_UNIT_WAIT_STOP_BRAKE / RUN_CTRL_PERIOD)))
            {
                runTimeTicker = 0;
                dspMainCmd.bit.run = 1;
                dspSubCmd.bit.fanRunWhenWaitStopBrake = 0;
                stopRunStatus = STOP_RUN_STATUS_BRAKE;
            }
            else
            {
                dspMainCmd.bit.run = 0;     // ����PWM
                dspSubCmd.bit.fanRunWhenWaitStopBrake = 1;  // ͣ��ֱ���ƶ��ȴ�ʱ���ڷ������б�־
                break;
            }
            //lint -fallthrough
            
        case STOP_RUN_STATUS_BRAKE:     // ֱ���ƶ�
            if ((!enableStopBrake) // ͣ��ֱ���ƶ�ʹ�ܶ�����Ч�������˳�
                || (++runTimeTicker >= (Uint32)funcCode.code.stopBrakeTime // ͣ��ֱ���ƶ�ʱ��
                    * (Uint16)(TIME_UNIT_STOP_BRAKE / RUN_CTRL_PERIOD)))
            {
                //runTimeTicker = 0;
                runStatus = RUN_STATUS_SHUT_DOWN;
            }
            else
            {
                dspMainCmd.bit.stopBrake = 1;
            }
            
            break;

        default:
            break;
    }
#elif 1
    frqCurAim = 0;              // Ŀ��Ƶ��Ϊ0
    AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
    if ((!frqTmp)    // ���ܸ���Ƶ��Ϊ0�������ܸ�����ʵ��ת��С��1Hz���ض�
            && (ABS_INT32(frqRun) < 50)  // ���ܷ���С��_
        )
        runStatus = RUN_STATUS_SHUT_DOWN;
#endif
}


//=====================================================================
//
// �㶯���п���
//
//=====================================================================
LOCALF void JogRunCtrl(void)
{
#if DEBUG_F_JOG
    runFlag.bit.run = 1;
    runFlag.bit.jog = 1;

    dspMainCmd.bit.run = 1;

    if (runFlag.bit.dirReversing) // �㶯����ķ�������ת����ʱ��Ҳ��Ч�����ڷ���
    {
        frqCurAim = 0;
    }
#if 0//+==
    else
    {
        frqCurAim = frqAim;
    }
#endif

    if (!runCmd.bit.jog)
    {
        if (runFlag.bit.jogWhenRun) // �����е㶯���ָ����㶯ǰ������״̬����runFlag.bit.common�޹�.
        {
            runFlag.bit.jog = 0;
            runFlag.bit.jogWhenRun = 0;

            runStatus = runStatusOld4Jog;

            return;             // ��ʱ����ʹ�õ㶯�Ӽ���ʱ�䡣(���ǵ㶯�Ӽ���ʱ��Ϊ0)
        }
        else
        {
            if (!frqTmp)    // �㶯����ȡ��������0Ƶ�ʣ��Źض�PWM
            {
                runStatus = RUN_STATUS_SHUT_DOWN;
            }
            
            frqCurAim = 0;      // ȡ���㶯����㶯ͣ��
        }
    }

    if (!diFunc.f1.bit.forbidAccDecSpd) // �㶯ʱ���Ӽ��ٽ�ֹҲҪ������
    {
        AccDecFrqCalc(funcCode.code.jogAccTime, funcCode.code.jogDecTime, ACC_DEC_LINE);
    }
#endif
}


//=====================================================================
//
// ��г����
// 
// ��г�����У�ͣ��
// 1. ����ͣ�����ҹ��������õ�ͣ��ֱ���ƶ��������á�
// 2. ��������Ϊ����ͣ����������ͣ��
// 3. ��ͨ����������ͣ����������ͣ��
// 4. ����г�������й��ϣ�������ͣ��
//
//=====================================================================
#define TUNE_STEP_WAIT      0           // ���ڱ�ʶ������û���κ�ָ��
#define TUNE_STEP_ACC       50          // ��ʶ�����м���
#define TUNE_STEP_DEC       51          // ��ʶ�����м���
#define TUNE_STEP_END       100         // ��ʶ������������������ͣ����
#define PMSM_TUNE_SPEED     10UL        // ͬ�������ص�г�ٶ�
Uint16 motorCtrlTuneStatus;             // ������ʶ״̬�֣����ܴ���
LOCALF void TuneRunCtrl(void)
{
#if DEBUG_F_TUNE
    Uint16 motorCtrlTuneStatusTmp = motorCtrlTuneStatus;
    static Uint16 stopFlag = 0;

#if 0
    // motorCtrlTuneStatus�ڵ�г֮��Ÿ���
    if (runFlag.bit.tune)
    {
        motorCtrlTuneStatusTmp = motorCtrlTuneStatus;
    }
#endif

    runFlag.bit.run = 1;
    runFlag.bit.tune = 1;

    dspMainCmd.bit.run = 1;

    if (++runTimeTicker >= TUNE_RUN_OVER_TIME_MAX * (Uint16)(TIME_UNIT_MS_PER_SEC / RUN_CTRL_PERIOD)) // ��ʱ
    {
        errorOther = ERROR_TUNE;
        return;
    }

    if (saveTuneDataFlag)       // �Ѿ�����������������ͣ��״̬
    {
        motorCtrlTuneStatusTmp = TUNE_STEP_DEC;
    }

    if (!runCmd.bit.common)     // ��г����ȡ��(stop)������ͣ���˳���г��
    {
        stopFlag = 1;
    }
    // ȡ����г
    if (stopFlag)
    {
        motorCtrlTuneStatusTmp = TUNE_STEP_DEC;
    }
	
    switch (motorCtrlTuneStatusTmp)
    {
        case TUNE_STEP_WAIT:
            break;

        case TUNE_STEP_ACC:
            if (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType) 
            {
                // ͬ��������ص�г���ת��Ϊ10RPM
                if (motorFc.motorPara.elem.ratingSpeed > PMSM_TUNE_SPEED)
                {
                    frqCurAim = PMSM_TUNE_SPEED * motorFc.motorPara.elem.ratingFrq / motorFc.motorPara.elem.ratingSpeed;  // ͬ�������10RPMת������
                }
                else
                {
                    frqCurAim = motorFc.motorPara.elem.ratingFrq;
                }
            }
            else
            {
                frqCurAim = (Uint32)motorFc.motorPara.elem.ratingFrq * 4 / 5; // ����Ƶ�ʵ�80%
            }

            // ͬ���������,�첽������Ҫ
            if ((FUNCCODE_runDir_REVERSE == funcCode.code.runDir)
                && (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType))
            {
                frqCurAim = -frqCurAim;
            }
            
            AccDecFrqCalc(accFrqTime, decFrqTime, FUNCCODE_accDecSpdCurve_LINE);
            break;

        case TUNE_STEP_END:
            saveTuneDataFlag = 1;           // ��ʾ�Ѿ�����
            //tunePGflag = PGErrorFlag;
            SaveTuneData();                 // ֻ�ܱ���һ��

        case TUNE_STEP_DEC:
            frqCurAim = 0;
            AccDecFrqCalc(accFrqTime, decFrqTime, FUNCCODE_accDecSpdCurve_LINE);
            if (ABS_INT32(frqTmp) < 50)   // �����ٶ��Ѿ��ܵ�
            {
                if (TUNE_STEP_DEC != motorCtrlTuneStatus)   // ���ܴ��ݼ��������ͣ����
                {
                    runCmd.bit.common0 = 0;     // ��Ҫ���㡣֮ǰ��г�汾��û��ʹ�ø�bit
                    runStatus = RUN_STATUS_SHUT_DOWN;
                    stopFlag = 0;
                }
            }
            break;
            
        default:
            break;
    }
#endif
}


//=====================================================================
// 
// �������п���
//
//=====================================================================
LOCALF void LoseLoadRunCtrl(void)
{
#if DEBUG_F_LOSE_LOAD

    if ((!runCmd.bit.common) && (!runCmd.bit.jog)) // ��ͣ������
    {
        runStatus = RUN_STATUS_STOP;
        stopRunStatus = STOP_RUN_STATUS_INIT;
    }
    // �Ѿ�������ر�������������û�е��ر�־ʱ���Ƴ����ر�����
    // �磬�ڵ��ر���ʱ���޸ĵ��ر���������Ч�������˳����ر���
    else  if (!dspStatus.bit.outAirSwitchOff)
    {
        runStatus = runStatusOld4LoseLoad;
    }
    else
    {
        int32 tmp;

        if (GetErrorAttribute(ERROR_LOSE_LOAD) == ERROR_LEVEL_RUN)
        {
            tmp = ((int32)motorFc.motorPara.elem.ratingFrq * 7) / 100; // ����������7%��Ӧ�ø������ܵ���ֵ

            if (ABS_INT32(frqAim) > tmp)    // �趨Ƶ�ʴ�����ֵ
            {
                frqTmp = (FORWARD_DIR == runFlag.bit.dirFinal) ? (tmp) : (-tmp);
            }
            else                            // ����Ϊ�趨Ƶ��
            {
                frqTmp = frqAim;
            }
        }
    }
#endif
}


//=====================================================================
// 
// �ض�PWM����
//
//=====================================================================
LOCALF void ShutDownRunCtrl(void)
{
    dspMainCmd.bit.run = 0;         // dspMainCmd.all &= 0xf330;
    dspMainCmd1.bit.speedTrack = 0;
    dspMainCmd.bit.stopBrake = 0;
    dspMainCmd.bit.startBrake = 0;
    dspMainCmd.bit.startFlux = 0;
    dspMainCmd.bit.accDecStatus = 0;
    tuneCmd = 0;
    
//+==    if (!dspStatus.bit.run) // ���������Ĺض���PWM���ñ�־���Բ�ʹ�á�
    {
        Uint16 pause = runCmd.bit.pause;

        if (runFlag.bit.common)    // common����ͣ������������ͣ��
        {
            if (!pause)                                 // ��������ͣ�����ͣ��
            {
                stopRemFlag = STOP_REM_WAIT;            // ͣ����־Ϊ1
            }
            else
            {
                stopRemFlag = STOP_REM_PAUSE_JUDGE;     // ����֮��������ͣ�ˡ�
            }
        }

        if (runFlag.bit.common      // common����ͣ��
            && (!errorCode)         // �ǹ���ͣ��
//            && (!runFlag.bit.jog) �ǵ㶯
            )
        {
            // ����ͣ��ͣ��������
            if ((!pause) && (FUNCCODE_plcStopRemMode_REM != (funcCode.code.plcPowerOffRemMode/10)))     // ��������ͣ�����㡣Ŀǰû��ͣ�����䡣
            {
                plcStep = 0;
                plcTime = 0;
            }
            else
            {
                plcStepRemOld = plcStep;    // ������ͣ������plcStep��plcTime
                plcTimeRemOld = plcTime;
            }
        }

        runTimeTicker = 0;      // ticker����
        lowerDelayTicker = 0;
        shuntTicker = 0;

        runFlag.bit.run = 0;    // ֻ��һ���ֱ�־
        runFlag.bit.common = 0;
        runFlag.bit.jog = 0;
        runFlag.bit.tune = 0;
        runFlag.bit.jogWhenRun = 0;
        runFlag.bit.accDecStatus = 0;
        runFlag.bit.servo = 0;
        
        frqTmp = 0;                 // ����Ƶ������
        frqTmpFrac = 0;             // С��ҲҪ����
        swingStatus = SWING_NONE;   // ��Ƶ��ʼ��Ϊ�ް�Ƶ״̬

        frqCurAimOld = 0;           // S����ʹ��
        accTimeOld = 0;             // S����ʹ��
        decTimeOld = 0;             // S����ʹ��
        frqLine.remainder = 0;      // ����

        runStatus = RUN_STATUS_WAIT; // ͣ����ɣ��ȴ��ٴ�����
    }
}


//=====================================================================
//
// ���°�Ƶ����
// 
// ���°�Ƶ��:
// ����ֵ������ֵ(swingMaxFrq,swingMinFrq)
// ��Ӧ�ļӼ���ʱ��(swingAccTime,swingDecTime),��0�����Ƶ�ʵļӼ���ʱ��
// ��Ƶ����Ƶ����(swingJumpFrq)
// 
//=====================================================================
LOCALF void UpdateSwingPara(void)
{
#if DEBUG_F_SWING
    int32 swingAwBase;      // �ڷ��Ļ�׼Ƶ�ʣ��ڷ��Ļ�׼��
    int32 swingAw;
    Uint32 tmp;
    
	swingFrqLimit = 0;
	
    if (FUNCCODE_swingBaseMode_AGAIN_MAXFRQ == funcCode.code.swingBaseMode)
    {
        swingAwBase = maxFrq;               // ��ֵ
    }
    else
    {
        swingAwBase = ABS_INT32(frqAim); // ��ֵ
    }

    swingAw = (swingAwBase * funcCode.code.swingAmplitude) / 1000;  // ��ֵ

    swingMaxFrq = ABS_INT32(frqAim) + swingAw;   // ��Ƶ����Ƶ�ʣ���ֵ
    if (swingMaxFrq > upperFrq)     // ����Ƶ������
    {
    	swingFrqLimit = 1;
        swingMaxFrq = upperFrq;     // ��ֵ
    }

    swingMinFrq = ABS_INT32(frqAim) - swingAw;   // ��Ƶ����Ƶ�ʣ���ֵ
    if (swingMinFrq < lowerFrq)     // ����Ƶ�����ơ�����������Ƶ�ʷ���ͬ
    {
    	swingFrqLimit = 1;
        swingMinFrq = lowerFrq;     // ��ֵ
    }

    swingJumpFrq = (swingAw * funcCode.code.swingJumpRange) / 1000; // Ҫ�з��ţ���������Ƶ��Ϊ��ʱ��ͻ��Ƶ���д���

    if (REVERSE_DIR == runFlag.bit.dirFinal)            // ����Ƶ��Ϊ������
    {
        swingMaxFrq = -swingMaxFrq; // ��ֵ
        swingMinFrq = -swingMinFrq;
        swingJumpFrq = -swingJumpFrq;
    }

    swingDoubleAw = swingAw << 1;
//    swingDoubleAw = ABS_INT32(swingMaxFrq - swingMinFrq); Ŀǰ�������������: swingMaxFrq,swingMinFrq�ﵽ�����޵����

    tmp = ((Uint32)maxFrq * funcCode.code.swingPeriod)
        / (swingDoubleAw - swingJumpFrq) * TIME_UNIT_SWING_PERIOD / timeBench;
    swingAccTime = (tmp * funcCode.code.swingRiseTimeCoeff) / 1000;
    swingDecTime = tmp - swingAccTime;
#endif
}


//=====================================================================
//
// ��Ƶ�������£�
// ��Ƶ״̬��
// 
//=====================================================================
LOCALF void SwingDeal(void)
{
#if DEBUG_F_SWING
    static int32 frqAimOld;
    static Uint32 frqDeltaOldMax; // ��frqAim�ı���� > �ս����ƵʱswingDoubleAw��1/2^5�����˳���Ƶ

    UpdateSwingPara();              // ���°�Ƶ����

    if (funcCode.code.swingPeriod       // ��Ƶ���ڲ�Ϊ0
        && funcCode.code.swingAmplitude // ��Ƶ���Ȳ�Ϊ0
        && (!diFunc.f1.bit.swingPause)  // DI���ӵİ�Ƶ��ͣ��Ч
        )
    {
        if (SWING_NONE == swingStatus)       // �ոս����Ƶ������Ϊ�����׶�
        {
            frqAimOld = frqAim;   // �ս����Ƶ������frqAimOld
            frqDeltaOldMax = swingDoubleAw >> 5;
            //frqDeltaOldMax = 0; //+= ��Ƶ���˳��ͽ��룬�������Ż�

            swingStatus = SWING_UP;
        }
    }
    else
    {
        swingStatus = SWING_NONE; // �˳���Ƶ
        return;
    }
    
    if (ABS_INT32(frqAimOld - frqAim) > frqDeltaOldMax) // Ŀ��Ƶ�ʷ����ϴ�ı䣬�˳���Ƶ     
    {
        swingStatus = SWING_NONE; // �˳���Ƶ
        return;
    }
    
#if 0   // frqAim = 0ʱ������Ƶ������������Ƶ�ʣ�ҲҪ���а�Ƶ
    if (!frqAim)
    {
        frqCurAim = 0;
    }
    else
#endif
    if (SWING_UP == swingStatus)    // ��Ƶ��������
    {
        frqCurAim = swingMaxFrq;

        if (frqTmp == swingMaxFrq)
        {
            swingStatus = SWING_DOWN;
//            frqCurAim = swingMinFrq;

            frqTmp -= swingJumpFrq;
        }
    }

    if (SWING_DOWN == swingStatus)    // ��Ƶ���½���
    {
        frqCurAim = swingMinFrq;
        
        if (frqTmp == swingMinFrq)
        {
            swingStatus = SWING_UP;
            frqCurAim = swingMaxFrq;    // Ҫͬʱ���£���Ϊ�ڱ�����֮��Ż����AccDecFrqCalc()������
            
            frqTmp += swingJumpFrq;
        }
    }
#endif
}


//=====================================================================
//
// ���й����е�������Ƶ�ʴ���
//
//
//=====================================================================
LOCALF void LowerThanLowerFrqDeal(void)
{
#if DEBUG_F_LOWER_FRQ
    if (ABS_INT32(frqAim) < lowerFrq)
    {
        // ��������Ƶ����ʱͣ�������У�Ŀ��Ƶ��ҲӦ��������Ƶ��
        if (!bAntiReverseRun)   // û�д��ڷ�ת��ֹ
        {
            if (FUNCCODE_lowerDeal_RUN_ZERO == funcCode.code.lowerDeal)    // ��������Ƶ������������
            {
                frqCurAim = 0;
            }
			// ��ʱͣ����������Ƶ������
			else
			{
                frqCurAim = (FORWARD_DIR == runFlag.bit.dirFinal) ? (lowerFrq) : (-(int32)lowerFrq);
            }
			
        }

        if (FUNCCODE_lowerDeal_DELAY_STOP == funcCode.code.lowerDeal)
        {
            // ���ڼ��ٹ����У���û�дﵽ����Ƶ�ʣ������趨Ƶ�ʵ�������Ƶ�ʣ�����ʱֱ��ͣ��
            if (ABS_INT32(frq) < lowerFrq)
            {
                lowerDelayTicker = 0;
                otherStopLowerSrc++;     //+==

                runStatus = RUN_STATUS_STOP;
                stopRunStatus = STOP_RUN_STATUS_INIT;
            }
            // ���򣬴�����Ƶ�ʴﵽ����Ƶ�ʿ�ʼ��ʱ����ʱͣ��
            else if (ABS_INT32(frq) == lowerFrq)
            {
                {
                    lowerDelayTicker = 0;
                    otherStopLowerSrc++;//+==

                    runStatus = RUN_STATUS_STOP;
                    stopRunStatus = STOP_RUN_STATUS_INIT;
                }
            }
        }
        else if (FUNCCODE_lowerDeal_RUN_LOWER == funcCode.code.lowerDeal)
        {
            lowerDelayTicker = 0;
        }
        else
        {
            ;
        }
    }
    else
    {
        lowerDelayTicker = 0;
    }
#endif
}



//=====================================================================
//
// ˲ͣ��ͣ
//
// 0 - û��ʹ�ܣ������Ѿ����
// 1 - ����
//
//=====================================================================

#define OFF_THRESHOLD_STOP_DEC_VOL_RANGE  50   // ˲ͣ���� ֹͣ���ٵ�ѹ��ֵ
LOCALF void PowerOffNoStopDeal(void)
{
#if DEBUG_F_POWER_OFF_NO_STOP

    Uint16 ratingUdc;           // 220V/380Vʱ�ı�׼(�)ĸ�ߵ�ѹ

    if ((!funcCode.code.pOffTransitoryNoStop)    // ˲ͣ��ͣ������ѡ��
        || ((!runCmd.bit.common) && (!runCmd.bit.jog))) // ������/�㶯���������˲ͣ��ͣ
    {
        shuntTicker = 0;
        shuntFlag = 0;

        return;
    }

    runFlag.bit.run = 1;

    ratingUdc = (Uint32)invPara.ratingVoltage * 14482 >> 10;    // ��׼ĸ�ߵ�ѹ

    // ĸ�ߵ�ѹ������ֵ��ѹ
    // ˲ʱͣ�磬��ĸ�ߵ�ѹ�½���_ʱ����ʼ�����Լ���ĸ�ߵ�ѹ���½���
#if DSP_2803X 
    if (generatrixVoltage < (Uint32)funcCode.code.pOffThresholdVol * ratingUdc * 131  >> 17)
#else
    if (generatrixVoltage < (Uint32)funcCode.code.pOffThresholdVol * ratingUdc / 1000)
#endif
    {
        if (!shuntFlag)
        {
            runStatusOld4POffNoStop = runStatus;
        }
        shuntFlag = 1;
        shuntTicker = 0;
    }
    else    // ĸ�ߵ�ѹ������ֵ��ѹ
    {
        if (shuntFlag)          // ����˲ͣ��ͣ�ĵ�ѹ�����׶�
        {
            if (++shuntTicker >= (Uint32)funcCode.code.pOffVolBackTime
                * (Uint16)(TIME_UNIT_P_OFF_VOL_BACK / RUN_CTRL_PERIOD)) // ��ѹ�����ҳ�������_ʱ�䣬��Ϊ˲ͣ���ѹ�Ѿ��ָ�����
            {
                shuntTicker = 0;
                shuntFlag = 0;
                runStatus = runStatusOld4POffNoStop;    // �ָ�Ϊ˲ͣ��ͣǰ��״̬
            }
        }
        else
        {
            shuntTicker = 0;
        }
    }

    if (shuntFlag)          // ����˲ͣ��ͣ�������Լ���ĸ�ߵ�ѹ���½�
    {                       // �ڵ�ѹ�����ж�ʱ���ڣ�ҲҪ�����Լ���ĸ�ߵ�ѹ���½�
        frqCurAim = 0;      // ���ٵ�Ŀ��Ƶ��Ϊ0

        // ĸ�ߵ�ѹ���� ˲ͣ�жϵ�ѹ+0.5*��׼ĸ�ߵ�ѹʱ��ֹͣ����
        // ��ֹ���ٹ�ѹ
        if (generatrixVoltage < ((Uint32)funcCode.code.pOffTransitoryFrqDecSlope * ratingUdc / 1000))
        {
            AccDecFrqCalc(accFrqTime, funcCode.code.decTime4, funcCode.code.accDecSpdCurve);
        }
        
        // ���ٵ�0ʱͣ��
        if ((0 == frqRun)
            && (funcCode.code.pOffTransitoryNoStop == 2)  // ͣ��
			)
        {
            otherStopSrc++;
        }
    }

#endif
}


//=====================================================================
// 
// �Ӽ���ʱ����㣬��λ0.01s
// 
//=====================================================================
void AccDecTimeCalc(void)
{
    Uint16 accDecTimeSrc;       // �Ӽ���ʱ��ѡ��
	static Uint16 stop4DecFlag;
    
#if DEBUG_F_ACC_DEC_TIME
    if (RUN_MODE_TORQUE_CTRL == runMode)
    {
        accFrqTime = 0;  // ת�ؿ��Ƽ���ʱ��Ϊ0
        decFrqTime = 0;  // ת�ؿ��Ƽ���ʱ��Ϊ0
    }
    else
    {
        if ((runFlag.bit.plc)       // PLC���У���DI����û��ѡ��ѡ����ʱ��ѡ����
            && (!diSelectFunc.f1.bit.accDecTimeSrc)
            )
        {
            accDecTimeSrc = accDecTimeSrcPlc;
        }
        else
        {
            accDecTimeSrc = diFunc.f1.bit.accDecTimeSrc;
    		
    		// �Ӽ���ʱ��1/2�л�
            // ����δѡ��Ӽ���ʱ��
            if (!diSelectFunc.f1.bit.accDecTimeSrc)
            {
    			// ����
    			if (runFlag.bit.accDecStatus == ACC_SPEED)
    			{
    				if (ABS_INT32(frq) >= funcCode.code.accTimefrqChgValue)
    				{
    					accDecTimeSrc = 0;
    				}
    				else
    				{
    					accDecTimeSrc = 1;
    				}
    			}
    			// ����
    			else if (runFlag.bit.accDecStatus == DEC_SPEED)
    			{
    				if (ABS_INT32(frq) >= funcCode.code.decTimefrqChgValue)
    				{
    					accDecTimeSrc = 0;
    				}
    				else
    				{
    					accDecTimeSrc = 1;
    				}
    			}
    			// ����
    			else
    			{
    				accDecTimeSrc = 0;
    			}
            }
            
        }

        if ((MOTOR_SN_1 != motorSn) &&      // �ǵ�1���
            (0 != motorFc.accDecTimeMotor))
        {
            accDecTimeSrc = motorFc.accDecTimeMotor - 1;
        }

    // DI�ļӼ���ʱ��ѡ����Ӷ�PLCҲ������
    // PLC���С�PLC�Ӽ���ʱ���й��������ѡ����FrqPlcSetDeal()���м���
        if (3 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime4;
            decFrqTime = funcCode.code.decTime4;
        }
        else if (2 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime3;
            decFrqTime = funcCode.code.decTime3;
        }
        else if (1 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime2;
            decFrqTime = funcCode.code.decTime2;
        }
        else //if (0 == accDecTimeSrc)
#endif
        {
            accFrqTime = funcCode.code.accTime1;
            decFrqTime = funcCode.code.decTime1;
        }
    }
    // ���¼Ӽ���ʱ���׼


    // �ⲿ����ͣ��ʱ�Լ���ʱ��4Ϊ����ʱ��ͣ��(�ñ�־)
    if (diFunc.f2.bit.stop4dec)
    {
        stop4DecFlag = 1;
    }

    // ͣ�������ͣ��ʱ����Ч��־����
    if (!runFlag.bit.run)
    {
        stop4DecFlag = 0;
    }

    // ����ͣ��ʱ��
    if (stop4DecFlag)
    {
         decFrqTime = funcCode.code.decTime4;
    }


    // ����ͣ��ʱ����ʱ��Ϊ0
    if (diFunc.f2.bit.emergencyStop)
    {
        decFrqTime = 0;
    }
    
    UpdateBenchTime();
}

//=====================================================================
// 
// ���ߡ����Ѵ���
// 
//=====================================================================
void DormantDeal(void)
{
#if DEBUG_F_DORMANT_DEAL    
    int32 tmp = ABS_INT32(frqAimTmp);
    static Uint32 ticker;

    if (!runCmd.bit.common0)                        // ����������
    {
        ticker = 0;
        dormantStatus = DORMANT_RESPOND;            // �˳�����״̬
    }

    if (!runFlag.bit.run)                           // ��ǰͣ��
    {
        if (DORMANT_NO_RESPOND != dormantStatus)    // ��ǰ����������״̬
        {
            if (tmp >= funcCode.code.dormantFrq)    // ����ʱ��������Ƶ��(���ø��ڻ���Ƶ��)��������Ӧ
            {
                dormantStatus = DORMANT_RESPOND;
            }
            else                                    // ��������Ƶ��
            {
                dormantStatus = DORMANT_2;
            }
        }
        else                                        // ��ǰ��������״̬��Ҫ���ڻ���Ƶ�ʲ���Ӧ
        {
            if (tmp >= funcCode.code.wakeUpFrq)     // ���ڻ���Ƶ��
            {
                if (++ticker >= (Uint32)funcCode.code.wakeUpTime * TIME_UNIT_WAKE_UP / RUN_CTRL_PERIOD)
                {
                    dormantStatus = DORMANT_RESPOND;// �˳�����״̬

                    ticker = 0;
                }
            }
            else
            {
                ticker = 0;
            }
        }
    }
    else if ((runFlag.bit.common) && (DORMANT_NO_RESPOND != dormantStatus))   // ��ǰ��������״̬(�ǵ㶯���ǵ�г)
    {
        if (tmp < funcCode.code.dormantFrq)             // ��������Ƶ��
        {
            if (++ticker >= (Uint32)funcCode.code.dormantTime * TIME_UNIT_DORMANT / RUN_CTRL_PERIOD)
            {
                dormantStatus = DORMANT_NO_RESPOND;     // ��������״̬

                ticker = 0;
            }
        }
        else
        {
            ticker = 0;
        }
    }
#endif    
}



#if DEBUG_F_POSITION_CTRL
//=====================================================================
//
// ���ٶȼ���
// 
// 
//
//=====================================================================
#define ACCEL_NUM     10
int32 accelArray[ACCEL_NUM];
int32 accelTickerArray[ACCEL_NUM];
int32 accel;    // ���ٶȣ��Ѿ�����2^8��
Uint16 accelDisp;
void AccCalc(int32 frq0)
{
    int16 i;
    int32 accelSum;
    static int32 frqOld;
    static int16 k;
    int16 tickerSum;
    static Uint16 runOld;

    // ��ʼ����ʱ�������ݣ���ȫ������
    if ((!runOld) && (runFlag.bit.run))
    {
        for (i = ACCEL_NUM - 1; i >= 0; i--)    // ����
        {
            accelArray[i] = 0;
            accelTickerArray[i] = 0;
        }
        
        k = 0;
        frqOld = 0;
        accel = 0;  // ���ٶ�ҲҪ����
        accelSum = 0;
        tickerSum = 0;
    }
    runOld = runFlag.bit.run;

    if (!runFlag.bit.run)
    {
        accel = 0;
        return;
    }

    k++;
    if (frq0 != frqOld) // frq�ı�
    {
        accelSum = 0;
        tickerSum = 0;
        for (i = ACCEL_NUM - 1; i > 0; i--)
        {
            accelArray[i] = accelArray[i-1];
            accelSum += accelArray[i];
            
            accelTickerArray[i] = accelTickerArray[i-1];
            tickerSum += accelTickerArray[i];
        }
        accelArray[0] = frq0 - frqOld;
        accelSum += accelArray[0];
        accelTickerArray[0] = k;
        tickerSum += accelTickerArray[0];
        
        accel = (accelSum << 8) / tickerSum;

        k = 0;
    }
    else if (k >= 200)
    {
        accel = 0;
        //k = 0;

        for (i = ACCEL_NUM - 1; i >= 0; i--)    // ����
        {
            accelArray[i] = 0;
            //accelTickerArray[i] = 0;
        }
    }

    accelDisp = accel * 500 >> 8;

#if 0
    if (ABS_INT32((int16)accelDisp) > 900)
    {
        asm(" nop");
    }
#endif

#if 0
    if (ABS_INT32(accel) < 100)
    {
        for (i = ACCEL_NUM - 1; i >= 0; i--)    // ����
        {
            accelArray[i] = 0;
            accelTickerArray[i] = 0;
        }
        accelArray[0] = accel * ACCEL_NUM;
        accelTickerArray[0] = 1;
    }
#endif

#if 0
    if (ABS_INT32(frqTmp) < 50)
    {
        accel = accelArray[0];
    }
#endif

    frqOld = frq0;
}
#endif














