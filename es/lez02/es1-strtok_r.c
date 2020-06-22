#include <stdio.h>
#include <string.h>

void token_r (char *arg, FILE *out) {
	char *tmp;
	char *token=strtok_r(arg, " ", &tmp);
	while (token) {
		fprintf (out, "%s\n", token);
		token=strtok_r(NULL, " ", &tmp);
	}
}

int main (int argc, char*argv[]) {
	for (int i=1; i<argc; i++) 
		token_r(argv[i], stdout);
	return 0;
}
