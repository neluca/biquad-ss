/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "biquad_design.h"

static const char *type_names[] = {"LPF (低通)", "HPF (高通)", "BPF (带通)", "BSF (带阻)"};

static void print_usage(const char *prog) {
    fprintf(stderr, "用法: %s <type> <fs> <Q> <fc>\n\n", prog);
    fprintf(stderr, "参数:\n");
    fprintf(stderr, "  type  滤波器类型: 0=LPF(低通)  1=HPF(高通)  2=BPF(带通)  3=BSF(带阻)\n");
    fprintf(stderr, "  fs    采样率 (Hz)，必须 > 0\n");
    fprintf(stderr, "  Q     品质因数，建议范围 [0.5, 10.0]\n");
    fprintf(stderr, "         0.707 = Butterworth (最平坦)\n");
    fprintf(stderr, "         0.577 = Bessel (最线性相位)\n");
    fprintf(stderr, "  fc    截止频率 (Hz)，必须满足 0 < fc < fs/2\n\n");
    fprintf(stderr, "示例:\n");
    fprintf(stderr, "  %s 0 48000 0.707 1000   # 48kHz 采样, 1kHz 截止低通\n", prog);
    fprintf(stderr, "  %s 1 100   0.7   0.8    # 100Hz 采样, 0.8Hz 截止高通\n", prog);
    fprintf(stderr, "  %s 2 1000  5.0   60     # 1kHz 采样, 60Hz 中心带通, 窄带\n\n", prog);
    fprintf(stderr, "输出格式兼容 MATLAB/Octave，可直接粘贴到 biquad_quick_test.m 中验证。\n");
}

int main(int argc, char *argv[]) {

    /* 处理帮助选项 */
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    if (argc != 5) {
        fprintf(stderr, "错误: 需要 4 个参数，实际提供了 %d 个。\n\n", argc - 1);
        print_usage(argv[0]);
        return 1;
    }

    int type = atoi(argv[1]);
    float fs  = (float)atof(argv[2]);
    float Q   = (float)atof(argv[3]);
    float fc  = (float)atof(argv[4]);

    if (type < 0 || type > 3) {
        fprintf(stderr, "错误: type 必须是 0~3 之间的整数（0=LPF, 1=HPF, 2=BPF, 3=BSF），当前值: %d\n", type);
        return 1;
    }

    if (fs <= 0.0f) {
        fprintf(stderr, "错误: 采样率 fs 必须大于 0，当前值: %g\n", (double)fs);
        return 1;
    }

    if (fc <= 0.0f || fc >= fs / 2.0f) {
        fprintf(stderr, "错误: 截止频率 fc 必须满足 0 < fc < fs/2 (= %.4g Hz)，当前值: %g Hz\n",
                (double)(fs / 2.0f), (double)fc);
        return 1;
    }

    if (Q < 0.5f) {
        fprintf(stderr, "警告: Q 值 %g 低于最小限制，已自动修正为 0.5\n", (double)Q);
    }

    struct biquad filter;
    int ret = biquad_design(&filter, (biquad_type_t)type, fs, Q, fc);
    if (ret != 0) {
        fprintf(stderr, "错误: 滤波器设计失败（参数非法）\n");
        return 1;
    }

    /* 输出 MATLAB/Octave 兼容格式 */
    printf("# %s  fs=%.4gHz  fc=%.4gHz  Q=%.4g\n",
           type_names[type], (double)fs, (double)fc, (double)Q);
    printf("b = [%.9f, %.9f, %.9f];\n", (double)filter.b0, (double)filter.b1, (double)filter.b2);
    printf("a = [1, %.9f, %.9f];\n",    (double)filter.a1, (double)filter.a2);
    printf("fs = %.0f;\n",              (double)fs);

    return 0;
}
