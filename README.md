# 目录结构
- Intellengent_classroom是嵌入式设备端qt工程目录

# 硬件接口
![[figure/Pasted image 20250607211446.png]]
# Intellengent_classroom
## 数据交互格式约定
### 设备向云端报告的数据格式
```json
//设名字为jsonToCloud
{
  "device_id": "classroom-node-01",
  "time": {
    "year": 2025,
    "month": 6,
    "day": 8,
    "hour": 20,
    "minute": 30
  },
  "sensor_data": {
    "temp": 23.6,
    "humidity": 52.3,
    "lux": 285,
    "person": 1
  },
  "state": {
    "led": {
      "led1": 1,
      "led2": 0,
      "led3": 0,
      "led4": 0
    },
    "air_conditioner": {
	  "state": "on",
      "mode": "cool",
      "level": 2
    },
    "multimedia": 1
  }
}

```
### 云端向设备发送指令的数据格式

```json
//设名字为setByCloud
{
  "sensor_data": {
    "temp": 23.6,
    "humidity": 52.3,
    "lux": 285,
    "person": 1
  },
  "state": {
    "led": {
      "led1": 1,
      "led2": 0,
      "led3": 0,
      "led4": 0
    },
    "air_conditioner": {
	  "state": "on",
      "mode": "cool",
      "level": 2
    },
    "multimedia": 1
  }
}

```
## 嵌入式端模块
### 宏和全局变量
```

```
### 函数模块

#### `updateToCloud(string jsonToCloud)`
传入参数为jsonToCloud，调用该函数向云端发送该json串，包含传感器读数等。
#### `setControl(string setByCloud)`
传入参数为setByCloud，调用该函数根据setByCloud的json内容设置用电器的状态。