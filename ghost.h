#ifndef GHOST_H
#define GHOST_H

#include "defs.h"

// thread function
void *ghost_thread(void *arg);

// function to initalization the ghost struct
void ghost_init(struct House* house);

// function to take 1 ghost turn
void ghost_turn(struct Ghost* ghost);

// function to haunt
void ghost_haunt(struct Ghost* ghost,  enum EvidenceType evidence);

// function to move
void ghost_move(struct Ghost* ghost);

// function that choose either to haunt or idle
void idle_or_haunt(struct Ghost* ghost);

#endif // GHOST_H