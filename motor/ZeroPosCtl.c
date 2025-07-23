/***********************************Inovance***********************************
����������Function Description��:
����޸����ڣ�Date����
�޸���־��History��:�����¼�¼Ϊ��һ��ת���Ժ󣬿�ʼ��¼��
	���� 		ʱ�� 		����˵��
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance***********************************/

/* Includes ------------------------------------------------------------------*/
#include "MotorInclude.h"
#include "ZeroPosCtl.h"

/* Private variables ---------------------------------------------------------*/
ZERO_POS_STRUCT         gZeroPos;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

u32 GetPGPos(void)  
{
    return ((*EQepRegs).QPOSCNT);
}


/*******************************************************************************
* Function Name  : ���ŷ��жϳ���
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ZeroPosJudge(void)
{
    if(gMainCmd.FreqSet != 0)
    {
        gZeroPos.Flag = 0;
    }
    else
    {
        gZeroPos.Flag = 1;
        gZeroPos.PosInit = GetPGPos();
    }
}

/*******************************************************************************
* Function Name  : ���ŷ�   �ѵ����������λ��
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ZeroPosControl(void)
{
    u16 m_QepCnt;
    s32 m_ITKpPos;
    u32 m_CpuTime;

    if(gZeroPos.Flag == 0)        return;

    gZeroPos.Flag++;
    gZeroPos.Flag = Min(gZeroPos.Flag,10);
    
    m_QepCnt = GetPGPos();        
    gZeroPos.Pos = m_QepCnt - gZeroPos.PosInit;
    gZeroPos.PosInit = m_QepCnt;

    m_CpuTime = GetTime();
    gZeroPos.DetaTime = gZeroPos.TimeLast - m_CpuTime;
    gZeroPos.TimeLast = m_CpuTime;

    m_ITKpPos = ((s32)gZeroPos.Pos * (s32)gZeroPos.KPPos)<<4;
    gZeroPos.ItKpPos = Min(m_ITKpPos,8192);                     /*������2�����ص���*/
    
}
/******************************* END OF FILE***********************************/



