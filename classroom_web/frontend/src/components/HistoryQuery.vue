<template>
  <div class="history-query">
    <div class="panel-header">
      <h2>📊 历史数据查询</h2>
      <p>查看设备的历史监测数据变化趋势</p>
    </div>

        <div class="query-form-container">
          <el-form :model="queryForm" label-position="top" class="modern-form">
            <div class="form-row">
              <el-form-item label="📊 数据类型" class="form-item-enhanced">
                <el-select 
                  v-model="queryForm.dataType" 
                  placeholder="选择要查询的数据类型" 
                  class="enhanced-select"
                  size="large"
                >
                  <el-option label="🌡️ 温度" value="temperature" />
                  <el-option label="💧 湿度" value="humidity" />
                  <el-option label="💡 光照" value="light" />
                  <el-option label="👥 有无人" value="occupancy" />
                  <el-option label="⚡ 功耗" value="power" />
                </el-select>
              </el-form-item>

              <el-form-item label="⏰ 查询单位" class="form-item-enhanced">
                <el-radio-group 
                  v-model="queryForm.unit" 
                  class="enhanced-radio-group"
                  size="large"
                >
                  <el-radio-button 
                    label="hour" 
                    class="radio-button-enhanced"
                    :disabled="queryForm.dataType === 'occupancy' && queryForm.unit === 'day'"
                  >
                    📈 按小时
                  </el-radio-button>
                  <el-radio-button 
                    label="day" 
                    class="radio-button-enhanced"
                    :disabled="queryForm.dataType === 'occupancy'"
                  >
                    📅 按天
                  </el-radio-button>
                </el-radio-group>
              </el-form-item>
            </div>

            <el-form-item label="📅 时间范围" class="form-item-enhanced full-width">
              <el-date-picker
                v-model="queryForm.dateRange"
                type="daterange"
                range-separator="至"
                start-placeholder="开始日期"
                end-placeholder="结束日期"
                format="YYYY-MM-DD"
                value-format="YYYY-MM-DD"
                :disabled-date="disabledDate"
                class="enhanced-date-picker"
                size="large"
              />
            </el-form-item>

            <el-form-item class="form-item-enhanced full-width">
              <el-button 
                type="primary" 
                @click="handleQuery" 
                :loading="loading"
                class="enhanced-query-btn"
                size="large"
              >
                <span v-if="!loading">🔍 开始查询</span>
                <span v-else>⏳ 查询中...</span>
              </el-button>
            </el-form-item>
          </el-form>
        </div>

        <div v-if="chartData.length > 0" class="chart-section">
          <div class="chart-header">
            <h3>📊 数据图表</h3>
            <p>{{ getChartDescription }}</p>
            <p class="data-note">💡 注意：-1表示此时没有传感器数据</p>
          </div>
          <div class="chart-container" ref="chartContainer">
            <div ref="chartRef" class="chart"></div>
          </div>
        </div>

        <div v-else-if="!loading && queryForm.dateRange?.length" class="no-data-section">
          <div class="no-data-content">
            <div class="no-data-icon">📭</div>
            <h3>暂无数据</h3>
            <p>所选时间范围内没有找到相关数据</p>
          </div>
        </div>
  </div>
</template>

<script setup>
/* eslint-disable no-undef */
import { ref, onMounted, nextTick, computed } from 'vue'
import * as echarts from 'echarts'
import dayjs from 'dayjs'
import { ElMessage } from 'element-plus'

const loading = ref(false)
const chartData = ref([])
const availableDates = ref([])
const chartRef = ref(null)
const chartInstance = ref(null)

const queryForm = ref({
  dataType: 'temperature',
  unit: 'hour',
  dateRange: []
})

// 获取可用日期
const fetchAvailableDates = async () => {
  console.log('开始获取可用日期...')
  try {
    const response = await fetch('/api/available-dates')
    const data = await response.json()
    console.log('获取到的可用日期数据:', data)
    availableDates.value = data.dates
    console.log('设置后的 availableDates:', availableDates.value)
  } catch (error) {
    console.error('获取可用日期失败:', error)
  }
}

// 禁用没有数据的日期
const disabledDate = (date) => {
  const dateStr = dayjs(date).format('YYYY-MM-DD')
  console.log('当前检查的日期:', dateStr)
  console.log('可用日期列表:', availableDates.value)
  const dateInfo = availableDates.value.find(d => {
    console.log('比较:', d.date, dateStr, d.date === dateStr)
    return d.date === dateStr
  })
  const isDisabled = !dateInfo?.has_data
  console.log('日期信息:', dateInfo, '是否禁用:', isDisabled)
  return isDisabled
}

// 查询数据
const handleQuery = async () => {
  if (!queryForm.value.dateRange?.length) {
    ElMessage.warning('请选择时间范围')
    return
  }

  // 检查有无人数据的查询限制
  if (queryForm.value.dataType === 'occupancy' && queryForm.value.unit === 'day') {
    ElMessage.warning('有无人数据只能按小时查询')
    return
  }

  loading.value = true
  try {
    const response = await fetch('/api/query-history', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        start_date: queryForm.value.dateRange[0],
        end_date: queryForm.value.dateRange[1],
        data_type: queryForm.value.dataType,
        unit: queryForm.value.unit
      })
    })

    const data = await response.json()
    console.log('查询返回数据:', data)
    chartData.value = data.data
    console.log('图表数据:', chartData.value)
    
    if (chartData.value.length === 0) {
      ElMessage.warning('所选时间范围内没有数据')
      return
    }
    
    await nextTick()
    updateChart()
  } catch (error) {
    console.error('查询失败:', error)
    ElMessage.error('查询失败')
  } finally {
    loading.value = false
  }
}

// 更新图表
const updateChart = () => {
  if (!chartRef.value) {
    console.warn('图表容器未找到')
    return
  }

  if (!chartInstance.value) {
    console.log('初始化图表实例')
    chartInstance.value = echarts.init(chartRef.value)
  }

  console.log('开始更新图表，数据:', chartData.value)
  const option = {
    tooltip: {
      trigger: 'axis',
      formatter: function(params) {
        const data = params[0]
        let unit = ''
        switch (queryForm.value.dataType) {
          case 'temperature':
            unit = '°C'
            break
          case 'humidity':
            unit = '%'
            break
          case 'light':
            unit = 'lux'
            break
          case 'occupancy':
            unit = ''
            break
          case 'power':
            unit = queryForm.value.unit === 'day' ? 'kWh' : 'kW'
            break
        }
        return `${data.name}<br/>${data.value}${unit}`
      }
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '3%',
      containLabel: true
    },
    xAxis: {
      type: 'category',
      data: chartData.value.map(item => {
        if (queryForm.value.unit === 'hour') {
          return dayjs(item.timestamp).format('MM-DD HH:mm')
        }
        return dayjs(item.timestamp).format('YYYY-MM-DD')
      }),
      axisLabel: {
        rotate: 45
      }
    },
    yAxis: {
      type: 'value',
      name: getYAxisName(),
      axisLabel: {
        formatter: function(value) {
          if (queryForm.value.dataType === 'occupancy') {
            return value ? '有人' : '无人'
          }
          return value
        }
      }
    },
    dataZoom: [
      {
        type: 'slider',
        show: true,
        xAxisIndex: [0],
        start: 0,
        end: 100
      }
    ],
    series: [
      {
        type: 'line',
        data: chartData.value.map(item => item.value),
        smooth: true,
        connectNulls: true  // 连接空值点
      }
    ]
  }

  chartInstance.value.setOption(option)
}

// 获取Y轴名称
const getYAxisName = () => {
  const typeMap = {
    temperature: '温度 (°C)',
    humidity: '湿度 (%)',
    light: '光照 (lux)',
    occupancy: '有无人',
    power: queryForm.value.unit === 'day' ? '总功耗 (kWh)' : '功耗 (kW)'
  }
  return typeMap[queryForm.value.dataType]
}

// 监听窗口大小变化
window.addEventListener('resize', () => {
  chartInstance.value?.resize()
})

// 组件挂载时获取可用日期
onMounted(() => {
  fetchAvailableDates()
})

// 获取图表描述
const getChartDescription = computed(() => {
  const typeMap = {
    temperature: '温度变化趋势',
    humidity: '湿度变化趋势',
    light: '光照强度变化',
    occupancy: '人员活动情况',
    power: queryForm.value.unit === 'day' ? '每日总功耗统计' : '实时功耗监控'
  }
  return typeMap[queryForm.value.dataType] || '数据变化趋势'
})
</script>

<style scoped>
.history-query {
  max-width: 1200px;
  margin: 0 auto;
}

.panel-header {
  text-align: center;
  margin-bottom: 30px;
  padding: 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-radius: 16px;
  box-shadow: 0 4px 20px rgba(102, 126, 234, 0.3);
}

.panel-header h2 {
  margin: 0 0 10px 0;
  font-size: 1.8rem;
  font-weight: 600;
}

.panel-header p {
  margin: 0;
  opacity: 0.9;
  font-size: 1rem;
}

.query-form-container {
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(10px);
  border-radius: 20px;
  padding: 40px;
  margin-bottom: 30px;
  box-shadow: 0 10px 40px rgba(0,0,0,0.1);
}

.modern-form {
  max-width: 100%;
  margin: 0 auto;
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 30px;
  margin-bottom: 20px;
}

.form-item-enhanced {
  margin-bottom: 25px;
}

.form-item-enhanced.full-width {
  grid-column: 1 / -1;
}

/* 数据类型选择器样式 */
:deep(.enhanced-select) {
  width: 100%;
}

:deep(.enhanced-select .el-input__wrapper) {
  background: linear-gradient(135deg, #f8f9ff 0%, #e8f2ff 100%);
  border: 2px solid #e1e8f0;
  border-radius: 15px;
  padding: 12px 16px;
  transition: all 0.3s ease;
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.1);
}

:deep(.enhanced-select .el-input__wrapper:hover) {
  border-color: #667eea;
  box-shadow: 0 6px 20px rgba(102, 126, 234, 0.2);
  transform: translateY(-2px);
}

:deep(.enhanced-select .el-input__wrapper.is-focus) {
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.2);
}

:deep(.enhanced-select .el-input__inner) {
  color: #2d3748;
  font-weight: 500;
}

/* 单位选择器样式 */
:deep(.enhanced-radio-group) {
  display: flex;
  gap: 10px;
  width: 100%;
}

:deep(.enhanced-radio-group .el-radio-button) {
  flex: 1;
}

:deep(.enhanced-radio-group .el-radio-button__inner) {
  background: linear-gradient(135deg, #f8f9ff 0%, #e8f2ff 100%);
  border: 2px solid #e1e8f0;
  border-radius: 15px;
  padding: 12px 20px;
  font-weight: 600;
  color: #4a5568;
  transition: all 0.3s ease;
  width: 100%;
  text-align: center;
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.1);
}

:deep(.enhanced-radio-group .el-radio-button__inner:hover) {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-color: #667eea;
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(102, 126, 234, 0.3);
}

:deep(.enhanced-radio-group .el-radio-button.is-active .el-radio-button__inner) {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-color: #667eea;
  box-shadow: 0 6px 20px rgba(102, 126, 234, 0.3);
}

:deep(.enhanced-radio-group .el-radio-button.is-disabled .el-radio-button__inner) {
  background: #f5f5f5;
  color: #c0c4cc;
  border-color: #e4e7ed;
  cursor: not-allowed;
  opacity: 0.6;
}

/* 日期选择器样式 */
:deep(.enhanced-date-picker) {
  width: 100%;
}

:deep(.enhanced-date-picker .el-input__wrapper) {
  background: linear-gradient(135deg, #f8f9ff 0%, #e8f2ff 100%);
  border: 2px solid #e1e8f0;
  border-radius: 15px;
  padding: 12px 16px;
  transition: all 0.3s ease;
  box-shadow: 0 4px 15px rgba(102, 126, 234, 0.1);
}

:deep(.enhanced-date-picker .el-input__wrapper:hover) {
  border-color: #667eea;
  box-shadow: 0 6px 20px rgba(102, 126, 234, 0.2);
  transform: translateY(-2px);
}

:deep(.enhanced-date-picker .el-input__wrapper.is-focus) {
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.2);
}

/* 查询按钮样式 */
.enhanced-query-btn {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border: none;
  border-radius: 15px;
  padding: 15px 40px;
  font-size: 1.1rem;
  font-weight: 600;
  width: 100%;
  height: 55px;
  box-shadow: 0 8px 25px rgba(102, 126, 234, 0.3);
  transition: all 0.3s ease;
}

.enhanced-query-btn:hover {
  transform: translateY(-3px);
  box-shadow: 0 12px 35px rgba(102, 126, 234, 0.4);
}

.enhanced-query-btn:active {
  transform: translateY(-1px);
}

/* 表单标签样式 */
:deep(.form-item-enhanced .el-form-item__label) {
  font-weight: 600;
  color: #2d3748;
  font-size: 1rem;
  margin-bottom: 10px;
  padding-bottom: 5px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .form-row {
    grid-template-columns: 1fr;
    gap: 20px;
  }
  
  .query-form-container {
    margin: 0 10px 20px;
    padding: 25px;
  }
  
  :deep(.enhanced-radio-group) {
    flex-direction: column;
  }
  
  :deep(.enhanced-radio-group .el-radio-button__inner) {
    margin-bottom: 10px;
  }
}

.chart-section {
  background: #f8fafc;
  padding: 30px 40px;
  border-top: 1px solid #e0e0e0;
}

.chart-header {
  text-align: center;
  margin-bottom: 25px;
}

.chart-header h3 {
  font-size: 1.4rem;
  color: #2c3e50;
  margin-bottom: 8px;
}

.chart-header p {
  color: #7f8c8d;
  font-size: 1rem;
}

.data-note {
  color: #e67e22;
  font-size: 0.9rem;
  font-style: italic;
  margin-top: 5px;
  margin-bottom: 0;
}

.chart-container {
  background: white;
  border-radius: 12px;
  padding: 20px;
  box-shadow: 0 4px 15px rgba(0,0,0,0.05);
}

.chart {
  width: 100%;
  height: 450px;
}

.no-data-section {
  background: #f8fafc;
  padding: 60px 40px;
  border-top: 1px solid #e0e0e0;
}

.no-data-content {
  text-align: center;
  color: #7f8c8d;
}

.no-data-icon {
  font-size: 4rem;
  margin-bottom: 20px;
}

.no-data-content h3 {
  font-size: 1.3rem;
  margin-bottom: 10px;
  color: #2c3e50;
}

.no-data-content p {
  font-size: 1rem;
}

@media (max-width: 768px) {
  .dialog-header {
    padding: 25px 20px;
  }
  
  .dialog-header h2 {
    font-size: 1.5rem;
  }
  
  .query-form-container {
    padding: 25px 20px;
  }
  
  .chart-section {
    padding: 25px 20px;
  }
  
  .chart {
    height: 350px;
  }
  
  .no-data-section {
    padding: 40px 20px;
  }
  
  .no-data-icon {
    font-size: 3rem;
  }
  
  .query-trigger-btn {
    padding: 12px 30px;
    font-size: 1rem;
  }
}
</style> 