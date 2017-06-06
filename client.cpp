/* Client for simple chat-server
 *
 * 06/06/2017
 * tarasov@ucu.edu.ua
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define MSG_LEN 100
#define NICKNAME_LEN 20
#define SERVER_PORT 22232

int main() {
    int fd;
    fd_set  testfds, clientfds;
    char msg[MSG_LEN + NICKNAME_LEN + 20];
    char input_msg[MSG_LEN + 10];
    char nickname[NICKNAME_LEN] = "anon";
    int sockfd;
    int line_size;
    char hostname[] = "127.0.0.1";
    struct hostent *hostinfo;
    struct sockaddr_in address;


    printf("To exit write quit\nBE CAREFUL. YOUR FIRST MESSAGE WILL BE YOUR NICK \n");
    fflush(stdout);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    hostinfo = gethostbyname(hostname);
    address.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);


    if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("connecting");
        exit(1);
    }

    FD_ZERO(&clientfds);
    FD_SET(sockfd,&clientfds);
    FD_SET(0,&clientfds);


    while (1) {
        testfds=clientfds;
        select(FD_SETSIZE,&testfds,NULL,NULL,NULL);

        for(fd=0;fd<FD_SETSIZE;fd++){
            if(FD_ISSET(fd,&testfds)){
                if(fd==sockfd){
                    line_size = read(sockfd, msg, MSG_LEN);
                    msg[line_size] = '\0';
                    printf("%s", msg +1);

                    if (msg[0] == 'X') {
                        close(sockfd);
                        exit(0);
                    }
                }
                else if(fd == 0)
                {
                    fgets(input_msg, MSG_LEN+1, stdin);
                    if (strcmp(input_msg, "quit\n")==0)
                    {
                        strcpy(msg, "XClient is shutting down.\n");
                        write(sockfd, msg, strlen(msg));
                        close(sockfd);
                        exit(0);
                    }
                    else
                    {
                        if (strcmp(nickname, "anon") == 0) {
                            strcpy(nickname, input_msg);
                        }
                        strcpy(msg, "M");
                        strcat(msg, input_msg);
                        write(sockfd, msg, strlen(msg));
                    }
                }
            }
        }
    }
}
