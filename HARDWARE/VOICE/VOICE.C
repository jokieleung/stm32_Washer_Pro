#include "VOICE.H"

//初始化PD0-->PD.7（除去PD2） 输出口.并使能时钟		    
//RELAY_Init初始化
void Voice_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
		
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //使能PF端口时钟
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2;//|GPIO_Pin_3 |GPIO_Pin_4 |GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 ;	 //PD0-->PD.7 端口配置（除去PD2）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOF, &GPIO_InitStructure);					 //根据设定参数初始化
					 
}
u8 voiceWash_toggle_flag = 0,voiceDry_toggle_flag = 0,voiceWashDry_toggle_flag = 0;
//全局洗衣机工作状态位
extern u8 washing_flag,drying_flag,washDrying_flag;

//语音信号检测
void VoiceDetect(void){
	//语音洗衣信号
			if(WASH_VOICE&&(voiceWash_toggle_flag==0)){
				washing_flag = 1;
				voiceWash_toggle_flag =1;
			}
			if(!WASH_VOICE&&(voiceWash_toggle_flag==1)){
				washing_flag = 0;
				voiceWash_toggle_flag =0;
			}
			//语音烘衣信号
			if(DRY_VOICE&&(voiceDry_toggle_flag==0)){
				drying_flag = 1;
				voiceDry_toggle_flag =1;
			}
			if(!DRY_VOICE&&(voiceDry_toggle_flag==1)){
				drying_flag = 0;
				voiceDry_toggle_flag =0;
			}
			//语音洗烘信号
			if(WASH_DRY_VOICE&&(voiceWashDry_toggle_flag==0)){
				washDrying_flag = 1;
				voiceWashDry_toggle_flag =1;
			}
			if(!WASH_DRY_VOICE&&(voiceWashDry_toggle_flag==1)){
				washDrying_flag = 0;
				voiceWashDry_toggle_flag =0;
			}
}



 
