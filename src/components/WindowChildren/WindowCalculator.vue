<template>
  <Window
    :uuid="uuid"
    :startpos_x="startpos_x"
    :startpos_y="startpos_y"
    :zindex="zindex"
    :minimized="minimized"
    :default_width="320"
    :default_height="500"
    :fixedsize="true"
    :blacktheme="true"
  >
    <template v-slot:header>
      <div class="header-content">
        <span class="header-title">Calculator</span>
      </div>
    </template>
    <template v-slot:content>
      <div class="calculator">
        <!-- 显示区域 -->
        <div class="display">
          <div class="expression">{{ expression || '0' }}</div>
          <div class="result">{{ displayValue }}</div>
        </div>

        <!-- 功能按钮区 -->
        <div class="memory-row">
          <button class="btn memory" @click="memoryClear" :disabled="!memory">MC</button>
          <button class="btn memory" @click="memoryRecall" :disabled="!memory">MR</button>
          <button class="btn memory" @click="memoryAdd">M+</button>
          <button class="btn memory" @click="memorySubtract">M-</button>
          <button class="btn memory" @click="memoryStore">MS</button>
        </div>

        <!-- 主按钮区 -->
        <div class="buttons-grid">
          <!-- 第一行 -->
          <button class="btn function" @click="handlePercent">%</button>
          <button class="btn function" @click="clearEntry">CE</button>
          <button class="btn function" @click="clear">C</button>
          <button class="btn function" @click="backspace">⌫</button>

          <!-- 第二行 -->
          <button class="btn function" @click="handleReciprocal">1/x</button>
          <button class="btn function" @click="handleSquare">x²</button>
          <button class="btn function" @click="handleSquareRoot">√x</button>
          <button class="btn operator" @click="handleOperator('/')">÷</button>

          <!-- 第三行 -->
          <button class="btn number" @click="handleNumber('7')">7</button>
          <button class="btn number" @click="handleNumber('8')">8</button>
          <button class="btn number" @click="handleNumber('9')">9</button>
          <button class="btn operator" @click="handleOperator('*')">×</button>

          <!-- 第四行 -->
          <button class="btn number" @click="handleNumber('4')">4</button>
          <button class="btn number" @click="handleNumber('5')">5</button>
          <button class="btn number" @click="handleNumber('6')">6</button>
          <button class="btn operator" @click="handleOperator('-')">−</button>

          <!-- 第五行 -->
          <button class="btn number" @click="handleNumber('1')">1</button>
          <button class="btn number" @click="handleNumber('2')">2</button>
          <button class="btn number" @click="handleNumber('3')">3</button>
          <button class="btn operator" @click="handleOperator('+')">+</button>

          <!-- 第六行 -->
          <button class="btn function" @click="handleNegate">±</button>
          <button class="btn number" @click="handleNumber('0')">0</button>
          <button class="btn number" @click="handleDecimal">.</button>
          <button class="btn equals" @click="calculate">=</button>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'

export default {
  name: 'WindowCalculator',
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
    return {
      currentValue: '0',
      previousValue: null,
      operator: null,
      expression: '',
      waitingForOperand: false,
      justCalculated: false,
      memory: 0
    }
  },
  computed: {
    displayValue() {
      // 格式化显示数字，添加千分位分隔符
      const num = parseFloat(this.currentValue)
      if (isNaN(num)) return this.currentValue
      
      // 如果是整数，添加千分位
      if (Number.isInteger(num) && !this.currentValue.includes('.')) {
        return num.toLocaleString('en-US')
      }
      
      // 如果有小数点，保留小数部分
      const parts = this.currentValue.split('.')
      const intPart = parseInt(parts[0]).toLocaleString('en-US')
      if (parts.length > 1) {
        return intPart + '.' + parts[1]
      }
      return intPart
    }
  },
  mounted() {
    // 监听键盘事件
    window.addEventListener('keydown', this.handleKeydown)
  },
  beforeDestroy() {
    window.removeEventListener('keydown', this.handleKeydown)
  },
  methods: {
    handleKeydown(e) {
      // 数字键
      if (e.key >= '0' && e.key <= '9') {
        this.handleNumber(e.key)
      }
      // 运算符
      else if (e.key === '+') this.handleOperator('+')
      else if (e.key === '-') this.handleOperator('-')
      else if (e.key === '*') this.handleOperator('*')
      else if (e.key === '/') this.handleOperator('/')
      // 其他
      else if (e.key === '.') this.handleDecimal()
      else if (e.key === 'Enter' || e.key === '=') this.calculate()
      else if (e.key === 'Escape') this.clear()
      else if (e.key === 'Backspace') this.backspace()
      else if (e.key === '%') this.handlePercent()
    },

    handleNumber(num) {
      if (this.justCalculated) {
        this.currentValue = num
        this.expression = ''
        this.justCalculated = false
        return
      }

      if (this.waitingForOperand) {
        this.currentValue = num
        this.waitingForOperand = false
      } else {
        if (this.currentValue === '0' && num !== '0') {
          this.currentValue = num
        } else if (this.currentValue !== '0') {
          // 限制数字长度
          if (this.currentValue.replace('.', '').length < 16) {
            this.currentValue += num
          }
        }
      }
    },

    handleDecimal() {
      if (this.justCalculated) {
        this.currentValue = '0.'
        this.expression = ''
        this.justCalculated = false
        return
      }

      if (this.waitingForOperand) {
        this.currentValue = '0.'
        this.waitingForOperand = false
        return
      }

      if (!this.currentValue.includes('.')) {
        this.currentValue += '.'
      }
    },

    handleOperator(op) {
      const current = parseFloat(this.currentValue)

      if (this.previousValue === null) {
        this.previousValue = current
      } else if (!this.waitingForOperand) {
        const result = this.performCalculation()
        this.currentValue = String(result)
        this.previousValue = result
      }

      this.operator = op
      this.waitingForOperand = true
      this.justCalculated = false
      
      // 更新表达式显示
      const opSymbol = this.getOperatorSymbol(op)
      this.expression = `${this.previousValue} ${opSymbol}`
    },

    getOperatorSymbol(op) {
      const symbols = {
        '+': '+',
        '-': '−',
        '*': '×',
        '/': '÷'
      }
      return symbols[op] || op
    },

    performCalculation() {
      const prev = this.previousValue
      const current = parseFloat(this.currentValue)

      switch (this.operator) {
        case '+':
          return prev + current
        case '-':
          return prev - current
        case '*':
          return prev * current
        case '/':
          if (current === 0) {
            return 'Error'
          }
          return prev / current
        default:
          return current
      }
    },

    calculate() {
      if (this.operator === null || this.previousValue === null) {
        return
      }

      const current = parseFloat(this.currentValue)
      const opSymbol = this.getOperatorSymbol(this.operator)
      this.expression = `${this.previousValue} ${opSymbol} ${current} =`

      const result = this.performCalculation()
      
      if (result === 'Error') {
        this.currentValue = 'Cannot divide by zero'
      } else {
        // 处理浮点数精度问题
        this.currentValue = String(Math.round(result * 1e12) / 1e12)
      }

      this.previousValue = null
      this.operator = null
      this.waitingForOperand = true
      this.justCalculated = true
    },

    clear() {
      this.currentValue = '0'
      this.previousValue = null
      this.operator = null
      this.expression = ''
      this.waitingForOperand = false
      this.justCalculated = false
    },

    clearEntry() {
      this.currentValue = '0'
      this.waitingForOperand = false
    },

    backspace() {
      if (this.justCalculated || this.waitingForOperand) {
        return
      }
      
      if (this.currentValue.length > 1) {
        this.currentValue = this.currentValue.slice(0, -1)
      } else {
        this.currentValue = '0'
      }
    },

    handlePercent() {
      const current = parseFloat(this.currentValue)
      if (this.previousValue !== null) {
        // 如果有前一个值，计算百分比
        this.currentValue = String((this.previousValue * current) / 100)
      } else {
        this.currentValue = String(current / 100)
      }
    },

    handleNegate() {
      const current = parseFloat(this.currentValue)
      if (current !== 0) {
        this.currentValue = String(-current)
      }
    },

    handleSquare() {
      const current = parseFloat(this.currentValue)
      this.expression = `sqr(${current})`
      this.currentValue = String(current * current)
      this.justCalculated = true
    },

    handleSquareRoot() {
      const current = parseFloat(this.currentValue)
      if (current < 0) {
        this.currentValue = 'Invalid input'
        return
      }
      this.expression = `√(${current})`
      this.currentValue = String(Math.sqrt(current))
      this.justCalculated = true
    },

    handleReciprocal() {
      const current = parseFloat(this.currentValue)
      if (current === 0) {
        this.currentValue = 'Cannot divide by zero'
        return
      }
      this.expression = `1/(${current})`
      this.currentValue = String(1 / current)
      this.justCalculated = true
    },

    // Memory functions
    memoryClear() {
      this.memory = 0
    },

    memoryRecall() {
      if (this.memory !== 0) {
        this.currentValue = String(this.memory)
        this.waitingForOperand = false
      }
    },

    memoryAdd() {
      this.memory += parseFloat(this.currentValue)
    },

    memorySubtract() {
      this.memory -= parseFloat(this.currentValue)
    },

    memoryStore() {
      this.memory = parseFloat(this.currentValue)
    }
  }
}
</script>

<style scoped>
.calculator {
  width: 100%;
  height: 100%;
  background-color: #202020;
  display: flex;
  flex-direction: column;
  padding: 8px;
  box-sizing: border-box;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
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

/* 显示区域 */
.display {
  padding: 8px 8px 16px 8px;
  text-align: right;
  min-height: 90px;
  display: flex;
  flex-direction: column;
  justify-content: flex-end;
}

.expression {
  color: #888;
  font-size: 14px;
  min-height: 20px;
  word-break: break-all;
}

.result {
  color: #fff;
  font-size: 42px;
  font-weight: 300;
  line-height: 1.2;
  word-break: break-all;
  overflow: hidden;
}

/* 内存按钮行 */
.memory-row {
  display: flex;
  gap: 2px;
  margin-bottom: 4px;
}

.memory-row .btn {
  flex: 1;
  height: 32px;
  font-size: 13px;
}

/* 按钮网格 */
.buttons-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 2px;
  flex: 1;
}

/* 按钮基础样式 */
.btn {
  border: none;
  border-radius: 4px;
  font-size: 18px;
  font-weight: 400;
  cursor: pointer;
  transition: background-color 0.1s ease;
  display: flex;
  align-items: center;
  justify-content: center;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.btn:active {
  transform: scale(0.98);
}

.btn:disabled {
  opacity: 0.4;
  cursor: default;
}

.btn:disabled:active {
  transform: none;
}

/* 数字按钮 */
.btn.number {
  background-color: #3b3b3b;
  color: #fff;
}

.btn.number:hover {
  background-color: #4a4a4a;
}

.btn.number:active {
  background-color: #333;
}

/* 运算符按钮 */
.btn.operator {
  background-color: #323232;
  color: #fff;
}

.btn.operator:hover {
  background-color: #404040;
}

.btn.operator:active {
  background-color: #2a2a2a;
}

/* 功能按钮 */
.btn.function {
  background-color: #323232;
  color: #fff;
}

.btn.function:hover {
  background-color: #404040;
}

.btn.function:active {
  background-color: #2a2a2a;
}

/* 内存按钮 */
.btn.memory {
  background-color: #202020;
  color: #888;
}

.btn.memory:hover:not(:disabled) {
  background-color: #2a2a2a;
  color: #fff;
}

/* 等于按钮 */
.btn.equals {
  background-color: #4cc2ff;
  color: #000;
}

.btn.equals:hover {
  background-color: #5fcfff;
}

.btn.equals:active {
  background-color: #3ab5f5;
}
</style>
