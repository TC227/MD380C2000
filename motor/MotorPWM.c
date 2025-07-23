/****************************************************************
�ļ����ܣ�PWM���Ͳ��ֵĳ��򣬰����������������㡢VF�µ��Ʒ�����VC��SVPWM���㡢DSPģ��BUG���
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
#include "MotorPwmInclude.h"

// // ȫ�ֱ������� 
OUT_VOLT_STRUCT			gOutVolt;
PWM_OUT_STRUCT			gPWM;
Uint 					gRatio;			//����ϵ��

Uint 	EPWMFlagU;						//���ڽ��EPWMģ��BUG�ı���
Uint	EPWMFlagV;
Uint	EPWMFlagW;

Uint 	iTrig1_Cpwm[12] = {0,0,1,1,1,1,2,2,2,2,0,0};	// U,V,W ��һ���������
Uint 	iTrig1_Dpwm[12] = {0,2,2,1,1,0,0,2,2,1,1,0};
Uint 	iTrig2_Pwm[12]  = {1,1,0,0,2,2,1,1,0,0,2,2};	// U,V,W �����м��ȵ���
Uint 	iTrig3_Cpwm[12] = {2,2,2,2,0,0,0,0,1,1,1,1}; 	// U,V,W ��һ��������խ
Uint 	iTrig3_Dpwm[12] = {2,0,1,2,0,1,2,0,1,2,0,1};

/************************************************************
��������:��
�������:��
����λ��:��ѭ��֮ǰ
��������:��
��������:��ʼPWMģ��
************************************************************/
void InitSetPWM(void)
{
	EALLOW;
	/////////////PWM1//////////////
//Set the Time-Base (TB) Module
	EPwm1Regs.TBPRD = C_INIT_PRD; 
	EPwm1Regs.CMPB = EPwm1Regs.TBPRD - gADC.DelayApply;
	EPwm1Regs.TBPHS.all = 0;
	EPwm1Regs.TBCTL.all = 0xE012;
   
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = PWM_CLK_DIV;
	//EPwm1Regs.TBCTL.bit.PHSDIR = TB_UP;
//Set the Counter-compare (CC) Module
	EPwm1Regs.CMPCTL.all = 0x0100;
	//EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	//EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	//EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	//EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
	EPwm1Regs.CMPA.half.CMPA = C_INIT_PRD/2;
//Set the Action-qualifier (AQ) Module
	EPwm1Regs.AQCTLA.all = 0x0090;
	//EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
	//EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//Set the Dead-Band Generator (DB) Module
	EPwm1Regs.AQSFRC.all  = 0x0;
	EPwm1Regs.AQCSFRC.all = 0x0;	
	EPwm1Regs.DBCTL.all = 0x0007;
	//EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	//EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; 
	EPwm1Regs.DBFED = C_MAX_DB;
	EPwm1Regs.DBRED = C_MAX_DB;
//Set the PWM-chopper (PC) Module
//Set the Trip-zone (TZ) Module
#ifdef TMS320F2808
	EPwm1Regs.TZCLR.all = 0x07;
	//EPwm1Regs.TZSEL.all = 0x0104;
	EPwm1Regs.TZSEL.all = 0x010E;
	//EPwm1Regs.TZSEL.bit.OSHT1 = TZ_ENABLE;	//�����źŶ�PWM1�ķ���
	//EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		//EPWM1��������
	EPwm1Regs.TZCTL.all = 0x0005;
	//EPwm1Regs.TZCTL.bit.TZA = 1;
	//EPwm1Regs.TZCTL.bit.TZB = 1;
	EPwm1Regs.TZEINT.all = 0x0004;
	//EPwm1Regs.TZEINT.bit.OST = TZ_ENABLE;		//���ù����ж�
#else
    EPwm1Regs.TZCLR.all = 0x07;
    EPwm1Regs.TZSEL.all = 0x0102;       // OHS1:TZ1;
                                        // CBC2:TZ2;
    EPwm1Regs.TZCTL.all = 0x0005;
    EPwm1Regs.TZEINT.all = 0x0004;
#endif

//Set the Event-trigger (ET) Module	
	EPwm1Regs.ETCLR.bit.INT = 1;				//��������жϱ�־
	EPwm1Regs.ETSEL.all = 0x0F09;
	//EPwm1Regs.ETSEL.bit.INTEN = 1;
	//EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
	//EPwm1Regs.ETSEL.bit.SOCAEN = 1;
	//EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTRD_CMPB;	//COMPAR_B���½�������ADC
	EPwm1Regs.ETPS.all = 0x0101;
	//EPwm1Regs.ETPS.bit.INTPRD = 1;
	//EPwm1Regs.ETPS.bit.SOCAPRD = 1;				//ÿһ�¼�����һ��AD

	/////////////PWM2//////////////
//Set the Time-Base (TB) Module
	EPwm2Regs.TBPRD = C_INIT_PRD; 
	EPwm2Regs.TBPHS.all = 0;
	EPwm2Regs.TBCTL.all = 0xE006;
    //EPwm2Regs.TBCTL.bit.FREE_SOFT = 3;
	//EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
	//EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
	//EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
	//EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;	//��PWM1ͬ���ź�Ϊ���ͬ���ź�
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = PWM_CLK_DIV;
	//EPwm2Regs.TBCTL.bit.PHSDIR = TB_UP;
//Set the Counter-compare (CC) Module
	EPwm2Regs.CMPCTL.all = 0x0100;
	//EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	//EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	//EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	//EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
	EPwm2Regs.CMPA.half.CMPA = C_INIT_PRD/2;
//Set the Action-qualifier (AQ) Module
	EPwm2Regs.AQCTLA.all = 0x0090;
	//EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
	//EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//Set the Dead-Band Generator (DB) Module
	EPwm2Regs.AQSFRC.all  = 0x0;
	EPwm2Regs.AQCSFRC.all = 0x0;	
	EPwm2Regs.DBCTL.all = 0x0007;
	//EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	//EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; 
	EPwm2Regs.DBFED = C_MAX_DB;
	EPwm2Regs.DBRED = C_MAX_DB;
//Set the PWM-chopper (PC) Module
//Set the Trip-zone (TZ) Module
#ifdef TMS320F2808
	//EPwm2Regs.TZSEL.all = 0x0102;
	EPwm2Regs.TZSEL.all = 0x010E;
	//EPwm2Regs.TZSEL.bit.OSHT1 = TZ_ENABLE;
	//EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm2Regs.TZCTL.all = 0x0005;
	//EPwm2Regs.TZCTL.bit.TZA = 1;
	//EPwm2Regs.TZCTL.bit.TZB = 1;
#else
    EPwm2Regs.TZSEL.all = 0x0102;       // OHS1:TZ1;
                                        // CBC2:TZ2;
    EPwm2Regs.TZCTL.all = 0x0005;
    EPwm2Regs.TZEINT.all = 0x0004;
#endif

//Set the Event-trigger (ET) Module	
    EPwm2Regs.ETSEL.all = 0;
    EPwm2Regs.ETPS.all  = 0;
    
	/////////////PWM3//////////////
//Set the Time-Base (TB) Module
	EPwm3Regs.TBPRD = C_INIT_PRD; 
	EPwm3Regs.TBPHS.all = 0;
	EPwm3Regs.TBCTL.all = 0xE036;
    //EPwm3Regs.TBCTL.bit.FREE_SOFT = 3;
	//EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
	//EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
	//EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
	//EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;	//������ͬ���ź�
	EPwm3Regs.TBCTL.bit.HSPCLKDIV = PWM_CLK_DIV;
	//EPwm3Regs.TBCTL.bit.PHSDIR = TB_UP;
//Set the Counter-compare (CC) Module
	EPwm3Regs.CMPCTL.all = 0x0100;
	//EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	//EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	//EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	//EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
	EPwm3Regs.CMPA.half.CMPA = C_INIT_PRD/2;
//Set the Action-qualifier (AQ) Module
	EPwm3Regs.AQCTLA.all = 0x0090;
	//EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
	//EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
//Set the Dead-Band Generator (DB) Module
	EPwm3Regs.AQSFRC.all  = 0x0;
	EPwm3Regs.AQCSFRC.all = 0x00;	
	EPwm3Regs.DBCTL.all = 0x0007;
	//EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	//EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; 
	EPwm3Regs.DBFED = C_MAX_DB;
	EPwm3Regs.DBRED = C_MAX_DB;
//Set the PWM-chopper (PC) Module
//Set the Trip-zone (TZ) Module
#ifdef TMS320F2808
	//EPwm3Regs.TZSEL.all = 0x0108;
	EPwm3Regs.TZSEL.all = 0x010E;
	//EPwm3Regs.TZSEL.bit.OSHT1 = TZ_ENABLE;
	//EPwm3Regs.TZSEL.bit.CBC4 = TZ_ENABLE;
	EPwm3Regs.TZCTL.all = 0x0005;
	//EPwm3Regs.TZCTL.bit.TZA = 1;
	//EPwm3Regs.TZCTL.bit.TZB = 1;
#else
    EPwm3Regs.TZSEL.all = 0x0102;       // OHS1:TZ1;
                                        // CBC2:TZ2;
    EPwm3Regs.TZCTL.all = 0x0005;
    EPwm3Regs.TZEINT.all = 0x0004;
#endif

//Set the Event-trigger (ET) Module	
	EDIS;
}

/*************************************************************
˵���������ѹ��λgPhase.OutPhaseΪ0��ʱ�򣬱�ʾU���ѹ����ʱ��
��ˣ���Ҫ�����ҷ�ʽ���������ѹ��
gPWM.U��С����ʾU���ѹ���
*************************************************************/
/************************************************************
	�������ͳ��򣨵�ǰƵ�ʱȽϵͣ�ֱ�ӵ���ͬ�����Ƴ���
************************************************************/
/*
void SquarePWMAngleCal(void)
{

}
*/

/*************************************************************
	VC����ʱ�򣬷���PWM��(ֱ�Ӱ������ҷ�ʽ���Ƶõ�)
*************************************************************/
void OutPutPWMVC(void)
{
	int   	SinU,SinV,SinW,Comp3;
	int   	phase,phase3;
	int     m_Ratio;
    
	phase = gPhase.OutPhase;
	phase3= 16384 - phase;
	phase3= phase3 + (phase3<<1);
	Comp3 = qsin(phase3)>>2;
    //Comp3 = 0;

	SinU  = qsin(16384 - phase) + Comp3;
	phase = phase - 21845;
	SinV  = qsin(16384 - phase) + Comp3;
	phase = phase - 21845;
	SinW  = qsin(16384 - phase) + Comp3;

	m_Ratio = ((long)gRatio * 4710)>>12;			//��1.15ϵ��
	SinU = 4096 - (((long)m_Ratio * (long)SinU) >> 15);
	SinV = 4096 - (((long)m_Ratio * (long)SinV) >> 15);
	SinW = 4096 - (((long)m_Ratio * (long)SinW) >> 15);

	gPWM.U = (((long)SinU * (long)gPWM.gPWMPrdApply)>>13);
	gPWM.V = (((long)SinV * (long)gPWM.gPWMPrdApply)>>13);
	gPWM.W = (((long)SinW * (long)gPWM.gPWMPrdApply)>>13);

	DeadBandComp();
	SendPWM();
}

/************************************************************
	VF����ʱ�򣬷���PWM��(���տռ�ʸ�����õ�)

���з�����ʽ��CPWM��DPWM��������
ѡ��ʽ��
1. ����ֶ�ѡ������ѭ�ֶ�ѡ��
2. ѡ���Զ��л�������ѭ280���л�������
�Զ��л�ԭ��
      1. ���Ϊͬ�����ƣ������ز�С��1.5kHz���̶�ΪCPWM��
      2. ���Ϊ�첽���ƣ������ز�����1.5kHz���̶�ΪDPWM��
      3. �Զ��л�����12.5~15Hz���ͻ���
************************************************************/
void OutPutPWMVF(void)
{
	Uint 	m_iSec30;		
	Uint 	m_iSec60;
	Uint 	m_iRamainPhase;
	Uint 	m_FlagDPWMInverse;
	Ulong*	m_pUVW;

	Uint 	SinAlfa,SinAlfa60;
	Uint	m_iT1Length,m_iT2Length,m_iZeroLength,m_TotalLength;
	Uint 	m_UVWSel1,m_UVWSel2;
	Uint	phase;

	phase = gPhase.OutPhase;// + 32768;		//ȷ��gPhase.OutPhase=0��ӦU���ѹ���ֵ
	phase = (phase > 65531)?65531:phase;	//����������������
	//phase = __IQsat((Uint)gPhase.OutPhase, 65531, 0);
    
//...����30�ȷ�Χ��������
	m_iSec30 = phase/5461;
	if (m_iSec30 > 11)
    {
        m_iSec30 = 0;
    }
	m_iSec60 = m_iSec30 >> 1;

	m_iRamainPhase = phase - (m_iSec30 * 5461);	
	if((m_iSec30 & 0x01)  == 1) //judge it's odd or even
	{
		if(gPWM.PWMModle == MODLE_CPWM)  //CPWM
		{
			m_iRamainPhase = 5461 + m_iRamainPhase;
		}
		else
		{
			m_iRamainPhase = 5461 - m_iRamainPhase;
		}
	}

	m_FlagDPWMInverse = 0;
	if(gPWM.PWMModle == MODLE_DPWM)
	{
		if((((m_iSec30+1)>>1) & 0x01) == 1) m_FlagDPWMInverse = 1;
	}
	else
	{
		if((m_iSec60 & 0x01) == 1)	m_iRamainPhase = 10922 - m_iRamainPhase;	
	}

	SinAlfa60 = qsin(10922 - m_iRamainPhase);
	SinAlfa = qsin(m_iRamainPhase);

	m_iT1Length = ((long)SinAlfa60 * (long)gRatio)>>15;
	m_iT1Length = ((long)m_iT1Length * (long)gPWM.gPWMPrdApply)>>12;

	m_iT2Length = ((long)SinAlfa * (long)gRatio)>>15;
	m_iT2Length = ((long)m_iT2Length * (long)gPWM.gPWMPrdApply)>>12;

	m_TotalLength = m_iT1Length + m_iT2Length;
	if(m_TotalLength > gPWM.gPWMPrdApply)		
	{
		m_iT1Length = ((Ulong)m_iT1Length * (Ulong)gPWM.gPWMPrdApply)/m_TotalLength;
		m_iT2Length = gPWM.gPWMPrdApply - m_iT1Length;
		m_iZeroLength = 0;
	}										//ȷ��T1+T2<=T
	gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE;
	if(gPWM.PWMModle == MODLE_CPWM)
	{
		m_iZeroLength = (Uint)((gPWM.gPWMPrdApply - m_iT1Length - m_iT2Length)>>1);
		m_UVWSel1 = iTrig1_Cpwm[m_iSec30];	//Ԥ�����ò��λ��
		m_UVWSel2 = iTrig3_Cpwm[m_iSec30];
	}
	else
	{
		m_iZeroLength = 0;
		m_UVWSel1 = iTrig1_Dpwm[m_iSec30];	//Ԥ�����ò��λ��
		gPWM.gZeroLengthPhase = (ZERO_LENGTH_PHASE_SELECT_ENUM)m_UVWSel1;
		m_UVWSel2 = iTrig3_Dpwm[m_iSec30];
	}

	m_iT1Length = m_iZeroLength + m_iT1Length;
	m_iT2Length = m_iT1Length + m_iT2Length;

	if(m_FlagDPWMInverse == 1)
	{
		m_iZeroLength = gPWM.gPWMPrdApply - m_iZeroLength;
		m_iT1Length   = gPWM.gPWMPrdApply - m_iT1Length;
		m_iT2Length   = gPWM.gPWMPrdApply - m_iT2Length;
	}

// ��ʼ�������UVW�ıȽ�ֵ
	m_pUVW = (Ulong *)&gPWM.U;
	*(m_pUVW + m_UVWSel1) = m_iZeroLength;
	*(m_pUVW + iTrig2_Pwm[m_iSec30]) = m_iT1Length;
	*(m_pUVW + m_UVWSel2) = m_iT2Length;

	DeadBandComp();   
	SendPWM();
}

/*************************************************************
	�������������������С�������ơ�
*************************************************************/
void DeadBandComp(void)
{
	long m_Max,m_Min,m_Deta;

    if(gRatio != 0)
    {
        if(ZERO_VECTOR_U != gPWM.gZeroLengthPhase )
        {
            gPWM.U += (long)gDeadBand.CompU;
        }
        if(ZERO_VECTOR_V != gPWM.gZeroLengthPhase )
        {
            gPWM.V += (long)gDeadBand.CompV;
        }
        if(ZERO_VECTOR_W != gPWM.gZeroLengthPhase )
        {
            gPWM.W += (long)gDeadBand.CompW;
        }
    }

    /* ȡ��խ����2011.05.07 L1082*/
    gPWM.U = __IQsat(gPWM.U, gPWM.gPWMPrdApply, 0);
    gPWM.V = __IQsat(gPWM.V, gPWM.gPWMPrdApply, 0);
    gPWM.W = __IQsat(gPWM.W, gPWM.gPWMPrdApply, 0);
    
#if 0               
    if(gSubCommand.bit.CancelNarrorP == 0)      //����խ�������
    {
        gPWM.U = __IQsat(gPWM.U, gPWM.gPWMPrdApply, 0);
        gPWM.V = __IQsat(gPWM.V, gPWM.gPWMPrdApply, 0);
        gPWM.W = __IQsat(gPWM.W, gPWM.gPWMPrdApply, 0);
    }
    else                                        // ȥ��խ����
    {                                            
        m_Min = (long)gDeadBand.DeadBand + 1;
        m_Max = (long)(gPWM.gPWMPrdApply - m_Min);
// U��խ�������
        m_Deta = gPWM.U - m_Max;
        if(gPWM.U < (m_Min>>1))
        {
            gPWM.U = 0; 
        }
        else if(gPWM.U < m_Min)	
        {
            gPWM.U = m_Min;
        }
        else if(m_Deta < 0)				
        {
            ;   // ������
        }
        else if(m_Deta < (m_Min>>1))	
        {
            gPWM.U = m_Max;
        }
        else    // m_Deta > (m_Min>>1)
        {
            gPWM.U = gPWM.gPWMPrdApply;
        }
// V��խ�������
        m_Deta = gPWM.V - m_Max;
        if(gPWM.V < (m_Min>>1))	
        {
            gPWM.V = 0; 
        }
        else if(gPWM.V < m_Min)	
        {
            gPWM.V = m_Min;
        }
        else if(m_Deta < 0)	
        {
            ;
        }
        else if(m_Deta < (m_Min>>1))
        {
            gPWM.V = m_Max;
        }
        else
        {
            gPWM.V = gPWM.gPWMPrdApply;
        }
// W��խ�������
        m_Deta = gPWM.W - m_Max;
        if(gPWM.W < (m_Min>>1))
        {
            gPWM.W = 0; 
        }
        else if(gPWM.W < m_Min)
        {
            gPWM.W = m_Min;
        }
        else if(m_Deta < 0)
        {
            ;
        }
        else if(m_Deta < (m_Min>>1))
        {
            gPWM.W = m_Max;
        }
        else
        {
            gPWM.W = gPWM.gPWMPrdApply;
        }
    }
#endif

}

/*************************************************************
	����EPWMģ��ļĴ�����ʵ�ʷ���PWM���Σ�������EPWMģ��BUG�Ĺ��
*************************************************************/
void SendPWM()
{
//#ifdef TMS320F2808                  // ePWMģ��bug���, 28035������
	//...�ָ��Ĵ�������
	EALLOW;
	if(EPWMFlagU != 0)
	{
		EPWMFlagU = 0;
		//EPwm1Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
		//EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;
		EPwm1Regs.AQCTLA.all = 0x90;
		//EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;
		EPwm1Regs.CMPCTL.all = 0;
	}
	if(EPWMFlagV != 0)
	{
		EPWMFlagV = 0;
		//EPwm2Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
		//EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
		EPwm2Regs.AQCTLA.all = 0x90;
		//EPwm2Regs.CMPCTL.bit.LOADAMODE = 0;
		EPwm2Regs.CMPCTL.all = 0;
	}
	if(EPWMFlagW != 0)
	{
		EPWMFlagW = 0;
		//EPwm3Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
		//EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
		EPwm3Regs.AQCTLA.all = 0x90;
		//EPwm3Regs.CMPCTL.bit.LOADAMODE = 0;
		EPwm3Regs.CMPCTL.all = 0;
	}

	//...(CMPR=!0)=>(CMPR=0)��(CMPR=!0)=>(CMPR=0)��һ���޸ļĴ���
	if(gPWM.U == 0)
	{
		if(EPwm1Regs.CMPA.half.CMPA != 0)
		{
			EPwm1Regs.AQCTLA.all = 0x91;
			EPWMFlagU = 1;
		}
	}
	else
	{
		if(EPwm1Regs.CMPA.half.CMPA == 0)
		{
			EPwm1Regs.AQCTLA.all = 0x12;
			EPwm1Regs.CMPCTL.all = 2;
			EPWMFlagU = 1;
		}
	}

	if(gPWM.V == 0)
	{
		if(EPwm2Regs.CMPA.half.CMPA != 0)
		{
			EPwm2Regs.AQCTLA.all = 0x91;
			EPWMFlagV = 1;
		}
	}
	else
	{
		if(EPwm2Regs.CMPA.half.CMPA == 0)
		{
			EPwm2Regs.AQCTLA.all = 0x12;
			EPwm2Regs.CMPCTL.all = 2;
			EPWMFlagV = 1;
		}
	}

	if(gPWM.W == 0)
	{
		if(EPwm3Regs.CMPA.half.CMPA != 0)
		{
			EPwm3Regs.AQCTLA.all = 0x91;
			EPWMFlagW = 1;
		}
	}
	else
	{
		if(EPwm3Regs.CMPA.half.CMPA == 0)
		{
			EPwm3Regs.AQCTLA.all = 0x12;
			EPwm3Regs.CMPCTL.all = 2;
			EPWMFlagW = 1;
		}
	}
//#endif

	EPwm1Regs.TBPRD = gPWM.gPWMPrdApply; 
	EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
	EPwm2Regs.TBPRD = gPWM.gPWMPrdApply;
	EPwm3Regs.TBPRD = gPWM.gPWMPrdApply;

	EPwm1Regs.CMPA.half.CMPA = gPWM.U;
	EPwm2Regs.CMPA.half.CMPA = gPWM.V;
	EPwm3Regs.CMPA.half.CMPA = gPWM.W;
	EDIS;         
}

