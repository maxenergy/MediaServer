import axios from 'axios'
import { ElMessage } from 'element-plus'

// 创建axios实例
const api = axios.create({
  baseURL: '/api/v1',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器
api.interceptors.request.use(
  config => {
    return config
  },
  error => {
    return Promise.reject(error)
  }
)

// 响应拦截器
api.interceptors.response.use(
  response => {
    const { data } = response
    if (data.code && data.code !== '200') {
      ElMessage.error(data.msg || '请求失败')
      return Promise.reject(new Error(data.msg || '请求失败'))
    }
    return data
  },
  error => {
    ElMessage.error(error.message || '网络错误')
    return Promise.reject(error)
  }
)

// API接口定义
export const configAPI = {
  // 获取配置
  getConfig: () => api.get('/config'),
  // 更新配置
  updateConfig: (data) => api.post('/config', data)
}

export const streamAPI = {
  // 获取流列表
  getSourceList: () => api.get('/getSourceList'),
  // 获取流信息
  getSourceInfo: (params) => api.get('/getSourceInfo', { params }),
  // 关闭流
  closeSource: (data) => api.post('/closeSource', data),
  // 获取客户端列表
  getClientList: (params) => api.get('/getClientList', { params }),
  // 关闭客户端
  closeClient: (data) => api.post('/closeClient', data),
  // 获取关键帧
  getKeyframe: (params) => api.get('/streams/keyframe', { params }),
  // 设置时间戳模式
  setStampMode: (data) => api.post('/streams/setStampMode', data)
}

export const serverAPI = {
  // 获取服务器信息
  getServerInfo: () => api.get('/getServerInfo'),
  // 获取版本信息
  getVersion: () => api.get('/version'),
  // 获取事件循环列表
  getLoopList: () => api.get('/getLoopList'),
  // 退出服务器
  exitServer: () => api.post('/exitServer')
}

export const rtspAPI = {
  // 创建RTSP流
  createStream: (data) => api.post('/rtsp/streams/create', data),
  // 开始RTSP拉流
  startPlay: (data) => api.post('/rtsp/play/start', data),
  // 停止RTSP拉流
  stopPlay: (data) => api.post('/rtsp/play/stop', data),
  // 获取RTSP拉流列表
  getPlayList: () => api.get('/rtsp/play/list'),
  // 开始RTSP推流
  startPublish: (data) => api.post('/rtsp/publish/start', data),
  // 停止RTSP推流
  stopPublish: (data) => api.post('/rtsp/publish/stop', data),
  // 获取RTSP推流列表
  getPublishList: () => api.get('/rtsp/publish/list'),
  // 创建RTSP服务器
  createServer: (data) => api.post('/rtsp/server/create', data),
  // 停止RTSP服务器
  stopServer: (data) => api.post('/rtsp/server/stop', data)
}

export const rtmpAPI = {
  // 创建RTMP流
  createStream: (data) => api.post('/rtmp/create', data),
  // 开始RTMP拉流
  startPlay: (data) => api.post('/rtmp/play/start', data),
  // 停止RTMP拉流
  stopPlay: (data) => api.post('/rtmp/play/stop', data),
  // 获取RTMP拉流列表
  getPlayList: () => api.get('/rtmp/play/list'),
  // 开始RTMP推流
  startPublish: (data) => api.post('/rtmp/publish/start', data),
  // 停止RTMP推流
  stopPublish: (data) => api.post('/rtmp/publish/stop', data),
  // 获取RTMP推流列表
  getPublishList: () => api.get('/rtmp/publish/list')
}

export const webrtcAPI = {
  // WebRTC播放
  play: (data) => api.post('/rtc/play', data),
  // WebRTC推流
  publish: (data) => api.post('/rtc/publish', data),
  // WHEP播放 (WebRTC-HTTP Egress Protocol)
  whepPlay: (data) => api.post('/rtc/whep', data),
  // WHIP推流 (WebRTC-HTTP Ingress Protocol)
  whipPublish: (data) => api.post('/rtc/whip', data),
  // 开始WebRTC拉流
  startPull: (data) => api.post('/rtc/pull/start', data),
  // 停止WebRTC拉流
  stopPull: (data) => api.post('/rtc/pull/stop', data),
  // 获取WebRTC拉流列表
  getPullList: () => api.get('/rtc/pull/list'),
  // 开始WebRTC推流
  startPush: (data) => api.post('/rtc/push/start', data),
  // 停止WebRTC推流
  stopPush: (data) => api.post('/rtc/push/stop', data),
  // 获取WebRTC推流列表
  getPushList: () => api.get('/rtc/push/list')
}

export const srtAPI = {
  // 开始SRT拉流
  startPull: (data) => api.post('/srt/pull/start', data),
  // 停止SRT拉流
  stopPull: (data) => api.post('/srt/pull/stop', data),
  // 获取SRT拉流列表
  getPullList: () => api.get('/srt/pull/list'),
  // 开始SRT推流
  startPush: (data) => api.post('/srt/push/start', data),
  // 停止SRT推流
  stopPush: (data) => api.post('/srt/push/stop', data),
  // 获取SRT推流列表
  getPushList: () => api.get('/srt/push/list')
}

export const gb28181API = {
  // 创建GB28181接收器
  createReceiver: (data) => api.post('/gb28181/recv/create', data),
  // 创建GB28181发送器
  createSender: (data) => api.post('/gb28181/send/create', data),
  // 停止GB28181接收器
  stopReceiver: (data) => api.post('/gb28181/recv/stop', data),
  // 停止GB28181发送器
  stopSender: (data) => api.post('/gb28181/send/stop', data)
}

export const jt1078API = {
  // 创建JT1078
  create: (data) => api.post('/jt1078/create', data),
  // 删除JT1078
  delete: (data) => api.post('/jt1078/delete', data),
  // 开启JT1078服务器
  openServer: (data) => api.post('/jt1078/server/open', data),
  // 关闭JT1078服务器
  closeServer: (data) => api.post('/jt1078/server/close', data),
  // 开始对讲
  startTalk: (data) => api.post('/jt1078/talk/start', data),
  // 停止对讲
  stopTalk: (data) => api.post('/jt1078/talk/stop', data),
  // 开始发送
  startSend: (data) => api.post('/jt1078/send/start', data),
  // 停止发送
  stopSend: (data) => api.post('/jt1078/send/stop', data),
  // 获取端口信息
  getPortInfo: () => api.get('/jt1078/port/info')
}

export const recordAPI = {
  // 开始录制
  startRecord: (data) => api.post('/record/start', data),
  // 停止录制
  stopRecord: (data) => api.post('/record/stop', data),
  // 获取录制列表
  getRecordList: () => api.get('/record/list')
}

export const vodAPI = {
  // 开始点播
  start: (data) => api.post('/vod/start', data),
  // 停止点播
  stop: (data) => api.post('/vod/stop', data),
  // 控制点播
  control: (data) => api.post('/vod/control', data)
}

// RTP API - 实时传输协议
export const rtpAPI = {
  // 创建RTP接收器
  createReceiver: (data) => api.post('/rtp/recv/create', data),
  // 创建RTP发送器
  createSender: (data) => api.post('/rtp/send/create', data),
  // 启动RTP发送器
  startSender: (data) => api.post('/rtp/send/start', data),
  // 停止RTP接收器
  stopReceiver: (data) => api.post('/rtp/recv/stop', data),
  // 停止RTP发送器
  stopSender: (data) => api.post('/rtp/send/stop', data)
}

// HTTP流媒体API - FLV/HLS/PS VOD
export const httpStreamAPI = {
  // FLV播放相关
  flv: {
    // 开始FLV播放
    startPlay: (data) => api.post('/http/flv/play/start', data),
    // 停止FLV播放
    stopPlay: (data) => api.post('/http/flv/play/stop', data),
    // 获取FLV播放列表
    getPlayList: () => api.get('/http/flv/play/list')
  },
  // HLS播放相关
  hls: {
    // 开始HLS播放
    startPlay: (data) => api.post('/http/hls/play/start', data),
    // 停止HLS播放
    stopPlay: (data) => api.post('/http/hls/play/stop', data),
    // 获取HLS播放列表
    getPlayList: () => api.get('/http/hls/play/list')
  },
  // PS VOD播放相关
  psVod: {
    // 开始PS VOD播放
    startPlay: (data) => api.post('/http/ps/vod/play/start', data),
    // 停止PS VOD播放
    stopPlay: (data) => api.post('/http/ps/vod/play/stop', data),
    // 获取PS VOD播放列表
    getPlayList: () => api.get('/http/ps/vod/play/list')
  }
}

// FFmpeg API - 转码任务管理
export const ffmpegAPI = {
  // 添加转码任务
  addTask: (data) => api.post('/ffmpeg/task/add', data),
  // 删除转码任务
  deleteTask: (data) => api.post('/ffmpeg/task/del', data),
  // 重新配置转码任务
  reconfigTask: (data) => api.post('/ffmpeg/task/reconfig', data)
}

// Hook API - 事件钩子
export const hookAPI = {
  // 流状态变化钩子
  onStreamStatus: (data) => api.post('/onStreamStatus', data),
  // 推流鉴权钩子
  onPublish: (data) => api.post('/onPublish', data),
  // 播放鉴权钩子
  onPlay: (data) => api.post('/onPlay', data),
  // 无播放者钩子
  onNonePlayer: (data) => api.post('/onNonePlayer', data),
  // 保活钩子
  onKeepAlive: (data) => api.post('/onKeepAlive', data)
}

// Test API - 测试接口
export const testAPI = {
  // 添加工作任务
  addWorkTask: () => api.post('/test/work/task/add'),
  // 添加有序工作任务
  addWorkOrderTask: () => api.post('/test/work/ordertask/add')
}

// Websocket API - WebSocket推流
export const websocketAPI = {
  // 开始WebSocket推流
  startPublish: (data) => api.post('/websocket/publish/start', data)
}

export default api
