//======================================================================
//
// �˵�����
//
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_menu.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_ui.h"
#include "f_eeprom.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_invPara.h"
#include "f_fcDeal.h"
#include "f_p2p.h"

#if F_DEBUG_RAM

#define DEBUG_F_USER_MENU_MODE              0   // �û��˵�ģʽ
#define DEBUG_F_CHECK_MENU_MODE             0   // У��˵�ģʽ
#define DEBUG_F_NO_SAME                     0   // NoSameDeal
#define DEBUG_F_DISP_DIDO_STATUS_SPECIAL    0   // DIDOֱ����ʾ
#define DEBUG_F_GROUP_HIDE                  0   // groupHide
#define DEBUG_F_MOTOR_FUNCCODE1             0
#define DEBUG_F_MFK                         0   // MFK����
#define DEBUG_F_PASSWORD                    0   // ���봦��
#define DEBUG_F_ERROR_TUNE_USE_SHIFT        0   // ����/��гʱ����ʹ��shift
#define DEBUG_FRQ_POINT                     0
#define DEBUG_ACC_DEC_TIME_POINT            0

#elif 1

#define DEBUG_F_NO_SAME                     1
#define DEBUG_F_USER_MENU_MODE              1
#define DEBUG_F_CHECK_MENU_MODE             1
#define DEBUG_F_DISP_DIDO_STATUS_SPECIAL    1
#define DEBUG_F_GROUP_HIDE                  1
#define DEBUG_F_MOTOR_FUNCCODE1             1
#define DEBUG_F_MFK                         1
#define DEBUG_F_PASSWORD                    1
#define DEBUG_F_ERROR_TUNE_USE_SHIFT        1   // ����/��гʱ����ʹ��shift
#define DEBUG_FRQ_POINT                     1
#define DEBUG_ACC_DEC_TIME_POINT            1

#endif




// 2���˵�����ʹ��shift����
// 1,2���˵���˸, �����û�����ʱ��˸
//#define NEWMENU_MENU3_USE_LEFT_SFIFT    0  // 3���˵�����ʹ��MFK������Ϊ����
//#define NEWMENU_REMEMBER_GRADE          0  // ����group��grade


// ʮ���Ƶĸ�λ��ʮλ����λ��ǧλ����λ
const int16 decNumber[5] = {1,      10,     100,    1000, 10000};
// ʮ�����Ƶĸ�λ��ʮλ����λ��ǧλ
const int16 hexNumber[4] = {0x0001, 0x0010, 0x0100, 0x1000};
const int16 deltaK[3] = {0, 1, -1};

int16 userMenuModeFcIndex;



#if 0
struct RESTORE_COMPANY_PARA_EXCEPT_INDEX
{
    Uint16 start;
    Uint16 end;
};

#define RESTORE_COMPANY_PARA_EXCEPT_NUMBER  6
static const struct RESTORE_COMPANY_PARA_EXCEPT_INDEX exceptRestoreSeries[RESTORE_COMPANY_PARA_EXCEPT_NUMBER] =
{
{GetCodeIndex(funcCode.group.ff[0]), GetCodeIndex(funcCode.group.ff[FFNUM-1])}, // FF ���Ҳ���
{GetCodeIndex(funcCode.group.fp[0]), GetCodeIndex(funcCode.group.fp[FPNUM-1])}, // FP ���������
{GetCodeIndex(funcCode.group.f1[0]), GetCodeIndex(funcCode.group.f1[F1NUM-2])}, // F1 �������
{GetCodeIndex(funcCode.group.a3[0]), GetCodeIndex(funcCode.group.a3[F1NUM-2])}, // A3 ��2�������
{GetCodeIndex(funcCode.group.ae[0]), GetCodeIndex(funcCode.group.ae[AENUM-1])}, // AE AIAO����У��
{GetCodeIndex(funcCode.code.errorLatest1), LAST_ERROR_RECORD_INDEX},    // ��һ�ι������ͣ�..., ���һ�����ϼ�¼
};
#endif


Uint16 GetDispDigits(Uint16 index);

struct FC_MOTOR_DEBUG   // ���ܵ���ʹ�ù�������ĸ���
{
    Uint16 fc;  // ������
    Uint16 u;   // ����
};
struct FC_MOTOR_DEBUG motorDebugFc;


enum MENU0_DISP_STATUS menu0DispStatus;     // 0���˵�����ʾ״̬
void UpdateMenu0DispStatus(void);

struct MENU_FUNC_CODE
{
    Uint16 group;
    Uint16 grade;
};
struct MENU_FUNC_CODE menuFc[MENU_MODE_MAX+1];      // �л��˵�ģʽʱ����group��grade
// �˵�ģʽ
enum MENU_MODE menuMode;        // �˵�ģʽ
enum MENU_MODE menuModeTmp;     // �˵�ģʽ����ʱֵ
enum MENU_MODE menuModeOld;

enum MENU_MODE_OPERATE menuModeStatus;
enum FAC_PASS_RANDOM_VIEW_OPERATE facPassViewStatus;

#if DEBUG_F_NO_SAME
// Ƶ��Դ���������ֵҪ����
#define FRQ_SRC_NO_SAME_NUMER   2
const Uint16 frqSrcFuncIndex[] =
{
    GetCodeIndex(funcCode.code.frqXSrc),
    GetCodeIndex(funcCode.code.frqYSrc),
};

// DI��
const Uint16 diFuncIndex[] =
{
    GetCodeIndex(funcCode.code.diFunc[0]),
    GetCodeIndex(funcCode.code.diFunc[1]),
    GetCodeIndex(funcCode.code.diFunc[2]),
    GetCodeIndex(funcCode.code.diFunc[3]),
    GetCodeIndex(funcCode.code.diFunc[4]),

    GetCodeIndex(funcCode.code.diFunc[5]),
    GetCodeIndex(funcCode.code.diFunc[6]),
    GetCodeIndex(funcCode.code.diFunc[7]),
    GetCodeIndex(funcCode.code.diFunc[8]),
    GetCodeIndex(funcCode.code.diFunc[9]),

    GetCodeIndex(funcCode.code.vdiFunc[0]),
    GetCodeIndex(funcCode.code.vdiFunc[1]),
    GetCodeIndex(funcCode.code.vdiFunc[2]),
    GetCodeIndex(funcCode.code.vdiFunc[3]),
    GetCodeIndex(funcCode.code.vdiFunc[4]),

    GetCodeIndex(funcCode.code.aiAsDiFunc[0]),
    GetCodeIndex(funcCode.code.aiAsDiFunc[1]),
    GetCodeIndex(funcCode.code.aiAsDiFunc[2]),
};
#endif



#define DECIMAL_DISPLAY_UPDATE_TIME     6       // 0���˵���ʾ(����ʱ��ʾ��ͣ��ʱ��ʾ)С�����2λ��ʾ�������ʱ�䣬_*12ms
#define UP_DOWN_DEAL_DONE_TIME          800     // UP/DOWN����֮��Ĵ���ʱ�䣬����ֹͣ��˸���߿�����˸��_ms




#define ONE_PLACE           0   // ��λ
#define TEN_PLACE           1   // ʮλ
#define HUNDRED_PLACE       2   // ��λ
#define THOUSAND_PLACE      3   // ǧλ
#define TEN_THOUSAND_PLACE  4   // ��λ


//===================================================================
enum MENU_LEVEL menuLevel;      // ��ǰ�˵����𣬼�0,1,2,3���˵�
Uint16 menu3Number;             // 3���˵���ʾ��ֵ
Uint16 menuPwdNumber;           // pwd�˵���ֵ

struct CURRENT_FUNC_CODE
{
    Uint16 index;               // ��ǰ��������funcCode.all[]������±�

    Uint16 group;               // ��ǰ�������group
    Uint16 grade;               // ��ǰ�������grade
};
struct CURRENT_FUNC_CODE curFc; // ��ǰ������
Uint16 curFcDispDigits;         // ��ǰ���������ʾλ��


LOCALF Uint16 ticker4LowerDisp;  // ����ʱ��ʾ�����2λ��Ҫ���¹���

Uint16 superFactoryPass;
Uint16 groupHidePwdStatus;          // ������������
Uint16 groupHideChkOkFlag;          // ����������������У��ok

#define FC_READ_ONLY_FLAG                                               \
    ((!funcCode.code.userPasswordReadOnly) ||                           \
    (curFc.index == GetCodeIndex(funcCode.code.userPassword)) ||        \
    (curFc.index == GetCodeIndex(funcCode.code.userPasswordReadOnly))   \
    )

// �Ƿ��д��1-��ǰ��д��0-��ǰ����д
#define IsWritable(attribute)                                       \
 ((FC_READ_ONLY_FLAG) &&                                            \
 ((ATTRIBUTE_READ_AND_WRITE == (attribute).bit.writable) ||         \
 ((ATTRIBUTE_READ_ONLY_WHEN_RUN == (attribute).bit.writable) &&     \
 (!runFlag.bit.run)))                                               \
)                                                                   \

enum FACTORY_PWD_STATUS
{
    FACTORY_PWD_LOCK,          // ��������, lock״̬
    FACTORY_PWD_UNLOCK         // ��������, unlock״̬
};
LOCALF enum FACTORY_PWD_STATUS factoryPwdStatus;    // �������룬Ĭ��Ϊ0(lock)
// ��ʼֵΪlock״̬��
// ����������FF-00��������ȷ�����enter��factoryPwdStatus ==> unlock״̬
// �����˻ص�2���˵�(FF-xx -> Fx)
//     ���벻��FF���3���˵�(FF-xx -> Fx-xx��enter)
//     factoryPwdStatus ==> lock״̬

Uint16 accDecFrqPrcFlag;   // ����UP/DOWN�޸�Ƶ�ʱ�־
Uint16 bFrqDigitalDone4WaitDelay; // UP/DOWN���֮��һ��ʱ�����ʾ����ʱ��
LOCALF Uint16 accDecFrqTicker;
Uint16 frqDisp;
Uint16 frqAimDisp;
Uint16 frqPLCDisp;       // PLC�ɱ�̿���ȡ����
Uint16 frqAimPLCDisp;    // PLC����̿���ȡ�趨Ƶ��
Uint16 pidFuncRefDisp;
Uint16 pidFuncFdbDisp;
Uint16 outVoltageDisp;
Uint16 outCurrentDisp;      // ���������ʵ��ֵ��
Uint16 itDisp;              // ���ת��
Uint16 loadSpeedDisp;
Uint16 currentOcDisp;
Uint16 pulseInFrqDisp;
Uint16 frqRunDisp;
Uint16 pcOriginDisp;
Uint16 frqXDisp;
Uint16 frqYDisp;
Uint32 torqueCurrentAct;   // ת�ص���

// ��ʾ���ļ�
// U�飬ͣ��״̬��ʾ��
#include "f_funcCode_disp.c"


// ����RUN��
LOCALD void MenuOnRun(void);

// ����STOP��
LOCALD void MenuOnStop(void);

// ����MF.K��
LOCALD void MenuOnMfk(void);


// ����PRG��
LOCALD void Menu0OnPrg(void);
void Menu1OnPrg(void);
LOCALD void Menu2OnPrg(void);
LOCALD void Menu3OnPrg(void);

// ����UP��
LOCALD void Menu0OnUp(void);    // 0���˵��°���UP�ĺ���
LOCALD void Menu1OnUp(void);    // 1���˵��°���UP�ĺ���
LOCALD void Menu2OnUp(void);    // 2���˵��°���UP�ĺ���
LOCALD void Menu3OnUp(void);    // 3���˵��°���UP�ĺ���

// ����DOWN��
LOCALD void Menu0OnDown(void);
LOCALD void Menu1OnDown(void);
LOCALD void Menu2OnDown(void);
LOCALD void Menu3OnDown(void);

// ����ENTER��
LOCALD void Menu0OnEnter(void);
LOCALD void Menu1OnEnter(void);
LOCALD void Menu2OnEnter(void);
LOCALD void Menu3OnEnter(void);

// ����SHIFT��
LOCALD void Menu0OnShift(void);
LOCALD void Menu1OnShift(void);
LOCALD void Menu2OnShift(void);
LOCALD void Menu3OnShift(void);

// ����QUICK��
LOCALD void MenuOnQuick(void);


LOCALD void Menu0OnUpDown(void);
LOCALD void Menu1OnUpDown(Uint16 flag);
LOCALD void Menu2OnUpDown(Uint16 flag);
LOCALD void Menu3OnUpDown(Uint16 flag);


// ��ʾ�������
LOCALD void UpdateMenu0DisplayBuffer(void);
LOCALD void UpdateMenu1DisplayBuffer(void);
LOCALD void UpdateMenu2DisplayBuffer(void);
LOCALD void UpdateMenu3DisplayBuffer(void);
LOCALD void UpdateDisplayBufferAttribute(const Uint16 data, const union FUNC_ATTRIBUTE attribute);
void UpdateDisplayBufferVisualIoStatus(Uint32 value);
void UpdateDisplayBufferVisualDiFunc(Uint16 valueH, Uint32 valueL);
LOCALD void UpdateErrorDisplayBuffer(void);
LOCALD void UpdateTuneDisplayBuffer(void);


LOCALD void MenuPwdOnPrg(void);
LOCALD void MenuPwdHintOnUp(Uint16 flag);
LOCALD void MenuPwdHint2Input(void);
LOCALD void MenuPwdHintOnDown(Uint16 flag);
LOCALD void MenuPwdHintOnShift(void);
LOCALD void MenuPwdHintOnQuick(void);
LOCALD void UpdateMenuPwdHintDisplayBuffer(void);

LOCALD void MenuPwdInputOnPrg(void);
LOCALD void MenuPwdInputOnUp(void);
LOCALD void MenuPwdInputOnEnter(void);
LOCALD void MenuPwdInputOnDown(void);
LOCALD void MenuPwdInputOnShift(void);
LOCALD void MenuPwdInputOnQuick(void);
LOCALD void UpdateMenuPwdInputDisplayBuffer(void);

void MenuPwdInputOnUpDown(Uint16 flag);

void MenuModeSwitch(void);


struct MENU_ATTRIBUTE menuAttri[MENU_LEVEL_NUM];
#if F_DEBUG_RAM
#pragma DATA_SECTION(menu, "const_zone");
#endif
const sysMenu menu[MENU_LEVEL_NUM] =
{
// 0���˵�
    {Menu0OnPrg,        Menu0OnUpDown,      Menu0OnEnter,
     MenuOnMfk,         Menu0OnUpDown,      Menu0OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu0DisplayBuffer},
// 1���˵�
    {Menu1OnPrg,        Menu1OnUp,          Menu1OnEnter,
     MenuOnMfk,         Menu1OnDown,        Menu1OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu1DisplayBuffer},
// 2���˵�
    {Menu2OnPrg,        Menu2OnUp,          Menu2OnEnter,
     MenuOnMfk,         Menu2OnDown,        Menu2OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu2DisplayBuffer},
// 3���˵�
    {Menu3OnPrg,        Menu3OnUp,          Menu3OnEnter,
     MenuOnMfk,         Menu3OnDown,        Menu3OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu3DisplayBuffer},
// PwdHint�˵�
    {MenuPwdOnPrg,      MenuPwdHint2Input,  MenuPwdHint2Input,
     MenuOnMfk,         MenuPwdHint2Input,  MenuPwdHintOnShift,
     MenuOnRun,         MenuOnStop,         MenuPwdHintOnQuick,
     UpdateMenuPwdHintDisplayBuffer},
// PwdInput�˵�
    {MenuPwdOnPrg,      MenuPwdInputOnUp,   MenuPwdInputOnEnter,
     MenuOnMfk,         MenuPwdInputOnDown, MenuPwdInputOnShift,
     MenuOnRun,         MenuOnStop,         MenuPwdInputOnQuick,
     UpdateMenuPwdInputDisplayBuffer},
};
//(void (*)(void))Menu0OnPrg

   
// ��ת������
LOCALD Uint16 OverTurnDeal(Uint16 data, Uint16 upper, Uint16 lower, Uint16 flag);
// 0���˵�ѭ����λ����
LOCALD void cycleShiftDeal(Uint16 flag);



struct GROUP_DISPLAY
{
    Uint16 dispF;   // ��������F0, ��ʾF
    Uint16 disp0;   // ��������F0, ��ʾ0
};
struct GROUP_DISPLAY groupDisplay;
void UpdateGroupDisplay(Uint16 group);

Uint16 GroupUpDown(const Uint16 funcCodeGrade[], Uint16 group, Uint16 flag);
void DealUserMenuModeGroupGrade(Uint16 flag);
void GetGroupGrade(Uint16 index);

//-------------------------------------------------
Uint16 checkMenuModeCmd;    // ����ָ�1-��ʼ������0-������ָ��/�������
Uint16 checkMenuModePara;   // ����ָ��Ĳ���
enum CHECK_MENU_MODE_DEAL checkMenuModeDealStatus;
Uint16 checkMenuModeSerachNone; // ������û���ҵ������ֵ��ͬ�Ĺ����룬��־��
// 1-û���ҵ������ֵ��ͬ�Ĺ�����
// 0-�ҵ��������ֵ��ͬ�Ĺ�����
void DealCheckMenuModeGroupGrade(Uint16 flag);
//-------------------------------------------------

Uint16 LimitOverTurnDeal(const Uint16 limit[], Uint16 data, Uint16 upper, Uint16 low, Uint16 flag);
void MotorDebugFcDeal(void);
void fghldf(Uint16 dest[], const Uint16 src[], Uint16 length);
void Menu0AddMenuLevel(void);
Uint16 ValidateTuneCmd(Uint16 value, Uint16 motorIndex);




void GroupHideDeal(Uint16 funcCodeGrade[]);


//=====================================================================
//
// ���в˵������£�����run���Ĵ���
// run������ ����������д���ֱ��ʹ�õ�ǰ�İ�����
//
//=====================================================================
LOCALF void MenuOnRun(void)
{
    ;
}


//=====================================================================
//
// ���в˵������£�����stop���Ĵ���
// run������ ����������д���ֱ��ʹ�õ�ǰ�İ�����
//
//=====================================================================
LOCALF void MenuOnStop(void)
{
    ;
}



// quick����
void MenuOnQuick(void)
{
    Uint16 digit[5];
    enum MENU_MODE menuModeNext[3];

    // �����㼶������QUICK��Ч
    if (!funcCode.code.menuMode)
    {
        menuModeTmp = MENU_MODE_BASE;
        return;
    }
    
    if (MENU_MODE_ON_QUICK == menuModeStatus)   // ��1�ΰ�������ʾ��ǰģʽ��֮��Ÿı�ģʽ
    {
        GetNumberDigit1(digit, funcCode.code.menuMode);
        if (digit[0])
		{
            menuModeNext[0] = MENU_MODE_USER;
		}
        else
        {
            menuModeNext[0] = MENU_MODE_CHECK;
        }

        if (digit[1])
		{
            menuModeNext[1] = MENU_MODE_CHECK;
		}
        else
        {
            menuModeNext[1] = MENU_MODE_BASE;
        }

        menuModeNext[2] = MENU_MODE_BASE;

        menuModeTmp = menuModeNext[((Uint16)menuModeTmp) - 1];
        
    }

    menuModeStatus = MENU_MODE_ON_QUICK;
}



LOCALF void Menu1OnUp(void)
{
    Menu1OnUpDown(ON_UP_KEY);
}


LOCALF void Menu2OnUp(void)
{
    Menu2OnUpDown(ON_UP_KEY);
}


LOCALF void Menu3OnUp(void)
{
    Menu3OnUpDown(ON_UP_KEY);
}



LOCALF void Menu1OnDown(void)
{
    Menu1OnUpDown(ON_DOWN_KEY);
}


LOCALF void Menu2OnDown(void)
{
    Menu2OnUpDown(ON_DOWN_KEY);
}


LOCALF void Menu3OnDown(void)
{
    Menu3OnUpDown(ON_DOWN_KEY);
}


//=====================================================================
//
// ���в˵������£�����MF.K���Ĵ���
//
//=====================================================================
LOCALF void MenuOnMfk(void)
{
#if DEBUG_F_MFK
    if (tuneCmd)
    {
        return;
    }

    switch (funcCode.code.mfkKeyFunc)
    {
        case FUNCCODE_mfkKeyFunc_SWITCH: // ������������ͨ���л�
            // F0-00�������趨ֵΪ������壬MF.K���Ĳ����������Դͨ�л����������
            if (FUNCCODE_runSrc_PANEL != funcCode.code.runSrc)
            {
                keyFunc = KEY_SWITCH;
            }
            break;

        case FUNCCODE_mfkKeyFunc_REVERSE:   // ����ת�л�
            if (FUNCCODE_runSrc_PANEL == runSrc)    // ��ǰ����Դͨ��Ϊ�������
            {
                keyFunc = KEY_REV;
            }
            break;

        case FUNCCODE_mfkKeyFunc_FWD_JOG:
            keyFunc = KEY_FWD_JOG;
            break;

        case FUNCCODE_mfkKeyFunc_REV_JOG:
            keyFunc = KEY_REV_JOG;
            break;
            
        default:
            break;
    }
#endif
}


//=====================================================================
//
// menu0, �㼶�˵�
//
//=====================================================================
LOCALF void Menu0OnPrg(void)
{
    if (tuneCmd)
    {
        if (!runFlag.bit.tune)
        {
            menuLevel = MENU_LEVEL_2;    // ȡ����г���ص�2���˵�
            tuneCmd = 0;
        }
        else
            Menu0AddMenuLevel();
//        return;
    }

#if DEBUG_F_PASSWORD
    else if ((funcCode.code.userPassword)    // ���û�����
            && (menuMode != MENU_MODE_USER)  // ��ǰ��Ϊ�û����Ʋ���ģʽ
//       || (funcCode.code.userPasswordReadOnly)
        )
    {
        menuLevel = MENU_LEVEL_PWD_HINT;    // ����PWD_HINT�˵�����ʾ������������
    }
    else                        // û������
#endif
    {
        Menu0AddMenuLevel();
    }

    menuAttri[menuLevel].operateDigit = 0;
}


LOCALF void Menu0OnEnter(void)
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // ����QUICK���ٰ���ENTER
    {
        MenuModeSwitch();
        return;
    }
}


void MenuModeSwitch(void)
{
    menuModeOld = menuMode;

    menuModeStatus = MENU_MODE_NONE;
        
    // �л��˵�ģʽʱ����group��grade
    menuFc[menuMode].group = curFc.group;       // ����old
    menuFc[menuMode].grade = curFc.grade;
    menuMode = menuModeTmp;
    curFc.group = menuFc[menuModeTmp].group;    // �ָ�new
    curFc.grade = menuFc[menuModeTmp].grade;

// ���ָ�֮���groupΪ���Ҳ����飬��
// menuMode���ı䣬�������룬lock״̬
    if ((menuModeOld != menuMode)
        && (FC_GROUP_FACTORY == curFc.group)
        )
    {
        factoryPwdStatus = FACTORY_PWD_LOCK;
        curFc.grade = 0;                        // ����lock״̬��grade����
    }

    MenuModeDeal();
	
// �ı�menuModeʱ��3���˵�����Ϊ2���˵���
    if (menuModeOld == MENU_MODE_USER)
    {
        Menu0OnPrg();
    }
    else if ((menuLevel == MENU_LEVEL_3)
        && (menuModeOld != menuMode)
        )
    {
        menuLevel = MENU_LEVEL_2;
    }
// -C-ģʽû��1���˵�
// 0/1���˵��¸ı�Ϊ-C-ģʽ������2���˵�
    else if (((menuLevel == MENU_LEVEL_0) || (menuLevel == MENU_LEVEL_1))
        && (MENU_MODE_CHECK == menuMode)
        )
    {
        Menu0OnPrg();
    }
// �û����Ʋ˵���1���˵�
// 0/1���˵��¸ı�Ϊ�û�����ģʽ������2���˵�
    else if (((menuLevel == MENU_LEVEL_0) || (menuLevel == MENU_LEVEL_1))
        && (MENU_MODE_USER == menuMode)
        )
    {
        Menu0OnPrg();
    }
// 0���˵��¸��ģ�����1���˵�
    else if (menuLevel == MENU_LEVEL_0)
    {
        Menu0OnPrg();
    }

// �ı�menuModeʱ����ǰ����bitΪ0
    if (menuModeOld != menuMode)
    {
        menuAttri[menuLevel].operateDigit = 0;
    }

    
}


LOCALF void Menu0OnUpDown(void)
{
    if (!tuneCmd)
    {
        accDecFrqPrcFlag = ACC_DEC_FRQ_WAIT;
        accDecFrqTicker = 0;
        frqKeyUpDownDelta = upDownDelta;
    }
}


LOCALF void Menu0OnShift(void)
{
    ticker4LowerDisp = 0;       // 0���˵��°���shift����ʾ���2λ����

#if DEBUG_F_ERROR_TUNE_USE_SHIFT
// 0���˵��£��й���ʱ����shift��Ȼ��Ч
    if (MENU0_DISP_STATUS_RUN_STOP != menu0DispStatus)
    {
        menu0DispStatus = MENU0_DISP_STATUS_RUN_STOP;   // ��������/ͣ����ʾ״̬
    }
    else
#elif 1
    if ((!tuneCmd)
        && (!errorCode)         // 0���˵��£��й���ʱ����shift��Ч
        )
#endif
    {
        cycleShiftDeal(1);      // 0���˵���ʾ��ѭ����λ����
    }
}


//=====================================================================
//
// menu1, һ���˵�
//
//=====================================================================
void Menu1OnPrg(void)
{
    menuLevel = MENU_LEVEL_0;
    groupHideChkOkFlag = 0;

    ticker4LowerDisp = 0;   // �˵�����������Ϊ0ʱ��ticker����
}


LOCALF void Menu1OnEnter()
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // ����QUICK���ٰ���ENTER
    {
        MenuModeSwitch();
        return;
    }
    
    menuLevel = MENU_LEVEL_2;
    menuAttri[menuLevel].operateDigit = 0;

    if (FC_GROUP_FACTORY == curFc.group)    // ����FF�飬grade���ã���������ȷ������ٻָ�
        curFc.grade = 0;
}


LOCALF void Menu1OnUpDown(Uint16 flag)
{
    curFc.group = GroupUpDown(funcCodeGradeCurMenuMode, curFc.group, flag);

// ������grade�����޸�groupʱ��grade������
    curFc.grade = 0;
}


LOCALF void Menu1OnShift(void)
{
    ;
}


//=====================================================================
//
// menu2, �����˵�
//
//=====================================================================
LOCALF void Menu2OnPrg(void)
{
    menuLevel = MENU_LEVEL_1;
    
    if ((MENU_MODE_CHECK == menuMode)
        || (MENU_MODE_USER == menuMode)
        )
    {
        //menuLevel = MENU_LEVEL_0;
        Menu1OnPrg();
    }

//  �˻ص�2���˵�(FF-xx -> Fx)
//  factoryPwdStatus ==> lock״̬
    factoryPwdStatus = FACTORY_PWD_LOCK;
}


LOCALF void Menu2OnEnter(void)
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // ����QUICK���ٰ���ENTER
    {
        MenuModeSwitch();
        return;
    }

    if ((MENU_MODE_CHECK == menuMode)
        && (checkMenuModeSerachNone)
        )
    {
        return;
    }
    
//     ���벻��FF���3���˵�(FF-xx -> Fx-00��enter)
//     factoryPwdStatus ==> lock״̬
    if (curFc.group != FC_GROUP_FACTORY)
    {
        factoryPwdStatus = FACTORY_PWD_LOCK;
    }

    menuAttri[MENU_LEVEL_2].operateDigit = 0;
    curFc.index = GetGradeIndex(curFc.group, curFc.grade);

    {
        menuLevel = MENU_LEVEL_3;

        menuAttri[MENU_LEVEL_3].operateDigit = 0;
        menu3Number = funcCode.all[curFc.index];

        // ȷ����������ʾλ��
        curFcDispDigits = GetDispDigits(curFc.index);
    }
}


LOCALF void Menu2OnUpDown(Uint16 flag)
{
    if (MENU_MODE_USER == menuMode)
    {
        DealUserMenuModeGroupGrade(flag);
    }
    else if (MENU_MODE_CHECK == menuMode)
    {
        checkMenuModeCmd = 1;
        checkMenuModePara = flag;
    }
    else if (menuAttri[MENU_LEVEL_2].operateDigit >= 3)   // �޸�currentGroup
    {
        Menu1OnUpDown(flag);
    }
    else                        // �޸�currentGrade
    {
        int16 delta = 1;
        Uint16 tmp;

        if ((FC_GROUP_FACTORY == curFc.group) && (FACTORY_PWD_LOCK == factoryPwdStatus)) // ��������
        {
            Menu2OnEnter();     // FFʱ������UP/DOWNҲ���Խ�����������״̬
            return;
        }

#if 1
        tmp = OverTurnDeal(curFc.grade, funcCodeGradeCurMenuMode[curFc.group] - 1, 0, flag);
        if (curFc.grade == tmp)    // û�з�ת
        {
            if (1 == menuAttri[MENU_LEVEL_2].operateDigit)
                delta = 10;

            if (ON_DOWN_KEY == flag)
                delta = -delta;

            curFc.grade = LimitDeal(0, curFc.grade, funcCodeGradeCurMenuMode[curFc.group] - 1, 0, delta);
        }
        else
        {
            curFc.grade = tmp;
        }
#elif 1
        if (1 == menuAttri[MENU_LEVEL_2].operateDigit)
            delta = 10;

        if (ON_DOWN_KEY == flag)
            delta = -delta;

        curFc.grade += delta;
        if ((int16)curFc.grade >= (int16)(funcCodeGradeCurMenuMode[curFc.group])
        {
            curFc.grade = 0;
        }
        else if ((int16)curFc.grade < 0)
        {
            curFc.grade = funcCodeGradeCurMenuMode[curFc.group] - 1;
        }
#endif
    }
}


LOCALF void Menu2OnShift(void)
{
// �û�����
// �ǳ���ֵ
// �˵�ģʽ
// û��shift
    if (MENU_MODE_BASE == menuMode)
    {
        if (menuAttri[MENU_LEVEL_2].operateDigit == 0)
            menuAttri[MENU_LEVEL_2].operateDigit = 3;
        else if (menuAttri[MENU_LEVEL_2].operateDigit == 1)
            menuAttri[MENU_LEVEL_2].operateDigit = 0;
        else if (menuAttri[MENU_LEVEL_2].operateDigit == 3)
            menuAttri[MENU_LEVEL_2].operateDigit = 1;
    }
}


//=====================================================================
//
// menu3, �����˵�
//
//=====================================================================
LOCALF void Menu3OnPrg(void)
{
    menuLevel = MENU_LEVEL_2;
    menuAttri[MENU_LEVEL_2].operateDigit = 0;
}


LOCALF void Menu3OnEnter(void)
{
    Uint16 writable = funcCodeAttribute[curFc.index].attribute.bit.writable;

    if (MENU_MODE_ON_QUICK == menuModeStatus)       // ����QUICK���ٰ���ENTER
    {
        MenuModeSwitch();
        return;
    }

// ���ڲ���EEPROM������Ӧ"���湦����"��������Ӧ����������
    if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode)
    {
        if ((FACTORY_PWD_INDEX != curFc.index)        // FF-00
            && (ATTRIBUTE_READ_ONLY_ANYTIME != writable)    // (�κ�ʱ��)ֻ���Ĺ�����
            )
        {
            return;
        }
    }

// ����������ֻ���Ĺ����룬��ͣ��ʱ�޸���(��δ��ENTER)�������У���ENTER������Ӧ��
// �����κ�ʱ��ֻ���Ĺ����룬��Ҫ��ô����
    if ((runFlag.bit.run)      // ����
        && (ATTRIBUTE_READ_ONLY_WHEN_RUN == writable) // ����ʱֻ��
        && (funcCode.all[curFc.index] != menu3Number) // ֵ���ı�
        )
    {
        return;
    }

    // U�鹦����ȫ��ֻ��
    if (curFc.group >= FUNCCODE_GROUP_U0)
    {
        return;
    }

#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    // FP-01������ʼ����FF-00�������룬��г����Ҫ����
    if (GetCodeIndex(funcCode.code.paraInitMode) == curFc.index) // FP-01, ������ʼ��
    {
        if (FUNCCODE_paraInitMode_CLEAR_RECORD == menu3Number) // �����¼
        {
            ClearRecordDeal();
        }
        else
        {
            if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == menu3Number) // FP-01==1
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == menu3Number) // FP-01==3
            {
                //funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL;
            }
            else if (FUNCCODE_paraInitMode_SAVE_USER_PARA == menu3Number) // FP-01==4
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_SAVE_USER_PARA;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == menu3Number) // FP-01==5
            {
                if ((funcCode.code.saveUserParaFlag1 == USER_PARA_SAVE_FLAG1)
                    && (funcCode.code.saveUserParaFlag2 == USER_PARA_SAVE_FLAG2))
                {
                    // �ָ��ѱ�����û�����
                    funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_USER_PARA;
                }
                else
                {
                    // ֮ǰδ�����û������������,�ù�����Ч
                    return;
                }
            }
        }
    }
    else
#endif
        if (FACTORY_PWD_INDEX == curFc.index) // FF-00, ��������
//    else if ((DISPLAY_F == curFc.group) && (FACTORY_PWD_LOCK == factoryPwdStatus)) // FF-00, ��������
    {
        if (COMPANY_PASSWORD != menu3Number) // �������벻��ȷ
        {
            menu3Number = 0;
            menuAttri[MENU_LEVEL_3].operateDigit = 0;
            return;
        }
        else
        {
            factoryPwdStatus = FACTORY_PWD_UNLOCK;    // ����������ȷ������
        }
    }
    else if (GetCodeIndex(funcCode.code.tuneCmd) == curFc.index) // ���1��г
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_1))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM2.tuneCmd) == curFc.index) // ���2��г
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_2))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM3.tuneCmd) == curFc.index) // ���3��г
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_3))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM4.tuneCmd) == curFc.index) // ���4��г
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_4))
            return;
    }
    else if (ATTRIBUTE_READ_ONLY_ANYTIME != writable) // �� �κ�ʱ��ֻ�� �Ĺ�����
    // �����漸��������֮�⣬�޸ĵ�ֵ����Ҫ����
    {
// ĳЩ����Ĺ����롣ModifyFunccodeEnter()�л��funcCode��ֵ��
// ����� ���ʸı䴦�� �� ���͸ı䴦�� ���档
        if (COMM_ERR_PARA == ModifyFunccodeEnter(curFc.index, menu3Number))
            return;

        //funcCode.all[curFc.index] = menu3Number; // RAM
        SaveOneFuncCode(curFc.index);  // ��ʱ���ÿ�����EEPROM�е�ֵ�Ƿ���ͬ��
    }

    if (MENU_MODE_USER == menuMode)
    {
        DealUserMenuModeGroupGrade(ON_UP_KEY);
    }
    else if (MENU_MODE_CHECK == menuMode)
    {
        checkMenuModeCmd = 1;
        checkMenuModePara = ON_UP_KEY;
    }
    else if (++curFc.grade >= funcCodeGradeCurMenuMode[curFc.group])
    {
        curFc.grade = 0;
    }

    // ��г�����룬�����г״̬��menuLevel��Ϊ0����
    if ((tuneCmd) &&
        ((curFc.index == GetCodeIndex(funcCode.code.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM3.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM4.tuneCmd)) )
        )
    {
        menuLevel = MENU_LEVEL_0;
    }
    else
    {
        menuLevel = MENU_LEVEL_2;
        menuAttri[MENU_LEVEL_2].operateDigit = 0;
    }
}



// ȷ�ϵ�г�����Ƿ���Ч
Uint16 ValidateTuneCmd(Uint16 value, Uint16 motorsN)
{
    if (motorsN != motorSn)       // �Ƿ�Ϊ��ǰѡ����
    {
        return COMM_ERR_PARA;
    }
    
    if (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType)    // pmsm
    {
        if ((FUNCCODE_tuneCmd_PMSM_11 != value) &&
            (FUNCCODE_tuneCmd_PMSM_12 != value) &&
            (FUNCCODE_tuneCmd_PMSM_13 != value)
            )
        {
            return COMM_ERR_PARA;
        }
    }
    else        // �첽��
    {
        if ((FUNCCODE_tuneCmd_ACI_STATIC != value) &&
            (FUNCCODE_tuneCmd_ACI_WHOLE != value)
            )
        {
            return COMM_ERR_PARA;
        }
    }

    if ((errorCode == ERROR_NONE) && (FUNCCODE_runSrc_PANEL == runSrc))       // ���������ͨ�������ܵ�г
    {
        tuneCmd = value;
        return COMM_ERR_NONE;
    }
    else
    {
        return COMM_ERR_PARA;
    }
}



LOCALF void Menu3OnUpDown(Uint16 flag)
{
    int16 delta;
    Uint16 flag1 = 1;

    if (curFc.group >= FUNCCODE_GROUP_U0) // U�飬��ʾ
    {   // U�飬3���˵��£�UP/DOWN�Զ���next grade��ͬʱ��Ҫ����currentIndex
        Menu2OnUpDown(flag);
        curFc.index = GetGradeIndex(curFc.group, curFc.grade);

        flag1 = 0;
    }
#if DEBUG_F_USER_MENU_MODE
    else if (USER_MENU_GROUP == curFc.group)     // FE�飬�û����ƹ�����
    {
        if (2 == menuAttri[MENU_LEVEL_3].operateDigit)
        {
            Uint16 group;
            Uint16 funcCodeGrade[FUNCCODE_GROUP_NUM];   // ��ջ����
            
            memcpy(funcCodeGrade, funcCodeGradeAll, (FUNCCODE_GROUP_NUM));          
            funcCodeGrade[FC_GROUP_FACTORY] = 0;   // �û����Ʋ˵���������FF��
            funcCodeGrade[USER_MENU_GROUP] = 0;  // �û����Ʋ˵���������CC��

#if DEBUG_F_MOTOR_FUNCCODE
            MotorDebugFcDeal();
            funcCodeGrade[FUNCCODE_GROUP_CF] = motorDebugFc.fc;
            funcCodeGrade[FUNCCODE_GROUP_UF] = motorDebugFc.u;
#endif

            group = GroupUpDown(funcCodeGrade, menu3Number / 100, flag);
            menu3Number = group * 100; // �ı���ʱ��grade����

            flag1 = 0;
        }
    }
#endif

    if (flag1)  // �����ҪUP/DOWN
    {
        // ʮ����Լ��
        if (ATTRIBUTE_MULTI_LIMIT_HEX != funcCodeAttribute[curFc.index].attribute.bit.multiLimit)
        {
            delta = decNumber[menuAttri[MENU_LEVEL_3].operateDigit];
        }
        else    // ��������룬ʮ������
        {
            delta = hexNumber[menuAttri[MENU_LEVEL_3].operateDigit];
        }

        if (ON_DOWN_KEY == flag)
            delta = -delta;

        ModifyFunccodeUpDown(curFc.index, &menu3Number, delta);
    }
}


LOCALF void Menu3OnShift(void)
{
    Uint16 max = curFcDispDigits;
    
#if DEBUG_F_USER_MENU_MODE
    if (USER_MENU_GROUP == curFc.group)     // FE�飬�û����ƹ�����
    {
        max = 3;
    }
#endif

    if (!menuAttri[MENU_LEVEL_3].operateDigit)
        menuAttri[MENU_LEVEL_3].operateDigit = max - 1;
    else
        menuAttri[MENU_LEVEL_3].operateDigit--;
}


//=====================================================================
//
// �������޷�����
//
// ����:
// signal--- ��Ҫ�޷�����Ĺ������signal��
// data  --- �޸�֮ǰ������
// upper --- ����������
// lower --- ����������
// delta --- delta>0, UP; delta<0, DOWN; delta==0, ��⵱ǰֵ�Ƿ�����ֵ��Χ��
//
// ���أ��޸�֮���ֵ
//
// ע�⣺
// ���з���ʱ��Ҫ�������������
//   upper = 30000, data = +25000, delta = +10000   ==> data = 30000
// ���޷���ʱ��Ҫ�������������
//   lower = 5,     data = 4,       delta = -0      ==> data = 5
//   lower = 5,     data = 6,       delta = -10     ==> data = 5
//   upper = 65535, data = 60000,   delta = +10000  ==> data = 65535
//
//=====================================================================
Uint16 LimitDeal(Uint16 signal, Uint16 data, Uint16 upper, Uint16 lower, int16 delta)
{
    int32 data1, upper1, lower1;

    if (signal) // �з���
    {
        data1 = (int32)(int16)data;
        upper1 = (int32)(int16)upper;
        lower1 = (int32)(int16)lower;
    }
    else        // �޷���
    {
        data1 = (int32)data;
        upper1 = (int32)upper;
        lower1 = (int32)lower;
    }

// �൱�ڶ�ԭֵҲ���бȽ���
    data1 += delta;
    if (data1 > upper1)         // ���޴���
    {
        data1 = upper1;
    }
    if (data1 < lower1)         // ���޴���
    {
        data1 = lower1;
    }

    return (Uint16)data1;
}


//=====================================================================
//
// ĳЩ��������趨������ͬ��Ŀǰ280��DI���ӣ�320��������Ƶ��Դ
//
// ���룺
// index        -- ��Ҫ���д��������index
// funcIndex[]  -- ��Щ�趨������ͬ�������index����
// number       -- funcIndex[]�ĳ���
// data         -- ��ǰ�����룬���趨��ֵ
// upper        -- ����������
// lower        -- ����������
// delta        -- ������ֵ��ǰ�������ӵ�delta������--UP, ����--DOWN��0--���ж��Ƿ��������������趨��ͬ
//
// ����:
//      delta��Ϊ0ʱ�������������������趨��ͬ�����ݡ�
//      deltaΪ0ʱ����ͬ�򷵻�0�����򷵻�����ֵ��
//
//=====================================================================
Uint16 NoSameDeal(Uint16 index, const Uint16 funcIndex[], int16 number, int16 data, int16 upper, int16 lower, int16 delta)
{
#if DEBUG_F_NO_SAME
    Uint16 i;

#if 1
    // ��ת
    if ((data == upper) && (delta > 0))
    {
        data = lower;       // 0(���޹��ܣ�)�����ظ���
    }
    else if ((data == lower) && (delta < 0))
    {
        data = upper;       // upper�������ظ�����Ҫ���д���
    }
    else
#endif
    {
        data = LimitDeal(0, data, upper, lower, delta); // Ŀǰ�����޷���
    }

    if (data == lower)      // 0���޹��ܣ������ظ������ء�
        return data;

    for (;;)
    {
        for (i = 0; i < number; i++)    // �����в����ظ���index
        {
            if ((funcIndex[i] != index) && (data == funcCode.all[funcIndex[i]])) // ���������������趨��ͬ
            {
                if (delta > 0)
                {
                    data += 1;              // �����ظ����Զ���1(Ҳ����Ը�Ϊdelta)������
                    if (data > upper)       // ����1֮�󳬹����ֵ����Ϊ��Сֵ��
                    {
                        data = lower;       // 0���޹��ܣ������ظ�
                    }
                }
                else if (delta < 0)
                {
                    data -= 1;              // �����ظ����Զ���1(Ҳ����Ը�Ϊdelta)����
                    if (data < lower)       // ����1֮��С����Сֵ����Ϊ���ֵ��
                    {
                        data = upper;       // ���ֵ�����Ƚ�
                    }
                }
                else
                {
                    data = lower;
                }

                break;
            }
        }

        if ((i >= number) ||    // һֱû���ظ�
            (data == lower)     // �����ظ����Զ���1����������1֮�󳬹����ֵ����Ϊ��Сֵ��
            )
            break;
    }

    return data;
#endif
}


//=====================================================================
//
// 0���˵���ʾ��ѭ����λ������
// ����:
//      flag -- 1 ����shift֮�� ���ñ�����
//              0 �ı书����֮�󣬵��ñ�����
//
//
//=====================================================================
LOCALF void cycleShiftDeal(Uint16 flag)
{
    Uint16 *bit = &funcCode.code.dispParaStopBit;
    Uint32 para = funcCode.code.ledDispParaStop;
    Uint16 max = STOP_DISPLAY_NUM;

    if (runFlag.bit.run)
    {
        bit = &funcCode.code.dispParaRunBit;
        para = funcCode.code.ledDispParaRun1 + ((Uint32)funcCode.code.ledDispParaRun2 << 16);
        max = RUN_DISPLAY_NUM;
    }

    if (!para)  // ��ֹ����Ϊ0
    {
        para = 1;
    }

    if ((!flag) && (para & (0x1UL << *bit)))    // �ı书���룬�ҵ�ǰ��ʾbit��ȻҪ��ʾ
        return;

    do
    {
        if (++(*bit) >= max)
        {
            if (errorCode)  // �й���
            {
                menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // �������/�澯��ʾ״̬
            }

            if (tuneCmd)
            {
                menu0DispStatus = MENU0_DISP_STATUS_TUNE;   // �����г��ʾ
            }

            *bit = 0;
        }
    }
    while (!(para & (0x1UL << *bit)));
}


//=====================================================================
//
// ��ת������
//
// ����:
//      data  -- ������Ҫ��ת������
//      upper -- ����
//      lower -- ����
//      flag  -- UP/DOWN��־
//
// ���أ�
//      ��ת֮���ֵ��
//
//=====================================================================
LOCALF Uint16 OverTurnDeal(Uint16 data, Uint16 upper, Uint16 lower, Uint16 flag)
{
    if (ON_UP_KEY == flag)
    {
        if (upper == data)
        {
            data = lower;
        }
    }
    else
    {
        if (lower == data)
        {
            data = upper;
        }
    }

    return data;
}

//=====================================================================
//
// 0���˵��µ���ʾ������º���, 12ms����1��
// ����/ͣ��ʱLED��ʾ�������޸��趨Ƶ��ʱ������ʾ������ǰ���0
// ��ʾ����˸����ʱ��ʹ���˲�
//
//=====================================================================
LOCALF void UpdateMenu0DisplayBuffer(void)
{
    Uint16 menu0Number;             // 0���˵���ʾ��ֵ
    union FUNC_ATTRIBUTE attributeMenu0;

    UpdateMenu0DispStatus();

    if (MENU0_DISP_STATUS_RUN_STOP == menu0DispStatus)
    {
        if (bFrqDigital || bFrqDigitalDone4WaitDelay)   // �����޸��趨Ƶ��, �����޸�֮����һ��ʱ����
        {
            if (!bFrqDigital)
            {
                if (++accDecFrqTicker >= UP_DOWN_DEAL_DONE_TIME / 12)     // _*12ms. ��0���˵�����UP/DOWN�ı�Ƶ����ɺ���ͣһ��
                {
                    accDecFrqTicker = 0;
                    bFrqDigitalDone4WaitDelay = 0;
                }
            }

            menu0Number = frqAimDisp;
            attributeMenu0 = funcCodeAttribute[MAX_FRQ_INDEX].attribute;    // ��ʾ������MAX_FRQһ��
            attributeMenu0.bit.point = funcCode.code.frqPoint;
//                menuAttri[MENU_LEVEL_0].winkFlag = 0;        // ��0���˥�°���UP/DOWNʱ, ֮��_ʱ��Ҳ����˸
        }
        else
        {
            Uint16 bitDisp;

            if (runFlag.bit.run) // ����ʱLED��ʾ
            {
                bitDisp = funcCode.code.dispParaRunBit;
            }
            else                      // ͣ��ʱLED��ʾ
            {
                if (funcCode.code.dispParaStopBit >= STOP_DISPLAY_NUM)
                {
                    funcCode.code.dispParaStopBit = 0;
                }
                bitDisp = stopDispIndex[funcCode.code.dispParaStopBit];

                menuAttri[MENU_LEVEL_0].winkFlag = 0xf8; // 0���˵���ͣ��ʱȫ��˸
            }

            attributeMenu0 = dispAttributeU0[bitDisp];

            // ����Ƶ�ʡ��趨Ƶ����ʾ��С����
            if ((DISP_FRQ_RUN == bitDisp) 
                || (DISP_FRQ_AIM == bitDisp)
                || (DISP_FRQ_RUN_FDB == bitDisp)
                )
            {
                attributeMenu0.bit.point = funcCode.code.frqPoint;
            }
            // С����
            else if (DISP_OUT_CURRENT == bitDisp)  // �������
            {
                if (invPara.type > invPara.pointLimit)
                    attributeMenu0.bit.point--;
            }
            else if (DISP_LOAD_SPEED == bitDisp) // �����ٶ���ʾ
            {
                attributeMenu0.bit.point = funcCode.code.speedDispPointPos; // �ٶ���ʾС����λ
            }
            
            menu0Number = funcCode.group.u0[bitDisp];          
            
        }

        UpdateDisplayBufferAttribute(menu0Number, attributeMenu0);
    }
    else if (MENU0_DISP_STATUS_ERROR == menu0DispStatus)    // ����/�澯��ʾ
    {
        UpdateErrorDisplayBuffer();
    }
    else if (MENU0_DISP_STATUS_TUNE == menu0DispStatus)     // ��г��ʾ
    {
        UpdateTuneDisplayBuffer();
    }
}


// ����0���˵���ʾ״̬
void UpdateMenu0DispStatus(void)
{
    static Uint16 errorCodeOld4Menu0;
    static Uint16 tuneCmdOld4Menu0;

    if ((!errorCodeOld4Menu0)
        && errorCode
        )       // ���޹��ϵ��й���
    {
        menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // �������/�澯��ʾ״̬
    }
    errorCodeOld4Menu0 = errorCode;

    if ((!tuneCmdOld4Menu0)
        && tuneCmd
        )
    {
        menu0DispStatus = MENU0_DISP_STATUS_TUNE;   // �����г��ʾ
    }
    tuneCmdOld4Menu0 = tuneCmd;

    if ((!errorCode)
        && (!tuneCmd)
        )
    {
        menu0DispStatus = MENU0_DISP_STATUS_RUN_STOP;
    }
}


//=====================================================================
//
// 1���˵��µ���ʾ������º���
//
//=====================================================================
LOCALF void UpdateMenu1DisplayBuffer(void) // 1���˵���ʾ��@@@FX
{
    UpdateGroupDisplay(curFc.group);

// �������ʾ
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[3] = DISPLAY_CODE[groupDisplay.dispF];
    displayBuffer[4] = DISPLAY_CODE[groupDisplay.disp0];

//    menuAttri[MENU_LEVEL_1].winkFlag = 0x08;    // ��˸���һλ����MD320��ͬ
}


//=====================================================================
//
// 2���˵��µ���ʾ������º���
//
//=====================================================================
LOCALF void UpdateMenu2DisplayBuffer(void) // ��ʾ FX-XX
{
    Uint16 digit[5];
    Uint16 flag = 0;

    if ((MENU_MODE_CHECK == menuMode)
        && (checkMenuModeSerachNone)
        )
    {
        flag = 1;
    }
    else if ((MENU_MODE_USER == menuMode)
        && (0 == funcCode.code.userCustom[userMenuModeFcIndex])
        )
    {
        flag = 2;
    }

    if (!flag)
    {
        UpdateGroupDisplay(curFc.group);

        GetNumberDigit(digit, curFc.grade, DECIMAL);

        // �������ʾ
        displayBuffer[0] = DISPLAY_CODE[groupDisplay.dispF];
        displayBuffer[1] = DISPLAY_CODE[groupDisplay.disp0];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[3] = DISPLAY_CODE[digit[1]];
        displayBuffer[4] = DISPLAY_CODE[digit[0]];

        if (MENU_MODE_CHECK == menuMode)        // �ǳ���ֵģʽ
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_c];// & DISPLAY_CODE[DISPLAY_DOT];
            displayBuffer[1] = DISPLAY_CODE[groupDisplay.dispF];
            displayBuffer[2] = DISPLAY_CODE[groupDisplay.disp0] & DISPLAY_CODE[DISPLAY_DOT];
        }
        else if (MENU_MODE_USER == menuMode)    // �û�����ģʽ
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_u];// & DISPLAY_CODE[DISPLAY_DOT];
            displayBuffer[1] = DISPLAY_CODE[groupDisplay.dispF];
            displayBuffer[2] = DISPLAY_CODE[groupDisplay.disp0] & DISPLAY_CODE[DISPLAY_DOT];
        }

        menuAttri[MENU_LEVEL_2].winkFlag = 0x01 << (3 + menuAttri[MENU_LEVEL_2].operateDigit);
    }
    else
    {
        // �������ʾ
        if (1 == flag)
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_c] & DISPLAY_CODE[DISPLAY_DOT];
        }
        else
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_u] & DISPLAY_CODE[DISPLAY_DOT];
        }
        
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_N];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_U];
        displayBuffer[3] = DISPLAY_CODE[DISPLAY_L];
        displayBuffer[4] = DISPLAY_CODE[DISPLAY_L];
    }
}


//=====================================================================
//
// 3���˵��µ���ʾ������º���
// ����displayBuffer[]
//
//=====================================================================
LOCALF void UpdateMenu3DisplayBuffer(void)
{
    union FUNC_ATTRIBUTE attribute;
    Uint16 tmp = menu3Number;

#if (DEBUG_F_DISP_DIDO_STATUS_SPECIAL)
// DIDO״ֱ̬����ʾ
    if ((curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_STATUS_SPECIAL1])) || 
        (curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DO_STATUS_SPECIAL1]))
        )
    {
        Uint32 value;

        // DI����״ֱ̬����ʾ��DI1-DI10, VDI1-VDI5, AI1asDI-AI3asDI
        if (GetCodeIndex(funcCode.group.u0[DISP_DI_STATUS_SPECIAL1]) == curFc.index)
        {
            value = diStatus.a.all & 0x000FFFFF;
        }
        // DO����״ֱ̬����ʾ��FMR,RELAY1,RELAY2,DO1,DO2,VDO1-VDO5
        else if (GetCodeIndex(funcCode.group.u0[DISP_DO_STATUS_SPECIAL1]) == curFc.index)
        {
            value = doStatus.a.all & 0x000FFFFF;
        }
        
        UpdateDisplayBufferVisualIoStatus(value);
        
        return;
    }
// DI����״ֱ̬����ʾ
    else if ((curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL1])) ||
             (curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL2]))
             )
    {
        Uint16 high;        // ��8λ
        Uint32 low;         // ��32λ

        // DI����״ֱ̬����ʾ1��diFunc1-diFunc40
        if (GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL1]) == curFc.index)
        {
            high = (diFunc.f2.all >> 1) & 0x00FF;
            low = ((diFunc.f2.all & 0x0001) << 31) + (diFunc.f1.all >> 1);
        }
        // DI����״ֱ̬����ʾ2��diFunc41-diFunc80
        else if (GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL2]) == curFc.index)
        {
            high = 0;
            low = diFunc.f2.all >> 9;
        }
        
        UpdateDisplayBufferVisualDiFunc(high, low);
        
        return;
    }
#endif

    if (FUNCCODE_GROUP_U0 == curFc.group)               // U0����ʾ
    {
        attribute = dispAttributeU0[curFc.grade];      
        //funcCode.all[curFc.index] = *pDispValueU0[curFc.grade]; // UpdateU0Data()�и���
    }
    
#if DEBUG_F_POSITION_CTRL
    else if (FUNCCODE_GROUP_U0 + 1 == curFc.group)          // U1����ʾ
    {
        attribute = dispAttributeU1[curFc.grade];
    }
#endif
#if DEBUG_F_MOTOR_FUNCCODE
    else if (FUNCCODE_GROUP_UF == curFc.group)          // UF����ʾ
    {
        attribute.all = UF_VIEW_ATTRIBUTE;
    }
#endif
#if DEBUG_F_PLC_CTRL
    else if (FUNCCODE_GROUP_U0 + 3 == curFc.group)          // U3����ʾ
    {
        attribute.all = 0x1040;
    }
#endif
    else    // �������飬����U1��
    {
        attribute = funcCodeAttribute[curFc.index].attribute;
    }

    // ���������ͨ������г�����벻��˸��
    if (((TUNE_CMD_INDEX_1 == curFc.index) 
        || (TUNE_CMD_INDEX_2 == curFc.index)
        || (TUNE_CMD_INDEX_3 == curFc.index)
        || (TUNE_CMD_INDEX_4 == curFc.index)
        )
// 380Ŀǰ������ͨѶ��г
        && (FUNCCODE_runSrc_PANEL != runSrc)
        )
    {
        attribute.bit.writable = ATTRIBUTE_READ_ONLY_ANYTIME;
    }

    if (IsWritable(attribute))             // ��ǰ���޸�
    {
        menuAttri[MENU_LEVEL_3].winkFlag = 0x01 << (3 + menuAttri[MENU_LEVEL_3].operateDigit);
    }
    else                                    // ��ǰ�����޸�
    {
        tmp = funcCode.all[curFc.index];    // ��ʱ������ʾΪ�������ֵ
    }

    
    // ���һ�ι���Ƶ��
    if (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = funcCode.code.errorFrqUnit & 0x000F;
    }
    // �ڶ��ι���Ƶ��
    else if (GetCodeIndex(funcCode.code.errorScene2.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 4) & 0x000F;
    }
    // ��һ�ι���Ƶ��
    else if (GetCodeIndex(funcCode.code.errorScene1.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 8 ) & 0x000F;
    }
    // �����ٶ���ʾ
    else if(GetCodeIndex(funcCode.group.u0[DISP_LOAD_SPEED]) == curFc.index)
    {
        attribute.bit.point = funcCode.code.speedDispPointPos;
    }  
#if DEBUG_ACC_DEC_TIME_POINT
    // �Ӽ���ʱ���С����
    else if ((GetCodeIndex(funcCode.code.accTime1) == curFc.index) ||
        (GetCodeIndex(funcCode.code.accTime2) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.accTime3) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.accTime4) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.jogAccTime) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime1) == curFc.index) ||
        (GetCodeIndex(funcCode.code.decTime2) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime3) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime4) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.jogDecTime) == curFc.index) 
        )
    {
        attribute.bit.point = funcCode.code.accDecTimeUnit;
    }
#endif    
// Ƶ��ָ�λ
#if DEBUG_FRQ_POINT
    else if (((attribute.bit.point == 2)  // ��λΪ0.01Hz����F4-12 ����UP/DN����
        && (attribute.bit.unit == 1)) 
        || (GetCodeIndex(funcCode.code.diUpDownSlope)  == curFc.index)
        )
    {
        attribute.bit.point -= (2 - funcCode.code.frqPoint);
    }

#endif    
// ����������ص��������ϵ�����U0-04��ʾ����
    else if ((GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.errorScene3.elem.errorCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.errorScene2.elem.errorCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.group.u0[DISP_OUT_CURRENT]) == curFc.index)         
        )
    {
        if (invPara.type > invPara.pointLimit)
            attribute.bit.point--;
    }	    
// �������
    else if ((GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmRs) == curFc.index) ||   // ͬ�������ӵ���
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLd) == curFc.index) ||   // ͬ����d����
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLq) == curFc.index) ||   // ͬ����q����
        
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmRs) == curFc.index) ||   // ͬ�������ӵ���
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmLd) == curFc.index) ||   // ͬ����d����
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmLq) == curFc.index) ||   // ͬ����q����
       
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmRs) == curFc.index) ||   // ͬ�������ӵ���
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmLd) == curFc.index) ||   // ͬ����d����
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmLq) == curFc.index) ||   // ͬ����q����
       
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmRs) == curFc.index) ||   // ͬ�������ӵ���
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmLd) == curFc.index) ||   // ͬ����d����
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmLq) == curFc.index)      // ͬ����q����
        )
    {
        if (invPara.type > invPara.pointLimit)
        {
            attribute.bit.point++;
        }
    }
    
    UpdateDisplayBufferAttribute(tmp, attribute);
}


//=====================================================================
//
// �������ԣ�������ʾ���ݻ���ģ����š�λ����С����͵�λ
//
//=====================================================================
LOCALF void UpdateDisplayBufferAttribute(const Uint16 data, union FUNC_ATTRIBUTE attribute)
{
    static Uint16 digit[5];
    int16 i;
    Uint16 digits;              // data��λ��
    Uint16 bMinus = 0;          // �Ƿ���ʾ���ű�־��1-Ҫ��ʾ����
    Uint16 dataTmp;
    Uint16 a,b;
    Uint16 mode;

// 1. ��ʾλ������ʾ����
    // �з��ţ���Ϊ��������Ҫ��ʾ����-
    if ((attribute.bit.signal) // ���ţ�unsignal->0; signal->1.
        && ((int16)(data) < 0))
    {
        dataTmp = -(int16)(data);
        bMinus = 1;

         // Ϊ�з���������ֵΪ������ʾֵ����4λ
        if ((attribute.bit.point) 
            && ((int16)data < (-9999))
            )
        {
            attribute.bit.point--;
            dataTmp = dataTmp/10;
        }
    }
    else
    {
        dataTmp = data;
    }

// ��ȡÿһλ����ʾֵ
    a = digit[1];   // ����
    b = digit[0];

    mode = DECIMAL;
    if (ATTRIBUTE_MULTI_LIMIT_HEX == attribute.bit.multiLimit)  // ʮ������Լ��
    {
        mode = HEX;
    }
    digits = GetNumberDigit(digit, dataTmp, mode);

    if (ticker4LowerDisp)  // ��0���˵�������0���˵������2λ���˸���ʱ�䣬����0���˵�  ����shift
    {
        digit[1] = a;
        digit[0] = b;
    }

// �û����ƹ����������ʾ
#if DEBUG_F_USER_MENU_MODE
    // ��ʾ uFx.yz
    if ((USER_MENU_GROUP == curFc.group) &&      // �û����ƹ�������
        (MENU_LEVEL_3 == menuLevel))
    {
        Uint16 group;

        digit[4] = DISPLAY_u;

        group = menu3Number / 100;
        UpdateGroupDisplay(group);
        digit[3] = groupDisplay.dispF;
        digit[2] = groupDisplay.disp0;

        attribute.bit.point = 2;
    }
#endif

    if ((MENU_LEVEL_0 == menuLevel) // 0���˵�������ʾ����ǰ�����
        || ((MENU_LEVEL_3 == menuLevel) && (curFc.group >= FUNCCODE_GROUP_U0)) // U�飬��ʾ
            )
    {
        if (mode == HEX)
        {
            if (attribute.bit.displayBits < DISPLAY_8LED_NUM)
            {
                // ʮ��������ʾ��ǰ׺ H.
                digits = attribute.bit.displayBits + 1;
                digit[attribute.bit.displayBits] = DISPLAY_H;    // ��ʾH
                attribute.bit.point = attribute.bit.displayBits; // ��ʾH.
            }
        }

        if (++ticker4LowerDisp >= DECIMAL_DISPLAY_UPDATE_TIME)  // С�����2λ����ʱ�䣬_*12ms
        {
            ticker4LowerDisp = 0;
        }
    }
    else //if (MENU_LEVEL_3 == menuLevel)    // 0��3���˵��Ż���ñ�����
    {
        ticker4LowerDisp = 0;

        // ʮ����������
        if (mode == HEX)
        {
            // �п��������
            if (curFcDispDigits < DISPLAY_8LED_NUM)
            {
                // 3���˵���ʮ������������ʾ��ǰ׺ H.
                digits = curFcDispDigits + 1;          // ����һλ��ʾλ
                digit[curFcDispDigits] = DISPLAY_H;    // ��ʾ����ΪH
                attribute.bit.point = curFcDispDigits; // ��ʾ����ΪH.
            }
            // �������ʾ���޿��࣬����ʾH.
            else
            {
                digits = curFcDispDigits;       // 3���˵�
            }
        }
        // ʮ��������
        else
        {
            digits = curFcDispDigits;          // 3���˵�
        }
    }
    
    if (attribute.bit.point >= digits)  // ������ʾ��С����λ��
    {
        digits = attribute.bit.point + 1;
    }

    for (i = DISPLAY_8LED_NUM - 1; i >= 0; i--) // ��ʾλ��
    {
        if (i < digits)
        {
            displayBuffer[(DISPLAY_8LED_NUM - 1) - i] = DISPLAY_CODE[digit[i]];
        }
        else
        {
            displayBuffer[(DISPLAY_8LED_NUM - 1) - i] = DISPLAY_CODE[DISPLAY_NULL];
        }
    }
    
    if (bMinus)                 // ��ʾ����-
    {
        int16 tmp = DISPLAY_8LED_NUM - 1 - digits;
        if (tmp < 0)
            tmp = 0;

        if (!digit[4])  // ���λΪ0
        {
            displayBuffer[tmp] = DISPLAY_CODE[DISPLAY_LINE];
        }
        else
        {
            ///displayBuffer[(DISPLAY_8LED_NUM - 1)] &= DISPLAY_CODE[DISPLAY_DOT];// ���1λLED��ʾС����
            //displayBuffer[tmp] = DISPLAY_CODE[DISPLAY_LINE_1];
            //displayBuffer[tmp] &= 0xdf;
            //displayBuffer[tmp] ^= 0x40;
            //displayBuffer[tmp] ^= ~0xBf;
            //displayBuffer[tmp] ^= 0x02;
            attribute.bit.point = 0;
        }
    }


// 2. ��ʾС����
    if (attribute.bit.point)
    {
        displayBuffer[(DISPLAY_8LED_NUM - 1) - attribute.bit.point] &= DISPLAY_CODE[DISPLAY_DOT];
    }

// 3. ��ʾ��λ
    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_HZ_BIT))   // ��ʾ��λ,Hz
    {
        displayBuffer[5] &= LED_CODE[LED_HZ];   // ��
    }

    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_A_BIT))    // ��ʾ��λ,A
    {
        displayBuffer[5] &= LED_CODE[LED_A];
    }

    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_V_BIT))    // ��ʾ��λ,V
    {
        displayBuffer[5] &= LED_CODE[LED_V];
    }
}


//=====================================================================
//
// ��������ʾ
//
//=====================================================================
void UpdateErrorDisplayBuffer(void)
{
    Uint16 digit[5];

    GetNumberDigit1(digit, errorCode);

        // �������ʾ
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_E];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_r];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_r];
    displayBuffer[3] = DISPLAY_CODE[digit[1]];
    displayBuffer[4] = DISPLAY_CODE[digit[0]];

    if ((runFlag.bit.run)    // �澯
        || (ERROR_LEVEL_RUN == errorAttribute.bit.level)
        )
    {
        displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_A];
    }
}


//=====================================================================
//
// ��г��ʾ
//
//=====================================================================
LOCALF void UpdateTuneDisplayBuffer(void)
{
    // �������ʾ
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_T];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_U];
    displayBuffer[3] = DISPLAY_CODE[DISPLAY_N];
    displayBuffer[4] = DISPLAY_CODE[DISPLAY_E];
}


//=====================================================================
//
// ���룺
// *data  Ҫ�޸ĵ����ݵĵ�ǰֵ
//
// �����
// *data  �޸�֮�������
//
// ������
// index  �޸Ĺ������index
// delta  ������������UP��������DOWN��0��ͨѶ���á�
//
// ���أ�
// COMM_ERR_NONE
// COMM_ERR_PARA       ��Ч����
// COMM_ERR_READ_ONLY  ����������Ч
//
//=====================================================================
Uint16 ModifyFunccodeUpDown(Uint16 index, Uint16 *data, int16 delta)
{
    union FUNC_ATTRIBUTE attribute = funcCodeAttribute[index].attribute;
    Uint16 upper;
    Uint16 lower;
    Uint16 tmp = *data;
    Uint16 flag = COMM_ERR_NONE;
    int16 i;
    int16 flag1 = 0;
    Uint16 multiLimit;

// ������д���ԣ�0x-����ֻ����10-������ֻ����11-���Զ�д
    if (IsWritable(attribute))
    {
#if DEBUG_F_USER_MENU_MODE
        if ((USER_MENU_GROUP == curFc.group)     // FE�飬�û����ƹ�����
		|| ((index >= GetCodeIndex(funcCode.code.userCustom[0])) 
			&& (index <= GetCodeIndex(funcCode.code.userCustom[FENUM-1])))
		)
        {
            Uint16 group = *data / 100;
			
			if ((group >= FUNCCODE_GROUP_NUM) || (group == USER_MENU_GROUP))
			{
				return COMM_ERR_PARA;
			}
			
            upper = group * 100 + funcCodeGradeAll[group] - 1;
            lower = group * 100 + 0;

			if (upper < lower)
			{
				return COMM_ERR_PARA;
			}
        }
        else
#endif
        {
            upper = funcCodeAttribute[index].upper;
            if (attribute.bit.upperLimit)
                upper = funcCode.all[upper];

            // S���ߵ���ʼ�κͽ�����֮�����Ϊ1000.
            // sCurveStartPhaseTime + sCurveEndPhaseTime <= 100.0%
            if (index == GetCodeIndex(funcCode.code.sCurveStartPhaseTime))
            {
                upper = 1000 - funcCode.code.sCurveEndPhaseTime;
            }
            else if (index == GetCodeIndex(funcCode.code.sCurveEndPhaseTime))
            {
                upper = 1000 - funcCode.code.sCurveStartPhaseTime;
            }

            lower = funcCodeAttribute[index].lower;
            if (attribute.bit.lowerLimit)
                lower = funcCode.all[lower];

            // �ù�����Ϊ�������������
            multiLimit = funcCodeAttribute[index].attribute.bit.multiLimit;

            if ((ATTRIBUTE_MULTI_LIMIT_SINGLE != multiLimit) && (!delta))  // ��Ϲ�������ͨѶ����
            {
                Uint16 dataDigit[5],upperDigit[5], lowerDigit[5];
                Uint16 bit;
               // const int16 *p = decNumber;
                Uint16 mode = DECIMAL;

                if (ATTRIBUTE_MULTI_LIMIT_HEX == multiLimit)
                {
                    //p = hexNumber;
                    mode = HEX;
                }

				GetNumberDigit(dataDigit, *data, mode);
				GetNumberDigit(upperDigit, upper, mode);
				GetNumberDigit(lowerDigit, lower, mode);

                for (bit = 0; bit < 5; bit++)
                {
                    if ((dataDigit[bit] > upperDigit[bit]) 
                    || (dataDigit[bit] < lowerDigit[bit]))
					{
                        return COMM_ERR_PARA;
					}
                }

                return COMM_ERR_NONE;
            }
                
            
            if ((ATTRIBUTE_MULTI_LIMIT_SINGLE != multiLimit)   // ��Ϲ������ҷ�ͨѶ����
                && (delta)
                )
            {
                Uint16 digit[5];
                Uint16 tmp;
                Uint16 bit = menuAttri[MENU_LEVEL_3].operateDigit;
                const int16 *p = decNumber;
                Uint16 mode = DECIMAL;

                if (ATTRIBUTE_MULTI_LIMIT_HEX == multiLimit)
                {
                    p = hexNumber;
                    mode = HEX;
                }

                GetNumberDigit(digit, *data, mode);
                tmp = *data - digit[bit] * (*(p+bit));

                GetNumberDigit(digit, upper, mode);
                upper = tmp + digit[bit] * (*(p+bit));

                GetNumberDigit(digit, lower, mode);
                lower = tmp + digit[bit] * (*(p+bit));
            }

#if DEBUG_F_INV_TYPE_RELATE
            // ���Ͳ��ܳ�����Χ
            if (GetCodeIndex(funcCode.code.inverterType) == index)  // FF-01
            {
                if ((100 != delta) && (-100 != delta))   // û�����޸İ�λ
                {
                    Uint16 i;
                    i = *data / 100;

                    upper = invTypeLimitTable[i].upper + (i * 100);
                    lower = invTypeLimitTable[i].lower + (i * 100);
                }
            }
            else 
#endif
            // ���Ƶ�ʣ���ͬƵ��ָ��С����ķ�ΧӦ�ò�ͬ��
            if (GetCodeIndex(funcCode.code.maxFrq) == index)
            {
                lower = 50 * decNumber[funcCode.code.frqPoint];
            }
        }

// �����޴���
#if DEBUG_F_NO_SAME
//--------------------------------------------------------
// NoSameDeal()
        // ��Ƶ��ԴX�͸�Ƶ��ԴY���趨ֵ����һ��
        for (i = sizeof(frqSrcFuncIndex) - 1; i >= 0; i--)
        {
            if (frqSrcFuncIndex[i] == index)
            {
                *data = NoSameDeal(index, frqSrcFuncIndex, sizeof(frqSrcFuncIndex), *data, upper, lower, delta);
                flag1 = 1;
            }

            if (flag1)
                break;
        }

        // DI(DI, VDI, AiAsDi)���ӵĹ��ܣ������ظ�
        for (i = sizeof(diFuncIndex) - 1; i >= 0; i--)
        {
            if (diFuncIndex[i] == index)
            {
                *data = NoSameDeal(index, diFuncIndex, sizeof(diFuncIndex), *data, upper, lower, delta);
                flag1 = 1;
            }

            if (flag1)  // ����ȵ�index������
                break;
        }
//--------------------------------------------------------
#endif
        if (!flag1)
        {
            *data = LimitDeal(attribute.bit.signal, *data, upper, lower, delta);
        }

        if (*data != tmp)       // ͨѶ����ʱ���������ݱ����ģ�����������
            flag = COMM_ERR_PARA;
    }
    else
    {
        flag = COMM_ERR_READ_ONLY;
    }

    return flag;
}




//=====================================================================
//
// ������
// index  Ҫ�޸Ĺ������index
// *data  ����
//
// ���أ�
// COMM_ERR_NONE
// COMM_ERR_PARA       ��Ч����
//
//=====================================================================
Uint16 ModifyFunccodeEnter(Uint16 index, Uint16 dataNew)
{
    Uint16 dataOld;
    Uint16 ret = COMM_ERR_NONE;

    dataOld = funcCode.all[index];
    funcCode.all[index] = dataNew;        // ���浽RAM


    if (dataOld != dataNew)
    {
        // ������ĳЩ�����룬��Ҫ�޸�һЩ����
        // F0-07, Ƶ��Դѡ��
        // F0-08, Ԥ��Ƶ��
        if ((GetCodeIndex(funcCode.code.presetFrq) == index) ||
            (GetCodeIndex(funcCode.code.frqCalcSrc) == index))
        {
            // �����趨Ƶ��Դ���޸���Ԥ��Ƶ�ʣ�Ҫ��Ӧ�޸��趨Ƶ�ʡ�����Ԥ��Ƶ�ʺ�enter���޸�
            ResetUpDownFrq();
        }
#if DEBUG_F_MOTOR_POWER_RELATE
        else if (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower) == index)         // �޸ĵ������ʣ������޸���ع�����
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_1);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingPower) == index) // �޸ĵ������ʣ������޸���ع�����
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_2);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingPower) == index) // �޸ĵ������ʣ������޸���ع�����
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_3);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingPower) == index) // �޸ĵ������ʣ������޸���ع�����
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_4);
        }   
#endif
        else if (GetCodeIndex(funcCode.code.menuMode) == index)  // �޸Ĳ˵�����ģʽ
        {
            Uint16 digit[5];
            GetNumberDigit(digit, menu3Number, DECIMAL);
        		menuModeTmp = menuMode;
        		// ��ǰ�˵�����ģʽѡ����Ч��
        		if (((menuModeTmp == MENU_MODE_USER) && (!digit[0]))
                  || ((menuModeTmp == MENU_MODE_CHECK) && (!digit[1])))
        		{
        			// ���ĵ�ǰ�˵�����ģʽΪ����ѡ��ģʽ
        			menuModeTmp = MENU_MODE_BASE;
        			MenuModeSwitch();
        		}
        }
        // ���¹��ܲ�������ʾ��������
        else if ((GetCodeIndex(funcCode.code.aiaoCalibrateDisp) == curFc.index)
            || (GetCodeIndex(funcCode.code.funcParaView) == curFc.index))
        {
            MenuModeDeal();
        }
#if DEBUG_F_INV_TYPE_RELATE
        else if (GetCodeIndex(funcCode.code.inverterType) == index) // �޸ı�Ƶ������ FF-01�������޸���ع�����
        {
            // ���Ͳ��ܳ�����Χ
            ret = ValidateInvType();
            if (!ret)  // ������Ч
            {
                InverterTypeRelatedParaDeal();
            }
        }
#endif
#if 0
        else if (GetCodeIndex(funcCode.code.commOverTime) == index) // FA-04, ͨѶ��ʱʱ��
        {
            if ((dataNew) && (!dataOld))  // 0->��0
                commTicker = 0;  // �޸���ͨѶ��ʱʱ�䣬���¿�ʼ��ʱ��
        }
#endif
    }

    // ��DI5���Զ���ΪDI_FUNC_APTP_ZERO
    if ((GetCodeIndex(funcCode.code.diFunc[0]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[1]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[2]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[3]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[5]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[6]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[7]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[8]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[9]) == index)
        )
    {
        if (dataNew == DI_FUNC_APTP_ZERO)
        {
            ret = COMM_ERR_PARA;
        }
    }
#if DEBUG_F_GROUP_HIDE
    else if (GetCodeIndex(funcCode.code.funcParaView) == curFc.index)
    {
        MenuModeDeal();
    }
#endif
#if DEBUG_F_MOTOR_FUNCCODE1
    // ���ܵ��Թ�����
    else if (GetCodeIndex(funcCode.code.motorDebugFc) == curFc.index)
    {
        MenuModeDeal();
    }
#endif
    // AIAOУ������������
    else if (GetCodeIndex(funcCode.code.aiaoCalibrateDisp) == curFc.index)
    {
        MenuModeDeal();
    }
    else if ((GetCodeIndex(funcCode.code.ledDispParaRun1) == index) ||
        (GetCodeIndex(funcCode.code.ledDispParaRun2) == index) ||
        (GetCodeIndex(funcCode.code.ledDispParaStop) == index)
        )
    {
        cycleShiftDeal(0);      // 0���˵���ʾ��ѭ����λ����
    }
    // Ƶ��ָ��С����
    else if (GetCodeIndex(funcCode.code.frqPoint) == index)
    {
        if (funcCode.code.maxFrq < 50 * decNumber[funcCode.code.frqPoint])
        {
            funcCode.code.maxFrq = 50 * decNumber[funcCode.code.frqPoint];

            // ĳЩ�����������������������޵Ĵ���
            LimitOtherCodeDeal(MAX_FRQ_INDEX);   // ���Ƶ��
        }
    }
#if (DEBUG_F_POSITION_CTRL)
    // aptp���������ӣ�ʹ���ж�
    // ��ΪDI5ΪHDI
    else if (GetCodeIndex(funcCode.code.diFunc[4]) == index)
    {
        // �ı��aptp�������
        if ((dataOld != DI_FUNC_APTP_ZERO) && (dataNew == DI_FUNC_APTP_ZERO))
        {
            InitSetEcap4WithInt();
            InitSetAptpZero();
        }
        else if ((dataOld == DI_FUNC_APTP_ZERO) && (dataNew != DI_FUNC_APTP_ZERO))
        {
            ;
        }
    }
    // FVC��PG��ѡ��, 1-QEP1,0-QEP2(��չ)
    // ���������λ�ÿ���PG�ĳ�ʼ��
    else if (GetCodeIndex(funcCode.code.pgParaM1.elem.fvcPgSrc) == index)
    {
        if ((dataOld != FUNCCODE_fvcPgSrc_QEP1) && (dataNew == FUNCCODE_fvcPgSrc_QEP1))
        {
            //InitEQep2Gpio();
            aptpAbsZeroOk = 0;
            pEQepRegsFvc = &EQep1Regs;   // FVC��PG��

            EALLOW;
            pEQepRegsFvc->QEPCTL.bit.IEL = 01;      // FVC QEPx Index event latch
            EDIS;

            InitSetPcEQep();
        }
        else if ((dataOld == FUNCCODE_fvcPgSrc_QEP1) && (dataNew != FUNCCODE_fvcPgSrc_QEP1))
        {
            //InitEQep1Gpio();
            aptpAbsZeroOk = 0;
            pEQepRegsPc = &EQep1Regs;    // λ�ÿ��Ƶ�PG��

            EALLOW;
            pEQepRegsFvc->QEPCTL.bit.IEL = 01;      // FVC QEPx Index event latch
            EDIS;

            InitSetPcEQep();
        }
    }
    // λ��ָ�������߼�
    else if (GetCodeIndex(funcCode.code.pcPulseLogic) == index)
    {
        // ��QEP_PC-B����
        pEQepRegsPc->QDECCTL.bit.QBP = dataNew;
    }
    // λ��ָ�����巽ʽ
    else if (GetCodeIndex(funcCode.code.pcPulseType) == index)
    {
        if (FUNCCODE_pcPulseType_PULSE_AND_DIR == funcCode.code.pcPulseType)    // ����+����
        {
            pEQepRegsPc->QDECCTL.bit.QSRC = 01; // Direction-count mode (QCLK = xCLK, QDIR = xDIR)
        }
        else if (FUNCCODE_pcPulseType_QUADRATURE == funcCode.code.pcPulseType)  // 2·��������
        {
            pEQepRegsPc->QDECCTL.bit.QSRC = 00; // quadrature count mode
        }
    }
    // λ��ָ������
    else if (GetCodeIndex(funcCode.code.pcPulseSwap) == index)
    {
        pEQepRegsPc->QDECCTL.bit.SWAP = dataNew;
    }
    // �ٶȷ���AB����
#if 0    // AB���򽻸����ܴ���
    else if (GetCodeIndex(funcCode.code.fvcPgLogic) == index)
    {
        // ��QEP_�ٶȿ���-AB����
        pEQepRegsFvc->QDECCTL.bit.SWAP = dataNew;
    }
#endif
    // �ı����, aptpAbsZeroOk����
    else if (GetCodeIndex(funcCode.code.pcZeroSelect) == index)
    {
        if (dataOld != dataNew)
        {
            aptpAbsZeroOk = 0;
        }
    }
#endif
    else
    {
        LimitOtherCodeDeal(index);      // ĳЩ�����������������������޵Ĵ���
    }

    if (COMM_ERR_PARA == ret)           // ������Ч����ָ�֮ǰ��ֵ
    {
        funcCode.all[index] = dataOld; // �ָ�
    }

    return ret;
}


//=====================================================================
//
// ��ʾ���ݴ���Ŀǰ2ms����1��
//
//=====================================================================
#define OUT_TORQUE_FRQ_DISP_FILTER_TIME 75  // ���Ƶ���˲�ʱ��ϵ��
LOCALF LowPassFilter torQueFrqDispLpf = LPF_DEFALUTS;
void DispDataDeal(void)
{
// ����Ƶ����ʾ��
// ��ʾ���ܴ��ݸ����ܵ�˲ʱֵ�����������ܵķ���Ƶ��(��������ʵ�ʷ��͵�Ƶ��)
    frqDisp = ABS_INT32(frqDroop);

#if DEBUG_F_PLC_CTRL
    frqPLCDisp = (int16)(frqDroop*10000/maxFrq);
#endif


    if (RUN_MODE_TORQUE_CTRL == runMode)    // ת�ؿ���
    {
        torQueFrqDispLpf.t = OUT_TORQUE_FRQ_DISP_FILTER_TIME;
        torQueFrqDispLpf.in = ABS_INT32(frqRun);
        torQueFrqDispLpf.calc(&torQueFrqDispLpf);
        frqDisp = torQueFrqDispLpf.out;

#if DEBUG_F_PLC_CTRL
        frqPLCDisp = (int16)(torQueFrqDispLpf.out*10000/maxFrq);
#endif
    }

#if DEBUG_F_PLC_CTRL
     if (funcCode.code.runDir == FUNCCODE_runDir_REVERSE)
     {
        frqPLCDisp = -((int16)frqPLCDisp);
     }
#endif    

// �趨Ƶ����ʾ����ԾƵ��֮ǰ���㶯֮���ֵ
// ����UpdateFrqAim()�С�

    pidFuncRefDisp = ((Uint32)funcCode.code.pidDisp * ABS_INT32(pidFunc.ref) + (1 << 14)) >> 15;
    pidFuncFdbDisp = ((Uint32)funcCode.code.pidDisp * ABS_INT32(pidFunc.fdb) + (1 << 14)) >> 15;

    // �����ٶ�
    // ͣ��ʱ��ʾ�趨Ƶ��*ϵ��
    // ����ʱ��ʾ����Ƶ��*ϵ��
    if (runFlag.bit.run)
    {
        loadSpeedDisp = (ABS_INT32(frq) * funcCode.code.speedDispCoeff) / 10000;
    }
    else
    {
        loadSpeedDisp = (ABS_INT32(frqAim) * funcCode.code.speedDispCoeff) / 10000;
    }
    
    pulseInFrqDisp = (pulseInFrq + 5) / 10;

    torqueCurrentAct = (Uint32)torqueCurrent * currentPu >> 12;
     
// ������ʼ��㣬����������ֱ�Ӽ���
#if 0//DEBUG_F_OUT_POWER
    //+============ С����
    outPower = (Uint32)torqueCurrentAct * outVoltageDisp * 135 / (100UL * 1000);   // ��λΪ0.1KW��Ч��Ĭ��Ϊ1.35/1.732
    itDisp = (Uint32)torqueCurrentAct * 1000 / motorFc.motorPara.elem.ratingCurrent;       // ���ת��
#endif

    funcCode.code.inverterGpTypeDisp = funcCode.code.inverterGpType; // F0-00, GP������ʾ

//  �ٶȷ���PG��AB����������
//    pEQepRegsFvc->QDECCTL.bit.SWAP = funcCode.code.fvcPgLogic;
}


#if 0
//=====================================================================
//
// �ָ������������ָ���RAM
//
//=====================================================================
void RestoreCompanyParaRamDeal(Uint16 i)
{
    Uint16 flag = 0;    // 0, �ָ�

    if (indexRestoreExceptSeries < RESTORE_COMPANY_PARA_EXCEPT_NUMBER)
    {
        if ((exceptRestoreSeries[indexRestoreExceptSeries].start <= i) &&
            (i <= exceptRestoreSeries[indexRestoreExceptSeries].end))
        {
            flag = 1;
        }
        if (i >= exceptRestoreSeries[indexRestoreExceptSeries].end)
        {
            indexRestoreExceptSeries++;
        }
    }

    if (indexRestoreExceptSingle < CLEAR_RECORD_NUM)   // ��Щ�����벻�ָ�
    {
        if (clearRecord[indexRestoreExceptSingle] == i)
        {
            flag = 1;
            indexRestoreExceptSingle++;
        }
    }

    if (!flag)
    {
        funcCode.all[i] = funcCodeInit.all[i]; // �ָ��ɳ�������
    }
}
#endif


// 0���˵������Ӳ˵�����
void Menu0AddMenuLevel(void)
{
    menuLevel = MENU_LEVEL_1;

    if ((MENU_MODE_CHECK == menuMode)
        || (MENU_MODE_USER == menuMode)
        )
    {
        Menu1OnEnter();
    }
}


//=====================================================================
//
// ȷ�����������ʾλ��
//
// ʵ����ʾλ����Ӧ��Ϊ:
// 1. �޷���
//    ������ֵ��λ��
// 2. �з���
//    ���޺�������ֵ�ľ���ֵ�Ĵ�ֵ��λ��
//
//=====================================================================
Uint16 GetDispDigits(Uint16 index)
{
    Uint16 upper;
    Uint16 lower;
    Uint16 value;
    Uint16 digits;
    Uint16 mode;
    Uint16 digit[5];
    const FUNCCODE_ATTRIBUTE *p = &funcCodeAttribute[index];

// ���ʵ�ʵ�����

// ��ȡ������ֵ
    while (p->attribute.bit.upperLimit)
    {
        p = &funcCodeAttribute[p->upper];
    }
    upper = p->upper;
    
    if (!p->attribute.bit.signal)   // �޷���
    {
        value = upper;
    }
    else                            // �з���
    {
        // ��ȡ������ֵ
        while (p->attribute.bit.lowerLimit)
        {
            p = &funcCodeAttribute[p->lower];
        }
        lower = p->lower;

        // ȡ����ֵ
        upper = ABS_INT16((int16)upper);
        lower = ABS_INT16((int16)lower);

        // ȡ����ֵ�Ĵ�ֵ
        value = (upper > lower) ? upper : lower;
    }

// ��ȡ���޵�λ��
    mode = DECIMAL;

    if (ATTRIBUTE_MULTI_LIMIT_HEX == p->attribute.bit.multiLimit)   // ʮ������Լ��
    {
        mode = HEX;
    }

    digits = GetNumberDigit(digit, value, mode);

    return digits;
}


//=====================================================================
//
// ����currentGroup��������ʾ���飬��groupDisplay
// F0,��,FE,FF,FP,A0,��,AF,B0,��,BF,C0,��,CF,
//
//=====================================================================
void UpdateGroupDisplay(Uint16 group)
{
    Uint16 currentGroupDispF;
    Uint16 currentGroupDisp0;

// F0,��,FE,FF,FP,A0,��,AF,B0,��,BF,C0,��,CF,
    currentGroupDispF = DISPLAY_F;  //stamp:MD380_DISPLAY
    //currentGroupDispF = DISPLAY_E;    //
    if (group <= FUNCCODE_GROUP_FP)         // F0-FP
    {
        currentGroupDisp0 = group;

    }
    else if (group <= FUNCCODE_GROUP_CF)    // A0,��,AF,B0,��,BF,C0,��,CF,
    {
        Uint16 tmp1;
        Uint16 tmp2;

        tmp1 = (group - FUNCCODE_GROUP_FP - 1) / 16;
        tmp2 = (group - FUNCCODE_GROUP_FP - 1) % 16;

      currentGroupDispF = DISPLAY_A + tmp1; //MD380_DISPLAY
        //currentGroupDispF = DISPLAY_F + tmp1;//
        currentGroupDisp0 = tmp2;
    }
    else    // U��
    {
      currentGroupDispF = DISPLAY_U; //MD380_DISPLAY
        //currentGroupDispF = DISPLAY_P;//stamp:
        currentGroupDisp0 = group - FUNCCODE_GROUP_U0;
    }

    groupDisplay.dispF = currentGroupDispF;  
    groupDisplay.disp0 = currentGroupDisp0;  //MD380_DISPLAY

	//if((groupDisplay.dispF == DISPLAY_E)&&(groupDisplay.disp0 == 15)) // ע�͵��ָ� md380 
	//{
       // groupDisplay.dispF = DISPLAY_H;
       // groupDisplay.disp0 = 17;  //display HH
	//}
}


// group��UP/DOWN
Uint16 GroupUpDown(const Uint16 funcCodeGrade[], Uint16 group, Uint16 flag)
{
    group = LimitOverTurnDeal(funcCodeGrade, group, FUNCCODE_GROUP_NUM, FC_START_GROUP, flag);

    return group;
}


// �û����ƹ�����˵�ģʽ�Ĵ���
void DealUserMenuModeGroupGrade(Uint16 flag)
{
#if DEBUG_F_USER_MENU_MODE
    userMenuModeFcIndex = LimitOverTurnDeal(funcCode.code.userCustom, userMenuModeFcIndex, FENUM, 0, flag);

// ��λ��ʮλ��ʾ�û����Ƶ�grade
// ��λ��ǧλ��ʾ�û����Ƶ�group
    curFc.group = funcCode.code.userCustom[userMenuModeFcIndex] / 100;
    curFc.grade = funcCode.code.userCustom[userMenuModeFcIndex] % 100;
#endif
}


// data�ķ�Χ�� [low, upper-1]
Uint16 LimitOverTurnDeal(const Uint16 limit[], Uint16 data, Uint16 upper, Uint16 low, Uint16 flag)
{
    Uint16 loopNumber = 0;
    
    do
    {
        data += deltaK[flag];

        if ((int16)data >= (int16)upper)
        {
            data = low;
        }
        else if ((int16)data < (int16)low)
        {
            data = upper - 1;
        }

        loopNumber++;
    }
    while ((!limit[data]) 
        && (loopNumber < upper - low)
        );

    return data;
}


// У��˵�
#define CHECK_MENU_MODE_NUMBER_ONCE     70      // 1���ڵ�ѭ������
void DealCheckMenuModeGroupGrade(Uint16 flag)
{
#if DEBUG_F_CHECK_MENU_MODE
    int16 delta = 1;
    static Uint16 group;
    static Uint16 grade;
    Uint16 gradeTmp;
    Uint16 loopNumber = 0;
    Uint16 checkMenuModeFcIndex;

    if (CHECK_MENU_MODE_DEAL_CMD == checkMenuModeDealStatus)   // �µ�����ָ��
    {
        group = curFc.group;
        grade = curFc.grade;
    }
    checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_CMD;

    if (ON_DOWN_KEY == flag)    // ��������
    {
        delta = -delta;
    }

    do
    {
        gradeTmp = grade;
        if (funcCodeGradeCurMenuMode[group] > 1)    // ��group��Ч
        {
            grade = LimitDeal(0, grade, funcCodeGradeCurMenuMode[group] - 1, 0, delta); // funcCodeGradeAll
        }

        if (grade == gradeTmp)                      // grade������ֵ��Ҫ�ı�group
        {
            group = LimitOverTurnDeal(funcCodeGradeCurMenuMode, group, FUNCCODE_GROUP_U0 - 1, FUNCCODE_GROUP_F0, flag);

            if (ON_UP_KEY == flag)
            {
                grade = 0;
            }
            else
            {
                grade = funcCodeGradeCurMenuMode[group] - 1;    // ��group�����һ��grade
            }
        }

        checkMenuModeFcIndex = GetGradeIndex(group, grade);

        // 1���ڲ���ռ��̫��ʱ��
        // ������һ���ٽ�������
        if (++loopNumber >= CHECK_MENU_MODE_NUMBER_ONCE)        
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_SERACHING;
        }

        if ((curFc.grade == grade)      // if ֮ǰû��else
            && (curFc.group == group)
            )   // �ֻص�֮ǰ�Ĺ����룬˵��û�����������ֵ��ͬ�Ĺ�����
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_END_NONE;   // ����һ�飬û���ҵ��µ������ֵ��ͬ�Ĺ�����
            checkMenuModeSerachNone = 1;
        }
        
        // ���ǳ���ֵ�������صķǵ������������
        // if ֮ǰû��else
        if ((funcCode.all[checkMenuModeFcIndex] != GetFuncCodeInit(checkMenuModeFcIndex, 0)) &&
            (funcCodeAttribute[checkMenuModeFcIndex].attribute.bit.writable != 2))
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_END_ONCE;  // �ҵ��µ������ֵ��ͬ�Ĺ�����
            checkMenuModeSerachNone = 0;
        }
    }
    while (CHECK_MENU_MODE_DEAL_CMD == checkMenuModeDealStatus);

#if 0
    if (CHECK_MENU_MODE_DEAL_END_NONE == checkMenuModeDealStatus)  // û�й����������ֵ��ͬ����ʾF0-00
    {
        group = 0;
        grade = 0;
    }
#endif

    if (CHECK_MENU_MODE_DEAL_SERACHING != checkMenuModeDealStatus)  // �Ѿ��������
    {
        curFc.group = group;
        curFc.grade = grade;

        checkMenuModeCmd = 0;       // �������
        checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_CMD;         // ׼����һ������
    }
#endif
}



// �˵�ģʽ����
// ����funcCodeGradeCurrentMenuMode
void MenuModeDeal(void)
{
    Uint16 digit[5];

    UpdataFuncCodeGrade(funcCodeGradeCurMenuMode);
    GetNumberDigit1(digit, funcCode.code.menuMode);

    switch (menuMode)
    {
        case MENU_MODE_BASE:       // �����˵�����ҪΪĿǰ320������(����)
            break;       

#if DEBUG_F_USER_MENU_MODE
        case MENU_MODE_USER:        // �û����Ʋ˵�
            if (MENU_MODE_USER != menuModeOld)
            {
                DealUserMenuModeGroupGrade(ON_UP_KEY);
            }
            
            curFc.group = funcCode.code.userCustom[userMenuModeFcIndex] / 100;
            curFc.grade = funcCode.code.userCustom[userMenuModeFcIndex] % 100;
            break;
#endif

#if DEBUG_F_CHECK_MENU_MODE
        case MENU_MODE_CHECK:        // У�Բ˵�������ʾ�����ֵ��ͬ�Ĺ�����
            if (MENU_MODE_CHECK != menuModeOld)
            {
                checkMenuModeCmd = 1;
                checkMenuModePara = ON_UP_KEY;
            }
            break;
#endif
        default:
            break;
    }

}


void UpdataFuncCodeGrade(Uint16 funcCodeGrade[])
{
#if 1
    int16 i;
    Uint16 digit[5];

    memcpy(funcCodeGrade, funcCodeGradeAll, FUNCCODE_GROUP_NUM);
    
    GetNumberDigit(digit, funcCode.code.funcParaView, DECIMAL);  // �õ�ÿλ��ֵ��0 or 1

    // U����ʾ����
    if(digit[0] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_U0 + i] = 0;
        }
    }
    
    // A����ʾ����
    if(digit[1] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_A0 + i] = 0;
        }
    }
    
    // B����ʾ����
    if(digit[2] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_B0 + i] = 0;
        }
    }

    // C����ʾ����
    if(digit[3] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_C0 + i] = 0;
        }
    }

    // ���ܵ�����ʾ����
#if DEBUG_F_MOTOR_FUNCCODE
    MotorDebugFcDeal();
    funcCodeGrade[FUNCCODE_GROUP_CF] = motorDebugFc.fc; // CF�����
    funcCodeGrade[FUNCCODE_GROUP_UF] = motorDebugFc.u;  // UF�����
#endif
    
    // AIAOУ����������ʾ����
    if (funcCode.code.aiaoCalibrateDisp)
    {
        funcCodeGrade[FUNCCODE_GROUP_AE] = AENUM;
    }
#endif

}


//====================================================================
// ��λ(��λ��ʮλ����λ��ǧλ����λ)޸ĵ�ʮ���ƹ����룬ת��Ϊ������
// ÿλ����Ϊ0, 1
// ���磬������A6-06(����VDI���ӹ������趨��Ч״̬)��
// ��������ʾΪ 11101��ת��Ϊ 29��
//====================================================================
Uint16 FcDigit2Bin(Uint16 value)
{
    Uint16 tmp = 0;
    Uint16 digit[5];
    int16 i;

    GetNumberDigit(digit, value, DECIMAL);  // �õ�ÿλ��ֵ��0 or 1

    for (i = 5-1; i >= 0; i--)
    {
        if (digit[i])       //if (1 == digit[i])    // ���λΪ1.
        {
            tmp += 1 << i;
        }
    }

    return tmp;
}

//====================================================================
//
// ���ܵ��ԵĹ�������CF��UF��ĸ�������
//
//====================================================================
void MotorDebugFcDeal(void)
{
#if DEBUG_F_MOTOR_FUNCCODE
#if DEBUG_F_MOTOR_FUNCCODE1
    Uint16 fc;
    Uint16 u;

    fc = funcCode.code.motorDebugFc % 100;
    u = funcCode.code.motorDebugFc / 100;
    if (fc > CFNUM)
    {
        fc = CFNUM;
    }
    if (u > UFNUM)
    {
        u = UFNUM;
    }

    motorDebugFc.fc = fc;
    motorDebugFc.u = u;
#endif
#endif
}


//====================================================================
//
// �ϵ�ʱ�Ĳ˵���ʼ��
//
//====================================================================
void MenuInit(void)
{
    menuAttri[MENU_LEVEL_0].winkFlag = 0x00F8;
	// Ĭ�ϻ�ѡ���һ������
    menuModeTmp = MENU_MODE_BASE;
    menuMode = menuModeTmp;
    MenuModeDeal();
    menuLevel = MENU_LEVEL_0;
}


#if DEBUG_F_DISP_DIDO_STATUS_SPECIAL
// ����DIDO״̬��ֱ����ʾ
void UpdateDisplayBufferVisualIoStatus(Uint32 value)
{
    int16 i;
    Uint16 bit[4];

    for (i = 4; i >= 0; i--)
    {
        bit[0] = (value & (0x01 << 0)) >> 0;    // ���0/1
        bit[1] = (value & (0x01 << 1)) >> 1;
        bit[2] = (value & (0x01 << 2)) >> 2;
        bit[3] = (value & (0x01 << 3)) >> 3;

        displayBuffer[i] = (bit[0] << 1) |      // 1��ʾ��ʾλ�ã�������ܵ�λ��
                           (bit[1] << 2) |      // 2��ʾ��ʾλ�ã�������ܵ�λ��
                           (bit[2] << 5) |
                           (bit[3] << 4) |
                           (~DISPLAY_CODE[DISPLAY_LINE]);
        displayBuffer[i] = ~displayBuffer[i];

        value >>= 4;
    }

    //displayBuffer[5] = LED_CODE[LED_NULL];
}


// DI���ܵ���ʾ
// valueH, �� 8λ
// valueL, ��32λ
void UpdateDisplayBufferVisualDiFunc(Uint16 valueH, Uint32 valueL)
{
    //displayBuffer[5] = LED_CODE[LED_NULL];
    
    displayBuffer[4] = ~(Uint16)((valueL >> 0)  & 0xff);
    displayBuffer[3] = ~(Uint16)((valueL >> 8)  & 0xff);
    displayBuffer[2] = ~(Uint16)((valueL >> 16) & 0xff);
    displayBuffer[1] = ~(Uint16)((valueL >> 24) & 0xff);
    displayBuffer[0] = ~(Uint16)((valueH >> 0)  & 0xff);
}
#endif



#if DEBUG_F_PASSWORD      // ���롣�����û����룬ֻ���û����룬����������������
void MenuPwdOnPrg(void)
{
    // ������ڽ��������⣬����PRG�˻ص�0���˵���
    menuLevel = MENU_LEVEL_0;       // ���¸�λ

#if DEBUG_F_GROUP_HIDE
    if (groupHidePwdStatus)         // ���¸�λ���ص�2���˵���
    {
        groupHidePwdStatus = 0;
        menuLevel = MENU_LEVEL_2;
    }
#endif
}

// ����ENTER, UP, DOWN��������MENU_LEVEL_PWD_INPUT
LOCALD void MenuPwdHint2Input(void)
{
    menuLevel = MENU_LEVEL_PWD_INPUT;
    menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit = 0;
    menuPwdNumber = 0;                    // �����ʼΪ0
}

LOCALD void MenuPwdInputOnEnter(void)
{
    {
        if ((menuPwdNumber == funcCode.code.userPassword) ||    // �û�����
            (menuPwdNumber == SUPER_USER_PASSWORD))             // ��������
        {
            Menu0AddMenuLevel();
        }
        else
        {
            menuLevel = MENU_LEVEL_PWD_HINT;
        }
    }
}

// ������ʾ��(-----)��SHIFT����ʾ��������
LOCALD void MenuPwdHintOnShift(void)
{
#if DEBUG_RANDOM_FACPASS      
     facPassViewStatus= FAC_PASS_VIEW;

    // ���ɳ�������
    if (!factoryPwd)
    {
        Uint32 time;
        Uint16 pwd[4];
        time = GetTime();
        factoryPwd = (time >> 16) + (time & 0xFFFF);
        pwd[0] = factoryPwd>>8;
        pwd[1] = SUPER_USER_PASSWORD_SOURCE1&0xFF;
        pwd[2] = SUPER_USER_PASSWORD_SOURCE1>>8;
        pwd[3] = factoryPwd&0xFF;
        superFactoryPass = CrcValueByteCalc(pwd, 4);
    }
#else
    MenuPwdHint2Input();
#endif
}

LOCALD void MenuPwdInputOnUp(void)
{
    MenuPwdInputOnUpDown(ON_UP_KEY);
}



LOCALD void MenuPwdInputOnDown(void)
{
    MenuPwdInputOnUpDown(ON_DOWN_KEY);
}



void MenuPwdInputOnUpDown(Uint16 flag)
{
    int16 delta;

    delta = decNumber[menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit];

    if (ON_DOWN_KEY == flag)
        delta = -delta;

    menuPwdNumber = LimitDeal(0, menuPwdNumber, 65535, 0, delta);
}



LOCALD void MenuPwdInputOnShift(void)
{
    if (menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit == 0)
        menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit = 4;
    else
        menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit--;
}



LOCALD void UpdateMenuPwdHintDisplayBuffer(void)
{  
#if DEBUG_RANDOM_FACPASS    
    if (facPassViewStatus == FAC_PASS_VIEW)
    {
        // ��ʾ��������
        Uint16 digit[5];
        GetNumberDigit(digit, factoryPwd, 0);
        displayBuffer[0] = DISPLAY_CODE[digit[4]];
        displayBuffer[1] = DISPLAY_CODE[digit[3]];
        displayBuffer[2] = DISPLAY_CODE[digit[2]];
        displayBuffer[3] = DISPLAY_CODE[digit[1]];
        displayBuffer[4] = DISPLAY_CODE[digit[0]];
        menuAttri[MENU_LEVEL_PWD_HINT].winkFlag = 0;
    }
    else
#endif        
    {
        // ��ʾ-----
        displayBuffer[0] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[3] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[4] = DISPLAY_CODE[DISPLAY_LINE];
        menuAttri[MENU_LEVEL_PWD_HINT].winkFlag = 0x08; // 320��˸���һλ
    }
    
}



LOCALD void UpdateMenuPwdInputDisplayBuffer(void)
{   // �û���������
    Uint16 digit[5];

    GetNumberDigit(digit, menuPwdNumber, DECIMAL);

// �������ʾ
    displayBuffer[0] = DISPLAY_CODE[digit[4]];
    displayBuffer[1] = DISPLAY_CODE[digit[3]];
    displayBuffer[2] = DISPLAY_CODE[digit[2]];
    displayBuffer[3] = DISPLAY_CODE[digit[1]];
    displayBuffer[4] = DISPLAY_CODE[digit[0]];

    menuAttri[MENU_LEVEL_PWD_INPUT].winkFlag = 0x01U << (3 + menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit);
}

#elif 1

LOCALD void MenuPwdOnPrg(void){}
LOCALD void MenuPwdHint2Input(void){}
LOCALD void MenuPwdInputOnEnter(void){}
LOCALD void MenuPwdInputOnUp(void){}
LOCALD void MenuPwdInputOnDown(void){}
LOCALD void MenuPwdInputOnShift(void){}
LOCALD void UpdateMenuPwdHintDisplayBuffer(void){}
LOCALD void UpdateMenuPwdInputDisplayBuffer(void){}

#endif

LOCALD void MenuPwdHintOnQuick(void)
{
}
LOCALD void MenuPwdInputOnQuick(void)
{
}




