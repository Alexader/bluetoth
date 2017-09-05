#include<reg51.h>
#include "intrins.h"

typedef unsigned char u8;
typedef unsigned int u16;

sbit DHT11_DQ_OUT=P3^6;
sbit FAN = P2^1;
sbit TEMP_SWITCH=P2^7;
sbit HUMID_SWITCH=P2^6;

u8 temp;
u8 humi;
u8 temp_stand;//����ʪ�ȵ���ֵ
u8 humid_stand;//�����¶ȵ���ֵ

u8 index;
u8 value[3];
u8 DisplayData[8];




//��������
unsigned char SenData,            //��������
              Flag,               //��־λ
              ReData;             //��������

//��������
void usart_init();                //�����жϳ�ʼ��
void send_data(unsigned char a);  //�������ݷ���

void delay(u16 i)
{
	while(i--);
}

void delay_ms(u16 i)
{
	while(i--)
		delay(90);
}

void DHT11_Rst()	   
{                 
    DHT11_DQ_OUT=0; 
    delay_ms(20);    
    DHT11_DQ_OUT=1; 
	delay(3); 
}

u8 DHT11_Check() 	   
{   
	u8 retry=0;	 
    while (DHT11_DQ_OUT&&retry<100)
	{
		retry++;
		_nop_();
	};	 
	if(retry>=100)return 1;
	else retry=0;
    while (!DHT11_DQ_OUT&&retry<100)
	{
		retry++;
		_nop_();
	};
	if(retry>=100)return 1;	    
	return 0;
}

u8 DHT11_Read_Bit(void) 			 
{
 	u8 retry=0;
	while(DHT11_DQ_OUT&&retry<100)
	{
		retry++;
		_nop_();
	}
	retry=0;
	while((!DHT11_DQ_OUT)&&retry<100)
	{
		retry++;
		_nop_();
	}
	delay(1);
	if(DHT11_DQ_OUT)return 1;
	else return 0;		   
}

u8 DHT11_Read_Byte(void)    
{        
    u8 i,dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }						    
    return dat;
}

u8 DHT11_Read_Data(void)    
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			humi=buf[0];//��������
			temp=buf[2];//��������
		}
		
	}else return 1;
	return 0;	    
}

void datapros(void) 	 
{
//������ת������Ӧ���ַ�������ʹ�����·���
//char c = (char)(int temp)+'0';
	DisplayData[0] = humi / 100;
	DisplayData[1] = humi % 100 / 10;
	DisplayData[2] = humi % 10;
	DisplayData[3] = 0x00;
	DisplayData[4] = 0x00;                                          
	DisplayData[5] = temp /100;
	DisplayData[6] = temp % 100 /10;
	DisplayData[7] = temp % 10;
}

//---------------------------
//�����жϳ�ʼ��
//---------------------------     
void usart_init()
{
    SCON = 0x50;                  //REN=1�����н���״̬�����ڹ���ģʽ1                      
    TMOD = 0x20;                  //��ʱ��������ʽ2                    
    PCON = 0x00;                                                          
    TH1  = 0xFD;                  //������9600������λ8��ֹͣλ1��Ч��λ�� (11.0592M)
    TL1  = 0xFD;                                                           
    ES   = 1;                     //�������ж�                  
    EA   = 1;                     //�����ж�          
    TR1  = 1;                     //������ʱ��
}

//---------------------------
//�������ݷ���
//---------------------------
void send_data(unsigned char a)
{ 
    SBUF = a;                      //SUBF����/���ͻ�����
    while(0 == TI);                //ÿ�εȴ�������ϣ���ִ����һ��
    TI=0;                          //�ֶ���0
}
//�����յ����ַ�����ת��Ϊ����
u8 str2int(char* value)
{
	return (value[0]-'0')+(value[1]-'0');
}
//---------------------------
//�����жϳ���
//---------------------------
void ser_int (void) interrupt 4 using 1
{
    if(1 == RI)                    //RI�����жϱ�־
    {
        RI = 0;                    //���RI�����жϱ�־
        ReData = SBUF;             //SUBF����/���ͻ�����
        Flag=1;                    //��־λ��1��ʾ�������ݽ���
    }
} 

//---------------------------
//������
//---------------------------
void main()
{    
    usart_init();                  //�����жϳ�ʼ��
    while(1)
    {	DHT11_Read_Data();
		if(temp<temp_stand)
			TEMP_SWITCH = 1;//���豸����
		else TEMP_SWITCH = 0;
		
//		if(humid<humid_stand)
//			HUMID_SWITCH = 1;
//		else HUMID_SWITCH = 0;

		if(Flag==1){  
            EA=0;                  //����ѭ��֮��Ͱ����жϹرգ���ֹ��������������жϽ���
            //----------------------�öμ��Խ��յ����ݽ��д���
			switch(ReData)
			{
			case 'a':{
				DHT11_Read_Data();
				datapros();
				//ǰ�η���ʪ������
                send_data(DisplayData[0]+'0');
//				delay_ms(100);
				send_data(DisplayData[1]+'0');
//				delay_ms(100);
				send_data(DisplayData[2]+'0');
				break;
				}
			case 'b':{
				DHT11_Read_Data();
				datapros();
				//�����¶�����
				send_data(DisplayData[5]+'0');
				delay_ms(100);
				send_data(DisplayData[6]+'0');
				delay_ms(100);
				send_data(DisplayData[7]+'0');
				break;
				}
			case 'o'://��ʾopen
			{	
				FAN = 1;
				break;
			}
			case 'c'://��ʾclose
			{	
				FAN = 0;
				break;
			}
			default:
			{
				value[index] = ReData;
				index++;
				if(index==3) 
				{
					index=0;
					if(value[2]=='t')
						temp_stand = str2int(value);//�����¶�
					else 
						humid_stand = str2int(value);//����ʪ��
				}
			}
			
           }    
		   //----------------------
            Flag=0;                 //���ݴ�����Ϻ󽫱�־λ��0����ʾû��������Ҫ����
            EA=1;                   //�����жϣ��������������
    }        
}
	}