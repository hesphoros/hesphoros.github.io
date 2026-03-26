<template>
  <div
    class="tw-w-full tw-rounded-lg tw-flex tw-justify-start tw-items-center tw-py-1.5"
    :class="rowClass"
    @click="onClick"
  >
    <v-icon small style="color:rgba(66,77,104)" class="tw-flex tw-items-center tw-mb-0.5 tw-ml-1.5">mdi-{{icon}}</v-icon>
    <div style="color:rgba(66,77,104)" class="tw-flex tw-items-center tw-ml-2 tw-w-full tw-h-full">{{text}}</div>
  </div>
</template>

<script>
export default {
  name: 'WindowFolderIcon',
  components: {
  },
  data(){
    return {
    }
  },
  props:{
    text:{
      type:String,
      default:"Reecently",
    },
    icon:{
      type:String,
      default:"clock-time-three",
    },
    active:{
      type:Boolean,
      default:false,
    },
    disabled:{
      type:Boolean,
      default:false,
    },
  },
  created(){
  },
  mounted(){
  },
  watch:{
  },
  computed:{
    rowClass(){
      if (this.disabled) {
        return 'tw-opacity-50 tw-cursor-not-allowed'
      }
      if (this.active) {
        return 'tw-bg-blue-100 tw-cursor-pointer'
      }
      return 'hover:tw-bg-gray-200 tw-cursor-pointer'
    },
  },
  methods:{
    onClick(){
      if (this.disabled) return
      this.$emit('activate')
    },
  }
}
</script>

<style scoped>
</style>