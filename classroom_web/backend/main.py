# main.py
import json
import threading
from datetime import date, datetime, timedelta

from fastapi import FastAPI, Query, HTTPException
from paho.mqtt import client as mqtt
from sqlalchemy.orm import Session

# main.py 增加的 import
from sqlalchemy import func, cast, Date
from typing import List, Optional, Dict
from sqlalchemy import text
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from models import init_db, SessionLocal, DeviceData

app = FastAPI()

# 添加CORS中间件
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://localhost:8080",
        "http://127.0.0.1:8080"
    ],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 先建表
init_db()

# ---- 配置区，填你的 APIKey/Topic ----
MQTT_BROKER = "bemfa.com"
MQTT_PORT = 9501
CLIENT_ID   = "f7f3759ee3cc47068f3f28196cc83ef2"
TOPIC       = "dataUpdate"
CONTROL_TOPIC = "setControl"  # 控制命令的topic
# ---------------------------------------

# 存储最新数据的字典
latest_data: Dict = {}
latest_data_lock = threading.Lock()

# MQTT客户端全局变量
mqtt_client = None

def compute_power(state: dict) -> float:
    """根据 state 计算总功率，不带单位"""
    total = 0.0

    # 1. 多媒体
    mm = state.get("multimedia", "").lower()
    if mm == "on":
        total += 450.0
    elif mm == "standby":
        total += 0.5
    # off 为 0

    # 2. 灯
    leds = state.get("led", {})
    total += sum(leds.get(k, 0) for k in leds) * 100.0

    # 3. 空调
    ac = state.get("air_conditioner", {})
    if ac.get("state", "").lower() == "on":
        lvl = ac.get("level", 0)
        mode = ac.get("mode", "").lower()
        if mode == "cool":
            total += {1: 800, 2: 3000, 3: 6200}.get(lvl, 0)
        elif mode == "heat":
            total += {1: 800, 2: 4000, 3: 8400}.get(lvl, 0)
    # state off 不加功率

    return total

def should_save_to_db(timestamp: datetime) -> bool:
    """判断是否应该保存到数据库（整点）"""
    return timestamp.minute == 0

def mqtt_loop():
    global mqtt_client
    
    def on_connect(client, userdata, flags, rc):
        print("✅ MQTT 已连接，代码：", rc)
        client.subscribe(TOPIC)

    def on_message(client, userdata, msg):
        try:
            # 尝试解码和解析JSON
            try:
                payload_str = msg.payload.decode('utf-8')
                payload = json.loads(payload_str)
            except (UnicodeDecodeError, json.JSONDecodeError) as e:
                print(f"⚠️ 接收到无效消息数据，已忽略: {e}")
                return  # 忽略无效消息，继续处理下一条
            
            # 检查消息格式
            if not isinstance(payload, dict) or 'time' not in payload or 'state' not in payload:
                print(f"⚠️ 消息格式不正确，已忽略: {payload}")
                return
            
            # 解析时间
            t = payload["time"]
            ts = datetime(t["year"], t["month"], t["day"], t["hour"], t["minute"])

            # 计算功率
            power_value = compute_power(payload["state"])

            # 更新最新数据
            data = {
                "device_id": payload["device_id"],
                "timestamp": ts,
                "sensor_data": payload["sensor_data"],
                "state": payload["state"],
                "power": power_value
            }
            
            with latest_data_lock:
                global latest_data
                latest_data = data
                print(f"📡 更新最新数据：{payload['device_id']} @ {ts}，功率={power_value}")

            # 只在整点时保存到数据库
            if should_save_to_db(ts):
                # 创建记录
                rec = DeviceData(
                    device_id=payload["device_id"],
                    timestamp=ts,
                    sensor_data=payload["sensor_data"],
                    state=payload["state"],
                    power=power_value
                )
                db: Session = SessionLocal()
                try:
                    db.add(rec)
                    db.commit()
                    print(f"💾 存储到数据库：{payload['device_id']} @ {ts}，功率={power_value}")
                except Exception as db_error:
                    print(f"❌ 数据库存储失败: {db_error}")
                    db.rollback()
                finally:
                    db.close()
                    
        except Exception as e:
            print(f"❌ 处理MQTT消息时发生未知错误: {e}")
            # 继续处理下一条消息，不让异常中断MQTT循环

    mqtt_client = mqtt.Client(client_id=CLIENT_ID)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    
    # 添加连接重试机制
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT)
        mqtt_client.loop_forever()
    except Exception as e:
        print(f"❌ MQTT连接失败: {e}")
        # 可以在这里添加重连逻辑

@app.on_event("startup")
def start_mqtt():
    threading.Thread(target=mqtt_loop, daemon=True).start()

@app.get("/latest")
def get_latest():
    with latest_data_lock:
        if not latest_data:
            return {}
        # 添加调试信息
        print(f"🔍 返回最新数据: 设备={latest_data.get('device_id')}, 功率={latest_data.get('power')}")
        return latest_data

class HistoryQueryParams(BaseModel):
    start_date: str
    end_date: str
    data_type: str  # 'temperature', 'humidity', 'light', 'occupancy', 'power'
    unit: str  # 'day' or 'hour'

@app.get("/available-dates")
async def get_available_dates():
    db = SessionLocal()
    try:
        # 查询最早和最晚的记录时间
        earliest = db.query(func.min(DeviceData.timestamp)).scalar()
        latest = db.query(func.max(DeviceData.timestamp)).scalar()
        
        if not earliest or not latest:
            return {"dates": []}
        
        # 生成日期列表
        dates = []
        current = earliest.date()
        while current <= latest.date():
            # 检查该日期是否有数据
            has_data = db.query(DeviceData).filter(
                func.date(DeviceData.timestamp) == current
            ).first() is not None
            
            dates.append({
                "date": current.isoformat(),
                "has_data": has_data
            })
            current += timedelta(days=1)
            
        return {"dates": dates}
    finally:
        db.close()

@app.post("/query-history")
async def query_history(params: HistoryQueryParams):
    print(f"接收到查询请求: {params}")  # 调试日志
    db = SessionLocal()
    try:
        start_date = datetime.strptime(params.start_date, "%Y-%m-%d")
        end_date = datetime.strptime(params.end_date, "%Y-%m-%d") + timedelta(days=1)
        
        # 基础查询
        query = db.query(DeviceData).filter(
            DeviceData.timestamp >= start_date,
            DeviceData.timestamp < end_date
        )
        
        # 检查是否有数据
        total_records = query.count()
        print(f"查询到的总记录数: {total_records}")  # 调试日志
        
        if params.unit == "hour":
            # 按小时查询，直接返回原始数据
            results = query.order_by(DeviceData.timestamp).all()
            data = []
            for record in results:
                value = None
                if params.data_type == "temperature":
                    value = record.sensor_data.get("temp")
                elif params.data_type == "humidity":
                    value = record.sensor_data.get("humidity")
                elif params.data_type == "light":
                    value = record.sensor_data.get("lux")
                elif params.data_type == "occupancy":
                    value = record.sensor_data.get("person") == "true"
                elif params.data_type == "power":
                    value = record.power / 1000 if record.power is not None else None
                
                if value is not None:
                    data.append({
                        "timestamp": record.timestamp.isoformat(),
                        "value": value
                    })
            
            print(f"处理后的数据点数量: {len(data)}")  # 调试日志
        
        else:  # params.unit == "day"
            # 按天汇总数据
            if params.data_type == "power":
                # 按天汇总功耗，并转换为千瓦时
                results = db.query(
                    func.date(DeviceData.timestamp).label("date"),
                    func.sum(DeviceData.power).label("total_power")
                ).filter(
                    DeviceData.timestamp >= start_date,
                    DeviceData.timestamp < end_date
                ).group_by(
                    func.date(DeviceData.timestamp)
                ).all()
                
                data = [{
                    "timestamp": str(record.date),
                    # 将瓦时转换为千瓦时
                    "value": float(record.total_power) / 1000 if record.total_power is not None else None
                } for record in results if record.total_power is not None]  # 只添加有值的数据点
            else:
                # 其他类型数据按天平均值显示
                results = []
                current = start_date
                while current < end_date:
                    next_day = current + timedelta(days=1)
                    day_records = query.filter(
                        DeviceData.timestamp >= current,
                        DeviceData.timestamp < next_day
                    ).all()
                    
                    if day_records:  # 只处理有数据的日期
                        values = []
                        for record in day_records:
                            if params.data_type == "temperature":
                                value = record.sensor_data.get("temp")
                            elif params.data_type == "humidity":
                                value = record.sensor_data.get("humidity")
                            elif params.data_type == "light":
                                value = record.sensor_data.get("lux")
                            elif params.data_type == "occupancy":
                                value = record.sensor_data.get("person") == "true"
                            
                            if value is not None:
                                values.append(value)
                        
                        if values:  # 只添加有值的数据点
                            results.append({
                                "timestamp": current.date().isoformat(),
                                "value": sum(values) / len(values)  # 计算平均值
                            })
                    
                    current = next_day
                
                data = results
        
        return {
            "data": data,
            "unit": params.unit,
            "data_type": params.data_type
        }
    finally:
        db.close()

class DeviceControlRequest(BaseModel):
    state: Dict

@app.post("/control")
async def control_device(request: DeviceControlRequest):
    """发送设备控制命令"""
    global mqtt_client
    
    if not mqtt_client:
        raise HTTPException(status_code=500, detail="MQTT客户端未连接")
    
    try:
        # 发送控制命令到MQTT
        control_message = {
            "state": request.state
        }
        
        mqtt_client.publish(CONTROL_TOPIC, json.dumps(control_message))
        print(f"🎛️ 发送控制命令: {control_message}")
        
        return {"success": True, "message": "控制命令已发送"}
    except Exception as e:
        print(f"❌ 发送控制命令失败: {e}")
        raise HTTPException(status_code=500, detail=f"发送控制命令失败: {str(e)}")

