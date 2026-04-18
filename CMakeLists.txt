cmake_minimum_required(VERSION 3.14)
project(biquad-ss C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ------------------------------------------------------------
# 平台数学库检测
# Linux/macOS 需要显式链接 libm，MSVC 不需要
# ------------------------------------------------------------
if(UNIX)
    set(MATH_LIB m)
else()
    set(MATH_LIB "")
endif()

# ------------------------------------------------------------
# 编译选项
# ------------------------------------------------------------
if(MSVC)
    # /utf-8: 将源文件和执行字符集均视为 UTF-8，避免 GBK 代码页下中文注释解析异常
    add_compile_options(/W3 /utf-8)
    # 消除 fopen/fscanf 等标准 C 函数的 MSVC 安全废弃警告
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# ------------------------------------------------------------
# 目标 1: biquad_cli — 系数计算命令行工具
# ------------------------------------------------------------
add_executable(biquad_cli cli.c biquad_design.c)
target_link_libraries(biquad_cli PRIVATE ${MATH_LIB})

# ------------------------------------------------------------
# 目标 2: biquad_examples — 各类示例程序
# ------------------------------------------------------------
add_executable(biquad_examples examples.c biquad_design.c)
target_link_libraries(biquad_examples PRIVATE ${MATH_LIB})

# ------------------------------------------------------------
# 目标 3: biquad_test — 单元测试
# ------------------------------------------------------------
add_executable(biquad_test test.c biquad_design.c)
target_link_libraries(biquad_test PRIVATE ${MATH_LIB})

enable_testing()
add_test(NAME biquad_unit_test COMMAND biquad_test)

# ------------------------------------------------------------
# 安装规则（可选，执行 cmake --install . 时生效）
# ------------------------------------------------------------
install(TARGETS biquad_cli RUNTIME DESTINATION bin)
install(FILES
    biquad.h
    biquad_design.h
    biquad_cascade.h
    DESTINATION include/biquad-ss
)
install(FILES biquad_design.c DESTINATION src/biquad-ss)
