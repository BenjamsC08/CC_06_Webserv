CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98 -g3
NAME = webserv
SRCS = srcs/CGI/CgiProcess_exec_arrays.cpp srcs/CGI/cgiCtx_utils.cpp srcs/CGI/CgiProcess.cpp srcs/CGI/CgiProcess_spawn.cpp srcs/CGI/CgiCtx.cpp srcs/CGI/CgiProcess_setters_getters.cpp srcs/CGI/CgiProcess_exec_fail_pipe.cpp srcs/CGI/CgiProcess_stdin.cpp srcs/CGI/CgiProcess_error.cpp srcs/CGI/CgiProcess_fd_utils.cpp srcs/CGI/CgiProcess_stdout.cpp srcs/CGI/CgiProcess_non_blocking.cpp srcs/CGI/CgiResponseParser.cpp srcs/Logging/Logging.cpp srcs/Parsing/Config.cpp srcs/Parsing/HttpRules.cpp srcs/Parsing/ServConf.cpp srcs/Server/Client_CGI.cpp srcs/Server/Client.cpp srcs/Server/Response.cpp srcs/Server/Request.cpp srcs/Server/Server.cpp srcs/WebServ.cpp srcs/signal.cpp srcs/main.cpp
INCLUDE_PATH = -Isrcs -Isrcs/Parsing -Isrcs/Logging -Isrcs/CGI -Isrcs/Server

OBJ_DIR = obj/
OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRCS:.cpp=.o)))

VALID = \033[1;32m
NOTVALID = \033[1;31m
LOADING = \033[1;33m
INFOS = \033[1;36m
RESET = \033[0m

VPATH = srcs:srcs/Logging:srcs/Parsing:srcs/Server:srcs/CGI

all: $(NAME)

$(OBJ_DIR)%.o: %.cpp | $(OBJ_DIR)
	$(CC) $(FLAGS) $(INCLUDE_PATH) -MMD -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(INCLUDE_PATH) -o $(NAME)

clean:
	rm -f $(OBJ) obj/*/*.d obj/*.d

fclean: clean
	rm -rf $(NAME) .cache

re: fclean all

listC:
	@LIST=$$(find -wholename "./srcs/*.cpp" | cut -c 3- | tr '\n' ' '); \
	printf "🗅 $(INFOS)copy all CPP sources files into clipboard :\n$(RESET)$$LIST\n"; \
	echo -n $$LIST | wl-copy

-include $(OBJ:.o=.d)
