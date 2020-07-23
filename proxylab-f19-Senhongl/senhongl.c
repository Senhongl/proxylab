
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

static const char *header_user_agent = "Mozilla/5.0"
                                    " (X11; Linux x86_64; rv:3.10.0)"
                                    " Gecko/20191101 Firefox/63.0.1";

int main(int argc, char **argv) {
    // char buf[MAXLINE];
    // char *filename = "random-text.txt";
    // char *hostname = "Host: angelshark.ics.cs.cmu.edu";
    // if (strstr(hostname, "Get")) {
    //     printf("correct!\n");
    // } else {
    //     printf("false!\n");
    // }
    char *test = "\r\n";
    if (strcmp(test, "\r\n")) {
        printf("correct\n");
    } else {
        printf("incorrect\n");
    }
}