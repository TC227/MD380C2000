//======================================================================
//
// ��Ƶ���������Լ���ز���
// 
// ������
// ��Ƶ�����͡���Ƶ�����ʡ���Ƶ�����ѹ
// �ز�Ƶ�ʡ�
// �������ʡ��������������ӵ��衢ת�ӵ��衢©�п������п������ص���
// 
// ����������
//
// Time-stamp: <2012-08-14>
//
//======================================================================


#ifndef __F_INV_PARA_H__
#define __F_INV_PARA_H__



#if F_DEBUG_RAM
#define DEBUG_F_MOTOR_POWER_RELATE      0   // ������ʴ���
#define DEBUG_F_INV_TYPE_RELATE         0   // ��Ƶ�����ʹ���
#elif 1
#define DEBUG_F_MOTOR_POWER_RELATE      1
#define DEBUG_F_INV_TYPE_RELATE         1
#endif


#define INV_TYPE_VOLTAGE_NUM    6       // _�ֵ�ѹ�ȼ�

#define INV_TYPE_VOLTAGE_T380   0   // ����380V
#define INV_TYPE_VOLTAGE_T220   1   // ����220V
#define INV_TYPE_VOLTAGE_S220   2   // ����220V
#define INV_TYPE_VOLTAGE_T480   3   // ����480V
#define INV_TYPE_VOLTAGE_T690   4   // ����690V
#define INV_TYPE_VOLTAGE_T1140  5   // ����1140V

// ��Ƶ�����ͣ�1-7�� 2��
// ��Ƶ�����ͣ�>= 8��3��
// ���ܴ��ݵ�ĸ�ߵ�ѹ����һλС����
#define INV_TYPE_THREE_PHASE_START          8       // 3�����ʼ����
#define INV_TYPE_BIG_ACC_DEC_START_T380     21      // ����380V�Ӽ���ʱ�����ֵΪ��ֵ����ʼ����
#define INV_TYPE_BIG_ACC_DEC_START_T220     18      // ����220V�Ӽ���ʱ�����ֵΪ��ֵ����ʼ����
#define INV_TYPE_POINT_LIMIT_T380           21      // ����380V���������������С����
#define INV_TYPE_POINT_LIMIT_T220           18      // ����220V���������������С����
#define MOTOR_PARA_POINT_POWER_LIMIT        750     // ���������������С����Ĺ��ʷֽ���, 75.0kW��>=��ֵ��
#define RATING_SPEED_RPM                    1460    // �ת��

#define GetInvParaPointer(invType)            \
(pInvTypeParaTable[invPara.volLevel] + ((invType) - invTypeLimitTable[invPara.volLevel].lower)) \



#define INV_TYPE_LOWER_LIMIT_T380  8    // T380�Ļ�������
#define INV_TYPE_UPPER_LIMIT_T380  37   // T380�Ļ�������  ShenYang 630kw
#define INV_TYPE_LENGTH_T380                                    \
(INV_TYPE_UPPER_LIMIT_T380 - INV_TYPE_LOWER_LIMIT_T380 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T220  7    // T220�Ļ�������
#define INV_TYPE_UPPER_LIMIT_T220  22   // T220�Ļ�������
#define INV_TYPE_LENGTH_T220                                    \
(INV_TYPE_UPPER_LIMIT_T220 - INV_TYPE_LOWER_LIMIT_T220 + 1)     \

#define INV_TYPE_LOWER_LIMIT_S220  1    //  S220�Ļ�������
#define INV_TYPE_UPPER_LIMIT_S220  7    //  S220�Ļ�������
#define INV_TYPE_LENGTH_S220                                    \
(INV_TYPE_UPPER_LIMIT_S220 - INV_TYPE_LOWER_LIMIT_S220 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T480  8    // T480�Ļ�������
#define INV_TYPE_UPPER_LIMIT_T480  34    // T480�Ļ�������
#define INV_TYPE_LENGTH_T480                                    \
(INV_TYPE_UPPER_LIMIT_T480 - INV_TYPE_LOWER_LIMIT_T480 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T690  21    // T690�Ļ�������
#define INV_TYPE_UPPER_LIMIT_T690  36   // T690�Ļ�������
#define INV_TYPE_LENGTH_T690                                    \
(INV_TYPE_UPPER_LIMIT_T690 - INV_TYPE_LOWER_LIMIT_T690 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T1140  19    // T690�Ļ�������
#define INV_TYPE_UPPER_LIMIT_T1140  37   // T690�Ļ�������
#define INV_TYPE_LENGTH_T1140                                   \
(INV_TYPE_UPPER_LIMIT_T1140 - INV_TYPE_LOWER_LIMIT_T1140 + 1)   \


// ������ز���
// ���������˳���빦����Ҫһ��
struct INV_TYPE_RELATED_PARA_STRUCT
{
    Uint16 carrierFrq;              // �ز�Ƶ��

    Uint16 ratingPower;             // ��������
    Uint16 ratingCurrent;           // ��������
    Uint16 statorResistance;        // ���ӵ���
    Uint16 rotorResistance;         // ת�ӵ���
    Uint16 leakInductance;          // ©�п�
    Uint16 mutualInductance;        // ���п�
    Uint16 zeroLoadCurrent;         // ���ص���

    Uint16 antiVibrateGain;         // ����������
};
typedef union
{
    Uint16 all[sizeof(struct INV_TYPE_RELATED_PARA_STRUCT)];

    struct INV_TYPE_RELATED_PARA_STRUCT elem;
} INV_TYPE_RELATED_PARA;



struct INV_TYPE_RELATED_PARA_ALL
{
    INV_TYPE_RELATED_PARA t380[INV_TYPE_LENGTH_T380];   // T380
    INV_TYPE_RELATED_PARA t220[INV_TYPE_LENGTH_T220];   // T220
    INV_TYPE_RELATED_PARA s220[INV_TYPE_LENGTH_S220];   // S220
    INV_TYPE_RELATED_PARA t480[INV_TYPE_LENGTH_T480];   // T480
    INV_TYPE_RELATED_PARA t690[INV_TYPE_LENGTH_T690];   // T690
    INV_TYPE_RELATED_PARA t1140[INV_TYPE_LENGTH_T1140]; // T1140
};


struct INV_TYPE_LIMIT
{
    Uint16 vol;     // ����ѹ�ȼ��ĵ�ѹ

    Uint16 lower;   // ����ѹ�ȼ��Ļ�������
    Uint16 upper;   // ����ѹ�ȼ��Ļ�������
};
extern const struct INV_TYPE_LIMIT invTypeLimitTable[];

extern const INV_TYPE_RELATED_PARA * pInvTypeParaTable[];


LOCALD void InverterTypeRelatedNoMotorParaDeal(Uint16 invType);
Uint16 ValidateInvType(void);
void MotorPowerRelatedParaDeal(Uint16 power, Uint16 motorSn4Power);
void InverterTypeRelatedParaDeal(void);     // �޸ı�Ƶ������ʱ�Ĵ���
void InverterTypeRelatedNoMotorParaSaveDeal(void);




#endif  // __F_INV_PARA_H__


