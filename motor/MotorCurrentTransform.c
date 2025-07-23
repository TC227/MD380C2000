/****************************************************************
�ļ����ܣ���������ѹ������任
�ļ��汾��
���¸��£�
	
****************************************************************/
#include "MotorCurrentTransform.h"
#include "MotorInclude.h"
#include "MotorEncoder.h"

//extern MT_STRUCT_Q24           gIMTQ24_obs;
//extern MT_STRUCT_Q24           gIMTQ12_obs;
/*******************************************************************
    ���ڲ����˱�ôֵϵͳ��Ҫ�����е�����任�����뱣֤��ֵ��ȵı任��
���ɣ���ôֵϵͳ�£���ֵȷ����ų��任��
********************************************************************/
/*******************************************************************
Date Type Q24(��֤��ֵ���������任)(������ЧֵΪ1���Ҳ�������任��ֵΪ1)
	Alph= U * (1/2)^0.5 
	Beta= (3^0.5/2) * (U + 2*V)
	UVW�������Է�ֵ��ʾ�ģ�ALPH BETA��M T�����������Чֵ���
********************************************************************/
void inline UVWToAlphBetaAxes(UVW_STRUCT_Q24 * uvw, ALPHABETA_STRUCT * AlphBeta)
{
	AlphBeta->Alph = ((llong)uvw->U * 23170L)>>15;	

	AlphBeta->Beta = ((llong)((long)uvw->V - (long)uvw->W) * 13377L)>>15;
}

/*******************************************************************
Date Type Q12 ��q�ᳬd��90�ȣ�
	d= cos(theta)*alph + sin(theta)*beta;
	q= -sin(theta)*alph + cos(theta)*beta;
********************************************************************/
void AlphBetaToDQ(ALPHABETA_STRUCT * AlphBeta, int angle, MT_STRUCT_Q24 * MT)
{
	int m_sin,m_cos;

	m_sin  = qsin(angle);
	m_cos  = qsin(16384 - angle);
	MT->M = ( ((llong)m_cos * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_sin * (llong)(AlphBeta->Beta)) )>>15;
	MT->T = (-((llong)m_sin * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_cos * (llong)(AlphBeta->Beta)) )>>15;
}

/*******************************************************************
Date Type Q12
	A= (d*d + q*q)^0.5
	q= atan(q/d)
********************************************************************/
void DQToAmpTheta(MT_STRUCT * MT,AMPTHETA_STRUCT * AmpTheta)
{
	long m_Input;

	m_Input = (((long)MT->M * (long)MT->M) + ((long)MT->T * (long)MT->T));
	AmpTheta->Amp = (Uint)qsqrt(m_Input);

	AmpTheta->Theta = atan(MT->M,MT->T);
}

/*************************************************************
	�����任����
*************************************************************/
void ChangeCurrent(void)
{
    //Ulong   m_Long;    
    int temp;


    // ��ȡ�������˲ʱֵ, �������ת��Ϊ���������������µĵ���
	UVWToAlphBetaAxes((UVW_STRUCT_Q24*)&gIUVWQ24,(ALPHABETA_STRUCT*)&gIAlphBeta);
    gIAlphBetaQ12.Alph = gIAlphBeta.Alph>>12;
    gIAlphBetaQ12.Beta = gIAlphBeta.Beta>>12;
	// ���������������µ���ת��ΪDQ�µĵ���
	AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,(gPhase.IMPhase>>16), &gIMTQ24);
    
        //AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,(gPhase.IMPhase>>16), &gIMTQ24_obs);
	gIMTQ12.M = Filter2((gIMTQ24.M>>12), gIMTQ12.M);
	gIMTQ12.T = Filter2((gIMTQ24.T>>12), gIMTQ12.T);

   // gIAmpTheta.Theta = atan(gIMTSetQ12.M, gIMTSetQ12.T);	//����MT��н�

    gIAmpTheta.Theta = atan(gIMTQ12.M, gIMTQ12.T);	//����MT��н�

    temp = gOutVolt.VoltPhaseApply - gIAmpTheta.Theta;
    gIAmpTheta.PowerAngle = Filter8(temp, gIAmpTheta.PowerAngle);
}

