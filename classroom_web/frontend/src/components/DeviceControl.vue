<template>
  <div class="device-control">
    <h3>设备控制</h3>
    
    <!-- 灯光控制 -->
    <el-card class="control-card">
      <template #header>
        <div class="card-header">
          <span>💡 灯光控制</span>
        </div>
      </template>
      <div class="led-controls">
        <div v-for="(status, ledName) in ledStatus" :key="ledName" class="led-item">
          <div class="device-info">
            <span class="device-name">{{ getLedDisplayName(ledName) }}</span>
            <span class="device-power">{{ calculateDevicePower().led[ledName] || 0 }}W</span>
          </div>
          <el-switch 
            v-model="ledStatus[ledName]" 
            :active-value="1" 
            :inactive-value="0"
            @change="updateLedControl(ledName, $event)"
          />
        </div>
      </div>
    </el-card>

    <!-- 空调控制 -->
    <el-card class="control-card">
      <template #header>
        <div class="card-header">
          <span>❄️ 空调控制</span>
        </div>
      </template>
      <div class="ac-controls">
        <div class="ac-item">
          <div class="device-info">
            <span class="device-name">开关</span>
            <span class="device-power">{{ calculateDevicePower().ac }}W</span>
          </div>
          <el-switch 
            v-model="acStatus.state" 
            active-text="开启" 
            inactive-text="关闭"
            active-value="on"
            inactive-value="off"
            @change="updateAcControl"
          />
        </div>
        <div class="ac-item" v-if="acStatus.state === 'on'">
          <span>模式</span>
          <el-select v-model="acStatus.mode" @change="updateAcControl">
            <el-option label="制冷" value="cool" />
            <el-option label="制热" value="heat" />
          </el-select>
        </div>
        <div class="ac-item" v-if="acStatus.state === 'on'">
          <span>档位</span>
          <el-select v-model="acStatus.level" @change="updateAcControl">
            <el-option label="1档" :value="1" />
            <el-option label="2档" :value="2" />
            <el-option label="3档" :value="3" />
          </el-select>
        </div>
      </div>
    </el-card>

    <!-- 多媒体控制 -->
    <el-card class="control-card">
      <template #header>
        <div class="card-header">
          <span>📺 多媒体控制</span>
        </div>
      </template>
      <div class="multimedia-controls">
        <div class="multimedia-header">
          <span class="device-name">多媒体设备</span>
          <span class="device-power">{{ calculateDevicePower().multimedia }}W</span>
        </div>
        <el-radio-group v-model="multimediaStatus" @change="updateMultimediaControl">
          <el-radio :label="'off'" value="off">关闭</el-radio>
          <el-radio :label="'standby'" value="standby">待机</el-radio>
          <el-radio :label="'on'" value="on">开启</el-radio>
        </el-radio-group>
      </div>
    </el-card>
  </div>
</template>

<script setup>
/* eslint-disable no-undef */
import { ref, watch } from 'vue'
import { ElMessage } from 'element-plus'

// 设备状态
const ledStatus = ref({
  led1: 0,
  led2: 0,
  led3: 0,
  led4: 0
})

const acStatus = ref({
  state: 'off',
  mode: 'cool',
  level: 1
})

const multimediaStatus = ref('off')

// 当前完整的state
const currentState = ref({})

// 控制状态锁定（防止立即更新）
const controlLocks = ref({
  led: {},
  ac: false,
  multimedia: false
})

// 获取LED显示名称
const getLedDisplayName = (ledName) => {
  const nameMap = {
    led1: 'LED1',
    led2: 'LED2', 
    led3: 'LED3',
    led4: 'LED4'
  }
  return nameMap[ledName] || ledName
}

// 从props接收最新数据
const props = defineProps({
  latestData: {
    type: Object,
    default: () => ({})
  }
})

// 监听最新数据变化，更新控制组件状态
watch(() => props.latestData, (newData) => {
  if (newData && newData.state) {
    currentState.value = newData.state
    updateControlStatus(newData.state)
  }
}, { deep: true, immediate: true })

// 更新控制组件状态（考虑锁定状态）
const updateControlStatus = (state) => {
  // 更新LED状态（只有未锁定的LED才更新）
  if (state.led) {
    Object.keys(ledStatus.value).forEach(ledName => {
      if (!controlLocks.value.led[ledName]) {
        ledStatus.value[ledName] = state.led[ledName] || 0
      }
    })
  }

  // 更新空调状态（只有未锁定才更新）
  if (state.air_conditioner && !controlLocks.value.ac) {
    acStatus.value = {
      state: state.air_conditioner.state || 'off',
      mode: state.air_conditioner.mode || 'cool',
      level: state.air_conditioner.level || 1
    }
  }

  // 更新多媒体状态（只有未锁定才更新）
  if (state.multimedia && !controlLocks.value.multimedia) {
    multimediaStatus.value = state.multimedia
  }
}

// 发送控制命令
const sendControlCommand = async (newState) => {
  try {
    const response = await fetch('/api/control', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        state: newState
      })
    })

    const result = await response.json()
    if (result.success) {
      ElMessage.success('控制命令已发送')
    } else {
      ElMessage.error('控制命令发送失败')
    }
  } catch (error) {
    console.error('发送控制命令失败:', error)
    ElMessage.error('发送控制命令失败')
  }
}

// LED控制
const updateLedControl = (ledName, value) => {
  // 立即锁定该LED的状态更新
  controlLocks.value.led[ledName] = true
  
  // 立即更新UI状态
  ledStatus.value[ledName] = value
  
  // 发送控制命令
  const newState = {
    ...currentState.value,
    led: {
      ...currentState.value.led,
      [ledName]: value
    }
  }
  sendControlCommand(newState)
  
  // 1秒后解锁，允许接收后端数据更新
  setTimeout(() => {
    controlLocks.value.led[ledName] = false
  }, 1000)
}

// 空调控制
const updateAcControl = () => {
  // 立即锁定空调状态更新
  controlLocks.value.ac = true
  
  // 发送控制命令
  const newState = {
    ...currentState.value,
    air_conditioner: {
      state: acStatus.value.state,
      mode: acStatus.value.mode,
      level: acStatus.value.level
    }
  }
  sendControlCommand(newState)
  
  // 1秒后解锁，允许接收后端数据更新
  setTimeout(() => {
    controlLocks.value.ac = false
  }, 1000)
}

// 多媒体控制
const updateMultimediaControl = (value) => {
  // 立即锁定多媒体状态更新
  controlLocks.value.multimedia = true
  
  // 立即更新UI状态
  multimediaStatus.value = value
  
  // 发送控制命令
  const newState = {
    ...currentState.value,
    multimedia: value
  }
  sendControlCommand(newState)
  
  // 1秒后解锁，允许接收后端数据更新
  setTimeout(() => {
    controlLocks.value.multimedia = false
  }, 1000)
}

// 计算各设备功率
const calculateDevicePower = () => {
  if (!currentState.value) return { led: {}, ac: 0, multimedia: 0 }
  
  const powers = { led: {}, ac: 0, multimedia: 0 }
  
  // LED功率计算
  if (currentState.value.led) {
    Object.keys(currentState.value.led).forEach(ledName => {
      powers.led[ledName] = currentState.value.led[ledName] ? 100 : 0
    })
  }
  
  // 空调功率计算
  if (currentState.value.air_conditioner) {
    const ac = currentState.value.air_conditioner
    if (ac.state === 'on') {
      const level = ac.level || 1
      const mode = ac.mode || 'cool'
      if (mode === 'cool') {
        powers.ac = { 1: 800, 2: 3000, 3: 6200 }[level] || 0
      } else if (mode === 'heat') {
        powers.ac = { 1: 800, 2: 4000, 3: 8400 }[level] || 0
      }
    }
  }
  
  // 多媒体功率计算
  if (currentState.value.multimedia) {
    const mm = currentState.value.multimedia.toLowerCase()
    if (mm === 'on') {
      powers.multimedia = 450
    } else if (mm === 'standby') {
      powers.multimedia = 0.5
    }
  }
  
  return powers
}
</script>

<style scoped>
.device-control {
  max-width: 800px;
  margin: 0 auto 30px;
}

.device-control h3 {
  text-align: center;
  color: #2c3e50;
  font-size: 1.8rem;
  margin-bottom: 25px;
  font-weight: 600;
}

.control-card {
  margin-bottom: 20px;
  border: none;
  border-radius: 16px;
  overflow: hidden;
  box-shadow: 0 4px 20px rgba(0,0,0,0.08);
  transition: all 0.3s ease;
}

.control-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 30px rgba(0,0,0,0.12);
}

.control-card :deep(.el-card__header) {
  background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
  color: white;
  padding: 20px 25px;
  border-bottom: none;
}

.control-card:nth-child(2) :deep(.el-card__header) {
  background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
}

.control-card:nth-child(3) :deep(.el-card__header) {
  background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
}

.control-card:nth-child(4) :deep(.el-card__header) {
  background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
}

.card-header {
  display: flex;
  align-items: center;
  font-weight: 600;
  font-size: 1.1rem;
}

.control-card :deep(.el-card__body) {
  padding: 25px;
  background: white;
}

.led-controls {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 16px;
}

.led-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 12px;
  color: white;
  font-weight: 500;
  transition: all 0.3s ease;
}

.led-item:hover {
  transform: scale(1.02);
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.3);
}

.ac-controls {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.ac-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 18px 22px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 12px;
  color: white;
  font-weight: 500;
  transition: all 0.3s ease;
}

.ac-item:hover {
  transform: scale(1.01);
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.3);
}

.multimedia-controls {
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 12px;
  display: flex;
  flex-direction: column;
}

.multimedia-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15px;
  padding-bottom: 10px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.2);
}

.multimedia-header .device-name {
  font-size: 1.1rem;
  font-weight: 600;
  color: white;
}

.multimedia-header .device-power {
  font-size: 0.9rem;
  opacity: 0.9;
  color: white;
  background: rgba(255, 255, 255, 0.2);
  padding: 4px 12px;
  border-radius: 15px;
  font-weight: 500;
}

.multimedia-controls :deep(.el-radio-group) {
  display: flex;
  justify-content: center;
  gap: 15px;
}

.multimedia-controls :deep(.el-radio) {
  color: white;
  font-weight: 500;
  margin-right: 0;
  padding: 8px 16px;
  border-radius: 20px;
  transition: all 0.3s ease;
}

.multimedia-controls :deep(.el-radio:hover) {
  background: rgba(255, 255, 255, 0.1);
}

.multimedia-controls :deep(.el-radio__label) {
  color: white;
  font-weight: 500;
  font-size: 0.95rem;
}

.multimedia-controls :deep(.el-radio.is-checked) {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 20px;
}

.multimedia-controls :deep(.el-radio.is-checked .el-radio__label) {
  color: white;
  font-weight: 600;
}

.multimedia-controls :deep(.el-radio__input.is-checked .el-radio__inner) {
  background-color: #fff;
  border-color: #fff;
}

.multimedia-controls :deep(.el-radio__input.is-checked .el-radio__inner::after) {
  background-color: #667eea;
}

.el-select {
  width: 140px;
}

.el-select :deep(.el-input__inner) {
  background: rgba(255, 255, 255, 0.2);
  border: 1px solid rgba(255, 255, 255, 0.3);
  color: white;
  border-radius: 8px;
}

.el-select :deep(.el-input__inner::placeholder) {
  color: rgba(255, 255, 255, 0.7);
}

.el-select :deep(.el-input__suffix) {
  color: white;
}

/* 开关样式优化 */
.led-item :deep(.el-switch.is-checked .el-switch__core) {
  background-color: #13ce66;
}

.ac-item :deep(.el-switch.is-checked .el-switch__core) {
  background-color: #13ce66;
}

.led-item :deep(.el-switch__core) {
  background-color: rgba(255, 255, 255, 0.3);
  border-color: rgba(255, 255, 255, 0.3);
}

.ac-item :deep(.el-switch__core) {
  background-color: rgba(255, 255, 255, 0.3);
  border-color: rgba(255, 255, 255, 0.3);
}

.device-info {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
}

.device-name {
  font-size: 1rem;
  font-weight: 500;
}

.device-power {
  font-size: 0.85rem;
  opacity: 0.8;
  margin-top: 2px;
}

@media (max-width: 768px) {
  .led-controls {
    grid-template-columns: 1fr;
    gap: 12px;
  }
  
  .ac-controls {
    gap: 15px;
  }
  
  .led-item, .ac-item {
    padding: 14px 18px;
  }
  
  .multimedia-controls {
    padding: 15px;
  }
  
  .multimedia-controls :deep(.el-radio-group) {
    flex-direction: column;
    gap: 10px;
  }
  
  .device-control h3 {
    font-size: 1.5rem;
  }
}
</style> 