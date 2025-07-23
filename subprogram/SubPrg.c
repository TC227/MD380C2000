/****************************************************************
�ļ����ܣ��������е��õĴ��ģ�麯��
�ļ��汾��
���¸��£�
	
****************************************************************/
 
#include "MotorDefine.h"
#include "SubPrgInclude.h"

#define  SETOVM asm(" SETC OVM")
#define  CLROVM asm(" CLRC OVM")

/****************************************************************
����˵���������к������ú�������x��y����õķ����нǶ��Լ�4���޵ĽǶ�
****************************************************************/
int atan(int x, int y)
{
	int  result;
	long m_Input;

	if(x == 0)
	{
		if(y < 0)			
		{
			return(-16384);
		}
		else
		{
			return(16384);
		}
	}
	m_Input = (((long)y)<<16)/x;
	result = qatan(m_Input);
	if(x < 0)
	{
		result += 32768;
	}
	return result;
}

/****************************************************************
����˵����PID����(��ʱ������D���������)
����ƫ��Ϊint�ͱ���
������Ϊlong�ͱ���������16λ�õ���Ҫ�Ľ��
����������pid�ڲ������� 4λ
****************************************************************/
void PID(PID_STRUCT * pid)
{
	long m_Max,m_Min,m_Out,m_OutKp,m_OutKi;
    long vMax = 0x7FFFFFFF;
    
    SETOVM;
 	m_Max = ((long)pid->Max)<<16;						// ���ֵ
	m_Min = ((long)pid->Min)<<16;						// ��Сֵ
	
	m_OutKp = (long)pid->KP * (long)pid->Deta;			// ����
	m_OutKp = __IQsat(m_OutKp, (vMax>> (4+ pid->QP)), -(vMax>> (4+ pid->QP))); //��֤ʹ����������
    m_OutKp = m_OutKp << (4+ pid->QP);
    
 	m_OutKi = (long)pid->KI * (long)pid->Deta;// ����
    m_OutKi = __IQsat(m_OutKi, (vMax >> pid->QI), -(vMax >> pid->QI));
    m_OutKi = m_OutKi << pid->QI;
    
    // ��������µ�ȥ���ʹ���
    if((m_OutKp > m_Max) && (pid->Total > 0))
    {
        pid->Total -= (pid->Total>>8) + 1;  //��1ȥ���˲�����
        m_OutKi = 0;
    }
    else if((m_OutKp < m_Min) && (pid->Total < 0))
    {
        pid->Total -= (pid->Total>>8) - 1; 
        m_OutKi = 0;
    }
	pid->Total += m_OutKi;	 
	m_Out       = pid->Total + m_OutKp;
 	pid->Out    = __IQsat(m_Out,m_Max,m_Min);
    pid->Total  = __IQsat(pid->Total,m_Max,m_Min);

    CLROVM;
}
/****************************************************************
����˵����
PID����(��ʱ������D���������)
����ƫ��Ϊlong�ͱ���;��������Ҫ��ƫ���Qֵ��ͬ

���Ϊllong�ͱ���������16λ�õ���Ҫ�Ľ��
*�������ӱ�������µ�ȥ���ʹ���*

����������pid�ڲ������� 4λ
****************************************************************/
void PIDLongRegulate(PID_STRUCT_LONG * pid)
{
	llong  m_Max,m_Min,m_Out,m_OutKp,m_OutKi;
 	m_Max = ((llong)pid->Max)<<16;						//���ֵ
	m_Min = ((llong)pid->Min)<<16;						//��Сֵ
    m_OutKp = (llong)pid->KP * (llong)pid->Deta << 4;		//����
	
	m_OutKi = (llong)pid->KI * (llong)pid->Deta;

// ��������µ�ȥ���ʹ���
    if(((m_OutKp > m_Max) && (pid->Total > 0)) ||
       ((m_OutKp < m_Min) && (pid->Total < 0))) 
    {
        pid->Total -= (pid->Total>>8);
        m_OutKi = 0;
    }

	pid->Total += m_OutKi;		
	m_Out      = pid->Total + m_OutKp;
    if(m_Out < m_Min)               pid->Out = m_Min;
    else if(pid->Out > m_Max)       pid->Out = m_Max;
    else                            pid->Out = m_Out;

    if(pid->Total < m_Min)          pid->Total = m_Min;
    else if(pid->Total > m_Max)     pid->Total = m_Max;      
}


void PID32(PID32_STRUCT * pid)
{
	long  m_OutKp,m_OutKi;
	long  mTotalMax,mTotalMin;

	//if(pid->Deta == 0)		return;

	//����������ã���������������
	m_OutKp = ((llong)pid->KP * (llong)pid->Deta) >> (16-4);	
	//�������ֵ���(��)��
	if(pid->Deta > 0)		
	{
		mTotalMax = pid->Max - m_OutKp;
		if(mTotalMax < 0)	mTotalMax  = 0;

		mTotalMin = pid->Min;
	}
	else
	{
		mTotalMin = pid->Min - m_OutKp;
		if(mTotalMin > 0)	mTotalMin  = 0;

		mTotalMax = pid->Max;
	}

    //�����������
	m_OutKi = ((llong)pid->KI * (llong)pid->Deta)>>16;
	pid->Total = pid->Total + m_OutKi;
	pid->Total = __IQsat(pid->Total, mTotalMax, mTotalMin);

	//����PID�����
	pid->Out = pid->Total + m_OutKp;
	pid->Out = __IQsat(pid->Out, pid->Max, pid->Min);
}

/****************************************************************
	�޳�ë���˲��������
*****************************************************************/
void BurrFilter(BURR_FILTER_STRUCT * filter)
{
	int 	m_Deta;

	m_Deta = abs((filter->Input) - (filter->Output));
	if(m_Deta > filter->Err)
	{
		filter->Err = filter->Err << 1;
	}
	else
	{
		filter->Output = filter->Input;
	  	if(m_Deta < (filter->Err >> 1))
		{
			filter->Err = filter->Err >> 1;
		}
	}
	filter->Err = (filter->Err > filter->Max)?filter->Max:filter->Err;
	filter->Err = (filter->Err < filter->Min)?filter->Min:filter->Err;
}

/****************************************************************
	�����˲�����(�����˲�ʱ�䳣���趨��ȫƽ�������ݸ���)
*****************************************************************/
void SlipFilter(CUR_LINE_STRUCT_DEF * pCur)
{
	int  m_Index,m_IndexStart,m_Coff,m_TotalCoff;
	long m_Total;
	CUR_LINE_STRUCT_DEF * m_pCur = pCur;

	for(m_Index = 0;m_Index<(16-1);m_Index++)
	{
		m_pCur->Data[m_Index] = m_pCur->Data[m_Index+1];
	}
	m_pCur->Data[15] = m_pCur->Input;

	m_Total = 0;
	m_TotalCoff = 0;
	m_IndexStart = 16 - m_pCur->FilterTime;
	for(m_Coff = 0;m_Coff<(m_pCur->FilterTime);m_Coff++)
	{
		m_Total += (long)m_pCur->Data[m_IndexStart + m_Coff] * (m_Coff +1);
		m_TotalCoff += (m_Coff+1);
	}
	m_pCur->Output = m_Total/m_TotalCoff;
}


