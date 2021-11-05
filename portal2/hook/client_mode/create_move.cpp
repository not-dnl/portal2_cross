#include "../manager/hooking_manager.h"
#include "../../helper/interfaces.h"

bool s_hook_collection::create_move_hooked(HOOK_ARGS, float fl_input_sample_time, c_user_cmd* cmd)
{
    // this function is responsible for movement stuff and is called every client-tick
    // https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/client/clientmode_shared.cpp#L396

    auto local_player = interfaces::get().entity_list->get_client_entity(
            interfaces::get().engine_client->get_local_player());

    if (local_player)
    {
        // those flags are networked to the client, it's called "m_fFlags" in the SourceEngine
        auto local_flags = *(int*)(reinterpret_cast<uintptr_t>(local_player) + L_W(0xE4, 0xF8));
        auto on_ground = (local_flags & FL_ON_GROUND) != 0; // they hold the ON_GROUND information of our player

        static auto auto_bhop = interfaces::get().con_var->find_var("auto_bhop"); // find our var
        // if it exists, was set to >= 1, we're holding the jump key, and we're not on ground
        if (auto_bhop && auto_bhop->i_value >= 1 && cmd->buttons & IN_JUMP && !on_ground)
            cmd->buttons &= ~IN_JUMP; // remove the IN_JUMP flag, since we hold jump we'll set it as soon as we reach the ground.
    }

    // finally, call the original function, so the game can run as usual
    return hooking_manager::get_hook_collection().o_create_move_hooked(C_HOOK_ARGS, fl_input_sample_time, cmd);
}