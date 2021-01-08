# RDPBlocker

RDPBlocker是一款防止针对RDP(Remote Desktop Protocol)服务进行蛮力密码破解的工具。

由于RDP服务是由微软设计和维护的，其他操作系统很少使用，所以本工具是专门为Windows设计的，不支持其他操作系统。

本工具的优点是使用简单、体积小、安装方便。

## 运行需求
Windows 10 x64 或 Windows Server 2016 x64 以及更高版本。

理论上代码支持Windows 7 以后的所有操作系统，但未经过测试。

默认仅提供 X64版本，如需X86版本需要自行编译。

## 安装
本工具提供了一个由 Inno Setup 打包的安装向导，用户可以直接使用该向导进行安装。
程序将默认安装在 ```C:\Program Files\RDPBlocker```

```C:\Program Files\RDPBlocker\RDPBlocker.exe``` 将被注册为系统服务，作为系统服务，它将在系统启动时自动启动。

注：注册服务功能由 [nssm project](https://nssm.cc/) 提供.

```C:\Program Files\RDPBlocker\config.ini```  是配置文件。
配置文件可以根据用户的需要进行修改。
举例:

```ini
[Block]
; 阻挡阈值
; 当在指定的时间内输入错误的账户或密码超过阈值时，IP地址将被封锁。
threshold = 3
; 以秒为单位的阻挡时间
time = 300
```

如果有需要，也可以下载压缩包自行配置安装。

## 编译
如果你需要的话，也可以尝试自己编译这个项目。

### 构建要求
Visual Studio 2019 (MSVC 14.2)
Boost Library https://www.boost.org
spdlog https://github.com/gabime/spdlog

如果使用本项目提供的工程文件，需要在编译前设置环境变量。
```
$(BOOST_INCLUDE)
$(SPDLOG_INCLUDE)
$(BOOST_LIB)
$(SPDLOG_LIB)
```

如果你需要编译和安装向导，你还需要 Inno setup 编译器。

