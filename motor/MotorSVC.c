/****************************************************************
�ļ�����: ����ʸ������ת�Ӵ������㣬ת��ת�ٹ���
�ļ��汾�� 
���¸��£� 
�����������
�����������
ʹ���ⲿȫ�ֱ�����
                 gABVoltSet            //�����������¶���ʵ�������ѹ
				 gIAlphBetaQ12         //�����������¶��Ӳ�����������
                 gBasePar              //�ز�Ƶ����Чֵ,Ƶ�ʻ�ֵ
                 gMainCmd              //���ͬ��Ƶ��
                 gVCPar.VCSpeedFilter  //F2-07ת���˲�ϵ��
				 gMotorExtPer          //���ۻ���ĵ������
				 gPhase
				 gVCPar.SvcMode        //SVC�Ż�ģʽѡ��
�޸��ⲿȫ�ֱ�����
				 gFluxR                //ת�Ӵų��ķ�ֵ����λ��
				                       //����SVC1�Ż��㷨���в����������������طɳ����� 
				 gFluxS                //�����Ӷ��Ӵų��ķ�ֵ����λ�����
	             gMainCmd.FreqFeed     //���ٻ�SVC���Ƶõ���ת��ת��ֵ

�������ܵ����ù����룺
      SVC1:
                gTestDataReceive.TestData3 //��ͨ�۲��ͨ�˲���ֹƵ��
				gTestDataReceive.TestData2 //��ͨ�۲ⲹ��ϵ��
				gTestDataReceive.TestData4 //M������ջ�����ϵ��
				gTestDataReceive.TestData5 //T������ջ�����ϵ��
				gTestDataReceive.TestData8 //T���������ֵƽ���˲�ϵ��
****************************************************************/
#include "MotorVCInclude.h"
#include "MotorSvcInclude.h"
#include "MotorEncoder.h"

// // ȫ�ֱ�������
FLUX_STRUCT             gFluxR;
FLUX_STRUCT             gFluxS;
SVC_FLUX_CAL_STRUCT		gCalFlux;		
SVC_SPEED_STRUCT        gSVCSpeed;

//SVC 1 ר��
ALPHABETA_STRUCT		gABFluxS_LowFilter;		//�����������¶��Ӵ�ͨ����ֵ
ALPHABETA_STRUCT		gABFluxS_comp;	    //�����������¶��Ӵ�ͨ�۲ⲹ��ֵ���ñ���Ӧ����Ϊȫ�ֱ���
MT_STRUCT_Q24           gIMTQ24_obs;        //��ͨ�۲�����ϵ�·ֽ�õ���MT�����
MT_STRUCT_Q24           gIMTQ12_obs;
/************************************************************
��������:��
�������:��
����λ��:�ϵ��ʼ��������֮ǰ
��������:�첽��SVC����
��������:��ʼ���������õ��ı���
************************************************************/
void ResetSVC(void)
{
	gABVoltSet.Alph = 0;
	gABVoltSet.Beta = 0;


	gFluxR.Amp = 0;
	gFluxR.Theta = 0;
	gFluxS.Amp = 0;
	gFluxS.Theta = 0;


	gSVCSpeed.SvcRotorSpeed 	= 0;
    gMainCmd.FreqFeed      = 0;            //��ֹ���Ϻ�����������
	gSVCSpeed.SvcSynSpeed 	= 0;
    gMainCmd.FreqSynFilter = 0;

	gSVCSpeed.SvcWs 			= 0;
	gSVCSpeed.SvcSignal = 0;
	gMainCmd.FreqReal = 0;						//new reset words
	gSVCSpeed.SvcLastFluxPos = 0;
	gMainCmd.FirstCnt = 0;


	//Added by jxl:����������ͨ�۲ⲹ��ֵ����
    gABFluxS_LowFilter.Alph = 0;  //Q13:Alph�ᶨ�Ӵ�ͨ�õ�ѹģ�ͺ�һ�׹��Ի��ڵĹ۲�ֵ
    gABFluxS_LowFilter.Beta = 0;  //Q13:Beta�ᶨ�Ӵ�ͨ�õ�ѹģ�ͺ�һ�׹��Ի��ڵĹ۲�ֵ
	gABFluxS_comp.Alph = 0;       //Q13:Alph�ᶨ�Ӵ�ͨ����ֵ
	gABFluxS_comp.Beta = 0;       //Q13:Beta�ᶨ�Ӵ�ͨ����ֵ

	gFluxR.Theta = (Uint)((long)gPhase.IMPhase>>16);
}
/*************************************************************
	SVC���㶨�Ӵ�ͨʸ����ת�Ӵ�ͨʸ��
	SVC0 �޲���
	SVC1 �в���
*************************************************************/
/************************************************************
��������:
          gMainCmd.FreqSyn          //ʵ�����ͬ��Ƶ��
		  gIAlphBetaQ12             //��ֹ����ϵ�����������ӵ���
          gIMTSetApply              //MT�����ʵ����Ч���趨ֵ
		  gABVoltSet                //ʵ�������ѹ
		  gPhase.IMPhase            //�趨��ת�Ӵų�ͬ����
		  gPhase.StepPhaseApply     //ת�Ӵų�ͬ�����ۼӲ���
�������:
          gFluxR                    //������ϵת�Ӵ�ͨ�۲�ֵ
		  gFluxS                    //������ϵ���Ӵ�ͨ�۲�ֵ
���ù����룺
          gTestDataReceive.TestData3 //SVC1 ��ͨ�۲��ͨ�˲���ֹƵ��
          gTestDataReceive.TestData2 //SVC1 ��ͨ�۲ⲹ��ϵ��

����λ��:AD�ж�
��������:�첽��SVC���У�FVC��ͨ�ջ������طɳ�����
��������:�����ת�Ӵ�ͨ��ֵ����λ�۲�
************************************************************/

void SVCCalFlux_380(void)  //����¡�޸ĺ��㷨
{
	long  m_Long;
	Ulong m_ULong;
	int   m_LrDivLm,m_LLou;
    Ulong m_Timer;
	int   m_DetaTimer;
    int     m_Angle,m_Sin,m_Cos;
    ALPHABETA_STRUCT		m_ABFluxS_cmd;		//�����������¶��Ӵ�ͨ����ֵ
    ALPHABETA_STRUCT		m_ABFluxR_cmd;		//������������ת�Ӵ�ͨ����ֵ
    ALPHABETA_STRUCT		m_IAlphBeta_cmd;	    //�������������ᶨ�ӵ�������ֵ
    ALPHABETA_STRUCT		m_ABFluxS;		//�����������¶��Ӵ�ͨ
    ALPHABETA_STRUCT		m_ABFluxR;		//������������ת�Ӵ�ͨ
    
	gCalFlux.SampleTime = 25736L / (long)gBasePar.FcSetApply;	//�ز����� PI*2^13 / FC

   	if(0 == gVCPar.SvcMode)  //SVC0
	{
        gCalFlux.FilterTime = (gMainCmd.FreqReal * 839L >> 11) + 300;
        m_Timer = GetTime();
        m_DetaTimer = (((gSVCSpeed.Timer - m_Timer) & 0xFFFFFFul)>>3);
        gSVCSpeed.Timer = m_Timer;
	}
	else                     //SVC1
	{
	    //gCalFlux.FilterTime = 200;
	    gCalFlux.FilterTime = gTestDataReceive.TestData3;
	    gCalFlux.FilterTime = __IQsat(gCalFlux.FilterTime,2000,100);
	}

	m_LrDivLm = ((Ulong)gMotorExtPer.L1 << 14)/gMotorExtPer.LM;               // Lx
	//m_LrDivLm(Q14) = ((Ulong)gMotorExtPer.L1(Q9) << 14)/gMotorExtPer.LM(Q9);               // Lx
	m_LLou = ((Ulong)gMotorExtPer.L0 * 10000)/gBasePar.FullFreq01;    //2*L0
	//m_LLou(Q14)    = ((Ulong)gMotorExtPer.L0(Q14) * 5000(Q0))/gBasePar.FullFreq01(Q0);

/*******************************************/
    //���㾲ֹ����ϵ�Ķ��Ӵ�ͨ����ֵ�Թ۲�õ��Ķ��Ӵ�ͨ���в������ټ���ת�Ӵ�ͨ

    //���Ӵ�ͨ�۲⣺��ͨ�˲����濪������
	m_Long = (long)gMotorExtPer.R1 * (long)gIAlphBetaQ12.Alph;	//R1ΪQ16��ʽ
	//m_Long(Q28) = ((llong)gMotorExtPer.R1(Q16) * (llong)gIAlphBetaQ12.Alph(Q12));
	m_Long = (long)gABFluxS_LowFilter.Alph * (long)gCalFlux.FilterTime + m_Long;
	//m_Long(Q28) = (long)gABFluxS_LowFilter.Alph(Q14) * (long)gCalFlux.FilterTime(Q14) + m_Long(Q28);
	m_Long = (((long) gABVoltSet.Alph<<16) - m_Long)>>16;
	//m_Long(Q12) = (((long) gABVoltSet.Alph(Q12)<<16) - m_Long(Q28))>>16;
	m_Long = (long)m_Long * (long)gCalFlux.SampleTime;
	//m_Long(Q25) = (long)m_Long(12) * (long)gCalFlux.SampleTime(13);
	gABFluxS_LowFilter.Alph = (((long)gABFluxS_LowFilter.Alph<<11) + m_Long)>>11;
	//gABFluxS_LowFilter.Alph(Q14) = (((long)gABFluxS_LowFilter.Alph(Q14)<<11) + m_Long(Q25))>>11;

	m_Long = (long)gMotorExtPer.R1 * ((long)gIAlphBetaQ12.Beta);	//R1ΪQ16��ʽ
	//m_Long(Q28) = ((llong)gMotorExtPer.R1(Q16) * (llong)gIAlphBetaQ12.Beta(12));	//R1ΪQ16��ʽ
	m_Long = (long)gABFluxS_LowFilter.Beta * (long)gCalFlux.FilterTime + m_Long;
	//m_Long(Q28) = (long)gABFluxS_LowFilter.Beta(Q14) * (long)gCalFlux.FilterTime(Q14) + m_Long(Q28);
	m_Long = (((long)gABVoltSet.Beta<<16) - m_Long)>>16;
	//m_Long(Q12) = (((long)gABVoltSet.Beta(Q12)<<16) - m_Long(Q28))>>16;
	m_Long = (long)m_Long * (long)gCalFlux.SampleTime;
	//m_Long(Q25) = (long)m_Long(Q12) * (long)gCalFlux.SampleTime(Q13);
	gABFluxS_LowFilter.Beta = (((long)gABFluxS_LowFilter.Beta<<11) + m_Long)>>11;
	//gABFluxS_LowFilter.Beta(Q14) = (((long)gABFluxS_LowFilter.Beta(Q14)<<11) + m_LongQ25))>>11;

    //��ֹ����ϵ�¶��Ӵ�ͨ�۲�ֵ����ͨ�˲��۲�ֵ+����ֵ
   	if(0 == gVCPar.SvcMode)  //ʹ��ԭ380�㷨���Ա��ں�320����
    {
         m_ABFluxS.Alph = (long)gABFluxS_LowFilter.Alph; // - ((long)gABFluxS_comp.Alph * (long)gTestDataReceive.TestData2)/100L;
         m_ABFluxS.Beta = (long)gABFluxS_LowFilter.Beta; // - ((long)gABFluxS_comp.Beta * (long)gTestDataReceive.TestData2)/100L;
    }
    else  //SVC1   1 == gVCPar.SvcMode  
    {
        //���Ӵ�ͨ�۲�ֵ�������ø�����ͨ�Ե�ͨ�˲��۲�ֵ����
       	// ����������ϵĸ���ת�Ӵ�ͨ:DQ->AlphBeta�Ƕ�׼��  
    	    m_Angle = (int)(((long)gPhase.IMPhase>>16)+((long)gPhase.StepPhaseApply>>16));
    	    //m_Angle = (int)((long)gFluxR.Theta+((long)gPhase.StepPhaseApply>>16));
			
    	m_Sin  = qsin(m_Angle);                 //Q15
    	m_Cos  = qsin(16384 - m_Angle);         //Q15

        //����ת�Ӵ�ͨ�ھ�ֹ����ϵ�µĸ���ֵ��r/s �任
    	m_ABFluxR_cmd.Alph = ((((long)m_Cos * ((long)gIMTSetApply.M>>12))>>11)
    	                    * ((long)gMotorExtPer.LM*5000L/(long)gBasePar.FullFreq01))>>11;
    	//m_ABFluxR_cmd.Alph(Q14) = ((((long)m_Cos(Q15) * ((long)gIMTSet.M(Q24)>>12)>>11))
    	//                    * (long)gMotorExtPer.LM(Q9))>>11;
    	m_ABFluxR_cmd.Beta = ((((long)m_Sin * ((long)gIMTSetApply.M>>12))>>11)
    	                    * ((long)gMotorExtPer.LM*5000L/(long)gBasePar.FullFreq01))>>11;
    	//m_ABFluxR_cmd.Beta(Q13) = ((((long)m_Sin(Q15) * ((long)gIMTSet.M(Q24)>>12)>>11))
    	//                    * (long)gMotorExtPer.LM(Q9))>>11;

    	//���㶨�ӵ����ھ�ֹ����ϵ�µĸ���ֵ��r/s�任
        m_IAlphBeta_cmd.Alph = (((long)gIMTSetApply.M>>12) * (long)m_Cos
                              - ((long)gIMTSetApply.T>>12) * (long)m_Sin)>>15;
        //m_IAlphBeta_cmd.Alph(Q12) = (((long)gIMTSetApply.M(Q24)>>12) * (long)m_cos(Q15)
        //                      - ((long)gIMTSetApply.T(Q24)>>12) * (long)m_sin(Q15))>>15;
        m_IAlphBeta_cmd.Beta = (((long)gIMTSetApply.M>>12) * (long)m_Sin
                              + ((long)gIMTSetApply.T>>12) * (long)m_Cos)>>15;
        //m_IAlphBeta_cmd.Beta(Q12) = (((long)gIMTSetApply.M(Q24)>>12) * (long)m_sin(Q15)
        //                      + ((long)gIMTSetApply.T(Q24)>>12) * (long)m_cos(Q15))>>15;

    	//���㶨�Ӵ�ͨ�ھ�ֹ����ϵ�µĸ���ֵ��ת�Ӵ�ͨ��©��ͨ������
    	m_Long = ((long)m_LLou * (long)m_IAlphBeta_cmd.Alph)>>12;
    	//m_Long(Q14) = ((long)m_LLou(Q14) * (long)m_IAlphBeta_cmd.Alph(Q12))>>12;
    	m_Long = (long)m_ABFluxR_cmd.Alph + m_Long;
    	//m_Long(Q14) = (long)m_ABFluxR_cmd.Alph(Q14) + m_Long(Q14);
    	m_Long = (long)m_Long * (long)gMotorExtPer.LM;
    	//m_Long(Q23) = (long)m_Long(Q14) * (long)gMotorExtPer.LM(Q9)
        m_ABFluxS_cmd.Alph = (long)m_Long / (long)gMotorExtPer.L1;
        //m_ABFluxS_cmd.Alph(Q14) = (long)m_Long(Q23) / (long)gMotorExtPer.L1(Q9);

    	m_Long = ((long)m_LLou * (long)m_IAlphBeta_cmd.Beta)>>12;
    	//m_Long(Q14) = ((long)m_LLou(Q14) * (long)m_IAlphBeta_cmd.Beta(Q12))>>12; 
    	m_Long = (long)m_ABFluxR_cmd.Beta + m_Long;
    	//m_Long(Q14) = (long)m_ABFluxR_cmd.Beta(Q14) + m_Long(Q14);
    	m_Long = (long)m_Long * (long)gMotorExtPer.LM;
    	//m_Long(Q23) = (long)m_Long(Q14) * (long)gMotorExtPer.LM(Q9)
        m_ABFluxS_cmd.Beta = (long)m_Long / (long)gMotorExtPer.L1;
        //m_ABFluxS_cmd.Beta(Q14) = (long)m_Long(Q23) / (long)gMotorExtPer.L1(Q9);

    	//�Ծ�ֹ����ϵ���Ӵ�ͨ����ֵ����ͨ�˲������㲹��ֵ
    	m_Long = (long)m_ABFluxS_cmd.Alph - (long)gABFluxS_comp.Alph;
    	//m_Long(Q14) = m_ABFluxS_cmd.Alph(Q14) - gABFluxS_comp.Alph(Q14);
    	m_Long = ((long)m_Long * (long)gCalFlux.SampleTime)>>12;
    	//m_Long(Q15) = (m_Long(Q14) * gCalFlux.SampleTime(Q13))>>12;
        gABFluxS_comp.Alph += ((long)m_Long * (long)gCalFlux.FilterTime)>>15;
        //gABFluxS_comp.Alph(Q14) += (m_Long(Q15) * gCalFlux.FilterTime(Q14))>>15;

    	m_Long = (long)m_ABFluxS_cmd.Beta - (long)gABFluxS_comp.Beta;
    	//m_Long(Q14) = m_ABFluxS_cmd.Beta(Q14) - gABFluxS_comp.Beta(Q14);
    	m_Long = ((long)m_Long * (long)gCalFlux.SampleTime)>>12;
    	//m_Long(Q15) = (m_Long(Q14) * gCalFlux.SampleTime(Q13))>>12;
        gABFluxS_comp.Beta += ((long)m_Long * (long)gCalFlux.FilterTime)>>15;
        //gABFluxS_comp.Beta(Q14) += (m_Long(Q15) * gCalFlux.FilterTime(Q14))>>15;

         m_ABFluxS.Alph = (long)gABFluxS_LowFilter.Alph + ((long)gABFluxS_comp.Alph * (long)gTestDataReceive.TestData2)/100L;
         m_ABFluxS.Beta = (long)gABFluxS_LowFilter.Beta + ((long)gABFluxS_comp.Beta * (long)gTestDataReceive.TestData2)/100L;
    }

	m_Long = ((long)gIAlphBetaQ12.Alph * (long)m_LLou)<<2;
	//m_Long(Q28) = ((llong)gIAlphBetaQ12.Alph(Q12) * (llong)m_LLou(Q14))<<2;
	m_Long = ((long)m_ABFluxS.Alph * (long)m_LrDivLm) - (long)m_Long;
	//m_Long(Q28) = ((long)m_ABFluxS.Alph(Q14) * (long)m_LrDivLm(Q14)) - m_Long(Q28);
	m_ABFluxR.Alph = m_Long>>14;
	//m_ABFluxR.Alph(Q14) = m_Long(Q28)>>14;

	m_Long = ((long)gIAlphBetaQ12.Beta * (long)m_LLou)<<2;	
	//m_Long(Q28) = ((llong)gIAlphBetaQ12.Beta(Q12) * (llong)m_LLou(Q14))<<2;	
	m_Long = ((long)m_ABFluxS.Beta * (long)m_LrDivLm) - m_Long;
	//m_Long(Q28) = ((long)m_ABFluxS.Beta(Q14) * (long)m_LrDivLm(q14)) - m_Long(q28);
	m_ABFluxR.Beta = m_Long>>14;
	//m_ABFluxR.Beta(q13) = m_Long(q28)>>14;



	m_ULong = (((long)m_ABFluxR.Alph * (long)m_ABFluxR.Alph) + 
	          ((long)m_ABFluxR.Beta * (long)m_ABFluxR.Beta));	
	//tempL = (((long)m_ABFluxR.Alph(Q14) * (long)m_ABFluxR.Alph(Q14)) + 
	//          ((long)m_ABFluxR.Beta(Q14) * (long)m_ABFluxR.Beta(Q14)));	
	gFluxR.Amp = (Ulong)qsqrt(m_ULong);			//��������ѹ���?
	//gFluxSAmp(Q14) = (Ulong)qsqrt(tempL)Q28));			//��������ѹ���?
    gFluxR.Theta = atan(m_ABFluxR.Alph,m_ABFluxR.Beta);//�����ͨʸ���Ƕ�

	m_ULong = (((long)gABFluxS_LowFilter.Alph * (long)gABFluxS_LowFilter.Alph) + 
	          ((long)gABFluxS_LowFilter.Beta * (long)gABFluxS_LowFilter.Beta));	
	//tempL = (((long)gABFluxR.Alph(Q14) * (long)gABFluxR.Alph(Q14)) + 
	//          ((long)gABFluxR.Beta(Q14) * (long)gABFluxR.Beta(Q14)));	
	gFluxS.Amp = (Ulong)qsqrt(m_ULong);			//��������ѹ���?
	//gFluxSAmp(Q14) = (Ulong)qsqrt(tempL)Q28));			//��������ѹ���?
    gFluxS.Theta = atan(gABFluxS_LowFilter.Alph,gABFluxS_LowFilter.Beta);//�����ͨʸ���Ƕ�

    gSVCSpeed.DetaTimer += m_DetaTimer;

}

/*************************************************************
	����ʸ���±�ʶת���ٶ�
*************************************************************/
void SVCCalRotorSpeed(void)
{
	int     m_DetaTimer;
	int    m_Speed;
	int     m_LowVolt;
	long		m_Long;  

	DINT;  
    m_DetaTimer = gSVCSpeed.DetaTimer;
    gSVCSpeed.DetaPhase = gFluxR.Theta - gSVCSpeed.SvcLastFluxPos;
	gSVCSpeed.SvcLastFluxPos = gFluxR.Theta;
    gSVCSpeed.DetaTimer = 0; 
	EINT;
 
	if(gMainCmd.FirstCnt < 10)		//������Ҫ���⴦�� = 30*500us = 15.0ms
	{
		gMainCmd.FirstCnt +=1;
		gSVCSpeed.SvcRotorSpeed = 0;
		gSVCSpeed.SvcSynSpeed	= 0;
		return;
	}

	//ͨ��ת�Ӵ�ͨʸ���ĽǶȼ���ת�Ӵ�ͨ�ٶȣ�ͬ���ٶȣ�
	m_Speed = ((Ulong)abs(gSVCSpeed.DetaPhase) * 25000L) / gBasePar.FullFreq01;
    if(gSVCSpeed.DetaPhase < 0)
	{
		m_Speed = -m_Speed;
	}
	m_Speed = ((long)m_Speed * (DSP_CLOCK*250L))/m_DetaTimer;//����2msʱ��
	gSVCSpeed.SvcSynSpeed = Filter16((int)m_Speed, gSVCSpeed.SvcSynSpeed) ;

	m_LowVolt = 300;			//������Ҫ���⴦��  //18V
	if(gInvInfo.InvTypeApply >= 22)
    {
        m_LowVolt = 80;     // 7V
    }
	if(gRatio < m_LowVolt)
    {
        gSVCSpeed.SvcSynSpeed = gMainCmd.FreqSyn;
    }
    if(gMotorExtPer.I0 > (gIMTSetApply.M>>12))
	{
	    m_Long = ((long)gMotorExtPer.R2<<11)/gMotorExtPer.I0;	//����ת��Ƶ��
	}
    else
	{
	    m_Long = ((long)gMotorExtPer.R2<<11)/(gIMTSetApply.M>>12);	//����ת��Ƶ��
	}
	//UData = ((long)gMotorExtPer.R2<<11)/gMotorExtPer.I0;	//����ת��Ƶ��
	m_Long = ((long)m_Long<<11)/gMotorExtPer.L1;
	m_Long =  ((long)m_Long * (long)gIMTQ12.T)>>14;          // ����ط��õ���ʵ�ʵ�T�����
	gSVCSpeed.SvcWs = ((long)m_Long * (long)gVCPar.VCWsCoff)/100;
	//����ת���ٶ�
	m_Speed = gSVCSpeed.SvcSynSpeed - gSVCSpeed.SvcWs;
        if(m_Speed==0)
        {
            gSVCSpeed.SvcRotorSpeed = m_Speed;
    	    //return;
        }
        if(gSVCSpeed.SvcSignal==0)
        {
            if(gMainCmd.FreqSyn==0)	
            {
                gSVCSpeed.SvcRotorSpeed = 0;
    	        //return;
            }
            else
            {
                gSVCSpeed.SvcSignal=1;
            }
        }
     
       if(((long)gMainCmd.FreqSet * (long)m_Speed) <0)
        {
            m_Speed = 0;
        }
	    //gSVCSpeed.SvcRotorSpeed = m_Speed;
    	if(gVCPar.VCSpeedFilter <= 1)
    	{
    		gSVCSpeed.SvcRotorSpeed = m_Speed;	
    	}
    	else                        //F2-07���˲�����
    	{
        	m_Speed =  (long)gSVCSpeed.SvcRotorSpeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
        	gSVCSpeed.SvcRotorSpeed = m_Speed / (gVCPar.VCSpeedFilter + 1L);
    	}
        gMainCmd.FreqFeed = gSVCSpeed.SvcRotorSpeed;
	}


void SVCCalRotorSpeed_380(void)
{
	Ulong    m_ULong;  //add by jxl
	long    m_Long;  //add by jxl
	int     m_IntData;
//	int     m_DetaPos;
	long    m_Speed;
//	int     m_LowVolt;
//	int		m_UData;
    //int     temp;
        //ע�����˲���С�����˶�̬��Ӧ�ٶȣ���ĿǰΪ��֤���������ȶ��ԣ�����˲�����˲������������˲��ķ�ʽ
	    //����Ҫ�����ٵ�Ӧ�ó��ϣ��ɺ����С�˲�
        //����������µ��˲��㷨
        m_IntData = (gSVCSpeed.SvcSynSpeed < gMainCmd.FreqSyn) ? 1 : -1;
        m_IntData = (gSVCSpeed.SvcSynSpeed == gMainCmd.FreqSyn) ? 0 : m_IntData;
        //temp2 = 1;  //���˲�����
        m_ULong = (((Ulong)gMotorInfo.Frequency<<15)/gBasePar.FullFreq01)>>1;
	    //ȡ0.5���ĶƵ��
        m_ULong = ((Ulong)m_ULong<<7)/(abs(gMainCmd.FreqSyn));
	    if(m_ULong>1024) 
    	{
	        m_ULong = 1024;
    	}
        gSVCSpeed.SvcSynSpeed=(((long)gSVCSpeed.SvcSynSpeed*(1024-m_ULong)+(long)gMainCmd.FreqSyn*m_ULong)>>10) + m_IntData;
	    //gSVCSpeed.SvcSynSpeed = Filter16((int)gMainCmd.FreqSyn, gSVCSpeed.SvcSynSpeed) + m_IntData ;

   if(gMainCmd.FirstCnt < 10)		//������Ҫ���⴦�� = 50*500us = 25.0ms
	{
		gMainCmd.FirstCnt +=1;
		gSVCSpeed.SvcRotorSpeed = 0;
		gSVCSpeed.SvcSynSpeed	= 0;
		return;
	}
	else if(gMainCmd.FirstCnt < 1000)  //SVC1 Ϊ���س���������Ԥ���ų�����ʱ��
	{
		gMainCmd.FirstCnt +=1;
	}
	else
	{
		gMainCmd.FirstCnt = 1000;

	}

    //�ù۲�ͬ���Ƿֽ�õ���ת�ص����͵�����ص������㻬��
    //�ܵ�Ƶ��ͨ�۲�Ӱ�죬��������Ư��ɵĲ������Ӵ��ͨ�۲��ʱ�䳣������ѹ����Ư������̬��������
    //�Ե�Ƶ����״̬������

    m_Long = ((long)gMotorExtPer.R2<<11)/(long)((gIMTSet.M>>12));	//����ת��Ƶ��
    //m_Long = ((long)gMotorExtPer.R2<<11)/(long)((gIMTQ24_obs.M>>12));	//����ת��Ƶ��
    //m_UData(Q15) = ((Ulong)gMotorExtPer.R2(Q16)<<11)/gMotorExtPer.I0(Q12);	//����ת��Ƶ��
    m_Long = ((long)m_Long<<11)/(long)gMotorExtPer.L1;
    //m_UData(Q17) = ((long)m_UData(Q15)<<11)/gMotorExtPer.L1(Q9);
    m_Long =  ((long)m_Long * (long)(gIMTQ24_obs.T>>12))>>14;          // ����ط��õ���ʵ�ʵ�T�����
    //m_UData(Q15) =  ((long)m_UData(Q17) * (long)gIMTQ12.T(Q12))>>14;          // ����ط��õ���ʵ�ʵ�T�����

	gSVCSpeed.SvcWs = ((long)m_Long * (long)gVCPar.VCWsCoff)/100;
	     

	//����ת���ٶ�
	//ԭ380���룺����ת��ת��=����ͬ����-���ƻ��
	//�´��루Jxl��������ת��ת�٣�����ͬ����-���ƻ��
	gSVCSpeed.SvcRotorSpeed = gSVCSpeed.SvcSynSpeed - gSVCSpeed.SvcWs;

        if(gSVCSpeed.SvcSignal==0)
        {
            if(gMainCmd.FreqSyn==0)	
            {
                gSVCSpeed.SvcRotorSpeed = 0;
    	        //return;
            }
            else
            {
                gSVCSpeed.SvcSignal=1;
            }
        }
    
   	if(gVCPar.VCSpeedFilter <= 1)  //��ת���˲�
	{
		gSVCSpeed.SvcRotorSpeed = gSVCSpeed.SvcRotorSpeed;	
	}
	else                        //F2-07���˲�����
	{
       	m_Speed =  (long)gSVCSpeed.SvcRotorSpeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
       	gSVCSpeed.SvcRotorSpeed = m_Speed / (gVCPar.VCSpeedFilter + 1L);
	}
    gMainCmd.FreqFeed = gSVCSpeed.SvcRotorSpeed;
 }


