import json
import threading
import os
from openai import OpenAI
from dotenv import load_dotenv
from typing import List, Optional, Dict
from pydantic import BaseModel
from datetime import datetime, timedelta
import requests
from models import SessionLocal, AutomationRule  # 添加自动化规则模型


# 加载环境变量
load_dotenv()

# 大模型配置
DASHSCOPE_API_KEY = os.getenv("DASHSCOPE_API_KEY", "")
MODEL_NAME = os.getenv("MODEL_NAME", "qwen-turbo")

# 配置大模型服务
if DASHSCOPE_API_KEY:
    print(f"✅ 大模型服务已初始化: {MODEL_NAME}")
else:
    print("⚠️ 未设置DASHSCOPE_API_KEY")

# 初始化OpenAI客户端，用于调用阿里云大模型
llm_client = OpenAI(
    api_key=DASHSCOPE_API_KEY,
    base_url="https://dashscope.aliyuncs.com/compatible-mode/v1",
) if DASHSCOPE_API_KEY else None

# 全局变量引用（由main.py设置）
latest_data_container = None
latest_data_lock = None
mqtt_client = None
CONTROL_TOPIC = ""

def set_global_references(data_container_ref, lock_ref, mqtt_ref, control_topic):
    """设置全局变量引用"""
    global latest_data_container, latest_data_lock, mqtt_client, CONTROL_TOPIC
    latest_data_container = data_container_ref
    latest_data_lock = lock_ref
    mqtt_client = mqtt_ref
    CONTROL_TOPIC = control_topic

def get_available_dates():
    """获取所有可用的历史数据日期"""
    try:
        # 调用本地API获取可用日期
        response = requests.get("http://localhost:8000/available-dates")
        if response.status_code == 200:
            data = response.json()
            dates = data.get("dates", [])
            
            # 只返回有数据的日期
            available_dates = [d["date"] for d in dates if d["has_data"]]
            
            if available_dates:
                return {
                    "success": True,
                    "available_dates": available_dates,
                    "earliest_date": available_dates[0],
                    "latest_date": available_dates[-1],
                    "total_days": len(available_dates),
                    "message": f"找到{len(available_dates)}天的历史数据，日期范围：{available_dates[0]} 到 {available_dates[-1]}"
                }
            else:
                return {
                    "success": False,
                    "error": "没有找到任何历史数据",
                    "available_dates": []
                }
        else:
            return {"success": False, "error": f"API调用失败，状态码：{response.status_code}"}
    except Exception as e:
        return {"success": False, "error": f"查询可用日期时发生错误: {str(e)}"}

def get_energy_report_data(start_date: str, end_date: str):
    """获取指定日期范围内的节能报告数据，包括所有传感器数据和能耗数据"""
    try:
        # 调用本地API获取各种数据
        base_url = "http://localhost:8000/query-history"
        
        # 准备查询所有类型的数据
        data_types = ["temperature", "humidity", "light", "occupancy", "power"]
        report_data = {
            "date_range": {"start_date": start_date, "end_date": end_date},
            "summary": {},
            "daily_data": {}
        }
        
        for data_type in data_types:
            # 按天查询数据
            payload = {
                "start_date": start_date,
                "end_date": end_date,
                "data_type": data_type,
                "unit": "day"
            }
            
            response = requests.post(base_url, json=payload)
            if response.status_code == 200:
                result = response.json()
                data_points = result.get("data", [])
                
                if data_points:
                    # 计算统计信息
                    values = [point["value"] for point in data_points if point["value"] is not None]
                    if values:
                        if data_type == "power":
                            # 功耗数据特殊处理
                            total_energy = sum(values)
                            avg_daily_energy = total_energy / len(values)
                            max_daily = max(values)
                            min_daily = min(values)
                            report_data["summary"][data_type] = {
                                "total_consumption": round(total_energy, 2),
                                "average_daily": round(avg_daily_energy, 2),
                                "max_daily": round(max_daily, 2),
                                "min_daily": round(min_daily, 2),
                                "unit": "kWh",
                                "days": len(values)
                            }
                        elif data_type == "occupancy":
                            # 占用率数据特殊处理
                            occupied_days = sum(1 for v in values if v)
                            occupancy_rate = (occupied_days / len(values)) * 100
                            report_data["summary"][data_type] = {
                                "occupancy_rate": round(occupancy_rate, 1),
                                "occupied_days": occupied_days,
                                "total_days": len(values),
                                "unit": "%"
                            }
                        else:
                            # 温度、湿度、光照数据
                            report_data["summary"][data_type] = {
                                "average": round(sum(values) / len(values), 1),
                                "minimum": round(min(values), 1),
                                "maximum": round(max(values), 1),
                                "unit": "°C" if data_type == "temperature" else ("%" if data_type == "humidity" else "lux")
                            }
                        
                        # 保存每日数据
                        report_data["daily_data"][data_type] = data_points
        
        return {
            "success": True,
            "report_data": report_data,
            "message": f"成功获取{start_date}至{end_date}期间的节能报告数据"
        }
        
    except requests.exceptions.RequestException as e:
        return {"success": False, "error": f"网络请求错误: {str(e)}"}
    except Exception as e:
        return {"success": False, "error": f"获取节能报告数据时发生错误: {str(e)}"}

def get_current_status():
    """获取当前所有传感器和设备状态"""
    with latest_data_lock:
        if not latest_data_container or not latest_data_container.data:
            return {"error": "没有可用的设备数据"}
        
        latest_data = latest_data_container.data
        sensor_data = latest_data.get("sensor_data", {})
        state = latest_data.get("state", {})
        timestamp = latest_data.get("timestamp")
        
        # 格式化传感器数据
        sensor_info = {
            "温度": f"{sensor_data.get('temp', '未知')}°C",
            "湿度": f"{sensor_data.get('humidity', '未知')}%",
            "光照": f"{sensor_data.get('lux', '未知')}lux",
            "人员检测": "有人" if sensor_data.get('person') == "true" else "无人",
            "当前功耗": f"{latest_data.get('power', 0)/1000:.2f}kW"
        }
        
        # 格式化设备状态
        led_state = state.get("led", {})
        ac_state = state.get("air_conditioner", {})
        
        # LED灯状态描述
        led_on_count = sum(1 for v in led_state.values() if v == 1)
        led_desc = f"{led_on_count}/4个灯已开启" if led_on_count > 0 else "全部关闭"
        
        # 空调状态描述  
        if ac_state.get("state") == "on":
            mode = "制冷" if ac_state.get("mode") == "cool" else "制热"
            ac_desc = f"已开启 - {mode}模式 {ac_state.get('level', 1)}档"
        else:
            ac_desc = "关闭"
        
        device_info = {
            "LED灯": led_desc,
            "空调": ac_desc,
            "多媒体设备": state.get("multimedia", "未知")
        }
        
        return {
            "传感器状态": sensor_info,
            "设备状态": device_info,
            "最后更新时间": timestamp.isoformat() if timestamp else "",
            "当前日期": timestamp.strftime("%Y-%m-%d") if timestamp else "",
            "当前时间": timestamp.strftime("%H:%M") if timestamp else ""
        }

def get_current_date_info():
    """获取当前设备的最新日期信息，用于生成动态system prompt"""
    try:
        with latest_data_lock:
            if latest_data_container and latest_data_container.data and "timestamp" in latest_data_container.data:
                timestamp = latest_data_container.data["timestamp"]
                current_date = timestamp.strftime("%Y-%m-%d")
                current_time = timestamp.strftime("%H:%M")
                return {
                    "current_date": current_date,
                    "current_time": current_time,
                    "has_data": True
                }
            else:
                return {
                    "current_date": "无数据",
                    "current_time": "无数据", 
                    "has_data": False
                }
    except Exception as e:
        return {
            "current_date": "获取失败",
            "current_time": "获取失败",
            "has_data": False,
            "error": str(e)
        }

def control_device_by_command(device_type: str, action: str, **kwargs):
    """根据语义命令控制设备 - 发送完整的设备状态"""
    try:
        # 在锁内完成所有操作，确保数据一致性
        with latest_data_lock:
            # 调试信息：显示latest_data的内容
            print(f"🔍 调试信息:")
            print(f"   latest_data_container是否为None: {latest_data_container is None}")
            if latest_data_container:
                print(f"   latest_data_container.data是否为None: {latest_data_container.data is None}")
                print(f"   latest_data_container.data类型: {type(latest_data_container.data)}")
                if latest_data_container.data:
                    print(f"   latest_data的键: {list(latest_data_container.data.keys())}")
                    print(f"   是否包含state: {'state' in latest_data_container.data}")
                    if 'state' in latest_data_container.data:
                        print(f"   state内容: {latest_data_container.data['state']}")
            
            # 获取最新的设备状态作为基础
            if latest_data_container and latest_data_container.data and "state" in latest_data_container.data:
                current_state = latest_data_container.data["state"].copy()
                print(f"✅ 获取到最新状态: LED={current_state.get('led', {})}, 空调={current_state.get('air_conditioner', {})}, 多媒体={current_state.get('multimedia', 'off')}")
            else:
                # 默认状态：所有设备关闭
                current_state = {
                    "led": {"led1": 0, "led2": 0, "led3": 0, "led4": 0},
                    "air_conditioner": {"state": "off", "mode": "cool", "level": 1},
                    "multimedia": "off"
                }
                print("⚠️ 未找到最新状态，使用默认状态")
            
            # 确保所有设备字段都存在并且是正确的数据结构
            if "led" not in current_state or not isinstance(current_state["led"], dict):
                current_state["led"] = {"led1": 0, "led2": 0, "led3": 0, "led4": 0}
            if "air_conditioner" not in current_state or not isinstance(current_state["air_conditioner"], dict):
                current_state["air_conditioner"] = {"state": "off", "mode": "cool", "level": 1}
            if "multimedia" not in current_state:
                current_state["multimedia"] = "off"
            
            # 确保LED子字段都存在
            for led_num in [1, 2, 3, 4]:
                led_key = f"led{led_num}"
                if led_key not in current_state["led"]:
                    current_state["led"][led_key] = 0
            
            # 确保空调子字段都存在
            if "state" not in current_state["air_conditioner"]:
                current_state["air_conditioner"]["state"] = "off"
            if "mode" not in current_state["air_conditioner"]:
                current_state["air_conditioner"]["mode"] = "cool"
            if "level" not in current_state["air_conditioner"]:
                current_state["air_conditioner"]["level"] = 1
        
            # 打印当前状态用于调试
            print(f"🔍 修改前状态: {json.dumps(current_state, ensure_ascii=False, indent=2)}")
            
            action_description = ""
            
            # 根据设备类型和操作修改状态（在锁内完成所有修改）
            if device_type == "led" or device_type == "灯":
                led_numbers = kwargs.get("led_numbers", [1, 2, 3, 4])  # 默认所有灯
                
                if action == "on" or action == "开" or action == "打开":
                    for i in led_numbers:
                        current_state["led"][f"led{i}"] = 1
                    action_description = f"已打开LED灯: {led_numbers}"
                elif action == "off" or action == "关" or action == "关闭":
                    for i in led_numbers:
                        current_state["led"][f"led{i}"] = 0
                    action_description = f"已关闭LED灯: {led_numbers}"
                    
            elif device_type == "air_conditioner" or device_type == "空调":
                if action == "on" or action == "开" or action == "打开":
                    current_state["air_conditioner"]["state"] = "on"
                    if "mode" in kwargs:
                        # 将中文模式转换为英文
                        mode_input = kwargs["mode"]
                        if mode_input in ["制冷", "cool"]:
                            current_state["air_conditioner"]["mode"] = "cool"
                        elif mode_input in ["制热", "heat"]:
                            current_state["air_conditioner"]["mode"] = "heat"
                        else:
                            current_state["air_conditioner"]["mode"] = mode_input  # 保持原值
                    if "level" in kwargs:
                        current_state["air_conditioner"]["level"] = kwargs["level"]
                    # 正确显示模式描述
                    mode_str = "制冷" if current_state["air_conditioner"]["mode"] == "cool" else "制热"
                    action_description = f"已开启空调 - {mode_str}模式 {current_state['air_conditioner']['level']}档"
                    
                elif action == "off" or action == "关" or action == "关闭":
                    current_state["air_conditioner"]["state"] = "off"
                    action_description = "已关闭空调"
                    
                elif action == "level_up" or action == "调高":
                    if current_state["air_conditioner"]["state"] == "off":
                        current_state["air_conditioner"]["state"] = "on"
                    current_level = current_state["air_conditioner"].get("level", 1)
                    new_level = min(3, current_level + 1)
                    current_state["air_conditioner"]["level"] = new_level
                    action_description = f"空调档位调至{new_level}档"
                    
                elif action == "level_down" or action == "调低":
                    current_level = current_state["air_conditioner"].get("level", 1)
                    new_level = max(1, current_level - 1)
                    current_state["air_conditioner"]["level"] = new_level
                    if new_level == 1 and kwargs.get("turn_off_if_min", False):
                        current_state["air_conditioner"]["state"] = "off"
                        action_description = "空调档位已调至最低，已关闭空调"
                    else:
                        action_description = f"空调档位调至{new_level}档"
                        
            elif device_type == "multimedia" or device_type == "多媒体":
                if action == "on" or action == "开" or action == "打开":
                    current_state["multimedia"] = "on"
                    action_description = "已开启多媒体设备"
                elif action == "off" or action == "关" or action == "关闭":
                    current_state["multimedia"] = "off"
                    action_description = "已关闭多媒体设备"
                elif action == "standby" or action == "待机":
                    current_state["multimedia"] = "standby"
                    action_description = "多媒体设备已切换到待机模式"
                    
            elif device_type == "all" or device_type == "所有设备":
                if action == "on" or action == "开" or action == "打开":
                    # 开启所有设备
                    current_state["led"] = {"led1": 1, "led2": 1, "led3": 1, "led4": 1}
                    current_state["air_conditioner"]["state"] = "on"
                    current_state["multimedia"] = "on"
                    action_description = "已开启所有设备"
                elif action == "off" or action == "关" or action == "关闭":
                    # 关闭所有设备
                    current_state["led"] = {"led1": 0, "led2": 0, "led3": 0, "led4": 0}
                    current_state["air_conditioner"]["state"] = "off"
                    current_state["multimedia"] = "off"
                    action_description = "已关闭所有设备"
            
            # 打印修改后状态用于调试
            print(f"🔍 修改后状态: {json.dumps(current_state, ensure_ascii=False, indent=2)}")
        
        # 锁外发送MQTT命令
        if action_description:
            if mqtt_client:
                control_message = {"state": current_state}
                control_json = json.dumps(control_message, ensure_ascii=False, indent=2)
                
                # 详细的调试打印
                print(f"\n🤖 AI设备控制调试信息:")
                print(f"   用户操作: {device_type} -> {action}")
                print(f"   附加参数: {kwargs}")
                print(f"   操作描述: {action_description}")
                print(f"   发送到MQTT主题: {CONTROL_TOPIC}")
                print(f"   完整JSON内容:")
                print(f"   {control_json}")
                print(f"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n")
                
                mqtt_client.publish(CONTROL_TOPIC, json.dumps(control_message))
                return {
                    "success": True, 
                    "message": action_description, 
                    "command": current_state,
                    "action_type": f"{device_type}_{action}",
                    "sent_json": control_message  # 返回发送的JSON用于前端调试
                }
            else:
                print(f"❌ MQTT客户端未连接，无法发送控制命令")
                return {"error": "MQTT客户端未连接"}
        else:
            return {"error": f"不支持的设备类型({device_type})或操作({action})"}
            
    except Exception as e:
        print(f"❌ 控制设备错误: {e}")
        return {"error": f"控制设备时发生错误: {str(e)}"}

def create_automation_rule(name: str, description: str, schedule_type: str, time: str, days: list, actions: dict):
    """创建自动化规则"""
    try:
        # 检查规则名称是否已存在
        db = SessionLocal()
        existing_rule = db.query(AutomationRule).filter(AutomationRule.name == name).first()
        if existing_rule:
            db.close()
            return {
                "success": False,
                "error": f"规则名称 '{name}' 已存在，请使用其他名称"
            }
        
        # 构建调度配置
        schedule_config = {
            "type": schedule_type,  # "daily" 或 "weekly"
            "time": time,  # 格式 "HH:MM"
        }
        
        if schedule_type == "weekly":
            schedule_config["days"] = days  # 星期几的列表 [1,2,3,4,5] 表示周一到周五
        
        # 创建新规则
        new_rule = AutomationRule(
            name=name,
            description=description,
            enabled=True,
            schedule=schedule_config,
            actions=actions
        )
        
        db.add(new_rule)
        db.commit()
        db.refresh(new_rule)
        db.close()
        
        return {
            "success": True,
            "rule_id": new_rule.id,
            "message": f"成功创建自动化规则 '{name}'"
        }
        
    except Exception as e:
        print(f"❌ 创建自动化规则错误: {e}")
        return {
            "success": False,
            "error": f"创建自动化规则时发生错误: {str(e)}"
        }

def parse_natural_language_schedule(schedule_text: str):
    """解析自然语言的时间调度描述，返回标准化的调度配置"""
    import re
    
    schedule_text = schedule_text.lower()
    
    # 提取时间（HH:MM格式）
    time_pattern = r'(\d{1,2}):(\d{2})'
    time_match = re.search(time_pattern, schedule_text)
    
    # 提取简单的小时表达（如"8点"、"18点"）
    hour_pattern = r'(\d{1,2})点'
    hour_match = re.search(hour_pattern, schedule_text)
    
    if time_match:
        hour, minute = time_match.groups()
        time_str = f"{int(hour):02d}:{minute}"
    elif hour_match:
        hour = hour_match.group(1)
        time_str = f"{int(hour):02d}:00"
    else:
        return None  # 无法解析时间
    
    # 判断是每日还是每周
    if any(word in schedule_text for word in ["每天", "每日", "天天"]):
        return {
            "type": "daily",
            "time": time_str,
            "description": f"每天{time_str}"
        }
    
    # 解析星期几
    weekday_map = {
        "周一": 1, "星期一": 1, "礼拜一": 1,
        "周二": 2, "星期二": 2, "礼拜二": 2,
        "周三": 3, "星期三": 3, "礼拜三": 3,
        "周四": 4, "星期四": 4, "礼拜四": 4,
        "周五": 5, "星期五": 5, "礼拜五": 5,
        "周六": 6, "星期六": 6, "礼拜六": 6,
        "周日": 7, "星期日": 7, "礼拜日": 7, "周天": 7
    }
    
    days = []
    day_names = []
    for day_name, day_num in weekday_map.items():
        if day_name in schedule_text:
            days.append(day_num)
            day_names.append(day_name)
    
    if days:
        return {
            "type": "weekly",
            "time": time_str,
            "days": sorted(list(set(days))),  # 去重并排序
            "description": f"每周{'/'.join(day_names)} {time_str}"
        }
    
    # 检查工作日
    if any(word in schedule_text for word in ["工作日", "周一到周五", "星期一到星期五"]):
        return {
            "type": "weekly",
            "time": time_str,
            "days": [1, 2, 3, 4, 5],
            "description": f"工作日{time_str}"
        }
    
    # 检查周末
    if any(word in schedule_text for word in ["周末", "周六周日", "星期六星期日"]):
        return {
            "type": "weekly", 
            "time": time_str,
            "days": [6, 7],
            "description": f"周末{time_str}"
        }
    
    # 默认返回每日
    return {
        "type": "daily",
        "time": time_str,
        "description": f"每天{time_str}"
    }

def parse_natural_language_actions(action_text: str):
    """解析自然语言的动作描述，返回标准化的动作配置"""
    action_text = action_text.lower()
    
    actions = {
        "led": {"led1": 0, "led2": 0, "led3": 0, "led4": 0},
        "air_conditioner": {"state": "off", "mode": "cool", "level": 1},
        "multimedia": "off"
    }
    
    # 解析LED灯控制
    if any(word in action_text for word in ["开灯", "打开灯", "亮灯", "开所有灯", "打开所有灯"]):
        actions["led"] = {"led1": 1, "led2": 1, "led3": 1, "led4": 1}
    elif any(word in action_text for word in ["关灯", "关闭灯", "关所有灯", "关闭所有灯"]):
        actions["led"] = {"led1": 0, "led2": 0, "led3": 0, "led4": 0}
    
    # 解析空调控制
    if any(word in action_text for word in ["开空调", "打开空调", "启动空调"]):
        actions["air_conditioner"]["state"] = "on"
        # 检查模式
        if any(word in action_text for word in ["制热", "制暖", "热风", "暖风"]):
            actions["air_conditioner"]["mode"] = "heat"
        elif any(word in action_text for word in ["制冷", "冷风", "降温"]):
            actions["air_conditioner"]["mode"] = "cool"
        
        # 检查档位
        if "1档" in action_text or "一档" in action_text:
            actions["air_conditioner"]["level"] = 1
        elif "2档" in action_text or "二档" in action_text:
            actions["air_conditioner"]["level"] = 2
        elif "3档" in action_text or "三档" in action_text:
            actions["air_conditioner"]["level"] = 3
    elif any(word in action_text for word in ["关空调", "关闭空调", "空调关"]):
        actions["air_conditioner"]["state"] = "off"
    
    # 解析多媒体设备控制
    if any(word in action_text for word in ["开多媒体", "打开多媒体", "开投影", "启动多媒体"]):
        actions["multimedia"] = "on"
    elif any(word in action_text for word in ["关多媒体", "关闭多媒体", "多媒体关", "关投影"]):
        actions["multimedia"] = "off"
    elif any(word in action_text for word in ["多媒体待机", "投影待机"]):
        actions["multimedia"] = "standby"
    
    return actions

def create_automation_rule_standardized(name: str, description: str, schedule_type: str, time: str, 
                                       led1: int, led2: int, led3: int, led4: int,
                                       ac_state: str, ac_mode: str, ac_level: int, multimedia: str, **kwargs):
    """使用标准化参数创建自动化规则"""
    try:
        # 从kwargs中获取days参数
        days = kwargs.get('days', [])
        
        # 直接构建标准化的actions
        actions = {
            "led": {
                "led1": led1 if led1 != -1 else "no-change",
                "led2": led2 if led2 != -1 else "no-change", 
                "led3": led3 if led3 != -1 else "no-change",
                "led4": led4 if led4 != -1 else "no-change"
            },
            "air_conditioner": {
                "state": ac_state,
                "mode": ac_mode if ac_mode != "no-change" else "cool",
                "level": ac_level if ac_level != -1 else 1
            },
            "multimedia": multimedia
        }
        
        # 创建规则
        result = create_automation_rule(
            name=name,
            description=description,
            schedule_type=schedule_type,
            time=time,
            days=days or [],
            actions=actions
        )
        
        if result["success"]:
            # 构建成功消息
            if schedule_type == "daily":
                schedule_desc = f"每天{time}"
            else:
                day_names = ["", "周一", "周二", "周三", "周四", "周五", "周六", "周日"]
                day_list = [day_names[day] for day in days] if days else []
                schedule_desc = f"每周{'/'.join(day_list)} {time}"
            
            action_descriptions = []
            
            # 描述LED操作
            led_changes = []
            for i, led_val in enumerate([led1, led2, led3, led4], 1):
                if led_val == 1:
                    led_changes.append(f"LED{i}开启")
                elif led_val == 0:
                    led_changes.append(f"LED{i}关闭")
            
            if led_changes:
                if all(val == 1 for val in [led1, led2, led3, led4]):
                    action_descriptions.append("开启所有LED灯")
                elif all(val == 0 for val in [led1, led2, led3, led4]):
                    action_descriptions.append("关闭所有LED灯")
                else:
                    action_descriptions.extend(led_changes)
            
            # 描述空调操作
            if ac_state == "on":
                mode_desc = "制冷" if ac_mode == "cool" else "制热" if ac_mode == "heat" else "默认"
                level_desc = f"{ac_level}档" if ac_level != -1 else "默认档位"
                action_descriptions.append(f"开启空调{mode_desc}模式{level_desc}")
            elif ac_state == "off":
                action_descriptions.append("关闭空调")
            
            # 描述多媒体操作
            if multimedia == "on":
                action_descriptions.append("开启多媒体设备")
            elif multimedia == "off":
                action_descriptions.append("关闭多媒体设备")
            elif multimedia == "standby":
                action_descriptions.append("多媒体设备待机")
            
            action_desc = "、".join(action_descriptions) if action_descriptions else "无操作"
            
            result["detailed_message"] = f"✅ 自动化规则创建成功！\n规则名称：{name}\n执行时间：{schedule_desc}\n执行操作：{action_desc}"
        
        return result
        
    except Exception as e:
        print(f"❌ 创建自动化规则错误: {e}")
        return {
            "success": False,
            "error": f"创建自动化规则时发生错误: {str(e)}"
        }

# 大模型工具定义
tools = [
    {
        "type": "function",
        "function": {
            "name": "get_current_status",
            "description": "获取当前教室的所有传感器数据和设备状态，包括温度、湿度、光照、人员检测和设备工作状态，当任何的传感器返回值为-1，说明传感器故障",
            "parameters": {
                "type": "object",
                "properties": {},
                "required": []
            }
        }
    },
    {
        "type": "function", 
        "function": {
            "name": "control_device_by_command",
            "description": "根据用户的语义命令控制教室设备。教室有4个LED灯(led1-led4)、1个空调、1个多媒体设备。发送完整的设备状态给控制系统。",
            "parameters": {
                "type": "object",
                "properties": {
                    "device_type": {
                        "type": "string",
                        "description": "设备类型，可选值：led/灯、air_conditioner/空调、multimedia/多媒体、all/所有设备",
                        "enum": ["led", "灯", "air_conditioner", "空调", "multimedia", "多媒体", "all", "所有设备"]
                    },
                    "action": {
                        "type": "string", 
                        "description": "操作类型，可选值：on/开/打开、off/关/关闭、level_up/调高、level_down/调低、standby/待机",
                        "enum": ["on", "开", "打开", "off", "关", "关闭", "level_up", "调高", "level_down", "调低", "standby", "待机"]
                    },
                    "mode": {
                        "type": "string",
                        "description": "空调模式（仅空调使用），请使用英文：cool(制冷)、heat(制热)",
                        "enum": ["cool", "heat"]
                    },
                    "level": {
                        "type": "integer",
                        "description": "设备档位（1-3档，仅空调使用）",
                        "minimum": 1,
                        "maximum": 3
                    },
                    "led_numbers": {
                        "type": "array",
                        "description": "要控制的LED灯编号列表（1-4），默认控制所有灯",
                        "items": {"type": "integer", "minimum": 1, "maximum": 4}
                    }
                },
                "required": ["device_type", "action"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_available_dates",
            "description": "获取系统中所有可用的历史数据日期，用于节能报告生成或日期验证。当用户请求生成节能报告但未指定具体日期时，可以使用此工具获取可用的日期范围。",
            "parameters": {
                "type": "object",
                "properties": {},
                "required": []
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_energy_report_data",
            "description": "获取指定日期范围内的完整节能报告数据，包括温度、湿度、光照、人员占用率和能耗统计的原始数据和统计信息。返回纯数据，不包含建议。你需要根据返回的数据自己分析并生成节能建议和报告。",
            "parameters": {
                "type": "object",
                "properties": {
                    "start_date": {
                        "type": "string",
                        "description": "开始日期，格式：YYYY-MM-DD",
                        "pattern": "^\\d{4}-\\d{2}-\\d{2}$"
                    },
                    "end_date": {
                        "type": "string", 
                        "description": "结束日期，格式：YYYY-MM-DD",
                        "pattern": "^\\d{4}-\\d{2}-\\d{2}$"
                    }
                },
                "required": ["start_date", "end_date"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "create_automation_rule",
            "description": "创建自动化规则。请使用标准化的参数格式，不要使用自然语言描述。",
            "parameters": {
                "type": "object",
                "properties": {
                    "name": {
                        "type": "string",
                        "description": "规则名称，简洁明了"
                    },
                    "description": {
                        "type": "string", 
                        "description": "规则详细描述"
                    },
                    "schedule_type": {
                        "type": "string",
                        "description": "调度类型",
                        "enum": ["daily", "weekly"]
                    },
                    "time": {
                        "type": "string",
                        "description": "执行时间，格式为HH:MM，如'08:00'、'18:30'",
                        "pattern": "^([0-1][0-9]|2[0-3]):[0-5][0-9]$"
                    },
                    "days": {
                        "type": "array",
                        "description": "执行日期（仅weekly类型需要），1-7代表周一到周日。daily类型不需要此参数",
                        "items": {"type": "integer", "minimum": 1, "maximum": 7},
                        "default": []
                    },
                    "led1": {
                        "type": "integer",
                        "description": "LED1状态：1=开启，0=关闭，-1=不变",
                        "enum": [1, 0, -1]
                    },
                    "led2": {
                        "type": "integer", 
                        "description": "LED2状态：1=开启，0=关闭，-1=不变",
                        "enum": [1, 0, -1]
                    },
                    "led3": {
                        "type": "integer",
                        "description": "LED3状态：1=开启，0=关闭，-1=不变", 
                        "enum": [1, 0, -1]
                    },
                    "led4": {
                        "type": "integer",
                        "description": "LED4状态：1=开启，0=关闭，-1=不变",
                        "enum": [1, 0, -1]
                    },
                    "ac_state": {
                        "type": "string",
                        "description": "空调开关状态",
                        "enum": ["on", "off", "no-change"]
                    },
                    "ac_mode": {
                        "type": "string", 
                        "description": "空调模式（仅ac_state为on时有效）",
                        "enum": ["cool", "heat", "no-change"]
                    },
                    "ac_level": {
                        "type": "integer",
                        "description": "空调档位（仅ac_state为on时有效）：1-3档，-1=不变",
                        "enum": [1, 2, 3, -1]
                    },
                    "multimedia": {
                        "type": "string",
                        "description": "多媒体设备状态",
                        "enum": ["on", "off", "standby", "no-change"]
                    }
                },
                "required": ["name", "description", "schedule_type", "time", "led1", "led2", "led3", "led4", "ac_state", "ac_mode", "ac_level", "multimedia"]
            }
        }
    }
]

# 系统提示词
SYSTEM_PROMPT = """你是一个智能教室管理助手，负责帮助用户了解教室状态和控制教室设备。你要主动理解用户的隐含需求！

## 教室设备配置：
- LED灯：4个（led1, led2, led3, led4）
- 空调：1个（支持制冷/制热，1-3档）
- 多媒体设备：1个（支持开/关/待机）

## 你的能力：
1. 查询教室当前的传感器数据（温度、湿度、光照、人员检测）和设备状态
2. 控制教室设备：
   - LED灯：可单独或批量控制，支持"开灯"、"关灯"、"开第1个灯"等
   - 空调：开关、制冷/制热模式切换、档位调节(1-3档)
   - 多媒体设备：开关和待机模式
3. **🆕 生成节能报告**：
   - 查询可用的历史数据日期
   - 获取指定时间段的能耗和环境数据
   - 生成详细的节能分析报告和建议

## 重要：智能语义理解规则
你必须主动理解用户的隐含需求并采取行动！以下情况请立即调用相应工具：

### 温度相关（立即调用控制工具）：
- "有点热/太热了/感觉热" → 开启空调制冷模式或调高档位
- "有点冷/太冷了/感觉冷" → 开启空调制热模式或调高档位
- "闷热/很闷" → 开启空调制冷模式
- "空调太吵了" → 调低空调档位
- "不够凉快" → 调高空调档位

### 光照相关（立即调用控制工具）：
- "太亮了/亮瞎了/刺眼" → 关闭部分或全部LED灯
- "太暗了/看不清" → 打开LED灯
- "灯光刺眼" → 关闭部分LED灯
- "需要更亮一点" → 打开更多LED灯

### 环境查询（立即调用查询工具）：
- "现在怎么样/教室情况" → 查询所有状态
- "温度多少/热不热" → 查询温度数据
- "亮度如何/亮不亮" → 查询光照数据
- "有人吗/人多吗" → 查询人员检测

### 设备控制（立即调用控制工具）：
- "开所有电器/打开所有设备" → 打开所有LED灯、空调、多媒体设备
- "关所有电器/关闭所有设备" → 关闭所有设备
- "准备上课" → 打开灯和多媒体，调节空调
- "下课了" → 关闭所有设备或待机
- "休息一下" → 设备切换到节能模式
- "开始投影" → 打开多媒体设备

### 🆕 节能报告相关（立即调用报告工具）：
- "生成节能报告/能耗报告/用电报告" → 先获取可用日期，再获取报告数据
- "分析一下最近的能耗情况" → 获取最近几天的节能报告
- "这周/这个月用了多少电" → 根据时间范围生成报告
- "帮我看看能耗数据" → 获取并分析能耗数据
- "有什么节能建议" → 基于历史数据生成节能建议

### 🆕 自动化规则创建（立即调用自动化工具）：
- "每天8点开灯" → 创建每日8:00开启所有LED灯的规则
- "工作日18点关空调" → 创建工作日18:00关闭空调的规则
- "周一周三10点开空调制冷" → 创建每周一三10:00开启空调制冷的规则
- "添加一个自动化规则" → 引导用户提供规则详情并创建
- "帮我设置定时任务" → 理解用户需求并创建相应的自动化规则
- "每天下班后关闭所有设备" → 创建每日关闭所有设备的规则

## 🎯 自动化规则创建标准格式（重要！）
当调用create_automation_rule工具时，必须使用以下标准化格式，不要使用自然语言：

### 时间格式标准：
- schedule_type: 只能是 "daily" 或 "weekly"
- time: 必须是 "HH:MM" 格式，如 "08:00"、"18:30"
- days: 仅weekly需要，使用数字1-7（周一到周日），如[1,2,3,4,5]表示工作日

### 设备控制标准：
- LED灯状态: led1/led2/led3/led4 使用 1(开启)/0(关闭)/-1(不变)
- 空调控制: 
  - ac_state: "on"/"off"/"no-change"
  - ac_mode: "cool"/"heat"/"no-change" 
  - ac_level: 1/2/3/-1(不变)
- 多媒体: multimedia 使用 "on"/"off"/"standby"/"no-change"

### 标准化示例：
用户："每天8点开灯" → 
```
{
  "name": "每日开灯",
  "description": "每天8点自动开启所有LED灯",
  "schedule_type": "daily", 
  "time": "08:00",
  "led1": 1, "led2": 1, "led3": 1, "led4": 1,
  "ac_state": "no-change", "ac_mode": "no-change", "ac_level": -1,
  "multimedia": "no-change"
}
```

用户："工作日18点关空调" →
```
{
  "name": "工作日关空调",
  "description": "工作日18点自动关闭空调",
  "schedule_type": "weekly",
  "time": "18:00", 
  "days": [1,2,3,4,5],
  "led1": -1, "led2": -1, "led3": -1, "led4": -1,
  "ac_state": "off", "ac_mode": "no-change", "ac_level": -1,
  "multimedia": "no-change"
}
```

## 节能报告生成流程：
1. 当用户请求节能报告时，如果没有指定日期：
   - 首先调用get_available_dates()获取可用日期
   - 选择最近7天或用户意图的时间范围
2. 如果用户指定了日期范围：
   - 可以先调用get_available_dates()验证日期是否可用
   - 然后调用get_energy_report_data()获取详细数据
3. 基于获取的数据生成包含以下内容的报告：
   - 时间范围和数据概况
   - 能耗统计（总消耗、日均消耗、最高/最低日消耗）
   - 环境数据分析（温度、湿度、光照的平均/最高/最低值）
   - 使用率分析（占用天数、占用率）
   - 你需要根据数据智能生成节能建议和优化方案
   - 趋势分析和异常发现

## 自动化规则创建流程：
1. 当用户提到创建自动化规则、定时任务或类似需求时：
   - 智能解析用户的自然语言描述
   - 从用户的描述中提取时间调度和操作内容
   - 自动生成合适的规则名称和描述
2. 支持的时间格式：
   - 每天：每天、每日、天天 + 时间（如"每天8点"、"每日18:30"）
   - 每周：具体星期几 + 时间（如"周一10点"、"星期二15:30"）
   - 工作日：工作日 + 时间（如"工作日8点"）
   - 周末：周末 + 时间（如"周末10点"）
3. 支持的操作：
   - LED灯：开灯、关灯、开所有灯、关所有灯
   - 空调：开空调（默认制冷1档）、关空调、开空调制热/制冷 + 档位
   - 多媒体：开多媒体、关多媒体、多媒体待机
4. 当用户描述不够具体时，引导用户补充信息：
   - 缺少具体时间：询问具体执行时间
   - 缺少操作：询问要执行什么操作，例如用户说打开空调，需要询问用户几档，或者制冷还是制热
   - 缺少规则名称：根据描述自动生成合适的名称
5. 当用户的描述看起来像两条规则的时候，先后调用两次工具来创建：例如用户说每天8点开灯，下午5点关灯，你需要先调用一次工具来创建每天8点开灯的规则，然后调用一次工具来创建下午5点关灯的规则



## 控制逻辑：
- 控制时会发送完整的设备状态，包含所有设备的当前状态
- 只修改用户要求改变的设备，其他设备保持当前状态
- 空调模式参数请使用英文：cool(制冷)、heat(制热)
- 当用户发出模糊的命令时，例如：现在要上课了，需要你提前打开所有设备，你需要先调用工具查看当前的传感器数据，然后决定打开设备的设置，例如温度很低时打开空调制热，适宜时不开空调

## 交互原则：
- 用中文回复，语气亲切自然，像个贴心的助手
- 遇到隐含需求时，主动执行操作
- 执行操作后简洁地说明做了什么和为什么
- 对于与教室管理无关的问题，生气地说明只能帮助管理教室设备
- 生成节能报告时可以使用Markdown格式，除此之外不要使用Markdown格式
- 生成节能报告时要详细、专业，包含数据分析和实用建议，使用Markdown格式让报告更清晰易读
- 除了生成节能报告以外，其他回答尽量简短，只需要说明做了什么和为什么，不要Markdown格式

## 多步骤操作能力：
你可以在一次对话中连续调用多个工具来完成复杂任务。例如：
- "教室有人吗，有人就开所有灯，没人关掉所有电器"
  1. 先调用get_current_status()查询人员状态
  2. 根据人员检测结果调用control_device_by_command()执行相应操作
- "生成这个月的节能报告"
  1. 先调用get_available_dates()获取可用日期
  2. 确定合适的日期范围
  3. 调用get_energy_report_data()获取数据
  4. 分析数据并生成详细报告
- "每天上班时间开灯，下班关灯"
  1. 理解用户需要创建两个规则
  2. 引导用户确认具体时间（如8点上班，18点下班）
  3. 分别调用create_automation_rule创建上班开灯和下班关灯规则

## 操作验证流程：
1. 根据需要连续调用工具完成任务
2. 向用户报告完整的操作过程和结果

## 重要
当用户的要求不具体时，尤其是对于空调的操作，只说了打开的时候，一定要询问模式和档位！

记住：你是一个主动、智能的助手，可以连续使用多个工具来完成复杂的任务！现在你还具备了强大的节能分析能力和自动化规则创建能力！"""

class ChatMessage(BaseModel):
    message: str
    history: Optional[List[Dict]] = []  # 前端传递的对话历史

class ChatResponse(BaseModel):
    reply: str
    action_taken: Optional[Dict] = None

def process_chat_sync(message_content: str, history: List[Dict]):
    """同步处理聊天的函数，在线程池中运行"""
    try:
        print(f"💬 开始处理用户消息: {message_content}")
        
        # 获取当前日期信息用于动态system prompt
        date_info = get_current_date_info()
        
        # 动态生成包含当前日期信息的system prompt
        dynamic_system_prompt = SYSTEM_PROMPT
        if date_info["has_data"]:
            current_date_info = f"""

## 📅 当前系统信息：
- **当前设备最新数据日期**: {date_info["current_date"]}
- **最新数据时间**: {date_info["current_time"]}

当用户提及"今天"、"现在"、"最新"等时间概念时，请参考上述日期信息。"""
            dynamic_system_prompt = SYSTEM_PROMPT + current_date_info
        
        # 构建对话消息（包含历史）
        messages = [{"role": "system", "content": dynamic_system_prompt}]
        
        # 添加前端传来的历史对话
        messages.extend(history or [])
        
        # 添加当前用户消息
        messages.append({"role": "user", "content": message_content})
        
        print(f"📚 使用 {len(history or [])} 条历史消息进行对话")
        
        # 调用大模型
        response = llm_client.chat.completions.create(
            model=MODEL_NAME,
            messages=messages,
            tools=tools,
            tool_choice="auto",
            temperature=0.7
        )
        
        assistant_message = response.choices[0].message
        action_taken = None
        
        # 支持多轮工具调用
        max_tool_rounds = 3  # 最多3轮工具调用
        current_round = 0
        
        tool_calls = getattr(assistant_message, 'tool_calls', None)
        while tool_calls and current_round < max_tool_rounds:
            current_round += 1
            print(f"🔧 第{current_round}轮：AI需要调用{len(tool_calls)}个工具")
            
            # 执行工具调用
            for tool_call in tool_calls:
                function_name = tool_call.function.name
                function_args = json.loads(tool_call.function.arguments)
                
                print(f"🔧 调用工具: {function_name}, 参数: {function_args}")
                
                # 执行相应的工具函数
                if function_name == "get_current_status":
                    tool_result = get_current_status()
                elif function_name == "control_device_by_command":
                    tool_result = control_device_by_command(**function_args)
                    action_taken = tool_result
                elif function_name == "get_available_dates":
                    tool_result = get_available_dates()
                elif function_name == "get_energy_report_data":
                    tool_result = get_energy_report_data(**function_args)
                elif function_name == "create_automation_rule":
                    tool_result = create_automation_rule_standardized(**function_args)
                    action_taken = tool_result
                else:
                    tool_result = {"error": f"未知的工具函数: {function_name}"}
                
                print(f"🔧 工具执行结果: {tool_result}")
                
                # 将工具调用结果添加到对话中
                messages.append({
                    "role": "assistant", 
                    "content": assistant_message.content or "",
                    "tool_calls": tool_calls
                })
                messages.append({
                    "role": "tool",
                    "tool_call_id": tool_call.id,
                    "content": json.dumps(tool_result, ensure_ascii=False)
                })
            
            # 再次调用大模型，可能会继续调用工具
            print(f"🔄 第{current_round}轮完成，重新调用AI")
            response = llm_client.chat.completions.create(
                model=MODEL_NAME,
                messages=messages,
                tools=tools,
                tool_choice="auto",
                temperature=0.7
            )
            
            assistant_message = response.choices[0].message
            tool_calls = getattr(assistant_message, 'tool_calls', None)
        
        # 最终回复
        final_reply = assistant_message.content or "操作已完成"
        
        # 如果大模型没有给出有用的回复，提供默认回复
        if not final_reply or final_reply.strip() == "":
            final_reply = "抱歉，我不太明白您的意思。我只能帮您管理教室设备，比如查询温度湿度、控制灯光空调等。您可以试试说'现在温度怎么样'或'帮我开灯'之类的。"
        
        print(f"✅ AI处理完成，回复: {final_reply[:50]}...")
        
        return {
            "reply": final_reply,
            "action_taken": action_taken
        }
        
    except Exception as e:
        print(f"❌ AI对话处理失败: {e}")
        return {
            "reply": "哎呀，我现在有点迷糊，暂时无法回应您。不过您可以直接通过界面上的控制面板来管理设备哦！",
            "action_taken": None
        }

 