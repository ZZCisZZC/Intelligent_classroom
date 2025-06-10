<template>
  <!-- 已获得数据时显示面板 -->
  <div v-if="data && data.device_id" class="status-panel">
    <!-- Power Panel -->
    <div class="power-panel">
      <div class="power-value">{{ Math.round(data.power) }}</div>
      <div class="power-unit">瓦特 (W)</div>
      <div class="power-label">当前功率</div>
    </div>

    <!-- Sensor Data Panel -->
    <div class="sensor-panel">
      <h3>📊 环境数据</h3>
      <div class="sensor-grid">
        <div class="sensor-item" :class="{ 'error': tempError }">
          <div class="sensor-icon">🌡️</div>
          <div class="sensor-info">
            <div class="sensor-value">{{ tempDisplay }}</div>
            <div class="sensor-label">温度</div>
          </div>
        </div>
        
        <div class="sensor-item" :class="{ 'error': humidityError }">
          <div class="sensor-icon">💧</div>
          <div class="sensor-info">
            <div class="sensor-value">{{ humidityDisplay }}</div>
            <div class="sensor-label">湿度</div>
          </div>
        </div>
        
        <div class="sensor-item" :class="{ 'error': luxError }">
          <div class="sensor-icon">☀️</div>
          <div class="sensor-info">
            <div class="sensor-value">{{ luxDisplay }}</div>
            <div class="sensor-label">光照</div>
          </div>
        </div>
        
        <div class="sensor-item">
          <div class="sensor-icon">🚶</div>
          <div class="sensor-info">
            <div class="sensor-value">{{ isPerson ? '有人' : '无人' }}</div>
            <div class="sensor-label">人体感应</div>
          </div>
        </div>
      </div>
      
      <div class="update-time">
        📅 更新时间：{{ formattedTime }}
        <span class="connection-status" :class="connectionStatusClass">
          {{ connectionStatusText }}
        </span>
      </div>
    </div>
  </div>
  <!-- 数据未就绪时显示加载中 -->
  <div v-else class="status-panel">
    <div class="loading">
      <div class="loading-spinner"></div>
      <div>正在获取数据...</div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'StatusPanel',
  data() {
    return {
      data: {},
      currentTime: new Date(),
      lastReceiveTime: null // 新增：最近一次收到数据的本地时间
    }
  },
  computed: {
    formattedTime() {
      return new Date(this.data.timestamp).toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
      })
    },
    isPerson() {
      return String(this.data.sensor_data.person).toLowerCase() === 'true'
    },
    // 温度显示
    tempError() {
      return this.data?.sensor_data?.temp === -1
    },
    tempDisplay() {
      if (this.tempError) {
        return '错误或未连接'
      }
      return this.data?.sensor_data?.temp?.toFixed(1) + '°C' || '未知'
    },
    // 湿度显示
    humidityError() {
      return this.data?.sensor_data?.humidity === -1
    },
    humidityDisplay() {
      if (this.humidityError) {
        return '错误或未连接'
      }
      return this.data?.sensor_data?.humidity?.toFixed(1) + '%' || '未知'
    },
    // 光照显示
    luxError() {
      return this.data?.sensor_data?.lux === -1
    },
    luxDisplay() {
      if (this.luxError) {
        return '错误或未连接'
      }
      return this.data?.sensor_data?.lux?.toFixed(1) || '未知'
    },
    // 判断设备连接状态（用本地收到数据的时间）
    isDeviceConnected() {
      if (!this.lastReceiveTime) return false
      const timeDiff = (this.currentTime - this.lastReceiveTime) / 1000 // 秒
      return timeDiff <= 5
    },
    connectionStatusText() {
      return this.isDeviceConnected ? '🟢 已连接' : '🔴 未连接'
    },
    connectionStatusClass() {
      return this.isDeviceConnected ? 'connected' : 'disconnected'
    }
  },
  watch: {
    data: {
      handler(newVal, oldVal) {
        // 只有数据内容发生变化时才更新时间
        if (JSON.stringify(newVal) !== JSON.stringify(oldVal)) {
          this.lastReceiveTime = new Date()
        }
      },
      deep: true
    }
  },
  mounted() {
    this.timer = setInterval(() => {
      this.currentTime = new Date()
    }, 1000)
  },
  beforeUnmount() {
    if (this.timer) {
      clearInterval(this.timer)
    }
  }
}
</script>

<style scoped>
.status-panel {
  max-width: 800px;
  margin: 20px auto;
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.power-panel {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  padding: 30px;
  border-radius: 16px;
  text-align: center;
  box-shadow: 0 8px 32px rgba(102, 126, 234, 0.3);
  position: relative;
  overflow: hidden;
}

.power-panel::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: linear-gradient(45deg, transparent, rgba(255,255,255,0.1), transparent);
  transform: rotate(45deg);
  animation: shine 3s infinite;
}

@keyframes shine {
  0% { transform: translateX(-100%) translateY(-100%) rotate(45deg); }
  100% { transform: translateX(100%) translateY(100%) rotate(45deg); }
}

.power-value {
  font-size: 3.5rem;
  font-weight: bold;
  margin-bottom: 5px;
  text-shadow: 0 2px 4px rgba(0,0,0,0.3);
}

.power-unit {
  font-size: 1.2rem;
  margin-bottom: 10px;
  opacity: 0.9;
}

.power-label {
  font-size: 1rem;
  opacity: 0.8;
}

.sensor-panel {
  background: white;
  padding: 25px;
  border-radius: 16px;
  box-shadow: 0 4px 20px rgba(0,0,0,0.08);
  border: 1px solid #f0f0f0;
}

.sensor-panel h3 {
  margin: 0 0 20px 0;
  color: #2c3e50;
  font-size: 1.3rem;
}

.sensor-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
  gap: 20px;
  margin-bottom: 20px;
}

.sensor-item {
  display: flex;
  align-items: center;
  padding: 15px;
  background: #f8fafc;
  border-radius: 12px;
  transition: all 0.3s ease;
  border: 2px solid transparent;
}

.sensor-item:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 15px rgba(0,0,0,0.1);
  border-color: #e3f2fd;
}

.sensor-item.error {
  background: #fef2f2;
  border-color: #fecaca;
}

.sensor-item.error .sensor-value {
  color: #dc2626;
  font-weight: bold;
}

.sensor-item.error .sensor-icon {
  filter: grayscale(1);
  opacity: 0.6;
}

.sensor-icon {
  font-size: 2rem;
  margin-right: 12px;
  width: 40px;
  text-align: center;
}

.sensor-info {
  flex: 1;
}

.sensor-value {
  font-size: 1.3rem;
  font-weight: bold;
  color: #2c3e50;
  margin-bottom: 2px;
}

.sensor-label {
  font-size: 0.9rem;
  color: #7f8c8d;
}

.update-time {
  text-align: center;
  color: #7f8c8d;
  font-size: 0.9rem;
  padding-top: 15px;
  border-top: 1px solid #ecf0f1;
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 15px;
  flex-wrap: wrap;
}

.connection-status {
  padding: 4px 12px;
  border-radius: 15px;
  font-size: 0.85rem;
  font-weight: 500;
  transition: all 0.3s ease;
}

.connection-status.connected {
  background: linear-gradient(135deg, #4caf50 0%, #45a049 100%);
  color: white;
  box-shadow: 0 2px 8px rgba(76, 175, 80, 0.3);
}

.connection-status.disconnected {
  background: linear-gradient(135deg, #f44336 0%, #d32f2f 100%);
  color: white;
  box-shadow: 0 2px 8px rgba(244, 67, 54, 0.3);
  animation: blink 2s infinite;
}

@keyframes blink {
  0%, 50% { opacity: 1; }
  51%, 100% { opacity: 0.7; }
}

.loading {
  text-align: center;
  padding: 50px;
  color: #7f8c8d;
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 4px solid #f3f3f3;
  border-top: 4px solid #667eea;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto 20px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

@media (max-width: 768px) {
  .sensor-grid {
    grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
    gap: 15px;
  }
  
  .power-value {
    font-size: 2.5rem;
  }
  
  .sensor-item {
    padding: 12px;
  }
  
  .update-time {
    flex-direction: column;
    gap: 8px;
  }
}
</style>
