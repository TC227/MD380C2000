/****************************************************************
�ļ����ܣ��͵��������صĳ����ļ����������ģ������岿��
�ļ��汾�� 
�������ڣ� 

****************************************************************/
#include "MotorInclude.h"
#include "MotorDataExchange.h"
#include "SystemDefine.h"
#include "MotorDefine.h"

// // �ڲ���������
void RunCaseDeal05Ms(void);
void RunCaseDeal2Ms(void);
void RunCaseRun05Ms(void);
void RunCaseRun2Ms(void);
void RunCaseStop(void);
void SendDataPrepare(void);		

/************************************************************
    ����ģ���ʼ��������ѭ��ǰ��ʼ�����ܲ��ֵı���
(���е���0�ı��������ʼ��)
************************************************************/
void InitForMotorApp(void)
{
	DisableDrive();
	TurnOffFan();
    
// ����������ʼ��
	gMainStatus.RunStep = STATUS_LOW_POWER;	//������
	gMainStatus.SubStep = 1;				//������
	gMainStatus.ParaCalTimes = 0;
	gError.LastErrorCode = gError.ErrorCode.all;
	//gMainStatus.StatusWord.all = 0;
	gCBCProtect.EnableFlag = 1;				//Ĭ����������������
	gADC.ZeroCnt = 0;
	gADC.DelaySet = 100;
	gADC.DelayApply = 600;
	gFcCal.FcBak = 50;
	gBasePar.FcSetApply = 50;
	gUVCoff.UDivV = 4096;
    gPWM.gPWMPrd = C_INIT_PRD;

    gMainStatus.StatusWord.all = 0;
    
// ʸ����ر�����ʼ��
    gRotorTrans.Flag = 0;   //ͬ������ʼ��, ��������õ�
    gFVCSpeed.MTCnt = 0;
    gFVCSpeed.MTLimitTime = 0;
    gFVCSpeed.MSpeedSmooth.LastSpeed = 0;
    gFVCSpeed.MSpeedSmooth.SpeedMaxErr = 1500;
    gFVCSpeed.MTSpeedSmooth.LastSpeed = 0;
    gFVCSpeed.MTSpeedSmooth.SpeedMaxErr = 1500;
    gFVCSpeed.TransRatio = 1000;                  // ���ٴ����ȹ̶�ֵ
    gPGData.QEPIndex = QEP_SELECT_NONE; 
    gPGData.PGType = PG_TYPE_NULL;                  // ��ʼ��Ϊ null��
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE; 

// ���������صĳ�ʼ����Ĭ�����첽�����
    gMotorInfo.MotorType = MOTOR_TYPE_IM;
    //gMotorInfo.LastMotorType = gMotorInfo.MotorType;
    gMotorInfo.LastMotorType = MOTOR_NONE;                // ��֤������������ܽ�����س�ʼ��
    
    //(*EQepRegs).QEINT.all = 0x0;  //ȡ��QEP��I�ź��ж�
    
    gPGData.PGMode = 0;
    gFVCSpeed.MDetaPosBak = 0;

    gIPMPos.ZErrCnt = 0	;
    gIPMPosCheck.UvwRevCnt = 0	;
    gIPMInitPos.Flag = 0;

    gIPMInitPos.InitPWMTs = (50 * DSP_CLOCK);	  //500us
    gPWM.PWMModle = MODLE_CPWM;

    gIPMPos.Zcounter  = 0;              // ��¼����z�жϵĴ��������ڼ���ABZ,UVW ������ͬ������ʶ���ɹ�������
                                        // �鿴uf-25;
//
    ParSend2Ms();
    ParSend05Ms();
}

/************************************************************
�����򲻵ȴ�ѭ��������ִ����Ҫ����ˢ�µĳ���Ҫ�����ǳ����
************************************************************/
void Main0msMotor(void)
{
    if(gPGData.PGMode == 0)
    {
    GetMDetaPos();
    GetMTTimeNum();
    }
}

/************************************************************
����:
1. ���������١�SVC�ٶȼ��㣻
2. �ٶȱջ����ƣ�
3. ���¼���gMainCmd.FreqSetApply�� ���ϴ���gMainCmd.FreqToFunc��

4. SVC �����б��������٣� FVC��VF�����������ٶȣ�

************************************************************/
void Main05msMotor(void)
{
  if(ASYNC_SVC == gCtrMotorType)  // SVC
    {
        //gFVCSpeed.SpeedEncoder = 0;
        VCGetFeedBackSpeed();               //���������٣�ȷ���������        
    }
            // 2808ʱ��SVC������Ҳ����
    else
    {
        VCGetFeedBackSpeed();               //���������٣�ȷ���������        
    }    

    #ifdef MOTOR_SYSTEM_DEBUG
    DebugSaveDeal(3);
    #endif 
}
/************************************************************
������ĵ�2msѭ��������ִ�е�����Ƴ���
˼·����������->����ת��->�����㷨->������������->���ұ���->�������
ִ��ʱ�䣺�����жϴ�ϵ������Լ120us
************************************************************/
void Main2msMotorA(void)
{

//�ӿ��ư��ȡ����	
	ParGet2Ms();
    ParGet05Ms();

//�첽��������ʶʱ���Բ�������������Ҫ����Ҫ�Ż�
    if(STATUS_GET_PAR == gMainStatus.RunStep)
    {
        ChgParForEst();
    }
    
//ParameterChange2Ms();
    if(gMainCmd.Command.bit.Start == 0)
    {
        SystemParChg2Ms();
        SystemParChg05Ms();                 // ����ʱ��ת���Ĳ���

        ChangeMotorPar();       //�������ת���� ���в�ת��
    }
    RunStateParChg2Ms();
    //RunStateParChg05Ms();

}

void Main2msMotorB(void)
{  
  int     m05HzPu;
  int     m20HzPu;
    m20HzPu = (200L<<15) / gBasePar.FullFreq01;

	if(gIPMZero.zFilterCnt)				    // pm Z filter	
	{
		gIPMZero.zFilterCnt--;  
	}
    gMainCmd.FreqSetApply = (long)gMainCmd.FreqSet;
//    gMainCmd.FreqSetApply = Filter2((long)gMainCmd.FreqSet,gMainCmd.FreqSetApply);
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:
            if(gMainStatus.RunStep != STATUS_SPEED_CHECK)          // SVC speed-check
            {   
             	if(0 == gVCPar.SvcMode)  //ʹ��ԭ380�㷨���Ա��ں�320����
                {   
                    m05HzPu = (50L<<15) / gBasePar.FullFreq01;
                    if((abs(gMainCmd.FreqSet)+3) < m05HzPu) 
                    {
                        gMainCmd.FreqSet = 0;
                        gMainCmd.FreqSetApply = 0;
                    }
                    SVCCalRotorSpeed();
                    VcAsrControl();
                    CalWsAndSynFreq();   // ����ת��Ƶ��
                }
    			else
    			{
                    if((1 == gMainCmd.Command.bit.TorqueCtl)||(1 == gTestDataReceive.TestData1))
					{}
					else
					{
                        m05HzPu = (50L<<15) / gBasePar.FullFreq01;
                        if((abs(gMainCmd.FreqSet)+3) < m05HzPu) 
                        {
                            gMainCmd.FreqSet = 0;
                            gMainCmd.FreqSetApply = 0;
                        }
					}
                    SVCCalRotorSpeed_380();
                    VcAsrControl();
                    CalWsAndSynFreq_380();   // ����ת��Ƶ��
    			
    			}
                gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
                }
            else
            {
                gMainCmd.FreqSyn = gFeisu.SpeedCheck;
                gMainCmd.FreqToFunc = gFeisu.SpeedCheck;
                
               //IMTSet.M = (long)gMotorExtPer.IoVsFreq <<12;
                gIMTSet.T = 0L <<12;
                gOutVolt.VoltApply = (long)gOutVolt.VoltApply * gFeisu.VoltCheck >>12;
            }
            break;
            
        case ASYNC_FVC:
            VcAsrControl();     // FVC �ٶȻ�
            CalWsAndSynFreq();  // ����ת��Ƶ��
            gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            break;

        case SYNC_SVC:
        case SYNC_VF:
        case ASYNC_VF:
            if(gMainStatus.RunStep == STATUS_SPEED_CHECK)
            {                
                gOutVolt.Volt           = gFeisu.VoltCheck;
                gMainCmd.FreqSyn      = gFeisu.SpeedCheck;
                gMainCmd.FreqToFunc     = gMainCmd.FreqSyn;
                gVFPar.FreqApply        = gMainCmd.FreqSyn;
                gOutVolt.VoltPhaseApply = (gFeisu.SpeedLast > 0) ? 16384 : -16384;                
            }            
            break;

        case SYNC_FVC:
//            gMainCmd.FreqFeed = gFVCSpeed.SpeedEncoder;
            VcAsrControl();     // synFVC �ٶȻ�
            gMainCmd.FreqSyn = gMainCmd.FreqFeed;
            gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            break;           

        case DC_CONTROL:
            gMainCmd.FreqSyn = 0;
            RunCaseDcBrake();
            gOutVolt.VoltPhaseApply = 0;        // ���ǵ�ͬ�����������ѹ��׼ת�Ӵż���ֱ�������ͻ��ڸ÷���
		                                        // ���Ӵ����ͻ��ڸ÷����ϣ�
            gMainCmd.FreqToFunc = 0;		    
            break;

        case RUN_SYNC_TUNE:  // Ŀ������ͬ����������ʶ
            ;
            // ������ʶ�󣬵�����������Ҫ���ñ�ʶ�õ���pi����, �õ�pi����ǰ�ı�ʶ���̲���ʹ�õ�����
            gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 80;                
            gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply / 80;
            gImAcrQ24.KI = gPmParEst.IdKi;
            gItAcrQ24.KI = gPmParEst.IqKi;
            
           
            
            if((TUNE_STEP_NOW == PM_EST_NO_LOAD) ||                 // pm ���ر�ʶ�������Ƕ�
                (TUNE_STEP_NOW == PM_EST_BEMF))                      // pm ���綯�Ʊ�ʶ��
            {           
                gMainCmd.FreqSyn = 0;
                gIMTSet.T = 0;

                if(TUNE_STEP_NOW == PM_EST_BEMF)
                {
                    gMainCmd.FreqSyn = gEstBemf.TuneFreqSet;
                    gEstBemf.IdSetFilt = Filter4(gEstBemf.IdSet, gEstBemf.IdSetFilt);
                    gIMTSet.M = (long)gEstBemf.IdSetFilt << 12;       // Q12->Q24
                }
                gMainCmd.FreqToFunc = gMainCmd.FreqSyn;
            }
            
            if(TUNE_STEP_NOW == PM_EST_WITH_LOAD)       // pm ���ر�ʶ
            {
                PrepareAsrPar(); 
                CalTorqueLimitPar();
                CalUdcLimitIT();                   //ʸ�����ƵĹ�ѹ���ƹ���
                VcAsrControl();                         // synFVC �ٶȻ�
                
                gIMTSet.M = 0;
                gMainCmd.FreqSyn = gMainCmd.FreqFeed;
                gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            }
                
            break;

        default:
            gMainCmd.FreqSyn = 0;
            gMainCmd.FreqToFunc = 0;
            break;
    }

    //�����ز�Ƶ��
	CalCarrierWaveFreq();

    // ���ÿ���ģʽ�͵�����͵���ϣ����ڿ����߼�������
	if(MOTOR_TYPE_PM != gMotorInfo.MotorType)  
    {   
        if(gMainStatus.RunStep != STATUS_GET_PAR)
        {
            gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)gMainCmd.Command.bit.ControlMode;
        }
        else        // im tune
        {
            gCtrMotorType = ASYNC_VF;
        }        
    }
    else if(MOTOR_TYPE_PM ==gMotorInfo.MotorType)
    {
        gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)(gMainCmd.Command.bit.ControlMode + 10);
    }
    // ֱ���ƶ�
    if((1 == gMainCmd.Command.bit.StartDC) || (1 == gMainCmd.Command.bit.StopDC))
    {
        gCtrMotorType = DC_CONTROL;
    }

    //���ݱ�Ƶ��״̬�ֱ���, ���ܻ����¸��� gCtrMotorType(�������ڸ�2ms�����У���Ȼ�ᵼ�´���)
	switch(gMainStatus.RunStep)
	{
		case STATUS_RUN:		                    //����״̬������VF/FVC/SVC����
			RunCaseRun2Ms();
			break;

        case STATUS_STOP:
            RunCaseStop();
            break;

        case STATUS_IPM_INIT_POS:                   //ͬ������ʼλ�ýǼ��׶�
			RunCaseIpmInitPos();            
            break;
            
		case STATUS_SPEED_CHECK:                    //ת�ٸ���״̬
		
			if(gComPar.SpdSearchMethod == 3)    RunCaseSpeedCheck();
            else                                RunCaseSpeedCheck();
			break;

		case STATUS_GET_PAR:	                    //������ʶ״̬���Ƶ�0.5msʱҪͬʱ�޸Ĳ�������
			RunCaseGetPar();

            if(TUNE_STEP_NOW == IDENTIFY_LM_IO)
            { 
                 
        		VfOverCurDeal();
        		VfOverUdcDeal();
        		VfFreqDeal();                        // gVFPar.FreqApply

                gMainCmd.FreqToFunc = gVFPar.FreqApply;
                gMainCmd.FreqSetApply = gVFPar.FreqApply;

                gWsComp.CompFreq = 0;       // ת���Ϊ0�� ת������ҲΪ0
                VFSpeedControl();
                CalTorqueUp(); 
                HVfOscDampDeal();             // HVf �����ƣ� ������ѹ��λ��ȡ��MD320�����Ʒ�ʽ��2011.5.7 L1082
                gOutVolt.VoltPhaseApply = gHVfOscDamp.VoltPhase;            
                gOutVolt.Volt = gHVfOscDamp.VoltAmp;   
                VFOverMagneticControl();     
            }
			break;

		case STATUS_LOW_POWER:	                    //�ϵ绺��״̬/Ƿѹ״̬
			RunCaseLowPower();
			break;
            
		case STATUS_SHORT_GND:	                    //�ϵ�Եض�·�ж�״̬
			RunCaseShortGnd();
			break;
                       
		default:
            gMainStatus.RunStep = STATUS_STOP;      // �ϵ��һ�Ż����
			break;
	}	
}

void Main2msMotorC(void)
{
    InvCalcPower();     // ���ʡ�ת�صļ���
    VfOscIndexCalc();
    
//��Ƶ��������ͱ���	
	InvDeviceControl();			
}

void Main2msMotorD(void)
{
//������Ư��⣬AD��Ư�����Զȼ��	
	GetCurExcursion();				    

//׼����Ҫ���͸����ư������
    SendDataPrepare(); 
    
//��ʵʱ���ݴ��͸����ư�	
	ParSend2Ms();
    ParSend05Ms();

    gCpuTime.CpuBusyCoff = (Ulong)gCpuTime.Det05msClk * 655 >> 16;  // div100
    gCpuTime.CpuCoff0Ms = gCpuTime.tmp0Ms;
    gCpuTime.tmp0Ms = 0;
// End

    #ifdef MOTOR_SYSTEM_DEBUG
    DebugSaveDeal(2);
    #endif 
}

/*************************************************************
	Ϊ����ģ��׼����Ҫ�����в���
*************************************************************/
void SendDataPrepare(void)		
{
    Uint tempU;
    int   mAiCounter;
    Ulong mTotal1;
    Ulong mTotal2;
    Ulong mTotal3;
    Uint   mRatio;
    
	///////////////////////////////////////////////ͣ��ʱ����ʾ����Ϊ0����
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||
	   (gMainStatus.RunStep == STATUS_STOP) ||
	   (gLineCur.CurBaseInv < (4096/50))    ||          //������С�ڱ�Ƶ�������2%����ʾ0
	   (1 == gMainStatus.StatusWord.bit.OutOff ))	
	{
		gLineCur.CurPerShow = 0;
        gLineCur.CurTorque  = 0;
	}
	else
	{
		gLineCur.CurPerShow = gLineCur.CurPerFilter >> 7;
        gLineCur.CurTorque  = Filter32(abs(gIMTQ12.T), gLineCur.CurTorque);
	}
    
	//ͬ�����Ƕ�ת��
	tempU = (Uint)((int)gRotorTrans.RTPos + gRotorTrans.PosComp);
    gRotorTrans.RtRealPos = ((Ulong)tempU * 3600L + 10) >> 16;
	if(gMotorInfo.MotorType == MOTOR_TYPE_PM)
    {   
	    gIPMPos.RealPos = ((Ulong)gIPMPos.RotorPos * 3600L + 10) >> 16;
    }
    
    // ai ��������
    DINT;
    mTotal1 = gAI.ai1Total;
    mTotal2 = gAI.ai2Total;
    mTotal3 = gAI.ai3Total;
    mAiCounter = gAI.aiCounter;
    
    gAI.ai1Total = 0;
    gAI.ai2Total = 0;
    gAI.ai3Total = 0;
    gAI.aiCounter = 0;
    EINT;
    
    gAI.gAI1 = mTotal1 / mAiCounter;
    gAI.gAI2 = mTotal2 / mAiCounter;
    gAI.gAI3 = mTotal3 / mAiCounter;

    // ����ʵ�������ѹ
    mRatio = __IQsat(gRatio, 4096, 0);                              // û�й�����
    mRatio= (Ulong)mRatio * gUDC.uDC / gInvInfo.BaseUdc;            // �Ա�Ƶ�����ѹΪ��ֵ
    gOutVolt.VoltDisplay = Filter4(mRatio, gOutVolt.VoltDisplay);
}

/************************************************************
    ����������ʽ����Ҫ���Ϊ05ms�ٶȻ�����׼���ò�����
    
************************************************************/
void RunCaseRun2Ms(void)
{
    //EnableDrive();
    if(gMainCmd.Command.bit.Start == 0)         // ��������
    {
        gMainStatus.RunStep = STATUS_STOP;
        RunCaseStop();
        return;
    }

    gMainStatus.StatusWord.bit.StartStop = 1;        
    // Ϊת��׷��׼������
    gFeisu.SpeedLast = (gMainCmd.FreqSyn) ? gMainCmd.FreqSyn : gFeisu.SpeedLast; 
    
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:  //�첽��ʸ������            
        case ASYNC_FVC:
            CalTorqueLimitPar();                // ����ת�����޺�ת�ؿ���
            PrepareAsrPar();
            PrepImCsrPara();
            CalIMSet();							// ���ŵ�������      
            CalUdcLimitIT();                   //ʸ�����ƵĹ�ѹ���ƹ���
            break;
            
        case ASYNC_VF:  //�첽����ͬ����VF����,�ݱ������?
        case SYNC_VF:            
            VFWsTorqueBoostComm();				//ת�����ת�����������������㡣
            VFWSCompControl();					//ת�������(����F)
            VFAutoTorqueBoost();        
            #if 1                           // �������ƹ���Ƶ�����Ӹ���������´���
            if(speed_DEC &&(abs(gMainCmd.FreqSet) > abs(gVFPar.tpLst)))
            {
              gMainCmd.FreqSet = gVFPar.tpLst;
            }
            gVFPar.tpLst = gMainCmd.FreqSet;
            #endif
    		VfOverCurDeal();				        //�������ƴ���(����F)
    		VfOverUdcDeal();					    //��ѹ���ƴ���(����F)
    		VfFreqDeal();                           // gVFPar.FreqApply

            gMainCmd.FreqToFunc = gVFPar.FreqApply;
            gMainCmd.FreqSetApply = gVFPar.FreqApply;

            VFSpeedControl();
            CalTorqueUp(); 
            
            HVfOscDampDeal();             // HVf �����ƣ� ������ѹ��λ
            gOutVolt.VoltPhaseApply = gHVfOscDamp.VoltPhase;            
            gOutVolt.Volt = gHVfOscDamp.VoltAmp;
            
            VFOverMagneticControl();               
            break;
            
        case SYNC_SVC:
            break;
            
        case SYNC_FVC:
            CalTorqueLimitPar();
            CalUdcLimitIT();                   //ʸ�����ƵĹ�ѹ���ƹ���// ����ת�����޺�ת�ؿ���
            PrepareAsrPar();
            //PrepareCsrPara();
            PrepPmsmCsrPrar();
            
            PmFluxWeakDeal();                   // pm ���Ŵ���
            break;

        case DC_CONTROL:
            ;
            break;
                        
        default:            
            break;         
    }
}

/************************************************************
    �����������ǰ�����ݳ�ʼ������Ϊ�������׼����ʼ����

************************************************************/
void PrepareParForRun(void)
{
// ����������ʼ��
    gMainStatus.StatusWord.bit.StartStop = 0;
    gMainStatus.StatusWord.bit.SpeedSearchOver = 0;

	gMainStatus.PrgStatus.all = 0;
	gMainStatus.PrgStatus.bit.ACRDisable = 1;    
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gVarAvr.UDCFilter = gInvInfo.BaseUdc;
	gMainCmd.FreqSyn = 0;
	gMainCmd.FreqReal = 0;
	gOutVolt.Volt = 0;
	gOutVolt.VoltApply = 0;
	gRatio = 0;
	gCurSamp.U = 0;
	gCurSamp.V = 0;
	gCurSamp.W = 0;
	gCurSamp.UErr = 600L<<12;
	gCurSamp.VErr = 600L<<12;
    gIUVWQ24.U = 0;
    gIUVWQ24.V = 0;
    gIUVWQ24.W = 0;
	gIUVWQ12.U = 0;
	gIUVWQ12.V = 0;
	gIUVWQ12.W = 0;
	gLineCur.CurPerShow = 0;
    gLineCur.CurTorque  = 0;
	gLineCur.CurBaseInv = 0;
	gLineCur.CurPer = 0;
	gLineCur.CurPerFilter = 0;
	gIMTQ12.M = 0;
	gIMTQ12.T = 0;
    gIMTQ24.M = 0;
    gIMTQ24.T = 0;
	gDCBrake.Time = 0;
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE;
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;

// Vf ��ض���ʼ��
    VfVarInitiate();
    
// ʸ����ر�����ʼ��
    gSpeedFilter.Max = 32767;	//�ٶ��˲�		
    gSpeedFilter.Min = 3277;
    gSpeedFilter.Output = 0;
    //if((IDC_SVC_CTL == gMainCmd.Command.bit.ControlMode) ||
    //    (IDC_FVC_CTL == gMainCmd.Command.bit.ControlMode))
    if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
    {
  	    ResetParForVC();  //VF���øú��������⣬��Ϊ���ı��������ѹ��Ƶ��
    }
    
//ͬ����������ر���
	gFluxWeak.AdjustId = 0;

//ת�ٸ��ٷ�ʽ2������ʼ��
	gFeisuNew.gWs_out = 0;
	gFeisuNew.t_DetaTime = 0;
	gFeisuNew.stop_time = 0;
	gFeisuNew.inh_mag = 0;
	gFeisuNew.ang_amu =0;
	gFeisuNew.jicicg  =0;
	gFeisuNew.jicics=0;
}

/************************************************************
	�л���ͣ��״̬(�����Ӻ���)
************************************************************/
void TurnToStopStatus(void)
{
	DisableDrive();
	gMainStatus.RunStep = STATUS_STOP;
	gMainStatus.SubStep = 1;
}

/*******************************************************************
    ͣ��״̬�Ĵ���
********************************************************************/
void RunCaseStop(void)
{
//ͣ���������
	DisableDrive();	    
	PrepareParForRun();
    gMainCmd.FreqToFunc = 0; 

//�ȴ���Ư������
	if(gMainStatus.StatusWord.bit.RunEnable != 1)
    {
        return;
    }

//�ж��Ƿ���Ҫ�Եض�·���
	if((1 == gExtendCmd.bit.ShortGnd) && (gMainStatus.StatusWord.bit.ShortGndOver == 0))
	{
		gMainStatus.RunStep = STATUS_SHORT_GND;
		gMainStatus.SubStep = 1;        // ���½��жԵض�·���
		return;
	}
	else
	{
		gMainStatus.StatusWord.bit.ShortGndOver = 1;
	}

// ͬ����ͣ��ʱλ��У��
    if(gMotorInfo.MotorType == MOTOR_TYPE_PM)
	{
		IPMCheckInitPos();                  
	}

//�ж��Ƿ���Ҫ�𶯵��
	if(gMainCmd.Command.bit.Start == 1)	
	{
    #ifdef MOTOR_SYSTEM_DEBUG
        if(gTestDataReceive.TestData16)         // Cf-15
        {
            ResetDebugBuffer();
        }
    #endif
        // 
        PmChkInitPosRest();                     // ͬ����ͣ��λ��У�鸴λ

	    // ������ʶ
	    if(TUNE_NULL != gGetParVarable.TuneType)
	    {
		    gMainStatus.RunStep = STATUS_GET_PAR;
            PrepareParForTune();
		    return;			
	    }
        //ת�ٸ�����
		if(0 != gExtendCmd.bit.SpeedSearch)	
		{
			gMainStatus.RunStep = STATUS_SPEED_CHECK;
			gMainStatus.SubStep = 1;
			PrepareForSpeedCheck();
			EnableDrive();
            return;
		}
        //ͬ����ʶ��ż���ʼλ�ýǽ׶�
    	if((gIPMInitPos.Flag == 0) &&
		    (gMotorInfo.MotorType == MOTOR_TYPE_PM) && 
		    (gPGData.PGType == PG_TYPE_ABZ))
		{
			gMainStatus.RunStep = STATUS_IPM_INIT_POS;
			gMainStatus.SubStep = 1;
            gIPMInitPos.Step = 0;

            return;
		}
        
        // else ...STATUS_RUN
		gMainStatus.RunStep = STATUS_RUN;
		gMainStatus.PrgStatus.all = 0;            
		gMainStatus.SubStep = 1;      
        
		EnableDrive();

        RunCaseRun2Ms();        // �Ż�����ʱ�䣬�ڸ��ľ��ܷ���
	}
}

/*************************************************************
	�������ϵ�����������ѹ�������ϵ��
*************************************************************/
void CalRatioFromVot(void)
{
	Uint	m_Ratio;
    
    // �������ϵ��
    if( 1 == gMainStatus.StatusWord.bit.OutOff )   //�������
    {
	    gOutVolt.VoltApply = 287;                       
    } 
	
	m_Ratio = ((Ulong)gOutVolt.VoltApply * (Ulong)gMotorInfo.Votage)/gInvInfo.InvVolt;	
    //gOutVolt.VoltDisplay = (m_Ratio > 4096) ? 4096 : m_Ratio;
	m_Ratio = ((Ulong)m_Ratio<<2) * gInvInfo.BaseUdc / gUDC.uDC >> 2;  // ��ѹ��ʱ���С1
	gRatio = (m_Ratio > 8192) ? 8192 : m_Ratio;
}

/*************************************************************
    �����жϣ����ģ�����������������㡢VC���������ƵȲ���

ע��:�ú����ڲ�����ʶ��Ҳ��ʹ�ã��������޸ģ���Ҫ����Ƿ�Ӱ����ر�ʶ
*************************************************************/
void ADCOverInterrupt()
{
//    Uint tempU;
    
    if(gPGData.PGType == PG_TYPE_RESOLVER)          // ����оƬ��ʼ����
	{	
		//GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;       //GPIO34��������������λ�ý���
		RT_SAMPLE_START;
        //gRotorTrans.IntCnt ++;          // ��¼��������
	}

    if(GetOverUdcFlag())                    //��ѹ����
    {
       HardWareOverUDCDeal();				
    }

// ��ȡģ����
	//ADCProcess();							//ADC��������
	GetUDCInfo();							//��ȡĸ�ߵ�ѹ��������    
	GetCurrentInfo();						//��ȡ��������, �Լ��¶ȡ�ĸ�ߵ�ѹ�Ĳ���
	
	ChangeCurrent();						//������ֳ����µĵ�����
	OutputLoseAdd();						//�ۼ��������ȱ���жϵĵ���

// ������д������������ƣ���������ѹ���������ϵ��
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:                                     //�첽������ʸ������
            CalcABVolt();
            SVCCalFlux_380();        

	    	if(0 == gVCPar.SvcMode)  //ʹ��ԭ380�㷨���Ա��ں�320����
            {
                SvcCalOutVolt();     // �����ѹ�趨
			}
			else
			{
       	        AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,gFluxR.Theta, &gIMTQ24_obs);
                //gIMTQ24_obs.T = ((llong)gIMTQ24_obs.T>>12) * ((llong)gIMTQ24_obs.M>>12) / (llong)gMotorExtPer.I0;
                //gIMTQ24_obs.T(Q12) = (llong)gIMTQ24_obs.T(Q24) * (llong)gMotorExtPer.I0(Q12) / (llong)gIMTQ24_obs.M(Q24);
                gIMTQ12_obs.M = (gIMTQ24_obs.M>>12);
                gIMTQ12_obs.T = (gIMTQ24_obs.T>>12);

               SvcCalOutVolt_380();     // �����ѹ�趨
			   //VCCsrControl_380();							        //�ջ�ʸ��IT��IM����

			}
            break;
            
        case ASYNC_FVC:                                     //�첽���ջ�ʸ������
            CalcABVolt();
            SVCCalFlux_380();
		    VCCsrControl();							        //�ջ�ʸ��IT��IM����
            break;
            
        case ASYNC_VF:                                      //�첽����ͬ����VF����,��ʱ������?
        case SYNC_VF:

            break;
            
        case SYNC_SVC:
            ;
            break;
            
        case SYNC_FVC:
            PmDecoupleDeal();
            VCCsrControl();
            break;
            
        case RUN_SYNC_TUNE:
            ;                                                   // ����ͬ����������
            if(TUNE_STEP_NOW == PM_EST_NO_LOAD ||
                TUNE_STEP_NOW == PM_EST_BEMF ||
                TUNE_STEP_NOW == PM_EST_WITH_LOAD)
            {                
                PmDecoupleDeal();
                VCCsrControl();
            }
            break;
            
        default:            
            break;         
    }  	
   if(DEADBAND_COMP_280 == gExtendCmd.bit.DeadCompMode)  
    {
         CalDeadBandComp();
    }
   else if(DEADBAND_COMP_380== gExtendCmd.bit.DeadCompMode)
    {
         HVfDeadBandComp();
    }
   else  
    {
     gDeadBand.CompU = 0;
     gDeadBand.CompV = 0;
     gDeadBand.CompW = 0;  /*��������ģʽΪ0ʱ���������������� 2011.5.7 L1082*/       
    }
     
// ͬ����������ʶ������ABZ��������һ���ϵ��ʼλ�ü��
    if((STATUS_GET_PAR ==gMainStatus.RunStep || gMainStatus.RunStep ==STATUS_IPM_INIT_POS)
        && (MOTOR_TYPE_PM == gMotorInfo.MotorType) 
        && (gIPMInitPos.Step != 0))
	{
		SynInitPosDetect();
	}
// ͬ�������ر�ʶ
    else if((STATUS_GET_PAR == gMainStatus.RunStep) && 
             (TUNE_STEP_NOW == PM_EST_WITH_LOAD) &&
             (gUVWPG.UvwEstStep == 2))
    {
        GetUvwPhase();
        gUVWPG.TotalErr += (long)(gIPMPos.RotorPos - gUVWPG.UVWAngle);
        gUVWPG.UvwCnt ++;
    }
//�첽��������ʶ������ص�������
	else if((STATUS_GET_PAR == gMainStatus.RunStep) &&
             (MOTOR_TYPE_PM != gMotorInfo.MotorType))
	{
		LmIoInPeriodInt();					
	}
#ifndef MOTOR_SYSTEM_DEBUG

#endif
    
    if(gPGData.PGMode == 0)
    {
        GetMDetaPos();
       GetMTTimeNum();
    }
    else if(gPGData.PGType == PG_TYPE_RESOLVER)  // �����źŲ�����ɣ��ȴ������ж϶�ȡ
	{
		//GpioDataRegs.GPBSET.bit.GPIO34 = 1;
		RT_SAMPLE_END;
        
        #ifdef TMS320F28035
        ROTOR_TRANS_RD = 1;
        ROTOR_TRANS_SCLK   = 1;
        #endif
    }

#ifdef MOTOR_SYSTEM_DEBUG       //rt debug
    #ifdef TMS320F2808
    DebugSaveDeal(0);
    #endif
#endif 
}

/*************************************************************
�����жϣ����ڷ���PWM��

ע��:�ú����ڲ�����ʶ��Ҳ��ʹ�ã��������޸ģ���Ҫ����Ƿ�Ӱ����ر�ʶ
*************************************************************/
void PWMZeroInterrupt()
{    
    CalRatioFromVot();// �������ϵ��   
    SoftPWMProcess();//���PWM������ȡ�ز����ںͽǶȲ���						
   	CalOutputPhase();//���������λ						

    BrakeResControl();      //�ƶ��������

// ���ݿ��Ʒ�ʽ PWM ����, ������ʶʱ���ܲ���Ҫ���﷢��
    if(gMainStatus.PrgStatus.bit.PWMDisable)
    {
        asm(" NOP");
    }
	else
	{	
	    OutPutPWMVF();      // SVPWM
	    //OutPutPWMVC();      // SPWM
	}
    
    if(gPGData.PGMode == 0)
    {
        GetMDetaPos();
        GetMTTimeNum();
    }
    else if(gPGData.PGType == PG_TYPE_RESOLVER)
    {
        GetRotorTransPos();
     //    RotorTransSamplePos();
        gRotorTrans.IntCnt ++;          // ��¼��������
    }
    
#ifdef MOTOR_SYSTEM_DEBUG       //rt debug
    #ifdef TMS320F2808
    DebugSaveDeal(1);
    #endif
#endif 
}

