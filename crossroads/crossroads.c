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

#define clear() printf("\033c")

//path[1][0]-> B(1)에서 A(0)로 가는 경로의 row와 col value
//argv[0]이 실행할 때 첫 번째 파라미터
//argv[1]이 실행할 때 두 번째 파라미터 => aAB:bBC:cCA


static struct source_to_dest *vehicle_list;
char map_draw[7][7];
struct semaphore n, s;

int vehicle_index = 0;
int vehicle_num;
int finish_check = 0;


void run_crossroads(char **argv)
{
    int param_size;
    int i;
    
    sema_init(&s, 1);
    sema_init(&n, 0);

    // map data copy
    strlcpy(map_draw, map_draw_default, sizeof map_draw_default + 1);

	printf("implement crossroads !!!\n");

    param_size = strlen(argv[1]);
    
    // vehicle, source, destination 넣기
    vehicle_list = insert_vehicle(argv, param_size);

    // create vehicle thread
    for (i=0; i<param_size / 3; i++){
        thread_create(vehicle_list[i].vehicle, PRI_DEFAULT, vehicle_func, NULL);
    }

    // create main thread
    thread_create("main", PRI_DEFAULT, main_func, NULL);
    
}

void main_func(){
    while(true){
        print_map();

        timer_sleep(100); //1초 동안 block
        if(vehicle_num == finish_check){
            // initialize map
            strlcpy(map_draw, map_draw_default, sizeof map_draw_default + 1);    
            print_map();
            break;
        }
            
    }
    
}

void vehicle_func(){    
    int source, dest;
    int original_source, original_dest;
    int i, j, temp_i;
    char * temp_name = thread_name();

    // t_name[0] -> '/0' 제외한 vehicle name
    // a's ASCII code : 97
    // A's ASCII code : 65
    
    original_source = vehicle_list[vehicle_index].source;
    original_dest = vehicle_list[vehicle_index++].dest;
    
    source = original_source - 65;
    dest = original_dest - 65;


    i=1;

    // draw map
    map_draw[path[source][dest][0].row][path[source][dest][0].col] = temp_name[0];
    timer_sleep(100); //1초 동안 block
    while(! (path[source][dest][i].row == -1 && path[source][dest][i].col == -1)){
        temp_i = i;

        // draw vehicle
        map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = temp_name[0];
        temp_i = i - 1;
        map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = ' ';
        
        // remove vehicle
        if(path[source][dest][temp_i].row == 3){
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = '-';
        }
        else if( (path[source][dest][temp_i].row == 2 || path[source][dest][temp_i].row == 4) && path[source][dest][temp_i].col == 3){
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = '-';
        }
        else{
            map_draw[path[source][dest][temp_i].row][path[source][dest][temp_i].col] = ' ';
        }

        printf("%c : %d, %d\n", temp_name[0], path[source][dest][i].row, path[source][dest][i].col) ;
        // 교차로 제어
        if ( (path[source][dest][i].row == 2 && path[source][dest][i].col == 5 ) || 
            path[source][dest][i].row == 4 && path[source][dest][i].col == 1 || 
            path[source][dest][i].row == 5 && path[source][dest][i].col == 4 ){
                // A->B일 때, B->C, C->A만 이동 가능
                if( original_source == 'A' && original_dest == 'B' ){
                    for(j=0; j<vehicle_num; j++){
                        if( !strcmp(vehicle_list[j].vehicle, temp_name))
                            continue;
                        else{
                            if(vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B' || vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'C' || 
                            vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A' || vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'A'){
                                continue;
                            }
                            else{
                                printf("in sema1\n");
                                sema_down(&s);
                                sema_down(&n);
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
                            if(vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'C' || vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A'){
                                continue;
                            }
                            else{
                                printf("in sema2\n");
                                sema_down(&s);
                                sema_down(&n);
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
                            if(vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'A' || vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B'){
                                continue;
                            }
                            else{
                                printf("in sema3\n");
                                sema_down(&s);
                                sema_down(&n);
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
                            if(vehicle_list[j].source == 'B' && vehicle_list[j].dest == 'C' || vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'A' ||
                            vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'B' || vehicle_list[j].source == 'A' && vehicle_list[j].dest == 'C' ||
                            vehicle_list[j].source == 'C' && vehicle_list[j].dest == 'B'){
                                continue;
                            }
                            else{
                                printf("in sema4\n");
                                sema_down(&s);
                                sema_down(&n);
                            }
                        }
                    }
                }
        }
        else if( !sema_try_down(&n) && !sema_try_down(&s) && !is_at_intersection(source, dest, i)){
            printf("sema up!!\n");
            sema_up(&n);
            sema_up(&s);
        }

        i++;
        timer_sleep(100); //1초 동안 block
    }
    i -= 1;
    
    // path의 끝에 도달한 vehicle 제거
    map_draw[path[source][dest][i].row][path[source][dest][i].col] = ' ';
    printf("finished!\n");
    finish_check ++;
    thread_exit();
}

struct source_to_dest* insert_vehicle(char ** argv, int param_size){
    int i=0;
    int j=0;

    vehicle_num = param_size / 3;
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
    if(path[source][dest][i].row >= 2 && path[source][dest][i].row <= 4 ){
        if(path[source][dest][i].col >= 2 && path[source][dest][i].col <= 4 ){
            return true;
        }
    }
    
    return false;
}
