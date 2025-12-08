# 架构说明

## 总览

```
                ┌────────────┐
                │ Boost Docs │  (HTML, 多版本)
                └─────┬──────┘
                      │ crawl & clean
                ┌─────▼──────┐
                │  Parser    │ 解析标题/正文/URL/依赖关系
                └─────┬──────┘
                      │ write
                ┌─────▼──────┐
                │  Raw Data  │ data/output/raw.txt
                └─────┬──────┘
           build idx  │
                ┌─────▼──────┐
                │   Index    │ forward + inverted + metadata
                └─────┬──────┘
                      │ search
                ┌─────▼──────┐
                │  Searcher  │ 分词、拉链合并、排序、摘要
                └─────┬──────┘
                      │ JSON
                ┌─────▼──────┐
                │   Server   │ cpp-httplib + 静态资源
                └─────┬──────┘
                      │ HTTP
                ┌─────▼──────┐
                │   Frontend │ 初音绿 UI、分页、状态条
                └────────────┘
```

## 模块细节

### Parser (`parser.cpp`)
- 递归遍历 `Boost` 文档目录（支持离线文件）。
- 状态机解析 `<title> / <body>`，抽取 URL、正文纯文本。
- TODO：为每个文档抓取 `<code>` 示例链接、头文件、命名空间、版本信息，并检测外链可用性。

### Index (`index.hpp`)
- 维护 `forword_index`（正排）与 `inverted_index`（倒排）。
- `DocInfo` 目前包含 `title/body/url/id`，计划扩展 `header`, `namespace`, `version`, `examples`.
- 权重：`title * 10 + body * 1`，支持进一步引入 BM25/TF-IDF。

### Searcher (`searcher.hpp`)
- 通过 `util::JiebaUtil` 对查询切词并统一小写。
- 倒排结果聚合为 `InvertedNodePrint`，计算总权重并排序。
- 生成 JSON：`title / desc / url / score / id / keywords`，摘要通过上下文截取并高亮关键字。
- TODO：支持多关键词摘要、语义相似词扩展。

### Server (`server.cpp`)
- `cpp-httplib` 提供 HTTP API 与静态文件托管。
- API：`GET /s?word=` 返回 JSON；静态目录 `www/` 提供前端。
- 日志输出“索引构建进度”，可通过 `tail -f` 实时查看。

### 前端 (`www/index.html`)
- 纯静态页面：初音绿渐变背景、玻璃拟态卡片、分页、Chip 控件、关键词高亮。
- 状态条展示“检索中/成功/失败”，并支持复制链接。
- TODO：版本筛选、断链提示、暗色模式、键盘无障碍操作。

## 数据文件

| 路径 | 内容 |
| --- | --- |
| `data/output/raw.txt` | 解析后的行式数据：`title \3 body \3 url` |
| `cppjieba/dict/*` | 分词词典资源 |
| `www/` | 前端静态资源 |
| `docs/` | 愿景、架构、贡献等文档 |

## 部署建议

1. **本地/服务器**：使用 `make all && ./parser && ./server`；通过 `tmux`/`systemd` 守护。
2. **日志**：重定向 `server` 输出，或直接 tail `.cursor/.../terminals/*.txt`。
3. **CI/CD（计划）**：
   - 解析阶段运行在定时任务，产出数据快照。
   - Server 镜像化（Docker）+ 前端托管（GitHub Pages / Netlify）。

## 扩展方向

- **断链检测**：Parser 阶段使用 `libcurl` 或 `boost::asio` 发起 HEAD 请求，记录状态码。
- **语义检索**：在正排中缓存文档向量，结合 sqlite/faiss 等实现语义召回。
- **多版本索引**：支持 `boost 1.70+` 多版本共存，用户可选择目标版本。
- **监控**：暴露 Prometheus 指标，如“解析文档数”“断链数”“查询延迟”。

如需更深入的模块说明，可继续扩展本文件或拆分子章节。欢迎在 `docs/contribute.md` 中了解如何补充文档。***

