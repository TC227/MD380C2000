#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"

#pragma DATA_SECTION(gDebugBuffer, "debug_buffer");

#define MOTOR_SYSTEM_DEBUG      //���Դ��뿪��
#define CPU_TIME_DEBUG          // CPUʱ��Ƭ����

#ifdef MOTOR_SYSTEM_DEBUG
#ifdef TMS320F2808
#define  C_DEBUG_BUFF_SIZE	    5000		//���Ի�������С
#else   // 28035
#define C_DEBUG_BUFF_SIZE       1000
#endif
#else   

#define C_DEBUG_BUFF_SIZE   1

#endif

Uint gDebugBuffer[C_DEBUG_BUFF_SIZE];
Uint gDebugPoint = 0;
int  gDebugFlag;

#ifdef MOTOR_SYSTEM_DEBUG
Ulong   saveIndexFunc;      // �б�������ѡ����ҪCCS�ж���
Ulong   saveIndexP;         // pVD1��pVD4�������ݵĸ�������ҪCCS�ж���
Uint    saveCurveNum;       // �������ߵ����������Զ�
Uint    saveRstMem;         // ���渴λ�õı���
Uint    SaveTmCnt;
int     startSave;

int *   pVD1;
int *   pVD2;
int *   pVD3;
int *   pVD4;

// ������� DebugSaveDeal(0)
// ������� DebugSaveDeal(1)
// 2ms ���� DebugSaveDeal(2)
// 05ms���� DebugSaveDeal(3)
void DebugSaveDeal(Uint savePrgPos)
{
    Uint    save;
    
// ȷ����������ʱ��
    SaveTmCnt ++;
    if(savePrgPos == 2 &&                     // 2ms ����
        gTestDataReceive.TestData17 > 3)      // ѡ��ʱ����
    {
        if(SaveTmCnt >= (gTestDataReceive.TestData17/2))
        {
            SaveTmCnt = 0;  // ʱ�䵽����Ҫ��������
        }
        else
        {
            return;         // ����ʱ�仹û�е�
        }
    }
    else if(gTestDataReceive.TestData17 != savePrgPos)  // ����Ҫ����
    {
        return;             
    }
    //else  ��Ҫ��������

// ȷ�������Ƿ񱣴�
    switch (gTestDataReceive.TestData12)          // Cf-11    when to save data
    {
        case 0:
            save = 0;   // don't save any data
            break;
        case 1:
            save = 1;   // always save data
            break;
        case 2:         // save data when motor run
            save = (gMainCmd.Command.bit.Start) ? 1 : 0;
            break;
        default:
            save = 0;
            break;
    }

// ѭ������
    if(gDebugPoint >= C_DEBUG_BUFF_SIZE)
	{
	    // Cf-13 need to save data roll back and conver
        gDebugPoint = (gTestDataReceive.TestData14) ? 0 : C_DEBUG_BUFF_SIZE;
	}

// ���α�������
    if(save)            // begin to save
    {
        Uint i = 0;
        Uint totalN = 0;

        //saveIndex.all = gTestDataReceive.TestData11;        // Cf-10    select save content

        if(gTestDataReceive.TestData13)             // Cf-12 auto save, ����pVD1 - pVD4�е�����
        {
            totalN = 0;
            if(saveIndexP == 1)
            {
                SaveDebugData16(*pVD1);
                totalN = 1;
            }
            else if(saveIndexP == 2)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                totalN = 2;
            }
            else if(saveIndexP == 3)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                SaveDebugData16(*pVD3);
                totalN = 3;
            }
            else if(saveIndexP == 4)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                SaveDebugData16(*pVD3);
                SaveDebugData16(*pVD4);
                totalN = 4;
            }
            
        }
        else                                        // ͨ��������ӵ�ַ�б���ѡ���ַ
        {
            for(i = 0; i < 32; i++)
            {
                
                if(saveIndexFunc & (1L<<i))
                {
                    
                    totalN ++;
                }
            }
        }
        saveCurveNum = totalN;
    }

// �Ƿ�λ����
    if(gTestDataReceive.TestData15 && saveRstMem == 0)  // Cf-14 buffer reset
    {
        ResetDebugBuffer();
        saveRstMem ++;
    }
    if(saveRstMem && gTestDataReceive.TestData15 == 0)  // need Cf-14 to reset itself
    {
        saveRstMem = 0;
    }
}

void SaveDebugData16(Uint data)
{
    if(gDebugPoint >= C_DEBUG_BUFF_SIZE)
	{
        //gDebugPoint = 0;
	    gDebugPoint = C_DEBUG_BUFF_SIZE-1;
	}
    //gDebugBuffer[0] = gDebugPoint;
	gDebugBuffer[gDebugPoint++] = data;
}

void SaveDebugData32(Ulong data)
{
	Ulong * p;
	gDebugFlag = 1;

	p = (Ulong *)(&gDebugBuffer);
	if(gDebugFlag)
	{
		if(gDebugPoint >= C_DEBUG_BUFF_SIZE/2)	gDebugPoint = C_DEBUG_BUFF_SIZE/2 - 2;
		p[gDebugPoint++] = data;
	}
}

void ResetDebugBuffer(void)
{
    int m_index;
    
    for(m_index=0;m_index<(C_DEBUG_BUFF_SIZE-1);m_index++)
    {
        gDebugBuffer[m_index] = 0;
    }
	gDebugPoint = 0;
}
#endif

