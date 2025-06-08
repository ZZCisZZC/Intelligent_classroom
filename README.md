# 目录结构
- Intellengent_classroom是嵌入式设备端qt工程目录

# 硬件接口
![[figure/Pasted image 20250607211446.png]]
# Intellengent_classroom

## 嵌入式端模块
### 宏和全局变量
```
#define COLD 0x1100
#define HOT 0x1101
#define MultiMediaOn 0x1110
#define MultiMediaOff 0x1111
#define MultiMediaSleep 0x1112

typedef struct sensorData{
	float temperature;
	float moisture; //预留
	float illumination; //亮度
	int person; // 1表示有人，0表示无人
}
```
### 函数模块
#### `void getSensorData()`√
读取所有传感器数据，存到State全局变量
全局变量为存在Sensor单一实例里面

#### `int controlLight(int lightNum,bool data)`√
返回当前的状态值代表操作成功，-1代表操作失败

#### `void controlAirConditioner(bool state, int mode,int set)`√
`set`是档位，1到3档
`mode`是模式，1为制热，0为制冷

#### `int controlMultiMedia(int mode)`
mode是档位，MultiMediaOn等

#### `mqttUpdate()`
