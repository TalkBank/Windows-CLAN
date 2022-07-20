#include <stdio.h>

int main(int argc, char *argv[]) {
	int i;

	for (i=1; i < 256; i++) {
		fprintf(stdout, "0x%x=%c\n", i, i);
	}
}