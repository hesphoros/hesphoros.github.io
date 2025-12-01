<template>
  <div class="dock-container" @mouseleave="handleMouseLeave">
    <div 
      class="dock-el"
      ref="dockRef"
      @mousemove="handleMouseMove"
    >
      <!-- 左侧固定图标 -->
      <div class="dock-section dock-left">
        <DockItem 
          iconType="appIcon"
          appIcon="launchpad"
          :mouseX="mouseX"
          tooltip="Launchpad"
          @click="() => {}"
        />
        <DockItem 
          icon="desktop" 
          :mouseX="mouseX"
          tooltip="Show Desktop"
          @click="showdesktop_clicked"
        />
        <DockItem 
          icon="tasks" 
          :mouseX="mouseX"
          tooltip="Multitasking View"
          @click="exit_full_screen"
        />
        <div class="dock-divider"></div>
      </div>

      <!-- 中间应用图标 -->
      <div class="dock-section dock-center">
        <DockItem 
          v-if="has_explorer"
          iconType="appIcon"
          appIcon="finder"
          :mouseX="mouseX"
          tooltip="Finder"
          :isActive="$store.state.current_focus_type === 'explorer'"
          :isOpen="has_explorer"
          @click="explorer_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'explorer', has_explorer)"
        />
        <DockItem 
          v-if="has_text"
          iconType="appIcon"
          appIcon="notes"
          :mouseX="mouseX"
          tooltip="Notes"
          :isActive="$store.state.current_focus_type === 'text'"
          :isOpen="has_text"
          @click="text_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'text', has_text)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="safari"
          :mouseX="mouseX"
          tooltip="Browser"
          :isActive="$store.state.current_focus_type === 'browser'"
          :isOpen="has_browser"
          @click="browser_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'browser', has_browser)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="music"
          :mouseX="mouseX"
          tooltip="Music"
          :isActive="$store.state.current_focus_type === 'music'"
          :isOpen="has_music"
          @click="music_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'music', has_music)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="vscode"
          :mouseX="mouseX"
          tooltip="VSCode"
          :isActive="$store.state.current_focus_type === 'vscode'"
          :isOpen="has_vscode"
          @click="vscode_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'vscode', has_vscode)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="system-preferences"
          :mouseX="mouseX"
          tooltip="Settings"
          :isActive="$store.state.current_focus_type === 'settings'"
          :isOpen="has_settings"
          @click="settings_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'settings', has_settings)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="terminal"
          :mouseX="mouseX"
          tooltip="Terminal"
          :isActive="$store.state.current_focus_type === 'terminal'"
          :isOpen="has_terminal"
          @click="terminal_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'terminal', has_terminal)"
        />
        <DockItem 
          iconType="appIcon"
          appIcon="calculator"
          :mouseX="mouseX"
          tooltip="Calculator"
          :isActive="$store.state.current_focus_type === 'calculator'"
          :isOpen="has_calculator"
          @click="calculator_clicked"
          @contextmenu.prevent.native="mr_clicked($event, 'calculator', has_calculator)"
        />
        <div class="dock-divider"></div>
        <DockItem 
          icon="recycle" 
          :mouseX="mouseX"
          tooltip="Recycle Bin"
          @click="() => {}"
        />
      </div>

      <!-- 右侧系统图标 -->
      <div class="dock-section dock-right">
        <div class="dock-divider"></div>
        <!-- 时间显示 -->
        <div class="dock-time" @mouseenter="showTimeTooltip = true" @mouseleave="showTimeTooltip = false">
          <div class="time-display">
            <span class="time-text">{{ PrefixZero(date_hour, 2) }}:{{ PrefixZero(date_minute, 2) }}</span>
            <span class="date-text">{{ date_month }}/{{ date_date }}</span>
          </div>
          <div class="dock-tooltip" v-show="showTimeTooltip">
            {{ date_year }}/{{ date_month }}/{{ date_date }}
          </div>
        </div>
        <DockItem 
          iconType="mdi"
          mdiIcon="bell"
          :mouseX="mouseX"
          :tooltip="display_article_num + ' Messages'"
          @click="bell_clicked"
        />
        <DockItem 
          iconType="mdi"
          mdiIcon="power-standby"
          :mouseX="mouseX"
          tooltip="Power"
          @click="halt_clicked"
        />
      </div>
    </div>
  </div>
</template>

<script>
import DockItem from './DockItem.vue'

export default {
  name: 'Dock',
  components: {
    DockItem
  },
  data() {
    return {
      mouseX: null,
      date_hour: 0,
      date_minute: 0,
      date_year: 1970,
      date_month: 1,
      date_date: 1,
      showTimeTooltip: false,
      has_text: false,
      has_explorer: false,
      has_music: false,
      has_browser: false,
      has_settings: false,
      has_terminal: false,
      has_vscode: false,
      has_calculator: false
    }
  },
  created() {
    this.refresh_time()
    window.setInterval(() => {
      this.refresh_time()
    }, 1000)
  },
  watch: {
    global_window_list() {
      let status = {
        explorer: false,
        text: false,
        browser: false,
        music: false,
        settings: false,
        terminal: false,
        vscode: false,
        calculator: false
      }
      let keys = Object.keys(status)
      for (let item of this.$store.state.window_list) {
        if (keys.indexOf(item.type) >= 0) {
          status[item.type] = true
        }
      }
      for (let key of keys) {
        this['has_' + key] = status[key]
      }
    }
  },
  computed: {
    global_window_list() {
      return this.$store.state.window_list
    },
    display_article_num() {
      return this.$store.state.display_article_num
    }
  },
  methods: {
    handleMouseMove(e) {
      this.mouseX = e.clientX
    },
    handleMouseLeave() {
      this.mouseX = null
    },
    PrefixZero(num, n) {
      return (Array(n).join(0) + num).slice(-n)
    },
    refresh_time() {
      let date = new Date()
      this.date_year = date.getFullYear()
      this.date_month = date.getMonth() + 1
      this.date_date = date.getDate()
      this.date_hour = date.getHours()
      this.date_minute = date.getMinutes()
    },
    bell_clicked() {
      this.$store.commit('change_side_bar_status')
    },
    browser_clicked() {
      if (this.has_browser) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'browser' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'browser' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'browser' })
      }
      this.$store.commit('refresh_window_focus', { type: 'browser' })
    },
    music_clicked() {
      if (this.has_music) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'music' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'music' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'music' })
      }
      this.$store.commit('refresh_window_focus', { type: 'music' })
    },
    settings_clicked() {
      if (this.has_settings) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'settings' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'settings' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'settings' })
      }
      this.$store.commit('refresh_window_focus', { type: 'settings' })
    },
    terminal_clicked() {
      if (this.has_terminal) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'terminal' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'terminal' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'terminal' })
      }
      this.$store.commit('refresh_window_focus', { type: 'terminal' })
    },
    vscode_clicked() {
      if (this.has_vscode) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'vscode' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'vscode' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'vscode' })
      }
      this.$store.commit('refresh_window_focus', { type: 'vscode' })
    },
    calculator_clicked() {
      if (this.has_calculator) {
        if (this.window_type_all_hide_or_at_the_top({ type: 'calculator' })) {
          this.$store.commit('switch_global_window_show_status', { type: 'calculator' })
        }
      } else {
        this.$store.commit('open_new_window', { type: 'calculator' })
      }
      this.$store.commit('refresh_window_focus', { type: 'calculator' })
    },
    text_clicked() {
      if (this.window_type_all_hide_or_at_the_top({ type: 'text' })) {
        this.$store.commit('switch_global_window_show_status', { type: 'text' })
      }
      this.$store.commit('refresh_window_focus', { type: 'text' })
    },
    explorer_clicked() {
      if (this.window_type_all_hide_or_at_the_top({ type: 'explorer' })) {
        this.$store.commit('switch_global_window_show_status', { type: 'explorer' })
      }
      this.$store.commit('refresh_window_focus', { type: 'explorer' })
    },
    halt_clicked() {
      document.body.style.cursor = 'progress'
      window.setTimeout(() => {
        document.body.style.cursor = 'default'
        this.$router.push({
          name: 'LoginPage'
        })
      }, 800)
    },
    showdesktop_clicked() {
      this.$store.commit('switch_show_desktop')
    },
    exit_full_screen() {
      if (document.exitFullscreen) {
        document.exitFullscreen().catch(() => {})
      } else if (document.webkitExitFullscreen) {
        document.webkitExitFullscreen()
      } else if (document.mozCancelFullScreen) {
        document.mozCancelFullScreen()
      } else if (document.msExitFullscreen) {
        document.msExitFullscreen()
      }
    },
    window_type_all_hide_or_at_the_top(payload) {
      let max_index = 0
      let max_type_name = ''
      let allHide = true
      for (let item of this.$store.state.window_list) {
        if (item.zindex > max_index && !item.minimized) {
          max_type_name = item.type
          max_index = item.zindex
        }
        if (item.type === payload.type) {
          allHide &&= item.minimized
        }
      }
      return max_type_name === payload.type || allHide
    },
    mr_clicked(event, target, mode) {
      event.stopPropagation()
      this.$store.commit('show_context_menu_bottom_bar', { target: target, mode: mode })
    }
  }
}
</script>

<style scoped>
.dock-container {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  display: flex;
  justify-content: center;
  padding-bottom: 8px;
  z-index: 1001;
  pointer-events: none;
  height: 90px;
  padding: 6px;
}

.dock-el {
  display: flex;
  align-items: flex-end;
  justify-content: center;
  padding: 4px 10px;
  background-color: rgba(235, 235, 235, 0.45);
  backdrop-filter: blur(25px);
  -webkit-backdrop-filter: blur(25px);
  border-radius: 18px;
  box-shadow: 
    inset 0 0 0 0.5px rgba(255, 255, 255, 0.5),
    0 0 0 0.5px rgba(0, 0, 0, 0.15),
    0 4px 30px rgba(0, 0, 0, 0.25);
  pointer-events: auto;
  position: relative;
  height: 100%;
  transition: transform 0.3s ease;
}

.dock-el::before {
  content: '';
  border-radius: 18px;
  width: 100%;
  height: 100%;
  border: inherit;
  backdrop-filter: blur(25px);
  position: absolute;
  top: 0;
  left: 0;
  z-index: -1;
}

.dock-section {
  display: flex;
  align-items: flex-end;
}

.dock-divider {
  width: 1px;
  height: 100%;
  max-height: 42px;
  background-color: rgba(55, 55, 55, 0.35);
  margin: 0 6px;
  align-self: center;
}

.dock-time {
  position: relative;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 4px 10px;
  cursor: default;
  user-select: none;
}

.time-display {
  display: flex;
  flex-direction: column;
  align-items: center;
  line-height: 1.2;
}

.time-text {
  font-size: 14px;
  font-weight: 600;
  color: #333;
}

.date-text {
  font-size: 10px;
  font-weight: 500;
  color: #555;
}

.dock-tooltip {
  position: absolute;
  bottom: 100%;
  left: 50%;
  transform: translateX(-50%);
  margin-bottom: 8px;
  padding: 8px 12px;
  background-color: rgba(50, 50, 50, 0.85);
  backdrop-filter: blur(8px);
  border-radius: 6px;
  color: white;
  font-size: 13px;
  font-weight: 400;
  white-space: nowrap;
  pointer-events: none;
  z-index: 9999;
  box-shadow: 
    0 1px 5px 2px rgba(0, 0, 0, 0.3),
    inset 0 0 0 0.5px rgba(255, 255, 255, 0.1);
}
</style>
