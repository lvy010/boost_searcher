#include "cpp-httplib/httplib.h"
#include "searcher.hpp"

#include <iostream>
#include <string>

static const std::string input = "data/output/raw.txt";
static const std::string root_path = "./www";

int main()
{
    searcher_ns::Searcher search;
    search.initSearcher(input);

    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());
    svr.set_mount_point("/boost", "./data/input");
    svr.Get("/s", [&search](const httplib::Request &req, httplib::Response &rsp){
                if(!req.has_param("word")){
                    rsp.set_content("请输入搜索关键字!", "text/plain; charset=utf-8");
                    return;
                }
                std::string word = req.get_param_value("word");
                //std::cout << "用户搜索：" << word << std::endl;
                std::string json_string;
                search.search(word, json_string);
                rsp.set_content(json_string, "application/json");
            });

    svr.listen("0.0.0.0", 8081);
    return 0;
}