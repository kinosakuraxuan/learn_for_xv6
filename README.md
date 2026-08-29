# xv6 Operating Systems Course Design

本仓库基于 MIT `xv6-labs-2021`，使用十个 Git 分支分别保存十个实验的实现。十个实验均已完成并通过官方测试，总成绩为 `846/846`。

## 实验完成情况

| 分支 | 实验 | 官方测试 | 状态 |
| --- | --- | ---: | --- |
| `util` | Lab 1：Unix Utilities | `100/100` | 已完成 |
| `syscall` | Lab 2：System Calls | `35/35` | 已完成 |
| `pgtbl` | Lab 3：Page Tables | `46/46` | 已完成 |
| `traps` | Lab 4：Traps | `85/85` | 已完成 |
| `cow` | Lab 5：Copy-on-Write Fork | `110/110` | 已完成 |
| `thread` | Lab 6：Multithreading | `60/60` | 已完成 |
| `net` | Lab 7：Networking | `100/100` | 已完成 |
| `lock` | Lab 8：Locks | `70/70` | 已完成 |
| `fs` | Lab 9：File System | `100/100` | 已完成 |
| `mmap` | Lab 10：mmap | `140/140` | 已完成 |
| **合计** | **十个实验** | **`846/846`** | **全部完成** |

## 分支与实验报告

每个实验分支只保存该实验的实现和一份对应的 Markdown 报告：`reports/实验N报告.md`；报告引用的配图保存在 `reports/assets/`。报告目录不重复保存其他实验的报告或配图。

## 切换、运行与测试

```bash
git fetch origin
git switch <branch>

make clean
make qemu
```

退出 QEMU 后，可在 Ubuntu 中运行当前分支的官方测试：

```bash
make grade
```

## 上游项目

- MIT 6.S081 / 6.828：<https://pdos.csail.mit.edu/6.828/2021/xv6.html>
