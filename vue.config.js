const path = require('path')
const fs = require('fs')
const glob = require('glob')

// 自定义插件:复制 drawio 文件
class CopyDrawioPlugin {
  apply(compiler) {
    compiler.hooks.afterEmit.tapAsync('CopyDrawioPlugin', (compilation, callback) => {
      const blogDir = path.resolve(__dirname, 'blog')
      const docsDir = path.resolve(__dirname, 'docs')
      
      // 查找所有 .drawio 和 .dio 文件
      const patterns = ['**/*.drawio', '**/*.dio']
      const allFiles = []
      
      patterns.forEach(pattern => {
        const files = glob.sync(pattern, { cwd: blogDir })
        allFiles.push(...files)
      })
      
      if (allFiles.length === 0) {
        console.log('未找到 drawio 文件')
        callback()
        return
      }
      
      // 复制每个文件
      const promises = allFiles.map(file => {
        return new Promise((resolve, reject) => {
          const src = path.join(blogDir, file)
          const dest = path.join(docsDir, 'blog', file)
          const destDir = path.dirname(dest)
          
          // 递归创建目录
          fs.mkdir(destDir, { recursive: true }, (err) => {
            if (err) return reject(err)
            
            // 复制文件
            fs.copyFile(src, dest, (err) => {
              if (err) return reject(err)
              resolve()
            })
          })
        })
      })
      
      Promise.all(promises)
        .then(() => {
          console.log(`✓ 已复制 ${allFiles.length} 个 drawio 文件到 docs/blog/`)
          callback()
        })
        .catch(error => {
          console.error('✗ 复制 drawio 文件失败:', error)
          callback()
        })
    })
  }
}

module.exports = {
  transpileDependencies: [
    'vuetify'
  ],
  outputDir: './docs',
  devServer:{
    host:'0.0.0.0',
    contentBase: require('path').join(__dirname, './'),
    contentBasePublicPath: '/',
    proxy:{
      "/dev":{
        target:"http://127.0.0.1:8079",
        changeOrigin:true,
        pathRewrite:{
          '^/dev':''
        }
      }
    },
  },
  productionSourceMap: false,
  pwa:{
    iconPaths: {
      favicon32: 'favicon.ico',
      favicon16: 'favicon.ico',
      appleTouchIcon: 'favicon.ico',
      maskIcon: 'favicon.ico',
      msTileImage: 'favicon.ico'
    }
  },
  configureWebpack: {
    plugins: [
      new CopyDrawioPlugin()
    ]
  }
}
