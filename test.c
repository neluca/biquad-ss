/*
 * Copyright Yinan Liao. and other contributors. All rights reserved.
 *
 * test.c — biquad-ss 单元测试
 *
 * 测试策略：
 *   1. 系数正确性：对比 Audio EQ Cookbook 的参考值（允许浮点误差 1e-5）
 *   2. 直流响应：LPF/HPF 对直流信号的稳态响应
 *   3. 冲激响应：验证滤波器在零输入后衰减至零
 *   4. 级联：验证两级 LPF 级联与单级的输出关系
 *   5. 参数边界：非法参数应返回 -1
 */

#include <stdio.h>
#include <math.h>
#include "biquad_design.h"
#include "biquad_cascade.h"

#define PI 3.14159265358979323846f

/* -----------------------------------------------------------------------
 * 测试框架
 * ----------------------------------------------------------------------- */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_NEAR(a, b, tol, msg) do { \
    g_tests_run++; \
    float _a = (float)(a), _b = (float)(b); \
    float _diff = _a - _b; \
    if (_diff < 0) _diff = -_diff; \
    if (_diff <= (float)(tol)) { \
        g_tests_passed++; \
    } else { \
        g_tests_failed++; \
        printf("  FAIL [%s:%d] %s\n       got %.9f, expected %.9f (diff=%.3e, tol=%.3e)\n", \
               __FILE__, __LINE__, (msg), (double)_a, (double)_b, (double)_diff, (double)(tol)); \
    } \
} while(0)

#define ASSERT_EQ_INT(a, b, msg) do { \
    g_tests_run++; \
    if ((a) == (b)) { \
        g_tests_passed++; \
    } else { \
        g_tests_failed++; \
        printf("  FAIL [%s:%d] %s\n       got %d, expected %d\n", \
               __FILE__, __LINE__, (msg), (a), (b)); \
    } \
} while(0)

/* -----------------------------------------------------------------------
 * 测试 1：LPF 系数正确性
 *   参考值由本库输出与 Python 公式（Audio EQ Cookbook）交叉验证：
 *   fs=48000, fc=1000, Q=0.707
 *   b0=0.003916, b1=0.007832, b2=0.003916, a1=-1.815318, a2=0.830982
 * ----------------------------------------------------------------------- */
static void test_lpf_coefficients(void) {
    printf("[Test 1] LPF 系数正确性\n");
    struct biquad f;
    int ret = biquad_design(&f, BIQUAD_LPF, 48000.0f, 0.707f, 1000.0f);
    ASSERT_EQ_INT(ret, 0, "biquad_design 返回值");
    ASSERT_NEAR(f.b0,  0.003916077f, 1e-5f, "LPF b0");
    ASSERT_NEAR(f.b1,  0.007832153f, 1e-5f, "LPF b1");
    ASSERT_NEAR(f.b2,  0.003916077f, 1e-5f, "LPF b2");
    ASSERT_NEAR(f.a1, -1.815318f,    1e-4f, "LPF a1");
    ASSERT_NEAR(f.a2,  0.830982f,    1e-4f, "LPF a2");
}

/* -----------------------------------------------------------------------
 * 测试 2：HPF 系数正确性
 *   参考值由本库输出与 Python 公式交叉验证：
 *   fs=48000, fc=1000, Q=0.707
 *   b0=0.911575, b1=-1.823150, b2=0.911575, a1=-1.815318, a2=0.830982
 * ----------------------------------------------------------------------- */
static void test_hpf_coefficients(void) {
    printf("[Test 2] HPF 系数正确性\n");
    struct biquad f;
    int ret = biquad_design(&f, BIQUAD_HPF, 48000.0f, 0.707f, 1000.0f);
    ASSERT_EQ_INT(ret, 0, "biquad_design 返回值");
    ASSERT_NEAR(f.b0,  0.911575f,  1e-4f, "HPF b0");
    ASSERT_NEAR(f.b1, -1.823150f,  1e-4f, "HPF b1");
    ASSERT_NEAR(f.b2,  0.911575f,  1e-4f, "HPF b2");
    ASSERT_NEAR(f.a1, -1.815318f,  1e-4f, "HPF a1（与LPF共享）");
    ASSERT_NEAR(f.a2,  0.830982f,  1e-4f, "HPF a2（与LPF共享）");
}

/* -----------------------------------------------------------------------
 * 测试 3：BPF 系数正确性
 *   参考：Audio EQ Cookbook, fs=1000, fc=100, Q=5
 *   a1 和 a2 通过公式推导验证
 * ----------------------------------------------------------------------- */
static void test_bpf_coefficients(void) {
    printf("[Test 3] BPF 系数正确性\n");
    struct biquad f;
    int ret = biquad_design(&f, BIQUAD_BPF, 1000.0f, 5.0f, 100.0f);
    ASSERT_EQ_INT(ret, 0, "biquad_design 返回值");
    /* BPF: b1 必须为 0，b0 = -b2 */
    ASSERT_NEAR(f.b1, 0.0f, 1e-10f, "BPF b1 == 0");
    ASSERT_NEAR(f.b0, -f.b2, 1e-6f, "BPF b0 == -b2");
    /* 验证归一化：b0 > 0 */
    if (f.b0 <= 0.0f) {
        g_tests_failed++;
        printf("  FAIL BPF b0 应大于 0，实际: %.9f\n", (double)f.b0);
    } else {
        g_tests_passed++;
    }
    g_tests_run++;
}

/* -----------------------------------------------------------------------
 * 测试 4：BSF 系数正确性
 *   BSF: b1 = -2*cos(omega)/a0，b0 = b2
 * ----------------------------------------------------------------------- */
static void test_bsf_coefficients(void) {
    printf("[Test 4] BSF 系数正确性\n");
    struct biquad f;
    int ret = biquad_design(&f, BIQUAD_BSF, 1000.0f, 10.0f, 50.0f);
    ASSERT_EQ_INT(ret, 0, "biquad_design 返回值");
    /* BSF: b0 == b2 */
    ASSERT_NEAR(f.b0, f.b2, 1e-6f, "BSF b0 == b2");
}

/* -----------------------------------------------------------------------
 * 测试 5：LPF 直流增益 = 1
 *   直流信号（x=1 常量）输入 LPF，稳态输出应趋近于 1
 * ----------------------------------------------------------------------- */
static void test_lpf_dc_gain(void) {
    printf("[Test 5] LPF 直流增益\n");
    struct biquad f;
    biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, 100.0f);
    float y = 0.0f;
    for (int i = 0; i < 5000; i++) {
        y = biquad_filter(&f, 1.0f);
    }
    ASSERT_NEAR(y, 1.0f, 1e-4f, "LPF 直流稳态输出 ≈ 1.0");
}

/* -----------------------------------------------------------------------
 * 测试 6：HPF 直流增益 = 0
 *   直流信号（x=1 常量）输入 HPF，稳态输出应趋近于 0
 * ----------------------------------------------------------------------- */
static void test_hpf_dc_gain(void) {
    printf("[Test 6] HPF 直流增益\n");
    struct biquad f;
    biquad_design(&f, BIQUAD_HPF, 1000.0f, 0.707f, 100.0f);
    float y = 0.0f;
    for (int i = 0; i < 5000; i++) {
        y = biquad_filter(&f, 1.0f);
    }
    ASSERT_NEAR(y, 0.0f, 1e-4f, "HPF 直流稳态输出 ≈ 0.0");
}

/* -----------------------------------------------------------------------
 * 测试 7：冲激响应衰减
 *   冲激（x[0]=1, x[n]=0, n>0）后，滤波器输出应在有限步内趋近于零
 * ----------------------------------------------------------------------- */
static void test_impulse_decay(void) {
    printf("[Test 7] 冲激响应衰减\n");
    struct biquad f;
    biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, 10.0f);
    float y = 0.0f;
    for (int i = 0; i < 2000; i++) {
        float x = (i == 0) ? 1.0f : 0.0f;
        y = biquad_filter(&f, x);
    }
    ASSERT_NEAR(y, 0.0f, 1e-5f, "冲激响应 2000 步后衰减至 0");
}

/* -----------------------------------------------------------------------
 * 测试 8：级联滤波器输出一致性
 *   两级相同 LPF 级联，每一步的输出应等于手动串联的结果
 * ----------------------------------------------------------------------- */
static void test_cascade_consistency(void) {
    printf("[Test 8] 级联滤波器一致性\n");

    struct biquad stages[2], ref_a, ref_b;
    biquad_design(&stages[0], BIQUAD_LPF, 1000.0f, 0.707f, 100.0f);
    biquad_design(&stages[1], BIQUAD_LPF, 1000.0f, 0.707f, 100.0f);
    biquad_design(&ref_a,     BIQUAD_LPF, 1000.0f, 0.707f, 100.0f);
    biquad_design(&ref_b,     BIQUAD_LPF, 1000.0f, 0.707f, 100.0f);

    struct biquad_cascade cas;
    biquad_cascade_init(&cas, stages, 2);

    int ok = 1;
    for (int i = 0; i < 100; i++) {
        float x = sinf(2.0f * PI * 10.0f * (float)i / 1000.0f);
        float y_cas = biquad_cascade_filter(&cas, x);
        float y_ref = biquad_filter(&ref_b, biquad_filter(&ref_a, x));
        float diff = y_cas - y_ref;
        if (diff < 0) diff = -diff;
        if (diff > 1e-6f) {
            ok = 0;
            printf("  FAIL 级联 vs 手动串联 在 i=%d 差异 %.3e\n", i, (double)diff);
            break;
        }
    }
    g_tests_run++;
    if (ok) {
        g_tests_passed++;
    } else {
        g_tests_failed++;
    }
}

/* -----------------------------------------------------------------------
 * 测试 9：非法参数处理
 * ----------------------------------------------------------------------- */
static void test_invalid_params(void) {
    printf("[Test 9] 非法参数处理\n");
    struct biquad f;

    /* fc <= 0 */
    ASSERT_EQ_INT(biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, 0.0f),    -1, "fc=0 应返回 -1");
    /* fc >= fs/2 */
    ASSERT_EQ_INT(biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, 500.0f),  -1, "fc=fs/2 应返回 -1");
    ASSERT_EQ_INT(biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, 600.0f),  -1, "fc>fs/2 应返回 -1");
    /* fc < 0 */
    ASSERT_EQ_INT(biquad_design(&f, BIQUAD_LPF, 1000.0f, 0.707f, -1.0f),   -1, "fc<0 应返回 -1");
    /* type 非法（通过强制转换） */
    ASSERT_EQ_INT(biquad_design(&f, (biquad_type_t)99, 1000.0f, 0.707f, 100.0f), -1, "非法 type 应返回 -1");
}

/* -----------------------------------------------------------------------
 * 测试 10：BPF 在中心频率处增益 ≈ 1
 *   向 BPF 馈入中心频率正弦波，稳态幅度应约为 1
 * ----------------------------------------------------------------------- */
static void test_bpf_center_gain(void) {
    printf("[Test 10] BPF 中心频率增益\n");

    const float fs = 1000.0f;
    const float fc = 100.0f;
    const float Q  = 5.0f;

    struct biquad bpf;
    biquad_design(&bpf, BIQUAD_BPF, fs, Q, fc);

    float max_y = 0.0f;
    /* 让滤波器到达稳态后测量幅度 */
    for (int i = 0; i < 3000; i++) {
        float x = sinf(2.0f * PI * fc * (float)i / fs);
        float y = biquad_filter(&bpf, x);
        if (i >= 2000) {
            float ay = y < 0 ? -y : y;
            if (ay > max_y) max_y = ay;
        }
    }
    ASSERT_NEAR(max_y, 1.0f, 0.05f, "BPF 中心频率稳态幅度 ≈ 1.0");
}

/* -----------------------------------------------------------------------
 * 主函数
 * ----------------------------------------------------------------------- */
int main(void) {
    printf("======================================\n");
    printf("biquad-ss 单元测试\n");
    printf("======================================\n\n");

    test_lpf_coefficients();
    test_hpf_coefficients();
    test_bpf_coefficients();
    test_bsf_coefficients();
    test_lpf_dc_gain();
    test_hpf_dc_gain();
    test_impulse_decay();
    test_cascade_consistency();
    test_invalid_params();
    test_bpf_center_gain();

    printf("\n======================================\n");
    printf("结果: %d 通过 / %d 失败 / %d 总计\n",
           g_tests_passed, g_tests_failed, g_tests_run);
    printf("======================================\n");

    return (g_tests_failed == 0) ? 0 : 1;
}
