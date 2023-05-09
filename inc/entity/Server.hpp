#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "ErrorPage.hpp"
#include "Location.hpp"
#include "IScope.hpp"
#include "Variable.hpp"


class ErrorPage;
class Location;
class Server : public IScope
{
    private:
        std::vector<Location> _locations; //location sinifi gelecek
        std::string _port;// listen(string)[port(8080)]
        std::string _host; // listen(string)[host:port(127.0.0.1:80)]
        std::string _serverNames;
        std::string _root; // root ./tests/test1/;
        std::string _index;
        std::string _cgi_pass;

        bool _isServerNameNothing;
        ErrorPage _errorPage;
        DataBase<Variable<std::string> > _keywordDatabase;

    public:
        Server(/* args */);
        ~Server();

        void setPort(std::string port);
        void setHost(std::string host);
        void setErrorPage(ErrorPage errorPage);
        void setServerName(std::string serverName);
        void setRoot(std::string root);
        void setIndex(std::string index);
        void setCgi_pass(std::string cgi_pass);

        void setLocation(Location location);
        void setIsServerNameNothing(bool isServerNameNothing);
        void setKeywordDatabase(DataBase<Variable<std::string> > keywordDatabase);

        std::string getPort();
        std::string getHost();
        ErrorPage getErrorPage();
        std::vector<std::string> getServerName();
        std::string getRoot();
        std::string getIndex();
        std::string getCgi_pass();

        std::vector<Location> getLocations();
        bool getIsServerNameNothing();
        DataBase<Variable<std::string> > getKeywordDataBase();

        void keywordFill();
        
        std::string getName() const;
        Server* clone() const;

};

