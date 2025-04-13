#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<stdbool.h>

#define PORT 8898
#define MAX_CONN 10
#define BUFFER_SIZE 1024

typedef struct {
    int client_sock;
    struct sockaddr_in client_addr;
} client_data_t;

void *handle_client(void* arg) {
    client_data_t *data=(client_data_t *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read=read(data->client_sock, buffer, sizeof(buffer)))>0) {
        FILE *fp=fopen("database.txt", "r+");
        if (fp==NULL) {
            perror("File open failed");
            close(data->client_sock);
            free(data);
            pthread_exit(NULL);
        }
        buffer[bytes_read]='\0';
        // printf("%s\n", buffer);
        char *command=strtok(buffer, " ");
        if (!strcmp(command, "put")) {
            char* key=strtok(NULL, " "), *value=strtok(NULL, " ");
            char input[BUFFER_SIZE];
            bool found=0;
            while (fgets(input, BUFFER_SIZE, fp)!=NULL) {
                char* key_in=strtok(input, ",");
                if (!strcmp(key_in, key)) {
                    // strcpy(buffer, "Error: Key already exists in file.");
                    // buffer[strlen(buffer)]='\0';
                    // write(data->client_sock, buffer, strlen(buffer));
                    found=1;
                    // break;
                }
            }
            if (found) {
                strcpy(buffer, "Error: Key already exists in file.");
                // buffer[strlen(buffer)]='\0';
                write(data->client_sock, buffer, strlen(buffer));
            }
            else {
                fwrite("\n", 1, strlen("\n"), fp);
                fwrite(key, 1, strlen(key), fp);
                fwrite(",", 1, strlen(","), fp);
                fwrite(value, 1, strlen(value), fp);
                strcpy(buffer, "OK");
                // buffer[strlen(buffer)]='\0';
                write(data->client_sock, buffer, strlen(buffer));
            }
        }
        else if (!strcmp(command, "get")) {
            char* key=strtok(NULL, " ");
            char input[BUFFER_SIZE];
            bool found=0;
            while (fgets(input, BUFFER_SIZE, fp)!=NULL) {
                char* key_in=strtok(input, ",");
                if (!strcmp(key_in, key)) {
                    found=1;
                    break;
                }
            }
            if (found) {
                char* val_in=strtok(NULL, ",");
                strcpy(buffer, val_in);
                if (buffer[strlen(buffer)-1]=='\n') {
                    buffer[strlen(buffer)-1]='\0';
                }
                write(data->client_sock, buffer, strlen(buffer));
            }
            else {
                strcpy(buffer, "Error: No matching entry for key.");
                write(data->client_sock, buffer, strlen(buffer));
            }
        }
        else if (!strcmp(command, "del")) {
            char* key=strtok(NULL, " ");
            char input[BUFFER_SIZE];
            bool found=0;
            size_t input_len;
            while (fgets(input, BUFFER_SIZE, fp)!=NULL) {
                input_len=strlen(input);
                char* key_in=strtok(input, ",");
                if (!strcmp(key_in, key)) {
                    found=1;
                    break;
                }
            }
            // printf("%d %s\n", input_len, input);
            if (found) {
                fseek(fp, -input_len, SEEK_CUR);
                if (input[input_len-1]=='\n') --input_len;
                while (input_len--) fwrite("\\", 1, strlen("\\"), fp);
                strcpy(buffer, "OK");
                write(data->client_sock, buffer, strlen(buffer));
            }
            else {
                strcpy(buffer, "Error: No matching entry for key.");
                write(data->client_sock, buffer, strlen(buffer));
            }
        }
        else if (!strcmp(command, "Bye")) {
            strcpy(buffer, "Goodbye");
            write(data->client_sock, buffer, strlen(buffer));
            close(data->client_sock);
            free(data);
            fclose(fp);
            pthread_exit(NULL);
        }
        else {
            strcpy(buffer, "Error: Invalid command.");
            write(data->client_sock, buffer, strlen(buffer));
        }
        fclose(fp);
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len=sizeof(client_addr);
    pthread_t thread_id;
    if ((server_sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
        perror("Socket creation failed");
        exit(1);
    }
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
        perror("Bind failed");
        exit(1);
    }
    if (listen(server_sock, MAX_CONN)==-1) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }
    printf("Server listening on port %d...\n", PORT);
    while (1) {
        client_sock=accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock==-1) {
            perror("Accept failed");
            close(client_sock);
            continue;
        }
        printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        client_data_t *data=malloc(sizeof(client_data_t));
        data->client_sock=client_sock;
        data->client_addr=client_addr;
        if (pthread_create(&thread_id, NULL, handle_client, data)!=0) {
            perror("Thread creation failed");
            free(data);
            close(client_sock);
        }
        pthread_detach(thread_id);
    }
    close(server_sock);
    return 0;
}