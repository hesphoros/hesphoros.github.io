<template>
  <header class="topbar" :class="{ dark: isDarkMode }">
    <!-- 左侧菜单栏 -->
    <div class="menubar">
      <!-- Apple Logo -->
      <button 
        class="menu-button apple-icon-button"
        :class="{ active: activeMenu === 'apple' }"
        @click="toggleMenu('apple')"
        @mouseenter="hoverMenu('apple')"
      >
        <svg viewBox="0 0 24 24" width="16" height="16" fill="currentColor">
          <path d="M18.71 19.5c-.83 1.24-1.71 2.45-3.05 2.47-1.34.03-1.77-.79-3.29-.79-1.53 0-2 .77-3.27.82-1.31.05-2.3-1.32-3.14-2.53C4.25 17 2.94 12.45 4.7 9.39c.87-1.52 2.43-2.48 4.12-2.51 1.28-.02 2.5.87 3.29.87.78 0 2.26-1.07 3.81-.91.65.03 2.47.26 3.64 1.98-.09.06-2.17 1.28-2.15 3.81.03 3.02 2.65 4.03 2.68 4.04-.03.07-.42 1.44-1.38 2.83M13 3.5c.73-.83 1.94-1.46 2.94-1.5.13 1.17-.34 2.35-1.04 3.19-.69.85-1.83 1.51-2.95 1.42-.15-1.15.41-2.35 1.05-3.11z"/>
        </svg>
      </button>

      <!-- Finder 菜单 -->
      <button 
        class="menu-button app-menu"
        :class="{ active: activeMenu === 'finder' }"
        @click="toggleMenu('finder')"
        @mouseenter="hoverMenu('finder')"
      >
        <strong>Finder</strong>
      </button>

      <!-- File 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'file' }"
        @click="toggleMenu('file')"
        @mouseenter="hoverMenu('file')"
      >
        File
      </button>

      <!-- Edit 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'edit' }"
        @click="toggleMenu('edit')"
        @mouseenter="hoverMenu('edit')"
      >
        Edit
      </button>

      <!-- View 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'view' }"
        @click="toggleMenu('view')"
        @mouseenter="hoverMenu('view')"
      >
        View
      </button>

      <!-- Go 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'go' }"
        @click="toggleMenu('go')"
        @mouseenter="hoverMenu('go')"
      >
        Go
      </button>

      <!-- Window 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'window' }"
        @click="toggleMenu('window')"
        @mouseenter="hoverMenu('window')"
      >
        Window
      </button>

      <!-- Help 菜单 -->
      <button 
        class="menu-button"
        :class="{ active: activeMenu === 'help' }"
        @click="toggleMenu('help')"
        @mouseenter="hoverMenu('help')"
      >
        Help
      </button>
    </div>

    <!-- 中间弹性空间 -->
    <span class="spacer"></span>

    <!-- 右侧状态栏 -->
    <div class="status-bar">
      <!-- 控制中心按钮 -->
      <button class="status-button" @click="toggleControlCenter">
        <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
          <path d="M7.5 6a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM6 7.5a1.5 1.5 0 1 1 3 0 1.5 1.5 0 0 1-3 0z"/>
          <path d="M10.5 12a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM9 13.5a1.5 1.5 0 1 1 3 0 1.5 1.5 0 0 1-3 0z"/>
          <path d="M16.5 6a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM15 7.5a1.5 1.5 0 1 1 3 0 1.5 1.5 0 0 1-3 0z"/>
          <path d="M13.5 15a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM12 16.5a1.5 1.5 0 1 1 3 0 1.5 1.5 0 0 1-3 0z"/>
        </svg>
      </button>

      <!-- 时间日期 -->
      <button class="status-button time-button">
        {{ formattedDateTime }}
      </button>
    </div>

    <!-- 下拉菜单 -->
    <transition name="menu-fade">
      <div 
        v-if="activeMenu"
        class="dropdown-menu"
        :style="dropdownStyle"
        @click.stop
      >
        <div class="menu-content">
          <div v-for="(item, key) in currentMenuItems" :key="key" class="menu-item-wrapper">
            <button 
              class="menu-item"
              :class="{ disabled: item.disabled }"
              :disabled="item.disabled"
              @click="handleMenuItemClick(item)"
            >
              {{ item.title }}
            </button>
            <div v-if="item.breakAfter" class="menu-divider"></div>
          </div>
        </div>
      </div>
    </transition>

    <!-- 点击外部关闭菜单 -->
    <div 
      v-if="activeMenu"
      class="menu-overlay"
      @click="closeMenu"
    ></div>
  </header>
</template>

<script>
export default {
  name: 'TopBar',
  data() {
    return {
      activeMenu: null,
      currentTime: new Date(),
      timeInterval: null,
      isDarkMode: false,
      dropdownLeft: 0,
      menus: {
        apple: {
          'about-this-mac': { title: 'About This Mac', breakAfter: true },
          'system-preferences': { title: 'System Preferences...' },
          'app-store': { title: 'App Store...', breakAfter: true },
          'recent-items': { title: 'Recent Items', breakAfter: true },
          'force-quit': { title: 'Force Quit...', breakAfter: true },
          'sleep': { title: 'Sleep' },
          'restart': { title: 'Restart...' },
          'shutdown': { title: 'Shut Down...', breakAfter: true },
          'lock-screen': { title: 'Lock Screen' },
          'logout': { title: 'Log Out User...' }
        },
        finder: {
          'about-finder': { title: 'About Finder', breakAfter: true },
          'preferences': { title: 'Preferences...', breakAfter: true },
          'empty-trash': { title: 'Empty Trash...', breakAfter: true },
          'hide-finder': { title: 'Hide Finder' },
          'hide-others': { title: 'Hide Others' },
          'show-all': { title: 'Show All', disabled: true }
        },
        file: {
          'new-finder-window': { title: 'New Finder Window' },
          'new-folder': { title: 'New Folder' },
          'new-folder-selection': { title: 'New Folder with Selection', disabled: true },
          'new-smart-folder': { title: 'New Smart Folder' },
          'new-tab': { title: 'New Tab', breakAfter: true },
          'open': { title: 'Open' },
          'open-with': { title: 'Open With', breakAfter: true },
          'close-window': { title: 'Close Window' },
          'get-info': { title: 'Get Info', breakAfter: true },
          'rename': { title: 'Rename', disabled: true }
        },
        edit: {
          'undo': { title: 'Undo', disabled: true },
          'redo': { title: 'Redo', disabled: true, breakAfter: true },
          'cut': { title: 'Cut', disabled: true },
          'copy': { title: 'Copy', disabled: true },
          'paste': { title: 'Paste' },
          'select-all': { title: 'Select All', breakAfter: true },
          'show-clipboard': { title: 'Show Clipboard' }
        },
        view: {
          'as-icons': { title: 'as Icons' },
          'as-list': { title: 'as List' },
          'as-columns': { title: 'as Columns' },
          'as-gallery': { title: 'as Gallery', breakAfter: true },
          'clean-up': { title: 'Clean Up' },
          'clean-up-by': { title: 'Clean Up By', breakAfter: true },
          'show-path-bar': { title: 'Show Path Bar' },
          'show-status-bar': { title: 'Show Status Bar' }
        },
        go: {
          'back': { title: 'Back', disabled: true },
          'forward': { title: 'Forward', disabled: true },
          'enclosing-folder': { title: 'Enclosing Folder', breakAfter: true },
          'recents': { title: 'Recents' },
          'documents': { title: 'Documents' },
          'desktop': { title: 'Desktop' },
          'downloads': { title: 'Downloads' },
          'home': { title: 'Home', breakAfter: true },
          'computer': { title: 'Computer' },
          'network': { title: 'Network' }
        },
        window: {
          'minimize': { title: 'Minimize' },
          'zoom': { title: 'Zoom', breakAfter: true },
          'move-to-ipad': { title: 'Move to iPad', disabled: true, breakAfter: true },
          'cycle-windows': { title: 'Cycle Through Windows', breakAfter: true },
          'bring-all-to-front': { title: 'Bring All to Front' }
        },
        help: {
          'send-feedback': { title: 'Send Finder Feedback', breakAfter: true },
          'macos-help': { title: 'macOS Help' }
        }
      }
    }
  },
  computed: {
    formattedDateTime() {
      const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat']
      const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
      
      const day = days[this.currentTime.getDay()]
      const month = months[this.currentTime.getMonth()]
      const date = this.currentTime.getDate()
      
      let hours = this.currentTime.getHours()
      const minutes = this.currentTime.getMinutes().toString().padStart(2, '0')
      const ampm = hours >= 12 ? 'PM' : 'AM'
      hours = hours % 12 || 12
      
      return `${day} ${month} ${date}  ${hours}:${minutes} ${ampm}`
    },
    currentMenuItems() {
      return this.activeMenu ? this.menus[this.activeMenu] : {}
    },
    dropdownStyle() {
      return {
        left: this.dropdownLeft + 'px'
      }
    }
  },
  mounted() {
    this.timeInterval = setInterval(() => {
      this.currentTime = new Date()
    }, 1000)
    
    // 监听点击事件关闭菜单
    document.addEventListener('click', this.handleDocumentClick)
  },
  beforeDestroy() {
    if (this.timeInterval) {
      clearInterval(this.timeInterval)
    }
    document.removeEventListener('click', this.handleDocumentClick)
  },
  methods: {
    toggleMenu(menuName) {
      if (this.activeMenu === menuName) {
        this.activeMenu = null
      } else {
        this.activeMenu = menuName
        this.updateDropdownPosition(menuName)
      }
    },
    hoverMenu(menuName) {
      if (this.activeMenu) {
        this.activeMenu = menuName
        this.updateDropdownPosition(menuName)
      }
    },
    updateDropdownPosition(menuName) {
      this.$nextTick(() => {
        const button = this.$el.querySelector(`.menu-button.${menuName === 'apple' ? 'apple-icon-button' : ''}[class*="${menuName}"], .menu-button:nth-child(${this.getMenuIndex(menuName)})`)
        if (button) {
          const rect = button.getBoundingClientRect()
          this.dropdownLeft = rect.left
        } else {
          // 备用方案：基于菜单索引计算位置
          const menuOrder = ['apple', 'finder', 'file', 'edit', 'view', 'go', 'window', 'help']
          const index = menuOrder.indexOf(menuName)
          this.dropdownLeft = 8 + index * 60 // 估算位置
        }
      })
    },
    getMenuIndex(menuName) {
      const menuOrder = ['apple', 'finder', 'file', 'edit', 'view', 'go', 'window', 'help']
      return menuOrder.indexOf(menuName) + 1
    },
    closeMenu() {
      this.activeMenu = null
    },
    handleDocumentClick(e) {
      if (!this.$el.contains(e.target)) {
        this.activeMenu = null
      }
    },
    handleMenuItemClick(item) {
      if (!item.disabled) {
        // 处理菜单项点击
        console.log('Menu item clicked:', item.title)
        this.activeMenu = null
      }
    },
    toggleControlCenter() {
      // 打开控制中心（侧边栏）
      this.$store.commit('change_side_bar_status')
    }
  }
}
</script>

<style scoped>
.topbar {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  height: 28px;
  display: flex;
  align-items: center;
  /* 深色半透明背景，适配深色壁纸 */
  background-color: rgba(30, 30, 30, 0.55);
  backdrop-filter: saturate(180%) blur(25px);
  -webkit-backdrop-filter: saturate(180%) blur(25px);
  z-index: 2000;
  font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Text', 'Segoe UI', Roboto, sans-serif;
  font-size: 13px;
  font-weight: 400;
  /* 白色文字适配深色背景 */
  color: rgba(255, 255, 255, 0.9);
  padding: 0 8px;
  user-select: none;
  /* 添加底部细微边框增强层次感 */
  border-bottom: 0.5px solid rgba(255, 255, 255, 0.1);
}

.topbar.dark {
  background-color: rgba(20, 20, 20, 0.75);
  color: rgba(255, 255, 255, 0.9);
}

/* 亮色模式（如果需要） */
.topbar.light {
  background-color: rgba(255, 255, 255, 0.65);
  color: rgba(0, 0, 0, 0.85);
  border-bottom: 0.5px solid rgba(0, 0, 0, 0.1);
}

.menubar {
  display: flex;
  align-items: center;
  height: 100%;
  gap: 1px;
}

.menu-button {
  height: 100%;
  padding: 0 12px;
  background: transparent;
  border: none;
  cursor: pointer;
  font-size: 13px;
  font-weight: 400;
  font-family: inherit;
  color: inherit;
  border-radius: 4px;
  position: relative;
  display: flex;
  align-items: center;
  transition: background-color 0.1s ease;
}

.menu-button:hover,
.menu-button.active {
  background-color: rgba(255, 255, 255, 0.15);
}

.topbar.light .menu-button:hover,
.topbar.light .menu-button.active {
  background-color: rgba(0, 0, 0, 0.1);
}

.apple-icon-button {
  padding: 0 10px;
  margin-right: 4px;
}

.apple-icon-button svg {
  display: block;
}

.app-menu strong {
  font-weight: 600;
}

.spacer {
  flex: 1 1 auto;
}

.status-bar {
  display: flex;
  align-items: center;
  height: 100%;
  gap: 2px;
}

.status-button {
  height: 100%;
  padding: 0 10px;
  background: transparent;
  border: none;
  cursor: pointer;
  font-size: 13px;
  font-weight: 400;
  font-family: inherit;
  color: inherit;
  border-radius: 4px;
  display: flex;
  align-items: center;
  gap: 6px;
  transition: background-color 0.1s ease;
}

.status-button:hover {
  background-color: rgba(255, 255, 255, 0.15);
}

.topbar.light .status-button:hover {
  background-color: rgba(0, 0, 0, 0.1);
}

.time-button {
  font-weight: 500;
  letter-spacing: 0.2px;
}

/* 下拉菜单 - 深色半透明风格 */
.dropdown-menu {
  position: absolute;
  top: 100%;
  margin-top: 2px;
  min-width: 220px;
  background-color: rgba(40, 40, 40, 0.85);
  backdrop-filter: saturate(180%) blur(30px);
  -webkit-backdrop-filter: saturate(180%) blur(30px);
  border-radius: 6px;
  box-shadow: 
    0 10px 30px rgba(0, 0, 0, 0.35),
    0 0 0 0.5px rgba(255, 255, 255, 0.15),
    inset 0 0 0 0.5px rgba(255, 255, 255, 0.05);
  padding: 5px;
  z-index: 2001;
}

.topbar.light .dropdown-menu {
  background-color: rgba(255, 255, 255, 0.85);
  box-shadow: 
    0 10px 30px rgba(0, 0, 0, 0.2),
    0 0 0 0.5px rgba(0, 0, 0, 0.1),
    inset 0 0 0 0.5px rgba(255, 255, 255, 0.3);
}

.menu-content {
  display: flex;
  flex-direction: column;
}

.menu-item-wrapper {
  display: contents;
}

.menu-item {
  width: 100%;
  padding: 4px 12px;
  background: transparent;
  border: none;
  cursor: pointer;
  font-size: 13px;
  font-weight: 400;
  font-family: inherit;
  color: rgba(255, 255, 255, 0.9);
  text-align: left;
  border-radius: 4px;
  transition: none;
}

.topbar.light .menu-item {
  color: rgba(0, 0, 0, 0.85);
}

.menu-item:hover:not(.disabled) {
  background-color: #0066d6;
  color: white;
}

.menu-item.disabled {
  color: rgba(255, 255, 255, 0.35);
  cursor: default;
}

.topbar.light .menu-item.disabled {
  color: rgba(0, 0, 0, 0.35);
}

.menu-divider {
  height: 1px;
  background-color: rgba(255, 255, 255, 0.12);
  margin: 5px 10px;
}

.topbar.light .menu-divider {
  background-color: rgba(0, 0, 0, 0.1);
}

.menu-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: 1999;
}

/* 过渡动画 */
.menu-fade-enter-active,
.menu-fade-leave-active {
  transition: opacity 0.15s ease, transform 0.15s ease;
}

.menu-fade-enter-from,
.menu-fade-leave-to {
  opacity: 0;
  transform: translateY(-4px);
}

.menu-fade-enter-to,
.menu-fade-leave-from {
  opacity: 1;
  transform: translateY(0);
}
</style>
