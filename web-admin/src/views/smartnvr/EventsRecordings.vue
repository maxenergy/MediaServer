<template>
  <div class="events-recordings">
    <!-- 顶部筛选栏 -->
    <el-card class="filter-card">
      <el-row :gutter="20">
        <el-col :span="4">
          <el-select v-model="filters.type" placeholder="事件类型" clearable>
            <el-option label="全部" value="" />
            <el-option label="物体检测" value="object_detection" />
            <el-option label="人脸识别" value="face_recognition" />
            <el-option label="车牌识别" value="plate_recognition" />
            <el-option label="运动检测" value="motion_detection" />
            <el-option label="入侵检测" value="intrusion_detection" />
          </el-select>
        </el-col>
        
        <el-col :span="4">
          <el-select v-model="filters.camera" placeholder="摄像头" clearable>
            <el-option label="全部" value="" />
            <el-option label="前门摄像头" value="camera1" />
            <el-option label="后门摄像头" value="camera2" />
            <el-option label="停车场摄像头" value="camera3" />
            <el-option label="办公室摄像头" value="camera4" />
          </el-select>
        </el-col>
        
        <el-col :span="6">
          <el-date-picker
            v-model="filters.dateRange"
            type="datetimerange"
            range-separator="至"
            start-placeholder="开始时间"
            end-placeholder="结束时间"
            format="YYYY-MM-DD HH:mm:ss"
            value-format="YYYY-MM-DD HH:mm:ss"
          />
        </el-col>
        
        <el-col :span="4">
          <el-input v-model="filters.keyword" placeholder="搜索关键词" clearable>
            <template #prefix>
              <el-icon><Search /></el-icon>
            </template>
          </el-input>
        </el-col>
        
        <el-col :span="6">
          <el-button type="primary" @click="searchEvents" :icon="'Search'">搜索</el-button>
          <el-button @click="resetFilters" :icon="'Refresh'">重置</el-button>
          <el-button @click="exportEvents" :icon="'Download'">导出</el-button>
        </el-col>
      </el-row>
    </el-card>

    <!-- 事件时间线 -->
    <el-card class="timeline-card">
      <template #header>
        <div class="card-header">
          <span>事件时间线</span>
          <div class="view-controls">
            <el-radio-group v-model="viewMode" size="small">
              <el-radio-button label="timeline">时间线</el-radio-button>
              <el-radio-button label="grid">网格</el-radio-button>
              <el-radio-button label="list">列表</el-radio-button>
            </el-radio-group>
          </div>
        </div>
      </template>
      
      <!-- 时间线视图 -->
      <div v-if="viewMode === 'timeline'" class="timeline-view">
        <el-timeline>
          <el-timeline-item
            v-for="event in filteredEvents"
            :key="event.id"
            :timestamp="event.timestamp"
            :type="getEventType(event.type)"
            :icon="getEventIcon(event.type)"
            placement="top"
          >
            <el-card class="event-card" @click="showEventDetail(event)">
              <div class="event-content">
                <div class="event-thumbnail">
                  <img :src="event.thumbnail" :alt="event.type" />
                  <div class="event-overlay">
                    <el-icon><VideoPlay /></el-icon>
                  </div>
                </div>
                <div class="event-info">
                  <div class="event-title">{{ getEventTitle(event) }}</div>
                  <div class="event-meta">
                    <el-tag :type="getEventType(event.type)" size="small">
                      {{ getEventTypeName(event.type) }}
                    </el-tag>
                    <span class="event-camera">{{ event.camera_name }}</span>
                    <span class="event-confidence" v-if="event.confidence">
                      置信度: {{ Math.round(event.confidence * 100) }}%
                    </span>
                  </div>
                </div>
              </div>
            </el-card>
          </el-timeline-item>
        </el-timeline>
      </div>
      
      <!-- 网格视图 -->
      <div v-if="viewMode === 'grid'" class="grid-view">
        <el-row :gutter="20">
          <el-col :span="6" v-for="event in filteredEvents" :key="event.id">
            <el-card class="event-grid-card" @click="showEventDetail(event)">
              <div class="grid-thumbnail">
                <img :src="event.thumbnail" :alt="event.type" />
                <div class="grid-overlay">
                  <el-icon><VideoPlay /></el-icon>
                </div>
                <div class="grid-badge">
                  <el-tag :type="getEventType(event.type)" size="small">
                    {{ getEventTypeName(event.type) }}
                  </el-tag>
                </div>
              </div>
              <div class="grid-info">
                <div class="grid-title">{{ getEventTitle(event) }}</div>
                <div class="grid-meta">
                  <div>{{ event.camera_name }}</div>
                  <div>{{ event.timestamp }}</div>
                </div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>
      
      <!-- 列表视图 -->
      <div v-if="viewMode === 'list'" class="list-view">
        <el-table :data="filteredEvents" style="width: 100%">
          <el-table-column label="缩略图" width="100">
            <template #default="scope">
              <img :src="scope.row.thumbnail" class="table-thumbnail" @click="showEventDetail(scope.row)" />
            </template>
          </el-table-column>
          
          <el-table-column prop="timestamp" label="时间" width="180" />
          
          <el-table-column label="类型" width="120">
            <template #default="scope">
              <el-tag :type="getEventType(scope.row.type)" size="small">
                {{ getEventTypeName(scope.row.type) }}
              </el-tag>
            </template>
          </el-table-column>
          
          <el-table-column prop="camera_name" label="摄像头" width="150" />
          
          <el-table-column label="描述">
            <template #default="scope">
              {{ getEventTitle(scope.row) }}
            </template>
          </el-table-column>
          
          <el-table-column label="置信度" width="100">
            <template #default="scope">
              <span v-if="scope.row.confidence">
                {{ Math.round(scope.row.confidence * 100) }}%
              </span>
            </template>
          </el-table-column>
          
          <el-table-column label="操作" width="200">
            <template #default="scope">
              <el-button size="small" @click="playEvent(scope.row)" :icon="'VideoPlay'">播放</el-button>
              <el-button size="small" @click="downloadEvent(scope.row)" :icon="'Download'">下载</el-button>
              <el-button size="small" type="danger" @click="deleteEvent(scope.row)" :icon="'Delete'">删除</el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-card>

    <!-- 事件详情对话框 -->
    <el-dialog v-model="eventDetailDialog" :title="selectedEvent?.type" width="800px">
      <div v-if="selectedEvent" class="event-detail">
        <el-row :gutter="20">
          <el-col :span="12">
            <div class="detail-video">
              <video controls :poster="selectedEvent.thumbnail" style="width: 100%">
                <source :src="selectedEvent.video_url" type="video/mp4">
                您的浏览器不支持视频播放
              </video>
            </div>
          </el-col>
          
          <el-col :span="12">
            <div class="detail-info">
              <el-descriptions :column="1" border>
                <el-descriptions-item label="事件类型">
                  <el-tag :type="getEventType(selectedEvent.type)">
                    {{ getEventTypeName(selectedEvent.type) }}
                  </el-tag>
                </el-descriptions-item>
                <el-descriptions-item label="摄像头">{{ selectedEvent.camera_name }}</el-descriptions-item>
                <el-descriptions-item label="时间">{{ selectedEvent.timestamp }}</el-descriptions-item>
                <el-descriptions-item label="置信度" v-if="selectedEvent.confidence">
                  {{ Math.round(selectedEvent.confidence * 100) }}%
                </el-descriptions-item>
                <el-descriptions-item label="检测对象" v-if="selectedEvent.detected_objects">
                  <el-tag v-for="obj in selectedEvent.detected_objects" :key="obj" size="small" style="margin-right: 5px">
                    {{ obj }}
                  </el-tag>
                </el-descriptions-item>
                <el-descriptions-item label="文件大小">{{ selectedEvent.file_size }}</el-descriptions-item>
                <el-descriptions-item label="录制时长">{{ selectedEvent.duration }}</el-descriptions-item>
              </el-descriptions>
            </div>
          </el-col>
        </el-row>
      </div>
      
      <template #footer>
        <el-button @click="eventDetailDialog = false">关闭</el-button>
        <el-button type="primary" @click="downloadEvent(selectedEvent)">下载视频</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'

// 响应式数据
const viewMode = ref('timeline')
const eventDetailDialog = ref(false)
const selectedEvent = ref(null)

const filters = reactive({
  type: '',
  camera: '',
  dateRange: [],
  keyword: ''
})

// 模拟事件数据
const events = ref([
  {
    id: 1,
    type: 'object_detection',
    camera_name: '前门摄像头',
    timestamp: '2024-01-15 14:30:25',
    confidence: 0.85,
    detected_objects: ['person', 'car'],
    thumbnail: '/api/v1/thumbnail/event1.jpg',
    video_url: '/api/v1/video/event1.mp4',
    file_size: '2.5MB',
    duration: '30s'
  },
  {
    id: 2,
    type: 'face_recognition',
    camera_name: '办公室摄像头',
    timestamp: '2024-01-15 14:25:10',
    confidence: 0.92,
    detected_objects: ['张三'],
    thumbnail: '/api/v1/thumbnail/event2.jpg',
    video_url: '/api/v1/video/event2.mp4',
    file_size: '1.8MB',
    duration: '25s'
  },
  {
    id: 3,
    type: 'plate_recognition',
    camera_name: '停车场摄像头',
    timestamp: '2024-01-15 14:20:45',
    confidence: 0.78,
    detected_objects: ['京A12345'],
    thumbnail: '/api/v1/thumbnail/event3.jpg',
    video_url: '/api/v1/video/event3.mp4',
    file_size: '3.2MB',
    duration: '45s'
  },
  {
    id: 4,
    type: 'motion_detection',
    camera_name: '后门摄像头',
    timestamp: '2024-01-15 14:15:30',
    confidence: 0.65,
    detected_objects: [],
    thumbnail: '/api/v1/thumbnail/event4.jpg',
    video_url: '/api/v1/video/event4.mp4',
    file_size: '1.5MB',
    duration: '20s'
  }
])

// 计算属性
const filteredEvents = computed(() => {
  return events.value.filter(event => {
    if (filters.type && event.type !== filters.type) return false
    if (filters.camera && !event.camera_name.includes(filters.camera)) return false
    if (filters.keyword && !getEventTitle(event).includes(filters.keyword)) return false
    return true
  })
})

// 方法
const getEventType = (type) => {
  const types = {
    'object_detection': 'primary',
    'face_recognition': 'success',
    'plate_recognition': 'warning',
    'motion_detection': 'info',
    'intrusion_detection': 'danger'
  }
  return types[type] || 'info'
}

const getEventIcon = (type) => {
  const icons = {
    'object_detection': 'View',
    'face_recognition': 'User',
    'plate_recognition': 'Tickets',
    'motion_detection': 'Position',
    'intrusion_detection': 'Warning'
  }
  return icons[type] || 'Bell'
}

const getEventTypeName = (type) => {
  const names = {
    'object_detection': '物体检测',
    'face_recognition': '人脸识别',
    'plate_recognition': '车牌识别',
    'motion_detection': '运动检测',
    'intrusion_detection': '入侵检测'
  }
  return names[type] || type
}

const getEventTitle = (event) => {
  if (event.detected_objects && event.detected_objects.length > 0) {
    return `检测到: ${event.detected_objects.join(', ')}`
  }
  return getEventTypeName(event.type)
}

const searchEvents = () => {
  ElMessage.success('搜索完成')
}

const resetFilters = () => {
  Object.assign(filters, {
    type: '',
    camera: '',
    dateRange: [],
    keyword: ''
  })
  ElMessage.info('筛选条件已重置')
}

const exportEvents = () => {
  ElMessage.success('事件数据导出中...')
}

const showEventDetail = (event) => {
  selectedEvent.value = event
  eventDetailDialog.value = true
}

const playEvent = (event) => {
  showEventDetail(event)
}

const downloadEvent = (event) => {
  ElMessage.success(`下载事件视频: ${event.id}`)
}

const deleteEvent = (event) => {
  ElMessageBox.confirm('确定要删除这个事件记录吗？', '确认删除', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning'
  }).then(() => {
    const index = events.value.findIndex(e => e.id === event.id)
    if (index > -1) {
      events.value.splice(index, 1)
      ElMessage.success('事件已删除')
    }
  }).catch(() => {
    ElMessage.info('已取消删除')
  })
}

// 生命周期
onMounted(() => {
  // 初始化数据
})
</script>

<style scoped>
.events-recordings {
  padding: 20px;
}

.filter-card {
  margin-bottom: 20px;
}

.timeline-card {
  min-height: 600px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.event-card {
  cursor: pointer;
  transition: all 0.3s ease;
}

.event-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}

.event-content {
  display: flex;
  gap: 15px;
}

.event-thumbnail {
  position: relative;
  width: 120px;
  height: 80px;
  border-radius: 6px;
  overflow: hidden;
}

.event-thumbnail img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.event-overlay {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: rgba(0,0,0,0.6);
  color: white;
  border-radius: 50%;
  width: 30px;
  height: 30px;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.event-card:hover .event-overlay {
  opacity: 1;
}

.event-info {
  flex: 1;
}

.event-title {
  font-weight: bold;
  margin-bottom: 8px;
}

.event-meta {
  display: flex;
  align-items: center;
  gap: 10px;
  font-size: 12px;
  color: #666;
}

.grid-view {
  min-height: 400px;
}

.event-grid-card {
  cursor: pointer;
  margin-bottom: 20px;
  transition: all 0.3s ease;
}

.event-grid-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}

.grid-thumbnail {
  position: relative;
  width: 100%;
  height: 150px;
  border-radius: 6px;
  overflow: hidden;
  margin-bottom: 10px;
}

.grid-thumbnail img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.grid-overlay {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: rgba(0,0,0,0.6);
  color: white;
  border-radius: 50%;
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.event-grid-card:hover .grid-overlay {
  opacity: 1;
}

.grid-badge {
  position: absolute;
  top: 8px;
  right: 8px;
}

.grid-title {
  font-weight: bold;
  margin-bottom: 5px;
}

.grid-meta {
  font-size: 12px;
  color: #666;
}

.table-thumbnail {
  width: 60px;
  height: 40px;
  object-fit: cover;
  border-radius: 4px;
  cursor: pointer;
}

.event-detail {
  padding: 20px 0;
}

.detail-video {
  margin-bottom: 20px;
}

.detail-info {
  height: 100%;
}
</style>
