# 目录结构
- Intellengent_classroom是嵌入式设备端qt工程目录
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
#### `void getSensorData()`
读取所有传感器数据，存到State全局变量

#### `float controlLight(int lightNum,float data)`
返回当前的状态值代表操作成功，-1代表操作失败

#### `string controlAirConditioner(int mode,int set)`
`set`是档位，1到3档
`mode`是模式，HOT为制热，COLD为制冷
返回string是json，返回切换后的模式和档位。失败的时候为0.

#### `int controlMultiMedia(int mode)`
mode是档位，MultiMediaOn等

#### `mqttUpdate()`
