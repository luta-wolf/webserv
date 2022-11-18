
#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>
#include "Server.hpp"

//class Server;


class Location : public Server
{
        private :
                std::string location_match;
                std::string prefix_match;
                std::vector<Location> _locations;
        public :
            Location();
            ~Location();
            Location(Location const& c);
            Location &operator=(Location const& c);
            Location(Server & c);

            std::string getLocation_match() const;
            void setLocation_match(std::string c);
            void checkMissingFields() const;
};

//#include "CGI.h"

#endif 