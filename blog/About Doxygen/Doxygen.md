# vscode 配置doxygen注释和snippet 

Doxygen的C/C++注释原则

```
基于Doxygen的C/C++注释原则
标注总述
1.文件头标注
2. 命名空间标注
3. 类、结构、枚举标注
4. 函数注释原则
5. 变量注释
6. 模块标注
7. 分组标注
```

# 指令表格

| 命令        | 字段名                                   | 语法                                                      |
| :---------- | :--------------------------------------- | :-------------------------------------------------------- |
| @file       | 文件名                                   | file [< name >]                                           |
| @brief      | 简介                                     | brief { brief description }                               |
| @author     | 作者                                     | author { list of authors }                                |
| @mainpage   | 主页信息                                 | mainpage [(title)]                                        |
| @date       | 年-月-日                                 | date { date description }                                 |
| @author     | 版本号                                   | version { version number }                                |
| @copyright  | 版权                                     | copyright { copyright description }                       |
| @param      | 参数                                     | param [(dir)] < parameter-name> { parameter description } |
| @return     | 返回                                     | return { description of the return value }                |
| @retval     | 返回值                                   | retval  { description }                                   |
| @bug        | 漏洞                                     | bug { bug description }                                   |
| @details    | 细节                                     | details { detailed description }                          |
| @pre        | 前提条件                                 | pre { description of the precondition }                   |
| @see        | 参考                                     | see { references }                                        |
| @link       | 连接(与@see类库，{@link www.google.com}) | link < link-object>                                       |
| @sa         | 参考资料                                 | @sa  < link-object>                                       |
| @throw      | 异常描述                                 | throw < exception-object> { exception description }       |
| @todo       | 待处理                                   | todo { paragraph describing what is to be done }          |
| @warning    | 警告信息                                 | warning { warning message }                               |
| @deprecated | 弃用说明。可用于描述替代方案，预期寿命等 | deprecated { description }                                |
| @example    | 弃用说明。可用于描述替代方案，预期寿命等 | deprecated { description }                                |

~~~bas
@var、@enum、@struct、@class 对变量、枚举、结构体、类等进行标注
~~~



以下是常用的

![image-20250509155113680](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509155113680.png)



# 文件注释

~~~c
/**
 * @file 文件名
 * @brief 简介
 * @details 细节
 * @author 作者
 * @version 版本号
 * @date 年-月-日
 * @copyright 版权
 */
~~~

example:

![image-20250509151935308](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509151935308.png)

还有一种是这样 

~~~c
/*****************************************************************************
*  heschat                                                        
*  Copyright (C) 2025 hesphoros <hesphoros@gmail.con>
*                                                                           
*  This file is part of heschat.                                  
*                                                                           
*  This program is free software; you can redistribute it and/or modify    
*  it under the terms of the GNU General Public License version 3 as       
*  published by the Free Software Foundation.                              
*                                                                           
*  You should have received a copy of the GNU General Public License       
*  along with this program. If not, see <http://www.gnu.org/licenses/>.    
*                                                                           
*  @file     Border.cpp                                                
*  @brief    Border class implementation file                                                  
*  @details  None                                                 
*                                                                           
*  @author   hesphoros                                                
*  @email    hesphoros@gmail.con                                   
*  @version  1.0.0.1                                                       
*  @date     2025/05/09             
*  @license  GNU General Public License (GPL)                              
*---------------------------------------------------------------------------*
*  Remark         : 说明备注                                          
*---------------------------------------------------------------------------*
*  Change History :                                                        
*  <Date>     | <Version> | <Author>       | <Description>                 
*  2025/05/09 | 1.0.0.1   | hesphoros      | Create file           
*****************************************************************************/
~~~



![image-20250509153654211](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509153654211.png)

# 结构体注释

~~~c
/**
 * @brief 结构体简介
 * @details 
 * @note 
 */
~~~



![image-20250509152418483](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509152418483.png)

也可以这样

![image-20250509153749465](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509153749465.png)

# 函数注释

~~~c
 /**
     * @brief  函数描述
     * @param  参数描述
     * @return 返回描述
     * @retval 返回值描述
    */
~~~

example:

![image-20250509153015784](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509153015784.png)

# 常量/变量注释

一般常量/变量可以有两种形式：

- 常量/变量上一行注释
- 常量/变量后注释

~~~c
/**
* @brief description
*/
int val;

//desription
int val;

int a; /*!< 定义一个整型变量a */
int a; /**< 定义一个整型变量a */
int a; //!< 定义一个整型变量a
int a; ///< 定义一个整型变量a

~~~

# 模块标注

~~~c
/**
* @defgroup 模块名 
* @{ 
*/

/** @} */
            
~~~



# 分组标注

~~~c
/**
* @name 分组说明文字
* @{
*/
 content
/** @} */

~~~



# vscode  插件配置

在vscode中搜索 *Doxygen Documentation Generator* ,F1 settings(user) 进入配置

![image-20250509153934514](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509153934514.png)

![image-20250509154529686](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509154529686.png)

下面的自己注意修改一下字段

~~~json
"doxdocgen.generic.authorEmail": "hesphoros@gmail.com",
"doxdocgen.generic.authorName": "hesphoros",
~~~



~~~json
{
     // Doxygen documentation generator set
     "doxdocgen.c.triggerSequence": "/",   // 触发自动注释的生成
     "doxdocgen.c.commentPrefix": " * ",     // 注释行的前缀
     "doxdocgen.c.firstLine": "/**",         // 注释行的首行
     "doxdocgen.c.lastLine": "*/",          // 注释行的尾行
 
     // Smart text snippet for factory methods/functions.
     "doxdocgen.c.factoryMethodText": "Create a {name} object",
     // Smart text snippet for getters.
     "doxdocgen.c.getterText": "Get the {name} object",
     // Smart text snippet for setters.
     "doxdocgen.c.setterText": "Set the {name} object",
      // Smart text snippet for constructors.
      "doxdocgen.cpp.ctorText": "Construct a new {name} object",
       // Smart text snippet for destructors.
     "doxdocgen.cpp.dtorText": "Destroy the {name} object",
     // The template of the template parameter Doxygen line(s) that are generated. If empty it won't get generated at all.
     "doxdocgen.cpp.tparamTemplate": "@tparam {param} ",
 
     // 文件注释：版权信息模板
     "doxdocgen.file.copyrightTag": [
         "@copyright Copyright (c) {year}  hesphoros",
     ],
    //  文件注释：自定义模块，这里我添加一个修改日志
     "doxdocgen.file.customTag": [
         "@par 修改日志:",
         "<table>",
         "<tr><th>Date       <th>Version <th>Author  <th>Description",
         "<tr><td>{date} <td>1.0.1     <td>zhoulq     <td>内容",
         "</table>",
     ],
     // 文件注释的组成及其排序
     "doxdocgen.file.fileOrder": [
         "file",     // @file
         "brief",    // @brief 简介
         "author",   // 作者
         "version",  // 版本
         "date",     // 日期
         "copyright",// 版权
         "empty",
         "custom"    // 自定义
     ],
     // 下面时设置上面标签tag的具体信息
     "doxdocgen.file.fileTemplate": "@file {name}",
     "doxdocgen.file.versionTag": "@version 1.0.1",
     "doxdocgen.generic.authorEmail": "hesphoros@gmail.com",
     "doxdocgen.generic.authorName": "hesphoros",
     "doxdocgen.generic.authorTag": "@author {author} ({email})",
     // 日期格式与模板
     "doxdocgen.generic.dateFormat": "YYYY-MM-DD",
     "doxdocgen.generic.dateTemplate": "@date {date}",
     
     // 根据自动生成的注释模板（目前主要体现在函数注释上）
     "doxdocgen.generic.order": [
         "brief",
         "tparam",
         "param",
         "return",
         "retval"
     ],
     "doxdocgen.generic.paramTemplate": "@param{indent:8}{param}{indent:8}",
     "doxdocgen.generic.returnTemplate": "@return {type} ",
     "doxdocgen.generic.splitCasingSmartText": true,
 
     "doxdocgen.generic.includeTypeAtReturn": true,      // return 中包含类型信息
     "doxdocgen.generic.boolReturnsTrueFalse": false,    // bool 返回值拆分成 true 和 false 两种情况
     "doxdocgen.generic.linesToGet": 20,                  // 回车后最多向下多少行去找函数声明
     "doxdocgen.generic.useGitUserName": false,          // {author} 是都根据 git config --get user.name 替换
     "doxdocgen.generic.useGitUserEmail": false,
     //declarations or definitions anymore.

}
~~~

# vscode snippet 

F1 snippet 

![image-20250509155249533](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509155249533.png)

以下是我的配置

![image-20250509155451935](https://cdn.jsdelivr.net/gh/hesphoros/blogimages@main/img/image-20250509155451935.png)

~~~json
{
  "File Header (Doxygen Style)": {
    "prefix": "fileheader",
    "body": [
      "/*****************************************************************************",
      "*  ${1:Project Name}                                                        ",
      "*  Copyright (C) ${CURRENT_YEAR} ${2:Your Name} <${3:your.email@example.com}>",
      "*                                                                           ",
      "*  This file is part of ${1:Project Name}.                                  ",
      "*                                                                           ",
      "*  This program is free software; you can redistribute it and/or modify    ",
      "*  it under the terms of the GNU General Public License version 3 as       ",
      "*  published by the Free Software Foundation.                              ",
      "*                                                                           ",
      "*  You should have received a copy of the GNU General Public License       ",
      "*  along with this program. If not, see <http://www.gnu.org/licenses/>.    ",
      "*                                                                           ",
      "*  @file     ${TM_FILENAME}                                                ",
      "*  @brief    ${4:简要说明}                                                  ",
      "*  @details  ${5:详细描述}                                                 ",
      "*                                                                           ",
      "*  @author   ${2:Your Name}                                                ",
      "*  @email    ${3:your.email@example.com}                                   ",
      "*  @version  1.0.0.1                                                       ",
      "*  @date     ${CURRENT_YEAR}/${CURRENT_MONTH}/${CURRENT_DATE}             ",
      "*  @license  GNU General Public License (GPL)                              ",
      "*---------------------------------------------------------------------------*",
      "*  Remark         : ${6:说明备注}                                          ",
      "*---------------------------------------------------------------------------*",
      "*  Change History :                                                        ",
      "*  <Date>     | <Version> | <Author>       | <Description>                 ",
      "*  ${CURRENT_YEAR}/${CURRENT_MONTH}/${CURRENT_DATE} | 1.0.0.1   | ${2:Your Name}      | Create file           ",
      "*****************************************************************************/"
    ],
    "description": "Insert file header comment in Doxygen format"
  },
  "Section Divider": {
    "prefix": "divider",
    "body": [
      "//---------------------------------------------------------------------------",
      "// ${1:Section Name}",
      "//---------------------------------------------------------------------------"
    ],
    "description": "Insert section divider comment"
  }
}

~~~



