<template>
  <Window :uuid='uuid' :startpos_x="startpos_x" :startpos_y="startpos_y" :zindex="zindex"  :minimized="minimized">
    <template v-slot:header>
      <div class="tw-flex tw-items-center tw-select-none" style="pointer-events:none;"> 
        <img src="../../assets/images/icons/settings.png" alt="" style="pointer-events:auto;" class=" tw-w-7 tw-h-7 tw-ml-4">
        <div class=" tw-ml-4 tw-font-bold tw-tracking-wider" style="pointer-events:auto;"> About Hesphoros </div>
      </div>
    </template>
    <template v-slot:content>
      <div class=" tw-w-full  tw-h-full tw-rounded-b-2xl tw-overflow-hidden tw-flex
       tw-select-none" @contextmenu.prevent="mr_clicked">
        <div class=" tw-w-52 tw-bg-white tw-h-full tw-flex tw-flex-col tw-px-3 tw-py-2 tw-flex-none" >
          <WindowSettingIcon :tag="'Profile'" :img="'profile'" :selected_tag="selected_tag" @click.native="{selected_tag = 'Profile';selected_tag_2 = 'About Me'}"/>
          <WindowSettingIcon :tag="'Skills'" :img="'skills'" :selected_tag="selected_tag"  @click.native="{selected_tag = 'Skills';selected_tag_2 = 'Badges'}"/>
          <WindowSettingIcon :tag="'Wallpaper'" :appIcon="'wallpapers'" :selected_tag="selected_tag"  @click.native="{selected_tag = 'Wallpaper';selected_tag_2 = 'Video'}"/>
          <WindowSettingIcon :tag="'Resume'" :img="'paint'" :selected_tag="selected_tag"  @click.native="selected_tag = 'Resume'"/>
        </div>
        <div class=" tw-flex-grow tw-h-full  tw-p-2">
          <div class="tw-w-full tw-h-full  tw-flex">
            <div class="tw-w-48 tw-flex-none tw-rounded-xl tw-bg-white tw-p-2 tw-overflow-hidden">
              <div class="tw-w-full" v-show="selected_tag === 'Profile'">
                <WindowSettingIcon :tag="'About Me'" :mdi="'beaker-question'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'About Me'"/>
                <WindowSettingIcon :tag="'Github Stats'" :mdi="'card-account-details-star'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'Github Stats'"/>
                <WindowSettingIcon :tag="'CodeWars'" :mdi="'pistol'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'CodeWars'"/>
              </div>
              <div class="tw-w-full" v-show="selected_tag === 'Skills'">
                <WindowSettingIcon :tag="'Badges'" :mdi="'shield-half-full'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'Badges'"/>
                <WindowSettingIcon :tag="'And Some Else'" :mdi="'card-account-details-star'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'And Some Else'"/>
              </div>
              <div class="tw-w-full" v-show="selected_tag === 'Wallpaper'">
                <WindowSettingIcon :tag="'Video'" :mdi="'video'" :selected_tag="selected_tag_2"  @click.native="selected_tag_2 = 'Video'"/>
              </div>
              <div class="tw-w-full" v-show="selected_tag === 'Resume'">
              </div>
            </div>
            <div class="vl"></div>
            <div class="tw-flex-grow tw-bg-white tw-h-full tw-overflow-hidden tw-relative">
              <div class=" tw-w-full tw-h-full tw-absolute tw-inset-0" v-show="selected_tag === 'Profile'">
                <div ref="overall_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col" style="text-align:center" v-if="selected_tag_2 ==='About Me'">
                  <div class=" tw-w-20 tw-h-20 tw-rounded-full tw-overflow-hidden tw-mt-16">
                    <img src="../../assets/images/head2.png" alt="" class="tw-bg-red-500">
                  </div>
                  <div class="tw-mt-3 tw-text-gray-400">hesphoros</div>
                  <div class="tw-text-xl tw-mt-2 tw-tracking-wide"> Hi👋, this is hesphoros</div>
                  <div class="tw-text-lg tw-mt-2 tw-tracking-wide tw-text-left">
                    🎓 A high school student<br>
                    💻 Passionate about open-source projects.<br>                    
                    ⚙️ Skilled in C++, networking, Qt, Server, Python....<br>                  
                    🤝 Strong team player with real-world dev experience.<br>
                    🚀 Aspiring to become a senior systems architect.<br>
                    🌱 Currently learning about AI and machine learning.<br>
                    💬 Open to collaboration and knowledge sharing.<br>
                  </div>

                </div>
                <div ref="github_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col tw-justify-center" style="text-align:center" v-if="selected_tag_2 ==='Github Stats'">
                  <img src="https://github-readme-stats.vercel.app/api?username=hesphoros&show_icons=true&line_height=24" alt="" class="tw-w-140 tw-h-40">
                  <img src="https://github-readme-stats.vercel.app/api/top-langs?username=hesphoros" alt="" class="tw-mt-6  tw-w-120 tw-h-72">
                </div>
                <div ref="github_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col" style="text-align:center" v-if="selected_tag_2 ==='CodeWars'">
                  <div class=" tw-w-20 tw-h-20 tw-rounded-full tw-bg-red-50 tw-overflow-hidden tw-mt-16">
                    <img src="../../assets/images/head2.png" alt="" class="tw-bg-red-500">
                  </div>
                  <img src="https://www.codewars.com/users/hesphoros/badges/large" alt="" class="tw-mt-10">
                </div>
              </div>
              <div class="tw-w-full tw-h-full tw-absolute tw-inset-0" v-show="selected_tag === 'Skills'">
                <div ref="overall_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col tw-justify-center" style="text-align:center" v-if="selected_tag_2 ==='Badges'">
                  <div class="tw-text-xl tw-mt-2 tw-tracking-wide"> Frameworks </div>
                  <div class="tw-flex tw-flex-wrap tw-px-4 tw-py-2 tw-items-center">                    
                    <img src="https://img.shields.io/badge/fastapi%20-%2313988a.svg?&style=flat&logo=data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+CjxzdmcKICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICB4bWxuczpjYz0iaHR0cDovL2NyZWF0aXZlY29tbW9ucy5vcmcvbnMjIgogICB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiCiAgIHhtbG5zOnN2Zz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciCiAgIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIKICAgaWQ9InN2ZzgiCiAgIHZlcnNpb249IjEuMSIKICAgdmlld0JveD0iMCAwIDYuMzQ5OTk5OSA2LjM0OTk5OTkiCiAgIGhlaWdodD0iNi4zNDk5OTk5bW0iCiAgIHdpZHRoPSI2LjM0OTk5OTltbSI+CiAgPGRlZnMKICAgICBpZD0iZGVmczIiIC8+CiAgPG1ldGFkYXRhCiAgICAgaWQ9Im1ldGFkYXRhNSI+CiAgICA8cmRmOlJERj4KICAgICAgPGNjOldvcmsKICAgICAgICAgcmRmOmFib3V0PSIiPgogICAgICAgIDxkYzpmb3JtYXQ+aW1hZ2Uvc3ZnK3htbDwvZGM6Zm9ybWF0PgogICAgICAgIDxkYzp0eXBlCiAgICAgICAgICAgcmRmOnJlc291cmNlPSJodHRwOi8vcHVybC5vcmcvZGMvZGNtaXR5cGUvU3RpbGxJbWFnZSIgLz4KICAgICAgICA8ZGM6dGl0bGU+PC9kYzp0aXRsZT4KICAgICAgPC9jYzpXb3JrPgogICAgPC9yZGY6UkRGPgogIDwvbWV0YWRhdGE+CiAgPGcKICAgICB0cmFuc2Zvcm09InRyYW5zbGF0ZSgtODcuNTM5Mjg2LC04NC40MjYxOTEpIgogICAgIGlkPSJsYXllcjEiPgogICAgPHBhdGgKICAgICAgIGlkPSJwYXRoODE1IgogICAgICAgZD0ibSA4Ny41MzkyODYsODQuNDI2MTkxIGggNi4zNSB2IDYuMzUgaCAtNi4zNSB6IgogICAgICAgc3R5bGU9ImZpbGw6bm9uZTtzdHJva2Utd2lkdGg6MC4yNjQ1ODMzMiIgLz4KICAgIDxwYXRoCiAgICAgICBzdHlsZT0ic3Ryb2tlLXdpZHRoOjAuMjY0NTgzMzI7ZmlsbDojZmZmZmZmIgogICAgICAgaWQ9InBhdGg4MTciCiAgICAgICBkPSJtIDkwLjcxNDI4Niw4NC45NjA2NDkgYyAtMS40NTc4NTQsMCAtMi42NDA1NDIsMS4xODI2ODggLTIuNjQwNTQyLDIuNjQwNTQyIDAsMS40NTc4NTQgMS4xODI2ODgsMi42NDA1NDIgMi42NDA1NDIsMi42NDA1NDIgMS40NTc4NTQsMCAyLjY0MDU0MiwtMS4xODI2ODggMi42NDA1NDIsLTIuNjQwNTQyIDAsLTEuNDU3ODU0IC0xLjE4MjY4OCwtMi42NDA1NDIgLTIuNjQwNTQyLC0yLjY0MDU0MiB6IG0gLTAuMTM3NTgzLDQuNzU3MjA5IHYgLTEuNjU2MjkyIGggLTAuOTIwNzUgbCAxLjMyMjkxNiwtMi41NzcwNDIgdiAxLjY1NjI5MiBoIDAuODg2MzU0IHoiIC8+CiAgPC9nPgo8L3N2Zz4K" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/vuejs%20-%2335495e.svg?&style=flat&logo=vue.js&logoColor=%234FC08D" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/Flask%20-%23000.svg?&style=flat&logo=flask&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/Qt%20-%232E8BC0.svg?&style=flat&logo=Qt&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">                    
                    <img src="https://img.shields.io/badge/spring%20-%236DB33F.svg?&style=flat&logo=spring&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/Node.js%20-%2343853D.svg?&style=flat&logo=Node.js&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/Express.js%20-%23404d59.svg?&style=flat&logo=express&logoColor=%2361DAFB" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/TensorFlow%20-%23FF6F00.svg?&style=flat&logo=TensorFlow&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/numpy%20-%23013243.svg?&style=flat&logo=numpy&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/pandas%20-%23150458.svg?&style=flat&logo=pandas&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="https://img.shields.io/badge/scikit-learn%20-%23F7931E.svg?&style=flat&logo=scikit-learn&logoColor=white" alt="" class="tw-ml-3 tw-mt-2">

                  </div>
                  <div class="tw-text-xl tw-mt-2 tw-tracking-wide"> Tools </div>
                  <div class="tw-flex tw-flex-wrap tw-px-4 tw-py-2  tw-items-center">
                    <img src="https://img.shields.io/badge/-Redis-black?style=flat-square&logo=Redis" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-PostgreSQL-black?style=flat-square&logo=PostgreSQL" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-MongoDB-black?style=flat-square&logo=MongoDB" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">                    
                    <img src="https://img.shields.io/badge/-SQLite-black?style=flat-square&logo=SQLite" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/mysql-%2300f.svg?&style=flat&logo=mysql&logoColor=white" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/oracle%20-%23F00000.svg?&style=flat&logo=oracle&logoColor=white" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Celery-black?style=flat-square&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAB/ElEQVR4Ae2UA4xdQRSGbxo2Zh2niG2W0dq2FdXbuLbdtW3btm3b/nveNLOMM/V8yXf53j2jfxTJH4dEIknqsrtC3iO9RJncZe8V027iFddh/i1v8MbX5pkvXt2LIa87FwJO7S58mPxKQqTJ3faIaTNm13VTr9G7FAYqjs4FfzIglxVP7nE4To6TEGlKjyNiOkyR1uOCnqVQDK3Go2shUFV4W9aAhHbr3vh2ayQINqbFDPFtVqy3AyvR6Jj354XnyFJSQ4msM3YJLtFGcJkOQoSqi295F5DffQejm0noWNgufkPZTVilfqnqD6HluggRaFCpFqmJxqmPGFiN5sXfKPsJrdBbJiHawFJ1RFabsGHvWQ7jDTh3oAGJnbZLbMUKNq7dAqk9zmz+e5ZCeAPOHGgAxaSMhGgTu2yYLbNf0bccwRvw6ODG023vofpDiugIkjEdxqiafIzhtYTdCXBSdhPbZH4ovNpwIqzKABE1hqBrYQZXaLGzKvuDazHUCD/eiHGymLzwoxHNFmeCS7VXg0q0eITERLFcD94FlxFbZ4WBtWiMbCSgc/9GxIlqMDkaVmkQTRESmgb6JnwKryC8ygh142/RvxqJ7qVgvhVXKvuJqDEyDKvSf05/9hKnvhc1wiuwVMMrq/OaV8vMN6/upZB3NBpnld+NRCKRfAftOqATvvOv0AAAAABJRU5ErkJggg==" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-ElasticSearch-005571?style=flat&logo=elasticsearch" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/travisci%20-%232B2F33.svg?&style=flat&logo=travis&logoColor=white" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/docker%20-%230db7ed.svg?&style=flat&logo=docker&logoColor=white" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/nginx%20-%23009639.svg?&style=flate&logo=nginx&logoColor=white" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="" alt="" class="tw-ml-3 tw-mt-2">
                    <img src="" alt="" class="tw-ml-3 tw-mt-2">
                  </div>
                  <div class="tw-text-xl tw-mt-2 tw-tracking-wide"> Others </div>
                  <div class="tw-flex tw-flex-wrap tw-px-4 tw-py-2 tw-mb-16  tw-items-center">
                    <img src="https://img.shields.io/badge/-Linux-black?style=flat-square&logo=Linux" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Debian-007CFF?style=flat-square&logo=debian" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Centos-262577?style=flat-square&logo=Centos" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Raspberry%20Pi-C51A4A?style=flat-square&logo=Raspberry-Pi" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Arduino-00979D?style=flat-square&logo=Arduino" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Git-black?style=flat-square&logo=Git" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">                    
                    <img src="https://img.shields.io/badge/-Gitlab-black?style=flat-square&logo=Gitlab" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Jupyter-black?style=flat-square&logo=Jupyter" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Vim-black?style=flat-square&logo=Vim" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-VSCode-black?style=flat-square&logo=Visual-Studio-Code" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-IntelliJ-black?style=flat-square&logo=IntelliJ-Idea" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Eclipse-black?style=flat-square&logo=Eclipse" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">
                    <img src="https://img.shields.io/badge/-Postman-black?style=flat-square&logo=Postman" alt="" class="tw-ml-3 tw-mt-2 tw-h-5">

                  </div>
                </div>
                <div ref="github_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col tw-justify-center tw-mb-16" style="text-align:center" v-if="selected_tag_2 ==='And Some Else'">
                  <img src="../../assets/images/holo.gif" alt="" class="">
                  <div class="tw-text-lg tw-mt-2 tw-tracking-wide tw-mb-10"> Thanks for watching , hope you enjoy it. </div>
                </div> 
              </div>
              <div class="tw-w-full tw-h-full tw-absolute tw-inset-0" v-show="selected_tag === 'Resume'">
                <div ref="overall_page" class="tw-w-full tw-h-full tw-items-center tw-flex tw-flex-col tw-justify-center" style="text-align:center">
                  <div class="tw-text-4xl tw-mt-2 tw-tracking-wider"> Coming Soon </div>
                </div>
              </div>
              <!-- 壁纸设置页面 -->
              <div class="tw-w-full tw-h-full tw-overflow-auto tw-absolute tw-inset-0" v-show="selected_tag === 'Wallpaper'">
                <div class="tw-w-full tw-p-4" v-if="selected_tag_2 === 'Video'">
                  <div class="tw-text-xl tw-font-bold tw-mb-4">视频壁纸设置</div>
                  
                  <!-- 开关 -->
                  <div class="tw-flex tw-items-center tw-mb-6 tw-p-3 tw-bg-gray-50 tw-rounded-lg">
                    <div class="tw-flex-grow">
                      <div class="tw-font-medium">启用动态壁纸</div>
                      <div class="tw-text-sm tw-text-gray-500">使用视频作为桌面背景</div>
                    </div>
                    <v-switch
                      :input-value="useVideoWallpaper"
                      @change="toggleVideoWallpaper"
                      color="primary"
                      hide-details
                      inset
                      class="tw-mt-0 tw-pt-0"
                    ></v-switch>
                  </div>
                  
                  <!-- 壁纸选择 -->
                  <div class="tw-text-lg tw-font-medium tw-mb-3">选择壁纸</div>
                  <div class="tw-grid tw-grid-cols-3 tw-gap-3">
                    <div 
                      v-for="(wallpaper, index) in videoWallpapers" 
                      :key="index"
                      @click="setWallpaper(index)"
                      class="tw-relative tw-cursor-pointer tw-rounded-lg tw-overflow-hidden tw-border-2 tw-transition-all hover:tw-shadow-lg"
                      :class="currentWallpaperIndex === index ? 'tw-border-blue-500 tw-shadow-md' : 'tw-border-gray-200'"
                    >
                      <video 
                        :src="wallpaper.src" 
                        class="tw-w-full tw-h-20 tw-object-cover tw-bg-gray-100"
                        muted
                        preload="metadata"
                        @mouseenter="$event.target.play()"
                        @mouseleave="$event.target.pause(); $event.target.currentTime = 0"
                        @loadeddata="$event.target.currentTime = 1"
                      ></video>
                      <div class="tw-absolute tw-bottom-0 tw-left-0 tw-right-0 tw-bg-black tw-bg-opacity-50 tw-text-white tw-text-xs tw-p-1 tw-text-center tw-truncate">
                        {{ wallpaper.name }}
                      </div>
                      <div v-if="currentWallpaperIndex === index" class="tw-absolute tw-top-1 tw-right-1 tw-bg-blue-500 tw-rounded-full tw-p-1">
                        <v-icon small color="white">mdi-check</v-icon>
                      </div>
                    </div>
                  </div>
                  
                  <!-- 当前壁纸信息 -->
                  <div class="tw-mt-6 tw-p-3 tw-bg-gray-50 tw-rounded-lg">
                    <div class="tw-text-sm tw-text-gray-500">当前壁纸</div>
                    <div class="tw-font-medium">{{ currentWallpaperName }}</div>
                  </div>
                </div>
              </div>
            </div>
            
          </div>
        </div>
      </div>
    </template>
  </Window>
</template>

<script>
import Window from '../WindowBasic/Window.vue'
import WindowSettingIcon from './WindowSettingIcon.vue'

export default {
  name: 'WindowSettings',
  components: {
    Window,
    WindowSettingIcon,
  },
  data(){
    return {
      selected_tag:"Profile",
      selected_tag_2:"About Me",
      // 视频壁纸列表 (与 Desktop.vue 保持一致)
      // 使用 GitHub Releases 托管视频文件
      videoWallpapers: [
        { name: '海滨公园打伞的澪', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/default.mp4' },
        { name: 'Mona', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/MonaWallpaperFHD.mp4' },
        { name: 'Wallpaper 1', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/wallpaper.mp4' },
        { name: 'Wallpaper 2', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/wallpaper2.mp4' },
        { name: 'Wallpaper 3', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/wallpaper3.mp4' },
        { name: 'Wallpaper 4', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/wallpaper4.mp4' },
        { name: 'Wallpaper 5', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/wallpaper5.mp4' },
        { name: '合成动画', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/1_1.mp4' },
        { name: '16:9', src: 'https://github.com/hesphoros/hesphoros.github.io/releases/download/v1.0-videos/16.9.mp4' },
      ],
    }
  },
  props:{
    uuid:String,
    startpos_x:{
      default:60,
    },
    startpos_y:{
      default:60
    },
    zindex:{
      type:Number,
      default:999,
    },
    minimized:{
      type:Boolean,
      default:false,
    },
    default_width:{
      type:Number,
      default:680
    }
  },
  created(){
  },
  mounted(){
    
  },
  watch:{
  },
  computed:{
    useVideoWallpaper() {
      return this.$store.state.useVideoWallpaper
    },
    currentWallpaperIndex() {
      return this.$store.state.currentWallpaperIndex
    },
    currentWallpaperName() {
      if (this.videoWallpapers.length > 0 && this.currentWallpaperIndex < this.videoWallpapers.length) {
        return this.videoWallpapers[this.currentWallpaperIndex].name
      }
      return ''
    }
  },
  methods:{
    mr_clicked(){
      this.$store.commit('show_context_menu')
    },
    toggleVideoWallpaper() {
      this.$store.commit('toggle_video_wallpaper')
    },
    setWallpaper(index) {
      this.$store.commit('set_wallpaper_index', index)
    }
  }
}
</script>

<style scoped>
.vl {
  border-left: 1.5px solid rgba(244,244,244);
  height: 100%;
  left: 50%;
}
</style>
