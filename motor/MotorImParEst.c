/****************************************************************
�ļ����ܣ��첽��������ʶ
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
#include "MotorParaIDinclude.h"

// // �ļ��ڲ���������
interrupt void RrLoInPeriodAndCmpInt(void);

void RsIdentify(void);
void RrLoIdentify(void);
void LmIoIdentify(void);
void SetPwmForRrLoId(void);
void InitRrLoVariable(void);
void EnableTimer(void);
void RrWaitAddGetUdc(void);
void RrGetVoltReBegin(void);
void RrLoCalVaule(void);
void InitLmIoVaribale(void);
void LmIoCalValue(void);

/*******************************************************************
�������� ת�ӵ����©�б�ʶʱ���������жϺͱȽ��ж��м������͵�ѹ
********************************************************************/

/************************************************************
	������ʶ�׶�(�ó������ڼ��U���V��������ͨ��������ƫ��)
	ͬʱ��ⶨ�ӵ���ֵ��
************************************************************/

/****************************************************************
�������ܣ����ӵ����ʶ
*****************************************************************/
void RsIdentify(void)
{
	Uint  m_UData1,m_UData2,m_MaxCur;
	Ulong m_ULong1,m_ULong2;

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%��Ƶ�������
	switch(gGetParVarable.IdSubStep)
	{
		case 1:     // ��ʶ���ӵ���ʱ�����Ĵ���, EPWMģ�������, �����ĳ�ʼ��
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V���ӵ����ʿ��Ƶ�̶�1K��������ѹ�ȼ�4K��2011.5.7 L1082*/
              }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V���ӵ����ʿ��Ƶ�̶�1.5K��������ѹ�ȼ�4K��2011.6.20 L1082 */
              }
            else
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD;
              }  
			gUVCoff.Comper     = gUVCoff.Rs_PRD/2;
			gUVCoff.Number     = 0;
			gUVCoff.TotalU     = 0;
			gUVCoff.TotalV     = 0;
			gUVCoff.TotalI     = 0;
			gUVCoff.TotalIL    = 0;
			gUVCoff.TotalVolt  = 0;
			gUVCoff.TotalVoltL = 0;
            
			EALLOW;
            EPwm1Regs.ETSEL.bit.INTEN = 0;  //���ӵ����ʶʱ����ֹ�����ж�
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;

			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
			EPwm2Regs.CMPA.half.CMPA = gUVCoff.Comper;

		
			EPwm3Regs.AQCSFRC.all = 0x0A;		//�ر�3�ű�
			EPwm3Regs.DBCTL.all   = 0x0000;
			EDIS;
            
			gGetParVarable.IdSubStep = 2;               
			break;

		case 2:         // ���α�ʶʱ��ʱ֮���ٿ�������
            if(0 == gUVCoff.IdRsDelay)
            {
			    gGetParVarable.IdSubStep = 3;
			    EnableDrive();
            }
            else
            {
                gUVCoff.IdRsDelay--;
            }
			break;
            
		case 4:
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVoltL += gUDC.uDCFilter;
				gUVCoff.TotalIL    += abs(gIUVWQ12.U);
			}
			if(gUVCoff.Number >= 1024)
			{
				gUVCoff.ComperL = gUVCoff.Comper;
				gGetParVarable.IdSubStep = 5;
			}
			break;
			
		case 3:
		case 5:
            
			m_UData1 = (1024UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;//��Ƶ����������4�����ϵ�������
			if(gGetParVarable.IdSubStep == 5)
            {
                m_UData1 = (4096UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
                if( 16 < gInvInfo.InvTypeApply )
                {
                    m_UData1 = (3500UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;  //������ʱ������
                }
            }
            if( gMotorInfo.Current > m_MaxCur )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
            {
                m_UData1 = (((long)m_MaxCur) * (long)m_UData1) / gMotorInfo.Current;
            }

			gUVCoff.Number = 0;
			if(abs(gIUVWQ12.U) >= m_UData1)
			{
				gGetParVarable.IdSubStep++;
			}
			else
			{
			    if(gInvInfo.InvTypeApply > 16 )
                {         
                   gUVCoff.Comper += 1;         
                }
                else
                {
				   gUVCoff.Comper += 4;
                }
				if(gUVCoff.Comper >= gUVCoff.Rs_PRD)      // 125us
				{
			        gUVCoff.Comper = gUVCoff.Rs_PRD/2;  //�ɹ���ģ���жϳ�ʱ��ERR19
			        // ���Ա�ȱ��
			        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                    gError.ErrorInfo[1].bit.Fault1 = 11;
				}
				else
				{
					EALLOW;
					EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
					EPwm2Regs.CMPA.half.CMPA = gUVCoff.Rs_PRD - gUVCoff.Comper;
					EDIS;
				}
			}
			break;

		case 6:
			gUVCoff.TotalU += abs(gIUVWQ12.U);
			gUVCoff.TotalV += abs(gIUVWQ12.V);
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVolt += gUDC.uDCFilter;
				gUVCoff.TotalI += abs(gIUVWQ12.U);

				if(gUVCoff.Number >= 1024)
				{
                    DisableDrive();
					gUVCoff.Number = 0;
					m_UData1 = (gUVCoff.TotalU << 4) / (Uint)(gUVCoff.TotalV >> 8);
					gUVCoff.UDivVSave = ((Ulong)m_UData1 * 1000)>>12;

					m_UData2 = gUVCoff.Rs_PRD/2;
					//m_UData2 = C_INIT_PRD/2;
					m_UData1 = (gUVCoff.TotalVoltL<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.ComperL - m_UData2));
					m_UData1 = (gUVCoff.TotalVolt<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.Comper  - m_UData2)) - m_ULong1;
                    m_ULong1 =  m_ULong1 * 10;//��ѹ����һ��С���㣬����2��С���㣬����Ҫ����10
					//m_ULong1 = ��ѹ��(V)��10��2^14 
					m_UData2 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;
					m_ULong2 = ((Ulong)m_UData2 * (Ulong)gMotorInfo.Current)>>10;
					//m_ULong2 = ������(A)��10��2^4
					while((m_ULong2>>16) != 0)
					{
						m_ULong1 = m_ULong1>>1;
						m_ULong2 = m_ULong2>>1;
					}
					m_UData2 = m_ULong1/(Uint)m_ULong2;
                    m_UData2 = ((Ulong)m_UData2 * (Ulong)1000)>>11;
                    if(0 == gUVCoff.IdRsCnt)
                    {
                        gUVCoff.IdRsBak = m_UData2;
                        gGetParVarable.IdSubStep = 1;
                        gUVCoff.IdRsCnt++;
                        gUVCoff.IdRsDelay = 1000; //�ڶ��α�ʶ֮ǰ����ʱ2s
                    }
                    else
                    {
                        if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
                        {
                           gMotorExtReg.RsPm = m_UData2;
                        }
                        else
                        {
                    	    gMotorExtReg.R1 = m_UData2;
                        }
					    gGetParVarable.IdSubStep = 7;
                    }
				}
			}
			break;
            
		case 7:			
		default:
			DisableDrive();
  			gGetParVarable.IdSubStep = 1;
            gGetParVarable.ParEstMstep++; 
            InitSetPWM();                       //�ָ��޸ĵļĴ�������
			break;
	}	
}

/*******************************************************************
�������� ת�ӵ����©�б�ʶ����̬��ʶ��һ����
********************************************************************/
void RrLoIdentify(void)
{
    switch( gGetParVarable.IdSubStep )
    {
        case 1:	        //��һ�������ӵ����ʶ�����ʱ�׶Σ��ý׶���Ҫ���ĸ�ߵ�ѹ��
       	    gRrLoIdentify.WaitCnt++;
            EALLOW;
            PieCtrlRegs.PIEIER1.bit.INTx1 = 0;           //  ADC1INT
            EDIS;
			if( gRrLoIdentify.WaitCnt < 1000 )			//�ӳ�ʱ��2s
			{
			    RrWaitAddGetUdc();
			}
			else
			{
                SetPwmForRrLoId();              // configue ePWM module          			
			    InitRrLoVariable();
                EnableTimer();
                
	            gRrLoIdentify.PwmCompareValue = 200;			//��ʼ����120���
	            //�������Ӧ��AD����ֵ��AD����ֵ���Ϊ32767������һλ����16384�������������һ��
	            gRrLoIdentify.CurrentRatio = 8192L * (long)gMotorInfo.Current / gInvInfo.InvCurrent; 
	            gRrLoIdentify.CurrentRatio = (gRrLoIdentify.CurrentRatio > 8992)?8992:gRrLoIdentify.CurrentRatio;
                gRrLoIdentify.RrL0CycleCnt = 0;
	            gRrLoIdentify.RrAndRsAccValue = 0;
 	            gRrLoIdentify.LoAccValue = 0;                
				gGetParVarable.IdSubStep = 2;
			}
            break;
            
        case 2:                             //Ѱ�Һ��������ѹֵ�Ĺ���
            
            if(gRrLoIdentify.WaitCnt != 0)	//������ڵȴ��׶Σ�����ĸ�ߵ�ѹ
			{
				gRrLoIdentify.WaitCnt--;
				if(gRrLoIdentify.WaitCnt != 0)
				{
					RrWaitAddGetUdc();
				}   
				else
				{
					InitRrLoVariable();
                    EnableTimer();  
				}
			}
			else
			{                
				if((gRrLoIdentify.CurrentMax - 3200) > gRrLoIdentify.CurrentRatio)
				{	
                    gRrLoIdentify.PwmCompareValue -= 20;
					gRrLoIdentify.WaitCnt = 20;	//�������Ƚϴ��򽵵͵�ѹ��������
					RrGetVoltReBegin();
				}
				else if(1 == gRrLoIdentify.RrL07PulseOverSig)
				{
					//if(gRrLoIdentify.SampleTimes >= 14)
					//{
						if(gRrLoIdentify.CurrentMax > gRrLoIdentify.CurrentRatio)
						{
							gGetParVarable.IdSubStep = 3;	//���������������������STEP4
						}
						else							//����δ�������޸ĵ�ѹ����������
						{
							if((gRrLoIdentify.CurrentMax * 2) > gRrLoIdentify.CurrentRatio)
							{
									gRrLoIdentify.PwmCompareValue += 50;
							}
							else
							{
									gRrLoIdentify.PwmCompareValue += 100;
							}
							gRrLoIdentify.WaitCnt = 20;	//���������ѹ���ٷ�һ��
							RrGetVoltReBegin();
						}
					//}
					//else
					//{
					//	gRrLoIdentify.WaitCnt = 20;
					//	RrGetVoltReBegin();				//�Ƚϻ����ڼ��������ԣ���������
					//}
			    }
			}
            break;
            
        case 3:		                            //���ʵ�ѹ�Ѿ��ҵ�����ʼ��������Ĳ��������
			if(gRrLoIdentify.WaitCnt != 0)	    //������ڵȴ��׶Σ�����ĸ�ߵ�ѹ
			{
				gRrLoIdentify.WaitCnt--;
				if(gRrLoIdentify.WaitCnt != 0)
				{
					RrWaitAddGetUdc();
				}
				else
				{
					InitRrLoVariable();
                    EnableTimer();
				}
			}
			else
			{
				if(1 == gRrLoIdentify.RrL07PulseOverSig)
				{
					if(14 == gRrLoIdentify.SampleTimes)
					{								//��ʼ�����ʶ���
						RrLoCalVaule();
						if(6 <= gRrLoIdentify.RrL0CycleCnt)
						{
							gMotorExtReg.R2 = ((gRrLoIdentify.RrAndRsAccValue - gRrLoIdentify.RrAndRsMax - gRrLoIdentify.RrAndRsMin) / 4) - gMotorExtReg.R1;
							gMotorExtReg.L0 = (gRrLoIdentify.LoAccValue - gRrLoIdentify.LoMax - gRrLoIdentify.LoMin) / 4;
							if((gInvInfo.InvType > 18) && (gInvInfo.InvType < 23))
							{
								gMotorExtReg.R2 = ((long)gMotorExtReg.R2 * 768L)>>10;
							}
							else if(gInvInfo.InvType > 23)
							{
								gMotorExtReg.R2 = ((long)gMotorExtReg.R2 * 666L)>>10;
							}
							gGetParVarable.IdSubStep = 4;		//��ʶ������
						}
						else
						{
							gRrLoIdentify.WaitCnt = 20;
							RrGetVoltReBegin();				//û�����6�β�������Ҫ������
						}
					}
					else
					{
						gRrLoIdentify.WaitCnt = 20;
						RrGetVoltReBegin();					//�Ƚϻ����ڼ��������ԣ���������
					}
				}
			}           
            break;
            
        case 4:
        default:
	        DisableDrive();               
            EALLOW;
            PieCtrlRegs.PIEIER3.all = 1;                    //�ر�EPWM2�ж�
            PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//�ָ����жϵ��޸�
            EDIS;               
            gGetParVarable.IdSubStep = 1;
            gGetParVarable.ParEstMstep++;
	        InitSetPWM();         //�ָ�ת�ӵ����ʶʱ������Ĵ������޸�
   	        InitSetAdc();
	        SetInterruptEnable();	            
            break;
    }
}

/*******************************************************************
��������: 
    ��������PWMģ�飬׼��ת�ӵ����©�еı�ʶ

********************************************************************/
void SetPwmForRrLoId(void)
{
    EALLOW;
    PieVectTable.EPWM1_INT = &RrLoInPeriodAndCmpInt;    // �����������жϣ����ܸ��ıȽϼĴ���	
	EPwm1Regs.ETSEL.all = 0x0000A;		                //�����ж�
    PieVectTable.EPWM2_INT = &RrLoInPeriodAndCmpInt;    
    EPwm2Regs.ETSEL.all = 0x000C;                       //����EPWM2�ıȽ��ж� 
    EPwm2Regs.ETPS.all  = 1;
    
	EPwm1Regs.TBPRD = 12000; 
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;		// ��ʱֹͣTB����//
#ifdef DSP_CLOCK100
	EPwm1Regs.TBCTL.bit.CLKDIV = 1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 5;      // PWMʱ������Ϊ0.2us�����ϳ���һ��
#endif
#ifdef DSP_CLOCK60
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif
	EPwm1Regs.TBCTR = 0;	
    
	//EPwm1Regs.CMPB = 12000;				//��ʼ����ֵΪ200//
	EPwm1Regs.CMPA.half.CMPA = 200;
	EPwm1Regs.CMPCTL.all = 0x0100;		//�������CMPֵ//
	EPwm1Regs.AQSFRC.all = 0x000C0;		//U��������һֱ�رգ���������//
	EPwm1Regs.AQCSFRC.all = 0x00A;		//U��������һֱ�ر�//	
	EPwm1Regs.DBCTL.all = 0x0000;		//û������ʱ��//
	EPwm1Regs.ETCLR.bit.INT = 1;		//����жϱ�־//

	EPwm2Regs.TBPRD = 12000; 
    EPwm2Regs.TBCTL.bit.CTRMODE = 3;
#ifdef DSP_CLOCK100
	EPwm2Regs.TBCTL.bit.CLKDIV = 1;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 5;      //PWMʱ������Ϊ0.2us�����ϳ���һ��										/////////////PWM2//////////////
#endif
#ifdef DSP_CLOCK60
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.CMPA.half.CMPA = 200;
    EPwm2Regs.CMPCTL.all = 0x0100;
    EPwm2Regs.AQCTLA.all = 0;
	EPwm2Regs.AQSFRC.all = 0x00C0;	    
	EPwm2Regs.AQCSFRC.all = 0x0006;		//V������һֱ�ر�,����һֱ��//
	EPwm2Regs.DBCTL.all = 0x0000;		//û������ʱ��//
	EPwm2Regs.ETCLR.bit.INT = 1;

#ifdef DSP_CLOCK100
	EPwm3Regs.TBCTL.bit.CLKDIV = 1;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 5;      //PWMʱ������Ϊ0.2us�����ϳ���һ��                                        /////////////PWM3//////////////
#endif
#ifdef DSP_CLOCK60
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif   
	EPwm3Regs.TBPRD = 12000; 
    EPwm3Regs.AQCTLA.all = 0;
	EPwm3Regs.AQSFRC.all = 0x00C0;		    
	EPwm3Regs.AQCSFRC.all = 0x0006;		//W������һֱ�رգ�����һֱ��//
	EPwm3Regs.DBCTL.all = 0x0000;		//û������ʱ��//
    PieCtrlRegs.PIEIER3.all = 3;    
	EDIS;
}

/*******************************************************************
��������:
    ��ʼ��ת�ӵ����©�б�ʶ�������õ��ı���
    
********************************************************************/
void InitRrLoVariable(void)
{
    int m_index;
    
	gRrLoIdentify.WaitCnt = 0;
    for(m_index=0; m_index<14; m_index++)
    {
        gRrLoIdentify.IsSampleValue[m_index] = 0;
    }
	gRrLoIdentify.UdcVoltage = gUDC.uDCFilter;//���������ʶʱ�õ�ĸ�ߵ�ѹ
	gRrLoIdentify.CurrentMax = 0;
	gRrLoIdentify.SampleTimes = 0;
    gRrLoIdentify.RrL07PulseOverSig = 0;
}

/*******************************************************************
�������� ������ʱ�������ʹ��
********************************************************************/
void EnableTimer(void)
{
	EALLOW;
	EPwm1Regs.AQCTLA.all = 0x0021;		//����ʱ�õͣ��Ƚ�ʱ�ø�
	EPwm1Regs.AQCSFRC.all = 0x008;		//U������һֱ�ر�    
    EPwm1Regs.TBCTR = 0;
    EPwm2Regs.TBCTR = 0;
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;		//����ģʽ����������ʱ���Ĺ���
	EPwm2Regs.TBCTL.bit.CTRMODE = 0;
	EDIS; 
	EINT;								//���ж�    
    EnableDrive();	
}

/*******************************************************************
��������: 
    ת�ӵ����©�б�ʶʱ���������жϺͱȽ��ж��м������͵�ѹ
�ú����ϲ���ԭ�������ں���������̣���Ҫ�����Ƿ�Ա�ʶ��Ӱ�졣

********************************************************************/
interrupt void RrLoInPeriodAndCmpInt(void)
{
	long		m_Iu;
   	
   	EALLOW;
   	EPwm1Regs.ETCLR.bit.INT = 1;
    EPwm2Regs.ETCLR.bit.INT = 1;
   	EDIS;
    
	EINT;								
    RrWaitAddGetUdc();
    gRrLoIdentify.UdcVoltage = Filter4(gUDC.uDCFilter, gRrLoIdentify.UdcVoltage);
	m_Iu = (int)(ADC_IU - (Uint)32768);
	m_Iu =  (m_Iu - (long)gExcursionInfo.ErrIu)>>1;	//ȥ����Ư;Q14

	gRrLoIdentify.IsSampleValue[gRrLoIdentify.SampleTimes] = __IQsat(m_Iu, 32767, -32767);  
    if(gRrLoIdentify.CurrentMax < abs(m_Iu))
	{
		gRrLoIdentify.CurrentMax = abs(m_Iu);
	}
	gRrLoIdentify.SampleTimes++;  
    gRrLoIdentify.SampleTimes = __IQsat(gRrLoIdentify.SampleTimes, 15, 0);
    
	if(gRrLoIdentify.SampleTimes >= 14)
	{
		DisableDrive();						//����7�����壬��ر��������ʼ��
        EALLOW;
        EPwm1Regs.TBCTL.bit.CTRMODE = 3;	//ֹͣ����
		EPwm1Regs.TBCTR = 0;
		EPwm1Regs.AQCSFRC.all = 0x00A;		//U�������»ֱ�ر?/
		EPwm2Regs.TBCTL.bit.CTRMODE = 3;
        EPwm2Regs.TBCTR = 0;

        //SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.all &= 0xF8;    // disable ePWM clock
		EDIS;
		gRrLoIdentify.RrL07PulseOverSig = 1;	
    }    
    CBCLimitCurPrepare();
    DINT;
    
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;	// Acknowledge this interrupt
}

/*******************************************************************
�������� ����ʱ�ȴ������м��ĸ�ߵ�ѹ
********************************************************************/
void RrWaitAddGetUdc(void)
{
    Uint counter;
    
	EALLOW;
	ADC_RESET_SEQUENCE;		    //��λAD�ļ�����
	ADC_CLEAR_INT_FLAG;			//��������жϱ�־λ
	ADC_START_CONVERSION;		//�������AD
	EDIS;

    counter = 0;
	while(ADC_END_CONVERSIN == 0)   // �ȴ�ADת�����
    {
        counter ++;
        if(counter > 50)
        {
            gError.ErrorCode.all |= ERROR_TUNE_FAIL;    // ��г���ϣ��˳���г
            break;
        }
	}
    
    EALLOW;
    ADC_CLEAR_INT_FLAG;         // ���AD�ж�
    EDIS;
    
    GetUDCInfo();    
}

void RrGetVoltReBegin(void)
{
	DisableDrive();
    gRrLoIdentify.PwmCompareValue = __IQsat(gRrLoIdentify.PwmCompareValue,2000,1);
    
    EALLOW;
	EPwm1Regs.CMPA.half.CMPA = gRrLoIdentify.PwmCompareValue;
    EPwm2Regs.CMPA.half.CMPA = gRrLoIdentify.PwmCompareValue;

	EPwm1Regs.TBCTL.bit.CTRMODE = 3;		//ֹͣ����
	EPwm1Regs.TBCTR = 0;
	EPwm1Regs.AQCSFRC.all = 0x00A;		//U��������һֱ�ر�//
	EPwm2Regs.TBCTL.bit.CTRMODE = 3;
    EPwm2Regs.TBCTR = 0;

    //SysCtrlRegs.PCLKCR1.all |= 0x03;        // enable ePWM clockS
	EDIS;
}

/*******************************************************************
��������:
    ���ݲ������ĵ�����ѹֵ������ת�ӵ����©��
********************************************************************/
void RrLoCalVaule(void)
{
    int     m_IdVoltage,m_zeroVoltageTime,m_index,m_CurrMultiple;
    long    m_RrRsCalValue,m_LoCalVAlue,m_IsUdcVoltDiff,m_IsZeroVoltDiff,m_IsUdcVoltageSum,m_IsZeroVoltageSum;
    long    m_Value1,m_Value2;
    m_IsUdcVoltageSum = 0;
    m_IsUdcVoltDiff = 0;
    m_IsZeroVoltageSum = 0;
    m_IsZeroVoltDiff = 0;
	m_IdVoltage = ((long)gRrLoIdentify.UdcVoltage * 43690L)>>16;	//ĸ�ߵ�ѹ��2/3��ʵ�ʵ�ѹ��10��;
	m_zeroVoltageTime = 12000 - gRrLoIdentify.PwmCompareValue;		//�����ѹΪ0��ʱ��
    for(m_index=2;m_index<7;m_index++)
        m_IsUdcVoltDiff += ((long)gRrLoIdentify.IsSampleValue[2*m_index]-(long)gRrLoIdentify.IsSampleValue[2*m_index-1]);
    m_IsUdcVoltDiff = abs(m_IsUdcVoltDiff);     //6A  UDC�µĵ����仯
    for(m_index=2;m_index<7;m_index++)
        m_IsZeroVoltDiff += ((long)gRrLoIdentify.IsSampleValue[2*m_index+1]-(long)gRrLoIdentify.IsSampleValue[2*m_index]);
    m_IsZeroVoltDiff = abs(m_IsZeroVoltDiff);	//6B ��ѹΪ0�µ����仯
	for(m_index=3;m_index<13;m_index++)
        m_IsUdcVoltageSum += (long)abs(gRrLoIdentify.IsSampleValue[m_index]);
    m_IsUdcVoltageSum = m_IsUdcVoltageSum>>1;   //5*I_udc
    for(m_index=4;m_index<14;m_index++)
        m_IsZeroVoltageSum += (long)abs(gRrLoIdentify.IsSampleValue[m_index]);
    m_IsZeroVoltageSum = m_IsZeroVoltageSum>>1; //5*I_0
		
	m_Value1 = m_IsUdcVoltageSum + ((long long)m_IsZeroVoltageSum * (long long)m_IsUdcVoltDiff * (long long)m_zeroVoltageTime) /
                (m_IsZeroVoltDiff * (long)gRrLoIdentify.PwmCompareValue);
            //ʵ�ʵ������㷽ʽ  AD����ֵ*2*SQRT(2)*��Ƶ�������>>14

    m_Value2 = (m_Value1 * (llong)gInvInfo.InvCurrent)>>14;
            
    #ifdef TMS320F2808
    m_Value2 = ((llong)m_Value2 * 5793L)>>11; //*sqrt(2);
    #else
    m_Value2 = ((llong)m_Value2 * 6372L)>>11; //*sqrt(2); 
    #endif

    m_Value2 = ((llong)m_Value2 * (long)gInvInfo.CurrentCoff) / 1000;	  
    m_RrRsCalValue = ((long)m_IdVoltage * 50000L)/ m_Value2;	 //���ӵ����ת�ӵ���֮��
    m_LoCalVAlue = ((long long)m_RrRsCalValue * (long long)m_IsZeroVoltageSum * (long long)m_zeroVoltageTime) / (m_IsZeroVoltDiff * 50000L); //©�м��� 
    m_LoCalVAlue = m_LoCalVAlue>>1;    //©�м�����ϣ��ٶ���ת��©����ȣ�����2�õ�ת��©��


    
	if(0 == gRrLoIdentify.RrL0CycleCnt)//��¼��С���ֵ
	{
        gRrLoIdentify.LoMax = m_LoCalVAlue;
        gRrLoIdentify.LoMin = m_LoCalVAlue;
        gRrLoIdentify.RrAndRsMax = m_RrRsCalValue;
        gRrLoIdentify.RrAndRsMin = m_RrRsCalValue;
	}
	else
	{
	    gRrLoIdentify.LoMax = (m_LoCalVAlue > gRrLoIdentify.LoMax)?m_LoCalVAlue:gRrLoIdentify.LoMax;
        gRrLoIdentify.LoMin = (m_LoCalVAlue < gRrLoIdentify.LoMin)?m_LoCalVAlue:gRrLoIdentify.LoMin;
        gRrLoIdentify.RrAndRsMax = (m_RrRsCalValue > gRrLoIdentify.RrAndRsMax)?m_RrRsCalValue:gRrLoIdentify.RrAndRsMax;
        gRrLoIdentify.RrAndRsMin = (m_RrRsCalValue < gRrLoIdentify.RrAndRsMin)?m_RrRsCalValue:gRrLoIdentify.RrAndRsMin;
	}    
	gRrLoIdentify.RrAndRsAccValue += m_RrRsCalValue;
	gRrLoIdentify.LoAccValue += m_LoCalVAlue;
    gRrLoIdentify.RrL0CycleCnt++;
}

/************************************************************
	����Ӧ�����첽���������ĵ���������(����8KHz�ز�Ƶ�ʼ���)
	m_kp_m = m_l1*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_m = m_res*m_In*65536.0/(0.65*64*m_Un);
	
	m_kp_t = m_l1*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_t = m_res*m_In*65536.0/(1.3*64*m_Un);
	
	���գ����-mH������-A����ѹ-V������-ŷķ���ز�Ƶ��8KHz
	m_kp_m = m_l1*m_In*197/m_Un;
	m_ki_m = m_res*m_In*1575/m_Un;
	
	m_kp_t = m_l1*m_In*197/m_Un;
	m_ki_t = m_res*m_In*788/m_Un;
	
	���գ����-0.01mH������-0.01A����ѹ-V������-0.001ŷķ���ز�Ƶ��8KHz
	m_kp_m = m_l1*m_In/(51*m_Un);
	m_ki_m = m_res*m_In/(64*m_Un) = (m_res*m_In/m_Un)>>6;
	
	m_kp_t = m_l1*m_In/(51*m_Un);
	m_ki_t = m_res*m_In/(128*m_Un) = (m_res*m_In/m_Un)>>7;

	m_l1 = L1[1-(Lm*Lm/L1*L1)] = Lo + Lo*Lm/L1  ע�ⵥλӦ��Ϊ0.01mH
************************************************************/
void ImCalKpKi(void)
{
	Ulong tempL;
    Ulong tempL1;
    Ulong temp;

    tempL1 = gMotorExtReg.L0/10 + gMotorExtReg.LM;
    tempL  = gMotorExtReg.L0 * gMotorExtReg.LM / tempL1 ;    // 0.01mH
    tempL += gMotorExtReg.L0;
//
	temp = ((Ulong)tempL * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IdKp = (Ulong)temp * 1290L >> 10; // * 1.26
	gPmParEst.IqKp = gPmParEst.IdKp;
    // ��������
	gPmParEst.IdKi = (((Ulong)gMotorExtReg.R1 * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Votage)>>1;    // * 0.504
	gPmParEst.IqKi = gPmParEst.IdKi >> 1;    //
}

/*******************************************************************
��������:
    ���ص����ͻ��б�ʶ����̬��ʶ
********************************************************************/
void LmIoIdentify(void)
{
    //Uint m_AbsFreq;
    switch(gGetParVarable.IdSubStep)
    {
        case 1:            
            InitLmIoVaribale();   //��ʼ�����ر�ʶ��������Ҫ��WaitCnt��ѭ������
            gGetParVarable.IdSubStep++;
            
        case 2:         //���ر�ʶǰ�ĵȴ��׶Σ��ȴ�ת�ӵ����ʶʱ��������
            gLmIoIdentify.WaitCnt++;
            if( 800 < gLmIoIdentify.WaitCnt )
            {
                gLmIoIdentify.WaitCnt = 0;
                gGetParVarable.IdSubStep++;
			    gGetParVarable.StatusWord = TUNE_ACC;  //��ʼ����                
                EnableDrive();
                //
                gPGData.imPgEstTick = 0;
                gPGData.imDirAdder = 0;
                gPGData.imFreqErr = 0;
                gPGData.imFrqEncoder = 0;
                gLmIoIdentify.VFOvShock = gVFPar.VFOvShock;
            }
            break;
            
        case 3:         //ʵ�����Ƶ�ʴﵽ����Ƶ�ʵ�80%
            if(gMainCmd.FreqSyn >= (gLmIoIdentify.DestinationFreq - 11))
            {

                if(gLmIoIdentify.VFOvShock != 0)
                 { 
                   gLmIoIdentify.VFOvShock--;
                   gVFPar.VFOvShock = gLmIoIdentify.VFOvShock;
                    break;
                  }
                else
                 {
                    if(gLmIoIdentify.WaitCnt < 500)
                    {
                        gLmIoIdentify.WaitCnt ++; 
                        break; 
                    }
                 }
                // ��ʼ��������
                gPGData.imPgEstTick ++;
                // �����
                if((long)gFVCSpeed.SpeedEncoder * gMainCmd.FreqSyn < 0)
                {
                    //gPGData.PGDir = (gPGData.SpeedDir ^ 0x01);
                    gPGData.imDirAdder ++;
                }
                else
                {
                    gPGData.imDirAdder --;
                }
                // ������������
                gPGData.imFreqErr += (gMainCmd.FreqSyn - abs(gFVCSpeed.SpeedEncoder));
                gPGData.imFrqEncoder += abs(gFVCSpeed.SpeedEncoder);

                if(gPGData.imPgEstTick > 128)       // ����ж�
                {
                    int maxFrqErr;
                    
                    if(gPGData.imDirAdder > 50)
                    {
                        gPGData.PGDir = (gPGData.SpeedDir ^ 0x01);
                    }
                    else if(gPGData.imDirAdder > -50)
                    {
                        gPGData.PGErrorFlag = 1;        // ��Ϊδ�ӱ�����
                    }
                    // else encoder is ok
                    maxFrqErr = (long)gLmIoIdentify.DestinationFreq * 100L >> 10;    // 100/1024 = 10%
                    if((gPGData.imFreqErr >> 7) > maxFrqErr)    // 2^7 = 128
                    {
                        gPGData.PGErrorFlag = 2;        // ���ٲ��Ǻϣ���������������
                    }
                    if((gPGData.imFrqEncoder>>7) < maxFrqErr)   // 10% AimFrq
                    {
                        gPGData.PGErrorFlag = 1;        // ��Ϊδ��⵽������
                    }

                    gLmIoIdentify.WaitCnt = 0;
                    gLmIoIdentify.lImAccValue = 0;
                    gLmIoIdentify.lIsAccValue = 0;
                    gLmIoIdentify.DataSavedNum = 0;
			        gGetParVarable.IdSubStep++;
                    }
                }
            // ;
            break;          
            
        case 4:                 // �ȴ��ж�ִ��
              //gVFPar.VFOvShock = 0;
			if(gMainCmd.FreqSyn < (gLmIoIdentify.DestinationFreq - 20))
			{
                gGetParVarable.IdSubStep++; //��ʶ�������ٶȵ����趨ֵ���쳣�˳�
            }
            
	        if(120 <= gLmIoIdentify.WaitCnt)   
	        {
				gMotorExtReg.LM = (((gLmIoIdentify.LmAccValue * 10L) / gLmIoIdentify.WaitCnt) - (long)gMotorExtReg.L0) / 10;
				gMotorExtReg.I0 = gLmIoIdentify.IoAccValue / gLmIoIdentify.WaitCnt; //���ʹ���22������С����λ����ͬ
                #ifdef TMS320F28035
                gMotorExtReg.LM = ((long)gMotorExtReg.LM * 7447L) >> 13; //����1.1
                gMotorExtReg.I0 = ((long)gMotorExtReg.I0 * 9011L) >> 13; //����1.1
                #endif
	            //gGetParVarable.StatusWord = TUNE_SUCCESS;    //��ʼ����
	            gGetParVarable.StatusWord = TUNE_DEC;
                gLmIoIdentify.WaitCnt = 0;
				gGetParVarable.IdSubStep++;                    
			}
            break;
            
        case 5:
            if( 8 > gMainCmd.FreqSyn )
            {
                DisableDrive();
                gGetParVarable.ParEstMstep++;
                gGetParVarable.IdSubStep = 1;

                // ���������kp��ki
                ImCalKpKi();
            }            
            break;
            
    }

}

void InitLmIoVaribale(void)
{
    gLmIoIdentify.WaitCnt = 0;
    gLmIoIdentify.DestinationFreq = ((long)gMotorInfo.FreqPer<<3)/10;  //����Ƶ�ʵ�80%
    gLmIoIdentify.LmAccValue = 0;
    gLmIoIdentify.IoAccValue = 0;
}

/*******************************************************************
�������� ��������п��Ϳ��ص��������ۼ�
********************************************************************/

void LmIoCalValue(void)
{
    long    m_ComIm,m_ComLma,m_ComLmb,m_ComLmc;
	m_ComIm  =  gLmIoIdentify.lIsAccValue / 100; //Q16
	m_ComLma = ((gLmIoIdentify.lImAccValue / 200L ) * 1448L)>>10;  //*sqrt(2)
    m_ComLmb = (m_ComIm * m_ComIm)>>13;
	m_ComLmc = ((long)gOutVolt.Volt * m_ComLma) / m_ComLmb;//VOLTAGE*IM*SIN /{[(IM*SIN)^2+(IM*COS)^2] / 2^12}
	m_ComLmc = m_ComLmc * (long)gMotorInfo.Votage * 10L / (long)gInvInfo.InvCurrent;
    m_ComLmc = (m_ComLmc * 739L)>>13;
    m_ComLmc = m_ComLmc * 1000L / (long)gInvInfo.CurrentCoff;    
    m_ComLmb = (m_ComLmc * 10000L) / (long)gMainCmd.FreqReal;   			
	m_ComLmc = ((m_ComIm * (long)gInvInfo.InvCurrent)>>15) * (long)gInvInfo.CurrentCoff / 707L;//((AD*Iv)*Igain*2*2^0.5/1000)>>16
    gLmIoIdentify.LmAccValue += m_ComLmb;
    gLmIoIdentify.IoAccValue += m_ComLmc;
   	gLmIoIdentify.WaitCnt++;
}

/*******************************************************************
�������� ��ɵ�����ʸ���任�����ۼӵ���ֵ
********************************************************************/
void LmIoInPeriodInt(void)
{
    long    m_lIm,m_lIt,m_lIs;
    int     m_VoltagePhase;
    if(( 4 != gGetParVarable.IdSubStep )||
        ( 120 <= gLmIoIdentify.WaitCnt ))
    {       
        return;
    }
    m_VoltagePhase = gPhase.IMPhase>>16;
   	m_lIm = abs(((long)gExcursionInfo.IvValue * (long)qsin(m_VoltagePhase)                     
        		- (long)gExcursionInfo.IuValue * ((long)qsin(m_VoltagePhase - 21845)))>>15);//Q15 
	m_lIt = abs((long)gExcursionInfo.IvValue * (long)qsin(16384+m_VoltagePhase)
				- (long)gExcursionInfo.IuValue * (long)qsin(m_VoltagePhase - 5461)>>15);//Q15           
	m_lIs = ( 827L * (long)qsqrt(m_lIm * m_lIm + m_lIt * m_lIt))>>10;
    gLmIoIdentify.lIsValue = Filter16(m_lIs,gLmIoIdentify.lIsValue);       
    m_lIs = abs(((long)gLmIoIdentify.lIsValue * (long)qsin(16384 - gIAmpTheta.Theta))>>15);        
    gLmIoIdentify.lIsAccValue += gLmIoIdentify.lIsValue;
	gLmIoIdentify.lImAccValue += m_lIs;
 
    gLmIoIdentify.DataSavedNum++;
    if( 200 <= gLmIoIdentify.DataSavedNum )
    {
        LmIoCalValue();
        gLmIoIdentify.lIsAccValue = 0;
        gLmIoIdentify.lImAccValue = 0;
        gLmIoIdentify.DataSavedNum = 0;
    }    
}

void ChgParForEst(void)
{
    if(MOTOR_TYPE_PM != gMotorInfo.MotorType)
    {
        //gMainCmd.Command.bit.ControlMode = IDC_VF_CTL;
        gCtrMotorType = ASYNC_VF;
       if (500 < gInvInfo.InvTypeSet)
        {
         gBasePar.FcSet = 10;                            // �첽��������ʶ, 1140V��Ƶ�̶�Ϊ1.0KHz
        }
       else if (400 < gInvInfo.InvTypeSet)
        {
         gBasePar.FcSet = 20; 
        }
       else
        {
         gBasePar.FcSet = 30;                            // �첽��������ʶ, ��Ƶ�̶�Ϊ3.0KHz
        }
         gVFPar.VFWsComp = 0;
         gVFPar.VFLineType = 0;
         gVFPar.VFTorqueUp = 10;
         gPWM.SoftPWMTune = 0;
    }
    else                // pmsm
    {
       if (500 < gInvInfo.InvTypeSet)
       {
        gBasePar.FcSet =  10;                             // �첽��������ʶ, 1140V��Ƶ�̶�Ϊ1.0KHz
       }
       else if (400 < gInvInfo.InvTypeSet)
       {
        gBasePar.FcSet = 20; 
       }
       else
       {
        gBasePar.FcSet = (gBasePar.FcSet < 30) ? 30 : gBasePar.FcSet;       // min 3.0KHz
       }
    }
}

