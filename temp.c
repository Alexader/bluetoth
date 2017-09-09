#include<reg51.h>
#include "intrins.h"

typedef unsigned char u8;
typedef unsigned int u16;

sbit DHT11_DQ_OUT=P3^6;
//--�����¶�ʹ�õ�IO��--//
sbit DSPORT=P3^7;

sbit FAN = P2^0;
sbit TEMP_SWITCH=P2^7;
sbit HUMID_SWITCH=P2^6;

int temp;//��һ����λ��������λ��ʾ�¶ȵ���������λ��ʾ�¶ȵ�С��
u8 Int_temp;
u8 humi;
u8 temp_stand;//����ʪ�ȵ���ֵ
u8 humid_stand;//�����¶ȵ���ֵ
char num=0;

u8 index;
u8 value[3];
u8 DisplayData[8];//��ʾʪ�ȵ�����

u8 DisplayData_temp[8];//��ʾ�¶ȵ�����




//��������
unsigned char SenData,            //��������
              Flag,               //��־λ
              ReData;             //��������

//��������
void usart_init();                //�����жϳ�ʼ��
void send_data(unsigned char a);  //�������ݷ���
//--����ȫ�ֺ���--//
void Delay1ms(u16 );
u8 Ds18b20Init();
void Ds18b20WriteByte(u8 com);
u8 Ds18b20ReadByte();
void  Ds18b20ChangTemp();
void  Ds18b20ReadTempCom();
int Ds18b20ReadTemp();

void delay(u16 i)
{
	while(i--);
}

void delay_ms(u16 i)
{
	while(i--)
		delay(90);
}


void Delay1ms(u16 y)
{
	u16 x;
	for( ; y>0; y--)
	{
		for(x=110; x>0; x--);
	}
}
//�¶ȴ�������صĺ���
/*******************************************************************************
* �� �� ��         : Ds18b20Init
* ��������		   : ��ʼ��
* ��    ��         : ��
* ��    ��         : ��ʼ���ɹ�����1��ʧ�ܷ���0
*******************************************************************************/

u8 Ds18b20Init()
{
	u8 i;
	DSPORT = 0;			 //����������480us~960us
	i = 70;	
	while(i--);//��ʱ642us
	DSPORT = 1;			//Ȼ���������ߣ����DS18B20������Ӧ�Ὣ��15us~60us����������
	i = 0;
	while(DSPORT)	//�ȴ�DS18B20��������
	{
		Delay1ms(1);
		i++;
		if(i>5)//�ȴ�>5MS
		{
			return 0;//��ʼ��ʧ��
		}
	
	}
	return 1;//��ʼ���ɹ�
}
/*******************************************************************************
* �� �� ��         : Ds18b20WriteByte
* ��������		   : ��18B20д��һ���ֽ�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void Ds18b20WriteByte(u8 dat)
{
	u16 i, j;

	for(j=0; j<8; j++)
	{
		DSPORT = 0;	     	  //ÿд��һλ����֮ǰ�Ȱ���������1us
		i++;
		DSPORT = dat & 0x01;  //Ȼ��д��һ�����ݣ������λ��ʼ
		i=6;
		while(i--); //��ʱ68us������ʱ������60us
		DSPORT = 1;	//Ȼ���ͷ����ߣ�����1us�����߻ָ�ʱ����ܽ���д��ڶ�����ֵ
		dat >>= 1;
	}
}

/*******************************************************************************
* �� �� ��         : Ds18b20ReadByte
* ��������		   : ��ȡһ���ֽ�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

u8 Ds18b20ReadByte()
{
	u8 byte, bi;
	u16 i, j;	
	for(j=8; j>0; j--)
	{
		DSPORT = 0;//�Ƚ���������1us
		i++;
		DSPORT = 1;//Ȼ���ͷ�����
		i++;
		i++;//��ʱ6us�ȴ������ȶ�
		bi = DSPORT;	 //��ȡ���ݣ������λ��ʼ��ȡ
		/*��byte����һλ��Ȼ����������7λ���bi��ע���ƶ�֮���Ƶ���λ��0��*/
		byte = (byte >> 1) | (bi << 7);						  
		i = 4;		//��ȡ��֮��ȴ�48us�ٽ��Ŷ�ȡ��һ����
		while(i--);
	}				
	return byte;
}

/*******************************************************************************
* �� �� ��         : Ds18b20ChangTemp
* ��������		   : ��18b20��ʼת���¶�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void  Ds18b20ChangTemp()
{
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);		//����ROM��������		 
	Ds18b20WriteByte(0x44);	    //�¶�ת������
	//Delay1ms(100);	//�ȴ�ת���ɹ������������һֱˢ�ŵĻ����Ͳ��������ʱ��
   
}
/*******************************************************************************
* �� �� ��         : Ds18b20ReadTempCom
* ��������		   : ���Ͷ�ȡ�¶�����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

void  Ds18b20ReadTempCom()
{	

	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);	 //����ROM��������
	Ds18b20WriteByte(0xbe);	 //���Ͷ�ȡ�¶�����
}

/*******************************************************************************
* �� �� ��         : Ds18b20ReadTemp
* ��������		   : ��ȡ�¶�
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/

int Ds18b20ReadTemp()
{
	int temp = 0;
	u8 tmh, tml;
	Ds18b20ChangTemp();			 	//��д��ת������
	Ds18b20ReadTempCom();			//Ȼ��ȴ�ת������Ͷ�ȡ�¶�����
	tml = Ds18b20ReadByte();		//��ȡ�¶�ֵ��16λ���ȶ����ֽ�
	tmh = Ds18b20ReadByte();		//�ٶ����ֽ�
	temp = tmh;
	temp <<= 8;
	temp |= tml;
	return temp;
}

void datapros_temp(int temp) 	 
{
   	float tp;  
	if(temp< 0)				//���¶�ֵΪ����
  	{
		DisplayData_temp[0] = 0x40; 	  //
		//��Ϊ��ȡ���¶���ʵ���¶ȵĲ��룬���Լ�1����ȡ�����ԭ��
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*100+0.5;	
		//������С�����*100��+0.5���������룬��ΪC���Ը�����ת��Ϊ���͵�ʱ���С����
		//��������Զ�ȥ���������Ƿ����0.5����+0.5֮�����0.5�ľ��ǽ�1�ˣ�С��0.5�ľ�
		//�����0.5��������С������档
 
  	}
 	else
  	{			
		DisplayData_temp[0] = 0x00;
		tp=temp;//��Ϊ���ݴ�����С�������Խ��¶ȸ���һ�������ͱ���
		//����¶���������ô����ô������ԭ����ǲ���������
		temp=tp*0.0625*100+0.5;	
		//������С�����*100��+0.5���������룬��ΪC���Ը�����ת��Ϊ���͵�ʱ���С����
		//��������Զ�ȥ���������Ƿ����0.5����+0.5֮�����0.5�ľ��ǽ�1�ˣ�С��0.5�ľ�
		//�����0.5��������С������档
	}
	DisplayData_temp[1] = temp / 1000;
	DisplayData_temp[2] = temp % 1000 / 100;
	DisplayData_temp[3] = temp % 100 / 10;
	DisplayData_temp[4] = temp % 10;
}
//--------------------------------ʪ�ȴ�������صĺ���-----------------------------------
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
	DisplayData[5] = 0x00;
	DisplayData[6] = 0x00;
	DisplayData[7] = 0x00;
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
	temp_stand = 28;
	humid_stand = 60;
	TEMP_SWITCH = 0;
	HUMID_SWITCH = 0;
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
    {	
		DHT11_Read_Data();
		temp = Ds18b20ReadTemp();
		datapros_temp(temp);
		Int_temp = temp /100;
		if(Int_temp<temp_stand)
			TEMP_SWITCH = 1;//���豸����
		else TEMP_SWITCH = 0;
		
		if(humi>humid_stand)
			HUMID_SWITCH = 1;
		else HUMID_SWITCH = 0;

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
			case 'b':{//�����¶�����
				temp = Ds18b20ReadTemp();
				datapros_temp(temp);
				send_data(DisplayData_temp[1]+'0');
				send_data(DisplayData_temp[2]+'0');
				send_data(DisplayData_temp[3]+'0');
				send_data(DisplayData_temp[4]+'0');
				break;
				}
			case 'o'://��ʾ����open
			{	
				FAN = 1;
				break;
			}
			case 'c'://��ʾ����close
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