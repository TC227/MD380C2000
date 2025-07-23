/****************************************************************
�ļ����ܣ�VF������س��򣬰���VF���߼��㡢ת��������ת����������񵴵ȴ���
�ļ��汾�� 
���¸��£� 
	
*************************************************************/

#include "MotorVFInclude.h"
#include "MotorInclude.h"

// ȫ�ֱ�������
VF_INFO_STRUCT			gVFPar;		//VF����
VF_AUTO_TORQUEBOOST_VAR gVFAutoVar;

VF_WS_COMP_STRUCT		gWsComp;
VAR_AVR_STRUCT			gVarAvr;
//VF_CURRENT_CONTROL      gVfCsr;

OVER_SHOCK_STRUCT		gVfOsc;	//�������ýṹ����
THREE_ORDER_FILTER      gOscAmp;

// Vf ʧ�ٿ�����ر���
VF_OVER_UDC_DAMP        gVfOverUdc;
OVER_CURRENT_DAMP2	    gOvCur2;
VF_FREQ_DEAL            gVfFreq2;

OVER_CURRENT_DAMP	    gOvCur;
OVER_UDC_CTL_STRUCT		gOvUdc;

HVF_OSC_DAMP_STRUCT     gHVfOscDamp;
HVF_OSC_JUDGE_INDEX     gHVfOscIndex;           // HVf ��ϵ���ļ���
HVF_DB_COMP_OPT         gHVfDeadBandCompOpt; 
MT_STRUCT				gHVfCur;

// �ļ��ڲ���������
void CalTorqueUp(void);
void ResDropComp(MT_STRUCT * );

int VfOverCurDeal2(int step);
int VfOverUdcDeal2(int step);

void VFOVUdcLimit(void);

/************************************************************
��������:��
�������:��
����λ��:�ϵ��ʼ��������֮ǰ
��������:��
��������:��ʼ���������õ��ı���
************************************************************/
void VfVarInitiate(void)  //VF���б�����ʼ������
{
    gVFPar.FreqApply = 0;
    gVfFreq2.freqSet = 0;
    
	if(gMainCmd.FreqDesired > 0)		//����Ŀ��Ƶ�ʷ���ȷ����ʼ��λ��
	{
		gOutVolt.VoltPhaseApply = 16384;
	}
	else
	{
		gOutVolt.VoltPhaseApply = -16384;
	}		
	gPhase.OutPhase = (int)(gPhase.IMPhase>>16) + gOutVolt.VoltPhaseApply;

//VF�Զ�ת������������ʼ��
	gVFAutoVar.VfCurrentIs = 0; 
	gVFAutoVar.VfReverseAngle = 0;
    gVFAutoVar.AutoTorquePID.Total = 0;
    gVFAutoVar.AutoTorquePID.Out   = 0;
    gVFAutoVar.VfReverseVolt = 0;
    gVFAutoVar.VfTorqueEnableTime = 0;
    gWsComp.Coff = gVFPar.VFWsComp;
    gWsComp.DelayTime = 0;
    gWsComp.WsCompMT.M = 0;
    gWsComp.WsCompMT.T = 0;
    gWsComp.WsCompMTApply.M = 0;
    gWsComp.CompFreq = 0;
    gVFAutoVar.DestinationVolt = 0;
    gOutVolt.detVfVoltUp = 0;
    gVFAutoVar.VfAutoTorqueEnable = 0;


    gOscAmp.InData1 =   gIMTQ12.M;
    gOscAmp.Indata2 =   gIMTQ12.M;
    gOscAmp.Indata3 =   gIMTQ12.M;   
    gOscAmp.OutData1 =  gIMTQ12.M;
    gOscAmp.OutData2 =  gIMTQ12.M;
    gOscAmp.OutData3 =  gIMTQ12.M;

    gVfOsc.TimesNub = 0;
    gVfOsc.IO = 0;
    gVfOsc.ShockDecrease = 0;

    gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 
}

/************************************************************
	�����г�������ѹ����
************************************************************/
void VFOVUdcLimit(void)
{
	int	m_Mid;

	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	
	if((gUDC.uDCFilter > m_Mid) && (gOvUdc.StepApply <= 0) )
	{
		gOvUdc.OvUdcLimitTime++;
		if(gOvUdc.OvUdcLimitTime > 10000)
		{
			gOvUdc.OvUdcLimitTime = 10000;
		}
	}
	else
	{
		gOvUdc.OvUdcLimitTime = 0;
	}
}

/************************************************************
��������:��ôֵƵ��Q15,���VFʹ��Ƶ��ʵ��ֵ
�������:�����ѹQ12,
����λ��:����״̬��2msѭ��
��������:
��������:���ݸ���Ƶ�ʺ�VF���ߣ����������ѹ
************************************************************/
int CalOutVotInVFStatus(int freq)
{
	Uint m_Freq2,m_MotorFreq2;
	int  m_AbsFreq;
    int  mVolt;
	long m_DetaFreq,m_LowFreq,m_HighFreq,m_LowVolt,m_HighVolt;
    long m_VFLineFreq1,m_VFlineFreq2,m_VFLineFreq3,m_Frequency;
    
	m_AbsFreq = abs(freq);

	//gOutVolt.VoltPhase = 0;    
    gVFAutoVar.DestinationVolt = 0;

//...��ת������

// ����VF����
	if((gVFPar.VFLineType == 0) || (gMainStatus.RunStep == STATUS_SPEED_CHECK))
	{
		mVolt = ((Ulong)m_AbsFreq * 4096L)/gMotorInfo.FreqPer;
	}
//���VF���ߣ� ����ת������
	else if(gVFPar.VFLineType == 1)	
	{
	    m_VFLineFreq1 = (long)gVFPar.VFLineFreq1 * (long)gMainCmd.pu2siCoeff;
        m_VFlineFreq2 = (long)gVFPar.VFLineFreq2 * (long)gMainCmd.pu2siCoeff;
        m_VFLineFreq3 = (long)gVFPar.VFLineFreq3 * (long)gMainCmd.pu2siCoeff;
        m_Frequency   = (long)gMotorInfo.Frequency * (long)gMainCmd.pu2siCoeff;    

		if(gMainCmd.FreqReal < m_VFLineFreq1)
		{
			m_LowFreq  = 0;	
			m_HighFreq = m_VFLineFreq1;
			m_LowVolt  = gVFPar.VFTorqueUp;	
			m_HighVolt = gVFPar.VFLineVolt1;
		}
		else if(gMainCmd.FreqReal < m_VFlineFreq2)
		{
			m_LowFreq  = m_VFLineFreq1;
			m_HighFreq = m_VFlineFreq2;
			m_LowVolt  = gVFPar.VFLineVolt1;
			m_HighVolt = gVFPar.VFLineVolt2;
		}
		else if(gMainCmd.FreqReal < m_VFLineFreq3)
		{
			m_LowFreq  = m_VFlineFreq2;
			m_HighFreq = m_VFLineFreq3;
			m_LowVolt  = gVFPar.VFLineVolt2;
			m_HighVolt = gVFPar.VFLineVolt3;
		}
		else
		{
			m_LowFreq  = m_VFLineFreq3;
			m_HighFreq = m_Frequency;//���VF���ߵ����һ���Ե��������β
			m_LowVolt  = gVFPar.VFLineVolt3;
			m_HighVolt = 1000;
		}
		m_DetaFreq = gMainCmd.FreqReal - m_LowFreq;
		m_HighFreq = m_HighFreq - m_LowFreq;
		m_HighVolt = m_HighVolt - m_LowVolt;
		m_LowVolt  = (((long)m_LowVolt)<<12)/1000;
		m_HighVolt = (((long)m_HighVolt)<<12)/1000;	//�öεĵ�ѹ��Χ

		mVolt = (((long)m_HighVolt * (long)m_DetaFreq)/m_HighFreq) + m_LowVolt;
        return (mVolt);     // ���vf����ת������
	}	
//ƽ��VF����
	else if(gVFPar.VFLineType == 2)				
	{
		m_Freq2 = ((Ulong)m_AbsFreq * (Ulong)m_AbsFreq)>>15;
		m_MotorFreq2  = ((Ulong)gMotorInfo.FreqPer * (Ulong)gMotorInfo.FreqPer)>>15;
        
		mVolt = ((Ulong)m_Freq2 * 4096L)/m_MotorFreq2;
	}
// Сƽ��Vf����
    else if((gVFPar.VFLineType >= 3) && (gVFPar.VFLineType <= 8)) // 2, 3, 4, 5
    {
        Uint posFt, posEnd;
        Uint voltFt, voltEnd;
        Uint16 ptV;
        Uint detFrq;

        ptV = gVFPar.VFLineType - 3;
        if(ptV == 3) ptV = 2;   // 1.6
        if(ptV == 5) ptV = 3;   // 1.8
        
        //����������ϵķ�ʽ����ָ�����ߣ���0~~1��Ϊ128��
        m_Freq2 = (Ulong)m_AbsFreq * 4096 / gMotorInfo.FreqPer;
        
        posFt = m_Freq2>>5;                 //4096�ֳ�128�Σ�ÿ����ֵΪ32
        posFt = (posFt < 127) ? posFt : 127;

        posEnd = posFt + 1;
        detFrq = m_Freq2 - (posFt<<5);
        
        voltFt  = gExponentVf[ptV][posFt];    // 2����1.2�η�����
        voltEnd = gExponentVf[ptV][posEnd];
        
        mVolt = ((detFrq * (voltEnd - voltFt)) >> 5) + voltFt;
    }
// VF�����ѹ����
    else if(gVFPar.VFLineType == 10 || gVFPar.VFLineType == 11)
    {
        mVolt = ((Ulong)gOutVolt.vfSplit << 12) / gMotorInfo.Votage;
    }

// ����VF���ͽ���Vf���ߵı߽紦��
    
    if(m_AbsFreq == 0)          // 0Ƶ�������ѹΪ0�� VF����Ҳ�����
	{
		mVolt = 0;
		return (mVolt);		
	}
    else if(m_AbsFreq >= gMotorInfo.FreqPer)
	{
	    if(gVFPar.VFLineType < 10)                  // ������ƺ㹦����
        {   
		    mVolt = (mVolt > 4096) ? 4096 : mVolt;  
        }
        else// ��ȻVf�����ʱ��������
        {
            mVolt = (mVolt < gOutVolt.MaxOutVolt) ? mVolt : gOutVolt.MaxOutVolt;
        }                        
		return (mVolt);		
	}	
    
    //ת����������
	//CalTorqueUp();		// generate gOutVolt.detVfVoltUp		
	//mVolt += gOutVolt.detVfVoltUp;
	return (mVolt);
}

/*************************************************************
	ת������������I*R����������ѹ��������ѹ��VF�����ѹʸ����
*************************************************************/
void CalTorqueUp(void)
{
	int m_DetaFreq, m_VoltUp, m_ZeroUp, m_MaxFreqUp;
    
// some case Vf_curve needn't torque up compensation
    if((gVFPar.VFLineType == 1) || (gVFPar.VFLineType >= 10) ||
        //(gMainStatus.RunStep == STATUS_SPEED_CHECK) ||
        (gMainStatus.RunStep == STATUS_GET_PAR))
    {
        gOutVolt.detVfVoltUp = 0;
    }
    else
    {
    m_VoltUp = 0;
	m_ZeroUp = (((Ulong)gVFPar.VFTorqueUp)<<12)/1000; //0Hzʱ���������ѹ
	m_MaxFreqUp = (((Ulong)gVFPar.VFTorqueUpLim)<<15)/gBasePar.FullFreq; //��ôֵ��ʾ��ת����������Ƶ��   
	m_DetaFreq = m_MaxFreqUp - abs(gMainCmd.FreqSyn);
    if(( m_DetaFreq > 0 ) && ( 0 == gExtendCmd.bit.SpeedSearch )) //ת�ٸ���ʱ����Զ�ת�����?
    {
	    if((gVFPar.VFTorqueUp == 0)&&(gVFPar.VFOvShock == 0))		//�Զ�ת������,������������ʱ��Ч 2011.5.14 L1082
	    {
            if( gMainCmd.FreqDesiredReal > 40 )  //�趨Ƶ�ʵ���0.4HZ��ת��������Ч
			{
                gVFAutoVar.DestinationVolt = gOutVolt.Volt;
    	    	m_VoltUp = (int)(gVFAutoVar.AutoTorquePID.Out >>16);  //��ѹ�����ͺ�0.5ms���Ƿ����Ӱ���?
			}
            else
            {
                gVFAutoVar.VfTorqueEnableTime = 0;
                m_VoltUp = 0;
            }
	    }
    	else							//�ֶ�ת������
	    {
            m_VoltUp = ((Ulong)m_DetaFreq * (Ulong)m_ZeroUp)/(Uint)m_MaxFreqUp; 
		}
      }  
    gOutVolt.detVfVoltUp = m_VoltUp; 
    }
    gOutVolt.Volt = gOutVolt.Volt + gOutVolt.detVfVoltUp;
}

/************************************************************
     VF�Զ�ת��������ת���������������
************************************************************/
void VFWsTorqueBoostComm(void)
{
    int    m_CurrentPro,m_AngleCos,m_ResistanceVolt;
    int    m_RVcos;
    long   m_lTempVar;
    int    m_iTempVar1,m_iTempVar2;
    MT_STRUCT_Q24  m_NewMT;

    m_CurrentPro = gLineCur.CurPer;//gIAmpTheta.Amp    
    m_iTempVar1  = abs(gIMTQ12.T);
    gVFAutoVar.VfCurrentIs = Filter4(m_CurrentPro, gVFAutoVar.VfCurrentIs);

    if( m_iTempVar1 > gVFAutoVar.VfCurrentIs )
	{
        m_iTempVar1 = gVFAutoVar.VfCurrentIs;
	}
    m_AngleCos = (((long)m_iTempVar1)<<14) / gVFAutoVar.VfCurrentIs;  //���㹦�������ǵ�����
    m_lTempVar =  ( 0x4000L<<14) - (long)m_AngleCos * (long)m_AngleCos;  
    gVFAutoVar.VfAngleSin = qsqrt(m_lTempVar);  
    
    m_ResistanceVolt = ((Ulong)gMotorExtPer.R1 * (Ulong)gVFAutoVar.VfCurrentIs)>>16;  //���㶨�ӵ����ϵ�ѹ��,Q16��ʾ
    m_RVcos = ((long)m_ResistanceVolt * (long)m_AngleCos)>>14;
    gVFAutoVar.VfRIsSinFai = ((long)m_ResistanceVolt * (long)gVFAutoVar.VfAngleSin)>>14;
    gVFAutoVar.VfRVCosFai  = gOutVolt.VoltApply - 20 - m_RVcos; //�����ѹ��ôֵ��ȥ20����Ϊ����
    
    if(0 > gVFAutoVar.VfRVCosFai)
	{
        gVFAutoVar.VfRVCosFai = 0;
    }

    m_iTempVar2 = atan(gVFAutoVar.VfRVCosFai,gVFAutoVar.VfRIsSinFai);    
    gVFAutoVar.VfReverseAngle = Filter128(m_iTempVar2,gVFAutoVar.VfReverseAngle);//�����ѹ������
    m_iTempVar2 = atan( abs(gIMTQ12.M), abs(gIMTQ12.T) );
    if( gVFAutoVar.VfReverseAngle > m_iTempVar2 )
	{
        gVFAutoVar.VfReverseAngle = m_iTempVar2;  //����������Ǵ��ڹ��������ǵ���ǣ���ʹ�ù�����������Ǵ���
	}

    gWsComp.Coff = gVFPar.VFWsComp;
    if( gWsComp.Coff > 1400 )
    {                           //ת����������1400��ֱ����MT���������ת�����
        gWsComp.Coff = gVFPar.VFWsComp - 500;
        gWsComp.WsCompMT.M = abs( gIMTQ12.M );
        gWsComp.WsCompMT.T = abs( gIMTQ12.T );
    }
    else
    {                          //ת����������1400����������ĽǶ����¼���MT�����
        m_iTempVar1 = (int)(gPhase.IMPhase>>16) + gVFAutoVar.VfReverseAngle; 
        if( gMainCmd.FreqSet < 0 )
		{
            m_iTempVar1 = (int)(gPhase.IMPhase>>16) - gVFAutoVar.VfReverseAngle;
        }
        
        AlphBetaToDQ((ALPHABETA_STRUCT *)&gIAlphBeta, m_iTempVar1, (MT_STRUCT_Q24 *)&m_NewMT);
        gWsComp.WsCompMT.T = Filter128( abs(m_NewMT.T>>12), gWsComp.WsCompMT.T);
        if( gLineCur.CurPer < 4096 )	//����������ڶ���������ټ������ŵ���
        {       
            gWsComp.WsCompMT.M = Filter8( abs(m_NewMT.M>>12), gWsComp.WsCompMT.M);            
        }
    }
}

/************************************************************
��������:����VF���߼���������ѹ��Q12;MT����任��ĵ���,Q12
�������:VF��Ƶ��ѹ��������Q12��ʽ
����λ��:����״̬��2msѭ��
��������:
��������:VF�Զ�ת�����������ݵ��������ѹ������
************************************************************/
void VFAutoTorqueBoost(void)
{
    long  m_lTempVar1,m_lVoltDeta;
    int   m_iEsVolt,m_iEsnVolt,m_LowerLimit,m_TempVolt;
    
    m_lTempVar1  = (long)gVFAutoVar.VfRIsSinFai * (long)gVFAutoVar.VfRIsSinFai;
    m_lTempVar1 += (long)gVFAutoVar.VfRVCosFai * (long)gVFAutoVar.VfRVCosFai;
    m_iEsVolt    = qsqrt(m_lTempVar1);   //������������ѹ��ȥ���ӵ���ѹ�������ң��붨�ӵ���ѹ�������ҵ�ƽ����
    m_iEsnVolt   = ((long)gOutVolt.VoltApply * (long)gVFAutoVar.VfAngleSin)>>14;//���������������ѹ���Թ��������ǵ�����   				
    if(15 >= gInvInfo.InvTypeApply)        m_LowerLimit = 50;//���ݻ��ͼ���PI�����������ޣ������𶯻�����
    else if(27 >= gInvInfo.InvTypeApply)   m_LowerLimit = 35;
    else                                     m_LowerLimit = 0;

    m_LowerLimit = (long)(1000 - gMainCmd.FreqReal) * (long)m_LowerLimit / 1000; //�����ֶ���������Ƶ�ʼ����Ӧ���ޣ�����Ƶ����Ϊ��10HZ

    m_TempVolt = m_iEsnVolt;  			//���ݵ�ǰ����Ƶ�ʺ͵��裬�������綯�Ƽ��㷽ʽ
    if(27 < gInvInfo.InvTypeApply) m_TempVolt = m_iEsVolt;  //��������ʱ��һ��������ֵ�ܴ����ױ����� 
    
    if(gMainCmd.FreqReal > 150)         //  1.5Hz����, ����Ϊ�㣻
    {
        m_LowerLimit = 0;				
        m_TempVolt = m_iEsVolt;

        if(gMainCmd.FreqReal < 2000)	//����1.5HZ,С��20HZ
        {       
            m_TempVolt = m_iEsnVolt;
            if(gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff))//���ӵ���С��2ŷķ
            {   
                m_TempVolt = m_iEsVolt;
                if(gMainCmd.FreqReal < 950)//���ӵ���С��2ŷķ������Ƶ�ʵ���9.5Hz
                {   
                    m_TempVolt = (m_iEsnVolt + m_iEsVolt) >>1;
                }            
            }
        }
    }
    
    if((gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff)) && (350 > gVFAutoVar.VfTorqueEnableTime))             
    {      
        gVFAutoVar.VfTorqueEnableTime++;
        m_TempVolt = m_iEsVolt;
    }
    m_lVoltDeta = ((long)(gVFAutoVar.DestinationVolt - m_TempVolt))<<5; 	// ���⾫����ʧ����Q5��ʾƫ��
    m_TempVolt = __IQsat(m_lVoltDeta,32767,-32767);
    gVFAutoVar.VfReverseVolt = Filter32(m_TempVolt, gVFAutoVar.VfReverseVolt);
    
    if(19 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = (long)gVFAutoVar.VfReverseVolt>>5;
    }
    else if(13 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>3) & 0xFFFC;
    }
    else
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>1) & 0xFFF0;
    }
    gVFAutoVar.AutoTorquePID.Max  = 1228 ; 			//30% Torque Boost
    gVFAutoVar.AutoTorquePID.Min  = m_LowerLimit;
    gVFAutoVar.AutoTorquePID.KP   = 56;  		//PID����ʵ�������ϰ汾������ͬ
    gVFAutoVar.AutoTorquePID.KI   = 49;
    gVFAutoVar.AutoTorquePID.QP = 0;
    gVFAutoVar.AutoTorquePID.QI = 0;
    PID((PID_STRUCT *)&gVFAutoVar.AutoTorquePID); 
 }


/************************************************************
	�����񵴴���
************************************************************/
/*void OverShockControl(void)
{
    int m_DelayTime,m_DestCurrent;
    int gainOsc;

    gainOsc = gVFPar.VFOvShock;
    gVfOsc.IMFilter = gIMTQ12.M;
    
    gVfOsc.TimesNub++;
    if( 2000 < gVfOsc.TimesNub )
    {
        gVfOsc.TimesNub = 2001;
    }

//������ģʽΪ0��2ʱ��DPWM����ʱȡ��������
    if((gVfOsc.oscMode == 0) && (MODLE_DPWM == gPWM.PWMModle)) 
    {
        gVfOsc.ShockDecrease++;
        if(800 < gVfOsc.ShockDecrease)
        {
            gVfOsc.ShockDecrease = 808;
        }
        
        m_DelayTime = gVfOsc.ShockDecrease>>3;
        
        if(gVFPar.VFOvShock <= m_DelayTime)
        {
            gainOsc = 0;
        }
        else
        {
            gainOsc = gVFPar.VFOvShock - m_DelayTime;
        }
    }
    else
    {
        gVfOsc.ShockDecrease = 0;
    }

// ����������
	if((gainOsc == 0) || (gMainCmd.FreqReal <= 50) )//|| speed_DEC) // dec    // 0.50Hz
	{										
		gVfOsc.pid.Out -= gVfOsc.pid.Out>>5;
		gVfOsc.pid.Total = 0;
		return;
	}
    gVfOsc.pid.KP   = (gainOsc<<3);     // 280xp�汾������7λ�� ����PID������������4λ
	gVfOsc.pid.KI   = 0;			    //û�л���
	gVfOsc.pid.KD   = 0;
    gVfOsc.pid.QP = 0;
    gVfOsc.pid.QI = 0;
	gVfOsc.pid.Max  = (gainOsc<<2)+100;
	gVfOsc.pid.Min  = -gVfOsc.pid.Max;
    if(gVfOsc.TimesNub < 1500) //������ʵ�������ʱ����ʼ��Ƕλ����
    {
        gVfOsc.IO = Filter256(gMotorExtPer.I0, gVfOsc.IO);
    }
//������ģʽΪ1��2��3ʱ�������趨�Ŀ��ص���
    else// if(( 1500 < gVfOsc.TimesNub ) && (gVfOsc.oscMode >= 1))
    {
        gVfOsc.IO = gMotorExtPer.I0;
    }

	gVfOsc.pid.Deta = gVfOsc.IO - gVfOsc.IMFilter;    
	PID((PID_STRUCT *)&gVfOsc.pid);
    gVfOsc.OscVolt = gVfOsc.pid.Out >> 16; 
	gOutVolt.Volt += gVfOsc.OscVolt;
    if(gVFPar.FreqApply == 0)       // ���Ƶ��Ϊ0, ������ת���
    {
        gOutVolt.Volt  = 0;         // disabel torque-up part and osc-damp part
    }
	gOutVolt.Volt = __IQsat(gOutVolt.Volt, 32767, 0);
    
}
*/
/************************************************************
	ת�������
************************************************************/
void VFWSCompControl(void)
{
    int m_WsWindage, m_WsCurrentIm;

    m_WsWindage = 0;
    if(( gMainCmd.FreqDesiredReal < 40  ) ||
        ( 16 < gInvInfo.InvTypeApply)) 	//�趨Ƶ��С��0.4Hz,���߻��ʹ���16
 	{	
        gWsComp.CompFreq = 0;
        gMainCmd.FreqSyn = gMainCmd.FreqSetApply;
        return;
	}

    if(speed_CON && (0 != gVFPar.VFWsComp)) 
    {
        if( gWsComp.DelayTime > 100 )
        {
            gWsComp.WsCompMTApply.T = abs(gWsComp.WsCompMT.T);

            if(( 0 == gVFPar.VFOvShock ) && ( 20 <= gBasePar.FcSetApply ))
            {
                gWsComp.WsCompMTApply.M = Filter16(gWsComp.WsCompMT.M, gWsComp.WsCompMTApply.M);
                m_WsCurrentIm = abs(gWsComp.WsCompMTApply.M);
            }
            else
    		{
                m_WsCurrentIm = gMotorExtPer.I0;  //��������Ч��������Ƶ����2K�����ŵ���ʹ�ÿ��ص���
            }

            if( 0 == m_WsCurrentIm )	m_WsCurrentIm = 1;
                   
            m_WsWindage = (3038L) * (long)gWsComp.WsCompMTApply.T / (long)m_WsCurrentIm;
            m_WsWindage = (long)m_WsWindage * (long)gWsComp.Coff / 1000L;
            if( m_WsWindage > 0x2000 )	m_WsWindage = 0x2000; 
            if( m_WsWindage < 0 )		m_WsWindage = 0;//��󲹳������ת��
                    
            m_WsWindage = ( (long)m_WsWindage * (long)gMotorExtPer.RatedComp )>>12;
        }
        else	
        {
        	gWsComp.DelayTime++;   //��̬����Ҫ��ʱ0.2s��ʼת��� 
	    }
                      
    }
    else
    {
    	gWsComp.DelayTime = 0;  //�Ӽ��ٹ����У�����ת���
    }

    gWsComp.FilterCoff = 100;    //200ms�˲�
    if(( (gWsComp.CompFreq>>16) + 2 ) < m_WsWindage ) //�˲�����ƫ��Ϊ2
    {
        gWsComp.CompFreq = (( (long)m_WsWindage <<16 ) - gWsComp.CompFreq ) / (long)( gWsComp.FilterCoff + 1 ) + gWsComp.CompFreq;
    }
    else if(( (gWsComp.CompFreq>>16) - 2 ) >m_WsWindage )
    {
       gWsComp.CompFreq = ( (long long)gWsComp.CompFreq - ( (long long)m_WsWindage <<16 )) * (long long)gWsComp.FilterCoff / 
                             (long)( gWsComp.FilterCoff + 1) + ((long)m_WsWindage <<16);
    }
    else
    {
       gWsComp.CompFreq = ((long)m_WsWindage)<<16;
    }
}

/************************************************************
VF��������
	���ٹ����У��Ѿ����������ϵ���󣬸���ĸ�ߵ�ѹ�Ŵ����ϵ��
�������: ����ϵ�� gRatio;
�������: ����ϵ�� gRatio;
************************************************************/
void VFOverMagneticControl()
{
	Uint    m_UDCDesired;
    Uint    m_AVR;
    //Uint    m_Ratio;

    if((gMainStatus.RunStep == STATUS_SPEED_CHECK) ||   // ת��׷�ٲ���Ҫ������
        (gVFPar.VFLineType >= 10))                      // Vf���벻��Ҫ������
    {
        gOutVolt.VoltApply = gOutVolt.Volt;
        return;
    }
    
// ���ٹ����еĹ����Ŵ���
	m_UDCDesired = gInvInfo.BaseUdc;
	if(speed_DEC)
	{
		m_UDCDesired = gUDC.uDC;
	}

	if(abs(gVarAvr.UDCFilter - m_UDCDesired) < 10)
	{
		gVarAvr.UDCFilter = m_UDCDesired;
	}
	else if(gVarAvr.UDCFilter > m_UDCDesired)
	{
		gVarAvr.UDCFilter --;
	}
	else
	{
		gVarAvr.UDCFilter += 3;		
		//gVarAvr.UDCFilter = Filter4(m_UDCDesired, gVarAvr.UDCFilter) + 10;
	}
	gVarAvr.UDCFilter = (gVarAvr.UDCFilter < gInvInfo.BaseUdc)?gInvInfo.BaseUdc:gVarAvr.UDCFilter;
    //gVarAvr.UDCFilter = __IQsat(gVarAvr.UDCFilter, 32767, gInvInfo.BaseUdc);
    
	m_AVR = (((long)gVarAvr.UDCFilter)<<12)/gInvInfo.BaseUdc - 4096;
	m_AVR = ((long)m_AVR * (long)gVarAvr.CoffApply)>>6;
    gOutVolt.VoltApply = ((long)m_AVR + 4096L) * gOutVolt.Volt>> 12;

}

/************************************************************
VF ���ƹ���:
    Ƶ������:   gMainCmd.FreqSetApply
    ��ѹ����:   gOutVolt.Volt
************************************************************/
void VFSpeedControl()
{
    int slipComp;
//    int mVolt;

//����VF���Ƶ�ʣ� ת���
    slipComp = gWsComp.CompFreq >>16;
    slipComp = (gMainCmd.FreqSet > 0) ? slipComp : (-slipComp);
    gMainCmd.FreqSyn = gVFPar.FreqApply + slipComp;
    
//����VF�����ѹ
    gOutVolt.Volt = CalOutVotInVFStatus(gMainCmd.FreqSyn);
    
 
}

    
// ����280xp��ʧ�ٿ���
void VfOverCurDeal()
{
	int  m_Cur,m_DetaCur,m_Coff;
	int	 m_LimHigh,m_LimLow,m_MaxStep,m_AddStep;
	int	 m_Data;

	//������ǰ״̬�ж�
	m_Cur = gLineCur.CurBaseInv;
	m_Coff = gVFPar.ocGain;

// �жϼӼ��ٵ�һ��
gVfFreq2.spedChg = (gVfFreq2.preSpdFlag != gMainCmd.Command.bit.SpeedFlag) ? 1 : 0;
gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 

	//if(0x8000 == (gMainCmd.SpeedFalg & 0x8000))	//�Ӽ��ٱ�־�ı�ĵ�һ��
	if(gVfFreq2.spedChg)
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvCur.Flag = 0;
		gOvCur.StepLowLim = 1;
		gOvCur.StepLow = 1;
		gOvCur.StepHigh = 0;
		//if((m_Coff < 10) && ((gMainCmd.SpeedFalg & 0x7FFF) != C_SPEED_FLAG_CON))
		if((m_Coff < 10) && (!speed_CON))
		{
			gOvCur.StepLowLim = (20 - m_Coff)<<3;
			gOvCur.StepLow = gOvCur.StepLowLim;
		}
	}

	m_DetaCur = m_Cur - gOvCur.CurBak;
	gOvCur.LowFreq = (((long)(m_Coff + 50) * 10)<<15)/gBasePar.FullFreq01;
	if(m_Coff < 20)		
	{
		gOvCur.SubStep = 1;
		m_AddStep = (20 - m_Coff);//>>1;				//��������С��20����С���Ը���
	}
	else
	{
		gOvCur.SubStep = 2;
		m_AddStep = 1;
	}

	if(m_Coff < 40)
	{
		gOvCur.MaxStepLow = 10000 / 6 / m_Coff;
		gOvCur.MaxStepHigh = 10000;
	}
	else
	{
		m_Data = (13200-100 * m_Coff);
		gOvCur.MaxStepLow = m_Data/ 6 / m_Coff + 10;
		gOvCur.MaxStepHigh = m_Data>>4;
	}
    if(15 == gInvInfo.InvTypeApply) /*15KW�������ر������Ƶ���*2011.5.8 L1082*/
     {
       gOvCur.CurLim = ((long)170L <<12) / 100 ;
     }
    else
     {
       gOvCur.CurLim = ((long)gVFPar.ocPoint <<12) / 100 ;
      }
    
	m_LimLow  = ((long)gOvCur.CurLim * 3605L)>>12;
	m_LimHigh = ((long)gOvCur.CurLim * 3891L)>>12;

	if(m_Cur < m_LimLow)					//С��0.88������
	{
		gOvCur.StepHigh = 0;
		if((gOvCur.Flag & 0x01) == 0)
		{
			gOvCur.StepLow += m_AddStep;
			gOvCur.StepLowLim += m_AddStep;
		}
		if(abs(gMainCmd.FreqSyn) < gOvCur.LowFreq)	
		{
			m_MaxStep = gOvCur.MaxStepLow;
		}
		else
		{
			m_MaxStep = gOvCur.MaxStepHigh;
		}
		if(gOvCur.StepLowLim > m_MaxStep)
		{
			gOvCur.StepLowLim -= (gOvCur.StepLowLim - m_MaxStep)>>4;
		}
		//gOvCur.StepLowLim = (gOvCur.StepLowLim > m_MaxStep)?m_MaxStep:gOvCur.StepLowLim;
		gOvCur.StepLow = (gOvCur.StepLow > gOvCur.StepLowLim)?gOvCur.StepLowLim:gOvCur.StepLow;

		gOvCur.StepApply = gOvCur.StepLow;
		gOvCur.Flag &= 0x7FFF;				//����С��0.88�������־
	}
	else if(m_Cur < m_LimHigh)				//0.88��0.95������֮��
	{
		if(m_DetaCur < 0)	
		{
			gOvCur.StepHigh --;
			gOvCur.StepHigh = (gOvCur.StepHigh<-5000)?-5000:gOvCur.StepHigh;
		}
		if(gOvCur.StepHigh >= 0)
		{
			gOvCur.StepLow -= gOvCur.SubStep;
			gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		}
		else
		{
			gOvCur.StepLow = (gOvCur.StepLow < (-gOvCur.StepHigh))?(-gOvCur.StepHigh):gOvCur.StepLow;
		}
		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//��������0.88�������־
	}
	else									//����0.95������
	{
		gOvCur.StepLow -= gOvCur.SubStep;
		gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		if(m_DetaCur < 0)	gOvCur.StepHigh = 0;
		else
		{
			gOvCur.StepHigh += (4 + (m_Coff>>4));
			gOvCur.StepHigh = (gOvCur.StepHigh>10000)?10000:gOvCur.StepHigh;
		}

		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//��������0.88�������־
	}
	gOvCur.CurBak = m_Cur;
	gOvCur.Flag &= 0xFFFE;					//�������̫���־

}

void VfOverUdcDeal()
{
	int m_High,m_Mid,m_Low,m_Coff;
	int	m_Udc,m_DetaU,m_Add1,m_Add2;

	m_Udc = gUDC.uDC;
	m_Coff = gOvUdc.CoffApply;


	
	if((!speed_DEC) ||     // acc or cons
	    (gVfFreq2.spedChg) ||               // 1st step of acc or dec
	    (m_Coff == 0))
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvUdc.StepApply = 0;
        gOvUdc.LastStepApply = 0;
        gOvUdc.AccTimes = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		gOvUdc.ExeCnt = m_Coff;
		gOvUdc.UdcBak = m_Udc;
		gOvUdc.CoffApply = gVFPar.ovGain;
		gOvUdc.FreqMax = gVFPar.FreqApply;
		gOvUdc.OvUdcLimitTime = 0;
		return;
	}

	if(abs(gVFPar.FreqApply) > abs(gOvUdc.FreqMax))
	{
		gVFPar.FreqApply = gOvUdc.FreqMax;
		gOvUdc.StepApply = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		return;
	}	

    
	m_High = gOvUdc.Limit;
	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	//if(gInvInfo.InvTypeApply < MAX_220V_INV)	m_Low = 3522;
	//else										m_Low = 6100;
	m_Low = 6100;

    // if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
    /*if((m_Udc <= m_Mid) || ((gOvUdc.Flag & 0x01) == 0x01))	
    {
        gOvUdc.ExeCnt = 200;
    }*/
    if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//����������õı�־
		return;
	}

	gOvUdc.ExeCnt += 6;
	if(gOvUdc.ExeCnt < m_Coff)		
	{
		gOvUdc.StepApply = 0;
		//gOvUdc.Flag &= 0xFFFE;					//����������õı�־
		return;
	}
    else gOvUdc.ExeCnt= 0;

	m_DetaU = m_Udc - gOvUdc.UdcBak;
	gOvUdc.UdcBak = m_Udc;

	if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//����������õı�־
		return;
	}
    
	m_Add2 = 2;
	m_Add1 = 1;
	if(m_Coff < 10)	
	{
		m_Add1 = 1 + (10 - m_Coff);
		m_Add2 = m_Add1<<1;
	}

	if(m_Udc <= m_Low)
	{
	    gOvUdc.FreqMax = gVFPar.FreqApply;
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add2;
		else					gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_Mid)
	{
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_High)
	{
		if(m_DetaU > 5)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd++;				//������ѹ��ֵʱ׼��������������
		}
		else if(m_DetaU < -5)	gOvUdc.StepBak += m_Add1;
	}
	else
	{
		if(m_DetaU > 3)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd += 2;				//������ѹ��ֵʱ׼��������������
		}
	}
	gOvUdc.CoffAdd = (gOvUdc.CoffAdd > 100)?100:gOvUdc.CoffAdd;
	gOvUdc.StepBak = (gOvUdc.StepBak > 20000)?20000:gOvUdc.StepBak;
	gOvUdc.StepBak = (gOvUdc.StepBak < -20000)?-20000:gOvUdc.StepBak;

	gOvUdc.StepApply = gOvUdc.StepBak;
	if(((gOvUdc.Flag & 0x02) == 0) &&		//��ѹ���ͺ��������������
	   (gOvUdc.CoffAdd != 0) && 
	   (gVFPar.ovGain >= 10))		//��ѹ��������С��10������
	{	
		gOvUdc.CoffApply += 6;
		gOvUdc.CoffApply = (gOvUdc.CoffApply > 200)?200:gOvUdc.CoffApply;
		gOvUdc.CoffAdd -= 6;
		//gOvUdc.CoffAdd --;
		gOvUdc.CoffAdd = (gOvUdc.CoffAdd < 0)?0:gOvUdc.CoffAdd;
	}
	gOvUdc.Flag &= 0xFFFC;					//����������õı�־
}

void VfFreqDeal()
{
	int  m_Deta,m_Step,m_StepOI,m_StepOU,m_Freq,m_Dir;//m_Flag;
	int VfStepSet;

	VFOVUdcLimit();
	VfStepSet = abs((int)abs(gMainCmd.FreqSet) - (int)abs(gVFPar.FreqApply));    
	m_StepOI = gOvCur.StepApply;		
	m_StepOU = gOvUdc.StepApply;	
    
	//m_Flag = (gMainCmd.SpeedFalg & 0x7FFF);
    //if(C_SPEED_FLAG_DEC == m_Flag)
    if(speed_DEC)
    {
       m_StepOI = (m_StepOI>1)?m_StepOI:1;  //�����������������޷�ͣ��������
       if(0 != gOvUdc.StepApply)
       {
            if( (((long)gOvUdc.LastStepApply * (long)gOvUdc.StepApply) < 0) &&
                (gOvUdc.StepApply < 0))
                gOvUdc.AccTimes++;
            if( 10 < gOvUdc.AccTimes )
            {
                gOvUdc.OvUdcLimitTime = 6000;
                gOvUdc.AccTimes = 11;
            }
            gOvUdc.LastStepApply = gOvUdc.StepApply;
       }
    }
	if(gVFPar.ocGain == 0)		m_StepOI = 32767;
	if((gVFPar.ovGain == 0) || 
	   (gOvUdc.OvUdcLimitTime > 5000) ||
	   (!speed_DEC) ||
	   (abs(gVFPar.FreqApply) - abs(gMainCmd.FreqDesired) < 37))
	{
		m_StepOU = 32767;		
	}

	if(m_StepOI < m_StepOU)	
	{
		m_Step = m_StepOI;
 		gOvUdc.Flag |= 1;
	}
	else
	{
		m_Step = m_StepOU;
		gOvCur.Flag |= 1;
	}

	if(m_Step == 32767)
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
		return;		
	}

    if((speed_DEC) && (VfStepSet <= m_StepOU))
    {
        gOvUdc.StepBak = VfStepSet;
        gOvUdc.StepApply = VfStepSet;
        gOvUdc.Flag |= 1;
    }
    
	if(speed_DEC)
	{
		m_Step = - m_Step;
	}
	m_Freq = abs(gVFPar.FreqApply) + m_Step;


	m_Dir = gMainCmd.FreqSet;
	if(speed_DEC)
	{
		m_Dir = gVFPar.FreqApply;
	}
	if(m_Dir < 0)	m_Freq = - m_Freq;
	
	if(((long)m_Freq * (long)gVFPar.FreqApply) < 0)	m_Freq = 0;

	m_Deta = abs(m_Freq) - abs(gMainCmd.FreqSet);
	if(speed_DEC)
	{
		m_Deta = - m_Deta;
		if(gVFPar.FreqApply == 0)		m_Deta = 0;		//��ͼ��ٵ�0
	}
	if(m_Deta >= 0)	
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
	}
	else
	{
		gVFPar.FreqApply = m_Freq;
	}
}

/************************************************************
HVF:
    ������ϵ�����������ж��񵴳̶ȣ�
************************************************************/
void VfOscIndexCalc()
{
    int temp;
    int curPhase;
    
    if((gCtrMotorType != ASYNC_VF) ||
        (gMainCmd.Command.bit.Start == 0))
    {
        gHVfOscIndex.oscIndex = 0;
        return;
    }

    gHVfOscIndex.AnglePowerFactor = Filter4(abs(gIAmpTheta.PowerAngle), gHVfOscIndex.AnglePowerFactor);
    //gHVfOscIndex.wCntRltm = 1000L*100L*2L / freqRun;        // active window, 2* T_run
    gHVfOscIndex.wCntRltm = 1000L*100L*2L / gMainCmd.FreqReal;
    curPhase = gHVfOscIndex.AnglePowerFactor;
    
    gHVfOscIndex.wCntUse ++;    
    if(gHVfOscIndex.wCntUse < gHVfOscIndex.wCntRltm)
    {
        gHVfOscIndex.maxAngle = (gHVfOscIndex.maxAngle < curPhase) ? curPhase : gHVfOscIndex.maxAngle;
        gHVfOscIndex.minAngle = (gHVfOscIndex.minAngle > curPhase) ? curPhase : gHVfOscIndex.minAngle;
    }
    else                            // update osc-index
    {
        gHVfOscIndex.wCntUse = 0;
        gHVfOscIndex.oscIndex = (long)(gHVfOscIndex.maxAngle - gHVfOscIndex.minAngle) *180L >>15;        // diff(phi) / 90deg * 100%

        gHVfOscIndex.maxAngle = curPhase;
        gHVfOscIndex.minAngle = curPhase;
    }
    
}

// �첽�� HVf ����������
void HVfDeadBandComp()
{
	int   phase,m_Com;
	long  tempL;
    int   tempSect;

    int   phase_sect;
    int   phase_sect_pre;
    int   temp;
    int   gain;

// ȷ��������
	if(gMainCmd.FreqReal <= 40000)      m_Com = gDeadBand.Comp;
	else    m_Com = (int)(((long)gDeadBand.Comp * (long)(gMainCmd.FreqReal - 40000))>>15); 
	if((gMainCmd.Command.bit.StartDC == 1) || (gMainCmd.Command.bit.StopDC == 1))	
	{
        m_Com = 0;
	}
    
// �жϵ�������
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;

    gHVfDeadBandCompOpt.CurPhaseFeed_pre = gHVfDeadBandCompOpt.CurPhaseFeed;
	gHVfDeadBandCompOpt.CurPhaseFeed = (int)(gPhase.IMPhase>>16) + gIAmpTheta.ThetaFilter + gPhase.CompPhase + 16384;
    gHVfDeadBandCompOpt.CurPhaseStepFed = gHVfDeadBandCompOpt.CurPhaseFeed - gHVfDeadBandCompOpt.CurPhaseFeed_pre;// pos or neg

    if(gMainCmd.FreqReal <= 150 || gMainCmd.FreqReal >= gHVfDeadBandCompOpt.DbOptActHFreq) // 1.50Hz, 12.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 0;    //__IQsat(gTestDataReceive.TestData6, 128, 0);           // filter coeff
    }
    else if(gMainCmd.FreqReal < 800)    // 8.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    }
    else if(gMainCmd.FreqReal < 1200)   // 12.00Hz
    {
        temp = gHVfDeadBandCompOpt.DbOptActHFreq - gHVfDeadBandCompOpt.DbOptActLFreq + 1;     
        tempL = gHVfDeadBandCompOpt.DbOptActHFreq - gMainCmd.FreqReal;
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = (long)100L * tempL / temp;
    }

    gHVfDeadBandCompOpt.StepPhaseSet = gPhase.StepPhase >> 16;
    
    gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    tempL = (long)gHVfDeadBandCompOpt.CurPhaseStepFed * (128 -gHVfDeadBandCompOpt.PhaseFwdFedCoeff);
    tempL += (long)gHVfDeadBandCompOpt.StepPhaseSet * gHVfDeadBandCompOpt.PhaseFwdFedCoeff;
    gHVfDeadBandCompOpt.CurPhaseStepPredict = tempL >> 7;
    
    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed / 10922;        // 60deg
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect = gHVfDeadBandCompOpt.CurPhaseFeed - tempSect * 10922;           // present pos

    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed_pre / 10922;
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect_pre = gHVfDeadBandCompOpt.CurPhaseFeed_pre - tempSect * 10922;   // previous pos

    if((phase_sect_pre <= 5461 && phase_sect >= 5461) ||            // sample point
        (phase_sect_pre >= 5461 && phase_sect <= 5461))
    {
        gHVfDeadBandCompOpt.CurPhasePredict = gHVfDeadBandCompOpt.CurPhaseFeed;
    }
    else           // go to predict
    {
        gHVfDeadBandCompOpt.CurPhasePredict += gHVfDeadBandCompOpt.CurPhaseStepPredict;
    }

    phase = gHVfDeadBandCompOpt.CurPhasePredict;

    gHVfDeadBandCompOpt.DbCompCpwmWidth = 1;

    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompU = 0;
    else if(phase < 0)                          gDeadBand.CompU = m_Com;
    else if(phase > 0)                          gDeadBand.CompU = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompV = 0;
    else if(phase < 0)                          gDeadBand.CompV = m_Com;
    else if(phase > 0)                          gDeadBand.CompV = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompW = 0;
    else if(phase < 0)                          gDeadBand.CompW = m_Com;
    else if(phase > 0)                          gDeadBand.CompW = -m_Com;
}

void HVfOscDampDeal()
{
    long tempL;
   // int  tempLg;
    int  tempVolt;
    gHVfOscDamp.CurMagSet = gMotorExtPer.IoVsFreq;
    tempVolt = (long)gMotorExtPer.R1 * (gHVfOscDamp.CurMagSet - (gIMTQ24.M>>12)) >>15;
    //gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gHVfOscDamp.OscDampGain /10L;
    gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gVFPar.VFOvShock /10L;

    gHVfOscDamp.VoltAmp = gOutVolt.Volt; // �Ѽ�ת������ 
    gHVfOscDamp.VoltPhase = atan(gHVfOscDamp.VoltSmSet, gHVfOscDamp.VoltAmp);

    if(gMainCmd.FreqSyn < 0)      // ��ת
    {
        gHVfOscDamp.VoltPhase = - gHVfOscDamp.VoltPhase;
    }
}

/*********************************************************************
consider the stator resitance in low-frequency;
reconstruct the M-T current

Q-current:      Q12
Q-resistance:   Q16
Q-voltage:      Qxx
**********************************************************************/
void HVfCurReDecomp()
{
    int     m_CosPhi;     // Q15, angle phi is the angle of power factor
    int     m_SinPhi;     // Q15,
    long    m_AntiVolt;
    long    m_ResVolt;

    int m_phi2;
    int mDir;

    long temp1;
    long temp2;
    long temp3;
    
    // prepare reistance value, in p.u.
    // pi/2: 16384
    //m_CosPhi = qsin(gIAmpTheta.Theta);
    m_CosPhi = qsin(16384 - gIAmpTheta.PowerAngle);
    m_SinPhi = qsin(gIAmpTheta.PowerAngle);

    m_ResVolt = (long)((Ulong)gLineCur.CurPer * gMotorExtPer.R1 >> 16);   // voltage: Q12
    m_ResVolt = (m_ResVolt < gOutVolt.VoltApply) ? m_ResVolt : gOutVolt.VoltApply;
    
    temp1 = (long)gOutVolt.VoltApply * gOutVolt.VoltApply >> 12;
    temp2 = m_ResVolt * m_ResVolt >> 12;
    temp3 = gOutVolt.VoltApply * m_ResVolt >> 12;
    temp3 = temp3 * m_CosPhi >> 14;
    
    m_AntiVolt = (temp1 + temp2 - temp3) << 12;
    gOutVolt.antiVolt = qsqrt(m_AntiVolt);

    temp1 = ((long)gOutVolt.VoltApply * m_CosPhi >> 15) - m_ResVolt;
    temp2 = (long)gOutVolt.VoltApply * m_SinPhi >> 15;
    m_phi2 = atan((int)temp1, (int)temp2);

    // *generate results
    mDir = (gMainCmd.FreqSyn >= 0) ? 1 : -1;
    gHVfCur.M = (int)((long)gLineCur.CurPer * qsin(m_phi2) >> 15) * mDir;
    gHVfCur.T = (int)((long)gLineCur.CurPer * qsin(16384-m_phi2) >> 15) * mDir;

}
