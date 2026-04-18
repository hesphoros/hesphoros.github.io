首先你得拥有如下文件
~~~bash
Lncf_GuiCtrlLibrary.rar
	- Lncf_QtCommonLibs
	- Lncf_QtSocialLibs
LightCone_Soical.rar
~~~

构建环境如下
~~~bash
V145 for Microsoft C++ Build Tools
Qt5.15.2
MSVC 2019 64-bit
Visual Studio 2019以上版本
	安装QT VS Tools扩展
QtCreater
10Gb 物理空间
32G RAM 
Windows SDK版本: 最新即可
注:
推荐构建为Release x64(默认x86)
~~~

构建顺序如下
1. Lncf_QtCommon
2. Lncf_QtSocialLibs
3. LightCone_Soical
# Lncf_QtCommon
输出产物:
~~~bash
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtCommonLibs.dll
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtCommonLibs.lib
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtCommonLibs.exp
注:
构建x64产物需要在配置管理器中自行配置
以上输出库为Lncf_QtSocialLibs LightCone_Soical依赖
~~~
# Lncf_QtSocialLibs

请配置:
~~~bash
Lncf_QtCommonLibs.lib 库目录
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\
Release x64
~~~
输出产物:
~~~bash
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtSocialLibs.dll
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtSocialLibs.lib
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\Lncf_QtSocialLibs.exp
~~~
# LightCone_Soical
请配置:
~~~bash
Release x64
外部包含目录:
...\Lncf_GuiCtrlLibrary\Lncf_QtSocialLibs
...\Lncf_GuiCtrlLibrary\Lncf_QtCommonLibs
库目录:
...\Lncf_QtLibraryBin\Lncf_QtBuild_Release_X64\
~~~
以上配置需要配置LightCone_Soical的所有子项目

另外Lcs_EmojiLibs需要如下额外配置:
~~~bash
/utf-8 -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding -Zc:__cplusplus -w34100 -w34189 -w44996 -w44456 -w44457 -w44458 %(AdditionalOptions)
~~~

以上构建完毕
进入打包流程即可,构建的dll也别忘了打包依赖的Qt dll (Lcs_FrameWork Lcs_CommonUI等)