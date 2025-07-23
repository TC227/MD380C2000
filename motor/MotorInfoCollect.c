/****************************************************************
�ļ�����: ADģ���ʼ����ģ����������ת��
�ļ��汾��
���¸��£�
	
****************************************************************/
#include "MotorInfoCollectInclude.h"

// // ȫ�ֱ�������
ADC_STRUCT				gADC;		    //ADC���ݲɼ��ṹ
UDC_STRUCT				gUDC;		    //ĸ�ߵ�ѹ����
IUVW_SAMPLING_STRUCT	gCurSamp;

UVW_STRUCT				gIUVWQ12;		//�����������������
UVW_STRUCT_Q24          gIUVWQ24;       //Q24��ʽ�����ඨ�ӵ���
MT_STRUCT				gIMTQ12;		//MT��ϵ�µĵ���,Q12��ʾ
MT_STRUCT_Q24           gIMTQ24;        //MT��ϵ�µĵ���,Q24��ʾ
MT_STRUCT               gIMTSetQ12;
ALPHABETA_STRUCT		gIAlphBeta;	    //�����������������
ALPHABETA_STRUCT		gIAlphBetaQ12;	    //�����������������
AMPTHETA_STRUCT			gIAmpTheta;	    //�������ʾ�ĵ���
LINE_CURRENT_STRUCT		gLineCur;	
CUR_EXCURSION_STRUCT	gExcursionInfo; //�����Ưʹ�õĽṹ
TEMPLETURE_STRUCT		gTemperature;
AI_STRUCT				gAI;

/************************************************************
	������Ư����
************************************************************/
void GetCurExcursion(void)
{
	int m_ErrIu,m_ErrIv;
	
	if((gMainStatus.RunStep != STATUS_LOW_POWER) && 
	   (gMainStatus.RunStep != STATUS_STOP))
	{
		gExcursionInfo.EnableCount = 0;
		return;
	}

	gExcursionInfo.EnableCount++;
	gExcursionInfo.EnableCount = (gExcursionInfo.EnableCount>200)?200:gExcursionInfo.EnableCount;
	if((gExcursionInfo.EnableCount < 200))
	{
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.Count = 0;
		return;		
	}
	gExcursionInfo.TotalIu += gExcursionInfo.Iu;
	gExcursionInfo.TotalIv += gExcursionInfo.Iv;
	gExcursionInfo.Count++;

	if(gExcursionInfo.Count >= 32)					//ÿ32�ļ��һ����Ư
	{
		m_ErrIu = gExcursionInfo.TotalIu >> 5;
		m_ErrIv = gExcursionInfo.TotalIv >> 5;
        if(-32768 == m_ErrIu)                       //��ֹȡ����ֵʱ���
            m_ErrIu = -32767;
        if(-32768 == m_ErrIv)
            m_ErrIv = -32767;		
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.Count = 0;
		
		gMainStatus.StatusWord.bit.RunEnable = 1;
		if( (abs(m_ErrIu) < 5120) && (abs(m_ErrIv) < 5120) )
		{
			gExcursionInfo.ErrIu = m_ErrIu;
			gExcursionInfo.ErrIv = m_ErrIv;
			gExcursionInfo.ErrCnt = 0;
		}
		else if((gExcursionInfo.ErrCnt++) > 5)		//����5����Ư������ű�18����
		{
			gError.ErrorCode.all |= ERROR_CURRENT_CHECK;
			gExcursionInfo.ErrCnt = 0;
			gExcursionInfo.EnableCount = 0;
		}
	}
}

/****************************************************************
	��ȡĸ�ߵ�ѹ���ݣ����gUDC
*****************************************************************/
void GetUDCInfo(void)
{
	Uint m_uDC;
	//int	 m_DetaUdc;

   	m_uDC = ((Uint32)ADC_UDC * gUDC.Coff)>>16;                  //9
   	gUDC.uDC = (gUDC.uDC + m_uDC)>>1;
   	gUDC.uDCFilter = gUDC.uDCFilter - (gUDC.uDCFilter>>3) + (gUDC.uDC>>3);

	gUDC.uDCBigFilter = Filter32(gUDC.uDC,gUDC.uDCBigFilter);   // Wc = 1Hz; trise > 1ms
}

/****************************************************************
	��ȡ�����������ݣ����gCurSamp
*****************************************************************/
void GetCurrentInfo(void)
{
	long  m_Iu,m_Iv;


	gExcursionInfo.Iu = (int)(ADC_IU - (Uint)32768);
	m_Iu = (long)gExcursionInfo.Iu - (long)gExcursionInfo.ErrIu;	//ȥ����Ư
    gExcursionInfo.IuValue = m_Iu;                                  //���ڲ�����ʶ��������ʶ�Ż��󽫲���ʹ�øñ���                       
	gShortGnd.ShortCur = Filter32(m_Iu, gShortGnd.ShortCur);
	m_Iu = (m_Iu * gCurSamp.Coff) >> 3;
	m_Iu = __IQsat(m_Iu, C_MAX_PER, -C_MAX_PER);   
    
	gExcursionInfo.Iv = (int)(ADC_IV - (Uint)32768);
	m_Iv = (long)gExcursionInfo.Iv - (long)gExcursionInfo.ErrIv;	//ȥ����Ư
    gExcursionInfo.IvValue = m_Iv;    
	m_Iv = (m_Iv * gUVCoff.UDivV) >> 12;						    //��������ƫ��
	m_Iv = (m_Iv * gCurSamp.Coff) >> 3;	
	m_Iv = __IQsat(m_Iv, C_MAX_PER, -C_MAX_PER);

    gIUVWQ24.U = m_Iu;                      /*��ʹ���޳�ë���˲�����2011.05.07 L1082*/
    gIUVWQ24.V = m_Iv;

	gIUVWQ24.W = - (gIUVWQ24.U + gIUVWQ24.V);
	gIUVWQ24.W = __IQsat(gIUVWQ24.W,C_MAX_PER,-C_MAX_PER);

    gTemperature.TempAD = Filter16((ADC_TEMP &0xFFF0), gTemperature.TempAD);
    gAI.ai1Total += (ADC_AI1);
    gAI.ai2Total += (ADC_AI2);
    gAI.ai3Total += (ADC_AI3);
    gAI.aiCounter ++;
}


