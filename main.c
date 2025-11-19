#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"
#include "ghost.h"
#include "hunter.h"

int main() {
    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

    // intialize house structure
    struct House house;

    // populate house with rooms 
    house_populate_rooms(&house);

    // lets begin with ghost initialization
    ghost_init(&house);

    // to proceed with hunter, we need to keep asking the user for id and name
    insert_hunters(&house);

    printf("\nStarting simulation with %d %s\n",
         house.hunter_collection.size, 
         house.hunter_collection.size > 1 ? "hunters..." : "hunter..."
    );

    // start simulation 
    start_simulation(&house);

    // evidence counter
    int hunter_exited_evidence=0;
    
    printf("=====================================\n");
    printf("Investigation Results: \n");
    printf("=====================================\n");
    for(int i =0;i<house.hunter_collection.size;i++){
          char k=house.hunter_collection.array[i].log_reason==LR_EVIDENCE;
          hunter_exited_evidence+=k;
          printf("[%s] Hunter %s (ID %d) exited because of [%s] (bored=%d fear=%d)\n",
               k > 0 ? "*" : " ",
               house.hunter_collection.array[i].name, house.hunter_collection.array[i].id,
               exit_reason_to_string(house.hunter_collection.array[i].log_reason),
               house.hunter_collection.array[i].boredom,
               house.hunter_collection.array[i].fear   
          );
    }
    printf("\n");
    printf("Shared Case File Checklist:\n");
    const enum EvidenceType* list;
    int size=get_all_evidence_types(&list);
    for(int i =0;i<size;i++){
          printf("  - [%s] %s\n",
                ((int)list[i] & house.case_file.collected) > 0 ? "*" : " ",
                evidence_to_string(list[i])
          );
    }

    printf("Victor Results: \n");
    printf("-------------------------------------\n");
    printf("- Hunters exited after identifying the ghost: %d/%d\n", hunter_exited_evidence, house.hunter_collection.size);
    printf("- Ghost Guess: %s\n", ghost_to_string((enum GhostType) house.case_file.collected));
    printf("- Actual Ghost Type: %s\n", ghost_to_string(house.ghost.ghost_type));

    printf("\nOverall Result: %s\n", house.case_file.solved ? "Hunters Win!" : "Ghost Win");

    // clean memory
    clean_memory(&house);

    return 0;
}
