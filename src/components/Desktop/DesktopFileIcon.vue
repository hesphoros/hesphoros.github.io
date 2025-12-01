<template>
  <div ref="iconRef" class="tw-pt-4 tw-pb-1 tw-px-6 tw-flex tw-flex-col tw-justify-center tw-items-center tw-relative" style="height:120px;width:120px" @click="oneClick" @mouseenter="onMouseEnter" @mouseleave="onMouseLeave" :style="{zIndex: is_hover ? 50 : 'auto'}">

    <img :src="require('../../assets/images/icons/folder.png')" class=" tw-w-12" v-if="item_type==0"/>
    <img :src="require('../../assets/images/icons/text.png')" class=" tw-w-10" v-if="item_type==1"/>
    <div class="tw-flex  tw-justify-center tw-items-start tw-overflow-hidden" style="height:46px;width:72px">
      <div class="tw-mt-1.5 tw-leading-4  tw-pt-1 tw-pb-0.5  tw-px-1 tw-rounded-md tw-text-sm" style="text-align:center;text-shadow:0px 1px 4px rgba(16,16,16,.9);color:#f2f2f2;display: -webkit-box;-webkit-box-orient: vertical;-webkit-line-clamp: 2;overflow: hidden;" :class="{'tw-bg-blue-500':global_focus === uuid}">{{item_name}}</div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'DesptopFileIcon',
  components: {
  },
  data(){
    return {
      result: [],
      delay: 200,
      clicks: 0,
      uuid: "",
      show_tooltip: false,
      tooltip_timer: null,
      is_hover: false,
      tooltipEl: null
    }
  },
  props: {
    item_name: {
      type:String,
      default:"",
    },
    item_type: {
      type: [Number, String],
      default:0,
    },
    item_size: {
      type: Number,
      default: 0
    }
  },
  created(){
    this.uuid = this.$utils.get_uuid()
  },
  mounted(){
  },
  beforeDestroy(){
    this.removeTooltip();
  },
  watch:{
  },
  computed:{
    global_focus(){
      return this.$store.state.current_focus
    },
    formattedSize() {
      let size = this.item_size || 0;
      if (size < 1024) {
        return size + ' B';
      } else if (size < 1024 * 1024) {
        return (size / 1024).toFixed(2) + ' KB';
      } else {
        return (size / (1024 * 1024)).toFixed(2) + ' MB';
      }
    }
  },
  methods:{
    onMouseEnter() {
      this.is_hover = true;
      this.tooltip_timer = setTimeout(() => {
        this.showTooltip();
      }, 800);
    },
    onMouseLeave() {
      this.is_hover = false;
      clearTimeout(this.tooltip_timer);
      this.removeTooltip();
    },
    showTooltip() {
      if (this.$refs.iconRef) {
        const rect = this.$refs.iconRef.getBoundingClientRect();
        
        // 创建 tooltip 元素
        this.tooltipEl = document.createElement('div');
        this.tooltipEl.className = 'desktop-tooltip-box';
        this.tooltipEl.style.cssText = `
          position: fixed;
          top: ${rect.bottom + 5}px;
          left: ${rect.left + rect.width / 2}px;
          transform: translateX(-50%);
          background-color: #ffffff;
          border: 1px solid #767676;
          padding: 4px 6px;
          z-index: 99999;
          box-shadow: 2px 2px 4px rgba(0,0,0,0.2);
          font-size: 12px;
          color: #333;
          white-space: nowrap;
          pointer-events: none;
          text-align: left;
        `;
        
        // 构建内容
        let content = `<div style="line-height: 1.5;">名称: ${this.item_name}</div>`;
        content += `<div style="line-height: 1.5;">类型: ${this.item_type === 0 ? '文件夹' : '文本文件'}</div>`;
        if (this.item_type === 1) {
          content += `<div style="line-height: 1.5;">大小: ${this.formattedSize}</div>`;
        }
        this.tooltipEl.innerHTML = content;
        
        // 添加到 body
        document.body.appendChild(this.tooltipEl);
        this.show_tooltip = true;
      }
    },
    removeTooltip() {
      if (this.tooltipEl && this.tooltipEl.parentNode) {
        this.tooltipEl.parentNode.removeChild(this.tooltipEl);
        this.tooltipEl = null;
      }
      this.show_tooltip = false;
    },
    dbclicked(){
      this.$emit("dbclicked", this.folder)
    },
    oneClick(){
      this.clicks++ 
      this.$store.commit('refresh_window_focus', {uuid:this.uuid})
      this.$store.commit('close_side_bar')
      if(this.clicks === 1) {
        let self = this
        this.timer = setTimeout(function() {
          self.clicks = 0
        }, this.delay);
      } else{
        clearTimeout(this.timer);  
        this.$emit("dbclicked", this.item_name, this.item_type)
        this.clicks = 0;
      }
    }
  }
}
</script>

<style scoped>
</style>