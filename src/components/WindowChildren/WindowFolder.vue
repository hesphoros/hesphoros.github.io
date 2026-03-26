<template>
  <Window @height_changed="window_height_changed" @width_changed="window_width_changed" :startpos_x="startpos_x" :startpos_y="startpos_y" :uuid="uuid" :zindex="zindex" :minimized="minimized">
    <template v-slot:header>
      <div class=" tw-ml-4 tw-font-bold tw-tracking-wider tw-flex tw-items-center tw-select-none" style="pointer-events:none;" > 
        <img src="../../assets/images/icons/explorer.png" alt="" style="pointer-events:auto;" class=" tw-w-8 tw-h-8">
        <div class="tw-w-9 tw-h-9 tw-rounded-l-lg tw-flex tw-justify-center tw-items-center tw-ml-6 tw-cursor-pointer" style="background-color:#f0f0f0;pointer-events:auto;" :style="{'opacity':global_focus===uuid?'1':'0.85'}" @click="nav_up">
          <v-icon style="color:#b0b4bf">mdi-chevron-left</v-icon>
        </div>
        <div class=" tw-h-9 tw-w-0.5" style="background-color:#ebebeb"></div>
        <div class="tw-w-9 tw-h-9 tw-rounded-r-lg tw-flex tw-justify-center tw-items-center" style="background-color:#f0f0f0;pointer-events:auto;" :style="{'opacity':global_focus===uuid?'1':'0.85'}">
          <v-icon style="color:#b0b4bf">mdi-chevron-right</v-icon>
        </div>
        <div class="tw-h-9 tw-px-3  tw-flex tw-items-center tw-justify-center tw-bg-mygray-b2  tw-rounded-lg tw-ml-3 tw-cursor-pointer" style="pointer-events:auto;" :style="{'opacity':global_focus===uuid?'1':'0.85'}" @click="go_breadcrumb_desktop" title="桌面根目录">
          <v-icon small>mdi-home-heart</v-icon>
        </div>
        <div class="tw-h-9 tw-px-3 tw-flex tw-items-center tw-justify-center tw-rounded-lg tw-ml-3 tw-text-sm tw-font-normal tw-cursor-pointer" style="pointer-events:auto;" :class="breadcrumbDesktopClass" :style="{'opacity':global_focus===uuid?'1':'0.85'}" @click="go_breadcrumb_desktop">
          Desktop
        </div>
        <div class="tw-h-9 tw-px-3 tw-flex tw-items-center tw-justify-center tw-rounded-lg tw-ml-3 tw-text-sm tw-font-normal tw-cursor-pointer" style="pointer-events:auto;" v-for="(item,i) in open_openpath" :key="'p-'+i" :class="breadcrumbSegmentClass(i)" :style="{'opacity':global_focus===uuid?'1':'0.85'}" @click="go_breadcrumb_at(i)">
          {{item}}
        </div>
      </div>
    </template>
    <template v-slot:content>
      <div class="tw-w-full tw-rounded-b-2xl tw-flex tw-select-none" :style="{'height':cont_height + 'px'}" v-if="true" @contextmenu.prevent="mr_clicked">
        <div ref="leftbar" class="tw-flex-none tw-bg-white tw-rounded-bl-2xl tw-overflow-hidden tw-flex tw-flex-row tw-select-none" style="min-width:160px;max-width:260px;width:208px" :style="{'opacity':global_focus===uuid?'1':'0.85'}">
          <div class=" tw-flex-grow tw-h-full tw-flex tw-items-center tw-flex-col tw-px-3">
            <WindowFolderIcon :icon="'clock-time-three'" :text="'Recently'" :disabled="true"/>
            <WindowFolderIcon :icon="'home-heart'" :text="'Home'" :active="atDesktopRoot" @activate="goSidebarPlace('home')"/>
            <WindowFolderIcon :icon="'television-guide'" :text="'Desktop'" :active="atDesktopRoot" @activate="goSidebarPlace('desktop')"/>
            <WindowFolderIcon :icon="'filmstrip'" :text="'Videos'" :disabled="true"/>
            <WindowFolderIcon :icon="'music'" :text="'Musics'" :disabled="true"/>
            <WindowFolderIcon :icon="'image'" :text="'Pictures'" :disabled="true"/>
            <WindowFolderIcon :icon="'file-document'" :text="'Documents'" :disabled="true"/>
            <WindowFolderIcon :icon="'delete'" :text="'Recycle Bin'" :disabled="true"/>
            <div style="width:100%;height:2px;background-color:#efefef;margin-top:1px;margin-bottom:1px"></div>
            <WindowFolderIcon :icon="'desktop-mac-dashboard'" :text="'This PC'" :active="atDesktopRoot" @activate="goSidebarPlace('thispc')"/>
            <WindowFolderIcon :icon="'harddisk'" :text="'Root'" :active="atDesktopRoot" @activate="goSidebarPlace('root')"/>
            <div style="width:100%;height:2px;background-color:#efefef;margin-top:1px;margin-bottom:1px"></div>
            <WindowFolderIcon :icon="'earth'" :text="'Network'" :disabled="true"/>
          </div>
          <div class="tw-h-full tw-select-none" style="width:1px">
            <WindowSider :mode="1" @mousedown.native="right_resize" />
          </div>
        </div>
        <div class="tw-flex-grow  tw-h-full tw-rounded-br-2xl tw-flex tw-flex-col" >
          <div ref="main_body" class="tw-flex-grow tw-w-full tw-flex tw-flex-col tw-min-h-0 tw-overflow-hidden" style="background-color:#f8f8f8" >
            <div class="tw-w-full tw-h-8 tw-border-b border tw-border-gray-300 tw-flex tw-items-center tw-text-mygray-b6 tw-tracking-tight tw-select-none tw-flex-none" style="background-color:#e9e9e9">
              <div ref="name_col" class="tw-h-full tw-flex tw-items-center" style="" :style="{'width':name_col_width + 'px'}" @click="switch_selected('Name')">
                <div class="tw-flex-none tw-ml-2">Name</div>
                <div class="tw-flex-grow tw-flex tw-flex-row-reverse">
                  <v-icon v-if="selected==='Name'">{{mdi_computed_direction}}</v-icon>
                </div>
              </div>
              <WindowFolderSlider @mousedown.native="right_resize_grid_1"/>
              <div ref="date_col" class="tw-h-full tw-flex tw-items-center" style=""  :style="{'width':date_col_width + 'px'}" @click="switch_selected('Date Modified')">
                <div class="tw-flex-none">Date Modified</div>
                <div class="tw-flex-grow tw-flex tw-flex-row-reverse">
                  <v-icon v-if="selected==='Date Modified'">{{mdi_computed_direction}}</v-icon>
                </div>
              </div>
              <WindowFolderSlider @mousedown.native="right_resize_grid_2"/>
              <div ref="size_col" class="tw-h-full tw-flex tw-items-center" style=""  :style="{'width':size_col_width + 'px'}" @click="switch_selected('Size')">
                <div class="tw-flex-none">Size</div>
                <div class="tw-flex-grow tw-flex tw-flex-row-reverse">
                  <v-icon v-if="selected==='Size'">{{mdi_computed_direction}}</v-icon>
                </div>
              </div>
            </div>
            <div class="tw-flex-grow tw-w-full tw-flex tw-min-h-0 tw-overflow-y-auto change-srollbar">
              <div ref="name_col_cont" :style="{'width':name_col_width + 9 + 'px'}" class=" tw-h-full">
                <div class=" tw-w-full tw-text-sm tw-flex tw-items-center" style="height:28px;" :style="{'background-color':current_focus===item.fileuuid?'rgba(0,129,255)':i%2===0?'rgba(255,255,255,0)':'rgba(55,55,55,.04)'}" v-for="(item ,i) in current_dir_list" :key="i" @click="item_oneClick(i)"> 
                  <div :style="{'height':item.children === undefined?'24px':'18px'}" class=" tw-flex tw-justify-center tw-items-center tw-w-7 tw-flex-none tw-ml-7 tw-pb-0.5">
                    <img :src="getFileIcon(item)" alt="" class="tw-h-full" >
                  </div>
                  <div class=" tw-flex-grow tw-pr-1 tw-pl-2 tw-tracking-wide  tw-text-mygray-b6 tw-font-semibold" style="white-space:nowrap;text-overflow: ellipsis;overflow: hidden;" :style="{'color':current_focus===item.fileuuid?'#fefefe !important':''}"> {{item.name}}</div>
                </div>
              </div>
              <div ref="date_col_cont" :style="{'width':date_col_width + 9 + 'px'}" class="tw-h-full">
                <div class=" tw-w-full tw-text-sm tw-flex tw-items-center" style="height:28px;" :style="{'background-color':current_focus===item.fileuuid?'rgba(0,129,255)':i%2===0?'rgba(255,255,255,0)':'rgba(55,55,55,.04)'}" v-for="(item ,i) in current_dir_list" :key="i"> 
                  <div class=" tw-flex-grow tw-pr-1 tw-pl-3 tw-tracking-wide  tw-text-mygray-b7" style="white-space:nowrap;text-overflow: ellipsis;overflow: hidden;" :style="{'color':current_focus===item.fileuuid?'#fefefe !important':''}"> {{time_format_converter(item.lastedittime)}}</div>
                </div>
              </div>
              <div ref="size_col_cont" :style="{'width':size_col_width + 'px'}" class="tw-h-full tw-flex-grow">
                <div class=" tw-w-full tw-text-sm tw-flex tw-items-center" style="height:28px;" :style="{'background-color':current_focus===item.fileuuid?'rgba(0,129,255)':i%2===0?'rgba(255,255,255,0)':'rgba(55,55,55,.04)'}" v-for="(item ,i) in current_dir_list" :key="i"> 
                  <div class=" tw-flex-grow tw-pr-1 tw-pl-3 tw-tracking-wide  tw-text-mygray-b7" style="white-space:nowrap;text-overflow: ellipsis;overflow: hidden;" :style="{'color':current_focus===item.fileuuid?'#fefefe !important':''}"> {{size_format_converter(item.size)}}</div>
                </div>
              </div>
            </div>
          </div>
          <div class=" tw-flex-none tw-h-6 tw-w-full tw-rounded-br-2xl tw-flex tw-justify-center tw-items-center tw-text-sm tw-text-gray-900">
            {{current_dir_list.length}} Items
          </div>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'
import WindowFolderIcon from './WindowFolderIcon.vue'
import WindowFolderSlider from './WindowFolderSlider.vue'
import WindowSider from '../WindowBasic/WindowSider.vue'

export default {
  name: 'WindowFolder',
  components: {
    Window,
    WindowSider,
    WindowFolderIcon,
    WindowFolderSlider,
  },
  data(){
    return {
      result: [],
      delay: 200,
      clicks: 0,
      focus: false,
      cont_margin: 50,
      cont_height: 550,
      cont_width: 800,
      body_margin: 208,
      selected: "Date Modified",
      direction_down: true,
      name_col_width: 160,
      date_col_width: 110,
      size_col_width: 50,
      current_focus:'',
      open_openpath:[],
    }
  },
  props: {
    uuid:String,
    startpos_x:{
      default:60,
    },
    startpos_y:{
      default:60
    },
    zindex:{
      type:Number,
      default:999,
    },
    minimized:{
      type:Boolean,
      default:false,
    },
    filemap:{
      default:[]
    },
    openpath:{
      default:[]
    },
  },
  created(){
    this.open_openpath = Array.isArray(this.openpath) ? [...this.openpath] : []
  },
  mounted(){
    this.$store.commit('refresh_window_focus', {uuid:this.uuid})
  },
  watch:{
    openpath(val){
      this.open_openpath = Array.isArray(val) ? [...val] : []
    },
  },
  computed:{
    mdi_computed_direction() {
      if (this.direction_down) {
        return 'mdi-chevron-down'
      } else {
        return 'mdi-chevron-up'
      }
    },
    atDesktopRoot(){
      return this.open_openpath.length === 0
    },
    breadcrumbDesktopClass(){
      const on = this.atDesktopRoot
      return {
        'tw-bg-mygray-b4': on,
        'tw-bg-mygray-b2': !on,
        'tw-text-gray-50': on,
        'tw-text-mygray-b6': !on,
        'tw-font-semibold': true,
        'hover:tw-bg-mygray-b5': on,
        'hover:tw-bg-mygray-b3': !on,
      }
    },
    current_dir_list() {
      const root = this.filemap
      if (!root || root.length === 0) {
        return []
      }
      const current_folder_pointer = this.open_openpath.length === 0
        ? root
        : this.resolvePathChildren(root, this.open_openpath)
      if (!current_folder_pointer || current_folder_pointer.length === 0) {
        return []
      }
      let result = JSON.parse(JSON.stringify(current_folder_pointer))
      let compare_tag = "name"
      if (this.selected === "Name") {
        compare_tag = 'name'
      } else if (this.selected === "Date Modified") {
        compare_tag = 'lastedittime'
      } else if (this.selected === "Size") {
        compare_tag = 'size'
      }
      let sortfunc = (a,b) => {
        let res = 0;
        if (a.size < 0 && b.size >= 0) {
          res = -1
        } else if (a.size >= 0 && b.size < 0) {
          res = 1
        } else {
          if (a[compare_tag] === b[compare_tag]) {
            res = 0
          } else if (a[compare_tag] > b[compare_tag]) {
            res = 1
          } else {
            res = -1
          }
        }
        if (this.direction_down) {
          return -res
        } else {
          return res
        }
      }
      result.sort(sortfunc)
      return result
    },
    folder_name(){
      if (this.open_openpath.length === 0) {
        return 'Desktop'
      }
      return this.open_openpath[this.open_openpath.length - 1]
    },
    global_focus(){
      return this.$store.state.current_focus
    },
  },
  methods:{
    breadcrumbSegmentClass(i){
      const last = i === this.open_openpath.length - 1
      return {
        'tw-bg-mygray-b4': last,
        'tw-bg-mygray-b2': !last,
        'tw-text-gray-50': last,
        'tw-text-mygray-b6': !last,
        'tw-font-semibold': true,
        'hover:tw-bg-mygray-b5': last,
        'hover:tw-bg-mygray-b3': !last,
      }
    },
    resolvePathChildren(root, segments){
      if (!root || !segments || segments.length === 0) {
        return root
      }
      let cur = root
      for (const name of segments) {
        let next = null
        for (const item of cur) {
          if (item.name === name && item.children !== undefined) {
            next = item.children
            break
          }
        }
        if (next == null) {
          return []
        }
        cur = next
      }
      return cur
    },
    go_breadcrumb_desktop(){
      this.open_openpath = []
      this.current_focus = ''
    },
    go_breadcrumb_at(i){
      if (i < 0 || i >= this.open_openpath.length) {
        return
      }
      this.open_openpath = this.open_openpath.slice(0, i + 1)
      this.current_focus = ''
    },
    nav_up(){
      if (this.open_openpath.length > 0) {
        this.open_openpath.pop()
        this.current_focus = ''
      }
    },
    goSidebarPlace(place){
      const toRoot = ['home', 'desktop', 'thispc', 'root']
      if (toRoot.indexOf(place) >= 0) {
        this.go_breadcrumb_desktop()
      }
    },
    getFileIcon(item) {
      // 如果是文件夹,返回文件夹图标
      if (item.children !== undefined) {
        return require('../../assets/images/icons/folder.png');
      }
      
      // 根据文件扩展名返回相应的图标
      const fileName = item.name.toLowerCase();
      const ext = fileName.substring(fileName.lastIndexOf('.'));
      
      // 代码文件类型
      const codeExtensions = ['.cc', '.cpp', '.cxx', '.c', '.h', '.hpp', '.hxx', 
                             '.py', '.js', '.ts', '.java', '.go', '.rs', 
                             '.sh', '.bat', '.ps1'];
      
      // Markdown文件
      if (ext === '.md') {
        return require('../../assets/images/icons/text2.png');
      }
      
      // Draw.io 文件
      if (ext === '.drawio' || ext === '.dio') {
        return require('../../assets/images/icons/drawio.png');
      }
      
      // 代码文件 - 可以使用vscode图标或text2图标
      if (codeExtensions.includes(ext)) {
        return require('../../assets/images/icons/vscode.png');
      }
      
      // 配置文件
      if (['.json', '.xml', '.yaml', '.yml'].includes(ext)) {
        return require('../../assets/images/icons/text2.png');
      }
      
      // 文本文件
      if (ext === '.txt') {
        return require('../../assets/images/icons/text.png');
      }
      
      // 默认文件图标
      return require('../../assets/images/icons/text2.png');
    },
    PrefixZero(num, n) {
      return (Array(n).join(0) + num).slice(-n);
    },
    getOffset(offset,min,max){
      if (offset < min) {
        return min;
      } else if(offset > max){
        return max;
      } 
      return offset;
    },
    window_height_changed(val){
      this.cont_height = val - this.cont_margin
    },
    window_width_changed(val){
      this.cont_width = val - this.body_margin - 2
      let sum = this.name_col_width + this.date_col_width + this.size_col_width
      let available_width = this.cont_width - 18
      this.name_col_width = this.name_col_width * available_width / sum
      this.date_col_width = this.date_col_width * available_width / sum
      this.size_col_width = this.size_col_width * available_width / sum
    },
    right_resize(){
      let orn_mousedown = document.onmousedown;
      let e = e || window.event || e.which;
      let downX = e.clientX;
      let orn_width = this.$refs.leftbar.style.width;
      orn_width = parseFloat(orn_width.substr(0,orn_width.length-2))
      let name_orn_width = this.$refs.name_col.style.width;
      name_orn_width = parseFloat(name_orn_width.substr(0,name_orn_width.length-2))
      let date_orn_width = this.$refs.date_col.style.width;
      date_orn_width = parseFloat(date_orn_width.substr(0,date_orn_width.length-2))
      let size_orn_width = this.$refs.size_col.style.width;
      size_orn_width = parseFloat(size_orn_width.substr(0,size_orn_width.length-2))
      let sum = name_orn_width + date_orn_width + size_orn_width;
      let max = Math.max(Math.min(sum - date_orn_width - size_orn_width - 110,260),160)
      let min = 160
      let mouseMoveHandler = () => {
        let e = e || window.event || e.which;
        let moveX = e.clientX;
        let offsetX = this.getOffset(moveX-downX+orn_width,min,max);
        this.$refs.leftbar.style.width = offsetX + 'px';
        this.body_margin = offsetX
        let name_col_width = name_orn_width + orn_width - offsetX ;
        this.$refs.name_col.style.width = name_col_width + 'px';
        this.$refs.name_col_cont.style.width = (name_col_width + 9) + 'px';
      }
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
      }
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = ()=>{};
    },
    switch_selected(val) {
      this.selected = val
      this.direction_down = ! this.direction_down
    },
    right_resize_grid_1(){
      let orn_mousedown = document.onmousedown;
      let e = e || window.event || e.which;
      let downX = e.clientX;downX
      let name_orn_width = this.$refs.name_col.style.width;
      name_orn_width = parseFloat(name_orn_width.substr(0,name_orn_width.length-2))
      let date_orn_width = this.$refs.date_col.style.width;
      date_orn_width = parseFloat(date_orn_width.substr(0,date_orn_width.length-2))
      let size_orn_width = this.$refs.size_col.style.width;
      size_orn_width = parseFloat(size_orn_width.substr(0,size_orn_width.length-2))
      let sum = name_orn_width + date_orn_width + size_orn_width;
      let min_size = 110;
      let max_size = sum - 190;
      let mouseMoveHandler = () => {
        let e = e || window.event || e.which;
        let moveX = e.clientX;
        let offsetX = this.getOffset(moveX-downX+name_orn_width,min_size,max_size);
        let size_col_width = Math.max(sum - date_orn_width - offsetX, 70)
        let date_col_width = sum - size_col_width - offsetX
        this.$refs.name_col.style.width = offsetX + 'px';
        this.$refs.size_col.style.width = size_col_width + 'px';
        this.$refs.date_col.style.width = date_col_width + 'px';
        this.$refs.name_col_cont.style.width = (offsetX + 9) + 'px';
        this.$refs.size_col_cont.style.width = size_col_width + 'px';
        this.$refs.date_col_cont.style.width = (date_col_width  + 9) + 'px';
      }
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
      }
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = ()=>{};
    },
    right_resize_grid_2(){
      let orn_mousedown = document.onmousedown;
      let e = e || window.event || e.which;
      let downX = e.clientX;downX
      let name_orn_width = this.$refs.name_col.style.width;
      name_orn_width = parseFloat(name_orn_width.substr(0,name_orn_width.length-2))
      let date_orn_width = this.$refs.date_col.style.width;
      date_orn_width = parseFloat(date_orn_width.substr(0,date_orn_width.length-2))
      let size_orn_width = this.$refs.size_col.style.width;
      size_orn_width = parseFloat(size_orn_width.substr(0,size_orn_width.length-2))
      let sum = name_orn_width + date_orn_width + size_orn_width;
      let min_size = 120;
      let max_size = sum - name_orn_width - 70;
      let mouseMoveHandler = () => {
        let e = e || window.event || e.which;
        let moveX = e.clientX;
        let offsetX = this.getOffset(moveX-downX+date_orn_width,min_size,max_size);
        let size_col_width = sum - offsetX - name_orn_width;
        this.$refs.date_col.style.width = offsetX + 'px';
        this.$refs.size_col.style.width = size_col_width + 'px';
        this.$refs.date_col_cont.style.width = (offsetX + 9) + 'px';
        this.$refs.size_col_cont.style.width = size_col_width + 'px';
      }
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
      }
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = ()=>{};
    },
    time_format_converter(val) {
      let newdate = new Date(val * 1000)
      return newdate.getFullYear() + '/' + this.PrefixZero(newdate.getMonth() + 1,2) + '/' + this.PrefixZero(newdate.getDate(),2) + ' ' + this.PrefixZero(newdate.getHours(),2) + ':' + this.PrefixZero(newdate.getMinutes(),2) + ':' + this.PrefixZero(newdate.getSeconds(),2)
    },
    size_format_converter(val) {
      if (val < 0) {
        return '-'
      } else if (val >= (1<<20)) {
        return (val / (1<<20)).toFixed(1) + 'MB'
      } else if (val >= (1<<10)) {
        return (val / (1<<10)).toFixed(1) + 'KB'
      } else {
        return val + "B"
      }
    },
    isCodeFile(filename) {
      const codeExtensions = ['.cc', '.cpp', '.cxx', '.c', '.h', '.hpp', '.hxx', 
                             '.py', '.js', '.ts', '.java', '.go', '.rs', 
                             '.sh', '.bat', '.ps1', '.json', '.xml', '.yaml', '.yml'];
      const ext = filename.substring(filename.lastIndexOf('.')).toLowerCase();
      return codeExtensions.includes(ext);
    },
    item_oneClick(val){
      this.current_focus = this.current_dir_list[val].fileuuid
      this.clicks++ 
      if(this.clicks === 1) {
        let self = this
        this.timer = setTimeout(function() {
          self.clicks = 0
        }, this.delay);
      } else{
        clearTimeout(this.timer);  
        let item = this.current_dir_list[val]
        if (item.children === undefined) {
          document.body.style.cursor='progress'
          window.setTimeout(()=>{
            document.body.style.cursor='default'
          },500)
          
          // 判断文件类型
          const fileName = item.name.toLowerCase();
          const ext = fileName.substring(fileName.lastIndexOf('.'));
          
          if (ext === '.drawio' || ext === '.dio') {
            // Draw.io 文件
            this.$store.commit('open_new_window', {
              type: 'drawio',
              filesrc: item.path,
              filename: item.name,
              size: item.size,
            })
          } else if (this.isCodeFile(item.name)) {
            // 代码文件
            let filePath = 'blog/' + this.open_openpath.join('/');
            if (filePath && !filePath.endsWith('/')) {
              filePath += '/';
            }
            filePath += item.name;
            
            this.$store.commit('open_new_window', {
              type: 'code',
              filepath: filePath,
              filename: item.name,
              size: item.size,
            })
          } else {
            // Markdown 和其他文本文件
            this.$store.commit('open_new_window', {
              type: 'text',
              filesrc: item.path,
              filename: item.name,
              size: item.size,
            })
          }
        } else {
          this.open_openpath = this.open_openpath.concat([item.name])
        }
        this.clicks = 0;
      } 
    },
    mr_clicked(){
      this.$store.commit('show_context_menu')
    },
  }
}
</script>

<style scoped>
.change-srollbar{
  left:0;
  margin-right:3px;
}
.change-srollbar::-webkit-scrollbar {/*滚动条整体样式*/
  width: 6px;     /*高宽分别对应横竖滚动条的尺寸*/
  height: 6px;
  scrollbar-arrow-color:red;

}
.change-srollbar::-webkit-scrollbar-thumb {/*滚动条里面小方块*/
  border-radius: 7px;
  -webkit-box-shadow: inset 0 0 7px rgba(0,0,0,0.3);
  background: rgba(0,0,0,0.3);
  scrollbar-arrow-color:red;
}
.change-srollbar::-webkit-scrollbar-track {/*滚动条里面轨道*/
  /* -webkit-box-shadow: inset 0 0 7px rgba(0,0,0,0.2); */
  border-radius: 0;
  /* background: rgba(0,0,0,0.1); */
}
</style>
