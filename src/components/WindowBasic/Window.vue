<template>
  <div ref="window_mainbody" class="tw-absolute animate__animated animate__faster"
    :class="{ 'animate__zoomOut': closed, 'animate__zoomOutDown': minimized, 'animate__zoomIn': !minimized }"
    :style="{ 
      'min-width': minWidth + 'px', 
      'min-height': minHeight + 'px', 
      'height': actualHeight + 'px', 
      'width': actualWidth + 'px',
      'pointer-events': 'auto',
      'top': default_top + 'px', 
      'left': default_left + 'px', 
      'z-index': zindex 
    }" 
    @click="window_clicked"
    @contextmenu.prevent="mr_clicked($event)">
    <div
      class="tw-flex tw-justify-center tw-items-center tw-flex-nowrap tw-w-full tw-h-full tw-border tw-border-gray-500 "
      :style="{ 'box-shadow': global_focus === uuid ? '0 0px 30px rgba(16,16,16,.6)' : '0 0px 15px rgba(16,16,16,.3)', 'background-color': blacktheme ? '#111111' : '#f8f8f8' }"
      :class="{ 'tw-rounded-2xl': !full_windowed }">
      <WindowSider :mode="1" v-if="false" />
      <div class="tw-flex-grow tw-flex tw-flex-col tw-justify-center tw-items-center tw-flex-nowrap tw-h-full">
        <WindowSider :mode="0" v-if="false" />
        <div class="tw-w-full tw-h-12  tw-select-none"
          :style="{ 'background-color': blacktheme ? '#282828' : '#fcfcfc' }"
          :class="{ 'tw-rounded-t-xl': !full_windowed }">
          <div class="tw-w-full tw-h-24 tw-absolute tw-overflow-hidden "
            style="left:-0px;pointer-events:none;opacity:.5" :class="{ 'tw-rounded-xl': !full_windowed }">
            <div class="tw-w-full tw-h-12 tw-absolute" style="pointer-events:none;"
              :style="{ 'box-shadow': blacktheme ? '0 0px 24px rgba(33,33,33,.8)' : '0 0px 24px rgba(212,212,212,.8)' }">
            </div>
          </div>
          <WindowHeaderCursor @mousedown.native="move_clicked" />
          <div class="tw-w-full tw-h-12 cursor-move tw-absolute" v-if="show_cursor_move"
            :class="{ 'tw-rounded-t-xl': !full_windowed }"></div>
          <div class="tw-w-full tw-h-12 tw-absolute tw-overflow-hidden" style="pointer-events:none;"
            :class="{ 'tw-rounded-t-xl': !full_windowed }">
            <div class="tw-flex tw-flex-row-reverse tw-items-center tw-overflow-hidden tw-w-full tw-h-full red-200"
              :class="{ 'tw-rounded-t-xl': !full_windowed }">
              <div class="tw-flex-none tw-h-full tw-w-40  tw-flex-row-reverse tw-flex tw-overflow-hidden"
                style="padding-right:2px;pointer-events:auto;" :class="{ 'tw-rounded-t-3xl': !full_windowed }"
                @click="refocus">
                <WindowHeaderButton :bname="blacktheme ? 'd2' : 'b2'" :blacktheme="blacktheme"
                  @button_clicked="close_clicked" />
                <WindowHeaderButton :bname="blacktheme ? 'd1' : 'b1'" :blacktheme="blacktheme"
                  @button_clicked="fullwindow_clicked" />
                <WindowHeaderButton :bname="blacktheme ? 'd3' : 'b3'" :blacktheme="blacktheme"
                  @button_clicked="minimizewindow_clicked" />
              </div>
              <div ref="user_header_content" class="tw-flex-grow">
                <slot name="header"></slot>
              </div>
            </div>
          </div>
          <div ref="header_content" class="tw-w-full tw-h-full">
          </div>
        </div>
        <div class="tw-flex-grow tw-w-full tw-flex tw-justify-center tw-items-center">
          <div ref="user_content" class="tw-flex-none tw-w-full tw-h-full">
            <slot name="content"></slot>
          </div>
        </div>
        <div class="tw-w-full tw-flex-none tw-select-none" style="height:0">
          <div class="tw-absolute tw-w-full tw-flex tw-justify-center tw-items-center tw-flex-nowrap"
            style="height:3px;bottom:-3px">
            <WindowSider :mode="0" @mousedown.native="bottom_resize" v-if="(!full_windowed) && (!fixedsize)" />
          </div>
        </div>
      </div>
      <div class="tw-h-full tw-flex-none tw-select-none" style="width:0">
        <div class="tw-absolute tw-h-full tw-flex tw-flex-col tw-justify-center tw-items-center tw-flex-nowrap"
          style="right:-3px;width:3px">
          <div class="tw-w-0.5 tw-h-0.5"></div>
          <WindowSider :mode="1" @mousedown.native="right_resize" v-if="(!full_windowed) && (!fixedsize)" />
          <WindowSider :mode="2" @mousedown.native="bottom_right_resize" v-if="(!full_windowed) && (!fixedsize)" />
        </div>
      </div>
    </div>
  </div>

</template>

<script>
import WindowHeaderCursor from './WindowHeaderCursor.vue';
import WindowHeaderButton from './WindowHeaderButton.vue';
import WindowSider from './WindowSider.vue';
export default {
  name: 'Window',
  components: {
    WindowSider,
    WindowHeaderCursor,
    WindowHeaderButton
  },
  data() {
    return {
      show_cursor_move: false,
      closed: false,
      // minimized:false,
      full_windowed: false,
      backup_width: "800px",
      backup_height: "600px",
      backup_top: "0px",
      backup_left: "0px",
      default_top: 60,
      default_left: 60,
      TOPBAR_HEIGHT: 28, // TopBar 高度
    }
  },
  props: {
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
    default_width: {
      type: Number,
      default: -1
    },
    default_height: {
      type: Number,
      default: -1
    },
    fixedsize: {
      type: Boolean,
      default: false,
    },
    blacktheme: {
      type: Boolean,
      default: false,
    },
    min_width: {
      type: Number,
      default: 680
    },
    min_height: {
      type: Number,
      default: 600
    }
  },
  created() {
  },
  mounted() {
    this.$emit("width_changed", this.actualWidth)
    this.$emit("height_changed", this.actualHeight)
    this.default_top = this.startpos_x
    this.default_left = this.startpos_y
  },
  watch: {
  },
  computed: {
    fullHeight() {
      return this.$store.state.fullHeight
    },
    fullWidth() {
      return this.$store.state.fullWidth
    },
    global_focus() {
      return this.$store.state.current_focus
    },
    minWidth() {
      // 如果传入了 default_width 并且小于默认最小值，使用 default_width 作为最小值
      if (this.default_width > 0 && this.default_width < this.min_width) {
        return this.default_width
      }
      return this.min_width
    },
    minHeight() {
      // 如果传入了 default_height 并且小于默认最小值，使用 default_height 作为最小值
      if (this.default_height > 0 && this.default_height < this.min_height) {
        return this.default_height
      }
      return this.min_height
    },
    actualWidth() {
      if (this.default_width > 0) {
        return this.default_width
      }
      return Math.max(Math.min(1000, 0.6 * this.fullWidth), this.minWidth)
    },
    actualHeight() {
      if (this.default_height > 0) {
        return this.default_height
      }
      return this.minHeight
    },
  },
  methods: {
    getOffset(offset, min, max) {
      if (offset < min) {
        return min;
      } else if (offset > max) {
        return max;
      }
      return offset;
    },
    bottom_resize(event) {
      if (this.full_windowed || this.fixedsize) {
        return;
      }
      this.$emit("resize_start");
      let orn_mousedown = document.onmousedown;
      let downY = event.clientY; // 使用事件对象
      let min = this.$refs.window_mainbody.style.top;
      min = parseFloat(min.substr(0, min.length - 2));
      min = min + this.minHeight - downY;
      let max = this.fullHeight - downY;
      let orn_height = this.$refs.window_mainbody.style.height;
      orn_height = parseFloat(orn_height.substr(0, orn_height.length - 2));
      let mouseMoveHandler = (moveEvent) => {
        let moveY = moveEvent.clientY; // 使用事件对象
        let offsetY = this.getOffset(moveY - downY, min, max);
        this.$refs.window_mainbody.style.height = (offsetY + orn_height) + 'px';
        this.$emit("height_changed", offsetY + orn_height);
      };
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
        this.$emit("resize_end");
      };
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = () => { };
    },

    right_resize(event) {
      if (this.full_windowed || this.fixedsize) {
        return;
      }
      this.$emit("resize_start");
      let orn_mousedown = document.onmousedown;
      let downX = event.clientX; // 使用事件对象
      let min = this.$refs.window_mainbody.style.left;
      min = parseFloat(min.substr(0, min.length - 2));
      min = min + this.minWidth - downX;
      let max = this.fullWidth - downX;
      let orn_width = this.$refs.window_mainbody.style.width;
      orn_width = parseFloat(orn_width.substr(0, orn_width.length - 2));
      let mouseMoveHandler = (moveEvent) => {
        let moveX = moveEvent.clientX; // 使用事件对象
        let offsetX = this.getOffset(moveX - downX, min, max);
        this.$refs.window_mainbody.style.width = (offsetX + orn_width) + 'px';
        this.$emit("width_changed", offsetX + orn_width);
      };
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
        this.$emit("resize_end");
      };
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = () => { };
    },

    bottom_right_resize(event) {
      if (this.full_windowed || this.fixedsize) {
        return;
      }
      this.$emit("resize_start");
      let orn_mousedown = document.onmousedown;
      let downY = event.clientY; // 使用事件对象
      let downX = event.clientX; // 使用事件对象
      let minX = this.$refs.window_mainbody.style.left;
      minX = parseFloat(minX.substr(0, minX.length - 2));
      minX = minX + this.minWidth - downX;
      let minY = this.$refs.window_mainbody.style.top;
      minY = parseFloat(minY.substr(0, minY.length - 2));
      minY = minY + this.minHeight - downY;
      let maxX = this.fullWidth - downX;
      let maxY = this.fullHeight - downY;
      let orn_width = this.$refs.window_mainbody.style.width;
      orn_width = parseFloat(orn_width.substr(0, orn_width.length - 2));
      let orn_height = this.$refs.window_mainbody.style.height;
      orn_height = parseFloat(orn_height.substr(0, orn_height.length - 2));
      let mouseMoveHandler = (moveEvent) => {
        let moveX = moveEvent.clientX; // 使用事件对象
        let moveY = moveEvent.clientY; // 使用事件对象
        let offsetX = this.getOffset(moveX - downX, minX, maxX);
        let offsetY = this.getOffset(moveY - downY, minY, maxY);
        this.$refs.window_mainbody.style.width = (offsetX + orn_width) + 'px';
        this.$refs.window_mainbody.style.height = (offsetY + orn_height) + 'px';
        this.$emit("height_changed", offsetY + orn_height);
        this.$emit("width_changed", offsetX + orn_width);
      };
      let mouseUpHandler = () => {
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;
        this.$emit("resize_end");
      };
      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = () => {};
    },

    move_clicked(e) {
      this.$store.commit('close_side_bar');
      this.$emit("resize_start");
      this.refocus();

      if (this.full_windowed) {
        this.$emit("resize_end");
        return;
      }

      const orn_mousedown = document.onmousedown;
      this.show_cursor_move = true;

      const downY = e.clientY;
      const downX = e.clientX;
      const minX = -e.clientX;
      const minY = this.TOPBAR_HEIGHT - e.clientY; // 限制不能拖到 TopBar 区域
      const maxX = this.fullWidth;
      const maxY = this.fullHeight;

      let ornX = this.$refs.window_mainbody.style.left;
      ornX = parseFloat(ornX.slice(0, -2));
      let ornY = this.$refs.window_mainbody.style.top;
      ornY = parseFloat(ornY.slice(0, -2));

      const mouseMoveHandler = (moveEvent) => {
        const moveX = moveEvent.clientX;
        const moveY = moveEvent.clientY;
        const offsetX = this.getOffset(moveX - downX, minX, maxX);
        const offsetY = this.getOffset(moveY - downY, minY, maxY);
        this.$refs.window_mainbody.style.left = offsetX + ornX + 'px';
        this.$refs.window_mainbody.style.top = offsetY + ornY + 'px';
      };

      const mouseUpHandler = () => {
        this.show_cursor_move = false;
        document.onmousemove = null;
        document.onmouseup = null;
        document.onmousedown = orn_mousedown;

        // 保存最终位置到响应式数据
        const finalLeft = parseFloat(this.$refs.window_mainbody.style.left);
        let finalTop = parseFloat(this.$refs.window_mainbody.style.top);
        
        // 确保窗口不会停留在 TopBar 区域
        if (finalTop < this.TOPBAR_HEIGHT) {
          finalTop = this.TOPBAR_HEIGHT;
          this.$refs.window_mainbody.style.top = finalTop + 'px';
        }
        
        this.default_left = finalLeft;
        this.default_top = finalTop;

        this.$emit("resize_end");
      };

      document.onmousemove = mouseMoveHandler;
      document.onmouseup = mouseUpHandler;
      document.onmousedown = () => { };
    },

    close_clicked() {
      this.closed = true
      window.setTimeout(() => {
        // let parentNode = this.$refs.window_mainbody.parentNode
        // parentNode.removeChild(this.$refs.window_mainbody)
        this.$store.commit('close_window_with_uuid', { uuid: this.uuid, })
      }, 400)
    },
    fullwindow_clicked() {
      if (!this.full_windowed) {
        this.full_windowed = true
        this.backup_width = this.$refs.window_mainbody.style.width
        this.backup_height = this.$refs.window_mainbody.style.height
        this.backup_left = this.$refs.window_mainbody.style.left
        this.backup_top = this.$refs.window_mainbody.style.top
        this.$refs.window_mainbody.style.width = this.fullWidth + 'px';
        // 全屏时从 TopBar 下方开始，减去 TopBar 高度和 Dock 高度
        this.$refs.window_mainbody.style.height = (this.fullHeight - this.TOPBAR_HEIGHT - 75) + 'px'
        this.$refs.window_mainbody.style.top = this.TOPBAR_HEIGHT + 'px'
        this.$refs.window_mainbody.style.left = 0 + 'px'
        this.$emit("height_changed", this.fullHeight - this.TOPBAR_HEIGHT - 75)
        this.$emit("width_changed", this.fullWidth)
      } else {
        this.full_windowed = false
        this.$refs.window_mainbody.style.width = this.backup_width;
        this.$refs.window_mainbody.style.height = this.backup_height;
        this.$refs.window_mainbody.style.top = this.backup_top;
        this.$refs.window_mainbody.style.left = this.backup_left;
        let targ_height = parseFloat(this.backup_height.substr(0, this.backup_height.length - 2))
        let targ_width = parseFloat(this.backup_width.substr(0, this.backup_width.length - 2))
        this.$emit("height_changed", targ_height)
        this.$emit("width_changed", targ_width)
      }
    },
    close_side_bar() {
      this.$store.commit('close_side_bar')
    },
    content_clicked() {

    },
    minimizewindow_clicked() {
      this.$store.commit('minimize_window_with_uuid', { uuid: this.uuid, })
      window.setTimeout(() => {
        this.$store.commit('refresh_window_focus', { uuid: "" })
      }, 400)
    },
    window_clicked() {
      this.$store.commit('close_side_bar')
      this.refocus()
    },
    refocus() {
      this.$store.commit('refresh_window_focus', { uuid: this.uuid })
    },
    mr_clicked(event) {
      event.stopPropagation()
    }
  }
}
</script>

<style scoped>
.init_size {
  width: min(1000px, 65vw);
  height: min(620px, 65vh);
}

.cursor-move:hover {
  cursor: move;
}
</style>