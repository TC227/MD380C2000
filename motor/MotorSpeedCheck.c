/****************************************************************
�ļ����ܣ�ת�ٸ�����س���
�ļ��汾�� 
�������ڣ� 

****************************************************************/

#include "MotorSpeedCheceInclude.h"

// // ȫ�ֱ�������
FEISU_STRUCT			gFeisu;			//ת�ٸ����ñ���
FEISU_STRUCT_NEW        gFeisuNew;
PID_STRUCT				gSpeedCheckPID;
PID_STRUCT				gSpeedCheckPID1;
 
// // �ļ��ڲ��������� 
void SpeedCheckUdcRiseDeal(void);
void SpeedCheckCycleOver(void);
void SpeedCheckSUB1(void);
void SpeedCheckCycleSUB2(void);
void PrepareForSpeedCheck(void);

void Cur_PID(void);
void Cur_PIDonefive(void);
void SaveTheta(void);
 
/************************************************************
	׼������ת�ٸ���״̬
************************************************************/
void PrepareForSpeedCheck(void)
{
    gFeisu.Case4Sig2.all = 0;	
	gFeisu.Case4Sig1.all = 0;
	gFeisu.AlmostCNT     = 0;
	gFeisu.CycleCnt      = 0;
	gFeisu.GuoduCnt      = 0;
	gFeisu.LowFreqCNT    = 0;
	gFeisu.CurDelta      = 0;
	gFeisu.SpeedCheck    = 0;
	gFeisu.UdcOvCnt      = 0;
	gFeisu.UdcRiseCnt    = 0;
	gFeisu.VoltCheck     = 0;
	gFeisu.VoltCNT       = 0;
	gFeisu.UdcBak        = gUDC.uDC;
	gFeisu.CheckMode     = gComPar.SpdSearchMethod;
	gFeisu.SpeedMaxPu    = (((Ulong)gBasePar.MaxFreq)<<15)/gBasePar.FullFreq;
	gFeisu.Speed5hz      = (500L <<15)/gBasePar.FullFreq01;
    gIMTSetApply.T       = 0;
    gSpeedCheckPID.Total = 0;
	gSpeedCheckPID1.Total = 0;

    gFeisu.Case4Sig1.all = 0x00;
    gFeisu.Case4Sig1.bit.SpdSig = (gFeisu.SpeedLast >= 0) ? 1 : 0;
    gFeisu.CsrWtOver = 0;
    
	if(gFeisu.CheckMode == 2)				//�����Ƶ�ʸ���
	{
		gFeisu.Case4Sig1.bit.CHECK_BIT15 = 1;
		//gFeisu.SpeedCheck = gFeisu.Speed5hz;		
        gFeisu.SpeedCheck = (gFeisu.SpeedLast < 0) ? (-gFeisu.SpeedMaxPu) : gFeisu.SpeedMaxPu;
	}
	else if((gFeisu.CheckMode == 1) ||	(abs(gFeisu.SpeedLast) < gFeisu.Speed5hz))
	{	//�����ٸ��ٻ��С��ͣ��Ƶ�ʸ���							
		// gFeisu.Case4Sig1.bit.CHECK_BIT14 = 0;
		// gFeisu.Case4Sig1.bit.CHECK_BIT15 = 0;
        gFeisu.SpeedCheck = (gFeisu.SpeedLast < 0) ? (-gFeisu.Speed5hz) : gFeisu.Speed5hz;
	}
	else								//��ͣ��
	{
		gFeisu.Case4Sig1.bit.CHECK_BIT14 = 1;
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	gMainCmd.FreqSyn = gFeisu.SpeedCheck;	    //	set h_freq_syn
	gFeisu.VoltTemp = CalOutVotInVFStatus(gMainCmd.FreqSyn);	    //	cal voltage of VF.


}

/************************************************************
	ת�ٸ���״̬����
************************************************************/
void RunCaseSpeedCheck(void)
{
	Uint m_UData;
    
    //gComPar.SpdSearchMethod = 2;    // 0: ͣ��Ƶ�ʿ�ʼ��1: 0��׷�٣�2: �����׷�٣�
    //gComPar.SpdSearchTime = 20;

	if((gMainCmd.Command.bit.Start == 0) ||     //ͣ������
	   (gError.ErrorCode.all != 0))			    //����
	{ 
		gMainStatus.RunStep = STATUS_STOP;
		gMainStatus.SubStep = 1;
		gMainStatus.StatusWord.bit.StartStop = 0;
		DisableDrive();
		return;
	}
	gMainStatus.StatusWord.bit.StartStop = 1;

    if(ASYNC_SVC == gCtrMotorType)
    {
        //SVCCalRotorSpeed();           // 0.5ms�м���
        gFeisu.VoltCheckAim = 4096;
    }
    else    // VF
    {
        gFeisu.VoltTemp = CalOutVotInVFStatus(gFeisu.SpeedCheck);
        CalTorqueUp();
        gFeisu.VoltTemp += gOutVolt.detVfVoltUp;
	    gFeisu.VoltCheckAim = (((Ulong)gMotorExtPer.R1 * 2048L)>>15) + gFeisu.VoltTemp;
        gComPar.SpdSearchTime = gComPar.SpdSearchTimeSet;
        // �����ٸ���ʱ���Ӵ�Ŀ���ѹ����ֹ���ж�
        if(gInvInfo.InvTypeApply > 13)         
        {   
           gComPar.SpdSearchTime = (long)gComPar.SpdSearchTimeSet * 10L/(gInvInfo.InvTypeApply + 10);
           if(gFeisu.CheckMode == 1)
            {
              gFeisu.VoltCheckAim = (((Ulong)gMotorExtPer.R1 * 5125L)>>15) + gFeisu.VoltTemp;
            }
        }        
    }
    
/////////////////////////////////////////////////////////////////
//�Ѿ��������ٶȣ��Ե�ѹ���ȴ��� ����׷���ϵ�
	if(gFeisu.Case4Sig1.bit.CHECK_GET == 1)	
	{				
		if(gFeisu.VoltCheck > (gFeisu.VoltCheckAim + 3))
		{	
			gFeisu.VoltCheck -= 2;
			gFeisu.GuoduCnt = 0;
		}
		else if(gFeisu.VoltCheck < (gFeisu.VoltCheckAim - 3)) //����ʸ����Ҫʹ��Ŀ���ѹֵ	
		{	
			gFeisu.VoltCheck += 2;
			gFeisu.GuoduCnt = 0;

		}
		gFeisu.GuoduCnt++;
		if(gFeisu.GuoduCnt > 400)  			//voltage equals with * for 1 second 
		{
			gMainStatus.RunStep = STATUS_RUN;		//�ٶ��������
			gMainStatus.SubStep = 1;
			gMainStatus.StatusWord.bit.SpeedSearchOver = 1;
			gFeisu.Case4Sig1.all=0;
		}
		SpeedCheckSUB1();
		return;
	}
    
/////////////////////////////////////////////////////////////////
// �ٶ������׶Σ������ѹ�Ĳ���(gFeisu.VoltCheck)
	//gSpeedCheckPID.KP   = 6400;       //current pid control
	gSpeedCheckPID.KP = 200; 
	gSpeedCheckPID.KI = 400;
    gSpeedCheckPID.QP = 0;
    gSpeedCheckPID.QI = 0;
	gSpeedCheckPID.KD = 0;
	gSpeedCheckPID.Max = 4096;
	gSpeedCheckPID.Min = 0;
  #if 0
	if(gInvInfo.InvCurrent < gMotorInfo.Current)
	{
		gFeisu.CurDelta = ((Ulong)gInvInfo.InvCurrent * 5125L) / gMotorInfo.Current;
		gFeisu.CurDelta = (gFeisu.CurDelta > 5125)?5125:gFeisu.CurDelta;
	}
	else
  #endif
	{
		gFeisu.CurDelta = 5125;					//130% rate current
	}
	gSpeedCheckPID.Deta = gFeisu.CurDelta - (int)gLineCur.CurPer;
	PID((PID_STRUCT *)&gSpeedCheckPID);
	gFeisu.VoltCheck = (gSpeedCheckPID.Out>>16);
    
///////////////////////////////////////////////////////////////
// �ٶ������ӽ�����, Ҳ�ǹ��ɹ���
	if(gFeisu.Case4Sig2.bit.DelayOver == 1)	// delay (bit7)is over
	{
		gFeisu.VoltCNT++;
		if(gFeisu.VoltCNT > 100)		
		{
			gFeisu.Case4Sig1.bit.CHECK_GET = 1; // speed search complete
		}
		SpeedCheckSUB1();
		return;
	}
    
	if(gFeisu.Case4Sig2.bit.AlmostOver == 1) //almost over
	{
		gFeisu.AlmostCNT++;
		if((gFeisu.AlmostCNT > 1000) || 
		   ( (gFeisu.Case4Sig2.bit.TwoCyclesOver == 0) &&       // �Ѿ�����������
		     (gFeisu.VoltCheck < gFeisu.VoltCheckAim) ) )       // 
		{
			gFeisu.Case4Sig2.bit.DelayOver = 1;
		}
		SpeedCheckSUB1();
		return;
	}
	gFeisu.AlmostCNT = 0;

// �������������ݵ�ѹ�ж��Ƿ��������
	m_UData = abs(gFeisu.SpeedCheck)>>8;                        
	m_UData = m_UData + 512;
	m_UData = ((Ulong)m_UData * (Ulong)gFeisu.VoltCheck)>>9;
	if(m_UData > gFeisu.VoltCheckAim)
	{
		gFeisu.Case4Sig2.bit.AlmostOver =1;
		SpeedCheckSUB1();
		return;
	}

	gFeisu.LowFreqCNT=0;
//////////////////////////////////////////////////////////////
// Ƶ�������׶Σ��ִ�0�������ʹ����������
	if(gFeisu.Case4Sig1.bit.UdcHigh == 0)  //change freqency
	{
	// ������������
		if(((gLineCur.CurPer + 1000) < gFeisu.CurDelta) &&  	// �ȴ������ջ���λ
		    (gFeisu.CsrWtOver == 0))                            // ֻ�õ�һ����Ч
		{
			gFeisu.Ger4A = 0;
		}
		else								
		{
			gFeisu.Ger4A = gComPar.SpdSearchTime + 1;
            gFeisu.CsrWtOver = 1;
		}
        
    // search form speed-0
		if(((gFeisu.Case4Sig1.all & 0x0C000) == 0) ||       // 
		   ((gFeisu.Case4Sig1.all & 0x0C000) == 0x0C000))
		{
			if(gFeisu.Case4Sig1.bit.SpdSig == 0)	// negative
			{
				gFeisu.SpeedCheck -= gFeisu.Ger4A;
			}
			else								    // positive
			{
				gFeisu.SpeedCheck += gFeisu.Ger4A;
			}

			if(abs(gFeisu.SpeedCheck) >= gFeisu.SpeedMaxPu)
			{
				if(gFeisu.CycleCnt == 0)    // ��һ����������
				{
					gFeisu.CycleCnt = 0x1111;
					gFeisu.Case4Sig1.all ^= 0x2000;         // Ƶ�ʷ�����(��������������)
					if(gFeisu.Case4Sig1.bit.SpdSig == 1)	
					{
						gFeisu.SpeedCheck = gFeisu.Speed5hz;
					}
					else									
					{
						gFeisu.SpeedCheck = -gFeisu.Speed5hz;
					}
					SpeedCheckCycleSUB2();
				}
				else                       // �ڶ����������ڽ�������
				{
					SpeedCheckCycleOver();	
				}
			}
		}
    //search from f_max or stop freq
		else
		{
			if(gFeisu.Case4Sig1.bit.SpdSig == 1)	//positive
			{
				gFeisu.SpeedCheck -= gFeisu.Ger4A;
				if(gFeisu.SpeedCheck < 0)
				//if(gFeisu.SpeedCheck < gFeisu.Speed5hz) //rd
				{
					if(gFeisu.CycleCnt == 0)
					{
						gFeisu.CycleCnt = 0x1111;
						gFeisu.Case4Sig1.bit.SpdSig = 0;
						gFeisu.SpeedCheck = -gFeisu.SpeedMaxPu;
						SpeedCheckCycleSUB2();
					}
					else
					{
						SpeedCheckCycleOver();
					}
				}
			}
			else								
			{									//negtive
				gFeisu.SpeedCheck += gFeisu.Ger4A;
				if(gFeisu.SpeedCheck > 0)     //rd
				//if(gFeisu.SpeedCheck > (-(int)gFeisu.Speed5hz))
				{
					if(gFeisu.CycleCnt == 0)
					{
						gFeisu.CycleCnt = 0x1111;
						gFeisu.Case4Sig1.bit.SpdSig = 1;
						gFeisu.SpeedCheck = gFeisu.SpeedMaxPu;
						SpeedCheckCycleSUB2();
					}
					else
					{
						SpeedCheckCycleOver();
					}
				}
			}
		}        
    //the following: decide whether the UDC is high or not
		gFeisu.UdcDelta = gUDC.uDCFilter - gFeisu.UdcOld;
		gFeisu.UdcOld   = gUDC.uDCFilter;
		if((gUDC.uDCFilter - 800) > gFeisu.UdcBak)      // bus voltage rised much more: 80.0V
		{
			//gFeisu.Ger4A = (gComPar.SpdSearchTime << 3)+5;
			if((gFeisu.CheckMode != 1)||(gMainCmd.FreqReal> 1000))
			{
				gFeisu.Case4Sig1.bit.UdcHigh = 1;
				gFeisu.Ger4A = 400;
			}
			else
			{
    			gFeisu.Ger4A = 0;
			}
			SpeedCheckUdcRiseDeal();
			SpeedCheckSUB1();
			return;
		}
		else
		{
			if(gFeisu.UdcDelta > 0)		// bus rised a little
			{
				gFeisu.UdcRiseCnt++;
				if(gFeisu.UdcRiseCnt >= 1000)
				{
					gFeisu.Ger4A=(gComPar.SpdSearchTime << 3)+5;
					gFeisu.Case4Sig1.bit.UdcHigh=1;
					SpeedCheckUdcRiseDeal();
					SpeedCheckSUB1();
					return;
				}
				else
				{		
					SpeedCheckSUB1();
					return;
				}
			}
			else            // bus decreased
			{
				gFeisu.Case4Sig1.bit.UdcHigh=0;
				gFeisu.UdcRiseCnt=0;
				SpeedCheckSUB1();
				return;
			}
		}
	}
	else    //gFeisu.Case4Sig1.bit.UdcHigh == 1  //UDC is too high		
	{
		gFeisu.UdcOvCnt ++;
		if(gFeisu.UdcOvCnt > 1)
		{
			gFeisu.UdcOvCnt = 0;
			gFeisu.UdcDelta = gUDC.uDCFilter - gFeisu.UdcOld;
			gFeisu.UdcOld   = gUDC.uDCFilter;
			if(gFeisu.UdcDelta > 0)
			{
				gFeisu.Ger4A = (gComPar.SpdSearchTime << 3)+5;
				SpeedCheckUdcRiseDeal();
			}
			else if(gUDC.uDCFilter - gFeisu.UdcBak <= 350)	
			{
				gFeisu.Case4Sig2.bit.AlmostOver = 1;
			}
		}
		SpeedCheckSUB1();
		return;
	}
}

/************************************************************
	ת�ٸ����ӳ���, �õ����Ƶ�ʺ͵�ѹ
outputVar:  
frequency -> gMainCmd.FreqSyn;
voltage   -> gOutVolt.VoltApply;
************************************************************/
void SpeedCheckSUB1(void)
{
    // gFeisu.SpeedCheck �� gFeisu.VoltCheck�Ѿ�׼���ã�
    // �ȴ�������ѡ��

    gMainCmd.FreqSyn  = gFeisu.SpeedCheck;
	gMainCmd.FreqSet       = gFeisu.SpeedCheck;
	gMainCmd.FreqDesired   = gFeisu.SpeedCheck;
    
    if(IDC_VF_CTL == gMainCmd.Command.bit.ControlMode)
    {
        gFeisu.VoltTemp = CalOutVotInVFStatus(gMainCmd.FreqSyn);		// Ϊ����Ŀ���ѹ
    	//gOutVolt.VoltPhaseApply = gOutVolt.VoltPhase;
    	if(gMainCmd.FreqSetBak > 0)
        {
            gOutVolt.VoltPhaseApply = 16384;
        }
    	else if(gMainCmd.FreqSetBak < 0)
        {
            gOutVolt.VoltPhaseApply = -16384;      
        }
    	gOutVolt.VoltApply      = gFeisu.VoltCheck;
    }

}

/************************************************************
	ת�ٸ����ӳ��� ---------�����ٶȣ�update gFeisu.SpeedCheck
************************************************************/
void SpeedCheckUdcRiseDeal(void)
{
//	if((gFeisu.Case4Sig1.bit.CHECK_BIT14 == 1)||
//	   (gFeisu.Case4Sig1.bit.CHECK_BIT15 == 1))
//	{
//		gFeisu.Ger4A = -gFeisu.Ger4A;
//	}
	if(gFeisu.SpeedCheck >= 0)	
	{
		gFeisu.SpeedCheck += abs(gFeisu.Ger4A);
	}
	else
	{
		gFeisu.SpeedCheck -=abs( gFeisu.Ger4A);
	}

	if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000)||
	   ((gFeisu.Case4Sig1.all & 0x0C000) == 0x8000))
	{
		if((gFeisu.Case4Sig1.bit.SpdSig == 1) && (gFeisu.SpeedCheck < 0))
		{
			gFeisu.SpeedCheck = 0;
		}
		if((gFeisu.Case4Sig1.bit.SpdSig == 0) && (gFeisu.SpeedCheck > 0))
		{
			gFeisu.SpeedCheck = 0;
		}
	}
	if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000) && 
	   (gFeisu.SpeedLast >= 0) &&
	   (gFeisu.SpeedCheck > gFeisu.SpeedLast))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000) &&
	        (gFeisu.SpeedLast < 0) &&
	        (gFeisu.SpeedCheck < gFeisu.SpeedLast))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) != 0x4000) && 
	        (gFeisu.SpeedCheck > gFeisu.SpeedMaxPu))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedMaxPu;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) != 0x4000) && 
	        (gFeisu.SpeedCheck < -gFeisu.SpeedMaxPu))
	{
		gFeisu.SpeedCheck = -gFeisu.SpeedMaxPu;
	}
}

/************************************************************
	ת�ٸ����ӳ���
************************************************************/
void SpeedCheckCycleOver(void)          // �������ɹ��ٶȴ���	
{
	if(gMainCmd.FreqDesired > 0)	            
	{
		gFeisu.SpeedCheck = (gFeisu.Speed5hz >> 3);
	}
	else	
	{
		gFeisu.SpeedCheck = -(gFeisu.Speed5hz >> 3);
	}
	gFeisu.Case4Sig2.all |= 0x8040;
	gFeisu.VoltCheck      = 0;
	gSpeedCheckPID.Total  = 0;
	gFeisu.Case4Sig1.bit.CHECK_GET = 1;
}

/************************************************************
	ת�ٸ����ӳ��� ------- ��������
************************************************************/
void SpeedCheckCycleSUB2(void)
{
	gFeisu.VoltCheck     = 0;
	gSpeedCheckPID.Total = 0;
}


