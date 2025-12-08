#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>

#include <jsoncpp/json/json.h>

#include "util.hpp"
#include "index.hpp"

namespace searcher_ns
{
    struct InvertedNodePrint
    {
        long long id;
        int weight;   // 总权重
        std::vector<std::string> words; // 所有命中词

        InvertedNodePrint()
            : id(0)
            , weight(0)
        {}
    };

    class Searcher
    {
    public:
        void initSearcher(const std::string& input)
        {
            index = index_ns::Index::getInstance();
            std::cout << "开始建立索引..." << std::endl;
            index->buildIndex(input);
            std::cout << "建立索引结束..." << std::endl;
        }

        void search(const std::string& query, std::string& result)
        {
            // 对查询 query 分词
            std::vector<std::string> words;
            util::JiebaUtil::CutString(query, words);

            // 获取倒排拉链
            std::unordered_map<long long, InvertedNodePrint> token_map;

            for (std::string word : words)
            {
                boost::to_lower(word);

                index_ns::invertedList* inverted_list = index->getInvertedList(word);
                if (inverted_list == nullptr)
                    continue;


                for (auto& node : *inverted_list)
                {
                    auto& token = token_map[node.id];
                    token.id = node.id;
                    token.weight += node.weight; // 所有id相同的倒排节点,权重加和
                    token.words.push_back(node.word);
                }
            }

            std::vector<InvertedNodePrint> inverted_list_all;
            std::transform(token_map.begin(), token_map.end(), std::back_inserter(inverted_list_all),
                            [](const std::pair<const long long, InvertedNodePrint>& pair) {
                                return pair.second;
                            });

            // 权重排序
            auto comp = [](const InvertedNodePrint &e1, const InvertedNodePrint &e2){
                            return e1.weight > e2.weight; };

            std::sort(inverted_list_all.begin(), inverted_list_all.end(), comp);

            // 序列化
            Json::Value root;
            for (auto& item : inverted_list_all)
            {
                index_ns::DocInfo* doc = index->getForwordIndex(item.id); // 获取正排拉链
                if (doc == nullptr)
                    continue;
                if (item.words.empty())
                    continue;

                Json::Value keyword_array(Json::arrayValue);
                std::string anchor_word = item.words[0];
                std::unordered_set<std::string> uniq;
                for (auto& keyword : item.words)
                {
                    if (keyword.empty())
                        continue;
                    if (uniq.insert(keyword).second)
                    {
                        if (anchor_word.empty())
                            anchor_word = keyword;
                        keyword_array.append(keyword);
                    }
                }

                Json::Value elem;
                elem["title"] = doc->title;
                elem["desc"] = getDesc(doc->body, anchor_word); // 摘要
                elem["url"] = doc->url;
                elem["score"] = item.weight;
                elem["id"] = static_cast<Json::UInt64>(doc->id);
                elem["keywords"] = keyword_array;
                elem["version"] = doc->version.empty() ? "unknown" : doc->version;
                elem["mirror"] = doc->mirror_path.empty() ? "" : "/boost/" + doc->mirror_path;
                elem["link_status"] = doc->has_mirror ? "mirror_available" : "remote_only";
                root.append(elem);
            }

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            result = Json::writeString(builder, root);
        }

        std::string getDesc(const std::string& body, const std::string& word)
        {
            if (word.empty())
                return fallbackDesc(body);

            //找到word在body中的首次出现，然后往前找50字节(如果没有，从begin开始)，往后找100字节(如果没有，到end)

            //1. 找到首次出现
            auto iter = std::search(body.begin(), body.end(), word.begin(), word.end(), [](int x, int y){
                    return (std::tolower(x) == std::tolower(y)); // 忽略大小写
                    });

            if(iter == body.end())
                return fallbackDesc(body);

            int pos = std::distance(body.begin(), iter);

            //2. 获取start，end
            int start = 0; 
            int end = body.size() - 1;

            //如果之前有50+字符，就更新开始位置
            if(pos > start + prev_step) 
                start = pos - prev_step;

            if(pos + next_step < end) 
                end = pos + next_step;

            //3. 截取子串,return
            if(start >= end) return fallbackDesc(body);
            std::string desc = body.substr(start, end - start);
            desc += "...";
            return desc;
        }

    private:
        index_ns::Index* index;
        std::string fallbackDesc(const std::string& body) const
        {
            static const size_t preview = 180;
            if (body.empty())
                return "该文档暂无可展示内容";

            if (body.size() <= preview)
                return body;

            return body.substr(0, preview) + "...";
        }

        static const int prev_step = 50;
        static const int next_step = 100;
    };
}