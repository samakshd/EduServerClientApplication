#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//File To Store Database Information and Operations

//Server Configuration Parameters
#define MAX_COURSE_NAME_LEN 20
#define MAX_TEACHER_NAME_LEN 10
#define MAX_TEACHERS_PER_COURSE 10
#define MAX_TEACHERS 10
#define MAX_COURSES 15
#define MIN_TEACHERS 5
#define MIN_COURSES 10

//Final Configuration Parameters 
int max_courses;
int min_courses;
int max_teachers;
int min_teachers;

//Structure to define Teacher
//   - Teacher Name
struct teacher{
    char teacher_name[MAX_TEACHER_NAME_LEN];
};


//Structure to define Course
//   - Course Name
//   - List of Teachers allocated to that course
struct course_info{
    char course_name[MAX_COURSE_NAME_LEN];
    struct teacher* teachers_list[MAX_TEACHERS_PER_COURSE];
};

//Data Structure (Array of Course Info Structs) to store data 
struct course_info* DataTable[MAX_COURSES+1];

//Internal Logic For Course Allocation for teacher
int allocate_teacher_to_course(char* teacher_name) {
    
    int course_count = 0;
    for(int i=1;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            course_count++;
        }
    }
    int alloted_course;
    if(course_count == 0) {
        alloted_course = 0;
    }
    else {
        alloted_course = rand()%course_count + 1;
    }

    
    int index = 0;
    for(int i=0;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            if(index == alloted_course) {
                for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                    if((DataTable[i]->teachers_list)[j] == NULL) {
                        (DataTable[i]->teachers_list)[j] = (struct teacher*) malloc(sizeof(struct teacher));
                        strcpy((DataTable[i]->teachers_list)[j]->teacher_name,teacher_name);
                        return 0;
                    }
                }
            }
            index++;
        }
    }
}

int allocate_unallocated_teachers() {
    for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
        if(DataTable[0]->teachers_list[j] != NULL) {
            allocate_teacher_to_course(DataTable[0]->teachers_list[j]->teacher_name);
            DataTable[0]->teachers_list[j] = NULL;
        }
    }
    return 0;
}

//Function To Add Course
int add_course(char* course_name) {
    
    for(int i=1;i<max_courses+1;i++) {

        if(DataTable[i]!=NULL) {
            if(strcmp(course_name,DataTable[i]->course_name) == 0)
                return 1; //Duplicate Course
        }
    }

    for(int i=1;i<max_courses+1;i++) {
        if(DataTable[i] == NULL) {
            struct course_info *info = malloc(sizeof(struct course_info));
            strcpy(info->course_name,course_name);
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                (info->teachers_list)[j] = NULL;
            }
            DataTable[i] = info;
            allocate_unallocated_teachers();
            return 0; //Success
        }
    }


    return 2; //Course Limit Exceeded
}

//Function To Delete Course
int delete_course(char* course_name) {

    int flag = 1;
    int curr_count = 0;
    for(int i=1;i<max_courses+1;i++) {
        if(DataTable[i]!=NULL) {
            curr_count++;
        }
    }

    for(int i=1;i<max_courses+1;i++) {
        if(DataTable[i]!=NULL && strcmp(course_name,DataTable[i]->course_name)==0) {

            if(curr_count - 1 < min_courses) {
                return 2;//Preceded Minimum Course Limit
            }

            struct course_info* temp = DataTable[i];
            DataTable[i] = NULL;

            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(temp->teachers_list[j] != NULL) {
                    allocate_teacher_to_course(temp->teachers_list[j]->teacher_name);
                    free(temp->teachers_list[j]);
                }
            }

            flag = 0;
        }
    }

    return flag;
}

//Function To Add Teacher
int add_teacher(char* teacher_name) {

    int curr_count = 0;
    for(int i=0;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if((DataTable[i]->teachers_list)[j] != NULL) {
                    curr_count++;
                    if(strcmp((DataTable[i]->teachers_list)[j]->teacher_name,teacher_name) == 0) return 1;//Duplicate Teacher Found
                }
            }
        }
    }

    if(curr_count == max_teachers) {
        return 2; //Max Limit Exceeded
    }

    allocate_teacher_to_course(teacher_name);
    return 0; //Success
}

//Function to Delete Teacher
int delete_teacher(char* teacher_name) {

    int flag = 1;

    int curr_count = 0;
    for(int i=0;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(DataTable[i]->teachers_list[j] != NULL) {
                    curr_count++;
                }
            }
        }
    }
    int teacher_count =0;
    for(int i=0;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(DataTable[i]->teachers_list[j] != NULL && strcmp(DataTable[i]->teachers_list[j]->teacher_name,teacher_name) == 0) {
                    teacher_count++;
                }
            }
        }
    }

    if(curr_count - teacher_count < min_teachers) {
        return 2;//Preceded Minimum Teachers Limit
    }

    for(int i=0;i<max_courses+1;i++) {
        if(DataTable[i] != NULL) {
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(DataTable[i]->teachers_list[j] != NULL && strcmp(DataTable[i]->teachers_list[j]->teacher_name,teacher_name) == 0) {
                    free(DataTable[i]->teachers_list[j]);
                    DataTable[i]->teachers_list[j] = NULL;
                    flag = 0;
                }
            }
        }
    }

    return flag;
}

//Function To Print Report
int print_report() {

    printf("---------------------------------------------------------------\n");
    printf("---------- COURSE/TEACHER ALLOCATION SUMMARY REPORT------------\n");
    printf("---------------------------------------------------------------\n");

    for(int i=0;i<MAX_COURSES+1;i++) {
        if(DataTable[i] == NULL) {
            continue;
        }
        else {
            printf("%s: ",DataTable[i]->course_name);
            for(int j=0;j<MAX_TEACHERS_PER_COURSE;j++) {
                if(DataTable[i]->teachers_list[j] == NULL) {
                    continue;
                }
                else {
                    printf("%s",DataTable[i]->teachers_list[j]->teacher_name);
                    if(j<MAX_TEACHERS_PER_COURSE-1)
                        printf(",");
                }
            }
        }
        printf("\n");
    }
    printf("---------------------------------------------------------------\n");
    printf("---------------------------------------------------------------\n\n");
}