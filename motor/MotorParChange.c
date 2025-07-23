/****************************************************************
�ļ����ܣ�����ӹ��ܲ��ֻ�õ����в���
�ļ��汾�� 
�������ڣ� 

****************************************************************/
#include "MotorInclude.h"

// // ͣ������ת��
void SystemParChg2Ms()	
{
    long	m_Long;
	Ulong	m_ULong;
    Uint    minType, maxType;
    Uint    *pInvVolt, *pInvCur;
    Uint 	m_UData;
    //Uint	m_UData1,m_UData2,m_UData3;
    
// ��ȡ��Ƶ��ϵͳ����
    //gInvInfo.GPType = gInvInfo.GpTypeSet;
	if(100 > gInvInfo.InvTypeSet)           // 380V
	{
        pInvVolt  = (Uint *)gInvVoltageInfo380T;
        pInvCur = (Uint *)gInvCurrentTable380T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet;
        gInvInfo.InvVoltageType = INV_VOLTAGE_380V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
	}
	else if(200 > gInvInfo.InvTypeSet)      // 220T, û��P�ͻ�
	{
	    pInvVolt  = (Uint *)gInvVoltageInfo220T;
        //pInvCur = (Uint *)gInvCurrentTable380T;            //ʹ��380V��Ƶ���Ķ������Ҳ���Ǳ����ƵĻ���
        pInvCur   = (Uint *)gInvCurrentTable220T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 100;
        
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;
        gInvInfo.GPType   = 1;                                      //������P�ͻ�
	}
    else if(300 > gInvInfo.InvTypeSet) //220S
    {
        pInvVolt  = (Uint *)gInvVoltageInfo220S;
        pInvCur = (Uint *)gInvCurrentTable220S;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 200;
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;  
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }
    else if(400 > gInvInfo.InvTypeSet) //480V
    {
        pInvVolt  = (Uint *)gInvVoltageInfo480T;
        pInvCur = (Uint *)gInvCurrentTable380T;  // ��380V���õ�����
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 300;
        gInvInfo.InvVoltageType = INV_VOLTAGE_480V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }
    else if(500 > gInvInfo.InvTypeSet) //690V                               //690V
    {
        pInvVolt  = (Uint *)gInvVoltageInfo690T;
        pInvCur = (Uint *)gInvCurrentTable690T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 400;
        gInvInfo.InvVoltageType = INV_VOLTAGE_690V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }
    else
    {
        pInvVolt  = (Uint *)gInvVoltageInfo1140T;
        pInvCur = (Uint *)gInvCurrentTable1140T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 500;
        gInvInfo.InvVoltageType = INV_VOLTAGE_1140V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }    
    
	gInvInfo.InvVolt     = *(pInvVolt + 0);
	gInvInfo.InvLowUdcStad = *(pInvVolt + 1);
	gInvInfo.InvUpUDC    = *(pInvVolt + 2);
	gInvInfo.BaseUdc     = *(pInvVolt + 3);
    minType              = *(pInvVolt + 4);  //��Ƶ������Ļ�������
    maxType              = *(pInvVolt + 5);  //��Ƶ������Ļ�������
    gInLose.ForeInvType  = *(pInvVolt + 6);  //��ʼȱ�ౣ������ʼ����
    gUDC.uDCADCoff       = *(pInvVolt + 7); 
    gVFPar.ovPointCoff   = *(pInvVolt + 8);     // ���ڼ����ѹ���Ƶ�

	gInvInfo.InvType = (gInvInfo.InvType > maxType) ? maxType : gInvInfo.InvType;
	if(gInvInfo.InvType <= minType)
    {
        gInvInfo.GPType = 1;	                        //��С�����޷�����GP
        gInvInfo.InvType = minType;
	}

    if(gInvInfo.GPType == 1)    gInvInfo.InvTypeApply = gInvInfo.InvType;
    else                        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P�ͻ�����������һ��
    
	gInvInfo.InvCurrent  = *(pInvCur + gInvInfo.InvTypeApply - minType);        //��������ʹ�õĶ����
    gInvInfo.InvCurrentOvload = *(pInvCur + gInvInfo.InvType - minType);
    if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)                             // ����С�����ȷ��
    {   
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 19) ? 10 : 1;
    }
    else
    {
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 21) ? 10 : 1; 
    }
    
	if((1 != gInvInfo.GPType) && (22 == gInvInfo.InvType))
	{
        //gInvInfo.InvCurrent /= 10;      // ����Ϊ22��P�ͻ�������Ӧ����1λС���㣬���Ƕ�ȡ�Ĳ���������2λ   
        gInvInfo.InvCurrent = (Ulong)gInvInfo.InvCurrent * 3264 >> 15;
    }
    gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P�ͻ�ʹ�õĶ����

//������������������
	m_UData = (gInvInfo.InvTypeApply > 24) ? 24 : gInvInfo.InvTypeApply;
	m_UData = (m_UData < 12) ? 12 : m_UData;
    if(500 <= gInvInfo.InvTypeSet)                //1140V����������������ȷ�� 2011.5.8 L1082
    {
        gDeadBand.DeadBand = DBTIME_1140V*gDeadBand.DeadTimeSet/10;                      //�����̶�7.0us��������2.5us��������ɵ�
        gDeadBand.Comp     = DCTIME_1140V*gDeadBand.DeadTimeSet/10;
    }
    else if(400 <= gInvInfo.InvTypeSet)                       //690V���������Ͳ������̶�
    {
	    gDeadBand.DeadBand = gDeadBandTable[13];         //�����̶�4.8us��������2.5us
	    gDeadBand.Comp     = gDeadCompTable[13];
    }
    else
    {
	    gDeadBand.DeadBand = gDeadBandTable[m_UData - 12];
	    gDeadBand.Comp     = gDeadCompTable[m_UData - 12];
    }
	EALLOW;									//��������ʱ��
	EPwm1Regs.DBFED = gDeadBand.DeadBand;
	EPwm1Regs.DBRED = gDeadBand.DeadBand;
	EPwm2Regs.DBFED = gDeadBand.DeadBand;
	EPwm2Regs.DBRED = gDeadBand.DeadBand;
	EPwm3Regs.DBFED = gDeadBand.DeadBand;
	EPwm3Regs.DBRED = gDeadBand.DeadBand;
	EDIS;
        
//����ADC�����ӳ�ʱ��
    #ifdef		DSP_CLOCK100
	gADC.DelayApply = gADC.DelaySet * 10;       // default: 0.5us
    #else
	gADC.DelayApply = gADC.DelaySet * 6;
    #endif

//�������ϵ���͵�ѹϵ��
	//�������̫С�Ĵ���
	m_UData = gInvInfo.InvCurrent>>2;
	DINT;
	gMotorInfo.Current = (gMotorInfo.CurrentGet < m_UData) ? m_UData : gMotorInfo.CurrentGet;
	m_ULong = (((Ulong)gMotorInfo.Current)<<8) / gMotorInfo.CurrentGet;
	gMotorInfo.CurBaseCoff = (m_ULong > 32767) ? 32767 : m_ULong;
	EINT;
    
	//ADֵ32767��Ӧ��Ƶ�������ֵ��2�� ת��Ϊ���������ı�ôֵ��ʾ(Q24)
	//ADת��ֵ�л�Ϊ�����ôֵ��ʾֵ�ķ���Ϊ: (AD/32767 * 2sqrt(2) * Iv/Im) << 24 *8
	// (1/32767 * 2sqrt(2) << 24) * 8 == 11586
	// CPU28035ʱ��32767��Ӧ�ĵ���ֵΪ: 2sqrt(2) *3.3/3.0 * Iv
/*
#ifdef TMS320F2808
    m_Long = ((long)gInvInfo.InvCurrent * 11586L)/gMotorInfo.Current;
#else
	m_Long = ((long)gInvInfo.InvCurrent * 12745L)/gMotorInfo.Current;
#endif
	gCurSamp.Coff = (m_Long * (long)gInvInfo.CurrentCoff) / 1000;

#ifdef TMS320F28035
    gUDC.uDCADCoff = (long)gUDC.uDCADCoff * 3300L / 3000;   // *1.1
#endif
	gUDC.Coff = ((Ulong)gUDC.uDCADCoff * (Ulong)gInvInfo.UDCCoff) / 1000;
*/

//���㲻ͬ�ֱ��ʵ�Ƶ�ʱ�ʾ
    if( 0 == gExtendCmd.bit.FreqUint )      // unit: 1Hz
    {
        gMainCmd.si2puCoeff = 1;
        gMainCmd.pu2siCoeff = 100;
    }        
    else if(1 == gExtendCmd.bit.FreqUint)   // unit: 0.1Hz
    {
        gMainCmd.si2puCoeff = 10;
        gMainCmd.pu2siCoeff = 10;
    }
    else // 2 == frqUnit                        // unit: 0.01Hz
    {
        gMainCmd.si2puCoeff = 100;       // si 2 pu
        gMainCmd.pu2siCoeff = 1;         // pu 2 si
    }    
	gBasePar.FullFreq01 = (Ulong)gBasePar.MaxFreq * (Ulong)gMainCmd.pu2siCoeff + 2000;	//32767��ʾ��Ƶ��ֵ
	gBasePar.FullFreq =   gBasePar.MaxFreq + 20 * gMainCmd.si2puCoeff;	//Ƶ�ʻ�ֵ
	gMotorInfo.FreqPer =  ((Ulong)gMotorInfo.Frequency <<15) / gBasePar.FullFreq;

    gMotorInfo.Motor_HFreq = ((Ulong)gMotorInfo.Frequency * 410) >>10;
    gMotorInfo.Motor_LFreq = ((Ulong)gMotorInfo.Frequency * 205) >>10;

    // ������һЩ�����ļ��㣬��ʡcpuʱ�俪��
    //gUVWPG.UvwPolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gUVWPG.UvwPoles;    // Q8 ���
    gRotorTrans.PolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gRotorTrans.Poles; // Q8
}

// // �����в���ת��
void RunStateParChg2Ms()	
{
    //int     temp;
    Uint    m_UData, tempU;
    Ulong   m_Long;    
    Uint    m_AbsFreq;
    //long    mCurM, mCurT;
     Ulong   tmpAmp;
	gIUVWQ12.U = (int)(gIUVWQ24.U>>12);				
	gIUVWQ12.V = (int)(gIUVWQ24.V>>12);
	gIUVWQ12.W = (int)(gIUVWQ24.W>>12);

    gIMTSetQ12.M = (int)(gIMTSetApply.M >> 12);
    gIMTSetQ12.T = (int)(gIMTSetApply.T >> 12);

	m_AbsFreq = abs(gMainCmd.FreqSyn);
    tempU = ((Ulong)gMotorExtPer.I0 * gMotorInfo.FreqPer) / m_AbsFreq;
    gMotorExtPer.IoVsFreq = (m_AbsFreq < gMotorInfo.FreqPer) ? gMotorExtPer.I0 : tempU;
      
    //�����ߵ���
    gIAmpTheta.CurTmpM = abs(gIMTQ12.M);
    gIAmpTheta.CurTmpT = abs(gIMTQ12.T);
    tmpAmp = (long)gIAmpTheta.CurTmpM * gIAmpTheta.CurTmpM;
    tmpAmp += (long)gIAmpTheta.CurTmpT * gIAmpTheta.CurTmpT;
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);
    //...................................�����ߵ���
    if((gMainCmd.Command.bit.StartDC == 1) || 
       (gMainCmd.Command.bit.StopDC == 1))	/****ֱ���ƶ�״̬��ʾ�����Ŵ���****/
    {
        gIAmpTheta.Amp = ((Ulong)gIAmpTheta.Amp * 5792)>>12;
    }
	gLineCur.CurPer = Filter2(gIAmpTheta.Amp, gLineCur.CurPer);   
	gLineCur.CurPerFilter += gLineCur.CurPer - (gLineCur.CurPerFilter>>7);	

	// �����Ƶ�������Ϊ��ֵ�ı�ôֵ����
	m_Long = (Ulong)gLineCur.CurPer * gMotorInfo.Current;
	gLineCur.CurBaseInv = m_Long/gInvInfo.InvCurrentOvload;
    
    // ͬ���� ������������
    gDeadBand.InvCurFilter = Filter2(gLineCur.CurBaseInv, gDeadBand.InvCurFilter);
	m_UData = abs(gMainCmd.FreqSyn);                                   //������ʵ��ֵ�������Ƶ�?
	gMainCmd.FreqReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    gMainCmd.FreqRealFilt += (gMainCmd.FreqReal>>4) - (gMainCmd.FreqRealFilt>>4);
    m_UData = abs(gMainCmd.FreqDesired);
    gMainCmd.FreqDesiredReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    m_UData = abs(gMainCmd.FreqSet);
    gMainCmd.FreqSetReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    gMainCmd.FreqSetBak = gMainCmd.FreqSet;
    
// Ƿѹ��ɸ��ݹ��������
	gInvInfo.InvLowUDC = (long)gInvInfo.InvLowUdcStad * gInvInfo.LowUdcCoff / 1000;     

// ��гʱ�������洦��
    if(gMainStatus.RunStep == STATUS_GET_PAR)
	{
		gUVCoff.UDivV = 4096;
	}
	else
	{
		//gUVCoff.UDivV = ((Ulong)gUVCoff.UDivVGet<<12)/1000;
        gUVCoff.UDivV = (Ulong)gUVCoff.UDivVGet * 4160L >> 10; // Q12
	}
    

// ����ĸ�ߵ�ѹ������������ѹ
    //��������ѹ��Сʱ���п����������Ҫ�ر���	
    if((abs(gUDC.uDC - gUDC.uDcCalMax) > 200)//ĸ�ߵ�ѹ�仯20V�����¼�������ѹ
       || (gMainStatus.RunStep == STATUS_STOP))
    {
        gUDC.uDcCalMax = gUDC.uDC;    
        m_UData = ((long)gUDC.uDC * 3251L)>>13;//���ҷ���������ѹΪUdc/2.52
        gOutVolt.MaxOutVolt = ((long)m_UData * 710L) / gMotorInfo.Votage;//����ѹֵ��0.1VΪ��λ�������ѹ��1V��ʹ�����ѹΪ��ֵ 
    }

// VF �����Ŵ���
    gVarAvr.CoffApply = gVFPar.VFOverExc;
   
// ͬ������ز������⴦�� 
	if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
	{
        // ת�������趨�ı��������λ�ý�
        //gIPMPos.RotorZero = ((Ulong)gIPMPos.RotorZeroGet<<16)/3600;
        gIPMPos.RotorZero = (Ulong)gIPMPos.RotorZeroGet * 18641L >> 10;
        //gUVWPG.UvwZeroPhase = (Ulong)gUVWPG.UvwZeroPhase_deg * 18641L >> 10;
        gUVWPG.UvwZeroPos = gUVWPG.UvwZeroPos_deg * 18641L >> 10;

        gUVWPG.UvwZIntErr_deg = (long)gUVWPG.UvwZIntErr * 180L >>15;
        gIPMPosCheck.UvwStopErr_deg = (long)gIPMPosCheck.UvwStopErr *180L >>15;
        gIPMPos.AbzErrPos_deg = (long)gIPMPos.AbzErrPos * 180L >>15;
        #if 0
        tempU = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;
        gUVWPG.UvwRealErr_deg = (int)(gIPMPos.RotorPos - tempU) * 180L >>15;
        #endif
        
        // �ֶ��޸����λ�ý�ʱ��Ҫ��Ӧ, ������Զ�����
        if(gPGData.PGMode == 0 && gMainStatus.ParaCalTimes == 1 && // �Ѿ��ϵ�
            gMainStatus.RunStep != STATUS_GET_PAR &&
            gIPMPos.ZeroPosLast != gIPMPos.RotorZero)
        {
            tempU = gIPMPos.RotorZero - gIPMPos.ZeroPosLast;
            
            SetIPMPos(gIPMPos.RotorPos + tempU);
            SetIPMPos_ABZRef(gIPMPos.RotorPos + tempU);            
        }   
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
        gPmDecoup.EnableDcp = 0;          
	}

// ���ݱ�ʶ�õ��ķ����޸�QEP����
// ������ʶ��ʱ�����ﲻ��Ӱ��
    if(gMainStatus.RunStep != STATUS_GET_PAR)
    {
        EQepRegs->QDECCTL.bit.SWAP = gPGData.SpeedDir;
    }

// ������ѹУ��ϵ��ʵʱ���㣬�������
#ifdef TMS320F2808
    m_Long = ((long)gInvInfo.InvCurrent * 11586L)/gMotorInfo.Current;
#else
	m_Long = ((long)gInvInfo.InvCurrent * 12745L)/gMotorInfo.Current;
#endif
	//gCurSamp.Coff = (m_Long * (long)gInvInfo.CurrentCoff) / 1000;
	gCurSamp.Coff = (m_Long * (Ulong)gInvInfo.CurrentCoff) * 33 >>15;
#ifdef TMS320F2808
    //gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff / 1000;
    gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff * 33 >>15;
#else   // TMS320F28035
    gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff * 36 >>15;     // *1.1
#endif
    gOvUdc.Limit = gVFPar.ovPoint * gVFPar.ovPointCoff;
    if     (100 > gInvInfo.InvTypeSet)
        {
          gOvUdc.Limit = (gOvUdc.Limit>7500)?7500:gOvUdc.Limit;
        }
    else if(300 > gInvInfo.InvTypeSet)
        {
          gOvUdc.Limit = (gOvUdc.Limit>3700)?3700:gOvUdc.Limit;
        }
    else if(400 > gInvInfo.InvTypeSet)
        {
          gOvUdc.Limit = (gOvUdc.Limit>8300)?8300:gOvUdc.Limit;
        }
    else if(500 > gInvInfo.InvTypeSet)
        {
          gOvUdc.Limit = (gOvUdc.Limit>13000)?13000:gOvUdc.Limit;
        }
    else
        {
         gOvUdc.Limit = (gOvUdc.Limit>19000)?19000:gOvUdc.Limit;
        }
}

/***************************************************************
����������򣺴���0.5Msѭ���У����ܴ��ݵĲ�������ɲ���ת�������в���׼���ȹ���
1. update gCtrMotorType;
2. measure speed ;
*************************************************************/
void SystemParChg05Ms()
{
    //Ulong m_Long;

// ������͸���
    if(gMotorInfo.LastMotorType != gMotorInfo.MotorType)
    {
        gMotorInfo.LastMotorType = gMotorInfo.MotorType;

        gPGData.PGType = PG_TYPE_NULL;     // ��������޸ĺ����³�ʼ��������
    }
    
// PG�����͸���
    if(gPGData.PGType != (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun)
    {
        gPGData.PGType = (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun;
        ReInitForPG();

        gIPMInitPos.Flag = 0;
    }
    
// QEP���ٵĴ��� -- ѡ��QEP
	if(gPGData.QEPIndex != (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex)
	{
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
                
        if(gPGData.QEPIndex == QEP_SELECT_1) // ����PG������
        {
            EQepRegs = (struct EQEP_REGS *)&EQep1Regs;
            EALLOW;
            PieVectTable.EQEP1_INT = &PG_Zero_isr;
            PieCtrlRegs.PIEIER5.bit.INTx1 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;
            EDIS;
        }
        #ifdef TMS320F2808                      // 28035 ֻ��һ��QEP
        if(gPGData.QEPIndex == QEP_SELECT_2)
        {
            EQepRegs = (struct EQEP_REGS *)&EQep2Regs;
            EALLOW;
            PieVectTable.EQEP2_INT = &PG_Zero_isr;
            PieCtrlRegs.PIEIER5.bit.INTx2 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;
            EDIS;
        }
        #endif        
        InitSetQEP();        
    }    

    if(MOTOR_TYPE_PM != gMotorInfo.MotorType ||     // �첽��
        gPGData.PGMode != 0)                        // ����λ�ñ����� -- ����
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0;  //ȡ��QEP��I�ź��ж�
        EDIS;
    }
    else
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0400; //����λ�ñ������Ƿ���Ҫ���ж�?
        EDIS;
    }
}

/*************************************************************
	ͬ��������첽��������任
	
*************************************************************/
void ChangeMotorPar(void)
{
	Uint m_UData,m_BaseL;
	Ulong m_Ulong;
	//Uint m_AbsFreq;
    
	//��л�ֵΪ���迹�ֵ/2*pi*���Ƶ��
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01;
    
    if(MOTOR_TYPE_PM != gMotorInfo.MotorType)
    {
    	//�迹��ֵΪ���ѹ/������������ôֵQ16��ʽ
    	// sqrt(3)/1000/100 <<16 = 18597 >>14
    	m_UData = ((Ulong)gMotorExtInfo.R1 * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    	gMotorExtPer.R1 = ((Ulong)m_UData * 18597)>>14;                 // �첽�����ӵ���
        
    	m_UData = ((Ulong)gMotorExtInfo.R2 * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    	gMotorExtPer.R2 = ((Ulong)m_UData * 18597)>>14;                 // �첽�����ӵ���

    	gMotorExtInfo.L1 = 	gMotorExtInfo.LM + ((Ulong)gMotorExtInfo.L0 * 102 >> 10);
    	gMotorExtInfo.L2 = gMotorExtInfo.L1;

    	m_Ulong = (((Ulong)gMotorExtInfo.L1<<11) + m_BaseL)>>1;		
    	gMotorExtPer.L1 = m_Ulong/m_BaseL;
    	gMotorExtPer.L2 = gMotorExtPer.L1;							//���ӡ�ת�ӵ�б�ôֵQ9��ʽ
    	m_Ulong = (((Ulong)gMotorExtInfo.LM<<11) + m_BaseL)>>1;
    	gMotorExtPer.LM = m_Ulong/m_BaseL;							//©�б�ôֵQ14��ʽ
    	gMotorExtPer.L0 = (gMotorExtPer.L1 - gMotorExtPer.LM)<<5;

        gMotorExtPer.I0 = (((Ulong)gMotorExtInfo.I0)<<12)/gMotorInfo.Current;	//���ص���

        m_Ulong = 4096L * 4096L - (long)gMotorExtPer.I0 * gMotorExtPer.I0;
        gMotorExtPer.ItRated = qsqrt(m_Ulong);
        gPowerTrq.rpItRated = (1000L<<12) / gMotorExtPer.ItRated;
    }
    else    // PMSM
    {
        m_UData = ((Ulong)gMotorExtInfo.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
        gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
        
        m_Ulong = (((Ulong)gMotorExtInfo.LD <<11) + m_BaseL) >>1;
        gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // ͬ����d����Q9, ��λ���첽��С��������
        m_Ulong = (((Ulong)gMotorExtInfo.LQ <<11) + m_BaseL) >>1;
        gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;                   // ͬ����q����Q9����λ���첽��С��������

        // ����ͬ����ת�Ӵ���
        m_Ulong = ((long)gMotorExtInfo.BemfVolt <<12) / (gMotorInfo.Votage *10);      // Q12
        gMotorExtPer.FluxRotor = (m_Ulong << 15) / gMotorInfo.FreqPer;                   // Q12\

        //gMotorExtPer.ItRated = 4096L;
        //gPowerTrq.rpItRated = (1000L<<12) / gMotorExtPer.ItRated;
        gPowerTrq.rpItRated = 1000;
    }

	//....������������
	m_Ulong = (((Ullong)gMotorInfo.Frequency * (Ullong)gMainCmd.pu2siCoeff * 19661L)>>15);
	gMotorExtInfo.Poles = (m_Ulong + (gMotorExtInfo.Rpm>>1)) / gMotorExtInfo.Rpm;

    //0.01HzΪ��λ�Ķת����
    //m_Ulong = ((Ulong)gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 100L)/60;
    m_Ulong = gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 6830L >> 12;
    gMotorExtInfo.RatedComp = (Ulong)gMotorInfo.Frequency * gMainCmd.pu2siCoeff - m_Ulong;
                              
    //��ô���Ķת����       
    gMotorExtPer.RatedComp = ((long)gMotorExtInfo.RatedComp << 15)/gBasePar.FullFreq01; 
}

// // ��Ƶ��������ʡ�ת�ؼ���
void InvCalcPower(void)
{
    int  temp;
    //long InvPowerPU;
    long m_PowerN;
    
    gPowerTrq.anglePF = Filter16(abs(gIAmpTheta.PowerAngle), gPowerTrq.anglePF);
    gPowerTrq.anglePF = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.anglePF : 0;
    gPowerTrq.Cur_Ft4 = Filter16(gLineCur.CurPer, gPowerTrq.Cur_Ft4);
    
    gPowerTrq.InvPowerPU = (1732L * (long)gOutVolt.VoltApply /1000L) * gPowerTrq.Cur_Ft4 >> 12;
    gPowerTrq.InvPowerPU = (long)gPowerTrq.InvPowerPU * qsin(16384 - gPowerTrq.anglePF) >> 15;

	//m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage)/1000;
    m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) >> 10;
    m_PowerN = (gInvInfo.InvTypeApply >= 22) ? m_PowerN : (m_PowerN*409L>>12);   // 0.01->0.1
	gPowerTrq.InvPower_si= ((long)gPowerTrq.InvPowerPU * m_PowerN) >> 12;    	
    // invType: < 22 : 0.01Kw;       current: 0.01A
    // invType: >=22 : 0.1Kw;        current: 0.1A
    gPowerTrq.InvPower_si = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.InvPower_si : 0;

    //temp = (gInvInfo.InvTypeApply < 22) ? (gMotorInfo.Power*10) : gMotorInfo.Power;
    //tempL = ((long)gInvPower_si << 10) / temp;
    //gTrqOut_pu = tempL * gMotorInfo.FreqPer / temp;
    //gTrqOut_pu = (gMainStatus.StatusWord.bit.StartStop) ? gTrqOut_pu : 0;       // ͣ��״̬���⴦��
    //gTrqOut_pu = __IQsat(gTrqOut_pu, 3000, - 3000);      // abs(trq) < 300.0%
    
    gPowerTrq.TrqOut_pu = (long)gLineCur.CurTorque * gPowerTrq.rpItRated >> 13;
    gPowerTrq.TrqOut_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOut_pu : 0;       // ͣ��״̬���⴦��
}

