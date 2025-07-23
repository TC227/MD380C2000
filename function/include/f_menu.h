#ifndef __F_MENU_H__
#define __F_MENU_H__

#include "f_funcCode.h"

//=================================
enum MENU_MODE
{
    MENU_MODE_NULL,
    MENU_MODE_BASE,         // 1, �����˵�����ҪΪĿǰ320������(����)
    MENU_MODE_USER,         // 4, �û����Ʋ˵�
    MENU_MODE_CHECK        // 5, У��˵�������ʾ�����ֵ��ͬ�Ĺ�����
};
//=================================


Uint16 FcDigit2Bin(Uint16 value);



extern enum MENU_MODE menuModeTmp; 
extern enum MENU_MODE menuMode; 

extern Uint16 superFactoryPass;

enum MENU_MODE_OPERATE
{
    MENU_MODE_NONE,         // 
    MENU_MODE_ON_QUICK      // ����QUICK��
};
extern enum MENU_MODE_OPERATE menuModeStatus;

// �����������������ʾ
enum FAC_PASS_RANDOM_VIEW_OPERATE
{
    FAC_PASS_NONE,    // ��ʾ��������
    FAC_PASS_VIEW     // ����ʾ
};
extern enum FAC_PASS_RANDOM_VIEW_OPERATE facPassViewStatus;

#define DISP_OUT_CURRENT        4   //  4, ����������

#define DEBUG_RANDOM_FACPASS    0   // �����������

#if F_DEBUG_RAM
#define COMPANY_PASSWORD        1           // �������룬���ܵ���ʱ��������Ϊ1���������
#else
#define COMPANY_PASSWORD        11          // ��������
#endif

#if DEBUG_RANDOM_FACPASS  
#define SUPER_USER_PASSWORD     superFactoryPass   // �����û�����
#else
#define SUPER_USER_PASSWORD     22                 // �����û�����
#endif
 
#define SUPER_USER_PASSWORD_SOURCE1    13131      // ����������Դ

#define ON_UP_KEY       ACC_SPEED
#define ON_DOWN_KEY     DEC_SPEED

#define STOP_DISPLAY_NUM 16     // ͣ��ʱ��LED��ʾ����������
#define RUN_DISPLAY_NUM  32     // ����ʱ��LED��ʾ����������
#define COMM_PARA_NUM    33     // ͨѶ��ȡͣ����������ʾ�����ĸ���

#define USER_PARA_SAVE_FLAG1   1
#define USER_PARA_SAVE_FLAG2   0xFFFF - USER_PARA_SAVE_FLAG1

// ���˵�������װ��һ��
typedef struct tagSysMenu
{
    void (*onPrgFunc)();         // �ڵ�ǰ�˵����� PRG   ���Ĵ�����ָ��
    void (*onUpFunc)();          // �ڵ�ǰ�˵����� UP    ���Ĵ�����ָ��
    void (*onEnterFunc)();       // �ڵ�ǰ�˵����� ENTER ���Ĵ�����ָ��
    void (*onMfkFunc)();         // �ڵ�ǰ�˵����� MF.K  ���Ĵ�����ָ��
    void (*onDownFunc)();        // �ڵ�ǰ�˵����� DOWN  ���Ĵ�����ָ��
    void (*onShiftFunc)();       // �ڵ�ǰ�˵����� SHIFT ���Ĵ�����ָ��
    void (*onRunFunc)();         // �ڵ�ǰ�˵����� RUN   ���Ĵ�����ָ��
    void (*onStopFunc)();        // �ڵ�ǰ�˵����� STOP  ���Ĵ�����ָ��
    void (*onQuickFunc)();       // �ڵ�ǰ�˵����� QUICK ���Ĵ�����ָ��

    void (*UpdateDisplayBuffer)(); // ��ǰ�˵��¸�����ʾ���ݻ���ĺ���ָ��
}sysMenu, *sysMenuHandle;

extern const sysMenu menu[];

#define MENU_LEVEL_NUM  6   // һ����_���˵�
enum MENU_LEVEL
{        
    MENU_LEVEL_0,           // 0���˵�
    MENU_LEVEL_1,           // 1���˵�
    MENU_LEVEL_2,           // 2���˵�
    MENU_LEVEL_3,           // 3���˵�
    MENU_LEVEL_PWD_HINT,    // ��ʾ��������
    MENU_LEVEL_PWD_INPUT    // ��������
};
extern enum MENU_LEVEL menuLevel;


struct MENU_ATTRIBUTE
{
    Uint16 operateDigit;    // �����˵��£���ǰ����λ. 0-4
    
    Uint16 winkFlag;        // �������˸���ƼĴ���, ��ӦλΪ1ʱ��˸.
// bit7,�������ߵ�1��(�����); bit3, �������ߵ�5��(���ұ�)

    Uint16 winkFlagLed;     // led��˸���ƼĴ���. bit0-Led0; bit1-Led1,...
// ���Խ�winkFlag��winkFlagLed����һ��
};
extern struct MENU_ATTRIBUTE menuAttri[];


// ��ѹ�ȼ�����ѹ�Ĺ�ϵ�� invTypeLimitTable[]
struct INV_PARA
{
    Uint16 type;                // ��Ƶ������
    
    Uint16 ratingVoltage;       // ��Ƶ���Ķ��ѹ
    Uint16 volLevel;            // ��Ƶ���ĵ�ѹ�ȼ�
    Uint16 pointLimit;          // ���������������С����
    Uint16 bitAccDecStart;      // �Ӽ���ʱ�����ֵΪ��ֵ����ʼ����
};
extern struct INV_PARA invPara;


enum MENU0_DISP_STATUS
{
    MENU0_DISP_STATUS_RUN_STOP,     // ����/ͣ����ʾ
    MENU0_DISP_STATUS_UP_DOWN,      // up/downʱ��ʾ
    MENU0_DISP_STATUS_ERROR,        // ����/�澯��ʾ
    MENU0_DISP_STATUS_TUNE          // ��г��ʾ
};
extern enum MENU0_DISP_STATUS menu0DispStatus;     // 0���˵�����ʾ״̬

extern Uint16 accDecFrqPrcFlag;
extern Uint16 bFrqDigital;
extern Uint16 bFrqDigitalDone4WaitDelay;
extern Uint16 outCurrentDisp;
extern Uint16 outCurrentDispView;
extern Uint16 currentOcDisp;
extern Uint16 frqAimDisp;
extern Uint16 frqPLCDisp;       // PLC�ɱ�̿���ȡ����
extern Uint16 frqAimPLCDisp;    // PLC����̿���ȡ�趨Ƶ��
extern Uint16 outVoltageDisp;
extern Uint16 frqRunDisp;
extern Uint16 pcOriginDisp;

extern Uint16 itDisp;
extern Uint16 frqDisp;
extern Uint16 frqXDisp;
extern Uint16 frqYDisp;
extern Uint32 torqueCurrentAct;   // ת�ص���

extern Uint16 * const pDispValueU0[];
extern Uint16 * const pOscValue[];
extern Uint16 const commDispIndex[];

typedef struct
{
    Uint16 flag;                // 0 -- �������index; 1 --
    Uint16 data;                //

    Uint16 signal;              // ���޷���
    Uint16 upper;               // ����
    Uint16 lower;               // ����
    int16  delta;               //

    Uint16 index;               // �������index
} LIMIT_DEAL_STRUCT;
extern LIMIT_DEAL_STRUCT limitDealData;

// �����޴���
Uint16 LimitDeal(Uint16 signal, Uint16 data, Uint16 upper, Uint16 lower, int16 delta);

// ��Щ��������趨������ͬ��Ŀǰ280F��DI���ӣ�320��DI���ӣ�������Ƶ��Դѡ��
Uint16 NoSameDeal(Uint16 index, const Uint16 funcIndex[], int16 number, int16 data, int16 upper, int16 lower, int16 delta);

Uint16 ModifyFunccodeUpDown(Uint16 index, Uint16 *data, int16 delta);
Uint16 ModifyFunccodeEnter(Uint16 index, Uint16 data);
Uint16 TorqueBoostDeal(Uint16 power);
void DispDataDeal(void);
void UpdateErrorDisplayBuffer(void);
void ClearRecordDeal(void);

void MenuModeDeal(void);
void UpdataFuncCodeGrade(Uint16 funcCodeGrade[]);

extern const int16 decNumber[];

extern Uint16 loadSpeedDisp;
extern Uint16 pidFuncRefDisp;
extern Uint16 pidFuncFdbDisp;
extern Uint16 pulseInFrqDisp;
extern Uint16 vfSeparateVol;
extern Uint16 vfSeprateVolAim; 

void RestoreCompanyParaOtherDeal(void);
Uint16 GetFuncCodeInit(Uint16 index, Uint16 type);
Uint16 GetFuncCodeInitOriginal(Uint16 index);


// ĳЩ�����������������������޵Ĵ���
LOCALD void LimitOtherCodeDeal(Uint16 index);

void MenuInit(void);

void UpdateInvType(void);



extern const union FUNC_ATTRIBUTE dispAttributeU0[];



enum CHECK_MENU_MODE_DEAL
{
    CHECK_MENU_MODE_DEAL_CMD,           // ָ�׼����ʼ
    CHECK_MENU_MODE_DEAL_SERACHING,     // ��������
    CHECK_MENU_MODE_DEAL_END_NONE,      // ����ȫ�������룬û�з��������ֵ��ͬ�Ĺ�����
    CHECK_MENU_MODE_DEAL_END_ONCE       // �����ҵ�һ��
};

extern Uint16 mainLoopTicker;



#endif  // __F_MENU_H__



