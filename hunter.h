#ifndef HUNTER_H
#define HUNTER_H

#include "defs.h"

// thread function
void *hunter_thread(void *arg);

// function to add hunters
void insert_hunters(struct House* house);

// function that simulates 1 hunter turn
void hunter_turn(struct Hunter* hunter);

// function that removes a hunter from their current room 
void remove_from_room(struct Hunter* hunter);


// this function is used to move a hunter from room a to room b
void hunter_relocate(struct Hunter* hunter, struct Room* room, bool add_to_stack);

// find a random adjacent room
void find_adjacent_room(struct Hunter* hunter, struct Room** room);

// function to clean hunter memory
void clean_memory(struct House* house);

// this function cleans the hunter breadstack
void clean_stack(struct Hunter* hunter);

#endif // HUNTER_H