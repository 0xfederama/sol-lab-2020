#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tokenizer.h>

int main(int argc, char *argv[]) {
    for(int i=1;i<argc;++i) {
		char *str = strndup(argv[i], strlen(argv[i]));
		tokenizer(str, stdout);
		free(str);
    }

    for(int i=1;i<argc;++i) {
		tokenizer_r(argv[i], stdout);
    }
    
    return 0;
}
