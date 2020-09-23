/**
 * @charanre_assignment1
 * @author  Charan Reddy Bodennagari <charanre@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"
using namespace std;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

struct block_list{
        char IPA[INET_ADDRSTRLEN];
        struct block_list *next;
        };

struct buffer_msg{
	char message[BUFFER_SIZE];
	struct buffer_msg *msglist;
	};
 
struct ClientList{
	int portno;
	int fd;
	int fd_client;
	int msgs_sent;
	int msgs_recv;
	char IP[INET_ADDRSTRLEN];
	char FQDN[128];
	vector <string> blocked;
	//char vblockedips[4][INET_ADDRSTRLEN];
	struct block_list blockedips;
	int login;
	};

char your_ubit_name[32] = "charanre";

void Author(){
	cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
        cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", your_ubit_name);
	cse4589_print_and_log("[%s:END]\n", "AUTHOR");
}

void printlist(char * command, std::vector<struct ClientList> *clientlist){
	vector<struct ClientList>::iterator i;
        cse4589_print_and_log("[%s:SUCCESS]\n", command);
        int j =1;
        for( i = clientlist->begin();i!=clientlist->end();i++){
		if(i->login ==1){
        		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",j , i->FQDN, i->IP, i->portno);
                	j++;
		}
        }
        cse4589_print_and_log("[%s:END]\n", command);


}

void getip(char* command, char * ip){
	char * server_IP = (char*) malloc (INET_ADDRSTRLEN);
	int cmd_len = strlen(command);
	char * command_new;
	for (int j = 0; j< cmd_len; j++){
               if(command[j]==' '){
                        ip[j]='\0';
                       break;
               }
               else{
                       ip[j] = command[j];
               }
       }
	//return server_IP;
}
int main(int argc, char * argv[]){
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/


	if(argc != 3) {
		printf("Usage:%s [port]\n", argv[0]);
		exit(-1);
	}
	//if(strcmp(argv[1], "s")==0){
	int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	bool login = false;
	fd_set master_list, watch_list;
	vector<struct ClientList> clientlist;
	vector<struct ClientList> :: iterator myit;
	vector<string>myblocklist;
	vector<string>::iterator stritr;
	
	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;
	int yes = 1;
	/* Fill up address structures */
	//if(strcmp(argv[1], "s")==0){
	if(validatenumber(argv[2])!=-1){
	if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
		perror("getaddrinfo failed");
	
	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(server_socket < 0)
		perror("Cannot create socket");
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	/* Bind */
	if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
		perror("Bind failed");

	freeaddrinfo(res);
	
	/* Listen */
	if(listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");
	}
	//create_connection(&server_socket, argv[2]);	
	//cout<<"server_socket" << server_socket<<endl;
	/* ---------------------------------------------------------------------------- */
	
	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	
	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);
	
	head_socket = server_socket;



	if(strcmp(argv[1], "s")==0){
		while(TRUE){
		memcpy(&watch_list, &master_list, sizeof(master_list));
		
		printf("\n[PA1-Server@CSE489/589]$ ");
		fflush(stdout);
		
		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(selret < 0)
			perror("select failed.");
//		cout<<"select returned "<<selret<<endl;
		/* Check if we have sockets/STDIN to process */
		if(selret > 0){
			/* Loop through socket descriptors to check which ones are ready */
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){
				char  sock_index_char[2];
				 sprintf(sock_index_char,"%d",sock_index);		
				if(FD_ISSET(sock_index, &watch_list)){
					/* Check if new command on STDIN */
					if (sock_index == STDIN){
						char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
						memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) /*Mind the newline character that will be written to cmd*/
							exit(-1);
						int cmd_size = sizeof(cmd);
						int cmd_len = strlen(cmd);
						if(cmd[cmd_len-1] == '\n'){
							cmd[cmd_len-1] = '\0';
							cmd_len = strlen(cmd);
						}
						/*Process PA1 commands here ...*/
						char *command = (char*) malloc(sizeof(char)*CMD_SIZE);
						memset(command, '\0', CMD_SIZE);
						char *command_new = (char*) malloc(sizeof(char)*CMD_SIZE);
						for (int j = 0; j< cmd_len; j++){
							//char *command = (char*) malloc(sizeof(char)*CMD_SIZE);
							//string command[cmd_len];
							if(cmd[j]==' '){
								command_new = cmd+j+1;
								cmd= command_new;
								break;
							}
							else{
								//	strcat(command,cmd[j].c_str());
								command[j] = cmd[j];
							}


						}
						if(strcmp(command,"AUTHOR")==0){
							Author();	
						}
						if (strcmp(command,"IP")==0){
							getipaddress(command);
						}
						else if (strcmp(command,"PORT")==0){
							validateport(command,argv[2]);
						}
						else if (strcmp(command, "LIST")==0){
							printlist(command, &clientlist);
						}
						else if (strcmp(command,"BLOCKED")==0){
							if(validateip(command_new)==false||verifyIP(command_new,&clientlist)==false){
							cse4589_print_and_log("[%s:ERROR]\n", command);
							cse4589_print_and_log("[%s:END]\n", command);
							break;
							}
							displayblocked(&clientlist, command_new);
						}
						else if (strcmp(command , "STATISTICS")==0){
							vector<struct ClientList>::iterator i;
							int j = 1;
        						cse4589_print_and_log("[%s:SUCCESS]\n", command);
                                                	for( i = clientlist.begin();i!=clientlist.end();i++){
								const char *loginstatus =i->login?"logged-in":"logged-out";
								cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n",j,i->FQDN,i->msgs_sent,i->msgs_recv,loginstatus);
								j++;
							}
        						cse4589_print_and_log("[%s:END]\n", command);
						}
						else if (strcmp(command ,"REFRESH")==0){
							/*refresh_list(,&clientlist);*/
						}
						
						//free(cmd);
					}

					/* Check if new client is requesting connection */
					else if(sock_index == server_socket){
						//accept_connection
						int fdaccept= accept_connection(server_socket,&clientlist);
						FD_SET(fdaccept, &master_list);
						if(fdaccept > head_socket) head_socket = fdaccept;
//						cout<<"remote host connected with sockfd "<<fdaccept<<endl;
					}
					/* Read from existing clients */
					else{
						/* Initialize buffer to receieve response */
						char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
						char *buffer_orig = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
						memset(buffer, '\0', BUFFER_SIZE*2);
						memset(buffer_orig, '\0', BUFFER_SIZE*2);
						char *command = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
                                                char *command_new = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
						memset(command, '\0', BUFFER_SIZE*2);
						memset(command_new, '\0', BUFFER_SIZE*2);
						if(recv(sock_index, buffer, BUFFER_SIZE*2, 0) <= 0){
							close(sock_index);
							printf("Remote Host terminated connection!\n");
							delete_fd(sock_index, &clientlist);		
							/* Remove from watched list */
							FD_CLR(sock_index, &master_list);
						}

						else {
							/*Process incoming data from existing clients here ...*/
							for(myit = clientlist.begin();myit!=clientlist.end();myit++){
								if(myit->fd == sock_index){
									myit->msgs_sent++;
									break; 
								}	
							}
//							printf("ECHOing it back to the remote host ... ");
							int cmd_len = strlen(buffer);
                                                	for (int j = 0; j< cmd_len; j++){
                                                        if((buffer[j]==' ')||(buffer[j]=='\0')){
                                                                command_new = buffer+j+1;
                                                                buffer= command_new;
								command[j] = '\0';
                                                                break;
                                                        }
                                                        else{
                                                                command[j] = buffer[j];
                                                        }
                                               		}		
							if(strcmp(command,"REFRESH")==0){
								char * list = (char *) malloc (BUFFER_SIZE);
								memset(list,'\0', BUFFER_SIZE);
								refresh_list_server(sock_index,&clientlist,list);
							}
							else if(strcmp(command,"SEND")==0){
								server_send(sock_index,buffer,&clientlist);
							}
							else if(strcmp(command,"LOGIN")==0){
								for(myit = clientlist.begin();myit!=clientlist.end();myit++){
									if(myit->fd == sock_index){
										myit->login = 1;
										myit->msgs_sent--;
										break;
									} 
								}	
							}
							else if(strcmp(command,"LOGOUT")==0){
								for(myit = clientlist.begin();myit!=clientlist.end();myit++){
									if(myit->fd == sock_index){
										myit->login = 0;
										myit->msgs_sent--;
										break;
									} 
								}	
							}
							else if (strcmp(command,"BLOCK")==0){
								blockip(sock_index,buffer,&clientlist);	
							}
							else if (strcmp(command,"UNBLOCK")==0){
								unblockip(sock_index, buffer, &clientlist);
							}
							else if (strcmp(command,"BLOCKED")==0){
								cout<<"inside server blocked loop"<<endl;
								displayblocked(&clientlist, buffer);
							}
							else if (strcmp(command,"BROADCAST")==0){
								server_broadcast(sock_index,buffer,&clientlist);
			
							}
//							if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
//								printf("Done!\n");
//							fflush(stdout);
						}
						
				
					}
				}
			}
		}
	}
	}
	else{
	int login =0;
	while(TRUE){
                memcpy(&watch_list, &master_list, sizeof(master_list));
                selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		int sockfd;
                if(selret < 0)
                        perror("select failed.");
                if(selret > 0){
                /* Loop through socket descriptors to check which ones are ready */
                for(int sock_index=0; sock_index<=head_socket; sock_index+=1){
                        if(FD_ISSET(sock_index, &watch_list)){

                                /* Check if new command on STDIN */
                                if (sock_index == STDIN){
                                        char *cmd = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
					char *cmd_orig = (char *)malloc (sizeof(char)*BUFFER_SIZE*2);
					vector<string> commandrecvd;
					split_string(cmd,' ',commandrecvd);
					for (auto i:commandrecvd)
                                        memset(cmd, '\0', BUFFER_SIZE*2);
                                        memset(cmd_orig, '\0', BUFFER_SIZE*2);
                                        if(fgets(cmd, BUFFER_SIZE*2-1, stdin) == NULL)
                                                exit(-1);

  //                                      printf("\nI got: %s\n", cmd);
                                        int cmd_len = strlen(cmd);
                                        if(cmd[cmd_len-1] == '\n'){
                                                cmd[cmd_len-1] = '\0';
                                                cmd_len = strlen(cmd);
                                        }
					strcpy(cmd_orig,cmd);
                                        char *command = (char*) malloc(sizeof(char)*CMD_SIZE);
                                        memset(command, '\0', CMD_SIZE);
                                        char *command_new = (char*) malloc(sizeof(char)*CMD_SIZE);
					for (int j = 0; j< cmd_len; j++){
                                                if(cmd[j]==' '){
                                                        command_new = cmd+j+1;
                                                        cmd= command_new;
							command[j] = '\0';
                                                        break;
                                                }
                                                else{
                                                        command[j] = cmd[j];
                                                }
                                        }
					char * iparg= (char*) malloc (INET_ADDRSTRLEN);;
					getip(cmd,iparg);
                                        /*Process PA1 commands here ...*/
                                        if (strcmp(command,"IP")==0){
                                                getipaddress(command);
                                                }
					else if(strcmp(command,"AUTHOR")==0){
						Author();	
					}
                                        else if (strcmp(command,"PORT")==0){
						validateport(command,argv[2]);
                                        }
					else if (strcmp(command,"EXIT")==0){
						return 0;
					}
                                        else if (strcmp(command,"LIST")==0){
						if(login){	
							printlist(command, &clientlist);
						}
						else{
							cse4589_print_and_log("[%s:ERROR]\n", command);
            						cse4589_print_and_log("[%s:END]\n", command);
						}
					}
					else if (strcmp(command,"REFRESH")==0){
						char * list = (char *) malloc(sizeof(char)*BUFFER_SIZE);
						memset(list,'\0',BUFFER_SIZE);
						int refresh_sent = send(sockfd, command,strlen(command),0);
						if(refresh_sent == -1)
							perror("failed to send refresh"); 	
						if(int recv_bytes = recv(sockfd, list, BUFFER_SIZE-1, 0)!=0)
       						refresh_list_client(&clientlist, list);
                                        }
                                        else if (strcmp(command, "LOGIN")==0){
                                        //char *server_IP = (char*) malloc(sizeof(char)*CMD_SIZE);
                                        //char *server_Port = (char*) malloc(sizeof(char)*CMD_SIZE);
						
                                        if(login == 2){
						send(sockfd,command,strlen(command),0);
						login =1;
					}
					if(login == 0){
                                        	sockfd=login_client(cmd,argv[2],&clientlist,&master_list);
					}
					if(sockfd <=0){
						cse4589_print_and_log("[%s:ERROR]\n", command);
                                        	cse4589_print_and_log("[%s:END]\n", command);
						break;
					}
						if(sockfd > head_socket)
							head_socket = sockfd;
					login = 1;
					cse4589_print_and_log("[%s:SUCCESS]\n", command);
					cse4589_print_and_log("[%s:END]\n", command);
                                        }
                                        else if (strcmp(command, "LOGOUT")==0){
						send(sockfd, cmd_orig ,cmd_len,0);
						login = 2;
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						cse4589_print_and_log("[%s:END]\n", command);
                                        }
					else if (strcmp(command,"SEND")==0){
						cmd_len = strlen(cmd_orig);
						int bytes_sent =0;
                                        	if(validateip(iparg)== false||verifyIP(iparg,&clientlist)==false){
							cse4589_print_and_log("[%s:ERROR]\n", command);
                                        		cse4589_print_and_log("[%s:END]\n", command);
							break;
						}
						bytes_sent = sendall(sockfd, cmd_orig );
							cse4589_print_and_log("[%s:SUCCESS]\n", command);
                                        		cse4589_print_and_log("[%s:END]\n", command);
							
						/*if(bytes_sent<cmd_len){
 							bytes_sent=bytes_sent + send(sockfd, cmd_orig ,cmd_len,0);
							if(bytes_sent == -1){
								perror("failed to send message");
							}
						}*/
					}
					else if (strcmp(command,"BROADCAST")==0){
						cmd_len = strlen(cmd_orig);
						int bytes_sent =0;
                                        	/*if(validateip(iparg)== false||verifyIP(iparg,&clientlist)==false){
							cse4589_print_and_log("[%s:ERROR]\n", command);
                                        		cse4589_print_and_log("[%s:END]\n", command);
							break;
						}*/
						bytes_sent = send(sockfd, cmd_orig ,cmd_len,0);
						if(bytes_sent<cmd_len){
 							bytes_sent=bytes_sent + send(sockfd, cmd_orig ,cmd_len,0);
							if(bytes_sent == -1){
								perror("failed to send message");
							}
						}
					}
					else if (strcmp(command,"BLOCK")==0){
						cmd_len = strlen(cmd_orig);
						cout<<cmd<<"BLOCK"<<myblocklist.size()<<endl;
						int ifblocked =0;
						for(auto i:myblocklist){
                                                	cout<<i<<cmd<<strcmp(i.c_str(),cmd)<<endl;
                                                        if(strcmp(i.c_str(),cmd)==0){
                                                        	cse4589_print_and_log("[%s:ERROR]\n", command);
                                                                cse4589_print_and_log("[%s:END]\n", command);
								ifblocked =1;
                                                                break;
                                                         }
                                                }

                                        	if(validateip(cmd)== 0||verifyIP(cmd,&clientlist)==false){
							cout<<"inside first if"<<endl;
							cse4589_print_and_log("[%s:ERROR]\n", command);
                                        		cse4589_print_and_log("[%s:END]\n", command);
							break;
						}
						if(ifblocked == 0){
						int bytes_sent =0;
						if(bytes_sent<cmd_len){
 							bytes_sent+=send(sockfd, cmd_orig ,cmd_len,0);
							if(bytes_sent == -1){
								perror("failed to send message");
							}
						}
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
                                        	cse4589_print_and_log("[%s:END]\n", command);
						}
						myblocklist.push_back(cmd);
					}
					else if (strcmp(command,"UNBLOCK")==0){
						cmd_len = strlen(cmd_orig);
						int bytes_sent =0;
						cout<<cmd<<"validateip"<<":"<<iparg<<":"<<validateip(iparg)<<"verifyip"<<verifyIP<<endl;
                                        	if(validateip(cmd)== false||verifyIP(cmd,&clientlist)==false){
							cse4589_print_and_log("[%s:ERROR]\n", command);
                                        		cse4589_print_and_log("[%s:END]\n", command);
							break;
						}
						if(bytes_sent<cmd_len){
 							bytes_sent+=send(sockfd, cmd_orig ,cmd_len,0);
							if(bytes_sent == -1){
								perror("failed to send message");
							}
						}
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
                                        	cse4589_print_and_log("[%s:END]\n", command);
					}
					else if (strcmp(command,"BLOCKED")==0){
						cmd_len = strlen(cmd_orig);
						int bytes_sent =0;
						if(bytes_sent<cmd_len){
 							bytes_sent+=send(sockfd, cmd_orig ,cmd_len,0);
							if(bytes_sent == -1){
								perror("failed to send message");
							}
						}
					}
					}
					else if(sock_index == server_socket){
//						cout<<"something on server_socket"<<endl;
						int fdaccept=client_accept(server_socket);
                                                FD_SET(fdaccept, &master_list);
                                                if(fdaccept > head_socket) 
						head_socket = fdaccept;
						char * buffer= (char*) malloc(BUFFER_SIZE);
					}	
					else{
                                                char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
                                                char *buffer_1 = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
                                                char *message = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
                                                memset(buffer, '\0', BUFFER_SIZE*2);
                                                memset(message, '\0', BUFFER_SIZE*2);
                                                char srcip[INET_ADDRSTRLEN];
						
                                                int recvd_bytes = recv(sock_index, buffer, BUFFER_SIZE*2, 0);
						int buffer_len = strlen(buffer);
						strcpy(buffer_1, buffer);
						/*while(buffer[buffer_len-1] !='@'){
							cout<<"entered while"<<endl;
							recvd_bytes = recv(sock_index, buffer+buffer_len+1, BUFFER_SIZE*2, 0);
							strcpy(buffer_1, buffer);
							cout<< "inside while "<<buffer[buffer_len-1]<<endl; 
							int buffer_len = strlen(buffer);
							
						}*/
						 for(int i =0; i <strlen(buffer);i++){
                                                        if(buffer[i] == ' '){
                                                                message = buffer+i+1;
                                                                break;
                                                        }
                                                        else
                                                                srcip[i] = buffer[i];
                                                }
						cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
						cse4589_print_and_log("msg from:%s\n[msg]:%s\n", srcip, message);
						cse4589_print_and_log("[%s:END]\n", "RECEIVED");
					}
				}
			
			}}
		}	
	}
	return 0;


}
