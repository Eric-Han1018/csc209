#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define BUF_SIZE 128

#define MAX_AUCTIONS 5
#ifndef VERBOSE
#define VERBOSE 0
#endif

#define ADD 0
#define SHOW 1
#define BID 2
#define QUIT 3

/* Auction struct - this is different than the struct in the server program
 */
struct auction_data {
    int sock_fd;
    char item[BUF_SIZE];
    int current_bid;
};

/* Displays the command options available for the user.
 * The user will type these commands on stdin.
 */

void print_menu() {
    printf("The following operations are available:\n");
    printf("    show\n");
    printf("    add <server address> <port number>\n");
    printf("    bid <item index> <bid value>\n");
    printf("    quit\n");
}

/* Prompt the user for the next command 
 */
void print_prompt() {
    printf("Enter new command: ");
    fflush(stdout);
}


/* Unpack buf which contains the input entered by the user.
 * Return the command that is found as the first word in the line, or -1
 * for an invalid command.
 * If the command has arguments (add and bid), then copy these values to
 * arg1 and arg2.
 */
int parse_command(char *buf, int size, char *arg1, char *arg2) {
    int result = -1;
    char *ptr = NULL;
    if(strncmp(buf, "show", strlen("show")) == 0) {
        return SHOW;
    } else if(strncmp(buf, "quit", strlen("quit")) == 0) {
        return QUIT;
    } else if(strncmp(buf, "add", strlen("add")) == 0) {
        result = ADD;
    } else if(strncmp(buf, "bid", strlen("bid")) == 0) {
        result = BID;
    } 
    ptr = strtok(buf, " "); // first word in buf

    ptr = strtok(NULL, " "); // second word in buf
    if(ptr != NULL) {
        strncpy(arg1, ptr, BUF_SIZE);
    } else {
        return -1;
    }
    ptr = strtok(NULL, " "); // third word in buf
    if(ptr != NULL) {
        strncpy(arg2, ptr, BUF_SIZE);
        return result;
    } else {
        return -1;
    }
    return -1;
}

/* Connect to a server given a hostname and port number.
 * Return the socket for this server
 */
int add_server(char *hostname, int port) {
        // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }
    
    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    struct addrinfo *ai;
    
    /* this call declares memory and populates ailist */
    if(getaddrinfo(hostname, NULL, NULL, &ai) != 0) {
        close(sock_fd);
        return -1;
    }
    /* we only make use of the first element in the list */
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;

    // free the memory that was allocated by getaddrinfo for this list
    freeaddrinfo(ai);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        return -1;
    }
    if(VERBOSE){
        fprintf(stderr, "\nDebug: New server connected on socket %d.  Awaiting item\n", sock_fd);
    }
    return sock_fd;
}
/* ========================= Add helper functions below ========================
 * Please add helper functions below to make it easier for the TAs to find the 
 * work that you have done.  Helper functions that you need to complete are also
 * given below.
 */

/* Print to standard output information about the auction
 */
void print_auctions(struct auction_data *a, int size) {
    printf("Current Auctions:\n");
    for(int i = 0; i < size; i++){
        if (a[i].sock_fd != -1){
            printf("(%d) %s bid = %d\n", i, a[i].item, a[i].current_bid);
        } 
    }
    /* TODO Print the auction data for each currently connected 
     * server.  Use the follosing format string:
     *     "(%d) %s bid = %d\n", index, item, current bid
     * The array may have some elements where the auction has closed and
     * should not be printed.
     */
}

/* Process the input that was sent from the auction server at a[index].
 * If it is the first message from the server, then copy the item name
 * to the item field.  (Note that an item cannot have a space character in it.)
 */
void update_auction(char *buf, int size, struct auction_data *a, int index) {
    char *name, *bid, *time;
    
    name = strtok(buf, " "); // first word in buf
    if(a[index].current_bid < 0){
        strcpy(a[index].item, name);
        a[index].item[strlen(name)+1] = '\0';
    }
    // check item name? fprintf? 
    bid = strtok(NULL, " "); // second word in buf
    int new_bid = atoi(bid);
    if(bid != NULL) {
        if(new_bid > a[index].current_bid){
            a[index].current_bid = new_bid;
        }
    } 
    else {
        fprintf(stderr, "ERROR malformed bid: %s", buf);
        exit(1);
    }
    time = strtok(NULL, " "); // third word in buf
    int sec = atoi(time);
    if(time == NULL) {
        fprintf(stderr, "ERROR malformed bid: %s", buf);
        exit(-1); 
    } 
    printf("\nNew bid for %s [%d] is %d (%d seconds left)\n", name, index, new_bid, sec);
}

int main(void) {

    char name[BUF_SIZE];
    char arg1[BUF_SIZE], arg2[BUF_SIZE];
    char buf[BUF_SIZE];

    // Declare and initialize necessary variables
    struct auction_data auctions[MAX_AUCTIONS];
    for (int i = 0; i < MAX_AUCTIONS; i++) {
        auctions[i].sock_fd = -1;
        auctions[i].item[0] = '\0';
        auctions[i].current_bid = -1;
    }

    // Get the user to provide a name.
    printf("Please enter a username: ");
    fflush(stdout);
    int num_read = read(STDIN_FILENO, name, BUF_SIZE);
    name[num_read] = '\0';
    if(num_read <= 0){
        fprintf(stderr, "ERROR: read from stdin failed\n");
        exit(1);
    }
    print_menu();

    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(STDIN_FILENO, &all_fds);
    int max_fd = STDIN_FILENO;
    while(1) {
        print_prompt();
        fd_set listen_fds = all_fds;
        if (select(max_fd + 1, &listen_fds, NULL, NULL, NULL) == -1) {
            perror("client: select");
            exit(1);
        }
        if (FD_ISSET(STDIN_FILENO, &listen_fds)) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if(num_read <= 0){
                fprintf(stderr, "ERROR: read from stdin failed\n");
                exit(1);
            }
            buf[num_read] = '\0';
            int command = parse_command(buf, num_read, arg1, arg2);
            if(command == ADD){
                int auction_index = 0;
                while (auction_index < MAX_AUCTIONS && auctions[auction_index].sock_fd != -1) {
                    auction_index++;
                }
                if(auction_index == MAX_AUCTIONS){
                    fprintf(stderr, "ERROR: Auctions are full\n");
                }
                else{
                    int port = atoi(arg2);
                    int fd = add_server(arg1, port);
                    if(fd != -1) {
                        if(fd > max_fd){
                            max_fd = fd;
                        }
                        num_read = write(fd, name, strlen(name));
                        if(num_read <= 0){
                            perror("client: write");
                            close(fd);
                            exit(1);
                        }
                        auctions[auction_index].sock_fd = fd;
                        FD_SET(fd, &all_fds);
                    }
                }
            }
            else if(command == BID){
                int index = atoi(arg1);
                if(index >= MAX_AUCTIONS || auctions[index].sock_fd == -1){
                    fprintf(stderr, "There is no auction open at %d\n", index);
                }
                else{
                    if(write(auctions[index].sock_fd, arg2, sizeof(char)*num_read)==-1){
                        perror("client: write");
                        close(auctions[index].sock_fd);
                        exit(1);
                    };
                }
            }
            else if(command == SHOW){
                print_auctions(auctions, MAX_AUCTIONS);
            }
            else if(command == QUIT){
                for(int i = 0; i < MAX_AUCTIONS; i++){
                    if(auctions[i].sock_fd != -1){
                        if(close(auctions[i].sock_fd) == -1){
                            perror("close");            
                            exit(1);
                        }   
                    }
                }
                exit(0);
            }
        }
        // Auction
        for (int k = 0; k < MAX_AUCTIONS; k++) {
            if (auctions[k].sock_fd > -1 && FD_ISSET(auctions[k].sock_fd, &listen_fds)) {
                int num = read(auctions[k].sock_fd, buf, sizeof(char)*BUF_SIZE);
                if(num <= 0){
                    fprintf(stderr, "ERROR: read from server failed\n");
                    exit(1);
                }
                buf[num] = '\0';
                if(strncmp(buf, "Auction closed:", strlen("Auction closed:"))== 0){         //include "Auction closed:", means being closed
                    printf("%s", buf);
                    FD_CLR(auctions[k].sock_fd, &all_fds);
                    if(close(auctions[k].sock_fd) == -1){
                        perror("close");            
                        exit(1);
                    }  
                    auctions[k].sock_fd = -1;
                    auctions[k].current_bid = -1;
                    auctions[k].item[0] = '\0';
                }
                else{
                    update_auction(buf, num, auctions, k);
                }
            }
        }
    }
    return 0; // Shoud never get here
}
