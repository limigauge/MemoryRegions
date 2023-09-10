/**
 * regions.c
 *
 * COMP 2160 SECTION A01
 * INSTRUCTOR    NIKNAM
 * ASSIGNMENT    Assignment 4, question 1
 * AUTHOR        Michelle Li, 7866927
 * DATE          2021-12-12
 *
 * PURPOSE: Named Memory regions implementation. This program uses a list of linked list implemented regions which each hold another linked list of Nodes. Nodes hold the position of addresses in the region allocated memory that can be used.
 * The program is able to create and allocate regions, use memory in those regions, remove blocks of memory, and destroy regions.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "regions.h"

#define BYTE_8 8

typedef struct NODE Node;
typedef struct REGION Region;
typedef struct REGION_LIST r_List;

struct NODE {
    void *block; //address of start of block
    int start;  //start of block in terms of number of bytes into the region's buffer
    rsize_t size; //size of block of memory
    Node *next;
};

struct REGION {
    Region *next;
    char *name;
    void *buffer; //address of the allocated memory for the region
    rsize_t size; //size of the region's allocated memory
    Node *top;  
    int length; //the number of blocks of Nodes within this regions (used to test invariants)
}; //REGION struct

struct REGION_LIST {
    Region *top;
    Region *last;
    int size; //number of regions in the list
}; //list of regions

//static global variables for the current region chosen and the list of regions.
static Region *current = NULL;
static r_List *region_list = NULL;

/**
 * PURPOSE: checks invariants for the Region.
 * INPUT PARAMETERS:
 *    Region *region - Region to check for invariants.
 */

static void validate_region(Region *region){
    int count = 0;
    rsize_t sum = 0;
    Node *curr = NULL;
    Node *next = NULL;
    
    if(region->top != NULL){
        curr = region->top;

        while(curr != NULL){
            count++;
            sum = sum + curr->size;
            curr = curr->next;
        }

        assert(region->length == count); //make sure number of nodes matches expected count

        curr = region->top;
        if(curr->next != NULL){
            next = curr->next;

            while(next != NULL){
                assert((curr->start + curr->size) <= next->start); //make sure each start point is greater than the previous end point
                curr = next;
                next = curr->next;
            }
        }

        assert(sum <= region->size);

    }
}

/**
 * PURPOSE: Checks the invariants for the entire list of regions. It also calls to check the invariants within each region using validate_region();
 */

static void validate_r_list(){
    Region *curr = NULL;
    int count = 0;

    if(region_list != NULL){

        if(region_list->size == 1){
            assert(region_list->top == region_list->last);
        }
        curr = region_list->top;
        while(curr != NULL){
            count++;
            validate_region(curr); //each region in the list should also be valid.
            curr = curr->next;
        }
        assert(count == region_list->size);
    }

}

/**
 * PURPOSE: Creates a memory region and allocates memory for it. Names the region. Saves the region into a list and sets the current region to the newly created region.
 *          Will also create a new list if a list of regions has not been created yet.
 * INPUT PARAMETERS:
 *    const char *name - String to name the region
 *    rsize_t size - the amount of space to allocate for this region
 * OUTPUT PARAMETERS:
 *    Returns a boolean for whether or not the region creation was a success.
 */

Boolean rinit(const char *name, rsize_t size) {
    Boolean success = TRUE;
    Region *region = NULL;
    r_List *list = NULL;
    Region *curr = NULL;

    //check for dupes:
    if(region_list != NULL && region_list->top != NULL){
        curr = region_list->top;
        while(curr != NULL){
            if(strcmp(name, curr->name) == 0){
                success = FALSE;
            }
            curr = curr->next;
        }
    }

    if(success == TRUE){
        if (size <= 0 || strlen(name) < 1){
            success = FALSE;
        } else if(size % BYTE_8 != 0){
            region = malloc(sizeof(Region));
            region->buffer = malloc( (size/BYTE_8)*BYTE_8 + BYTE_8 );
            region->size = (size/BYTE_8)*BYTE_8 + BYTE_8;
        } else {
            region = malloc(sizeof(Region));
            region->buffer = malloc(size);
            region->size = size;
        }
    }

    if(success == TRUE){
        if(region_list == NULL){
            //create new list of regions
            list = malloc (sizeof(r_List));
            list->top = NULL;
            list->last = NULL;
            list->size = 0; //number of regions
            region_list = list;
        }

        region->next = NULL;
            
        region->name = malloc((strlen(name) + 1));
        strcpy(region->name, name);
        region->length = 0;
        region->top = NULL; //List for stored data yet.

        if(region_list->top == NULL) {
            //empty list; add first region
            region_list->top = region;
            region_list->last = region_list->top;
            region_list->size = region_list->size + 1;
        } else {
            //add new region to end of list
            region_list->last->next = region;
            region_list->last = region;
            region_list->size = region_list->size + 1;
        }

        memset(region->buffer, 0, region->size);
        current = region;
    }

    return success;
} //used list code from my assignment 3 submission

/**
 * PURPOSE: Chooses a new region in the list. Takes in a String containing the name of the region to change to and searches for that region in the list of regions.
 * INPUT PARAMETERS:
 *    const char *region_name - the name of the region the user would like to change to. 
 * OUTPUT PARAMETERS:
 *    Returns a boolean. True if the region change was a success, false if there was an error such as region does not exist.
 */

Boolean rchoose(const char *region_name){
    Boolean out = TRUE;
    Region *curr = NULL;

    validate_r_list();

    if(region_list->top != NULL){
        if(region_list->top != NULL){
            curr = region_list->top; 

            while (curr != NULL && strcmp(region_name, curr->name) != 0){
                curr = curr->next;
            }
            if(curr != NULL && strcmp(region_name, curr->name) == 0){
                current = curr;
            } else {
                out = FALSE;
            }
        }
    } else {
        out = FALSE;
        //no regions created
    }

    return out;
}

/**
 * PURPOSE: Tells the user which region they are on by returning the name of the current region.
 * OUTPUT PARAMETERS:
 *    const char *out - a String containing the name of the current region
 */

const char *rchosen(){
    char *out = NULL;
    validate_r_list();

    if(current != NULL){
        out = current->name;
    }

    return out;
}

/**
 * PURPOSE: Reserves a block of memory in the allocated region for the user to use. It saves a Node containing the address to where the memory is in the region to the linked list existing in the Region.
 * INPUT PARAMETERS:
 *    rsize_t block_size - the size of the memory the user would like to reserve. Can only reserve this if there is room in the region.
 * OUTPUT PARAMETERS:
 *    void * - returns a void pointer for the start of the allocated block in the region. 
 */

void *ralloc(rsize_t block_size){
    Region *region = current;
    Node *new_node;
    Node *curr;
    Node* next;
    void *out = NULL;
    Boolean found = FALSE; //if we found a place for it...
    rsize_t new_size;
    
    if(block_size % BYTE_8 != 0){
        new_size = (block_size/BYTE_8)*BYTE_8 + BYTE_8;
    } else {
        new_size = block_size;
    }

    validate_r_list();
    if(new_size <= current->size){
        if(region->top == NULL){
            //first node
            new_node = malloc(sizeof(Node));
            region->top = new_node;
            new_node->next = NULL;
            new_node->start = 0;
            new_node->size = new_size;
            new_node->block = current->buffer;
            out = new_node->block;
            region->length = region->length + 1;

            validate_r_list();

        } else if (region->top->start >= new_size){
            //check buffer start point to first node's size because first node doesn't start at beginning of allocated memory
            new_node = malloc(sizeof(Node));
            new_node->next = region->top;
            region->top = new_node;
            new_node->start = 0;
            new_node->size = new_size;
            new_node->block = current->buffer;
            out = new_node->block;
            region->length = region->length + 1;

            validate_r_list();

        } else {
            curr = region->top;
            while(found == FALSE && curr->next != NULL){
                next = curr->next;
                if(new_size <= (next->start - (curr->start + curr->size))){ //next's start - curr's end = space in between
                    //can fit here!
                    new_node = malloc(sizeof(Node));
                    curr->next = new_node;
                    new_node->next = next;
                    found = TRUE;
                }
                if(found == FALSE){
                    curr = next;
                }
            }
            if(found == FALSE && curr->next == NULL){
                //check if there is room at the very end of the list...
                if(new_size <= region->size - (curr->start + curr->size)){
                    new_node = malloc(sizeof(Node));
                    curr->next = new_node;
                    new_node->next = NULL;
                    found = TRUE;
                }
            } 

        }
        if(found == TRUE){
            new_node->start = curr->start + curr->size;
            new_node->size = new_size;
            new_node->block = region->buffer + new_node->start;
            out = new_node->block;
            region->length = region->length + 1;
        }
    }

    if(out != NULL){
        memset(new_node->block, 0, new_node->size);
    }

    validate_r_list();

    return out;

}

/**
 * PURPOSE: Takes in a pointer to a block allocated in the region and returns how many bytes are in that block.
 * INPUT PARAMETERS:
 *     void *block_ptr - void pointer to the start of a block in the region.
 * OUTPUT PARAMETERS:
 *    rsize_t - returns the number of bytes in the region pointed at by block_ptr. Returns 0 if the region does not exist.
 */

rsize_t rsize(void *block_ptr){
    rsize_t size = 0;
    Node *curr = NULL;

    validate_r_list();

    if(current != NULL){
        curr = current->top;
        while(curr != NULL && curr->block != block_ptr){
            curr = curr->next;
        }
        if(curr != NULL){
            size = curr->size;
        }
    }
    
    return size;
}

/**
 * PURPOSE: Removes the Node/block of memory in its allocated region. It also frees the Node (but not the allocated memory) so it can be used again.
 * INPUT PARAMETERS:
 *    void *block_ptr - a void pointer to the block that needs to be freed.
 * OUTPUT PARAMETERS:
 *    Boolean - returns false if the block does not exist.
 */

Boolean rfree(void *block_ptr){
    Boolean out = TRUE;
    Node *curr = NULL;
    Node *prev = NULL;
    //void *ptr = NULL;

    validate_r_list();

    if(current != NULL){
        curr = current->top;
        //ptr = curr;
        while(curr != NULL && curr->block != block_ptr){
            prev = curr;
            curr = curr->next;
        }

        if(curr == NULL){
            out = FALSE;
        } else {

            if(prev == NULL){
                current->top = curr->next; //remove first node
            } else {
                 prev->next = curr->next; //removes node from list
            }
            
            free(curr);
            current->length = current->length - 1;
        }
        
    }

    validate_r_list();
    
    return out;

}

/**
 * PURPOSE: Destroys a region, freeing everything within it. Uses a loop to free each region's Nodes one by one, then removes the region from the region list and frees the region as well.
 * INPUT PARAMETERS:
 *    const char *region_name - The name of the region the user would like to free.
 */

void rdestroy(const char * region_name){
    Region *prev_region = NULL;
    Region *curr_region = NULL;
    Node *curr = NULL;
    Node *next = NULL;

    validate_r_list();

    if(region_list != NULL && region_list->top != NULL){
        curr_region = region_list->top;
        while(curr_region != NULL && strcmp(region_name, curr_region->name) != 0){
            prev_region = curr_region;
            curr_region = curr_region->next;
        }

        if(curr_region != NULL){
            //found the region to be removed

            curr = curr_region->top;

            while(curr != NULL){
                next = curr->next;
                free(curr);
                curr = next;
            } // free all nodes in region

            if(current == curr_region){
                if(prev_region != NULL){
                    current = prev_region;
                } else if (curr_region->next != NULL){
                    current = curr_region->next;
                }//else, no regions left....
            } //setting a new current region in advance if it's the one to be deleted

            if(region_list->top == curr_region){
                region_list->top = curr_region->next;
                //removing first region on list
            } else {
                prev_region->next = curr_region->next;
            }

            if(region_list->size - 1 <= 1){
                //only one region left or no regions left...
                //either sets last region as top, or sets last region as NULL (also top)
                region_list->last = region_list->top;
            }

            free(curr_region->name);
            free(curr_region->buffer);
            free(curr_region);
            region_list->size = region_list->size - 1;
        }
    }
    validate_r_list();
}

/**
 * PURPOSE: Prints data about all the memory regions: Name of the region, followed by the address of each block of memory within the region and its size. It also prints the percentage of space remaining within the region.
 *          It repeats this for each region in the region list.
 */

void rdump(){
    Region *curr_reg = NULL;
    Node *curr = NULL;
    rsize_t curr_size = 0;

    validate_r_list();
    printf("\nPerforming rdump()...\n");

    if(region_list != NULL && current != NULL){

        curr_reg = region_list->top;
        while(curr_reg != NULL){
            printf("\nRegion name: %s\n", curr_reg->name);
            curr = curr_reg->top;
            while(curr != NULL){
                printf("    %p, size: %d\n", (curr_reg->buffer + curr->start), curr->size);
                curr_size = curr_size + curr->size;
                curr = curr->next;
            }
            printf("Percentage remaining: %d\n", (100 - 100*curr_size/curr_reg->size));
            curr_size = 0;
            curr_reg = curr_reg->next;
        }
    }
    printf("\n");
}