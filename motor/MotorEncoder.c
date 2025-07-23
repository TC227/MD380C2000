/***************************************************************
�ļ����ܣ� ���б��������ٵĴ���ABZ, UVW�źţ�����
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
// ˵��:
//2808��gpio20-23��������ΪQEP�������������٣�Ҳ��������ΪspiC����������٣� �ڸ���ʱ��Ҫ��ʼ����gpio�ĸ��ã�
//28035��gpio20-23��������ΪQEP������gpio�ڣ���ģ��������٣���Ҫע�����֣�

#include "MotorEncoder.h"
#include "MotorPmsmMain.h"
#include "MotorPmsmParEst.h"
#include "MotorVCInclude.h"
#include "SubPrgInclude.h"

// ȫ�ֱ������� 
struct EQEP_REGS       *EQepRegs;      // QEP ָ��

PG_DATA_STRUCT			gPGData;        // ���������ýṹ��
IPM_UVW_PG_STRUCT       gUVWPG;         // UVW�����������ݽṹ
IPM_PG_DIR_STRUCT       gPGDir;         // ��ʶ���������߷�ʽ�Ľṹ��
FVC_SPEED_STRUCT		gFVCSpeed;    // �����������ٶ����ݽṹ 
ROTOR_TRANS_STRUCT		gRotorTrans;    // ��ת��ѹ���ٶ�λ�÷�������
BURR_FILTER_STRUCT		gSpeedFilter;   // �ٶȷ����˲��ṹ
CUR_LINE_STRUCT_DEF		gSpeedLine;	    // ..

Uint const gUVWAngleTable[6] = {        // UVW �Ƕȱ�����
    //001                //010               //011
    (330*65536L/360), (210*65536L/360),	(270*65536L/360), 
    //100                //101                //110
    (90 *65536L/360), (30 *65536L/360), (150*65536L/360)       
};

// // �ڲ���������
void GetSpeedMMethod(void);
void GetSpeedMTMethod(void);
void SpeedSmoothDeal( ROTOR_SPEED_SMOOTH * pSpeedError,int iSpeed );
void RotorTransCalVel_old(void);

/*************************************************************
    ����Ӳ���ӿڵĳ�ʼ���� 2808 ��28035��һ��
*************************************************************/
void InitRtInterface(void)
{
    EALLOW;
#ifdef TMS320F2808                             // 2808 ʹ��spiC������оƬͨѶ

	SysCtrlRegs.PCLKCR0.bit.SPICENCLK = 1;      //BIT6 SPI-C
    // ��ȡ��ת��ѹ��λ��SPI����
    GpioCtrlRegs.GPAPUD.bit.GPIO21  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 2;        // SPIC MI 
    GpioCtrlRegs.GPAQSEL2.bit.GPIO21= 3;        // Asynch        
    GpioCtrlRegs.GPAPUD.bit.GPIO22  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 2;        // SPIC CLK 
    GpioCtrlRegs.GPAQSEL2.bit.GPIO22= 3;        // Asynch        
    GpioCtrlRegs.GPAPUD.bit.GPIO23  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 2;        // SPIC STE
    GpioCtrlRegs.GPAQSEL2.bit.GPIO23= 3;        // Asynch    

    // Initialize SPI FIFO registers
    SpicRegs.SPIFFTX.all=0xE040;
    SpicRegs.SPIFFRX.all=0x405f;                // Receive FIFO reset
    SpicRegs.SPIFFRX.all=0x205f;                // Re-enable transmit FIFO operation
    SpicRegs.SPIFFCT.all=0x0000;

    // Initialize  SPI
    SpicRegs.SPICCR.all =0x000F;                // Reset on, , 16-bit char bits

    #if 1                                       // 12λPG��spiģʽ
    SpicRegs.SPICCR.bit.CLKPOLARITY = 1;        // falling edge receive
    SpicRegs.SPICTL.all =0x000E;                // Enable master mode, SPICLK signal delayed by one half-cycle
                                                // enable talk, and SPI int disabled.
                                                // CLOCK PHASE = 1
    #else                                       // 16λPG��spiģʽ
	SpicRegs.SPICCR.bit.CLKPOLARITY = 0;	    //16bit pg
    SpicRegs.SPICTL.all =0x0006;      		    //16bit pg
    #endif
                                       
    // SPI������    LSPCLK = 100MHz/4
    SpicRegs.SPIBRR = 48;                       // 100/4 * 10^6 / (49)  = 510KHz
   
    
    SpicRegs.SPICCR.bit.SPISWRESET = 1;     
    SpicRegs.SPIPRI.bit.FREE = 1;          

    // �����RD�ź�GPIO34 ����
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;         // Enable pull-up       
    GpioCtrlRegs.GPBMUX1.bit.GPIO34= 0;        
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;         // Set sample to High
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;		    // Resolver sample signal config: output
    
#else                                           // 28035 ��Ҫʹ��IO��ģ�⴮��ͨѶ

    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;        // REDVEL\  ->
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;        // SO       <-
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;        // SCLK     ->
    GpioCtrlRegs.AIOMUX1.bit.AIO10 = 0;         // SAMPLE    AIO-port ->
    GpioCtrlRegs.AIOMUX1.bit.AIO12 = 0;         // RD\       AIO-port ->

    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;         // 
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 0;         // 
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;         //
    GpioCtrlRegs.AIODIR.bit.AIO10 = 1;
    GpioCtrlRegs.AIODIR.bit.AIO12 = 1;

    ROTOR_TRANS_RDVEL = 1;                      // һֱѡ���ȡλ��
#endif
    EDIS;
}


/*************************************************************
	����QEP�ӿ�
*************************************************************/
void InitSetQEP(void)
{
   	EALLOW;
	(*EQepRegs).QEPCTL.bit.QPEN = 0;
	(*EQepRegs).QDECCTL.all = 0;
	(*EQepRegs).QEPCTL.bit.PCRM = 1;    // pos reset mode
	(*EQepRegs).QEPCTL.bit.QPEN = 1;
	(*EQepRegs).QEPCTL.bit.QCLM = 1;
	(*EQepRegs).QEPCTL.bit.UTE = 1;
	(*EQepRegs).QEPCTL.bit.WDE = 0;

	(*EQepRegs).QPOSCTL.all = 0;
	(*EQepRegs).QCAPCTL.bit.CEN = 0;
	(*EQepRegs).QCAPCTL.bit.CCPS = 2;		//CAPʱ������4��Ƶ,��֤������Ϊ2ms
	(*EQepRegs).QCAPCTL.bit.UPPS = 2;
	(*EQepRegs).QCAPCTL.bit.CEN = 1;

	(*EQepRegs).QPOSCNT = 0;
	(*EQepRegs).QPOSINIT = 0;
	(*EQepRegs).QPOSMAX = 0xFFFFFFFF;
	(*EQepRegs).QPOSCMP = 0;
	(*EQepRegs).QUTMR = 0;
	(*EQepRegs).QWDTMR = 0;
	(*EQepRegs).QUPRD = C_TIME_05MS;			//0.5ms��ʱ��(��ʱ��Ҫ���ڲ����жϼ��)
	(*EQepRegs).QCLR.all = 0x0FFF;

	(*EQepRegs).QCTMR = 0;
	(*EQepRegs).QCPRD = 0;

    (*EQepRegs).QEPCTL.bit.IEL = 1;

	EDIS;
}

//**************************************************************************
// �޸�PG�����ͺ���Ҫ���³�ʼ����������

//**************************************************************************
void ReInitForPG(void)
{
    switch(gPGData.PGType)
    {
        case PG_TYPE_UVW:
            gPGData.PGMode = 0;
            EALLOW;
            GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1;    //eQEP1A
            GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1;    //eQEP1B
            GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;    //eQEP1I
        #ifdef TMS320F2808
            GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 2;    //eQEP2A
            GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 2;    //eQEP2B
            GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 2;    //eQEP2I
        #endif   
        #ifdef TMS320F28035
            GpioCtrlRegs.AIOMUX1.bit.AIO6  = 2;     // ad uvw�ź�
            GpioCtrlRegs.AIOMUX1.bit.AIO10 = 2;
            GpioCtrlRegs.AIOMUX1.bit.AIO12 = 2;
        #endif
            EDIS;            
            break;
            
        case PG_TYPE_SPECIAL_UVW:
            gPGData.PGMode = 0;
            break;
            
        case PG_TYPE_RESOLVER:      // ����2808��28035��������
            gPGData.PGMode = 1;
            EALLOW;
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 0;//����ҪQEPģ��
        #ifdef TMS320F2808
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 0;
        #endif
            EDIS;
            
            InitRtInterface();      // ��ʼ������Ӳ���ӿ�
            break;
            
        case PG_TYPE_SC:
            gPGData.PGMode = 0;
            break;
            
        default:        // PG_TYPE_ABZ
            gPGData.PGMode = 0;
            EALLOW;
            GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1;    //eQEP1A
            GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1;    //eQEP1B
            GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;    //eQEP1I
            //GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 0x2;              //�˲��趨  
   			//GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 0x2;              //�˲��趨  
            //GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 0x2;              //�˲��趨  
			//GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0x03; //PULS_IN�˲�ʱ��5*5*20ns = 300ns���˿��˲�����ɾ���
        #ifdef TMS320F2808
            GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 2;    //eQEP2A
            GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 2;    //eQEP2B
            GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 2;    //eQEP2I
        #endif            
            EDIS;
            break;
    }

    // ȷ�������������޸ĺ�����ȷ��ʼ��ѡ����qep
    if(gPGData.PGMode == 0)
    {
        gPGData.QEPIndex = QEP_SELECT_NONE;
    }
    else        // ��������䣬�Ͳ����ٳ�ʼ��qep�ӿڵ�·
    {
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
    }
}

/************************************************************
    ���Ƶ����е����ת(���߷�ת)һȦ��ʶ��AB�źŵ�ʱ��
************************************************************/
Uint JudgeABDir(void)
{
    int m_Deta;

    m_Deta = gIPMZero.FeedPos - gPGDir.ABAngleBak;
    if(m_Deta > 1820)
    {
        if(gPGDir.ABDirCnt < 32767)
        {
            gPGDir.ABDirCnt ++;
        }
        gPGDir.ABAngleBak = gIPMZero.FeedPos;
    }
    else if(m_Deta < -1820)
    {
        if(gPGDir.ABDirCnt > -32767)
        {
            gPGDir.ABDirCnt --;
        }
        gPGDir.ABAngleBak = gIPMZero.FeedPos;
    }

    if(gPGDir.ABDirCnt > 2)
    {
        return (DIR_FORWARD);
    }
    else if(gPGDir.ABDirCnt < -2)
    {
        return (DIR_BACKWARD);
    }
	else
    {
        return (DIR_ERROR);
    }
}

/************************************************************
    ���Ƶ����е����ת(���߷�ת)һȦ��ʶ��UVW�źŵ�ʱ��
************************************************************/
Uint JudgeUVWDir(void)
{
    int m_Deta;

    m_Deta = (int)gUVWPG.UVWAngle - (int)gPGDir.UVWAngleBak;
    if(m_Deta > 1820)   // 10�ȣ�10/360  *65536(Q16��ʽ)
    {
        if(gPGDir.UVWDirCnt < 32767)
        {
            gPGDir.UVWDirCnt ++;
        }
        gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    }
    else if(m_Deta < -1820)
    {
        if(gPGDir.UVWDirCnt > -32767)
        {
            gPGDir.UVWDirCnt --;
        }
        gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    }

    if(gPGDir.UVWDirCnt > 2)
    {
        if(gMainCmd.FreqSet >= 0)
        {
             return (DIR_FORWARD);
        }
        else
        {
            return (DIR_BACKWARD);
        }
    }
    else if(gPGDir.UVWDirCnt < -2)
    {
        if(gMainCmd.FreqSet >= 0)
        {
            return (DIR_BACKWARD);
        }
        else
        {
            return (DIR_FORWARD);
        }
    }
	else
    {
        return (DIR_ERROR);
    }
}

/************************************************************
    ���Ƶ����е����ת(���߷�ת)һȦ��ʶ�� ���� �źŵ�ʱ��
************************************************************/
Uint JudgeRTDir()
{
    int m_Deta;

    m_Deta = (int)gRotorTrans.RTPos - gPGDir.RtPhaseBak;
    
    if(m_Deta > 1820)   // 10deg
    {
        if(gPGDir.RtDirCnt < 32767)
        {
            gPGDir.RtDirCnt ++;
        }
        gPGDir.RtPhaseBak = (int)gRotorTrans.RTPos;
    }
    else if(m_Deta < -1820)
    {
        if(gPGDir.RtDirCnt > -32767)
        {
            gPGDir.RtDirCnt --;
        }
        gPGDir.RtPhaseBak = (int)gRotorTrans.RTPos;
    }

    if(gPGDir.RtDirCnt > 2)
    {
        return (DIR_FORWARD);
    }
    else if(gPGDir.RtDirCnt < -2)
    {
        return (DIR_BACKWARD);
    }
	else
    {
        return (DIR_ERROR);
    }
}

/*******************************************************************************
	����:        UVW_GetUVWState(void)
	����:        UVW��������ȡUVW����λ�� ---- ��AD�ж���ִ��
	             ���UVW��������UVW�źŵ�ƽ����1~6��ʾ,�������Ӧ�Ƕ�
	�жϷ���:    �����жϱ������Ľ��߷�ʽ��Ȼ�����ȡ��Ӧ�ĽǶ�
	����:	 
	���:        gUVWPG.UVWAngle
	
	������:      PG_Zero_isr(void)
*******************************************************************************/
void GetUvwPhase()
{
    if((MOTOR_TYPE_PM != gMotorInfo.MotorType) ||
        (gPGData.PGType != PG_TYPE_UVW)   ||
        (gMainCmd.Command.bit.ControlMode != IDC_FVC_CTL))
    {
        return;
    }
        
// ����AD�����õ�UVW�źŵ��߼�ֵ
    if((Uint)UVW_PG_U > (Uint)0xA500)
    {
        gUVWPG.LogicU = ACTIVE_HARDWARE_LOGICAL_U;
    }
    else //if((Uint)UVW_PG_U < 0xC000)
    {
        gUVWPG.LogicU = ! ACTIVE_HARDWARE_LOGICAL_U;
    }
    if((Uint)UVW_PG_V > (Uint)0xA500)
    {
        gUVWPG.LogicV = ACTIVE_HARDWARE_LOGICAL_V;
    }
    else //if((Uint)UVW_PG_V < 0xC000)
    {
        gUVWPG.LogicV = ! ACTIVE_HARDWARE_LOGICAL_V;
    }

    if((Uint)UVW_PG_W > (Uint)0xA500)
    {
        gUVWPG.LogicW = ACTIVE_HARDWARE_LOGICAL_W;
    }
    else //if((Uint)UVW_PG_W < 0xC000)
    {
        gUVWPG.LogicW = ! ACTIVE_HARDWARE_LOGICAL_W;
    }


// ����UVW�߼�ֵ����uvw����λ�ýǶ�, ���ֲ�����ʶʱ�ͷǲ�����ʶ
    if((gPmParEst.UvwDir == 0 && gMainStatus.RunStep == STATUS_GET_PAR) ||
        (gUVWPG.UvwDir == 0  && gMainStatus.RunStep != STATUS_GET_PAR))
    {
        gUVWPG.UVWData = (gUVWPG.LogicU<<2) + (gUVWPG.LogicV<<1) + gUVWPG.LogicW;
    }
    else
    {
        gUVWPG.UVWData = (gUVWPG.LogicU<<2) + (gUVWPG.LogicW<<1) + gUVWPG.LogicV;
    }
    
    if((gUVWPG.UVWData > 0) && (gUVWPG.UVWData < 7))
    {
        gUVWPG.UVWAngle =  gUVWAngleTable[gUVWPG.UVWData - 1];
    }
    else if(gMainCmd.Command.bit.Start)
    {
        gError.ErrorCode.all |= ERROR_ENCODER;
        if(gUVWPG.UVWData == 7) gError.ErrorInfo[4].bit.Fault1 = 11;     // δ�ӱ�����
        else                    gError.ErrorInfo[4].bit.Fault1 = 12;     // uvw�źŹ���
    }

}

/*************************************************************
	�ջ�ʸ���»�ȡʵ���ٶ�0.5msִ�� gFVCSpeed.SpeedEncoder

*************************************************************/
void VCGetFeedBackSpeed(void)
{
    long tempLong;
	//GetMTTimeNum();						//ȡ����ʱ����
	
    switch(gPGData.PGType)
    {        
        case PG_TYPE_UVW:
        case PG_TYPE_SPECIAL_UVW:
            GetUvwPhase();                                      // UVW ��������ȡuvw�ź�
            ;
        case PG_TYPE_ABZ:        
	         if((*EQepRegs).QFLG.bit.WTO == 1)		            //10ms��û�����壬��Ϊ0��
	         {
		        (*EQepRegs).QCLR.all = 0x0010;
	         } 

	         GetSpeedMMethod();
	         GetSpeedMTMethod();
             gFVCSpeed.SpeedTemp = gFVCSpeed.MTSpeed;        // ѡ�� MT ��
             //gFVCSpeed.SpeedTemp = gFVCSpeed.MSpeed;
             break;
             
        case PG_TYPE_RESOLVER:                                  //���ж������,�Ƿ�Ҫ�Ӳ����˲�?ʵ����֤
            RotorTransCalVel();   
            gFVCSpeed.SpeedTemp = gRotorTrans.FreqFeed;
            break;
              
        case PG_TYPE_SC:
            ;
            break;
        default:  
             break;
    }

    // ���㴫����
    //gFVCSpeed.SpeedEncoder = (long)gFVCSpeed.SpeedTemp * gFVCSpeed.TransRatio / 1000;
    tempLong = (long)gFVCSpeed.SpeedTemp * gFVCSpeed.TransRatio;
    gFVCSpeed.SpeedEncoder = (llong)tempLong * 16777 >> 24;
    
    gMainCmd.FreqFeed = gFVCSpeed.SpeedEncoder;

}


/*************************************************************
	M�����٣�ͨ��0.5ms�ڵ������������ٶȣ���Ӧ����Ƶ�ʵ��ٶ�Ϊ��
	��������0.5ms������������1��/0.5ms��/����������������4��Hz
��ʾΪ��gMotorExtInfo.Poles * gFVCSpeed.DetaPos * 2000 / (gPGData.PulseNum * 4)Hz
��ôֵ��Poles * DetaPos * 500 * 100 * 32768 / (PulseNum * gBasePar.FullFreq)
      = Poles * DetaPos * 50000 * 32768 / (PulseNum * FullFreq)
returnVar: gFVCSpeed.MSpeed
*************************************************************/
void GetSpeedMMethod(void)
{
	Ullong m_Llong,m_Long;
	Uint   m_UData;
	int	   m_Speed;
	
	m_UData = abs(gFVCSpeed.MDetaPos);
	m_Long  = (Ulong)gMotorExtInfo.Poles * (Ulong)m_UData;
	m_Llong = ((Ullong)m_Long * 50000)<<15;
	m_Long  = (Ullong)gPGData.PulseNum * (Ullong)gBasePar.FullFreq01;

	m_Speed = __IQsat((m_Llong / m_Long), 32767, -32767);
	if(gFVCSpeed.MDetaPosBak < 0)
	{
		m_Speed = -m_Speed;
	}
    SpeedSmoothDeal( &gFVCSpeed.MSpeedSmooth, m_Speed);
    //gFVCSpeed.MSpeed = Filter8(gFVCSpeed.MSpeedSmooth.LastSpeed, gFVCSpeed.MSpeed);
    gFVCSpeed.MSpeed = Filter4(gFVCSpeed.MSpeedSmooth.LastSpeed, gFVCSpeed.MSpeed);
    //gFVCSpeed.MSpeed = gFVCSpeed.MSpeedSmooth.LastSpeed;
    
    if( gFVCSpeed.MSpeedSmooth.LastSpeed > gFVCSpeed.MSpeed )
    {
        gFVCSpeed.MSpeed++;
    }
    else if(gFVCSpeed.MSpeedSmooth.LastSpeed < gFVCSpeed.MSpeed )
    {
        gFVCSpeed.MSpeed--;
    }
    if(abs(gFVCSpeed.MSpeed - gFVCSpeed.MSpeedSmooth.LastSpeed ) < 8 )
    {
        gFVCSpeed.MSpeed = gFVCSpeed.MSpeedSmooth.LastSpeed;
    }
}

/*************************************************************
	MT�����٣�ͨ������N���������׼ȷʱ������ٶ�
	��������Deta_T����������?��/Deta_T��/����������������4��Hz
��ʾΪ��gMotorExtInfo.Poles * gFVCSpeed.DetaPos / (Deta_T(��) * gPGData.PulseNum * 4)Hz
��ôֵ��Poles * DetaPos * 100 * 32768 / (Deta_T(��) * PulseNum * gBasePar.FullFreq * 4)
      = Poles * DetaPos * 375 * 10^6 * 32768 / (Deta_T * PulseNum * FullFreq)
returnVar: gFVCSpeed.MTSpeed
*************************************************************/
void GetSpeedMTMethod(void)
{
	Ullong m_Llong,m_Long;
	Ulong  m_Time;
	Uint   m_UData,m_DetaPos;
	int	   m_Speed;
    long   tempL;
	if((*EQepRegs).QEPSTS.bit.COEF == 1)		//CAP��ʱ�����
	{
		gFVCSpeed.Flag = 1;
		(*EQepRegs).QEPSTS.all = 0x0008;
	}
    DINT;
	if((gFVCSpeed.MTPulseNum == 0) || 
	   (gFVCSpeed.MTTime > 65535) ||   //MS
	   (gFVCSpeed.Flag != 0))
	{
        if( 0 < gFVCSpeed.MTPulseNum )    gFVCSpeed.Flag = 0;    //�����׽���¼���ʱ���ǲ�׼ȷ��    	            
		gFVCSpeed.MTPulseNum  = 0;
		gFVCSpeed.MTTime      = 0;
		gFVCSpeed.MTLimitTime = 0;        
	EINT;
		gFVCSpeed.MTSpeed = gFVCSpeed.MSpeed; //M�������������Բ���,��û�н��յ�����ʱ��
        return;
	}

    DINT;
	m_DetaPos = gFVCSpeed.MTPulseNum;
	m_Time    = gFVCSpeed.MTTime;
	gFVCSpeed.MTPulseNum = 0;
	gFVCSpeed.MTTime     = 0;
	EINT;
	gFVCSpeed.MTLimitTime = (Uint)m_Time/m_DetaPos;

	m_UData = (Ulong)gMotorExtInfo.Poles * (Ulong)m_DetaPos;
    m_Long = ((Ullong)DSP_CLOCK * 100L * 1000000L) / (16 * (long)m_Time);
	m_Llong = ((Ullong)m_Long * (Ullong)m_UData)<<15;
	m_Long  = (Ullong)gPGData.PulseNum * (Ullong)gBasePar.FullFreq01;
	while((m_Long>>16) != 0)
	{
		m_Long  = m_Long>>1;
		m_Llong = m_Llong>>1;
	}
	if((m_Llong>>15) >= m_Long)	
	{
		m_Speed = 32767;
	}
	else
	{
		m_UData = m_Long>>1;
		m_Long  = m_Llong>>1;
		m_Speed = (int)(m_Long/m_UData);
	}

	if(gFVCSpeed.MDetaPosBak < 0)
	{
		m_Speed = -m_Speed;
	}
    if(gVCPar.VCSpeedFilter <= 1)
     {
       gFVCSpeed.MTSpeed = Filter4(m_Speed, gFVCSpeed.MTSpeed);
     }
    else
     {
       tempL =  (long)gFVCSpeed.MTSpeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
       gFVCSpeed.MTSpeed = tempL / (gVCPar.VCSpeedFilter + 1L);
     } /*MT�����ټ���F2-07�˲���2011.5.7 L1082*/
    if(gFVCSpeed.MTSpeed < m_Speed)   gFVCSpeed.MTSpeed ++;
    else                                gFVCSpeed.MTSpeed --;
    if(abs(gFVCSpeed.MTSpeed - m_Speed) < 8)
    {
        gFVCSpeed.MTSpeed  = m_Speed;
    }
}

/******************************************************************************
    �ٶ��˲����޳�ë�̵Ĵ���
******************************************************************************/
void SpeedSmoothDeal( ROTOR_SPEED_SMOOTH * pSpeedError, int iSpeed )
{
    if( abs( pSpeedError->LastSpeed - iSpeed ) < pSpeedError->SpeedMaxErr )
    {
        pSpeedError->LastSpeed = iSpeed;
        pSpeedError->SpeedMaxErr = pSpeedError->SpeedMaxErr>>1;
        if( 1000 > pSpeedError->SpeedMaxErr )
            pSpeedError->SpeedMaxErr = 1000;
    }
    else
    {
        if( 16383 > pSpeedError->SpeedMaxErr )
            pSpeedError->SpeedMaxErr = pSpeedError->SpeedMaxErr<<1;
        else
            pSpeedError->SpeedMaxErr = 32767;
    }
}

/*************************************************************
    M�����٣����жϳ������ж�0.5MSʱ���Ƿ�(��PWM�ж���ִ��)
*************************************************************/
void GetMDetaPos(void)
{
	DINT;
	if((*EQepRegs).QFLG.bit.UTO == 0)		
	{
	EINT;
		return;							//QEP�е�2ms��ʱ��û�е�
	}
    EALLOW;
	(*EQepRegs).QCLR.all = 0x0800;
    EDIS;

	gFVCSpeed.MDetaPos = (int)((long)(*EQepRegs).QPOSLAT - gFVCSpeed.MLastPos);
    if(0 != gFVCSpeed.MDetaPos)
    {
        gFVCSpeed.MDetaPosBak = gFVCSpeed.MDetaPos;
    }    
	gFVCSpeed.MLastPos = (*EQepRegs).QPOSLAT;
	EINT;
}

/*************************************************************
T�����ٹ�õ��Ѽ���������Ϣ�ĳ��򣬸ó����ɢ�ڲ�ͬ�ĵط�ִ�?
*************************************************************/
void GetMTTimeNum(void)
{
    EALLOW;
	if((*EQepRegs).QEPSTS.bit.UPEVNT == 0)	//��û�н��յ��㹻��������
	{
		return;
	}
	if((Uint)(*EQepRegs).QCPRD < (Uint)gFVCSpeed.MTLimitTime)
	{
	    (*EQepRegs).QEPSTS.all = 0x0080;	
		return;
	}
    DINT;
	gFVCSpeed.MTPulseNum += 4;
	gFVCSpeed.MTTime += (*EQepRegs).QCPRD;
	EINT;

	(*EQepRegs).QEPSTS.all = 0x0080;
    EDIS;
}

/************************************************************
    ��ȡ��ת��ѹ��λ���ź�
* �������ʱ��Ҫ��λ������
* ���ڱ�ʶ���λ�õ���Ҫ��ģ����һ��Z�ź�
************************************************************/
void GetRotorTransPos()
{    
#ifdef TMS320F28035
	Uint  mData,mBit;
	//Uint  mWatie;

    mData = 0;
	DINT;
	ROTOR_TRANS_RD=0;		//begin to transmit data
    
    gCpuTime.tmpBase = GetTime();
// 1st Fall-edge
	ROTOR_TRANS_SCLK  = 0;	//Set SCLK
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 15;     //MSB-bit15	    
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 2nd Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 14;     // bit14
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 3rd Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 13;     // bit13
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 4th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 12;     // bit12
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 5th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 11;     // bit11
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 6th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 10;     // bit10
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 7th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 9;     // bit9
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 8th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 8;     // bit8
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 9th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 7;     // bit7
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 10th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 6;     // bit6
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 11th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 5;     // bit5
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    asm(" RPT #5 || NOP ");
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 4;     // bit4
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
    //asm(" RPT #5 || NOP ");
    EINT;
    // ������������Ĵ���   (λ�ô�������ٶȵķ���Ͳ��ô�����)
    if((gPGData.SpeedDir && gMainStatus.RunStep != STATUS_GET_PAR) ||   // ����ʱ
        (gPGData.PGDir && gMainStatus.RunStep == STATUS_GET_PAR))  // ��ʶʱ
    {
	    gRotorTrans.RtorBuffer = 65535 - (Uint)mData;
    }
    else
    {
        gRotorTrans.RtorBuffer = (Uint)mData;
    }

    gCpuTime.tmpTime = gCpuTime.tmpBase- GetTime();

#else   // TMS320F2808
 
    // ������������Ĵ���   (λ�ô�������ٶȵķ���Ͳ��ô�����)
    if((gPGData.SpeedDir && gMainStatus.RunStep != STATUS_GET_PAR) ||   // ����ʱ
        (gPGData.PGDir && gMainStatus.RunStep == STATUS_GET_PAR))  // ��ʶʱ
    {
        gRotorTrans.RtorBuffer = 65535 - (Uint)SpicRegs.SPIRXBUF;
    }
    else
    {
        gRotorTrans.RtorBuffer = SpicRegs.SPIRXBUF;
    }
#endif

    gRotorTrans.AbsRotPos = gRotorTrans.RtorBuffer >> 4;    // �����ܰ���ʹ�õ������еλ�ýǶ�

    // ģ��һ��Z�źţ� ���ڴ��ر�ʶ������������: �������ĸ��Ƕȣ���ǰ�����Ƕ�, ͬʱ����4ms ���ʱ��
    if(gMainStatus.RunStep == STATUS_GET_PAR)
    {        
        if(((int)gRotorTrans.RtorBuffer > 0) &&
            ((int)gRotorTrans.SimuZBack <= 0) &&
            ((int)gRotorTrans.SimuZBack2 <= 0) &&
            (gIPMZero.zFilterCnt == 0))
        {
            gIPMZero.Flag = 1;
            gIPMZero.zFilterCnt = 8; 
            gIPMPos.PosInZInfo = gIPMPos.RotorPos;
        }
        gRotorTrans.SimuZBack2 = gRotorTrans.SimuZBack;
        gRotorTrans.SimuZBack = gRotorTrans.RtorBuffer;
    }
    
    // ��������ļ������� ��ȡ��Ƕ�λ��
    //gRotorTrans.Poles = (gRotorTrans.Poles > 0) ? gRotorTrans.Poles : 1;
    gRotorTrans.RTPos = (Ulong)gRotorTrans.RtorBuffer * gRotorTrans.PolesRatio >>8; // Q8
    
#ifdef TMS320F2808
    // ׼���´ζ�ȡ
    SpicRegs.SPITXBUF = 0xFFFF;
#endif
}
/*************************************************************
	��ת��ѹ����ȡλ��ǰ�����ݲ�������ͬʱ��¼׼ȷ��ʱ����
	ÿ0.45ms���һ���ٶȣ������жϴ���
*************************************************************/
void RotorTransSamplePos(void)
{
	Ulong	mTime;
	Ulong 	mDetaTime;	

	mTime = GetTime();
	mDetaTime = labs((long)(gRotorTrans.TimeBak - mTime));
	gRotorTrans.Flag = 0;
	if(mDetaTime >= C_TIME_045MS)
	{
		gRotorTrans.Flag = 1;					//��ʾӦ�ÿ�ʼ����һ���ٶ�
		gRotorTrans.DetaTime = mDetaTime;
		gRotorTrans.TimeBak = mTime;
	}
}

/************************************************************
    ����������ٶȷ���
�����ԣ����ַ������һ�����������ٶ�������50Hz�������Ʋ�

************************************************************/
void RotorTransCalVel(void)
{      
	Ulong  mUlong;
	Ulong  m_05ms;
	Uint   m_DetaPos;
	int	   m_Speed;
    long   tempL;

#if 0       // IS300��ʽ
    DINT;
    gRotorTrans.DetaTime = gRotorTrans.TimeBak - GetTime();
    gRotorTrans.TimeBak = GetTime();
    gRotorTrans.DetaPos = (int)((Uint)gRotorTrans.RTPos - (Uint)gRotorTrans.PosBak);
	gRotorTrans.PosBak  = gRotorTrans.RTPos;
    EINT;

    m_DetaPos = abs(gRotorTrans.DetaPos);
    tempL = gRotorTrans.DetaTime >> 1;
    mUlong = (50000L * (Ulong)m_DetaPos + tempL) / gRotorTrans.DetaTime;
    tempL = gBasePar.FullFreq01 >> 1;
    tempL = (mUlong * (1000L * DSP_CLOCK) + tempL) / gBasePar.FullFreq01;	
    m_Speed = __IQsat(tempL, 32767, -32767);

    if(gRotorTrans.DetaPos < 0)
	{
		m_Speed = -m_Speed;
	}
#else       // 380��ʽ ��¼�жϸ����ķ�ʽ
    DINT;
    gRotorTrans.IntNum = gRotorTrans.IntCnt;
    gRotorTrans.IntCnt = 0;    
    gRotorTrans.DetaPos = (int)((Uint)gRotorTrans.RTPos - (Uint)gRotorTrans.PosBak);
	gRotorTrans.PosBak  = gRotorTrans.RTPos;
    EINT;

    if(gRotorTrans.IntNum == 0)         // ��Ƶ��05msѭ������(2KHz)
    {
        return;
    }

    m_DetaPos = abs(gRotorTrans.DetaPos);
    m_05ms = gRotorTrans.IntNum << 2; // *4
    tempL = (Ulong)m_DetaPos * 100000L / (Ulong)gPWM.gPWMPrdApply;         // 1e5 -> 2500L
    tempL = tempL * DSP_CLOCK / m_05ms;
    m_Speed = tempL * 1000L / gBasePar.FullFreq01;

    if(gRotorTrans.DetaPos < 0)
	{
		m_Speed = -m_Speed;
	}
#endif

    //�޳�ë���˲�����
    gSpeedFilter.Input = m_Speed;
    BurrFilter((BURR_FILTER_STRUCT * )&gSpeedFilter);

	// ���ٶ��˲�����������
    if(gVCPar.VCSpeedFilter <= 1)   //�������Ĭ�����������Ҫ���˲�
	{
        tempL =  (long)gRotorTrans.FreqFeed * 29L + 2L * m_Speed;
        gRotorTrans.FreqFeed = tempL / 31L;	
	}
	else                        //F2-07���˲�����, Ĭ��20ms
	{
        tempL =  (long)gRotorTrans.FreqFeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
        gRotorTrans.FreqFeed = tempL / (gVCPar.VCSpeedFilter + 1L);
	}

    gRotorTrans.RealTimeSpeed = gRotorTrans.FreqFeed;
    tempL = ((long)gRotorTrans.RealTimeSpeed * 500L /gBasePar.FcSetApply)>>3;/*����ǶȲ����̻�500 2011.5.7L1082*/
	gRotorTrans.PosComp = tempL * gBasePar.FullFreq>>16;
	return;
}

