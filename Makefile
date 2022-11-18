 NAME	= 	webserv

CC		=	c++

#FLAGS	=	-Wall -Werror -Wextra -std=c++98
FLAGS	=

SRC		=	RequestParser.cpp CGI.cpp main.cpp TCPlistener.cpp Config.cpp Location.cpp Server.cpp  Request.cpp Response.cpp

HDR		=	CGI.hpp TCPlistener.hpp Config.hpp Location.hpp Server.hpp Request.hpp RequestParser.hpp Response.hpp

OBJS	=	$(SRC:.cpp=.o)

RM		=	rm -rf

.PHONY:		all clean fclean re

all:		$(NAME)

$(NAME):	$(OBJS)
				@$(CC) $(FLAGS) $(OBJS) -o $(NAME)
				@echo "Compiled "$(NAME)" successfully!"
				@make clean

%.o:%.cpp $(HDR)
				@$(CC) $(FLAGS) -c $< -o $@
				@echo "---Compiled "$<" successfully!---"

clean:
				$(RM) $(OBJS)

fclean:		clean
				$(RM) $(NAME)

re:			fclean all