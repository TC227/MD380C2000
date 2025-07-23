/***************************************************************
�ļ����ܣ�
�ļ��汾��VERSION 1.0
���¸��£�
************************************************************/
#ifndef MOTOR_PMSM_INCLUDE_H
#define MOTOR_PMSM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//**************************************************************************
typedef struct IPM_POS_CHECK_STRUCT_DEF {
	Uint	FirstPos;				//��һ��λ��
	Uint	Cnt;					//������
	long	TotalErr;				//λ��ƫ���ۼ�
	Ulong	TotalErrAbs;			//λ��ƫ��ľ���ֵ�ۼ�
	int     UvwStopErr;
    int     UvwStopErr_deg;              // uvw�����Ƕ�
    Uint    UvwRevCnt;              // ��UVW�ź���������������
}IPM_POS_CHECK_STRUCT;         //����ͬ������ϵ��⵱ǰ����λ�ýǵ����ݽṹ

typedef struct IPM_POSITION_STRUCT_DEF {
    Uint	InitPosMethod;          //��ʼ�ż�λ�ü�ⷽ��
    
	Uint	RotorPos;				//ת��λ�ýǱ�ôֵ
	Uint    ABZ_RotorPos_Ref;
	Uint	RealPos;				//ת��ʵʱ�Ƕ�(360.0�ȷ�Χ)

    Uint    InitPos;                // �ż���ʶ�õ���λ��
    Uint    InitAngle_deg;          // ��ʶ�õ��Ĵż�λ�ã�0-360.0deg
	
	Uint	RotorZero;				//���������λ�ýǣ�65536��Χ��
	Uint	RotorZeroGet;			//���������λ�ý�(360.0�ȷ�Χ)
	Uint    ZeroPosLast;            // ���ABZ������ʱ�ֶ��޸����λ�ý�

    Uint    PowerOffPos;            // �ϴε����ӵĽǶ�
	Uint	PowerOffPosDeg;			//�ϴ��µ�ʱ��ĽǶ�(360.0�ȷ�Χ)
	
	Uint	PosInZInfo;				//Z�źŵ���ʱ���ʵʱ�Ƕ�
	long	QepBak;					//��һ��QEP�ۼ���ֵ
	long	QepTotal;

    long      ABZ_QepTotal;
    long      ABZ_QepBak;  
    
    Uint    Zcounter;               // ��⵽Z�źŵĸ������������

	Uint    ZErrCnt;			    //Z�źŴ��������
	int     AbzErrPos;   
    int     AbzErrPos_deg;
}IPM_POSITION_STRUCT; //����ͬ�������ת�ӽǶ���صĽṹ

typedef struct PM_INIT_POSITION_DEF
{
	int SubStep;
	int PeriodCnt;
	int Section;	
	int CurFirst;
	int Cur[12];
	int Timer;
	unsigned int PWMTs;
	unsigned int PWMTGet;
	unsigned int PWMTSet;
}PM_INIT_POSITION;

typedef struct PM_FLUX_WEAK_DEF
{
    Uint    Mode;              // ����ģʽѡ�� 0: �����ţ�1: ����ֱ�Ӽ��㣻2: �����Զ�����
    int     DecoupleMode;      // ����ģʽ 0: �����1: ����
    int     CsrGainMode;        // ��������������ģʽ 0: ��������1: ����
    int     CoefFlux;          // ����ϵ��
	long     VoltCoef;          // voltage coefficient (%)
    long    FluxD;             // d���ܴ��� PhiD = PhiSd + FluxRotor
    int     FluxSd;            // PhiSd = Ld * gIMTQ12.M
	long     IqLpf;
    int     VoltLpf;            // �����ѹ�˲�ֵ
    int     AbsFrqLpf;
    int     IdSet;
    int     IdMax;              // ����d ��������� 1%
    int     Vd;
    int     Vq;
	int     AdjustId;           // ģʽ2 ������
    int     AdjustLimit;        // ģʽ2 ����������
	int     CoefAdj;            // ģʽ2 ����ϵ�� 
    int     CoefKI;             // ����ʱ�����������������ϵ��

    //�������ӵı���
    long    VoltMax;
    long    CurrCoef;
    long    Iq;   // Q����� ��λΪuH ��Ҳ�ɵ���Q24��ʽ
    long    Omg;  // ʵ�ʽ��ٶȣ�Q10��ʽ
    long    Id;   // ���ŵ�����Q12��ʽ��ôֵ
    long    IqFBLpf;// Q���˲�����,Q12��ʽ��ôֵ
    long    IqLimit;
    long    TorqeCurr;//������û��ʹ��
    long    AdjustId1;// ������û��ʹ��
    int     CoefIdComp;// IS300 ��gSendToMotorDataBuff1[14]
    long    IqErr;     // Q�����ƫ�Q12��ʽ��ôֵ
    int     IqErrAbs;
    long    OmgQ15;   // ʵ��Ƶ�ʵ��˲�ֵ
    int     Ratio;   
    int     VoltOut;


    long    data0;
    long    data1;
    long    data2;
    long    data3;
    long    data4;
    long    data5;
    long    data6;
    long    data7;
    long    data8;
    long    data9;
    long    ud;
    long    uq;
   
        
}PM_FLUX_WEAK;

typedef struct PM_DECOUPLE_DEF{
    int Omeg;   // Q15
    int Isd;    // Q12
    int Isq;    // Q12
    int PhiSd;  // Q12      d��ȫ����
    int PhiSq;  // Q12
    int RotVd;  // Q12
    int RotVq;  // Q12
    int EnableDcp;
}PM_DECOUPLE;

typedef struct PM_FW_IN_DEF
{
	int  Time;			//���ſ���ʱ����,0.5MS
	int  Bemf;			//1mv/(rad/s),���綯��ϵ���ķ�ֵ
	int  Udc;			//0.1V��ĸ�ߵ�ѹ
	int  R;				//1mohm,�����
	int  IsSetMax;      //�趨���������������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  IsAsrSetMax;	//�趨���ٶȻ�PI��������������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  IqSet;			//Q���趨������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  IqFeed;		//Q�ᷴ��������Чֵ���Ե�������Ϊ��ֵ��Q12	
	int  IdSet;			//Q���趨������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  IdFeed;		//Q�ᷴ��������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  VolCsrOut;		//������PI����������ߵ�ѹ��Ч���Ե����ߵ�ѹΪ��ֵ��Q12
	int  UdCsrOut;		//������D��PI����������ߵ�ѹ��Ч���Ե����ߵ�ѹΪ��ֵ��Q12
	int  UqCsrOut;		//������Q��PI����������ߵ�ѹ��Ч���Ե����ߵ�ѹΪ��ֵ��Q12

	int  IsAsrOut;		//�ٶȻ�PI����������������Чֵ���Ե�������Ϊ��ֵ��Q12
	int  MotorPoles;	//����ż�����
	int  FullFreq;		//0.1HZ�������׼Ƶ��
	int  FreqFeed;		//��������ٶȣ���FullFreqΪ��ֵ��Q15
	int  MotorVol;		//1V,������ѹ
	long MotorCurr;		//0.01A,��������

	int  MaxSetUdc;		//0.1V,�趨��������ĸ�ߵ�ѹ��Ϊ0ʱ���ֱ��ĸ�ߵ�ѹ
	int  RatioSet;		//���ſ���ʱ�����ѹ�趨ֵ������ȷ�����ŵ���
						//�Ա�Ƶ�������������ѹΪ��ֵ��Q12
						//����С��4096���Ƽ�ֵ3700��Udc==540V)
						//ĸ�ߵ�ѹԽС��ֵӦ�����ԽС
	long KpId;			//���ŵ�������KP

	long MaxDelId;		//ÿ������СID��������ֵ;
	int  AdjuMode;		//������ģʽ��0����������1���������ת�ص���,
						//2:����������ŵ�����3���������ת�ص��������ŵ���

	int  UdForIqMax;	//���ſ���ʱQ�������ѹ�趨ֵ,����ȷ�����ת�ص���
						//�Ա�Ƶ�������������ѹΪ��ֵ��Q12
						//�������RatioSet���Ƽ�ֵ3900��Udc==540V)
						//ĸ�ߵ�ѹԽС��ֵӦ�����ԽС

	long KpIqMax;		//���ת�ص�����С����KP
	long KpIqMax1;		//���ת�ص����������KP
							
	int  UqForIdMax;	//���ſ���ʱQ�������ѹ�趨ֵ������ȷ��������ŵ���
						//�Ա�Ƶ�������������ѹΪ��ֵ��Q12
						//�Ƽ�ֵ100��Udc==540V)��ĸ�ߵ�ѹԽС��ֵӦ�����Խ��
	long KpIdMax;		//������ŵ�������KP
	int  CsrMaxVolt;	//������D��Q����������ѹ
	
}PM_FW_IN;

typedef struct PM_FW_DEF
{	
	
	int  IdMax;
	int  IdMax1;
	int  PhiPerLd;
	int  UdcLpf;
	int  UdLpf;
	int  UqLpf;
	int  RatioLpf;
	int  UdForIqMax;
	long  RatioLpf1;
	int  CurPerLpf;
	int  TorqPerAmp;
	int  IqErrLpf;
	int  IdErrLpf;
	int  IdForTorq;
	
	long AbsOmgPer;
	long Omg;
	long OmgLpf;
	long CurrCoef;
	long CoefIqMax;
	
	long AdId;
	long AdIdIntg;

	long AdIqMax;	
	long AdIqMaxIntg;
	long AdIqMaxIntgMax;
	long AdIqMaxIntgMin;
	long AdIdMaxIntg;
	long TorqueCurr;
	long TorqueCurrMax;
	long MinUqLpf;
	long IqMax;
	long IqMax1;
	long PowerLpf;
	long TorqueEst;	
	long Ld;
	long Lq;
	long Lq1;
	int  TorqRevi;
	int  TReviCoef1;
	int  PosComp;
	int  MaxPosComp;
	int  MpcBack;
	int  MinPosComp;
	long IqSetLpf;
	int  UqCompStat;
	int  UqRatio;
	int  imSet;
	int  itSet;
	int  umSet;
	int  utSet;
	int  utSetLpf;
}PM_FW;
typedef struct PM_FW_OUT_DEF
{	
	
	int  IdSet;			//������D�����ֵ���Ե�������Ϊ��ֵ��Q12
	int  IqSet;			//������Q�����ֵ���Ե�������Ϊ��ֵ��Q12
	int  IsLimit;	   //�ٶȻ�PI��������޶�ֵ���Ե�������Ϊ��ֵ��Q12
	int  UqComp;		//Q�Ჹ����ѹ���Ե�����ѹΪ��ֵ��Q12
    int  PosComp;
	int  ClearKID;
}PM_FW_OUT;


//**************************************************************************
extern IPM_POSITION_STRUCT		gIPMPos;
extern PM_INIT_POSITION         gPMInitPos;
extern PM_FLUX_WEAK             gFluxWeak;
extern PM_DECOUPLE              gPmDecoup;
extern IPM_POS_CHECK_STRUCT	    gIPMPosCheck;
extern PM_FW_IN				   gPMFwIn;
extern PM_FW				   gPMFwCtrl;
extern PM_FW_OUT			   gPMFwOut;

//**************************************************************************
extern void SynCalRealPos(void);
extern void IPMCheckInitPos(void);
extern void PmChkInitPosRest(void);
extern void RunCaseIpmInitPos(void);
extern void SynCalLdAndLq(Uint m_Pos);
extern void IPMCalAcrPIDCoff(void);
extern void PmFluxWeakDeal();
extern void PmDecoupleDeal(void);
extern void PMClacFluxWeakId(void);
extern void PMSetFwPara1(void);
extern void PMSetFwPara(void);
#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
