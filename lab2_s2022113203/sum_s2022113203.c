#include <stdio.h>

int main() {
	int a, total = 0;
	for(a = 0; a <= 500; a++) {
		total += a;
	}

	printf("The sum of 1 to 500 is %d\n", total);

	return 0;
}
