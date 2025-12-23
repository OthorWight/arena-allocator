#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

/* --- BigInt Logic (Base 1,000,000,000) --- */

#define BIGINT_BASE 1000000000ULL

typedef struct {
    uint32_t *digits; /* Each digit is 0..999,999,999 */
    size_t len;
    size_t capacity; 
} BigInt;

BigInt *bigint_from_int(Arena *a, int value) {
    BigInt *res = arena_alloc_struct(a, BigInt);
    res->capacity = 4; /* Small start */
    res->digits = arena_alloc_array(a, uint32_t, res->capacity);
    
    if (value == 0) {
        res->len = 1; 
        res->digits[0] = 0;
    } else {
        res->len = 0;
        while (value > 0) {
            res->digits[res->len++] = value % BIGINT_BASE;
            value /= BIGINT_BASE;
        }
    }
    return res;
}

BigInt *bigint_add(Arena *a, BigInt *n1, BigInt *n2) {
    BigInt *sum = arena_alloc_struct(a, BigInt);
    
    size_t max_len = (n1->len > n2->len) ? n1->len : n2->len;
    sum->capacity = max_len + 1;
    sum->digits = arena_alloc_array(a, uint32_t, sum->capacity);
    sum->len = 0;

    uint64_t carry = 0;
    for (size_t i = 0; i < max_len || carry; i++) {
        uint64_t val1 = (i < n1->len) ? n1->digits[i] : 0;
        uint64_t val2 = (i < n2->len) ? n2->digits[i] : 0;
        
        uint64_t total = val1 + val2 + carry;
        
        /* Modulo arithmetic for Base 1e9 */
        sum->digits[sum->len++] = (uint32_t)(total % BIGINT_BASE);
        carry = total / BIGINT_BASE;
    }
    
    return sum;
}

BigInt *bigint_copy(Arena *dest, BigInt *src) {
    BigInt *copy = arena_alloc_struct(dest, BigInt);
    copy->len = src->len;
    copy->capacity = src->len;
    copy->digits = arena_alloc_array(dest, uint32_t, copy->capacity);
    /* Note: size is calculated in bytes, so multiply by sizeof(uint32_t) */
    memcpy(copy->digits, src->digits, src->len * sizeof(uint32_t));
    return copy;
}

/* Helper to print Base 1e9 numbers correctly */
void bigint_print_head(BigInt *n, int digit_count) {
    if (n->len == 0) {
        printf("0");
        return;
    }

    /* 1. Print the most significant block normally (no leading zeros) */
    printf("%u", n->digits[n->len - 1]);

    /* 2. Print remaining blocks with 9-digit padding (e.g. 000000123) */
    int printed_digits = 0; // Rough count, not exact
    
    for (int i = (int)n->len - 2; i >= 0; i--) {
        if (printed_digits >= digit_count) break;
        printf("%09u", n->digits[i]);
        printed_digits += 9;
    }
}

/* --- Main Logic --- */

int main() {
    Arena a1 = {0}; arena_init(&a1);
    Arena a2 = {0}; arena_init(&a2);

    Arena *curr_arena = &a1;
    Arena *next_arena = &a2;

    int target_n = 5000000;
    
    printf("Calculating Fibonacci(%d) (Base 1e9 + Ping-Pong)...\n", target_n);
    clock_t start = clock();

    BigInt *n1 = bigint_from_int(curr_arena, 0);
    BigInt *n2 = bigint_from_int(curr_arena, 1);
    BigInt *result = NULL;

    for (int i = 2; i <= target_n; i++) {
        BigInt *sum = bigint_add(next_arena, n1, n2);
        BigInt *n2_copy = bigint_copy(next_arena, n2);

        arena_reset(curr_arena);

        Arena *temp = curr_arena;
        curr_arena = next_arena;
        next_arena = temp;

        n1 = n2_copy;
        n2 = sum;
    }
    result = n2;

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Done!\n");
    /* Approximate digit count: (blocks-1)*9 + digits_in_top_block */
    /* Exact calculation is verbose, this is close enough for stats */
    printf("Result Blocks: %zu (Approx %zu decimal digits)\n", 
           result->len, result->len * 9);
    printf("Time Taken:    %.4f seconds\n", time_taken);
    
    printf("First ~50 digits: ");
    bigint_print_head(result, 50);
    printf("...\n\n");

    printf("--- Memory Usage ---\n");
    printf("[Arena 1] "); arena_print_stats(&a1);
    printf("[Arena 2] "); arena_print_stats(&a2);

    arena_free(&a1);
    arena_free(&a2);

    return 0;
}
