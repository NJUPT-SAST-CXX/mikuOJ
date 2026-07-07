# 席源负责模块交付说明

## 1. 模块名称

Sandbox Core 与 Compiler。

## 2. 模块定位

Sandbox Core 是 mikuOJ 判题系统中的沙箱执行编排层，位于 CLI / Judge 层与 Namespace、Cgroup、Seccomp、Language Runtime 等底层模块之间。

Compiler 是编译阶段调度层，位于 Judge / CLI 与 Sandbox Core 之间，负责根据 Language Manager 的配置准备源码、执行编译或跳过编译，并返回后续运行阶段所需的信息。

## 3. 主负责代码范围

Sandbox Core 主负责文件：

- `include/cppjudge/sandbox.h`
- `include/cppjudge/sandbox_internal.h`
- `src/sandbox_common.cpp`
- `src/sandbox_linux.cpp`

Compiler 主负责文件：

- `include/cppjudge/compiler.h`
- `src/compiler.cpp`

强相关但非单独主责文件：

- `include/cppjudge/language.h`
- `src/language.cpp`
- `include/cppjudge/cgroup_manager.h`
- `src/cgroup_manager.cpp`
- `include/cppjudge/seccomp_manager.h`
- `src/seccomp_manager.cpp`
- `include/cppjudge/ns_manager.h`
- `src/ns_manager.cpp`

## 4. Sandbox Core 核心职责

Sandbox Core 负责一次沙箱运行任务的生命周期编排：

```text
接收 SandboxRequest
  -> 选择 linux-ns / nsjail 安全后端
  -> 准备 work_dir、mount、stdio
  -> clone 隔离子进程
  -> 设置 rootfs / namespace
  -> 父进程 attach cgroup
  -> 子进程降权
  -> 运行阶段安装 seccomp
  -> execve 用户程序或编译器
  -> 父进程 wait / 采样资源
  -> 收集退出码、信号、时间、内存、输出大小
  -> 返回 SandboxResult
```

Sandbox Core 不负责：

- 解析 CLI 参数。
- 加载题目。
- 比较答案。
- 维护每种语言的完整配置。
- 直接实现 cgroup / seccomp / namespace 的所有底层细节。

## 5. Compiler 核心职责

Compiler 负责把用户提交准备成可运行目标：

```text
接收 source_file、Language、work_dir
  -> 查询 Language Manager
  -> 复制源码到 work_dir
  -> 如果是解释型语言：跳过编译，返回解释器运行信息
  -> 如果是编译型语言：构造 SandboxRequest
  -> 调用 Sandbox Core 在沙箱内执行编译器
  -> 读取编译器输出
  -> 检查编译产物
  -> 返回 CompileResult
```

编译成功条件：

- Sandbox 返回 `AC`。
- 编译器退出码为 0。
- 目标 artifact 存在。

如果 Sandbox Core 自身失败，则返回 `SE`，不能伪装成 `CE`。

## 6. 与其他模块的边界

### CLI / Judge

CLI / Judge 负责参数解析、题目加载、测试点遍历、调用 Compiler / Sandbox Core、比较输出和聚合 JSON 结果。

Sandbox Core 只负责安全运行单个命令。

### Language Manager

Language Manager 负责语言检测和语言配置，包括：

- 源码落地名。
- 编译器路径。
- 编译参数。
- 编译环境变量。
- 解释器路径。
- 运行参数。
- seccomp profile。
- mount 依赖。
- 编译期资源限制。

Compiler 和 Sandbox Core 消费这些配置，但不直接维护每种语言的全部策略。

### Cgroup Manager

Cgroup Manager 负责 cgroup v2 底层文件操作。

Sandbox Core 负责调用时机：

```text
create -> apply -> attach -> collect -> destroy
```

当前 CPU 时间限制由 Sandbox Core 采样 `cpu.stat` 并主动 kill；内存和进程数通过 `memory.max`、`memory.swap.max=0`、`pids.max` 限制。

### Seccomp Manager

Seccomp Manager 负责 profile 到 syscall 白名单的转换和过滤器安装。

当前正式运行路径中，运行阶段会在子进程 `execve` 前安装 seccomp。编译阶段目前没有启用 seccomp，但仍受 namespace、cgroup、降权和最小文件系统约束。

### Namespace Manager

Namespace Manager 负责底层 Linux namespace、rootfs、mount、pivot_root 和最小设备节点等能力。

Sandbox Core 负责在子进程中按正确顺序调用这些能力。

## 7. 当前实现状态

已实现：

- `linux-ns` 安全沙箱后端。
- `auto / linux-ns / nsjail` 后端类型入口。
- SandboxRequest / SandboxResult 统一接口。
- 编译型语言在沙箱内编译。
- 解释型语言跳过编译。
- cgroup v2 内存、swap、进程数限制。
- CPU 时间采样与主动 kill。
- 运行阶段 seccomp 白名单接入。
- 多语言基础支持：C++、C、Python3、Java、Go、Rust。
- 单元测试和集成测试入口。

部分实现或后续可扩展：

- 独立 libsandbox 分层仍可继续拆分。
- Namespace Manager 与 Sandbox Core 的边界还可以进一步模块化。
- 编译阶段 seccomp 白名单已有代码能力，但当前 Linux 后端没有启用。
- Security 测试需要单独手动执行并确认环境权限。

## 8. 阶段验证结果

已执行：

```bash
cmake --build build -j
ctest --test-dir build -L unit --output-on-failure
sudo ctest --test-dir build -L integration --output-on-failure
```

结果：

- `cmake --build build -j`：通过。
- `ctest --test-dir build -L unit --output-on-failure`：通过，53 个单元测试全部通过，其中 3 个因环境条件按预期跳过。
- `sudo ctest --test-dir build -L integration --output-on-failure`：通过，11 个集成测试全部通过。

未执行：

- `ctest -L security`。该项属于手动安全测试套件，需要单独授权和环境确认。

## 9. 个人负责内容总结

本人负责 mikuOJ 判题系统中的 Sandbox Core 与 Compiler 模块。Sandbox Core 位于 CLI / Judge 层和底层隔离机制之间，负责一次用户程序运行任务的生命周期编排，包括运行配置接收、沙箱后端选择、运行环境准备、namespace 隔离、cgroup 资源限制、seccomp 系统调用过滤、子进程执行、状态等待、资源统计和统一结果返回。Compiler 模块负责根据 Language Manager 提供的语言配置，在沙箱环境中执行编译型语言的编译流程，并对解释型语言跳过编译阶段，最终向 Judge 层提供统一的编译结果。
