cmake_minimum_required(VERSION 3.21)
project(new_webserv)

set(CMAKE_CXX_STANDARD 98)

include_directories(.)

add_executable(new_webserv
        CGI.cpp
        CGI.hpp
        Config.cpp
        Config.hpp
        Location.cpp
        Location.hpp
        main.cpp
        Request.cpp
        Request.hpp
        RequestParser.cpp
        RequestParser.hpp
        Response.cpp
        Response.hpp
        Server.cpp
        Server.hpp
        TCPlistener.cpp
        TCPlistener.hpp)
