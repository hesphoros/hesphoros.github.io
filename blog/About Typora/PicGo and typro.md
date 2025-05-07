# github+ Picgo

本文将介绍如何使用Picgo在typora中实现上传服务

# 创建github仓库以及配置token

# 创建仓库

![image-20250507222243951](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222243951.png)

- 注意需要Initialize 
  - 添加README
- 配置为public

# 配置token

github点击头像找到setting



![image-20250507222435120](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222435120.png)

选择Developer setting

![image-20250507222523570](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222523570.png)

配置token

![image-20250507222537970](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222537970.png)

generate

![image-20250507222710953](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222710953.png)

选第一个第二个都行(我这里选第二个)

![image-20250507222752456](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222752456.png)

如上配置

![image-20250507222820353](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507222820353.png)

然后复制你的token就行



# 配置PicGo

像这样

![image-20250507223049986](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507223049986.png)

设定自定义域名：https://cdn.jsdelivr.net/gh/[github用户名]/[仓库名]@main，注意，此处的分支一定要填写@main，否则默认使用master分支。

# 集成typora

打开typora

![image-20250507223247596](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507223247596.png)

配置如下

![image-20250507223318070](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507223318070.png)

测试通过 

![image-20250507223352353](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250507223352353.png)