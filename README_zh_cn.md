# RDPBlocker

RDPBlocker是一款防止针对 RDP(Remote Desktop Protocol) 服务进行暴力密码破解的工具。

本工具由 CPP 开发，专为 Windows 设计的，优点是使用简单、体积小、安装方便。


## 运行需求
Windows 10 x64 或 Windows Server 2016 x64 以及更高版本。

注意：理论上代码支持 Windows 7 以后的所有操作系统，但未经过测试。

注意：默认仅提供 X64 版本，如需 X86 版本需要自行编译。


## 注意事项

1、需要 Windows 防火墙要求保持为开启状态，否则无法程序无法正常工作。

2、如果系统位于局域网环境，Windows 可能无法正确记录外部访问者的 IP，这种情况可能无法符合预期的工作。

3、本工具不能防护由系统漏洞导致的安全问题，所以依然需要安装系统更新。


## 安装
本工具提供了一个由 Inno Setup 打包的安装向导，用户可以直接使用该向导进行安装。

程序将默认安装在 ```C:\Program Files\RDPBlocker```

```C:\Program Files\RDPBlocker\RDPBlocker.exe``` 将被注册为系统服务，作为系统服务，它将在系统启动时自动启动。

注：注册服务功能由 [nssm project](https://nssm.cc/) 提供.

```C:\Program Files\RDPBlocker\config.ini```  是配置文件。

配置文件可以根据用户的需要进行修改。

举例:

```ini
[Log]
; 日志文件的文件名
filename = logger.txt
; 日志文件的最大大小为10MB
max_size = 10485760
; 日志文件的最大数量
max_files = 3
; 日志输出等级
; 值可为:
; trace debug info warning error critical off
level = info

[Block]
; 阻挡阈值
; 当在指定的时间内输入错误的账户或密码超过阈值时，IP地址将被封锁。
threshold = 3
; 以秒为单位的阻挡时间
time = 300

[Whitelist]
; 位于 Whitelist 中的地址将不会受到限制
; 语法为 : key = 正则表达式
; 注意 : key 不能重复
001 = 127\.0\.0\.1
; 192.168.0.0-192.168.255.255
002 = ^192\.168.*
; 172.16.0.0-172.31.255.255
003 = ^172\.(1[6-9]|2[0-9]|3[0-1])\..*
; 10.0.0.0-10.255.255.255
004 = ^10\..*
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

