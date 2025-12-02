<template>
  <div 
    class="bootup-screen" 
    :class="{ 'fade-out': !show_loading_page }" 
    :style="{ 'z-index': zindex }" 
    @contextmenu.prevent="{}"
  >
    <!-- Apple Logo -->
    <svg 
      class="apple-logo" 
      viewBox="0 0 170 170" 
      xmlns="http://www.w3.org/2000/svg"
    >
      <path 
        fill="currentColor" 
        d="M150.37 130.25c-2.45 5.66-5.35 10.87-8.71 15.66-4.58 6.53-8.33 11.05-11.22 13.56-4.48 4.12-9.28 6.23-14.42 6.35-3.69 0-8.14-1.05-13.32-3.18-5.197-2.12-9.973-3.17-14.34-3.17-4.58 0-9.492 1.05-14.746 3.17-5.262 2.13-9.501 3.24-12.742 3.35-4.929.21-9.842-1.96-14.746-6.52-3.13-2.73-7.045-7.41-11.735-14.04-5.032-7.08-9.169-15.29-12.41-24.65-3.471-10.11-5.211-19.9-5.211-29.378 0-10.857 2.346-20.221 7.045-28.068 3.693-6.303 8.606-11.275 14.755-14.925s12.793-5.51 19.948-5.629c3.915 0 9.049 1.211 15.429 3.591 6.362 2.388 10.447 3.599 12.238 3.599 1.339 0 5.877-1.416 13.57-4.239 7.275-2.618 13.415-3.702 18.445-3.275 13.63 1.1 23.87 6.473 30.68 16.153-12.19 7.386-18.22 17.731-18.1 31.002.11 10.337 3.86 18.939 11.23 25.769 3.34 3.17 7.07 5.62 11.22 7.36-.9 2.61-1.85 5.11-2.86 7.51zM119.11 7.24c0 8.102-2.96 15.667-8.86 22.669-7.12 8.324-15.732 13.134-25.071 12.375a25.222 25.222 0 0 1-.188-3.07c0-7.778 3.386-16.102 9.399-22.908 3.002-3.446 6.82-6.311 11.45-8.597 4.62-2.252 8.99-3.497 13.1-3.71.12 1.083.17 2.166.17 3.24z"
      />
    </svg>

    <!-- Progress Bar -->
    <div class="progress-container">
      <div 
        class="progress-bar" 
        role="progressbar"
        :aria-valuenow="progress"
        aria-valuemin="0"
        aria-valuemax="100"
        aria-valuetext="Loading macOS Web"
      >
        <div 
          class="progress-indicator" 
          :style="{ transform: `translateX(${progress - 100}%)` }"
        ></div>
      </div>
    </div>

    <!-- Startup Sound (optional, only in production) -->
    <audio 
      v-if="playSound && isProduction" 
      ref="startupSound" 
      preload="auto"
    >
      <source src="/sounds/mac-startup-sound.mp3" type="audio/mpeg">
    </audio>
  </div>
</template>

<script>
export default {
  name: 'Interlude',
  data() {
    return {
      timer: null,
      zindex: 9999,
      progress: 0,
      animationFrame: null,
      startTime: null,
      duration: 3000, // 3秒动画
      playSound: false, // 是否播放启动音效
      isProduction: process.env.NODE_ENV === 'production'
    }
  },
  computed: {
    show_loading_page() {
      return this.$store.state.show_loading_page
    }
  },
  watch: {
    show_loading_page(val) {
      if (!val) {
        this.timer = window.setTimeout(() => {
          this.zindex = -1
          this.timer = null
        }, 500) // 淡出动画时间
      } else {
        this.zindex = 9999
        if (this.timer != null) {
          clearTimeout(this.timer)
        }
        // 重新开始进度动画
        this.startProgressAnimation()
      }
    }
  },
  mounted() {
    this.startProgressAnimation()
    // 尝试播放启动音效
    if (this.playSound && this.$refs.startupSound) {
      this.$refs.startupSound.play().catch(() => {
        // 静默处理自动播放限制
      })
    }
  },
  beforeDestroy() {
    if (this.animationFrame) {
      cancelAnimationFrame(this.animationFrame)
    }
    if (this.timer) {
      clearTimeout(this.timer)
    }
  },
  methods: {
    startProgressAnimation() {
      this.progress = 0
      this.startTime = performance.now()
      this.animateProgress()
    },
    animateProgress() {
      const elapsed = performance.now() - this.startTime
      // 使用 easeInOutQuint 缓动函数
      const t = Math.min(elapsed / this.duration, 1)
      const eased = t < 0.5 
        ? 16 * t * t * t * t * t 
        : 1 - Math.pow(-2 * t + 2, 5) / 2
      
      this.progress = eased * 100

      if (t < 1) {
        this.animationFrame = requestAnimationFrame(() => this.animateProgress())
      }
    }
  }
}
</script>

<style scoped>
.bootup-screen {
  position: fixed;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  gap: 3rem;
  
  background-color: #000;
  
  cursor: none;
  pointer-events: auto;
  overflow: hidden;
  
  transition: opacity 0.5s ease-out;
}

.bootup-screen.fade-out {
  opacity: 0;
  pointer-events: none;
}

/* Apple Logo */
.apple-logo {
  width: 80px;
  height: 80px;
  color: #fff;
  
  /* 淡入动画 */
  animation: logo-fade-in 0.8s ease-out;
}

@keyframes logo-fade-in {
  from {
    opacity: 0;
    transform: scale(0.8);
  }
  to {
    opacity: 1;
    transform: scale(1);
  }
}

/* Progress Bar Container */
.progress-container {
  width: 200px;
  
  /* 淡入动画，延迟显示 */
  animation: progress-fade-in 0.5s ease-out 0.3s both;
}

@keyframes progress-fade-in {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

/* Progress Bar */
.progress-bar {
  height: 5px;
  border-radius: 50px;
  
  background-color: #3a3a3a;
  
  overflow: hidden;
}

/* Progress Indicator */
.progress-indicator {
  width: 100%;
  height: 100%;
  
  background-color: #e5e5e5;
  
  border-radius: inherit;
  
  transition: transform 0.05s linear;
}

/* 响应式调整 */
@media (max-width: 480px) {
  .apple-logo {
    width: 60px;
    height: 60px;
  }
  
  .progress-container {
    width: 150px;
  }
}
</style>