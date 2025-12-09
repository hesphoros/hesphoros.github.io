# c++使用dlib + OpenCV 实现人脸识别 编译debug

----

Development  enviromnmen:

~~~b
vs2022 
cmake version 3.31.4
~~~



# 编译Dlib

我们clone dlib的仓库

~~~bash
git clone https://github.com/davisking/dlib.git
~~~

然后在该目录执行

~~~bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

~~~

![image-20250515204350836](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515204350836.png)

![image-20250515204425844](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515204425844.png)

![image-20250515204442449](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515204442449.png)

最后使用vs打开

![image-20250515211940853](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515211940853.png)

生成这个就好了 



![image-20250515212027762](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515212027762.png)

![image-20250515212046940](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515212046940.png)

然后你编译好的文件就在这里了

![image-20250515212644749](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515212644749.png)

然后找到这个目录把他复制到你的3rdparty的include里面，

![image-20250515213331353](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515213331353.png)

*替换config,h文件* 在你的build 目录里又一个conifg，你需要把它复制到dlib里面，替换原有的config.h

![image-20250515213458575](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250515213458575.png)

这样就没问题了 （opencv 环境请自行配置

No finished to update