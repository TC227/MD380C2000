#ifndef __F_DEBUG_H__
#define __F_DEBUG_H__



//=====================================================================
#if defined(DSP2803X)
#define DSP_2803X  1      // 28035
#elif 1
#define DSP_2803X  0
#endif
//=====================================================================



//=====================================================================
#if defined(FUNC_DEBUG_RAM)         // �����Թ��ܣ���CCS��build option�ж���ĺ�
#define F_DEBUG_RAM             1   // ������RAM�е���
#elif 1
#define F_DEBUG_RAM             0   // ����RAM�е���
#endif
//=====================================================================



//=====================================================================
// ר����MD380M/IS380
#if !defined(ZJ_MD380M)
#define DEBUG_F_POSITION_CTRL   0   // ������λ�ÿ���
#elif 1
#define DEBUG_F_POSITION_CTRL   1   // ����λ�ÿ���
#endif
//=====================================================================




//=====================================================================
// ע��EEPROM�Ĵ�С
//#define DEBUG_F_MOTOR_FUNCCODE    0   // ��Ԥ������������ܵ���
#define DEBUG_F_MOTOR_FUNCCODE    1   // Ԥ������������ܵ���
//=====================================================================

#define DEBUG_F_PLC_CTRL          1
#define DEBUG_F_P2P_CTRL          1

//#if F_DEBUG_RAM && (!defined(_lint))
#if 1
#define LOCALF
#define LOCALD extern
#else
#define LOCALF static
#define LOCALD static
#endif
// LOCALD stands for local Declaration.
// LOCALF stands for local deFinition.


//=====================================================================
// DSP Ƶ��: 60MHz/100MHz. ������20MHz
#if (DSP_CLOCK == 100)
#define TIME_10US       1000UL
#define TIME_50US       5000UL
#define TIME_100US      10000UL // 0.1ms��Ӧ�Ķ�ʱ������ֵ����ʱ��1 10ns��λ��
#define TIME_1MS        100000UL
#elif (DSP_CLOCK == 60)
#define TIME_10US       600UL
#define TIME_50US       3000UL
#define TIME_100US      6000UL  // 0.1ms��Ӧ�Ķ�ʱ������ֵ����ʱ��1 100/6ns��λ��
#define TIME_1MS        60000UL
#endif
//=====================================================================

// dest��bit1 = source��bit2
#define BitAssign(source, bit1, dest, bit2)     \
{                                               \
    if ((source) & (0x1UL << (bit1)))           \
        (dest) |= (0x1UL << (bit2));            \
    else                                        \
        (dest) &= (~(0x1UL << (bit2)));         \
}


//void BitAssign(Uint32 source, Uint32 *dest, int16 bit);

Uint16 qsqrt(Uint32 data);


void UniversalDebug(Uint16 sn);  // ͨ�ò��Ժ���


#if F_DEBUG_RAM
void UniversalDebug32(Uint32 a);
void PowerOnDebug(Uint16 value);

extern Uint16 daValue[];
void InitDa(void);
void UpdateDaBuffer(Uint16 port);
void DaOut(Uint16 port);
#endif


#endif  // #ifndef __F_DEBUG_H__



