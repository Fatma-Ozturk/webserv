#include "../inc/entity/Response.hpp"

Response::Response() {}

Response::~Response() {}

std::string		Response::getResponse()
{
    return (this->_response);
}

int	Response::getStatusCode()
{
    return (this->statusCode);
}

void			Response::setAllowMethods(std::vector<std::string> methods)
{
	std::vector<std::string>::iterator it = methods.begin();

	while (it != methods.end())
	{
		this->_allow_methods += *(it++);

		if (it != methods.end())
			this->_allow_methods += ", ";
	}
}

void			Response::setContentLength()
{
	_contentLength = std::to_string(this->_response.size());
}

void			Response::setContentType()
{
	if (this->_type != "")
	{
		_contentType = this->_type;
		return ;
	}
	this->_type = this->_path.substr(this->_path.rfind(".") + 1, this->_path.size() - this->_path.rfind("."));
	this->_contentType = _httpContentType.contentTypeGenerator(this->_type);
	// if (this->_type == "html")
	// 	_contentType = "text/html";
	// else if (this->_type == "css")
	// 	_contentType = "text/css";
	// else if (this->_type == "js")
	// 	_contentType = "text/javascript";
	// else if (this->_type == "jpeg" || this->_type == "jpg")
	// 	_contentType = "image/jpeg";
	// else if (this->_type == "png")
	// 	_contentType = "image/png";
	// else if (this->_type == "bmp")
	// 	_contentType = "image/bmp";
	// else
	// 	_contentType = "text/plain";
}

void			Response::setDate()
{
	char			buffer[100];
	struct timeval	tv;
	struct tm		*tm;

	gettimeofday(&tv, NULL);
	tm = gmtime(&tv.tv_sec);
	strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
	_date = std::string(buffer);
}

void			Response::setLastModified()
{
	char			buffer[100];
	struct stat		stats;
	struct tm		*tm;

	if (stat(this->_path.c_str(), &stats) == 0)
	{
		tm = gmtime(&stats.st_mtime);
		strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
		_lastModified = std::string(buffer);
	}
}

void			Response::setLocation()
{
	if (this->statusCode == 201 || this->statusCode / 100 == 3)
	{
		_location = this->_path;
	}
}

void			Response::setRetryAfter()
{
	if (this->statusCode == 503 || this->statusCode == 429 || this->statusCode == 301)
	{
		_retryAfter = std::to_string(3);
	}
}

void			Response::setServer(void)
{
	_server = "webserv";
}

void			Response::setTransferEncoding(void)
{
	_transferEncoding = "identity";
}


void			Response::setValues()
{
	setContentLength();
	setContentType();
	setDate();
	setLastModified();
	setLocation();
	setRetryAfter();
	setServer();
	setTransferEncoding();
}

void	Response::setIndexs(std::vector<std::string> _locationIndex, std::vector<std::string> _serverIndex)
{
	//_locationIndex ve _serverIndex ayrı ayrı vector olarak al
	for (std::vector<std::string>::iterator it = _locationIndex.begin(); it != _locationIndex.end(); it++)
		this->_indexs.push_back(*it);
	for (std::vector<std::string>::iterator itt = _serverIndex.begin(); itt != _serverIndex.end(); itt++)
		this->_indexs.push_back(*itt);
}

/* void    Response::setParams(std::vector<std::string> _paramKeyword, std::vector<std::string> _paramValue)
{
	int i = 0;
	if (_paramValue.size() == _paramKeyword.size())
	{
		for (std::vector<std::string>::iterator it = _paramValue.begin(); it != _paramValue.end(); it++)
		{
			this->_cgi_params[_paramKeyword.at(i)] = *it;
			i++;
		}
	}
} */

void Response::setLanguage(std::vector<std::pair<std::string, float> > languages)
{

	for (size_t i = 0; i < languages.size(); i++)
	{
		if (!this->_contentLanguage.empty()) {
            this->_contentLanguage += ", ";
        }
        this->_contentLanguage += languages[i].first;	
	}
	
}

std::string Response::selectIndex()
{
	for(std::vector<std::string>::iterator it = this->_indexs.begin(); it != this->_indexs.end(); it++){
		if (!pathIsFile(*it))
			return(*it);
	}
	std::cerr << RED << "No index found" << RESET << std::endl;
    return (0);
}

void    Response::createResponse(Request *request, ServerScope *server, LocationScope *location)
{
	this->_error_page = location->getErrorPage();
	this->_allow = request->getHttpMethodName();
	this->statusCode = request->getReturnCode();//statusCode 200 olarak initledik. İlk 200 olarak atanacak.
	this->_cgi_pass = location->getCgiPass();
  	setIndexs(location->getIndex(), server->getIndex());
	//setParams(location->getParamKeyword(), location->getParamValues());
  	this->_contentLocation = selectIndex();
	this->_path = location->getRoot() + this->_contentLocation;//this->_path = "./tests/test1/index.html";
	setLanguage(request->getAcceptLanguages());
	std::cout << YELLOW << "_LANGUAGE : " << this->_contentLanguage << RESET << std::endl;
	std::cout << YELLOW << "_cgi_pass : " << this->_cgi_pass << RESET << std::endl;
	std::cout << YELLOW << "_contentLocation : " << this->_contentLocation << RESET << std::endl;
	std::cout << YELLOW << "_path : " << this->_path << RESET << std::endl;

    if (std::find(location->getAllowMethods().begin(), location->getAllowMethods().end(), this->_allow) == location->getAllowMethods().end())
		this->statusCode = 405;
	else if (atoi(location->getClientBodyBufferSize().c_str()) < static_cast<int>(request->getBody().size()))
		this->statusCode = 413;

    if (this->statusCode == 405 || this->statusCode == 413)
	{
		_response = notAllowed(location->getAllowMethods()) + "\r\n";
		return ;
	}

    if (this->statusCode == 200)
    {
        if (request->getHttpMethodName() == "GET")
            GETmethod(request, server);
        else if (request->getHttpMethodName() == "POST")
            POSTmethod(request, server);
        else if (request->getHttpMethodName() == "DELETE")
            DELETEmethod();
        else
            std::cerr << "buraya ne eklememiz lazım bilemedim" << std::endl;   
    }
}

std::string		Response::writeHeader(void)
{
	std::string	header = "";

	if (_allow_methods != "")
		header += "Allow: " + _allow_methods + "\r\n";
	if (_contentLanguage != "")
		header += "Content-Language: " + _contentLanguage + "\r\n";
	if (_contentLength != "")
		header += "Content-Length: " + _contentLength + "\r\n";
	if (_contentLocation != "")
		header += "Content-Location: " + _contentLocation + "\r\n";
	if (_contentType != "")
		header += "Content-Type: " + _contentType + "\r\n";
	if (_date != "")
		header += "Date: " + _date + "\r\n";
	if (_lastModified != "")
		header += "Last-Modified: " + _lastModified + "\r\n";
	if (_location != "")
		header += "Location: " + _location + "\r\n";
	if (_retryAfter != "")
		header += "Retry-After: " + _retryAfter + "\r\n";
	if (_server != "")
		header += "Server: " + _server + "\r\n";
	if (_transferEncoding != "")
		header += "Transfer-Encoding: " + _transferEncoding + "\r\n";
	header += "\r\n";
	return (header);
}


std::string		Response::notAllowed(std::vector<std::string> methods)
{
	std::string	header;

	_response = "";

	setValues();
	setAllowMethods(methods);

	if (this->statusCode == 405)
		header = "HTTP/1.1 405 Method Not Allowed\r\n";
	else if (this->statusCode == 413)
		header = "HTTP/1.1 413 Payload Too Large\r\n";
	header += writeHeader();

	return (header);
}

void			Response::GETmethod(Request* request, ServerScope* server)
{
	if (this->_cgi_pass != "")
	{
		Cgi	cgi(request, server, this->_path);
		size_t		i = 0;
		size_t		j = _response.size() - 2;

		_response = cgi.executeCgi(this->_cgi_pass);
		while (_response.find("\r\n\r\n", i) != std::string::npos || _response.find("\r\n", i) == i)
		{
			std::string	str = _response.substr(i, _response.find("\r\n", i) - i);
			if (str.find("Status: ") == 0)
				this->statusCode = std::atoi(str.substr(8, 3).c_str());
			else if (str.find("Content-type: ") == 0)
				_type = str.substr(14, str.size());
			i += str.size() + 2;
		}
		while (_response.find("\r\n", j) == j)
			j -= 2;
		_response = _response.substr(i, j - i);
	}
	else if  (this->statusCode == 200)
		readContent();
	else
		_response = this->readHtml();
	
	if (this->statusCode == 500)
		_response = "<!DOCTYPE html>\n<html><title>500</title><body>Server couldn't handle your request. Still, you won't kill it so easily !</body></html>\n";

	_response = getHeader() + "\r\n" + _response;
}

void			Response::DELETEmethod()
{
	_response = "";
	if (pathIsFile(_path))
	{
		if (remove(_path.c_str()) == 0)
			this->statusCode = 204;
		else
			this->statusCode = 403;
	}
	else
		this->statusCode = 404;
	if (this->statusCode == 403 || this->statusCode == 404)
		_response = this->readHtml();
	_response = getHeader() + "\r\n" + _response;
}

void			Response::POSTmethod(Request* request, ServerScope* server)
{
	if (this->_cgi_pass != "")
	{
		Cgi	cgi(request, server, this->_path);
		size_t		i = 0;
		size_t		j = _response.size() - 2;

		_response = cgi.executeCgi(this->_cgi_pass);
		while (_response.find("\r\n\r\n", i) != std::string::npos || _response.find("\r\n", i) == i)
		{
			std::string	str = _response.substr(i, _response.find("\r\n", i) - i);
			if (str.find("Status: ") == 0)
				this->statusCode = std::atoi(str.substr(8, 3).c_str());
			else if (str.find("Content-Type: ") == 0)
				_type = str.substr(14, str.size());
			i += str.size() + 2;
		}
		while (_response.find("\r\n", j) == j)
			j -= 2;

		_response = _response.substr(i, j - i);
	}
	else
	{
		this->statusCode = 204;
		_response = "";
	}
	if (this->statusCode == 500)
		_response = "<!DOCTYPE html>\n<html><title>500</title><body>Server couldn't handle your request. Still, you won't kill it so easily !</body></html>\n";
	_response = getHeader() + "\r\n" + _response;
}

std::string		Response::readHtml()
{
	std::ofstream		file;
	std::stringstream	buffer;
	if (this->statusCode == 403)
		return ("<!DOCTYPE html>\n<html><title>403</title><body>This request cannot be authorized (invalid permissions or credentials)</body></html>\n");
	if (pathIsFile(this->_error_page))
	{
		file.open(this->_error_page.c_str(), std::ifstream::in);
		if (file.is_open() == false)
			return ("<!DOCTYPE html>\n<html><title>40404</title><body>There was an error finding your error page</body></html>\n");

		buffer << file.rdbuf();
		file.close();
		_type = "text/html";

		return (buffer.str());
	}
	else
		return ("<!DOCTYPE html>\n<html><title>404</title><body>There was an error finding your error page</body></html>\n");
}

void				Response::readContent()
{
	std::ifstream		file;
	std::stringstream	buffer;

	_response = "";

	if (pathIsFile(_path))
	{
		file.open(_path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			this->statusCode = 403;
			_response = "<!DOCTYPE html>\n<html><title>403</title><body>This request cannot be authorized (invalid permissions or credentials)</body></html>\n";
			return ;
		}
		buffer << file.rdbuf();
		_response = buffer.str();
		std::cout << YELLOW << "_response :" << _response << RESET << std::endl;
		file.close();
		return ;
	}
	this->statusCode = 404;
	_response = this->readHtml();
	return ;
}


std::string		Response::getHeader()
{
	std::string	header;

	setValues();

	header = "HTTP/1.1 " + std::to_string(this->statusCode) + " " + _httpStatusCode.getByStatusCode(this->statusCode).getValue() + "\r\n";
	header += writeHeader();

	return (header);
}