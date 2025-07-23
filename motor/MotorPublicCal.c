/****************************************************************
�ļ����ܣ�����������������Ʒ�ʽ�޹صĶ���ģ��
�ļ��汾��
�������ڣ�

****************************************************************/
#include "MotorPublicCalInclude.h"

/************************************************************/
/********************ȫ�ֱ�������****************************/
DC_BRAKE_STRUCT			gDCBrake;	//ֱ���ƶ��ñ���
BRAKE_CONTROL_STRUCT	gBrake;		//�ƶ���������ñ���
SHORT_GND_STRUCT		gShortGnd;
JUDGE_POWER_LOW			gLowPower;	    //�ϵ绺���ж�ʹ�����ݽṹ
/************************************************************
	�ϵ�Եض�·�ж�
************************************************************/
void RunCaseShortGnd(void)
{
	switch(gMainStatus.SubStep)
	{
		case 1:
            gMainStatus.PrgStatus.bit.PWMDisable = 1;
			gShortGnd.Comper = SHORT_GND_PERIOD;
			gShortGnd.ocFlag = 0;
			gShortGnd.BaseUDC = gUDC.uDC;
			gShortGnd.ShortCur = 0;

			EALLOW;
			EPwm1Regs.TBPRD = SHORT_GND_PERIOD;
			EPwm1Regs.CMPA.half.CMPA = SHORT_GND_PERIOD;
			EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = SHORT_GND_PERIOD;
			EPwm3Regs.TBPRD = SHORT_GND_PERIOD;


			EPwm1Regs.AQCSFRC.all = 0x08;
			//EPwm1Regs.AQCSFRC.bit.CSFB = 1;	//ǿ�ƹر�ĳЩ�ű�
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//EPwm2Regs.AQCSFRC.bit.CSFA = 2;
			//EPwm2Regs.AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//EPwm3Regs.AQCSFRC.bit.CSFA = 2;
			//EPwm3Regs.AQCSFRC.bit.CSFB = 2;
			EPwm1Regs.DBCTL.all = 0;
			EPwm2Regs.DBCTL.all = 0;
			EPwm3Regs.DBCTL.all = 0;
			//EPwm1Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			//EPwm2Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			//EPwm3Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			EDIS;

			gMainStatus.SubStep = 2;
			break;

		case 2:
			gMainStatus.SubStep = 3;
			EnableDrive();
			break;

		case 3:
			if((gShortGnd.ocFlag != 0) || 
			  (abs(gShortGnd.ShortCur) > (30 * 32)) ||	//410*2 Ϊ��ֵ���������Ƶ�������10%�Ƚ�
			  (gUDC.uDC > gShortGnd.BaseUDC + 650))	//ĸ�ߵ�ѹ����65V
			{							
				//�ϵ�Եض�·����
				DisableDrive();
				gError.ErrorCode.all |= ERROR_SHORT_EARTH;
				gMainStatus.SubStep = 4;
				break;
			}
			
			if(gShortGnd.Comper <= SHORT_GND_CMPR_INC)
			{
				gMainStatus.SubStep = 4;
			}
			else
			{
				gShortGnd.Comper -= SHORT_GND_CMPR_INC;
				EALLOW;
				EPwm1Regs.CMPA.half.CMPA = gShortGnd.Comper;
				EDIS;
			}
			break;

		case 4:
			DisableDrive();
			EALLOW;
			EPwm1Regs.DBCTL.all = 0x0007;
			EPwm2Regs.DBCTL.all = 0x0007;
			EPwm3Regs.DBCTL.all = 0x0007;
			//EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			//EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			//EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			EPwm1Regs.AQCSFRC.all = 0x0;
			EPwm2Regs.AQCSFRC.all = 0x0;
			EPwm3Regs.AQCSFRC.all = 0x0;
		
			EDIS;
			gMainStatus.StatusWord.bit.ShortGndOver = 1;
			gMainStatus.RunStep = STATUS_STOP;
            gMainStatus.PrgStatus.bit.PWMDisable = 0;
			gMainStatus.SubStep = 0;
			if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
			{
				gMainStatus.ErrFlag.bit.OvCurFlag = 0;
				EALLOW;
				EPwm1Regs.TZCLR.bit.INT = 1;
				EDIS;
			}
			break;

		default:
			break;
	}	
}

/************************************************************
	��Ƶ�����ϵ紦����̣��ж�ĸ�ߵ�ѹ�ȶ��������ϵ绺�����
�ж����ݣ�1��ĸ�ߵ�ѹ����Ƿѹ�� 
       && 2��ĸ�ߵ�ѹû�������� 
       && 3������200ms
		  4���ж�ĸ�ߵ�ѹ�ȶ�������ʱ200ms������ͣ��״̬��
************************************************************/
void RunCaseLowPower(void)
{
	Uint uDCLowLimt;

	uDCLowLimt = gInvInfo.InvLowUDC + 200;
	if(INV_VOLTAGE_220V == gInvInfo.InvVoltageType)
	{
		uDCLowLimt = gInvInfo.InvLowUDC + 160;
	}

	switch(gMainStatus.SubStep)
	{
		case 2://�ж�ĸ�ߵ�ѹ����Ƿѹ��&&��ѹû������&&����200ms
			if((gUDC.uDCFilter > uDCLowLimt) && 
			   (gUDC.uDCFilter <= gLowPower.UDCOld))
			{
				if((gLowPower.WaiteTime++) >= 100)
				{
					gExcursionInfo.EnableCount = 199;
					gMainStatus.SubStep ++;	
					gLowPower.WaiteTime = 0;			
				}
			}
			else
			{
				gLowPower.WaiteTime = 0;
				gLowPower.UDCOld = gUDC.uDCFilter;
			}

			break;

		case 3:	//200ms��ʱ
			if(((gLowPower.WaiteTime++) >= 100) && 
			   (gExcursionInfo.EnableCount >= 200))
			{
				gMainStatus.RunStep = STATUS_STOP;
				gMainStatus.SubStep = 1;
                gMainStatus.StatusWord.bit.LowUDC = 1;
				gMainStatus.StatusWord.bit.RunEnable = 1;
        		if((gError.ErrorCode.all & ERROR_LOW_UDC) == ERROR_LOW_UDC)
                {
                    gError.ErrorCode.all = 0;                   //Ƿѹ����Ҫ�������?
		            if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)  //�޸�Ƿѹ���޷���������жϵĴ���
		            {
			            gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			            EALLOW;
			            EPwm2Regs.TZCLR.bit.OST = 1;
			            EPwm3Regs.TZCLR.bit.OST = 1;
			            EPwm1Regs.TZCLR.bit.OST = 1;
			            EPwm1Regs.TZCLR.bit.INT = 1;
			            EDIS;
		            }     
        		}
				ConnectRelay();				
				gBuffResCnt += 30000;
				gPhase.IMPhase = GetTime() << 28;	//�ϵ�����ѡ���ʼ��λ
			}
			break;

		default:
			DisConnectRelay();	
            gError.ErrorCode.all |= ERROR_LOW_UDC;				//�����־
        	gMainStatus.StatusWord.bit.LowUDC = 0;
			gMainStatus.StatusWord.bit.StartStop = 0;
			gLowPower.WaiteTime = 0;
			gLowPower.UDCOld = gUDC.uDCFilter;
			gMainStatus.SubStep = 2;
			break;
	}
}

/************************************************************
	�ж�UVW�����Ƿ���������״̬���������ñ�־
************************************************************/
void CBCLimitCurPrepare(void)
{
	EALLOW;
	if(EPwm1Regs.TZFLG.bit.CBC == 1) 		//��ʾU�ദ��������״̬
	{
		EPwm1Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_U = 1;
	}
	
	if(EPwm2Regs.TZFLG.bit.CBC == 1) 		//��ʾV�ദ��������״̬
	{
		EPwm2Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_V = 1;
	}

	if(EPwm3Regs.TZFLG.bit.CBC == 1) 		//�ʾW�ദ��������״̬
	{
		EPwm3Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_W = 1;
	}
	EDIS;
}
/*************************************************************
	ѡ����������е����ֵ
*************************************************************/
Uint MaxUVWCurrent(void)
{
	Uint m_IU,m_IV,m_IW;

	m_IU = abs(gIUVWQ24.U>>12);
	m_IV = abs(gIUVWQ24.V>>12);
	m_IW = abs(gIUVWQ24.W>>12);

	m_IU = (m_IU >= m_IV)?m_IU:m_IV;
	m_IU = (m_IU >= m_IW)?m_IU:m_IW;

	return m_IU;
}
/*************************************************************
	�����жϴ�����򣨿������жϣ���ƽ������

28035 Ҳ�����ǹ�ѹ�жϣ���Ҫ����жϣ�
28035 �ڷ�����ѹ�󣬻�Ƶ���Ľ�����жϣ� ��֤����û�����Ե�����
*************************************************************/
void HardWareErrorDeal()
{
#ifdef TMS320F28035 // ��Ҫ�ж��Ƿ����
    int sum = 0, i;
    
    gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//�����˹����жϵı�־

    for(i = 0; i < 10; i++)     // ��Ҫ2us�� ĸ�߿϶��ǲ����
    {
        sum += GpioDataRegs.AIODAT.bit.AIO2;
    }
    if(sum < 5)                // io��Ϊ�ͣ� �ж�Ϊ��ѹ��
    {
        HardWareOverUDCDeal();
        return;         // ��ѹ��AD�ж��д���, 2ms���д���
    }
    
#else
	gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//�����˹����жϵı�־
#endif

    if(gMainStatus.RunStep == STATUS_SHORT_GND)
	{
		gShortGnd.ocFlag = 1;						//�ϵ�Եض�·
	}
	else if((gError.ErrorCode.all & ERROR_OVER_CURRENT) != ERROR_OVER_CURRENT)
	{
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
        gError.ErrorInfo[0].bit.Fault1 = 1;
		gLineCur.ErrorShow = MaxUVWCurrent();	//Ӳ����������¼���ϵ���
	}

	EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
	EPwm2Regs.TZCLR.bit.OST = 1;
	EPwm3Regs.TZCLR.bit.OST = 1;
	EDIS;
    
    return;
}

/*************************************************************
	��ѹ�жϴ�����򣨲��������жϣ������ش�����
*************************************************************/
void HardWareOverUDCDeal(void)					
{
    /*
	if(gUDC.uDC < gInvInfo.InvUpUDC - 300)
	{
		return;									//ĸ�ߵ�ѹ�ȹ�ѹ���30V�������󱨾�
	}	                                              //����AD��������Ҳ���ܵ��²�����
    */
	DisableDrive();								//�������
	if(gMainStatus.RunStep == STATUS_SHORT_GND)
	{
		gShortGnd.ocFlag = 2;						//�ϵ�Եض�·���׶εı�־
	}
	else 
	{
		gError.ErrorCode.all |= ERROR_OVER_UDC;				//��ѹ����
        gError.ErrorInfo[0].bit.Fault2 = 1;
	}
    
    EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
	EPwm2Regs.TZCLR.bit.OST = 1;
	EPwm3Regs.TZCLR.bit.OST = 1;
	EDIS;

}

/************************************************************
ֱ���ƶ��������ز������У�ͨ��PI����������ֱ���ƶ��µ������?
ֱ���ƶ���ʱ��ֻ��һ�࿪�عܶ���
************************************************************/
void RunCaseDcBrake(void)		
{
	int m_BrakeCur;
	
	if(gMainCmd.Command.bit.StartDC == 1)
	{
		//m_BrakeCur = (((long)gComPar.StartDCBrakeCur)<<12)/100;
		m_BrakeCur = gComPar.StartDCBrakeCur * 41;			//4096/100 ~= 41
	}
	else if(gMainCmd.Command.bit.StopDC == 1)
	{
		//m_BrakeCur = (((long)gComPar.StopDCBrakeCur)<<12)/100;
		m_BrakeCur = gComPar.StopDCBrakeCur * 41;			//4096/100 ~= 41
	}

	gDCBrake.Time++;
	if(gDCBrake.Time < 2)		//ֱ���ƶ���ǰ���ģ����ն��ӵ���͵��������ѹ
	{
		gOutVolt.Volt = ((Ulong)m_BrakeCur * (Ulong)gMotorExtPer.R1)>>16;
		gDCBrake.PID.Total = ((long)gOutVolt.Volt<<16);
	}
	else						//ͨ��PI����������ֱ���ƶ�����
	{
		gDCBrake.Time = 10;
		gDCBrake.PID.Deta = m_BrakeCur - (int)gLineCur.CurPer;
        gDCBrake.PID.KP   = 1600/16;
		//gDCBrake.PID.KP   = 1600;
		gDCBrake.PID.KI   = 300;
        if( 16 < gInvInfo.InvTypeApply )  //����ֱ���ƶ�ʱKI��Сһ�룬��ֹ�����񵴡�
        {
            gDCBrake.PID.KI = 150;
        }
        gDCBrake.PID.QP = 0;
        gDCBrake.PID.QI = 0;
		gDCBrake.PID.KD   = 0;
		gDCBrake.PID.Max  = 4096;
		gDCBrake.PID.Min  = 0;
		PID((PID_STRUCT *)&gDCBrake.PID);
		gOutVolt.Volt = gDCBrake.PID.Out>>16;		
	}
	gOutVolt.VoltApply = gOutVolt.Volt;  	
                                        //ֱ���ƶ�����£����޸������ѹ��λ��, ��ѹ��λ������
}

/************************************************************
	�ƶ��������
************************************************************/
void BrakeResControl(void)
{
    //Uint m_UData, m_UData1, m_UData2, m_UData3;
/* 
// �����ƶ����迪ͨ�͹ضϴ������ز������жϴ�����
    //m_UData = (Ulong)PWM_CLOCK * 1000 / gPWM.gPWMPrd;   //  2ms������PWM����
    //m_UData = 4;
    m_UData = 1;
	m_UData2 = 100 - gComPar.BrakeCoff;
	m_UData3 = (m_UData2<gComPar.BrakeCoff)?m_UData2:gComPar.BrakeCoff;     // min
	m_UData2 = (m_UData2>gComPar.BrakeCoff)?m_UData2:gComPar.BrakeCoff;     // max
	
	if(m_UData3 == 0)	
	{
		m_UData1 = 65535;
		m_UData  = 0;
	}
	else
	{
		m_UData1 = ((Ulong)m_UData * (Ulong)m_UData2)/m_UData3;		
	}

	if(gComPar.BrakeCoff < 50)      // swap m_UData1 with m_UData
	{
		m_UData3 = m_UData1;
		m_UData1 = m_UData;
		m_UData  = m_UData3;
	}
	gBrake.OnCnt  = m_UData1;
	gBrake.OffCnt = m_UData;
*/
    //Uint max, min;

    if(gComPar.BrakeCoff == 100)
    {
        gBrake.OnCnt = 65535;
        gBrake.OffCnt = 0;
    }
    else if(gComPar.BrakeCoff == 0)
    {
        gBrake.OnCnt = 0;
        gBrake.OffCnt = 65535;
    }
    else if(gComPar.BrakeCoff <= 50)
    {
        gBrake.OnCnt = 1;
        gBrake.OffCnt = (100 - gComPar.BrakeCoff) / gComPar.BrakeCoff;  // ������Ȼ����ȷ�����Ǽ�
    }
    else    // 50-100
    {
        gBrake.OnCnt = (gComPar.BrakeCoff) / (100 - gComPar.BrakeCoff);
        gBrake.OffCnt = 1;
    }
    
// 
	if(gBrake.OnCnt == 0)
	{
		TurnOffBrake();
		return;
	}

    gBrake.VoltageLim = gVFPar.ovPoint * gVFPar.ovPointCoff;            // �ƶ����趯������ǹ�ѹ���Ƶ����
    
// �жϱ�����Ӧ�ÿ�ͨ���ǹض�
	if(gUDC.uDC < (gBrake.VoltageLim + 90))	
	{
		gBrake.Flag &= 0x0FFFC;		//���0��1bit
		gBrake.Cnt = 0;
	}
	else if((gBrake.Flag & 0x01) == 0)      //��ͨ��һ�ģ���0��1bit
	{
		if(gUDC.uDC > (gBrake.VoltageLim + 110))	
		{
			gBrake.Flag |= 0x03;		
			gBrake.Cnt = 0;
		}
	}
	else        // on or off
	{
		gBrake.Cnt++;
		if((gBrake.Flag & 0x02) == 0)       // off
		{
			if(gBrake.Cnt > gBrake.OffCnt)
			{
				gBrake.Flag |= 0x02;
				gBrake.Cnt = 0;
			}
		}
		else        // bit1 != 0, switch on
		{
			if(gBrake.Cnt > gBrake.OnCnt)
			{
				gBrake.Flag &= 0x0FFFD;
				gBrake.Cnt = 0;
			}
		}
	}

// ��ʼִ���ƶ�����ĵ�ͨ�͹ض�
	if((gBrake.Flag & 0x02) == 0x02)        // bit1
	{
		TurnOnBrake();				//...��ͨ		
	}
	else
	{
		TurnOffBrake();				//...�ض�		
	}
}

