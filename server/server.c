#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/uio.h>


typedef struct clnt_data {
    int sockfd;
    struct sockaddr_in clnt;
} Clnt_data;

typedef struct  sock_node {
    Clnt_data clnt_node;
    struct sock_node *next;
} Sock_node;


static Sock_node *head = NULL;

pthread_mutex_t mutex;


/*
    ret :  0 (success)
        : -1 (failed)
*/
int add_sock(Clnt_data data) {
    Sock_node *new_sock, *now;

    new_sock = (Sock_node*)malloc(sizeof(Sock_node));
    if (new_sock == NULL) {
        perror("malloc");
        return -1;
    }
    new_sock->clnt_node = data;
    new_sock->next = NULL;

    if (head == NULL) {
        head = new_sock;
    } else {
        now = head;
        while (now->next != NULL) {
            now = now->next;
        }
        now->next = new_sock;
    }

    return 0;
}

/*
    ret :  0 (success)
        : -1(failed)
*/
int del_sock(Clnt_data data) {
    Sock_node *now;
    Sock_node *prev;
    if (head == NULL) {
        return -1;
    }

    now = head;
    if (now->clnt_node.sockfd == data.sockfd) {
        head = now->next;
        free(now);

        return 0;
    }

    while (now != NULL) {
        if (now->clnt_node.sockfd == data.sockfd) {
            //tail = prev;
            prev->next = now->next;
            free(now);

            return 0;
        }

        prev = now;
        now = now->next;
    }

    return -1;
}


/* void parse_msg(char *msg) {

}*/

size_t send_msg_all(Clnt_data data, char *msg, size_t len) {
    size_t ret_len;
    Sock_node *now = head;
    while (now != NULL) {
        if (now->clnt_node.sockfd == data.sockfd) {
            ret_len = send(now->clnt_node.sockfd, msg, len, 0);
            printf("%d<==%s\n", now->clnt_node.sockfd, msg);
        } else {
            send(now->clnt_node.sockfd, msg, len, 0);
            printf("%d<==%s\n", now->clnt_node.sockfd, msg);
        }
        now = now->next;
    }
    return ret_len;
}

void print_sock_list() {
    Sock_node *now = head;
    while (now != NULL) {
        printf("-->%d, %s %d\n", 
                    now->clnt_node.sockfd, 
                    inet_ntoa(now->clnt_node.clnt.sin_addr),
                    ntohs(now->clnt_node.clnt.sin_port));

        now = now->next;
    }
    printf("-->NULL\nhead=%p\n", head);
}

void *msg_con(Clnt_data *data) {
    Clnt_data new_data;
    int fd;
    char buf[BUFSIZ];
    new_data.clnt = data->clnt;
    new_data.sockfd = data->sockfd;
    pthread_detach(pthread_self());

    printf("connect from %s: %d, sockfd:%d\n", 
                inet_ntoa(new_data.clnt.sin_addr), 
                ntohs(new_data.clnt.sin_port), 
                new_data.sockfd);
    memset(buf, 0, BUFSIZ);

    print_sock_list();

    while(strncasecmp(buf, "exit", 4) != 0) {
        size_t len = recv(new_data.sockfd, buf, BUFSIZ, 0);
        buf[len] = '\0';
        printf("%d, %s %d==>%s\n", 
                    new_data.sockfd,
                    inet_ntoa(new_data.clnt.sin_addr), 
                    ntohs(new_data.clnt.sin_port), 
                    buf);

        pthread_mutex_lock(&mutex);

        if (strncasecmp(buf, "!history", 8) == 0) {
            fd = open("./history.txt", O_RDONLY);
            char buf_h[BUFSIZ], c;
            int cnt = 0;
            if (read(fd, (void *)buf_h, sizeof(buf_h)) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            while (buf_h[cnt] != '\0') {
                cnt++;
            }
            send(new_data.sockfd, buf_h, cnt, 0);
        } else if (strncasecmp(buf, "exit", 4) != 0) {
            len = send_msg_all(new_data, buf, len);

            fd = open("./history.txt", O_WRONLY);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            if (write(fd, buf, len+1) < 0) {
                    perror("write");
                    exit(EXIT_FAILURE);
            }

            close(fd);
        }

        pthread_mutex_unlock(&mutex);
    }
    printf("[sock:%d] socket closed\n", new_data.sockfd);
    if (del_sock(new_data) < 0) {
        perror("del_sock");
        exit(EXIT_FAILURE);
    }
    print_sock_list();
    close(new_data.sockfd);
    pthread_exit(0);
}

void signal_handler(int sig, siginfo_t *siginfo, void *context) {
    int sockfd = siginfo->si_value.sival_int;
    printf("---CLOSE---\n");
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd;
    size_t len;
    char buf[BUFSIZ];
    struct sockaddr_in serv;
    socklen_t sin_siz;
    int port;
    
    struct sigaction act;
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    if(argc != 3) {
        printf("Usage:./prog host port\n");
        exit(EXIT_FAILURE);
    }
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("socket() called\n");
    serv.sin_family = PF_INET;
    port = strtol(argv[2], NULL, 10);
    serv.sin_port = htons(port);
    inet_aton(argv[1], &serv.sin_addr);
    sin_siz = sizeof(struct sockaddr_in);
    if(bind(sockfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("bind() called\n");

    if(listen(sockfd,SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("listen() called\n");

    while(1) {
        pthread_t th;
        Clnt_data clnt_data;
        if((clnt_data.sockfd = accept(sockfd, (struct sockaddr*)&clnt_data.clnt, &sin_siz)) < 0) {
            perror("accept");
        }
        
        if (add_sock(clnt_data) < 0) {
            perror("add_socket");
            exit(EXIT_FAILURE);
        }
        
        if (pthread_create(&th, NULL, (void *)msg_con, (void *)&clnt_data) != 0) {
            perror("thread");
            exit(EXIT_FAILURE);
        }
    }
    close(sockfd);
    return 0;
}