/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 */

#ifndef SRC_BIQUAD_CASCADE_H
#define SRC_BIQUAD_CASCADE_H

/**
 * biquad_cascade.h — 多级联双二阶滤波器
 *
 * 将多个 biquad 滤波器串联，实现更高阶的滤波效果。
 * 例如，两个相同的 2 阶 Butterworth 低通级联可等效一个 4 阶低通。
 *
 * 使用方法：
 *   1. 分别初始化每一级 struct biquad（通过 biquad_init 或 biquad_design）；
 *   2. 将 biquad 数组和级数传入 biquad_cascade_init；
 *   3. 调用 biquad_cascade_filter 处理每个采样点。
 *
 * 注意：
 *   - 调用者负责管理 stages 数组的生命周期，须保证在 cascade 使用期间有效；
 *   - 每一级的状态相互独立，重置时需分别对每级调用 biquad_init。
 */

#include "biquad.h"

struct biquad_cascade {
    struct biquad *stages; /* 指向 biquad 级数组（调用者分配） */
    int n_stages;          /* 总级数，必须 >= 1 */
};

/**
 * biquad_cascade_init - 初始化级联滤波器
 *
 * @cas      级联结构体指针
 * @stages   已初始化好的 biquad 数组（至少 n 个元素）
 * @n        级数
 */
static inline void biquad_cascade_init(struct biquad_cascade *cas,
                                        struct biquad *stages, int n) {
    cas->stages   = stages;
    cas->n_stages = n;
}

/**
 * biquad_cascade_filter - 处理单个采样点
 *
 * 输入信号依次流经每一级，前一级的输出作为下一级的输入。
 *
 * @cas  级联滤波器指针
 * @x    输入采样值
 * @return 最终输出采样值
 */
static inline float biquad_cascade_filter(struct biquad_cascade *cas, float x) {
    float y = x;
    for (int i = 0; i < cas->n_stages; i++) {
        y = biquad_filter(&cas->stages[i], y);
    }
    return y;
}

#endif /* SRC_BIQUAD_CASCADE_H */
