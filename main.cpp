/* simple chat-server
 *
 * 06/06/2017
 * tarsov@ucu.edu.ua
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 15
#define MSG_LEN 100
#define NICKNAME_LEN 20
#define SERVER_PORT 22232


int kill_client(int fd, fd_set *readfds, char *fd_array, int num_clients){

    int client_index = -1;

    close(fd);
    FD_CLR(fd, readfds);

    for (int i = 0; i < num_clients - 1; i++)
        if (fd_array[i] == fd) {
            client_index = i;
            break;
        }

    for (int i = client_index; i < num_clients - 1; i++)
        fd_array[i] = fd_array[i + 1];

    return client_index;
}


int main() {
    int fd;
    int line_size;
    int num_clients = 0;
    int killed_client_index;
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_address;
    char fd_array[MAX_CONNECTIONS];
    char nicknames[MAX_CONNECTIONS][NICKNAME_LEN];
    fd_set read_fds, clone_fds;
    char client_line[MSG_LEN + 1];
    char server_line[MSG_LEN + NICKNAME_LEN + 20];


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address));


    listen(server_sockfd, 1);
    FD_ZERO(&read_fds);
    // allow fd_set to see input of server socket
    FD_SET(server_sockfd, &read_fds);
    // allow fd_set to see input of server stdin
    FD_SET(0, &read_fds);

    for (;;) {
        clone_fds = read_fds;
        select(FD_SETSIZE, &clone_fds, NULL, NULL, NULL);

        for (fd = 0; fd < FD_SETSIZE; fd++) {

            if (FD_ISSET(fd, &clone_fds)) {

                // new client
                if (fd == server_sockfd) {

                    client_sockfd = accept(server_sockfd, NULL, NULL);




                    if (num_clients <= MAX_CONNECTIONS) {

                        FD_SET(client_sockfd, &read_fds);
                        fd_array[num_clients]= (char) client_sockfd;
                        strcpy(nicknames[num_clients], "anon");

                        printf("Anonymus client %d joined\n",num_clients++);

                    }
                    else {
                        strcpy(client_line, "SERVER: TOO MANY CLIENTS.\n");
                        write(client_sockfd, client_line, strlen(client_line));
                        close(client_sockfd);
                    }
                }
                    // stdin
                else if (fd == 0)  {
                    fgets(server_line, MSG_LEN + 1, stdin);

                    if (strcmp(server_line, "quit\n")==0) {
                        for (int i = 0; i < num_clients ; i++) {
                            write(fd_array[i], client_line, strlen(client_line));
                            close(fd_array[i]);
                        }
                        close(server_sockfd);
                        exit(0);
                    }
                }
                    //client message
                else if(fd) {

                    line_size = read(fd, client_line, MSG_LEN);

                    if(line_size==-1) perror("read()");
                    else if(line_size>0){

                        strcpy(server_line, "M");
                        // ignore new line
                        client_line[line_size-1]='\0';


                        for (int i = 0; i < num_clients; i++) {
                            if (fd_array[i] == fd) {
                                if (strcmp(nicknames[i], "anon") == 0) {
                                    strcpy(nicknames[i], client_line+1);
                                }
                                strcat(server_line, nicknames[i]);
                                break;
                            }

                        }
                        strcat(server_line, ":   ");

                        strcat(server_line, client_line+1);
                        strcat(server_line, "\n");


                        for(int i=0;i<num_clients;i++){
                            if (fd_array[i] != fd)
                                write(fd_array[i],server_line,strlen(server_line));
                        }
                        printf("%s",server_line+1);

                        if(client_line[0] == 'X'){
                            if ((killed_client_index = kill_client(fd, &read_fds, fd_array, num_clients)) != -1) {
                                num_clients--;
                                for (int i = killed_client_index; i < num_clients; i++){
                                    strcpy(nicknames[i], nicknames[i+1]);
                                }
                            }
                        }
                    }
                }
                else {
                    if ((killed_client_index = kill_client(fd, &read_fds, fd_array, num_clients)) != -1) {
                        num_clients--;
                        for (int i = killed_client_index; i < num_clients; i++){
                            strcpy(nicknames[i], nicknames[i+1]);
                        }
                    }
                }
            }
        }
    }
}



