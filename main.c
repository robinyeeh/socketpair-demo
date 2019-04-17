#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

char *cat_string(char* first, char *second) {
    char *str = malloc(strlen(first) + strlen(second));
    strcpy(str, first);
    strcat(str, second);

    return str;
}

char *getpid_str() {
    int pid2 = getpid();
    char * mypid = malloc(6);
    sprintf(mypid, "%d", pid2);

    return mypid;
}

void handle_child(int sv) {
    char *msg = "I'm child process : ";

    msg = cat_string(msg, getpid_str());
    msg = cat_string(msg, "\n");

    char buf[1024];

    while (1) {
        write(sv, msg, strlen(msg));

        sleep(5);

        ssize_t s = read(sv, buf, sizeof(buf) -1 );
        if (s > 0 ) {
            buf[s] = '\0';

            printf("[%d], child process : %s\n", getpid(), buf);
        }
    }
}

void handle_parent(int sv) {
    char * msg = "I'm parent process: ";
    msg = cat_string(msg, getpid_str());
    msg = cat_string(msg, "\n");

    char buf[1024];

    while (1) {
        ssize_t s = read(sv, buf, sizeof(buf) - 1);

        if (s > 0) {
            buf[s] = '\0';

            printf("[%d], parent process say: %s\n", getpid(), buf);
        }

        sleep(3);

        write(sv, msg, strlen(msg));
    }
}

int main() {
    int sv[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        printf("Failed to create socket pair : %d\n", errno);

        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (pid) {
        case 0:
            close(sv[1]);
            handle_child(sv[0]);
        case -1: {
            printf("Failed to fork child process: %d\n", errno);

            exit(EXIT_FAILURE);
        }
        default: {
            waitpid(pid, NULL, WNOHANG);
        }
    }

    close(sv[0]);

    handle_parent(sv[1]);

    return 0;
}