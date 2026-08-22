# ADR 007: 代码质量 Harness（CRAP + 变异清单）

## 状态

已接受（2026-08-22）

## 背景

Phase 2 需要可重复的「好程序」判定方式，与 MinGW Qt Kit 兼容，不引入 Mull/Clang。

## 决策

采用 **方案 A**：

| 工具 | 用途 |
|------|------|
| `ENABLE_COVERAGE` + gcov | 编译插桩 |
| gcovr | 生成 JSON/HTML 覆盖率 |
| Lizard | 圈复杂度 CCN |
| `tools/crap_score.py` | CRAP = CC² × (1-cov)³ + CC |
| `docs/MUTATION_CHECKLIST.md` | Domain 层手工变异清单 |

阈值：

- CRAP > 30：需关注
- CRAP > 60：CI/本地 gate 失败（可选）

## 后果

- 与现有 GTest + MinGW 工具链一致
- 变异测试不自动化（Phase 4 再评估 Mull）
- 会话 0/5 必须跑 `tools/run_quality.ps1`
