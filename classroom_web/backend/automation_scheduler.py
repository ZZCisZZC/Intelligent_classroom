"""
自动化任务调度器
"""
import asyncio
import json
import threading
import time
from datetime import datetime, timedelta
from typing import Dict, List, Optional

from sqlalchemy.orm import Session
from models import SessionLocal, AutomationRule


class AutomationScheduler:
    def __init__(self, mqtt_client=None, control_topic=None):
        self.mqtt_client = mqtt_client
        self.control_topic = control_topic
        self.running = False
        self.thread = None
        self.last_device_time = None  # 记录上次收到的设备时间
        self.last_check_time = None   # 记录上次检查的时间
        self.executed_rules = {}      # 记录已执行的规则 {rule_id: last_execution_time}
        self.latest_data_container = None  # 当前设备状态容器
        self.latest_data_lock = None       # 数据锁
        
    def start(self):
        """启动调度器"""
        if not self.running:
            self.running = True
            self.thread = threading.Thread(target=self._run_scheduler, daemon=True)
            self.thread.start()
            print("自动化调度器已启动")
    
    def stop(self):
        """停止调度器"""
        self.running = False
        if self.thread:
            self.thread.join()
        print("自动化调度器已停止")
    
    def set_mqtt_client(self, mqtt_client, control_topic):
        """设置MQTT客户端"""
        self.mqtt_client = mqtt_client
        self.control_topic = control_topic
    
    def set_data_references(self, latest_data_container, latest_data_lock):
        """设置数据容器引用"""
        self.latest_data_container = latest_data_container
        self.latest_data_lock = latest_data_lock
    
    def update_device_time(self, device_time: datetime):
        """更新设备时间"""
        self.last_device_time = device_time
    
    def _run_scheduler(self):
        """调度器主循环"""
        while self.running:
            try:
                # 如果有设备时间，使用设备时间进行检查
                if self.last_device_time:
                    self._check_and_execute_rules_by_device_time(self.last_device_time)
                
                # 每1秒检查一次
                time.sleep(1)
                
            except Exception as e:
                print(f"调度器执行错误: {e}")
                time.sleep(5)  # 发生错误时等待5秒再继续
    
    def _check_and_execute_rules_by_device_time(self, device_time: datetime):
        """根据设备时间检查并执行规则"""
        db: Session = SessionLocal()
        try:
            # 获取所有启用的规则
            rules = db.query(AutomationRule).filter(AutomationRule.enabled == True).all()
            
            # 如果是第一次收到设备时间，直接检查当前时间点
            if self.last_check_time is None:
                self.last_check_time = device_time
                for rule in rules:
                    if self._should_execute_rule(rule, device_time) and not self._is_recently_executed(rule, device_time):
                        print(f"执行自动化规则 (初始检查): {rule.name}")
                        self._execute_rule(rule)
                        self._mark_rule_executed(rule, device_time)
                return
            
            # 检查从上次检查时间到当前设备时间之间是否有错过的规则
            time_gap_minutes = int((device_time - self.last_check_time).total_seconds() / 60)
            
            if time_gap_minutes > 0:
                print(f"检测到时间跳跃: {self.last_check_time.strftime('%H:%M')} -> {device_time.strftime('%H:%M')} (跨越{time_gap_minutes}分钟)")
                
                # 检查这个时间段内应该执行的规则
                for minute_offset in range(time_gap_minutes + 1):
                    check_time = self.last_check_time + timedelta(minutes=minute_offset)
                    
                    for rule in rules:
                        if self._should_execute_rule(rule, check_time) and not self._is_recently_executed(rule, check_time):
                            print(f"执行错过的自动化规则: {rule.name} (应在 {check_time.strftime('%H:%M')} 执行)")
                            self._execute_rule(rule)
                            self._mark_rule_executed(rule, check_time)
            
            # 更新最后检查时间
            self.last_check_time = device_time
                    
        except Exception as e:
            print(f"检查自动化规则时出错: {e}")
        finally:
            db.close()
    
    def _check_and_execute_rules(self, current_time: datetime):
        """检查并执行符合条件的规则（保留原方法作为备用）"""
        db: Session = SessionLocal()
        try:
            # 获取所有启用的规则
            rules = db.query(AutomationRule).filter(AutomationRule.enabled == True).all()
            
            for rule in rules:
                if self._should_execute_rule(rule, current_time):
                    print(f"🎯 执行自动化规则: {rule.name}")
                    self._execute_rule(rule)
                    
        except Exception as e:
            print(f"检查自动化规则时出错: {e}")
        finally:
            db.close()
    
    def _should_execute_rule(self, rule: AutomationRule, current_time: datetime) -> bool:
        """判断规则是否应该在当前时间执行"""
        try:
            schedule = rule.schedule
            schedule_type = schedule.get("type", "daily")
            schedule_time = schedule.get("time", "00:00")  # HH:MM格式
            
            # 解析执行时间
            try:
                time_parts = schedule_time.split(":")
                target_hour = int(time_parts[0])
                target_minute = int(time_parts[1])
            except (ValueError, IndexError):
                print(f"规则 {rule.name} 的时间格式无效: {schedule_time}")
                return False
            
            # 检查当前时间是否匹配
            if current_time.hour != target_hour or current_time.minute != target_minute:
                return False
            
            if schedule_type == "daily":
                # 每日执行
                return True
            elif schedule_type == "weekly":
                # 周期执行，检查星期几
                target_days = schedule.get("days", [])  # 1=周一, 7=周日
                current_weekday = current_time.weekday() + 1  # Python的weekday: 0=周一，转换为1=周一
                return current_weekday in target_days
            else:
                print(f"规则 {rule.name} 的调度类型未知: {schedule_type}")
                return False
                
        except Exception as e:
            print(f" 判断规则执行条件时出错: {e}")
            return False
    
    def _execute_rule(self, rule: AutomationRule):
        """执行自动化规则"""
        try:
            actions = rule.actions
            
            # 获取当前设备状态并合并规则变更
            complete_state = self._build_complete_control_state(actions)
            
            if complete_state and self.mqtt_client and self.control_topic:
                # 发送MQTT控制命令
                control_message = {
                    "state": complete_state,
                    "source": "automation",
                    "rule_id": rule.id,
                    "rule_name": rule.name,
                    "timestamp": datetime.now().isoformat()
                }
                
                message_json = json.dumps(control_message)
                self.mqtt_client.publish(self.control_topic, message_json)
                print(f"发送自动化控制命令: {rule.name} -> {complete_state}")
            else:
                print(f"无法执行规则 {rule.name}: MQTT客户端未设置或状态为空")
                
        except Exception as e:
            print(f" 执行规则时出错: {e}")
    
    def _get_current_device_state(self) -> Dict:
        """获取当前设备状态"""
        if not self.latest_data_container or not self.latest_data_lock:
            print("数据容器未设置，使用默认状态")
            return {
                "led": {"led1": 0, "led2": 0, "led3": 0, "led4": 0},
                "air_conditioner": {"state": "off", "mode": "cool", "level": 1},
                "multimedia": "off"
            }
        
        try:
            with self.latest_data_lock:
                if self.latest_data_container.data and "state" in self.latest_data_container.data:
                    current_state = self.latest_data_container.data["state"]
                    # 确保状态完整性
                    complete_state = {
                        "led": current_state.get("led", {"led1": 0, "led2": 0, "led3": 0, "led4": 0}),
                        "air_conditioner": current_state.get("air_conditioner", {"state": "off", "mode": "cool", "level": 1}),
                        "multimedia": current_state.get("multimedia", "off")
                    }
                    return complete_state
                else:
                    print("当前状态数据为空，使用默认状态")
                    return {
                        "led": {"led1": 0, "led2": 0, "led3": 0, "led4": 0},
                        "air_conditioner": {"state": "off", "mode": "cool", "level": 1},
                        "multimedia": "off"
                    }
        except Exception as e:
            print(f"获取当前状态时出错: {e}")
            return {
                "led": {"led1": 0, "led2": 0, "led3": 0, "led4": 0},
                "air_conditioner": {"state": "off", "mode": "cool", "level": 1},
                "multimedia": "off"
            }

    def _build_complete_control_state(self, actions: Dict) -> Dict:
        """构建完整的控制状态（当前状态 + 规则变更）"""
        try:
            # 获取当前设备状态作为基础
            complete_state = self._get_current_device_state()
            
            # 应用规则中的变更
            if "led" in actions:
                led_actions = actions["led"]
                for led_name, led_value in led_actions.items():
                    complete_state["led"][led_name] = int(led_value)
            
            if "air_conditioner" in actions:
                ac_actions = actions["air_conditioner"]
                complete_state["air_conditioner"].update({
                    "state": ac_actions.get("state", complete_state["air_conditioner"]["state"]),
                    "mode": ac_actions.get("mode", complete_state["air_conditioner"]["mode"]),
                    "level": int(ac_actions.get("level", complete_state["air_conditioner"]["level"]))
                })
            
            if "multimedia" in actions:
                complete_state["multimedia"] = actions["multimedia"]
            
            return complete_state
            
        except Exception as e:
            print(f"构建完整控制状态时出错: {e}")
            return {}

    def _build_control_state(self, actions: Dict) -> Dict:
        """根据动作配置构建控制状态（保留原方法作为备用）"""
        state = {}
        
        try:
            # 处理LED控制
            if "led" in actions:
                led_actions = actions["led"]
                state["led"] = {}
                for led_name, led_value in led_actions.items():
                    state["led"][led_name] = int(led_value)
            
            # 处理空调控制
            if "air_conditioner" in actions:
                ac_actions = actions["air_conditioner"]
                state["air_conditioner"] = {
                    "state": ac_actions.get("state", "off"),
                    "mode": ac_actions.get("mode", "cool"),
                    "level": int(ac_actions.get("level", 1))
                }
            
            # 处理多媒体控制
            if "multimedia" in actions:
                state["multimedia"] = actions["multimedia"]
            
            return state
            
        except Exception as e:
            print(f"构建控制状态时出错: {e}")
            return {}
    
    def _is_recently_executed(self, rule: AutomationRule, current_time: datetime) -> bool:
        """检查规则是否在最近已经执行过（避免重复执行）"""
        if rule.id not in self.executed_rules:
            return False
        
        last_execution = self.executed_rules[rule.id]
        # 如果是同一天的同一分钟，认为是重复执行
        if (last_execution.date() == current_time.date() and 
            last_execution.hour == current_time.hour and 
            last_execution.minute == current_time.minute):
            return True
        
        return False
    
    def _mark_rule_executed(self, rule: AutomationRule, execution_time: datetime):
        """标记规则已执行"""
        self.executed_rules[rule.id] = execution_time
        
        # 清理超过24小时的执行记录，避免内存泄漏
        cutoff_time = execution_time - timedelta(days=1)
        self.executed_rules = {
            rule_id: exec_time 
            for rule_id, exec_time in self.executed_rules.items() 
            if exec_time > cutoff_time
        }


# 全局调度器实例
automation_scheduler = AutomationScheduler() 