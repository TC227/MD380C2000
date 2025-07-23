/****************************************************************
�ļ����ܣ�ʸ�����Ƶ���Ҫ���򲿷�
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
#include "MotorVCInclude.h"
#include "MotorEncoder.h"
#include "MotorPmsmParEst.h"
#include "MotorPmsmMain.h"

// // ȫ�ֱ�������
VC_INFO_STRUCT			gVCPar;			//VC����
MT_STRUCT_Q24           gIMTSet;        //MT��ϵ�µ��趨������Q24��ʾ
MT_STRUCT_Q24           gIMTSetApply;	//MT��ϵ�µĵ���ָ��ֵ
MT_STRUCT				gUMTSet;		//MT��ϵ�µ��趨��ѹ
AMPTHETA_STRUCT			gUAmpTheta;		//���������趨��ѹ

PID32_STRUCT        gImAcrQ24;
PID32_STRUCT        gItAcrQ24;

ASR_STRUCT				gAsr;			//�ٶȻ�
VC_CSR_PARA             gVcCsrPara;  //�ջ�ʸ�������������õ��Ĳ���
PID_STRUCT              gIMSetAcr;   //�������ŵ�������ֵ
PID_STRUCT              gWspid;      //�첽���ջ�ʸ���ٶȻ�ת�������������
//MODIFYWS_STRUCT         gModifyws;   //�첽���ջ�ʸ��ת����������
MT_STRUCT               gPWMVAlphBeta;//�������PWM������alfa��beta���ѹ
UDC_LIMIT_IT_STRUCT     gUdcLimitIt;
PID_STRUCT              gSvcVoltAdj;    // SVC �����ѹ����
ALPHABETA_STRUCT        gABVoltSet;		//�����������¶���ʵ�������ѹ
SVC0_STRUCT             SVC0Signal;
// // �ļ��ڲ��������� 
void PrepareAsrPar(void);
void ResetAsrPar(void);
void ResetCsrPar(void);
void VcAsrControl(void);
void CalTorqueLimitPar(void);
void CalWsAndSynFreq(void);
void CalUdcLimitIT(void);

/*************************************************************
	ʸ�����Ƶ����ݸ�λ������ͣ���׶�ִ��
*************************************************************/
void ResetParForVC(void)
{
    if( gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL )
    {       //Ӧ���ȳ�ʼ��SVC�Ĳ�������Ϊ��������ʼ�������á�
	    ResetSVC();
    }
	ResetAsrPar();
	ResetCsrPar();
}

/*************************************************************
	ʸ�����Ƶ��ٶȻ�������λ
*************************************************************/
void ResetAsrPar(void)
{
	gAsr.Asr.Total = 0;
    gAsr.TorqueLimitPid.Total = 0;    
    gWspid.Total = 0;
	gIMTSet.T = 0;
	gIMTSet.M = 0;
}

/*************************************************************
	ʸ�����Ƶĵ�������λ
*************************************************************/
void ResetCsrPar(void)
{
    //int m_AbsFreq;
    //int m_ITAcrTotal;

	gIMTSetApply.M = 0;
	gIMTSetApply.T = 0;
	
	gUMTSet.M = 0;
	gUMTSet.T = 0;

    if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
    {
        gIMSetAcr.Total = 0;
#if 0
		//gItAcrQ24.Total = 0;
        m_ITAcrTotal = (((long)gMainCmd.FreqFeed << 12) / gMotorInfo.FreqPer); //����ͬ��������������ʱ�񵴺͹���
        gItAcrQ24.Total = (long)m_ITAcrTotal << 12;                 //total�������� 
#else
        gImAcrQ24.Total = (long)gPmDecoup.RotVd << 12;
        gItAcrQ24.Total = (long)gPmDecoup.RotVq << 12;
#endif
    }
    else
    {
        gIMSetAcr.Total = ((((long)gMotorExtPer.IoVsFreq)<<12)/gMotorInfo.Current)<<15;  //Ӧ������Ϊ����ֵ���ӿ�����ʱ�Ĵų�����

        gImAcrQ24.Total = 0;
        gItAcrQ24.Total = 0;
    }

    gVcCsrPara.ImModify = 0;
	gUAmpTheta.Amp   = 0;
	gUAmpTheta.Theta = 0;
    gOutVolt.VoltApply = 0;
	gOutVolt.VoltPhaseApply = 0;
    //���Ϻ�����
    gMainCmd.FreqSyn = gMainCmd.FreqFeed;
}

/*************************************************************
	ʸ�����������ŵ������㣬---- �첽�����Ŵ���
*************************************************************/
void CalIMSet(void)
{
	Uint    Im_AbsFreq,m_AbsFreq,tempU;
    int     m_TempVar;    
	Uint 	m_UData2,m_UData3;  
    Ulong   m_MaxI0Freq,m_Im;
    Ulong   m_HigFreq,m_LowFreq;

    Im_AbsFreq = ((Ulong)(abs(gMainCmd.FreqSyn)) * gBasePar.FullFreq01) >>15; 
    m_HigFreq    = __IQsat(gVFPar.SVCTorqueUpLim,gMotorInfo.Motor_HFreq,0);
    m_LowFreq   = gVFPar.SVCTorqueUpLim - 1000;
    m_LowFreq    = __IQsat(m_LowFreq, gMotorInfo.Motor_LFreq, 0);

    if(1 == gMainCmd.Command.bit.PreExcFlux)
    {
        gIMTSet.M = (long)gComPar.StartDCBrakeCur * 167772L;  // 2^12*2^12/100
        return;
    }  
    m_AbsFreq = abs(gMainCmd.FreqSyn);

    if(m_AbsFreq < gMotorInfo.FreqPer)
      {
        m_MaxI0Freq = gMotorExtPer.I0;
      }
    else
     {
        m_AbsFreq = ((long)( abs(gMainCmd.FreqSyn) - gMotorInfo.FreqPer) * (long)gMotorExtPer.FluxLeakCurveGain/100L) +gMotorInfo.FreqPer;
        m_MaxI0Freq = ((long)gMotorExtPer.I0 * (long)gMotorInfo.FreqPer) / m_AbsFreq ;
       
      }


    
    if(0 != gVFPar.SVCTorqueUp)
	 {
        m_Im= (Ulong)((Ulong)m_MaxI0Freq *(1024L + (Ulong)gVFPar.SVCTorqueUp) >>10);

        if(Im_AbsFreq < m_LowFreq)
        {
            gIMTSet.M = m_Im <<12;
        }  
		else if((Im_AbsFreq >= m_LowFreq)&&(Im_AbsFreq < m_HigFreq))
		{
            m_Im = m_Im - (Im_AbsFreq-m_LowFreq)*(m_Im-m_MaxI0Freq)/(m_HigFreq-m_LowFreq); 
            gIMTSet.M = m_Im <<12;
		}
		else
		{
		    gIMTSet.M = m_MaxI0Freq <<12;
		}
     }
    else
	 {
	     gIMTSet.M = m_MaxI0Freq <<12;
	 }

    }

/*************************************************************
	ʸ�����Ƶ�ת�ؼ��㣨Ҫ��ת�ص��������ŵ���ʸ���Ͳ�����2��Ƶ������2����
ͬ��������첽�������ת�ؿ��Ƴ���
*************************************************************/
void CalTorqueLimitPar(void)
{
	int m_TorLimit,m_MaxLimit;
	int m_InvIM;
	Ulong m_Long,m_SpeedFilter;

	//m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrForP; //ʹ���û��趨���Ͷ�Ӧ�ĵ���
	m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrent; //ʹ���û��趨���Ͷ�Ӧ�ĵ���
	m_InvIM = ((long)m_InvIM * 1000)>>12;
//	m_Long  = (1800L * 1800L) - ((long)m_InvIM * (long)m_InvIM);    //����������ʸ����С�ڱ�Ƶ�������2��
    if(15 == gInvInfo.InvTypeApply)
    {
      m_Long  = (1700L * 1700L) - ((long)m_InvIM * (long)m_InvIM); /*15KW�����ر���*/
    }
    else
    {
      m_Long  = (1800L * 1800L) - ((long)m_InvIM * (long)m_InvIM);    //����������ʸ����С�ڱ�Ƶ�������1.8��
    }
    m_MaxLimit = qsqrt(m_Long);					
	
    m_TorLimit = (gVCPar.VCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.VCTorqueLim;

    // IM ת�ؿ���ʱ�����������ز���
    
	if ((1 == gMainCmd.Command.bit.TorqueCtl)&&
        (gMotorInfo.MotorType != MOTOR_TYPE_PM)&&
        (abs(gMainCmd.FreqSyn) >= gMotorInfo.FreqPer))
	{
		m_TorLimit = ((long)m_TorLimit * (long)gMotorExtPer.I0) / (gIMTSet.M>>12);
	}											

    if((gMainCmd.Command.bit.ControlMode ==IDC_SVC_CTL) &&        // IM SVC deal
        (gMotorInfo.MotorType != MOTOR_TYPE_PM)&&(0 == gVCPar.SvcMode))
    {
	    gMainCmd.FreqSynFilter = Filter16(gMainCmd.FreqFeed,gMainCmd.FreqSynFilter);
	    m_SpeedFilter = abs(gMainCmd.FreqSynFilter);
	    m_SpeedFilter = ((llong)m_SpeedFilter * (llong)gBasePar.FullFreq01)>>15;
	    if(m_SpeedFilter < 40) 
	    {
		    m_TorLimit = (m_TorLimit > 1200)?1200:m_TorLimit;	
	    }											//SVC 0.4HZ�������120%������ʱת��������

	    m_SpeedFilter = abs(gIMTSet.T>>12);
	    if(m_SpeedFilter >= gAsr.TorqueLimit)
	    {
		    m_SpeedFilter = abs(gIMTQ12.T);
		    if((m_SpeedFilter + 2048) < gAsr.TorqueLimit)
		    {
			    m_TorLimit = (m_TorLimit > 1000)?1000:m_TorLimit;	
		    }
	    }											//��ɢ�����ȴ�ʩ
    }

	m_TorLimit = ((long)m_TorLimit * (long)gInvInfo.InvCurrForP)/gMotorInfo.Current;
    //m_TorLimit = ((long)m_TorLimit * (long)gInvInfo.InvCurrent)/gMotorInfo.Current;
	m_TorLimit = ((long)m_TorLimit<<12)/1000;	//ת��Ϊ����ֵ

	
	if((1 == gMainCmd.Command.bit.TorqueCtl)	//���ת�ؿ��Ƶ�Ƶת������
	    || (IDC_FVC_CTL == gMainCmd.Command.bit.ControlMode))  //||(1 == gVCPar.SvcMode)
	{
		gAsr.TorqueLimit = m_TorLimit;          // PM �� IM����ת�ؿ���T������趨
	}
	else
	{											//��ת�ؿ��ƣ���Ҫ�������ƴ�?
	                                            //rn �����ԣ������Ƽ���û
		gAsr.TorqueLimitPid.Max = m_TorLimit;
    	gAsr.TorqueLimitPid.Min = 2;
    	gAsr.TorqueLimitPid.KP = 1000;
    	gAsr.TorqueLimitPid.KI = 1000;
        gAsr.TorqueLimitPid.QP = 0;
        gAsr.TorqueLimitPid.QI = 0;
    	gAsr.TorqueLimitPid.Deta = m_TorLimit - abs(gIMTSetApply.T>>12);//(int)gLineCur.CurPer;
    	if( gInvInfo.InvTypeApply > 20 )
    	{
       		gAsr.TorqueLimitPid.KP = 100;  //12000;
        	gAsr.TorqueLimitPid.KI = 100;  //12000;
    	}
   		PID( (PID_STRUCT *)&gAsr.TorqueLimitPid);
		gAsr.TorqueLimit = gAsr.TorqueLimitPid.Out>>16;
	}    

}

/*************************************************************
	ʸ�����Ƶ��ٶȻ�����������ת����Ԥ��ת���ò�����
׼������:(����ѡ�����)
 Kp = (Kp_func << 8)* (f_base/10 >>12) << 3 := Kp_func * 100 |(f_base == 80.00Hz)
 Ki = (Kp_func/Ki_func << 10) * (f_base/10 >>12) << 3 := Kp_func/Ki_func * 1600 |(f_base == 80.00Hz)
*************************************************************/
void PrepareAsrPar(void)
{
    int	  m_AbsFreq, m_FreqUp;
    int   m_DetaKP, m_DetaKI, m_DetaFreq;
    int   m_KpLimit;
    long  tempKp;
    long  tempKi;

    // ׼���л�����
	
    if(MOTOR_TYPE_PM ==gMotorInfo.MotorType)
    {
        gAsr.KPHigh = gVCPar.ASRKpHigh<<7;
        gAsr.KPLow  = gVCPar.ASRKpLow<<7;
    }
    else
    {
        gAsr.KPHigh = gVCPar.ASRKpHigh<<8;
	    gAsr.KPLow  = gVCPar.ASRKpLow<<8;
    }
	if((gVCPar.ASRKpHigh>>5) >= gVCPar.ASRTIHigh)
	{
		gAsr.KIHigh = 32767;
	}
	else
	{
       if(MOTOR_TYPE_PM ==gMotorInfo.MotorType)
        {
          gAsr.KIHigh = ((Ulong)gVCPar.ASRKpHigh<<9)/gVCPar.ASRTIHigh;
        }
       else
        {
          gAsr.KIHigh = ((Ulong)gVCPar.ASRKpHigh<<10)/gVCPar.ASRTIHigh;
        }
        
	}

	if((gVCPar.ASRKpLow>>5) >= gVCPar.ASRTILow)
	{
		gAsr.KILow = 32767;
	}
	else
	{ 
	   if(MOTOR_TYPE_PM ==gMotorInfo.MotorType)
        {   
		  gAsr.KILow = ((Ulong)gVCPar.ASRKpLow<<9)/gVCPar.ASRTILow;
        }
       else
        {   
		  gAsr.KILow = ((Ulong)gVCPar.ASRKpLow<<10)/gVCPar.ASRTILow;
        }
	}

	gAsr.SwitchHigh = ((Ulong)gVCPar.ASRSwitchHigh<<15)/gBasePar.FullFreq;
	gAsr.SwitchLow  = ((Ulong)gVCPar.ASRSwitchLow<<15)/gBasePar.FullFreq;
	
    // ѡ�����
	m_DetaKP   = gAsr.KPHigh - gAsr.KPLow;
	m_DetaKI   = gAsr.KIHigh - gAsr.KILow;
    m_AbsFreq  = abs(gMainCmd.FreqSyn);
	if(m_AbsFreq <= gAsr.SwitchLow)
	{
		gAsr.Asr.KP = gAsr.KPLow;
		gAsr.Asr.KI = gAsr.KILow;
	}
	else if(m_AbsFreq >= gAsr.SwitchHigh)
	{
		gAsr.Asr.KP = gAsr.KPHigh;
		gAsr.Asr.KI = gAsr.KIHigh;
	}
	else
	{
		m_FreqUp    = m_AbsFreq - gAsr.SwitchLow;
		m_DetaFreq  = gAsr.SwitchHigh - gAsr.SwitchLow;
		gAsr.Asr.KP = ((long)m_DetaKP * (long)m_FreqUp)/m_DetaFreq + gAsr.KPLow;
		gAsr.Asr.KI = ((long)m_DetaKI * (long)m_FreqUp)/m_DetaFreq + gAsr.KILow;
	}
    
    tempKp = ( (long)gAsr.Asr.KP * (long)gBasePar.FullFreq01 / 10L)>>12;
    tempKi = ( (long)gAsr.Asr.KI * (long)gBasePar.FullFreq01 / 10L)>>12;
    gAsr.Asr.KP = __IQsat(tempKp, 32767, 1);        // ���ǻ����
    gAsr.Asr.KI = __IQsat(tempKi, 32767, 1);
    
// ���ַ���Ĵ���
    m_KpLimit = ((long)abs(gMainCmd.FreqDesired - gMainCmd.FreqFeed) *(long)gAsr.Asr.KP) >>16;
    if(( 64 < m_KpLimit ) &&                            // ���������С�������ַ���
        (0 == gMainCmd.Command.bit.TorqueCtl) &&        // ת�ؿ��Ƶ�ʱ�������ַ���
        (1 == gMainCmd.Command.bit.IntegralDiscrete))
    {
        gAsr.Asr.KI = 0;		
    }
    gAsr.Asr.KD = 0;

    // �첽�����ٶȻ�����Ĵ��� /*�ٶȻ����ڹ����޸�ȡ����ʹ��Ĭ��ֵ3 2010.5.07 L1082
    if(gMotorInfo.MotorType != MOTOR_TYPE_PM)   gAsr.Asr.QP = 3;         // ��λ
    else                                        gAsr.Asr.QP = 2;     // ʮλ
    gAsr.Asr.QI = 3;    // 380�ٶȻ���������Ϊ0.5����320��4�� 
    
}
/*************************************************************
	ʸ�����ƵĹ�ѹ���ƹ��ܣ�ͨ��ĸ�ߵ�ѹ�������ת��(����ת��)�����ֵ
*************************************************************/
void CalUdcLimitIT(void)
{
    if(gVFPar.ovGain != 0)//ʸ���Ĺ�ѹ���ƹ���
    {
        gUdcLimitIt.UDCBakCnt++;
        if(gUdcLimitIt.UDCBakCnt >= 5)
        {
            gUdcLimitIt.UDCBakCnt = 0;
            gUdcLimitIt.UDCDeta = gUDC.uDCFilter - gUdcLimitIt.UDCBak;
            gUdcLimitIt.UDCBak = gUDC.uDCFilter;
            if(gUdcLimitIt.UDCDeta < 200)
            {
                gUdcLimitIt.UDCDeta = 0;
            }
        }
        if(gUDC.uDCFilter < gOvUdc.Limit - 100 - (gVFPar.ovGain<<2) - gUdcLimitIt.UDCDeta)
        {
            gUdcLimitIt.UdcPid.Total = (long)-gIMTQ12.T<<16;
            gUdcLimitIt.UDCLimit = gAsr.TorqueLimit;
        }
        else if((gUDC.uDCFilter < gOvUdc.Limit) && (gUdcLimitIt.FirstOvUdcFlag == 0))
        {
            gUdcLimitIt.FirstOvUdcFlag = 1;
            gUdcLimitIt.UDCLimit = gUdcLimitIt.UdcPid.Total>>16;
        }//��һ�ν����ѹΣ���������ز������Ӽ���
        else
        {
            gUdcLimitIt.UdcPid.KP = 10000 + 200*gVFPar.ovGain;
            gUdcLimitIt.UdcPid.KI = 2000 + 100*gVFPar.ovGain;
            gUdcLimitIt.UdcPid.KD = 0;
            gUdcLimitIt.UdcPid.QP = 0;
            gUdcLimitIt.UdcPid.QI = 0;
            gUdcLimitIt.UdcPid.Max = gAsr.TorqueLimit;
            gUdcLimitIt.UdcPid.Min = 0;
            //gUdcLimitIt.UdcPid.Deta = (gOvUdc.Limit- gUdcLimitIt.UDCDeta) - gUDC.uDCFilter;
            gUdcLimitIt.UdcPid.Deta = gOvUdc.Limit - gUDC.uDCFilter;
            PID((PID_STRUCT *) &gUdcLimitIt.UdcPid);
            gUdcLimitIt.UDCLimit = gUdcLimitIt.UdcPid.Out>>16;
        }
    }
    else
    {
        gUdcLimitIt.UDCLimit = gAsr.TorqueLimit;
        gUdcLimitIt.UdcPid.Total = 0;
        gUdcLimitIt.FirstOvUdcFlag = 0;
    }
}

/*************************************************************
	ʸ�����Ƶ��ٶȻ������ٶȻ�����ִ��
set: gMainCmd.FreqSet��
fbk:gMainCmd.FreqFeed��

*************************************************************/
void VcAsrControl(void)
{
	long m_Long;
	llong    m_LLong;  //add by jxl
    int     m05HzPu;
    int     m03HzPu; 

    m05HzPu = (50L<<15) / gBasePar.FullFreq01;
    m03HzPu = (30L<<15) / gBasePar.FullFreq01;

    if((1 == gMainCmd.Command.bit.PreExcFlux)
        && (MOTOR_TYPE_PM != gMotorInfo.MotorType))  //�첽��Ԥ����ʱ��ת�ظ���Ϊ��
    {
        gIMTSet.T = 0;
        gAsr.Asr.Total = 0;
        return;
    }
    
	gAsr.Asr.Max = gUdcLimitIt.UDCLimit;
	gAsr.Asr.Min = -gAsr.Asr.Max;

	m_Long = (long)gMainCmd.FreqSetApply - (long)gMainCmd.FreqFeed;

//    m_ULong = (((Ulong)gMotorInfo.Frequency<<15)/gBasePar.FullFreq01)>>3;
//	gAsr.Asr.Deta = __IQsat(m_Long, m_ULong, -m_ULong);
    //�����ٶȻ�PI��DetaΪ1/8�Ķת��

	gAsr.Asr.Deta = __IQsat(m_Long, 16383, -16383);
	PID((PID_STRUCT *)&gAsr.Asr);

    if(gCtrMotorType == ASYNC_SVC)
    {
        if(1 == gVCPar.SvcMode)
		{
            if((1 != gMainCmd.Command.bit.TorqueCtl)&&(abs(gMainCmd.FreqSetApply) < m03HzPu &&// 0.3Hz
                abs(gMainCmd.FreqFeed) < m05HzPu))
			{
                gIMTSet.T       = 0;
                gAsr.Asr.Total  = 0;
                gMainCmd.FirstCnt = 0;	
		        gMainCmd.FreqSyn = 0;
                gMainCmd.FreqFeed = 0;				//svc�����ٶ�
			}
			else
			{
                m_LLong = (llong)(gAsr.Asr.Out>>(16-12));       // Q12 -> Q24
                gIMTSet.T = ((llong)m_LLong*(32L-gTestDataReceive.TestData8)
                          + (llong)gIMTSet.T*((llong)gTestDataReceive.TestData8))>>5;       // Q12 -> Q24
			}
		}
		else
		{
            if((abs(gMainCmd.FreqSetApply) < m03HzPu &&// 0.3Hz
                abs(gMainCmd.FreqFeed) < m05HzPu))          // 0.5Hz
            {
                gIMTSet.T       = 0;
                gAsr.Asr.Total  = 0;
                gMainCmd.FirstCnt = 0;	
		        gMainCmd.FreqSyn = 0;
                gMainCmd.FreqFeed = 0;				//svc�����ٶ�
            }
			else
			{
	            gIMTSet.T = gAsr.Asr.Out>>(16-12);
			}
		}
	}
	else
	{
	    gIMTSet.T = gAsr.Asr.Out>>(16-12);
	}



    if( 1 == gMainStatus.StatusWord.bit.OutOff )  //�������
    {
	    ResetParForVC();
    } 
}

/*************************************************************
	ʸ�������¼���ת���ͬ���ٶ�
	ws = ITSet * R2 / (IMSet * L2)
*************************************************************/
void CalWsAndSynFreq(void)
{
	int m_UData,m_FreqWs;  //ת��Ƶ��
	int m_ImSet;
    int m_WsOut = 0;
    int	m_Angle,m_Sin,m_Cos;

    if(gMainCmd.Command.bit.PreExcFlux == 1)
    {
        gMainCmd.FreqWs = 0;
    	gMainCmd.FreqSyn = gMainCmd.FreqFeed;
        return;
    }
    m_ImSet = (gIMTSet.M>>12);
    m_UData = ((long)gMotorExtPer.R2<<11)/m_ImSet;
    m_UData = ((long)m_UData<<11)/gMotorExtPer.L1;
    m_UData = ((long)m_UData * (long)(gIMTSet.T>>12))>>14;          // ����ط��õ���T���趨����
    m_FreqWs = ((long)m_UData * (long)gVCPar.VCWsCoff)/100 ;
    
    gMainCmd.FreqWs = m_FreqWs;
    gMainCmd.FreqSyn = gMainCmd.FreqFeed + gMainCmd.FreqWs;
}

//��̬����������Ļ����������
void CalWsAndSynFreq_380(void)
{
	int m_UData,m_FreqWs;  //ת�Ƶ��
	int m_ImSet;
    int m_WsOut = 0;
    int	m_Angle,m_Sin,m_Cos;
	long m_Long;

    if(gMainCmd.Command.bit.PreExcFlux == 1)
    {
        gMainCmd.FreqWs = 0;
    	gMainCmd.FreqSyn = gMainCmd.FreqFeed;
        return;
    }
 
    m_ImSet = (gIMTSetApply.M>>12);  //�첽��SVC���������ŵ�����ʹ��ʵ�������趨ֵ

    m_Long = (long)(gMotorExtPer.R2) * (long)(gIMTSet.T>>12);  //ԭ380����ΪIO
    //m_Long(Q28) = (long)gMotorExtPer.R2(Q16) * ((long)gIMTSet.T(Q24)>>12);
	m_Long = (long)m_Long / ((long)m_ImSet);
	//m_Long(Q16) = (long)m_Long(Q28) / ((long)m_ImSet(Q12));
	m_Long = (long)(m_Long<<8) / (long)gMotorExtPer.L1;
	//m_Long(Q15) = ((long)m_Long(Q16)<<8) * (long)gMotorExtPer.L1(Q9);
    m_FreqWs = ((long)m_Long * (long)gVCPar.VCWsCoff)/100L;
    gMainCmd.FreqWs = m_FreqWs;
    gMainCmd.FreqSyn = gMainCmd.FreqFeed + gMainCmd.FreqWs;
}

/*************************************************************
    �첽������������׼��
??�Ժ�����ڴ����ͬ�������첽���ĵ���������������
*************************************************************/
void PrepImCsrPara()
{  


    // ���ø����ز�����csr������������Ҫ������֤
    //gImAcrQ24.KP = gVCPar.AcrImKp; 
    gImAcrQ24.KI= gVCPar.AcrImKi;
    //gItAcrQ24.KP = gVCPar.AcrItKp;
    gItAcrQ24.KI = gVCPar.AcrItKi;
    gImAcrQ24.KP = (long)gVCPar.AcrImKp * gBasePar.FcSetApply / 80;
    gItAcrQ24.KP = (long)gVCPar.AcrItKp * gBasePar.FcSetApply / 80;
    /*if(20 < gInvInfo.InvTypeApply )
    {
        ImKp = 1020L;
        ItKp = 1020L;
        ImKi = 1280/3;         //�ж���ִ�У��ʵ���СKI���൱��6K ��Ƶ
        ItKi = 1281/3;
    }*/
    //gImAcrQ24.KP = ImKp;
    //gImAcrQ24.KI = ImKi;
    //gItAcrQ24.KP = ItKp;
    //gItAcrQ24.KI = ItKi;
}

#if 0
void PrepareCsrPara()
{  
    long    ImKp, ImKi, ItKp, ItKi, temp;
    int     Nf;
    int     sGain;  // ���ֵ�������
    
    if(gMotorInfo.MotorType != MOTOR_TYPE_PM)       // motor-IM
    {
        //gVCPar.AcrImKp = __IQsat(gVCPar.AcrImKp, 32767, -32767);
        //gVCPar.AcrItKp = __IQsat(gVCPar.AcrItKp, 32767, -32767);

        // ���ø����ز�����csr������������Ҫ������֤
        //gImAcrQ24.KP = gVCPar.AcrImKp; 
        gImAcrQ24.KI= gVCPar.AcrImKi;
        //gItAcrQ24.KP = gVCPar.AcrItKp;
        gItAcrQ24.KI = gVCPar.AcrItKi;
        gImAcrQ24.KP = (long)gVCPar.AcrImKp * gBasePar.FcSetApply / 80;
        gItAcrQ24.KP = (long)gVCPar.AcrItKp * gBasePar.FcSetApply / 80;
        /*if(20 < gInvInfo.InvTypeApply )
        {
            ImKp = 1020L;
            ItKp = 1020L;
            ImKi = 1280/3;         //�ж���ִ�У��ʵ���СKI���൱��6K ��Ƶ
            ItKi = 1281/3;
        }*/
        //gImAcrQ24.KP = ImKp;
        //gImAcrQ24.KI = ImKi;
        //gItAcrQ24.KP = ItKp;
        //gItAcrQ24.KI = ItKi;
    }
    else                        // motor-PM
    {        
    // ͬ���������ز���������������
    	ImKp = (long)gVCPar.AcrImKp * gBasePar.FcSetApply / 80;
        ImKi = gVCPar.AcrImKi;
        
        ItKp = (long)gVCPar.AcrItKp * gBasePar.FcSetApply / 80;
        ItKi = gVCPar.AcrItKi;

        //gFluxWeak.CsrGainMode = 1;      //d 
        //if(gFluxWeak.CsrGainMode == 0)    // ͬ����������pi���治����         //rt
        if(gFluxWeak.Mode == 0)         // ͬ����ֻҪ���ţ��Ͳ��õ�������������
        {
            gImAcrQ24.KP = ImKp;
            gImAcrQ24.KI = ImKi;
            gItAcrQ24.KP = ItKp;
            gItAcrQ24.KI = ItKi;
            return;
        }
        		
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
        }    
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
}
#endif

/*************************************************************
	FVCʸ�����Ƶĵ�����, ����ͬ�������첽��
*************************************************************/
void VCCsrControl(void)
{
	Ulong 		m_Long;
    long        m_Data;
	long        m_MaxVolt, maxVolt2;
    long        m_DriveState;                   //�綯�ͷ����ж�    
	MT_STRUCT	m_UMT;
    long        tempL;
    
    m_MaxVolt = gOutVolt.MaxOutVolt;
    maxVolt2  = m_MaxVolt + 500;
	//m_MaxVolt = 5079L * (long)gInvInfo.InvVolt / gMotorInfo.Votage; //ʸ�������ѹ����Ϊ��Ƶ�����ѹ��1.24��
	//MT����������Ľ��䴦��
	DINT;
	gIMTSetApply.T = gIMTSet.T;
	gIMTSetApply.M = gIMTSet.M;
    EINT;
    /*
    m_DriveState = (long)gMainCmd.FreqFeed * (long)gIMTSetApply.T; //ͨ�������жϵ綯�ͷ�������״̬
    if(m_DriveState < 0 && gMotorInfo.MotorType != MOTOR_TYPE_PM)       // �첽��������
    {
        if(gUAmpTheta.Amp > (m_MaxVolt + 100))
        {
            gVcCsrPara.ImModify -= gVcCsrPara.ImModefySub;
        }
        else  
        {
            gVcCsrPara.ImModify += gVcCsrPara.ImModefyAdd;
        }
        gVcCsrPara.ImModify = __IQsat(gVcCsrPara.ImModify,0,-1000);
        gIMTSetApply.M += gVcCsrPara.ImModify;
        if(-1000 > gIMTSetApply.M)
        {
            gIMTSetApply.M = -1000; //��ֵ̫����������͵�ѹͻ������
        }
    }
    */
// Csr axis-M
    gImAcrQ24.Max = m_MaxVolt << 12;
    gImAcrQ24.Min = - gImAcrQ24.Max;
    gImAcrQ24.Deta = gIMTSetApply.M - gIMTQ24.M;
    PID32(&gImAcrQ24);
/******************************/
//    m_Data = ((long)gIMTSetQ12.T * (long)gMainCmd.FreqSyn)>>15;
//    m_Data = ((long)gMotorExtPer.L0 * (long)m_Data)>>13;
/*****************************/
    gUMTSet.M = (int)(gImAcrQ24.Out >> 12); //+(int)m_Data;
// Csr axis-T
    gItAcrQ24.Max = m_MaxVolt << 12;
    gItAcrQ24.Min = - gItAcrQ24.Max;
    gItAcrQ24.Deta = gIMTSetApply.T - gIMTQ24.T;
    PID32(&gItAcrQ24);

/******************************/
//	m_Data = ((long)gIMTSetQ12.M * (long)gMainCmd.FreqSyn)>>15 ;
//	m_Data = ((long)m_Data * (long)gMotorExtPer.L1)>>9;
/*****************************/
    gUMTSet.T = (int)(gItAcrQ24.Out >> 12); //+(int)m_Data;   

    // ͬ�����������
    #if 0
    gPmDecoup.EnableDcp = 0;
    if(gPmDecoup.EnableDcp)
    {
        gUMTSet.T += gPmDecoup.RotVq;
        gUMTSet.M += gPmDecoup.RotVd;
    }
    #endif

    
	//���������ѹ��ֵ
	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);

	//����MT���ѹ�нǣ������ͷ�ʽ��        .... 
	if(gUAmpTheta.Amp < maxVolt2)   // + 500))	//rt	
	{
		m_UMT.M = gUMTSet.M;
		m_UMT.T = gUMTSet.T;
		gOutVolt.VoltApply = gUAmpTheta.Amp;
	}
	else    //d
	{
	    if(gMotorInfo.MotorType != MOTOR_TYPE_PM)       // �첽��
        {   
    		m_UMT.M = gUMTSet.M>>2;
    		m_UMT.T = gUMTSet.T>>2;
		    gOutVolt.VoltApply = maxVolt2;//gUAmpTheta.Amp;
        }
        else                     // ͬ����, ��Ҫ�������·���
        {
            long temp;
            long minVq, maxVd;

            //temp = (long)gMotorExtPer.LD * (gIMTSet.M>>12) >> 9;            // Q12;
            //gFluxWeak.FluxD = temp + (long)gMotorExtPer.FluxRotor;          // Q12;
            //minVq = gFluxWeak.FluxD * gFluxWeak.FreqLpf >> 15;                     // Q12;
            //minVq = gFluxWeak.FluxD * gMainCmd.FreqSyn >> 15;         // Q12;
            minVq = gPmDecoup.RotVq;
            minVq = 100;
            
            if(abs(minVq) < m_MaxVolt)
            {
                temp = (long)m_MaxVolt * m_MaxVolt - (long)minVq * minVq;
                maxVd = qsqrt(temp);
            }
            else
            {
                maxVd = 0;
            }

            gFluxWeak.DecoupleMode = 1;     // rt ͬ������ʱ��������
            if(gFluxWeak.DecoupleMode ==1)
            {
                m_UMT.M = (gUMTSet.M > maxVd) ? maxVd : gUMTSet.M;      // ��������d����q�����ת���綯��
            }
            else
            {          
                m_UMT.M = gUMTSet.M;                                    // ��������M���ѹ�� ��������
            }
            
            temp = (long)maxVolt2 * maxVolt2 - (long)m_UMT.M * m_UMT.M;
            temp = __IQsat(temp, 0x7FFFFFFF, 0);
            temp = qsqrt(temp);
            m_UMT.T = (gUMTSet.T > 0) ? temp : (-temp);
        }
        gUMTSet.M = m_UMT.M;
        gUMTSet.T = m_UMT.T;
		gOutVolt.VoltApply = maxVolt2;     // + 500;
	}
    
	gUAmpTheta.Theta = atan(m_UMT.M, m_UMT.T);
	gOutVolt.VoltPhaseApply = gUAmpTheta.Theta;
}

/*************************************************************
	����ʸ���µ��������ƣ�ͨ����ѹ���̼��������ѹ��
var Generate:
1. gOutVolt.VoltApply;         // output voltage amplitude
2. gOutVolt.VoltPhaseApply     // output voltage phase
*************************************************************/
void SvcCalOutVolt(void)
{
	MT_STRUCT	m_UMT;
    long        m_Data;
	int			m_Data1,m_Umt_Kp,m_UMBak,m_UTBak;    
	Ulong		m_Long;
	Uint        m_MaxVolt;
    int         m05HzPu;
	//MT����������Ľ��䴦��
	gIMTSetApply.T = gIMTSet.T;
	gIMTSetApply.M = gIMTSet.M;

    m05HzPu = (50L<<15) / gBasePar.FullFreq01;
	if(gMainCmd.FreqReal < 700)         // 7.00Hz
	{
		SVC0Signal.VoltSVCCalSignal = 1;
	}
	else if(gMainCmd.FreqReal > 1050)  // 10.50Hz
	{
		SVC0Signal.VoltSVCCalSignal = 0;
	}

	m_Data = ((gIMTSetApply.M>>12) * (long)gMainCmd.FreqSyn)>>12;
	m_Data = (m_Data * (long)gMotorExtPer.L1)>>12;
	m_Data1= ((long)gMotorExtPer.R1 * (gIMTSetApply.T>>12))>>16;
	m_UTBak = (int)((long)m_Data1 + m_Data);
	
	m_Data = ((gIMTSetApply.T>>12) * (long)gMainCmd.FreqSyn)>>15;  //���к�©�е�Qֵ��ͬ������λ��Ҳ��ͬ
	m_Data = (m_Data * (long)gMotorExtPer.L0)>>13;
	m_Data1= ((long)gMotorExtPer.R1 * (gIMTSetApply.M>>12))>>16;
	if(SVC0Signal.VoltSVCCalSignal==0)
	{
		m_UMBak = m_Data1;      // ֻ�е���ѹ������
	}
	else
	{
		m_UMBak = m_Data1 - m_Data;
	}

// ;SVC�ĵ�ѹ��������
	m_MaxVolt = 200;        // 18V 
	if(gInvInfo.InvTypeApply > 12)
	{
		m_MaxVolt = 100;
	}
	if(gMainCmd.FreqRealFilt < m_MaxVolt)
	{
		SVC0Signal.VoltSVCCalSignalB = 0;     // �͵�ѹ
	}
	else if(gMainCmd.FreqRealFilt > (m_MaxVolt +120))
	{
		SVC0Signal.VoltSVCCalSignalB = 1;     // �ߵ�ѹ
	}	

    if((gMainStatus.RunStep != STATUS_RUN) || (SVC0Signal.VoltSVCCalSignalB == 1) || (abs(gMainCmd.FreqSetApply) < m05HzPu))
    {
        m_Umt_Kp = 0;  // 0.5HZ����ȡ����Ƶ�µĵ�ѹ������0.5HZ���µ�Ƶת�����������½�����������˷�ɢ����
        gSvcVoltAdj.Total = 0;
        SVC0Signal.VoltSVCCalSignalB = 1;
    }
    else
    {
        gSvcVoltAdj.KP = 1500;
        gSvcVoltAdj.KI = 5000/4;
        gSvcVoltAdj.Max = 4096L;
        gSvcVoltAdj.Min = -(gSvcVoltAdj.Max);
        if(gInvInfo.InvTypeApply >= 18)
        {
            gSvcVoltAdj.KP = 5000;
            gSvcVoltAdj.KI = 1300/4;
        }
        gSvcVoltAdj.Deta = (gIMTSetApply.M - gIMTQ24.M) >> 12;
        PID((PID_STRUCT *)&gSvcVoltAdj);
        m_Umt_Kp = gSvcVoltAdj.Out >> 16;        
    }

	m_Umt_Kp = m_Umt_Kp + 4096;
	gUMTSet.T = ((long)m_UTBak * (long)m_Umt_Kp)>>12;
	gUMTSet.M = ((long)m_UMBak * (long)m_Umt_Kp)>>12;	


    // ** ��ѹ�������

	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));	
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);			//���������ѹ��ֵ
		
	if(gUAmpTheta.Amp < 4916)// 1.2		
	{												//����MT���ѹ�нǣ������ͷ�ʽ��
		m_UMT.M = gUMTSet.M;
		m_UMT.T = gUMTSet.T;
		gOutVolt.VoltApply = gUAmpTheta.Amp;
	}
	else
	{
		m_UMT.M = gUMTSet.M >> 2;         //�Ƿ�Ҫ�޸�ʵ�������MT���ѹ����Ϊ�ڼ���ALFA_BETA���ѹʱ���õ�
		                                  //�����ѹ��ͬ������Ĵų�ͬ����Ҳ�Ͳ�ͬ?
		m_UMT.T = gUMTSet.T >> 2;
		gOutVolt.VoltApply = 4916;
	}
	
	if((gMainStatus.RunStep != STATUS_SPEED_CHECK) && (gMainStatus.RunStep != STATUS_RUN))
	{
		m_UMT.M = 0;				//�����������״̬���������ѹΪ0;
		m_UMT.T = 0;
		gOutVolt.VoltApply = 0;
		gUMTSet.M = 0;
		gUMTSet.T = 0;
	}

	gUAmpTheta.Theta = atan(m_UMT.M, m_UMT.T);
	gOutVolt.VoltPhaseApply = gUAmpTheta.Theta;

}
void SvcCalOutVolt_380(void)
{
	MT_STRUCT	m_UMT;
    long        m_Data;
	long		m_Data1,m_UMBak,m_UTBak;
	//int		    m_Angle,m_Sin,m_Cos;    
	Ulong		m_Long;
	Uint        m_MaxVolt;
//    long        ADSampleTime;

//	ADSampleTime = 25736L / (long)gBasePar.FcSetApply;	//�ز����� PI*2^13 / FC

        gImAcrQ24.KI= gVCPar.AcrImKi;
        //gItAcrQ24.KP = gVCPar.AcrItKp;
        gItAcrQ24.KI = gVCPar.AcrItKi;
        gImAcrQ24.KP = (long)gVCPar.AcrImKp * gBasePar.FcSetApply / 80;
        gItAcrQ24.KP = (long)gVCPar.AcrItKp * gBasePar.FcSetApply / 80;

    
	//MT����������Ľ��䴦��
	gIMTSetApply.T = gIMTSet.T;
	gIMTSetApply.M = gIMTSet.M;

	m_Data = ((long)gIMTSetQ12.M * (long)gMainCmd.FreqSyn)>>15 ;

	m_Data = ((long)m_Data * (long)gMotorExtPer.L1)>>9;

	m_Data1= ((long)gMotorExtPer.R1 * (long)gIMTSetQ12.T)>>16;

	m_UTBak = (int)((long)m_Data1 + (long)m_Data);


    //����T����������ջ�
if(0 == gTestDataReceive.TestData0)
{
    m_Data = (long)gIMTSetQ12.T - (long)gIMTQ12_obs.T;
}
else
{
    m_Data = (long)gIMTSetQ12.T - (long)gIMTQ12.T;
}
    m_Data = ((((long)gMotorExtPer.R1)*(long)gTestDataReceive.TestData5 / 100L) * (long)m_Data)>>16;
    m_UTBak = (int)((long)m_UTBak + (long)m_Data);

/******************************

// Csr axis-T
    gItAcrQ24.Max = m_MaxVolt << 12;
    gItAcrQ24.Min = - gItAcrQ24.Max;
    gItAcrQ24.Deta = gIMTSetApply.T - (gIMTQ12_obs.T<<12);
    PID32(&gItAcrQ24);

//	m_Data = ((long)gIMTSetQ12.M * (long)gMainCmd.FreqSyn)>>15 ;
//	m_Data = ((long)m_Data * (long)gMotorExtPer.L1)>>9;
    m_UTBak = (int)(gItAcrQ24.Out >> 12); //+(int)m_Data;   
*****************************/



    m_Data = ((long)gIMTSetQ12.T * (long)gMainCmd.FreqSyn)>>15;

    m_Data = ((long)gMotorExtPer.L0 * (long)m_Data)>>13;
   
	m_Data1= ((long)gMotorExtPer.R1 * (long)gIMTSetQ12.M)>>16;


	m_UMBak = (int)((long)m_Data1 - (long)m_Data);

if(0 == gTestDataReceive.TestData0)
{
    m_Data = (long)gIMTSetQ12.M - (long)gIMTQ12_obs.M;
}
else
{
    m_Data = (long)gIMTSetQ12.M - (long)gIMTQ12.M;
}
    m_Data = (((long)gMotorExtPer.R1*(long)gTestDataReceive.TestData4 /100L ) * (long)m_Data)>>16;

    m_UMBak = (int)((long)m_UMBak + (long)m_Data);
/******************************
    gImAcrQ24.Max = m_MaxVolt << 12;
    gImAcrQ24.Min = - gImAcrQ24.Max;
    gImAcrQ24.Deta = gIMTSetApply.M - (gIMTQ12_obs.M<<12);
    PID32(&gImAcrQ24);
//    m_Data = ((long)gIMTSetQ12.T * (long)gMainCmd.FreqSyn)>>15;
//    m_Data = ((long)gMotorExtPer.L0 * (long)m_Data)>>13;
    m_UMBak = (int)(gImAcrQ24.Out >> 12); //+(int)m_Data;
*****************************/

	gUMTSet.T = m_UTBak;
	gUMTSet.M = m_UMBak;

		
    // ** �ѹ������?

	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));	
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);			//��������ѹ���?
		
	if(gUAmpTheta.Amp < 4916)// 1.2		
	{												//����MT���ѹ�нǣ������ͷ�ʽ��
		m_UMT.M = gUMTSet.M;
		m_UMT.T = gUMTSet.T;
		gOutVolt.VoltApply = gUAmpTheta.Amp;
	}
	else
	{
		m_UMT.M = gUMTSet.M >> 2;         //�Ƿ�Ҫ�޸�ʵ�������MT���ѹ����Ϊ�ڼ���ALFA_BETA���ѹʱ���õ�
		                                  //�����ѹ��ͬ������Ĵų�ͬ����Ҳ�Ͳ�ͬ?
		m_UMT.T = gUMTSet.T >> 2;
		gOutVolt.VoltApply = 4916;
	}
	
	if((gMainStatus.RunStep != STATUS_SPEED_CHECK) && (gMainStatus.RunStep != STATUS_RUN))
	{
		m_UMT.M = 0;				//�����������״̬���������ѹΪ0;
		m_UMT.T = 0;
		gOutVolt.VoltApply = 0;
		gUMTSet.M = 0;
		gUMTSet.T = 0;
	}

	gUAmpTheta.Theta = atan(m_UMT.M, m_UMT.T);
	gOutVolt.VoltPhaseApply = gUAmpTheta.Theta;

}

/*******************************************/
//���޵�ѹ��������ʵ�ʵ�ѹ��������ʱ����
//���ݸ�����MT���ѹ�͸���ͬ���Ǽ���AlphBeta��ĸ�����ѹ
//����õ���AlphBeta��ĸ�����ѹ�����ڴ�ͨ�۲��㷨
//Input:  gUMTSet
//Output��gABVoltSet
//Used��  gPhase.IMPhase
/******************************************/
void CalcABVolt(void)
{
    int     m_Angle,m_Sin,m_Cos;
    long    m_Long;
    
	// ����������ϵ������ѹ
	m_Angle = (int)(gPhase.IMPhase>>16);
	m_Sin  = qsin(m_Angle);
	m_Cos  = qsin(16384 - m_Angle);
	m_Long = (((long)m_Cos * (long)gUMTSet.M) - 
	           ((long)m_Sin * (long)gUMTSet.T))>>15;
	gABVoltSet.Alph = __IQsat(m_Long,32767,-32767);
	m_Long = (((long)m_Sin * (long)gUMTSet.M) + 
	           ((long)m_Cos * (long)gUMTSet.T))>>15;
	gABVoltSet.Beta = __IQsat(m_Long,32767,-32767);
}


/*************************************************************
	FVCʸ�����Ƶĵ�����, ����ͬ�������첽��
*************************************************************/
void VCCsrControl_380(void)
{
	Ulong 		m_Long;
    long        m_Data;
	long        m_MaxVolt, maxVolt2;
    long        m_DriveState;                   //�綯�ͷ����ж�    
	MT_STRUCT	m_UMT;
    long        tempL;
	long		m_Data1,m_UMBak,m_UTBak;
    
    m_MaxVolt = gOutVolt.MaxOutVolt;
    maxVolt2  = m_MaxVolt + 500;
	//m_MaxVolt = 5079L * (long)gInvInfo.InvVolt / gMotorInfo.Votage; //ʸ�������ѹ����Ϊ��Ƶ�����ѹ��1.24��
	//MT����������Ľ��䴦��
	DINT;
	gIMTSetApply.T = gIMTSet.T;
	gIMTSetApply.M = gIMTSet.M;
    EINT;
 
    m_Data = ((long)gIMTSetQ12.T * (long)gMainCmd.FreqSyn)>>15;
    m_Data = ((long)gMotorExtPer.L0 * (long)m_Data)>>13;
	m_Data1= ((long)gMotorExtPer.R1 * (long)gIMTSetQ12.M)>>16;

	m_UMBak = (int)((long)m_Data1 - (long)m_Data);


	m_Data = ((long)gIMTSetQ12.M * (long)gMainCmd.FreqSyn)>>15 ;
	m_Data = ((long)m_Data * (long)gMotorExtPer.L1)>>9;
	m_Data1= ((long)gMotorExtPer.R1 * (long)gIMTSetQ12.T)>>16;

	m_UTBak = (int)((long)m_Data1 + (long)m_Data);


// Csr axis-M
    gImAcrQ24.Max = m_MaxVolt << 12;
    gImAcrQ24.Min = - gImAcrQ24.Max;
    gImAcrQ24.Deta = gIMTSetApply.M - gIMTQ24_obs.M;
    PID32(&gImAcrQ24);
    gUMTSet.M = (int)(gImAcrQ24.Out >> 12) + m_UMBak; //+(int)m_Data;
// Csr axis-T
    gItAcrQ24.Max = m_MaxVolt << 12;
    gItAcrQ24.Min = - gItAcrQ24.Max;
    gItAcrQ24.Deta = gIMTSetApply.T - gIMTQ24_obs.T;
    PID32(&gItAcrQ24);

    gUMTSet.T = (int)(gItAcrQ24.Out >> 12) + m_UTBak; //+(int)m_Data;   

    
	//���������ѹ��ֵ
	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);

	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));	
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);			//��������ѹ���?
		
	if(gUAmpTheta.Amp < 4916)// 1.2		
	{												//����MT���ѹ�нǣ������ͷ�ʽ��
		m_UMT.M = gUMTSet.M;
		m_UMT.T = gUMTSet.T;
		gOutVolt.VoltApply = gUAmpTheta.Amp;
	}
	else
	{
		m_UMT.M = gUMTSet.M >> 2;         //�Ƿ�Ҫ�޸�ʵ�������MT���ѹ����Ϊ�ڼ���ALFA_BETA���ѹʱ���õ�
		                                  //�����ѹ��ͬ������Ĵų�ͬ����Ҳ�Ͳ�ͬ?
		m_UMT.T = gUMTSet.T >> 2;
		gOutVolt.VoltApply = 4916;
	}
	
	if((gMainStatus.RunStep != STATUS_SPEED_CHECK) && (gMainStatus.RunStep != STATUS_RUN))
	{
		m_UMT.M = 0;				//�����������״̬���������ѹΪ0;
		m_UMT.T = 0;
		gOutVolt.VoltApply = 0;
		gUMTSet.M = 0;
		gUMTSet.T = 0;
	}

	gUAmpTheta.Theta = atan(m_UMT.M, m_UMT.T);
	gOutVolt.VoltPhaseApply = gUAmpTheta.Theta;
}
