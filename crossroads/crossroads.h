#ifndef __PROJECTS_PROJECT1_CROASSROADS_H__
#define __PROJECTS_PROJECT1_CROASSROADS_H__

struct source_to_dest {
    char vehicle[2];
    char source;
    char dest;

};

void initialize();
void run_crossroads(char **argv);
struct source_to_dest* insert_vehicle(char ** argv, int param_size);
void vehicle_func();
void main_func();
void print_map();
bool is_at_intersection(int source, int dest, int i);
void init_waiting_list();
int max_list_size();
#endif /* __PROJECTS_PROJECT1_CROASSROADS_H__ */
