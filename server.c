#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#define BUFFER_SIZE 256
#define INIT_CLIENT 5
#define INIT_PROJECT 3
#define INIT_MEMBER 3
#define INIT_LISTEN 7
#define UDP_PORT 8888

struct Project {
	int id;
	int valid;
	int group_mem[INIT_MEMBER];
};

int count_digit(int n) {
    if (n >= 0 && n <= 9)
        return 1;

    int len = 0;
    while (n != 0) {
		len++;
		n /= 10;
	}
    return len;
}

char* to_string(int n) {
	int len = count_digit(n);
	int rem = 0;
	char* str = (char*) malloc((len + 1) * sizeof(char));
	memset(str, 0, sizeof(str));
	for (int i = 0; i < len; i++) {
		rem = n % 10;
		str[len - i - 1] = rem + '0';
		n = n / 10;
	}
    return str;
}

char* join_str(char* s1, char* s2) {
    int index = strlen(s1);
    int len = strlen(s1) + strlen(s2);
    char* str = (char*) malloc((len + 1) * sizeof(char));
    memset(str, 0, sizeof(str));
	for (int i = 0; i < strlen(s1); i++) {
		str[i] = s1[i];
	}
    
    if(strlen(s1) != 0) {
        str[strlen(s1)] = ' ';
        index = strlen(s1) + 1;
    }

    for (int i = 0; i < strlen(s2); i++) {
        index += i;
		str[index] = s2[i];
	}
    return str;
}

void announce_winner(int client_fd) {
	write(1, "Client with fd ", 15);
	char* c_fd = to_string(client_fd);
	write(1, c_fd, strlen(c_fd));
	write(1, " wins the project!\n", 20);
}

void add_member(int client_fd, int proj_id, struct Project proj[]){
	int member = 0;
	for(int i = 0; i < INIT_MEMBER; i++) {
		if(proj[proj_id - 1].group_mem[i] == 0){
			proj[proj_id - 1].group_mem[i] = client_fd;
			member = i + 1;
		if(member == INIT_MEMBER)
			proj[proj_id - 1].valid = 0;
		send(client_fd, "You are added to the project group now.\n", 41,0);
		break;
		}
	}
}

void start_activity(int proj_id, int udp_port, struct Project p[]) {
	char* p_id = to_string(proj_id);
	char* port = to_string(udp_port);
	for(int i = 0; i < INIT_MEMBER; i++) {
		send(p[proj_id - 1].group_mem[i], "Group activity of project ", 26, 0);
		send(p[proj_id - 1].group_mem[i], p_id, strlen(p_id), 0);
		sleep(1);
		send(p[proj_id - 1].group_mem[i], " started on port ", 17, 0);
		sleep(1);
		send(p[proj_id - 1].group_mem[i], port, strlen(port), 0);
		sleep(2);
		send(p[proj_id - 1].group_mem[i], "Your turn is ", 13, 0);
		sleep(0.5);
		send(p[proj_id - 1].group_mem[i], to_string(i+1), 1, 0);
	}
}

int main(int argc, char *argv[]) {
	int serv_sock, new_socket, valread, port, max_sd,i, sd,
	activity, full, udp_port =UDP_PORT, client_size = INIT_CLIENT, opt = 1;
	char* str_id;
	char* str_fd;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	char buffer[BUFFER_SIZE] = {0};
	fd_set fds;
	int *client_sock;
	struct Project projects[INIT_PROJECT];

	for (i = 0; i < INIT_PROJECT; i++) {
		projects[i].id = i + 1;
		projects[i].valid = 1;
		memset(projects[i].group_mem, 0, sizeof(projects[i].group_mem));
	}
	
	client_sock = (int *)malloc(INIT_CLIENT * sizeof(int));
	for (i = 0; i < INIT_CLIENT; i++)
		client_sock[i] = 0;

	if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < 5; i++)
	 	argv[1][i] = '0';

	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[1]);
	char *port_num = to_string(port);
	write(1, "Waiting for connection on port ", 31);
	write(1, port_num, strlen(port_num));
	write(1, "...\n", 5);
	
	if (listen(serv_sock, INIT_LISTEN) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while(1) {
		FD_ZERO(&fds);
		FD_SET(serv_sock, &fds);
		max_sd = serv_sock;
		for ( i = 0 ; i < client_size ; i++) {
			sd = client_sock[i];
			if(sd > 0)
				FD_SET(sd, &fds);
			if(sd > max_sd)
				max_sd = sd;
		}

		activity = select( max_sd + 1 , &fds , NULL , NULL , NULL);
		if ((activity < 0) && (errno!=EINTR)) {
			write(1, "select error\n", 14);
		}

		if (FD_ISSET(serv_sock, &fds)) {
			if ((new_socket = accept(serv_sock, (struct sockaddr *)&addr, (socklen_t*)&addrlen))<0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			write(1, "New connection! socket fd is: ", 30);
			char *client_fd = to_string(new_socket);
			write(1, client_fd, strlen(client_fd));
			write(1, "\n", 2);

			char* proj_list = "";
			for(i = 0; i < INIT_PROJECT; i++) {
				if(projects[i].valid == 1) {
					str_id = to_string(projects[i].id);
        			proj_list = join_str(proj_list, str_id);
				}
			}
			send(new_socket, "Please choose your project id:\n", 32, 0);
			sleep(0.5);
			send(new_socket, proj_list, strlen(proj_list), 0);
			
			full = 1;
			for(i = 0; i < client_size; i++) {
				if (client_sock[i] == 0) {
					full = 0;
					client_sock[i] = new_socket;
					break;
				}
			}
			if(full) {
				client_size++;
				client_sock = realloc(client_sock, client_size * sizeof(int));
				client_sock[client_size - 1] = new_socket;
			}
		}

		for (i = 0; i < client_size; i++) {
			if (FD_ISSET(client_sock[i], &fds)) {
				valread = read(client_sock[i], buffer, BUFFER_SIZE);
				if (valread == 0) {
					write(1, "Client with fd ", 15);
					str_fd = to_string(client_sock[i]);
					write(1, str_fd, strlen(str_fd));
					write(1, " disconnected\n", 15);
					close(client_sock[i]);
					client_sock[i] = 0;
				}
				else {
					int project_id = atoi(buffer);
					if(projects[project_id - 1].valid == 0){
						announce_winner(client_sock[i]);
					}
					else {
						add_member(client_sock[i], project_id, projects);
						str_id = to_string(project_id);
						str_fd = to_string(client_sock[i]);
						write(1, "Client with fd ", 15);
						write(1, str_fd, strlen(str_fd));
						write(1, " chooses project ", 17);
						write(1, str_id, strlen(str_id));
						write(1, "\n", 2);
						if(projects[project_id - 1].valid == 0) {
							start_activity(project_id, udp_port, projects);
							udp_port++;
						}
					}
				}
			}
		}
	}
	return 0;
}
