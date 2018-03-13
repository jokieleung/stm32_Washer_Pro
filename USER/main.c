#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "usart3.h"
#include "beep.h"
#include "gizwits_product.h" 

#include "tim5.h"
#include "tim4.h"

#include "stm32f10x_conf.h"
#include "adc.h"
#include "USART_TO_LCD.H"
#include "washer.h"
#include "dry.h"
#include "VOICE.H"
/************************************************
 ������-IOT���������΢��ϴ��һ��� V1.0
 �人���׿Ƽ����޹�˾ 
 ���ߣ�Jokie @jokieleung@qq.com
************************************************/


/* �û�����ǰ�豸״̬�ṹ��*/
dataPoint_t currentDataPoint;

//ȫ��ϴ�»�����״̬λ
u8 washing_flag=0,drying_flag=0,washDrying_flag=0;

//����ֵ������ֵ��ȫ�ֱ��� 
extern float Temp,Humi,Pres;
extern int RemainWashTime,DryTime,WashDryTime;

//UI��תָ�� ����
extern u8 START_W_ARY[];//����ϴ�½���
extern u8 START_D_ARY[];//���ں�ɽ���
extern u8 START_WD_ARY[];//���ں�ϴ����

//Э���ʼ��
void Gizwits_Init(void)
{  
	
	TIM3_Int_Init(9,7199);//1MSϵͳ��ʱ
    usart3_init(9600);//WIFI��ʼ��
	memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));//�豸״̬�ṹ���ʼ��
	gizwitsInit();//��������ʼ��
}
//���ݲɼ��������߼���
void userHandle(void)
{
   
	//����״̬�ϴ�
	currentDataPoint.valueWash = washing_flag;
	currentDataPoint.valueDry = drying_flag;
	currentDataPoint.valueWashDry = washDrying_flag;
	//��նȡ���ʪ���ϴ�
	currentDataPoint.valuePres = Pres;
	currentDataPoint.valueTemp = Temp;
	currentDataPoint.valueHumi = Humi;
	//������״̬��ʵʱʱ���ϴ�
	currentDataPoint.valueWashTime = RemainWashTime;
  currentDataPoint.valueDryTime = DryTime;
  currentDataPoint.valueWashDryTime = WashDryTime;
	
}


//������
 int main(void)
 {		
	 u8 DRY_FSH[]={0XA5,0X5A,0X04,0X80,0X03,0X00,0X0D};//������ɽ���
	 u8 WSHDRY_FSH[]={0XA5,0X5A,0X04,0X80,0X03,0X00,0X0C};//��ϴ��ɽ���
	 delay_init();	    	 //��ʱ������ʼ��	  
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	 Adc_Init();
	 SHT2x_Init();       //SHT2x��ʼ�� ��I2C��ʼ��
	 USART2_Init(115200);	//����2��ʼ��Ϊ115200���봮����ͨ��
	 RELAY_Init();//�̵�����ʼ������ʼ��Ĭ������Ϊ�ߵ�ƽ���̵����رգ�
	 TIM4_Int_Init(99,7199);//��������Ϊ10ms ������Ļ�İ������
	 uart_init(115200);	    //����1��ʼ��Ϊ115200 ���Դ�ӡ��
	 LED_Init();			    //LED�˿ڳ�ʼ��
	 KEY_Init();             //������ʼ�� 
	 BEEP_Init();            //��������ʼ��
   Gizwits_Init();         //Э���ʼ��
	 TIM5_Int_Init(9999,7199);//��������Ϊ1s   �˺��޸�Ϊ��ʱϴ��ʱ���Լ����ʱ����
	 Voice_Init();
	 printf("--------������IOT-VacuumWasher----------\r\n");
	 printf("Init Over\r\n\r\n");
   	while(1)
	{
		
		//ϴ��״̬
		if(washing_flag){
			JumpToUI(START_W_ARY);
			wash_func();
			JumpToFinishedUI();//��ת��ϴ����ɵ�ҳ��
			washing_flag = 0;
		}
		//����״̬
		if(drying_flag){
			JumpToUI(START_D_ARY);
			dry_func();
			JumpToUI(DRY_FSH);//��ת��������ɵ�ҳ��
			drying_flag = 0;
		}
		//ϴ��״̬
		if(washDrying_flag){
			JumpToUI(START_WD_ARY);
			WashDryfunc();
			JumpToUI(WSHDRY_FSH);//��ת��ϴ������ɵ�ҳ��
			washDrying_flag = 0;
		}
		
	}	 

} 
