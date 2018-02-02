#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#define SIZE sizeof(struct sockaddr_in)
int main()
{
    int sockfd, nread;
    char buf[254], enter, resp;
    fd_set fds;
    char IP[20];
    struct sockaddr_in server = {AF_INET, 3251 };

    printf("\n\n\nEnter IP address of the Server\n");
    scanf("%s%c", IP, &enter);
    server.sin_addr.s_addr = inet_addr(IP);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating SOCKET\n");
        return (0);
    }
    if (connect(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
        printf("Connect failed\n");
        return (0);
    }else{
        printf("Listening PORT 3251\n");
        printf("Connected to server\n");

        printf("\n1 - Room1\n2 - Room2\n3 - Room3\n4 - Room4\n5 - Room5\n");

        printf("Please choose a room :\n");

    }

    do
    {
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        FD_SET(0, &fds);
        /* Wait for some input. */
        select(sockfd + 1, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
        /* If either device has some input,read it and copy it to the other. */
        if (FD_ISSET(sockfd, &fds))
        {
            nread = recv(sockfd, buf, sizeof(buf), 0);
            /* If error or eof, terminate. */
            if (nread < 1)
            {

                close(sockfd);
                exit(0);
            }
            buf[nread] = 0;
            printf("Incoming Message : %s", buf);
            buf[0] = '\0';
        }
        if (FD_ISSET(0, &fds))
        {
            nread = read(0, buf, sizeof(buf));
            /* If error or eof, terminate. */
            if (nread < 1)
            {
                close(sockfd);
                exit(0);
            }
            else if ((buf[0] == 'e' || buf[0] == 'E') && nread == 2)
            {
                close(sockfd);
                exit(0);
            }
            else{
                send(sockfd, buf, nread, 0);
                buf[0] = '\0';
            }
        }
    } while (1);
}