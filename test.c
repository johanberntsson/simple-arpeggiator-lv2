#include <stdio.h>
#include "minunit.h"

#include "simplearpeggiator.c"

int tests_run = 0;

int foo = 7;

static char* test_calculateArpeggiatorStep() {
    // how big a note is of a full bar with different time signatures
    float d = 0.001;
	mu_assert("error, 1/1 in 4/4", fabs(calculateArpeggiatorStep(NOTE_1_1, 4, 4) - 1.0) < d);
	mu_assert("error, 1/2 in 3/4", fabs(calculateArpeggiatorStep(NOTE_1_2, 3, 4) - 0.666) < d);
	mu_assert("error, 1/8 in 4/4", fabs(calculateArpeggiatorStep(NOTE_1_8, 4, 4) - 0.125) < d);
	mu_assert("error, 1/32 in 4/4", fabs(calculateArpeggiatorStep(NOTE_1_32, 4, 4) - 0.03125) < d);
	mu_assert("error, 1/8 in 3/4", fabs(calculateArpeggiatorStep(NOTE_1_8, 3, 4)  - 0.1666) < d);
	return 0;
}

static char* test_foo() {
	mu_assert("error, foo != 7", foo == 7);
	return 0;
}

static char* all_tests() {
	mu_run_test(test_calculateArpeggiatorStep);
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
