# Boost Searcher · 文档修复计划

Boost Searcher 是一个基于 C++ 的 Boost 文档全文搜索引擎，近期版本围绕“修复 Boost 文档体验”展开：我们希望通过更好的索引、摘要、链接治理与现代化 Web UI，让开发者不必在断链和老旧页面中迷失，能有更好的文档查阅体验。

---

## 项目故事


| 时间 | 事件                                                                                                                                               |
| ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| 2023 | 在 CSDN 专栏中完成第一版 Parser/Index/Searcher，全流程纯 C++ 实现。                                                                                |
| 2024 | 重新审视项目时，看到 Reddit 讨论《Is it just me, or is the quality of the Boost API documentation...》——大量开发者吐槽断链、示例缺失、语义晦涩。 |
| 2025 | 决定把原型升级为“文档修复计划”：搜集断链、补完上下文、输出更友好的 UI，并沉淀成一份可以放在 GitHub / 简历里的完整作品。                          |

---

## 愿景与动机

> 结合近期前端体验反馈：首屏“待输入/待检索”提示不一致、页脚漂浮等细节问题，进一步印证了 Boost 官方文档/导航的易用性欠佳，社区吐槽仍在持续。

### 💡 项目背景（结合社区与线上体验）

- **文档质量**：页面老旧、断链与排版混乱，比 cppreference/Qt 文档可读性差。  
- **API 可发现性**：缺少全限定名的清晰展示，整体架构难以快速理解。  
- **示例与导航**：场景化示例缺位，搜索后仍需频繁跳转或依赖 AI；前端提示/状态需更简洁一致（已在站点迭代修正“待输入”类文案、页脚位置）。

### 我们的解决方案
构建一个“查得快、看得懂、能落地”的智能 Boost 搜索体验：
- 🔍 **精准搜索**：全限定名/模糊/语义检索，权重排序与关键词高亮。  
- 📚 **示例补全**：按场景聚合可运行示例，减少二次查找。  
- 🗺️ **导航与镜像**：提供层级结构与本地镜像跳转，减少断链与等待。  
- 🤖 **智能推荐**：基于上下文的关联 API 建议，辅助继续探索。  

目标：让 Boost 文档体验接近现代化标准，降低“到处查、到处等”的时间成本。

1. **修复体验**：自动识别 Boost 官方站中的断链、老版本链接，提供跳转提示/镜像聚合。
2. **提供上下文**：在搜索结果中展示相关度、摘要、关键词、头文件/命名空间等关键信息。
3. **统一叙事**：在 README 与 `docs/` 中记录项目缘起、架构、路线图，让任何人都能快速了解并贡献。
4. **创造模板**：把这种“针对大型开源文档的修复型项目”做成可复用的范例，不止服务 Boost。

---

## 功能亮点

- **全文检索**：基于自建正排/倒排索引 + cppjieba 中文分词，支持权重排序与摘要生成。
- **现代前端**：初音绿玻璃拟态 UI，集成搜索状态条、分页 Chips、快捷关键词、关键词高亮与链接复制。
- **语义增强**：后端返回 `score/keywords/id` 等字段，为未来的语义/向量检索留接口。
- **日志与可观测性**：`tail -f` 实时追踪 server 构建索引的进度，方便部署和故障排查。

---

## 技术栈


| 模块           | 技术                                                  |
| ---------------- | ------------------------------------------------------- |
| Parser / Index | C++11、Boost.Filesystem、cppjieba、单例索引管理       |
| Web Server     | cpp-httplib、JsonCpp、pthread                         |
| 前端           | 原生 HTML/CSS/JS + jQuery 3.6，零构建部署，响应式设计 |
| 部署工具       | Makefile、`tail -f` 日志监控                          |

---

## 快速开始

```bash
sudo apt-get update
sudo apt-get install -y libboost-filesystem-dev libjsoncpp-dev

cd /root/boost_searcher
make all           # 生成 parser、server、debug

./parser           # 第一次运行需要解析 Boost 文档生成数据
./server           # 启动 Web 服务，默认监听 0.0.0.0:8081
```

浏览器访问 `http://localhost:8081`，即可使用前端界面检索 Boost 文档。
若需要后台运行并实时观察日志，可以：

```bash
./server > server.log 2>&1 &
tail -f server.log
```

---

## 系统架构

```
┌────────────┐   crawl & extract    ┌────────────┐
│  Parser    │ ───────────────────▶ │  Raw Data  │  (data/output/raw.txt)
└────────────┘                      └────────────┘
           │ build index
           ▼
┌────────────┐   inverted/forward   ┌──────────────┐
│   Index    │ ◀─────────────────── │  Searcher    │
└────────────┘                      └──────────────┘
                                             │ JSON
                                             ▼
                                      ┌──────────────┐
                                      │  HTTP Server │──▶ 前端 UI
                                      └──────────────┘
```

- Parser：递归遍历 Boost 文档目录，抽取标题 / 正文 / URL。
- Index：维护正排 + 倒排结构，标题/正文差异化权重。
- Searcher：分词、合并倒排节点、排序、生成摘要 & 关键词列表。
- Server：cpp-httplib 提供 `/s?word=` API，同时托管 `www/` 静态页面。

---

## API

`GET /s?word=<关键词>`
返回 JSON 数组，字段示例：

```json
{
  "title": "Rationale",
  "desc": "…自动提取的摘要…",
  "url": "https://www.boost.org/doc/libs/1_87_0/doc/html/lockfree/rationale.html",
  "score": 215,
  "keywords": ["atomic", "lockfree"],
  "id": 1024
}
```

---

## Roadmap（持续更新）

- [ ] Parser 阶段记录超链接可用性，前端提示“断链/镜像”。
- [ ] 为文档补充头文件、命名空间、示例链接字段，并在 UI 中展示。
- [ ] 引入 BM25/向量检索，支持语义相关词扩展。
- [ ] 文档站：使用 Docusaurus/MkDocs 承载 `docs/` 内容与 API 说明。
- [ ] 录制 Demo 视频 + 托管在线示例。

---

## 贡献指南（草案）

1. 提交 Issue / Discussions：欢迎报告断链、文档缺失、UI 建议。
2. PR 流程：
   - Parser/Index 改动请附带小型数据集复现方法。
   - 前端改动请附截图或录屏。
   - 文档贡献可直接编辑 `README` / `docs`。
3. 计划引入“AI 写作模板”帮助贡献者生成示例说明，敬请期待。

---

## 致谢

- [Reddit: Is it just me, or is the quality of the Boost API documentation...](https://www.reddit.com/r/cpp/comments/10bc7mu/is_it_just_me_or_is_the_quality_of_the_boost_api/) 提供了项目升级的契机。
- cppreference、Qt 文档等优秀项目树立了我们追求的文档标准。
- 所有参与 Boost 开发与文档的志愿者——让这个生态得以延续。
