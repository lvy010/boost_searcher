# 贡献指南

欢迎加入 Boost Searcher 文档修复计划！无论你想改进代码、补写文档、反馈断链还是贡献示例，这份指南将帮助你快速上手。

## 环境准备

1. 安装依赖：
   ```bash
   sudo apt-get update
   sudo apt-get install -y libboost-filesystem-dev libjsoncpp-dev
   ```
2. 构建：
   ```bash
   make all
   ./parser   # 首次运行生成 data/output/raw.txt
   ./server   # 启动服务
   ```
3. 推荐使用 `tail -f server.log` 或 `.cursor/.../terminals/*.txt` 观察索引构建状态。

## Issue / Discussion

- **断链/缺失信息**：请附上搜索关键词、结果 URL、状态码或截图。
- **功能建议**：描述场景、痛点、期望行为，可引用 Reddit/cppreference 等案例。
- **文档更新**：指出 README/docs 中的过时内容，或直接提交草稿。

## 提交 PR

1. Fork & 新建分支。
2. 遵循以下约定：
   - **C++**：保持 C++11 兼容；必要时引入 `<algorithm>/<unordered_set>` 等标准库；添加必要注释。
   - **前端**：纯静态资源，无构建步骤；若使用外部 CDN，请注明来源。
   - **文档**：Markdown 使用中文全角符号；引用外链请附原文说明。
3. 运行 `make all && ./parser` 确保无编译错误（可使用小型数据集）。
4. 提交 PR 时附截图 / 日志 / 测试说明。

## 推荐贡献方向

- Parser：断链检测、示例代码抓取、版本识别。
- Index/Searcher：BM25/语义检索、metadata 扩展、摘要质量优化。
- 前端：版本筛选、暗色模式、可访问性（键盘、ARIA）、断链提示 UI。
- 文档：翻译/补写 Boost 模块指南、整理常见问题、撰写使用案例。

## AI 辅助写作（草案）

- 可以使用 ChatGPT 等工具生成示例或文案，但请确保**亲自验证**代码与事实。
- 建议在 PR 中附“Prompt + 人工修订说明”，方便 reviewer 理解来源。

## 行为准则

- 尊重每一位贡献者；Boost Searcher 是社区协作项目。
- 在 Issue/PR 中保持中文或英文均可，但需表述清晰、礼貌。
- 如需引用他人内容，请明确出处。

如有任何疑问，可在 Discussions 区创建“Q&A”，或邮件联系仓库作者。期待你的加入！💚

