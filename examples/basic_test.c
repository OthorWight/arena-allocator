#include <stdio.h>
#include <string.h>
#include <assert.h>

/* --- SETUP --- */
#define ARENA_IMPLEMENTATION
#include "arena.h"

/* --- TEST HELPERS --- */

typedef struct {
    int id;
    double matrix[16];
    char name[64];
} GameObject;

void simulate_heavy_processing(Arena *a, int count) {
    ArenaTemp scratch = arena_temp_begin(a);
    
    for (int i = 0; i < count; i++) {
        /* Allocate junk that we don't need after this function */
        int *junk = arena_alloc_array(a, int, 1000); 
        junk[0] = i; /* Touch memory to ensure OS commits it */
    }

    arena_temp_end(scratch);
}

/* --- MAIN TESTS --- */

int main() {
    printf("=== Arena Stress Test ===\n\n");

    Arena a = {0};
    arena_init(&a);

    /* ------------------------------------------------------------------------
       TEST 1: Forced Growth
       ------------------------------------------------------------------------ */
    printf("1. Stressing Allocation & Growth...\n");
    
    int obj_count = 10000;
    GameObject *first_obj = NULL;
    GameObject *last_obj = NULL;

    for (int i = 0; i < obj_count; i++) {
        GameObject *obj = arena_alloc_struct(&a, GameObject);
        obj->id = i;
        sprintf(obj->name, "Object_%d", i);
        
        if (i == 0) first_obj = obj;
        if (i == obj_count - 1) last_obj = obj;
    }

    printf("   Allocated %d objects (~%zu KB total).\n", 
           obj_count, (obj_count * sizeof(GameObject)) / 1024);
    
    assert(first_obj->id == 0);
    assert(strcmp(first_obj->name, "Object_0") == 0);
    assert(last_obj->id == 9999);
    assert(strcmp(last_obj->name, "Object_9999") == 0);
    
    arena_print_stats(&a);
    printf("   [PASS] Growth successful. Data integrity verified.\n\n");


    /* ------------------------------------------------------------------------
       TEST 2: The Scratchpad (Temporary Memory)
       ------------------------------------------------------------------------ */
    printf("2. Stressing Scratchpad (Loop Re-use)...\n");
    
    ArenaTemp baseline = arena_temp_begin(&a);
    
    printf("   Starting loop (1000 iterations)...\n");
    for (int i = 0; i < 1000; i++) {
        simulate_heavy_processing(&a, 10); 
    }
    
    arena_temp_end(baseline);
    
    printf("   Loop finished.\n");
    arena_print_stats(&a);
    printf("   [PASS] If 'used' bytes is same as Test 1, scratchpad worked.\n\n");


    /* ------------------------------------------------------------------------
       TEST 3: The Reset (Mass Recycle)
       ------------------------------------------------------------------------ */
    printf("3. Stressing Arena Reset & Reuse...\n");
    
    arena_reset(&a);
    printf("   Arena reset called.\n");
    
    size_t huge_size = 50 * 1024; // 50KB
    void *huge_chunk = arena_alloc(&a, huge_size);
    memset(huge_chunk, 0xFF, huge_size); 

    printf("   Allocated 50KB chunk after reset.\n");
    arena_print_stats(&a);
    
    printf("   [PASS] Memory reused without crash.\n\n");


    /* ------------------------------------------------------------------------
       TEST 4: Zero Initialization
       ------------------------------------------------------------------------ */
    printf("4. Testing Zero Alloc...\n");
    typedef struct { int a; float b; void* c; } TestStruct;
    
    TestStruct *z = (TestStruct*)arena_alloc_zero(&a, sizeof(TestStruct));
    assert(z->a == 0);
    assert(z->b == 0.0f);
    assert(z->c == NULL);
    
    printf("   [PASS] memory was zeroed.\n\n");

    /* Cleanup */
    arena_free(&a);
    printf("=== All Tests Passed ===\n");

    return 0;
}
