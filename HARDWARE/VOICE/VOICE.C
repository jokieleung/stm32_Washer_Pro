#include "VOICE.H"

//��ʼ��PD0-->PD.7����ȥPD2�� �����.��ʹ��ʱ��		    
//RELAY_Init��ʼ��
void Voice_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
		
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //ʹ��PF�˿�ʱ��
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2;//|GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 ;	 //PD0-->PD.7 �˿����ã���ȥPD2��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOF, &GPIO_InitStructure);					 //�����趨������ʼ��
					 
}
u8 voiceWash_toggle_flag = 0,voiceDry_toggle_flag = 0,voiceWashDry_toggle_flag = 0;
//ȫ��ϴ�»�����״̬λ
extern u8 washing_flag,drying_flag,washDrying_flag;

//�����źż��
void VoiceDetect(void){
	//����ϴ���ź�
			if(WASH_VOICE&&(voiceWash_toggle_flag==0)){
				washing_flag = 1;
				voiceWash_toggle_flag =1;
			}
			if(!WASH_VOICE&&(voiceWash_toggle_flag==1)){
				washing_flag = 0;
				voiceWash_toggle_flag =0;
			}
			//���������ź�
			if(DRY_VOICE&&(voiceDry_toggle_flag==0)){
				drying_flag = 1;
				voiceDry_toggle_flag =1;
			}
			if(!DRY_VOICE&&(voiceDry_toggle_flag==1)){
				drying_flag = 0;
				voiceDry_toggle_flag =0;
			}
			//����ϴ���ź�
			if(WASH_DRY_VOICE&&(voiceWashDry_toggle_flag==0)){
				washDrying_flag = 1;
				voiceWashDry_toggle_flag =1;
			}
			if(!WASH_DRY_VOICE&&(voiceWashDry_toggle_flag==1)){
				washDrying_flag = 0;
				voiceWashDry_toggle_flag =0;
			}
}



 