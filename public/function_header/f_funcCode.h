/*****************************************************************
 *
 * �����붨���ͷ�ļ�
 * 
 * Time-stamp: <2012-08-20 08:57:13  Shisheng.Zhi, 0354>
 * 
 *
 *
 *
 *****************************************************************/

#ifndef __F_FUNCCODE_H__
#define __F_FUNCCODE_H__



       // 2803x����2808ƽ̨
#include "DSP28x_Project.h"

#include "f_debug.h"
#include "f_interface.h"
#include "f_p2p.h"




// �벻Ҫ�����޸�������ֵ������EEPROM�е�ֵ��ȫ��(����P0�顢P1�顢���ּ�¼)�ָ�����ֵ��
#define EEPROM_CHECK1       (0x0009)                        // EEPROMУ����1
#define EEPROM_CHECK2       (0xFFFF - EEPROM_CHECK1)        // EEPROMУ����2


#define USER_MENU_GROUP     FUNCCODE_GROUP_FE

// ��ù�����code��ȫ���������е�index����������������funcCode.all[]�е��±ꡣ
// ���� FUNCCODE_ALL �Ķ��壬һ��������ı�ʾ��4�ְ취����Ӧ�Ļ��index�İ취��
// 1. funcCode.all[i]     ---- i
// 2. funcCode.f5[7]      ---- GetCodeIndex(funcCode.group.f5[7])
// 3. group, grade        ---- GetGradeIndex(group, grade)
// 4. funcCode.code.maxFrq---- GetCodeIndex(funcCode.code.maxFrq)
#define GetCodeIndex(code)    ((Uint16)((&(code)) - (&(funcCode.all[0]))))
#define GetGradeIndex(group, grade)  (funcCodeGradeSum[group] + (grade))


//=====================================================================
// ����������λ����
//
// ��λ
#define ATTRIBUTE_UNIT_HZ_BIT           0
#define ATTRIBUTE_UNIT_A_BIT            1
#define ATTRIBUTE_UNIT_V_BIT            2

// ��д����
#define ATTRIBUTE_READ_AND_WRITE        0   // (�κ�ʱ��)��д
#define ATTRIBUTE_READ_ONLY_WHEN_RUN    1   // ����ʱֻ��
#define ATTRIBUTE_READ_ONLY_ANYTIME     2   // ֻ��

// �๦�����������
#define ATTRIBUTE_MULTI_LIMIT_SINGLE    0   // �����Ĺ�����
#define ATTRIBUTE_MULTI_LIMIT_DEC       1   // ��������룬ʮ����
#define ATTRIBUTE_MULTI_LIMIT_HEX       2   // ��������룬ʮ������

struct  FUNC_ATTRIBUTE_BITS
{                           // bits   description
    Uint16 point:3;         // 2:0    radix point,С���㡣
                            //        0-��С���㣬1-1λС����...��4-4λС��
                            //        (0.0000-100,00.000-011,000.00-010,0000.0-001,00000-000)
    Uint16 unit:3;          // 5:3    unit,��λ
                            //        1-hz, 2-A, 3-RPM, 4-V, 6-%; 001-Hz, 010-A, 100-V
    Uint16 displayBits:3;   // 8:6    5�������Ҫ��ʾ��λ����0-��ʾ0λ��1-��ʾ1λ��...��5-��ʾ5λ
    Uint16 upperLimit:1;    // 9      1-������������ع��������ƣ�0-ֱ������������
    Uint16 lowerLimit:1;    // 10     1-������������ع��������ƣ�0-ֱ������������
    Uint16 writable:2;      // 12:11  ������д���ԣ�00-���Զ�д, 01-������ֻ����10-����ֻ��
    Uint16 signal:1;        // 13     ���ţ�unsignal-0; signal-1
    Uint16 multiLimit:2;    // 15:14  �ù�����Ϊ�������������. 
                            //        00-����������(�����); 
                            //        01-ʮ����,  �������������; 
                            //        10-ʮ������,�������������; 
};

union FUNC_ATTRIBUTE
{
   Uint16                      all;
   struct FUNC_ATTRIBUTE_BITS  bit;
};
//=====================================================================



//=====================================================================
// ���������Ա����ޡ����ޡ�����
// ������ĳ���ֵ������EEPROM_CHECK��������䣬����������ʾ��
typedef struct FUNCCODE_ATTRIBUTE_STRUCT
{
    Uint16                  lower;          // ����
    Uint16                  upper;          // ����
    Uint16                  init;           // ����ֵ
    union FUNC_ATTRIBUTE    attribute;      // ����

    Uint16                  eepromIndex;    // ��ӦEEPROM�洢��index
} FUNCCODE_ATTRIBUTE;

extern const FUNCCODE_ATTRIBUTE funcCodeAttribute[];
//=====================================================================



//=====================================================================
// �������code��һЩ���ݽṹ����
struct PLC_STRUCT
{
    Uint16 runTime;         // PLC��_������ʱ��
    Uint16 accDecTimeSet;   // PLC��_�μӼ���ʱ��ѡ��
};
//=================================


//=================================
struct AI_SET_CURVE  // AI�趨����
{
    Uint16 minIn;       // ������С����(��ѹֵ)
    Uint16 minInSet;    // ������С�����Ӧ�趨(�ٷֱ�)
    Uint16 maxIn;       // �����������(��ѹֵ)
    Uint16 maxInSet;    // ������������Ӧ�趨(�ٷֱ�)
};
//=================================


//=================================
struct AI_JUMP
{
    Uint16 point;   // �趨��Ծ��
    Uint16 arrange; // �趨��Ծ����
};
//=================================


//=================================
struct ANALOG_CALIBRATE_CURVE  // ģ����У�����ߣ�AIAO
{
    Uint16 before1;     // У��ǰ��ѹ1
    Uint16 after1;      // У�����ѹ1
    
    Uint16 before2;     // У��ǰ��ѹ2
    Uint16 after2;      // У�����ѹ2
};
//=================================


//=================================
typedef struct AO_PARA_STRUCT
{
    Uint16 offset;          // AO��ƫϵ��
    Uint16 gain;            // AO����
} AO_PARA;
//=================================


//=================================
struct FC_GROUP_HIDE_STRUCT
{
    Uint16 password;    // ��������������������
    
    Uint16 f;           // F���������ԣ�F0-FF��
    Uint16 a;           // A���������ԣ�A0-AF��
    Uint16 b;           // B���������ԣ�B0-BF��
    Uint16 c;           // C���������ԣ�C0-CF��
};
union FC_GROUP_HIDE
{
    Uint16 all[5];
    struct FC_GROUP_HIDE_STRUCT elem;
};
//=================================

//=================================

//=================================
struct ERROR_SCENE_STRUCT
{
    Uint16 errorFrq;                    // ������(���һ��)����ʱƵ��
    Uint16 errorCurrent;                // ������(���һ��)����ʱ����
    Uint16 errorGeneratrixVoltage;      // ������(���һ��)����ʱĸ�ߵ�ѹ
    Uint16 errorDiStatus;               // ������(���һ��)����ʱ�������״̬
    Uint16 errorDoStatus;               // ������(���һ��)����ʱ�������״̬
    
    Uint16 errorInverterStatus;         // ������(���һ��)����ʱ��Ƶ��״̬
    Uint16 errorTimeFromPowerUp;        // ������(���һ��)����ʱʱ�䣨�ӱ����ϵ翪ʼ��ʱ��
    Uint16 errorTimeFromRun;            // ������(���һ��)����ʱʱ�䣨������ʱ��ʼ��ʱ��
};

union ERROR_SCENE
{
    Uint16 all[sizeof(struct ERROR_SCENE_STRUCT)];

    struct ERROR_SCENE_STRUCT elem;
};
//=================================


//=================================
#define APTP_NUM    32  // aptp����
typedef struct
{
    Uint16 low;     // aptp��λ��0-9999
    Uint16 high;    // aptp��λ��0-65535
} LENGTH_SET;
// ��Χ: 0-655359999
//=================================


//=================================
#define MOTOR_TYPE_ACI_GENERAL  0   // ��ͨ�첽���
#define MOTOR_TYPE_ACI_INV      1   // ��Ƶ�첽���
#define MOTOR_TYPE_PMSM         2   // ����ͬ�����
struct MOTOR_PARA_STRUCT
{
    // �����������
    Uint16 motorType;               // F1-00  �������ѡ��
    Uint16 ratingPower;             // F1-01  ��������
    Uint16 ratingVoltage;           // F1-02  ������ѹ
    Uint16 ratingCurrent;           // F1-03  ��������
    Uint16 ratingFrq;               // F1-04  ����Ƶ��
    Uint16 ratingSpeed;             // F1-05  ����ת��

    // �첽����г����
    Uint16 statorResistance;        // F1-06  �첽�����ӵ���
    Uint16 rotorResistance;         // F1-07  �첽��ת�ӵ���
    Uint16 leakInductance;          // F1-08  �첽��©�п�
    Uint16 mutualInductance;        // F1-09  �첽�����п�
    Uint16 zeroLoadCurrent;         // F1-10  �첽�����ص���
    Uint16 rsvdF11[5];

    // ͬ������г����
    Uint16 pmsmRs;                  // F1-16  ͬ�������ӵ���
    Uint16 pmsmLd;                  // F1-17  ͬ����d����
    Uint16 pmsmLq;                  // F1-18  ͬ����q����
    Uint16 pmsmRsLdUnit;            // F1-19  ͬ������е��赥λ
    Uint16 pmsmCoeff;               // F1-20  ͬ�������綯��ϵ��
    Uint16 pmsmCheckTime;           // F1-21  ͬ������ȱ����ʱ��
    Uint16 rsvdF12[5];
    
};

struct PG_PARA_STRUCT
{
    // PG������
    Uint16 encoderPulse;            // F1-27    ��������������
    Uint16 pgType;                  // F1-28    ����������
    Uint16 fvcPgSrc;                // F1-29    �ٶȷ���PG��ѡ��, 0-QEP1,1-QEP2(��չ)    
    Uint16 enCoderDir;              // F1-30    ����������/������
    Uint16 enCoderAngle;            // F1-31    ��������װ��
    Uint16 uvwSignDir;              // F1-32    UVW�źŷ���
    Uint16 uvwSignAngle;            // F1-33    UVW�ź����λ�ý�
    Uint16 enCoderPole;             // F1-34    ���伫����
    Uint16 rsvdF11;                 // F1-35    UVW������ 
    Uint16 fvcPgLoseTime;           // F1-36    �ٶȷ���PG���߼��ʱ��    
};

union MOTOR_PARA
{
    Uint16 all[sizeof(struct MOTOR_PARA_STRUCT)];
    
    struct MOTOR_PARA_STRUCT elem;
};

union PG_PARA
{
    Uint16 all[sizeof(struct PG_PARA_STRUCT)];
    
    struct PG_PARA_STRUCT elem;
};

//=================================

//=================================
enum MOTOR_SN
{
    MOTOR_SN_1,     // ��1���
    MOTOR_SN_2,     // ��2���
    MOTOR_SN_3,     // ��3���
    MOTOR_SN_4      // ��4���
};
//=================================


//=================================


//------------------------------------------------
struct VC_PARA
{
    Uint16 vcSpdLoopKp1;            // F2-00  �ٶȻ���������1
    Uint16 vcSpdLoopTi1;            // F2-01  �ٶȻ�����ʱ��1
    Uint16 vcSpdLoopChgFrq1;        // F2-02  �л�Ƶ��1
    Uint16 vcSpdLoopKp2;            // F2-03  �ٶȻ���������2
    Uint16 vcSpdLoopTi2;            // F2-04  �ٶȻ�����ʱ��2
    
    Uint16 vcSpdLoopChgFrq2;        // F2-05  �л�Ƶ��2
    Uint16 vcSlipCompCoef;          // F2-06  ת���ϵ��
    Uint16 vcSpdLoopFilterTime;     // F2-07  �ٶȻ��˲�ʱ�䳣��
    Uint16 vcOverMagGain;           // F2-08  ʸ�����ƹ���������
    Uint16 spdCtrlDriveTorqueLimitSrc;  // F2-09  �ٶȿ���(����)ת������Դ
    
    Uint16 spdCtrlDriveTorqueLimit;     // F2-10  �ٶȿ���(����)ת�����������趨
    Uint16 spdCtrlBrakeTorqueLimitSrc;  // F2-11  �ٶȿ���(�ƶ�)ת������Դ
    Uint16 spdCtrlBrakeTorqueLimit;     // F2-12  �ٶȿ���(�ƶ�)ת�����������趨
    Uint16 mAcrKp;                  // F2-13  M�������Kp
    Uint16 mAcrKi;                  // F2-14  M�������Ki
    
    Uint16 tAcrKp;                  // F2-15  T�������Kp
    Uint16 tAcrKi;                  // F2-16  T�������Ki
    Uint16 spdLoopI;                // F2-17  �ٶȻ���������

    Uint16 weakFlusMode;            // F2-18 ͬ��������ģʽ
    Uint16 weakFlusCoef;            // F2-19 ͬ��������ϵ��
    Uint16 weakFlusCurMax;          // F2-20 ������ŵ���
    Uint16 weakFlusAutoCoef;        // F2-21 �����Զ���гϵ��
    Uint16 weakFlusIntegrMul;       // F2-22 ���Ż��ֱ���
};
//------------------------------------------------



//------------------------------------------------
// ��2����Ĺ����룬����������������Ʋ���
struct MOTOR_FC
{
    union MOTOR_PARA motorPara;     // Ax-00  Ax-26 ��2/3/4�������. ͬ��1�������
    union PG_PARA    pgPara;        // Ax-27  Ax-36 ��1���PG������
    Uint16 tuneCmd;                 // Ax-37  ��г����
    struct VC_PARA vcPara;          // Ax-38  Ax-60
    
    Uint16 motorCtrlMode;           // Ax-61  ��2/3/4������Ʒ�ʽ
    Uint16 accDecTimeMotor;         // Ax-62  ��2/3/4����Ӽ���ʱ��ѡ��
    Uint16 torqueBoost;             // Ax-63  ת������
    Uint16 rsvdA21;                 // Ax-64  ����������ģʽ/
    Uint16 antiVibrateGain;         // Ax-65  ����������
};
//------------------------------------------------/





#define AI_NUMBER               3           // AI���Ӹ���

#define AO_NUMBER               2           // AO���Ӹ���

#define HDI_NUMBER              1           // HDI���Ӹ���

#define HDO_NUMBER              1           // HDO���Ӹ���

#define DI_NUMBER_PHSIC         10          // ����DI����
#define DI_NUMBER_V             5           // ����DI����
#define DI_NUMBER_AI_AS_DI      AI_NUMBER   // AI��ΪDI

#define DO_NUMBER_PHSIC         5           // ����DO����
#define DO_NUMBER_V             5           // ����DO����

#define PLC_STEP_MAX            16          // PLC�����ָ�����




//=====================================================================
// EEPROM��ʹ�ó��ȣ������м�Ԥ������
// EEPROM��ַ���䣬2010-08-13
// 0            -   ������rsvd4All
// 1,2          -   EEPROM-CHK
// 3            -   AIAO CHK
// 4-944        -   MD380ʹ��
// 945-1149     -   Ԥ��MD380ʹ��
// 1150-1329    -   IS380/MD380Mʹ��
// ����,16-63   -   �������ʹ��
#define EEPROM_INDEX_USE_LENGTH     994     // ���һ��eeprom��ַ+1
#define EEPROM_INDEX_USE_INDEX        4     // ���ܲ���ʹ��EEPROM��ʼ
// Ԥ��������ʹ�õĹ�����index
#define FUNCCODE_RSVD4ALL_INDEX     GetCodeIndex(funcCode.code.rsvd4All)


#define REM_P_OFF_MOTOR     5   // ����ʹ�õĵ�����䡣������CORE_TO_FUNC_DISP_DATA_NUM����


// �����������
#define FUNCCODE_GROUP_NUM  83  // ����Ԥ������, ��funcCodeGradeCurrentMenuMode. 
// EEPROM��˳��: EEPROM_CHK, �������, FF, FP, F0-FE, A0-AF, B0-BF, C0-CF
// ��ʾ˳��:     F0-FE, FF, FP, A0-AF, B0-BF, C0-CF, EEPROM_CHK(����ʾ), �������(����ʾ), U0-UF
// U0-UF��ռ��EEPROM�ռ�

#define FUNCCODE_GROUP_F0   0       // F0��
#define FUNCCODE_GROUP_F1   1       // F1��
#define FUNCCODE_GROUP_FE   14      // FF��
#define FUNCCODE_GROUP_FF   15      // FF��
#define FUNCCODE_GROUP_FP   16      // FP��
#define FUNCCODE_GROUP_A0   17      // A0��
#define FUNCCODE_GROUP_A5   22      // A5��
#define FUNCCODE_GROUP_AA   27      // AA��
#define FUNCCODE_GROUP_AB   28      // AB��
#define FUNCCODE_GROUP_AE   31      // AE��
#define FUNCCODE_GROUP_AF   32      // AF��
#define FUNCCODE_GROUP_B0   33      // B0��
#define FUNCCODE_GROUP_BF   48      // BF��
#define FUNCCODE_GROUP_C0   49      // C0��
//#define FUNCCODE_GROUP_CC   61      // CC�� �û����ƹ�������
#define FUNCCODE_GROUP_CF   64      // CF��
#define FUNCCODE_GROUP_U0   67      // U0�飬��ʾ
#define FUNCCODE_GROUP_U3   70      // U3�飬��ʾ
#define FUNCCODE_GROUP_UF   (FUNCCODE_GROUP_NUM - 1)    // UF�����ܵ�����ʾ

// ÿ�鹦����ĸ���
// ��FF��֮�⣬ÿ��Ԥ��2�������롣Ϊ�������ӹ�����ʱ���������ûָ�����������
#define F0NUM           (28+ 1  )   // F0  ����������
#define F1NUM           (38+ 0  )   // F1  �������
#define F2NUM           (23+ 0  )   // F2  ʸ�����Ʋ���
#define F3NUM           (16+ 0  )   // F3  V/F���Ʋ���

#define F4NUM           (40+ 0  )   // F4  �������
#define F5NUM           (23+ 0  )   // F5  �������
#define F6NUM           (16+ 0  )   // F6  ��ͣ����
#define F7NUM           (15+ 0  )   // F7  ��������ʾ

#define F8NUM           (54+ 0  )   // F8  ��������
#define F9NUM           (71+ 0  )   // F9  �����뱣��
#define FANUM           (29+ 0  )   // FA  PID���� 
#define FBNUM           (10+ 0  )   // FB  ��Ƶ�������ͼ���

#define FCNUM           (52+ 0  )   // FC  ����١�PLC
#define FDNUM           ( 8+ 0  )   // FD  ͨѶ����
#define FENUM           (32+ 0  )   // FE  280��320û�еĹ�����
#define FFNUM           (13+ 0  )   // FF  ���Ҳ���

#define FPNUM           ( 6+ 0  )   // FP  �û�����, ������ʼ��

#define A0NUM           ( 9+ 0  )   // A0
#define A1NUM           (22+0   )   // A1
#define A2NUM           (F1NUM+2+F2NUM+3) // A2
#define A3NUM           (A2NUM)             // A3
#define A4NUM           (A2NUM)             // A4
#define A5NUM           (10+ 0  )   // A5
#define A6NUM           (30+0   )   // A6
#define A7NUM           (9+3    )   // A7
#define A8NUM           (8+0    )   // A8
#define A9NUM           (30     )   // A9
#define AANUM           (1+ 0  )    // AA
#define ABNUM           (1+ 0   )   // AB
#define ACNUM           (20     )   // AC
#define ADNUM           (1+0    )   // AD
#define AENUM           (ACNUM  )   // AE
#define AFNUM           ( 1+0   )   // AF

#define B0NUM           ( 0+1   )   // B0
#define B1NUM           ( 0+1   )   // B1
#define B2NUM           ( 0+1   )   // B2
#define B3NUM           ( 0+1   )   // B3
                                    
#define B4NUM           ( 0+1   )   // B4
#define B5NUM           ( 0+1   )   // B5
#define B6NUM           ( 0+1   )   // B6
#define B7NUM           ( 0+1   )   // B7
                                    
#define B8NUM           ( 0+1   )   // B8
#define B9NUM           ( 0+1   )   // B9
#define BANUM           ( 0+1   )   // BA
#define BBNUM           ( 0+1   )   // BB
                                    
#define BCNUM           ( 0+1   )   // BC
#define BDNUM           ( 0+1   )   // BD
#define BENUM           ( 0+1   )   // BE
#define BFNUM           ( 0+1   )   // BF

#define C0NUM           ( 0+1   )   // C0
#define C1NUM           ( 0+1   )   // C1
#define C2NUM           ( 0+1)      // C2
#define C3NUM           ( 0+1)      // C3

#define C4NUM           ( 0+1)      // C4
#define C5NUM           ( 0+1)      // C5
#define C6NUM           ( 0+1)      // C6
#define C7NUM           ( 0+1)      // C7

#define C8NUM           ( 0+1)      // C8
#define C9NUM           ( 0+1)      // C9
#define CANUM           ( 0+1)      // CA
#define CBNUM           ( 0+1)      // CB
                                
#define CCNUM           ( 1+0)      // CC   
#define CDNUM           ( 0+1)      // CD
#define CENUM           ( 0+1)      // CE
#if DEBUG_F_MOTOR_FUNCCODE
#define CFNUM           FUNC_TO_CORE_DEBUG_DATA_NUM     // CF  ���ԣ�func2motor
#elif 1
#define CFNUM           ( 0+1)      // CF
#endif

#define CHK_NUM   (4 +0  )  //     eepromCheckWord(2)��rsvd4All(1)�����������ǰ��, AIAOChk(1)Ҳ������
#define REM_NUM   (48)      // ������䣬��������ʹ�õĵ�����䡣(�������ܵĵ������)

#define U0NUM     (100+0)   // U0  ��ʾʹ�ã���ռ��EEPROM�������ٵ�ռ�ó���ռ�(�޳���ֵ�������ޣ���������)
#define U1NUM     ( 0+1)    // U1
#define U2NUM     ( 0+1)    // U2
#define U3NUM     (10+2)    // U3

#define U4NUM     ( 0+1)    // U4
#define U5NUM     ( 0+1)    // U5
#define U6NUM     ( 0+1)    // U6
#define U7NUM     ( 0+1)    // U7

#define U8NUM     ( 0+1)    // U8
#define U9NUM     ( 0+1)    // U9
#define UANUM     ( 0+1)    // UA
#define UBNUM     ( 0+1)    // UB

#define UCNUM     ( 0+1)    // UC
#define UDNUM     ( 0+1)    // UD
#define UENUM     ( 0+1)    // UE
#if DEBUG_F_MOTOR_FUNCCODE
#define UFNUM     CORE_TO_FUNC_DISP_DATA_NUM  // UF  ���ԣ�motor2func
#elif 1
#define UFNUM     ( 0+1)    // UF ���ܵ���ʹ��
#endif

#define FNUM_PARA      (F0NUM + F1NUM + F2NUM + F3NUM +     \
                        F4NUM + F5NUM + F6NUM + F7NUM +     \
                        F8NUM + F9NUM + FANUM + FBNUM +     \
                        FCNUM + FDNUM + FENUM + FFNUM +     \
                                                            \
                        FPNUM +                             \
                                                            \
                        A0NUM + A1NUM + A2NUM + A3NUM +     \
                        A4NUM + A5NUM + A6NUM + A7NUM +     \
                        A8NUM + A9NUM + AANUM + ABNUM +     \
                        ACNUM + ADNUM + AENUM + AFNUM +     \
                                                            \
                        B0NUM + B1NUM + B2NUM + B3NUM +     \
                        B4NUM + B5NUM + B6NUM + B7NUM +     \
                        B8NUM + B9NUM + BANUM + BBNUM +     \
                        BCNUM + BDNUM + BENUM + BFNUM +     \
                                                            \
                        C0NUM + C1NUM + C2NUM + C3NUM +     \
                        C4NUM + C5NUM + C6NUM + C7NUM +     \
                        C8NUM + C9NUM + CANUM + CBNUM +     \
                        CCNUM + CDNUM + CENUM + CFNUM       \
                        )                                   // ���й����룬��������ʾ
#define FNUM_EEPROM    (FNUM_PARA + CHK_NUM + REM_NUM)      // ��Ҫ�洢��EEPROM�е����в���
#define FNUM_ALL       (FNUM_EEPROM +                       \
                        U0NUM + U1NUM + U2NUM + U3NUM +     \
                        U4NUM + U5NUM + U6NUM + U7NUM +     \
                        U8NUM + U9NUM + UANUM + UBNUM +     \
                        UCNUM + UDNUM + UENUM + UFNUM       \
                       )                                    // ���й����롢������������ʾ


//=====================================================================
//
// �������飬F0�飬F1��, ...
//
// eepromCheckWord������ǰ��
// ���Ҳ������û����롢�û����Ʋ˵�����ǰ�棬��ֹ��ɾ������ʱ�����޸ġ�
// fc��eeprom�ж�Ӧ��ϵ����������Ҫ����Ҫ��
//
//=====================================================================
struct FUNCCODE_GROUP 
{
//======================================
    Uint16 f0[F0NUM];               // F0 ����������
    Uint16 f1[F1NUM];               // F1 �������
    Uint16 f2[F2NUM];               // F2 ʸ�����Ʋ���
    Uint16 f3[F3NUM];               // F3 V/F���Ʋ���
    
//======================================
    Uint16 f4[F4NUM];               // F4 �������
    Uint16 f5[F5NUM];               // F5 �������
    Uint16 f6[F6NUM];               // F6 ��ͣ����
    Uint16 f7[F7NUM];               // F7 ��������ʾ
    
//======================================
    Uint16 f8[F8NUM];               // F8 ��������
    Uint16 f9[F9NUM];               // F9 �����뱣��
    Uint16 fa[FANUM];               // FA PID����
    Uint16 fb[FBNUM];               // FB ��Ƶ�������ͼ���

//======================================
    Uint16 fc[FCNUM];               // FC ����١�PLC
    Uint16 fd[FDNUM];               // FD ͨѶ����
    Uint16 fe[FENUM];               // FE 280��320û�еĹ�����
    Uint16 ff[FFNUM];               // FF ���Ҳ���
    
//======================================
    Uint16 fp[FPNUM];               // FP ���������
    
//======================================
    Uint16 a0[A0NUM];               // A0
    Uint16 a1[A1NUM];               // A1
    Uint16 a2[A2NUM];               // A2
    Uint16 a3[A3NUM];               // A3

//======================================
    Uint16 a4[A4NUM];               // A4
    Uint16 a5[A5NUM];               // A5
    Uint16 a6[A6NUM];               // A6
    Uint16 a7[A7NUM];               // A7

//======================================
    Uint16 a8[A8NUM];               // A8
    Uint16 a9[A9NUM];               // A9
    Uint16 aa[AANUM];               // AA
    Uint16 ab[ABNUM];               // AB

//======================================
    Uint16 ac[ACNUM];               // AC
    Uint16 ad[ADNUM];               // AD
    Uint16 ae[AENUM];               // AE AIAO����У��
    Uint16 af[AFNUM];               // AF ������������
    
//======================================
    Uint16 b0[B0NUM];               // B0
    Uint16 b1[B1NUM];               // B1
    Uint16 b2[B2NUM];               // B2
    Uint16 b3[B3NUM];               // B3

//======================================
    Uint16 b4[B4NUM];               // B4
    Uint16 b5[B5NUM];               // B5
    Uint16 b6[B6NUM];               // B6
    Uint16 b7[B7NUM];               // B7

//======================================
    Uint16 b8[B8NUM];               // B8
    Uint16 b9[B9NUM];               // B9
    Uint16 ba[BANUM];               // BA
    Uint16 bb[BBNUM];               // BB

//======================================
    Uint16 bc[BCNUM];               // BC
    Uint16 bd[BDNUM];               // BD
    Uint16 be[BENUM];               // BE
    Uint16 bf[BFNUM];               // BF

//======================================
    Uint16 c0[C0NUM];               // C0
    Uint16 c1[C1NUM];               // C1
    Uint16 c2[C2NUM];               // C2
    Uint16 c3[C3NUM];               // C3

//======================================
    Uint16 c4[C4NUM];               // C4
    Uint16 c5[C5NUM];               // C5
    Uint16 c6[C6NUM];               // C6
    Uint16 c7[C7NUM];               // C7

//======================================
    Uint16 c8[C8NUM];               // C8
    Uint16 c9[C9NUM];               // C9
    Uint16 ca[CANUM];               // CA
    Uint16 cb[CBNUM];               // CB

//======================================
    Uint16 cc[CCNUM];               // CC   
    Uint16 cd[CDNUM];               // CD
    Uint16 ce[CENUM];               // CE
    Uint16 cf[CFNUM];               // CF

// ֮ǰ�Ĺ������������ޣ����ԣ�֮��û�У���ʡ�ռ�
//======================================

//======================================
    Uint16 fChk[CHK_NUM];           // eepromCheckWord

//======================================
    Uint16 remember[REM_NUM];       // �������

// ֮ǰ������Ҫ����EEPROM��
//======================================

//======================================
// ֮������ݲ���Ҫ����EEPROM�У���RAM
    Uint16 u0[U0NUM];               // U0 ��ʾ
    Uint16 u1[U1NUM];               // U1
    Uint16 u2[U2NUM];               // U2
    Uint16 u3[U3NUM];               // U3

//======================================
    Uint16 u4[U4NUM];               // U4
    Uint16 u5[U5NUM];               // U5
    Uint16 u6[U6NUM];               // U6
    Uint16 u7[U7NUM];               // U7

//======================================
    Uint16 u8[U8NUM];               // U8
    Uint16 u9[U9NUM];               // U9
    Uint16 ua[UANUM];               // UA
    Uint16 ub[UBNUM];               // UB

//======================================
    Uint16 uc[UCNUM];               // UC
    Uint16 ud[UDNUM];               // UD
    Uint16 ue[UENUM];               // UE
    Uint16 uf[UFNUM];               // UF, ��ʾ�����ܵ���ʹ��
//======================================
};


//=====================================================================
//
// �����룬F0-00, F0-01, ..., F1-00, F1-01, ...
//
//=====================================================================
struct FUNCCODE_CODE 
{
//======================================
// F0 ����������
    Uint16 inverterGpTypeDisp;      // F0-00  GP������ʾ
    Uint16 motorCtrlMode;           // F0-01  (���)���Ʒ�ʽ
    Uint16 runSrc;                  // F0-02  ����Դѡ��
    Uint16 frqXSrc;                 // F0-03  ��Ƶ��ԴXѡ��
    Uint16 frqYSrc;                 // F0-04  ����Ƶ��ԴYѡ��
    Uint16 frqYRangeBase;           // F0-05  ����Ƶ��ԴY��Χѡ��
    Uint16 frqYRange;               // F0-06  ����Ƶ��ԴY��Χ
    Uint16 frqCalcSrc;              // F0-07  Ƶ��Դѡ��
    Uint16 presetFrq;               // F0-08  Ԥ��Ƶ��
    Uint16 runDir;                  // F0-09  ���з���
    Uint16 maxFrq;                  // F0-10  ���Ƶ��
    Uint16 upperFrqSrc;             // F0-11  ����Ƶ��Դ
    Uint16 upperFrq;                // F0-12  ����Ƶ����ֵ�趨
    Uint16 upperFrqOffset;          // F0-13  ����Ƶ��ƫ��
    Uint16 lowerFrq;                // F0-14  ����Ƶ����ֵ�趨
    Uint16 carrierFrq;              // F0-15  �ز�Ƶ��
    Uint16 varFcByTem;              // F0-16  �ز�Ƶ�����¶ȵ���
    Uint16 accTime1;                // F0-17  ����ʱ��1
    Uint16 decTime1;                // F0-18  ����ʱ��1
    Uint16 accDecTimeUnit;          // F0-19  �Ӽ���ʱ��ĵ�λ
    Uint16 frqYOffsetSrc;           // F0-20  ����Ƶ��Դƫ��ѡ��
    Uint16 frqYOffsetFc;            // F0-21  ����Ƶ��Դƫ�õ������趨
    Uint16 frqPoint;                // F0-22  Ƶ��ָ��С����
    Uint16 frqRemMode;              // F0-23  �����趨Ƶ�ʼ���ѡ��
    enum MOTOR_SN motorSn;          // F0-24  ���ѡ��
    Uint16 accDecBenchmark;         // F0-25  �Ӽ���ʱ���׼Ƶ��
    Uint16 updnBenchmark;           // F0-26  ����ʱƵ��ָ��UP/DOWN��׼
    Uint16 frqRunCmdBind;           // F0-27  ����Դ����Ƶ��Դ
    Uint16 commProtocolSec;         // F0-28  ͨѶЭ��ѡ��

//======================================
// F1 �������
    union MOTOR_PARA motorParaM1;   // F1-00  F1-26 ��1�������
    union PG_PARA    pgParaM1;      // f1-27  F1-36 ��1���PG������
    Uint16 tuneCmd;                 // F1-37  ��гѡ��

//======================================
// F2 ʸ�����Ʋ���
    struct VC_PARA vcParaM1;        // ��1���ʸ�����Ʋ���


//======================================
// F3 V/F���Ʋ���
    Uint16 vfCurve;                 // F3-00  VF�����趨
    Uint16 torqueBoost;             // F3-01  ת������
    Uint16 boostCloseFrq;           // F3-02  ת��������ֹƵ��
    Uint16 vfFrq1;                  // F3-03  ���VFƵ�ʵ�1
    Uint16 vfVol1;                  // F3-04  ���VF��ѹ��1
    Uint16 vfFrq2;                  // F3-05  ���VFƵ�ʵ�2
    Uint16 vfVol2;                  // F3-06  ���VF��ѹ��2
    Uint16 vfFrq3;                  // F3-07  ���VFƵ�ʵ�3
    Uint16 vfVol3;                  // F3-08  ���VF��ѹ��3
    Uint16 slipCompCoef;            // F3-09  ת���ϵ��
    Uint16 vfOverMagGain;           // F3-10  VF����������
    Uint16 antiVibrateGain;         // F3-11  ����������

    Uint16 rsvdF31;//antiVibrateGainMode;     // F3-12  ����������ģʽ
    Uint16 vfVoltageSrc;            // F3-13  VF����ĵ�ѹԴ
    Uint16 vfVoltageDigtalSet;      // F3-14  VF����ĵ�ѹԴ�����趨
    Uint16 vfVoltageAccTime;        // F3-15  VF����ĵ�ѹ����ʱ��

//======================================
// F4 �������
    Uint16 diFunc[DI_NUMBER_PHSIC]; // F4-00  --F4-09   DI1���ӹ���ѡ��
    
    Uint16 diFilterTime;            // F4-10  DI�˲�ʱ��
    Uint16 diControlMode;           // F4-11  �������ʽ
    Uint16 diUpDownSlope;           // F4-12  ����UP/DOWN���ʣ���Ϊ0.001Hz

    Uint16 curveSet2P1[4];          // F4-13,...,F4-16  ����1��2�㣬���ֵ����Сֵ
    Uint16 ai1FilterTime;           // F4-17  AI1�˲�ʱ��, 10ms

    Uint16 curveSet2P2[4];          // F4-18,...,F4-21  ����2��2��
    Uint16 ai2FilterTime;           // F4-22  AI2�˲�ʱ��, 10ms

    Uint16 curveSet2P3[4];          // F4-23,...,F4-26  ����3��2��
    Uint16 ai3FilterTime;           // F4-27  AI3�˲�ʱ��, 10ms

    Uint16 curveSet2P4[4];          // F4-28,...,F4-31  HDI���ߣ�2��
    Uint16 pulseInFilterTime;       // F4-32  PULSE�˲�ʱ��, 10ms

    Uint16 aiCurveSrc;              // F4-33  AI�趨����ѡ��

    Uint16 aiLimitSrc;              // F4-34  AI����ѡ��

    Uint16 diDelayTime[3];          // F4-35  DI1�ӳ�ʱ��
    Uint16 diLogic[2];              // F4-38  DI��Ч״̬ѡ��1
                                    // F4-39  DI��Ч״̬ѡ��2

//======================================
// F5 �������
    Uint16 fmOutSelect;             // F5-00  �๦�ܶ������ѡ��
    Uint16 doFunc[DO_NUMBER_PHSIC]; // F5-01  FMR���ѡ��
                                    // F5-02  ���ư�RELAY���ѡ��
                                    // F5-03  ��չ��RELAY���ѡ��
                                    // F5-04  DO1���ѡ��
                                    // F5-05  ��չ��DO2���ѡ��

    Uint16 aoFunc[AO_NUMBER+HDO_NUMBER];    // F5-06  FMP���ѡ��
                                            // F5-07  AO1���ѡ��
                                            // F5-08  ��չ��AO2���ѡ��
    Uint16 fmpOutMaxFrq;                    // F5-09  FMP������Ƶ��

    AO_PARA aoPara[AO_NUMBER];              // F5-10  AO1��ƫϵ��
                                            // F5-11  AO1����
                                            // F5-12  AO2��ƫϵ��
                                            // F5-13  AO2����
    Uint16 aoLpfTime[AO_NUMBER+HDO_NUMBER]; // F5-14  HDO,AO1,AO2����˲�ʱ��
    
    Uint16 doDelayTime[DO_NUMBER_PHSIC];// F5-17  RELAY1����ӳ�ʱ��
                                        // F5-18  RELAY2����ӳ�ʱ��
                                        // F5-19  DO1����ӳ�ʱ��
                                    
                                        // F5-20  DO2����ӳ�ʱ��
                                        // F5-21  DO3����ӳ�ʱ��
    Uint16 doLogic;                     // F5-22  DO��Ч״̬ѡ��

//======================================
// F6 ��ͣ����
    Uint16 startMode;               // F6-00  ������ʽ
    Uint16 speedTrackMode;          // F6-01  ת�ٸ��ٷ�ʽ
    Uint16 speedTrackVelocity;      // F6-02  ת�ٸ��ٿ���
    Uint16 startFrq;                // F6-03  ����Ƶ��
    Uint16 startFrqTime;            // F6-04  ����Ƶ�ʱ���ʱ��
    Uint16 startBrakeCurrent;       // F6-05  ����ֱ���ƶ�����
    Uint16 startBrakeTime;          // F6-06  ����ֱ���ƶ�ʱ��
    Uint16 accDecSpdCurve;          // F6-07  �Ӽ��ٷ�ʽ
    Uint16 sCurveStartPhaseTime;    // F6-08  S���߿�ʼ��ʱ�����
    Uint16 sCurveEndPhaseTime;      // F6-09  S���߽�����ʱ�����
    Uint16 stopMode;                // F6-10  ͣ����ʽ
    Uint16 stopBrakeFrq;            // F6-11  ͣ��ֱ���ƶ���ʼƵ��
    Uint16 stopBrakeWaitTime;       // F6-12  ͣ��ֱ���ƶ��ȴ�ʱ��
    Uint16 stopBrakeCurrent;        // F6-13  ͣ��ֱ���ƶ�����
    Uint16 stopBrakeTime;           // F6-14  ͣ��ֱ���ƶ�ʱ��
    Uint16 brakeDutyRatio;          // F6-15  �ƶ�ʹ����

//======================================
// F7 ��������ʾ
    Uint16 rsvdF71;                 // F7-00  ����
    Uint16 mfkKeyFunc;              // F7-01  MF.K������ѡ��
    Uint16 stopKeyFunc;             // F7-02  STOP������
    Uint16 ledDispParaRun1;         // F7-03  LED������ʾ����1
    Uint16 ledDispParaRun2;         // F7-04  LED������ʾ����2
    
    Uint16 ledDispParaStop;         // F7-05  LEDͣ����ʾ����
    Uint16 speedDispCoeff;          // F7-06  �����ٶ���ʾϵ��
    Uint16 radiatorTemp;            // F7-07  �����ģ��ɢ�����¶�
    Uint16 temp2;                   // F7-08  ������ɢ�����¶�
    Uint16 runTimeAddup;            // F7-09  �ۼ�����ʱ��, ��λ: h
    
    Uint16 productVersion;          // F7-10  ��Ʒ��
    Uint16 softVersion;             // F7-11  ����汾��
    Uint16 speedDispPointPos;       // F7-12  �����ٶ���ʾС����λ��
    Uint16 powerUpTimeAddup;        // F7-13  �ۼ��ϵ�ʱ��
    Uint16 powerAddup;              // F7-14  �ۼƺĵ���
    

//======================================
// F8 ��������
    Uint16 jogFrq;                  // F8-00  �㶯����Ƶ��
    Uint16 jogAccTime;              // F8-01  �㶯����ʱ��
    Uint16 jogDecTime;              // F8-02  �㶯����ʱ��
    Uint16 accTime2;                // F8-03  ����ʱ��2
    Uint16 decTime2;                // F8-04  ����ʱ��2
    Uint16 accTime3;                // F8-05  ����ʱ��3
    Uint16 decTime3;                // F8-06  ����ʱ��3
    Uint16 accTime4;                // F8-07  ����ʱ��4
    Uint16 decTime4;                // F8-08  ����ʱ��4
    Uint16 jumpFrq1;                // F8-09  ��ԾƵ��1
    Uint16 jumpFrq2;                // F8-10  ��ԾƵ��2
    Uint16 jumpFrqRange;            // F8-11  ��ԾƵ�ʷ���
    Uint16 zeroSpeedDeadTime;       // F8-12  ����ת����ʱ��
    Uint16 antiReverseRun;          // F8-13  ��ת����, 0-����ת��1-��ֹ��ת
    Uint16 lowerDeal;               // F8-14  Ƶ�ʵ�������Ƶ�����ж���
    Uint16 droopCtrl;               // F8-15  �´�����
    Uint16 powerUpTimeArriveSet;    // F8-16  �趨�ϵ絽��ʱ��
    Uint16 runTimeArriveSet;        // F8-17  �趨���е���ʱ��
    Uint16 startProtect;            // F8-18  ��������ѡ��
    Uint16 frqFdtValue;             // F8-19  Ƶ�ʼ��ֵ(FDT��ƽ)
    Uint16 frqFdtLag;               // F8-20  Ƶ�ʼ���ͺ�ֵ
    Uint16 frqArriveRange;          // F8-21  Ƶ�ʵ���������
    Uint16 jumpFrqMode;             // F8-22  �Ӽ��ٹ�������ԾƵ���Ƿ���Ч
    Uint16 runTimeOverAct;          // F8-23  �趨����ʱ�䵽�ﶯ��ѡ��
  
    // ADD
    Uint16 powerUpTimeOverAct;      // F8-24  �趨�ϵ�ʱ�䵽�ﶯ��ѡ��
    Uint16 accTimefrqChgValue;      // F8-25  ����ʱ��1/2�л�Ƶ�ʵ� 
    Uint16 decTimefrqChgValue;      // F8-26  ����ʱ��1/2�л�Ƶ�ʵ� 
    Uint16 jogWhenRun;              // F8-27  ���ӵ㶯����
    Uint16 frqFdt1Value;            // F8-28  Ƶ�ʼ��ֵ(FDT1��ƽ)
    Uint16 frqFdt1Lag;              // F8-29  Ƶ�ʼ��1�ͺ�ֵ
    Uint16 frqArriveValue1;         // F8-30  Ƶ�ʵ�����ֵ1 
    Uint16 frqArriveRange1;         // F8-31  Ƶ�ʵ�����1����
    Uint16 frqArriveValue2;         // F8-32  Ƶ�ʵ�����ֵ2 
    Uint16 frqArriveRange2;         // F8-33  Ƶ�ʵ�����2����

    Uint16 oCurrentChkValue;        // F8-34  ��������ֵ
    Uint16 oCurrentChkTime;         // F8-35  ���������ӳ�ʱ��
    Uint16 softOCValue;             // F8-36  ���������
    Uint16 softOCDelay;             // F8-37  �����������ӳ�ʱ��

    Uint16 currentArriveValue1;     // F8-38  ����������ֵ1
    Uint16 currentArriveRange1;     // F8-39  ����������1����
    Uint16 currentArriveValue2;     // F8-40  ����������ֵ1
    Uint16 currentArriveRange2;     // F8-41  ����������1����

    Uint16 setTimeMode;             // F8-42  ��ʱ����ѡ��
    Uint16 setTimeSource;           // F8-43  ��ʱʱ���趨ѡ��
    Uint16 setTimeValue;            // F8-44  �趨����ʱ��
    
    Uint16 ai1VoltageLimit;         // F8-45  AI1�����ѹ����
    Uint16 ai1VoltageUpper;         // F8-46  AI1�����ѹ����

    Uint16 temperatureArrive;       // F8-47  ģ���¶ȵ���
    Uint16 fanControl;              // F8-48  ���ȿ���
    Uint16 wakeUpFrq;               // F8-49  ����Ƶ��
    Uint16 wakeUpTime;              // F8-50  �����ӳ�ʱ��
    Uint16 dormantFrq;              // F8-51  ����Ƶ��
    Uint16 dormantTime;             // F8-52  �����ӳ�ʱ��
    Uint16 setTimeArrive;           // F8-53  ��ǰ���е���ʱ��
    
//======================================
// F9 �����뱣��
    Uint16 overloadMode;                // F9-00  ������ر���ѡ��
    Uint16 overloadGain;                // F9-01  ������ر�������
    Uint16 foreOverloadCoef;            // F9-02  �������Ԥ��ϵ��
    Uint16 ovGain;                      // F9-03  ��ѹʧ������
    Uint16 ovPoint;                     // F9-04  ��ѹʧ�ٱ�����ѹ
    
    Uint16 ocGain;                      // F9-05  ����ʧ������
    Uint16 ocPoint;                     // F9-06  ����ʧ�ٱ�������
    Uint16 shortCheckMode;              // F9-07  �ϵ�Եض�·��������
    Uint16 rsvdF91;                     // F9-08  ����
    Uint16 errAutoRstNumMax;            // F9-09  �����Զ���λ����
    
    Uint16 errAutoRstRelayAct;          // F9-10  �����Զ���λ�ڼ���ϼ̵�������ѡ��
    Uint16 errAutoRstSpaceTime;         // F9-11  �����Զ���λ���ʱ��, 0.1s 
    Uint16 inPhaseLossProtect;          // F9-12  ����ȱ�ౣ��ѡ��
    Uint16 outPhaseLossProtect;         // F9-13  ���ȱ�ౣ��ѡ��
    Uint16 errorLatest1;                // F9-14  ��һ�ι�������
    
    Uint16 errorLatest2;                // F9-15  �ڶ��ι�������
    Uint16 errorLatest3;                // F9-16  ������(���һ��)��������

    union ERROR_SCENE errorScene3;      // F9-17  ������(���һ��)����ʱƵ��                              
                                        // F9-18  ������(���һ��)����ʱ����                             
                                        // F9-19  ������(���һ��)����ʱĸ�ߵ�ѹ                                                                                  
                                        // F9-20  ������(���һ��)����ʱ�������״̬                     
                                        // F9-21  ������(���һ��)����ʱ�������״̬   
                                        
                                        // F9-22  ������(���һ��)����ʱ��Ƶ��״̬                       
                                        // F9-23  ������(���һ��)����ʱʱ�䣨�ӱ����ϵ翪ʼ��ʱ��       
                                        // F9-24  ������(���һ��)����ʱʱ�䣨������ʱ��ʼ��ʱ��         

    Uint16 rsvdF92[2];                  // F9-25  F9-26
    
    union ERROR_SCENE errorScene2;      // F9-27  �ڶ��ι����ֳ�
    Uint16 rsvdF921[2];                 // F9-35  F9-36

    union ERROR_SCENE errorScene1;      // F9-37  ��һ�ι����ֳ�
    Uint16 rsvdF922[2];                 // F9-45  F9-46
    
    Uint16 errorAction[5];              // F9-47  -F9-51  ����ʱ��������ѡ��1-5
    Uint16 errorShow[2];                // F9-52  -F9-53  ����ָʾѡ��1,2
    Uint16 errorRunFrqSrc;              // F9-54  ����ʱ��������Ƶ��ѡ��
    Uint16 errorSecondFrq;              // F9-55  �쳣����Ƶ���趨
    
    Uint16 motorOtMode;                 // F9-56  ����¶ȴ���������
    Uint16 motorOtProtect;              // F9-57  ������ȱ�����ֵ
    Uint16 motorOtCoef;                 // F9-58  �������Ԥ������ֵ
     
    Uint16 pOffTransitoryNoStop;        // F9-59  ˲ͣ��ͣ����ѡ��
    Uint16 pOffTransitoryFrqDecSlope;   // F9-60  ˲ͣ������ͣ�жϵ�ѹ
    Uint16 pOffVolBackTime;             // F9-61  ˲ͣ��ͣ��ѹ�����ж�ʱ��
    
    Uint16 pOffThresholdVol;            // F9-62  ˲ͣ��ͣ�����жϵ�ѹ
    Uint16 loseLoadProtectMode;         // F9-63  ���ر���ѡ��
    Uint16 loseLoadLevel;               // F9-64  ���ؼ��ˮƽ
    Uint16 loseLoadTime;                // F9-65  ���ؼ��ʱ��
    Uint16 rsvdF923;                    // F9-66  ����Ƶ�ʵ�С����

    Uint16 osChkValue;                  // F9-67 ���ٶȼ��ֵ
    Uint16 osChkTime;                   // F9-68 ���ٶȼ��ʱ��
    Uint16 devChkValue;                 // F9-69 �ٶ�ƫ�������ֵ
    Uint16 devChkTime;                  // F9-70 �ٶ�ƫ�������ʱ��

#if 0 
    Uint16 losePowerStopEnable;         // F9-71 ����ͣ��������Ч
    Uint16 losePowerStopSel;            // F9-72 ����ͣ������ʽ
    Uint16 losePowerLowerFrq;           // F9-73 �����ʼ����Ƶ��
    Uint16 losePowerLowerTime;          // F9-74 �����ʼ����ʱ��
    Uint16 losePowerDectime;            // F9-75 �������ʱ��
    Uint16 losePowerP;                  // F9-76 ��������ѹ�ջ�����
    Uint16 losePowerI;                  // F9-77 ��������ѹ�ջ�����
    
#endif
    

//======================================
// FA PID����
    Uint16 pidSetSrc;               // FA-00  PID����Դ
    Uint16 pidSet;                  // FA-01  PID��ֵ����, 0.1%
    Uint16 pidFdbSrc;               // FA-02  PID����Դ
    Uint16 pidDir;                  // FA-03  PID���÷���
    Uint16 pidDisp;                 // FA-04  PID������������
    
    Uint16 pidKp;                   // FA-05  ��������P
    Uint16 pidTi;                   // FA-06  ����ʱ��I
    Uint16 pidTd;                   // FA-07  ΢��ʱ��D
    Uint16 reverseCutOffFrq;        // FA-08  PID��ת��ֹƵ��
    Uint16 pidErrMin;               // FA-09  PIDƫ���
    Uint16 pidDLimit;               // FA-10  PID΢���޷�
    Uint16 pidSetChangeTime;        // FA-11  PID�����仯ʱ��
    Uint16 pidFdbLpfTime;           // FA-12  PID�����˲�ʱ��
    Uint16 pidOutLpfTime;           // FA-13  PID����˲�ʱ��
    Uint16 pidSampleTime;           // FA-14  PID��������(��δ��)
    Uint16 pidKp2;                  // FA-15  PID��������P2
    Uint16 pidTi2;                  // FA-16  PID����ʱ��I2
    Uint16 pidTd2;                  // FA-17  PID΢��ʱ��D2
    Uint16 pidParaChgCondition;     // FA-18  PID�����л�����
    Uint16 pidParaChgDelta1;        // FA-19  PID�����л�ƫ��1
    Uint16 pidParaChgDelta2;        // FA-20  PID�����л�ƫ��2
    Uint16 pidInit;                 // FA-21  PID��ֵ
    Uint16 pidInitTime;             // FA-22  PID��ֵ����ʱ��
    Uint16 pidOutDeltaMax;          // FA-23  PID�������֮��ƫ������ֵ
    Uint16 pidOutDeltaMin;          // FA-24  PID�������֮��ƫ�����Сֵ
    Uint16 pidIAttribute;           // FA-25  PID��������
    Uint16 pidFdbLoseDetect;        // FA-26  PID������ʧ���ֵ
    Uint16 pidFdbLoseDetectTime;    // FA-27  PID������ʧ���ʱ��
    Uint16 pidCalcMode;             // FA-28  PID����ģʽ(ͣ���Ƿ�����). ��ˮģʽ�£�ͣ��ʱPIDҲ����.

//======================================
// FB ��Ƶ�������ͼ���
    Uint16 swingBaseMode;           // FB-00  ��Ƶ�趨��ʽ
    Uint16 swingAmplitude;          // FB-01  ��Ƶ����
    Uint16 swingJumpRange;          // FB-02  ͻ��Ƶ�ʷ���
    Uint16 swingPeriod;             // FB-03  ��Ƶ����
    Uint16 swingRiseTimeCoeff;      // FB-04  ��Ƶ�����ǲ�����ʱ��
    Uint16 lengthSet;               // FB-05  �趨����
    Uint16 lengthCurrent;           // FB-06  ʵ�ʳ���
    Uint16 lengthPulsePerMeter;     // FB-07  ÿ������������λ: 0.1
    Uint16 counterSet;              // FB-08  �趨����ֵ
    Uint16 counterPoint;            // FB-09  ָ������ֵ

//======================================
// FC ����١�PLC
    Uint16 plcFrq[PLC_STEP_MAX];                  // FC-00  --FC-15   ���ָ��0-���ָ��15
    Uint16 plcRunMode;                  // FC-16  PLC���з�ʽ
    Uint16 plcPowerOffRemMode;          // FC-17  PLC�������ѡ��
    struct PLC_STRUCT plcAttribute[PLC_STEP_MAX]; // FC-18  --FC-49   PLC����ʱ�䣬�Ӽ���ʱ��ѡ��
    Uint16 plcTimeUnit;                 // FC-50  PLC����ʱ�䵥λ
    Uint16 plcFrq0Src;                  // FC-51  ���ָ��0������ʽ

// FD ͨѶ����
    Uint16 commBaudRate;            // FD-00  ������
    Uint16 commParity;              // FD-01  ���ݸ�ʽ
    Uint16 commSlaveAddress;        // FD-02  ������ַ
    Uint16 commDelay;               // FD-03  Ӧ���ӳ�
    Uint16 commOverTime;            // FD-04  ͨѶ��ʱʱ��
    Uint16 commProtocol;            // FD-05  ͨѶ���ݴ��͸�ʽѡ��   
    Uint16 commReadCurrentPoint;    // FD-06  ͨѶ��ȡ�����ֱ���
    Uint16 commMaster;              // FD-07  ͨѶ���ӷ�ʽ

// FE  280��320û�еĹ�����
    Uint16 userCustom[FENUM];       // FE �û����ƹ�������

// FF ���Ҳ���
    Uint16 factoryPassword;         // FF-00  ��������
    Uint16 inverterType;            // FF-01  ��Ƶ������
    Uint16 inverterGpType;          // FF-02  G/P�ͺ�
    Uint16 inverterPower;           // FF-03  ��Ƶ������
    Uint16 tempCurve;               // FF-04  �¶�����
    
    Uint16 uvGainWarp;              // FF-05  UV���������������ƫ��
    Uint16 funcSoftVersion;         // FF-06  ����
    Uint16 motorSoftVersion;        // FF-07  ��������汾��
    Uint16 volJudgeCoeff;           // FF-08  ��ѹУ��ϵ��
    Uint16 curJudgeCoeff;           // FF-09  ����У��ϵ��
    
    Uint16 motorDebugFc;            // FF-10  ���ܵ��Թ�������ʾ����
    Uint16 aiaoCalibrateDisp;       // FF-11  AIAOУ����������ʾ
    Uint16 memoryAddr;              // FF-12  �ڴ��ַ�鿴

// FP �û�����, ������ʼ��
    Uint16 userPassword;            // FP-00  �û�����
    Uint16 paraInitMode;            // FP-01  ������ʼ��
    Uint16 funcParaView;            // FP-02  ���ܲ���ģʽ����
    Uint16 menuMode;                // FP-03  ���Ի�����ģʽѡ��
    
    Uint16 userPasswordReadOnly;    // FP-04  ֻ���û�����
    Uint16 rsvdFp;                  // FP-05  ����

// A0 ת�ؿ��ƺ��޶�����
    Uint16 torqueCtrl;              // A0-00  ת�ؿ���
    Uint16 driveUpperTorqueSrc;     // A0-01  ����ת������Դ
    Uint16 brakeUpperTorqueSrc;     // A0-02  �ƶ�ת������Դ
    Uint16 driveUpperTorque;        // A0-03  ����ת������
    Uint16 torqueFilter;            // A0-04  ת���˲�
    Uint16 torqueCtrlFwdMaxFrq;     // A0-05  ת�ؿ����������Ƶ��
    Uint16 torqueCtrlRevMaxFrq;     // A0-06  ת�ؿ��Ʒ������Ƶ��
    Uint16 torqueCtrlAccTime;       // A0-07  ת�ؼ���ʱ��
    Uint16 torqueCtrlDecTime;       // A0-08  ת�ؼ���ʱ��
    
// A1 ����DI������DO
    Uint16 vdiFunc[5];              // A1-00  --A1-04 VDI1���ӹ���ѡ��
    Uint16 vdiSrc;                  // A1-05  VDI������Ч״̬��Դ
    Uint16 vdiFcSet;                // A1-06  VDI���ӹ������趨��Ч״̬
    Uint16 aiAsDiFunc[3];           // A1-07  --A1-09 AI1���ӹ���ѡ�񣨵���DI��
    Uint16 diLogicAiAsDi;           // A1-10  AI��ΪDI��Ч״̬ѡ��
    Uint16 vdoFunc[5];              // A1-11  --A1-15 ����VDO1��VDO5���ѡ��
    Uint16 vdoDelayTime[5];         // A1-16  --A1-20 VDO1��VDO5�ӳ�ʱ��
    Uint16 vdoLogic;                // A1-21  VDO���������Ч״̬ѡ��
    
// A2 ��2�������
    struct MOTOR_FC motorFcM2;      // ��2�������
    
// A3 ��2�������
    struct MOTOR_FC motorFcM3;      // ��3�������
    
// A4 ��2�������
    struct MOTOR_FC motorFcM4;      // ��4�������
    
// A5 �����Ż�����
    Uint16 pwmMode;                 // A5-00    DPWM�л�����Ƶ��
    Uint16 modulationMode;          // A5-01    ���Ʒ�ʽ��0-�첽���ƣ�1-ͬ������
    Uint16 deadCompMode;            // A5-02    ��������ģʽѡ��
    Uint16 softPwm;                 // A5-03    ���PWM
    Uint16 cbcEnable;               // A5-04    ������ʹ��
    Uint16 curSampleDelayComp;      // A5-05    ���������ʱ����
    Uint16 uvPoint;                 // A5-06    Ƿѹ������
    Uint16 svcMode;                 // A5-07    SVC�Ż�ѡ�� 0-���Ż�  1-�Ż�ģʽ1  2-�Ż�ģʽ2
    Uint16 deadTimeSet;             // A5-08    ����ʱ�����-1140Vר��
    Uint16 ovPointSet;              // A5-09    ��ѹ������

      
//======================================
// A6 ģ��������
    Uint16 curveSet4P1[8];          // A6-00    --A6-07  ����4��4�㣬���ֵ����Сֵ��2���м��
    Uint16 curveSet4P2[8];          // A6-08    --A6-15  ����5��4��
    Uint16 rsvdA41[8];

    struct AI_JUMP aiJumpSet[AI_NUMBER]; // A6-24 --A6-29, AI1, AI2, AI3��Ծ

// A7 ����               
    Uint16 plcEnable;               // A7-00 PLC������ѡ��
    Uint16 outPortControl;          // A7-01 ������ӿ���
    Uint16 plcAI3Cfg;               // A7-02 PLC AI3��������
    Uint16 fmpValue;                // A7-03 FMP��� 
    Uint16 ao1Value;                // A7-04 AO1���
    Uint16 inPortOut;               // A7-05 ���������
    Uint16 plcFrqSet;               // A7-06 PLC��Ƶ�ʸ���
    Uint16 plcTorqueSet;            // A7-07 PLC��ת�ظ���
    Uint16 plcCmd;                  // A7-08 PLC���������
    Uint16 plcErrorCode;            // A7-09 PLC�����ϸ���
    Uint16 rsvdA7[2];

    
// A8 ����  
    Uint16 p2pEnable;               // A8-00 ��Ե�ͨѶ����ѡ��
    Uint16 p2pTypeSel;              // A8-01 ����ѡ��
    Uint16 p2pSendDataSel;          // A8-02 ������������   0:���ת��  1:����Ƶ��  2:�趨Ƶ��  3:�����ٶ�
    Uint16 p2pRevDataSel;           // A8-03 �ӻ���������   0:ת�ظ���  1:Ƶ�ʸ���  
    Uint16 p2pRevOffset;            // A8-04 ����������ƫ
    Uint16 p2pRevGain;              // A8-05 ������������
    Uint16 p2pOverTime;             // A8-06 ��Ե�ͨѶ�жϼ��ʱ��
    Uint16 p2pSendPeriod;           // A8-07 ��Ե�ͨѶ�������ݷ�������
    
// A9 ����  
    Uint16 A9[A9NUM];               // A9

// AA ʸ���Ż�����

    Uint16 AA[AANUM];               // AA
    #if 0
    Uint16 motorCtrlM1;             // AA-00 ���ŵ�����ʽ
    Uint16 motorCtrlM2;             // AA-01 ����ѹ���㷽ʽ
    Uint16 motorCtrlM3;             // AA-02 ���ŵ���������KP
    Uint16 motorCtrlM4;             // AA-03 ���ŵ���������KI
    Uint16 motorCtrlM5;             // AA-04 ���ŵ�������������
    Uint16 motorCtrlM6;             // AA-05 ���ŵ�������������
    Uint16 motorCtrlM7;             // AA-06 ת���������
    Uint16 motorCtrlM8;             // AA-07 ת���������
    Uint16 motorCtrlM9;             // AA-08 ��������
    Uint16 motorCtrlM10;            // AA-09 ���Ƶ������ģʽ
    Uint16 motorCtrlM11;            // AA-10 ��ѹ������ֵ����
    Uint16 motorCtrlM12;            // AA-11 ��ѹ��������
    Uint16 motorCtrlM13;            // AA-12 �ٶȻ�����
    Uint16 motorCtrlM14;            // AA-13 �������˲�
    Uint16 motorCtrlM15;            // AA-14 ����ǶȲ���
    Uint16 motorCtrlM16;            // AA-15 SVCת�ؿ����Ż�
    #endif
// AB VF�Ż�����    
    Uint16 AB[ABNUM];                // AB
    #if 0
    Uint16 vfCtrlM2;                // AB-01 DPWM�л�����Ƶ��   86
    Uint16 vfCtrlM3;                // AB-02 ���������Ż�����   87
    Uint16 vfCtrlM4;                // AB-03 ����ǯλ����ϵ��    1
    Uint16 vfCtrlM5;                // AB-04 ǯλ�Ż�����Ƶ��   101
    Uint16 vfCtrlM6;                // AB-05 ǯλ�Ż�����Ƶ��   102
    Uint16 vfCtrlM7;                // AB-06 ������ģʽ       89
    Uint16 vfCtrlM8;                // AB-07 �����Ʒ�ֵ����   90
    #endif
// AC AIAOУ��
    struct ANALOG_CALIBRATE_CURVE aiCalibrateCurve[AI_NUMBER];  // AC-00    ----AC-11, AI1/2/3У������
    struct ANALOG_CALIBRATE_CURVE aoCalibrateCurve[AO_NUMBER];  // AC-12    ----AC-19, AO1/AO2У������
    
// AD ����    
    Uint16 AD[ADNUM]; 

// AE AIAO����У��ֵ
    struct ANALOG_CALIBRATE_CURVE aiFactoryCalibrateCurve[AI_NUMBER];   // AE-00 
    struct ANALOG_CALIBRATE_CURVE aoFactoryCalibrateCurve[AO_NUMBER];   // AE-12

    Uint16 AF[AFNUM];               // AF
                                    
//======================================
    Uint16 b0[B0NUM];               // B0
    Uint16 b1[B1NUM];               // B1
    Uint16 b2[B2NUM];               // B2
    Uint16 b3[B3NUM];               // B3

//======================================
    Uint16 b4[B4NUM];               // B4
    Uint16 b5[B5NUM];               // B5
    Uint16 b6[B6NUM];               // B6
    Uint16 b7[B7NUM];               // B7
    
//======================================
    Uint16 b8[B8NUM];               // B8
    Uint16 b9[B9NUM];               // B9
    Uint16 ba[BANUM];               // BA
    Uint16 bb[BBNUM];               // BB

//======================================
    Uint16 bc[BCNUM];               // BC
    Uint16 bd[BDNUM];               // BD
    Uint16 be[BENUM];               // BE
    Uint16 bf[BFNUM];               // BF
//======================================


//======================================
    Uint16 c0[C0NUM];               // C0
    Uint16 c1[C1NUM];               // C1
    Uint16 c2[C2NUM];               // C2
    Uint16 c3[C3NUM];               // C3

//======================================
    Uint16 c4[C4NUM];               // C4
    Uint16 c5[C5NUM];               // C5
    Uint16 c6[C6NUM];               // C6
    Uint16 c7[C7NUM];               // C7

//======================================
    Uint16 c8[C8NUM];               // C8
    Uint16 c9[C9NUM];               // C9
    Uint16 ca[CANUM];               // CA
    Uint16 cb[CBNUM];               // CB

//======================================
    Uint16 cc[CCNUM];               // CC   
    Uint16 cd[CDNUM];               // CD
    Uint16 ce[CENUM];               // CE
    Uint16 cf[CFNUM];               // CF
//======================================

//======================================
// eepromCheckWord
    Uint16 rsvd4All;                // ������������ǰ��
    Uint16 eepromCheckWord1;        //        eepromCheckWord1
    Uint16 eepromCheckWord2;        //        eepromCheckWord2
    Uint16 aiaoChkWord;             // AIAO����У��

//======================================
// REMEMBER ���籣�棬��48��
    Uint16 extendType;                  // FR-00  extendType
    Uint16 plcStepRem;                  // FR-01  PLC��ǰstep
    Uint16 plcTimeHighRem;              // FR-02  PLC��ǰstep���е�ʱ�䣬��λ
    Uint16 plcTimeLowRem;               // FR-03  PLC��ǰstep���е�ʱ�䣬��λ
    Uint16 dispParaRunBit;              // FR-04  ����ʱLED��ʾ������bitλֵ
    Uint16 dispParaStopBit;             // FR-05  ͣ��ʱLED��ʾ������bitλ��
    Uint16 runTimeAddupSec;             // FR-06  �ۼ�����ʱ���s(��)
    Uint16 counterTicker;               // FR-07  �����������ticker
    Uint16 lengthTickerRemainder;       // FR-08  ���ȼ�������tickerDelta��Remainder
    Uint16 frqComm;                     // FR-09  ͨѶ�޸�Ƶ��ֵ, 100.00%-maxFrq
    Uint16 upDownFrqRem;                // FR-10  UP/DOWN��Ƶ��
    Uint16 pmsmRotorPos;                // FR-11  ͬ����ת��λ��
    Uint16 powerAddupInt;               // FR-12  �ۼƺĵ�����������
    Uint16 powerUpTimeAddupSec;         // FR-13  �ۼ��ϵ�ʱ���s(��)
    Uint16 errorFrqUnit;                // FR-14  ����ʱƵ�ʼ�¼
    Uint16 saveUserParaFlag1;           // FR-15  �ѱ����û�������־1
    Uint16 saveUserParaFlag2;           // FR-16  �ѱ����û�������־2
    Uint16 speedFdbDir;                 // FR-17  ��������ٶȷ���
    Uint16 rsvdRem[2];                  // FR-18~FR-19
    Uint16 rsvdRem1[23];                // Ԥ��
    Uint16 remPOffMotorCtrl[REM_P_OFF_MOTOR];  // FR-43~FR-47  ����ʹ�õĵ������
//======================================
    Uint16 u0[U0NUM];               // U0 ��ʾ
    Uint16 u1[U1NUM];               // U1
    Uint16 u2[U2NUM];               // U2
    Uint16 u3[U3NUM];               // U3

//======================================
    Uint16 u4[U4NUM];               // U4
    Uint16 u5[U5NUM];               // U5
    Uint16 u6[U6NUM];               // U6
    Uint16 u7[U7NUM];               // U7

//======================================
    Uint16 u8[U8NUM];               // U8
    Uint16 u9[U9NUM];               // U9
    Uint16 ua[UANUM];               // UA
    Uint16 ub[UBNUM];               // UB

//======================================
    Uint16 uc[UCNUM];               // UC
    Uint16 ud[UDNUM];               // UD
    Uint16 ue[UENUM];               // UE
    Uint16 uf[UFNUM];               // UF, ��ʾ�����ܵ���ʹ��
//======================================
};


//=====================================================================
//
// ������Ķ��塣
// �����壬��Ա�ֱ�Ϊ���飬�ṹ�壬�ṹ��
// ���ǣ�һ��������ķ��ʣ������ַ�ʽ:
// funcCode.all[index]     index = GetCodeIndex(funcCode.code.presetFrq);
// funcCode.group.f0[8]    index = GetCodeIndex(funcCode.group.f0[8]);
// funcCode.code.presetFrq
// 
//=====================================================================
typedef union FUNCCODE_ALL_UNION
{
    Uint16 all[FNUM_ALL];

    struct FUNCCODE_GROUP group;

    struct FUNCCODE_CODE code;
} FUNCCODE_ALL;


// ����Ƶ��Դѡ��
#define FUNCCODE_frqXySrc_FC                0   // �������趨�����粻����
#define FUNCCODE_frqXySrc_FC_P_OFF_REM      1   // �������趨���������
#define FUNCCODE_frqXySrc_AI1               2   // AI1
#define FUNCCODE_frqXySrc_AI2               3   // AI2
#define FUNCCODE_frqXySrc_AI3               4   // AI3
#define FUNCCODE_frqXySrc_PULSE             5   // PULSE�����趨(DI5)
#define FUNCCODE_frqXySrc_MULTI_SET         6   // ���ָ��
#define FUNCCODE_frqXySrc_PLC               7   // PLC
#define FUNCCODE_frqXySrc_PID               8   // PID
#define FUNCCODE_frqXySrc_COMM              9   // ͨѶ�趨

// ����Ƶ��ԴY��Χѡ��
#define FUNCCODE_frqYRangeBase_MAX_FRQ      0   // ��������Ƶ��
#define FUNCCODE_frqYRangeBase_FRQ_X        1   // �������Ƶ��ԴX

// Ƶ��Դ(�л���ϵ)ѡ��
#define FUNCCODE_frqCalcSrc_X               0   // ��Ƶ��ԴX
#define FUNCCODE_frqCalcSrc_COMPOSE         1   // ����������
#define FUNCCODE_frqCalcSrc_X_OR_Y          2   // �� <--> ��
#define FUNCCODE_frqCalcSrc_X_OR_COMPOSE    3   // �� <--> ����������
#define FUNCCODE_frqCalcSrc_Y_OR_COMPOSE    4   // �� <--> ����������

// ����Ƶ�������ϵ
#define FUNCCODE_frqCalcSrc_ADD             0   // �� + ��
#define FUNCCODE_frqCalcSrc_SUBTRATION      1   // �� - ��
#define FUNCCODE_frqCalcSrc_MAX             2   // MAX(��, ��)
#define FUNCCODE_frqCalcSrc_MIN             3   // MIN(��, ��)
#define FUNCCODE_frqCalcSrc_4               4   // 
#define FUNCCODE_frqCalcSrc_5               5   // 

// ����Ƶ��Դ
#define FUNCCODE_upperFrqSrc_FC         0   // �������趨
#define FUNCCODE_upperFrqSrc_AI1        1   // AI1
#define FUNCCODE_upperFrqSrc_AI2        2   // AI2
#define FUNCCODE_upperFrqSrc_AI3        3   // AI3
#define FUNCCODE_upperFrqSrc_PULSE      4   // PULSE�����趨(DI5)
#define FUNCCODE_upperFrqSrc_COMM       5   // ͨѶ����

// Ƶ��ָ��С����
#define FUNCCODE_frqPoint_1             0   // 0: 0��С���㣬1Hz
#define FUNCCODE_frqPoint_0_1           1   // 1: 1��С���㣬0.1Hz
#define FUNCCODE_frqPoint_0_01          2   // 2: 2��С���㣬0.01Hz

// �ز�Ƶ�ʵ���ѡ��
//#define FUNCCODE_autoCarrierFrq_0

// �Ӽ���ʱ��ĵ�λ
#define FUNCCODE_accDecTimeUnit_0POINT  0   // 0��С���㣬1s
#define FUNCCODE_accDecTimeUnit_1POINT  1   // 1��С���㣬0.1s
#define FUNCCODE_accDecTimeUnit_2POINT  2   // 2��С���㣬0.01s

// ��ֵ�趨Ƶ�ʼ����趨
#define FUNCCODE_frqRemMode_POWEROFF_NO     0   // ���粻����
#define FUNCCODE_frqRemMode_POWEROFF_YES    1   // �������
#define FUNCCODE_frqRemMode_STOP_NO         0   // ͣ��������
#define FUNCCODE_frqRemMode_STOP_YES        1   // ͣ������

// �Ӽ��ٷ�ʽ
#define FUNCCODE_accDecSpdCurve_LINE        0   // ֱ�߼Ӽ���
#define FUNCCODE_accDecSpdCurve_S_CURVE_A   1   // S����1����ͨ���η�
#define FUNCCODE_accDecSpdCurve_S_CURVE_B   2   // S����2���ο�����S����B
#define ACC_DEC_LINE    FUNCCODE_accDecSpdCurve_LINE
#define ACC_DEC_SA      FUNCCODE_accDecSpdCurve_S_CURVE_A
#define ACC_DEC_SB      FUNCCODE_accDecSpdCurve_S_CURVE_B

// ת������Դ
#define FUNCCODE_upperTorqueSrc_FC      0   // �������趨
#define FUNCCODE_upperTorqueSrc_AI1     1   // AI1
#define FUNCCODE_upperTorqueSrc_AI2     2   // AI2
#define FUNCCODE_upperTorqueSrc_AI3     3   // AI3
#define FUNCCODE_upperTorqueSrc_PULSE   4   // PULSE
#define FUNCCODE_upperTorqueSrc_COMM    5   // ͨѶ
#define FUNCCODE_upperTorqueSrc_MIN_AI1_AI2 6  // min(ai1,ai2)
#define FUNCCODE_upperTorqueSrc_MAX_AI1_AI2 7  // max(ai1,ai2)

// FVC��PG��ѡ��, 0-QEP1(����PG),1-QEP2(��չPG)
#define FUNCCODE_fvcPgSrc_QEP1          0   // QEP1
#define FUNCCODE_fvcPgSrc_QEP2          1   // QEP2, ��չPG��

#define TIME_UNIT_ACC_DEC_SPEED         100 // �Ӽ���ʱ�䵥λ, ms


// VF�����趨
#define FUNCCODE_vfCurve_Line               0   // ֱ��VF
#define FUNCCODE_vfCurve_DOT                1   // ���VF
#define FUNCCODE_vfCurve_SQUARE             2   // ƽ��VF
#define FUNCCODE_vfCurve_ALL_SEPARATE       10  // VF��ȫ����ģʽ
#define FUNCCODE_vfCurve_HALF_SEPARATE      11  // VF�����ģʽ

// vfVoltageSrc, VF����ĵ�ѹԴ
#define FUNCCODE_vfVoltageSrc_FC            0   // �������趨
#define FUNCCODE_vfVoltageSrc_AI1           1   // AI1
#define FUNCCODE_vfVoltageSrc_AI2           2   // AI2
#define FUNCCODE_vfVoltageSrc_AI3           3   // AI3
#define FUNCCODE_vfVoltageSrc_PULSE         4   // PULSE�����趨(DI5)
#define FUNCCODE_vfVoltageSrc_MULTI_SET     5   // ���ָ��
#define FUNCCODE_vfVoltageSrc_PLC           6   // PLC
#define FUNCCODE_vfVoltageSrc_PID           7   // PID
#define FUNCCODE_vfVoltageSrc_COMM          8   // ͨѶ�趨

// λ�ÿ���ѡ��
#define FUNCCODE_posCtrl_NONE               0   // ��λ�ÿ���
#define FUNCCODE_posCtrl_POSITION_CTRL      1   // λ�ÿ���
#define FUNCCODE_posCtrl_SWITCH_TO_PC       2   // �ٶ�/ת�ؿ���<->λ�ÿ���
#define FUNCCODE_posCtrl_SWITCH_FROM_PC     3   // λ�ÿ���<->�ٶ�/ת�ؿ���

// λ�ÿ���ģʽ
#define FUNCCODE_pcMode_PCMD            0   // Pcmd
#define FUNCCODE_pcMode_APTP            1   // APTP
#define FUNCCODE_pcMode_SWITCH_TO_APTP  2   // Pcmd<->AP2P

// λ��ָ�����巽ʽ
#define FUNCCODE_pcPulseType_PULSE_AND_DIR  0   // ����+����
#define FUNCCODE_pcPulseType_QUADRATURE     1   // 2·��������
#define FUNCCODE_pcPulseType_CW_AND_CCW     2   // CW+CCW

// ��λ����ģʽ
#define FUNCCODE_aptpMode_RELATIVE      0   // ���ʽ
#define FUNCCODE_aptpMode_ABSOLUTE      1   // ����ʽ
#define FUNCCODE_aptpMode_INDEX         2   // �ֶ���

// λ�ÿ������ѡ��
#define FUNCCODE_pcZeroSelect_ENCODER   0   // ������index�ź�
#define FUNCCODE_pcZeroSelect_DI        1   // DI����

// PG����װλ��
#define FUNCCODE_pgLocation_MOTOR       0   // �����
#define FUNCCODE_pgLocation_AXIS        1   // ��������

//=====================================================================
// (���)���Ʒ�ʽ
#define FUNCCODE_motorCtrlMode_SVC  0   // SVC
#define FUNCCODE_motorCtrlMode_FVC  1   // FVC
#define FUNCCODE_motorCtrlMode_VF   2   // VF

// ����Դѡ��
#define FUNCCODE_runSrc_PANEL       0   // ����������ͨ��
#define FUNCCODE_runSrc_DI          1   // ��������ͨ��
#define FUNCCODE_runSrc_COMM        2   // ���п�ͨѶ����ͨ��
#define FUNCCODE_runSrc_AUTO_RUN    3   // �ϵ�����

// ���з���
#define FUNCCODE_runDir_NO_REVERSE      0   // ����һ��
#define FUNCCODE_runDir_REVERSE         1   // �����෴

// ��гѡ��
#define FUNCCODE_tuneCmd_NONE           0   // �޲���
#define FUNCCODE_tuneCmd_ACI_STATIC     1   // �첽����ֹ��г
#define FUNCCODE_tuneCmd_ACI_WHOLE      2   // �첽��������г
#define FUNCCODE_tuneCmd_PMSM_11        11  // ͬ����
#define FUNCCODE_tuneCmd_PMSM_12        12  // ͬ����
#define FUNCCODE_tuneCmd_PMSM_13        13  // ͬ����

// �������ʽ
#define FUNCCODE_diControlMode_2LINE1   0   // ����ʽ1
#define FUNCCODE_diControlMode_2LINE2   1   // ����ʽ2
#define FUNCCODE_diControlMode_3LINE1   2   // ����ʽ1
#define FUNCCODE_diControlMode_3LINE2   3   // ����ʽ2

// �๦�ܶ������ѡ��
#define FUNCCODE_fmOutSelect_PULSE      0   // FMP�������
#define FUNCCODE_fmOutSelect_DO         1   // DO
#define FUNCCODE_fmOutSelect_AO         2   // AO

// ������ʽ
#define FUNCCODE_startMode_DIRECT_START 0   // ֱ������
#define FUNCCODE_startMode_SPEED_TRACK  1   // ת�ٸ���������
#define FUNCCODE_startMode_FORE_MAG     2   // �첽����������

// ͣ����ʽ
#define FUNCCODE_stopMode_DEC_STOP      0   // ����ͣ��
#define FUNCCODE_stopMode_FREESTOP      1   // ����ͣ��
#define FUNCCODE_stopMode_HURRY_STOP    2   // ��ͣͣ��

// Ƶ�ʵ�������Ƶ�����ж���
#define FUNCCODE_lowerDeal_RUN_LOWER    0   // ������Ƶ������
#define FUNCCODE_lowerDeal_DELAY_STOP   1   // ��ʱͣ��
#define FUNCCODE_lowerDeal_RUN_ZERO     2   // ��������

// �趨����ʱ�䵽�ﶯ��ѡ��
#define FUNCCODE_runTimeOverAct_RUN     0   // ��������
#define FUNCCODE_runTimeOverAct_STOP    1   // ͣ��

// �趨�ϵ�ʱ�䵽�ﶯ��ѡ��
#define FUNCCODE_powerUpTimeOverAct_RUN     0   // ��������
#define FUNCCODE_powerUpTimeOverAct_STOP    1   // ͣ��

// PID����Դ
#define FUNCCODE_pidSetSrc_FC               0   // �������趨
#define FUNCCODE_pidSetSrc_AI1              1   // AI1
#define FUNCCODE_pidSetSrc_AI2              2   // AI2
#define FUNCCODE_pidSetSrc_AI3              3   // AI3
#define FUNCCODE_pidSetSrc_PULSE            4   // PULSE
#define FUNCCODE_pidSetSrc_COMM             5   // ͨѶ
#define FUNCCODE_pidSetSrc_MULTI_SET        6   // ���ָ��

// PID����Դ
#define FUNCCODE_pidFdbSrc_AI1              0   // AI1
#define FUNCCODE_pidFdbSrc_AI2              1   // AI2
#define FUNCCODE_pidFdbSrc_AI3              2   // AI3
#define FUNCCODE_pidFdbSrc_AI1_SUB_AI2      3   // AI1-AI2
#define FUNCCODE_pidFdbSrc_PULSE            4   // PULSE
#define FUNCCODE_pidFdbSrc_COMM             5   // ͨѶ
#define FUNCCODE_pidFdbSrc_AI1_ADD_AI2      6   // AI1+AI2
#define FUNCCODE_pidFdbSrc_MAX_AI           7   // MAX(|AI1|, |AI2|)
#define FUNCCODE_pidFdbSrc_MIN_AI           8   // MIN(|AI1|, |AI2|)

// PID�����л�����
#define FUNCCODE_pidParaChgCondition_NO         0   // ���л�
#define FUNCCODE_pidParaChgCondition_DI         1   // DI����
#define FUNCCODE_pidParaChgCondition_PID_ERROR  2   // ����ƫ���Զ��л�

// PID����ģʽ
#define FUNCCODE_pidCalcMode_NO             0   // ͣ��ʱ������
#define FUNCCODE_pidCalcMode_YES            1   // ͣ��ʱ����

// ��Ƶ�趨��ʽ
#define FUNCCODE_swingBaseMode_AGAIN_FRQSETAIM  0   // ���������Ƶ��(�趨Ƶ��)
#define FUNCCODE_swingBaseMode_AGAIN_MAXFRQ     1   // ��������Ƶ��

// MF.K������ѡ��
#define FUNCCODE_mfkKeyFunc_NONE        0   // MF.K��������Ч
#define FUNCCODE_mfkKeyFunc_SWITCH      1   // ��������ͨ���л�
#define FUNCCODE_mfkKeyFunc_REVERSE     2   // ����ת�л�
#define FUNCCODE_mfkKeyFunc_FWD_JOG     3   // ��ת�㶯����
#define FUNCCODE_mfkKeyFunc_REV_JOG     4   // ��ת�㶯����

// STOP/RES������
#define FUNCCODE_stopKeyFunc_KEYBOARD   0   // ͣ�����ܽ��ڼ��̿��Ʒ�ʽʱ��Ч
#define FUNCCODE_stopKeyFunc_ALL        1   // ����Ч

// ���ָ��0������ʽ
#define FUNCCODE_plcFrq0Src_FC          0   // ������FC-00����
#define FUNCCODE_plcFrq0Src_AI1         1   // AI1
#define FUNCCODE_plcFrq0Src_AI2         2   // AI2
#define FUNCCODE_plcFrq0Src_AI3         3   // AI3
#define FUNCCODE_plcFrq0Src_PULSE       4   // PULSE
#define FUNCCODE_plcFrq0Src_PID         5   // PID����
#define FUNCCODE_plcFrq0Src_PRESET_FRQ  6   // Ԥ��Ƶ��

// PLC���з�ʽ
#define FUNCCODE_plcRunMode_ONCE_STOP   0   // �������н���ͣ��
#define FUNCCODE_plcRunMode_ONCE_RUN    1   // �������н���������ֵ
#define FUNCCODE_plcRunMode_REPEAT      2   // һֱѭ��

// PLC�������ѡ��
#define FUNCCODE_plcPowerOffRemMode_NO_REM  0   // ���粻����
#define FUNCCODE_plcPowerOffRemMode_REM     1   // �������
// PLCͣ������ѡ��
#define FUNCCODE_plcStopRemMode_NO_REM  0   // ���粻����
#define FUNCCODE_plcStopRemMode_REM     1   // �������

// PLC����ʱ�䵥λ
#define FUNCCODE_plcTimeUnit_S      0   // S(��)
#define FUNCCODE_plcTimeUnit_H      1   // H(Сʱ)

// ����¶ȴ���������
#define FUNCCODE_tempSenorType_NONE         0       // PTC100
#define FUNCCODE_tempSenorType_PTC100       1       // PTC100
#define FUNCCODE_tempSenorType_PTC1000      2       // PTC1000
#define FUNCCODE_tempSenorType_NTC          3       // NTC

// ���ݸ�ʽ
#define FUNCCODE_sciParity_NONE     0   // ��У��(8-N-2)
#define FUNCCODE_sciParity_EVEN     1   // żУ��(8-E-1)
#define FUNCCODE_sciParity_ODD      2   // ��У��(8-O-1)
#define FUNCCODE_sciParity_NONE1    3   // ��У��(8-N-1)



// �������ʱ�䵥λ
// ע�⣬������Ϊ�˼�С����Ҫ�ļ���Ϳռ�ռ�ã�����ʹ����
// X * (TIME_UNIT_WAIT_STOP_BRAKE / RUN_CTRL_PERIOD) �ķ�ʽ
// �����ǣ�(X * TIME_UNIT_WAIT_STOP_BRAKE) / RUN_CTRL_PERIOD
// ֮���޸���Щʱ�䵥λ�������б�Ҫ�޸ġ�
#define TIME_UNIT_SEC_PER_HOUR          3600    // 1hour = 3600sec
#define TIME_UNIT_MIN_PER_HOUR          60      // 1hour = 60min
#define TIME_UNIT_SEC_PER_MIN           60      // 1min  = 60sec
#define TIME_UNIT_MS_PER_SEC            1000    // 1s = 1000ms

#define TIME_UNIT_VF_VOL_ACC_TIME       100     // VF����ĵ�ѹ����ʱ��

#define TIME_UNIT_AI_PULSE_IN_FILTER    10      // AI,pulseIn�˲�ʱ��, ms
#define TIME_UNIT_DI_DELAY              100     // DI����ӳ�ʱ��, ms
#define TIME_UNIT_DO_DELAY              100     // DO����ӳ�ʱ��, ms
#define TIME_UNIT_START_FRQ_WAIT        100      // ����Ƶ�ʱ���ʱ�䣬ms
#define TIME_UNIT_START_BRAKE           100     // ����ֱ���ƶ�ʱ�䣬ms
#define TIME_UNIT_WAIT_STOP_BRAKE       100     // ͣ��ֱ���ƶ��ȴ�ʱ�䣬ms
#define TIME_UNIT_STOP_BRAKE            100     // ͣ��ֱ���ƶ�ʱ�䣬ms
#define TIME_UNIT_ZERO_SPEED_DEAD       100     // ����ת����ʱ��
#define TIME_UNIT_LOWER_STOP_DELAY      100     // Ƶ�ʵ�������Ƶ��ʱͣ�����ӳ�ʱ��
#define TIME_UNIT_PID_SET_CHANGE        10      // PID�����仯ʱ��
#define TIME_UNIT_PID_FILTER            10      // PID����������˲�ʱ��
#define TIME_UNIT_PID_INIT              10      // PID��ֵ����ʱ��
#define TIME_UNIT_PID_FDB_LOSE          100     // PID������ʧ���ʱ��
#define TIME_UNIT_SWING_PERIOD          100      // ��Ƶ����
#define TIME_UNIT_sciCommOverTime       100     // ͨѶ��ʱʱ��
#define TIME_UNIT_ERR_AUTO_RST_DELAY    100     // �����Զ���λ���ʱ�䣬ms
#define TIME_UNIT_ERR_AUTO_RST_CLR      (TIME_UNIT_SEC_PER_HOUR*100UL) // �����Զ���λ�������ʱ��, 0.1h
#define TIME_UNIT_P_OFF_VOL_BACK        10      // ˲ͣ��ͣ��ѹ�����ж�ʱ��
#define TIME_UNIT_PLC                   100     // PLC����ʱ�䵥λ

#define TIME_UNIT_ACC_DEC_SPEED_SERVO   10      // �ŷ��Ӽ���ʱ�䵥λ
#define TIME_UNIT_WAKE_UP               100     // ����ʱ��ĵ�λ
#define TIME_UNIT_DORMANT               100     // ����ʱ��ĵ�λ
#define TIME_UNIT_CURRENT_CHK           10      // �������ʱ�䵥λ
#define TIME_UNIT_TORQUE_CTRL_ACC_DEC   10      // ת�ؿ���ʱ�䵥λ
//=====================================================================

#if 0//F_DEBUG_RAM
#define ACC_DEC_T_INIT1  ((Uint32)2*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // �Ӽ���ʱ�����ֵ��2s������ <= 20
#define ACC_DEC_T_INIT2  ((Uint32)5*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // 5s������ > 20
#else
#define ACC_DEC_T_INIT1  ((Uint32)10*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // �Ӽ���ʱ�����ֵ��20s������ <= 20
#define ACC_DEC_T_INIT2  ((Uint32)30*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // 50s������ > 20
#endif

#define RATING_FRQ_INIT_0   50      // ����Ƶ�ʣ�0��С����
#define RATING_FRQ_INIT_1   500     // ����Ƶ�ʣ�1��С����
#define RATING_FRQ_INIT_2   5000    // ����Ƶ�ʣ�2��С����
#define BAUD_NUM_MAX        12   // ������ѡ��Χ�����ֵ
//#define BAUD_NUM_MAX 10
#define PARA_INIT_MODE_MAX  501       // ������ʼ������ֵ
#define INV_TYPE_MAX   30

#define MENU_MODE_MAX       3   // �˵�ģʽ�����ֵ

//=====================================================================
// ��Щ�������������������ĳ�������룬��������funcCode�е�index

// ������ʹ�õ�һЩ�������index
//= �������/ɾ���˹����룬����Ҫ�޸�!
#define FACTORY_PWD_INDEX      (GetCodeIndex(funcCode.code.factoryPassword))   // FF-00 ��������
#define INV_TYPE_INDEX         (GetCodeIndex(funcCode.code.inverterType))      // FF-01 ��Ƶ������
#define RATING_POWER_INVERTER_INDEX  (GetCodeIndex(funcCode.code.inverterPower))     // FF-03 ��Ƶ������
#define FUNCCODE_FACTORY_START_INDEX     (GetCodeIndex(funcCode.group.ff[0]))            // FF��Ŀ�ʼ
#define FUNCCODE_FACTORY_END_INDEX       (GetCodeIndex(funcCode.group.ff[FFNUM - 1]))    // FF��Ľ���

#define FC_MOTOR1_START_INDEX   (GetCodeIndex(funcCode.code.motorParaM1.all[0]))      // ��1�����������ʼ
#define FC_MOTOR1_END_INDEX     (GetCodeIndex(funcCode.code.pgParaM1.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // ��1��������Ľ���

#define FC_MOTOR2_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM2.motorPara.all[0]))      // ��2�����������ʼ
#define FC_MOTOR2_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM2.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // ��2��������Ľ���

#define FC_MOTOR3_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM3.motorPara.all[0]))      // ��3�����������ʼ
#define FC_MOTOR3_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM3.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // ��3��������Ľ���

#define FC_MOTOR4_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM4.motorPara.all[0]))      // ��4�����������ʼ
#define FC_MOTOR4_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM4.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // ��4��������Ľ���



#define PRESET_FRQ_INDEX        (GetCodeIndex(funcCode.code.presetFrq))      // F0-08   Ԥ��Ƶ��
#define MAX_FRQ_INDEX           (GetCodeIndex(funcCode.code.maxFrq))         // F0-10   ���Ƶ��
#define UPPER_FRQ_INDEX         (GetCodeIndex(funcCode.code.upperFrq))       // F0-12   ����Ƶ��
#define LOWER_FRQ_INDEX         (GetCodeIndex(funcCode.code.lowerFrq))       // F0-14   ����Ƶ��
#define ACC_TIME1_INDEX         (GetCodeIndex(funcCode.code.accTime1))       // F0-17   ����ʱ��1
#define DEC_TIME1_INDEX         (GetCodeIndex(funcCode.code.decTime1))       // F0-18   ����ʱ��1
#define CARRIER_FRQ_INDEX       (GetCodeIndex(funcCode.code.carrierFrq))     // F0-15   �ز�Ƶ��

#define RATING_POWER_INDEX      (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower))    // ��������
#define RATING_VOL_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingVoltage))  // ������ѹ
#define RATING_CUR_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent))  // ��������
#define RATING_CUR_INDEX2       (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingCurrent))    // ��2��������
#define RATING_CUR_INDEX3       (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingCurrent))   // ��3��������
#define RATING_CUR_INDEX4       (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingCurrent))   // ��4��������


#define RATING_FRQ_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingFrq))      // ����Ƶ��

#define ZERO_LOAD_CURRENT_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent))// ���ص���
#define STATOR_RESISTANCE_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance))// ���ӵ���

#define ANTI_VIBRATE_GAIN_INDEX (GetCodeIndex(funcCode.code.antiVibrateGain))// F3-11   ����������
#define ANTI_VIBRATE_GAIN_MOTOR2_INDEX (GetCodeIndex(funcCode.code.motorFcM2.antiVibrateGain))// A4-52   ����������
#define ANTI_VIBRATE_GAIN_MOTOR3_INDEX (GetCodeIndex(funcCode.code.motorFcM3.antiVibrateGain))// A5-52   ����������
#define ANTI_VIBRATE_GAIN_MOTOR4_INDEX (GetCodeIndex(funcCode.code.motorFcM4.antiVibrateGain))// A6-52   ����������

#define TUNE_CMD_INDEX_1  (GetCodeIndex(funcCode.code.tuneCmd))  // ��г
#define TUNE_CMD_INDEX_2  (GetCodeIndex(funcCode.code.motorFcM2.tuneCmd))  // ��г
#define TUNE_CMD_INDEX_3  (GetCodeIndex(funcCode.code.motorFcM3.tuneCmd))  // ��г
#define TUNE_CMD_INDEX_4  (GetCodeIndex(funcCode.code.motorFcM4.tuneCmd))  // ��г

#define VC_CHG_FRQ1_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq1))  // ʸ�������ٶȻ� �л�Ƶ��1
#define VC_CHG_FRQ2_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq2))  // ʸ�������ٶȻ� �л�Ƶ��2

#define VC_CHG_FRQ1_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq1))  // ʸ�������ٶȻ� �л�Ƶ��1
#define VC_CHG_FRQ2_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq2))  // ʸ�������ٶȻ� �л�Ƶ��2

#define VC_CHG_FRQ1_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq1))  // ʸ�������ٶȻ� �л�Ƶ��1
#define VC_CHG_FRQ2_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq2))  // ʸ�������ٶȻ� �л�Ƶ��2

#define VC_CHG_FRQ1_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq1))  // ʸ�������ٶȻ� �л�Ƶ��1
#define VC_CHG_FRQ2_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq2))  // ʸ�������ٶȻ� �л�Ƶ��2


#define TORQUE_BOOST_INDEX      (GetCodeIndex(funcCode.code.torqueBoost))    // F1-05   ת������

#define TORQUE_BOOST_MOTOR2_INDEX      (GetCodeIndex(funcCode.code.motorFcM2.torqueBoost))    // D0-52   ��2���ת������
#define TORQUE_BOOST_MOTOR3_INDEX      (GetCodeIndex(funcCode.code.motorFcM3.torqueBoost))    // D0-52   ��3���ת������
#define TORQUE_BOOST_MOTOR4_INDEX      (GetCodeIndex(funcCode.code.motorFcM4.torqueBoost))    // D0-52   ��4���ת������

#define SVC_MODE_INDX           (GetCodeIndex(funcCode.code.svcMode))        // A5-07 SVCģʽѡ��
#define OV_POINT_SET_INDEX      (GetCodeIndex(funcCode.code.ovPointSet))     // A5-09 ��ѹ������

#define VF_FRQ1_INDEX           (GetCodeIndex(funcCode.code.vfFrq1))         // F3-03   ���VFƵ�ʵ�1
#define VF_FRQ2_INDEX           (GetCodeIndex(funcCode.code.vfFrq2))         // F3-05   ���VFƵ�ʵ�2
#define VF_FRQ3_INDEX           (GetCodeIndex(funcCode.code.vfFrq3))         // F3-07   ���VFƵ�ʵ�3


#define CURVE1_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[0]))       // F2-08   AI1��С����
#define CURVE1_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[2]))       // F2-10   AI1�������
#define CURVE2_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[0]))       // F2-14   AI2��С����
#define CURVE2_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[2]))       // F2-16   AI2�������
#define CURVE3_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[0]))       // F2-14   AI3��С����
#define CURVE3_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[2]))       // F2-16   AI3�������
#define PULSE_IN_MIN_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[0]))     // F2-20   PULSE��С����
#define PULSE_IN_MAX_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[2]))     // F2-22   PULSE�������

#define CURVE4_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[0]))       // A8-00   AI4��С����
#define CURVE4_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[2]))       // A8-02   AI4�յ�1����
#define CURVE4_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[4]))       // A8-04   AI4�յ�2����
#define CURVE4_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[6]))       // A8-06   AI4�������
#define CURVE5_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[0]))       // A8-08   AI5��С����
#define CURVE5_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[2]))       // A8-10   AI5�յ�1����
#define CURVE5_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[4]))       // A8-12   AI5�յ�2����
#define CURVE5_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[6]))       // A8-14   AI5�������


#define ACC_TIME2_INDEX         (GetCodeIndex(funcCode.code.accTime2))       // F8-03 ����ʱ��2
#define DEC_TIME2_INDEX         (GetCodeIndex(funcCode.code.decTime2))       // F8-04 ����ʱ��2

#define ACC_TIME3_INDEX         (GetCodeIndex(funcCode.code.accTime3))       // F8-05 ����ʱ��3
#define DEC_TIME3_INDEX         (GetCodeIndex(funcCode.code.decTime3))       // F8-06 ����ʱ��3

#define ACC_TIME4_INDEX         (GetCodeIndex(funcCode.code.accTime4))       // F8-07 ����ʱ��4
#define DEC_TIME4_INDEX         (GetCodeIndex(funcCode.code.decTime4))       // F8-08 ����ʱ��4

#define RUN_TIME_ADDUP_INDEX    (GetCodeIndex(funcCode.code.runTimeAddup))     // F7-09  �ۼ�����ʱ��
#define POWER_TIME_ADDUP_INDEX  (GetCodeIndex(funcCode.code.powerUpTimeAddup)) // F7-13  �ۼ��ϵ�ʱ��
#define POWER_ADDUP_INDEX       (GetCodeIndex(funcCode.code.powerAddup))       // F7-14  �ۼƺĵ���


#define AI1_LIMIT               (GetCodeIndex(funcCode.code.ai1VoltageLimit)) //  F8-45  AI��������
#define AI1_UPPER               (GetCodeIndex(funcCode.code.ai1VoltageUpper)) //  F8-46  AI��������

#define PID_PARA_CHG_DELTA1_MAX (GetCodeIndex(funcCode.code.pidParaChgDelta2))  // FA-20  PID�����л�ƫ��2
#define PID_PARA_CHG_DELTA2_MIN (GetCodeIndex(funcCode.code.pidParaChgDelta1))  // FA-19  PID�����л�ƫ��1

#define DORMANT_UPPER           (GetCodeIndex(funcCode.code.wakeUpFrq))       // ����Ƶ������
#define WAKE_UP_LIMIT           (GetCodeIndex(funcCode.code.dormantFrq))      // ����Ƶ������
#define RADIATOR_TEMP_INDEX     (GetCodeIndex(funcCode.code.radiatorTemp))   // FB-19   �����ģ��ɢ�����¶�
#define ERROR_LATEST1_INDEX     (GetCodeIndex(funcCode.code.errorLatest1))   // FB-20   ��һ�ι�������
#define ERROR_LATEST2_INDEX     (GetCodeIndex(funcCode.code.errorLatest2))   // FB-21   �ڶ��ι�������
#define ERROR_LATEST3_INDEX     (GetCodeIndex(funcCode.code.errorLatest3))   // FB-22   (���һ��)�����ι�������
#define ERROR_FRQ_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq))       // FB-23   ����ʱƵ��
#define ERROR_CURRENT_INDEX     (GetCodeIndex(funcCode.code.errorScene3.elem.errorCurrent))   // FB-24   ����ʱ����
#define ERROR_UDC_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorGeneratrixVoltage)) // FB-25 ����ʱĸ�ߵ�ѹ
#define ERROR_DI_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDiStatus))  // FB-26   ����ʱ�������״̬
#define ERROR_DO_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDoStatus))  // FB-27   ����ʱ�������״̬
#define LAST_ERROR_RECORD_INDEX (GetCodeIndex(funcCode.code.errorScene1.all[sizeof(struct ERROR_SCENE_STRUCT) - 1]))  // ���һ�����ϼ�¼

#define MIN_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMinTime))         // A0-14   ������ʱ������
#define MAX_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMaxTime))         // A0-15   ������ʱ������



#define PC_LOOP_CHG_FRQ1_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq1))     //          �л�Ƶ��1
#define PC_LOOP_CHG_FRQ2_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq2))     //          �л�Ƶ��2

#define EEPROM_CHECK_INDEX      (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1

#define RUN_TIME_ADDUP_SEC_INDEX    (GetCodeIndex(funcCode.code.runTimeAddupSec))   // FR-07 F209  �ۼ�����ʱ���s


#define EEPROM_CHECK_INDEX1     (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1
#define EEPROM_CHECK_INDEX2     (GetCodeIndex(funcCode.code.eepromCheckWord2))  // eepromCheckWord2

#define SAVE_USER_PARA_PARA1    (GetCodeIndex(funcCode.code.saveUserParaFlag1))
#define SAVE_USER_PARA_PARA2    (GetCodeIndex(funcCode.code.saveUserParaFlag2))

#define AI_AO_CHK_FLAG          (GetCodeIndex(funcCode.code.aiaoChkWord))       // AIAOУ����־
#define AI_AO_CALIB_START       (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))  // aiao����У����ʼ
#define AI_AO_CALIB_STOP        (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2))   // aiao����У������

#define AI1_CALB_START          (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))
#define AI2_CALB_STOP           (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[1].after2))
#define AO1_CALB_START          (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].before1))
#define AO1_CALB_STOP           (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2)) 

//-------------------------------
#define FC_GROUP_FACTORY    FUNCCODE_GROUP_FF   // ���Ҳ�����
#define FC_GROUP_FC_MANAGE  FUNCCODE_GROUP_FP   // ���������
#define FC_START_GROUP      FUNCCODE_GROUP_F0   // ����������ʾ�ĵ�1��
//--------------------------------


extern FUNCCODE_ALL funcCode;           // �������RAMֵ
//extern FUNCCODE_ALL funcCodeEeprom;     // �������EEPROMֵ

extern const Uint16 funcCodeGradeSum[];

extern Uint16 saveEepromIndex;    // DP������洢
extern const Uint16 ovVoltageInitValue[];
extern const Uint16 funcCodeGradeAll[];
extern Uint16 funcCodeGradeCurMenuMode[];

#endif  // __F_FUNCCODE_H__





