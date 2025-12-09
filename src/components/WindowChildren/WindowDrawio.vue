<template>
  <Window 
    @height_changed="window_height_changed" 
    :startpos_x="startpos_x" 
    :startpos_y="startpos_y" 
    :uuid="uuid" 
    :zindex="zindex" 
    :minimized="minimized" 
    @width_changed="window_width_changed"
    @resize_start="show_blocker=true" 
    @resize_end="show_blocker=false"
  >
    <template v-slot:header>
      <div class="tw-flex tw-items-center tw-select-none" style="pointer-events:none;"> 
        <img src="../../assets/images/icons/drawio.png" alt="" style="pointer-events:auto;" class="tw-w-8 tw-h-7 tw-ml-4">
        <div class="tw-h-9 tw-flex tw-items-center tw-justify-center tw-rounded-lg tw-bg-mygray-b4 hover:tw-bg-mygray-b5 tw-ml-3" 
             style="pointer-events:auto;" 
             :style="{'opacity':global_focus===uuid?'1':'0.85'}" 
             @click="title_clicked">
          <div class="tw-h-full tw-px-3 tw-text-sm tw-font-normal tw-text-gray-50 tw-pt-2" 
               style="white-space:nowrap;text-overflow: ellipsis;overflow: hidden;max-width:300px">
            {{filename}}
          </div>
        </div>
      </div>
    </template>
    <template v-slot:content>
      <div class="tw-w-full tw-h-full tw-rounded-b-2xl tw-flex tw-flex-col tw-bg-white" @contextmenu.prevent="mr_clicked">
        <!-- 工具栏 -->
        <div class="tw-h-10 tw-w-full tw-border-b tw-border-gray-200 tw-items-center tw-flex tw-flex-row tw-px-3 tw-gap-2" style="background-color:#fdfdfd">
          <button 
            class="tw-flex tw-justify-center tw-items-center hover:tw-bg-gray-100 tw-px-3 tw-py-1 tw-rounded-lg tw-text-sm tw-text-gray-700 tw-outline-none"
            @click="zoomIn"
            title="放大">
            <v-icon small>mdi-magnify-plus</v-icon>
          </button>
          <button 
            class="tw-flex tw-justify-center tw-items-center hover:tw-bg-gray-100 tw-px-3 tw-py-1 tw-rounded-lg tw-text-sm tw-text-gray-700 tw-outline-none"
            @click="zoomOut"
            title="缩小">
            <v-icon small>mdi-magnify-minus</v-icon>
          </button>
          <button 
            class="tw-flex tw-justify-center tw-items-center hover:tw-bg-gray-100 tw-px-3 tw-py-1 tw-rounded-lg tw-text-sm tw-text-gray-700 tw-outline-none"
            @click="resetZoom"
            title="重置缩放">
            <v-icon small>mdi-magnify</v-icon>
          </button>
          <div class="vl tw-mx-1"></div>
          <button 
            class="tw-flex tw-justify-center tw-items-center hover:tw-bg-gray-100 tw-px-3 tw-py-1 tw-rounded-lg tw-text-sm tw-text-gray-700 tw-outline-none"
            @click="openInNewTab"
            title="在新标签页中打开">
            <v-icon small>mdi-open-in-new</v-icon>
            <span class="tw-ml-1">在浏览器中打开</span>
          </button>
          <div class="tw-flex-grow"></div>
          <div class="tw-text-xs tw-text-gray-600">
            缩放: {{Math.round(zoom * 100)}}%
          </div>
        </div>
        
        <!-- Draw.io 查看器容器 -->
        <div class="tw-flex-grow tw-w-full tw-relative tw-overflow-auto change-srollbar" :style="{'height': cont_height +'px'}">
          <div v-if="loading" class="tw-absolute tw-inset-0 tw-flex tw-items-center tw-justify-center">
            <div class="tw-text-center">
              <v-progress-circular indeterminate color="primary" size="64"></v-progress-circular>
              <div class="tw-mt-4 tw-text-gray-600">加载中...</div>
            </div>
          </div>
          
          <div v-else-if="error" class="tw-absolute tw-inset-0 tw-flex tw-items-center tw-justify-center">
            <div class="tw-text-center tw-px-4">
              <v-icon large color="error">mdi-alert-circle</v-icon>
              <div class="tw-mt-4 tw-text-gray-700 tw-font-medium">加载失败</div>
              <div class="tw-mt-2 tw-text-gray-600 tw-text-sm">{{errorMessage}}</div>
              <button 
                class="tw-mt-4 tw-px-4 tw-py-2 tw-bg-blue-500 tw-text-white tw-rounded-lg hover:tw-bg-blue-600 tw-outline-none"
                @click="reload">
                重试
              </button>
            </div>
          </div>
          
          <!-- 使用 iframe 嵌入 draw.io 查看器 -->
          <div v-else class="tw-w-full tw-h-full tw-flex tw-items-center tw-justify-center tw-bg-gray-50">
            <iframe 
              v-if="viewerUrl"
              ref="drawioFrame"
              :src="viewerUrl"
              class="tw-w-full tw-h-full tw-border-0"
              @load="onFrameLoad"
              style="border:0;"
            ></iframe>
            <div v-else class="tw-text-gray-500">
              生成查看器 URL 中...
            </div>
          </div>
          
          <!-- 遮罩层用于调整窗口大小时阻止 iframe 捕获事件 -->
          <div v-if="show_blocker" class="tw-absolute tw-inset-0 tw-bg-transparent" style="z-index:9999"></div>
        </div>
        
        <!-- 状态栏 -->
        <div class="tw-h-8 tw-w-full tw-rounded-b-2xl tw-border-t tw-border-gray-200 tw-items-center tw-flex tw-flex-row-reverse tw-px-4" style="background-color:#fdfdfd">
          <div class="tw-flex-none tw-h-full tw-flex tw-justify-center tw-items-center tw-select-none">
            <div class="tw-flex tw-justify-center tw-items-center tw-rounded-lg tw-text-xs tw-text-gray-700">
              {{size_format_converter(size)}}
            </div>
          </div>
          <div class="vl tw-mx-3"></div>
          <div class="tw-flex-none tw-h-full tw-flex tw-justify-center tw-items-center tw-select-none">
            <div class="tw-flex tw-justify-center tw-items-center tw-rounded-lg tw-text-xs tw-text-gray-700">
              Draw.io 图表
            </div>
          </div>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'

export default {
  name: 'WindowDrawio',
  components: {
    Window,
  },
  data(){
    return {
      cont_margin: 90,
      cont_height: 600 - 90,
      cont_width: 800,
      loading: true,
      error: false,
      errorMessage: '',
      show_blocker: false,
      zoom: 1.0,
      diagramXml: null, // 存储 Draw.io XML 数据
    }
  },
  props:{
    uuid: String,
    startpos_x: {
      default: 60,
    },
    startpos_y: {
      default: 60
    },
    zindex: {
      type: Number,
      default: 999,
    },
    minimized: {
      type: Boolean,
      default: false,
    },
    filesrc: {
      type: String,
      default: "",
    },
    filename: {
      type: String,
      default: "",
    },
    size: {
      type: Number,
      default: 0,
    },
  },
  created(){
  },
  async mounted(){
    this.$store.commit('refresh_window_focus', {uuid: this.uuid})
    // 加载 Draw.io 文件
    await this.loadDrawioFile()
  },
  watch:{
  },
  computed:{
    global_focus(){
      return this.$store.state.current_focus
    },
    // 生成 draw.io 查看器的 URL
    viewerUrl() {
      if (!this.diagramXml) {
        return ''
      }
      
      // 使用 viewer.diagrams.net 配合 URL fragment
      const compressed = encodeURIComponent(this.diagramXml)
      
      // 使用 #R 协议直接传递 XML 数据
      return `https://viewer.diagrams.net/?lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=${encodeURIComponent(this.filename)}#R${compressed}`
    }
  },
  methods:{
    async loadDrawioFile() {
      try {
        this.loading = true
        this.error = false
        
        // 构建文件 URL - 需要对路径的每个部分进行编码
        let fileUrl = '/' + this.filesrc.replace(/^\.?\//, '')
        
        // 将路径拆分,对每个部分单独编码(保留斜杠)
        const parts = fileUrl.split('/')
        fileUrl = parts.map(part => encodeURIComponent(part)).join('/')
        
        console.log('正在加载 Draw.io 文件:', fileUrl)
        
        // 使用 fetch 获取文件内容
        const response = await fetch(fileUrl)
        
        if (!response.ok) {
          throw new Error(`HTTP ${response.status}: ${response.statusText}`)
        }
        
        // 读取文件内容
        const xmlText = await response.text()
        this.diagramXml = xmlText
        console.log('Draw.io 文件加载成功,长度:', xmlText.length)
        
        // 数据加载完成，viewerUrl 会自动生成
        this.loading = false
      } catch (err) {
        console.error('加载 Draw.io 文件失败:', err)
        this.error = true
        this.errorMessage = `加载失败: ${err.message}`
        this.loading = false
      }
    },

    window_height_changed(val){
      this.cont_height = val - this.cont_margin
    },
    window_width_changed(val){
      this.cont_width = val
    },
    title_clicked(){
      this.$store.commit('refresh_window_focus', {uuid: this.uuid})
    },
    mr_clicked(){
      this.$store.commit('show_context_menu')
    },
    go_focus(){
      this.$store.commit('refresh_window_focus', {uuid: this.uuid})
    },
    size_format_converter(val){
      if (val < 1000) {
        return val + " B"
      } else if (val < 1000000) {
        return (val / 1000).toFixed(2) + " KB"
      } else if (val < 1000000000) {
        return (val / 1000000).toFixed(2) + " MB"
      } else {
        return (val / 1000000000).toFixed(2) + " GB"
      }
    },
    onFrameLoad() {
      // iframe 加载完成
      this.loading = false
      this.error = false
    },
    reload() {
      this.loading = true
      this.error = false
      this.errorMessage = ''
      // 重新加载 iframe
      if (this.$refs.drawioFrame) {
        const currentSrc = this.$refs.drawioFrame.src
        this.$refs.drawioFrame.src = ''
        this.$nextTick(() => {
          this.$refs.drawioFrame.src = currentSrc
        })
      }
    },
    zoomIn() {
      this.zoom = Math.min(this.zoom + 0.1, 2.0)
    },
    zoomOut() {
      this.zoom = Math.max(this.zoom - 0.1, 0.5)
    },
    resetZoom() {
      this.zoom = 1.0
    },
    openInNewTab() {
      if (!this.diagramXml) {
        return
      }
      // 在新标签页中打开 draw.io 编辑器，使用 XML 数据
      const editUrl = `https://app.diagrams.net/#R${encodeURIComponent(this.diagramXml)}`
      window.open(editUrl, '_blank')
    }
  }
}
</script>

<style scoped>
.vl {
  border-left: 1.5px solid rgba(200, 200, 200, 0.5);
  height: 60%;
}

.change-srollbar::-webkit-scrollbar {
  width: 10px;
  height: 10px;
}

.change-srollbar::-webkit-scrollbar-track {
  background: #f1f1f1;
}

.change-srollbar::-webkit-scrollbar-thumb {
  background: #888;
  border-radius: 5px;
}

.change-srollbar::-webkit-scrollbar-thumb:hover {
  background: #555;
}
</style>
