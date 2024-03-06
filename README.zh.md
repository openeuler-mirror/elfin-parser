Libelfin 是一个从头开始的 C++11 库，用于读取 ELF 二进制文件和 DWARFv4 调试信息。

## 快速入门

 `make` 和 （可选） `make install` 。您需要 GCC 4.7 或更高版本。

另一种方式：

 `bash build.sh` 。您需要 cmake 3.12 或更高版本。

## 特性

- 原生 C++11 代码和接口，从头开始设计，可与 C++11 功能良好交互，从基于范围的 for 循环到将语义移动到枚举类。
- Libelfin 完全实现了对调试信息条目 （DIE） 的解析，这是 DWARF 格式使用的核心数据结构，以及大多数 DWARFv4 表。
- 支持除位置列表和宏之外的所有 DWARFv4 DIE 值类型。
- 几乎完整的 DWARFv4 表达式和位置描述的计算器。
- DWARFv4 行表的完整解释器。
- 用于轻松自然地遍历编译单元、类型单元、DIE 树和 DIE 属性列表的迭代器。
- 每个枚举值都可以打印得很漂亮。
- 大量类型安全的 DIE 属性提取器。
- 支持 DWARF5，添加信号处理程序和添加函数，将函数名转换为字符串。

## 非特性

Libelfin 实现了 DWARF 和 ELF 的句法层，但不是语义层。解释存储在 DWARF DIE 树中的信息仍然需要对 DWARF 有大量的了解，但 libelfin 会为您理解字节。

## 使用 libelfin

要针对 `libdwarf++` ，请使用，例如

```
g++ -std=c++11 a.cc $(pkg-config --cflags --libs libdwarf++)
```

要使用 libelfin 的本地版本，请设置 `PKG_CONFIG_PATH` .例如

```
export PKG_CONFIG_PATH=$PWD/elf:$PWD/dwarf
```

其中 `examples/` 提供了各种示例程序。

另一种方式：

```
LD_PRELOAD=$PWD/install/lib64/libstdc++.so workload
```

## 状态

Libelfin是一个好的开始。它还没有准备好生产，而且 DWARF 规范的许多部分还没有实现，但它足够完整，对很多事情都很有用，并且比我尝试过的所有其他调试信息库使用起来都更愉快。

## 感谢

感谢 Austin T. Clements 为软件的基本功能做出了贡献