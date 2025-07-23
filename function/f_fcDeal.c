//======================================================================
//
// �������һЩ����
//
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_menu.h"
#include "f_main.h"
#include "f_eeprom.h"
#include "f_invPara.h"
#include "f_fcDeal.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"

#if F_DEBUG_RAM
#define DEBUG_F_P_OFF_REM               0   // �������
#elif 1
#define DEBUG_F_P_OFF_REM               1   // �������
#endif


#if DEBUG_F_POSITION_CTRL
#pragma DATA_SECTION(limitedByOtherCodeIndex, "data_ram");
#endif
// �ù������������������������
#define LIMITED_BY_OTHER_CODE_NUM_MAX   150  
// ��ǰʹ����21�����������50��Ӧ���ܱ�֤�ܳ�һ��ʱ���޸Ĺ������ڵ�����ʹ�ö����������2007.10.29
// 33��
Uint16 limitedByOtherCodeIndex[LIMITED_BY_OTHER_CODE_NUM_MAX];
Uint16 limitedByOtherCodeIndexNum;  // ���������������������ƵĹ���������
#if DEBUG_F_LIMIT_OTHER_CODE
// �ù������������������������
#define LIMIT_OTHER_CODE_NUM  13    // λ����funcCode��λ���ǴӺ�ǰ������Ϊʹ��limitOtherCodeIndex[]ʱ�ǴӺ�ǰ��
LOCALF Uint16 const limitOtherCodeIndex[LIMIT_OTHER_CODE_NUM] =
{
    PULSE_IN_MAX_INDEX,         // F2-22  0  PULSE�������
    PULSE_IN_MIN_INDEX,         // F2-20  1  PULSE��С����
    CURVE2_MAX_INDEX,           // F2-16  2  AI2�������
    CURVE2_MIN_INDEX,           // F2-14  3  AI2��С����
    CURVE1_MAX_INDEX,           // F2-10  4  AI1�������
    CURVE1_MIN_INDEX,           // F2-08  5  AI1��С����
    VF_FRQ3_INDEX,              // F1-11  6  ���VFƵ�ʵ�3
    VF_FRQ2_INDEX,              // F1-09  7  ���VFƵ�ʵ�2
    VF_FRQ1_INDEX,              // F1-07  8  ���VFƵ�ʵ�1
    RATING_FRQ_INDEX,           // F1-03  9  ����Ƶ��
    LOWER_FRQ_INDEX,            // F0-07  10 ����Ƶ��
    UPPER_FRQ_INDEX,            // F0-06  11 ����Ƶ��
    MAX_FRQ_INDEX,              // F0-04  12 ���Ƶ��
};
#endif


//=====================================================================
// 
// 
// �������صĹ����룬�ǵ������
// ���ɸ��Ĳ������Ѿ�����
// 
//=====================================================================
Uint16 GetFuncCodeInit(Uint16 index, Uint16 type)
{
//#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL || DEBUG_F_CHECK_MENU_MODE
#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    Uint16 init;

// �ָ�����������ĳЩ��������Ҫ�ָ�
// ĳЩ�����Ĺ����벻��Ҫ�ָ�
    if (((INIT_EXCEPT_SERIES_S_0 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_0))
      || ((INIT_EXCEPT_SERIES_S_1 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_1))         
      || ((INIT_EXCEPT_SERIES_S_4 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_4))
      || ((INIT_EXCEPT_SERIES_S_5 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_5))      
      || (( ((INIT_EXCEPT_SERIES_S_2 <= index)      // ��һ�������
            && (index <= INIT_EXCEPT_SERIES_E_2))           
         || ((INIT_EXCEPT_SERIES_S_3 <= index)      // �ڶ��������
            && (index <= INIT_EXCEPT_SERIES_E_3))               
         || ((INIT_EXCEPT_SERIES_S_6 <= index)      // �����������
            && (index <= INIT_EXCEPT_SERIES_E_6))               
         || ((INIT_EXCEPT_SERIES_S_7 <= index)      // ���ĵ������
            && (index <= INIT_EXCEPT_SERIES_E_7))
        ) 
        && (!type)) // �ָ��������
        )
    {
        init = funcCode.all[index];
    }
// ĳЩ�����Ĺ����벻��Ҫ�ָ�
    else if ((INIT_EXCEPT_SINGLE_0 == index) 
            || (INIT_EXCEPT_SINGLE_1 == index)  
            || (INIT_EXCEPT_SINGLE_2 == index)
            || (INIT_EXCEPT_SINGLE_3 == index)
            || (INIT_EXCEPT_SINGLE_4 == index)
            || ((INIT_EXCEPT_SINGLE_5 == index) 
            	&& (!type)   // �ָ��������ʱ�ָ�Ƶ��С����
			   )
            || (INIT_EXCEPT_SINGLE_6 == index)
            || (INIT_EXCEPT_SINGLE_7 == index)
            || (INIT_EXCEPT_SINGLE_8 == index)
            || (INIT_EXCEPT_SINGLE_9 == index)
            || (INIT_EXCEPT_SINGLE_10 == index)
            || (INIT_EXCEPT_SINGLE_11 == index)
        )
    {
        init = funcCode.all[index];
    }
// A7 AIAOУ������AE��ָ�
    else if (((GetCodeIndex(funcCode.code.aiCalibrateCurve[0].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aoCalibrateCurve[AO_NUMBER-1].after2))
            )
    {
        // AI3���ָ�
        if (((GetCodeIndex(funcCode.code.aiCalibrateCurve[2].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aiCalibrateCurve[2].after2))
            )
        {
            init = funcCode.all[index];
        }
        // AO2���ָ�
        else if (((GetCodeIndex(funcCode.code.aoCalibrateCurve[1].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aoCalibrateCurve[1].after2))
            )
        {
            init = funcCode.all[index];
        }
        else
        {
            init = funcCode.group.ae[index - GetCodeIndex(funcCode.code.aiCalibrateCurve[0].before1)];
        }
    }
// �Ӽ���ʱ��
    else if ((ACC_TIME1_INDEX == index) ||   
        (DEC_TIME1_INDEX == index) ||
        (ACC_TIME2_INDEX == index) ||
        (DEC_TIME2_INDEX == index) ||
        (ACC_TIME3_INDEX == index) ||
        (DEC_TIME3_INDEX == index) ||
        (ACC_TIME4_INDEX == index) ||
        (DEC_TIME4_INDEX == index)
        )
    {
        if (invPara.type < invPara.bitAccDecStart)  // ���� < 21
        {
            init = ACC_DEC_T_INIT1;
        }
        else                    // ���ʹ��ڵ���21���Ӽ���ʱ�����ֵΪ50s
        {
            init = ACC_DEC_T_INIT2;
        }
    }
#if (DEBUG_F_INV_TYPE_RELATE || DEBUG_F_MOTOR_POWER_RELATE)
// �ز�Ƶ��
    else if (CARRIER_FRQ_INDEX == index)    
    {
        init = GetInvParaPointer(invPara.type)->elem.carrierFrq;
    }
// ����������
    else if ((ANTI_VIBRATE_GAIN_INDEX == index)          
            || (ANTI_VIBRATE_GAIN_MOTOR2_INDEX == index)
            || (ANTI_VIBRATE_GAIN_MOTOR3_INDEX == index)
            || (ANTI_VIBRATE_GAIN_MOTOR4_INDEX == index)
            )
    {
        init = GetInvParaPointer(invPara.type)->elem.antiVibrateGain;
    }
// ת������
    else if (TORQUE_BOOST_INDEX == index)   
    {
        init = TorqueBoostDeal(funcCode.code.motorParaM1.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR2_INDEX == index)   // ��2���ת������
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM2.motorPara.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR3_INDEX == index)   // ��3���ת������
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM3.motorPara.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR4_INDEX == index)   // ��4���ת������
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM4.motorPara.elem.ratingPower);
    }
    else if (SVC_MODE_INDX == index)    // SVC�Ż���ʽ
    {
        // 1140V����SVCĬ�ϲ��Ż�
        if (invPara.ratingVoltage == 1140)
        {
            init = 0;
        }
        else
        {
            init = 1;
        }
    }
    // ��ѹ������
    else if (OV_POINT_SET_INDEX == index)
    {
        init = ovVoltageInitValue[invPara.volLevel];
    }
#endif
    else
    {
        init = GetFuncCodeInitOriginal(index);
    }
    
    return init;
#endif
}


//=====================================================================
//
// �����¼��Ϣ
// 
//=====================================================================
void ClearRecordDeal(void)
{
#if DEBUG_F_CLEAR_RECORD
    int16 i;

    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
    startIndexWriteSeries = CLEAR_RECORD_SERIES_S_0;    // ��һ�ι�������
    endIndexWriteSeries = CLEAR_RECORD_SERIES_E_0;      // ���һ�����ϼ�¼
    for (i = startIndexWriteSeries; i <= endIndexWriteSeries; i++)
    {
        funcCode.all[i] = 0;
    }

    // �ۼ�����ʱ��
    funcCode.all[CLEAR_RECORD_SINGLE_0] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_0);
    // �ۼ�����ʱ�����
    funcCode.all[CLEAR_RECORD_SINGLE_1] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_1);
    // �ۼ��ϵ�ʱ��
    funcCode.all[CLEAR_RECORD_SINGLE_2] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_2);
    // �ۼ��ϵ�ʱ�����
    funcCode.all[CLEAR_RECORD_SINGLE_3] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_3);
    // �ۼƺĵ���
    funcCode.all[CLEAR_RECORD_SINGLE_4] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_4);
    // ����Ƶ��С����
    funcCode.all[CLEAR_RECORD_SINGLE_5] = 0x222;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_5);
#endif
}


// �ָ���������֮��Ĵ���
void RestoreCompanyParaOtherDeal(void)
{
#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    
    funcCode.code.motorDebugFc = 0;       // �ָ����ܵ���
	funcCode.code.aiaoCalibrateDisp = 0;  // �ָ�AIAOУ����������ʾ
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorDebugFc));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.aiaoCalibrateDisp));
    MenuModeDeal();

    // ĳЩ�����������������������޵Ĵ���
    LimitOtherCodeDeal(RATING_FRQ_INDEX);   // ����Ƶ��

    upDownFrq = 0;          // up/down�޸�֮���ٻָ�������������ʾΪԤ��Ƶ�ʡ�
    frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    runCmd.all = 0;         // ���������������
    runDirPanelOld = 0;
#endif
}

//=====================================================================
//
// ������䴦������������Ҫ��������Ĺ�����
//
// ���ȼ�������ticker�������������ticker���ۼ�����ʱ���s��
// ͨѶ�޸�Ƶ��ֵ������/ͣ��ʱLED��ʾ������bitλ�á�
// ������ֱ��ʹ�ã�fr[]�Ѿ�������
//
//=====================================================================
void PowerOffRemDeal(void)
{
#if DEBUG_F_P_OFF_REM

#if DEBUG_F_MOTOR_FUNCCODE
    int16 i;
#endif

    // ʵ�ʳ���
    SaveOneFuncCode(GetCodeIndex(funcCode.code.lengthCurrent));
    
    // �ۼ�����ʱ��
    SaveOneFuncCode(RUN_TIME_ADDUP_INDEX);
    // �ۼ��ϵ�ʱ��
    SaveOneFuncCode(POWER_TIME_ADDUP_INDEX);
    // �ۼƺĵ���
    SaveOneFuncCode(POWER_ADDUP_INDEX);
    // DP��ͨѶ��Ҫ�洢EEPROM����
    if (saveEepromIndex)
    {
        SaveOneFuncCode(saveEepromIndex);
    }

    // ����ΪFR��
    // �����趨Ƶ��UP/DOWN�������
    funcCode.code.upDownFrqRem = upDownFrq;     // ���������������
    
    // PLC�������. �����䣬���ǽ��������趨Ϊ�������ʱ�������á�
    funcCode.code.plcStepRem = plcStep;
    if (plcStep >= PLC_STEP_MAX) // ���Ѿ���MAX���´��ϵ��step0��ʼ���С���ֹ�´��ϵ����Ӧ�������
    {
        funcCode.code.plcStepRem = 0;
    }
    funcCode.code.plcTimeHighRem = (plcTime & 0xFFFF0000UL) >> 16;
    funcCode.code.plcTimeLowRem = plcTime & 0x0000FFFFUL;

    funcCode.code.pmsmRotorPos = pmsmRotorPos;   // ͬ����ת��λ��
    //funcCode.code.extendType = ;

#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM
    for (i = REM_P_OFF_MOTOR - 1; i >= 0; i--)  // ������Ҫ�ĵ���������
    {
        funcCode.code.remPOffMotorCtrl[i] = gSendToFunctionDataBuff[i + MOTOR_TO_Func_2MS_DATA_NUM];
    }
#endif
#endif

    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
    startIndexWriteSeries = GetCodeIndex(funcCode.group.remember[0]);
    endIndexWriteSeries = GetCodeIndex(funcCode.group.remember[REM_NUM-1]);

#endif
}


//=====================================================================
//
// ĳЩ�����������������������޵Ĵ���
//
// 
// ˵����
// 1�����޸���ĳ���������ֵʱ��Ӧ�õ��ñ�������
// 
// 2���޸�����Щ�������������������޵Ĺ�����ʱ��Ҫͬʱ�޸ı����������޹������ֵ��
// 
// 3��Ŀǰ��޵������: �޸����Ƶ��maxFrq��
// ����Ƶ�ʣ� ��Χ�� 0.01Hz        - ���Ƶ��
// ���VFƵ�ʵ�1����Χ�� 0.00Hz        - ���VFƵ�ʵ�1
// ���VFƵ�ʵ�2����Χ�� ���VFƵ�ʵ�1 - ���VFƵ�ʵ�3
// ���VFƵ�ʵ�3����Χ�� ���VFƵ�ʵ�2 - ����Ƶ��
//
//=====================================================================
LOCALF void LimitOtherCodeDeal(Uint16 index)
{
#if DEBUG_F_LIMIT_OTHER_CODE
    int16 i, j, k;

    for (j = LIMIT_OTHER_CODE_NUM - 1; j >= 0; j--)
    {
        if (limitOtherCodeIndex[j] == index)
        {
            for (k = 2 - 1; k >= 0; k--)    // Ŀǰ�����ǰ��_�������ܱ�֤Ŀǰ���������ȷ��
            {   // limitedByOtherCodeIndex[]�����ݣ������±��С���������ֵʵ�����ǴӴ�С��
                i = (int16)limitedByOtherCodeIndexNum - 1;
                for (; i >= 0; i--)    // ����limitedByOtherCodeIndexNumΪ0�����
                {
                    const FUNCCODE_ATTRIBUTE *pAttribute = &funcCodeAttribute[limitedByOtherCodeIndex[i]];
                    Uint16 *pCode = &funcCode.all[limitedByOtherCodeIndex[i]];
                    Uint16 tmp = 0;
                    Uint16 upper;
                    Uint16 lower;
                    
                    if (pAttribute->attribute.bit.upperLimit)
                    {
                        int32 fc1, upper1;
                        
                        upper = funcCode.all[pAttribute->upper];

                        if (pAttribute->attribute.bit.signal)   // �з���
                        {
                            fc1 = (int32)(int16)*pCode;
                            upper1 = (int32)(int16)upper;
                        }
                        else
                        {
                            fc1 = *pCode;
                            upper1 = upper;
                        }
                        
                        //if (*pCode > upper) // ����û�п��Ǹù�����ķ��ţ��д���֧��ʤ��2009-07-22
                        if (fc1 > upper1)
                        {
                            *pCode = upper;       // ����RAM
                            SaveOneFuncCode(limitedByOtherCodeIndex[i]);    // ����EEPROM
                            
                            tmp = 1;
                        }
                    }
                    
                    if (pAttribute->attribute.bit.lowerLimit)
                    {
                        int32 fc1, lower1;
                        
                        lower = funcCode.all[pAttribute->lower];

                        if (pAttribute->attribute.bit.signal)   // �з���
                        {
                            fc1 = (int32)(int16)*pCode;
                            lower1 = (int32)(int16)lower;
                        }
                        else
                        {
                            fc1 = *pCode;
                            lower1 = lower;
                        }
                        
                        //if ((int16)*pCode < (int16)lower)   // ����û�п��Ǹù�����ķ��ţ��д���֧��ʤ��2009-07-26
                        if (fc1 < lower1)
                        {
                            if (!tmp)
                            {
                                *pCode = lower; // ...
                                SaveOneFuncCode(limitedByOtherCodeIndex[i]);// ����EEPROM
                            }
                            else
                            {
                                funcCode.all[pAttribute->lower] = *pCode;   // ����RAM
                                SaveOneFuncCode(pAttribute->lower);         // Ӧ��д��EEPROM����lower��������limitedByOtherCodeIndex[i]
                            }
                        }
                    }
                }
            }
            
            break;
        }
    }
#endif
}



// ֱ�Ӹ�������õ��ĳ���ֵ
Uint16 GetFuncCodeInitOriginal(Uint16 index)
{
    Uint16 init;

    // F0-FF,FP,A0-AF,B0-BF,C0-CF
    // ������ fChk��remember
    if (index < FNUM_PARA)                  // EEPROM CHK ֮ǰ
    {
        init = (funcCodeAttribute[index].init);
    }
#if 0     
    else if (EEPROM_CHECK_INDEX1 == index)  // EEPROM CHK1
    {
        init = EEPROM_CHECK1;
    }   
    else if (EEPROM_CHECK_INDEX2 == index)  // EEPROM CHK2
    {
        init = EEPROM_CHECK2;
    }
    else if (AI_AO_CHK_FLAG == index)  // AIAO CHK2
    {
        init = AIAO_CHK_WORD;
    }
#endif    
    else if (SAVE_USER_PARA_PARA1 == index)   // USER PARA SAVE CHK1
    {
        init = funcCode.code.saveUserParaFlag1;  // �ָ�����ֵ���ָ���ֵ����EEPROM�ָ���0
    }
    else if (SAVE_USER_PARA_PARA2 == index)   // USER PARA SAVE CHK2
    {
        init = funcCode.code.saveUserParaFlag2;
    }
    else                                    // REM(�������)
    {
        init = 0;
    }

    return init;
}









