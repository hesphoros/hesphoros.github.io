<template>
  <Window 
    :startpos_x="startpos_x" 
    :startpos_y="startpos_y" 
    :uuid="uuid" 
    :zindex="zindex" 
    :minimized="minimized" 
    :default_width="1000" 
    :default_height="600"
    :blacktheme="true"
    @resize_start="show_blocker=true" 
    @resize_end="show_blocker=false"
  >
    <template v-slot:header>
      <div class="tw-flex tw-items-center tw-select-none" style="pointer-events:none;"> 
        <img src="../../assets/images/icons/vscode.png" alt="" style="pointer-events:none;" class="tw-w-8 tw-h-7 tw-ml-4">
        <div class="tw-ml-4 tw-font-semibold tw-tracking-wider tw-h-8 tw-rounded-lg tw-px-4 tw-py-1 tw-text-white tw-bg-mygray-b13 hover:tw-bg-mygray-b14" style="pointer-events:auto;max-width:400px;white-space:nowrap;text-overflow: ellipsis;overflow: hidden;">
          {{ filename }} - Visual Studio Code
        </div>
      </div>
    </template>
    <template v-slot:content>
      <div class="tw-w-full tw-h-full tw-flex tw-overflow-hidden" @click="go_focus">
        <iframe :src="vscodeUrl" frameborder="0" class="zoomined-frame"></iframe>
        <div class="tw-w-full tw-h-full tw-absolute background-color" style="top:0" v-if="show_blocker">
          <span class="tw-hidden"> this div is to prevent iframe take control over mouse event </span>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'

export default {
  name: 'WindowCode',
  components: {
    Window,
  },
  data() {
    return {
      show_blocker: false,
    }
  },
  props: {
    uuid: String,
    startpos_x: { default: 60 },
    startpos_y: { default: 60 },
    zindex: { type: Number, default: 999 },
    minimized: { type: Boolean, default: false },
    filepath: { type: String, default: "" },
    filename: { type: String, default: "" },
    size: { type: Number, default: 0 },
  },
  computed: {
    vscodeUrl() {
      const baseUrl = 'https://github1s.com/hesphoros/hesphoros.github.io/blob/main/';
      let cleanPath = this.filepath.replace(/^\.?\//, '');
      return baseUrl + cleanPath;
    }
  },
  methods: {
    go_focus() {
      this.$store.commit('refresh_window_focus', { uuid: this.uuid })
    }
  },
  mounted() {
    this.$store.commit('refresh_window_focus', { uuid: this.uuid })
  }
}
</script>

<style scoped>
.zoomined-frame {
  width: 100%;
  height: 100%;
}
</style>
