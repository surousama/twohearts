# 本机环境记录

## 用途

1. 记录 `palladianli` 当前常用机器上的本地路径与构建入口。
2. 这里只放“开发机私有信息”，不写进全局 `.trellis/spec`，避免污染项目长期规则。
3. 家里电脑与公司电脑可以继续在本文中分节维护。

## 当前环境判断

1. 当前会话对应的是家里电脑。
2. 当前项目工作目录位于：
   `G:\twohearts\twohearts`

## 家里电脑

### Unreal Engine

1. 当前确认可用的 Unreal Engine 根目录：
   `G:\UE_5.6`
2. 当前确认可用的构建脚本：
   `G:\UE_5.6\Engine\Build\BatchFiles\Build.bat`

### Visual Studio / MSBuild

1. 当前确认可用的 `MSBuild.exe`：
   `D:\VS2022\MSBuild\Current\Bin\MSBuild.exe`
2. 当前确认可用的 VS 工具链来自：
   `D:\VS2022`

### 当前项目常用构建命令

1. 直接整编编辑器模块：
```powershell
& 'D:\VS2022\MSBuild\Current\Bin\MSBuild.exe' 'twohearts.sln' /t:Build /p:Configuration="Development Editor" /p:Platform=Win64 /m
```

2. Unreal 原生命令等价入口：
```powershell
& 'G:\UE_5.6\Engine\Build\BatchFiles\Build.bat' twoheartsEditor Win64 Development -Project='G:\twohearts\twohearts\twohearts.uproject' -WaitMutex -FromMsBuild -architecture=x64
```

## 额外说明

1. 旧 journal 中曾出现过 `H:\UE_5.6` 记录；当前这台家里电脑应以 `G:\UE_5.6` 为准。
2. 若后续 Unreal 或 VS 安装盘变化，优先更新本文，而不是让后续任务重新全盘搜索。
3. 公司电脑路径尚未在当前会话核实；等切到公司环境后，再在本文新增“公司电脑”小节。
