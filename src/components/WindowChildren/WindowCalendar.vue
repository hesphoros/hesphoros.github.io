<template>
  <Window
    :uuid="uuid"
    :startpos_x="startpos_x"
    :startpos_y="startpos_y"
    :zindex="zindex"
    :minimized="minimized"
    :default_width="400"
    :default_height="520"
    :fixedsize="false"
    :blacktheme="true"
  >
    <template v-slot:header>
      <div class="header-content">
        <span class="header-title">Calendar</span>
      </div>
    </template>
    <template v-slot:content>
      <div class="calendar-container">
        <!-- 顶部导航区域 -->
        <div class="calendar-header">
          <div class="header-left">
            <button class="nav-btn month-year-btn" @click="toggleViewMode">
              <span class="current-month">{{ monthNames[currentMonth] }}</span>
              <span class="current-year">{{ currentYear }}</span>
              <svg class="chevron-icon" :class="{ rotated: viewMode !== 'days' }" viewBox="0 0 12 12" fill="currentColor">
                <path d="M2.5 4.5L6 8L9.5 4.5" stroke="currentColor" stroke-width="1.5" fill="none" stroke-linecap="round" stroke-linejoin="round"/>
              </svg>
            </button>
          </div>
          <div class="header-right">
            <button class="nav-btn icon-btn" @click="goToToday" title="Today">
              <svg viewBox="0 0 20 20" fill="currentColor">
                <path d="M10 2a8 8 0 100 16 8 8 0 000-16zm0 14.5a6.5 6.5 0 110-13 6.5 6.5 0 010 13z"/>
                <circle cx="10" cy="10" r="3" fill="currentColor"/>
              </svg>
            </button>
            <button class="nav-btn icon-btn" @click="goPrevious" title="Previous">
              <svg viewBox="0 0 20 20" fill="currentColor">
                <path d="M12.5 15L7.5 10L12.5 5" stroke="currentColor" stroke-width="1.8" fill="none" stroke-linecap="round" stroke-linejoin="round"/>
              </svg>
            </button>
            <button class="nav-btn icon-btn" @click="goNext" title="Next">
              <svg viewBox="0 0 20 20" fill="currentColor">
                <path d="M7.5 15L12.5 10L7.5 5" stroke="currentColor" stroke-width="1.8" fill="none" stroke-linecap="round" stroke-linejoin="round"/>
              </svg>
            </button>
          </div>
        </div>

        <!-- 日历主体 -->
        <div class="calendar-body">
          <!-- 日视图 -->
          <transition name="fade" mode="out-in">
            <div v-if="viewMode === 'days'" key="days" class="days-view">
              <!-- 星期标题 -->
              <div class="weekday-header">
                <div v-for="day in weekDays" :key="day" class="weekday-cell">
                  {{ day }}
                </div>
              </div>
              <!-- 日期网格 -->
              <div class="days-grid">
                <button
                  v-for="(day, index) in calendarDays"
                  :key="index"
                  class="day-cell"
                  :class="{
                    'other-month': !day.isCurrentMonth,
                    'today': day.isToday,
                    'selected': day.isSelected,
                    'weekend': day.isWeekend
                  }"
                  @click="selectDate(day)"
                >
                  <span class="day-number">{{ day.date }}</span>
                  <span v-if="day.isToday" class="today-indicator"></span>
                </button>
              </div>
            </div>

            <!-- 月视图 -->
            <div v-else-if="viewMode === 'months'" key="months" class="months-view">
              <button
                v-for="(month, index) in monthNames"
                :key="index"
                class="month-cell"
                :class="{
                  'current': index === new Date().getMonth() && currentYear === new Date().getFullYear(),
                  'selected': index === currentMonth
                }"
                @click="selectMonth(index)"
              >
                {{ month.slice(0, 3) }}
              </button>
            </div>

            <!-- 年视图 -->
            <div v-else key="years" class="years-view">
              <button
                v-for="year in yearRange"
                :key="year"
                class="year-cell"
                :class="{
                  'current': year === new Date().getFullYear(),
                  'selected': year === currentYear
                }"
                @click="selectYear(year)"
              >
                {{ year }}
              </button>
            </div>
          </transition>
        </div>

        <!-- 底部信息区域 -->
        <div class="calendar-footer">
          <div class="selected-date-info">
            <div class="date-display">
              <span class="day-name">{{ selectedDayName }}</span>
              <span class="full-date">{{ formattedSelectedDate }}</span>
            </div>
          </div>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'

export default {
  name: 'WindowCalendar',
  components: {
    Window
  },
  props: {
    uuid: String,
    startpos_x: {
      default: 100
    },
    startpos_y: {
      default: 100
    },
    zindex: {
      type: Number,
      default: 999
    },
    minimized: {
      type: Boolean,
      default: false
    }
  },
  data() {
    const today = new Date()
    return {
      currentYear: today.getFullYear(),
      currentMonth: today.getMonth(),
      selectedDate: new Date(today),
      viewMode: 'days', // 'days', 'months', 'years'
      weekDays: ['Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa', 'Su'],
      monthNames: [
        'January', 'February', 'March', 'April', 'May', 'June',
        'July', 'August', 'September', 'October', 'November', 'December'
      ],
      dayNames: [
        'Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'
      ]
    }
  },
  computed: {
    calendarDays() {
      const days = []
      const firstDay = new Date(this.currentYear, this.currentMonth, 1)
      const lastDay = new Date(this.currentYear, this.currentMonth + 1, 0)
      const today = new Date()
      
      // 获取本月第一天是星期几 (0=周日, 需要转换为周一开始)
      let startDay = firstDay.getDay()
      startDay = startDay === 0 ? 6 : startDay - 1 // 转换为周一开始
      
      // 上个月的天数
      const prevMonthLastDay = new Date(this.currentYear, this.currentMonth, 0).getDate()
      
      // 添加上个月的日期
      for (let i = startDay - 1; i >= 0; i--) {
        const date = prevMonthLastDay - i
        const fullDate = new Date(this.currentYear, this.currentMonth - 1, date)
        days.push({
          date,
          isCurrentMonth: false,
          isToday: false,
          isSelected: this.isSameDay(fullDate, this.selectedDate),
          isWeekend: fullDate.getDay() === 0 || fullDate.getDay() === 6,
          fullDate
        })
      }
      
      // 添加本月的日期
      for (let date = 1; date <= lastDay.getDate(); date++) {
        const fullDate = new Date(this.currentYear, this.currentMonth, date)
        days.push({
          date,
          isCurrentMonth: true,
          isToday: this.isSameDay(fullDate, today),
          isSelected: this.isSameDay(fullDate, this.selectedDate),
          isWeekend: fullDate.getDay() === 0 || fullDate.getDay() === 6,
          fullDate
        })
      }
      
      // 添加下个月的日期以填满网格 (6行7列 = 42格)
      const remainingDays = 42 - days.length
      for (let date = 1; date <= remainingDays; date++) {
        const fullDate = new Date(this.currentYear, this.currentMonth + 1, date)
        days.push({
          date,
          isCurrentMonth: false,
          isToday: false,
          isSelected: this.isSameDay(fullDate, this.selectedDate),
          isWeekend: fullDate.getDay() === 0 || fullDate.getDay() === 6,
          fullDate
        })
      }
      
      return days
    },
    yearRange() {
      const startYear = Math.floor(this.currentYear / 10) * 10 - 1
      const years = []
      for (let i = 0; i < 12; i++) {
        years.push(startYear + i)
      }
      return years
    },
    selectedDayName() {
      return this.dayNames[this.selectedDate.getDay()]
    },
    formattedSelectedDate() {
      const day = this.selectedDate.getDate()
      const month = this.monthNames[this.selectedDate.getMonth()]
      const year = this.selectedDate.getFullYear()
      return `${month} ${day}, ${year}`
    }
  },
  methods: {
    isSameDay(date1, date2) {
      return date1.getDate() === date2.getDate() &&
             date1.getMonth() === date2.getMonth() &&
             date1.getFullYear() === date2.getFullYear()
    },
    selectDate(day) {
      this.selectedDate = day.fullDate
      if (!day.isCurrentMonth) {
        this.currentMonth = day.fullDate.getMonth()
        this.currentYear = day.fullDate.getFullYear()
      }
    },
    selectMonth(monthIndex) {
      this.currentMonth = monthIndex
      this.viewMode = 'days'
    },
    selectYear(year) {
      this.currentYear = year
      this.viewMode = 'months'
    },
    toggleViewMode() {
      if (this.viewMode === 'days') {
        this.viewMode = 'months'
      } else if (this.viewMode === 'months') {
        this.viewMode = 'years'
      } else {
        this.viewMode = 'days'
      }
    },
    goPrevious() {
      if (this.viewMode === 'days') {
        if (this.currentMonth === 0) {
          this.currentMonth = 11
          this.currentYear--
        } else {
          this.currentMonth--
        }
      } else if (this.viewMode === 'months') {
        this.currentYear--
      } else {
        this.currentYear -= 10
      }
    },
    goNext() {
      if (this.viewMode === 'days') {
        if (this.currentMonth === 11) {
          this.currentMonth = 0
          this.currentYear++
        } else {
          this.currentMonth++
        }
      } else if (this.viewMode === 'months') {
        this.currentYear++
      } else {
        this.currentYear += 10
      }
    },
    goToToday() {
      const today = new Date()
      this.currentYear = today.getFullYear()
      this.currentMonth = today.getMonth()
      this.selectedDate = new Date(today)
      this.viewMode = 'days'
    }
  }
}
</script>

<style scoped>
.calendar-container {
  width: 100%;
  height: 100%;
  background: linear-gradient(180deg, #2d2d2d 0%, #1f1f1f 100%);
  display: flex;
  flex-direction: column;
  font-family: 'Segoe UI Variable', 'Segoe UI', system-ui, -apple-system, sans-serif;
  color: #fff;
  overflow: hidden;
}

.header-content {
  display: flex;
  align-items: center;
  padding-left: 12px;
  height: 100%;
}

.header-title {
  color: #fff;
  font-size: 12px;
  font-weight: 400;
}

/* 日历头部 */
.calendar-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 16px 8px 16px;
}

.header-left {
  display: flex;
  align-items: center;
}

.header-right {
  display: flex;
  align-items: center;
  gap: 4px;
}

.nav-btn {
  background: transparent;
  border: none;
  color: #fff;
  cursor: pointer;
  border-radius: 4px;
  transition: background-color 0.15s ease;
}

.nav-btn:hover {
  background-color: rgba(255, 255, 255, 0.08);
}

.nav-btn:active {
  background-color: rgba(255, 255, 255, 0.04);
}

.month-year-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  font-size: 14px;
  font-weight: 600;
}

.current-month {
  color: #60cdff;
}

.current-year {
  color: #fff;
}

.chevron-icon {
  width: 12px;
  height: 12px;
  color: rgba(255, 255, 255, 0.6);
  transition: transform 0.2s ease;
}

.chevron-icon.rotated {
  transform: rotate(180deg);
}

.icon-btn {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0;
}

.icon-btn svg {
  width: 18px;
  height: 18px;
}

/* 日历主体 */
.calendar-body {
  flex: 1;
  padding: 0 16px;
  overflow: hidden;
}

/* 日视图 */
.days-view {
  height: 100%;
  display: flex;
  flex-direction: column;
}

.weekday-header {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  margin-bottom: 4px;
}

.weekday-cell {
  text-align: center;
  padding: 8px 0;
  font-size: 12px;
  font-weight: 600;
  color: rgba(255, 255, 255, 0.6);
}

.days-grid {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  grid-template-rows: repeat(6, 1fr);
  gap: 2px;
  flex: 1;
}

.day-cell {
  position: relative;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.15s ease;
  color: #fff;
  font-size: 14px;
  padding: 4px;
}

.day-cell:hover {
  background-color: rgba(255, 255, 255, 0.08);
}

.day-cell:active {
  background-color: rgba(255, 255, 255, 0.04);
  transform: scale(0.95);
}

.day-cell.other-month {
  color: rgba(255, 255, 255, 0.35);
}

.day-cell.weekend:not(.other-month) {
  color: rgba(255, 255, 255, 0.7);
}

.day-cell.today {
  background-color: #60cdff;
  color: #000;
}

.day-cell.today:hover {
  background-color: #7dd5ff;
}

.day-cell.selected:not(.today) {
  background-color: rgba(96, 205, 255, 0.2);
  outline: 2px solid #60cdff;
  outline-offset: -2px;
}

.day-number {
  font-weight: 400;
  line-height: 1;
}

.today-indicator {
  position: absolute;
  bottom: 4px;
  width: 4px;
  height: 4px;
  border-radius: 50%;
  background-color: #000;
}

/* 月视图 */
.months-view {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  grid-template-rows: repeat(3, 1fr);
  gap: 8px;
  height: 100%;
  padding: 8px 0;
}

.month-cell {
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.15s ease;
  color: #fff;
  font-size: 14px;
  font-weight: 400;
}

.month-cell:hover {
  background-color: rgba(255, 255, 255, 0.08);
}

.month-cell:active {
  background-color: rgba(255, 255, 255, 0.04);
  transform: scale(0.95);
}

.month-cell.current {
  background-color: #60cdff;
  color: #000;
}

.month-cell.current:hover {
  background-color: #7dd5ff;
}

.month-cell.selected:not(.current) {
  background-color: rgba(96, 205, 255, 0.2);
  outline: 2px solid #60cdff;
  outline-offset: -2px;
}

/* 年视图 */
.years-view {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  grid-template-rows: repeat(3, 1fr);
  gap: 8px;
  height: 100%;
  padding: 8px 0;
}

.year-cell {
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.15s ease;
  color: #fff;
  font-size: 14px;
  font-weight: 400;
}

.year-cell:hover {
  background-color: rgba(255, 255, 255, 0.08);
}

.year-cell:active {
  background-color: rgba(255, 255, 255, 0.04);
  transform: scale(0.95);
}

.year-cell.current {
  background-color: #60cdff;
  color: #000;
}

.year-cell.current:hover {
  background-color: #7dd5ff;
}

.year-cell.selected:not(.current) {
  background-color: rgba(96, 205, 255, 0.2);
  outline: 2px solid #60cdff;
  outline-offset: -2px;
}

/* 底部信息区域 */
.calendar-footer {
  padding: 12px 16px 16px 16px;
  border-top: 1px solid rgba(255, 255, 255, 0.08);
  background: rgba(0, 0, 0, 0.2);
}

.selected-date-info {
  display: flex;
  align-items: center;
  gap: 12px;
}

.date-display {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.day-name {
  font-size: 14px;
  font-weight: 600;
  color: #60cdff;
}

.full-date {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.7);
}

/* 过渡动画 */
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.15s ease, transform 0.15s ease;
}

.fade-enter-from {
  opacity: 0;
  transform: scale(0.98);
}

.fade-leave-to {
  opacity: 0;
  transform: scale(0.98);
}
</style>
