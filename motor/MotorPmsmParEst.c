/***************************************************************************
�ļ����ܣ�ͬ����������ʶ��������ʼλ�ü�⣬���������λ�á������ʶ
�ļ��汾��
���¸��£�

***************************************************************************/
//�ļ����ܣ�
//    1. ͬ�����ż�λ�ü����жϳ����2ms���� �����޸�ePWMģ��ʵ�ָ��෢����
//    2. ���ż�λ��ʱ˳���ʶ��ͬ�����ߵ�У�d��q���У�
//    3. ���λ�ýǵĿ��ر�ʶ��������������
//    4. ���λ�ýǵĴ��ر�ʶ��(*** ע����Ҫ�û����ı�����������̽)
//    5. �Ա�������֧�ְ���ABZ��UVW, ��ת��ѹ����
//       UVW����������uvw�źŵ����Ϊֱ�Ǻ�uvw�źŷ���
//    6. ͬ�������綯�Ƶı�ʶ��

#include "MotorInclude.h"

/*******************�ṹ������******************************************/
IPM_ZERO_POS_STRUCT		gIPMZero;         //����ͬ����������������λ�ýǵ����ݽṹ
IPM_INITPOS_PULSE_STR	gIPMInitPos;      // pmsm ��ʶ�ż�λ��
PMSM_EST_BEMF           gEstBemf;         // PMSM��ʶ���綯������
PMSM_EST_PARAM_DATA		gPmParEst;        // ����ͬ������ʶʱ�ĺ͹��ܵ����ݽ�����


/*******************��غ�������******************************************/
void InitSetPosTune(void);
void DetectZeroPosOnce(void);
void SynInitPosDetSetTs(void);
void SynCalLabAndLbc(void);
void SynInitPosDetCal(void);


/************************************************************
	��ʼλ�ýǼ�������ʼ��ʶ���������λ�ýǵĳ�ʼ������
************************************************************/
void InitSetPosTune(void)
{
	ResetParForVC();

    gPGDir.ABDirCnt = 0;                    // ��λ���б���������
    gPGDir.UVWDirCnt = 0;
    gPGDir.CDDirCnt = 0;
    gPGDir.ABAngleBak = gIPMZero.FeedPos;
    gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    
	gIPMZero.Flag 		= 0;
	gIPMZero.TotalErr 	= 0;
	gIPMZero.DetectCnt  = 0;
    
	gMainStatus.PrgStatus.all = 0;
}

/*************************************************************
	pm �ż�λ�ñ�ʶ, ��Ҫ���ж��г������
��ʶ����: �����ѹ��, dq������˳��õ���
��ʶʱ�����Լ��������������״̬��
*************************************************************/
void SynTuneInitPos(void)
{	
	switch(gGetParVarable.IdSubStep)
	{
		case 1:                                             // ��ʼ������
			gIPMInitPos.Waite = 0;
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
            gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
			gGetParVarable.IdSubStep++;
			break;
            
		case 2:                                             // ��ʱ�ȴ�20ms, �ٳ�ʼ����ر�������PWM����
			gIPMInitPos.Waite ++;
			if(gIPMInitPos.Waite > 10)			
			{
				gIPMInitPos.Waite = 0;
       			gIPMInitPos.Step = 1;                       // �þ�̬��ʶ��־
       			gMainStatus.PrgStatus.bit.PWMDisable = 1;   // ��EPWM ����
       			gGetParVarable.IdSubStep++;
			}
			break;
            
		case 3:                                             // �ȴ��ж��о�̬��ʶ���...
			if(gIPMInitPos.Step == 0)
			{
				SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMPos_ABZRef((Uint)gIPMPos.InitPos);
                
				gGetParVarable.IdSubStep++;
			}
			break;
            
		case 4:
			DisableDrive();
			SynCalLdAndLq(gIPMPos.RotorPos);
			IPMCalAcrPIDCoff();
			gGetParVarable.IdSubStep++;
            gIPMInitPos.Waite  = 0;
			break;
            
		case 5:
		    if(gIPMInitPos.Waite < 500)
            {
                gIPMInitPos.Waite  ++;
            }
            else
            {
    			InitSetPWM(); 
    			gGetParVarable.IdSubStep = 1;
    		    gGetParVarable.ParEstMstep++;               //�л�����һ��ʶ����

                gMainStatus.PrgStatus.all = 0;
            }
			break;	

        default:
            break;
	}
}

/************************************************************
�ϵ��ʼλ�ýǼ�����(�������жϣ���ADת�������ж���ִ��)��
��U���ź�VW���Ŵ�Ϊ��˵����
	1��PWM1B��PWM2A��PWM3Aǿ�ƹرգ��ߵ�ƽ����
	2����ʼǿ������PWM1A��PWM2B��PWM3BΪ�ߵ�ƽ
	3����������������Ϊ0 (���μ����ȡƽ��ֵ)
	4) ����gIPMInitPos.StepΪ1��ʼ������ֹλ�õ�г, ��ɺ�����0
************************************************************/
void SynInitPosDetect(void)
{
	int  m_Cur;
    int  m_Index;

	switch(gIPMInitPos.Step)
	{
		case 1:							                    //��ʼ����
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section   = 0;
			gIPMInitPos.PWMTs 	  = gIPMInitPos.InitPWMTs;
			gIPMInitPos.CurFirst  = 0;

            gIPMInitPos.CurLimit = 5792;        // 4096*sqrt(2) (Q12)
        	if(gInvInfo.InvCurrent < gMotorInfo.Current)
        	{
        	    Ulong temp; 
                temp = (Ulong)gIPMInitPos.CurLimit * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimit = temp / gMotorInfo.Current;
        	}
            
        	            
			SynInitPosDetSetTs();
			SynInitPosDetSetPwm(7);

            gIPMInitPos.Step ++;
			break;

		case 2:							                            //�����ʵ�������
			EnableDrive();
			gIPMInitPos.PeriodCnt++;
			if(gIPMInitPos.PeriodCnt >= 6)	                        // step: 0 - 5
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					gIPMInitPos.Section = (gIPMInitPos.Section + 1) & 0x01;     // ���巽����

                    //m_Cur = abs(gExcursionInfo.Iu);
                    m_Cur = abs(gIUVWQ24.U >> 12);
                    if(m_Cur < abs(gIPMInitPos.CurFirst))
                    {
                        m_Cur = abs(gIPMInitPos.CurFirst);
                    }

                    #if 0
                    if(gIPMInitPos.PWMTs >= 60000)  //������̫�󣬻�������Ͽ�������ʼλ�ýǼ�����
                    {
                        if(m_Cur < (gIPMInitPos.CurLimit>>2))
                        {
                            
                            gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                            gError.ErrorInfo[1].bit.Fault1 = 10;
                        }
                        else
                        {
                            gError.ErrorCode.all |= ERROR_INIT_POS;
                            gError.ErrorInfo[3].bit.Fault3 = 5;
                        }
                    }  
                    #else                    
                    if((gIPMInitPos.PWMTs >= 65000) && (m_Cur < (gIPMInitPos.CurLimit>>2)))
                    {
                        gError.ErrorCode.all |= ERROR_INIT_POS;
                        gError.ErrorInfo[3].bit.Fault3 = 1;
                    }
                    #endif

                    if((m_Cur > gIPMInitPos.CurLimit) || (gIPMInitPos.PWMTs >= 60000))
					{
						gIPMInitPos.Section     = 0;						
						gIPMInitPos.PeriodCnt   = 0;
                        gIPMInitPos.InitPWMTs = gIPMInitPos.PWMTs;  // ��¼��������ʼλ�ü��ʹ��
                        gIPMInitPos.Step = 3;                       // �����Ѽ�⵽,����һ��
					}
					else
					{                        
				        if(m_Cur > (gIPMInitPos.CurLimit >> 1))
                        {
						    gIPMInitPos.PWMTs += (2 * DSP_CLOCK);      //2us
                        }
                        else
                        {
						    gIPMInitPos.PWMTs += (gMotorExtInfo.Poles *DSP_CLOCK *2); 
                        }
						
						SynInitPosDetSetTs();
					}
					break;

				case 3:
					SynInitPosDetSetPwm(7);
					gIPMInitPos.Section = (gIPMInitPos.Section + 1) & 0x01;     // ���巽����
					//gIPMInitPos.CurFirst = gExcursionInfo.Iu;
					gIPMInitPos.CurFirst = gIUVWQ24.U >> 12;
					break;

				case 1:
				case 4:
					SynInitPosDetSetPwm(gIPMInitPos.Section);                   // ������
					break;

				default:
					break;
			}
			break;

		case 3:							
		case 4:
			gIPMInitPos.PeriodCnt++;
			if(gIPMInitPos.PeriodCnt >= 3)  // step: 0-2
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			if(gIPMInitPos.Section <= 1)
			{
				m_Cur  = gIUVWQ24.U >> 12;
			}
			else if(gIPMInitPos.Section <= 3)
			{
				m_Cur  = gIUVWQ24.V >> 12;
			}
			else
			{
				m_Cur  = gIUVWQ24.W >> 12;
			}

			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					m_Cur = m_Cur - gIPMInitPos.CurFirst;
					m_Index = (gIPMInitPos.Step - 3) * 6 + gIPMInitPos.Section;					
					gIPMInitPos.Cur[m_Index] = abs(m_Cur);

                    gIPMInitPos.Section++;
					if(gIPMInitPos.Section == 6)
					{
						gIPMInitPos.Section = 0;
						gIPMInitPos.Step++;
					}
					break;

				case 1:
					SynInitPosDetSetPwm(gIPMInitPos.Section);
					break;

				case 2:
					gIPMInitPos.CurFirst  = m_Cur;
					break;
			}
			break;

		case 5:							//�������
			DisableDrive();
			SynInitPosDetSetPwm(6);
            
			SynCalLabAndLbc();
			SynInitPosDetCal();

            if(gMainStatus.RunStep == STATUS_GET_PAR)
            {
                gIPMInitPos.Step ++; // ����ͬ����ȱ����
            }
			else
            {
                gIPMInitPos.Step = 0;
                DisableDrive();
            }
			
			break;

        case 6:
			SynInitPosDetSetTs();
			SynInitPosDetSetPwm(7);
            EnableDrive();

            gIPMInitPos.Step ++;  
            gIPMInitPos.PhsChkStep = 0;
            break;

        case 7:
            switch (gIPMInitPos.PhsChkStep)
            {
                case 0:
                    SynInitPosDetSetPwm(0);     // A+, B-
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 1:
                case 4:
                    // wait a step
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 2:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIUVWQ24.U>>12) < (gIPMInitPos.CurLimit>>3))    // U��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 10;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIUVWQ24.V>>12) < (gIPMInitPos.CurLimit>>3))    // V��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 11;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 3:
                    SynInitPosDetSetPwm(2);     // B+ C-
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 5:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIUVWQ24.W>>12) < (gIPMInitPos.CurLimit>>3))    // W��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 12;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIUVWQ24.V>>12) < (gIPMInitPos.CurLimit>>3))    // V��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 11;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;
                    
                default:
                    DisableDrive();
                    SynInitPosDetSetPwm(6);         // recove PWM regester
                    gIPMInitPos.Step ++;
                    break; 
            }   
            break;

		default:
			gIPMInitPos.Step = 0;           // �ж�ִ�����
			DisableDrive();
			break;
	}
}

/*************************************************************
	��ת�Ӵż�λ��ǿ������ΪPos�ĺ���
*************************************************************/
void SetIPMPos(Uint Pos)
{
	static Uint m_Pos,m_Cnt;

	m_Pos = Pos;
	m_Cnt = ((Ulong)(m_Pos/gMotorExtInfo.Poles) *(Ulong)gPGData.PulseNum + (2<<13))>>14;

	gIPMPos.RotorPos = m_Pos;
    DINT;
	gIPMPos.QepTotal = m_Cnt;
	gIPMPos.QepBak   = GetQepCnt();
	EINT;
}

/*************************************************************
	��ת�Ӵż�λ��ǿ������ΪPos�ĺ���,��Ҫ����ABZ������ʱ���ο���
*************************************************************/

void SetIPMPos_ABZRef(Uint Pos)
{
	static Uint m_Pos,m_Cnt;
	
	m_Pos = Pos;
	m_Cnt = ((Ulong)(m_Pos/gMotorExtInfo.Poles) *(Ulong)gPGData.PulseNum + (2<<13))>>14;//��ת��λת��Ϊqep��Ӧ�ļ���ֵ

    gIPMPos.ABZ_RotorPos_Ref = m_Pos;
	DINT;
	gIPMPos.ABZ_QepTotal = m_Cnt;
	gIPMPos.ABZ_QepBak   = GetQepCnt();
	EINT;
}


/************************************************************
	LAB��LBC��LCA���м��㺯��
	gIPMInitPos.Cur���棺IU+��IU-��IV+��IV-��IW+��IW-��
	                     IU+��IU-�IV+��IV-��IW+��IW-
************************************************************/
void SynCalLabAndLbc(void)
{
	Uint  m_Index,m_Sel;
	Uint  m_DetaI,m_Cur1,m_Cur2,m_UDC;
	Ulong m_LTemp;

	//���LAB��LBC��LCA
	for(m_Index = 0;m_Index < 3;m_Index++)
	{
		m_Sel = m_Index<<1;
		m_Cur1 = (gIPMInitPos.Cur[m_Sel] <= gIPMInitPos.Cur[m_Sel+1]) ?
						gIPMInitPos.Cur[m_Sel] : gIPMInitPos.Cur[m_Sel+1];
		m_Cur2 = (gIPMInitPos.Cur[m_Sel+6] <= gIPMInitPos.Cur[m_Sel+7]) ?
						gIPMInitPos.Cur[m_Sel+6] : gIPMInitPos.Cur[m_Sel+7];
		m_DetaI = (m_Cur1>>1) + (m_Cur2>>1);

		m_DetaI = ((Ulong)m_DetaI * (Ulong)gMotorInfo.Current) >> 12;	//(��λ0.01A or 0.1A)
		m_UDC	= gUDC.uDCFilter - (Ulong)m_DetaI * (Ulong)gMotorExtReg.RsPm / 10000;      // 0.1V
		m_LTemp = (Ulong)gIPMInitPos.PWMTs*2L * 100L/DSP_CLOCK;				            //(��λ10ns)
		m_LTemp = (Ulong)m_UDC * (Ulong)m_LTemp;
		//L=((��ѹ/10)*(ʱ��/10^9)/(����/100))*100 (0.01mH)
		//L = ��ѹ*ʱ��/(����*100)  (0.01mH)
		gIPMInitPos.LPhase[m_Index] = (m_LTemp)/((Ulong)m_DetaI * 100);
	}
}

/************************************************************
	��ʼλ�ü��㺯����ͬʱ���������С�
************************************************************/
void SynInitPosDetCal(void)
{
	static int  m_X,m_Y;
	Uint  m_Index;

	for(m_Index = 0;m_Index<12;)
	{
		gIPMInitPos.Cur[m_Index] = gIPMInitPos.Cur[m_Index+1] - gIPMInitPos.Cur[m_Index];
		m_Index = m_Index + 2;
	}
	gIPMInitPos.Cur[0] = (gIPMInitPos.Cur[0] + gIPMInitPos.Cur[0+6]);
	gIPMInitPos.Cur[2] = (gIPMInitPos.Cur[2] + gIPMInitPos.Cur[2+6]);
	gIPMInitPos.Cur[4] = (gIPMInitPos.Cur[4] + gIPMInitPos.Cur[4+6]);

	m_X = gIPMInitPos.Cur[0] - (gIPMInitPos.Cur[2]>>1) - (gIPMInitPos.Cur[4]>>1);
	m_Y = ((long)(gIPMInitPos.Cur[2] - gIPMInitPos.Cur[4]) * 28378L)>>15;
    if(abs(m_X) + abs(m_Y) < (4096/40))
    {
        gError.ErrorCode.all |= ERROR_INIT_POS;
        gError.ErrorInfo[3].bit.Fault3 = 2;
    }//��Ч��:�������ƫ��֮��С�ڵ���������1/40����Ϊ�޷�ʶ��ż���ʼλ��

	gIPMPos.InitPos = (Uint)atan(m_X, m_Y) - 5461;   // 30deg
	gIPMPos.InitAngle_deg = (Ulong)gIPMPos.InitPos * 3600 >> 16;
}

/************************************************************
	�����ز����ڼĴ�������
************************************************************/
void SynInitPosDetSetTs(void)
{
	EALLOW;
	EPwm1Regs.TBPRD = gIPMInitPos.PWMTs;
	EPwm1Regs.CMPB  = gIPMInitPos.PWMTs;
	EPwm2Regs.TBPRD = gIPMInitPos.PWMTs;
	EPwm2Regs.CMPB  = gADC.DelayApply;
	EPwm3Regs.TBPRD = gIPMInitPos.PWMTs;
	EDIS;
}

/************************************************************
	��ʼλ�ü��׶Σ�����PWM�Ķ����Ĵ���������PWM�͵�ƽ��Ч
************************************************************/
void SynInitPosDetSetPwm(Uint Section)
{
	Uint m_Section;
    
	m_Section = Section;

	EALLOW;
	switch(m_Section)
	{
		case 0:									// A+, B-
			EPwm1Regs.AQCSFRC.all = 0x08;       
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x02;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 1:									// A-, B+
			EPwm1Regs.AQCSFRC.all = 0x02;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x08;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 2:									// B+, C-
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x08;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x02;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 3:									// B-, C+
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x02;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x08;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 4:									// A-, C+
			EPwm1Regs.AQCSFRC.all = 0x02;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x08;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 5:									// A+, C-
			EPwm1Regs.AQCSFRC.all = 0x08;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x02;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 6:									//�ָ�PWMģ��ļĴ�������
			EPwm1Regs.DBCTL.all 	= 0x0007;
			EPwm1Regs.AQCTLA.all 	= 0x0090;
			EPwm1Regs.AQCTLB.all 	= 0x00;
			EPwm1Regs.AQCSFRC.all	= 0x00;

			EPwm2Regs.DBCTL.all 	= 0x0007;
			EPwm2Regs.AQCTLA.all 	= 0x0090;
			EPwm2Regs.AQCTLB.all 	= 0x00;
			EPwm2Regs.AQCSFRC.all 	= 0x00;

			EPwm3Regs.DBCTL.all 	= 0x0007;
			EPwm3Regs.AQCTLA.all 	= 0x0090;
			EPwm3Regs.AQCTLB.all 	= 0x00;
			EPwm3Regs.AQCSFRC.all 	= 0x00;
			break;

		default:								//ͬ������ʼλ�ýǼ���ʼ���Ĵ���
			EPwm1Regs.AQCSFRC.all 	= 0x0A;		
			EPwm1Regs.DBCTL.all 	= 0;			
			EPwm1Regs.AQCTLA.all 	= 0x000C;
			EPwm1Regs.AQCTLB.all 	= 0x000C;

			EPwm2Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.DBCTL.all 	= 0;
			EPwm2Regs.AQCTLA.all 	= 0x000C;
			EPwm2Regs.AQCTLB.all 	= 0x000C;

			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			EPwm3Regs.DBCTL.all 	= 0;
			EPwm3Regs.AQCTLA.all 	= 0x000C;
			EPwm3Regs.AQCTLB.all 	= 0x000C;
			break;
	}
	EDIS;
}

/************************************************************
	ͬ�������ر�����λ�ýǱ�ʶ��ʱ�򣬼��һ�νǶ�
************************************************************/
void DetectZeroPosOnce(void)
{
	Uint m_Data;

    if(gPGData.PGMode == 0)
    {
	    m_Data = gIPMPos.RotorPos - gIPMZero.FeedPos;
    }
    else
    {
        m_Data = gIPMPos.RotorPos - gRotorTrans.RTPos;
    }
    
	if(gIPMZero.DetectCnt == 2000)
	{
		gIPMZero.FirstPos = m_Data;
	}
	else
	{
		gIPMZero.TotalErr += ((long)m_Data - (long)gIPMZero.FirstPos);
	}
	//gIPMZero.DetectCnt++;
}

/************************************************************
	���������λ�ü�⣻
* ���ֱ��������λ�ýǶȣ�����������(����uvw�źŷ���)һ����ʶ�ó���
* ��Ҫ�����ջ���Ϸ�M�������
************************************************************/

void SynTunePGZero_No_Load(void)
{
	int	 m_Dir1, m_Dir2, m_Dir3;

    gCtrMotorType = RUN_SYNC_TUNE;
    
	switch(gGetParVarable.IdSubStep)
	{
		case 1:            
            if(gIPMZero.DetectCnt == 0)                     // ��ʼ����ر���
            {           
    			InitSetPosTune();
    			//gMainStatus.PrgStatus.bit.ASRDisable = 1;       //��ֹ�ٶȻ�����
                
    			//gPGData.PGDir.all = 0;                               // ��ʶʱ��ǿ�Ʒ���Ϊ0
    			gPGData.PGDir = 0;
                gPmParEst.UvwDir = 0;
                EALLOW;
                EQepRegs->QDECCTL.all = 0;
                EDIS;             

    			gIPMZero.DetectCnt ++;
                gIMTSet.M = 0L <<12;
                gIMTSet.T = 0;
                gIPMZero.CurLimit = 4096UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current;
            	if(gInvInfo.InvCurrent < gMotorInfo.Current)
            	{
            	    Ulong temp; 
                    temp = (Ulong)gIPMZero.CurLimit * gInvInfo.InvCurrent;                
            		gIPMZero.CurLimit = temp / gMotorInfo.Current;
            	}
                break;
            }
            else if(gIPMZero.DetectCnt == 1)    // �ӳ�һ��2ms�ٿ�����
            {
                gIPMZero.DetectCnt ++;
                EnableDrive();
            }
            
            if(gIMTSet.M < ((long)gIPMZero.CurLimit <<12))      // M ������������̴���, 200ms
            {
                gIMTSet.M += (50L << 12);
            }
            else
            {
                gIMTSet.M = (long)gIPMZero.CurLimit << 12;
                gIPMZero.DetectCnt = 0;
                
                gPGDir.ABDirCnt = 0;
                gPGDir.UVWDirCnt = 0;
                gPGDir.RtDirCnt = 0;

                gGetParVarable.IdSubStep++;
            }
			break;
            
		case 2:                                             // �����������            
            gIPMPos.RotorPos += (gMotorExtInfo.Poles * 11);  
            gIPMZero.DetectCnt++;
            
			m_Dir1 = JudgeABDir();											
   			m_Dir2 = JudgeUVWDir();
            m_Dir3 = JudgeRTDir();                 // �ж��������������
            
            if(gIPMZero.DetectCnt > 6000)                   //תһȦ���жϽ���
            {
                if(gPGData.PGMode == 0)     // ABZ, UVW
                {
                    if(m_Dir1 == DIR_ERROR)
                  	{
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 1;         // abz �źŷ������
    				}                
                    else if(gPGData.PGType == PG_TYPE_UVW)
                    {
                        if(m_Dir2 == DIR_ERROR)
                        {
                            gError.ErrorCode.all |= ERROR_ENCODER;
                            gError.ErrorInfo[4].bit.Fault1 = 2;         // uvw �źŷ������
                        }
                        if(m_Dir2 == DIR_BACKWARD)    
    					{
                            gPmParEst.UvwDir = ((gPmParEst.UvwDir+1)&0x01) ;            // UVW�ź�Ӳ�����ò���
      
                        }
                    }

                    if(m_Dir1 == DIR_BACKWARD)                  //AB����ȡ��
                    {
                        EALLOW;
                        gPGData.PGDir = 1;
                        EQepRegs->QDECCTL.all = 0x0400;
                    	EDIS;
                    }
                    gGetParVarable.IdSubStep = 3;
                }
                else                            // Rotor Transformer
                {
                    if(m_Dir3 == DIR_ERROR)
                    {
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 3;         // ���䷽��������
                    }
                    else if(m_Dir3 == DIR_BACKWARD)
                    {
                        gPGData.PGDir = 1;
                    }
                    gGetParVarable.IdSubStep = 4;           // ������ABZ������
                }
                
                gIPMZero.DetectCnt = 0;
                gIPMZero.Flag = 0;
			}
			break;
            
		case 3:                                             // �ȴ�Z�ź�, �����ʶʱ������˲���
			gIPMPos.RotorPos += (gMotorExtInfo.Poles * 22);  
            gIPMZero.DetectCnt++;
            if(gIPMZero.DetectCnt > 6000)                   // 725����е�Ƕ�
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault1 = 4;         // δ��⵽z�ź�
            }
            if(1 == (gIPMZero.Flag & 0x01))                 // Z �źŵ�
            {
                //gUVWPG.UvwZeroPhase = gIPMPos.RotorPos - gUVWPG.UVWAngle;   // �ýǶ��������
                //gPmParEst.PreUvwZPos = gPmParEst.UvwZPos;
                gUVWPG.TotalErr = 0;
                gIPMZero.DetectCnt = 0;
                gGetParVarable.IdSubStep++;
                gUVWPG.TuneFlag = 0;
            }
			break;
            
		case 4:                                             // ��ʼ��ʶZ�ź����Ǻ�UVW�ź�����
            gIPMPos.RotorPos += (gMotorExtInfo.Poles * 6);   // Լ5RPM���ٶȼ�����λ�ý�

			gIPMZero.DetectCnt++;
			if(gIPMZero.DetectCnt >= 2000)
			{
				DetectZeroPosOnce();
			}
            
            if(gPGData.PGType == PG_TYPE_UVW && gIPMZero.DetectCnt < 5461)
            {
                gUVWPG.lastAgl = gUVWPG.NewAgl;
                gUVWPG.NewAgl = gUVWPG.UVWAngle;
                if(((gUVWPG.lastAgl >= 60072)&&(gUVWPG.lastAgl <= 60074))
                    &&(( gUVWPG.NewAgl >=5460)&&(gUVWPG.NewAgl <=5462))
                   )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }                   
            }
            else if(gIPMZero.DetectCnt == 5461)
            {
               gPmParEst.UvwZeroAng = gUVWPG.ErrAgl;
                gPmParEst.UvwZeroAng_deg = ((Ulong)gPmParEst.UvwZeroAng * 3600L +10) >>16;
            }                    
                
			if(gIPMZero.DetectCnt > 6096)
			{
				gIPMPos.RotorZero = gIPMZero.FirstPos + (int)(gIPMZero.TotalErr>>12);
                gPmParEst.CoderPos_deg = ((Ulong)gIPMPos.RotorZero * 3600L + 10)>>16;
                gIPMPos.RotorZeroGet = gPmParEst.CoderPos_deg;
                
				gIPMZero.DetectCnt = 0;
				gGetParVarable.IdSubStep ++;                
			}
			break;
            
		case 5:                             // �ȴ��ż��ȶ��� m_RotorPosLast ����ʱ��λ��
                                            // �ù��� gIPMPos.RotorPos ����
			if(gIPMZero.DetectCnt < 500)
			{
				gIPMZero.DetectCnt++;       
			}
			else
			{   
			    if(gPGData.PGMode == 0)     // �趨QEP
                {         
                    SetIPMPos(gIPMPos.RotorPos);
                    SetIPMPos_ABZRef(gIPMPos.RotorPos);
                }
                gIPMZero.DetectCnt = 0;
                gGetParVarable.IdSubStep ++;
			}
			break;
            
		case 6:
			DisableDrive();
            if(gIPMZero.DetectCnt < 500)
            {
                gIPMZero.DetectCnt ++;
            }
            else
            {
    			gIPMInitPos.Flag = 0;                   //rt ���ر�ʶ�����ֱ�ӽ������У�λ�ø�׼ȷ
    			gMainStatus.PrgStatus.all = 0;    
    			gGetParVarable.IdSubStep ++;
            }
			break;
            
		case 7:
		default:
			gGetParVarable.IdSubStep = 1;
		    gGetParVarable.ParEstMstep++;       //�л�����һ��ʶ����
			break;
	}
}

/************************************************************
	���ر��������λ�ñ�ʶ
* ��Ҫ�ջ�ʸ�����У�
* ��Ҫ������ϸ����Ӽ��ٹ���Ƶ�ʸ���
* Ŀ��Ƶ�ʣ� �Ӽ���ʱ�����û����ã�
* ������������Ҫ�û����ã��粻�ܳɹ�����Ҫ������̽��
* uvw������ʱ��uvw�źŵķ��������ã�����ֱ�ӱ�ʶ�ó���
************************************************************/
void SynTunePGZero_Load(void)
{
	int	 m_Dir1,m_Dir2, m_Dir3;

    gCtrMotorType = RUN_SYNC_TUNE;
	switch(gGetParVarable.IdSubStep)
	{
		case 1:
            InitSetPosTune();
            // ��Ҫ���������ѹ�����Ĵż�λ��׼��һ�����λ�ýǣ��ջ�ʸ����������
            if(gPGData.PGMode == 0)
            {
                ;// ABZ, UVW
            }
            else
            {
                gPmParEst.EstZero = gIPMPos.InitPos - gRotorTrans.RTPos;
                gPmParEst.CoderPos_deg = ((Ulong)gPmParEst.EstZero * 3600L + 10)>>16;
            }
                        
            gGetParVarable.IdSubStep ++;
            break;

        case 2:			
			//gTune.StatusWord = SUB_TUNE_PM_LOAD_ZERO_ACC;             //���µ�г״̬�֣���ʼ����
            gGetParVarable.StatusWord = TUNE_ACC;    //��ʼ����
            gIPMZero.DetectCnt = 0;
                        
            EnableDrive();            
            gGetParVarable.IdSubStep++;
            gIPMZero.time = 0 ;
			break;
            
		case 3:                                 // �ȴ�������ɲ����յ�Z�ź�
            gIPMZero.time ++ ;
            if(gIPMZero.time >= 6000)    // �ȴ���Լ12������û���յ�Z�źű�����20
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault1 = 5;  
            }
			if(speed_CON && 1 == (gIPMZero.Flag & 0x01))
            {
                gIPMZero.DetectCnt++;
                if(gIPMZero.DetectCnt > 500)     //�ȴ�1sec������ٽ׶�
                {
                    gIPMZero.DetectCnt = 0;
                    gPGDir.ABDirCnt = 0;
                    gPGDir.UVWDirCnt = 0;
                    gPGDir.RtDirCnt = 0;

                    gUVWPG.TotalErr = 0;
                    gUVWPG.UvwCnt = 0;
                    gUVWPG.UvwEstStep = 0;
                    
                    gIPMZero.Flag &= 0xFFFE;
                    gGetParVarable.IdSubStep++;
                }
            }            
			break;
            
		case 4:                                 //���ٽ׶� ---- �жϱ�������������
		
            if(gPGData.PGMode == 0)
            {
                m_Dir1 = JudgeABDir();											
   			    m_Dir2 = JudgeUVWDir();
                if(gPGData.PGType == PG_TYPE_UVW && abs(gPGDir.UVWDirCnt) > 2) 
                {   
                    if(m_Dir2 == DIR_BACKWARD)  // UVW ���������ر�ʶʱ���Ա�ʶUVW �źŵķ���
                    {
                        gPmParEst.UvwDir = (gPmParEst.UvwDir+1)&0x01;  // UVW ������
                    }
                    else if(m_Dir2 == DIR_ERROR)
                    {
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 6;         // ���ر�ʶuvw����������
                    }
                }

                if(abs(gPGDir.ABDirCnt) > 3)
                {
                    if(gPGData.PGType != PG_TYPE_UVW)
                    {
                        gGetParVarable.IdSubStep++;     //
                    }
                    else if(abs(gPGDir.UVWDirCnt)> 3)
                    {
                        gGetParVarable.IdSubStep++;     //
                    }
                }
               gUVWPG.lastAgl = 0;
               gUVWPG.NewAgl = 0;
               gUVWPG.TuneFlag = 0;
            }
            else if(gPGData.PGMode == 1)
            {
                m_Dir3 = JudgeRTDir();
                if(abs(gPGDir.RtDirCnt) > 5)
                {
                    gIPMZero.DetectCnt = 0;
                    gGetParVarable.IdSubStep++;         //
                }
            }
            break;
            
        case 5:                     //���ٽ׶� ---- ABZ������ ���Ƕȱ�ʶ

            if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 0)
            {
                gUVWPG.UvwEstStep ++;           // gUVWPG.UvwEstStep==1, main2ms loop start
            }
            else if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 2)
            {
                GetUvwPhase();
                gUVWPG.lastAgl = gUVWPG.NewAgl;
                gUVWPG.NewAgl = gUVWPG.UVWAngle;
                if(((gUVWPG.lastAgl >= 60072)&&(gUVWPG.lastAgl <= 60074))
                    &&(( gUVWPG.NewAgl >=5460)&&(gUVWPG.NewAgl <=5462))
                   )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }  
                else if(((gUVWPG.lastAgl >= 5460)&&(gUVWPG.lastAgl <= 5462))
                          &&(( gUVWPG.NewAgl >=60072)&&(gUVWPG.NewAgl <=60074))
                    )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }

                //���ߴż��Ƕ��ǵݼ���
                
            }
            else if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 3)
            {
               // gPmParEst.UvwZeroAng = (Uint)(gUVWPG.TotalErr /gUVWPG.UvwCnt);
               gPmParEst.UvwZeroAng = gUVWPG.ErrAgl;
                gPmParEst.UvwZeroAng_deg = ((Ulong)gPmParEst.UvwZeroAng * 3600L +10) >>16;
                gUVWPG.UvwEstStep ++;           // gUVWPG.UvwEstStep==4, uvw est complete totally
            }
    
            if(1 == (gIPMZero.Flag & 0x01))             // һ����е�Ƕ�����, total==16
            {
                gIPMZero.Flag &= 0xFFFE;
    			if(gIPMZero.DetectCnt == 0)
    			{
    				gIPMZero.FirstPos = gIPMPos.PosInZInfo;
    				gIPMZero.TotalErr = 0;
    			}
    			else if(gIPMZero.DetectCnt < 5)
    			{				
    				gIPMZero.TotalErr += (int)(gIPMPos.PosInZInfo - gIPMZero.FirstPos);
    			}
                else if(gIPMZero.DetectCnt == 5)
                {                    
                    gIPMPos.RotorZero = gIPMZero.FirstPos + (int)(gIPMZero.TotalErr >>4);
                    gPmParEst.CoderPos_deg = ((Ulong)gIPMPos.RotorZero * 3600L + 10)>>16;
                    gIPMPos.RotorZeroGet = gPmParEst.CoderPos_deg;
                }
                gIPMZero.DetectCnt++;                
            }   

            if(gIPMZero.DetectCnt > 5)
            {
                if((gPGData.PGType != PG_TYPE_UVW) ||
                    (gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep >=4))
                {
                    gGetParVarable.StatusWord = TUNE_DEC;    //��ʼ����
                    gGetParVarable.IdSubStep++;
                }
            }
		    break;
            
		case 6:                                 // ���ٽ׶�
			if(abs(gMainCmd.FreqSet) <= 10)
            {
                gIPMZero.DetectCnt++;
                if(gIPMZero.DetectCnt > 250)
                {
                    gIPMZero.DetectCnt = 0;
					DisableDrive();
                    
                    gGetParVarable.IdSubStep++;
                }
            }
			break;
            
		case 7:
            if(gIPMZero.DetectCnt < 500)
            {
                gIPMZero.DetectCnt ++;
            }
            else
            {
    			gIPMInitPos.Flag = 0;               //rt 
    			SetIPMPos(gIPMPos.RotorPos);
                SetIPMPos_ABZRef(gIPMPos.RotorPos);

                gMainStatus.PrgStatus.all = 0;
    			gGetParVarable.ParEstMstep++; //�л�����һ��ʶ����
    			gGetParVarable.IdSubStep = 1;
            }                
			break;

		default:
			;
			break;
	}
}

/*************************************************************
	ͬ�������綯�Ʊ�ʶ
* ��Ҫ�����ջ���Ϸ�M�������
* ���������ΪΪ: IdSet = 1500, 3000(Q12)��
* �����ϼ�¼d�������Լ�����ߵ�����q�������С��
* �����ϼ�¼q���ѹ��Լ������(��)��ѹ��d���ѹ��С��
* �򻯷ų�: Uq �� w(Ld * Id + Phi_r);

* �Ӽ��ٵļӼ��ٹ���Ƶ��ָ�� �ɱ�ʶ���������
* Ŀ��Ƶ�ʣ��Ӽ���ʱ���ɹ��ܲ���������:
    Ŀ��Ƶ��:   40% ����ת�٣�
    �Ӽ���ʱ��: ����22����: 30sec������22����: 50sec��
*************************************************************/
void SynTuneBemf()
{
    Ulong temp1, temp2, temp3;
    Ulong fluxRotor;

    gCtrMotorType = RUN_SYNC_TUNE;       //
    
    switch(gGetParVarable.IdSubStep)
    {
        case 1:
            gEstBemf.TotalId1 = 0;
            gEstBemf.TotalId2 = 0;
            gEstBemf.TotalVq1 = 0;
            gEstBemf.TotalVq2 = 0;

            gEstBemf.IdSet = 1500;      // Q12;
            gEstBemf.IqSet = 0;
            gEstBemf.IdSetFilt = 0;            
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
            }
            ResetParForVC();

           
            gEstBemf.TuneFreqAim = (long)gMotorInfo.FreqPer * 2L / 5L;      // 40% ����Ƶ��
            gEstBemf.FreqRem = 0;                                           // Ƶ�ʲ�����������
            gEstBemf.AccDecTick = (gInvInfo.InvTypeApply <= 20) ? (30L*1000L/2) : (60L*1000L/2);
                                                                    // ����22����: 30sec ���ٵ��Ƶ��
                                                                    // ����22����: 60sec ���ٵ��Ƶ��
            gEstBemf.TuneFreqSet = 0;
            //EnableDrive();
            gGetParVarable.IdSubStep ++;
            break;
            
        case 2:                        // ���ڼ��ٹ���
            EnableDrive();
            //if(speed_CON)
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet < gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet += gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = gEstBemf.TuneFreqAim;
                gEstBemf.Cnt ++;              
            }
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 3:                         // ���ֵ����͵�ѹ----1
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId1 += gLineCur.CurPer;           // �������Ϊ��ֵ
                gEstBemf.TotalVq1 += gOutVolt.VoltApply;        // �����ѹΪ��ֵ
            }
            else
            {
                gEstBemf.TotalId1 = gEstBemf.TotalId1 >> 11;
                gEstBemf.TotalVq1 = gEstBemf.TotalVq1 >> 11;

                gEstBemf.TuneFreq = abs(gMainCmd.FreqSyn);
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 4:                     // �޸ĵ���ֵ
            gEstBemf.IdSet = 3000;
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
            }
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 5:                         // ���ֵ�����ѹ2
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId2 += gLineCur.CurPer;           // �������Ϊ��ֵ
                gEstBemf.TotalVq2 += gOutVolt.VoltApply;        // �����ѹΪ��ֵ
            }
            else
            {
                gEstBemf.TotalId2 = gEstBemf.TotalId2 >> 11;
                gEstBemf.TotalVq2 = gEstBemf.TotalVq2 >> 11;

                gEstBemf.Cnt = 0;
                //gGetParVarable.StatusWord = TUNE_DEC;
                gEstBemf.TuneFreqAim = 0;
                gEstBemf.FreqRem = 0;     
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 6:                         // �ȴ�����
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet > gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet -= gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = 0;
                DisableDrive();
                gEstBemf.Cnt  = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;
            
        case 7:
            if(gEstBemf.Cnt < 500)          // �ȴ�ͣ��ͣ��
            {
                gEstBemf.Cnt ++;
            }
            else
            {        
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 8:                         // ���㷴�綯��ϵ������ɱ�ʶ           
            temp1 = (Ulong)gEstBemf.TotalVq2 * gEstBemf.TotalId1;
            temp2 = (Ulong)gEstBemf.TotalVq1 * gEstBemf.TotalId2;
            temp3 = (gEstBemf.TotalId2 - gEstBemf.TotalId1) * gEstBemf.TuneFreq;
            fluxRotor = (((Ullong)temp2 - (Ullong)temp1) <<15) /temp3; // Q: 12+12-15 + 15-12 = Q12

            temp3 = fluxRotor * abs(gMotorInfo.FreqPer) >> 12;        // Q15
            gEstBemf.BemfVolt = temp3 * gMotorInfo.Votage * 10L >> 15;              // 0.1 V
            
            gMainStatus.PrgStatus.all = 0;
            gGetParVarable.IdSubStep = 1;
			gGetParVarable.ParEstMstep ++; //�л�����һ��ʶ����

            SetIPMPos(gIPMPos.RotorPos);
            SetIPMPos_ABZRef(gIPMPos.RotorPos);
			break;

        default:
            break;
    }            
}

