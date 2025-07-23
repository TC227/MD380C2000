/****************************************************************
�ļ�����: �첽����ͬ����������ʶ���������ͺ�������
�ļ��汾�� 
���¸��£� 

*************************************************************/
#include "MotorParaIDinclude.h"
#include "MotorPmsmParEst.h"
#include "MotorEncoder.h"

// // ȫ�ֱ�������
UV_BIAS_COFF_STRUCT		 gUVCoff;
MOTOR_EXTERN_STRUCT		 gMotorExtReg;	            //�����չ��Ϣ�����������ʶ�õ������ݣ�
MOTOR_PARA_EST           gGetParVarable;
IDENTIFY_RRLO_VARIABLE   gRrLoIdentify;
IDENTIFY_LMIO_VARIABLE   gLmIoIdentify;

// // �첽����̬��ʶ˳��
PAR_EST_MAIN_STEP const AsynParEstStatic[IDENTIFY_PROGRESS_LENGTH] =
{
    IDENTIFY_RS,
    IDENTIFY_RR_LO,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

// // �첽��������ʶ˳��
PAR_EST_MAIN_STEP const AsynParEstRotor[IDENTIFY_PROGRESS_LENGTH] =   
{
    IDENTIFY_RS, 
    IDENTIFY_RR_LO,
    IDENTIFY_LM_IO,
    IDENTIFY_END,
    IDENTIFY_END
};

// // ͬ�������ر�ʶ˳��
PAR_EST_MAIN_STEP const pmTuneProgNoLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_NO_LOAD,
    PM_EST_BEMF,
    //IDENTIFY_END,
    //IDENTIFY_END,
    //IDENTIFY_END,
    IDENTIFY_END
};

// // ͬ�������ر�ʶ˳��
PAR_EST_MAIN_STEP const pmTuneProgLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_WITH_LOAD,
    //IDENTIFY_END,
    //IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

PAR_EST_MAIN_STEP const noTuneProgress[IDENTIFY_PROGRESS_LENGTH] =      
{
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

PAR_EST_MAIN_STEP const debugTuneProcess[IDENTIFY_PROGRESS_LENGTH] = 
{
    PM_EST_BEMF,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

// // �ڲ���������
void EndOfParIdentify(void);

/****************************************************************
    �������ܣ�������ʶ��ѭ���ṹ����
    
*****************************************************************/
void RunCaseGetPar(void)
{    
// ���ϻ��߹���ֹͣ��г��ֹͣ
	if((gError.ErrorCode.all != 0) || (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        DINT;                                       // ��ʱֹͣ�жϣ� ���ж���������
        
        gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep] = IDENTIFY_END;
        gGetParVarable.StatusWord = TUNE_FINISH;
	}
    
// ������г
    switch(gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep])
    {
       case IDENTIFY_RS:
           RsIdentify();
           break;
           
       case IDENTIFY_RR_LO:
            RrLoIdentify();
            break;
            
       case IDENTIFY_LM_IO:
            LmIoIdentify();
            break;
            
       case PM_EST_POLSE_POS:
			SynTuneInitPos();
            break;
            
	   case PM_EST_NO_LOAD:
			SynTunePGZero_No_Load();           
			break;

        case PM_EST_WITH_LOAD:
            SynTunePGZero_Load();
            break;

        case PM_EST_BEMF:
            SynTuneBemf();
            break;
            
       default: 
            EndOfParIdentify();
            break;            
    }
    
    ParSendTune();    
}

/************************************************************
	������ʶ��Ҫ�Ĳ����ĳ�ʼ��
	
************************************************************/
void PrepareParForTune(void)
{
    int m_index;
    PAR_EST_MAIN_STEP *m_PIdentifyFlow;
    
    //���в�����ʶ�����з��صı�������ҪԤ�ȸ�ֵ������ᵼ�¶�Ӧ�Ĺ��������
	gUVCoff.UDivVSave   = gUVCoff.UDivVGet;
    
	gMotorExtReg.R1     = gMotorExtInfo.R1;         // IM motor
    gMotorExtReg.R2     = gMotorExtInfo.R2;
    gMotorExtReg.L0     = gMotorExtInfo.L0;
    gMotorExtReg.LM     = gMotorExtInfo.LM;
    gMotorExtReg.I0     = gMotorExtInfo.I0;

    gMotorExtReg.RsPm   = gMotorExtInfo.RsPm;       // PM motor
    gMotorExtReg.LD     = gMotorExtInfo.LD;         
    gMotorExtReg.LQ     = gMotorExtInfo.LQ;
    gEstBemf.BemfVolt   = gMotorExtInfo.BemfVolt;               // PM ת�Ӵ��� %

    gPGData.PGDir               = gPGData.SpeedDir;
    gPGData.PGErrorFlag         = 0;
    gPmParEst.CoderPos_deg      = gIPMPos.RotorZeroGet;
    gPmParEst.UvwDir            = gUVWPG.UvwDir;
    gPmParEst.UvwZeroAng_deg       = gUVWPG.UvwZeroPos_deg;

    gPmParEst.IdKp = gVCPar.AcrImKp;
    gPmParEst.IdKi = gVCPar.AcrImKi;
    gPmParEst.IqKp = gVCPar.AcrItKp;
    gPmParEst.IqKi = gVCPar.AcrItKi;
    
    gGetParVarable.ParEstMstep = 0;
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gGetParVarable.IdSubStep = 1;                               // �ӹ��̲���
    gUVCoff.IdRsCnt = 0;
    gUVCoff.IdRsDelay = 0;    

    gIPMZero.DetectCnt = 0;                 // must be initiated
    gGetParVarable.QtEstDelay = 0;
            
    switch(gGetParVarable.TuneType)
    {
        case TUNE_IM_STATIC:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)AsynParEstStatic;
            break;
            
        case TUNE_IM_ROTOR:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)AsynParEstRotor;
            break;
            
        case TUNE_PM_COMP_LOAD:
			m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgLoad;
			break;
            
        case TUNE_PM_COMP_NO_LOAD:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgNoLoad;
            break;

        case TUNE_PM_PARA_temp:         //rt debug
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)debugTuneProcess;
			break;
            
		default:            
             m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)noTuneProgress;
            break;
    }
    
    for(m_index=0; m_index < IDENTIFY_PROGRESS_LENGTH; m_index++)
    {
        gGetParVarable.ParEstContent[m_index] = *(m_PIdentifyFlow + m_index);
    }
    
    InitRrLoVariable();                             //��ʼ��ת�ӱ�ʶ�ı�������Ҫ��WaitCntѭ������
    
}

/*******************************************************************
    ��������: ����������ʶ���ָ����ⲿģ����޸ģ�׼������
    �ӳ��Ƴ��������˳�������ʶ�󷴸�����
********************************************************************/
void EndOfParIdentify(void)
{ 
    if(gGetParVarable.QtEstDelay == 0)
    {
        gGetParVarable.ParEstMstep = 0;
        gMainStatus.PrgStatus.all = 0;
        if(TUNE_FINISH != gGetParVarable.StatusWord)
        {
            gGetParVarable.StatusWord = TUNE_SUCCESS;
        }
    	DisableDrive();
        
        EALLOW;  						                //�����û��������
        PIE_VECTTABLE_ADCINT = &ADC_Over_isr;		    //ADC�����ж�--INT1
        PieVectTable.EPWM1_TZINT = &EPWM1_TZ_isr;		//�����ж�--INT2
        PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//�����ж�--INT3
        PieCtrlRegs.PIEIER3.bit.INTx2 = 0;              //�ر�EPWM2�ж�
        EDIS;
        
    	InitSetPWM();
       	InitSetAdc();
        SetInterruptEnable();	                        // �����ʶ��Ŀ��;�˳����ж��п����ǹرյģ����ڴ˴�
       	EINT;   							    
       	ERTM;
    
    }
    else if(gGetParVarable.QtEstDelay >= 5)
    {
        if(gPGData.PGErrorFlag && gMainCmd.Command.bit.ControlMode == IDC_FVC_CTL)
        {
            gError.ErrorCode.all |= ERROR_ENCODER;      // �첽����ʶʱ�ٶȷ�������
            gError.ErrorInfo[4].bit.Fault1 = 8 +gPGData.PGErrorFlag;   // 9: δ�ӱ�����   
                                                                       // 10:��������������
            gPGData.PGErrorFlag = 0;
        }   

        gMainStatus.RunStep = STATUS_STOP;
    	gGetParVarable.IdSubStep = 1;
    }

    gGetParVarable.QtEstDelay ++;
}


