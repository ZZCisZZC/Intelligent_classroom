<template>
  <div class="chatbot-container">
    <!-- 聊天按钮 -->
    <div 
      class="chat-button" 
      @click="toggleChat"
      :class="{ 'has-new-message': hasNewMessage }"
    >
      <div class="chat-icon">
        <svg v-if="!isChatOpen" viewBox="0 0 24 24" fill="currentColor">
          <path d="M20 2H4c-1.1 0-2 .9-2 2v18l4-4h14c1.1 0 2-.9 2-2V4c0-1.1-.9-2-2-2zm-2 12H6v-2h12v2zm0-3H6V9h12v2zm0-3H6V6h12v2z"/>
        </svg>
        <svg v-else viewBox="0 0 24 24" fill="currentColor">
          <path d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
        </svg>
      </div>
      <div class="chat-text">智能助手</div>
    </div>

    <!-- 聊天窗口 -->
    <transition name="chat-window">
      <div v-if="isChatOpen" class="chat-window">
        <div class="chat-header">
          <div class="header-content">
            <div>
              <h3>🤖 智能教室助手</h3>
              <p>我可以帮您控制设备和查询状态</p>
            </div>
            <button @click="clearConversation" class="clear-btn" title="清空对话">
              <svg viewBox="0 0 24 24" fill="currentColor">
                <path d="M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19A2,2 0 0,0 8,21H16A2,2 0 0,0 18,19V7H6V19Z"/>
              </svg>
            </button>
          </div>
        </div>
        
        <div class="chat-messages" ref="messagesContainer">
          <div v-if="messages.length === 0" class="welcome-message">
            <div class="ai-message">
              <div class="message-content">
                👋 您好！我是智能教室助手。<br/>
                您可以问我：<br/>
                • "现在温度怎么样？"<br/>
                • "帮我开灯"<br/>
                • "我有点热"<br/>
                • "关闭所有设备"
              </div>
            </div>
          </div>
          
          <div 
            v-for="(msg, index) in messages" 
            :key="index" 
            class="message"
            :class="{ 'user-message': msg.type === 'user', 'ai-message': msg.type === 'ai' }"
          >
            <div class="message-content">
              <div 
                v-if="msg.type === 'ai'" 
                v-html="renderMarkdown(msg.content)"
                class="markdown-content"
              ></div>
              <div v-else>{{ msg.content }}</div>
            </div>
            <div class="message-time">{{ formatTime(msg.timestamp) }}</div>
          </div>
          
          <div v-if="isLoading" class="ai-message loading">
            <div class="message-content">
              <div class="typing-indicator">
                <span></span>
                <span></span>
                <span></span>
              </div>
              正在思考中...
            </div>
          </div>
        </div>
        
        <div class="chat-input">
          <div class="input-wrapper">
            <input 
              v-model="inputMessage" 
              @keypress.enter="sendMessage"
              @input="hasNewMessage = false"
              placeholder="请输入您的问题..." 
              :disabled="isLoading"
              ref="messageInput"
            />
            <button 
              @click="sendMessage" 
              :disabled="!inputMessage.trim() || isLoading"
              class="send-button"
            >
              <svg viewBox="0 0 24 24" fill="currentColor">
                <path d="M2.01 21L23 12 2.01 3 2 10l15 2-15 2z"/>
              </svg>
            </button>
          </div>
        </div>
      </div>
    </transition>
  </div>
</template>

<script>
import MarkdownIt from 'markdown-it'
import { emit, EVENTS } from '../utils/eventBus.js'

export default {
  name: 'ChatBot',
  data() {
    return {
      isChatOpen: false,
      messages: [],
      inputMessage: '',
      isLoading: false,
      hasNewMessage: false,
      md: new MarkdownIt({
        html: false,
        linkify: true,
        typographer: true,
        breaks: true
      })
    }
  },
  methods: {
    toggleChat() {
      this.isChatOpen = !this.isChatOpen
      if (this.isChatOpen) {
        // 每次打开聊天窗口都清空消息，开始新对话
        this.messages = []
        this.hasNewMessage = false
        console.log('🆕 打开聊天窗口，开始新对话')
        
        this.$nextTick(() => {
          if (this.$refs.messageInput) {
            this.$refs.messageInput.focus()
          }
        })
      }
    },
    
    async sendMessage() {
      if (!this.inputMessage.trim() || this.isLoading) return
      
      const userMessage = this.inputMessage.trim()
      this.inputMessage = ''
      
      // 添加用户消息
      this.messages.push({
        type: 'user',
        content: userMessage,
        timestamp: new Date()
      })
      
      this.isLoading = true
      this.scrollToBottom()
      
      try {
        // 使用普通的聊天接口
        const response = await fetch('/api/chat', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            message: userMessage,
            history: this.buildHistory()
          })
        })
        
        const data = await response.json()
        
        if (data.reply) {
          // 添加AI回复
          this.messages.push({
            type: 'ai',
            content: data.reply,
            timestamp: new Date()
          })
          
          // 检查是否创建了自动化规则
          if (data.action_taken && data.action_taken.success && 
              (data.reply.includes('自动化规则创建成功') || 
               data.reply.includes('自动化规则') || 
               data.action_taken.detailed_message)) {
            console.log('🎯 AI创建了自动化规则，通知更新...')
            emit(EVENTS.AUTOMATION_RULE_CREATED, data.action_taken)
          }
        } else {
          // 错误处理
          this.messages.push({
            type: 'ai',
            content: '抱歉，出现了错误，请稍后重试。',
            timestamp: new Date()
          })
        }
        
      } catch (error) {
        console.error('聊天请求失败:', error)
        this.messages.push({
          type: 'ai',
          content: '抱歉，连接出现问题，请稍后重试。',
          timestamp: new Date()
        })
      } finally {
        this.isLoading = false
        this.scrollToBottom()
      }
    },
    
    formatTime(timestamp) {
      return timestamp.toLocaleTimeString('zh-CN', { 
        hour: '2-digit', 
        minute: '2-digit' 
      })
    },
    
    scrollToBottom() {
      this.$nextTick(() => {
        if (this.$refs.messagesContainer) {
          this.$refs.messagesContainer.scrollTop = this.$refs.messagesContainer.scrollHeight
        }
      })
    },
    
    clearConversation() {
      this.messages = []
      this.hasNewMessage = false
      console.log('🗑️ 对话已清空')
    },
    
    renderMarkdown(content) {
      let html = this.md.render(content)
      // 给表格添加包装容器
      html = html.replace(/<table>/g, '<div class="table-wrapper"><table>')
      html = html.replace(/<\/table>/g, '</table></div>')
      return html
    },
    
    buildHistory() {
      // 构建对话历史（最多保留最近2轮对话）
      const chatHistory = []
      const recentMessages = this.messages.slice(-4) // 最多4条消息（2轮对话）
      
      for (const msg of recentMessages) {
        if (msg.type === 'user') {
          chatHistory.push({ role: 'user', content: msg.content })
        } else if (msg.type === 'ai' && msg.content) {
          chatHistory.push({ role: 'assistant', content: msg.content })
        }
      }
      
      return chatHistory
    }
  }
}
</script>

<style scoped>
.chatbot-container {
  position: fixed;
  bottom: 20px;
  right: 20px;
  z-index: 1000;
}

.chat-button {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  width: 120px;
  height: 60px;
  border-radius: 30px;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  box-shadow: 0 4px 20px rgba(102, 126, 234, 0.3);
  transition: all 0.3s ease;
  gap: 8px;
  position: relative;
  overflow: hidden;
}

.chat-button:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 25px rgba(102, 126, 234, 0.4);
}

.chat-button.has-new-message::before {
  content: '';
  position: absolute;
  top: 5px;
  right: 5px;
  width: 10px;
  height: 10px;
  background: #ff4757;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0% {
    transform: scale(1);
    opacity: 1;
  }
  50% {
    transform: scale(1.2);
    opacity: 0.7;
  }
  100% {
    transform: scale(1);
    opacity: 1;
  }
}

.chat-icon {
  width: 24px;
  height: 24px;
}

.chat-text {
  font-size: 14px;
  font-weight: 600;
}

.chat-window {
  position: absolute;
  bottom: 80px;
  right: 0;
  width: 480px;
  height: 650px;
  background: white;
  border-radius: 16px;
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.15);
  display: flex;
  flex-direction: column;
  overflow: hidden;
  border: 1px solid #e0e0e0;
  max-width: calc(100vw - 30px);
}

.chat-window-enter-active, .chat-window-leave-active {
  transition: all 0.3s ease;
}

.chat-window-enter-from, .chat-window-leave-to {
  opacity: 0;
  transform: translateY(20px) scale(0.95);
}

.chat-header {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  padding: 20px;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.header-content > div {
  text-align: center;
  flex: 1;
}

.clear-btn {
  background: rgba(255, 255, 255, 0.2);
  border: none;
  border-radius: 50%;
  width: 36px;
  height: 36px;
  color: white;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: background 0.2s;
}

.clear-btn:hover {
  background: rgba(255, 255, 255, 0.3);
}

.clear-btn svg {
  width: 18px;
  height: 18px;
}

.chat-header h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
}

.chat-header p {
  margin: 5px 0 0 0;
  font-size: 12px;
  opacity: 0.9;
}

.chat-messages {
  flex: 1;
  padding: 16px;
  overflow-y: auto;
  background: #f8f9fa;
}

.welcome-message {
  margin-bottom: 16px;
}

.message {
  margin-bottom: 16px;
  display: flex;
  flex-direction: column;
}

.user-message {
  align-items: flex-end;
}

.ai-message {
  align-items: flex-start;
}

.message-content {
  max-width: 80%;
  padding: 12px 16px;
  border-radius: 18px;
  font-size: 14px;
  line-height: 1.4;
  word-wrap: break-word;
  overflow-wrap: break-word;
}

.user-message .message-content {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border-bottom-right-radius: 4px;
}

.ai-message .message-content {
  background: white;
  color: #333;
  border: 1px solid #e0e0e0;
  border-bottom-left-radius: 4px;
  max-width: 80%;
  width: fit-content;
  box-sizing: border-box;
}

.message-time {
  font-size: 11px;
  color: #999;
  margin-top: 4px;
  padding: 0 4px;
}

.loading .message-content {
  display: flex;
  align-items: center;
  gap: 8px;
}

.typing-indicator {
  display: flex;
  gap: 4px;
}

.typing-indicator span {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: #667eea;
  animation: typing 1.4s infinite ease-in-out both;
}

.typing-indicator span:nth-child(1) { animation-delay: -0.32s; }
.typing-indicator span:nth-child(2) { animation-delay: -0.16s; }

@keyframes typing {
  0%, 80%, 100% {
    transform: scale(0);
    opacity: 0.5;
  }
  40% {
    transform: scale(1);
    opacity: 1;
  }
}

.chat-input {
  padding: 16px;
  background: white;
  border-top: 1px solid #e0e0e0;
}

.input-wrapper {
  display: flex;
  gap: 8px;
  align-items: center;
}

.input-wrapper input {
  flex: 1;
  padding: 12px 16px;
  border: 1px solid #ddd;
  border-radius: 22px;
  font-size: 14px;
  outline: none;
  transition: border-color 0.2s;
}

.input-wrapper input:focus {
  border-color: #667eea;
}

.input-wrapper input:disabled {
  background: #f5f5f5;
  cursor: not-allowed;
}

.send-button {
  width: 44px;
  height: 44px;
  border: none;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;
}

.send-button:hover:not(:disabled) {
  transform: scale(1.05);
}

.send-button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.send-button svg {
  width: 20px;
  height: 20px;
}

/* 移动端适配 */
@media (max-width: 768px) {
  .chatbot-container {
    position: fixed;
    bottom: 10px;
    right: 10px;
    left: 10px;
    z-index: 1000;
  }
  
  .chat-window {
    width: 100%;
    height: 70vh;
    max-height: 500px;
    right: 0;
    left: 0;
    bottom: 60px;
    margin: 0;
    border-radius: 12px;
  }
  
  .chat-button {
    width: 100px;
    height: 50px;
    font-size: 12px;
    position: absolute;
    right: 0;
    bottom: 0;
  }
  
  .chat-icon {
    width: 20px;
    height: 20px;
  }
}

/* 小屏手机适配 */
@media (max-width: 480px) {
  .chatbot-container {
    bottom: 5px;
    right: 5px;
    left: 5px;
  }
  
  .chat-window {
    height: 65vh;
    max-height: 450px;
    bottom: 55px;
  }
  
  .chat-button {
    width: 90px;
    height: 45px;
    font-size: 11px;
  }
}

/* 桌面端大屏优化 */
@media (min-width: 1200px) {
  .chat-window {
    width: 520px;
    height: 600px;
  }
}

.chat-messages::-webkit-scrollbar {
  width: 4px;
}

.chat-messages::-webkit-scrollbar-track {
  background: transparent;
}

.chat-messages::-webkit-scrollbar-thumb {
  background: #ddd;
  border-radius: 2px;
}

.chat-messages::-webkit-scrollbar-thumb:hover {
  background: #bbb;
}

/* Markdown 样式 */
.markdown-content {
  line-height: 1.6;
  word-wrap: break-word;
  word-break: break-word;
  overflow-wrap: break-word;
  max-width: 100%;
}

.markdown-content h1,
.markdown-content h2,
.markdown-content h3,
.markdown-content h4,
.markdown-content h5,
.markdown-content h6 {
  margin: 16px 0 8px 0;
  font-weight: 600;
  color: #2c3e50;
}

.markdown-content h1 { font-size: 1.5em; }
.markdown-content h2 { font-size: 1.3em; }
.markdown-content h3 { font-size: 1.2em; }
.markdown-content h4 { font-size: 1.1em; }
.markdown-content h5 { font-size: 1.05em; }
.markdown-content h6 { font-size: 1em; }

.markdown-content p {
  margin: 8px 0;
  color: #333;
}

.markdown-content ul,
.markdown-content ol {
  margin: 8px 0;
  padding-left: 0;
  list-style-position: outside;
}

.markdown-content li {
  margin: 4px 0;
  word-wrap: break-word;
  overflow-wrap: break-word;
  position: relative;
  padding-left: 16px;
}

.markdown-content ul {
  list-style-type: none;
}

.markdown-content ul li::before {
  content: "• ";
  color: #667eea;
  position: relative;
  left: 0;
  top: 0;
}

.markdown-content strong {
  font-weight: 600;
  color: #2c3e50;
}

.markdown-content em {
  font-style: italic;
  color: #7f8c8d;
}

.markdown-content code {
  background: #f8f9fa;
  padding: 2px 6px;
  border-radius: 4px;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 0.9em;
  color: #e74c3c;
  border: 1px solid #e9ecef;
}

.markdown-content pre {
  background: #f8f9fa;
  padding: 12px;
  border-radius: 8px;
  overflow-x: auto;
  margin: 12px 0;
  border: 1px solid #e9ecef;
  max-width: 100%;
  word-wrap: break-word;
  white-space: pre-wrap;
}

.markdown-content pre code {
  background: none;
  padding: 0;
  border: none;
  color: #2c3e50;
}

.markdown-content blockquote {
  border-left: 4px solid #667eea;
  margin: 12px 0;
  padding: 8px 16px;
  background: #f8f9fd;
  color: #5a67d8;
  font-style: italic;
}

/* 表格容器 */
.markdown-content .table-wrapper {
  overflow-x: auto;
  margin: 12px 0;
  max-width: 100%;
  width: 100%;
  box-sizing: border-box;
}

.markdown-content table {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.85em;
  min-width: 300px;
}

.markdown-content th,
.markdown-content td {
  border: 1px solid #e9ecef;
  padding: 8px 12px;
  text-align: left;
}

.markdown-content th {
  background: #f8f9fa;
  font-weight: 600;
  color: #2c3e50;
}

.markdown-content tr:nth-child(even) {
  background: #fdfdfe;
}

.markdown-content a {
  color: #667eea;
  text-decoration: none;
}

.markdown-content a:hover {
  text-decoration: underline;
}

.markdown-content hr {
  border: none;
  border-top: 2px solid #e9ecef;
  margin: 16px 0;
}

/* 节能报告特殊样式 */
.markdown-content .energy-report {
  background: #f0f8ff;
  border: 1px solid #b3d9ff;
  border-radius: 8px;
  padding: 16px;
  margin: 12px 0;
}

.markdown-content .energy-summary {
  background: #fff5f5;
  border-left: 4px solid #e53e3e;
  padding: 12px 16px;
  margin: 8px 0;
}

.markdown-content .energy-recommendation {
  background: #f0fff4;
  border-left: 4px solid #38a169;
  padding: 12px 16px;
  margin: 8px 0;
}

.streaming-cursor {
  display: inline-block;
  animation: blink 1s infinite;
  color: #667eea;
  margin-left: 2px;
}

@keyframes blink {
  0%, 50% { opacity: 1; }
  51%, 100% { opacity: 0; }
}
</style> 