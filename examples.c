/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 *
 * 示例程序：演示 biquad 滤波器的多种使用场景
 */

#include <stdio.h>
#include <math.h>
#include "biquad_design.h"
#include "biquad_cascade.h"

#define PI 3.14159265358979323846f

/* -----------------------------------------------------------------------
 * 示例 1：对文件中的数据应用高通滤波器（去除直流漂移）
 *   数据：data/data1.txt（100Hz 采样的加速度计数据）
 *   目标：0.8Hz 截止高通，去除低频漂移
 * ----------------------------------------------------------------------- */
static void example1_file_hpf(void) {
    printf("=== 示例 1：文件高通滤波 ===\n");

    struct biquad hpf;
    if (biquad_design(&hpf, BIQUAD_HPF, 100.0f, 0.7f, 0.8f) != 0) {
        fprintf(stderr, "  错误: 滤波器设计失败\n");
        return;
    }

    FILE *fp_in  = fopen("./data/data1.txt", "r");
    FILE *fp_out = fopen("./data/data1_hpf.txt", "w");

    if (!fp_in) {
        fprintf(stderr, "  错误: 无法打开 data/data1.txt\n");
        return;
    }
    if (!fp_out) {
        fprintf(stderr, "  错误: 无法创建 data/data1_hpf.txt\n");
        fclose(fp_in);
        return;
    }

    float d;
    int count = 0;
    while (fscanf(fp_in, "%f", &d) == 1) {
        float out = biquad_filter(&hpf, d);
        fprintf(fp_out, "%f\n", out);
        count++;
    }

    fclose(fp_in);
    fclose(fp_out);
    printf("  处理完成：共 %d 个采样点 → data/data1_hpf.txt\n\n", count);
}

/* -----------------------------------------------------------------------
 * 示例 2：对文件中的数据应用低通滤波器（平滑）
 *   数据：data/data2.txt
 *   目标：设计一个低通滤波器对信号进行平滑处理
 * ----------------------------------------------------------------------- */
static void example2_file_lpf(void) {
    printf("=== 示例 2：文件低通滤波 ===\n");

    struct biquad lpf;
    /* 采样率 1000Hz，截止 50Hz，Butterworth Q=0.707 */
    if (biquad_design(&lpf, BIQUAD_LPF, 1000.0f, 0.707f, 50.0f) != 0) {
        fprintf(stderr, "  错误: 滤波器设计失败\n");
        return;
    }

    FILE *fp_in  = fopen("./data/data2.txt", "r");
    FILE *fp_out = fopen("./data/data2_lpf.txt", "w");

    if (!fp_in) {
        fprintf(stderr, "  错误: 无法打开 data/data2.txt\n");
        return;
    }
    if (!fp_out) {
        fprintf(stderr, "  错误: 无法创建 data/data2_lpf.txt\n");
        fclose(fp_in);
        return;
    }

    float d;
    int count = 0;
    while (fscanf(fp_in, "%f", &d) == 1) {
        float out = biquad_filter(&lpf, d);
        fprintf(fp_out, "%f\n", out);
        count++;
    }

    fclose(fp_in);
    fclose(fp_out);
    printf("  处理完成：共 %d 个采样点 → data/data2_lpf.txt\n\n", count);
}

/* -----------------------------------------------------------------------
 * 示例 3：带通滤波器 — 提取特定频率成分
 *   生成一个包含 50Hz 和 200Hz 成分的合成信号，用带通滤波器提取 50Hz
 * ----------------------------------------------------------------------- */
static void example3_bandpass_synth(void) {
    printf("=== 示例 3：带通滤波器（合成信号）===\n");

    const float fs  = 1000.0f;  /* 采样率 1kHz */
    const float f1  = 50.0f;    /* 目标频率 50Hz */
    const float f2  = 200.0f;   /* 干扰频率 200Hz */
    const int   N   = 200;      /* 采样点数 */

    struct biquad bpf;
    /* 中心频率 50Hz，Q=5 → 带宽约 10Hz */
    if (biquad_design(&bpf, BIQUAD_BPF, fs, 5.0f, f1) != 0) {
        fprintf(stderr, "  错误: 滤波器设计失败\n");
        return;
    }

    FILE *fp = fopen("./data/example3_bpf.txt", "w");
    if (!fp) {
        fprintf(stderr, "  错误: 无法创建 data/example3_bpf.txt\n");
        return;
    }

    fprintf(fp, "# n  input  output\n");
    for (int i = 0; i < N; i++) {
        float t = (float)i / fs;
        float x = sinf(2.0f * PI * f1 * t) + sinf(2.0f * PI * f2 * t);
        float y = biquad_filter(&bpf, x);
        fprintf(fp, "%d %f %f\n", i, x, y);
    }

    fclose(fp);
    printf("  生成合成信号（50Hz + 200Hz），带通提取 50Hz\n");
    printf("  结果保存至 data/example3_bpf.txt\n\n");
}

/* -----------------------------------------------------------------------
 * 示例 4：级联低通滤波器 — 更陡的滚降特性
 *   两级相同 Butterworth 低通级联 → 等效 4 阶滤波器
 * ----------------------------------------------------------------------- */
static void example4_cascade_lpf(void) {
    printf("=== 示例 4：级联低通滤波器（4 阶等效）===\n");

    const float fs = 1000.0f;
    const float fc = 100.0f;
    const int   N  = 100;

    struct biquad stages[2];
    biquad_design(&stages[0], BIQUAD_LPF, fs, 0.707f, fc);
    biquad_design(&stages[1], BIQUAD_LPF, fs, 0.707f, fc);

    struct biquad_cascade cascade;
    biquad_cascade_init(&cascade, stages, 2);

    /* 对冲激响应采样 */
    printf("  冲激响应前 10 点（4 阶低通，fc=%.0fHz，fs=%.0fHz）:\n", (double)fc, (double)fs);
    for (int i = 0; i < N; i++) {
        float x = (i == 0) ? 1.0f : 0.0f;  /* 冲激信号 */
        float y = biquad_cascade_filter(&cascade, x);
        if (i < 10) {
            printf("    y[%2d] = %9.6f\n", i, (double)y);
        }
    }
    printf("\n");
}

/* -----------------------------------------------------------------------
 * 示例 5：带阻滤波器（陷波）— 抑制工频干扰
 *   模拟 50Hz 工频干扰叠加在有用信号上，用 BSF 滤除
 * ----------------------------------------------------------------------- */
static void example5_notch_filter(void) {
    printf("=== 示例 5：带阻陷波滤波器（抑制 50Hz 工频干扰）===\n");

    const float fs     = 1000.0f;
    const float f_sig  = 10.0f;    /* 有用信号频率 */
    const float f_hum  = 50.0f;    /* 工频干扰频率 */
    const int   N      = 300;

    struct biquad bsf;
    /* 中心频率 50Hz，Q=10 → 陷波很窄，不影响有用信号 */
    if (biquad_design(&bsf, BIQUAD_BSF, fs, 10.0f, f_hum) != 0) {
        fprintf(stderr, "  错误: 滤波器设计失败\n");
        return;
    }

    FILE *fp = fopen("./data/example5_notch.txt", "w");
    if (!fp) {
        fprintf(stderr, "  错误: 无法创建 data/example5_notch.txt\n");
        return;
    }

    fprintf(fp, "# n  input  output\n");

    /* 稳态后统计幅度 */
    float max_in = 0.0f, max_out = 0.0f;
    for (int i = 0; i < N; i++) {
        float t = (float)i / fs;
        float x = sinf(2.0f * PI * f_sig * t) + 0.5f * sinf(2.0f * PI * f_hum * t);
        float y = biquad_filter(&bsf, x);
        fprintf(fp, "%d %f %f\n", i, x, y);
        if (i >= N / 2) {
            if (x < 0 ? -x : x) max_in  = (x < 0 ? -x : x) > max_in  ? (x < 0 ? -x : x) : max_in;
            if (y < 0 ? -y : y) max_out = (y < 0 ? -y : y) > max_out ? (y < 0 ? -y : y) : max_out;
        }
    }

    fclose(fp);
    printf("  稳态输入幅度约: %.4f，滤波后约: %.4f\n", (double)max_in, (double)max_out);
    printf("  50Hz 工频干扰被抑制，信号幅度接近 1.0（有用信号保留）\n");
    printf("  结果保存至 data/example5_notch.txt\n\n");
}

/* -----------------------------------------------------------------------
 * 主函数
 * ----------------------------------------------------------------------- */
int main(void) {
    printf("biquad-ss 滤波器示例程序\n");
    printf("========================================\n\n");

    example1_file_hpf();
    example2_file_lpf();
    example3_bandpass_synth();
    example4_cascade_lpf();
    example5_notch_filter();

    printf("所有示例运行完毕。\n");
    return 0;
}
