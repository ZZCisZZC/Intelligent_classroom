// eventBus.js - 简单的事件总线
import { ref } from 'vue'

// 创建一个响应式的事件总线
const eventBus = ref(new Map())

// 事件监听器
export const on = (event, callback) => {
  if (!eventBus.value.has(event)) {
    eventBus.value.set(event, [])
  }
  eventBus.value.get(event).push(callback)
}

// 移除事件监听器
export const off = (event, callback) => {
  if (eventBus.value.has(event)) {
    const callbacks = eventBus.value.get(event)
    const index = callbacks.indexOf(callback)
    if (index > -1) {
      callbacks.splice(index, 1)
    }
  }
}

// 触发事件
export const emit = (event, data) => {
  if (eventBus.value.has(event)) {
    eventBus.value.get(event).forEach(callback => {
      callback(data)
    })
  }
}

// 预定义的事件类型
export const EVENTS = {
  AUTOMATION_RULE_CREATED: 'automation_rule_created',
  AUTOMATION_RULE_UPDATED: 'automation_rule_updated',
  AUTOMATION_RULE_DELETED: 'automation_rule_deleted'
} 