#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"
#include "projects/crossroads/source-list.h"

#define clear() printf("\033c")

//path[1][0]-> B(1)에서 A(0)로 가는 경로의 row와 col value
//argv[0]이 실행할 때 첫 번째 파라미터
//argv[1]이 실행할 때 두 번째 파라미터 => aAB:bBC:cCA

static struct source_to_dest *vehicle_list;
char map_draw[7][7];
struct semaphore * n, * s;
struct semaphore * control_creating_thread;
struct semaphore * control_behind_vehicle[3];
struct semaphore * control_duplicate_vehicle;
struct semaphore * control_flag, * control_flag2, * control_flag3;

int vehicle_index = 0;
int vehicle_num = 0;
int finish_check = 0;
int param_size;
int priority_num = 1;

struct source_list *a_list;
struct source_list *b_list;
struct source_list *c_list;

void run_crossroads(char **argv)
{
    int i;

	printf("implement crossroads !!!\n");

    param_size = strlen(argv[1]);

    // vehicle, source, destination 넣기
    vehicle_list = insert_vehicle(argv, param_size);

    initialize();
    
    // create main thread
    thread_create("main", PRI_DEFAULT, main_func, NULL);

    for(i=0; i<max_list_size(); i++){
        if(i < a_list->index ){
            sema_down(control_creating_thread);
            sema_down(control_flag);
            if(map_flag[4][0] == 1){
                printf("control sema\n");
                sema_down(control_duplicate_vehicle);
                printf("escape control sema\n");
            }
            
            thread_create(a_list->list[i], priority_num++, vehicle_func, NULL);
        }
        if(i < b_list->index ){
            sema_down(control_creating_thread);
            sema_down(control_flag2);
            if(map_flag[6][4] == 1){
                printf("%s control sema\n", b_list->list[i]);
                sema_down(control_duplicate_vehicle);
                printf("%s escape control sema\n", b_list->list[i]);
            }
             
            thread_create(b_list->list[i], priority_num++, vehicle_func, NULL);
        }
        if(i < c_list->index ){
            sema_down(control_creating_thread);
            sema_down(control_flag3);
            if(map_flag[2][6] == 1){
                printf("control sema\n");
                sema_down(control_duplicate_vehicle);
                printf("escape control sema\n");
            }
            
            thread_create(c_list->list[i], priority_num++, vehicle_func, NULL);
        }
        timer_sleep(100); //1초 동안 block
    }
    
}

void main_func(){
    while(true){
        print_map();

        timer_sleep(100); //1초 동안 block
        if(vehicle_num == finish_check){
            // initialize map
            strlcpy(map_draw, map_draw_default, sizeof map_draw_default + 1);    
            print_map() ;
            break;
        }
            
    }
    
}

void vehicle_func(){    
    int source, dest;
    int original_source, original_dest;
    int i, j, temp_i;
    char * temp_name = thread_name();
    int index;
    bool flag[3] = {false, false, false};

    index = get_index(temp_name);
    
    // t_name[0] -> '/0' 제외한 vehicle name
    // a's ASCII code : 97
    // A's ASCII code : 65

    //printf("priority : %d\n", thread_get_priority());
    //printf("%d : %c, (%c, %c)\n", vehicle_index, temp_name[0], vehicle_list[temp].source, vehicle_list[temp].dest);

    original_source = vehicle_list[index].source;
    original_dest = vehicle_list[index].dest;

    source = original_source - 65;
    dest = original_dest - 65;
    
    i=1;

    printf("%c : %d, %d\n", temp_name[0], path[source][dest][0].row, path[source][dest][0].col);
    // draw map
    map_draw[path[source][dest][0].row][path[source][dest][0].col] = vehicle_list[index].vehicle[0];
    map_flag[path[source][dest][0].row][path[source][dest][0].col] = 1;

    sema_up(control_creating_thread);
    timer_sleep(100); //1초 동안 block
    

    while(! (path[source][dest][i].row == -1 && path[source][dest][i].col == -1)){
        temp_i = i;

        
        if(flag[source] == false){
            if(map_flag[path[source][dest][temp_i].row][path[source][dest][temp_i].col] == 1){
                printf("%c control sema in!\n", temp_name[0]);
                sema_down(control_behind_vehicle[source]);
                printf("%c : ########\n", temp_name[0]);
            }
        }
        else if(flag[source] == true){
            printf("%c : fdsfsfsf\n", temp_name[0]);
            sema_up(control_behind_vehicle[source]);
            flag[source] = false;

            if(control_creating_thread->value == 0)
                sema_up(control_creating_thread);
        }        
        

        // draw vehicle
        printf("%c : %d, %d\n", temp_name[0], path[source][dest][i].row, path[source][dest][i].col);
        map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = temp_name[0];
        sema_up(control_duplicate_vehicle);

        // set flag
        // flag 1 : using map
        // flag 0 : not using map
        
        map_flag[path[source][dest][temp_i-1].row][path[source][dest][temp_i-1].col] = 0;
        map_flag[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = 1;
        

        if(original_source == 'A' && control_flag->value == 0)
            sema_up(control_flag);
        else if (original_source == 'B' && control_flag2->value == 0 ){
            sema_up(control_flag2);
        }
        else if (original_source == 'C' && control_flag3->value == 0)
            sema_up(control_flag3);


        // remove vehicle
        temp_i = i - 1;
        map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = ' ';

        // map redraw
        if(path[source][dest][temp_i].row == 3){
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = '-';
        }
        else if( (path[source][dest][temp_i].row == 2 || path[source][dest][temp_i].row == 4) && path[source][dest][temp_i].col == 3){
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = '-';
        }
        else{
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = ' ';
        }

        // 교차로 제어
        if ( ( path[source][dest][i].row == 2 && path[source][dest][i].col == 5 ) || 
            ( path[source][dest][i].row == 4 && path[source][dest][i].col == 1 ) || 
            ( path[source][dest][i].row == 5 && path[source][dest][i].col == 4 ) ){
                // A->B일 때, B->C, C->A만 이동 가능
                if( original_source == 'A' && original_dest == 'B' ){
                    for(j=0; j<vehicle_num; j++){
                        if( !strcmp(vehicle_list[j].vehicle, temp_name))
                            continue;
                        else{
                            if( ( vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B' ) || ( vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'C' ) || 
                            ( vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A' ) || ( vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'A' ) ){
                                continue;
                            }
                            else{
                                printf("in sema1\n");
                                sema_down(n);
                                flag[source] = false;
                                thread_set_priority(PRI_MAX);
                                flag[source] = true;
                                break;
                            }
                        }
                    }
                }
                else if(original_source == 'A' && original_dest == 'C' ){
                    printf("in AC\n");
                    for(j=0; j<vehicle_num; j++){
                        if( !strcmp(vehicle_list[j].vehicle, temp_name))
                            continue;
                        else{
                            if( ( vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'C' ) || ( vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A' ) ||
                            ( vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'A' ) ){
                                continue;
                            }
                            else{
                                printf("in sema2\n");
                                sema_down(n);
                                flag[source] = false;
                                thread_set_priority(PRI_MAX);
                                flag[source] = true;
                                break;
                            }
                        }
                    }
                }
                else if(original_source == 'B' && original_dest == 'A' ){
                    printf("in BA\n");
                    for(j=0; j<vehicle_num; j++){
                        if( !strcmp(vehicle_list[j].vehicle, temp_name))
                            continue;
                        else{
                            if( ( vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'A' ) || ( vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B' ) ){
                                continue;
                            }
                            else{
                                printf("%c : in sema3\n", temp_name[0]);
                                sema_down(n);
                                flag[source] = false;
                                thread_set_priority(PRI_MAX);
                                flag[source] = true;
                                break;
                            }
                        }
                    }
                }
                else if(original_source == 'B' && original_dest == 'C' ){
                    printf("in BC\n");
                    for(j=0; j<vehicle_num; j++){
                        if( !strcmp(vehicle_list[j].vehicle, temp_name))
                            continue;
                        else{
                            if( ( vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'C' ) || ( vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A' )||
                            ( vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B' ) || ( vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'C' ) ||
                            ( vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'B' ) ){
                                continue;
                            }
                            else{
                                printf("in sema4\n");
                                sema_down(n);
                                flag[source] = false;
                                thread_set_priority(PRI_MAX);
                                flag[source] = true;
                                break;
                            }
                        }
                    }
                }
        }
        
        // 교차로에서 빠져 나올 때
        if( !is_at_intersection(source, dest, i) && n->value == 0 ){
            if(map_flag[path[source][dest][i-1].row][path[source][dest][i-1].col] == 0){
                printf("%c sema up!!\n", temp_name[0]);
                sema_up(n);
                printf("n : %d\n", n->value);
            }
        }

        i++;

        timer_sleep(100); //1초 동안 block        
    }
    i -= 1;
    
    // path의 끝에 도달한 vehicle 제거
    map_draw[path[source][dest][i].row][path[source][dest][i].col] = ' ';
    
    // 마지막 위치 flag 제거
    map_flag[path[source][dest][i].row][path[source][dest][i].col] = 0;

    printf("finished!\n");
    finish_check ++;
    thread_exit();
}


struct source_to_dest* insert_vehicle(char ** argv, int param_size){
    int i=0;
    int j=0;

    char *token, *save_ptr;

    for (token = strtok_r(argv[1], ":", &save_ptr); token != NULL; token = strtok_r(NULL, ":", &save_ptr)){
        vehicle_num++;
    }

    //vehicle_num = param_size / 3;
    struct source_to_dest * vehicle_list = (struct source_to_dest*) malloc(sizeof(struct source_to_dest)*vehicle_num);	

    for(i=0; i<param_size; i++){
        vehicle_list[i].vehicle[0] = argv[1][j];
        vehicle_list[i].vehicle[1] = '\0';
        vehicle_list[i].source = argv[1][j+1];
        vehicle_list[i].dest = argv[1][j+2];
     
        j += 4;
    }

    return vehicle_list;
}

void print_map(){
    int i, j;

    //clear();
    for (i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%c", map_draw[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

bool is_at_intersection(int source, int dest, int i){
    int temp_i;
    
    // I로 둘러 싸인 9칸 중에서
    // I를 제외한 5칸에 대해서만
    // 교차로로 인식한다.
    // I는 교차로를 빠져나가는 곳으로
    // 빠져 나가면, 이제 다른 차들이 진입할 수 있기 때문이다.
    
    // XXXXXXX
    // XXXXXXX
    //   I-I
    // -------
    //   I-I
    // XX - XX
    // XX - XX
    
    // (2,2) -> (2,1) or (3,2)
    if( path[source][dest][i].row == 2 && path[source][dest][i].col == 2 ){
        temp_i = i + 1;
        if( ( path[source][dest][temp_i].row == 2 && path[source][dest][temp_i].col == 1 ) ){
            return false;
        }
    }

    // (2,4) -> (2,5)
    if( path[source][dest][i].row == 2 && path[source][dest][i].col == 4 ){
        temp_i = i + 1;
        if( path[source][dest][temp_i].row == 2 && path[source][dest][temp_i].col == 5 ){
            return false;
        }
    }

    // (4,4) -> (4,5)
    if( path[source][dest][i].row == 4 && path[source][dest][i].col == 4 ){
        temp_i = i + 1;
        if( path[source][dest][temp_i].row == 4 && path[source][dest][temp_i].col == 5 ){
            return false;
        }
    }
    
    // (4,2) -> (5,2) or (4,1)
    if( path[source][dest][i].row == 4 && path[source][dest][i].col == 2 ){
        temp_i = i + 1;
        if( ( path[source][dest][temp_i].row == 5 && path[source][dest][temp_i].col == 2 ) || ( path[source][dest][temp_i].row == 4 && path[source][dest][temp_i].col == 1 ) ){
            return false;
        }
    }

    return true;
}

void init_waiting_list(){
    int i;

    for(i=0; i<param_size / 3; i++){
        if(vehicle_list[i].source == 'A'){
            a_list->list[a_list->index++] = vehicle_list[i].vehicle;
        }
        if(vehicle_list[i].source == 'B'){
            b_list->list[b_list->index++] = vehicle_list[i].vehicle;
        }
        if(vehicle_list[i].source == 'C'){
            c_list->list[c_list->index++] = vehicle_list[i].vehicle;
        }
    }
}

int max_list_size(){
    int max = 0;

    if(a_list->index <= b_list->index){
        if(b_list->index <= c_list->index)
            max = c_list->index;
        else
            max = b_list->index;
    }
    else{
        if(a_list->index <= c_list->index)
            max = c_list->index;
        else
            max = a_list->index;
    }

    return max;
}


void initialize(){
    int i;
    printf("!!!!\n");

    // allocate memory
    s = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    n = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    control_duplicate_vehicle = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    control_creating_thread = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    control_flag = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    control_flag2 = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
    control_flag3 = (struct semaphore*)malloc(sizeof(struct semaphore)*1);

    for(i=0; i<3; i++){
        control_behind_vehicle[i] = (struct semaphore*)malloc(sizeof(struct semaphore)*1);
        sema_init(control_behind_vehicle[i], 0);
    }
        

    printf("!!!!\n");
    
    // initialize semaphore
    sema_init(s, 1);
    sema_init(n, 0);
    sema_init(control_creating_thread, 1);
    sema_init(control_duplicate_vehicle, 1);
    sema_init(control_flag, 1);
    sema_init(control_flag2, 1);
    sema_init(control_flag3, 1);

    // initialize waiting list
    a_list = (struct source_list *)malloc(sizeof(struct source_list)*1);
    b_list = (struct source_list *)malloc(sizeof(struct source_list)*1);
    c_list = (struct source_list *)malloc(sizeof(struct source_list)*1);
    
    a_list->index = 0;
    b_list->index = 0;
    c_list->index = 0;
    
    for(i=0; i<10; i++){
        a_list->list[i] = NULL;
        b_list->list[i] = NULL;
        c_list->list[i] = NULL;
    }
    
    // map data copy
    strlcpy(map_draw, map_draw_default, sizeof map_draw_default + 1);
 
    init_waiting_list();
}

int get_index(char * name){
    int i;
    int index;

    for(i=0; i< vehicle_num; i++){
        if(!strcmp(vehicle_list[i].vehicle, name))
            index = i;
    }

    return index;
}
