//======================================================================
//
// U�飬ͣ��״̬��ʾ��
//
// Time-stamp: <2012-08-1 9:01:32  Shisheng.Zhi, 0354>
//
//======================================================================

#define DISP_FRQ_RUN            0   //  0, ����Ƶ��
#define DISP_FRQ_AIM            1   //  1, �趨Ƶ��
#define DISP_OUT_CURRENT        4   //  4, �������
#define DISP_OUT_POWER          5   //  5, �������
#define DISP_FRQ_RUN_FDB        19  //  19,�����ٶ�
#define DISP_FRQ_COMM           28  //  28,ͨѶ�趨ֵ
#define DISP_P2P_COMM_SEND      63  //  63,��Ե�ͨѶ���ݷ���
#define DISP_P2P_COMM_REV       64  //  64,��Ե�ͨѶ���ݽ���
#define DISP_FRQ_FDB            29  //  29,�����ٶ� 
#define DISP_FRQ_X              30  //  30, ��Ƶ��X��ʾ
#define DISP_FRQ_Y              31  //  31, ����Ƶ��Y��ʾ
#define DISP_LOAD_SPEED         14  //  ����ʱ�����ٶ���ʾ��bitλ��

#define DISP_DI_STATUS_SPECIAL1 41  // DI����״ֱ̬����ʾ��DI1-DI19,VDI1
#define DISP_DO_STATUS_SPECIAL1 42  // DO����״ֱ̬����ʾ��R1-R10,DO1,R11,R12,VDO1-VDO5
#define DISP_DI_FUNC_SPECIAL1   43  // DI����״ֱ̬����ʾ1������01-����40
#define DISP_DI_FUNC_SPECIAL2   44  // DI����״ֱ̬����ʾ2������41-����80

extern Uint16 ticker05msFuncEachDisp[4];
extern Uint16 ticker05msFuncInternalDisp[4];
extern Uint16 vfSeparateVol;
extern Uint16 vfSeprateVolAim;
extern Uint16 temperature;     // ��⵽�õ���¶�ֵ
extern Uint16 ZCount;          // Z�źż�����
extern Uint16 antiVibrateCoeff;       // VF��ϵ��
#define UF_VIEW_ATTRIBUTE  0x3140

//=====================================================================
// ���²�������Ϊ��ʾʹ�ã�Ŀǰʹ�õĵط���:
// 1. ����ʱ��ʾ
// 2. ͣ��ʱ��ʾ
// 3. ��������ʾ
// 4. ���
// ǰ��32����ͬ����ʱ��ʾ(MD320, F7-04������)˳��

// ����U0����ʾ����
void UpdateU0Data(void)
{
    funcCode.group.u0[0]  = frqDisp;                     //  0; ����Ƶ��
    funcCode.group.u0[1]  = frqAimDisp;                  //  1; �趨Ƶ��
    funcCode.group.u0[2]  = generatrixVoltage;           //  2; ĸ�ߵ�ѹ
    funcCode.group.u0[3]  = outVoltageDisp;              //  3; �����ѹ
    funcCode.group.u0[4]  = outCurrentDisp;              //  4; �������
    funcCode.group.u0[5]  = outPower;                    //  5; �������
    funcCode.group.u0[6]  = itDisp;                      //  6; ���ת��
    funcCode.group.u0[7]  = diStatus.a.all&0x7FFF;       //  7; DI����״̬
    funcCode.group.u0[8]  = doStatus.a.all;              //  8; DO���״̬
    funcCode.group.u0[9]  = aiDeal[0].voltage;           //  9; AI1��ѹ
    funcCode.group.u0[10] = aiDeal[1].voltage;           // 10; AI2��ѹ
    funcCode.group.u0[11] = aiDeal[2].voltage;           // 11; AI3��ѹ
    funcCode.group.u0[12] = funcCode.code.counterTicker; // 12; ����ֵ
    funcCode.group.u0[13] = funcCode.code.lengthCurrent; // 13; ����ֵ
    funcCode.group.u0[14] = loadSpeedDisp;               // 14; �����ٶ���ʾ(ͣ��ʱ��ʾ�趨Ƶ��*ϵ��  ����ʱ��ʾ����Ƶ��*ϵ��)
    funcCode.group.u0[15] = pidFuncRefDisp;              // 15; PID�趨
    funcCode.group.u0[16] = pidFuncFdbDisp;              // 16; PID����
    funcCode.group.u0[17] = plcStep;                     // 17; PLC�׶�
    funcCode.group.u0[18] = pulseInFrqDisp;              // 18; PULSE��������Ƶ�ʣ���λ0.01KHz
    funcCode.group.u0[19] = frqRunDisp;                  // 19�������ٶȣ���λ0.1Hz    // �������Ƶ��
    funcCode.group.u0[20] = setRunLostTime;              // 20; ʣ������ʱ��  
    funcCode.group.u0[21] = aiDeal[0].voltageOrigin;     // 21; AI1У��ǰ��ѹ
    funcCode.group.u0[22] = aiDeal[1].voltageOrigin;     // 22; AI2У��ǰ��ѹ
    funcCode.group.u0[23] = aiDeal[2].voltageOrigin;     // 23; AI3У��ǰ��ѹ                         
    funcCode.group.u0[24] = lineSpeed;                   // 24; ���ٶ�
    funcCode.group.u0[25] = curTime.powerOnTime;         // 25; ��ǰ�ϵ�ʱ��
    funcCode.group.u0[26] = curTime.runTime;             // 26; ��ǰ����ʱ��
    funcCode.group.u0[27] = pulseInFrq;                  // 27; PULSE��������Ƶ�ʣ���λ1Hz
    funcCode.group.u0[28] = funcCode.code.frqComm;       // 28; ͨѶ�趨ֵ
    funcCode.group.u0[29] = frqFdb;                      // 29; ʵ�ʷ����ٶ�
    funcCode.group.u0[30] = frqXDisp;                    // 30; ��Ƶ��X��ʾ
    funcCode.group.u0[31] = frqYDisp;                    // 31; ��Ƶ��Y��ʾ
    funcCode.group.u0[32] = memoryValue;                 // 32; �鿴�����ڴ��ֵַ
    funcCode.group.u0[33] = pmsmRotorPos;                // 33; ͬ����ת��λ��(����ʵʱ����)
    funcCode.group.u0[34] = temperature;                 // 34; ����¶�ֵ
    funcCode.group.u0[35] = torqueAim;                   // 35; Ŀ��ת��
    funcCode.group.u0[36] = enCoderPosition;             // 36; ����λ��
    funcCode.group.u0[37] = gPhiRtDisp;                  // 37; �������ؽǶ�
    funcCode.group.u0[38] = ABZPos;                      // 38; ABZλ��
    funcCode.group.u0[39] = vfSeprateVolAim;             // 39; VF����Ŀ���ѹ
    funcCode.group.u0[40] = vfSeparateVol;               // 40; VF���������ѹ
    //funcCode.group.u0[41] = rsvdData;                    // 41; DI����ֱ����ʾ
    //funcCode.group.u0[42] = rsvdData;                    // 42; DO����ֱ����ʾ
    //funcCode.group.u0[43] = rsvdData;                    // 43; DI����״ֱ̬����ʾ
    //funcCode.group.u0[44] = rsvdData;                    // 44; DO����״ֱ̬����ʾ
    funcCode.group.u0[45] = errorInfo;                   // 45; ������Ϣ
    //funcCode.group.u0[46] = rsvdData;                    // 46; ����
    //funcCode.group.u0[47] = rsvdData;                    // 47; ����
    //funcCode.group.u0[48] = rsvdData;                    // 48; ����
    //funcCode.group.u0[49] = rsvdData;                    // 49; ����
    funcCode.group.u0[50] = ticker05msFuncInternalDisp[0]; // 50; ����
    funcCode.group.u0[51] = ticker05msFuncInternalDisp[1]; // 51; ����
    funcCode.group.u0[52] = ticker05msFuncInternalDisp[2]; // 52; ����
    funcCode.group.u0[53] = ticker05msFuncInternalDisp[3]; // 53; ����
    funcCode.group.u0[54] = ticker05msFuncEachDisp[0];     // 54; ����
    funcCode.group.u0[55] = ticker05msFuncEachDisp[1];     // 55; ����
    funcCode.group.u0[56] = ticker05msFuncEachDisp[2];     // 56; ����
    funcCode.group.u0[57] = ticker05msFuncEachDisp[3];     // 57; ����
    funcCode.group.u0[58] = ZCount;                        // 58; Z�źż�����
    funcCode.group.u0[59] = frqAimPLCDisp;                 // 59; �趨Ƶ�� -100.00%~100.00%
    funcCode.group.u0[60] = frqPLCDisp;                    // 60; ����Ƶ�� -100.00%~100.00%
    funcCode.group.u0[61] = invtStatus.all;                // 61; ��Ƶ������״̬
    //funcCode.group.u0[62] = antiVibrateCoeff;              // 62; VF��ϵ��
    funcCode.group.u0[63] = p2pData.P2PSendData;           // 63; ��Ե�ͨѶ����ֵ
    funcCode.group.u0[64] = p2pData.P2PRevData;            // 64; ��Ե�ͨѶ����ֵ
 #if 0      
    funcCode.group.u0[62] = 0;
    funcCode.group.u0[63] = 0;
    funcCode.group.u0[64] = 0;
    funcCode.group.u0[65] = 0;
    funcCode.group.u0[66] = 0;
    funcCode.group.u0[67] = 0;
    funcCode.group.u0[68] = 0;
    funcCode.group.u0[69] = 0;
    funcCode.group.u0[70] = 0;
    funcCode.group.u0[71] = 0;
    funcCode.group.u0[72] = 0;
    funcCode.group.u0[73] = 0;
    funcCode.group.u0[74] = 0;
    funcCode.group.u0[75] = 0;
    funcCode.group.u0[76] = 0;
    funcCode.group.u0[77] = 0;
    funcCode.group.u0[78] = 0;
    funcCode.group.u0[79] = 0;
    funcCode.group.u0[80] = 0;
    funcCode.group.u0[81] = 0;
    funcCode.group.u0[82] = 0;
    funcCode.group.u0[83] = 0;
    funcCode.group.u0[84] = 0;
    funcCode.group.u0[85] = 0;
    funcCode.group.u0[86] = 0;
    funcCode.group.u0[87] = 0;
    funcCode.group.u0[88] = 0;
    funcCode.group.u0[89] = 0;
    funcCode.group.u0[90] = 0;
    funcCode.group.u0[91] = 0;
    funcCode.group.u0[92] = 0;
    funcCode.group.u0[93] = 0;
    funcCode.group.u0[94] = 0;
    funcCode.group.u0[95] = 0;
    funcCode.group.u0[96] = 0;
    funcCode.group.u0[97] = 0;
    funcCode.group.u0[98] = 0;
    funcCode.group.u0[99] = 0;
#endif
}


// ������pDispValueU0�γɽṹ�壬��ռ�ÿռ���
union FUNC_ATTRIBUTE const dispAttributeU0[U0NUM] =
{
    0x094A & ~(1U<<11) | (1U<<12),     // 0, ����Ƶ��
    0x094A & ~(1U<<11) | (1U<<12),     // 1, �趨Ƶ��
    0x1161 & ~(1U<<11) | (1U<<12),     // 2, ĸ�ߵ�ѹ
    0x08E0 & ~(1U<<11) | (1U<<12),     // 3, �����ѹ
    0x0952 & ~(1U<<11) | (1U<<12),     // 4, �������

    0x2941 & ~(1U<<11) | (1U<<12),     // 5, �������
    0x2131 & ~(1U<<11) | (1U<<12),     // 6, ���ת��
    0x9D00 & ~(1U<<11) | (1U<<12),     // 7, DI����״̬
    0x9D00 & ~(1U<<11) | (1U<<12),     // 8, DO���״̬
    0x2122 & ~(1U<<11) | (1U<<12),     // 9, AI1��ѹ

    0x2122 & ~(1U<<11) | (1U<<12),     // 10, AI2��ѹ
    0x2122 & ~(1U<<11) | (1U<<12),     // 11, AI3��ѹ
    0x0140 & ~(1U<<11) | (1U<<12),     // 12, ����ֵ
    0x0140 & ~(1U<<11) | (1U<<12),     // 13, ����ֵ
    0x0140 & ~(1U<<11) | (1U<<12),     // 14, �����ٶ���ʾ

    0x0170 & ~(1U<<11) | (1U<<12),     // 15, PID�趨
    0x0170 & ~(1U<<11) | (1U<<12),     // 16, PID����
    0x0140 & ~(1U<<11) | (1U<<12),     // 17, PLC�׶�
    0x1142 & ~(1U<<11) | (1U<<12),     // 18, PULSE��������Ƶ�ʣ���λ0.01kHz
    0x294A & ~(1U<<11) | (1U<<12),     // 19, ����Ƶ�ʣ�0.1Hz
    0x0141 & ~(1U<<11) | (1U<<12),     // 20, ʣ������ʱ��
    
    0x2163 & ~(1U<<11) | (1U<<12),      // 21, AI1У��ǰ��ѹ
    0x2163 & ~(1U<<11) | (1U<<12),      // AI2У��ǰ��ѹ
    0x2163 & ~(1U<<11) | (1U<<12),      // AI3У��ǰ��ѹ
    
    0x1D00 & ~(1U<<11) | (1U<<12),

    0x0140 & ~(1U<<11) | (1U<<12),     // 25,
    0x0141 & ~(1U<<11) | (1U<<12),     // 26,
    0x0148 & ~(1U<<11) | (1U<<12),     // 27,
    0x2972 & ~(1U<<11) | (1U<<12),     // 28,
    0x294A & ~(1U<<11) | (1U<<12),     // 29,

    0x094A & ~(1U<<11) | (1U<<12),     // 30,
    0x094A & ~(1U<<11) | (1U<<12),     // 31,
    0x0140 & ~(1U<<11) | (1U<<12),     // 32,
    0x0101 & ~(1U<<11) | (1U<<12),     // 33,
    0x0140 & ~(1U<<11) | (1U<<12),     // 34,

    0x2131 & ~(1U<<11) | (1U<<12),     // 35,
    0x0140 & ~(1U<<11) | (1U<<12),     // 36,
    0x2101 & ~(1U<<11) | (1U<<12),     // 37,
    0x0140 & ~(1U<<11) | (1U<<12),     // 38,
    0x0140 & ~(1U<<11) | (1U<<12),     // 39,

    0x0140 & ~(1U<<11) | (1U<<12),     // 40,
    0x0140 & ~(1U<<11) | (1U<<12),     // 41,
    0x0140 & ~(1U<<11) | (1U<<12),     // 42,
    0x0140 & ~(1U<<11) | (1U<<12),     // 33,
    0x0140 & ~(1U<<11) | (1U<<12),     // 44,

    0x0140 & ~(1U<<11) | (1U<<12),     // 45,
    0x0140 & ~(1U<<11) | (1U<<12),     // 46,
    0x0140 & ~(1U<<11) | (1U<<12),     // 47,
    0x0140 & ~(1U<<11) | (1U<<12),     // 48,
    0x0140 & ~(1U<<11) | (1U<<12),     // 49,

    0x1001,                             // 50
    0x1001,                             // 51
    0x1001,                             // 52
    0x1001,                             // 53
    0x1001,                             // 54
    0x1001,                             // 55
    0x1001,                             // 56
    0x1001,                             // 57
    0x0140 & ~(1U<<11) | (1U<<12),      // 58

    0x3172 & ~(1U<<11) | (1U<<12),      // 59
    0x3172 & ~(1U<<11) | (1U<<12),      // 60
    0x0140 & ~(1U<<11) | (1U<<12),      // 61
    0x0140 & ~(1U<<11) | (1U<<12),      // 62
    0x2972 & ~(1U<<11) | (1U<<12),      // 63
    0x2972 & ~(1U<<11) | (1U<<12),      // 64
    0x0140 & ~(1U<<11) | (1U<<12),      // 65
	0x0140 & ~(1U<<11) | (1U<<12),      // 65
};
//=====================================================================
// ͣ�����Ӳ���
// 1.��Ӧ�����в����е����
//=====================================================================
const Uint16 stopDispIndex[STOP_DISPLAY_NUM] =
{
    1,      // 0  �趨Ƶ��
    2,      // 1  ĸ�ߵ�ѹ
    7,      // 2  DI����״̬
    8,      // 3  DO���״̬
    9,      // 4  AI1��ѹ

    10,     // 5  AI2��ѹ
    11,     // 6  AI3��ѹ
    12,     // 7  ����ֵ
    13,     // 8  ����ֵ
    17,     // 9  plcStep

    14,     // 10 �����ٶ���ʾ
    15,     // 11 PID�趨
    18,     // 12 PULSE��������Ƶ��
};

Uint16 const commDispIndex[COMM_PARA_NUM] =  // ͨѶ��ȡͣ����������ʾ����
{
    28,         // ͨѶ�趨ֵ
    0,          // ����Ƶ��
    2,          // ĸ�ߵ�ѹ
    3,          // �����ѹ
    4,          // �������
    5,          // �������
    6,          // ���ת��
    19,         // �����ٶ�
    7,          // DI����״̬
    8,          // DO���״̬
    9,          // AI1��ѹ
    10,         // AI2��ѹ
    11,         // AI3��ѹ
    12,         // ����ֵ����
    13,         // ����ֵ����
    14,         // �����ٶ�
    15,         // PID�趨
    16,         // PID����
    17,         // PLC�׶�
    
    18,         // PULSE��������Ƶ�ʣ���λ0.01KHz
    19,         // �����ٶȣ���λ0.1Hz    // �������Ƶ��
    20,         //  ʣ������ʱ��  
    21,         //  AI1У��ǰ��ѹ
    22,         //  AI2У��ǰ��ѹ
    23,         //  AI3У��ǰ��ѹ                         
    24,         //  ���ٶ�
    25,         //  ��ǰ�ϵ�ʱ��
    26,         //  ��ǰ����ʱ��
    27,         //  PULSE��������Ƶ�ʣ���λ1Hz
    28,         //  ͨѶ�趨ֵ
    29,         //  ʵ�ʷ����ٶ�
    30,         //  ��Ƶ��X��ʾ
    31,         //  ��Ƶ��Y��ʾ
};






