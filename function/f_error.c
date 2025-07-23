//======================================================================
//
// ���ϴ���
//
// Time-stamp: <2012-03-14 14:08:23  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_error.h"
#include "f_main.h" 
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_eeprom.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_invPara.h"

#if F_DEBUG_RAM

#define DEBUG_F_ERROR                   0   // ���ϴ�����
#define DEBUG_F_ERROR_FROM_MOTOR        0   // ʹ�����ܵĹ���

#elif 1

#define DEBUG_F_ERROR                   1   // ���ϴ�����
//=m
#if defined(FUNC_DEBUG)
#define DEBUG_F_ERROR_FROM_MOTOR        0   // ʹ�����ܵĹ���
#else
#define DEBUG_F_ERROR_FROM_MOTOR        1   // ʹ�����ܵĹ���
#endif

#define DEBUG_F_ERROR_LEVEL             1   // ���Ϸּ�

#endif



//------------------------------------------
Uint16 errorEeprom;     // EEPROM��д����
Uint16 errorOther;      // �������ϣ������ⲿ���ϡ���г���ϡ�ͨѶ���ϡ�����ʱ�䵽���
Uint16 errorOtherOld;
Uint16 errorCode;       // �¹���
// #define ERROR_NONE                0     // 0  -- ��
// #define ERROR_INVERTER_UNIT       1     // 1  -- ��䵥Ԫ����
// #define ERROR_OC_ACC_SPEED        2     // 2  -- ���ٹ�����
// #define ERROR_OC_DEC_SPEED        3     // 3  -- ���ٹ�����
// #define ERROR_OC_CONST_SPEED      4     // 4  -- ���ٹ�����
// #define ERROR_OV_ACC_SPEED        5     // 5  -- ���ٹ���ѹ
// #define ERROR_OV_DEC_SPEED        6     // 6  -- ���ٹ���ѹ
// #define ERROR_OV_CONST_SPEED      7     // 7  -- ���ٹ���ѹ
// #define ERROR_8                   8     // 8  -- ���Ƶ�Դ����
// #define ERROR_UV                  9     // 9  -- Ƿѹ����
// #define ERROR_OL_INVERTER         10    // 10 -- ��Ƶ������
// #define ERROR_OL_MOTOR            11    // 11 -- �������
// #define ERROR_LOSE_PHASE_INPUT    12    // 12 -- ����ȱ��
// #define ERROR_LOSE_PHASE_OUTPUT   13    // 13 -- ���ȱ��
// #define ERROR_OT_IGBT             14    // 14 -- ɢ��������
// #define ERROR_EXTERNAL            15    // 15 -- �ⲿ����
// #define ERROR_COMM                16    // 16 -- ͨѶ����
// #define ERROR_CONTACTOR           17    // 17 -- �Ӵ�������
// #define ERROR_CURRENT_SAMPLE      18    // 18 -- ����������
// #define ERROR_TUNE                19    // 19 -- �����г����
// #define ERROR_ENCODER             20    // 20 -- ���̹���
// #define ERROR_EEPROM              21    // 21 -- �������
// #define ERROR_22                  22    // 22 -- ��Ƶ��Ӳ������
// #define ERROR_MOTOR_SHORT_TO_GND  23    // 23 -- ����Եض�·����
// #define ERROR_24                  24    // 24 -- ����
// #define ERROR_25                  25    // 25 -- ����
// #define ERROR_RUN_TIME_OVER       26    // 26 -- ����ʱ�䵽��
// #define ERROR_SOFT_OC             27    // 31 -- �������
// #define ERROR_CBC                 40    // 40 -- ����������ʱ����
// #define ERROR_SWITCH_MOTOR        41    // 41 -- ����ʱ�л����

//struct ERROR_DATA errorData;
union ERROR_ATTRIBUTE errorAttribute;   // ��ǰ���ϵ�����
Uint16 errorInfo;

struct CUR_TIME curTime;   // ����ʱʱ�����

LOCALF enum ERROR_DEAL_STATUS errorDealStatus;

enum UV_DEAL_STATUS
{
    UV_READY_FOR_WRITE_EEPROM,      // Ƿѹ״̬��׼�������������
    UV_WAIT_WRITE_EEPROM,           // Ƿѹ״̬���ȴ��������
    UV_WRITE_EEPROM_OK              // ���������ɡ��ȴ��ָ���ѹ
};
LOCALF enum UV_DEAL_STATUS uvDealStatus;

Uint16 bUv;
LOCALF Uint32 errAutoRstClrWaitTicker;  // �����Զ���λ��������ĵȴ�ʱ���ticker
LOCALF Uint16 errorSpaceTicker;         // �����Զ���λ���ʱ��

Uint16 errAutoRstNum;                   // �Զ���λ���ϴ�������

Uint16 errorInfoFromMotor;
Uint16 errorCodeOld;   // �Ϲ���
Uint16 errorInfoOld;
union ERROR_ATTRIBUTE errorAttributeOld;

const Uint16 errorCodeFromMotorList[] =
{
    ERROR_OC_ACC_SPEED,       // 2  -- ���ٹ�����
    ERROR_OV_ACC_SPEED,       // 5  -- ���ٹ���ѹ
    ERROR_BUFFER_RES,         // 8  -- ���������ع���
    ERROR_UV,                 // 9  -- Ƿѹ����
    
    ERROR_OL_INVERTER,        // 10 -- ��Ƶ������
    ERROR_OL_MOTOR,           // 11 -- �������
    ERROR_LOSE_PHASE_INPUT,   // 12 -- ����ȱ��
    ERROR_LOSE_PHASE_OUTPUT,  // 13 -- ���ȱ��
    
    ERROR_OT_IGBT,            // 14 -- ɢ��������
    ERROR_CONTACTOR,          // 17 -- �Ӵ�������
    ERROR_CURRENT_SAMPLE,     // 18 -- ����������
    ERROR_MOTOR_SHORT_TO_GND, // 23 -- ����Եض�·����
    
    ERROR_LOSE_LOAD,          // 30 -- ����
    ERROR_CBC,                // 40 -- ����������
    ERROR_INIT_POSITION,      // 51 -- ��ʼλ�ô���
    ERROR_RSVD,               // 52 -- ����
    
    ERROR_RSVD,               // 99 -- ����ִ���߼�����
    ERROR_TUNE,               // 19 -- ��г����
    ERROR_UVW_FDB             // 53 -- uvw�źŷ�������
};

//------------------------------------------
LOCALD void underVoltageDeal(void);
void PowerOffRemDeal(void);
void ErrorSceneSave(void);
void UpdateError(void);
Uint16 AutoResetErrorTrueDeal(void);
Uint16 AutoResetErrorFcDeal(void);
Uint16 AutoResetErrorDeal(void);
void GetNewErrorAttribute(void);
void GetNewError(void);
void GetErrorResetDeal(void);



#if DEBUG_F_ERROR
//=====================================================================
//
// ����: ���ϴ���
// ����: ����errorCode������
//          1. Ƿѹ����
//          2. ���籣��
//          3. ���¹���
//          4. �����ֳ�����
// 
//=====================================================================
void ErrorDeal(void)
{
    // Ƿѹ����
    underVoltageDeal();
    // ���¹���
    UpdateError();
    // �����ֳ�����
    ErrorSceneSave();
}


// Ƿѹ����
LOCALF void underVoltageDeal(void)
{
#if DEBUG_F_ERROR_FROM_MOTOR
    // �ϵ���ɺ����ܱ�ĸ�ߵ�ѹ��ѹ �� ���ܵȴ�ʱ�䳬ʱ
    if (((!dspStatus.bit.uv) && (POWER_ON_WAIT != powerOnStatus))    // ���ϵ����֮�󣬲��ж��Ƿ�Ƿѹ
        || (POWER_ON_FUNC_WAIT_OT == powerOnStatus))                // ���ܵȴ�ʱ�䳬ʱ����ΪǷѹ
    {
        bUv = 1;  // ��Ƿѹ��־
    }
#endif

    if (bUv)          // Ƿѹ
    {
        if (runFlag.bit.run)        // ��Ҫ�ȴ����籣����ϲ���ʾ���ϡ�������ʾ����
                                    // Ƿѹʱ���������runFlag.bit.runҲΪ1����������PWM����������
        {
            errorOther = ERROR_UV;  // ����ʱ������(����)����
        }
        // ��errorReset()�������ض�PWM�������������
        
        switch (uvDealStatus)       // Ƿѹ״̬
        {
            // ׼�������������
            case UV_READY_FOR_WRITE_EEPROM:
                // û������EEPROM��������
                if (!funcCodeRwModeTmp)
                {
                    PowerOffRemDeal();          // ������䴦��
                    // ��Ƿѹ״̬Ϊ�ȴ��������
                    uvDealStatus = UV_WAIT_WRITE_EEPROM;
                }
                break;
            // �ȴ��������
            case UV_WAIT_WRITE_EEPROM:          // �ȴ����籣�����
                // û��EEPROM���������ʾ����EEPROM���������
                if (!funcCodeRwMode)       
                {
                    // ��Ƿѹ״̬Ϊ�ȴ��ָ���ѹ
                    uvDealStatus = UV_WRITE_EEPROM_OK;
                }
                break;
            // �ȴ��ָ���ѹ
            case UV_WRITE_EEPROM_OK:
                // �жϲ���Ƿѹ�������ǣ�����û��Ƿѹ��־���ҹ��ܵ��ϵ��߼��Ѿ����
                if ((dspStatus.bit.uv)      // ������Ƿѹ��־�������籣�����֮�󣬲��ж��Ƿ��Ѿ���Ƿѹ
//                    && (POWER_ON_CORE_OK == powerOnStatus)
                    )
                {
                    // ��Ƿѹ״̬��Ƿѹ��־
                    uvDealStatus = UV_READY_FOR_WRITE_EEPROM;
                    bUv = 0;
                }
                break;

            default:
                break;
        }
    } // if (bUv)
}


// ���¹���
void UpdateError(void)
{
    // �����Ϲ���, �ϵĹ�������
    {
        errorCodeOld = errorCode;                   
        errorAttributeOld.all = errorAttribute.all;
        errorInfoOld = errorInfo;
    }

    // ����µĹ���
    GetNewError();

    // ����¹��ϵ�����
    GetNewErrorAttribute();

    // ȷ���Ƿ����Ϊ�µĹ���
    if (errorCodeOld != errorCode)  // �����и���
    {
        if (ERROR_LEVEL_NO_ERROR != errorAttribute.bit.level)  // ���µĹ���
        {
            if (ERROR_LEVEL_NO_ERROR == errorAttributeOld.bit.level)  // ֮ǰ�޹���
            {
                ;
            }
#if DEBUG_F_ERROR_LEVEL             
            // �µĹ��ϣ����ȼ�����
            else if (errorAttribute.bit.level < errorAttributeOld.bit.level)
            {
                ;
            }
			// �ϵĹ������ȼ�Ϊ�������У���Ӧ�¹���
            else if (errorAttributeOld.bit.level == ERROR_LEVEL_RUN)
			{
				;
			}
#endif            
            // �µĹ��ϣ����ȼ������ϵĸ�,�ָ�Ϊ�ϵĹ���
            else
            {
                errorCode = errorCodeOld;
                errorAttribute.all = errorAttributeOld.all;
            }
        }
        else    // �޹���. �ָ�Ϊ�ϵĹ���
        {
            errorCode = errorCodeOld;
            errorAttribute.all = errorAttributeOld.all;
        }
    }
}

//=====================================================================
//
// ����: ������ܹ�����Ϣ
// ����: 
// 
//=====================================================================
#if 0
Uint16 getMotorErrorInfo(Uint16 index)
{
    Uint16 i, j, errorInfo;
#if DEBUG_F_ERROR_FROM_MOTOR  
    
#if 1
    errorInfo = (motorErrInfor[index/4] << ((index%4)*4)) >> 12;
#else
    // �ṹ����λ��
    //size = sizeof(errorCodeFromMotorList);
    i = index%4;  // ���ڹ�����Ϣ�ṹ�������
    j = index/4;  // ���ڹ�����Ϣ��������
    
    switch(i)
    {
        case 0:   
            errorInfo = motorErrInfor[j].bits.errorInfo1;
            break;
            
        case 1:   
            errorInfo = motorErrInfor[j].bits.errorInfo2;
            break;
            
        case 2:   
            errorInfo = motorErrInfor[j].bits.errorInfo3;
            break;
            
        case 3:   
            errorInfo = motorErrInfor[j].bits.errorInfo4;
            break;
            
        default:
		   errorInfo = 0;
            break;    
    }
#endif    

#endif
    return errorInfo;

    
}
#endif

//=====================================================================
//
// ����: ������ܹ���
// ����: ���ع��ϴ������ȼ���ߵĹ���
// 
//=====================================================================
void getMotorError(void)
{
#if DEBUG_F_ERROR_FROM_MOTOR
    Uint16 errorStatus;
    Uint16 errorCodeTmp, errorCode;
    Uint16 errorAttributeTmp, errorAttribute; 
    Uint16 errorInfo;
    Uint16 i;
    
    // ��ʼΪ�޹���
    errorCode = ERROR_NONE;
    errorAttribute = GetErrorAttribute(errorCode);
    
    // �������ܴ��ݵĹ���
    if (errorsCodeFromMotor[0] || errorsCodeFromMotor[1])
    {
        for (i = 0; i < sizeof(errorCodeFromMotorList); i++)
        {
            // 0~15�����ϴ��������1
            if ( i < 16)
            {
                // ����״̬(0-��Ч  1-��Ч)
                errorStatus = errorsCodeFromMotor[0] >> i;
            }
            // 16~31�����ϴ��������2
            else
            {
                // ����״̬(0-��Ч  1-��Ч)
                errorStatus = errorsCodeFromMotor[1] >> (i - 16);
            }

            // ������Ч
            if (errorStatus & 0x01)
            {
                // �����Ч���ϵĹ��ϱ���
                errorCodeTmp = errorCodeFromMotorList[i];
                // ��ù�������(�������С�����ͣ��������ͣ��)
                errorAttributeTmp = GetErrorAttribute(errorCodeTmp);
                // ���ȼ��Ƿ����ǰһ������
                if (errorAttributeTmp < errorAttribute)
                {
                    errorAttribute = errorAttributeTmp;
                    errorCode = errorCodeTmp;

                    // ��ȡ������Ϣ    
                    errorInfo = (motorErrInfor[i/4] >> ((i%4)*4)) & 0xF;

                    // Ϊ����ͣ���������ټ�������
                    if (ERROR_LEVEL_FREE_STOP == errorAttribute)
                    {
                        break;
                    }
                }
            }
            
        }
    }
    else
    {
        errorInfo = 0;
    }
#if 0    
    else
    {
        errorCode = 0;
    }
#endif    

    // ������ܹ���
    errorCodeFromMotor = errorCode;
    // ������ܹ�����Ϣ
    errorInfoFromMotor = errorInfo;
#endif    
}

// ����µĹ���
// Ƿѹ״̬����û�б�Ƿѹ����ʱ��ҲҪ��Ӧ��������
#define ERROR_EXTERNAL_INFO_OPEN    1
#define ERROR_EXTERNAL_INFO_CLOSE   2
#define DEV_CHK_TIME_MIN            5   //  ��С10ms���ʱ��
void GetNewError(void)
{
    static Uint16 devTcnt = 0;
    static Uint16  osTcnt = 0;
    static Uint16 motorSnOld;
	Uint16 errorLevel;
    
#if DEBUG_F_ERROR_FROM_MOTOR
    // ������ܴ��ݵĹ��ϼ���Ϣ
    getMotorError();
    errorInfo = errorInfoFromMotor;

    // ���ܹ�ѹ����
    if ((generatrixVoltage >= funcCode.code.ovPointSet)
        && (powerOnStatus == POWER_ON_CORE_OK)
        )
    {
        errorOther = ERROR_OV_ACC_SPEED;   // ������
        errorInfo = 0;
    }
    
    if (ERROR_OC_ACC_SPEED == errorCodeFromMotor)// ����
    {
        if (runFlag.bit.accDecStatus == ACC_SPEED)
        {
            errorCode = ERROR_OC_ACC_SPEED;     // ���ٹ�����
        }
        else if (runFlag.bit.accDecStatus == DEC_SPEED)
        {
            errorCode = ERROR_OC_DEC_SPEED;     // ���ٹ�����
        }
        else
        {
            errorCode = ERROR_OC_CONST_SPEED;   // ���ٹ�����
        }
    }
    else if ((ERROR_OV_ACC_SPEED == errorCodeFromMotor) // ��ѹ
            || (errorOther == ERROR_OV_ACC_SPEED)       // ���ܴ��ݵĹ�ѹ
            )
    {
        if (runFlag.bit.accDecStatus == ACC_SPEED)
        {
            errorCode = ERROR_OV_ACC_SPEED;     // ���ٹ���ѹ
        }
        else if (runFlag.bit.accDecStatus == DEC_SPEED)
        {
            errorCode = ERROR_OV_DEC_SPEED;     // ���ٹ���ѹ
        }
        else
        {
            errorCode = ERROR_OV_CONST_SPEED;   // ���ٹ���ѹ
        }
        
    }
    else if (ERROR_UVW_FDB == errorCodeFromMotor)
    {
        errorCode = ERROR_ENCODER;
        errorCodeFromMotor = ERROR_ENCODER;
    }
    else if (errorCodeFromMotor && (ERROR_UV != errorCodeFromMotor)) // ���������ܴ��ݵĹ���, Ƿѹ�Ѿ���ǰ�洦��
    {                       // �ϵ�ʱ����ֱ����ʾERR09
        errorCode = errorCodeFromMotor;     
        
    }
    else 
    {
         errorInfo = 0;
    }
#endif

	errorLevel = GetErrorAttribute(errorOther);

    // ����ʱ���ĵ������
    if ((motorSnOld != motorSn)
        && (runFlag.bit.run)
        )
    {
        errorOther = ERROR_SWITCH_MOTOR_WHEN_RUN;   // ������
        errorLevel = ERROR_LEVEL_FREE_STOP;
    }
    motorSnOld = motorSn;

    

    // �ⲿ��������;����/����   
	if ((diFunc.f1.bit.externalErrOpenIn) ||   // �ⲿ�������볤��
        ( (!diFunc.f2.bit.externalErrCloseIn) &&
          (diSelectFunc.f2.bit.externalErrCloseIn)
         )                                      // �ⲿ���ϳ���  
        )
    {
	    if(GetErrorAttribute(ERROR_EXTERNAL) < errorLevel)
	    {
            errorOther = ERROR_EXTERNAL;
			errorLevel = GetErrorAttribute(errorOther);
            
            if (diFunc.f1.bit.externalErrOpenIn)
            {
                errorInfo = ERROR_EXTERNAL_INFO_OPEN;   // ����
            }
            else
            {
                errorInfo = ERROR_EXTERNAL_INFO_CLOSE;  // ����
            }
        }
    }
    // �û��Զ������1
	if(diFunc.f2.bit.userError1)
    {
	    if(GetErrorAttribute(ERROR_USER_1) < errorLevel)
	    {
            errorOther = ERROR_USER_1;
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

    // �û��Զ������2
	if(diFunc.f2.bit.userError2)
    {
	    if(GetErrorAttribute(ERROR_USER_2) < errorLevel) 
	    {
            errorOther = ERROR_USER_2; 
			errorLevel = GetErrorAttribute(errorOther);
        }
    }
    
    // EEPROM���� 
	if (errorEeprom)
    {
	    if(GetErrorAttribute(ERROR_EEPROM) < errorLevel)
	    {
            errorOther = ERROR_EEPROM;   // ���ϱ�ʾ
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

    // �ٶ�ƫ�����
    if( (ABS_INT32(frqRun - frq) > ((Uint32)maxFrq*funcCode.code.devChkValue/1000)) // �ٶ�ƫ�����
	    && (RUN_MODE_TORQUE_CTRL != runMode)                      // ��ת�ؿ���
	    && runFlag.bit.run                                        // ������
	    && funcCode.code.devChkTime                               // ���ʱ�䲻Ϊ0
	    && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)  // �ջ�����
	    && (!runFlag.bit.tune)                                    // �����ڵ�г����
       )
    {
        if(devTcnt <= (funcCode.code.devChkTime*(100/ERROR_DEAL_PERIOD) + DEV_CHK_TIME_MIN))
        {
            devTcnt++;
        }         
    }
    else
    {
        devTcnt = 0;
    }

    if (devTcnt > (funcCode.code.devChkTime*(100/ERROR_DEAL_PERIOD)))
    {
	    if (GetErrorAttribute(ERROR_DEV) < errorLevel)
	    {
            errorOther = ERROR_DEV;
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

     // ������ٶ�
     // VF������Ч��ת�ؿ�����Ч
    if ((ABS_INT32(frqFdbTmp) > ((Uint32)maxFrq * (1000 + funcCode.code.osChkValue)/1000)) 
        && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)  // �ջ�����
        && funcCode.code.osChkTime                                // ���ʱ�䲻Ϊ0
        && (!runFlag.bit.tune)                                    // �����ڵ�г����
        && (runFlag.bit.run)                                      // ������
       )
    {
        if (osTcnt <= (funcCode.code.osChkTime*(100/ERROR_DEAL_PERIOD)))
        {
            osTcnt++;
        }  
    }
    else
    {
        osTcnt = 0;
    }   
    
	if (osTcnt > (funcCode.code.osChkTime*(100/ERROR_DEAL_PERIOD)))
    {
    	if (GetErrorAttribute(ERROR_OS) < errorLevel)
    	{
           errorOther = ERROR_OS;
		   errorLevel = GetErrorAttribute(errorOther);
        }
    }

#if DEBUG_F_PLC_CTRL
    // PLC�ɱ�̿����ݵĹ���
    if (funcCode.code.plcEnable && funcCode.code.plcErrorCode)
    {
		if((funcCode.code.plcErrorCode >= PLC_DEFINE_ERROR_START)
		 && (funcCode.code.plcErrorCode <= PLC_DEFINE_ERROR_END)
		 )
        {
            errorOther = funcCode.code.plcErrorCode;
            errorLevel = ERROR_LEVEL_FREE_STOP;
        }
    }
    // ��PLC������
    funcCode.code.plcErrorCode = 0;
#endif    
    
    // ���ܵĹ�����Ӧ�����������ʱ����Ӧ���ܹ���
    if (errorLevel < GetErrorAttribute(errorCode))
    {
        errorCode = errorOther; // other error, ���ⲿ����
    }
    
    errorOtherOld = errorOther;
    errorOther = ERROR_NONE;
}



// ��ȡ�¹��ϵĹ�������
void GetNewErrorAttribute(void)
{
    if (errorCode == ERROR_NONE)
    {
       errorAttribute.bit.level = ERROR_LEVEL_NO_ERROR;
    }
    else
    {
        errorAttribute.bit.level = GetErrorAttribute(errorCode);
    }
}



// �����ֳ����棬RAM����
void ErrorSceneSave(void)
{
    if (errorCode 
        && (errorCode != errorCodeOld) // �����µĹ��ϣ��Ž��й��ϼ�¼
        && ((errorCode < PLC_DEFINE_ERROR_START) || (errorCode > PLC_DEFINE_ERROR_END))
        ) 
    {
        int16 i;

        funcCode.code.errorLatest1 = funcCode.code.errorLatest2; // ���3�ι�������
        funcCode.code.errorLatest2 = funcCode.code.errorLatest3;
        funcCode.code.errorLatest3 = errorCode;

#define ERROR_SCENE_NUMBER  8  // �����ֳ���¼����

        // ��һ�ι��ϼ�¼
        for (i = ERROR_SCENE_NUMBER - 1; i >= 0; i--)
        {
            funcCode.code.errorScene1.all[i] = funcCode.code.errorScene2.all[i];
        }

        // �ڶ��ι��ϼ�¼
        for (i = ERROR_SCENE_NUMBER - 1; i >= 0; i--)
        {
            funcCode.code.errorScene2.all[i] = funcCode.code.errorScene3.all[i];
        }

        funcCode.code.errorScene3.elem.errorFrq = ABS_INT32(frqRun);         // ����ʱƵ��
        funcCode.code.errorScene3.elem.errorCurrent = outCurrentDispOld;     // ����ʱ������Ӧ��Ϊ��һ�ĵ��������
        funcCode.code.errorScene3.elem.errorGeneratrixVoltage = generatrixVoltage;   // ����ʱĸ�ߵ�ѹ
        funcCode.code.errorScene3.elem.errorDiStatus = diStatus.a.all;     // ����ʱDI״̬
        funcCode.code.errorScene3.elem.errorDoStatus = doStatus.a.all;     // ����ʱDO״̬
        funcCode.code.errorScene3.elem.errorInverterStatus = runFlag.all;  // ����ʱ��Ƶ��״̬
        funcCode.code.errorScene3.elem.errorTimeFromPowerUp = curTime.powerOnTime;     // ����ʱʱ�䣨�ӱ����ϵ翪ʼ��ʱ��
        funcCode.code.errorScene3.elem.errorTimeFromRun = curTime.runTime;         // ����ʱʱ�䣨������ʱ��ʼ��ʱ��

        // ����Ƶ�ʵ�С����
        funcCode.code.errorFrqUnit = ((funcCode.code.errorFrqUnit & 0x0FFF) << 4) + funcCode.code.frqPoint;

        if (ERROR_OC_ACC_SPEED == errorCodeFromMotor)   // ������// ���ܴ��ݹ���
        {
            // ���ϼ�¼
            if (currentOcDisp > outCurrentDispOld)      // ���ݵĹ��ϵ�������ǰһ�ĵ���
            {
                funcCode.code.errorScene3.elem.errorCurrent = currentOcDisp;
            }
        }
    }
}



//=====================================================================
//
// ���ϴ�����
//
// 1. �����ֳ�����, EEPROM
// 2. ���ϸ�λ����
// 
//=====================================================================
void ErrorReset(void)
{
    GetErrorResetDeal();

    //if (runFlag.bit.run)
    {
        // �ﵽ�����Զ���λ�������ʱ��(1h)�������Զ���λ��������
        if (++errAutoRstClrWaitTicker > 10*TIME_UNIT_ERR_AUTO_RST_CLR / ERROR_DEAL_PERIOD)
        {
            errAutoRstClrWaitTicker = 0;
            errAutoRstNum = 0;
        }
    }

    if (errorCode)
    {
        tuneCmd = 0;
        if (ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level)
        {
            runFlag.bit.run = 0;        // Ƿѹʱ���������Ҫrun����һ��
            
            // ��ǰ�������ض�PWM
            dspMainCmd.bit.run = 0;
            dspMainCmd1.bit.speedTrack = 0;
            dspMainCmd.bit.stopBrake = 0;
            dspMainCmd.bit.startBrake = 0;
            dspMainCmd.bit.startFlux = 0;
            dspMainCmd.bit.accDecStatus = 0;
        }

        errAutoRstClrWaitTicker = 0;    // �������ϣ������Զ���λ�������ʱ������

        switch (errorDealStatus)
        {
            case ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM:   // ������ϼ�¼
            
                if (!funcCodeRwModeTmp)
                {
                    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
                    startIndexWriteSeries = GetCodeIndex(funcCode.code.errorLatest1);     // ��һ�����ϼ�¼
                    endIndexWriteSeries = GetCodeIndex(funcCode.code.errorScene1.all[sizeof(struct ERROR_SCENE_STRUCT) - 1]);     // ���һ�����ϼ�¼

                    //SaveOneFuncCode(GetCodeIndex(funcCode.code.errorFrqUnit));  // ���� ����Ƶ�ʵ�С����

                    errorDealStatus = ERROR_DEAL_WAIT_FOR_WRITE_EEPROM;
                }
                break;

            case ERROR_DEAL_WAIT_FOR_WRITE_EEPROM:
                if (!funcCodeRwModeTmp)   // ������ϼ�¼���
                {
                    errorDealStatus = ERROR_DEAL_WRITE_EEPROM_OK;
                    
                    errorEeprom = ERROR_EEPROM_NONE;    // �����ܹ�ִ�е��������EEPROM�ܳɹ���д��
                }
                break;

            case ERROR_DEAL_WRITE_EEPROM_OK:
                if (AutoResetErrorDeal())       // ��λ����
                {
                    errorSpaceTicker = 0;       // ���Զ���λ(�����ֶ�)ʱ���Զ���λ���ʱ������

                    if (errorCodeFromMotor)     // ���ܴ��ݵĹ���
                    {
                        ERROR_DEALING = 1;      // ��������Ϣ: �����Ѿ�����ù���
                    }

                    errorDealStatus = ERROR_DEAL_OK;
                }

                break;

            case ERROR_DEAL_OK:
                // ��errorDealingΪ1ʱ��������_���ڣ����errorCodeFromMotor���㡣
                // ����Ҫ��֤_����errorDealingû�����±��0��
                // ������ʱ...��
                ERROR_DEALING = 0;              // ��� ��������Ϣ: �����Ѿ�����ù���

                if (errorCode == errorOther)    // ��λ֮ǰû���µĹ���
                {
                    errorOther = ERROR_NONE;
                }
                if (errorCode == errorCodeFromMotor)
                {
                    errorCodeFromMotor = ERROR_NONE; // ҲҪ���. �����Ժ�ͨѶ����
                }

                errorCode = ERROR_NONE;

                errorAttribute.bit.level = ERROR_LEVEL_NO_ERROR;

                errorDealStatus = ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM;
                break;

            default:
                break;
        }
    }    
}



// ��ȡ���ϸ�λ����
extern Uint16 parallelMasterSendErrorResetTicker;
void GetErrorResetDeal(void)
{
    static Uint16 diFuncErrRstOld;
    
    // �ж��Ƿ�error reset��������Ч
    if ((!diFuncErrRstOld) && (diFunc.f1.bit.errorReset))   // DI�Ĺ��ϸ�λ״̬��Ϊ��Ч
    {
        runCmd.bit.errorReset = 1;          // ������Դ�Ŀ�ʼ�Ѿ�����
    }
    diFuncErrRstOld = diFunc.f1.bit.errorReset;
    
    if ((KEY_STOP == keyFunc) // �κο��Ʒ�ʽ�£�STOP���Ĺ��ϸ�λ���ܾ���Ч
        && ((ERROR_LEVEL_RUN != errorAttribute.bit.level)    // ���Ϻ�������еĹ��ϣ�stop��Ϊ���٣���Ϊ��λ
            || (!runFlag.bit.run)
            )
        )
    {
        runCmd.bit.errorReset = 1;
    }
}



// �Զ���λ���ϴ���
Uint16 AutoResetErrorDeal(void)
{
    Uint16 bResetError = 0;
    
    if ((AutoResetErrorTrueDeal()) 
        || AutoResetErrorFcDeal())
        bResetError = 1;

    return bResetError;
}


// ����Ҫ���������ã����Զ���λ���ϵĴ���
// �磬Ƿѹ����
Uint16 AutoResetErrorTrueDeal(void)
{
    Uint16 bResetError = 0;
    
    // Ƿѹ֮���ѹ�ָ������������ܰ��ա�ֻҪ����û��Ƿѹ��־������Ϊ��ѹ�Ѿ��ָ��������Զ����Ƿѹ���ϡ�
    if (ERROR_UV == errorCode)
    {
        if (!bUv)                       // ���ܴ��ݵ�Ƿѹ�����Ѿ�ȡ��
        {
            bResetError = 1;            // �Զ���λ����
        }
    }
    else if (ERROR_EEPROM == errorCode) // EEPROM����
    {
        if (ERROR_EEPROM_NONE == errorEeprom)   // EEPROM�޹���
        {
            bResetError = 1;                    // �Զ���λ����
        }
    }
#if DEBUG_F_ERROR_LEVEL    
    // �������еĹ��ϣ��Զ���λ
    else if (ERROR_LEVEL_RUN == errorAttribute.bit.level)
    {
        // ʵ�ʹ��ϲ�Ϊ��ǰ��ʾ����
        if((errorCode != errorOtherOld) && 
           (errorCode != errorCodeFromMotor))
        {
            bResetError = 1;
        }
    }
#endif
    return bResetError;
}



// ������Ĺ����Զ���λ�������ֶ����ϸ�λ
Uint16 AutoResetErrorFcDeal(void)
{
    Uint16 bResetError = 0;
    Uint16 errAutoRstSpaceTime;
    
    if (ERROR_MOTOR_SHORT_TO_GND != errorCode)  // �ϵ�Եض�·�����ܸ�λ
    {
        if (runCmd.bit.errorReset)  // �ֶ���λ����(ǷѹҲ�����ֶ���λ)
        {
            //runCmd.bit.errorReset = 0;//���Բ��������ÿ1�ĵĿ�ʼ��������
            errAutoRstNum = 0;      // �ֶ����ϸ�λ�������Զ���λ��������
            // ֻ������ͣ�����ϲ��ֶ���λ��ͣ��������Լ������������趨��δ�ӱ��������Ͽ��ֶ���λ
            if (((ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level) || (!runFlag.bit.run))
               )
            {
                bResetError = 1;
            }
        }
        else //if ((ERROR_EXTERNAL != errorCode)             // �����ⲿ���ϸ�λ
             //   && (ERROR_RUN_TIME_OVER != errorCode)      // ��������ʱ�䵽���Զ���λ
             //   && (ERROR_POWER_UP_TIME_OVER != errorCode) // �����ϵ�ʱ�䵽���Զ���λ
             //   && (ERROR_USER_1 != errorCode)             // �����û��Զ������1�Զ���λ 
             //   && (ERROR_USER_2 != errorCode)             // �����û��Զ������2�Զ���λ
             //   )            
        {   
            // �Զ���λ����������Ϊ0ʱ�������Զ���λ���ʱ����Ч, ���ԣ�����Ϊ<��������<=
            if (errAutoRstNum < funcCode.code.errAutoRstNumMax)
            {
                // Ϊ��������ʱ�����Զ���λʱ�䴦��Ϊ��С0.5s
                if ((errorCode == ERROR_OC_ACC_SPEED) 
                    || (errorCode == ERROR_OC_DEC_SPEED)
                    || (errorCode == ERROR_OC_CONST_SPEED))
                {
                    errAutoRstSpaceTime = ((funcCode.code.errAutoRstSpaceTime > 5) ? funcCode.code.errAutoRstSpaceTime : 5);
                }
                else
                {
                    errAutoRstSpaceTime = funcCode.code.errAutoRstSpaceTime;
                }
                
                if (++errorSpaceTicker >= (Uint32)errAutoRstSpaceTime 
                    * (Uint16)(TIME_UNIT_ERR_AUTO_RST_DELAY / RUN_CTRL_PERIOD))
                {
                    errAutoRstNum++;                // �Զ���λ���ϴ�������
                    runCmd.bit.startProtect = 0;    // ��������

                    bResetError = 1;
                }
            }
        }
    }

    return bResetError;
}


//======================================================= 
//��ȡ���ϵĹ�������
//=======================================================
Uint16 GetErrorAttribute(Uint16 errCode)
{
    Uint16 digit[5];
    Uint16 level;

    level = ERROR_LEVEL_FREE_STOP;
#if DEBUG_F_ERROR_LEVEL
    if (errCode == ERROR_NONE)
	{
        level = 4;
    }   
    // ÿ5�����ϵĹ��ϱ�������ѡ�񣬷���һ�����������5λ
    else if ((errCode == ERROR_OL_MOTOR)             // 11
    	||  (errCode == ERROR_LOSE_PHASE_INPUT)  // 12
    	||  (errCode == ERROR_LOSE_PHASE_OUTPUT) // 13
    	||  (errCode == ERROR_EXTERNAL)          // 15
   		||  (errCode == ERROR_COMM)              // 16
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[0]);
        if(errCode < ERROR_OT_IGBT)
        {
            level = digit[errCode - ERROR_OL_MOTOR] + 1;
        }
        else
        {
            level = digit[errCode - ERROR_LOSE_PHASE_INPUT] + 1;
        }
	}
    else if( (errCode == ERROR_ENCODER)           // 20
        ||  (errCode == ERROR_EEPROM)            // 21
        ||  (errCode == ERROR_24)                // 24
        ||  (errCode == ERROR_25)                // 25
        ||  (errCode == ERROR_RUN_TIME_OVER)     // 26
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[1]); 
        if(errCode < ERROR_22)
        {
            level = digit[errCode - ERROR_ENCODER] + 1;
        }
        else
        {
            level = digit[errCode - ERROR_24 + 2] + 1;
        }
    }
    else if( (errCode == ERROR_USER_1)              // 27 -- �û��Զ������1
    	||  (errCode == ERROR_USER_2)              // 28 -- �û��Զ������2
    	||  (errCode == ERROR_POWER_UP_TIME_OVER)  // 29 -- �ϵ�ʱ�䵽��
    	||  (errCode == ERROR_LOSE_LOAD)           // 30 -- ����
    	||  (errCode == ERROR_FDB_LOSE)            // 31 -- ����ʱPID������ʧ
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[2]); 
        level = digit[errCode - ERROR_USER_1] + 1;
    }     
    else if ((errCode == ERROR_DEV)                       // 42 �ٶ�ƫ�����
    	||  (errCode == ERROR_OS)                       // 43 ������ٶ�  
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[errCode - ERROR_DEV] + 1;
    }
    else if (errCode == ERROR_INIT_POSITION)            // 51 ��ʼλ�ô���
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[2] + 1;
    } 
#if 0     
    else if (errCode == ERROR_SPEED_DETECT)              // 52 �ٶȷ�������
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[3] + 1;
    }
    else if (errCode == ERROR_PROGRAM_LOGIC)             // 99 ����ִ���߼�����
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[4] + 1;
    }
#endif
 #endif     
    return level;
}


#elif 1

void ErrorDeal(void){}

#endif



