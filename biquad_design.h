/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 */

#ifndef SRC_BIQUAD_DESIGN_H
#define SRC_BIQUAD_DESIGN_H

#include "biquad.h"

/* 滤波器类型枚举 */
typedef enum {
    BIQUAD_LPF, /* 低通滤波器 */
    BIQUAD_HPF, /* 高通滤波器 */
    BIQUAD_BPF, /* 带通滤波器 */
    BIQUAD_BSF, /* 带阻滤波器 */
} biquad_type_t;

int biquad_design(struct biquad *iir, biquad_type_t type, float fs, float Q, float fc);

#endif /* SRC_BIQUAD_DESIGN_H */
