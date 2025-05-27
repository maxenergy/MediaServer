<template>
  <div class="face-database">
    <!-- 顶部操作栏 -->
    <el-card class="actions-card">
      <el-row :gutter="20">
        <el-col :span="12">
          <el-input v-model="searchKeyword" placeholder="搜索人员姓名或ID" clearable>
            <template #prefix>
              <el-icon><Search /></el-icon>
            </template>
          </el-input>
        </el-col>
        <el-col :span="12" class="actions-right">
          <el-button type="primary" @click="showAddPersonDialog" :icon="'Plus'">添加人员</el-button>
          <el-button @click="importPersons" :icon="'Upload'">批量导入</el-button>
          <el-button @click="exportPersons" :icon="'Download'">导出数据</el-button>
        </el-col>
      </el-row>
    </el-card>

    <!-- 人员列表 -->
    <el-card class="persons-card">
      <template #header>
        <div class="card-header">
          <span>人员数据库 ({{ filteredPersons.length }} 人)</span>
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
          <el-col :span="6" v-for="person in filteredPersons" :key="person.id">
            <el-card class="person-card" @click="showPersonDetail(person)">
              <div class="person-avatar">
                <img :src="person.avatar" :alt="person.name" />
                <div class="avatar-overlay">
                  <el-icon><View /></el-icon>
                </div>
              </div>
              <div class="person-info">
                <div class="person-name">{{ person.name }}</div>
                <div class="person-id">ID: {{ person.id }}</div>
                <div class="person-meta">
                  <el-tag :type="person.status === 'active' ? 'success' : 'info'" size="small">
                    {{ person.status === 'active' ? '启用' : '禁用' }}
                  </el-tag>
                  <span class="face-count">{{ person.face_count }} 张照片</span>
                </div>
                <div class="person-actions">
                  <el-button size="small" @click.stop="editPerson(person)" :icon="'Edit'">编辑</el-button>
                  <el-button size="small" type="danger" @click.stop="deletePerson(person)" :icon="'Delete'">删除</el-button>
                </div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>
      
      <!-- 列表视图 -->
      <div v-if="viewMode === 'list'" class="list-view">
        <el-table :data="filteredPersons" style="width: 100%">
          <el-table-column label="头像" width="80">
            <template #default="scope">
              <el-avatar :src="scope.row.avatar" :size="50" @click="showPersonDetail(scope.row)" style="cursor: pointer" />
            </template>
          </el-table-column>
          
          <el-table-column prop="name" label="姓名" width="150" />
          <el-table-column prop="id" label="人员ID" width="120" />
          <el-table-column prop="department" label="部门" width="120" />
          <el-table-column prop="position" label="职位" width="120" />
          
          <el-table-column label="状态" width="100">
            <template #default="scope">
              <el-tag :type="scope.row.status === 'active' ? 'success' : 'info'" size="small">
                {{ scope.row.status === 'active' ? '启用' : '禁用' }}
              </el-tag>
            </template>
          </el-table-column>
          
          <el-table-column label="照片数量" width="100">
            <template #default="scope">
              {{ scope.row.face_count }} 张
            </template>
          </el-table-column>
          
          <el-table-column prop="created_at" label="创建时间" width="180" />
          <el-table-column prop="last_seen" label="最后识别" width="180" />
          
          <el-table-column label="操作" width="200">
            <template #default="scope">
              <el-button size="small" @click="showPersonDetail(scope.row)" :icon="'View'">详情</el-button>
              <el-button size="small" @click="editPerson(scope.row)" :icon="'Edit'">编辑</el-button>
              <el-button size="small" type="danger" @click="deletePerson(scope.row)" :icon="'Delete'">删除</el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-card>

    <!-- 添加/编辑人员对话框 -->
    <el-dialog v-model="personDialog" :title="isEditing ? '编辑人员' : '添加人员'" width="600px">
      <el-form :model="personForm" :rules="personRules" ref="personFormRef" label-width="100px">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="姓名" prop="name">
              <el-input v-model="personForm.name" placeholder="请输入姓名" />
            </el-form-item>
            
            <el-form-item label="人员ID" prop="id">
              <el-input v-model="personForm.id" placeholder="请输入人员ID" :disabled="isEditing" />
            </el-form-item>
            
            <el-form-item label="部门">
              <el-input v-model="personForm.department" placeholder="请输入部门" />
            </el-form-item>
            
            <el-form-item label="职位">
              <el-input v-model="personForm.position" placeholder="请输入职位" />
            </el-form-item>
            
            <el-form-item label="状态">
              <el-radio-group v-model="personForm.status">
                <el-radio label="active">启用</el-radio>
                <el-radio label="inactive">禁用</el-radio>
              </el-radio-group>
            </el-form-item>
          </el-col>
          
          <el-col :span="12">
            <el-form-item label="人脸照片" prop="faces">
              <el-upload
                class="face-uploader"
                :action="uploadUrl"
                :show-file-list="false"
                :before-upload="beforeFaceUpload"
                :on-success="handleFaceUpload"
                multiple
                accept="image/*"
              >
                <div class="upload-area">
                  <el-icon class="upload-icon"><Plus /></el-icon>
                  <div class="upload-text">点击上传人脸照片</div>
                  <div class="upload-tip">支持 JPG、PNG 格式，建议上传多张不同角度的照片</div>
                </div>
              </el-upload>
              
              <div v-if="personForm.faces.length > 0" class="face-preview">
                <div v-for="(face, index) in personForm.faces" :key="index" class="face-item">
                  <img :src="face.url" :alt="`Face ${index + 1}`" />
                  <div class="face-actions">
                    <el-button size="small" type="danger" @click="removeFace(index)" :icon="'Delete'"></el-button>
                  </div>
                </div>
              </div>
            </el-form-item>
          </el-col>
        </el-row>
        
        <el-form-item label="备注">
          <el-input v-model="personForm.notes" type="textarea" placeholder="备注信息" />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="personDialog = false">取消</el-button>
        <el-button type="primary" @click="savePerson">保存</el-button>
      </template>
    </el-dialog>

    <!-- 人员详情对话框 -->
    <el-dialog v-model="detailDialog" :title="selectedPerson?.name" width="800px">
      <div v-if="selectedPerson" class="person-detail">
        <el-row :gutter="20">
          <el-col :span="8">
            <div class="detail-avatar">
              <img :src="selectedPerson.avatar" :alt="selectedPerson.name" />
            </div>
          </el-col>
          
          <el-col :span="16">
            <el-descriptions :column="2" border>
              <el-descriptions-item label="姓名">{{ selectedPerson.name }}</el-descriptions-item>
              <el-descriptions-item label="人员ID">{{ selectedPerson.id }}</el-descriptions-item>
              <el-descriptions-item label="部门">{{ selectedPerson.department }}</el-descriptions-item>
              <el-descriptions-item label="职位">{{ selectedPerson.position }}</el-descriptions-item>
              <el-descriptions-item label="状态">
                <el-tag :type="selectedPerson.status === 'active' ? 'success' : 'info'">
                  {{ selectedPerson.status === 'active' ? '启用' : '禁用' }}
                </el-tag>
              </el-descriptions-item>
              <el-descriptions-item label="照片数量">{{ selectedPerson.face_count }} 张</el-descriptions-item>
              <el-descriptions-item label="创建时间">{{ selectedPerson.created_at }}</el-descriptions-item>
              <el-descriptions-item label="最后识别">{{ selectedPerson.last_seen }}</el-descriptions-item>
              <el-descriptions-item label="识别次数">{{ selectedPerson.recognition_count }} 次</el-descriptions-item>
              <el-descriptions-item label="备注" :span="2">{{ selectedPerson.notes || '无' }}</el-descriptions-item>
            </el-descriptions>
          </el-col>
        </el-row>
        
        <el-divider>人脸照片</el-divider>
        
        <div class="face-gallery">
          <div v-for="(face, index) in selectedPerson.faces" :key="index" class="gallery-item">
            <img :src="face.url" :alt="`Face ${index + 1}`" @click="previewFace(face)" />
          </div>
        </div>
        
        <el-divider>识别记录</el-divider>
        
        <el-table :data="selectedPerson.recognition_history" style="width: 100%" max-height="300">
          <el-table-column prop="timestamp" label="时间" width="180" />
          <el-table-column prop="camera_name" label="摄像头" width="150" />
          <el-table-column prop="confidence" label="置信度" width="100">
            <template #default="scope">
              {{ Math.round(scope.row.confidence * 100) }}%
            </template>
          </el-table-column>
          <el-table-column label="截图" width="100">
            <template #default="scope">
              <img :src="scope.row.snapshot" class="recognition-snapshot" @click="previewSnapshot(scope.row)" />
            </template>
          </el-table-column>
        </el-table>
      </div>
      
      <template #footer>
        <el-button @click="detailDialog = false">关闭</el-button>
        <el-button type="primary" @click="editPerson(selectedPerson)">编辑</el-button>
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
const personDialog = ref(false)
const detailDialog = ref(false)
const isEditing = ref(false)
const selectedPerson = ref(null)
const personFormRef = ref()

const uploadUrl = '/api/v1/upload/face'

// 人员表单
const personForm = reactive({
  id: '',
  name: '',
  department: '',
  position: '',
  status: 'active',
  faces: [],
  notes: ''
})

// 表单验证规则
const personRules = {
  name: [{ required: true, message: '请输入姓名', trigger: 'blur' }],
  id: [{ required: true, message: '请输入人员ID', trigger: 'blur' }],
  faces: [{ required: true, message: '请上传至少一张人脸照片', trigger: 'change' }]
}

// 模拟人员数据
const persons = ref([
  {
    id: 'P001',
    name: '张三',
    department: '技术部',
    position: '软件工程师',
    status: 'active',
    avatar: '/api/v1/avatar/p001.jpg',
    face_count: 3,
    created_at: '2024-01-10 09:00:00',
    last_seen: '2024-01-15 14:25:10',
    recognition_count: 45,
    notes: '技术部核心成员',
    faces: [
      { url: '/api/v1/face/p001_1.jpg' },
      { url: '/api/v1/face/p001_2.jpg' },
      { url: '/api/v1/face/p001_3.jpg' }
    ],
    recognition_history: [
      { timestamp: '2024-01-15 14:25:10', camera_name: '办公室摄像头', confidence: 0.92, snapshot: '/api/v1/snapshot/r001.jpg' },
      { timestamp: '2024-01-15 09:15:30', camera_name: '前门摄像头', confidence: 0.88, snapshot: '/api/v1/snapshot/r002.jpg' }
    ]
  },
  {
    id: 'P002',
    name: '李四',
    department: '市场部',
    position: '市场经理',
    status: 'active',
    avatar: '/api/v1/avatar/p002.jpg',
    face_count: 2,
    created_at: '2024-01-08 10:30:00',
    last_seen: '2024-01-14 16:45:20',
    recognition_count: 32,
    notes: '市场部负责人',
    faces: [
      { url: '/api/v1/face/p002_1.jpg' },
      { url: '/api/v1/face/p002_2.jpg' }
    ],
    recognition_history: [
      { timestamp: '2024-01-14 16:45:20', camera_name: '前门摄像头', confidence: 0.85, snapshot: '/api/v1/snapshot/r003.jpg' }
    ]
  }
])

// 计算属性
const filteredPersons = computed(() => {
  if (!searchKeyword.value) return persons.value
  return persons.value.filter(person => 
    person.name.includes(searchKeyword.value) || 
    person.id.includes(searchKeyword.value)
  )
})

// 方法
const showAddPersonDialog = () => {
  isEditing.value = false
  resetPersonForm()
  personDialog.value = true
}

const editPerson = (person) => {
  isEditing.value = true
  Object.assign(personForm, person)
  personDialog.value = true
}

const resetPersonForm = () => {
  Object.assign(personForm, {
    id: '',
    name: '',
    department: '',
    position: '',
    status: 'active',
    faces: [],
    notes: ''
  })
}

const beforeFaceUpload = (file) => {
  const isImage = file.type.startsWith('image/')
  const isLt2M = file.size / 1024 / 1024 < 2

  if (!isImage) {
    ElMessage.error('只能上传图片文件!')
    return false
  }
  if (!isLt2M) {
    ElMessage.error('图片大小不能超过 2MB!')
    return false
  }
  return true
}

const handleFaceUpload = (response, file) => {
  personForm.faces.push({
    url: URL.createObjectURL(file.raw),
    file: file.raw
  })
  ElMessage.success('照片上传成功')
}

const removeFace = (index) => {
  personForm.faces.splice(index, 1)
}

const savePerson = () => {
  personFormRef.value.validate((valid) => {
    if (valid) {
      if (isEditing.value) {
        // 更新现有人员
        const index = persons.value.findIndex(p => p.id === personForm.id)
        if (index > -1) {
          Object.assign(persons.value[index], personForm)
          ElMessage.success('人员信息更新成功')
        }
      } else {
        // 添加新人员
        const newPerson = {
          ...personForm,
          avatar: personForm.faces[0]?.url || '/api/v1/avatar/default.jpg',
          face_count: personForm.faces.length,
          created_at: new Date().toLocaleString(),
          last_seen: '-',
          recognition_count: 0,
          recognition_history: []
        }
        persons.value.push(newPerson)
        ElMessage.success('人员添加成功')
      }
      personDialog.value = false
    }
  })
}

const deletePerson = (person) => {
  ElMessageBox.confirm(`确定要删除人员 "${person.name}" 吗？`, '确认删除', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    type: 'warning'
  }).then(() => {
    const index = persons.value.findIndex(p => p.id === person.id)
    if (index > -1) {
      persons.value.splice(index, 1)
      ElMessage.success('人员已删除')
    }
  }).catch(() => {
    ElMessage.info('已取消删除')
  })
}

const showPersonDetail = (person) => {
  selectedPerson.value = person
  detailDialog.value = true
}

const importPersons = () => {
  ElMessage.info('批量导入功能开发中...')
}

const exportPersons = () => {
  ElMessage.success('人员数据导出中...')
}

const previewFace = (face) => {
  // 实现图片预览功能
  ElMessage.info('图片预览功能')
}

const previewSnapshot = (record) => {
  // 实现识别记录截图预览
  ElMessage.info('识别记录预览功能')
}

// 生命周期
onMounted(() => {
  // 初始化数据
})
</script>

<style scoped>
.face-database {
  padding: 20px;
}

.actions-card {
  margin-bottom: 20px;
}

.actions-right {
  text-align: right;
}

.persons-card {
  min-height: 600px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.person-card {
  cursor: pointer;
  margin-bottom: 20px;
  transition: all 0.3s ease;
  text-align: center;
}

.person-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
}

.person-avatar {
  position: relative;
  width: 100px;
  height: 100px;
  margin: 0 auto 15px;
  border-radius: 50%;
  overflow: hidden;
}

.person-avatar img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.avatar-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0,0,0,0.6);
  color: white;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.person-card:hover .avatar-overlay {
  opacity: 1;
}

.person-name {
  font-weight: bold;
  font-size: 16px;
  margin-bottom: 5px;
}

.person-id {
  color: #666;
  font-size: 12px;
  margin-bottom: 8px;
}

.person-meta {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 10px;
  margin-bottom: 15px;
}

.face-count {
  font-size: 12px;
  color: #666;
}

.person-actions {
  display: flex;
  justify-content: center;
  gap: 5px;
}

.face-uploader {
  width: 100%;
}

.upload-area {
  border: 2px dashed #d9d9d9;
  border-radius: 6px;
  padding: 40px;
  text-align: center;
  cursor: pointer;
  transition: border-color 0.3s ease;
}

.upload-area:hover {
  border-color: #409eff;
}

.upload-icon {
  font-size: 28px;
  color: #8c939d;
  margin-bottom: 10px;
}

.upload-text {
  color: #606266;
  margin-bottom: 5px;
}

.upload-tip {
  color: #909399;
  font-size: 12px;
}

.face-preview {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  margin-top: 15px;
}

.face-item {
  position: relative;
  width: 80px;
  height: 80px;
  border-radius: 6px;
  overflow: hidden;
}

.face-item img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.face-actions {
  position: absolute;
  top: 5px;
  right: 5px;
}

.detail-avatar {
  text-align: center;
}

.detail-avatar img {
  width: 150px;
  height: 150px;
  border-radius: 50%;
  object-fit: cover;
}

.face-gallery {
  display: flex;
  flex-wrap: wrap;
  gap: 15px;
  margin: 20px 0;
}

.gallery-item {
  width: 100px;
  height: 100px;
  border-radius: 6px;
  overflow: hidden;
  cursor: pointer;
}

.gallery-item img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: transform 0.3s ease;
}

.gallery-item:hover img {
  transform: scale(1.1);
}

.recognition-snapshot {
  width: 60px;
  height: 40px;
  object-fit: cover;
  border-radius: 4px;
  cursor: pointer;
}
</style>
