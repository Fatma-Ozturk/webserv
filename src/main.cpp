#include "../inc/parser/Parser.hpp"

#include <iostream>
#include <string>

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "Hata" << std::endl;
        return (-1);
    }
    std::string av1;
    Parser *parser = new Parser();
    HttpScope *http;

    av1 = av[1];
    parser->parse(av1);
    http = parser->getHttp();

    std::cout << "result : " << http->getServers().at(0)->getServerName().at(0) << std::endl;
    std::cout << "result : " << http->getServers().at(0)->getHost() << std::endl;
    return (0);
}