#include "../inc/server/Client.hpp"

Client::Client(Server *sv, HttpScope *Hscope)
{
	this->_response = "";
	this->sv = sv;
	this->multi = 0;
	this->status = 0;
	this->ContentLen = 0;
	this->_parser = NULL;
	this->_request = NULL;
	this->_http = Hscope;
	this->response = NULL;
	this->locationIndex = 0;
	this->_response = "";
	this->_host = sv->getHost();
	this->_port = sv->getPort();
    this->isFav = 0;
}

/* Client::Client() {};

Client::Client(const Client &client)
{
	*this = client;
}

Client& Client::operator=(const Client &client)
{
	if (this == &client)
        return (*this);
	this->_response = client._response;
	this->sv = client.sv;
	this->multi = client.multi;
	this->status = client.status;
	this->postLen = client.postLen;
	this->postVal = client.postVal;
	this->_parser = client._parser;
	this->_request = client._request;
	this->_http = client._http;
	this->response = client.response;
	this->locationIndex = client.locationIndex;
	this->_response = client._response;
	this->_host = client._host;
	this->_port = client._port;
    this->_isFav = client._isFav;
	return (*this);
}

Client::~Client() {}*/

int	Client::getStatus() const
{
	return this->status;
}

int	Client::getMulti() const
{
	return this->multi;
}

int	Client::getIsFav() const
{
	return this->isFav;
}

std::string		Client::getMethod() const
{
	return this->method;
}

size_t			Client::getContentLen() const
{
	return this->ContentLen;
}

std::string		Client::getBody() const
{
    return this->body;
}

void   Client::setRequest(Request *request)
{
    this->_request = request;
}

void   Client::setMulti(int multi)
{
    this->multi = multi;
}

void   Client::setStatus(int status)
{
    this->status = status;
}

void   Client::setMethod(std::string method)
{
    this->method = method;
}

void   Client::setContentLen(int ContentLen)
{
    this->ContentLen = ContentLen;
}

void   Client::setIsFav(int isFav)
{
    this->isFav = isFav;
}

void   Client::setBody(std::string body)
{
    this->body = body;
}


void   Client::setParserRequest(std::string buffer)
{
    this->_parser = new ParserRequest();
    this->_parser->parse(buffer);
    setStatus(this->_parser->getStatus());
    setMulti(this->_parser->getMulti());
    setRequest(this->_parser->getRequest());
    setMethod(this->_request->getHttpMethodName());
	setContentLen(this->_request->getContentLength());
    setBody(this->_request->getBody());
    if (this->_request->getPath() == "/favicon.ico")
        this->isFav = 1;
    delete _parser;
}

std::string    Client::process(std::string multiBody)
{
    std::cout << RED << "Processing" << RESET << std::endl;
    ServerScope     *matchedServer;
    LocationScope   *matchedLocation;
    this->response = new Response();

    matchedServer = this->getServerForRequest();
    this->getLocationForRequest(matchedServer, _request->getPath());

    if (matchedServer == NULL || this->locationIndex == -1)
    {
        _clientException.run(300);
        return NULL;
    }
    matchedLocation = matchedServer->getLocations().at(this->locationIndex);
    this->_response = this->response->createResponse(_request, matchedServer, matchedLocation, multiBody);
    if (this->_response == "")
    {
        delete this->response;
        delete this->_request;
        return NULL;
    }
    delete this->response;
    delete this->_request;
    return _response;
}

ServerScope*        Client::getServerForRequest()
{
    std::vector<ServerScope *>  serverScope;

    serverScope = this->_http->getServers();
    for (std::vector<ServerScope *>::const_iterator it = serverScope.begin() ; it != serverScope.end(); it++)
    {
        if (this->_host == (*it)->getListen().host && this->_port == (*it)->getListen().port)
            return(*it);
    }
    _clientException.run(301);
    return NULL;
}


//benim yazdığım daha basic olan
void  Client::getLocationForRequest(ServerScope *matchedServerScope, const std::string& path) 
{
    this->locationIndex = 0;

    this->locationIndex = getMatchLocationPathIndex(matchedServerScope, path);
    if(this->locationIndex == -1)
    {
        _clientException.run(302);
    }
}
