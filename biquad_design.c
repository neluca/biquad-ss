/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 */

#include "biquad_design.h"
#include <math.h>

#define _2PI (2.0f * 3.14159265358979323846f)

int biquad_design(struct biquad *iir, biquad_type_t type, float fs, float Q, float fc) {
    if (fc <= 0 || fc >= fs / 2) {
        return -1;
    }

    if (Q < 0.5f) {
        Q = 0.5f;
    }

    /* 计算极点参数 */
    float omega = _2PI * fc / fs;           /* 数字角频率 */
    float alpha = sinf(omega) / (2.0f * Q); /* 带宽参数 */
    float cos_w = cosf(omega);

    /* 公共分母系数 */
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_w / a0;
    float a2 = (1.0f - alpha) / a0;

    /* 根据滤波器类型计算系数 */
    float b0, b1, b2;
    switch (type) {
        case BIQUAD_LPF:
            b0 = (1.0f - cos_w) / 2.0f;
            b1 = 1.0f - cos_w;
            b2 = (1.0f - cos_w) / 2.0f;
            break;
        case BIQUAD_HPF:
            b0 = (1.0f + cos_w) / 2.0f;
            b1 = -(1.0f + cos_w);
            b2 = (1.0f + cos_w) / 2.0f;
            break;
        case BIQUAD_BPF:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            break;
        case BIQUAD_BSF:
            b0 = 1.0f;
            b1 = -2.0f * cos_w;
            b2 = 1.0f;
            break;

        default: return -1;
    }

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;

    biquad_init(iir, b0, b1, b2, a1, a2);

    return 0;
}