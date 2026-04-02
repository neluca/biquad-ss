clear all; close all;

% 从C程序输出的内容复制到这里
b = [0.006735, 0.013471, 0.006735];
a = [1, -1.718483, 0.745425];
fs = 100;

[h, f] = freqz(b, a, 1024, fs);

% 绘制幅频响应
figure;
subplot(2,1,1);
semilogx(f, 20*log10(abs(h)));
xlabel('频率 (Hz)');
ylabel('幅度 (dB)');
title('幅频响应');
grid on;
xlim([1, fs/2]);

% 绘制相频响应
subplot(2,1,2);
semilogx(f, unwrap(angle(h))*180/pi);
xlabel('频率 (Hz)');
ylabel('相位 (度)');
title('相频响应');
grid on;
xlim([1, fs/2]);