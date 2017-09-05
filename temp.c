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
u8 temp_stand;//设置湿度的阙值
u8 humid_stand;//设置温度的阙值

u8 index;
u8 value[3];
u8 DisplayData[8];




//变量声明
unsigned char SenData,            //发送数据
              Flag,               //标志位
              ReData;             //接收数据

//函数声明
void usart_init();                //串口中断初始化
void send_data(unsigned char a);  //串口数据发送

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
			humi=buf[0];//整数部分
			temp=buf[2];//整数部分
		}
		
	}else return 1;
	return 0;	    
}

void datapros(void) 	 
{
//把整数转换成响应的字符，可以使用以下方法
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
//串口中断初始化
//---------------------------     
void usart_init()
{
    SCON = 0x50;                  //REN=1允许串行接受状态，串口工作模式1                      
    TMOD = 0x20;                  //定时器工作方式2                    
    PCON = 0x00;                                                          
    TH1  = 0xFD;                  //波特率9600、数据位8、停止位1。效验位无 (11.0592M)
    TL1  = 0xFD;                                                           
    ES   = 1;                     //开串口中断                  
    EA   = 1;                     //开总中断          
    TR1  = 1;                     //启动定时器
}

//---------------------------
//串口数据发送
//---------------------------
void send_data(unsigned char a)
{ 
    SBUF = a;                      //SUBF接受/发送缓冲器
    while(0 == TI);                //每次等待发送完毕，再执行下一条
    TI=0;                          //手动清0
}
//将接收到的字符数据转化为数字
u8 str2int(char* value)
{
	return (value[0]-'0')+(value[1]-'0');
}
//---------------------------
//串口中断程序
//---------------------------
void ser_int (void) interrupt 4 using 1
{
    if(1 == RI)                    //RI接受中断标志
    {
        RI = 0;                    //清除RI接受中断标志
        ReData = SBUF;             //SUBF接受/发送缓冲器
        Flag=1;                    //标志位置1表示有新数据进来
    }
} 

//---------------------------
//主函数
//---------------------------
void main()
{    
    usart_init();                  //串口中断初始化
    while(1)
    {	DHT11_Read_Data();
		if(temp<temp_stand)
			TEMP_SWITCH = 1;//打开设备升温
		else TEMP_SWITCH = 0;
		
//		if(humid<humid_stand)
//			HUMID_SWITCH = 1;
//		else HUMID_SWITCH = 0;

		if(Flag==1){  
            EA=0;                  //进入循环之后就把总中断关闭，防止处理过程中又有中断进来
            //----------------------该段即对接收的数据进行处理
			switch(ReData)
			{
			case 'a':{
				DHT11_Read_Data();
				datapros();
				//前段发送湿度数据
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
				//发送温度数据
				send_data(DisplayData[5]+'0');
				delay_ms(100);
				send_data(DisplayData[6]+'0');
				delay_ms(100);
				send_data(DisplayData[7]+'0');
				break;
				}
			case 'o'://表示open
			{	
				FAN = 1;
				break;
			}
			case 'c'://表示close
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
						temp_stand = str2int(value);//设置温度
					else 
						humid_stand = str2int(value);//设置湿度
				}
			}
			
           }    
		   //----------------------
            Flag=0;                 //数据处理完毕后将标志位置0，表示没有新数据要处理
            EA=1;                   //打开总中断，允许接收新数据
    }        
}
	}