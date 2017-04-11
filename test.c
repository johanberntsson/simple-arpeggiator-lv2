#include <stdio.h>
#include "minunit.h"

#include "simplearpeggiator.c"

int tests_run = 0;

int foo = 7;

static char* test_calculateArpeggiatorStep() {
	mu_assert("error, 1/1 in 4/4 != 1", calculateArpeggiatorStep(NOTE_1_1, 4, 4) == 16);
	mu_assert("error, 1/2 in 3/4 != 0.66", calculateArpeggiatorStep(NOTE_1_2, 3, 4) == 6);
	mu_assert("error, 1/8 in 4/4 != 0.125", calculateArpeggiatorStep(NOTE_1_8, 4, 4) == 32);
	mu_assert("error, 1/32 in 4/4 != 0.3125", calculateArpeggiatorStep(NOTE_1_8, 4, 4) == 32);
	mu_assert("error, 1/8 in 3/4 != 24", calculateArpeggiatorStep(NOTE_1_8, 3, 4) == 24);
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
