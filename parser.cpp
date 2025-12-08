#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>

#include "util.hpp"

const std::string src_path = "data/input"; // boost所有的html文件
const std::string raw_path = "data/output/raw.txt"; // 文本清洗后的文件

namespace fs = boost::filesystem;

enum class RETURNVALUE
{
    NORMAL = 0,
    ENUMFILE_ERROR,
    PARSEHTML_ERROR,
    SAVEHTML_ERROR
};

struct DocInfo
{
    std::string title;         // 文档标题
    std::string body;          // 文档内容
    std::string url;           // 指向 release 的官方链接
    std::string version;       // 原始文档版本，如 1_87_0
    std::string relative_path; // data/input 下的相对路径，供本地镜像使用
};

std::string normalizeRelative(const std::string& src)
{
    std::string relative = src;
    if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\'))
        relative.erase(relative.begin());

    std::replace(relative.begin(), relative.end(), '\\', '/');
    return relative;
}

std::string extractVersion(const std::string& relative_path)
{
    static const std::string marker = "/doc/libs/";
    auto pos = relative_path.find(marker);
    if (pos == std::string::npos)
        return "unknown";

    auto start = pos + marker.size();
    auto end = relative_path.find('/', start);
    if (end == std::string::npos || end <= start)
        return "unknown";

    return relative_path.substr(start, end - start);
}

// 把每个html文件保存到files数组
bool enumFile(const std::string& src_path, std::vector<std::string>& files)
{
    if (!fs::exists(src_path))
    {
        std::cerr << "路径 " << src_path << " 不存在!" << std::endl;
        return false;
    }

    fs::recursive_directory_iterator end; // 空的迭代器，表示结尾
    // 文件迭代器，递归迭代目录下的所有文件
    for (fs::recursive_directory_iterator it(src_path); it != end; it++)
    {
        // 这是一个常规文件，并且以 html 作为扩展名
        if (!fs::is_regular_file(*it) || it->path().extension() != ".html")
            continue;

        files.push_back(it->path().string());
    }

    return true;
}

bool parserTitle(const std::string& file, std::string& title)
{
    std::size_t begin = file.find("<title>");
    std::size_t end = file.find("</title>");

    if (begin == std::string::npos || end == std::string::npos)
        return false;

    begin += std::string("<title>").size();

    if (begin > end)
        return false;

    title = file.substr(begin, end - begin);
    return true;
}

bool parserContent(const std::string& file, std::string& content)
{
    // 状态机
    enum Status
    {
        LABLE,  // 处于网页标签
        CONTENT // 处于网页内容
    };

    enum Status s = LABLE;
    for (char ch : file)
    {
        switch(s)
        {
            case LABLE:
                if (ch == '>') s = CONTENT;
                break;
            case CONTENT:
                if (ch == '<') s = LABLE;
                else content += ch == '\n' ? ' ' : ch; // 去除 \n 字符
                break;
            default:
                break;
        }
    }

    return true;
}

bool parserUrl(const std::string& file, std::string& url, std::string& version, std::string& relative_path)
{
    std::string relative = file.substr(src_path.size());
    if (relative.empty())
        relative_path.clear();
    else
        relative_path = normalizeRelative(relative);

    std::string relative_with_slash = relative;
    if (relative_with_slash.empty() || relative_with_slash[0] != '/')
        relative_with_slash.insert(relative_with_slash.begin(), '/');

    version = extractVersion(relative_with_slash);

    const std::string release_head = "https://www.boost.org/doc/libs/release";
    if (version != "unknown")
    {
        std::string marker = "/doc/libs/" + version;
        auto pos = relative_with_slash.find(marker);
        if (pos != std::string::npos)
        {
            std::string tail = relative_with_slash.substr(pos + marker.size());
            url = release_head + tail;
            return true;
        }
    }

    url = "https://www.boost.org" + relative_with_slash;
    return true;
}

// 解析files数组，结构放到results
bool parserHtml(const std::vector<std::string>& files, std::vector<DocInfo>& results)
{
    for (auto& file : files)
    {
        std::string result;
        if (!util::FileUtil::readFile(file, result))
            continue;

        DocInfo doc;

        if (!parserTitle(result, doc.title))
            continue;

        if (!parserContent(result, doc.body))
            continue;

        if (!parserUrl(file, doc.url, doc.version, doc.relative_path))
            continue;
        
        results.emplace_back(std::move(doc));
    }

    return true;
}

// 持久化保存解析结果
bool saveHtml(const std::vector<DocInfo>& results, const std::string& raw)
{
    const char SEP = '\3';

    std::ofstream ofs(raw, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
    {
        std::cerr << "open file error!" << std::endl;
        return false;
    }

    // 格式: title \3 body \3 url \3 version \3 relative_path \n
    for (auto& result : results)
    {
        std::string outstr;
        outstr += result.title;
        outstr += SEP;
        outstr += result.body;
        outstr += SEP;
        outstr += result.url;
        outstr += SEP;
        outstr += result.version;
        outstr += SEP;
        outstr += result.relative_path;
        outstr += '\n';

        ofs.write(outstr.c_str(), outstr.size());
    }

    ofs.close();

    return true;
}

int main()
{
    std::vector<std::string> files;
    // 递归式把所有文件名保存到数组 file_list 中
    if (!enumFile(src_path, files))
    {
        std::cerr << "enum file name error!" << std::endl;
        return (int)RETURNVALUE::ENUMFILE_ERROR;
    }

    std::vector<DocInfo> results;
    // 读取所有 file_list 内容，去标签后保存在 DocInfo
    if (!parserHtml(files, results))
    {
        std::cerr << "parser html error!" << std::endl;
        return (int)RETURNVALUE::PARSEHTML_ERROR;
    }

    // 持久化保存结果
    if (!saveHtml(results, raw_path))
    {
        std::cerr << "save html error!" << std::endl;
        return (int)RETURNVALUE::SAVEHTML_ERROR;
    }
    
    return (int)RETURNVALUE::NORMAL;
} 