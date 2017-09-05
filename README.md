###单片机程序简介

  这是我的一个项目中的单片机程序部分。
  主要器件：
  * 单片机STC89C52
  * 温度传感器DS18B20
  * 湿度传感器DHT11
  * 蓝牙模块HC-05
  * 若干杜邦线和电阻

  主要完成的任务如下：
  * 读取温度传感器DS18B20的十六位的温度数据，转化为四个字节的字符数组，方便传输。
  * 读取湿度传感器 DHT11的一个字节的湿度数据，并转化为三个字节的数组，方便传输（虽然DHT11也能收集温度数据，但是精度上大打折扣）。
  * 利用蓝牙芯片HC-05将上述的温度和湿度数据进行串口传送，发送到手机APP中。
这是几张运行时的单片机图片，需要提醒的是，温度传感器和湿度传感器的数据引脚都需要上拉电阻，网上的教程很多，就不再赘述。
![单片机](https://github.com/Alexader/bluetoth/blob/master/microcontroller.jpg)
![传感器](https://github.com/Alexader/bluetoth/blob/master/sensors.jpg)
