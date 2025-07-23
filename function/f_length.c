#include "f_length.h"


#define SERVO_ADJUST_MULTIPLY                   8
#define SERVO_ENABLE_AIM_FREQ                   4000    //���Ŀ��Ƶ�ʳ�����Ƶ������ٹ��̲���λ

Uint16 servoRunStatus;
SERVO_RUN_PARA_STRUCT servoRunControl;

LOCALF Uint16 runTickerServo;
LOCALF Uint32 qepPulseTemp;
Uint32 qepPulseCalALl;
Uint32 qepPulsePLC;
LOCALF Uint16 servoTicker;

unsigned int intervalTime;
unsigned int rangePer;
unsigned int bobofrq;
unsigned int boboreal;
unsigned int boboerr;
Uint16 servoRunStarFlag;
Uint16 servoControlEnable;
Uint32 pulseNote1;
Uint32 pulseNote2;
Uint32 pulseNote3;
Uint32 pulseTriangleLeft;
Uint16 timeLeft1;
Uint16 timeLeft2;
Uint16 frqSaveDis;
Uint32 motorPulseAll;
Uint16 servoRunResetFlag;
Uint16 servoCalFlag;
Uint16 servoRunDIFlag;
Uint32 servoPulseRunToPLC;
Uint16 servoOverFlag;
Uint16 servoEndTime;

LOCALF void servoParaCal(void);
LOCALF int32 ServoTrackPulseCal(int32 f0,int32 f1,int32 t);

//============================================================
//
// �ŷ����п���
//
//============================================================
void ServoRunCtrl(void)
{   
        Uint32 mAdjustTmp;
        Uint32 i;
        
        if (!runCmd.bit.common)                         // ��������ͣ������
        {
                swingStatus = SWING_NONE;
                runStatus = RUN_STATUS_STOP;
                stopRunStatus = STOP_RUN_STATUS_INIT;

                frqTmp = frqRun;                        // ����Ƶ��
                runFlag.bit.servo = 0;
                return;
        }

        runFlag.bit.run = 1;                            // �����б�־��֮ǰ��ʹ�øñ�־
        runFlag.bit.common = 1;
        runFlag.bit.servo = 1;
        dspMainCmd.bit.run = 1;
        
        // ���λ������ȷ��
        if(0)             
        {
            servoRunResetFlag =1;                   //��ʱ��λ��Ҫ���¼��㣬���ݸ������������͵�ǰ������Ƶ�ʼ������ʱ��
            servoControlEnable = 1;                 //��ʾ��λ������Ч

            qepPulsePLC = EQep1Regs.QPOSCNT;        //����PLC���������� 
            servoPulseRunToPLC = 0;
            servoOverFlag = 0;
            servoEndTime = 0;

            servoRunControl.motorRealRunPulse = 0;  //������е�����Ƶ��
            qepPulseTemp = EQep1Regs.QPOSCNT;       //��ȡ��ǰ����ֵ

            servoRunControl.servoLeftPulse = funcCode.code.servoPulseCheck ;          //��λ�������������,У�������������

            pulseTriangleLeft = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);   //���㵱ǰƵ���µ�ͣ�����߹�������
            //------------------------------------------------------
            // �����ǰƵ�ʰ�����ʱ����ٳ�������Ҫ���¼������ʱ��
            //------------------------------------------------------
            if( servoRunControl.servoLeftPulse <= pulseTriangleLeft)
            {
                servoRunControl.pulseSet = servoRunControl.servoLeftPulse; //��ؼ���

                timeLeft1 = (Uint64)servoRunControl.servoLeftPulse * servoRunControl.motorPolePairs * 2 *100000
                           /((Uint32)frqTmp * servoRunControl.encoderPulse);
                           
                timeLeft2 = (Uint32) maxFrq * timeLeft1 / frqTmp;
                servoRunControl.decTime = timeLeft2 / 10;       //�Ӽ���ʱ��2λС������Ҫ����10
                servoRunControl.constTime =0;
                servoRunStatus = DEC_RUN_STATUS;
            }
            else
            {
                    servoParaCal();                                 //����Ŀ��Ƶ�ʺͺ�������ʱ��
                    servoRunStatus = ACC_RUN_STATUS;
            }
        }

        //--------------------------------------------------------
        // ���������ڷ�Χ���������40Hz������λ����
        // ���й��������¶�λ servoRunResetFlag =1���ڱȽ�40hz����
        //--------------------------------------------------------        
        if((servoRunControl.aimFreq > SERVO_ENABLE_AIM_FREQ) && (!servoRunResetFlag))
        {
                if((ABS_INT16(frqTmp) <= SERVO_ENABLE_AIM_FREQ) && (DEC_RUN_STATUS == servoRunStatus))      //С��40Hz��ʱ��λ
                {
                        servoControlEnable = 1;
                        
                        if(!servoCalFlag)
                        {
                                frqSaveDis = frqTmp;
                                servoRunStarFlag = 1;
                                servoCalFlag = 1;
                                servoOverFlag = 0;
                                servoEndTime = 0;
                                
                                servoRunControl.motorRealRunPulse += (int32)(EQep1Regs.QPOSCNT - qepPulseTemp); 
                                pulseNote2 = (servoRunControl.motorRealRunPulse >> 2);       
                                qepPulseTemp = EQep1Regs.QPOSCNT;                       
                                servoRunControl.servoLeftPulse = servoRunControl.pulseSet - (servoRunControl.motorRealRunPulse >> 2); 
                                servoRunControl.motorRealRunPulse = 0;
                                
                                pulseTriangleLeft = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);
                                
                                pulseNote1 = servoRunControl.calRunPulse; 
                                
                                //----------------------------------------------------
                                //���ʣ�µı�ֱ�Ӽ��ٵ�����������Ҫ�����������ʱ��
                                //����Ҫ���¼������ʱ��
                                //----------------------------------------------------
                                if( servoRunControl.servoLeftPulse >= pulseTriangleLeft)
                                {                               
                                        servoRunControl.constTime = (int64)servoRunControl.motorPolePairs* (servoRunControl.servoLeftPulse - pulseTriangleLeft)
                                                                  * FRQ_UINT * (TIME_UNIT_MS_PER_SEC/RUN_CTRL_PERIOD)
                                                                  / ((int64)frqTmp  * servoRunControl.encoderPulse); 
                                        servoRunStatus = CONST_RUN_STATUS;      //תΪ��������
                                        servoRunControl.constTicker = 1;        //ʱ�����
                                        servoRunControl.accCoursePulse =0;      //���ٹ��̵����������                        
                                }
                                //--------------------------
                                // С����Ҫ���¼������ʱ��
                                //--------------------------
                                else
                                {
                                        timeLeft1 = (Uint64)servoRunControl.servoLeftPulse * servoRunControl.motorPolePairs * 2 *100000
                                                   /((Uint32)frqTmp * servoRunControl.encoderPulse);
                                                   
                                        timeLeft2 = (Uint32) maxFrq * timeLeft1 / frqTmp;
                                        servoRunControl.decTime = timeLeft2 / 10;
                                        servoRunControl.constTime =0;
                                }            
                        }
                }
                else
                {
                        mAdjustTmp = 0; 
                        //servoRunStarFlag = 0;  
                }                            
        }   

        //-----------------------------------------------------
        // ��λ�������й��̳���
        //-----------------------------------------------------
        switch (servoRunStatus)
        {
                case ZERO_PREPARE_PARA_CAL:             //�ŷ����в�������׼������
                {
                        servoParaCal();
                        servoControlEnable = 0; 
                        if(servoRunControl.aimFreq <= SERVO_ENABLE_AIM_FREQ)
                        {
                            servoControlEnable = 1;     // ����λ���б�־
                        }
                        servoRunStatus = ACC_RUN_STATUS; 
                        servoRunStarFlag = 0;
                        motorPulseAll = 0; 
                        servoPulseRunToPLC = 0;
                        qepPulsePLC = EQep1Regs.QPOSCNT;
                        servoOverFlag = 0;
                        servoEndTime = 0;
                        break;
                } 
                case ACC_RUN_STATUS:                    //�ŷ��������й���
                {
                        frqCurAim = servoRunControl.aimFreq;
                        AccDecFrqCalc(servoRunControl.accTime, servoRunControl.decTime, 0);
                        //-----------------------------------------
                        // ��ΪĿ��Ƶ�ʳ���40HZ���Դ��ڲ���λ�׶�
                        //-----------------------------------------
                        if(servoControlEnable)
                        {
                            servoRunControl.accCoursePulse = ServoTrackPulseCal(servoRunControl.runStarFreq,frqTmp,servoRunControl.accTime);
                            servoRunControl.calRunPulse = servoRunControl.accCoursePulse;
                        } 
                        
                        if (frqCurAim == frqTmp)        //���ٵ�Ŀ��Ƶ����     
                        {
                                runTickerServo = 0;    
                                servoRunStatus = CONST_RUN_STATUS;
                                if(servoRunControl.constTime)
                                {
                                        servoRunControl.constTicker++;
                                }
                                else
                                {
                                    servoRunStatus = DEC_RUN_STATUS;// �޺�������ʱ��תΪ���ٹ���
                                }
                        }
                        break;
                }
                case CONST_RUN_STATUS:                  //�ŷ��������й���
                {     
                        //-------------------------------------------------
                        // �����������40HZ���У���Ҫ�Ե�ǰ������Ƶ��������
                        //-------------------------------------------------
                        if(servoCalFlag)
                        {
                            servoRunControl.aimFreq = frqTmp;
                        }
                        if(servoControlEnable)          //��λʹ������ʱ�Ŵ���
                        {
                            servoRunControl.constCoursePulse = ((int64)servoRunControl.aimFreq * servoRunControl.encoderPulse * servoRunControl.constTicker * 2)
                                                                / ((int32)servoRunControl.motorPolePairs * TIME_UNIT_MS_PER_SEC * FRQ_UINT);
                            servoRunControl.calRunPulse = servoRunControl.accCoursePulse + servoRunControl.constCoursePulse;
                        }
                        if (servoRunControl.constTicker++ >= (servoRunControl.constTime-1) )
                        {
                                servoRunStatus = DEC_RUN_STATUS;
                        }
                        break;
                }
                case DEC_RUN_STATUS:                    //�ŷ��������й���
                {
                        frqCurAim = 0;
                        AccDecFrqCalc(servoRunControl.accTime, servoRunControl.decTime, 0); 
                        
                        if(!servoControlEnable)         //��λ��ʹ�ܴ���
                        {
                            break;
                        }
                        servoRunControl.decCoursePulse = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);  
                        //-----------------------------
                        // �����Ҫ�������̵ļ��ټ���
                        // ���Ŀ��Ƶ�ʳ���40HZ��������С��40HZʱ����
                        //-----------------------------
                        if(servoRunStarFlag)
                        {
                                servoRunControl.calRunPulse = servoRunControl.servoLeftPulse - servoRunControl.decCoursePulse;

                        }
                        else
                        {                             
                                servoRunControl.calRunPulse = servoRunControl.pulseSet - servoRunControl.decCoursePulse;
                        }                          
                        break;
                } 
                default:
                {
                        break;
                }    
        }

        // �鿴����PLC��������
        servoPulseRunToPLC += (int32)(EQep1Regs.QPOSCNT - qepPulsePLC);
        qepPulsePLC = EQep1Regs.QPOSCNT;
        
        // �����λ������Ч����ֱ���Ƴ�
        if(!servoControlEnable)
        {
            servoRunControl.servoDspFreq = frqTmp;
            return;
        }
        
        // ���������ڷ�Χ���㣬����λ�ջ�����ʱ��Ϊ��
        mAdjustTmp = (Uint32)funcCode.code.servoAdjustRange * maxFrq / 1000;                 // ���������ڷ�Χ 
        
        // �鿴�ܹ��ߵ�������
        motorPulseAll += (int32)(EQep1Regs.QPOSCNT - qepPulseCalALl);         
        qepPulseCalALl = EQep1Regs.QPOSCNT; 
   
        // ���¼������߹���������   
        servoRunControl.motorRealRunPulse += (int32)(EQep1Regs.QPOSCNT - qepPulseTemp);        
        qepPulseTemp = EQep1Regs.QPOSCNT;
        
        // ���¼�������ƫ��      
        servoRunControl.errorPulse =(int32)(servoRunControl.calRunPulse - (servoRunControl.motorRealRunPulse >> 2));
        boboerr = servoRunControl.errorPulse;
        
        // ���µ��ŷ�����ƫ��Ƚ�Сʱ��Ҫ��ǿ�ٶȻ�����     
        if (ABS_INT16(servoRunControl.errorPulse) < funcCode.code.servoSwitchPulseErr)
        {
                Uint16 upper;
            
                vcSpdLoopKp1 = funcCode.code.servoVcSpdLoopKp1 << 1;
                upper = funcCodeAttribute[GetCodeIndex(funcCode.code.servoVcSpdLoopKp1)].upper;
                if (vcSpdLoopKp1 > upper)
                {
                        vcSpdLoopKp1 = upper;
                }
                vcSpdLoopKp2 = funcCode.code.servoVcSpdLoopKp2 << 1;
                upper = funcCodeAttribute[GetCodeIndex(funcCode.code.servoVcSpdLoopKp2)].upper;
                if (vcSpdLoopKp2 > upper)
                {
                        vcSpdLoopKp2 = upper;
                }
        }
        servoRunControl.servoKp  = funcCode.code.servoLoopKp1;
        
        // ���µ��ŷ�����ƫ��Ƚϴ�ʱ���л���ǿ���ŷ�����                                  
        if(ABS_INT16(servoRunControl.errorPulse) >= funcCode.code.servoSwitchPulseErr)
        {
                servoRunControl.servoKp = funcCode.code.servoLoopKp2;                
        } 
        
        servoRunControl.adjustFreq = (((int32)servoRunControl.errorPulse *  servoRunControl.servoKp) >> SERVO_ADJUST_MULTIPLY);

        // �ŷ�΢��Ƶ���޷�
        if(ABS_INT16(servoRunControl.adjustFreq) > mAdjustTmp)
        {
                if(servoRunControl.adjustFreq < 0) 
                {
                        servoRunControl.adjustFreq = - mAdjustTmp;
                }
                else
                {                     
                        servoRunControl.adjustFreq =  mAdjustTmp;
                }        
        }
        
        // λ�ö�λ��ǰ���㴫��DSP��Ƶ��
        servoRunControl.servoDspFreq = frqTmp + servoRunControl.adjustFreq; 
}     


//=======================================================
//
// һЩ�ŷ���������
//
//=======================================================
LOCALF void servoParaCal(void)
{
        Uint32 mFrqCal; 
        Uint32 pulseTriangle;
        Uint32 pulseRectangle;     

        // ��������ǰ�Ĳ�����ʼ��׼��      
        qepPulseTemp = EQep1Regs.QPOSCNT;
        qepPulseCalALl = EQep1Regs.QPOSCNT; 
        servoRunControl.motorRealRunPulse =0;
        servoRunControl.accCoursePulse = 0;
        servoRunControl.constCoursePulse = 0;
        servoRunControl.decCoursePulse = 0;
        servoTicker = 0;
        servoRunControl.constTicker = 0;
        servoRunControl.calRunPulse = 0;  
        servoCalFlag =0; 
        
        servoRunControl.runStarFreq = frqTmp;                           //��λ����ʱ�̵�����Ƶ��
        servoRunControl.pulseSet = (Uint32)funcCode.code.servoPulseHigh *65535 + funcCode.code.servoPulseLower;//��λ�������������
        servoRunControl.accTime = funcCode.code.servoCtrlAccTime;
        servoRunControl.decTime = funcCode.code.servoCtrlDecTime;
        servoRunControl.encoderPulse = funcCode.code.servoEncoderPulse;

        // �������������
        servoRunControl.motorPolePairs = 60UL * funcCode.code.servoRatingFrq / ((int32)FRQ_UINT * funcCode.code.servoRatingSpeed); 
        

        // ���¼���Ҫ�ߵ��������
        servoRunControl.pulseSet = (Uint64)servoRunControl.pulseSet * funcCode.code.servoElectroGearB / funcCode.code.servoElectroGearA;


        // �ж��Ƿ���Ҫ���¼���
        // servoRunControl.servoLeftPulse�����¶�λ��������
        if(servoRunResetFlag)
        {
                servoRunControl.pulseSet = servoRunControl.servoLeftPulse; 
        }

        // ���㶨λĿ��Ƶ��
        mFrqCal = ((Uint64)servoRunControl.motorPolePairs * 2 * maxFrq * servoRunControl.pulseSet * FRQ_UINT
                * (TIME_UNIT_MS_PER_SEC / SERVO_ADD_DEC_TIME_UINT) + (Uint64)servoRunControl.encoderPulse 
                * servoRunControl.accTime * frqTmp * frqTmp)
                / ((Uint32)servoRunControl.encoderPulse  * (servoRunControl.accTime + servoRunControl.decTime));
                
        servoRunControl.aimFreq = qsqrt(mFrqCal);      //������        

        // ����Ŀ��Ƶ���Ƿ񳬹����Ƶ��      
        if (servoRunControl.aimFreq > maxFrq)
        {
                servoRunControl.aimFreq = maxFrq;

                pulseTriangle = ServoTrackPulseCal(frqTmp,servoRunControl.aimFreq, servoRunControl.accTime)
                              + ServoTrackPulseCal( 0,servoRunControl.aimFreq, servoRunControl.decTime);
                              
                servoRunControl.constTime = (int64)servoRunControl.motorPolePairs* (servoRunControl.pulseSet - pulseTriangle)
                                          * FRQ_UINT *  (TIME_UNIT_MS_PER_SEC/RUN_CTRL_PERIOD)//TIME_UNIT_MS_PER_SEC
                                          / ((int64)servoRunControl.aimFreq  * servoRunControl.encoderPulse);
        }
        else
        {
                servoRunControl.constTime = 0;
        } 
}


//============================================================
//
// �����Ƶ��F0�Ӽ��ٵ�F1�߹���������,tΪ�Ӽ���ʱ��
//
//============================================================
LOCALF int32 ServoTrackPulseCal(int32 f0,int32 f1,int32 t)
{
        int32 mPulse;
        
        mPulse = ((int64)servoRunControl.encoderPulse * ((int64) f1 * f1 - f0 * f0) * t)
                 /((int32)servoRunControl.motorPolePairs * 2* maxFrq * FRQ_UINT * (TIME_UNIT_MS_PER_SEC / SERVO_ADD_DEC_TIME_UINT));          
        
        return mPulse;
}

