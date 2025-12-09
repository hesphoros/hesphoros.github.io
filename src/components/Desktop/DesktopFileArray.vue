<template>
  <div class="tw-w-full tw-h-full tw-flex tw-flex-col tw-flex-wrap tw-content-start tw-justify-start tw-items-start tw-select-none">
    <DesktopFileIcon v-for="(item, i) in folders" :key="i" :item_name="item.name" :item_type="item.mode" :item_size="item.size" @dbclicked="dbclicked" style="pointer-events: auto"/>
  </div>
</template>

<script>
import DesktopFileIcon from './DesktopFileIcon.vue'
export default {
  name: 'DesptopFileArray',
  components: {
    DesktopFileIcon
  },
  data(){
    return {

    }
  },
  props: {
    filemap:{
      default:[]
    }
  },
  created(){
  },
  mounted(){
  },
  watch:{
  },
  computed:{
    folders(){
      let result = []
      for (let item of this.filemap) {
        if (item.children === undefined) {
          result.push({name:item.name,mode:1, size: item.size})
        } else {
          result.push({name:item.name,mode:0, size: item.size})
        }
      } 
      let sortfunc = (a,b) => {
        if (a.mode>b.mode) {
          return 1
        } else if (a.mode<b.mode) {
          return -1
        } else {
          if (a.name === b.name) {
            return 0
          } else if (a.name > b.name) {
            return 1
          } else {
            return -1
          }
        }
      }
      result.sort(sortfunc)
      return result
    },
  },
  methods:{
    dbclicked(item_name, item_type){
      document.body.style.cursor='progress'
      window.setTimeout(()=>{
        document.body.style.cursor='default'
        if (item_type === 0) {
          this.$store.commit('open_new_window', {'type':'explorer','openpath':[item_name,]})
        } else if (item_type === 1) {
          for (let loop_item of this.filemap) {
            if (loop_item.name === item_name) {
              // 判断文件类型
              const fileName = loop_item.name.toLowerCase();
              const ext = fileName.substring(fileName.lastIndexOf('.'));
              
              if (ext === '.drawio' || ext === '.dio') {
                // Draw.io 文件
                this.$store.commit('open_new_window', {
                  type: 'drawio',
                  filesrc: loop_item.path,
                  filename: loop_item.name,
                  size: loop_item.size,
                })
              } else {
                // 其他文本文件
                this.$store.commit('open_new_window', {
                  type:'text',
                  filesrc: loop_item.path,
                  filename: loop_item.name,
                  size: loop_item.size,
                })
              }
            }
          }
        }
      },500)
    }
  }
}
</script>

<style scoped>
</style>