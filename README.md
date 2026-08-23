# xv6 Operating Systems Course Design

本仓库基于 MIT `xv6-labs-2021`，使用不同 Git 分支保存各实验的实现。

## 实验分支

| 分支 | 实验 | 官方测试 |
| --- | --- | --- |
| `util` | Lab 1：Unix Utilities | `100/100` |
| `syscall` | Lab 2：System Calls | `35/35` |
| `pgtbl` | Lab 3：Page Tables | `46/46` |

## 切换与运行

```bash
git fetch origin
git checkout util       # 实验一
git checkout syscall    # 实验二
git checkout pgtbl      # 实验三

make clean
make qemu
```

在 Ubuntu 中运行当前实验的官方测试：

```bash
make grade
```

## 实验报告

- [实验一：Unix Utilities](reports/lab1.md)
- [实验二：System Calls](reports/lab2.md)

## 上游项目

- MIT 6.S081 / 6.828: <https://pdos.csail.mit.edu/6.828/2021/xv6.html>
