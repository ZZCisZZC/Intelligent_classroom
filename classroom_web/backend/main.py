# main.py
import json
import threading
from datetime import date, datetime, timedelta
import os
from dotenv import load_dotenv
import asyncio
from concurrent.futures import ThreadPoolExecutor

from fastapi import FastAPI, Query, HTTPException
from paho.mqtt import client as mqtt
from sqlalchemy.orm import Session

# main.py 增加的 import
from sqlalchemy import func, cast, Date
from typing import List, Optional, Dict
from sqlalchemy import text
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from models import init_db, SessionLocal, DeviceData, AutomationRule
# 导入AI服务
from ai_service import (
    llm_client, 
    set_global_references, 
    process_chat_sync, 
    ChatMessage, 
    ChatResponse
)
# 导入自动化调度器
from automation_scheduler import automation_scheduler

# 加载.env文件
load_dotenv()

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
MQTT_BROKER = os.getenv("MQTT_BROKER", "")  
MQTT_PORT = int(os.getenv("MQTT_PORT", ""))
CLIENT_ID   = os.getenv("CLIENT_ID", "")
TOPIC       = os.getenv("TOPIC", "")
CONTROL_TOPIC = os.getenv("CONTROL_TOPIC", "")  # 控制命令的topic
# ---------------------------------------

# 创建线程池用于异步处理AI调用
executor = ThreadPoolExecutor(max_workers=3)

# 使用容器类来确保引用传递
class DataContainer:
    def __init__(self):
        self.data = None

# 存储最新数据的容器
latest_data_container = DataContainer()
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
        # 设置AI服务的全局变量引用
        set_global_references(latest_data_container, latest_data_lock, mqtt_client, CONTROL_TOPIC)
        # 设置自动化调度器的MQTT客户端和数据引用
        automation_scheduler.set_mqtt_client(mqtt_client, CONTROL_TOPIC)
        automation_scheduler.set_data_references(latest_data_container, latest_data_lock)

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
                latest_data_container.data = data
                print(f"更新最新数据：{payload['device_id']} @ {ts}，功率={power_value}")
            
            # 更新自动化调度器的设备时间
            automation_scheduler.update_device_time(ts)

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
                    print(f"存储到数据库：{payload['device_id']} @ {ts}，功率={power_value}")
                except Exception as db_error:
                    print(f"数据库存储失败: {db_error}")
                    db.rollback()
                finally:
                    db.close()
                    
        except Exception as e:
            print(f"处理MQTT消息时发生未知错误: {e}")
            # 继续处理下一条消息，不让异常中断MQTT循环

    mqtt_client = mqtt.Client(client_id=CLIENT_ID)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    
    # 添加连接重试机制
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT)
        mqtt_client.loop_forever()
    except Exception as e:
        print(f"MQTT连接失败: {e}")
        # 可以在这里添加重连逻辑

@app.on_event("startup")
def start_mqtt():
    threading.Thread(target=mqtt_loop, daemon=True).start()
    # 启动自动化调度器
    automation_scheduler.start()

@app.on_event("shutdown")
def shutdown_event():
    """应用关闭时清理资源"""
    executor.shutdown(wait=True)
    automation_scheduler.stop()
    print("🔄 线程池已关闭")

@app.get("/latest")
def get_latest():
    with latest_data_lock:
        if not latest_data_container.data:
            return {}
        # 添加调试信息
        print(f"返回最新数据: 设备={latest_data_container.data.get('device_id')}, 功率={latest_data_container.data.get('power')}")
        return latest_data_container.data

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
                            
                            # 剔除-1的异常数据和None值
                            if value is not None and value != -1:
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
        print(f"发送控制命令: {control_message}")
        
        return {"success": True, "message": "控制命令已发送"}
    except Exception as e:
        print(f"发送控制命令失败: {e}")
        raise HTTPException(status_code=500, detail=f"发送控制命令失败: {str(e)}")

@app.post("/chat")
async def chat_with_ai(message: ChatMessage):
    """与AI助手对话 - 异步处理避免阻塞其他请求，支持多轮对话"""
    if not llm_client:
        raise HTTPException(status_code=500, detail="大模型服务未配置，请设置DASHSCOPE_API_KEY环境变量")
    
    try:
        # 在线程池中异步执行AI处理
        loop = asyncio.get_event_loop()
        result = await loop.run_in_executor(
            executor, 
            process_chat_sync, 
            message.message,
            message.history
        )
        
        return ChatResponse(
            reply=result["reply"],
            action_taken=result["action_taken"]
        )
        
    except Exception as e:
        print(f"异步AI对话处理失败: {e}")
        error_reply = "哎呀，我现在有点迷糊，暂时无法回应您。不过您可以直接通过界面上的控制面板来管理设备哦！"
        return ChatResponse(reply=error_reply)

# ======== 自动化规则相关API ========

class AutomationRuleCreate(BaseModel):
    name: str
    description: Optional[str] = ""
    schedule: Dict  # {"type": "daily/weekly", "time": "HH:MM", "days": [1,2,3]}
    actions: Dict   # 执行的操作

class AutomationRuleUpdate(BaseModel):
    name: Optional[str] = None
    description: Optional[str] = None
    enabled: Optional[bool] = None
    schedule: Optional[Dict] = None
    actions: Optional[Dict] = None

@app.get("/automation/rules")
async def get_automation_rules():
    """获取所有自动化规则"""
    db: Session = SessionLocal()
    try:
        rules = db.query(AutomationRule).order_by(AutomationRule.created_at.desc()).all()
        return {
            "success": True,
            "data": [
                {
                    "id": rule.id,
                    "name": rule.name,
                    "description": rule.description,
                    "enabled": rule.enabled,
                    "schedule": rule.schedule,
                    "actions": rule.actions,
                    "created_at": rule.created_at.isoformat(),
                    "updated_at": rule.updated_at.isoformat()
                }
                for rule in rules
            ]
        }
    except Exception as e:
        print(f"获取自动化规则失败: {e}")
        raise HTTPException(status_code=500, detail="获取自动化规则失败")
    finally:
        db.close()

@app.post("/automation/rules")
async def create_automation_rule(rule_data: AutomationRuleCreate):
    """创建自动化规则"""
    db: Session = SessionLocal()
    try:
        # 验证调度配置
        schedule = rule_data.schedule
        if not schedule.get("type") in ["daily", "weekly"]:
            raise HTTPException(status_code=400, detail="调度类型必须是 daily 或 weekly")
        
        if not schedule.get("time"):
            raise HTTPException(status_code=400, detail="必须指定执行时间")
        
        # 验证时间格式
        try:
            time_parts = schedule["time"].split(":")
            hour, minute = int(time_parts[0]), int(time_parts[1])
            if not (0 <= hour <= 23 and 0 <= minute <= 59):
                raise ValueError()
        except (ValueError, IndexError):
            raise HTTPException(status_code=400, detail="时间格式无效，请使用 HH:MM 格式")
        
        # 如果是周期性，验证星期几
        if schedule["type"] == "weekly":
            days = schedule.get("days", [])
            if not days or not all(1 <= day <= 7 for day in days):
                raise HTTPException(status_code=400, detail="周期性调度必须指定有效的星期几（1-7）")
        
        # 检查规则名称是否已存在
        existing_rule = db.query(AutomationRule).filter(AutomationRule.name == rule_data.name).first()
        if existing_rule:
            raise HTTPException(status_code=400, detail=f"规则名称 '{rule_data.name}' 已存在，请使用其他名称")
        
        # 创建规则
        rule = AutomationRule(
            name=rule_data.name,
            description=rule_data.description,
            schedule=rule_data.schedule,
            actions=rule_data.actions
        )
        
        db.add(rule)
        db.commit()
        db.refresh(rule)
        
        return {
            "success": True,
            "message": "自动化规则创建成功",
            "data": {
                "id": rule.id,
                "name": rule.name,
                "description": rule.description,
                "enabled": rule.enabled,
                "schedule": rule.schedule,
                "actions": rule.actions,
                "created_at": rule.created_at.isoformat(),
                "updated_at": rule.updated_at.isoformat()
            }
        }
    except HTTPException:
        raise
    except Exception as e:
        print(f"创建自动化规则失败: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail="创建自动化规则失败")
    finally:
        db.close()

@app.put("/automation/rules/{rule_id}")
async def update_automation_rule(rule_id: int, rule_data: AutomationRuleUpdate):
    """更新自动化规则"""
    db: Session = SessionLocal()
    try:
        rule = db.query(AutomationRule).filter(AutomationRule.id == rule_id).first()
        if not rule:
            raise HTTPException(status_code=404, detail="规则不存在")
        
        # 更新字段
        if rule_data.name is not None:
            # 检查规则名称是否已存在（排除当前规则）
            existing_rule = db.query(AutomationRule).filter(
                AutomationRule.name == rule_data.name,
                AutomationRule.id != rule_id
            ).first()
            if existing_rule:
                raise HTTPException(status_code=400, detail=f"规则名称 '{rule_data.name}' 已存在，请使用其他名称")
            rule.name = rule_data.name
        if rule_data.description is not None:
            rule.description = rule_data.description
        if rule_data.enabled is not None:
            rule.enabled = rule_data.enabled
        if rule_data.schedule is not None:
            # 验证调度配置
            schedule = rule_data.schedule
            if schedule.get("type") and schedule["type"] not in ["daily", "weekly"]:
                raise HTTPException(status_code=400, detail="调度类型必须是 daily 或 weekly")
            
            if schedule.get("time"):
                try:
                    time_parts = schedule["time"].split(":")
                    hour, minute = int(time_parts[0]), int(time_parts[1])
                    if not (0 <= hour <= 23 and 0 <= minute <= 59):
                        raise ValueError()
                except (ValueError, IndexError):
                    raise HTTPException(status_code=400, detail="时间格式无效，请使用 HH:MM 格式")
            
            rule.schedule = rule_data.schedule
        if rule_data.actions is not None:
            rule.actions = rule_data.actions
        
        rule.updated_at = datetime.utcnow()
        db.commit()
        db.refresh(rule)
        
        return {
            "success": True,
            "message": "自动化规则更新成功",
            "data": {
                "id": rule.id,
                "name": rule.name,
                "description": rule.description,
                "enabled": rule.enabled,
                "schedule": rule.schedule,
                "actions": rule.actions,
                "created_at": rule.created_at.isoformat(),
                "updated_at": rule.updated_at.isoformat()
            }
        }
    except HTTPException:
        raise
    except Exception as e:
        print(f"更新自动化规则失败: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail="更新自动化规则失败")
    finally:
        db.close()

@app.delete("/automation/rules/{rule_id}")
async def delete_automation_rule(rule_id: int):
    """删除自动化规则"""
    db: Session = SessionLocal()
    try:
        rule = db.query(AutomationRule).filter(AutomationRule.id == rule_id).first()
        if not rule:
            raise HTTPException(status_code=404, detail="规则不存在")
        
        db.delete(rule)
        db.commit()
        
        return {
            "success": True,
            "message": "自动化规则删除成功"
        }
    except HTTPException:
        raise
    except Exception as e:
        print(f"删除自动化规则失败: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail="删除自动化规则失败")
    finally:
        db.close()

@app.post("/automation/rules/{rule_id}/toggle")
async def toggle_automation_rule(rule_id: int):
    """切换自动化规则启用状态"""
    db: Session = SessionLocal()
    try:
        rule = db.query(AutomationRule).filter(AutomationRule.id == rule_id).first()
        if not rule:
            raise HTTPException(status_code=404, detail="规则不存在")
        
        rule.enabled = not rule.enabled
        rule.updated_at = datetime.utcnow()
        db.commit()
        db.refresh(rule)
        
        return {
            "success": True,
            "message": f"规则已{'启用' if rule.enabled else '禁用'}",
            "enabled": rule.enabled
        }
    except HTTPException:
        raise
    except Exception as e:
        print(f"切换规则状态失败: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail="切换规则状态失败")
    finally:
        db.close()



