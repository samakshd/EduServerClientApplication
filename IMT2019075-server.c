// OS LAB Mini Project Edu Server
// Compilation of this file
// gcc -D_REENTRANT -o edu_server IMT2019075-server.c -lrt -lpthread
// Run this file: ./edu_server

//Including Basic Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

//Contains Database Information and Operations
#include "IMT2019075-database.c"

//Including POSIX MQUEUE Library
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

//Including POSIX Threads and Semaphores Library
#include <unistd.h>   // sleep() is declared here
#include <pthread.h>
#include <semaphore.h>

//Library to Detect Signal Interupts
#include <signal.h>

void INThandler(int);


int pthread_join(pthread_t thread, void **status_ptr);
void *report_thread_function(void *);

//Semaphore to protect various regions
sem_t bin_sem;

#define MSG_VAL_LEN  200
// For the client queue message
#define CLIENT_Q_NAME_LEN 16

// For the server queue message
#define MSG_TYPE_LEN 16

//Client Message Structure
typedef struct{
char client_q[CLIENT_Q_NAME_LEN];
char msg_val[MSG_VAL_LEN];
} client_msg_t;

//Server Message Structure
typedef struct{
char msg_type[MSG_TYPE_LEN];
char msg_val[MSG_VAL_LEN];
} server_msg_t;


static client_msg_t client_msg;

//Defining MQUEUE Parameters
#define SERVER_QUEUE_NAME   "/edu_server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES) 


//Function To Initialize Configuration Parameters
void set_configurations(int status) {

    //Protected By Semaphore so that Report does not get printed during taking input of parameters from user
    sem_wait(&bin_sem);

    //If Existing Data Log File is Loaded, Parameters are not asked from user and set to Server Parameters
    if(status == 0) {
        min_courses = MIN_COURSES;
        max_courses = MAX_COURSES;
        min_teachers = MIN_TEACHERS;
        max_teachers = MAX_TEACHERS;
        sem_post(&bin_sem);
        return;
    }

    //Taking Input of Configuration Parameters from User
    printf("Setting The Configurations:-\n\n");
    printf("Enter Minimum Courses Limit: ");
    scanf("%d", &min_courses);
    
    printf("Enter Maximum Courses Limit: ");
    scanf("%d", &max_courses);

    printf("Enter Minimum Teacher Limit: ");
    scanf("%d", &min_teachers);

    printf("Enter Maximum Teacher Limit: ");
    scanf("%d", &max_teachers);
    printf("\n");

    //Checking the Boundaries of User input Configuration Parameters and Adjusting them accordingly

    if(min_courses < MIN_COURSES || min_courses > MAX_COURSES) {

        printf("Minimum Course Limit Out of Bounds: Setting to Server's Configuration\n");
        min_courses = MIN_COURSES;
    }

    if(max_courses > MAX_COURSES || max_courses < MIN_COURSES) {
        printf("Maximum Course Limit Out of Bounds: Setting to Server's Configuration\n");
        max_courses = MAX_COURSES;
    }

    if(min_teachers < MIN_TEACHERS || min_teachers > MAX_TEACHERS) {
        printf("Minimum Teacher Limit Out of Bounds: Setting to Server's Configuration\n");
        min_teachers = MIN_TEACHERS;
    }

    if(max_teachers > MAX_TEACHERS || max_teachers < MIN_TEACHERS) {
        printf("Maximum Teacher Limit Out of Bounds: Setting to Server's Configuration\n");
        max_teachers = MAX_TEACHERS;
    }

    printf("\n");

    sem_post(&bin_sem);
}

//Function To load data from existing Data Log File After Server Restart
int load_data_from_file() {
    FILE* fp;
    fp = fopen("data.txt","r");
    if(fp==NULL) {
        return 1;
    }
    char dataRead[100];
    int row = 0;
    while(fscanf(fp,"%[^\n] ",dataRead) != EOF) {
        char* end_line;
        char* token = strtok_r(dataRead,",",&end_line);
        int i =-1;
        while(token) {
            if(i==-1) {
                if(strcmp(token,"NULL")==0) {
                    break;
                }
                else {
                    struct course_info *info = malloc(sizeof(struct course_info));
                    strcpy(info->course_name,token);
                    DataTable[row] = info;
                }
            }
            else {
                if(strcmp(token,"NULL")!=0) {
                    (DataTable[row]->teachers_list)[i] = (struct teacher*) malloc(sizeof(struct teacher));
                    strcpy((DataTable[row]->teachers_list)[i]->teacher_name,token);
                }
            }
            i++;
            token = strtok_r(NULL, ",",&end_line);
        }

        row++;
    }
    fclose(fp);
    return 0;
}


//Function To Parse the Command Message Recieved from Client
//Protected By Semaphore as it is part where data is accessed
int parse_command(char* command, char* output) {

    sem_wait(&bin_sem);
    char* end_command;
    char* token1 = strtok_r(command, " ",&end_command);
    char* token2 = strtok_r(NULL, " ",&end_command);

    if(strcmp(token1,"ADD_COURSE") == 0) {
        char* end_token;
        char* token = strtok_r(token2,",",&end_token);
        while(token) {
            strcat(output,token);
            strcat(output,": ");

            int status = add_course(token);
            if(status == 0) {
                strcat(output,"Course Added Successfully");
            }
            else if(status == 1) {
                strcat(output,"Error: Course Duplicate Found");
            }
            else if(status == 2) {
                strcat(output,"Error: Course Limit Exceeded");
            }
            strcat(output,"\n");

            token = strtok_r(NULL,",",&end_token);
        }
    }
    else if(strcmp(token1,"DEL_COURSE") == 0) {
        char* end_token;
        char* token = strtok_r(token2,",",&end_token);
        while(token) {
            strcat(output,token);
            strcat(output,": ");

            int status = delete_course(token);
            if(status == 0) {
                strcat(output,"Course Deleted Successfully");
            }
            else if(status == 1) {
                strcat(output,"Error: Course Not Found");
            }
            else if(status == 2) {
                strcat(output,"Error: Minimum Course Limit Preceded");
            }

            strcat(output,"\n");
            token = strtok_r(NULL,",",&end_token);
        }
    }
    else if(strcmp(token1,"ADD_TEACHER") == 0) {
        char* end_token;
        char* token = strtok_r(token2,",",&end_token);
        while(token) {
            strcat(output,token);
            strcat(output,": ");

            int status = add_teacher(token);
            if(status == 0) {
                strcat(output,"Teacher Added Successfully");
            }
            else if(status == 1) {
                strcat(output,"Error: Teacher Duplicate Found");
            }
            else if(status == 2) {
                strcat(output,"Error: Max Teachers Limit Exceeded");
            }
            strcat(output,"\n");
            token = strtok_r(NULL,",",&end_token);
        }
    }
    else if(strcmp(token1,"DEL_TEACHER") == 0) {
        char* end_token;
        char* token = strtok_r(token2,",",&end_token);
        while(token) {
            strcat(output,token);
            strcat(output,": ");

            int status = delete_teacher(token);
            if(status == 0) {
                strcat(output,"Teacher Deleted Successfully");
            }
            else if(status == 1) {
                strcat(output,"Error: No Teacher Found");
            }
            else if(status == 2) {
                strcat(output,"Error: Minimum Teachers Limit Preceded");
            }

            strcat(output,"\n");
            token = strtok_r(NULL,",",&end_token);
        }
    }
    else {
        strcat(output,"No Such Command Exists\n");
    } 

    sem_post(&bin_sem);   
}



int main (int argc, char **argv)
{   

    signal(SIGINT, INThandler);
    int data_file_status = load_data_from_file();

    int res_thread,res_sem;
    pthread_t report_thread;

    // Initialize semaphore
    // Pointer to Semaphore handle, pshared = 0, value = 1 (free)  
    res_sem = sem_init(&bin_sem, 0, 1);  
    
    if(res_sem != 0) {
        printf("Semaphore creation failure: %d\n", res_sem);
        exit(1);
    }  

    /* Create independent threads each of which will execute different threead functions */
    if( (res_thread = pthread_create( &report_thread, NULL, &report_thread_function, "Report Thread")) )  {
        printf("Report Thread creation failed: %d\n", res_thread);
        exit(1);
    }

    mqd_t qd_srv, qd_client;   // Server and Client Msg queue descriptors

    printf("\n");
    printf("--------------------------\n");
    printf("|| WELCOME TO EDU SERVER ||\n");
    printf("--------------------------\n\n");

    printf("Instructions:-\n\n");
    printf("1. Summary Report will be generated in every 10 Seconds\n");
    printf("2. Exit Server: Ctrl + C\n\n");

    if(data_file_status == 0) {
        printf("Data File: Successfully Loaded\n\n");
    } 
    else if(data_file_status == 1) {
        printf("Data File: Does not exist\n\n");

        //Seperate Row for Unallocated Teachers
        struct course_info *info = malloc(sizeof(struct course_info));
        strcpy(info->course_name,"Unallocated Teachers");
        for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
            (info->teachers_list)[j] = NULL;
        }
        DataTable[0] = info;
    }   

    //Setting The Configurations
    set_configurations(data_file_status);

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                           &attr)) == -1) {
        perror ("Server MsgQ: mq_open (qd_srv)");
        exit (1);
    }

    client_msg_t in_msg;
    char output[200];
    
    while (1) {
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_srv,(char *) &in_msg, MAX_MSG_SIZE, NULL) == -1) {
            perror ("Server msgq: mq_receive");
            exit (1);
        }

        printf("Message from Client %s: %s\n\n", in_msg.client_q,in_msg.msg_val);

        
        strcpy(output,"");
        parse_command(in_msg.msg_val,output);
        
        server_msg_t out_msg; 
        strcpy(out_msg.msg_type, "Server msg");   // strcpy(destPtr, srcPtr)  
        strcpy(out_msg.msg_val, output);
		             		       
		// Open the client queue using the client queue name received
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1) {
            perror ("Server MsgQ: Not able to open the client queue");
            continue;
        }     
        
        // Send back the status message back to client        
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send(qd_client, (char *) &out_msg, sizeof(out_msg), 0) == -1) {
            perror ("Server MsgQ: Not able to send message to the client queue");
            continue;
        }  

    } // end of while(1) 
    
}  // end of main()

//Seperate Thread Function to print Report Summary after every 10 seconds
//Protected by Semaphore as it accesses the database
void *report_thread_function(void *pThreadName)
{
    while(1) {
        sleep(10);
        sem_wait(&bin_sem);
        print_report();
        sem_post(&bin_sem);   
    }   
    
} // end of report_thread_function()

//Saving the state of data in text file so that can be accessed after server restarts
void save_data_to_file() {
    
    FILE *fp;
    fp = fopen("data.txt", "w");
    if(fp == NULL ) {
        printf( "data.txt file failed to open." );
        return;
    }

    char output[100];
    for(int i=0;i<MAX_COURSES+1;i++) {
        if(DataTable[i] == NULL) {
            fprintf(fp,"NULL");
        }
        else {
            fprintf(fp,"%s,",DataTable[i]->course_name);
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(DataTable[i]->teachers_list[j] == NULL) {
                    fprintf(fp,"NULL");
                }
                else {
                    fprintf(fp,"%s",DataTable[i]->teachers_list[j]->teacher_name);
                }

                if(j<MAX_TEACHERS_PER_COURSE-1)
                    fprintf(fp,",");
            }
        }
        fprintf(fp,"\n");
    }

    fclose(fp);
}

//Function to catch Ctrl + C Interupt and perform saving of data log file accordingly
void  INThandler(int sig) {
    signal(sig, SIG_IGN);
    save_data_to_file();
    printf("\nData Log File Saved Successfully\n\n");
    exit(0);
}