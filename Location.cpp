
#include "Location.hpp"
#include "Server.hpp"

 Location::Location() {}

 Location::~Location() {
//     std::cout << "deleteLocation\n";
 }

 Location::Location(Location const& c) {*this = c;}
        
 Location::Location(Server & c)
 {
    this->root = c.getRoot();
    this->_allowed_methods = c.getAllowedMethods();
    this->_index = c.getIndex();
    this->_clinet_max_body_size = c.getClientMaxBodySize();
    this->_autoindex = c.getAutoIndex();
    this->redirect_uri = c.getRedirectUri();
    this->errorPages = c.getErrorPages();
    this->cgiPath = c.getCgiPath();
    this->uploadPath = c.getUploadPath();
 }

Location &Location::operator=(Location const& c)
{
    location_match = c.location_match;
    prefix_match = c.prefix_match;
    this->root = c.root;
    this->_allowed_methods = c._allowed_methods;
    this->_index = c._index;
    this->_clinet_max_body_size = c._clinet_max_body_size;
    this->_autoindex = c._autoindex;
    this->redirect_uri = c.redirect_uri;
    this->errorPages = c.errorPages;
    this->cgiPath = c.cgiPath;
    this->uploadPath = c.uploadPath;
    return *this;
}

std::string Location::getLocation_match() const { return location_match;}

void Location::setLocation_match(std::string c) { location_match = c;}

void Location::checkMissingFields() const
{
    if (uploadPath.empty())
    {
        throw std::runtime_error("uploadPath is missing");
    }
    if (redirect_uri.empty())
    {
        throw std::runtime_error("redict path is missing");
    }
    if (_allowed_methods.size() == 0)
    {
        throw std::runtime_error("methods are missing");
    }
    if (_index.size() == 0)
    {
        throw std::runtime_error("index is missing");
    }
}