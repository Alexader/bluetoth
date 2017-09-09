#include<reg51.h>
#include "intrins.h"

typedef unsigned char u8;
typedef unsigned int u16;

sbit DHT11_DQ_OUT=P3^6;
//--定义温度使用的IO口--//
sbit DSPORT=P3^7;

sbit FAN = P2^0;
sbit TEMP_SWITCH=P2^7;
sbit HUMID_SWITCH=P2^6;

int temp;//是一个四位整数，两位表示温度的整数，两位表示温度的小数
u8 Int_temp;
u8 humi;
u8 temp_stand;//设置湿度的阙值
u8 humid_stand;//设置温度的阙值
char num=0;

u8 index;
u8 value[3];
u8 DisplayData[8];//显示湿度的数组

u8 DisplayData_temp[8];//显示温度的数组




//变量声明
unsigned char SenData,            //发送数据
              Flag,               //标志位
              ReData;             //接收数据

//函数声明
void usart_init();                //串口中断初始化
void send_data(unsigned char a);  //串口数据发送
//--声明全局函数--//
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
//温度传感器相关的函数
/*******************************************************************************
* 函 数 名         : Ds18b20Init
* 函数功能		   : 初始化
* 输    入         : 无
* 输    出         : 初始化成功返回1，失败返回0
*******************************************************************************/

u8 Ds18b20Init()
{
	u8 i;
	DSPORT = 0;			 //将总线拉低480us~960us
	i = 70;	
	while(i--);//延时642us
	DSPORT = 1;			//然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	i = 0;
	while(DSPORT)	//等待DS18B20拉低总线
	{
		Delay1ms(1);
		i++;
		if(i>5)//等待>5MS
		{
			return 0;//初始化失败
		}
	
	}
	return 1;//初始化成功
}
/*******************************************************************************
* 函 数 名         : Ds18b20WriteByte
* 函数功能		   : 向18B20写入一个字节
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void Ds18b20WriteByte(u8 dat)
{
	u16 i, j;

	for(j=0; j<8; j++)
	{
		DSPORT = 0;	     	  //每写入一位数据之前先把总线拉低1us
		i++;
		DSPORT = dat & 0x01;  //然后写入一个数据，从最低位开始
		i=6;
		while(i--); //延时68us，持续时间最少60us
		DSPORT = 1;	//然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		dat >>= 1;
	}
}

/*******************************************************************************
* 函 数 名         : Ds18b20ReadByte
* 函数功能		   : 读取一个字节
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

u8 Ds18b20ReadByte()
{
	u8 byte, bi;
	u16 i, j;	
	for(j=8; j>0; j--)
	{
		DSPORT = 0;//先将总线拉低1us
		i++;
		DSPORT = 1;//然后释放总线
		i++;
		i++;//延时6us等待数据稳定
		bi = DSPORT;	 //读取数据，从最低位开始读取
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte = (byte >> 1) | (bi << 7);						  
		i = 4;		//读取完之后等待48us再接着读取下一个数
		while(i--);
	}				
	return byte;
}

/*******************************************************************************
* 函 数 名         : Ds18b20ChangTemp
* 函数功能		   : 让18b20开始转换温度
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void  Ds18b20ChangTemp()
{
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);		//跳过ROM操作命令		 
	Ds18b20WriteByte(0x44);	    //温度转换命令
	//Delay1ms(100);	//等待转换成功，而如果你是一直刷着的话，就不用这个延时了
   
}
/*******************************************************************************
* 函 数 名         : Ds18b20ReadTempCom
* 函数功能		   : 发送读取温度命令
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void  Ds18b20ReadTempCom()
{	

	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);	 //跳过ROM操作命令
	Ds18b20WriteByte(0xbe);	 //发送读取温度命令
}

/*******************************************************************************
* 函 数 名         : Ds18b20ReadTemp
* 函数功能		   : 读取温度
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

int Ds18b20ReadTemp()
{
	int temp = 0;
	u8 tmh, tml;
	Ds18b20ChangTemp();			 	//先写入转换命令
	Ds18b20ReadTempCom();			//然后等待转换完后发送读取温度命令
	tml = Ds18b20ReadByte();		//读取温度值共16位，先读低字节
	tmh = Ds18b20ReadByte();		//再读高字节
	temp = tmh;
	temp <<= 8;
	temp |= tml;
	return temp;
}

void datapros_temp(int temp) 	 
{
   	float tp;  
	if(temp< 0)				//当温度值为负数
  	{
		DisplayData_temp[0] = 0x40; 	  //
		//因为读取的温度是实际温度的补码，所以减1，再取反求出原码
		temp=temp-1;
		temp=~temp;
		tp=temp;
		temp=tp*0.0625*100+0.5;	
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
 
  	}
 	else
  	{			
		DisplayData_temp[0] = 0x00;
		tp=temp;//因为数据处理有小数点所以将温度赋给一个浮点型变量
		//如果温度是正的那么，那么正数的原码就是补码它本身
		temp=tp*0.0625*100+0.5;	
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
	}
	DisplayData_temp[1] = temp / 1000;
	DisplayData_temp[2] = temp % 1000 / 100;
	DisplayData_temp[3] = temp % 100 / 10;
	DisplayData_temp[4] = temp % 10;
}
//--------------------------------湿度传感器相关的函数-----------------------------------
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
	DisplayData[5] = 0x00;
	DisplayData[6] = 0x00;
	DisplayData[7] = 0x00;
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
	temp_stand = 28;
	humid_stand = 60;
	TEMP_SWITCH = 0;
	HUMID_SWITCH = 0;
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
    {	
		DHT11_Read_Data();
		temp = Ds18b20ReadTemp();
		datapros_temp(temp);
		Int_temp = temp /100;
		if(Int_temp<temp_stand)
			TEMP_SWITCH = 1;//打开设备升温
		else TEMP_SWITCH = 0;
		
		if(humi>humid_stand)
			HUMID_SWITCH = 1;
		else HUMID_SWITCH = 0;

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
			case 'b':{//发送温度数据
				temp = Ds18b20ReadTemp();
				datapros_temp(temp);
				send_data(DisplayData_temp[1]+'0');
				send_data(DisplayData_temp[2]+'0');
				send_data(DisplayData_temp[3]+'0');
				send_data(DisplayData_temp[4]+'0');
				break;
				}
			case 'o'://表示风扇open
			{	
				FAN = 1;
				break;
			}
			case 'c'://表示风扇close
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