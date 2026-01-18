#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

/* --- BigInt Logic (Base 1,000,000,000) --- */
#define BIGINT_BASE 1000000000ULL

typedef struct {
    uint32_t *digits; 
    size_t len;
    size_t capacity; 
} BigInt;

BigInt *bigint_alloc(size_t capacity) {
    BigInt *res = malloc(sizeof(BigInt));
    res->capacity = capacity;
    res->digits = calloc(capacity, sizeof(uint32_t));
    res->len = 1; 
    return res;
}

void bigint_set_int(BigInt *n, int value) {
    if (value == 0) {
        n->len = 1;
        n->digits[0] = 0;
    } else {
        n->len = 0;
        while (value > 0) {
            n->digits[n->len++] = value % BIGINT_BASE;
            value /= BIGINT_BASE;
        }
    }
}

/* OPTIMIZED ADDITION 
   - No 'if' statements inside the loop (Branchless)
   - No bounds checking per digit (Unrolled)
*/
void bigint_add_branchless(BigInt *dest, BigInt *n1, BigInt *n2) {
    // In Fib calculation, n1 and n2 are either equal length or off by 1.
    // We process the common length first to avoid checks.
    size_t len = (n1->len < n2->len) ? n1->len : n2->len;
    
    uint64_t carry = 0;
    
    // Direct pointer access for speed
    uint32_t *d_ptr = dest->digits;
    uint32_t *n1_ptr = n1->digits;
    uint32_t *n2_ptr = n2->digits;

    // --- HOT LOOP START ---
    // This loop runs millions of times. It must be branch-free.
    size_t i = 0;
    for (; i < len; i++) {
        uint64_t total = (uint64_t)n1_ptr[i] + n2_ptr[i] + carry;
        
        // Branchless Carry Calculation:
        // 'carry' becomes 1 if total >= BASE, else 0.
        carry = (total >= BIGINT_BASE); 
        
        // Subtract BASE if carry is 1. 
        // We use multiplication to avoid an 'if'. 
        // (carry * BASE) is either 0 or 1,000,000,000.
        d_ptr[i] = (uint32_t)(total - (carry * BIGINT_BASE));
    }
    // --- HOT LOOP END ---

    // Handle the tail (if n1 or n2 was longer)
    for (; i < n1->len; i++) {
        uint64_t total = n1_ptr[i] + carry;
        carry = (total >= BIGINT_BASE);
        d_ptr[i] = (uint32_t)(total - (carry * BIGINT_BASE));
    }
    for (; i < n2->len; i++) {
        uint64_t total = n2_ptr[i] + carry;
        carry = (total >= BIGINT_BASE);
        d_ptr[i] = (uint32_t)(total - (carry * BIGINT_BASE));
    }

    // Handle final carry
    if (carry) {
        d_ptr[i++] = 1;
    }

    dest->len = i;
}

void bigint_print_head(BigInt *n, int digit_count) {
    if (n->len == 0) { printf("0"); return; }
    printf("%u", n->digits[n->len - 1]);
    int printed_digits = 0;
    for (int i = (int)n->len - 2; i >= 0; i--) {
        if (printed_digits >= digit_count) break;
        printf("%09u", n->digits[i]);
        printed_digits += 9;
    }
}

void print_progress(int current, int total) {
    // Only update every ~1% to save I/O time
    if (current % (total/100) != 0 && current != total) return;

    const int bar_width = 40; 
    float progress = (float)current / total;
    int filled = (int)(bar_width * progress);

    printf("\r[");
    for (int i = 0; i < bar_width; ++i) {
        printf(i < filled ? "=" : (i == filled ? ">" : " "));
    }
    printf("] %d%%", (int)(progress * 100.0));
    fflush(stdout);
}

int main() {
    int target_n = 5000000;
    size_t MAX_BLOCKS = 120000; 

    printf("Calculating Fibonacci(%d) (Base 1e9 + Branchless Logic)...\n", target_n);
    clock_t start = clock();

    // 3 Static Buffers (Rotating strategy)
    BigInt *b0 = bigint_alloc(MAX_BLOCKS); 
    BigInt *b1 = bigint_alloc(MAX_BLOCKS); 
    BigInt *b2 = bigint_alloc(MAX_BLOCKS); 

    bigint_set_int(b0, 0);
    bigint_set_int(b1, 1);

    for (int i = 2; i <= target_n; i++) {
        print_progress(i, target_n);

        // Branchless Add
        bigint_add_branchless(b2, b1, b0);

        // Pointer Swap
        BigInt *temp = b0;
        b0 = b1;
        b1 = b2;
        b2 = temp; 
    }

    printf("\n\n");
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    BigInt *result = b1;

    printf("Done!\n");
    printf("Result Blocks: %zu (Approx %zu decimal digits)\n", 
           result->len, result->len * 9);
    printf("Time Taken:    %.4f seconds\n", time_taken);
    
    printf("First ~50 digits: ");
    bigint_print_head(result, 50);
    printf("...\n");

    free(b0->digits); free(b0);
    free(b1->digits); free(b1);
    free(b2->digits); free(b2);

    return 0;
}
