#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <boost/algorithm/string.hpp>

#include "cppjieba/include/cppjieba/Jieba.hpp"

namespace util
{
    class FileUtil
    {
    public:
        static bool readFile(const std::string& path, std::string& out)
        {
            std::ifstream ifs(path, std::ios::in);
            if (!ifs.is_open())
            {
                std::cerr << path << " 文件打开失败" << std::endl;
                return false;
            }

            std::string line;
            while (std::getline(ifs, line))
            {
                out += line;
            }

            ifs.close();
            return true;
        }

        static bool exists(const std::string& path)
        {
            std::ifstream ifs(path, std::ios::in);
            return ifs.is_open();
        }
    };

    class StringUtil
    {
    public:
        static void split(const std::string& target, std::vector<std::string>& result, const std::string& sep)
        {
            boost::split(result, target, boost::is_any_of(sep), boost::token_compress_on);
        }
    };

    class JiebaUtil
    {
    private:
        JiebaUtil()
            : jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
        {
            // InitStopWord(); // 注释该行代码关闭暂停词功能
        }

        JiebaUtil(const JiebaUtil&) = delete;

    public:
        static JiebaUtil* getInstance()
        {
            static JiebaUtil instance;
            return &instance;
        }


        void InitStopWord()
        {
            std::ifstream ifs(STOP_WORD_PATH);
            if(!ifs.is_open())
            {
                std::cerr << "load stop words file error" << std::endl;
                return;
            }

            std::string line;
            while(std::getline(ifs, line))
                stop_words.insert(line);

            ifs.close();
        }

        static void CutString(const std::string &src, std::vector<std::string>& out)
        {
            JiebaUtil::getInstance()->CutStringHelper(src, out);
        }

    private:
        void CutStringHelper(const std::string &src, std::vector<std::string>& out)
        {
            jieba.CutForSearch(src, out);
            auto iter = out.begin();
            while (iter != out.end())
            {
                auto it = stop_words.find(*iter);
                if(it != stop_words.end())
                    iter = out.erase(iter); //当前的string 是暂停词，需要去掉
                else
                    iter++;
            }
        }

    private:
        const char* const DICT_PATH = "./cppjieba/dict/jieba.dict.utf8";
        const char* const HMM_PATH = "./cppjieba/dict/hmm_model.utf8";
        const char* const USER_DICT_PATH = "./cppjieba/dict/user.dict.utf8";
        const char* const IDF_PATH = "./cppjieba/dict/idf.utf8";
        const char* const STOP_WORD_PATH = "./cppjieba/dict/stop_words.utf8";
        cppjieba::Jieba jieba;
        std::unordered_set<std::string> stop_words;
    };
}
