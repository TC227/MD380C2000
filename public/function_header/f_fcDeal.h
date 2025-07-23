//======================================================================
//
// �������һЩ����
//
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#ifndef __F_FC_DEAL_H__
#define __F_FC_DEAL_H__


#if F_DEBUG_RAM

#define DEBUG_F_RESTORE_COMPANY_PARA_DEAL   0   // �ָ���������
#define DEBUG_F_CLEAR_RECORD                0   // ClearRecordDeal
#define DEBUG_F_LIMIT_OTHER_CODE            0   // LimitOtherCodeDeal

#elif 1

#define DEBUG_F_RESTORE_COMPANY_PARA_DEAL   1
#define DEBUG_F_CLEAR_RECORD                1
#define DEBUG_F_LIMIT_OTHER_CODE            1

#endif


// �ָ����������������¼
// ����Ҫ��һЩindex�궨��


//====================================================
//
// �ָ�����������ĳЩ��������Ҫ�ָ�
//
//====================================================

// ĳЩ�����Ĺ����벻��Ҫ�ָ�
#define INIT_EXCEPT_SERIES_S_0  GetCodeIndex(funcCode.group.ff[0])          // FF ���Ҳ���
#define INIT_EXCEPT_SERIES_E_0  GetCodeIndex(funcCode.group.ff[FFNUM-1])

#define INIT_EXCEPT_SERIES_S_1  GetCodeIndex(funcCode.group.fp[0])          // FP ���������
#define INIT_EXCEPT_SERIES_E_1  GetCodeIndex(funcCode.group.fp[1])          //GetCodeIndex(funcCode.group.fp[FPNUM-1])

#define INIT_EXCEPT_SERIES_S_2  FC_MOTOR1_START_INDEX           // ��1�������
#define INIT_EXCEPT_SERIES_E_2  FC_MOTOR1_END_INDEX

#define INIT_EXCEPT_SERIES_S_3  FC_MOTOR2_START_INDEX           // ��2�������
#define INIT_EXCEPT_SERIES_E_3  FC_MOTOR2_END_INDEX

#define INIT_EXCEPT_SERIES_S_6  FC_MOTOR3_START_INDEX           // ��3�������
#define INIT_EXCEPT_SERIES_E_6  FC_MOTOR3_END_INDEX

#define INIT_EXCEPT_SERIES_S_7  FC_MOTOR4_START_INDEX           // ��4�������
#define INIT_EXCEPT_SERIES_E_7  FC_MOTOR4_END_INDEX

#define INIT_EXCEPT_SERIES_S_4  GetCodeIndex(funcCode.group.ae[0])          // AE AIAO����У��
#define INIT_EXCEPT_SERIES_E_4  GetCodeIndex(funcCode.group.ae[AENUM-1])

#define INIT_EXCEPT_SERIES_S_5  GetCodeIndex(funcCode.code.errorLatest1)    // ��һ�ι�������
#define INIT_EXCEPT_SERIES_E_5  LAST_ERROR_RECORD_INDEX                     // ���һ�����ϼ�¼


// ĳЩ�����Ĺ����벻��Ҫ�ָ�
#define INIT_EXCEPT_SINGLE_0    GetCodeIndex(funcCode.code.runTimeAddup)        // �ۼ�����ʱ��
#define INIT_EXCEPT_SINGLE_1    GetCodeIndex(funcCode.code.runTimeAddupSec)     // �ۼ�����ʱ��
#define INIT_EXCEPT_SINGLE_2    GetCodeIndex(funcCode.code.softVersion)         // ����汾��
#define INIT_EXCEPT_SINGLE_3    GetCodeIndex(funcCode.code.radiatorTemp)        // �����ģ��ɢ�����¶�
#define INIT_EXCEPT_SINGLE_4    GetCodeIndex(funcCode.code.temp2)               // ������ɢ�����¶�
#define INIT_EXCEPT_SINGLE_5    GetCodeIndex(funcCode.code.frqPoint)            // Ƶ��ָ��С����

#define INIT_EXCEPT_SINGLE_6    GetCodeIndex(funcCode.code.powerUpTimeAddup)    // �ۼ�����ʱ��
#define INIT_EXCEPT_SINGLE_7    GetCodeIndex(funcCode.code.powerUpTimeAddupSec) // �ۼ�����ʱ��
#define INIT_EXCEPT_SINGLE_8    GetCodeIndex(funcCode.code.powerAddup)          // �ۼƺĵ���
#define INIT_EXCEPT_SINGLE_9    GetCodeIndex(funcCode.code.errorFrqUnit)        // ����ʱƵ��С����

#define INIT_EXCEPT_SINGLE_10   GetCodeIndex(funcCode.code.productVersion)      // ��Ʒ��
#define INIT_EXCEPT_SINGLE_11   GetCodeIndex(funcCode.code.softVersion)         // ����汾��


//====================================================
//
// �����¼
//
//====================================================

// ĳЩ�����Ĺ����������¼
#define CLEAR_RECORD_SERIES_S_0  GetCodeIndex(funcCode.code.errorLatest1)        // ��һ�ι�������
#define CLEAR_RECORD_SERIES_E_0  LAST_ERROR_RECORD_INDEX                         // ���һ�����ϼ�¼

// ĳЩ�����Ĺ����������¼
#define CLEAR_RECORD_SINGLE_0    GetCodeIndex(funcCode.code.runTimeAddup)        // �ۼ�����ʱ��
#define CLEAR_RECORD_SINGLE_1    GetCodeIndex(funcCode.code.runTimeAddupSec)     // �ۼ�����ʱ���s
#define CLEAR_RECORD_SINGLE_2    GetCodeIndex(funcCode.code.powerUpTimeAddup)    // �ۼ��ϵ�ʱ��
#define CLEAR_RECORD_SINGLE_3    GetCodeIndex(funcCode.code.powerUpTimeAddupSec) // �ۼ��ϵ�ʱ���s
#define CLEAR_RECORD_SINGLE_4    GetCodeIndex(funcCode.code.powerAddup)          // �ۼƺĵ���
#define CLEAR_RECORD_SINGLE_5    GetCodeIndex(funcCode.code.errorFrqUnit)        // ����ʱƵ��С����

#endif  // __F_FC_DEAL_H__










