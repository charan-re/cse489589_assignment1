#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>




int main(){

        int udppingsock;
        struct addrinfo hints, *server;
        struct sockaddr_in my_ip;
        char s[INET_ADDRSTRLEN];
        memset (&hints, 0 , sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;


        if((x=getaddrinfo("8.8.8.8", "53", &hints, &server))!=0){
                return 1;
        }
        for ( server; server!=NULL; server = server->ai_next){
        udppingsock = socket (AF_INET, SOCK_DGRAM,0);
        connect (udppingsock, server->ai_addr, server ->ai_addrlen);
        }
  
	bzero(&my_ip, sizeof(my_ip));
        int len = sizeof(my_ip);
        getsockname(udppingsock, (struct sockaddr *) &my_ip,(socklen_t *) &len);
        inet_ntop(AF_INET, &my_ip.sin_addr, s, sizeof(s));

        printf("Local ip address: %s\n", s);

	return 1;
}
