# models.py
import os
from sqlalchemy import Column, Integer, String, JSON, DateTime, Float, create_engine, Boolean, Text
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from datetime import datetime

# 数据库文件名
DB_FILE = "iot_data.db"
# 对应的连接串
DB_URL  = f"sqlite:///{DB_FILE}"

Base = declarative_base()
engine = create_engine(DB_URL, echo=False)
SessionLocal = sessionmaker(bind=engine)

class DeviceData(Base):
    __tablename__ = "device_data"
    id           = Column(Integer, primary_key=True, index=True)
    device_id    = Column(String, index=True)
    timestamp    = Column(DateTime, default=datetime.utcnow)
    sensor_data  = Column(JSON)
    state        = Column(JSON)
    power        = Column(Float, default=0.0) 

class AutomationRule(Base):
    __tablename__ = "automation_rules"
    id           = Column(Integer, primary_key=True, index=True)
    name         = Column(String, nullable=False)  # 规则名称
    description  = Column(Text)  # 规则描述
    enabled      = Column(Boolean, default=True)  # 是否启用
    schedule     = Column(JSON, nullable=False)  # 调度配置 {"type": "daily/weekly", "time": "HH:MM", "days": [1,2,3]} 
    actions      = Column(JSON, nullable=False)  # 执行的操作
    created_at   = Column(DateTime, default=datetime.utcnow)
    updated_at   = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

def init_db():
    """
    如果数据库文件不存在，则创建数据库文件与表结构；
    如果已存在，则直接打开使用，不做任何修改。
    """
    if not os.path.exists(DB_FILE):
        Base.metadata.create_all(bind=engine)
        print(f"数据库 {DB_FILE} 不存在，已创建表结构。")
    else:
        # 检查是否需要更新表结构
        Base.metadata.create_all(bind=engine)
        print(f"数据库 {DB_FILE} 已存在，已检查表结构。")