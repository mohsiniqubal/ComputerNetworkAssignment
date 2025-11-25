// server.c  (TCP MULTI-CLIENT FRUIT SERVER)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 9000

struct Fruit {
    char name[20];
    int qty;
    char lastSold[50];
} fruits[3] = {
    {"apple", 20, "N/A"},
    {"banana", 15, "N/A"},
    {"mango", 10, "N/A"}
};

char customers[20][50];
int customerCount = 0;

int isNewCustomer(char *entry) {
    for (int i = 0; i < customerCount; i++)
        if (strcmp(customers[i], entry) == 0)
            return 0;
    return 1;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("SERVER RUNNING on PORT %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

        char clientEntry[50];
        sprintf(clientEntry, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (isNewCustomer(clientEntry)) {
            strcpy(customers[customerCount++], clientEntry);
        }

        char buffer[100];
        recv(new_socket, buffer, sizeof(buffer), 0);

        char fruit[20];
        int qty;
        sscanf(buffer, "%s %d", fruit, &qty);

        char response[500];
        int found = 0;

        for (int i = 0; i < 3; i++) {
            if (strcmp(fruit, fruits[i].name) == 0) {
                found = 1;
                if (fruits[i].qty >= qty) {
                    fruits[i].qty -= qty;

                    time_t t = time(NULL);
                    strcpy(fruits[i].lastSold, ctime(&t));
                    fruits[i].lastSold[strlen(fruits[i].lastSold)-1] = '\0';

                    sprintf(response,
                        "Purchase SUCCESS!\nFruit: %s\nRemaining Qty: %d\nLast Sold: %s\n\n",
                        fruits[i].name, fruits[i].qty, fruits[i].lastSold);
                } else {
                    sprintf(response, "SORRY! Not enough stock for %s\n", fruit);
                }
                break;
            }
        }

        if (!found) sprintf(response, "Fruit not found!\n");

        // append customer list & count
        strcat(response, "Customers so far:\n");
        for (int i = 0; i < customerCount; i++)
            strcat(response, customers[i]), strcat(response, "\n");

        char countStr[50];
        sprintf(countStr, "\nTotal Unique Customers: %d\n", customerCount);
        strcat(response, countStr);

        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
}

