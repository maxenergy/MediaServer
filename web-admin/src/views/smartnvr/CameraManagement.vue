<template>
  <div class="camera-management">
    <!-- 顶部操作栏 -->
    <el-card class="actions-card">
      <el-row :gutter="20">
        <el-col :span="12">
          <el-input v-model="searchKeyword" placeholder="搜索摄像头名称或IP" clearable>
            <template #prefix>
              <el-icon><Search /></el-icon>
            </template>
          </el-input>
        </el-col>
        <el-col :span="12" class="actions-right">
          <el-button type="primary" @click="showAddCameraDialog" :icon="'Plus'">添加摄像头</el-button>
          <el-button @click="scanNetwork" :icon="'Search'">网络扫描</el-button>
          <el-button @click="refreshCameras" :icon="'Refresh'">刷新</el-button>
        </el-col>
      </el-row>
    </el-card>

    <!-- 摄像头列表 -->
    <el-card class="cameras-card">
      <template #header>
        <div class="card-header">
          <span>摄像头列表 ({{ filteredCameras.length }} 个)</span>
          <div class="view-controls">
            <el-radio-group v-model="viewMode" size="small">
              <el-radio-button label="grid">网格</el-radio-button>
              <el-radio-button label="list">列表</el-radio-button>
            </el-radio-group>
          </div>
        </div>
      </template>
      
      <!-- 网格视图 -->
      <div v-if="viewMode === 'grid'" class="grid-view">
        <el-row :gutter="20">
          <el-col :span="6" v-for="camera in filteredCameras" :key="camera.id">
            <el-card class="camera-card" :class="{ 'online': camera.status === 'online' }">
              <div class="camera-preview">
                <img v-if="camera.status === 'online'" :src="camera.snapshot" :alt="camera.name" />
                <div v-else class="offline-preview">
                  <el-icon class="offline-icon"><VideoCameraFilled /></el-icon>
                  <div class="offline-text">离线</div>
                </div>
                <div class="preview-overlay">
                  <el-button size="small" @click="testCamera(camera)" :icon="'VideoPlay'">测试</el-button>
                </div>
              </div>
              
              <div class="camera-info">
                <div class="camera-name">{{ camera.name }}</div>
                <div class="camera-ip">{{ camera.ip }}:{{ camera.port }}</div>
                <div class="camera-meta">
                  <el-tag :type="camera.status === 'online' ? 'success' : 'danger'" size="small">
                    {{ camera.status === 'online' ? '在线' : '离线' }}
                  </el-tag>
                  <el-tag v-if="camera.ai_enabled" type="primary" size="small">AI</el-tag>
                  <el-tag v-if="camera.recording" type="warning" size="small">录制</el-tag>
                </div>
                
                <div class="camera-actions">
                  <el-button size="small" @click="editCamera(camera)" :icon="'Edit'">编辑</el-button>
                  <el-button size="small" @click="toggleAI(camera)" :type="camera.ai_enabled ? 'success' : ''" :icon="'View'">
                    AI
                  </el-button>
                  <el-button size="small" @click="toggleRecording(camera)" :type="camera.recording ? 'warning' : ''" :icon="'VideoCamera'">
                    录制
                  </el-button>
                  <el-button size="small" type="danger" @click="deleteCamera(camera)" :icon="'Delete'">删除</el-button>
                </div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>
      
      <!-- 列表视图 -->
      <div v-if="viewMode === 'list'" class="list-view">
        <el-table :data="filteredCameras" style="width: 100%">
          <el-table-column label="预览" width="120">
            <template #default="scope">
              <div class="table-preview">
                <img v-if="scope.row.status === 'online'" :src="scope.row.snapshot" :alt="scope.row.name" />
                <div v-else class="table-offline">
                  <el-icon><VideoCameraFilled /></el-icon>
                </div>
              </div>
            </template>
          </el-table-column>
          
          <el-table-column prop="name" label="名称" width="150" />
          <el-table-column label="地址" width="200">
            <template #default="scope">
              {{ scope.row.ip }}:{{ scope.row.port }}
            </template>
          </el-table-column>
          <el-table-column prop="protocol" label="协议" width="100" />
          <el-table-column prop="resolution" label="分辨率" width="120" />
          
          <el-table-column label="状态" width="100">
            <template #default="scope">
              <el-tag :type="scope.row.status === 'online' ? 'success' : 'danger'" size="small">
                {{ scope.row.status === 'online' ? '在线' : '离线' }}
              </el-tag>
            </template>
          </el-table-column>
          
          <el-table-column label="功能" width="150">
            <template #default="scope">
              <el-tag v-if="scope.row.ai_enabled" type="primary" size="small">AI</el-tag>
              <el-tag v-if="scope.row.recording" type="warning" size="small">录制</el-tag>
              <el-tag v-if="scope.row.ptz_enabled" type="info" size="small">PTZ</el-tag>
            </template>
          </el-table-column>
          
          <el-table-column prop="created_at" label="添加时间" width="180" />
          
          <el-table-column label="操作" width="300">
            <template #default="scope">
              <el-button size="small" @click="testCamera(scope.row)" :icon="'VideoPlay'">测试</el-button>
              <el-button size="small" @click="editCamera(scope.row)" :icon="'Edit'">编辑</el-button>
              <el-button size="small" @click="toggleAI(scope.row)" :type="scope.row.ai_enabled ? 'success' : ''" :icon="'View'">
                AI
              </el-button>
              <el-button size="small" @click="toggleRecording(scope.row)" :type="scope.row.recording ? 'warning' : ''" :icon="'VideoCamera'">
                录制
              </el-button>
              <el-button size="small" type="danger" @click="deleteCamera(scope.row)" :icon="'Delete'">删除</el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-card>

    <!-- 添加/编辑摄像头对话框 -->
    <el-dialog v-model="cameraDialog" :title="isEditing ? '编辑摄像头' : '添加摄像头'" width="600px">
      <el-form :model="cameraForm" :rules="cameraRules" ref="cameraFormRef" label-width="120px">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="摄像头名称" prop="name">
              <el-input v-model="cameraForm.name" placeholder="请输入摄像头名称" />
            </el-form-item>
            
            <el-form-item label="IP地址" prop="ip">
              <el-input v-model="cameraForm.ip" placeholder="192.168.1.100" />
            </el-form-item>
            
            <el-form-item label="端口" prop="port">
              <el-input-number v-model="cameraForm.port" :min="1" :max="65535" controls-position="right" />
            </el-form-item>
            
            <el-form-item label="协议" prop="protocol">
              <el-select v-model="cameraForm.protocol" placeholder="选择协议">
                <el-option label="RTSP" value="rtsp" />
                <el-option label="ONVIF" value="onvif" />
                <el-option label="GB28181" value="gb28181" />
                <el-option label="HTTP" value="http" />
              </el-select>
            </el-form-item>
          </el-col>
          
          <el-col :span="12">
            <el-form-item label="用户名">
              <el-input v-model="cameraForm.username" placeholder="用户名" />
            </el-form-item>
            
            <el-form-item label="密码">
              <el-input v-model="cameraForm.password" type="password" placeholder="密码" show-password />
            </el-form-item>
            
            <el-form-item label="流地址">
              <el-input v-model="cameraForm.stream_url" placeholder="rtsp://..." />
            </el-form-item>
            
            <el-form-item label="分辨率">
              <el-select v-model="cameraForm.resolution" placeholder="选择分辨率">
                <el-option label="1920x1080" value="1920x1080" />
                <el-option label="1280x720" value="1280x720" />
                <el-option label="640x480" value="640x480" />
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
        
        <el-form-item label="功能设置">
          <el-checkbox-group v-model="cameraFeatures">
            <el-checkbox label="ai_enabled">启用AI检测</el-checkbox>
            <el-checkbox label="recording">启用录制</el-checkbox>
            <el-checkbox label="ptz_enabled">支持PTZ控制</el-checkbox>
            <el-checkbox label="audio_enabled">启用音频</el-checkbox>
          </el-checkbox-group>
        </el-form-item>
        
        <el-form-item label="备注">
          <el-input v-model="cameraForm.notes" type="textarea" placeholder="备注信息" />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="cameraDialog = false">取消</el-button>
        <el-button @click="testConnection">测试连接</el-button>
        <el-button type="primary" @click="saveCamera">保存</el-button>
      </template>
    </el-dialog>

    <!-- 网络扫描对话框 -->
    <el-dialog v-model="scanDialog" title="网络扫描" width="500px">
      <div class="scan-content">
        <el-form :model="scanForm" label-width="100px">
          <el-form-item label="IP范围">
            <el-input v-model="scanForm.ip_range" placeholder="192.168.1.1-192.168.1.254" />
          </el-form-item>
          <el-form-item label="端口">
            <el-input v-model="scanForm.ports" placeholder="554,8080,80" />
          </el-form-item>
        </el-form>
        
        <div v-if="scanning" class="scan-progress">
          <el-progress :percentage="scanProgress" />
          <div class="scan-status">正在扫描网络设备...</div>
        </div>
        
        <div v-if="scanResults.length > 0" class="scan-results">
          <h4>发现的设备:</h4>
          <el-table :data="scanResults" style="width: 100%" max-height="300">
            <el-table-column prop="ip" label="IP地址" width="120" />
            <el-table-column prop="port" label="端口" width="80" />
            <el-table-column prop="protocol" label="协议" width="80" />
            <el-table-column label="操作" width="100">
              <template #default="scope">
                <el-button size="small" @click="addFromScan(scope.row)">添加</el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </div>
      
      <template #footer>
        <el-button @click="scanDialog = false">关闭</el-button>
        <el-button type="primary" @click="startScan" :loading="scanning">开始扫描</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'

// 响应式数据
const viewMode = ref('grid')
const searchKeyword = ref('')
const cameraDialog = ref(false)
const scanDialog = ref(false)
const isEditing = ref(false)
const scanning = ref(false)
const scanProgress = ref(0)
const cameraFormRef = ref()

// 摄像头表单
const cameraForm = reactive({
  id: '',
  name: '',
  ip: '',
  port: 554,
  protocol: 'rtsp',
  username: '',
  password: '',
  stream_url: '',
  resolution: '1920x1080',
  notes: ''
})

const cameraFeatures = ref([])

// 扫描表单
const scanForm = reactive({
  ip_range: '192.168.1.1-192.168.1.254',
  ports: '554,8080,80'
})

const scanResults = ref([])

// 表单验证规则
const cameraRules = {
  name: [{ required: true, message: '请输入摄像头名称', trigger: 'blur' }],
  ip: [{ required: true, message: '请输入IP地址', trigger: 'blur' }],
  protocol: [{ required: true, message: '请选择协议', trigger: 'change' }]
}

// 模拟摄像头数据
const cameras = ref([
  {
    id: 'cam1',
    name: '前门摄像头',
    ip: '192.168.1.100',
    port: 554,
    protocol: 'rtsp',
    resolution: '1920x1080',
    status: 'online',
    ai_enabled: true,
    recording: true,
    ptz_enabled: false,
    audio_enabled: true,
    snapshot: '/api/v1/snapshot/cam1.jpg',
    created_at: '2024-01-10 09:00:00',
    notes: '主入口监控'
  },
  {
    id: 'cam2',
    name: '后门摄像头',
    ip: '192.168.1.101',
    port: 554,
    protocol: 'rtsp',
    resolution: '1920x1080',
    status: 'online',
    ai_enabled: true,
    recording: false,
    ptz_enabled: true,
    audio_enabled: false,
    snapshot: '/api/v1/snapshot/cam2.jpg',
    created_at: '2024-01-10 09:30:00',
    notes: '后门监控，支持PTZ'
  },
  {
    id: 'cam3',
    name: '停车场摄像头',
    ip: '192.168.1.102',
    port: 554,
    protocol: 'onvif',
    resolution: '1280x720',
    status: 'offline',
    ai_enabled: false,
    recording: false,
    ptz_enabled: false,
    audio_enabled: false,
    snapshot: '/api/v1/snapshot/cam3.jpg',
    created_at: '2024-01-10 10:00:00',
    notes: '停车场监控'
  }
])

// 计算属性
const filteredCameras = computed(() => {
  if (!searchKeyword.value) return cameras.value
  return cameras.value.filter(camera => 
    camera.name.includes(searchKeyword.value) || 
    camera.ip.includes(searchKeyword.value)
  )
})

// 方法
const showAddCameraDialog = () => {
  isEditing.value = false
  resetCameraForm()
  cameraDialog.value = true
}

const editCamera = (camera) => {
  isEditing.value = true
  Object.assign(cameraForm, camera)
  cameraFeatures.value = []
  if (camera.ai_enabled) cameraFeatures.value.push('ai_enabled')
  if (camera.recording) cameraFeatures.value.push('recording')
  if (camera.ptz_enabled) cameraFeatures.value.push('ptz_enabled')
  if (camera.audio_enabled) cameraFeatures.value.push('audio_enabled')
  cameraDialog.value = true
}

const resetCameraForm = () => {
  Object.assign(cameraForm, {
    id: '',
    name: '',
    ip: '',
    port: 554,
    protocol: 'rtsp',
    username: '',
    password: '',
    stream_url: '',
    resolution: '1920x1080',
    notes: ''
  })
  cameraFeatures.value = []
}

const testConnection = () => {
  ElMessage.info('正在测试连接...')
  setTimeout(() => {
    ElMessage.success('连接测试成功')
  }, 2000)
}

const saveCamera = () => {
  cameraFormRef.value.validate((valid) => {
    if (valid) {
      const newCamera = {
        ...cameraForm,
        ai_enabled: cameraFeatures.value.includes('ai_enabled'),
        recording: cameraFeatures.value.includes('recording'),
        ptz_enabled: cameraFeatures.value.includes('ptz_enabled'),
        audio_enabled: cameraFeatures.value.includes('audio_enabled'),
        status: 'online',
        snapshot: `/api/v1/snapshot/${cameraForm.id || 'new'}.jpg`,
        created_at: new Date().toLocaleString()
      }
      
      if (isEditing.value) {
        const index = cameras.value.findIndex(c => c.id === cameraForm.id)
        if (index > -1) {
          Object.assign(cameras.value[index], newCamera)
          ElMessage.success('摄像头更新成功')
        }
      } else {
        newCamera.id = 'cam' + Date.now()
        cameras.value.push(newCamera)
        ElMessage.success('摄像头添加成功')
      }
      cameraDialog.value = false
    }
  })
}

const deleteCamera = (camera) => {
  ElMessageBox.confirm(`确定要删除摄像头 "${camera.name}" 吗？`, '确认删除', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning'
  }).then(() => {
    const index = cameras.value.findIndex(c => c.id === camera.id)
    if (index > -1) {
      cameras.value.splice(index, 1)
      ElMessage.success('摄像头已删除')
    }
  }).catch(() => {
    ElMessage.info('已取消删除')
  })
}

const testCamera = (camera) => {
  ElMessage.info(`测试摄像头: ${camera.name}`)
}

const toggleAI = (camera) => {
  camera.ai_enabled = !camera.ai_enabled
  ElMessage.success(`${camera.name} AI检测已${camera.ai_enabled ? '启用' : '禁用'}`)
}

const toggleRecording = (camera) => {
  camera.recording = !camera.recording
  ElMessage.success(`${camera.name} 录制已${camera.recording ? '启用' : '禁用'}`)
}

const refreshCameras = () => {
  ElMessage.success('摄像头列表已刷新')
}

const scanNetwork = () => {
  scanDialog.value = true
  scanResults.value = []
}

const startScan = () => {
  scanning.value = true
  scanProgress.value = 0
  scanResults.value = []
  
  // 模拟扫描过程
  const interval = setInterval(() => {
    scanProgress.value += 10
    if (scanProgress.value >= 100) {
      clearInterval(interval)
      scanning.value = false
      
      // 模拟扫描结果
      scanResults.value = [
        { ip: '192.168.1.103', port: 554, protocol: 'rtsp' },
        { ip: '192.168.1.104', port: 8080, protocol: 'http' },
        { ip: '192.168.1.105', port: 554, protocol: 'onvif' }
      ]
      ElMessage.success('网络扫描完成')
    }
  }, 200)
}

const addFromScan = (device) => {
  resetCameraForm()
  cameraForm.ip = device.ip
  cameraForm.port = device.port
  cameraForm.protocol = device.protocol
  cameraForm.name = `摄像头_${device.ip}`
  scanDialog.value = false
  cameraDialog.value = true
}

// 生命周期
onMounted(() => {
  // 初始化数据
})
</script>

<style scoped>
.camera-management {
  padding: 20px;
}

.actions-card {
  margin-bottom: 20px;
}

.actions-right {
  text-align: right;
}

.cameras-card {
  min-height: 600px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.camera-card {
  margin-bottom: 20px;
  transition: all 0.3s ease;
  border: 2px solid transparent;
}

.camera-card.online {
  border-color: #67c23a;
}

.camera-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}

.camera-preview {
  position: relative;
  width: 100%;
  height: 150px;
  border-radius: 6px;
  overflow: hidden;
  margin-bottom: 15px;
  background: #000;
}

.camera-preview img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.offline-preview {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #666;
}

.offline-icon {
  font-size: 32px;
  margin-bottom: 10px;
}

.preview-overlay {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  opacity: 0;
  transition: opacity 0.3s ease;
}

.camera-card:hover .preview-overlay {
  opacity: 1;
}

.camera-name {
  font-weight: bold;
  font-size: 16px;
  margin-bottom: 5px;
}

.camera-ip {
  color: #666;
  font-size: 12px;
  margin-bottom: 8px;
}

.camera-meta {
  display: flex;
  gap: 5px;
  margin-bottom: 15px;
  flex-wrap: wrap;
}

.camera-actions {
  display: flex;
  gap: 5px;
  flex-wrap: wrap;
}

.table-preview {
  width: 80px;
  height: 60px;
  border-radius: 4px;
  overflow: hidden;
  background: #000;
}

.table-preview img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.table-offline {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #666;
}

.scan-content {
  padding: 20px 0;
}

.scan-progress {
  margin: 20px 0;
  text-align: center;
}

.scan-status {
  margin-top: 10px;
  color: #666;
}

.scan-results {
  margin-top: 20px;
}
</style>
