#include "washer.h"

u8 Washer_Mode;//ϴ��ģʽ��־λ

//����ȫ�ֱ���
//u8 process;				    //��ǰϴ�»�����״̬ȫ�ֱ���
//u8 method;				    //��ǰϴ�»����з�ʽȫ�ֱ���
//u8 speed;				    //��ǰ�綯���ٶ�ȫ�ֱ���
//u8 flag;						//ϴ����ɱ�־λ
//u8 state;					//ϴ�»�����״̬ѡ��
//u16 ad_level;				    //ˮλ����ȫ�ֱ���
//u16 ad_voltage;			    //��ѹ����ȫ�ֱ���
//u8 st_pau;				    //��ǰϴ�»�����ȫ�ֱ���
//u8 step;					    //��ǰϴ�»����в���ȫ�ֱ���

extern u16 waterin_count;//��ˮ����ֵ
extern u16 wash_count;//ϴ�¼���ֵ
extern u16 waterout_count;//��ˮ����ֵ
//���ܣ�����ϴ��ģʽ
void wash_func(){
	
	waterin_count = 0;//�����ˮʱ�����ֵ
	wash_count = 0;//����ϴ��ʱ�����ֵ
	waterout_count = 0;//������ˮʱ�����ֵ
	Washer_Mode = 1;	//����ϴ��ģʽ��־λ
	
	//****************S1:��ˮ*************************
	
	WATER_IN = 0;//�򿪽�ˮ��
	while(waterin_count < WATERIN_TIME)//�ȴ���ˮ���
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
		UpdateRemainingWashTim();
	}
	WATER_IN = 1;//�رս�ˮ��
	
	//****************S2:��ϴ*************************
	
	UNTRALSONIC = 0;//������
	while(wash_count < WASH_TIME)//�ȴ�ϴ�����
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
		UpdateRemainingWashTim();
	}
	UNTRALSONIC = 1;//�ر�����
	
	//****************S3:��ˮ*************************
	
	WATER_OUT = 0;//����ˮ��
	while(waterout_count < WATEROUT_TIME)//�ȴ���ˮ���
	{
		UpdateDisPres();
		UpdateDisTemp();
		UpdateDisHumi();
		UpdateRemainingWashTim();
	}
	WATER_OUT = 1;//�ر���ˮ��
	
	Washer_Mode = 0;//�ر�ϴ��ģʽ��־λ
	
}