// OS Lab Mini Project Client
// Compilation of this file
// gcc -o client IMT2019075-client.c -lrt
// Run This File:- ./client
// Run this file only after server is running


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>

#define MSG_VAL_LEN  200
// For the client queue message
#define CLIENT_Q_NAME_LEN 16

// For the server queue message
#define MSG_TYPE_LEN 16

typedef struct{
    char client_q[CLIENT_Q_NAME_LEN];
    char msg_val[MSG_VAL_LEN];
} client_msg_t;

typedef struct{
    char msg_type[MSG_TYPE_LEN];
    char msg_val[MSG_VAL_LEN];
} server_msg_t;

#define SERVER_QUEUE_NAME   "/edu_server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

#define MAX_COMMAND_SIZE 100

int main (int argc, char **argv)
{
    char client_queue_name [64];
    mqd_t qd_srv, qd_client;   // Server and Client Msg queue descriptors


    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client MsgQ: mq_open (qd_srv)");
        exit (1);
    }

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    client_msg_t out_msg;
    // create the client queue for receiving messages from the server
    sprintf (out_msg.client_q, "/clientQ-%d", getpid ());

    if ((qd_client = mq_open(out_msg.client_q, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                           &attr)) == -1) {
        perror ("Client msgq: mq_open (qd_client)");
        exit (1);
    }

    printf("\n");
    printf("--------------------------\n");
    printf("|| WELCOME TO EDU CLIENT ||\n");
    printf("--------------------------\n\n");

    printf("Commands Available:-\n\n");
    printf("1. ADD_COURSE C1,C2,....\n");
    printf("2. DEL_COURSE C1,C2.....\n");
    printf("3. ADD_TEACHER T1,T2.....\n");
    printf("4. DEL_TEACHER T1,T2.....\n");
    printf("5. EXIT CLIENT\n\n");

    while (1) {
        
        printf("Enter the command: ");
        char command[MAX_COMMAND_SIZE];
        scanf("%[^\n]%*c",command);

        if(strcmp(command,"EXIT CLIENT") == 0) {
            break;
        }

		strcpy(out_msg.msg_val,command);         
        // send message to my_msgq_rx queue
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send (qd_srv, (char *) &out_msg, sizeof(out_msg), 0) == -1) {
            perror ("Client MsgQ: Not able to send message to the queue /server_msgq");
            continue;
        }

        printf("----Message sent successfully to Server Queue-----\n\n");

        sleep(1);  // sleep for 1 seconds

		server_msg_t in_msg;
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_client,(char *) &in_msg, MAX_MSG_SIZE, NULL) == -1) {
            perror ("Client MsgQ: mq_receive from server");
            exit (1);
        }
        
        printf("---------------------\n");
        printf("Server Reply Message\n");
        printf("---------------------\n");
        printf("%s",in_msg.msg_val);
        printf("---------------------\n\n");

        //Unlinking for allowing sending long messages
        mq_unlink(SERVER_QUEUE_NAME);
    }

    printf ("Client MsgQ: Bye\n");

    exit (0);
}
