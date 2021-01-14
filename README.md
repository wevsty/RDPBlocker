# RDPBlocker

RDPBlocker is a tool to prevent brute force password cracking against RDP (Remote Desktop Protocol) services.

This tool is developed by cpp and designed for Windows, with the advantages of simplicity of use, small size and easy installation.


## Runing Requirements
Windows 10 x64 or Windows Server 2016 x64 and higher.

Note: Theoretically the code supports all operating systems from Windows 7 onwards, but it has not been tested.

Note: Only X64 version is provided by default, you need to compile it yourself if you need X86 version.


## Cautions

1、Requires Windows Firewall to be kept on, otherwise the program will not work properly.

2、If the system is located in a LAN environment, Windows may not record the IP of external visitors correctly, and the situation may not work as expected.

3、This tool cannot protect against security problems caused by system vulnerabilities, so you still need to install system updates.


## Install
This tool provides an installation wizard packaged by Inno Setup, which users can use directly to install.

The program will be installed by default in the ```C:\Program Files\RDPBlocker```

```C:\Program Files\RDPBlocker\RDPBlocker.exe``` will be registered as a system service, and as a system service it will start automatically at system startup.

Note: Registration service function provided by [nssm project](https://nssm.cc/).

```C:\Program Files\RDPBlocker\config.ini```  is the configuration file.
The configuration file can be changed according to the user's needs.
Example :

```ini
[Block]
; Blocking Threshold
; IP addresses will be blocked when an incorrect account or password is entered within a specified period of time greater than a threshold value.
threshold = 3
; Blocking time in the seconds
time = 300

[Whitelist]
; Addresses in the whitelist will not be subject to any restrictions
; The syntax is key = Regular expressions
; Note : Keys cannot be repeated
001 = 127.0.0.1
002 = 192.168.*
```

If users need, they can also download the zip package to configure and install it by themselves.


## Build
You can also try to compile this project yourself if you need to.


### Build Requirements
Visual Studio 2019 (MSVC 14.2)

Boost Library https://www.boost.org

spdlog https://github.com/gabime/spdlog

If you use the project files provided in this project, you need to set the environment variables before compiling.
```
$(BOOST_INCLUDE)
$(SPDLOG_INCLUDE)
$(BOOST_LIB)
$(SPDLOG_LIB)
```

If you need to compile and install the wizard, you will also need Inno setup compilers.
