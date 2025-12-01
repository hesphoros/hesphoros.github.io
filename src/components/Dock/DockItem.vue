<template>
  <button 
    ref="itemRef"
    class="dock-item"
    :class="{ 'is-active': isActive }"
    @mouseenter="handleMouseEnter"
    @mouseleave="handleMouseLeave"
    @click="handleClick"
  >
    <!-- Tooltip -->
    <p 
      class="dock-item-tooltip" 
      :class="{ 'tooltip-visible': showTooltip }"
      :style="{ top: tooltipTop }"
    >
      {{ tooltip }}
    </p>

    <!-- Icon with spring animation -->
    <span 
      class="dock-icon-wrapper"
      :style="{ transform: `translateY(${bounceOffset}px)` }"
    >
      <img 
        v-if="iconType === 'image'" 
        ref="imgRef"
        :src="require('../../assets/images/icons/' + icon + '.png')" 
        :style="imgStyle"
        draggable="false"
      />
      <img 
        v-else-if="iconType === 'appIcon'" 
        ref="imgRef"
        :src="getAppIconSrc()"
        :style="imgStyle"
        draggable="false"
      />
      <div 
        v-else-if="iconType === 'mdi'"
        class="mdi-icon-wrapper"
        :style="imgStyle"
      >
        <v-icon class="mdi-icon" :style="{ fontSize: (currentWidth * 0.55) + 'px' }">
          mdi-{{ mdiIcon }}
        </v-icon>
      </div>
    </span>

    <!-- Open indicator dot -->
    <div class="dock-item-dot" :style="{ opacity: isOpen ? 1 : 0 }"></div>
  </button>
</template>

<script>
// 基础配置 - 对标 macos-web
const BASE_WIDTH = 57.6
const DISTANCE_LIMIT = BASE_WIDTH * 6
const BEYOND_DISTANCE_LIMIT = DISTANCE_LIMIT + 1

// 距离到宽度的映射表 - 从 macos-web 直接移植
const DISTANCE_INPUT = [
  -DISTANCE_LIMIT,
  -DISTANCE_LIMIT / 1.25,
  -DISTANCE_LIMIT / 2,
  0,
  DISTANCE_LIMIT / 2,
  DISTANCE_LIMIT / 1.25,
  DISTANCE_LIMIT
]

const WIDTH_OUTPUT = [
  BASE_WIDTH,
  BASE_WIDTH * 1.1,
  BASE_WIDTH * 1.414,
  BASE_WIDTH * 2,
  BASE_WIDTH * 1.414,
  BASE_WIDTH * 1.1,
  BASE_WIDTH
]

// 线性插值函数
function lerp(a, b, t) {
  return a + (b - a) * t
}

// 创建插值函数 - 模拟 popmotion 的 interpolate
function createInterpolator(inputRange, outputRange) {
  return function(value) {
    // 边界检查
    if (value <= inputRange[0]) return outputRange[0]
    if (value >= inputRange[inputRange.length - 1]) return outputRange[outputRange.length - 1]

    // 找到value所在的区间
    let i = 0
    for (i = 0; i < inputRange.length - 1; i++) {
      if (value >= inputRange[i] && value <= inputRange[i + 1]) break
    }

    // 计算在区间内的位置 (0-1)
    const rangeLength = inputRange[i + 1] - inputRange[i]
    const t = (value - inputRange[i]) / rangeLength

    // 线性插值
    return lerp(outputRange[i], outputRange[i + 1], t)
  }
}

// 创建宽度插值器
const getWidthFromDistance = createInterpolator(DISTANCE_INPUT, WIDTH_OUTPUT)

export default {
  name: 'DockItem',
  props: {
    icon: {
      type: String,
      default: ''
    },
    iconType: {
      type: String,
      default: 'image'
    },
    appIcon: {
      type: String,
      default: ''
    },
    mdiIcon: {
      type: String,
      default: ''
    },
    tooltip: {
      type: String,
      default: ''
    },
    mouseX: {
      type: Number,
      default: null
    },
    isActive: {
      type: Boolean,
      default: false
    },
    isOpen: {
      type: Boolean,
      default: false
    }
  },
  data() {
    return {
      showTooltip: false,
      tooltipTimer: null,
      // Spring 动画状态
      currentWidth: BASE_WIDTH,
      targetWidth: BASE_WIDTH,
      lastValue: BASE_WIDTH,
      lastTime: 0,
      animationFrame: null,
      isAnimating: false,
      // 点击弹跳
      bounceOffset: 0,
      bounceFrame: null,
      // 距离
      distance: BEYOND_DISTANCE_LIMIT
    }
  },
  computed: {
    imgStyle() {
      return {
        width: (this.currentWidth / 16) + 'rem',
        height: (this.currentWidth / 16) + 'rem'
      }
    },
    tooltipTop() {
      // Tooltip 位置随图标大小变化
      return `-${35 + (this.currentWidth - BASE_WIDTH) * 0.3}px`
    }
  },
  watch: {
    mouseX: {
      handler() {
        this.updateDistance()
      },
      immediate: true
    },
    distance() {
      // 根据距离计算目标宽度
      this.targetWidth = getWidthFromDistance(this.distance)
      this.startSpringAnimation()
    }
  },
  methods: {
    updateDistance() {
      if (!this.$refs.imgRef && !this.$refs.itemRef) {
        this.distance = BEYOND_DISTANCE_LIMIT
        return
      }

      if (this.mouseX === null) {
        this.distance = BEYOND_DISTANCE_LIMIT
        return
      }

      const el = this.$refs.imgRef || this.$refs.itemRef
      const rect = el.getBoundingClientRect()
      const imgCenterX = rect.left + rect.width / 2
      
      // 计算有符号距离（用于插值）
      this.distance = this.mouseX - imgCenterX
    },

    startSpringAnimation() {
      // 如果已经在动画中，不需要重新启动
      if (this.isAnimating) return
      
      this.isAnimating = true
      this.lastTime = performance.now()
      this.springLoop()
    },

    springLoop() {
      const currentTime = performance.now()
      // 限制最大时间步长，避免跳跃
      const deltaTime = Math.min(currentTime - this.lastTime, 64) * 0.06

      const delta = this.targetWidth - this.currentWidth
      
      // Spring 物理参数 - 与 macos-web 一致
      const stiffness = 0.12
      const damping = 0.47
      const precision = 0.01

      // 计算速度（基于位置变化）
      const velocity = (this.currentWidth - this.lastValue) / (deltaTime || 1)
      const spring = stiffness * delta
      const damper = damping * velocity
      const acceleration = spring - damper
      const d = (velocity + acceleration) * deltaTime

      this.lastValue = this.currentWidth

      // 检查是否已经接近目标
      if (Math.abs(d) < precision && Math.abs(delta) < precision) {
        this.currentWidth = this.targetWidth
        this.isAnimating = false
        this.animationFrame = null
        return
      }

      this.currentWidth += d
      this.lastTime = currentTime
      this.animationFrame = requestAnimationFrame(() => this.springLoop())
    },

    handleMouseEnter() {
      this.tooltipTimer = setTimeout(() => {
        this.showTooltip = true
      }, 400)
    },

    handleMouseLeave() {
      if (this.tooltipTimer) {
        clearTimeout(this.tooltipTimer)
        this.tooltipTimer = null
      }
      this.showTooltip = false
    },

    handleClick() {
      this.$emit('click')
      this.bounceEffect()
    },

    // 点击弹跳效果
    async bounceEffect() {
      if (this.bounceFrame) {
        cancelAnimationFrame(this.bounceFrame)
      }

      const duration = 400
      const maxBounce = -40
      const startTime = performance.now()

      const animate = () => {
        const elapsed = performance.now() - startTime
        const progress = Math.min(elapsed / duration, 1)

        // 使用 sine easing
        const easeProgress = Math.sin(progress * Math.PI)
        this.bounceOffset = easeProgress * maxBounce * (1 - progress * 0.5)

        if (progress < 1) {
          this.bounceFrame = requestAnimationFrame(animate)
        } else {
          this.bounceOffset = 0
          this.bounceFrame = null
        }
      }

      this.bounceFrame = requestAnimationFrame(animate)
    },

    // 获取 appIcon 的图标路径（支持 .png 和 .webp）
    getAppIconSrc() {
      // 优先使用 256.png，某些图标目录只有 png
      try {
        return require('../../icon/' + this.appIcon + '/256.png')
      } catch (e) {
        // 如果没有 png，尝试 webp
        try {
          return require('../../icon/' + this.appIcon + '/256.webp')
        } catch (e2) {
          // 最后尝试 512.png
          return require('../../icon/' + this.appIcon + '/512.png')
        }
      }
    }
  },

  mounted() {
    // 立即更新一次距离
    this.$nextTick(() => {
      this.updateDistance()
    })
  },

  beforeDestroy() {
    if (this.animationFrame) {
      cancelAnimationFrame(this.animationFrame)
    }
    if (this.bounceFrame) {
      cancelAnimationFrame(this.bounceFrame)
    }
    if (this.tooltipTimer) {
      clearTimeout(this.tooltipTimer)
    }
  }
}
</script>

<style scoped>
.dock-item {
  position: relative;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-end;
  padding: 0;
  background: transparent;
  border: none;
  cursor: pointer;
  outline: none;
  border-radius: 8px;
}

.dock-item:focus-visible .dock-item-tooltip {
  display: block;
}

.dock-icon-wrapper {
  display: flex;
  justify-content: center;
  align-items: center;
  will-change: transform;
}

.dock-icon-wrapper img {
  display: block;
  object-fit: contain;
  will-change: width, height;
  /* 无过渡，完全依赖 spring 动画 */
}

.mdi-icon-wrapper {
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(100, 100, 100, 0.3);
  border-radius: 12px;
}

.mdi-icon-wrapper .mdi-icon {
  color: #333;
}

.dock-item-tooltip {
  position: absolute;
  left: 50%;
  transform: translateX(-50%);
  padding: 8px 12px;
  background-color: rgba(50, 50, 50, 0.85);
  backdrop-filter: blur(8px);
  -webkit-backdrop-filter: blur(8px);
  border-radius: 6px;
  color: white;
  font-size: 13px;
  font-weight: 400;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  letter-spacing: 0.3px;
  line-height: 1.4;
  white-space: nowrap;
  pointer-events: none;
  z-index: 9999;
  box-shadow: 
    0 1px 5px 2px rgba(0, 0, 0, 0.3),
    inset 0 0 0 0.5px rgba(255, 255, 255, 0.1);
  display: none;
  margin: 0;
  padding: 8px 12px;
  text-align: center;
}

.dock-item-tooltip.tooltip-visible {
  display: block;
}

.dock-item-dot {
  height: 4px;
  width: 4px;
  margin: 0;
  margin-top: 2px;
  border-radius: 50%;
  background-color: #333;
  transition: opacity 0.15s ease;
}

.dock-item.is-active .dock-item-dot {
  background: linear-gradient(
    to right,
    rgba(82, 153, 195, 0.7),
    rgba(77, 191, 255, 1),
    rgba(82, 153, 195, 0.7)
  );
  width: 16px;
  height: 3px;
  border-radius: 2px;
}

/* 悬停效果 */
.dock-item:hover .dock-icon-wrapper img {
  filter: brightness(1.05);
}

.dock-item:active .dock-icon-wrapper img {
  filter: brightness(0.95);
}
</style>
