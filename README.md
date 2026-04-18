# biquad-ss

一个用标准 C 语言编写的、高效且易于集成的**双二阶（Biquad）IIR 滤波器**库。

本项目采用独特的**状态空间（State-Space）实现**，提供了与标准直接形式 I（Direct-Form I）在数学上完全等价，但计算结构更清晰、更适合并行化与缓存优化的解决方案。

---

## 特性

- ✅ 纯标准 C11，零外部依赖，可直接嵌入任何项目
- ✅ 支持四种经典滤波器类型：低通（LPF）、高通（HPF）、带通（BPF）、带阻（BSF）
- ✅ 基于 Audio EQ Cookbook 的双线性变换设计，参数直观（截止频率 + Q 值）
- ✅ 状态空间实现，状态更新可并行，缓存访问模式优化
- ✅ 提供 `biquad_cascade.h` 支持任意级数的级联滤波器
- ✅ 提供命令行工具 `biquad_cli`，快速导出系数给 MATLAB/Python 验证

---

## 原理：为什么是状态空间形式？

标准直接形式 I（DF-I）的差分方程为：

```
y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
```

本库将其重写为等价的状态空间递推：

```
s0 += b0 * x[n]
s1 += b1 * x[n]     ← 三路输入同时累积（可并行）
s2 += b2 * x[n]

y = s0

s0 = s1 - a1 * y   ← 状态转移
s1 = s2 - a2 * y
s2 = 0
```

**优势：**
1. 输入馈入（`s += b * x`）三路可并行计算，适合 SIMD 或 DSP 流水线；
2. 状态数组连续内存访问，提高 CPU 缓存命中率；
3. 代码结构清晰，便于向量化编译器优化。

---

## 快速开始

### 只需两个文件

将 `biquad.h` 和 `biquad_design.h` + `biquad_design.c` 复制到你的项目中即可。

若只需要运行滤波器（系数已知），只需 `biquad.h`（纯头文件，无需编译）。

### 示例：设计一个低通滤波器并滤波

```c
#include "biquad_design.h"

int main(void) {
    struct biquad lpf;

    // 采样率 1000 Hz，截止频率 50 Hz，Q = 0.707（Butterworth）
    biquad_design(&lpf, BIQUAD_LPF, 1000.0f, 0.707f, 50.0f);

    float input[] = {1.0f, 0.5f, -0.3f, 0.8f, -1.0f};
    for (int i = 0; i < 5; i++) {
        float output = biquad_filter(&lpf, input[i]);
        printf("input: %6.3f  output: %6.3f\n", input[i], output);
    }
    return 0;
}
```

### 示例：级联两个滤波器（更陡的滚降）

```c
#include "biquad_cascade.h"
#include "biquad_design.h"

int main(void) {
    struct biquad stages[2];
    biquad_design(&stages[0], BIQUAD_LPF, 1000.0f, 0.707f, 50.0f);
    biquad_design(&stages[1], BIQUAD_LPF, 1000.0f, 0.707f, 50.0f);

    struct biquad_cascade cascade;
    biquad_cascade_init(&cascade, stages, 2);

    float y = biquad_cascade_filter(&cascade, 1.0f);
    return 0;
}
```

---

## API 参考

### `biquad.h` — 核心滤波器

#### 结构体

```c
struct biquad {
    float b0, b1, b2; /* 前向（分子）系数 */
    float a1, a2;     /* 反馈（分母）系数，a0 已归一化为 1 */
    float s[3];       /* 内部状态变量 */
};
```

#### `biquad_init`

```c
void biquad_init(struct biquad *iir,
                 float b0, float b1, float b2,
                 float a1, float a2);
```

用给定系数初始化滤波器，并清零状态。

| 参数 | 说明 |
|------|------|
| `iir` | 滤波器实例指针 |
| `b0,b1,b2` | 前向系数（已除以 a0 归一化） |
| `a1,a2` | 反馈系数（已除以 a0 归一化，注意符号） |

#### `biquad_filter`

```c
float biquad_filter(struct biquad *iir, float x);
```

处理单个采样点，返回滤波后的输出。线程不安全（修改内部状态）。

---

### `biquad_design.h` — 自动设计系数

#### `biquad_design`

```c
int biquad_design(struct biquad *iir,
                  biquad_type_t type,
                  float fs, float Q, float fc);
```

根据参数自动计算双线性变换系数并初始化滤波器。

| 参数 | 说明 |
|------|------|
| `iir` | 滤波器实例指针 |
| `type` | 滤波器类型，见下表 |
| `fs` | 采样率（Hz），必须 > 0 |
| `Q` | 品质因数，控制谐振峰/滚降特性，最小值 0.5 |
| `fc` | 截止频率（Hz），必须满足 `0 < fc < fs/2` |

**返回值：** 成功返回 `0`，参数非法返回 `-1`。

**滤波器类型：**

| 枚举值 | 类型 | 说明 |
|--------|------|------|
| `BIQUAD_LPF` | 低通 | 保留低频，抑制高于 fc 的信号 |
| `BIQUAD_HPF` | 高通 | 保留高频，抑制低于 fc 的信号 |
| `BIQUAD_BPF` | 带通 | 保留 fc 附近频段，Q 越大带宽越窄 |
| `BIQUAD_BSF` | 带阻 | 陷波，抑制 fc 附近频段 |

**Q 值参考：**

| Q 值 | 特性 |
|------|------|
| 0.707 | Butterworth（最平坦幅频响应，无峰值） |
| 0.577 | Bessel（最线性相位） |
| 1.0 | 轻微谐振 |
| > 2.0 | 明显谐振峰，适合带通滤波 |

---

### `biquad_cascade.h` — 级联滤波器

#### 结构体

```c
struct biquad_cascade {
    struct biquad *stages; /* 指向 biquad 数组 */
    int n_stages;          /* 级数 */
};
```

#### `biquad_cascade_init`

```c
void biquad_cascade_init(struct biquad_cascade *cas,
                         struct biquad *stages, int n);
```

初始化级联滤波器，绑定已初始化好的 `biquad` 数组。

#### `biquad_cascade_filter`

```c
float biquad_cascade_filter(struct biquad_cascade *cas, float x);
```

将输入信号依次通过每一级，返回最终输出。

---

## 命令行工具 `biquad_cli`

快速计算滤波器系数，输出格式兼容 MATLAB/Octave：

```bash
biquad_cli <type> <fs> <Q> <fc>
```

| 参数 | 说明 |
|------|------|
| `type` | 0=LPF, 1=HPF, 2=BPF, 3=BSF |
| `fs` | 采样率（Hz） |
| `Q` | 品质因数 |
| `fc` | 截止频率（Hz） |

**示例：**

```bash
# 采样率 48000 Hz，截止 1000 Hz 的低通，Q=0.707
$ ./biquad_cli 0 48000 0.707 1000
b = [0.001323, 0.002646, 0.001323];
a = [1, -1.990079, 0.995372];
fs = 48000;
```

将输出粘贴到 `biquad_quick_test.m` 中，即可在 MATLAB/Octave 中绘制频率响应图。

---

## 构建

### 依赖

- CMake ≥ 3.14
- C11 兼容编译器（GCC / Clang / MSVC）
- 链接数学库（Linux 下需 `-lm`，CMake 已自动处理）

### 编译

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

编译产物输出到项目根目录：

| 文件 | 说明 |
|------|------|
| `biquad_cli` | 系数计算命令行工具 |
| `biquad_examples` | 示例程序（需 `data/` 目录下有数据文件） |
| `biquad_test` | 单元测试 |

### 仅使用库（不需要命令行工具）

直接将以下文件复制到你的项目：

```
biquad.h          ← 必须
biquad_design.h   ← 如需自动设计系数
biquad_design.c   ← 如需自动设计系数
biquad_cascade.h  ← 如需级联滤波器（可选）
```

---

## 与 MATLAB/Octave 验证

1. 运行 `biquad_cli` 得到系数输出；
2. 将系数粘贴到 `biquad_quick_test.m` 中；
3. 在 MATLAB/Octave 中运行，查看幅频和相频响应图。

---

## 参考资料

- [Audio EQ Cookbook — Robert Bristow-Johnson](https://www.w3.org/TR/audio-eq-cookbook/)
- 双线性变换（Bilinear Transform）数字滤波器设计

---

## 许可证

MIT License — 详见 [LICENSE](LICENSE)
