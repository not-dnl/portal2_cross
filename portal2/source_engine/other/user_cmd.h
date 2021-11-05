#ifndef PORTAL2_CROSS_USER_CMD_H
#define PORTAL2_CROSS_USER_CMD_H


#define IN_JUMP            (1 << 1)

#define FL_ON_GROUND       (1 << 0)

class c_user_cmd
{
public:
    void* this_ptr;
    int command_number;
    int tick_count;
    float viewangles_x;
    float viewangles_y;
    float viewangles_z;
    float forward_move;
    float side_move;
    float up_move;
    int buttons;
};


#endif //PORTAL2_CROSS_USER_CMD_H
