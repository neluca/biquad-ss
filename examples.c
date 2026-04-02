#include <stdio.h>
#include "biquad_design.h"

void data1_hpf() {
    /* 设计一个采样率 100Hz，截止频率 0.8Hz的高通滤波器 */
    struct biquad hpf;
    biquad_design(&hpf, BIQUAD_HPF, 100, 0.7f, 0.8f);

    FILE *fp = fopen("./data/data1.txt", "r");
    FILE *fp_out = fopen("./data/data1_hpf.txt", "w+");

    float d;
    while (!feof(fp)) {
        fscanf(fp, "%f", &d);
        float out = biquad_filter(&hpf, d);
        fprintf(fp_out, "%f\n", out);
    }

    fclose(fp);
    fclose(fp_out);
}

int main(void) {

    data1_hpf();
    return 0;
}
