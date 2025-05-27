<template>
  <div class="ai-management">
    <!-- 顶部统计卡片 -->
    <el-row :gutter="20" class="stats-row">
      <el-col :span="6">
        <el-card class="stats-card">
          <div class="stats-content">
            <div class="stats-icon detection">
              <el-icon><View /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ aiStats.total_detections }}</div>
              <div class="stats-label">总检测数</div>
            </div>
          </div>
        </el-card>
      </el-col>
      
      <el-col :span="6">
        <el-card class="stats-card">
          <div class="stats-content">
            <div class="stats-icon face">
              <el-icon><User /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ aiStats.total_faces_detected }}</div>
              <div class="stats-label">人脸识别</div>
            </div>
          </div>
        </el-card>
      </el-col>
      
      <el-col :span="6">
        <el-card class="stats-card">
          <div class="stats-content">
            <div class="stats-icon plate">
              <el-icon><Tickets /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ aiStats.total_plates_detected }}</div>
              <div class="stats-label">车牌识别</div>
            </div>
          </div>
        </el-card>
      </el-col>
      
      <el-col :span="6">
        <el-card class="stats-card">
          <div class="stats-content">
            <div class="stats-icon performance">
              <el-icon><Timer /></el-icon>
            </div>
            <div class="stats-info">
              <div class="stats-value">{{ aiStats.avg_processing_time_ms.toFixed(1) }}ms</div>
              <div class="stats-label">平均处理时间</div>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- AI配置和控制 -->
    <el-row :gutter="20">
      <el-col :span="12">
        <el-card title="AI检测配置" class="config-card">
          <template #header>
            <div class="card-header">
              <span>AI检测配置</span>
              <el-switch v-model="aiConfig.enabled" @change="toggleAI" />
            </div>
          </template>
          
          <el-form :model="aiConfig" label-width="120px">
            <el-form-item label="检测间隔">
              <el-input-number
                v-model="aiConfig.detection_interval_ms"
                :min="100"
                :max="10000"
                :step="100"
                controls-position="right"
              />
              <span class="form-help">毫秒</span>
            </el-form-item>
            
            <el-form-item label="置信度阈值">
              <el-slider
                v-model="aiConfig.confidence_threshold"
                :min="0.1"
                :max="1.0"
                :step="0.1"
                show-input
              />
            </el-form-item>
            
            <el-form-item label="GPU加速">
              <el-switch v-model="aiConfig.gpu_enabled" />
            </el-form-item>
            
            <el-form-item label="检测类型">
              <el-checkbox-group v-model="enabledDetections">
                <el-checkbox label="object_detection">物体检测</el-checkbox>
                <el-checkbox label="face_recognition">人脸识别</el-checkbox>
                <el-checkbox label="plate_recognition">车牌识别</el-checkbox>
              </el-checkbox-group>
            </el-form-item>
            
            <el-form-item>
              <el-button type="primary" @click="saveAIConfig">保存配置</el-button>
              <el-button @click="resetAIConfig">重置</el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-col>
      
      <el-col :span="12">
        <el-card title="活跃流状态" class="streams-card">
          <div class="streams-list">
            <div
              v-for="stream in activeStreams"
              :key="stream.id"
              class="stream-item"
              :class="{ 'ai-enabled': stream.ai_enabled }"
            >
              <div class="stream-info">
                <div class="stream-name">{{ stream.name }}</div>
                <div class="stream-status">
                  <el-tag :type="stream.status === 'online' ? 'success' : 'danger'" size="small">
                    {{ stream.status === 'online' ? '在线' : '离线' }}
                  </el-tag>
                  <el-tag v-if="stream.ai_enabled" type="primary" size="small">AI已启用</el-tag>
                </div>
              </div>
              <div class="stream-controls">
                <el-switch
                  v-model="stream.ai_enabled"
                  @change="toggleStreamAI(stream.id, stream.ai_enabled)"
                />
              </div>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <!-- AI模型管理 -->
    <el-card title="AI模型管理" class="models-card">
      <el-table :data="aiModels" style="width: 100%">
        <el-table-column prop="name" label="模型名称" width="200" />
        <el-table-column prop="type" label="类型" width="150">
          <template #default="scope">
            <el-tag :type="getModelTypeColor(scope.row.type)">
              {{ getModelTypeName(scope.row.type) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="path" label="路径" />
        <el-table-column prop="loaded" label="状态" width="100">
          <template #default="scope">
            <el-tag :type="scope.row.loaded ? 'success' : 'info'">
              {{ scope.row.loaded ? '已加载' : '未加载' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="200">
          <template #default="scope">
            <el-button
              v-if="!scope.row.loaded"
              size="small"
              type="primary"
              @click="loadModel(scope.row)"
            >
              加载
            </el-button>
            <el-button
              v-else
              size="small"
              type="danger"
              @click="unloadModel(scope.row)"
            >
              卸载
            </el-button>
            <el-button size="small" @click="editModel(scope.row)">编辑</el-button>
          </template>
        </el-table-column>
      </el-table>
      
      <div class="models-actions">
        <el-button type="primary" @click="showAddModelDialog">添加模型</el-button>
        <el-button @click="refreshModels">刷新</el-button>
      </div>
    </el-card>

    <!-- 添加模型对话框 -->
    <el-dialog v-model="addModelDialog" title="添加AI模型" width="500px">
      <el-form :model="newModel" label-width="100px">
        <el-form-item label="模型名称" required>
          <el-input v-model="newModel.name" placeholder="请输入模型名称" />
        </el-form-item>
        <el-form-item label="模型类型" required>
          <el-select v-model="newModel.type" placeholder="选择模型类型">
            <el-option label="物体检测" value="object_detection" />
            <el-option label="人脸检测" value="face_detection" />
            <el-option label="人脸识别" value="face_recognition" />
            <el-option label="车牌检测" value="plate_detection" />
            <el-option label="车牌识别" value="plate_recognition" />
          </el-select>
        </el-form-item>
        <el-form-item label="模型路径" required>
          <el-input v-model="newModel.path" placeholder="请输入模型文件路径" />
        </el-form-item>
        <el-form-item label="描述">
          <el-input v-model="newModel.description" type="textarea" placeholder="模型描述" />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="addModelDialog = false">取消</el-button>
        <el-button type="primary" @click="addModel">确定</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { ElMessage } from 'element-plus'

// 响应式数据
const aiStats = ref({
  total_detections: 1234,
  total_faces_detected: 567,
  total_plates_detected: 89,
  avg_processing_time_ms: 45.6,
  active_streams: 4
})

const aiConfig = reactive({
  enabled: true,
  detection_interval_ms: 1000,
  confidence_threshold: 0.5,
  gpu_enabled: false
})

const enabledDetections = ref(['object_detection', 'face_recognition', 'plate_recognition'])

const activeStreams = ref([
  { id: 'stream1', name: '前门摄像头', status: 'online', ai_enabled: true },
  { id: 'stream2', name: '后门摄像头', status: 'online', ai_enabled: true },
  { id: 'stream3', name: '停车场摄像头', status: 'offline', ai_enabled: false },
  { id: 'stream4', name: '办公室摄像头', status: 'online', ai_enabled: false }
])

const aiModels = ref([
  { name: 'YOLOv8n', type: 'object_detection', path: 'models/yolov8n.onnx', loaded: true },
  { name: 'RetinaFace', type: 'face_detection', path: 'models/retinaface.onnx', loaded: true },
  { name: 'ArcFace', type: 'face_recognition', path: 'models/arcface.onnx', loaded: false },
  { name: 'PlateDetect', type: 'plate_detection', path: 'models/plate_detect.onnx', loaded: true },
  { name: 'PlateOCR', type: 'plate_recognition', path: 'models/plate_ocr.onnx', loaded: false }
])

const addModelDialog = ref(false)
const newModel = reactive({
  name: '',
  type: '',
  path: '',
  description: ''
})

// 方法
const toggleAI = (enabled) => {
  ElMessage.success(`AI检测已${enabled ? '启用' : '禁用'}`)
}

const saveAIConfig = () => {
  ElMessage.success('AI配置已保存')
}

const resetAIConfig = () => {
  Object.assign(aiConfig, {
    enabled: true,
    detection_interval_ms: 1000,
    confidence_threshold: 0.5,
    gpu_enabled: false
  })
  enabledDetections.value = ['object_detection', 'face_recognition', 'plate_recognition']
  ElMessage.info('配置已重置')
}

const toggleStreamAI = (streamId, enabled) => {
  ElMessage.success(`流 ${streamId} 的AI检测已${enabled ? '启用' : '禁用'}`)
}

const getModelTypeColor = (type) => {
  const colors = {
    'object_detection': 'primary',
    'face_detection': 'success',
    'face_recognition': 'success',
    'plate_detection': 'warning',
    'plate_recognition': 'warning'
  }
  return colors[type] || 'info'
}

const getModelTypeName = (type) => {
  const names = {
    'object_detection': '物体检测',
    'face_detection': '人脸检测',
    'face_recognition': '人脸识别',
    'plate_detection': '车牌检测',
    'plate_recognition': '车牌识别'
  }
  return names[type] || type
}

const loadModel = (model) => {
  model.loaded = true
  ElMessage.success(`模型 ${model.name} 加载成功`)
}

const unloadModel = (model) => {
  model.loaded = false
  ElMessage.success(`模型 ${model.name} 卸载成功`)
}

const editModel = (model) => {
  ElMessage.info(`编辑模型: ${model.name}`)
}

const showAddModelDialog = () => {
  addModelDialog.value = true
}

const addModel = () => {
  if (!newModel.name || !newModel.type || !newModel.path) {
    ElMessage.error('请填写必填字段')
    return
  }
  
  aiModels.value.push({
    name: newModel.name,
    type: newModel.type,
    path: newModel.path,
    loaded: false
  })
  
  // 重置表单
  Object.assign(newModel, {
    name: '',
    type: '',
    path: '',
    description: ''
  })
  
  addModelDialog.value = false
  ElMessage.success('模型添加成功')
}

const refreshModels = () => {
  ElMessage.success('模型列表已刷新')
}

// 生命周期
onMounted(() => {
  // 初始化数据
})
</script>

<style scoped>
.ai-management {
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

.stats-icon.detection { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
.stats-icon.face { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); }
.stats-icon.plate { background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%); }
.stats-icon.performance { background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%); }

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
  margin-top: 5px;
}

.config-card, .streams-card, .models-card {
  margin-bottom: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.form-help {
  margin-left: 10px;
  color: #909399;
  font-size: 12px;
}

.streams-list {
  max-height: 300px;
  overflow-y: auto;
}

.stream-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px;
  border: 1px solid #ebeef5;
  border-radius: 6px;
  margin-bottom: 8px;
  transition: all 0.3s ease;
}

.stream-item:hover {
  background-color: #f5f7fa;
}

.stream-item.ai-enabled {
  border-color: #409eff;
  background-color: #ecf5ff;
}

.stream-info {
  flex: 1;
}

.stream-name {
  font-weight: bold;
  margin-bottom: 4px;
}

.stream-status {
  display: flex;
  gap: 8px;
}

.models-actions {
  margin-top: 20px;
  text-align: right;
}
</style>
