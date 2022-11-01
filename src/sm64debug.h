#ifndef SM64DEBUG_H
#define SM64DEBUG_H

#include <stdio.h>
#include <stdint.h>

struct SM64StatesDictionary
{
    char name[64];
    uint32_t index;
};

#define SM64_DICTIONARY_SIZE 236

struct SM64StatesDictionary SM64_States_Dictionary[SM64_DICTIONARY_SIZE] =
    {{"ACT_IDLE",0x0C400201 },
    {"ACT_START_SLEEPING",0x0C400202 },
    {"ACT_SLEEPING",0x0C000203 },
    {"ACT_WAKING_UP",0x0C000204 },
    {"ACT_PANTING",0x0C400205 },
    {"ACT_HOLD_PANTING_UNUSED",0x08000206 },
    {"ACT_HOLD_IDLE",0x08000207 },
    {"ACT_HOLD_HEAVY_IDLE",0x08000208 },
    {"ACT_STANDING_AGAINST_WALL",0x0C400209 },
    {"ACT_COUGHING",0x0C40020A },
    {"ACT_SHIVERING",0x0C40020B },
    {"ACT_IN_QUICKSAND",0x0002020D },
    {"ACT_UNKNOWN_0002020E",0x0002020E },
    {"ACT_CROUCHING",0x0C008220 },
    {"ACT_START_CROUCHING",0x0C008221 },
    {"ACT_STOP_CROUCHING",0x0C008222 },
    {"ACT_START_CRAWLING",0x0C008223 },
    {"ACT_STOP_CRAWLING",0x0C008224 },
    {"ACT_SLIDE_KICK_SLIDE_STOP",0x08000225 },
    {"ACT_SHOCKWAVE_BOUNCE",0x00020226 },
    {"ACT_FIRST_PERSON",0x0C000227 },
    {"ACT_BACKFLIP_LAND_STOP",0x0800022F },
    {"ACT_JUMP_LAND_STOP",0x0C000230 },
    {"ACT_DOUBLE_JUMP_LAND_STOP",0x0C000231 },
    {"ACT_FREEFALL_LAND_STOP",0x0C000232 },
    {"ACT_SIDE_FLIP_LAND_STOP",0x0C000233 },
    {"ACT_HOLD_JUMP_LAND_STOP",0x08000234 },
    {"ACT_HOLD_FREEFALL_LAND_STOP",0x08000235 },
    {"ACT_AIR_THROW_LAND",0x80000A36 },
    {"ACT_TWIRL_LAND",0x18800238 },
    {"ACT_LAVA_BOOST_LAND",0x08000239 },
    {"ACT_TRIPLE_JUMP_LAND_STOP",0x0800023A },
    {"ACT_LONG_JUMP_LAND_STOP",0x0800023B },
    {"ACT_GROUND_POUND_LAND",0x0080023C },
    {"ACT_BRAKING_STOP",0x0C00023D },
    {"ACT_BUTT_SLIDE_STOP",0x0C00023E },
    {"ACT_HOLD_BUTT_SLIDE_STOP",0x0800043F },
    {"ACT_WALKING",0x04000440 },
    {"ACT_HOLD_WALKING",0x00000442 },
    {"ACT_TURNING_AROUND",0x00000443 },
    {"ACT_FINISH_TURNING_AROUND",0x00000444 },
    {"ACT_BRAKING",0x04000445 },
    {"ACT_RIDING_SHELL_GROUND",0x20810446 },
    {"ACT_HOLD_HEAVY_WALKING",0x00000447 },
    {"ACT_CRAWLING",0x04008448 },
    {"ACT_BURNING_GROUND",0x00020449 },
    {"ACT_DECELERATING",0x0400044A },
    {"ACT_HOLD_DECELERATING",0x0000044B },
    {"ACT_BEGIN_SLIDING",0x00000050 },
    {"ACT_HOLD_BEGIN_SLIDING",0x00000051 },
    {"ACT_BUTT_SLIDE",0x00840452 },
    {"ACT_STOMACH_SLIDE",0x008C0453 },
    {"ACT_HOLD_BUTT_SLIDE",0x00840454 },
    {"ACT_HOLD_STOMACH_SLIDE",0x008C0455 },
    {"ACT_DIVE_SLIDE",0x00880456 },
    {"ACT_MOVE_PUNCHING",0x00800457 },
    {"ACT_CROUCH_SLIDE",0x04808459 },
    {"ACT_SLIDE_KICK_SLIDE",0x0080045A },
    {"ACT_HARD_BACKWARD_GROUND_KB",0x00020460 },
    {"ACT_HARD_FORWARD_GROUND_KB",0x00020461 },
    {"ACT_BACKWARD_GROUND_KB",0x00020462 },
    {"ACT_FORWARD_GROUND_KB",0x00020463 },
    {"ACT_SOFT_BACKWARD_GROUND_KB",0x00020464 },
    {"ACT_SOFT_FORWARD_GROUND_KB",0x00020465 },
    {"ACT_GROUND_BONK",0x00020466 },
    {"ACT_DEATH_EXIT_LAND",0x00020467 },
    {"ACT_JUMP_LAND",0x04000470 },
    {"ACT_FREEFALL_LAND",0x04000471 },
    {"ACT_DOUBLE_JUMP_LAND",0x04000472 },
    {"ACT_SIDE_FLIP_LAND",0x04000473 },
    {"ACT_HOLD_JUMP_LAND",0x00000474 },
    {"ACT_HOLD_FREEFALL_LAND",0x00000475 },
    {"ACT_QUICKSAND_JUMP_LAND",0x00000476 },
    {"ACT_HOLD_QUICKSAND_JUMP_LAND",0x00000477 },
    {"ACT_TRIPLE_JUMP_LAND",0x04000478 },
    {"ACT_LONG_JUMP_LAND",0x00000479 },
    {"ACT_BACKFLIP_LAND",0x0400047A },
    {"ACT_JUMP",0x03000880 },
    {"ACT_DOUBLE_JUMP",0x03000881 },
    {"ACT_TRIPLE_JUMP",0x01000882 },
    {"ACT_BACKFLIP",0x01000883 },
    {"ACT_STEEP_JUMP",0x03000885 },
    {"ACT_WALL_KICK_AIR",0x03000886 },
    {"ACT_SIDE_FLIP",0x01000887 },
    {"ACT_LONG_JUMP",0x03000888 },
    {"ACT_WATER_JUMP",0x01000889 },
    {"ACT_DIVE",0x0188088A },
    {"ACT_FREEFALL",0x0100088C },
    {"ACT_TOP_OF_POLE_JUMP",0x0300088D },
    {"ACT_BUTT_SLIDE_AIR",0x0300088E },
    {"ACT_FLYING_TRIPLE_JUMP",0x03000894 },
    {"ACT_SHOT_FROM_CANNON",0x00880898 },
    {"ACT_FLYING",0x10880899 },
    {"ACT_RIDING_SHELL_JUMP",0x0281089A },
    {"ACT_RIDING_SHELL_FALL",0x0081089B },
    {"ACT_VERTICAL_WIND",0x1008089C },
    {"ACT_HOLD_JUMP",0x030008A0 },
    {"ACT_HOLD_FREEFALL",0x010008A1 },
    {"ACT_HOLD_BUTT_SLIDE_AIR",0x010008A2 },
    {"ACT_HOLD_WATER_JUMP",0x010008A3 },
    {"ACT_TWIRLING",0x108008A4 },
    {"ACT_FORWARD_ROLLOUT",0x010008A6 },
    {"ACT_AIR_HIT_WALL",0x000008A7 },
    {"ACT_RIDING_HOOT",0x000004A8 },
    {"ACT_GROUND_POUND",0x008008A9 },
    {"ACT_SLIDE_KICK",0x018008AA },
    {"ACT_AIR_THROW",0x830008AB },
    {"ACT_JUMP_KICK",0x018008AC },
    {"ACT_BACKWARD_ROLLOUT",0x010008AD },
    {"ACT_CRAZY_BOX_BOUNCE",0x000008AE },
    {"ACT_SPECIAL_TRIPLE_JUMP",0x030008AF },
    {"ACT_BACKWARD_AIR_KB",0x010208B0 },
    {"ACT_FORWARD_AIR_KB",0x010208B1 },
    {"ACT_HARD_FORWARD_AIR_KB",0x010208B2 },
    {"ACT_HARD_BACKWARD_AIR_KB",0x010208B3 },
    {"ACT_BURNING_JUMP",0x010208B4 },
    {"ACT_BURNING_FALL",0x010208B5 },
    {"ACT_SOFT_BONK",0x010208B6 },
    {"ACT_LAVA_BOOST",0x010208B7 },
    {"ACT_GETTING_BLOWN",0x010208B8 },
    {"ACT_THROWN_FORWARD",0x010208BD },
    {"ACT_THROWN_BACKWARD",0x010208BE },
    {"ACT_WATER_IDLE",0x380022C0 },
    {"ACT_HOLD_WATER_IDLE",0x380022C1 },
    {"ACT_WATER_ACTION_END",0x300022C2 },
    {"ACT_HOLD_WATER_ACTION_END",0x300022C3 },
    {"ACT_DROWNING",0x300032C4 },
    {"ACT_BACKWARD_WATER_KB",0x300222C5 },
    {"ACT_FORWARD_WATER_KB",0x300222C6 },
    {"ACT_WATER_DEATH",0x300032C7 },
    {"ACT_WATER_SHOCKED",0x300222C8 },
    {"ACT_BREASTSTROKE",0x300024D0 },
    {"ACT_SWIMMING_END",0x300024D1 },
    {"ACT_FLUTTER_KICK",0x300024D2 },
    {"ACT_HOLD_BREASTSTROKE",0x300024D3 },
    {"ACT_HOLD_SWIMMING_END",0x300024D4 },
    {"ACT_HOLD_FLUTTER_KICK",0x300024D5 },
    {"ACT_WATER_SHELL_SWIMMING",0x300024D6 },
    {"ACT_WATER_THROW",0x300024E0 },
    {"ACT_WATER_PUNCH",0x300024E1 },
    {"ACT_WATER_PLUNGE",0x300022E2 },
    {"ACT_CAUGHT_IN_WHIRLPOOL",0x300222E3 },
    {"ACT_METAL_WATER_STANDING",0x080042F0 },
    {"ACT_HOLD_METAL_WATER_STANDING",0x080042F1 },
    {"ACT_METAL_WATER_WALKING",0x000044F2 },
    {"ACT_HOLD_METAL_WATER_WALKING",0x000044F3 },
    {"ACT_METAL_WATER_FALLING",0x000042F4 },
    {"ACT_HOLD_METAL_WATER_FALLING",0x000042F5 },
    {"ACT_METAL_WATER_FALL_LAND",0x000042F6 },
    {"ACT_HOLD_METAL_WATER_FALL_LAND",0x000042F7 },
    {"ACT_METAL_WATER_JUMP",0x000044F8 },
    {"ACT_HOLD_METAL_WATER_JUMP",0x000044F9 },
    {"ACT_METAL_WATER_JUMP_LAND",0x000044FA },
    {"ACT_HOLD_METAL_WATER_JUMP_LAND",0x000044FB },
    {"ACT_DISAPPEARED",0x00001300 },
    {"ACT_INTRO_CUTSCENE",0x04001301 },
    {"ACT_STAR_DANCE_EXIT",0x00001302 },
    {"ACT_STAR_DANCE_WATER",0x00001303 },
    {"ACT_FALL_AFTER_STAR_GRAB",0x00001904 },
    {"ACT_READING_AUTOMATIC_DIALOG",0x20001305 },
    {"ACT_READING_NPC_DIALOG",0x20001306 },
    {"ACT_STAR_DANCE_NO_EXIT",0x00001307 },
    {"ACT_READING_SIGN",0x00001308 },
    {"ACT_JUMBO_STAR_CUTSCENE",0x00001909 },
    {"ACT_WAITING_FOR_DIALOG",0x0000130A },
    {"ACT_DEBUG_FREE_MOVE",0x0000130F },
    {"ACT_STANDING_DEATH",0x00021311 },
    {"ACT_QUICKSAND_DEATH",0x00021312 },
    {"ACT_ELECTROCUTION",0x00021313 },
    {"ACT_SUFFOCATION",0x00021314 },
    {"ACT_DEATH_ON_STOMACH",0x00021315 },
    {"ACT_DEATH_ON_BACK",0x00021316 },
    {"ACT_EATEN_BY_BUBBA",0x00021317 },
    {"ACT_END_PEACH_CUTSCENE",0x00001918 },
    {"ACT_CREDITS_CUTSCENE",0x00001319 },
    {"ACT_END_WAVING_CUTSCENE",0x0000131A },
    {"ACT_PULLING_DOOR",0x00001320 },
    {"ACT_PUSHING_DOOR",0x00001321 },
    {"ACT_WARP_DOOR_SPAWN",0x00001322 },
    {"ACT_EMERGE_FROM_PIPE",0x00001923 },
    {"ACT_SPAWN_SPIN_AIRBORNE",0x00001924 },
    {"ACT_SPAWN_SPIN_LANDING",0x00001325 },
    {"ACT_EXIT_AIRBORNE",0x00001926 },
    {"ACT_EXIT_LAND_SAVE_DIALOG",0x00001327 },
    {"ACT_DEATH_EXIT",0x00001928 },
    {"ACT_UNUSED_DEATH_EXIT",0x00001929 },
    {"ACT_FALLING_DEATH_EXIT",0x0000192A },
    {"ACT_SPECIAL_EXIT_AIRBORNE",0x0000192B },
    {"ACT_SPECIAL_DEATH_EXIT",0x0000192C },
    {"ACT_FALLING_EXIT_AIRBORNE",0x0000192D },
    {"ACT_UNLOCKING_KEY_DOOR",0x0000132E },
    {"ACT_UNLOCKING_STAR_DOOR",0x0000132F },
    {"ACT_ENTERING_STAR_DOOR",0x00001331 },
    {"ACT_SPAWN_NO_SPIN_AIRBORNE",0x00001932 },
    {"ACT_SPAWN_NO_SPIN_LANDING",0x00001333 },
    {"ACT_BBH_ENTER_JUMP",0x00001934 },
    {"ACT_BBH_ENTER_SPIN",0x00001535 },
    {"ACT_TELEPORT_FADE_OUT",0x00001336 },
    {"ACT_TELEPORT_FADE_IN",0x00001337 },
    {"ACT_SHOCKED",0x00020338 },
    {"ACT_SQUISHED",0x00020339 },
    {"ACT_HEAD_STUCK_IN_GROUND",0x0002033A },
    {"ACT_BUTT_STUCK_IN_GROUND",0x0002033B },
    {"ACT_FEET_STUCK_IN_GROUND",0x0002033C },
    {"ACT_PUTTING_ON_CAP",0x0000133D },
    {"ACT_HOLDING_POLE",0x08100340 },
    {"ACT_GRAB_POLE_SLOW",0x00100341 },
    {"ACT_GRAB_POLE_FAST",0x00100342 },
    {"ACT_CLIMBING_POLE",0x00100343 },
    {"ACT_TOP_OF_POLE_TRANSITION",0x00100344 },
    {"ACT_TOP_OF_POLE",0x00100345 },
    {"ACT_START_HANGING",0x08200348 },
    {"ACT_HANGING",0x00200349 },
    {"ACT_HANG_MOVING",0x0020054A },
    {"ACT_LEDGE_GRAB",0x0800034B },
    {"ACT_LEDGE_CLIMB_SLOW_1",0x0000054C },
    {"ACT_LEDGE_CLIMB_SLOW_2",0x0000054D },
    {"ACT_LEDGE_CLIMB_DOWN",0x0000054E },
    {"ACT_LEDGE_CLIMB_FAST",0x0000054F },
    {"ACT_GRABBED",0x00020370 },
    {"ACT_IN_CANNON",0x00001371 },
    {"ACT_TORNADO_TWIRLING",0x10020372 },
    {"ACT_PUNCHING",0x00800380 },
    {"ACT_PICKING_UP",0x00000383 },
    {"ACT_DIVE_PICKING_UP",0x00000385 },
    {"ACT_STOMACH_SLIDE_STOP",0x00000386 },
    {"ACT_PLACING_DOWN",0x00000387 },
    {"ACT_THROWING",0x80000588 },
    {"ACT_HEAVY_THROW",0x80000589 },
    {"ACT_PICKING_UP_BOWSER",0x00000390 },
    {"ACT_HOLDING_BOWSER",0x00000391 },
    {"ACT_RELEASING_BOWSER",0x00000392 },
    {"ACT_LADDER_START_GRAB",0x000003C0 },
    {"ACT_LADDER_IDLE",0x000003C1 },
    {"ACT_LADDER_MOVING_VERTICAL",0x000005c2 },
    {"ACT_LADDER_MOVING_HORIZONTAL",0x000005c3}};

struct SM64StatesDictionary *get_mario_state_name(uint32_t index)
{
    for(int i=0; i<SM64_DICTIONARY_SIZE; i++)
    {
        if(SM64_States_Dictionary[i].index==index)
        {
            return &(SM64_States_Dictionary[i]);
        }
    }
    return NULL;
}

#endif