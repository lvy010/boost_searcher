#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "util.hpp"

namespace index_ns
{
    struct DocInfo
    {
        std::string title; // 文档标题
        std::string body;  // 文档内容
        std::string url;   // release 链接
        std::string version; // 本地文档版本
        std::string mirror_path; // data/input 下的相对路径
        bool has_mirror;   // 是否存在本地镜像
        unsigned long long id;            // 文档id
    };

    // 倒排索引节点
    struct InvertedNode
    {
        unsigned long long id;
        std::string word;
        int weight;
    };

    using invertedList = std::vector<InvertedNode>;
    
    class Index
    {
    private:
        // 单例模式
        Index() = default;
        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;

    public:

        static Index* getInstance()
        {
            static Index instance;
            return &instance;
        }

        // 通过 id 找到文档 - 正排索引
        DocInfo* getForwordIndex(unsigned long long id)
        {
            if (id >= forword_index.size())
            {
                std::cerr << "id error" << std::endl;
                return nullptr;
            }

            return &forword_index[id];
        }

        // 通过 id 找到文档 - 正排索引
        invertedList* getInvertedList(const std::string& word)
        {
            auto it = inverted_index.find(word);
            if (it == inverted_index.end())
            {
                std::cerr << "not found" << std::endl;
                return nullptr;
            }

            return &(it->second);
        }

        // 构建索引
        bool buildIndex(const std::string& path)
        {
            std::ifstream ifs(path, std::ios::in | std::ios::binary);
            if (!ifs.is_open())
            {
                std::cerr << "path error" << std::endl;
                return false;
            }

            std::string doc;
            while (std::getline(ifs, doc))
            {
                DocInfo* ret = buildForwordIndex(doc);
                if (!ret)
                {
                    std::cerr << "doc error" << std::endl;
                    continue;
                }

                buildInvertedIndex(ret);
                std::cout << "已建立 [" << ret->id << "] 个索引" << std::endl;
            }

            return true;
        }
    
    private:
        // 建立正排索引
        DocInfo* buildForwordIndex(const std::string& doc)
        {
            // 拆解doc
            std::vector<std::string> results;
            static const std::string sep = "\3";

            util::StringUtil::split(doc, results, sep);

            if (results.size() < MIN_FIELD_COUNT)
                return nullptr;

            // 填充doc到DocInfo
            DocInfo info;
            info.title = results[0];
            info.body = results[1];
            info.url = results[2];
            info.version = results.size() >= 4 ? results[3] : "unknown";
            info.mirror_path = results.size() >= 5 ? results[4] : "";
            info.has_mirror = !info.mirror_path.empty() && util::FileUtil::exists(std::string(LOCAL_DOC_ROOT) + info.mirror_path);
            info.id = forword_index.size();

            forword_index.emplace_back(std::move(info));
            return &forword_index.back();
        }

        // 构建倒排索引
        bool buildInvertedIndex(DocInfo* doc)
        {
            struct WordCount
            {
                int title;
                int content;

                WordCount()
                    : title(0)
                    , content(0)
                {}
            };

            std::unordered_map<std::string, WordCount> word_map; // 词频表

            // 标题统计
            std::vector<std::string> title_word;
            util::JiebaUtil::CutString(doc->title, title_word);

            for (auto& word : title_word)
            {
                boost::to_lower(word);
                word_map[word].title++;
            }

            // 内容统计
            std::vector<std::string> content_word;
            util::JiebaUtil::CutString(doc->body, content_word);

            for (auto& word : content_word)
            {
                boost::to_lower(word);
                word_map[word].content++;
            }

            // 处理倒排索引节点
            for (auto& word_pair : word_map)
            {
                InvertedNode node;
                node.id = doc->id;
                node.word = word_pair.first;
                node.weight = word_pair.second.title * TITLE_POWER + word_pair.second.content * CONTENT_POWER;
                inverted_index[word_pair.first].emplace_back(std::move(node)); // 插入倒排索引
            }

            return true;
        }

    private:  
        static const int TITLE_POWER = 10;
        static const int CONTENT_POWER = 1;
        static constexpr const char* LOCAL_DOC_ROOT = "data/input/";
        static const int MIN_FIELD_COUNT = 3;
        std::vector<DocInfo> forword_index; // 正排索引
        std::unordered_map<std::string, invertedList> inverted_index; // 倒排索引
    };
}