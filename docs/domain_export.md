# Domain 静态库导出（无 Qt）

Phase 4 Session 3：`FleetSimCore` / `FleetSimDomain` 可 install + `find_package`，并提供无 Qt smoke 可执行文件。

## 同树构建 smoke（推荐本地验证）

主工程 Configure 时保持默认即可（仍会找 Qt 编 UI），额外目标：

```
# Qt Creator / CMake 勾选或命令行：
-DFLEETSIM_BUILD_DOMAIN_SMOKE=ON
```

构建目标 `fleet_domain_smoke` 后运行：应打印 `domain_smoke ok pose=(...)`。

该目标 **只链接** `FleetSimDomain` + `FleetSimCore`，不链接 Qt / rclcpp。

## install + find_package（外部工程）

```bash
cmake --build <build-dir> --target install
# 或在 Qt Creator 中执行 Install 步骤（前缀自定）
```

外部 `CMakeLists.txt` 示例：

```cmake
cmake_minimum_required(VERSION 3.19)
project(consumer LANGUAGES CXX)

find_package(FleetSimDomain REQUIRED)  # 需 CMAKE_PREFIX_PATH 指向安装前缀

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE FleetSim::FleetSimDomain)
```

安装产物包含：

- `lib/cmake/FleetSimDomain/FleetSimDomainConfig.cmake`
- `lib/cmake/FleetSimDomain/FleetSimDomainTargets.cmake`
- Core / Domain 头文件与静态库

## 注意

- 主工程默认仍 `find_package(Qt6)`——无 Qt 环境编完整 UI 需另开纯 Domain 超集工程（后续可拆）。
- smoke 证明 Domain **链接**不依赖 Qt；不等于主 `CMakeLists.txt` 可在无 Qt 机器上完整 Configure UI。
- Windows：安装路径勿含异常权限；中文路径请用户本地执行，Agent 不代跑。
