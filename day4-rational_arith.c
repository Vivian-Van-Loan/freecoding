#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <stdbit.h>

// typedef unsigned long ulong;

typedef struct rational_t {
    long numerator;
    long denominator;
} rational_t;

unsigned long gcd(unsigned long u, unsigned long v) { // Thank you wikipedia from: https://en.wikipedia.org/wiki/Binary_GCD_algorithm#Implementation on 2026-05-11
    if (!u || !v) return u | v;	// Identity #1

    unsigned shift = stdc_trailing_zeros(u | v);
    u >>= stdc_trailing_zeros(u);

    uint64_t min, max;
    while (v) {
        v >>= stdc_trailing_zeros(v); // Identity #3

        min = (u < v) ? u : v; // Identity #4
        max = (u > v) ? u : v;
        u = min;
        v = max - min;
    }
    return u << shift; // Identity #2
}

rational_t rational_canonize(rational_t val) {
    rational_t result;
    unsigned long div = gcd(val.numerator, val.denominator);
    result.numerator = val.numerator / div;
    result.denominator = val.denominator / div;
    if (result.numerator < 0 && result.denominator < 0) {
        result.numerator = -result.numerator;
        result.denominator = -result.denominator;
    } else if (result.denominator < 0) {
        result.numerator = -result.numerator;
    }
    return result;
}

#define RATIONAL_T_PRINTF "(%ld/%ld)"
void rational_print(rational_t val) {
    printf(RATIONAL_T_PRINTF, val.numerator, val.denominator);
}

char* rational_string(rational_t val) {
    int needed = snprintf(nullptr, 0, RATIONAL_T_PRINTF, val.numerator, val.denominator);
    char* str = malloc(needed + 1);
    snprintf(str, needed + 1, RATIONAL_T_PRINTF, val.numerator, val.denominator);
    return str;
}

rational_t rational_make(long numerator, long denominator) {
    rational_t result;
    result.numerator = numerator;
    result.denominator = denominator;
    return rational_canonize(result);
}

rational_t rational_from_integer(long integer) {
    return (rational_t){integer, 1};
}

rational_t rational_negate(rational_t val) {
    return (rational_t){-val.numerator, val.denominator};
}

rational_t rational_inverse(rational_t val) {
    return (rational_t){val.denominator, val.numerator}; //If you pass 0 to this you're the fool dealing with it
}

bool rational_equal(rational_t lhs, rational_t rhs) {
    return lhs.numerator == rhs.numerator && lhs.denominator == rhs.denominator;
}

bool rational_less(rational_t lhs, rational_t rhs) {
    return (lhs.numerator * rhs.denominator) < (rhs.numerator * lhs.denominator);
}

rational_t rational_add(rational_t lhs, rational_t rhs) {
    rational_t result;
    result.numerator = lhs.numerator * rhs.denominator + rhs.numerator * lhs.denominator;
    result.denominator = lhs.denominator * rhs.denominator;
    return rational_canonize(result);
}

rational_t rational_subtract(rational_t lhs, rational_t rhs) {
    return rational_add(lhs, rational_negate(rhs)); //I thought about it for a moment to make sure, but yeah it is correct. a - b = a + -b across all sets from Z to C
}

rational_t rational_multiply(rational_t lhs, rational_t rhs) {
    rational_t result;
    result.numerator = lhs.numerator * rhs.numerator;
    result.denominator = lhs.denominator * rhs.denominator;
    return rational_canonize(result);
}

rational_t rational_divide(rational_t lhs, rational_t rhs) {
    return rational_multiply(lhs, rational_inverse(rhs));
}

int main() {
    rational_t half = rational_make(1, 2);
    rational_t half_2 = rational_make(2, 4);
    printf("Half is equal to half? %c\n", rational_equal(half, half_2) ? 'Y' : 'N');

    printf("Halves are: ");
    rational_print(half);
    printf(", and ");
    rational_print(half_2);
    puts("");

    rational_t big = rational_make(4294967296, 1);
    rational_t small = rational_make(1, 16777216);
    printf("Let's multiply ");
    rational_print(big);
    printf(" and ");
    rational_print(small);
    printf(". Result: ");
    rational_t mul = rational_multiply(big, small);
    rational_print(mul);
    puts("");

    printf("Now some division of ");
    rational_print(big);
    printf(" by ");
    rational_print(mul);
    printf(". Result: ");
    rational_t div = rational_divide(big, mul);
    rational_print(div);
    puts("");

    printf("Now let's multiply those back: ");
    rational_print(big);
    printf(" * ");
    rational_print(div);
    printf(" = ");
    rational_t mul_big = rational_multiply(big, div);
    rational_print(mul_big);
    puts("");

    printf("Now divide that by 128398: ");
    rational_t arbitary = rational_divide(mul_big, rational_from_integer(128398));
    rational_print(arbitary);
    puts("");

    printf("Now let's add some stuff to that, like ");
    rational_print(half);
    printf(". Which is: ");
    rational_t add = rational_add(arbitary, half);
    rational_print(add);
    puts("");

    rational_t frac = rational_make(21783, 92198);
    printf("Now let's do some subtraction: ");
    rational_print(add);
    printf(" - ");
    rational_print(frac);
    printf(" = ");
    rational_t sub = rational_subtract(add, frac);
    rational_print(sub);
    puts("");

    printf("Now double that and subtract it from itself: ");
    rational_print(sub);
    printf(" - ");
    rational_t dubl = rational_multiply(sub, rational_from_integer(2));
    rational_print(dubl);
    printf(" = ");
    rational_t neg = rational_subtract(sub, dubl);
    rational_print(neg);
    puts("");

    printf("Is that less than 1? %c\n", rational_less(neg, rational_from_integer(1)) ? 'Y' : 'N');
    puts("Ok we're done here, wrap it up team.");
}
