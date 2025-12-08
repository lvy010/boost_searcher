#include "searcher.hpp"

#include <iostream>
#include <string>

static const std::string input = "data/output/raw.txt";

int main()
{
    searcher_ns::Searcher* search = new searcher_ns::Searcher();
    search->initSearcher(input);

    std::string query;
    std::string json_str;
    while (true)
    {
        std::cout << "enter: ";
        std::getline(std::cin, query);
        search->search(query, json_str);

        std::cout << json_str << std::endl;
    }

    return 0;
} 