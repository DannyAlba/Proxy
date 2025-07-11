#include <stdio.h>
#include <stdlib.h>
#include <string.h>     
#include <strings.h>    
#include <unistd.h>      
#include <sys/socket.h>  
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#define MAXLINE  8192  
#define MAXBUF   8192  
#define LISTENQ  1024  
#define BUFSIZE  1024

pthread_mutex_t cache;


void echo(int connfd);
void *thread(void *vargp);
int open_listenfd(int port);
void respond(int connfd, char *response)
{
    write(connfd, response, strlen(response));
    return;
}

int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port, clientlen=sizeof(struct sockaddr_in);
    int timeout;
    struct sockaddr_in clientaddr;
    pthread_t tid; 

    if (argc != 3) {
	fprintf(stderr, "usage: %s <port> <timeout>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);
    timeout = atoi(argv[2]);

    listenfd = open_listenfd(port);
    while (1) {
	connfdp = malloc(sizeof(int));
	*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
	pthread_create(&tid, NULL, thread, connfdp);
    }
}

int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}

/* thread routine */
void * thread(void * vargp) 
{  
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self()); 
    free(vargp);
    echo(connfd);
    close(connfd);
    return NULL;
}

void echo(int connfd)
{
    char buf[MAXLINE];
    char bufcopy[MAXLINE];
    char *firstline;
    size_t n;
    char get[20];
    char url[500];
    char http[50];
    char fullurl[40];
    char host[50];
    int port;
    struct sockaddr_in serveraddr;

    printf("%s\n", "Starting.");
    while((n = read(connfd, buf, MAXLINE)) > 0)
    {

    if(n > 0)
    {
        // printf("%s", buf);
        // firstline = strtok(buf, "\n");
        // printf("%s", firstline);
        if(strlen(buf) > sizeof(bufcopy))
        {
            printf("%s", "shit too large");
            return;
        }
        strcpy(bufcopy, buf);
        firstline = strtok(bufcopy, "\n");
        // printf("%s", "jello");
        if(strstr(firstline, "GET") != NULL)
        {
            printf("%s\n %s","BUFF to be edited:", buf);
            // strcpy(bufcopy, buf);
            // firstline = strtok(bufcopy, "\n");
            // bzero(bufcopy, sizeof(bufcopy));
            // printf("%s, %s\n","firstline", firstline);
            sscanf(firstline, "%s %s %s", get, url, http);       
            // printf("%s, %s, %s\n", "url:", url, http);  
            sscanf(url, "http://%49[^:]:%d", host, &port);
            printf("%s, %d", host, port);
            if(port <= 1)
            {
                port = 80;
            }

            int sockfd;

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in address;
            // printf("%s, %s %d\n", "host, port: ", host, port);
            // memset(&address,0, sizeof(address));
            bzero((char *)&serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_port = htons(port);
            char *fullhost;
            // fullhost = strtok(host, '/');
            int namelen = strlen(host);
            fullhost = strchr(host, '/');
            char hostname[256];
            if(!fullhost)
            {
                // host[namelen - 1] = '\0';
                strcpy(hostname, host);
                // printf("%s %s, %d\n", "host, port:", hostname, port);               
            }
            else
            {
                size_t hostname_length = fullhost - host;
                strncpy(hostname, host, hostname_length);
                hostname[hostname_length] = '\0'; 
                // printf("%s %s\n", "hostname: ", hostname);
                // strcpy(path, fullhost); 
            }
            // printf("%s %s, %d\n", "host, port:", hostname, port);
                        // printf("%s", "test");

            struct in_addr inAddress;

            struct hostent* server;

            char line[100];
            char *newline;
            FILE *block;
            block = fopen("blocklist", "r");
            while(fgets(line, sizeof(line), block))
            {
                newline = strchr(line, '\n');
                // printf("%s: %s", "blocklist", line);
                if(strstr(line, hostname))
                {
                    printf("%s\n", "This website is in the blacklist and cannot be accessed.");
                    fclose(block);
                    return;
                }
            }
            fclose(block);

            server = gethostbyname(hostname);
            
            if(server == NULL)
            {
                printf("%s", "server not catching");
            }
            else
            {
                bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
            }
            char ipBuf[20];
            if(inet_ntop(AF_INET, (char *)&serveraddr.sin_addr.s_addr, ipBuf, (socklen_t)20) == NULL)
            {
                printf("%s", "not even sure");
                return;
            }
            int serverlen = sizeof(serveraddr);
            int size;
            size = connect(sockfd, (struct sockaddr*)&serveraddr, serverlen);
            if(size < 0)
            {
                printf("%s", "cannot connect");
            }
            else
            {
                printf("%s\n", "connected");
            }
            // printf("%s\n %s", "This is the contents that is being written:", buf);
            bzero(buf,sizeof(buf));
            char path[1024] = "/";
            char *path_start = strchr(url + 7, '/');
            // sscanf(full_url, "/http://%255[^/]", host);  
            strncpy(path, path_start, sizeof(path) - 1);

            sprintf(buf, "GET %s %s\r\nHost: %s\r\n\r\n", path, http, hostname);
            printf("%s\n %s\n", "This is BUFF that is being written to the server website:", buf);
            // printf("%s", "Im gonna cry");
            write(sockfd, buf, strlen(buf));

            bzero(buf, sizeof(buf));
            int m;
            // printf("%s", "Still don't know");



            // DIR *dir = opendir("./cache");
            // struct dirent *entry;
            // char *hostpath = strcat(hostname,path);


            //     unsigned long hash = 0;
            //     for (size_t i = 0; hostpath[i] != '\0'; i++)
            //     {
            //         hash += (unsigned char)hostpath[i]; // Add ASCII values
            //     }
            //     printf("%lu", hash);
      




            // while((entry = readdir(dir)) != NULL)
            // {
            //     if(strcmp(entry->d_name, hash) == 0)
            //     {
            //         printf("%s %s\n", "filename: ", entry->d_name);
            //         printf("%s\n", "this file exists in cache");
            //         FILE *cache;
            //         char cacheLine[100];
            //         char direc[256];
            //         strcpy(direc, "./cache/");
            //         printf("%s\n", direc);
            //         char *cacheName = strcat(direc, hash);
            //         cache = fopen(cacheName, "r");

            //         if(cache == NULL)
            //         {
            //             printf("%s\n", "idk");
            //             return;
            //         }
            //         printf("%s", "we're in");
            //         char contents[BUFSIZE];
            //         int filebyte;
            //         fseek(cache,0,SEEK_END);
            //         size_t filesize = ftell(cache);
            //         fseek(cache,0,SEEK_SET);
            //         m = read(sockfd, buf, MAXLINE);
            //         // write(connfd, buf, sizeof(buf));
            //         while((filebyte = fread(contents, 1, BUFSIZE, cache)) > 0)
            //         {
            //             write(connfd, contents, sizeof(contents)); 
            //             bzero(contents, sizeof(contents));
            //         }
            //         close(sockfd);
            //         return;
            //     }
            // }

            // printf("%s", "Not sure");

            // char direcn[256];
            // strcpy(direcn, "./cache/");
            // char *nf = strcat(direcn, hash);

 

            // FILE *newCache;
            // newCache = fopen(nf, "a");
            // pthread_mutex_lock(&cache);
            while((m = read(sockfd, buf, MAXLINE)) > 0)
            {
                // fprintf(newCache, buf);
                write(connfd, buf, m);
                printf("%s\n %s\n","Contents of read:", buf);  
                bzero(buf, MAXLINE);
            }      
            printf("%s", "Socket is closed");
            close(sockfd);
            // pthread_mutex_unlock(&cache);

            // write(connfd, buf, sizeof(buf));




            // fclose(newCache);
            printf("%s\n", "Reached the end.");

             
        }
    }
    }
}
