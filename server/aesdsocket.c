// aesdsocket.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <syslog.h>
#include <sys/stat.h>

#define PORT 9000
#define DATA_FILE "/var/tmp/aesdsocketdata"
int server_fd, client_fd;
char  buffer[20000] ;
void signal_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        remove(DATA_FILE);
        closelog();
    
        exit(EXIT_SUCCESS);
    }
}
void setup_signal_handlers(void) {
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &sigact, NULL) != 0) {
        syslog(LOG_ERR, "Failure when trying to register a handler for the SIGTERM signal");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sigact, NULL) != 0) {
        syslog(LOG_ERR, "Failure when trying to register a handler for the SIGINT signal");
        exit(EXIT_FAILURE);
    }

}
void create_daemon()
{
    pid_t pid;
    
    /* Fork off the parent process */
    pid = fork();
    
    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);
    
     /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    
    /* Catch, ignore and handle signals */
    /*TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    /* Fork off for the second time*/
    pid = fork();
    
    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    /* Set new file permissions */
    umask(0);
    
    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Open /dev/null and redirect standard file descriptors
    int devnull = open("/dev/null", O_RDWR);
    if (devnull < 0) {
        perror("open /dev/null failed");
        exit(EXIT_FAILURE);
    }

    dup2(devnull, STDIN_FILENO);
    dup2(devnull, STDOUT_FILENO);
    dup2(devnull, STDERR_FILENO);

    // Close the file descriptor for /dev/null
    close(devnull);
}
int main(int argc, char *argv[]) {
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        create_daemon();
    }

    setup_signal_handlers();

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1 ;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
    }
    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while ((client_fd = accept(server_fd, (struct sockaddr *)&server_addr, &addr_len)) > 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(server_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        int read_bytes ;
        while (1) {
        read_bytes = recv(client_fd, buffer, sizeof(buffer),0);
        if(read_bytes == -1)
        {
           exit(EXIT_FAILURE); 
        }

        if(memchr(buffer, '\n', read_bytes) != NULL)
        {
            buffer[read_bytes]='\0';
            break ;
        }
        }

        int fd = open(DATA_FILE, O_RDWR | O_CREAT | O_APPEND| O_SYNC, 0777);

        if (fd == -1) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        //lseek(fd, 0, SEEK_END);
        ssize_t bytesWritten = write(fd, buffer, read_bytes);
        if (bytesWritten == -1) {
            perror("Error writing to file");
            close(fd);
            exit(EXIT_FAILURE);
        }
        
        close(fd);
        fd = open(DATA_FILE, O_RDONLY);
        if (fd == -1) {
            perror("Error opening file for reading");
            exit(EXIT_FAILURE);
        }
        char readfile[20000];
        ssize_t bytesRead;    
        while ((bytesRead = read(fd, readfile, sizeof(readfile))) > 0) {
        // Process or print the data in the buffer
        send(client_fd, readfile, bytesRead, 0);
        }
        close(fd);
        close(client_fd);

    }

    return 0;
}