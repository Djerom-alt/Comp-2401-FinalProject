#include "hunter.h"
#include "helpers.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

// function to add hunters
void insert_hunters(struct House* house) {

    // hunter information
    char hunter_name[MAX_HUNTER_NAME];
    int hunter_id;

    // we will use a while loop, exit when 'done' is entered
    while(true){
        
        printf("Enter hunter name (max %d characters) or 'done' to finish: ", MAX_HUNTER_NAME);
       
        // accepts hunter name from user
        fgets(hunter_name, MAX_HUNTER_NAME, stdin);

        // letting the program know where the string ends
        hunter_name[strnlen(hunter_name, MAX_HUNTER_NAME)-1]='\0';

        // check whether to exit 
        if(strcmp(hunter_name, "done")==0){
            break;
        }

        // accepts hunter id from user
        printf("Enter hunter ID (integer): ");
        scanf("%d", &hunter_id);
        getchar();
        printf("\n");

        // too long to type multiple times
        struct Hunter** array = &(house->hunter_collection.array);

        // we need to then realloc memory then setup the hunter struct
        // we would need to realloc each time a data is inserted
        if(*array == NULL){
            // we need to malloc 
            *array=(struct Hunter*) malloc(sizeof(struct Hunter));
            // check if heap allocation was succesfull
            if(*array == NULL){
                printf("Heap alloc failed\n");
                exit(1);
            }
        }else{
            // we need to realloc memory, size + 1
            struct Hunter* temp = (struct Hunter*) realloc(*array, sizeof(struct Hunter)*(house->hunter_collection.size+1));
            if(temp == NULL){
                printf("Heap alloc failed\n");
                free(*array);
                exit(1);
            }
            *array=temp;
        }    

        // lets intialize Hunter values
        // copy name
        strcpy((*array)[house->hunter_collection.size].name, hunter_name);
        
        // setup id
        (*array)[house->hunter_collection.size].id=hunter_id;
        // case file point to the one in House
        (*array)[house->hunter_collection.size].case_file=&(house->case_file);
        // point hunter to the starting location, first room in house
        (*array)[house->hunter_collection.size].current_room=house->starting_room;
        
        // set boredom to 0
        (*array)[house->hunter_collection.size].boredom=0;
        // set fear to 0
        (*array)[house->hunter_collection.size].fear=0;
        // set return to exit false
        (*array)[house->hunter_collection.size].return_to_exit=false;
        (*array)[house->hunter_collection.size].has_exited=false;

        // set starting room pointer
        (*array)[house->hunter_collection.size].starting_room=house->starting_room;


        // RANDOM DEVICE SELECTION


        // old way
        // // randomly pick a device
        // const enum EvidenceType* list;
        // int index=rand_int_threadsafe(0, get_all_evidence_types(&list));
        // (*array)[house->hunter_collection.size].device=list[index];
        // old way

        // hunter->device collection needs to be malloc'ed
        (*array)[house->hunter_collection.size].device_collection.size=0;
        get_all_evidence_types(&((*array)[house->hunter_collection.size].device_collection.devices));

        // then select the size=0 device to start with
        (*array)[house->hunter_collection.size].device=(*array)[house->hunter_collection.size].device_collection.devices[0];
        // increase size
        (*array)[house->hunter_collection.size].device_collection.size++;



        // RANDOM DEVICE SELECTION

        
        // only setup breadcrumb stack if pathfinding isn't active
        if(!PATH_FINDING){
            // setup the roomstack
            // RoomNode head would be the starting location
            struct RoomNode* node=(struct RoomNode*)malloc(sizeof(struct RoomNode));

            node->room=house->starting_room;
            node->prev=NULL;

            // attach to the head
            (*array)[house->hunter_collection.size].path.head=node;
    
        }else{
            // init with NULL
            (*array)[house->hunter_collection.size].path.head=NULL;
        }


        //log this insertion
        log_hunter_init(
            (*array)[house->hunter_collection.size].id,
            house->starting_room->name, 
            hunter_name, 
            (*array)[house->hunter_collection.size].device
        );

        // increase size
        house->hunter_collection.size++;
    }
}

// thread function
void *hunter_thread(void *arg){
    struct Hunter* hunter = (struct Hunter*) arg;
    // run the hunter_turn
    while(!hunter->has_exited){
        hunter_turn(hunter);
    }
}

// function to clean hunter memory
void clean_memory(struct House* house){
    // loop through hunter collection
    for(int i =0; i < house->hunter_collection.size;i++){
        // free the stack first
        clean_stack(house->hunter_collection.array+i);
    }
    // clean this itself but 
    free(house->hunter_collection.array);
    house->hunter_collection.array=NULL;
    house->hunter_collection.size=0;
    return;
}

// this function cleans the hunter breadstack
void clean_stack(struct Hunter* hunter){
    // we need to clean memory 
    struct RoomNode* curr=hunter->path.head;
    // loop until we clean all of the memory in the stack
    while (curr != NULL){
        struct RoomNode* temp=curr->prev;
        free(curr);
        curr=temp;
    }
    // clear breadstack 
    hunter->path.head=NULL;
    return;
}

// bfs_search function to find a path for the hunter
void bfs_search(struct Hunter* hunter, struct RoomStack* stack) {

    // target room
    struct Room* start=hunter->current_room;
    // need a way to start a starting room
    struct Room* target=hunter->starting_room;

    // linked list of RoomStacks ?
    struct PathList path;

    // allocate twice memory per RoomItem and RoomNode
    path.head=(struct RoomItem *)malloc(sizeof(struct RoomItem));
    // sets its states
    path.head->node=(struct RoomNode *)malloc(sizeof(struct RoomNode));
    path.head->next=NULL;
    // setup the states of RoomNode
    path.head->node->room=start;
    path.head->node->prev=NULL;

    // setup the tail, 
    // for now point to head
    path.tail=path.head;

    // this one is used to track all RoomItem removed from head
    struct RoomItem* ref=NULL;

    // it is gurantted we will find the target, so we can run until we find it
    while(true) {

        // check if we have reached the target
        if(path.head->node->room==target){
            // stack->head=path.head->node;
            // exit out of loop
            break;
        }

        // continue 
        // go through all adjacent rooms 
        for(int i =0;i<path.head->node->room->connection_count;i++) {

            // before adding, check the name of this room with the current room, so we don't loop unnecessarily
            if(strcmp(path.head->node->room->name, path.head->node->room->adjacent_rooms[i]->name)!=0){
                // allocate memory for RoomItem and RoomNode
                struct RoomNode* new_node=(struct RoomNode *)malloc(sizeof(struct RoomNode));
                // set the states of RoomNode
                new_node->room=path.head->node->room->adjacent_rooms[i];
                // point the prev to the RoomNode of the head 
                new_node->prev=path.head->node;

                // allocate memory for RoomItem
                struct RoomItem* new_item=(struct RoomItem *)malloc(sizeof(struct RoomItem));
                new_item->node=new_node;
                new_item->next=NULL;

                // make this the tail
                path.tail->next=new_item;
                path.tail=new_item;
            }

        }


        // instead of free-ing memory here
        struct RoomItem* temp=path.head;
        // save to a memory reference
        path.head=path.head->next;
        // point the head to the ref
        temp->next=ref;
        // this head becomes the new start of the ref (nothing special)
        ref=temp;
        
    }


    // then we need to make brand new 
    // path.head->node is the path found, node would the van -> target
    struct RoomNode* stack_generated=NULL;
    // reference to the nodes found needs to kept
    struct RoomNode* current_room=path.head->node;

    // through the loop to copy over room reference
    while(current_room != NULL){
        // alloc new memory
        struct RoomNode* new_node=(struct RoomNode*)malloc(sizeof(struct RoomNode));
        // copy over room
        new_node->room=current_room->room;
        // attach old to the new node
        new_node->prev=stack_generated;
        // new node becomes the head of the new stack
        stack_generated=new_node;

        // go to the next room
        current_room=current_room->prev;
    }
    // attach the new stack to the stack passed over
    stack->head=stack_generated;



    // need to call a function, for now put it here
    // attach the ref to the tail of Path list
    path.tail->next=ref;
    path.tail=ref;
    
    // now we can start free-ing memory of both RoomItems and RoomNodes
    while(path.head != NULL){
        // make a temp copy
        struct RoomItem* temp=path.head->next;
        // check if path.head->node is empty or not
        if(path.head->node != NULL){
            free(path.head->node);
            path.head->node=NULL;
        }
        // free the roomItem
        free(path.head);
        // then make the next roomitem the head
        path.head=temp;
    }


    return;
}

// function that simulates 1 hunter turn
void hunter_turn(struct Hunter* hunter) {

    // ignore if returning to exit
    // check if current room has a ghostc
    if(hunter->current_room->ghost != NULL){
        // resets boredom to 0
        hunter->boredom=0;
        // increases fear by 1
        hunter->fear++;

    } else {
        // increases boredom
        hunter->boredom++;
    }

    
    //check if hunter in exit room, check if we have complete evidence
    if(hunter->current_room->is_exit && hunter->return_to_exit) {
        // reached van complete
        // log it
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, false);

        // function to clean and clea breadcrumpstack
        clean_stack(hunter);

        // only add to the stack if pathfinding isn't active
        if(!PATH_FINDING){
            // wise to add back the starting room
            struct RoomNode* node=(struct RoomNode*)malloc(sizeof(struct RoomNode));
            node->prev=NULL;
            node->room=hunter->current_room;
            hunter->path.head=node;
        }

        // check if hunter has enough evidence
        // all we have to do is type cast collected evidencebyte into a ghostype
        // ghost_to_string and check if it isn't unknown
        const char* ghost_type = ghost_to_string((enum GhostType) hunter->case_file->collected);
        if(strcmp(ghost_type, "unknown") != 0){
            // we can enough evidence for our guess
            hunter->case_file->solved=true;
            // has exit var
            hunter->has_exited=true;
            // log the reason of exit
            hunter->log_reason=LR_EVIDENCE;
            // log the exit
            log_exit(
                hunter->id, hunter->boredom, 
                hunter->fear, hunter->current_room->name,
                hunter->device, LR_EVIDENCE
            );

            // make a ref to current room
            struct Room* current_room=hunter->current_room;

            // wait
            sem_wait(&current_room->mutex);

            // remove hunter from the current room
            remove_from_room(hunter);
            
            // continue
            sem_post(&current_room->mutex);
            
            // exits the simulation
            return;
        }

        // HERE IS THE DEVICE SWAP

        

        // // swap and randomly pick a device, this is not currently smart since we can pick the same device again
        // const enum EvidenceType* list;
        // int index=rand_int_threadsafe(0, get_all_evidence_types(&list));

        // // log swap device
        // log_swap(hunter->id, hunter->boredom, hunter->fear, hunter->device, list[index]);

        // // swap the devices
        // hunter->device=list[index];

        enum EvidenceType oldDevice=hunter->device;

        // look into hunter device_collection
        // MAX_EVIDENCE_TYPES
        // check size first, if false then reset it to 0
        if(MAX_EVIDENCE_TYPES <= hunter->device_collection.size){
            hunter->device_collection.size=0;
        }

        // when swapping we are gonna check the evidence was already found using it if not use it
        while((hunter->device_collection.devices[hunter->device_collection.size] & hunter->case_file->collected) > 0) {
            printf("already used: %s\n", evidence_to_string(hunter->device_collection.devices[hunter->device_collection.size]));
            if(hunter->device_collection.size < MAX_EVIDENCE_TYPES){
                hunter->device_collection.size++;
            }else{
                hunter->device_collection.size=0;
            }
        }

        // pick the next device
        hunter->device = hunter->device_collection.devices[hunter->device_collection.size++];

        // log swap device
        log_swap(hunter->id, hunter->boredom, hunter->fear, oldDevice, hunter->device);


        // HERE IS THE DEVICE SWAP

        // we can turn off the return to the exit flag
        hunter->return_to_exit=false;
        return;
    }


    // boredom checker
    if(hunter->boredom >= ENTITY_BOREDOM_MAX){
        // has exit var
        hunter->has_exited=true;
        // log the reason of exit
        hunter->log_reason=LR_BORED;
        // log the exit
        log_exit(
            hunter->id, hunter->boredom, 
            hunter->fear, hunter->current_room->name,
            hunter->device, LR_BORED
        );

        // need a reference to current room
        struct Room* current_room=hunter->current_room;

        // start wait
        sem_wait(&current_room->mutex);

        // remove hunter from the current room
        remove_from_room(hunter);

        // wait over
        sem_post(&current_room->mutex);

        return;
    }
    
    // fear checker
    if(hunter->fear >=  HUNTER_FEAR_MAX){
        // has exit var
        hunter->has_exited=true;
        // log the reason of exit
        hunter->log_reason=LR_AFRAID;
        // log the exit
        log_exit(
            hunter->id, hunter->boredom, 
            hunter->fear, hunter->current_room->name,
            hunter->device, LR_AFRAID
        );

        struct Room* current_room=hunter->current_room;

        // start wait
        sem_wait(&current_room->mutex);

        // remove hunter from the current room
        remove_from_room(hunter);

        // wait over
        sem_post(&current_room->mutex);

        return;
    }


    // if we reach this point, evidence collection
    // if room is the starting location, we will skip collecting evidence and move to a new room
    if(!hunter->current_room->is_exit && !hunter->return_to_exit) {
        // evidence gathering 
        // check for a match
        // use hunter device
        unsigned char evidence_f=(hunter->device & hunter->current_room->evidence);
        // check if greater than 0
        if(evidence_f > 0){

            // RETURN TO EXIT

            // check the definition if path finding it turned on
            if(PATH_FINDING){
                // updates the breadcrumb stack
                // then bfs search
                bfs_search(hunter, &(hunter->path));
            }

            // use semphore to pause other threads from modifying
            sem_wait(&hunter->case_file->mutex);

            // evidence found
            // clear appropriate evidence bit in the room's evidence byte
            // invert device
            hunter->current_room->evidence&=~(hunter->device);
            // add evidence to the case file
            hunter->case_file->collected|=hunter->device;
            // set return to exit room
            hunter->return_to_exit=true;

            sem_post(&hunter->case_file->mutex);

            // RETURN TO EXIT

            //log evidence
            log_evidence(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device);
            // we need to log that hunter is heading back to van
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, true);

        }else{
            // if no evidence found, continue moving to the next room
            // small chance that hunter will return ot eh van to change equipment
            int k=rand_int_threadsafe(0, 12);
            int b=rand_int_threadsafe(0, 12);

            // maybe we can use boredom and fear to influence the decision to move or swap
            int boredom=hunter->boredom;
            int fear=hunter->fear;

            float comb=(float) (boredom*k+fear*b)/100;
            //printf("weighted value: %f\n", comb);

            // if k and b matches then return to exit room to change, small chance to return to exit
            if(0.5 < comb){

                // RETURN TO EXIT

                // check the definition if path finding it turned on
                if(PATH_FINDING){
                    bfs_search(hunter, &(hunter->path));
                }
                hunter->return_to_exit=true;

                // RETURN TO EXIT

                // we need to log that hunter is heading back to van
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, true);
            }else{
                struct Room* random_room;
                // finds a random adjacent room
                find_adjacent_room(hunter, &random_room);
                // keep moving to a new room
                hunter_relocate(hunter, random_room, true);
            }
        }
        return;
    }

    // else keeping moving to the next door
    // check if we are returning to exit
    if(hunter->return_to_exit) {
        // so we need to relocate to the prev room
        // we know which one it is by checking prev.head
        struct Room* prev_room=hunter->path.head->prev->room;
        // this is the previous room, we can move to it if it has space
        // we can pass the room address to hunter_relocate
        // and a flag to note, DONT add to breadstack
        hunter_relocate(hunter, prev_room, false);

        // to check if succesfull
        // check if the hunter current_room is equal to prev_room
        if(prev_room==hunter->current_room){
            // pop the top of the breadstack
            // we will need to free memory of the current room
            // since the head is malloced we would need to free it
            struct RoomNode* temp=hunter->path.head;
            // replace head wtih prev
            hunter->path.head=hunter->path.head->prev;
            // free memory
            free(temp);
        }
    }else{
        struct Room* random_room;
        // finds a random adjacent room
        find_adjacent_room(hunter, &random_room);
        // keep moving to a new room
        hunter_relocate(hunter, random_room, true);
    }
    return;
}

// find a random adjacent room
void find_adjacent_room(struct Hunter* hunter, struct Room** room){
    // select a random adjacent room
    int x=rand_int_threadsafe(0, hunter->current_room->connection_count);
    // set the pointer to point to this new room
    *room=hunter->current_room->adjacent_rooms[x];
    return;
}

// this function is used to move a hunter from room a to room b
void hunter_relocate(struct Hunter* hunter, struct Room* room, bool add_to_stack) {
    
    // old room
    struct Room* from_room=hunter->current_room;
    // two rooms
    struct Room* room_a=NULL;
    struct Room* room_b=NULL;

    // compare which room will be a and b
    // we have [room, hunter->current_room]
    int cmp=strcmp(room->name, from_room->name);
    room_a=(cmp < 0) ? room : from_room;
    room_b=(cmp < 0) ? from_room : room;

    // pass the mutex of this room and next room
    sem_wait(&room_a->mutex);
    sem_wait(&room_b->mutex);

    // check if the rooom has space for one more hunter
    if (room->hunter_count < MAX_ROOM_OCCUPANCY) {

        // printf("-----------------------------\n");
        // printf("Current room name: %s\n", hunter->current_room->name);
        // printf("hunter counter before: %d\n", room->hunter_count);

        // remove hunter from room
        remove_from_room(hunter);


        // printf("New room name: %s\n", room->name);
        // printf("hunter counter after: %d\n", room->hunter_count);
        // printf("maximum hunter: %d\n", MAX_ROOM_OCCUPANCY);

        // there is space here 
        // next room is the current room
        room->hunters[room->hunter_count]=hunter;
        
        // increases the hunter count
        room->hunter_count++;

        // update the current room
        hunter->current_room=room;


        // printf("Unlocked from room: %s\n", room->name);
        // printf("Unlocked to room: %s\n", from_room->name);
        // printf("-----------------------------\n");
    
        // logs the move
        log_move(hunter->id, hunter->boredom, hunter->fear, from_room->name, room->name, hunter->device);


        // only pushes to the breadstack if flag is true
        if(add_to_stack && !PATH_FINDING){
            // pushes the next room to the top of the breadcrumb stack
            // to_room <- from_room <- prev
            // need to malloc memory
            struct RoomNode* new_node=(struct RoomNode*)malloc(sizeof(struct RoomNode));
                    
            // set the current room
            new_node->room=room;

            // check if head is empty or not
            if(hunter->path.head != NULL){
                // prev would be the current head
                new_node->prev=hunter->path.head;
            } else {
                new_node->prev=NULL;
            }

            // set this node as the head
            hunter->path.head=new_node;
        }
    }

    // semtex wait over
    sem_post(&room_b->mutex);
    sem_post(&room_a->mutex);

    // no space so we stay here
    return;
}

// this function is used to remove the connection between a room and hunter
void remove_from_room(struct Hunter* hunter) {

    // printf("Going to remove hunter from '%s' \n", hunter->current_room->name);
    // printf("Hunter ID: [%d]\n", hunter->id);

    if(hunter->current_room == NULL) {
        return;
    }

    // when multithreading, we would need to use mutex to lock this room
    // we need to through the current room's hunters array, find our hunter by comparing id and name
    // then start shifting hunters.
    int index=-1;    
    for(int i =0;i<hunter->current_room->hunter_count;i++){
        // comparing hunter name and id
        if((hunter->current_room->hunters[i]->id==hunter->id) 
            && (strcmp(hunter->current_room->hunters[i]->name, hunter->name)==0)){
                index=i;
                break;
        }
    }   

    if(index == -1){
        // Hunter not in starting room
        hunter->current_room=NULL;
        return;
    }

    
    // i = i + 1
    for(int i =index;i<hunter->current_room->hunter_count-1;i++){
        hunter->current_room->hunters[i]=hunter->current_room->hunters[i+1];
    }

    // decrement hunter_count in the room
    hunter->current_room->hunter_count--;

    // remove hunter from the current room
    hunter->current_room=NULL;
    
    return;
}