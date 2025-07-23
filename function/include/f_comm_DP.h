#ifndef __F_COMM_DP_H__
#define __F_COMM_DP_H__

#include "f_funcCode.h"

// ֡��ʽ����
#define DP_TO_DSP 4
#define DSP_TO_DP 5
#define READ_DP_PARAMETER 2
#define DSP_TO_DP_PARAMETER 3
#define ERR_FRAME 7

// ��������
#define DP_OK 0
#define DP_ERR 5
#define DSP_REWORK_ADD 1
#define DSP_REWORK_MODE 2

// DP������ģʽ
#define PPO1 1
#define PPO2 2
#define PPO3 3
#define PPO5 5

// DP������ģʽ ��Ӧ�����������ֽڸ���
#define PPO1_NUMBER 12
#define PPO2_NUMBER 20
#define PPO3_NUMBER 4
#define PPO5_NUMBER 32

// DP�����ڲ���������
#define SCI_BAUD_RATE1 0           // 115.2kpbs
#define SCI_BAUD_RATE2 1           // 208.3kpbs
#define SCI_BAUD_RATE3 2           // 256kpbs
#define SCI_BAUD_RATE4 3           // 512kpbs

#define OLD_DATA_MODE  0           // �ɵ����ݴ���ģʽ,��÷���ṩ�ķ���

#define BUG_SCI_BACK_DATA 0        // ����DSP���ڷ������ܵ�����


#define SCI_SEND_CONNECT_NUMBER 5  // ���Դ��ڷ��͵����ݸ���
#define SCI_SEND_READ_NUMBER 5

#define SEND_DATA_NUMBER 32        // profibus ����ģʽ���������ݸ��� PPO5 ���32���ֽ�
#define RCV_DATA_NUMBER 32

#define SEND_DATA_SCI_NUMBER 40    // ����ͨѶ���ݽ����Ļ��������ݸ��� ʵ�����ʹ��35��
#define RCV_DATA_SCI_NUMBER 40

#define SCI_RCV_ONE_FARME_TIME 60000

#define SCI_RCV_ONE_FARME_TIME1 600000

#define RCV_CRC_ERR_NUMBER 10

#define NO_FIFO 1

 // DP����
typedef  struct
{
    Uint16 dpAddress;      // DP����ַ
    Uint16 dpDataFormat;   // DP������ģʽ
} DP_PARAMETER;  

extern DP_PARAMETER dpParameter;

enum COMM_DP_STATUS
{
    SCI_CONNECT = 0,                 // SCI������
    SCI_CONNECT_OK = 1               // SCI���ӳɹ�
};

enum DP_READ_FLAG_State{DP_READ_NO=0, DP_READ_YES=!DP_READ_NO};
enum SCI_RCV_FLAG_State{SCI_RCV_NO=0, SCI_RCV_YES=!SCI_RCV_NO};
enum SCI_RCV_TICK_FLAG_State{SCI_RCV_TICK_NO=0, SCI_RCV_TICK_YES=!SCI_RCV_TICK_NO};

struct FRAME_START_BITS
{  
    Uint16 frameType:3;               // ֡����    ��3λ
    Uint16 errType:3;                 // ��������
    Uint16 commSciFlag:1;             // ͨѶ��־   ��λ   ��������λ��    
    Uint16 commProfibusFlag:1;        // ͨѶ��־   ��λ   ��������λ��  
};

union FRAME_START {
    Uint16                   all;
    struct FRAME_START_BITS   bit;
};


struct SCI_DATA_DP
{
    volatile struct SCI_REGS *pSciRegs;
    
    Uint16 sendData_SCI[SEND_DATA_SCI_NUMBER];   // ���͸�STM32����
    Uint16 rcvData_SCI[RCV_DATA_SCI_NUMBER];     // ����STM32������
    Uint16 commDpRcvNumber;                      // �������ݼ���
    Uint16 commDpSendNum;                        // ���͵����ݼ���
    Uint16 commDpSendNumMax;                     // ÿ�η��͵����ݸ���
    Uint16 frameFlagDp;                          // ��ͬ����֡
    Uint16 rcvTickDp;                            // �������ݵ�ʱ����
    Uint16 rcvDataJuageFlag;
    Uint16 rcvCrcErrCounter;                     // ��������CRCУ�����
    Uint16 rcvRigthFlag;                         // ������ͷ��һ������ȷ��־
    
    union FRAME_START frameStart;                // ���յ�����ͷ
    union FRAME_START frameSendStart;            // ���͵���ͷ
    
    enum SCI_RCV_TICK_FLAG_State rcvTickFlag;    // ���ռ�ʱ�ı�־
    enum DP_READ_FLAG_State dpReadFlag;          // dp������ַ ģʽ��־
    enum SCI_RCV_FLAG_State sciRcvFlag;          // sci������ɱ�־

    enum COMM_DP_STATUS commDpStatus;            // SCI����״̬
    
};
extern struct SCI_DATA_DP sciM380DpData;


// �����ݴ���ӿڵ����鼰��־
extern Uint16 sendData_DP[SEND_DATA_NUMBER];     // ���͵�DP������
extern Uint16 rcvData_DP[RCV_DATA_NUMBER];       // ����DSP������
enum DP_SCI_COMM_RCV_FLAG{DP_SCI_COMM_RCV_NO=0, DP_SCI_COMM_RCV_YES=!DP_SCI_COMM_RCV_NO};     // SCI�����������
enum DP_SCI_COMM_SEND_FLAG{DP_SCI_COMM_SEND_NO=0, DP_SCI_COMM_SEND_YES=!DP_SCI_COMM_SEND_NO}; // DSP׼�����͵�����׼�����


void InitSetSciDp(struct SCI_DATA_DP *p);
void InitSciaGpioDp(void);
void SciDpDeal(struct SCI_DATA_DP *p);
extern void InitSciDpBaudRate(struct SCI_DATA_DP *p);
#endif




























