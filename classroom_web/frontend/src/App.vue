<template>
  <div id="app">
    <header class="app-header">
      <h1>🏠 智能教室管理系统</h1>
      <p>实时监控 · 智能控制 · 数据分析</p>
    </header>
    
    <main class="app-main">
      <!-- 导航标签 -->
      <el-tabs v-model="activeTab" class="main-tabs">
        <el-tab-pane label="📊 状态监控" name="status">
          <StatusPanel ref="statusPanel" />
        </el-tab-pane>
        
        <el-tab-pane label="🎛️ 设备控制" name="control">
          <DeviceControl :latest-data="latestData" />
        </el-tab-pane>
        
        <el-tab-pane label="📈 历史数据" name="history">
          <HistoryQuery />
        </el-tab-pane>
        
        <el-tab-pane label="🤖 自动化管理" name="automation">
          <AutomationManager ref="automationManager" />
        </el-tab-pane>
      </el-tabs>
    </main>
    
    <footer class="app-footer">
      <p>&copy; 2024 智能教室管理系统 - 嵌入式小组</p>
    </footer>
    
    <!-- 聊天机器人组件 -->
    <ChatBot />
  </div>
</template>

<script>
import StatusPanel from './components/StatusPanel.vue'
import HistoryQuery from './components/HistoryQuery.vue'
import DeviceControl from './components/DeviceControl.vue'
import ChatBot from './components/ChatBot.vue'
import AutomationManager from './components/AutomationManager.vue'
import axios from 'axios'

export default {
  name: 'App',
  components: {
    StatusPanel,
    HistoryQuery,
    DeviceControl,
    ChatBot,
    AutomationManager
  },
  data() {
    return {
      latestData: {},
      activeTab: 'status'
    }
  },
  methods: {
    fetchLatest() {
      axios.get('/api/latest')
        .then(res => { 
          this.latestData = res.data || {}
          // 也传递给StatusPanel
          if (this.$refs.statusPanel) {
            this.$refs.statusPanel.data = this.latestData
          }
        })
        .catch(err => { console.error('拉取失败', err) })
    }
  },
  mounted() {
    this.fetchLatest()
    this.timer = setInterval(this.fetchLatest, 1000)
  },
  beforeUnmount() {
    clearInterval(this.timer)
  }
}
</script>

<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  min-height: 100vh;
  color: #333;
}

#app {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  background: linear-gradient(135deg, 
    rgba(102, 126, 234, 0.1) 0%, 
    rgba(118, 75, 162, 0.1) 100%),
    radial-gradient(circle at top right, 
    rgba(255, 255, 255, 0.8) 0%, 
    rgba(255, 255, 255, 0.6) 100%);
}

.app-header {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  text-align: center;
  padding: 40px 20px;
  box-shadow: 0 4px 20px rgba(0,0,0,0.1);
  position: relative;
  overflow: hidden;
}

.app-header::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(45deg, transparent, rgba(255,255,255,0.1), transparent);
  animation: headerShine 4s infinite;
}

@keyframes headerShine {
  0% { transform: translateX(-100%); }
  100% { transform: translateX(100%); }
}

.app-header h1 {
  font-size: 2.5rem;
  font-weight: 700;
  margin-bottom: 10px;
  text-shadow: 0 2px 4px rgba(0,0,0,0.3);
  position: relative;
  z-index: 1;
}

.app-header p {
  font-size: 1.1rem;
  opacity: 0.9;
  position: relative;
  z-index: 1;
}

.app-main {
  flex: 1;
  padding: 40px 20px;
  max-width: 1200px;
  margin: 0 auto;
  width: 100%;
}

.main-tabs {
  background: rgba(255, 255, 255, 0.9);
  border-radius: 16px;
  padding: 20px;
  box-shadow: 0 4px 20px rgba(0,0,0,0.08);
}

.main-tabs :deep(.el-tabs__header) {
  margin-bottom: 20px;
}

.main-tabs :deep(.el-tabs__nav-wrap) {
  background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
  border-radius: 12px;
  padding: 8px;
}

.main-tabs :deep(.el-tabs__item) {
  border-radius: 8px;
  margin: 0 4px;
  transition: all 0.3s ease;
  font-weight: 500;
  color: #666;
}

.main-tabs :deep(.el-tabs__item:hover) {
  background: rgba(102, 126, 234, 0.1);
  color: #667eea;
}

.main-tabs :deep(.el-tabs__item.is-active) {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

.main-tabs :deep(.el-tabs__active-bar) {
  display: none;
}

.app-footer {
  background: rgba(255, 255, 255, 0.9);
  color: #666;
  text-align: center;
  padding: 20px;
  border-top: 1px solid #e0e0e0;
}

/* 滚动条美化 */
::-webkit-scrollbar {
  width: 8px;
}

::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 4px;
}

::-webkit-scrollbar-thumb {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
  background: linear-gradient(135deg, #5a67d8 0%, #6b46c1 100%);
}

@media (max-width: 768px) {
  .app-header {
    padding: 30px 15px;
  }
  
  .app-header h1 {
    font-size: 2rem;
  }
  
  .app-header p {
    font-size: 1rem;
  }
  
  .app-main {
    padding: 20px 15px;
  }
}
</style>