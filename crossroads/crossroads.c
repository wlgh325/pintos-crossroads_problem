#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"

//path[1][0]-> B(1)에서 A(0)로 가는 경로의 row와 col value
//argv[0]이 실행할 때 첫 번째 파라미터
//argv[1]이 실행할 때 두 번째 파라미터 => aAB:bBC:cCA

static struct source_to_dest *vehicle_list;

void run_crossroads(char **argv)
{
    int param_size;
    char * temp_name = "";

	printf("implement crossroads !!!\n");

    param_size = strlen(argv[1]);
    
    // vehicle, source, destination 넣기
    vehicle_list = insert_vehicle(argv, param_size);
    
    thread_create("a", PRI_DEFAULT, vehicle_func, NULL);
    thread_create("b", PRI_DEFAULT, vehicle_func, NULL);
    thread_create("c", PRI_DEFAULT, vehicle_func, NULL);

    /*
    for (i=0; i<param_size / 3; i++){
        thread_create("vv", PRI_DEFAULT, vehicle_func, NULL);
    }
    */
}

void vehicle_func(){    
    int source, dest;
    int i;
    char * temp_name = thread_name();
    int t_name = temp_name[0] - 97;
    char map_draw[7][7];
    struct semaphore s;

    sema_init(&s, 1);
    
    // t_name[0] -> '/0' 제외한 vehicle name
    // a's ASCII code : 97
    // A's ASCII code : 65
    
    source = vehicle_list[t_name].source - 65;
    dest = vehicle_list[t_name].dest - 65;

    intr_enable();

    i=0;
    // print map
    while(! (path[source][dest][i].row == -1 && path[source][dest][i].col == -1)){
        strlcpy(map_draw, map_draw_default, sizeof map_draw_default + 1);
        sema_down(&s);
        
        map_draw[path[source][dest][i].row][path[source][dest][i].col] = temp_name[0];
        print_map(map_draw);

        sema_up(&s);
        timer_msleep(1000);
        i++;
    }

    thread_block();
    timer_msleep(1000);
    thread_unblock(thread_current());
    
    printf("finished!\n");
}

struct source_to_dest* insert_vehicle(char ** argv, int param_size){
    int i=0;
    int j=0;

    int vehicle_num = param_size / 3;
    struct source_to_dest * vehicle_list = (struct source_to_dest*) malloc(sizeof(struct source_to_dest)*vehicle_num);	

    for(i=0; i<param_size; i++){
        vehicle_list[i].vehicle = argv[1][j];
        vehicle_list[i].source = argv[1][j+1];
        vehicle_list[i].dest = argv[1][j+2];
        j += 4;
    }

    return vehicle_list;
}

void print_map(char map_draw[7][7]){
    int i, j;
    for (i=0; i<7; i++){
        for(j=0; j<7; j++){
            printf("%c", map_draw[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
