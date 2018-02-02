//Mehmet Şerefoğlu - https://github.com/mhmtsrfglu
//99 means, room is empty

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define SIZE sizeof(struct sockaddr_in)
#define MAX 10
int client[MAX];
int ActiveClients = 0;
int msgcount[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct rooms
{
    int roomid;
    int person1;
    int person2;
};

void findMax(int *maxfd)
{
    int i;
    *maxfd = client[0];
    for (i = 1; i < MAX; i++)
        if (client[i] > *maxfd)
            *maxfd = client[i];
}

void logConnections(int type_con, char *IP_ADDR)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    FILE *fp = fopen("logs/connections.txt", "a+");
    if (type_con == 1)
    {
        fprintf(fp, "Connected : %s - Connection time is : %s", IP_ADDR, asctime(timeinfo));
    }
    else
    {
        fprintf(fp, "Disconnected : %s - Disconnection time is : %s", IP_ADDR, asctime(timeinfo));
    }
    fclose(fp);
}

void logMessages(int clientid, int rec, char *msg, int roomid)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char contitle[100];
    contitle[0] = roomid;

    FILE *fp = fopen("logs/conversations/chatlog.txt", "a+");
    fprintf(fp, "Room : %d - Date: %s ,%d -> %d : Message Content : %s\n", roomid, asctime(timeinfo), clientid, rec, msg);
    fclose(fp);
}

int main()
{
    int sockfd, maxfd, nread, found, i, j, k;
    char buf[128];
    struct rooms room[6];
    for (k = 0; k < 6; k++)
    {
        room[k].roomid = k;
        room[k].person1 = 99;
        room[k].person2 = 99;
    }

    fd_set fds;

    struct sockaddr_in server = {AF_INET, 3251, INADDR_ANY};
    struct sockaddr_in their_addr; // connector's address information
    int clientaddr = sizeof(their_addr);
    //Messages
    char rejectmessage[] = "Room is full\n";
    char acceptmessage[] = "Connection accepted\n";
    char roomerror[] = "We don't have a room like that\n";
    char alonemsg[] = "You are alone in room\n";

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating SOCKET\n");
        return (0);
    }
    if (bind(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
        printf("bind failed\n");
        return (0);
    }
    if (listen(sockfd, 5) == -1)
    {
        printf("listen failed\n");
        return (0);
    }
    else
    {
        puts("Server is running on PORT 3251...\n");
    }
    findMax(&maxfd);

    for (;;)
    {
        findMax(&maxfd);
        maxfd = (maxfd > sockfd ? maxfd : sockfd) + 1;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        for (i = 0; i < MAX; i++)
            if (client[i] != 0)
                FD_SET(client[i], &fds);

        /* Wait for some input or connection request. */
        select(maxfd, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
        /*If one of the clients has some input, read and send it to related  client.*/
        for (i = 0; i < MAX; i++)
            if (FD_ISSET(client[i], &fds))
            {
                nread = recv(client[i], buf, sizeof(buf), 0);
                /* If error or eof, terminate the connection */
                if (nread < 1)
                {
                    for (k = 0; k < 6; k++)
                    {
                        if (room[k].person1 == i)
                        {
                            room[k].person1 = 99;
                        }
                        else if (room[k].person2 == i)
                        {
                            room[k].person2 = 99;
                        }
                    }

                    close(client[i]);
                    client[i] = 0;
                    ActiveClients--;
                    printf("Disconnected : %s\n", inet_ntoa(their_addr.sin_addr));
                    logConnections(2, inet_ntoa(their_addr.sin_addr));
                }
                else
                {

                    if (msgcount[i] == 0)
                    {
                        int r = atoi(buf);
                        char conmsg[] = "You are connected to room ";
                        strcat(conmsg, buf);
                        //check room range
                        if (r > 0 && r <= 5)
                        {
                            if (room[r].person1 == 99)
                            {
                                //assing client to first person
                                room[r].person1 = i;
                                send(client[i], conmsg, strlen(conmsg) + 1, 0);
                                msgcount[i]++;
                            }
                            else if (room[r].person2 == 99)
                            {
                                //assing client to second person
                                room[r].person2 = i;
                                send(client[i], conmsg, strlen(conmsg) + 1, 0);
                                msgcount[i]++;
                            }
                            else
                            {
                                //room is full
                                send(client[i], rejectmessage, strlen(rejectmessage) + 1, 0);
                            }
                        }
                        else
                        {
                            //if we dont have room you will get error
                            send(client[i], roomerror, strlen(roomerror) + 1, 0);
                        }
                    }
                    else
                    {

                        for (k = 0; k < 6; k++)
                        {
                            if (room[k].person1 == i)

                            {
                                //if person2 is null then will send alone message
                                if (room[k].person2 == 99)
                                {
                                    send(client[i], alonemsg, strlen(alonemsg) + 1, 0);
                                    break;
                                }
                                else
                                {
                                    send(client[room[k].person2], buf, nread, 0);
                                    logMessages(i, room[k].person2, buf, room[k].roomid);
                                    buf[0] = '\0';
                                    break;
                                }

                                //if person1 is null then will send alone message
                            }
                            else if (room[k].person2 == i)
                            {
                                if (room[k].person1 == 99)
                                {
                                    send(client[i], alonemsg, strlen(alonemsg) + 1, 0);
                                    break;
                                }
                                else
                                {
                                    send(client[room[k].person1], buf, nread, 0);
                                    logMessages(i, room[k].person1, buf, room[k].roomid);
                                    buf[0] = '\0';
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        /* if there is a request for a new connection */
        if (FD_ISSET(sockfd, &fds))

        {
            /* If no of active clients is less than MAX clientaccept the request */
            if (ActiveClients < MAX)
            {
                found = 0;
                for (i = 0; i < MAX && !found; i++)
                    if (client[i] == 0)
                    {
                        client[i] = accept(sockfd, (struct sockaddr *)&their_addr, &clientaddr);
                        found = 1;
                        ActiveClients++;
                        printf("Connected : %s\n", inet_ntoa(their_addr.sin_addr));
                        logConnections(1, inet_ntoa(their_addr.sin_addr));
                    }
            }
        }
    }
}
