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
#define INIT_MEMBER 3

struct Price{
	int owner;
	int price;
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

int get_winner(struct Price p[]) {
	int min = p[0].price;
	int index = 0;
	for(int i = 0; i < INIT_MEMBER; i++) {
		if(p[i].price < min) {
			min = p[i].price;
			index = i;
		}
	}
	return index;
}

int main(int argc, char *argv[]) {
	int sock, valread, valsend, turn, udp_port, i;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};
	char buffer_rc[BUFFER_SIZE] = {0};

	for (i = 0; i < 5; i++)
	 	argv[1][i] = '0';

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		write(1, "Socket creation error\n", 23);
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		write(1, "Connection Failed\n", 19);
		return -1;
	}
	write(1, "Connection to server was successful!\n", 38);
	//choose annouce
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	//project id
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	write(1, "\n", 2);
	//add announce
	memset(buffer, 0, sizeof(buffer));
	read(0, buffer, BUFFER_SIZE);
	send(sock, buffer, BUFFER_SIZE, 0);
	
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	// activity announce
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);

	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	write(1, "\n", 2);
	udp_port = atoi(buffer);
	
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	
	memset(buffer, 0, sizeof(buffer));
	if(valread = recv(sock, buffer, BUFFER_SIZE , 0) < 0) {
        write(1, "error read\n", 12);
    }
    write(1, buffer, BUFFER_SIZE);
	write(1, "\n", 2);
	
	turn = atoi(buffer);
	sleep(2);
	
	// broadcast
	int broadcast_sock, winner, full_round = 0, round = 0, broadcast = 1, opt = 1;
	struct sockaddr_in brc_addr;
	struct Price prices[INIT_MEMBER];
	fd_set fds;
	char brc_buffer[BUFFER_SIZE] = {0};
	memset(brc_buffer, 0, sizeof(brc_buffer));

	broadcast_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(broadcast_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
	setsockopt(broadcast_sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast));
	brc_addr.sin_family = AF_INET;
	brc_addr.sin_port = htons(udp_port);
	brc_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

	bind(broadcast_sock, (struct sockaddr *)&brc_addr, sizeof(brc_addr));
	
	if(turn == 1)
		write(1, "Please enter your price:\n", 26);
	else
		write(1, "Please wait. You're not allowed to suggest a price now.\n", 57);

	while(1) {
		FD_ZERO(&fds);
		FD_SET(broadcast_sock, &fds);
		FD_SET(0, &fds);
		select(broadcast_sock + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(broadcast_sock, &fds)) {
			recvfrom(broadcast_sock, brc_buffer, BUFFER_SIZE, 0, NULL, 0);
			if(turn == 1){
				write(1, "Your price registered.\n", 24);
				turn = 0;
			}
			else {
				turn--;
				if(turn == 1) {
					write(1, "Please enter your price:\n", 26);
				}
				else {
					write(1, "new price suggested. please wait ...\n", 38);
				}
			}
			memset(brc_buffer, 0, sizeof(brc_buffer));
		}
		if(FD_ISSET(0, &fds)) {
			read(0, brc_buffer, BUFFER_SIZE);
			if(turn == 1) {
				prices[round].owner = broadcast_sock;
				prices[round].price = atoi(brc_buffer);
				round++;
				sendto(broadcast_sock, brc_buffer, sizeof(brc_buffer), 0, (struct sockaddr *) &brc_addr, sizeof(brc_addr));
			}
			memset(brc_buffer, 0, sizeof(brc_buffer));
		}
	}

	return 0;
}
