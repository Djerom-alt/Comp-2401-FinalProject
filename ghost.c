#include "ghost.h"
#include "helpers.h"
// #include "stdio.h"
#include "string.h"

// thread function
void *ghost_thread(void *arg){
    struct Ghost* ghost = (struct Ghost*) arg;
    // go until ghost has exited
    while(!ghost->has_exited){
        // run in the loop
        ghost_turn(ghost);
    }
}

// function to initalization the ghost struct
void ghost_init(struct House* house) {
    // randomly pick a room
    // set ghost id
    // set ghost boredem
    // set ghost has_exited
    // set ghost ghost_type
    house->ghost.id=DEFAULT_GHOST_ID;

    house->ghost.boredom=0;

    house->ghost.has_exited=false;

    // generate a rand int
    // first a room
    int room_rand=rand_int_threadsafe(1, house->room_count);

    // get the list of ghosts
    const enum GhostType* ghost_types;

    // second a ghost type, ghost shouldnt be spawning in van/exitroom/starting room
    int ghost_ran=rand_int_threadsafe(1, get_all_ghost_types(&(ghost_types)));

    // pointing to a room in house
    house->ghost.current_room = &(house->rooms[room_rand]);

    // room should point to a ghost
    house->ghost.current_room->ghost=&(house->ghost);

    // picking ghost type
    house->ghost.ghost_type=ghost_types[ghost_ran];

    // logging ghost init
    log_ghost_init(house->ghost.id, house->ghost.current_room->name, house->ghost.ghost_type);
}

// function to take 1 ghost turn
void ghost_turn(struct Ghost* ghost) {

    // check if current room has hunter, can't move
    if(ghost->current_room->hunter_count > 0) {
        // ghost cannot move
        // resets boredom to 0
        ghost->boredom=0;
        // ghost can haunt
        // ghost can do nothing

        // the program will either choose idle or haunt randomly
        idle_or_haunt(ghost);

    } else {

        // increases boredom
        ghost->boredom++;

        // boredom checkerc
        if(ghost->boredom >= ENTITY_BOREDOM_MAX){
            ghost->has_exited=true;
            // log the exit
            log_ghost_exit(ghost->id, ghost->boredom, ghost->current_room->name);
            // need to remove the ghost from the room
            ghost->current_room->ghost=NULL;
            // ghost current room is cleared why not
            ghost->current_room=NULL;
            return;
        }

        // ghost can move
        // ghost can do nothing
        // ghost can haunt
        // first we choose randomly either to (idle or haunt) or move
        int r = rand_int_threadsafe(0, 10);
        // switch(r){
        //     case 0:
        //         // the program will either choose idle or haunt randomly
        //         idle_or_haunt(ghost);
        //         break;
        //     case 1:
        //         // this makes the ghost move to a adjacent room
        //         ghost_move(ghost);
        //         return;
        // }

        // much better for tweaking
        if(r <= 18){
            idle_or_haunt(ghost);
        }else{
            ghost_move(ghost);
            return;
        }
    }
}


// function that choose either to haunt or idle
void idle_or_haunt(struct Ghost* ghost) {

    // get array of evidence options
    const enum EvidenceType* types;
    int upper_exclusive=get_all_evidence_types(&types);

    // generate random index
    int index=rand_int_threadsafe(0, upper_exclusive);

    // if we got a compatible evidence
    // two options, 50/50 chances for either idle or haunt
    if( ( ( ghost->ghost_type ) & ( types[index] ) )  > 0) {
        // this makes the ghost haunt
        ghost_haunt(ghost, types[index]);
    }else{
        // this makes the ghost do nothing
        log_ghost_idle(ghost->id, ghost->boredom, ghost->current_room->name);
        return;
    }
}

// function to haunt
void ghost_haunt(struct Ghost* ghost, enum EvidenceType evidence) {
    // drop evidence in current room
    // we know evidence bit can be upto 6 
    ghost->current_room->evidence |= evidence;
    // logs the result
    log_ghost_evidence(ghost->id, ghost->boredom, ghost->current_room->name, evidence);
}

// function to move
void ghost_move(struct Ghost* ghost) {

    // old room
    struct Room* from_room=ghost->current_room;

    // set current room ghost pointer to NULL
    from_room->ghost=NULL;
    
    // random int
    int r=rand_int_threadsafe(0, from_room->connection_count);

    // randomly pick an adjacent room
    struct Room* to_room=from_room->adjacent_rooms[r];

    // new room pointer to point to the ghost
    to_room->ghost=ghost;
    
    // set the ghost to point to new room
    ghost->current_room=to_room;

    // logs the results
    log_ghost_move(ghost->id, ghost->boredom, from_room->name, to_room->name);
}