/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 */

#ifndef SRC_BIQUAD_H
#define SRC_BIQUAD_H

/**
 * 代码实现了一个双二阶滤波器，但其状态更新方式与常见的直接1形式不同。
 * 它本质上是一种状态空间实现，与标准的直接1形式的差分方程是等价的。
 * 优势：
 *  1、状态更新可以并行计算，适合某些硬件架构；
 *  2、连续的内存访问模式可能提高缓存效率。
 */

struct biquad {
    float b0, b1, b2; /* 前向系数 */
    float a1, a2;     /* 反馈系数 */

    float s[3]; /* 状态变量 */
};

static inline void biquad_init(struct biquad *iir, float b0, float b1, float b2, float a1, float a2) {
    iir->b0 = b0;
    iir->b1 = b1;
    iir->b2 = b2;
    iir->a1 = a1;
    iir->a2 = a2;

    iir->s[0] = 0;
    iir->s[1] = 0;
    iir->s[2] = 0;
}

static inline float biquad_filter(struct biquad *iir, float x) {
    /* 状态累积（输入馈入）*/
    iir->s[0] += x * iir->b0;
    iir->s[1] += x * iir->b1;
    iir->s[2] += x * iir->b2;

    /* 输出计算 */
    float y = iir->s[0];

    /* 状态更新（状态转移与反馈）*/
    iir->s[0] = iir->s[1] - y * iir->a1;
    iir->s[1] = iir->s[2] - y * iir->a2;
    iir->s[2] = 0;

    return y;
}

#endif /* SRC_BIQUAD_H */
