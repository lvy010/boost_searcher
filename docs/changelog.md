# 变更记录

> 采用“Keep a Changelog”风格，记录关键版本与里程碑。日期为近似时间，可根据实际发布调整。

## [2025-11-29] Vision Refresh
- 重写 README（故事、愿景、Roadmap）。
- 新增 docs：`vision/architecture/changelog/contribute`。
- 后端 `searcher.hpp` 输出 `score/keywords/id` 并优化摘要回退逻辑。
- 前端改版为初音绿主题：搜索状态条、分页 Chips、快速关键词、关键词高亮、复制链接按钮。
- server 启动脚本支持 `tail -f` 实时追踪索引构建。

## [2024-xx-xx] Maintenance
- 更新依赖，适配新的 Boost 文档目录。
- 优化 parser 状态机，修复部分 HTML 解析错误。

## [2023-xx-xx] 初版
- Parser/Index/Searcher/Server 全流程打通。
- 支持基本的全文检索、摘要、JSON API 与简洁前端。
- 在 CSDN 专栏首次公开记录实现过程。

---

未来发布会在此处持续更新，欢迎通过 Issue/PR 补充历史信息。***

