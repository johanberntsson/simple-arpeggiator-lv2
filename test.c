#include <math.h>
#include <stdio.h>
#include "minunit.h"

#include "arpeggiator.c"

int tests_run = 0;

int foo = 7;

static char* test_note_as_fraction_of_bar() {
    // test notes a fraction of a bar in different time signatures
    float d = 0.001;
    setTime(NOTE_1_1);
    mu_assert("error, 1/1 in 4/4", fabs(note_as_fraction_of_bar(4, 4) - 1.0) < d);
    setTime(NOTE_1_2);
    mu_assert("error, 1/2 in 3/4", fabs(note_as_fraction_of_bar(3, 4) - 0.666) < d);
    setTime(NOTE_1_8);
    mu_assert("error, 1/8 in 4/4", fabs(note_as_fraction_of_bar(4, 4) - 0.125) < d);
    setTime(NOTE_1_32);
    mu_assert("error, 1/32 in 4/4", fabs(note_as_fraction_of_bar(4, 4) - 0.03125) < d);
    setTime(NOTE_1_8);
    mu_assert("error, 1/8 in 3/4", fabs(note_as_fraction_of_bar(3, 4)  - 0.1666) < d);
    return 0;
}

static char* all_tests() {
    mu_run_test(test_note_as_fraction_of_bar);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
