# fBm 水面参数敏感性实验

本项目实现了一个基于 OpenGL 的最小化水面渲染程序，用于定量分析 fBm（分形布朗运动）中 **octave 数、lacunarity、persistence** 三个核心参数对水面波形视觉效果与计算耗时的影响。  
实验数据用于支撑论文《实验一：fBm参数敏感性分析》。

## 环境依赖

- CMake >= 3.16
- Visual Studio 2022（或支持 C++17 的编译器）
- OpenGL
- GLFW（项目通过子模块或第三方库引入，见 `third_party/glfw`）
- GLAD（OpenGL 加载器，位于 `third_party/glad`）
- GLM（数学库，头文件）
- stb_image_write.h（截图保存，已包含在 `third_party/stb/include/STB_IMAGE/`）

## 构建与运行

### 1. 配置 CMake
在项目根目录下打开终端，执行：
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
