#include "httpd.h"
#include "chain.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#define CONNMAX 1000

static int listenfd, clients[CONNMAX];
static void error(char *);
static void startServer(const char *);
static void respond(int, Node*, Block*);

typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = { {"\0", "\0"} };
static int clientfd;

static char *buf;

void server_listen(const char *PORT)
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;    
    
    int slot=0;
    
    printf(
            "Server started %shttp://127.0.0.1:%s%s\n",
            "\033[92m",PORT,"\033[0m"
            );

    // element = -1 no client connected
    int i;
    for (i=0; i<CONNMAX; i++)
        clients[i]=-1;
    startServer(PORT);
    
    // Iignore SIGCHLD , avoid zombie threads
    signal(SIGCHLD,SIG_IGN);


    Node* self_node = malloc(sizeof(Node));
    Block* next_block = _init_block();

    _init_node(self_node, next_block);

    // accept connections in while 1
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
        {
            perror("accept() error");
        }
        else
        {
            if ( fork()==0 )
            {
                respond(slot, self_node, next_block);
                exit(0);
            }

            // if ( fork()==0 )
            // {
            //     // in client, close the listener bc client doesnt need it ??
            //     close(listenfd);
            //     respond(slot);
            //     exit(0);
            // } else
            // {
            //     // in server, close the accepted handle bc server doesnt need it ??
            //     close(clients[slot]);
            // }

        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }
}

void startServer(const char *port)
{

    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        int option = 1;
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
}


// get request header
char *request_header(const char* name)
{
    header_t *h = reqhdr;
    while(h->name) {
        if (strcmp(h->name, name) == 0) return h->value;
        h++;
    }
    return NULL;
}

//client connection
void respond(int n, Node* self_node, Block* next_block)
{
    int rcvd, fd, bytes_read;
    char *ptr;

    buf = (char *) malloc(65535);
    rcvd=recv(clients[n], buf, 65535, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // message received
    {

        buf[rcvd] = '\0';

        method = strtok(buf,  " \t\r\n");
        uri    = strtok(NULL, " \t");
        prot   = strtok(NULL, " \t\r\n"); 

        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
        
        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; //split URI
        } else {
            qs = uri - 1; //empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;
        while(h < reqhdr+16) {
            char *k,*v;
            k = strtok(NULL, "\r\n: \t"); if (!k) break;
            v = strtok(NULL, "\r\n");     while(*v && *v==' ') v++;
            h->name  = k;
            h->value = v;
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
        
            if (t[1] == '\r' && t[2] == '\n') { 
                payload = t+3;
                break;
            }
        }
        t++;
        t2 = request_header("Content-Length"); 
        payload_size = t2 ? atol(t2) : (rcvd-(t-buf));
        fprintf(stderr, "[P] %s\n",payload);

        // bind clientfd to stdout  (printf writes to client)
        clientfd = clients[n];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);

        // call router
        route(self_node, next_block);
        

        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);
        free(buf);
    }

    //closing SOCKET
    shutdown(clientfd, SHUT_RDWR);         // send and recieve operations are disabled
    close(clientfd);
    clients[n]=-1;
}
