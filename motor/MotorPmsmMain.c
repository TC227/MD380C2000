/****************************************************************
�ļ����ܣ�����ͬ�����ջ�ʸ�����Ʋ��֣����ŵ�
�ļ��汾��ͬ�����������������
���¸��£�

****************************************************************/
//�ļ�����: 
//    1. ͬ����ͣ��״̬λ�ü�飻
//    2. ͬ��������ǰ�ĳ�ʼλ��ȷ����
//    3. ͬ����ABZ�������ż�λ�õ��ۼӼ��
//    4. ͬ������м��㣬����������������
//    5. ͬ���������Ŵ���
//    6. ABZ/UVW������z�жϵĴ���

#include "MotorVCInclude.h"
#include "MotorInclude.h"

//************************************************************
IPM_POSITION_STRUCT		gIPMPos;          //����ͬ�������ת�ӽǶ���صĽṹ
PM_FLUX_WEAK            gFluxWeak;
PM_INIT_POSITION        gPMInitPos;
IPM_POS_CHECK_STRUCT	gIPMPosCheck;   //����ͬ������ϵ��⵱ǰ����λ�ýǵ����ݽṹ
PM_DECOUPLE             gPmDecoup;


/*************************************************************
	ͬ�����±������Ļ�׼�жϵ���Ĵ������(���ó���)
*************************************************************/
interrupt void PG_Zero_isr(void)
{
    int  errZFlag;
    Uint mPos;
    
	EALLOW;
	(*EQepRegs).QCLR.all = 0x0401;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;	            // Acknowledge this interrupt
    EDIS;
    
    if(gIPMZero.zFilterCnt != 0)                        // Z �źŵ��˲�����Ҫ������Z�������4ms(1e7rpm)
	{
	    //gIPMPos.ZErrCnt ++;
		return;
	}    
 
    
    gIPMZero.zFilterCnt = 8;	                        // 4*0.5ms = 2ms
	gIPMZero.Flag = (gIPMZero.Flag | 0x01);	            // ��Z�źŵ����־
    gIPMPos.Zcounter ++;
    	  
	if(gMainStatus.RunStep != STATUS_GET_PAR)
	{    
	    errZFlag = 0;
		if(gPGData.PGType == PG_TYPE_ABZ)      // ABZ 
		{
			gIPMPos.AbzErrPos = (int)(gIPMPos.RotorZero- gIPMPos.ABZ_RotorPos_Ref);
			if(abs(gIPMPos.AbzErrPos) > (35*65536L/360))                    // abs(30deg)
			{
	            errZFlag ++;
			}                        
		}
        else if(gPGData.PGType == PG_TYPE_UVW)   // UVW, ����abz�źſ�λ��Ҳ��uvw�źſ�λ
        {            
            gIPMPos.AbzErrPos = (int)(gIPMPos.RotorPos - gIPMPos.RotorZero);
			if(abs(gIPMPos.AbzErrPos) > (65*65536L/360))                    // abs(60deg)
			{
	            errZFlag ++;
			} 
            
            GetUvwPhase();
            mPos = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;
            gUVWPG.UvwZIntErr = (int)(gIPMPos.RotorPos  - mPos);
            if(abs(gUVWPG.UvwZIntErr ) > (65L*32767/180))                   // �����������
            {
                 errZFlag ++;
            }
            #if 0           // ���abz�Ƕ��Ѿ�ƫ������ʹ��abz�ź�����λ����������z�źŸ�λ�Ƕ�            
            else    errZFlag = 0;
            #endif            
        }

        if(errZFlag)  
            {
            gIPMPos.ZErrCnt ++;                 // ����Z�ź�
            }
        else  
            {

              SetIPMPos(gIPMPos.RotorZero);       // ��λת�ӽǶ�
            }
	}    
    else
	{
		if(TUNE_STEP_NOW == PM_EST_NO_LOAD)              // ���ص�г���������λ�ý�ʱ
		{
			gIPMZero.FeedPos = 0;
			gIPMPos.QepTotal = 0;
			gIPMPos.QepBak   = GetQepCnt();
		}
		else if(TUNE_STEP_NOW == PM_EST_WITH_LOAD)      // ���ر�ʶʱ
		{
			gIPMPos.PosInZInfo = gIPMPos.RotorPos;

            if(gUVWPG.UvwEstStep == 1)
            {
                gUVWPG.UvwEstStep ++;   // gUVWPG.UvwEstStep==2: uvw est counter start
                gUVWPG.UvwCnt2 = 0;
            }
            else if(gUVWPG.UvwEstStep == 2)
            {
                gUVWPG.UvwCnt2 ++;
                if(gUVWPG.UvwCnt2 >= 3)     // 12����е����
                {
                    gUVWPG.UvwEstStep ++;   // gUVWPG.UvwEstStep==3: int complete
                }
            }
		}

        if(gPGData.PGType == PG_TYPE_UVW)
        {
            GetUvwPhase();
            //gPmParEst.UvwZPos = gUVWPG.UVWAngle;
        }
	}  
}

/***********************************************************************
    ͬ����ͣ��״̬�½Ƕ��޶�����,�ж�if(gMotorInfo.Type == MOTOR_TYPE_PM)
	�ŵ���
************************************************************************/
void IPMCheckInitPos(void)
{
	Uint m_Pos;
	int  m_Deta;
    Uint uvwPos;
    int  uvwPowOn;
    
	//�����ϴε���Ƕȣ�����ʶ�𱣴�Ƕ��Ƿ����
	uvwPowOn = 0;
    if(gMainStatus.ParaCalTimes == 0)  //�ϵ�ֻ����һ�εĲ���ת��
	{
        gMainStatus.ParaCalTimes = 1;        
        
	    gIPMPos.PowerOffPosDeg = ((Ulong)gIPMPos.PowerOffPos << 16)/3600;	//ʶ���ʼ�Ƕ�,��ȡ�ϴ��µ�Ƕ�
	    SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);
        SetIPMPos_ABZRef((Uint)gIPMPos.PowerOffPosDeg);

        PmChkInitPosRest(); 
        uvwPowOn ++;
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
    }
    
    // UVW��������ͣ��ʱ��UVW�ź���У��   
	if(gPGData.PGType != PG_TYPE_UVW)
	{
        return;
	}
    #if 0

    if(gIPMPosCheck.Cnt == 0)	                                //��һ������
	{
		gIPMPosCheck.FirstPos = gIPMPos.RotorPos;
		gIPMPosCheck.Cnt++;
	}
	else if(gIPMPosCheck.Cnt <= 128)					        //�ۼ�����
	{
		m_Deta = (int)(gIPMPos.RotorPos - gIPMPosCheck.FirstPos);
		gIPMPosCheck.TotalErr += m_Deta;
		gIPMPosCheck.TotalErrAbs += abs(m_Deta);
		gIPMPosCheck.Cnt++;
    }

   // if((gIPMPosCheck.Cnt > 128) ||          // �ж�256ms
     //   (uvwPowOn == 1))                      // ��һ���ϵ�    
    if((gIPMPosCheck.Cnt > 128) ||          // �ж�256ms
    (uvwPowOn == 1))                      // ��һ���ϵ� 
	{
		if((gIPMPosCheck.TotalErrAbs>>7) < ((5L<<15)/180))//rt             //rt �Ƕ����ƽ��ֵС��3��
		{
		    GetUvwPhase();
			m_Pos = (Uint)((int)gIPMPosCheck.FirstPos + (gIPMPosCheck.TotalErr>>7));
            uvwPos = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;

            gIPMPosCheck.UvwStopErr = (int)(m_Pos - uvwPos);
			if(abs(gIPMPosCheck.UvwStopErr) > (36L*32768L/180))   // ����������30deg, ����uvw�źŲ�һ����ȫ����
            {
				SetIPMPos(uvwPos);	        //d
				SetIPMPos_ABZRef(uvwPos);
                gIPMPosCheck.UvwRevCnt ++;
			}            
		}
        //else        // ���û��ͣ��           
		PmChkInitPosRest();                      	        //���¿�ʼʶ��
	}
    #endif
    #if 1
    GetUvwPhase();

    if(abs(gFVCSpeed.SpeedTemp)<100)
    {
         if(gIPMPosCheck.Cnt > 128) 
        {
            if((gIPMPosCheck.TotalErrAbs>>7) < 5461)
            {
                m_Pos = (Uint)((int)gIPMPosCheck.FirstPos + (gIPMPosCheck.TotalErr>>7));
                
                uvwPos = m_Pos + gUVWPG.UvwZeroPos;

                gIPMPosCheck.UvwStopErr = (int)(gIPMPos.RotorPos - uvwPos);
    			if(abs(gIPMPosCheck.UvwStopErr) > (36L*32768L/180))   // ����������30deg, ����uvw�źŲ�һ����ȫ����
                {
    				SetIPMPos(uvwPos);	        //d
    				//SetIPMPos_ABZRef(uvwPos);
                    //gIPMPosCheck.UvwRevCnt ++;
    			}            
            }
            PmChkInitPosRest();   
        }
        else if(gIPMPosCheck.Cnt == 0)
        {
            gIPMPosCheck.FirstPos = gUVWPG.UVWAngle;
		    gIPMPosCheck.Cnt++;
        }
        else
        {
            m_Deta = (int)(gUVWPG.UVWAngle - gIPMPosCheck.FirstPos);
    		gIPMPosCheck.TotalErr += m_Deta;
    		gIPMPosCheck.TotalErrAbs += abs(m_Deta);
    		gIPMPosCheck.Cnt++;
        }
    }
    #endif
}

/************************************************************

************************************************************/
void PmChkInitPosRest(void)
{
	gIPMPosCheck.Cnt = 0;               // 
    
	gIPMPosCheck.TotalErr = 0;
	gIPMPosCheck.TotalErrAbs = 0;	
}

/************************************************************
	����ͬ�����ż���ʼλ�ýǼ��׶�
************************************************************/
void RunCaseIpmInitPos(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        //if(gIPMPos.InitPosMethod == INIT_POS_VOLT_PULSE)
        //{
	    SynInitPosDetSetPwm(6);		    //ͬ����������ʶ�ָ��Ĵ�������
        //}
		gIPMInitPos.Step = 0;
		TurnToStopStatus();
		return;
	}

    switch(gMainStatus.SubStep)
    {
        case 1:
            if(gIPMInitPos.Step == 0)
            {
                gIPMInitPos.Step = 1;
                //gIPMInitPos.InitPWMTs = (50 * DSP_CLOCK);	  //500us
                gMainStatus.PrgStatus.bit.PWMDisable = 1;

                gMainStatus.SubStep ++;
            }
            else
            {
                gError.ErrorCode.all |= ERROR_PROGRAM_LOGIC;
                gError.ErrorInfo[4].bit.Fault1 = 1;
            }
            break;

        case 2:
            if(gIPMInitPos.Step == 0)           // �жϱ�ʶ���      
        	{ 
    			SetIPMPos((Uint)gIPMPos.InitPos);
                SetIPMPos_ABZRef((Uint)gIPMPos.InitPos);

        		if(abs((int)(gIPMPos.PowerOffPosDeg - gIPMPos.RotorPos)) < 3641)
        		{
        			//SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);	    //rt
        		}

                gIPMInitPos.Flag = 1;
        		DisableDrive();
        		gMainStatus.SubStep ++;             //���Զ��һ�ģ���PWM�ָ����
        	}   //else waiting interrupt deal
            break;
            
        case 3:
            	InitSetPWM();
   	            InitSetAdc();
                SetInterruptEnable();	            // �����ʶ��Ŀ��;�˳����ж��п����ǹرյģ����ڴ˴�
                gMainStatus.SubStep ++; 
                break;
                
        case 4:
            PrepareParForRun();
            gMainStatus.RunStep = STATUS_RUN;
            gMainStatus.SubStep = 0;
            gMainStatus.PrgStatus.all = 0;		//���п�����Ч
            EnableDrive();
            break;
            
        default:
            gError.ErrorCode.all |= ERROR_PROGRAM_LOGIC;
            gError.ErrorInfo[4].bit.Fault1 = 2;
            
            break;   
    }
}

/*************************************************************
	ͬ������ ABZ������ʱ��QEP����ʵʱλ�ýǣ�
�������ܹ�֧�����32767�������ı�����
ouputVar: gIPMZero.FeedPos

?? �����ʱ��Z�ж��ж���?
*************************************************************/
void SynCalRealPos(void)
{
    long    mQepPos;
    long    mCntMod;       // ȡģֵ
    long    m_ABZCntMod;
	long    m_4Pluse;
    long    m_DetaCnt;
	int     m_Pos;
    int     m_ABZPos;
    int     mSign;

    long  m_ABZQepPos;
    long  m_ABZDetaCnt;
    
    DINT;
    mQepPos = GetQepCnt();		
    m_DetaCnt = mQepPos - gIPMPos.QepBak;
    gIPMPos.QepBak = mQepPos;
    gIPMPos.QepTotal += m_DetaCnt;
    
	m_4Pluse = ((long)gPGData.PulseNum)<<2;
	mCntMod = gIPMPos.QepTotal % m_4Pluse;
	m_Pos = (mCntMod<<14)/(int)gPGData.PulseNum;			
	gIPMZero.FeedPos = m_Pos * gMotorExtInfo.Poles;						//��Ƕ�
    

    if(gPGData.PGType == PG_TYPE_ABZ)
    {
        m_ABZQepPos = GetQepCnt();
        m_ABZDetaCnt = m_ABZQepPos - gIPMPos.ABZ_QepBak;
        gIPMPos.ABZ_QepBak = m_ABZQepPos;
        gIPMPos.ABZ_QepTotal += m_ABZDetaCnt;

        m_4Pluse = ((long)gPGData.PulseNum)<<2;
	    m_ABZCntMod = gIPMPos.ABZ_QepTotal % m_4Pluse;
        m_ABZPos = (m_ABZCntMod<<14)/(int)gPGData.PulseNum;
        gIPMZero.FeedABZPos = m_ABZPos* gMotorExtInfo.Poles;
        
    }
    EINT;
    
    if(labs(gIPMPos.QepTotal) > (50L*4L* gPGData.PulseNum))       // ����50Ȧ����ȥ30Ȧ����tota����
    {
        mSign = (gIPMPos.QepTotal > 0) ? 1 : -1;
        gIPMPos.QepTotal -= (30L*4L* gPGData.PulseNum * mSign);
    }
    if(labs(gIPMPos.ABZ_QepTotal)>(50L*4L* gPGData.PulseNum))
    {
        mSign = (gIPMPos.ABZ_QepTotal > 0) ? 1 : -1;
        gIPMPos.ABZ_QepTotal -= (30L*4L* gPGData.PulseNum * mSign);
    }

    
}

/************************************************************
	LD��LQ���м��㺯��
	LAB = A-B*Cos(2*Theta-4*pi/3)	:= gIPMInitPos.LPhase[0]
	LBC = A-B*Cos(2*Theta)		     := gIPMInitPos.LPhase[1]
	LCA = A-B*Cos(2*Theta+4*pi/3)	:= gIPMInitPos.LPhase[2]
************************************************************/
void SynCalLdAndLq(Uint m_Pos)
{
	int m_Cos1,m_Cos2;
	int m_CoffA,m_CoffB;
	Uint m_Angle;

	m_Angle = (m_Pos<<1);
	m_Cos1 = qsin(16384 - (int)m_Angle);        // cos(2*theta)
	m_Angle += 43691;
	m_Cos2 = qsin(16384 - (int)m_Angle);        // cos(2*theta + 4*pi/3)

	m_CoffB = gIPMInitPos.LPhase[2] - gIPMInitPos.LPhase[1];
	m_CoffB = ((long)m_CoffB<<15)/(m_Cos1 - m_Cos2);
	m_CoffA = gIPMInitPos.LPhase[1] + (((long)m_CoffB * (long)m_Cos1)>>15);

	m_CoffA = m_CoffA>>1;
	m_CoffB = m_CoffB>>1;
	gIPMInitPos.Ld = m_CoffA - m_CoffB;
	gIPMInitPos.Lq = m_CoffA + m_CoffB;

	//gMotorExtInfo.LD = gIPMInitPos.Ld;
	//gMotorExtInfo.LQ = gIPMInitPos.Lq;
    gMotorExtReg.LD = gIPMInitPos.Ld;
    gMotorExtReg.LQ = gIPMInitPos.Lq;
}

/************************************************************
	����Ӧ����������ĵ���������(����8KHz�ز�Ƶ�ʼ���)
	m_kp_m = m_ld*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_m = m_res*m_In*65536.0/(1.3*64*m_Un);
	m_kp_t = m_lq*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_t = m_res*m_In*65536.0/(1.3*64*m_Un);
	
	���գ����-mH������-A����ѹ-V������-ŷķ���ز�Ƶ��8KHz
	m_kp_m = m_ld*m_In*197/m_Un;
	m_ki_m = m_res*m_In*788/m_Un;
	m_kp_t = m_lq*m_In*197/m_Un;
	m_ki_t = m_ki_m
	
	���գ����-0.01mH������-0.01A����ѹ-V������-0.001ŷķ���ز�Ƶ��8KHz
	m_kp_m = m_ld*m_In/(51*m_Un);
	m_ki_m = m_res*m_In/(128*m_Un) = (m_res*m_In/m_Un)>>7;
	m_kp_t = m_lq*m_In/(51*m_Un);
	m_ki_t = m_ki_m
************************************************************/
void IPMCalAcrPIDCoff(void)
{
    Uint temp;
    
#if 1
    // gain of axis-d
	temp = ((Ulong)gMotorExtReg.LD * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IdKp = (Ulong)temp * 1290L >> 10; // * 1.26
    // gain of axis-q
	temp = ((Ulong)gMotorExtReg.LQ * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IqKp = (Ulong)temp * 1290L >> 10; // * 1.26
    // ��������
	gPmParEst.IdKi = (((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Votage)>>(1);    // * 0.504
	gPmParEst.IqKi = gPmParEst.IdKi;    //
#else       //d
;   gPmParEst.IdKp = 1000;
    gPmParEst.IdKi = 1000;
    gPmParEst.IqKp = 1000;
    gPmParEst.IqKi = 1000;
#endif
   
}

void PrepPmsmCsrPrar()
{     
    long    ImKp, ImKi, ItKp, ItKi, temp;
    int     Nf;
    int     sGain;  // ���ֵ�������
    int     tempFreq;

    tempFreq = (gBasePar.FcSetApply > 40) ? gBasePar.FcSetApply : 40;   // ���鷢�����������
    
// ͬ���������ز���������������
	ImKp = (long)gVCPar.AcrImKp * tempFreq  / 60;
    ImKi = gVCPar.AcrImKi;
    
    ItKp = (long)gVCPar.AcrItKp * tempFreq  / 60;
    ItKi = gVCPar.AcrItKi;

    //gFluxWeak.CsrGainMode = 1;      //d 
    //if(gFluxWeak.CsrGainMode == 0)    // ͬ����������pi���治����         //rt
    gImAcrQ24.KP = ImKp;
    gItAcrQ24.KP = ItKp;

   /*		
// ͬ��������ʱ��pi��������
    // ���� �ز��ȵ��ڱ�������
    if(gFluxWeak.AbsFrqLpf < gMotorInfo.FreqPer)
    {
        Nf = 40;
    }
    else
    {
        temp = (gBasePar.FullFreq01/100) * gFluxWeak.AbsFrqLpf >>15;        // si-1Hz
        Nf = ((long)gBasePar.FcSetApply * 100) / temp;                      // �ز��ȼ���
    }
    if(Nf >= 40)
    {
        gImAcrQ24.KP = ImKp;
        gItAcrQ24.KP = ItKp;
    }
    else if(Nf >= 20)
    {
        gImAcrQ24.KP = ImKp * 40L / Nf;
        gItAcrQ24.KP = ItKp * 40L / Nf;
    }
    else //(Nf <20)
    {
        gImAcrQ24.KP = ImKp * 2;
        gItAcrQ24.KP = ItKp * 2;
    }    */
    // ��������Ƶ�ʵ��ڻ�������
    if(gFluxWeak.AbsFrqLpf <= gMotorInfo.FreqPer)
    {
        gImAcrQ24.KI = ImKi;
        gItAcrQ24.KI = ItKi;
    }
    else    // gFluxWeak.FreqLpf < 32767
    {
        sGain = gFluxWeak.CoefKI;
        temp = 32767L - gMotorInfo.FreqPer;
        temp = (((long)gFluxWeak.AbsFrqLpf -gMotorInfo.FreqPer)<<5) / temp;       // Q5
        gImAcrQ24.KI = ((1L<<5) + temp * sGain) * ImKi >>5;            // d �������������1��
        gItAcrQ24.KI = ((1L<<5) + (temp * sGain<<2)) * ItKi >>5;       // q �������������4��
    }
}

/**************************************************************************************
   pm ���ż���, ����M ������趨��
   
**************************************************************************************/
void PmFluxWeakDeal()
{
    int     frq;
    int     minFrq;
    long    mVd;
	//long	mVq;
    long    temp1, temp2;
    int     maxCur;
    
    gFluxWeak.IqLpf   = Filter4(gIMTSet.T>>12, gFluxWeak.IqLpf);
    gFluxWeak.VoltLpf = Filter4(gOutVolt.VoltApply, gFluxWeak.VoltLpf);

    //minFrq = (long)gMotorInfo.FreqPer * 3 >> 2;               // 75% ����Ƶ�����²�����
    minFrq = 5;                                          // ��֤���Ƶ�ʷ�0
    frq = (abs(gMainCmd.FreqSyn) > minFrq) ? abs(gMainCmd.FreqSyn) : minFrq;
    gFluxWeak.AbsFrqLpf = Filter4(frq, gFluxWeak.AbsFrqLpf);

//...
    // ��Ҫ����
    temp1 = (long)gFluxWeak.AbsFrqLpf * gMotorExtPer.LQ >> 9;                   // Q15
    mVd   = (long)gFluxWeak.IqLpf * temp1 >> 15;                                // Q12
    gFluxWeak.Vd = __IQsat(mVd, 32767, -32767);

    temp1 = (long)gOutVolt.MaxOutVolt - 400L; 
    temp2 = temp1 * temp1 - (long)gFluxWeak.Vd * gFluxWeak.Vd;                  // Q24
    temp2 = __IQsat(temp2, 0x7FFFFFFF, 0);
    gFluxWeak.Vq   = qsqrt(temp2);                                                       // Q12
    
    temp2 = ((long)gFluxWeak.Vq<<15)/gFluxWeak.AbsFrqLpf - gMotorExtPer.FluxRotor;   // Q12
    gFluxWeak.FluxSd = __IQsat(temp2, 0, -32767);
    temp1 = (((long)gFluxWeak.FluxSd <<15) /gMotorExtPer.LD) >>6;                 // Q12                         // Q12
    gFluxWeak.IdSet = temp1 * gFluxWeak.CoefFlux / 100L;                        // Q12

    maxCur = 4096L * gFluxWeak.IdMax /100;              // Q12
    gFluxWeak.IdSet = __IQsat(gFluxWeak.IdSet, 0, -maxCur);
   

    if(gFluxWeak.Mode == 2)     // ����ģʽ2�� �ٽ�������
    {                
        gFluxWeak.CoefAdj = __IQsat(gFluxWeak.CoefAdj, 1000, 10);    // 10% - 1000%
        gFluxWeak.AdjustLimit = (long)maxCur * 100L / gFluxWeak.CoefAdj;
        
        gFluxWeak.AdjustId += (gOutVolt.MaxOutVolt - gFluxWeak.VoltLpf) / 4;
        gFluxWeak.AdjustId = __IQsat(gFluxWeak.AdjustId, gFluxWeak.AdjustLimit, -gFluxWeak.AdjustLimit);

        temp1 = (long)gFluxWeak.AdjustId * gFluxWeak.CoefAdj / (100L);    // ��2ms��05ms�����ּ���4��
        gFluxWeak.IdSet += temp1;
        gFluxWeak.IdSet = __IQsat(gFluxWeak.IdSet, 0, -maxCur);
    }

    // ѡ�����ŵ���
    gIMTSet.M = (gFluxWeak.Mode) ? ((long)gFluxWeak.IdSet << 12) : 0;
    return;
}

// ͬ����������㣬����������ת�綯��
void PmDecoupleDeal()
{
    long temp;
    
    gPmDecoup.Omeg = Filter2(gMainCmd.FreqSyn, gPmDecoup.Omeg);
    gPmDecoup.Isd  = Filter2((gIMTQ24.M>>12), gPmDecoup.Isd);
    gPmDecoup.Isq  = Filter2((gIMTQ24.T>>12), gPmDecoup.Isq);

    temp = (long)gPmDecoup.Isd * gMotorExtPer.LD >> 9;                        // Q12
    gPmDecoup.PhiSd = temp + gMotorExtPer.FluxRotor;                          // Q12
    gPmDecoup.RotVq = (long)gPmDecoup.Omeg * gPmDecoup.PhiSd >> 15;        // Q12

    gPmDecoup.PhiSq = (long)gPmDecoup.Isq * gMotorExtPer.LQ >> 9;            // Q12
    gPmDecoup.RotVd = - (long)gPmDecoup.Omeg * gPmDecoup.PhiSq >> 15;      // Q12
}

