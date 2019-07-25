【1】例程简介
	
	1、本例程是使用奇迹物联AM21E_V2模组进行的功能调试
	2、目前实现了使用TCP方式与使用HTTP协议上传数据（设备经纬度，温度，PM2.5等数据）到oneNET平台
	3、本例程是使用安富莱电子开源程序的BSP包
	4、本例程建议使用MDK软件进行开发
  	5、本例程在onenet平台上创建了自己的设备，用户使用需更换成自己的设备信息
	
	设置为 " 1 " 启动以下相关功能
	目标文件：\User\bsp\inc\AM21E.h
	#define ENABLEGPS			1 	//GPS功能使能

	#define ENABLETCP			1 	//TCP功能使能	
	#define ENABLEUDP			1 	//UDP功能使能	（暂未实现）

	#define ENABLEHTTP			0 	//HTTP功能使能
	
【2】修改记录
2019-07-24 V1.0 首版
