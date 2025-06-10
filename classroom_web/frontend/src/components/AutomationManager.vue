<template>
  <div class="automation-manager">
    <h3>自动化管理</h3>
    
    <!-- 添加规则按钮 -->
    <div class="header-actions">
      <el-button type="primary" @click="showCreateDialog = true">
        <el-icon><Plus /></el-icon>
        添加自动化规则
      </el-button>

    </div>

    <!-- 规则列表 -->
    <div class="rules-list">
      <el-card 
        v-for="rule in rules" 
        :key="rule.id" 
        class="rule-card"
        :class="{ disabled: !rule.enabled }"
      >
        <template #header>
          <div class="rule-header">
            <div class="rule-info">
              <h4>{{ rule.name }}</h4>
              <p v-if="rule.description">{{ rule.description }}</p>
            </div>
            <div class="rule-actions">
              <el-switch 
                v-model="rule.enabled" 
                @change="toggleRule(rule)"
                :active-text="rule.enabled ? '已启用' : '已禁用'"
                active-text-color="#ffffff"
                inactive-text-color="#ffffff"
              />
              <el-button type="primary" size="small" @click="editRule(rule)">
                <el-icon><Edit /></el-icon>
                编辑
              </el-button>
              <el-button type="danger" size="small" @click="deleteRule(rule)">
                <el-icon><Delete /></el-icon>
                删除
              </el-button>
            </div>
          </div>
        </template>
        
        <div class="rule-content">
          <div class="schedule-info">
            <h5>⏰ 执行时间</h5>
            <p>{{ formatSchedule(rule.schedule) }}</p>
          </div>
          
          <div class="actions-info">
            <h5>🎯 执行操作</h5>
            <div class="action-tags">
              <el-tag 
                v-for="action in formatActions(rule.actions)" 
                :key="action.key"
                :type="action.type"
                class="action-tag"
              >
                {{ action.text }}
              </el-tag>
            </div>
          </div>
        </div>
      </el-card>
      
      <div v-if="rules.length === 0" class="empty-state">
        <p>还没有自动化规则，点击上方按钮创建第一个规则吧！</p>
      </div>
    </div>

    <!-- 创建/编辑规则对话框 -->
    <el-dialog 
      v-model="showCreateDialog" 
      :title="editingRule ? '编辑自动化规则' : '创建自动化规则'"
      width="600px"
      @close="resetForm"
    >
      <el-form :model="ruleForm" :rules="formRules" ref="ruleFormRef" label-width="100px">
        <el-form-item label="规则名称" prop="name">
          <el-input v-model="ruleForm.name" placeholder="请输入规则名称" />
        </el-form-item>
        
        <el-form-item label="规则描述">
          <el-input 
            v-model="ruleForm.description" 
            type="textarea" 
            placeholder="请输入规则描述（可选）" 
            :rows="2"
          />
        </el-form-item>
        
        <el-form-item label="执行时间" prop="scheduleType">
          <el-radio-group v-model="ruleForm.scheduleType">
            <el-radio value="daily">每天执行</el-radio>
            <el-radio value="weekly">每周执行</el-radio>
          </el-radio-group>
        </el-form-item>
        
        <el-form-item label="具体时间" prop="time">
          <el-time-picker 
            v-model="ruleForm.time" 
            format="HH:mm"
            value-format="HH:mm"
            placeholder="选择执行时间"
          />
        </el-form-item>
        
        <el-form-item v-if="ruleForm.scheduleType === 'weekly'" label="执行日期" prop="days">
          <el-checkbox-group v-model="ruleForm.days">
            <el-checkbox :value="1">周一</el-checkbox>
            <el-checkbox :value="2">周二</el-checkbox>
            <el-checkbox :value="3">周三</el-checkbox>
            <el-checkbox :value="4">周四</el-checkbox>
            <el-checkbox :value="5">周五</el-checkbox>
            <el-checkbox :value="6">周六</el-checkbox>
            <el-checkbox :value="7">周日</el-checkbox>
          </el-checkbox-group>
        </el-form-item>
        
        <el-divider>执行操作</el-divider>
        
        <!-- LED控制 -->
        <el-form-item label="LED控制">
          <div class="action-controls">
            <div class="led-controls">
              <div v-for="ledName in ['led1', 'led2', 'led3', 'led4']" :key="ledName" class="led-item">
                <span>{{ ledName.toUpperCase() }}</span>
                <el-radio-group v-model="ruleForm.actions.led[ledName]">
                  <el-radio :value="0">关闭</el-radio>
                  <el-radio :value="1">开启</el-radio>
                  <el-radio value="no-change">不变</el-radio>
                </el-radio-group>
              </div>
            </div>
          </div>
        </el-form-item>
        
        <!-- 空调控制 -->
        <el-form-item label="空调控制">
          <div class="action-controls">
            <div class="ac-controls">
              <div class="ac-item">
                <span>开关状态：</span>
                <el-radio-group v-model="ruleForm.actions.air_conditioner.state">
                  <el-radio value="off">关闭</el-radio>
                  <el-radio value="on">开启</el-radio>
                  <el-radio value="no-change">不变</el-radio>
                </el-radio-group>
              </div>
              <div class="ac-item" v-if="ruleForm.actions.air_conditioner.state === 'on'">
                <span>制冷模式：</span>
                <el-radio-group v-model="ruleForm.actions.air_conditioner.mode">
                  <el-radio value="cool">制冷</el-radio>
                  <el-radio value="heat">制热</el-radio>
                </el-radio-group>
              </div>
              <div class="ac-item" v-if="ruleForm.actions.air_conditioner.state === 'on'">
                <span>档位设置：</span>
                <el-radio-group v-model="ruleForm.actions.air_conditioner.level">
                  <el-radio :value="1">1档</el-radio>
                  <el-radio :value="2">2档</el-radio>
                  <el-radio :value="3">3档</el-radio>
                </el-radio-group>
              </div>
            </div>
          </div>
        </el-form-item>
        
        <!-- 多媒体控制 -->
        <el-form-item label="多媒体控制">
          <el-radio-group v-model="ruleForm.actions.multimedia">
            <el-radio value="off">关闭</el-radio>
            <el-radio value="standby">待机</el-radio>
            <el-radio value="on">开启</el-radio>
            <el-radio value="no-change">不变</el-radio>
          </el-radio-group>
        </el-form-item>
      </el-form>
      
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="cancelEdit">取消</el-button>
          <el-button type="primary" @click="saveRule">
            {{ editingRule ? '更新' : '创建' }}
          </el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, defineExpose } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { Plus, Edit, Delete } from '@element-plus/icons-vue'
import { on, off, EVENTS } from '../utils/eventBus.js'

// 数据
const rules = ref([])
const showCreateDialog = ref(false)
const editingRule = ref(null)

// 表单数据
const ruleForm = ref({
  name: '',
  description: '',
  scheduleType: 'daily',
  time: '',
  days: [],
  actions: {
    led: {
      led1: 'no-change',
      led2: 'no-change',
      led3: 'no-change',
      led4: 'no-change'
    },
    air_conditioner: {
      state: 'no-change',
      mode: 'cool',
      level: 1
    },
    multimedia: 'no-change'
  }
})

const ruleFormRef = ref()

// 表单验证规则
const formRules = {
  name: [
    { required: true, message: '请输入规则名称', trigger: 'blur' }
  ],
  scheduleType: [
    { required: true, message: '请选择执行类型', trigger: 'change' }
  ],
  time: [
    { required: true, message: '请选择执行时间', trigger: 'change' }
  ],
  days: [
    {
      validator: (rule, value, callback) => {
        if (ruleForm.value.scheduleType === 'weekly' && (!value || value.length === 0)) {
          callback(new Error('请选择执行日期'))
        } else {
          callback()
        }
      },
      trigger: 'change'
    }
  ]
}

// 加载规则列表
const loadRules = async () => {
  try {
    const response = await fetch('/api/automation/rules')
    const result = await response.json()
    if (result.success) {
      rules.value = result.data
    } else {
      ElMessage.error('加载自动化规则失败')
    }
  } catch (error) {
    console.error('加载规则列表失败:', error)
    ElMessage.error('加载自动化规则失败')
  }
}

// 格式化调度信息
const formatSchedule = (schedule) => {
  if (schedule.type === 'daily') {
    return `每天 ${schedule.time}`
  } else if (schedule.type === 'weekly') {
    const dayNames = ['', '周一', '周二', '周三', '周四', '周五', '周六', '周日']
    const days = schedule.days.map(day => dayNames[day]).join('、')
    return `每周 ${days} ${schedule.time}`
  }
  return '未知调度'
}

// 格式化操作信息
const formatActions = (actions) => {
  const result = []
  
  // 处理LED操作
  if (actions.led) {
    Object.keys(actions.led).forEach(ledName => {
      const value = actions.led[ledName]
      if (value !== 'no-change') {
        result.push({
          key: `led-${ledName}`,
          text: `${ledName.toUpperCase()}: ${value ? '开启' : '关闭'}`,
          type: value ? 'success' : 'info'
        })
      }
    })
  }
  
  // 处理空调操作
  if (actions.air_conditioner && actions.air_conditioner.state !== 'no-change') {
    if (actions.air_conditioner.state === 'off') {
      result.push({
        key: 'ac-off',
        text: '空调: 关闭',
        type: 'info'
      })
    } else {
      const mode = actions.air_conditioner.mode === 'cool' ? '制冷' : '制热'
      const level = actions.air_conditioner.level
      result.push({
        key: 'ac-on',
        text: `空调: ${mode} ${level}档`,
        type: 'warning'
      })
    }
  }
  
  // 处理多媒体操作
  if (actions.multimedia && actions.multimedia !== 'no-change') {
    const statusMap = { off: '关闭', standby: '待机', on: '开启' }
    const typeMap = { off: 'info', standby: 'warning', on: 'success' }
    result.push({
      key: 'multimedia',
      text: `多媒体: ${statusMap[actions.multimedia]}`,
      type: typeMap[actions.multimedia]
    })
  }
  
  return result
}

// 重置表单
const resetForm = () => {
  ruleForm.value = {
    name: '',
    description: '',
    scheduleType: 'daily',
    time: '',
    days: [],
    actions: {
      led: {
        led1: 'no-change',
        led2: 'no-change',
        led3: 'no-change',
        led4: 'no-change'
      },
      air_conditioner: {
        state: 'no-change',
        mode: 'cool',
        level: 1
      },
      multimedia: 'no-change'
    }
  }
  editingRule.value = null
}

// 取消编辑
const cancelEdit = () => {
  showCreateDialog.value = false
  resetForm()
}

// 编辑规则
const editRule = (rule) => {
  editingRule.value = rule
  
  // 确保actions结构完整
  const completeActions = {
    led: {
      led1: 'no-change',
      led2: 'no-change',
      led3: 'no-change',
      led4: 'no-change',
      ...rule.actions.led
    },
    air_conditioner: {
      state: 'no-change',
      mode: 'cool',
      level: 1,
      ...rule.actions.air_conditioner
    },
    multimedia: rule.actions.multimedia || 'no-change'
  }
  
  ruleForm.value = {
    name: rule.name,
    description: rule.description || '',
    scheduleType: rule.schedule.type,
    time: rule.schedule.time,
    days: rule.schedule.days || [],
    actions: completeActions
  }
  showCreateDialog.value = true
}

// 保存规则
const saveRule = async () => {
  try {
    await ruleFormRef.value.validate()
    
    // 构建请求数据
    const requestData = {
      name: ruleForm.value.name,
      description: ruleForm.value.description,
      schedule: {
        type: ruleForm.value.scheduleType,
        time: ruleForm.value.time,
        ...(ruleForm.value.scheduleType === 'weekly' && { days: ruleForm.value.days })
      },
      actions: cleanActions(ruleForm.value.actions)
    }
    
    let response
    if (editingRule.value) {
      // 更新规则
      response = await fetch(`/api/automation/rules/${editingRule.value.id}`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestData)
      })
    } else {
      // 创建规则
      response = await fetch('/api/automation/rules', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestData)
      })
    }
    
    const result = await response.json()
    if (response.ok && result.success) {
      ElMessage.success(result.message)
      showCreateDialog.value = false
      resetForm()
      loadRules()
    } else {
      // 显示具体的错误信息，特别是重名错误
      ElMessage.error(result.detail || result.message || '操作失败')
    }
  } catch (error) {
    console.error('保存规则失败:', error)
    ElMessage.error('保存规则失败')
  }
}

// 清理操作数据，移除no-change项
const cleanActions = (actions) => {
  const cleaned = {}
  
  // 处理LED
  const ledActions = {}
  Object.keys(actions.led).forEach(ledName => {
    if (actions.led[ledName] !== 'no-change') {
      ledActions[ledName] = actions.led[ledName]
    }
  })
  if (Object.keys(ledActions).length > 0) {
    cleaned.led = ledActions
  }
  
  // 处理空调
  if (actions.air_conditioner.state !== 'no-change') {
    cleaned.air_conditioner = {
      state: actions.air_conditioner.state,
      ...(actions.air_conditioner.state === 'on' && {
        mode: actions.air_conditioner.mode,
        level: actions.air_conditioner.level
      })
    }
  }
  
  // 处理多媒体
  if (actions.multimedia !== 'no-change') {
    cleaned.multimedia = actions.multimedia
  }
  
  return cleaned
}

// 切换规则状态
const toggleRule = async (rule) => {
  try {
    const response = await fetch(`/api/automation/rules/${rule.id}/toggle`, {
      method: 'POST'
    })
    const result = await response.json()
    if (result.success) {
      ElMessage.success(result.message)
      rule.enabled = result.enabled
    } else {
      ElMessage.error('操作失败')
      // 恢复原状态
      rule.enabled = !rule.enabled
    }
  } catch (error) {
    console.error('切换规则状态失败:', error)
    ElMessage.error('操作失败')
    // 恢复原状态
    rule.enabled = !rule.enabled
  }
}

// 删除规则
const deleteRule = async (rule) => {
  try {
    await ElMessageBox.confirm('确定要删除这个自动化规则吗？', '确认删除', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    })
    
    const response = await fetch(`/api/automation/rules/${rule.id}`, {
      method: 'DELETE'
    })
    const result = await response.json()
    if (result.success) {
      ElMessage.success(result.message)
      loadRules()
    } else {
      ElMessage.error('删除失败')
    }
  } catch (error) {
    if (error !== 'cancel') {
      console.error('删除规则失败:', error)
      ElMessage.error('删除失败')
    }
  }
}

// 暴露方法供外部调用
defineExpose({
  loadRules
})

// 事件处理
const handleRuleCreated = () => {
  console.log('🔄 检测到新规则创建，刷新列表...')
  loadRules()
}

onMounted(() => {
  loadRules()
  // 监听规则创建事件
  on(EVENTS.AUTOMATION_RULE_CREATED, handleRuleCreated)
})

onUnmounted(() => {
  // 清理事件监听器
  off(EVENTS.AUTOMATION_RULE_CREATED, handleRuleCreated)
})
</script>

<style scoped>
.automation-manager {
  max-width: 1000px;
  margin: 0 auto 30px;
}

.automation-manager h3 {
  text-align: center;
  color: #2c3e50;
  font-size: 1.8rem;
  margin-bottom: 25px;
  font-weight: 600;
}

.header-actions {
  margin-bottom: 20px;
  text-align: right;
}



.rules-list {
  display: grid;
  gap: 20px;
}

.rule-card {
  border: none;
  border-radius: 16px;
  overflow: hidden;
  box-shadow: 0 4px 20px rgba(0,0,0,0.08);
  transition: all 0.3s ease;
}

.rule-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 30px rgba(0,0,0,0.12);
}

.rule-card.disabled {
  opacity: 0.6;
}

.rule-card :deep(.el-card__header) {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  padding: 20px 25px;
  border-bottom: none;
}

.rule-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.rule-info h4 {
  margin: 0 0 5px 0;
  font-size: 1.2rem;
  font-weight: 600;
}

.rule-info p {
  margin: 0;
  opacity: 0.9;
  font-size: 0.9rem;
}

.rule-actions {
  display: flex;
  align-items: center;
  gap: 12px;
}

.rule-content {
  display: grid;
  grid-template-columns: 1fr 2fr;
  gap: 20px;
  padding: 20px;
}

.schedule-info h5,
.actions-info h5 {
  margin: 0 0 10px 0;
  color: #2c3e50;
  font-size: 1rem;
}

.action-tags {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.action-tag {
  margin: 0;
}

.empty-state {
  text-align: center;
  padding: 40px;
  color: #909399;
}

.action-controls {
  border: 1px solid #dcdfe6;
  border-radius: 8px;
  padding: 16px;
  background: #f8f9fa;
}

.led-controls {
  display: grid;
  gap: 12px;
}

.led-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 0;
}

.led-item span {
  font-weight: 500;
  min-width: 60px;
}

.ac-controls {
  display: grid;
  gap: 12px;
}

.ac-item {
  display: flex;
  align-items: center;
  gap: 12px;
}

.ac-item span {
  font-weight: 500;
  min-width: 80px;
}

@media (max-width: 768px) {
  .rule-content {
    grid-template-columns: 1fr;
  }
  
  .rule-header {
    flex-direction: column;
    gap: 10px;
    align-items: flex-start;
  }
  
  .rule-actions {
    align-self: flex-end;
  }
}
</style> 