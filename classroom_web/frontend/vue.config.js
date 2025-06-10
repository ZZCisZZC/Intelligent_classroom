const { defineConfig } = require('@vue/cli-service')
// module.exports = defineConfig({
//   transpileDependencies: true
// })
// vue.config.js
module.exports = {
  devServer: {
    // 允许任何Host访问，解决反向代理的Host Header问题
    allowedHosts: 'all',
    
    // 监听所有网络接口，允许外部访问
    host: '0.0.0.0',
    port: 8080,
    
    // 启用HTTPS，使WebSocket使用WSS协议
    https: true,
    
    // 配置WebSocket URL使用WSS协议
    client: {
      webSocketURL: 'wss://bitqianrushi.oneclickai.top/ws'
    },
    
    proxy: {
      // 只要请求以 /api 开头，就走下面这个代理
      '/api': {
        target: 'http://127.0.0.1:8000',  // 你的 FastAPI 地址
        changeOrigin: true,
        pathRewrite: {
          '^/api': ''   // 把路径前缀 /api 去掉，真正发给后端的是 /latest
        },
      },
    },
  },
}