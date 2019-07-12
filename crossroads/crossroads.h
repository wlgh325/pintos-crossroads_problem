#ifndef __PROJECTS_PROJECT1_CROASSROADS_H__
#define __PROJECTS_PROJECT1_CROASSROADS_H__

struct source_to_dest {
    char vehicle;
    char source;
    char dest;

};

void run_crossroads(char **argv);
struct source_to_dest* insert_vehicle(char ** argv, int param_size);
void vehicle_func();
void print_map(char map_draw[7][7]);
#endif /* __PROJECTS_PROJECT1_CROASSROADS_H__ */
