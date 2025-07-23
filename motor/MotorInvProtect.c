/************************************************************
�ļ�����:����ͱ�Ƶ����������
�ļ��汾�� 
���¸��£� 

*************************************************************/
#include "DSP2803x_EPwm_defines.h"
#include "MotorInvProtectInclude.h"
// // ȫ�ֱ�������
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//������豣������
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;

//��Ƶ���͵���Ĺ��ر�
Uint const gInvOverLoadTable[9] =      /*����9������9%������Сֵ:115%�������ֵ:187%*/
{
		36000,				//115%��Ƶ������   		1Сʱ����  
		18000,				//124%��Ƶ������	  	30���ӹ���
        6000,				//133%��Ƶ������	  	10���ӹ���
        1800,				//142%��Ƶ������	  	3���ӹ��� 
        600,				//151%��Ƶ������   		1���ӹ���  
        200,				//160%��Ƶ������   		20�����   
        120,				//169%��Ƶ������   		12�����    
        20,					//178%��Ƶ������   		6�����    ��Ϊ178% 2S����
        20,					//187%��Ƶ������   		2�����    
};
Uint const gInvOverLoadTableForP[9] =       /*����9������4%������Сֵ:105%�������ֵ:137%*/
{
		36000,				//105%��Ƶ������   		1Сʱ����  
		15000,				//109%��Ƶ������	  	25���ӹ���
        6000,				//113%��Ƶ������	  	10���ӹ���
        1800,				//117%��Ƶ������	  	3���ӹ��� 
        600,				//121%��Ƶ������   		1���ӹ���  
        300,				//125%��Ƶ������   		30�����   
        100,				//129%��Ƶ������   		10�����    
        30,					//133%��Ƶ������   		3�����    
        10,					//137%��Ƶ������   		1�����    
};

//��Ƶ�������ۼ���������ϵ��
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%��Ƶ������    0.7������������
        (65536L*60/8),		//10%��Ƶ������   0.8������������
        (65536L*60/9),		//20%��Ƶ������   0.9������������
        (65536L*60/10),		//30%��Ƶ������   1.0������������
        (65536L*60/11),		//40%��Ƶ������   1.1������������
        (65536L*60/13),		//50%��Ƶ������   1.3������������
        (65536L*60/16),		//60%��Ƶ������   1.6������������
        (65536L*60/19),		//70%��Ƶ������   1.9������������
        (65536L*60/24),		//80%��Ƶ������   2.4������������
        (65536L*60/34),		//90%��Ƶ������   3.4������������
		(65536L*60/56),		//100%��Ƶ������  5.6������������
};

#define C_MOTOR_OV_TAB_NUM      7
Uint const gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%�������  1Сʱ20���ӹ���
		24000,				//125%�������  40���ӹ���
		9000,				//135%�������  15���ӹ��� 
		3000,				//145%�������  5���ӹ���
		1200,				//155%�������  2���ӹ���
		1200,				//165%�������  2���ӹ���
		1200				//175%�������  2���ӹ���
};
#define C_MOTOR_OV_MAX_CUR      1750
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//��������ۼ���������ϵ��
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%�������    3.0������������
        (65536L*60/40),		//10%�������   4.0������������
        (65536L*60/50),		//20%�������   5.0������������
        (65536L*60/60),		//30%�������   6.0������������
        (65536L*60/70),		//40%�������   7.0������������
        (65536L*60/80),		//50%�������   8.0������������
        (65536L*60/90),		//60%�������   9.0������������
        (65536L*60/100),	//70%�������   10.0������������
        (65536L*60/110),	//80%�������   11.0������������
        (65536L*60/120),	//90%�������   12.0������������
		(65536L*60/130),	//100%�������  13.0������������
};

// //�¶����߱�
Uint const gTempTableP44X[23] =
{
		624,614,603,590,576,561,		//6
		544,525,506,485,464,442,		//6
		419,395,373,350,327,305,		//6
		284,264,244,226,208				//5
};
Uint const gTempTableP8XX[23] =
{
		475,451,426,400,374,348,		//6
		323,299,275,253,232,212,		//6
		193,176,161,146,133,121,		//6
		110,100,91, 83, 76				//5
};
Uint const gTempTableBSMXX[23] =
{
		486,461,435,412,386,361,		//6
		337,313,291,269,248,228,		//6
		209,193,176,161,148,135,		//6
		123,113,103,94, 86				//5		
};
Uint const gTempTableSEMIKON[23] =
{
		558,519,480,451,418,392,		//6
		369,350,331,314,302,288,		//6
		278,269,262,254,247,243,		//6
		237,233,229,226,224				//5
};
Uint const gTempTableWAIZHI[23] =
{
		655,609,563,518,473,430,		//6
		389,350,314,282,251,224,		//6
		199,177,158,140,124,111,		//6
		99, 88, 78, 70, 62				//5		
};

// //�ļ��ڲ���������
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//�¶ȼ��
void	OutputPhaseLoseDetect(void);			//���ȱ����
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//����ȱ����
void	ControlFan(void);						//���ȿ���
void	OverLoadProtect(void);					//���ر���
void	CBCLimitCurProtect(void);
void 	SetCBCEnable(void);
void 	SetCBCDisable(void);
void	LoadLoseDetect(void);
void 	BufferResProtect(void);

/************************************************************
	��Ƶ����������
************************************************************/
void InvDeviceControl(void)			
{
	//if(gADC.ResetTime < 500)
    {
    //    return;                         //AD�����ȶ���ʼ
    }
	SoftWareErrorDeal();				//������
	TemperatureCheck();					//�¶ȼ��
	OutputPhaseLoseDetect();			//���ȱ����
	InputPhaseLoseDetect();				//����ȱ����
	ControlFan();						//���ȿ���
	OverLoadProtect();					//���ر���

    CBCLimitCurPrepare();					//��������������
	CBCLimitCurProtect();				//����������µĹ����ж�
	
	LoadLoseDetect();					//���ش���
	BufferResProtect();
}

/*************************************************************
	������ϴ���
ͣ��״̬Ҳ���ܱ��������ϡ���ѹ����
*************************************************************/
void SoftWareErrorDeal(void)					
{
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // Ƿѹ
	   ((gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH))
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//Ƿѹ״̬�²��ж��������
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x40000000L;				//���ϸ�λ�����нǶȷ����仯
        }
    }
    
// ��ʼ���ϸ�λ
	if(gSubCommand.bit.ErrorOK == 1) 			
	{
		gError.ErrorCode.all = 0;
        gError.ErrorInfo[0].all = 0;
        gError.ErrorInfo[1].all = 0;
        gError.ErrorInfo[2].all = 0;
        gError.ErrorInfo[3].all = 0;
        gError.ErrorInfo[4].all = 0;
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
		{
			gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			//gPhase.IMPhase += 16384;
			EALLOW;
			EPwm2Regs.TZCLR.bit.OST = 1;
			EPwm3Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.INT = 1;
			EDIS;
		}
	}

// ��ʼ�ж��������
	if(gLineCur.CurBaseInv > 10240)			// 2.5���������ж�
	// (��������Ϊ2�������������ʵ�ʵĹ�������û��)
	{
	    DINT;
		DisableDrive();
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
        gError.ErrorInfo[0].bit.Fault1 = 2;
		gLineCur.ErrorShow = gLineCur.CurPer;
        EINT;
	}
    
	if((gUDC.uDCBigFilter > gInvInfo.InvUpUDC) || //��ѹ�ж�,ʹ�ô��˲���ѹ
	    GetOverUdcFlag())
	{
	    DisableDrive(); //ͣ��Ҳ������ѹ�������û������ѹ����
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        gError.ErrorInfo[0].bit.Fault2 = 2;
	}
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //Ƿѹ�ж�,ʹ�ô��˲���ѹ
	{
	    DisableDrive();
        gMainStatus.RunStep = STATUS_LOW_POWER;
        gMainStatus.SubStep = 0;
		DisConnectRelay();	
		gError.ErrorCode.all |= ERROR_LOW_UDC;
        gMainStatus.StatusWord.bit.LowUDC = 0;
	}
}

/************************************************************
�¶ȼ��,˵������:

1������С�ڵ���10       (2.2Kw ����)
	�¶�����:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2�������ڣ�11��18֮�䣩 (11Kw �� 30Kw)
	�¶�����:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3�������ڣ�19��26֮�䣩 (37Kw �� 200Kw,���Ұ���37, ������200)
	�¶Ȳ������ã�		TABLE_WAIZHI

4�����ʹ��ڵ���27		TABLE_BSMXX

5�����ʹ��ڵ���27ʱ���¶Ȳ�����·��ͬ����Ҫ����3.3V��3V��ת����

6�����ʹ��ڵ���19��85�ȱ�������������95�ȱ�����

������з�ʽ��ÿ4����һ�����ݣ���ʼ��ַ����Ϊ12�ȣ�����ֵΪAD����ֵ
AD����ֵ��AD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...׼���¶ȱ�,690V���¶Ȳ����ͱ�����380V���ʹ���27��һ��
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
        mType = gInvInfo.InvTypeApply + 27;
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

// �����¶ȵ��ȷ��
    if(mType >= 19)
    {
        m_ErrTemp = 85;
    }
    else
    {
        m_ErrTemp= 95;
    }
    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
       m_ErrTemp = 70;
       gInvInfo.TempType = 1;  /*1140V ���µ���¶������趨 2011.5.7 L1082*/
    }
// �¶����ߵ�ѡ��
	if(mType <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else    
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(mType <= 18)    // [11, 18]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
	}
    else if(mType <= 26)    // [19, 26]
    {
        m_pTable = (Uint *)gTempTableWAIZHI;
    }
    else        // mType >= 27
    {
        m_pTable = (Uint *)gTempTableBSMXX;
    }

// Ӳ���������Լ�������·������
    m_TempAD = (gTemperature.TempAD>>6);

#ifdef  TMS320F2808
    if(mType >= 27) // 3V��3.3v ������·��ת��
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// ��ʼ��ѯ�¶ȱ�
	m_IndexLow  = 0;
	m_IndexHigh = 22;
	m_Index = 11;
	if(m_TempAD >= m_pTable[m_IndexLow])
	{	
		mType = 12 * 16;
	}
	else if(m_TempAD <= m_pTable[m_IndexHigh])
	{
		mType = 100 * 16;
	}
	else
	{
		m_LimtCnt = 0;
		while(m_LimtCnt < 7)
		{
			m_LimtCnt++;					//������ѭ��
			if(m_TempAD == m_pTable[m_Index])
			{
				mType = (m_Index<<6) + (12 * 16);
				break;
			}
			else if(m_IndexLow+1 == m_IndexHigh)
			{
				mType = (m_IndexLow<<6) + (12 * 16) 
							+ ((m_pTable[m_IndexLow] - m_TempAD)<<6)
				                /(m_pTable[m_IndexLow] - m_pTable[m_IndexHigh]);
				break;
			}
			
			if(m_TempAD > m_pTable[m_Index])
			{
				m_IndexHigh = m_Index;
			}
			else
			{
				m_IndexLow = m_Index;
			}
			m_Index = (m_IndexLow + m_IndexHigh)>>1;
		}
	}
	if(mType - gTemperature.TempBak >= 8)			//�¶ȱ仯����0.5�ȲŸ�ֵ
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// ��ʼ���¶��жϺͱ�������
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//���ȱ���
	}

/*
    	//...׼���¶ȱ�,690V���¶Ȳ����ͱ�����380V���ʹ���27��һ��
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)  m_Temp = gInvInfo.InvTypeApply + 27;
     else                                             m_Temp = gInvInfo.InvTypeApply;
	m_TempAD = (gTemperature.TempAD>>6);
	m_ErrTemp = 95;
	if(m_Temp <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(m_Temp <= 18)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
	}
	else if(m_Temp >= 27)
	{
		m_pTable = (Uint *)gTempTableBSMXX;
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15;   //���ʹ���26ʱ���¶Ȳ�����·��ͬ����Ҫ����
                                                            //3V��3.3v ������·��ת��
	}
	else
	{
		m_ErrTemp = 85;				//���ʹ��ڵ���19��85�ȱ���
		m_pTable = (Uint *)gTempTableWAIZHI;
	}
*/
}

/************************************************************
	���ȱ����
************************************************************/
void OutputPhaseLoseDetect(void)			
{
	Uint m_U,m_V,m_W;
	Uint m_Max,m_Min;
    
	if((gSubCommand.bit.OutputLost == 0) ||	
	   (gMainCmd.FreqReal < 80) ||					//0.8Hz���²����
	   (gMainCmd.Command.bit.StartDC == 1) ||		//ֱ���ƶ�״̬�²����
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//�������л����ٶ������׶�
	    (gMainStatus.RunStep != STATUS_SPEED_CHECK)))
	{
		OutputLoseReset();
		return;
	}    

	if((gPhaseLose.Time < (Ulong)DSP_CLOCK * 1000000L) && (gPhaseLose.Cnt < 50000))
	{
		return;
	}

	m_U = gPhaseLose.TotalU/gPhaseLose.Cnt;
	m_V = gPhaseLose.TotalV/gPhaseLose.Cnt;
	m_W = gPhaseLose.TotalW/gPhaseLose.Cnt;

	m_Max = (m_U > m_V) ? m_U : m_V;
	m_Min = (m_U < m_V) ? m_U : m_V;
	m_Max = (m_Max > m_W) ? m_Max : m_W;
	m_Min = (m_Min < m_W) ? m_Min : m_W;

	if((m_Max > 500) && (m_Max/m_Min > 10))
	{
		gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
        gPhaseLose.errMaxCur = m_Max;
        gPhaseLose.errMinCur = m_Min;
	}
	OutputLoseReset();
}

void OutputLoseAdd(void)		//���ȱ�����ۼӵ�������
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time   += gPWM.gPWMPrdApply;
    //gPhaseLose.Time += 2000L * DSP_CLOCK;
	gPhaseLose.Cnt++;
}
void OutputLoseReset(void)		//���ȱ���⸴λ�Ĵ�������
{
	gPhaseLose.Cnt = 0;
	gPhaseLose.TotalU = 0;
	gPhaseLose.TotalV = 0;
	gPhaseLose.TotalW = 0;
	gPhaseLose.Time   = 0;
}

/************************************************************
	����ȱ����
ȱ����ԭ��: 
    �̵��������ź���ȱ���źŸ��ϣ� ������ʱ��һֱΪ�ߣ�
    ��PLһֱΪ�ͣ���̵���û�����ϣ�
    ��PLΪ��������ȱ�ࣻ    
2808Dsp PL�ź���VOE(Ӳ����ѹ)�ź���Ӳ���ϸ�����һ��VOE��Чʱ������Ϊ��(0)��
28035Dsp��VOE�ź��ǵ����ģ�ȱ�����ΪPL�źţ�
18.5kw ���ϲ���ȱ���·
************************************************************/
void InputPhaseLoseDetect(void)			
{
	if((gSubCommand.bit.InputLost == 0) ||                      //���л�ת�ٸ���״̬�ż��
	   ((STATUS_RUN!=gMainStatus.RunStep)&&(STATUS_SPEED_CHECK!=gMainStatus.RunStep))||
	   (gInvInfo.InvTypeApply < gInLose.ForeInvType))
	{
		gInLose.Cnt = 0;
		gInLose.UpCnt = 0;
		gInLose.ErrCnt = 0;
		gInLose.CntRes = 0;
		gInLose.UpCntRes = 0;
		return;
	}
    
	if(PL_INPUT_HIGH)           // PL�Ǹߵ�ƽ		
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// ����500ms��PL�͵�ƽ�ж�Ϊ�̵�������
	{
		gInLose.CntRes++;	
		if(gInLose.CntRes >= 250)       // 500ms
		{
			if(gInLose.UpCntRes >= 249)
			{
			    gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;
			}
			gInLose.CntRes = 0;
			gInLose.UpCntRes = 0;
		}
	}

	gInLose.Cnt++;	
	if(gInLose.Cnt < 500)       //  ȱ����1secһ��ѭ�� 
    {
        return;
    }

	if((gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1sec��PL���ڵ͵�ƽ��Ϊȱ�෽��
	{
		gInLose.ErrCnt++;
		if(gInLose.ErrCnt >= 2)
		{
			gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
			gInLose.ErrCnt = 0;
		}
	}
	else
	{
		gInLose.ErrCnt = 0;
	}
	gInLose.Cnt = 0;
	gInLose.UpCnt = 0;
}

/************************************************************
	���ȿ���
1��	�ϵ绺��״̬������1�����ڣ����Ȳ����У�
2��	����״̬���������У�
3��	ֱ���ƶ��ȴ��ڼ䣬��������
4��	����¶ȵ���40�ȣ�����ֹͣ���¶ȸ���42�ȷ������У�40��42��֮�䲻�仯��
5)	��������������10��Źر�
************************************************************/
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
		TurnOnFan();
	}
	else if(gTemperature.Temp < 40)
	{
		gFanCtrl.RunCnt++;
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
			TurnOffFan();
		}
	}
}

/************************************************************
	��Ƶ���͵�����ر���
************************************************************/
void OverLoadProtect(void)				
{
	Ulong m_LDeta = 0;
	Uint m_Cur,m_Data,m_Index,m_CurBaseInv;
	Uint m_TorCurBottom,m_TorCurUpper,m_TorCurStep,m_TorCurData;
    Uint *m_TorCurLine;

    m_CurBaseInv = gLineCur.CurBaseInv;
    if( 28 <= gInvInfo.InvTypeApply )
    {
        //m_CurBaseInv = (s32)gLineCur.CurBaseInv * (s32)gInvInfo.InvCurrent / gInvInfo.InvOlCurrent;
        m_CurBaseInv = (long)gLineCur.CurBaseInv * (long)gInvInfo.InvCurrent / gInvInfo.InvCurrForP;
    }
    
	gOverLoad.FilterInvCur = Filter16(m_CurBaseInv,gOverLoad.FilterInvCur);
	gOverLoad.FilterMotorCur = Filter16(gLineCur.CurPer,gOverLoad.FilterMotorCur);
	gOverLoad.FilterRealFreq = Filter16(gMainCmd.FreqReal,gOverLoad.FilterRealFreq);

	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		gOverLoad.InvTotal.all = 0;
		gOverLoad.MotorTotal.all = 0;
		gOverLoad.Cnt = 0;
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	gOverLoad.Cnt++;
	if(gOverLoad.Cnt < 5)		
	{
        return;		    //ÿ10ms�ж�һ��
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//ѡ���������

    if(1 == gInvInfo.GPType)        //G�ͻ���������
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;
        m_TorCurUpper   = 1870;
        m_TorCurStep    = 90;
        m_TorCurData    = 20;
    }
    else                            //P�ͻ���������
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1370;
        m_TorCurStep    = 40;
        m_TorCurData    = 10;
    }

	////////////////////////////////////////////////////////////////
	//��ʼ�жϱ�Ƶ���Ĺ���
	m_Cur = ((Ulong)gOverLoad.FilterInvCur * 1000L) >> 12;
	if(m_Cur < m_TorCurBottom)
    {
		if(gOverLoad.InvTotal.half.MSW < 10)
		{
			gOverLoad.InvTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)       /*С�ڱ�Ƶ������������յ�ǰ������С���������ۼ���*/
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[m_Cur/100 + 1];
		}
	}
	else
	{
		if(gOverLoad.FilterRealFreq < 500)		//���� = ����/[0.9*(f/5)+0.1]
		{
			m_Data = gOverLoad.FilterRealFreq * 13 + 26214;
			m_Cur  = (((Ulong)m_Cur)<<15)/m_Data;
		}
		if(m_Cur >= m_TorCurUpper)
		{
			m_Data = m_TorCurData;
		}
		else
		{
			m_Index = (m_Cur - m_TorCurBottom)/m_TorCurStep;
			m_Data = *(m_TorCurLine + m_Index) -
			         (((long)(*(m_TorCurLine + m_Index)) - (*(m_TorCurLine + m_Index + 1))) * 
					  (long)(m_Cur - m_TorCurBottom - m_Index * m_TorCurStep))/m_TorCurStep;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.InvTotal.all += m_LDeta;
	
		if(gOverLoad.InvTotal.half.MSW >= 36000)
		{
			gOverLoad.InvTotal.half.MSW = 36000;
			//AddOneError(ERROR_INV_OVER_LOAD,1);

			gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
			gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
            gError.ErrorInfo[1].bit.Fault1 = 2;
		}
	}
    //��Ƶ������Ԥ��������
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}

	////////////////////////////////////////////////////////////////
	//��ʼ�жϵ���Ĺ���
	//if(gMainCmd.SubCmd.bit.MotorOvLoadEnable == 0)
	if(gSubCommand.bit.MotorOvLoad == 0)
	{
		gOverLoad.MotorTotal.all = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}
	//m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	//m_Cur = ((Ulong)m_Cur * 100L)/gBasePar.MotorOvLoad;
	//m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	m_Cur = ((Ulong)m_Cur * 100L)/gComPar.MotorOvLoad;          // ���ݹ��ر���ϵ�����������������
                                                            	//Ȼ���øñ���������ѯ���ر�������
	m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	if(m_LDeta >= (C_MOTOR_OV_MAX_CUR * 256L))
	{
		m_Cur = C_MOTOR_OV_MAX_CUR;
	}
	else
	{
		m_Cur = m_LDeta>>8;
	}

	if(m_Cur < C_MOTOR_OV_MIN_CUR)
	{
		if(gOverLoad.MotorTotal.half.MSW < 10)
		{
			gOverLoad.MotorTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)                   /*С��100%��������յ��������������*/
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[m_Cur/100 + 1];  
		}
	}
	else
	{
		if(m_Cur >= C_MOTOR_OV_MAX_CUR)
		{
			m_Data = gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM - 1];
		}
		else
		{
			m_Index = (m_Cur - C_MOTOR_OV_MIN_CUR)/C_MOTOR_OV_STEP_CUR;
			m_Data = gMotorOverLoadTable[m_Index] -
			         ((long)(gMotorOverLoadTable[m_Index] - gMotorOverLoadTable[m_Index+1]) * 
					  (long)(m_Cur - C_MOTOR_OV_MIN_CUR - m_Index * C_MOTOR_OV_STEP_CUR))/C_MOTOR_OV_STEP_CUR;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.MotorTotal.all += m_LDeta;

		if(gOverLoad.MotorTotal.half.MSW > 36000)
		{
			gOverLoad.MotorTotal.half.MSW = 36000;
			//AddOneError(ERROR_MOTOR_OVER_LOAD,1);
			gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
			gError.ErrorCode.all |= ERROR_MOTOR_OVER_LOAD;
		}		
	}
    //�������Ԥ��������   
	//if(gOverLoad.MotorTotal.half.MSW > gBasePar.PerMotorOvLoad * 360)
	if(gOverLoad.MotorTotal.half.MSW > gComPar.PerMotorOvLoad * 360)
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
	}
}


/************************************************************
	����������״̬�µĹ��ر���
���ܳ���������ʱ�䳬��500ms��������Ӧ250��2ms
************************************************************/
void CBCLimitCurProtect(void)
{
	int     m_CurU,m_CurV,m_CurW,m_Coff;
	int	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if(gSubCommand.bit.CBCEnable == 1)
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//��������������
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//ȡ������������
	}
		
	//��ʼ�ֱ���������������ֵ�Ļ���
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (int)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (int)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (int)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//��ʼ�ж��Ƿ���һ��������������5��
	if(m_CurU > 9267)	gCBCProtect.CntU++;             // 9267 = 4096 * 1.414 * 1.6
	else				gCBCProtect.CntU = gCBCProtect.CntU>>1;
	gCBCProtect.CntU = (gCBCProtect.CntU > 3000)?3000:gCBCProtect.CntU;

	if(m_CurV > 9267)	gCBCProtect.CntV++;
	else				gCBCProtect.CntV = gCBCProtect.CntV>>1;
	gCBCProtect.CntV = (gCBCProtect.CntV > 3000)?3000:gCBCProtect.CntV;

	if(m_CurW > 9267)	gCBCProtect.CntW++;
	else				gCBCProtect.CntW = gCBCProtect.CntW>>1;
	gCBCProtect.CntW = (gCBCProtect.CntW > 3000)?3000:gCBCProtect.CntW;
    
	if(gMainCmd.FreqReal > 20)
	{
		gCBCProtect.CntU = 0;
		gCBCProtect.CntV = 0;
		gCBCProtect.CntW = 0;
	}

	if((gCBCProtect.CntU > 2500) || 		//��������������, �κ�һ�����5000ms
	   (gCBCProtect.CntV > 2500) || 
	   (gCBCProtect.CntW > 2500) )
	{
		gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
        gError.ErrorInfo[1].bit.Fault1 = 1;
	}
    
	if(gCBCProtect.EnableFlag == 0)
	{
		gCBCProtect.TotalU = 0;
		gCBCProtect.TotalV = 0;
		gCBCProtect.TotalW = 0;
		return;
	}
    
    // �����¶Ⱥ��趨CBCʱ���ȷ��CBCʱ��
    m_Limit = 500;        //������ 500MS��ȡ���������� 2011.5.7 L1082��
   /*
    if(gTemperature.Temp < 40)  // 40��
    {
        m_Limit = gCBCProtect.maxCBCTime * 100;     // ��λת��: 0.1sec -> 1ms
    }
    else if(gTemperature.Temp > 60)
    {
        m_Limit = gCBCProtect.minCBCTime * 100;
    }
    else
    {
        m_Limit = (gCBCProtect.maxCBCTime - gCBCProtect.minCBCTime) * 100;
        m_Limit = (long)m_Limit * (60 - gTemperature.Temp)/20 + gCBCProtect.minCBCTime * 100;
    }
    m_Limit *= 3;       // ����3��ͬʱ�ضϣ� 
                        // 28035 �϶�������ͬʱ����
    */
    
	m_Add = 2;
	m_Sub = 1;
	if(gMainStatus.RunStep == STATUS_STOP)
    {
        m_Sub = 2;
    }

	if(gCBCProtect.Flag.bit.CBC_U == 1)   //����U������Ļ���
	{
		gCBCProtect.TotalU += m_Add;
	}
	else //if(m_CurU < 8688)					//С��1.5����Ƶ����ֵ�������ۼ�ֵ�ݼ� : 4096*1.5*sqrt(2)
	{
		gCBCProtect.TotalU -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//����V������Ļ���
	{
		gCBCProtect.TotalV += m_Add;
	}
	else //if(m_CurV < 8688)					//С��1.5����Ƶ���������ۼ�ֵ�ݼ�
	{
		gCBCProtect.TotalV -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//����W������Ļ���
	{
		gCBCProtect.TotalW += m_Add;
	}
	else //if(m_CurW < 8688)					//С��1.5����Ƶ���������ۼ�ֵ�ݼ�
	{
		gCBCProtect.TotalW -= m_Sub;
	}

	gCBCProtect.Flag.all = 0;

	//��������ֵ�޷�
	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
    if(m_Max >= m_Limit)      //��������40�Ź���
    {
        gError.ErrorCode.all |= ERROR_TRIP_ZONE;
    }
}

/*************************************************************
	��������������
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1��������
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_ENABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_ENABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_ENABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
#endif

	EDIS;
}
/*************************************************************
	�ر�����������
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;

#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1��������
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_DISABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_DISABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_DISABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
#endif

	EDIS;
}

/*************************************************************
	������ش���

������״̬������С�ڵ�������5%��������100ms��Ϊ�ǵ���
ͬ�����������ر���
*************************************************************/
void LoadLoseDetect(void)
{
	Uint m_Limit,m_LimitFreq;

	//�����жϷ�ֵΪ����������5%��
	m_Limit = (gLoadLose.ChkLevel * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Current << 2; // *4096/1000
	m_Limit = (long)m_Limit * 1447L >> 10;              // * sqrt(2)
	m_Limit = (m_Limit < 20) ? 20 : m_Limit;
	m_LimitFreq = ((Ulong)gMotorInfo.FreqPer * 1638) >> 15;

	if((gMotorInfo.MotorType == MOTOR_TYPE_PM)              // ͬ�����������ؼ��
        ||(gSubCommand.bit.LoadLose != 1)
        || (abs(gMainCmd.FreqSyn) < m_LimitFreq)			//����Ƶ��5%���²����
	    || (gMainCmd.Command.bit.StartDC == 1)
	    || (gMainCmd.Command.bit.StopDC  == 1)				//ֱ���ƶ�״̬�����
	    || (gMainStatus.RunStep != STATUS_RUN)				//������״̬�����
	    || (abs(gIUVWQ12.U) >= m_Limit)                     //�κ�һ���������200 (5%) ��Ϊ������
	    || (abs(gIUVWQ12.V) >= m_Limit) 
	    || (abs(gIUVWQ12.W) >= m_Limit) 
	    )							
	{
		gLoadLose.ErrCnt = 0;
		gMainStatus.StatusWord.bit.OutOff = 0;
		return;
	}
	
	gLoadLose.ErrCnt++;
    if((gLoadLose.ErrCnt<<1) > (gLoadLose.ChkTime*100))     // ���ؼ��ʱ��ȷ��
	//if(gLoadLose.ErrCnt > 50)
	{
		gLoadLose.ErrCnt = 50;
        gError.ErrorCode.all |= ERROR_LOAD_LOST;
		gMainStatus.StatusWord.bit.OutOff = 1;
	}
}
/*************************************************************
	������豣������
	
�����Ľ���Ƿѹ״̬����Ϊ�ǻ���������
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000)			//������豣������
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000,0);					
}

