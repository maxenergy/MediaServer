<template>
  <div class="smartnvr-dashboard">
    <!-- 顶部统计卡片 -->
    <el-row :gutter="20" class="stats-row">
      <el-col :span="6">
        <el-card class="stats-card cameras">
          <div class="stats-content">
            <div class="stats-icon">
              <el-icon><VideoCamera /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ systemStats.total_cameras }}</div>
              <div class="stats-label">摄像头总数</div>
              <div class="stats-detail">在线: {{ systemStats.online_cameras }}</div>
            </div>
          </div>
        </el-card>
      </el-col>

      <el-col :span="6">
        <el-card class="stats-card events">
          <div class="stats-content">
            <div class="stats-icon">
              <el-icon><Bell /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ systemStats.today_events }}</div>
              <div class="stats-label">今日事件</div>
              <div class="stats-detail">AI检测: {{ systemStats.ai_events }}</div>
            </div>
          </div>
        </el-card>
      </el-col>

      <el-col :span="6">
        <el-card class="stats-card storage">
          <div class="stats-content">
            <div class="stats-icon">
              <el-icon><FolderOpened /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ systemStats.storage_used }}TB</div>
              <div class="stats-label">存储使用</div>
              <div class="stats-detail">总容量: {{ systemStats.storage_total }}TB</div>
            </div>
          </div>
        </el-card>
      </el-col>

      <el-col :span="6">
        <el-card class="stats-card performance">
          <div class="stats-content">
            <div class="stats-icon">
              <el-icon><Monitor /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ systemStats.cpu_usage }}%</div>
              <div class="stats-label">CPU使用率</div>
              <div class="stats-detail">内存: {{ systemStats.memory_usage }}%</div>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- 主要内容区域 -->
    <el-row :gutter="20">
      <!-- 左侧：实时监控 -->
      <el-col :span="16">
        <el-card class="live-monitor-card">
          <template #header>
            <div class="card-header">
              <span>实时监控</span>
              <div class="monitor-controls">
                <el-button-group size="small">
                  <el-button :type="liveViewMode === '2x2' ? 'primary' : ''" @click="setLiveViewMode('2x2')">2x2</el-button>
                  <el-button :type="liveViewMode === '3x3' ? 'primary' : ''" @click="setLiveViewMode('3x3')">3x3</el-button>
                </el-button-group>
                <el-button size="small" @click="goToLiveView" :icon="'FullScreen'">全屏监控</el-button>
              </div>
            </div>
          </template>

          <div class="live-grid" :class="`grid-${liveViewMode}`">
            <div
              v-for="(camera, index) in displayCameras"
              :key="camera.id"
              class="live-slot"
              :class="{ 'online': camera.status === 'online' }"
            >
              <div v-if="camera.status === 'online'" class="live-video">
                <img :src="camera.snapshot" :alt="camera.name" />
                <div class="video-overlay">
                  <div class="camera-name">{{ camera.name }}</div>
                  <div class="camera-status">
                    <el-tag :type="camera.ai_enabled ? 'success' : 'info'" size="small">
                      {{ camera.ai_enabled ? 'AI已启用' : 'AI未启用' }}
                    </el-tag>
                  </div>
                </div>
                <!-- AI检测结果覆盖层 -->
                <div v-if="camera.ai_enabled && camera.detections" class="ai-overlay">
                  <div
                    v-for="detection in camera.detections"
                    :key="detection.id"
                    class="detection-box"
                    :style="getDetectionStyle(detection)"
                  >
                    <div class="detection-label">
                      {{ detection.class_name }} {{ Math.round(detection.confidence * 100) }}%
                    </div>
                  </div>
                </div>
              </div>
              <div v-else class="offline-slot">
                <el-icon class="offline-icon"><VideoCameraFilled /></el-icon>
                <div class="offline-text">{{ camera.name }}</div>
                <div class="offline-status">离线</div>
              </div>
            </div>
          </div>
        </el-card>
      </el-col>

      <!-- 右侧：事件和状态 -->
      <el-col :span="8">
        <!-- 最新事件 -->
        <el-card class="recent-events-card">
          <template #header>
            <div class="card-header">
              <span>最新事件</span>
              <el-button size="small" @click="goToEvents" :icon="'More'">查看全部</el-button>
            </div>
          </template>

          <div class="events-list">
            <div
              v-for="event in recentEvents"
              :key="event.id"
              class="event-item"
              @click="showEventDetail(event)"
            >
              <div class="event-thumbnail">
                <img :src="event.thumbnail" :alt="event.type" />
              </div>
              <div class="event-info">
                <div class="event-title">{{ getEventTitle(event) }}</div>
                <div class="event-meta">
                  <el-tag :type="getEventType(event.type)" size="small">
                    {{ getEventTypeName(event.type) }}
                  </el-tag>
                  <span class="event-time">{{ formatTime(event.timestamp) }}</span>
                </div>
                <div class="event-camera">{{ event.camera_name }}</div>
              </div>
            </div>
          </div>
        </el-card>

        <!-- 系统状态 -->
        <el-card class="system-status-card">
          <template #header>
            <span>系统状态</span>
          </template>

          <div class="status-list">
            <div class="status-item">
              <div class="status-label">AI检测服务</div>
              <el-tag :type="systemStatus.ai_service ? 'success' : 'danger'" size="small">
                {{ systemStatus.ai_service ? '运行中' : '已停止' }}
              </el-tag>
            </div>

            <div class="status-item">
              <div class="status-label">录制服务</div>
              <el-tag :type="systemStatus.recording_service ? 'success' : 'danger'" size="small">
                {{ systemStatus.recording_service ? '运行中' : '已停止' }}
              </el-tag>
            </div>

            <div class="status-item">
              <div class="status-label">流媒体服务</div>
              <el-tag :type="systemStatus.streaming_service ? 'success' : 'danger'" size="small">
                {{ systemStatus.streaming_service ? '运行中' : '已停止' }}
              </el-tag>
            </div>

            <div class="status-item">
              <div class="status-label">数据库连接</div>
              <el-tag :type="systemStatus.database ? 'success' : 'danger'" size="small">
                {{ systemStatus.database ? '正常' : '异常' }}
              </el-tag>
            </div>
          </div>

          <el-divider />

          <div class="performance-charts">
            <div class="chart-item">
              <div class="chart-label">CPU使用率</div>
              <el-progress :percentage="systemStats.cpu_usage" :color="getProgressColor(systemStats.cpu_usage)" />
            </div>

            <div class="chart-item">
              <div class="chart-label">内存使用率</div>
              <el-progress :percentage="systemStats.memory_usage" :color="getProgressColor(systemStats.memory_usage)" />
            </div>

            <div class="chart-item">
              <div class="chart-label">存储使用率</div>
              <el-progress :percentage="systemStats.storage_percentage" :color="getProgressColor(systemStats.storage_percentage)" />
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- 快速操作 -->
    <el-card class="quick-actions-card">
      <template #header>
        <span>快速操作</span>
      </template>

      <el-row :gutter="20">
        <el-col :span="4">
          <div class="action-item" @click="goToLiveView">
            <el-icon class="action-icon"><VideoCamera /></el-icon>
            <div class="action-label">实时监控</div>
          </div>
        </el-col>

        <el-col :span="4">
          <div class="action-item" @click="goToAIManagement">
            <el-icon class="action-icon"><View /></el-icon>
            <div class="action-label">AI管理</div>
          </div>
        </el-col>

        <el-col :span="4">
          <div class="action-item" @click="goToEvents">
            <el-icon class="action-icon"><Bell /></el-icon>
            <div class="action-label">事件记录</div>
          </div>
        </el-col>

        <el-col :span="4">
          <div class="action-item" @click="goToFaceDatabase">
            <el-icon class="action-icon"><User /></el-icon>
            <div class="action-label">人脸库</div>
          </div>
        </el-col>

        <el-col :span="4">
          <div class="action-item" @click="goToCameraManagement">
            <el-icon class="action-icon"><Setting /></el-icon>
            <div class="action-label">摄像头管理</div>
          </div>
        </el-col>

        <el-col :span="4">
          <div class="action-item" @click="goToSystemSettings">
            <el-icon class="action-icon"><Tools /></el-icon>
            <div class="action-label">系统设置</div>
          </div>
        </el-col>
      </el-row>
    </el-card>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'

const router = useRouter()

// 响应式数据
const liveViewMode = ref('2x2')

const systemStats = reactive({
  total_cameras: 8,
  online_cameras: 6,
  today_events: 156,
  ai_events: 89,
  storage_used: 2.3,
  storage_total: 10.0,
  cpu_usage: 45,
  memory_usage: 62,
  storage_percentage: 23
})

const systemStatus = reactive({
  ai_service: true,
  recording_service: true,
  streaming_service: true,
  database: true
})

// 摄像头数据
const cameras = ref([
  {
    id: 'cam1',
    name: '前门摄像头',
    status: 'online',
    ai_enabled: true,
    snapshot: '/api/v1/snapshot/cam1.jpg',
    detections: [
      { id: 1, class_name: 'person', confidence: 0.85, bbox: { x: 100, y: 150, width: 80, height: 120 } }
    ]
  },
  {
    id: 'cam2',
    name: '后门摄像头',
    status: 'online',
    ai_enabled: true,
    snapshot: '/api/v1/snapshot/cam2.jpg',
    detections: []
  },
  {
    id: 'cam3',
    name: '停车场摄像头',
    status: 'offline',
    ai_enabled: false,
    snapshot: '/api/v1/snapshot/cam3.jpg',
    detections: []
  },
  {
    id: 'cam4',
    name: '办公室摄像头',
    status: 'online',
    ai_enabled: true,
    snapshot: '/api/v1/snapshot/cam4.jpg',
    detections: [
      { id: 2, class_name: 'car', confidence: 0.92, bbox: { x: 200, y: 100, width: 150, height: 100 } }
    ]
  }
])

// 最新事件
const recentEvents = ref([
  {
    id: 1,
    type: 'face_recognition',
    camera_name: '办公室摄像头',
    timestamp: '2024-01-15 14:30:25',
    thumbnail: '/api/v1/thumbnail/event1.jpg',
    detected_objects: ['张三']
  },
  {
    id: 2,
    type: 'object_detection',
    camera_name: '前门摄像头',
    timestamp: '2024-01-15 14:25:10',
    thumbnail: '/api/v1/thumbnail/event2.jpg',
    detected_objects: ['person', 'car']
  },
  {
    id: 3,
    type: 'plate_recognition',
    camera_name: '停车场摄像头',
    timestamp: '2024-01-15 14:20:45',
    thumbnail: '/api/v1/thumbnail/event3.jpg',
    detected_objects: ['京A12345']
  }
])

// 计算属性
const displayCameras = computed(() => {
  const count = liveViewMode.value === '2x2' ? 4 : 9
  return cameras.value.slice(0, count)
})

// 方法
const setLiveViewMode = (mode) => {
  liveViewMode.value = mode
}

const getDetectionStyle = (detection) => {
  return {
    left: detection.bbox.x + 'px',
    top: detection.bbox.y + 'px',
    width: detection.bbox.width + 'px',
    height: detection.bbox.height + 'px'
  }
}

const getEventType = (type) => {
  const types = {
    'object_detection': 'primary',
    'face_recognition': 'success',
    'plate_recognition': 'warning',
    'motion_detection': 'info'
  }
  return types[type] || 'info'
}

const getEventTypeName = (type) => {
  const names = {
    'object_detection': '物体检测',
    'face_recognition': '人脸识别',
    'plate_recognition': '车牌识别',
    'motion_detection': '运动检测'
  }
  return names[type] || type
}

const getEventTitle = (event) => {
  if (event.detected_objects && event.detected_objects.length > 0) {
    return `检测到: ${event.detected_objects.join(', ')}`
  }
  return getEventTypeName(event.type)
}

const formatTime = (timestamp) => {
  const now = new Date()
  const eventTime = new Date(timestamp)
  const diff = now - eventTime

  if (diff < 60000) return '刚刚'
  if (diff < 3600000) return `${Math.floor(diff / 60000)}分钟前`
  if (diff < 86400000) return `${Math.floor(diff / 3600000)}小时前`
  return eventTime.toLocaleDateString()
}

const getProgressColor = (percentage) => {
  if (percentage < 50) return '#67c23a'
  if (percentage < 80) return '#e6a23c'
  return '#f56c6c'
}

// 导航方法
const goToLiveView = () => {
  router.push('/smartnvr/live')
}

const goToAIManagement = () => {
  router.push('/smartnvr/ai')
}

const goToEvents = () => {
  router.push('/smartnvr/events')
}

const goToFaceDatabase = () => {
  router.push('/smartnvr/faces')
}

const goToCameraManagement = () => {
  router.push('/smartnvr/cameras')
}

const goToSystemSettings = () => {
  router.push('/config/server')
}

const showEventDetail = (event) => {
  ElMessage.info(`查看事件详情: ${event.id}`)
}

// 定时更新数据
let updateTimer = null

const updateData = () => {
  // 模拟数据更新
  systemStats.cpu_usage = Math.floor(Math.random() * 30) + 30
  systemStats.memory_usage = Math.floor(Math.random() * 20) + 50

  // 更新摄像头检测结果
  cameras.value.forEach(camera => {
    if (camera.status === 'online' && camera.ai_enabled) {
      // 随机生成检测结果
      if (Math.random() > 0.7) {
        camera.detections = [
          {
            id: Date.now(),
            class_name: ['person', 'car', 'bicycle'][Math.floor(Math.random() * 3)],
            confidence: 0.7 + Math.random() * 0.3,
            bbox: {
              x: Math.floor(Math.random() * 200),
              y: Math.floor(Math.random() * 150),
              width: 80 + Math.floor(Math.random() * 50),
              height: 100 + Math.floor(Math.random() * 50)
            }
          }
        ]
      } else {
        camera.detections = []
      }
    }
  })
}

// 生命周期
onMounted(() => {
  updateTimer = setInterval(updateData, 5000) // 每5秒更新一次
})

onUnmounted(() => {
  if (updateTimer) {
    clearInterval(updateTimer)
  }
})
</script>

<style scoped>
.smartnvr-dashboard {
  padding: 20px;
}

.stats-row {
  margin-bottom: 20px;
}

.stats-card {
  height: 120px;
}

.stats-content {
  display: flex;
  align-items: center;
  height: 100%;
}

.stats-icon {
  width: 60px;
  height: 60px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-right: 15px;
  font-size: 24px;
  color: white;
}

.stats-card.cameras .stats-icon { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
.stats-card.events .stats-icon { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); }
.stats-card.storage .stats-icon { background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%); }
.stats-card.performance .stats-icon { background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%); }

.stats-info {
  flex: 1;
}

.stats-value {
  font-size: 28px;
  font-weight: bold;
  color: #303133;
  line-height: 1;
}

.stats-label {
  font-size: 14px;
  color: #909399;
  margin: 5px 0;
}

.stats-detail {
  font-size: 12px;
  color: #606266;
}

.live-monitor-card {
  margin-bottom: 20px;
  height: 400px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.live-grid {
  display: grid;
  gap: 8px;
  height: 320px;
}

.grid-2x2 { grid-template-columns: 1fr 1fr; }
.grid-3x3 { grid-template-columns: repeat(3, 1fr); }

.live-slot {
  position: relative;
  background: #000;
  border-radius: 6px;
  overflow: hidden;
}

.live-video {
  position: relative;
  width: 100%;
  height: 100%;
}

.live-video img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.video-overlay {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  background: linear-gradient(transparent, rgba(0,0,0,0.7));
  padding: 10px;
  color: white;
}

.camera-name {
  font-weight: bold;
  margin-bottom: 5px;
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
  font-size: 10px;
  border-radius: 3px;
  white-space: nowrap;
}

.offline-slot {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: #666;
  background: #2a2a2a;
  height: 100%;
}

.offline-icon {
  font-size: 32px;
  margin-bottom: 10px;
}

.recent-events-card, .system-status-card {
  margin-bottom: 20px;
}

.events-list {
  max-height: 300px;
  overflow-y: auto;
}

.event-item {
  display: flex;
  padding: 10px;
  border-bottom: 1px solid #ebeef5;
  cursor: pointer;
  transition: background-color 0.3s ease;
}

.event-item:hover {
  background-color: #f5f7fa;
}

.event-thumbnail {
  width: 60px;
  height: 40px;
  border-radius: 4px;
  overflow: hidden;
  margin-right: 10px;
}

.event-thumbnail img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.event-info {
  flex: 1;
}

.event-title {
  font-weight: bold;
  margin-bottom: 5px;
}

.event-meta {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 3px;
}

.event-time {
  font-size: 12px;
  color: #909399;
}

.event-camera {
  font-size: 12px;
  color: #606266;
}

.status-list {
  margin-bottom: 20px;
}

.status-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 0;
  border-bottom: 1px solid #f0f0f0;
}

.status-label {
  font-size: 14px;
  color: #606266;
}

.chart-item {
  margin-bottom: 15px;
}

.chart-label {
  font-size: 12px;
  color: #909399;
  margin-bottom: 5px;
}

.quick-actions-card {
  margin-top: 20px;
}

.action-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 20px;
  border: 1px solid #ebeef5;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
}

.action-item:hover {
  background-color: #f5f7fa;
  border-color: #409eff;
  transform: translateY(-2px);
}

.action-icon {
  font-size: 32px;
  color: #409eff;
  margin-bottom: 10px;
}

.action-label {
  font-size: 14px;
  color: #606266;
  font-weight: bold;
}
</style>
