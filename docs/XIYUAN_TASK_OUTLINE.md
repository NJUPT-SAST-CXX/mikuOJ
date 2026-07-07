# 席源负责模块任务大纲

## 1. 负责范围

席源主要负责两个模块：

- Task 5：Sandbox Core
- Task 8：Compiler（多语言编译 / 解释执行）

这两个模块位于判题流水线的中间层，连接上层的 CLI / Problem / Judge 调度逻辑，以及下层的 Namespace、Cgroup、Seccomp、Language Runtime 等基础能力。

## 2. Sandbox Core 模块大纲

### 2.1 模块定位

Sandbox Core 是一次用户程序运行任务的总控编排层。

它负责接收整理后的运行配置，选择正式沙箱后端，调用 namespace、cgroup、seccomp 等底层隔离与限制能力，执行用户程序，并收集退出状态、资源用量和错误信息，最终返回统一的运行结果。

### 2.2 主负责文件

- `include/cppjudge/sandbox.h`
- `include/cppjudge/sandbox_internal.h`
- `src/sandbox_common.cpp`
- `src/sandbox_linux.cpp`

### 2.3 核心职责

- 定义沙箱运行接口和运行结果。
- 根据配置选择沙箱后端。
- 准备一次运行所需的目录、文件和挂载信息。
- 调用 Namespace Manager 创建隔离环境。
- 调用 Cgroup Manager 设置 CPU、内存、进程数等资源限制。
- 调用 Seccomp Manager 安装系统调用过滤策略。
- 配置标准输入、标准输出和标准错误。
- 在子进程中执行目标程序。
- 等待子进程结束。
- 收集退出码、信号、时间、内存和错误信息。
- 清理运行过程中创建的临时资源。

### 2.4 核心流程

```text
CLI / Judge
  -> 构造 RunConfig
  -> Sandbox Core
  -> 选择 sandbox 后端
  -> 准备 rootfs / mount / stdio
  -> 创建 namespace
  -> 配置 cgroup
  -> 安装 seccomp
  -> exec 用户程序
  -> wait 子进程
  -> 收集 RunResult
  -> 返回给 Judge
```

### 2.5 与其他模块的边界

#### CLI Frontend

CLI 负责解析命令行参数、读取题目路径和提交文件路径、选择语言与输出 JSON。Sandbox Core 不负责命令行解析，只接收已经整理好的运行配置。

#### Problem / Judge

Problem / Judge 层负责加载题目、遍历测试点、调用编译和运行流程、比较输出并聚合结果。Sandbox Core 只负责安全地运行单个程序，不负责答案比较。

#### Namespace Manager

Namespace Manager 负责底层 Linux namespace 的创建和隔离细节。Sandbox Core 负责在合适的阶段调用它。

#### Cgroup Manager

Cgroup Manager 负责 cgroup v2 的创建、资源限制写入、统计读取和清理。Sandbox Core 负责把运行限制传递给它，并把资源统计写入运行结果。

#### Seccomp Manager

Seccomp Manager 负责系统调用白名单策略的加载与安装。Sandbox Core 负责保证 seccomp 在子进程 `exec` 前正确接入。

#### Language Manager

Language Manager 负责语言识别、编译器 / 解释器路径、编译参数、运行参数、运行时依赖和挂载清单。Sandbox Core 使用这些信息，但不直接维护每种语言的全部细节。

## 3. Compiler 模块大纲

### 3.1 模块定位

Compiler 模块负责把用户提交转换成可运行目标。

编译型语言需要在受限环境中完成编译；解释型语言跳过编译阶段，直接进入运行阶段。

### 3.2 主负责文件

- `include/cppjudge/compiler.h`
- `src/compiler.cpp`

### 3.3 强相关文件

- `include/cppjudge/language.h`
- `src/language.cpp`
- `include/cppjudge/sandbox.h`
- `src/sandbox_common.cpp`
- `src/sandbox_linux.cpp`

### 3.4 核心职责

- 接收用户提交路径和语言配置。
- 根据 Language Manager 提供的信息判断是否需要编译。
- 对 C++、C、Java、Go、Rust 等编译型语言构造编译命令。
- 通过 Sandbox Core 在沙箱内执行编译命令。
- 检查编译器退出状态。
- 检查编译产物是否生成。
- 收集编译错误信息。
- 返回统一的 `CompileResult`。
- 对 Python3 等解释型语言跳过编译，只准备后续运行所需信息。

### 3.5 编译流程

```text
Judge / CLI
  -> Compiler
  -> 查询 Language Manager
  -> 判断语言是否需要编译
  -> 构造编译命令
  -> 调用 Sandbox Core 执行编译
  -> 检查退出状态和产物
  -> 返回 CompileResult
```

## 4. 当前代码对应关系

当前代码中，Sandbox Core 主要落在：

- `sandbox_common.cpp`：负责公共后端选择逻辑。
- `sandbox_linux.cpp`：负责 Linux 环境下的沙箱执行编排。
- `sandbox.h`：定义对外沙箱接口。
- `sandbox_internal.h`：定义内部后端工厂接口。

Compiler 主要落在：

- `compiler.h`：定义编译接口。
- `compiler.cpp`：实现编译流程、解释型语言跳过编译、编译产物检查和错误信息收集。

## 5. 个人任务拆分

### 任务一：Sandbox Core 接口整理

目标：

- 明确沙箱模块对外暴露的统一接口。
- 保证上层只依赖 `Sandbox` 接口和 `RunResult`，不直接依赖底层 namespace、cgroup、seccomp 细节。

产出：

- 清晰的沙箱接口说明。
- 运行配置与运行结果字段说明。

阶段一具体工作：

- 整理 `SandboxRequest` 的字段含义，明确它是 Compiler / Judge 传给 Sandbox Core 的单次运行请求。
- 整理 `SandboxResult` 的字段含义，明确它是 Sandbox Core 返回给上层的统一运行结果。
- 整理 `SandboxBackend` 的接口契约，明确 `execute()` 负责一次沙箱执行，`name()` 用于日志和诊断。
- 整理 `make_sandbox()` 的后端选择规则，明确当前正式后端类型为 `auto`、`linux-ns`、`nsjail`。
- 整理内部辅助结构 `RawOutcome`，说明它负责承接 wait status、signal、cgroup 统计等底层观测量。
- 明确判决推导边界：Sandbox Core 负责 AC/TLE/MLE/OLE/RE/SV/SE 等运行侧结果，WA 由 Comparator 在之后判断。

阶段一不做的事情：

- 不改 CLI 参数行为。
- 不改 Judge 判题流程。
- 不改 Language Manager 的语言配置策略。
- 不重构 Namespace / Cgroup / Seccomp Manager。
- 不调整测试语义。

### 任务二：Linux 沙箱执行流程维护

目标：

- 维护 `linux-ns` 后端的核心运行流程。
- 保证 rootfs、mount、stdio、namespace、cgroup、seccomp、exec、wait、result collect 的顺序正确。

产出：

- 可用于判题运行的 Linux 沙箱执行流程。
- 对异常路径返回明确错误信息。

阶段二具体工作：

- 梳理 `src/sandbox_linux.cpp` 中父进程和子进程的职责分工。
- 明确父进程负责 cgroup 生命周期、同步管道、wait 循环、资源采样、超时处理和结果归一化。
- 明确子进程负责 stdio 重定向、rootfs 搭建、进入 `/box`、设置 rlimit、等待 cgroup attach、降权、安装运行期 seccomp、最终 `execve`。
- 明确 cgroup attach-before-exec 门控：子进程 setup 完成后暂停，父进程 attach 到 cgroup 后再放行，保证用户程序运行前资源限制已生效。
- 明确 seccomp 安装顺序：运行阶段在 `execve` 前安装，且位于子进程 setup 的最后阶段。
- 明确临时资源清理边界：cgroup、同步管道、临时 rootfs 由父进程负责回收。

阶段二不做的事情：

- 不改 namespace、mount、cgroup、seccomp 的底层实现算法。
- 不改变现有运行结果判定优先级。
- 不改变 CLI 参数和 JSON 输出格式。
- 不改变多语言配置策略。
- 不引入新的沙箱后端。

### 任务三：资源限制和安全策略接入

目标：

- 在 Sandbox Core 中正确接入 cgroup v2 和 seccomp。
- 保证资源限制和 syscall 限制在用户程序运行前生效。

产出：

- CPU / 内存 / 进程数限制接入。
- seccomp 白名单策略接入。
- 运行结束后的资源统计收集。

阶段三具体工作：

- 梳理 Sandbox Core 与 Cgroup Manager 的调用边界。
- 明确 Cgroup Manager 负责底层 cgroup v2 文件操作，包括创建 cgroup、写入 `memory.max`、写入 `memory.swap.max=0`、写入 `pids.max`、attach 子进程、读取统计和销毁 cgroup。
- 明确 Sandbox Core 负责 cgroup 的生命周期时机，包括 `create -> apply -> attach -> collect -> destroy`。
- 明确 CPU 时间限制当前由 Sandbox Core 在 wait 循环中采样 `cpu.stat` 并主动 kill，而不是依赖 `cpu.max` 硬节流。
- 明确内存超限通过 `memory.max`、`memory.current` / `memory.peak` 和 `memory.events` 判断。
- 明确进程数限制通过 `pids.max` 防止 fork 炸弹。
- 梳理 Sandbox Core 与 Seccomp Manager 的调用边界。
- 明确 Seccomp Manager 负责把语言 profile 转换为 syscall 白名单，并通过 libseccomp 安装过滤器。
- 明确当前运行阶段会在子进程 `execve` 前安装 seccomp，默认拒绝未白名单 syscall，违规映射为 `SV`。
- 明确当前编译阶段没有启用 seccomp，但仍受 namespace、cgroup、降权和最小文件系统约束；`SeccompManager::install(profile, true)` 中的编译白名单属于后续可启用能力。

阶段三不做的事情：

- 不改变 cgroup v2 的底层写入逻辑。
- 不改变现有 CPU / 内存 / 进程数判定语义。
- 不扩大或收缩 seccomp syscall 白名单。
- 不把编译期 seccomp 从“预留能力”改成“实际启用”。
- 不改变任何语言的 runtime profile。

### 任务四：Compiler 沙箱化编译

目标：

- 编译型语言通过 Sandbox Core 执行编译。
- 解释型语言跳过编译阶段。

产出：

- 多语言编译流程。
- 编译错误信息收集。
- 编译产物检查。

阶段四具体工作：

- 梳理 Compiler、Language Manager、Sandbox Core 三者之间的边界。
- 明确 Language Manager 是语言配置来源，提供源码落地名、编译器路径、编译参数、编译环境、解释器路径、运行参数、运行时挂载依赖和编译期资源限制。
- 明确 Compiler 负责把原始提交复制到 `work_dir`，并根据 Language Manager 的配置决定是否需要编译。
- 明确 C++、C、Java、Go、Rust 等编译型语言会构造 `SandboxRequest`，通过 Sandbox Core 在沙箱内执行编译器。
- 明确 Python3 等解释型语言跳过编译阶段，只返回解释器路径和脚本参数，后续运行仍由 Sandbox Core 负责。
- 明确编译成功条件：Sandbox 返回 `AC`、编译器退出码为 0、目标 artifact 存在。
- 明确编译失败时收集 `compile_stdout.txt` 和 `compile_stderr.txt`，作为 `CompileResult::output` 返回。
- 明确 Sandbox Core 自身故障不能伪装成 CE，必须作为 SE 返回。

阶段四不做的事情：

- 不新增语言。
- 不改变现有语言的编译参数。
- 不改变编译期资源限制。
- 不改变 Judge 的测试点遍历和答案比较逻辑。
- 不把 Language Manager 的配置职责混入 Compiler。

### 任务五：集成验证与交付说明

目标：

- 验证 Sandbox Core 和 Compiler 在完整判题流水线中的行为。
- 输出适合课程 / 项目交付的中文说明。

产出：

- 单元测试结果。
- 集成测试结果。
- 个人负责模块说明。
- 风险和后续扩展建议。

阶段五具体工作：

- 在 VM / WSL2 Linux 环境中执行构建验证，确认当前代码能够通过 CMake 编译。
- 执行单元测试，覆盖 Comparator、Problem、Language、Logger、Seccomp、Cgroup、Namespace 等基础模块。
- 在具备 root 权限时执行 integration 测试，覆盖 AC、WA、CE、TLE、MLE、OLE、RE、SV、SE、floating 和 all_languages。
- 输出个人负责模块交付说明，说明 Sandbox Core 和 Compiler 的职责、接口边界、当前实现状态和后续风险。
- 明确测试结果不能伪造；如果 integration 或 security 因 sudo、namespace、cgroup 权限不足无法执行，需要如实说明。

阶段五不做的事情：

- 不合并 PR。
- 不修改团队主仓库。
- 不把未运行的测试写成已通过。
- 不把安全测试结果和普通集成测试结果混淆。

当前阶段五验证状态：

- `cmake --build build -j`：已在 VM 通过。
- `ctest --test-dir build -L unit --output-on-failure`：已在 VM 通过，53 个单元测试全部通过，其中 3 个因环境条件按预期跳过。
- `sudo ctest --test-dir build -L integration --output-on-failure`：已在 VM 通过，11 个集成测试全部通过。
- `ctest -L security`：未执行，属于手动安全测试套件。

## 6. 汇报用总结

本人负责 mikuOJ 判题系统中的 Sandbox Core 与 Compiler 模块。Sandbox Core 位于 CLI / Judge 层和底层隔离机制之间，负责一次用户程序运行任务的生命周期编排，包括运行配置接收、沙箱后端选择、运行环境准备、namespace 隔离、cgroup 资源限制、seccomp 系统调用过滤、子进程执行、状态等待、资源统计和统一结果返回。Compiler 模块负责根据 Language Manager 提供的语言配置，在沙箱环境中执行编译型语言的编译流程，并对解释型语言跳过编译阶段，最终向 Judge 层提供统一的编译结果。
