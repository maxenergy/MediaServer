<template>
  <div class="live-view">
    <!-- 顶部工具栏 -->
    <div class="toolbar">
      <div class="toolbar-left">
        <el-button-group>
          <el-button :type="viewMode === '1x1' ? 'primary' : ''" @click="setViewMode('1x1')">1x1</el-button>
          <el-button :type="viewMode === '2x2' ? 'primary' : ''" @click="setViewMode('2x2')">2x2</el-button>
          <el-button :type="viewMode === '3x3' ? 'primary' : ''" @click="setViewMode('3x3')">3x3</el-button>
          <el-button :type="viewMode === '4x4' ? 'primary' : ''" @click="setViewMode('4x4')">4x4</el-button>
        </el-button-group>

        <el-divider direction="vertical" />

        <el-button @click="toggleFullscreen" :icon="isFullscreen ? 'FullScreen' : 'Rank'">
          {{ isFullscreen ? '退出全屏' : '全屏' }}
        </el-button>

        <el-button @click="toggleAI" :type="aiEnabled ? 'success' : ''" :icon="aiEnabled ? 'View' : 'Hide'">
          AI检测: {{ aiEnabled ? '开启' : '关闭' }}
        </el-button>
      </div>

      <div class="toolbar-right">
        <el-select v-model="selectedCamera" placeholder="选择摄像头" style="width: 200px" @change="addCameraToView">
          <el-option
            v-for="camera in availableCameras"
            :key="camera.id"
            :label="camera.name"
            :value="camera.id"
          />
        </el-select>

        <el-button @click="refreshStreams" :icon="'Refresh'">刷新</el-button>
      </div>
    </div>

    <!-- 视频网格 -->
    <div class="video-grid" :class="`grid-${viewMode}`" ref="videoGrid">
      <div
        v-for="(slot, index) in videoSlots"
        :key="index"
        class="video-slot"
        :class="{ 'active': slot.active, 'has-stream': slot.stream }"
        @click="selectSlot(index)"
        @dblclick="toggleSlotFullscreen(index)"
      >
        <!-- 视频播放器 -->
        <div v-if="slot.stream" class="video-player">
          <video
            :ref="`video-${index}`"
            class="video-element"
            autoplay
            muted
            :poster="slot.stream.snapshot"
          >
            <source :src="slot.stream.url" type="application/x-mpegURL">
            您的浏览器不支持视频播放
          </video>

          <!-- AI检测覆盖层 -->
          <div v-if="aiEnabled && slot.detections" class="ai-overlay">
            <div
              v-for="detection in slot.detections"
              :key="detection.id"
              class="detection-box"
              :style="getDetectionStyle(detection)"
            >
              <div class="detection-label">
                {{ detection.class_name }} ({{ Math.round(detection.confidence * 100) }}%)
              </div>
            </div>
          </div>

          <!-- 视频控制栏 -->
          <div class="video-controls">
            <div class="controls-left">
              <span class="camera-name">{{ slot.stream.name }}</span>
              <span class="stream-status" :class="slot.stream.status">
                {{ slot.stream.status === 'online' ? '在线' : '离线' }}
              </span>
            </div>
            <div class="controls-right">
              <el-button size="small" @click="toggleAudio(index)" :icon="slot.audioEnabled ? 'VideoPlay' : 'Mute'">
              </el-button>
              <el-button size="small" @click="takeSnapshot(index)" :icon="'Camera'">
              </el-button>
              <el-button size="small" @click="startRecording(index)" :icon="'VideoCamera'"
                :type="slot.recording ? 'danger' : ''">
              </el-button>
              <el-button size="small" @click="showPTZControl(index)" :icon="'Aim'">
              </el-button>
            </div>
          </div>
        </div>

        <!-- 空槽位 -->
        <div v-else class="empty-slot">
          <el-icon class="empty-icon"><VideoCamera /></el-icon>
          <p>点击添加摄像头</p>
          <p class="slot-number">{{ index + 1 }}</p>
        </div>
      </div>
    </div>

    <!-- PTZ控制面板 -->
    <el-drawer v-model="ptzDrawer" title="PTZ控制" direction="rtl" size="300px">
      <div class="ptz-control">
        <div class="ptz-direction">
          <div class="ptz-row">
            <el-button @click="ptzMove('up-left')" :icon="'Top'" size="large"></el-button>
            <el-button @click="ptzMove('up')" :icon="'Top'" size="large"></el-button>
            <el-button @click="ptzMove('up-right')" :icon="'Top'" size="large"></el-button>
          </div>
          <div class="ptz-row">
            <el-button @click="ptzMove('left')" :icon="'Back'" size="large"></el-button>
            <el-button @click="ptzMove('home')" :icon="'Aim'" size="large"></el-button>
            <el-button @click="ptzMove('right')" :icon="'Right'" size="large"></el-button>
          </div>
          <div class="ptz-row">
            <el-button @click="ptzMove('down-left')" :icon="'Bottom'" size="large"></el-button>
            <el-button @click="ptzMove('down')" :icon="'Bottom'" size="large"></el-button>
            <el-button @click="ptzMove('down-right')" :icon="'Bottom'" size="large"></el-button>
          </div>
        </div>

        <div class="ptz-zoom">
          <el-row :gutter="10">
            <el-col :span="12">
              <el-button @click="ptzZoom('in')" size="large" style="width: 100%">放大</el-button>
            </el-col>
            <el-col :span="12">
              <el-button @click="ptzZoom('out')" size="large" style="width: 100%">缩小</el-button>
            </el-col>
          </el-row>
        </div>

        <div class="ptz-presets">
          <h4>预置位</h4>
          <el-row :gutter="10">
            <el-col :span="8" v-for="i in 6" :key="i">
              <el-button @click="gotoPreset(i)" size="small" style="width: 100%">{{ i }}</el-button>
            </el-col>
          </el-row>
        </div>
      </div>
    </el-drawer>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'

// 响应式数据
const viewMode = ref('2x2')
const isFullscreen = ref(false)
const aiEnabled = ref(true)
const selectedCamera = ref('')
const ptzDrawer = ref(false)
const currentPTZSlot = ref(-1)

// 可用摄像头列表
const availableCameras = ref([
  { id: 'camera1', name: '前门摄像头', url: 'rtsp://demo:demo@ipvmdemo.dyndns.org:5541/onvif-media/media.amp', status: 'online' },
  { id: 'camera2', name: '后门摄像头', url: 'rtsp://demo:demo@ipvmdemo.dyndns.org:5542/onvif-media/media.amp', status: 'online' },
  { id: 'camera3', name: '停车场摄像头', url: 'rtsp://demo:demo@ipvmdemo.dyndns.org:5543/onvif-media/media.amp', status: 'offline' },
  { id: 'camera4', name: '办公室摄像头', url: 'rtsp://demo:demo@ipvmdemo.dyndns.org:5544/onvif-media/media.amp', status: 'online' }
])

// 视频槽位
const videoSlots = ref([])

// 初始化视频槽位
const initVideoSlots = () => {
  const slotCount = getSlotCount()
  videoSlots.value = Array.from({ length: slotCount }, (_, index) => ({
    active: index === 0,
    stream: null,
    audioEnabled: false,
    recording: false,
    detections: []
  }))
}

// 获取槽位数量
const getSlotCount = () => {
  const counts = { '1x1': 1, '2x2': 4, '3x3': 9, '4x4': 16 }
  return counts[viewMode.value] || 4
}

// 设置视图模式
const setViewMode = (mode) => {
  viewMode.value = mode
  initVideoSlots()
}

// 选择槽位
const selectSlot = (index) => {
  videoSlots.value.forEach((slot, i) => {
    slot.active = i === index
  })
}

// 切换槽位全屏
const toggleSlotFullscreen = (index) => {
  const slot = videoSlots.value[index]
  if (slot.stream) {
    ElMessage.info('双击全屏功能')
  }
}

// 添加摄像头到视图
const addCameraToView = () => {
  const camera = availableCameras.value.find(c => c.id === selectedCamera.value)
  if (!camera) return

  const activeSlot = videoSlots.value.find(slot => slot.active)
  if (activeSlot) {
    activeSlot.stream = {
      id: camera.id,
      name: camera.name,
      url: camera.url,
      status: camera.status,
      snapshot: '/api/v1/snapshot?stream=' + camera.id
    }

    // 模拟AI检测数据
    if (aiEnabled.value) {
      setTimeout(() => {
        activeSlot.detections = [
          {
            id: 1,
            class_name: 'person',
            confidence: 0.85,
            bbox: { x: 100, y: 150, width: 80, height: 120 }
          },
          {
            id: 2,
            class_name: 'car',
            confidence: 0.92,
            bbox: { x: 300, y: 200, width: 150, height: 100 }
          }
        ]
      }, 2000)
    }
  }

  selectedCamera.value = ''
}

// 获取检测框样式
const getDetectionStyle = (detection) => {
  return {
    left: detection.bbox.x + 'px',
    top: detection.bbox.y + 'px',
    width: detection.bbox.width + 'px',
    height: detection.bbox.height + 'px'
  }
}

// 切换全屏
const toggleFullscreen = () => {
  if (!document.fullscreenElement) {
    document.documentElement.requestFullscreen()
    isFullscreen.value = true
  } else {
    document.exitFullscreen()
    isFullscreen.value = false
  }
}

// 切换AI检测
const toggleAI = () => {
  aiEnabled.value = !aiEnabled.value
  ElMessage.success(`AI检测已${aiEnabled.value ? '开启' : '关闭'}`)
}

// 切换音频
const toggleAudio = (index) => {
  videoSlots.value[index].audioEnabled = !videoSlots.value[index].audioEnabled
}

// 截图
const takeSnapshot = (index) => {
  ElMessage.success('截图已保存')
}

// 开始/停止录制
const startRecording = (index) => {
  videoSlots.value[index].recording = !videoSlots.value[index].recording
  ElMessage.success(videoSlots.value[index].recording ? '开始录制' : '停止录制')
}

// 显示PTZ控制
const showPTZControl = (index) => {
  currentPTZSlot.value = index
  ptzDrawer.value = true
}

// PTZ移动
const ptzMove = (direction) => {
  ElMessage.info(`PTZ移动: ${direction}`)
}

// PTZ缩放
const ptzZoom = (type) => {
  ElMessage.info(`PTZ缩放: ${type}`)
}

// 跳转预置位
const gotoPreset = (preset) => {
  ElMessage.info(`跳转到预置位: ${preset}`)
}

// 刷新流
const refreshStreams = () => {
  ElMessage.success('流列表已刷新')
}

// 生命周期
onMounted(() => {
  initVideoSlots()

  // 监听全屏变化
  document.addEventListener('fullscreenchange', () => {
    isFullscreen.value = !!document.fullscreenElement
  })
})

onUnmounted(() => {
  document.removeEventListener('fullscreenchange', () => {})
})
</script>

<style scoped>
.live-view {
  height: calc(100vh - 140px);
  display: flex;
  flex-direction: column;
}

.toolbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px;
  background: white;
  border-radius: 8px;
  margin-bottom: 10px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.toolbar-left, .toolbar-right {
  display: flex;
  align-items: center;
  gap: 10px;
}

.video-grid {
  flex: 1;
  display: grid;
  gap: 8px;
  padding: 10px;
  background: #f5f5f5;
  border-radius: 8px;
}

.grid-1x1 { grid-template-columns: 1fr; }
.grid-2x2 { grid-template-columns: 1fr 1fr; }
.grid-3x3 { grid-template-columns: repeat(3, 1fr); }
.grid-4x4 { grid-template-columns: repeat(4, 1fr); }

.video-slot {
  position: relative;
  background: #000;
  border-radius: 8px;
  overflow: hidden;
  cursor: pointer;
  transition: all 0.3s ease;
  border: 3px solid transparent;
  aspect-ratio: 16/9;
}

.video-slot.active {
  border-color: #409EFF;
  box-shadow: 0 0 10px rgba(64, 158, 255, 0.3);
}

.video-slot.has-stream:hover {
  transform: scale(1.02);
}

.video-player {
  position: relative;
  width: 100%;
  height: 100%;
}

.video-element {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.ai-overlay {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
}

.detection-box {
  position: absolute;
  border: 2px solid #ff4444;
  background: rgba(255, 68, 68, 0.1);
}

.detection-label {
  position: absolute;
  top: -25px;
  left: 0;
  background: #ff4444;
  color: white;
  padding: 2px 6px;
  font-size: 12px;
  border-radius: 3px;
  white-space: nowrap;
}

.video-controls {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  background: linear-gradient(transparent, rgba(0,0,0,0.7));
  padding: 10px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.video-slot:hover .video-controls {
  opacity: 1;
}

.controls-left {
  display: flex;
  align-items: center;
  gap: 10px;
}

.camera-name {
  color: white;
  font-weight: bold;
}

.stream-status {
  padding: 2px 6px;
  border-radius: 3px;
  font-size: 12px;
}

.stream-status.online {
  background: #67c23a;
  color: white;
}

.stream-status.offline {
  background: #f56c6c;
  color: white;
}

.controls-right {
  display: flex;
  gap: 5px;
}

.empty-slot {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: #999;
  background: #2a2a2a;
  height: 100%;
}

.empty-icon {
  font-size: 48px;
  margin-bottom: 10px;
}

.slot-number {
  position: absolute;
  top: 10px;
  left: 10px;
  background: rgba(0,0,0,0.5);
  color: white;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 14px;
}

.ptz-control {
  padding: 20px;
}

.ptz-direction {
  margin-bottom: 20px;
}

.ptz-row {
  display: flex;
  justify-content: center;
  gap: 5px;
  margin-bottom: 5px;
}

.ptz-zoom {
  margin-bottom: 20px;
}

.ptz-presets h4 {
  margin-bottom: 10px;
  text-align: center;
}
</style>
