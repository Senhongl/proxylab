/*
 * Starter code for proxy lab.
 * Feel free to modify this code in whatever way you wish.
 */

/* Some useful includes to help you get started */
#define _XOPEN_SOURCE_EXTENDED 1
#include "csapp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <assert.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

/*
 * Debug macros, which can be enabled by adding -DDEBUG in the Makefile
 * Use these if you find them useful, or delete them if not
 */
#ifdef DEBUG
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_printf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dbg_assert(...)
#define dbg_printf(...)
#endif

/*
 * Max cache and object sizes
 * You might want to move these to the file containing your cache implementation
 */
#define MAX_CACHE_SIZE (1024*1024)
#define MAX_OBJECT_SIZE (100*1024)

/*
 * String to use for the User-Agent header.
 * Don't forget to terminate with \r\n
 */
static const char *header_user_agent = "Mozilla/5.0"
                                    " (X11; Linux x86_64; rv:3.10.0)"
                                    " Gecko/20191101 Firefox/63.0.1";

typedef struct sockaddr SA;
void echo(int fd);
void read_requesthdrs(rio_t *rp);
void parse_uri(char *request, char *filename, char *host, char *port);
void get_filetype(char *filename, char *filetype);
void send2server(int fd, char *filename, char *hostname, char *port, rio_t rio);
void send2client(int connfd, int clientfd);

// void client
int main(int argc, char** argv) {
    printf("%s", header_user_agent);
    printf("\n");
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    listenfd = open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        printf("fd is %d\n", connfd);
        echo(connfd);
        close(connfd);
    }
    
    
}

void echo(int fd) {
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char host[MAXLINE], port[MAXLINE], filename[MAXLINE];
    strcpy(port, ""); // clean the port;
    rio_t rio;

    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, url, version);
    
    // printf("%s\n", uri);
    if (strcasecmp(method, "GET")) {
        printf("buf is %s, fd is %d\n", buf, fd);
        return;
    }

    // read_requesthdrs(&rio);
    
    /* Parse URI from GET request */
    parse_uri(url, filename, host, port);

    /* Send request to server */
    // creat connection between proxy and server
    int clientfd;
    
    if (!strcmp(port, "")) {
        strcpy(port, "80"); // default HTTP port is 80
    }

    clientfd = open_clientfd(host, port);
    /* Send request from client to server */
    send2server(clientfd, filename, host, port, rio);
    /* Send response from server to client */
    send2client(fd, clientfd);

    close(clientfd);

    return;
}

/* Send the request header to server
 * filename is the name of request file
 * hostname:port would be the host the proxy going to connect to
 */
void send2server(int fd, char *filename, char *hostname, char *port, rio_t rio) {

    char buf[MAXLINE];
    sprintf(buf, "GET /%s HTTP/1.0\r\n", filename);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "User-Agent: %s\r\n", header_user_agent);
    rio_writen(fd, buf, strlen(buf));
    memset(buf, 0, MAXLINE);
    while (rio_readlineb(&rio, buf, MAXLINE) > 0) {
        if (strstr(buf, "Response")) {
            ;
        } else if (!strcmp(buf, "\r\n")) {
            sprintf(buf, "Response: Immediate\r\n\r\n");
            rio_writen(fd, buf, strlen(buf));
            return;
        } else {
            rio_writen(fd, buf, strlen(buf));
        }
    }

}

/* After receiving the response from sever, send it back to client
 */
void send2client(int connfd, int clientfd) {

    char buf[MAXLINE];
    rio_t rio;
    rio_readinitb(&rio, clientfd);
    // while (rio_readlineb(&rio, buf, MAXLINE) > 0) {
    //     rio_writen(connfd, buf, strlen(buf));        
    // }
    int n;
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        rio_writen(connfd, buf, strlen(buf)); 
    }
    
    sprintf(buf, "\r\n");
    rio_writen(connfd, buf, strlen(buf));
}


// /*
//  * get_filetype - Derive file type from filename
//  */
// void get_filetype(char *filename, char *filetype) {
//     if (strstr(filename, ".html")) {
//         strcpy(filetype, "text/html");
//     } else if (strstr(filename, ".gif")) {
//         strcpy(filetype, "image/gif");
//     } else if (strstr(filename, ".png")) {
//         strcpy(filetype, "image/png");
//     } else if (strstr(filename, ".jpg")) {
//         strcpy(filetype, "image/jpg");
//     } else if (strstr(filename, ".jpeg")) {
//         strcpy(filetype, "image/jpeg");
//     } else {
//         strcpy(filetype, "text/plain");
//     }
// }
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

void parse_uri(char *request, char *filename, char *host, char *port) {
    if (sscanf(request, "http://%[a-zA-Z.]:%[0-9]/%s[^\n]", host, port, filename) == 3);
    else if (sscanf(request, "http://%[a-zA-Z.]/%s", host, filename) == 2);
    else if (sscanf(request, "http://%[a-zA-Z.]:%[0-9]", host, port) == 2);
    else if (sscanf(request, "http://%s", host) == 1);

}

