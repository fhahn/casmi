#include <stdio.h>
#include <stdlib.h>

#include "libparse/parser.h"

extern char *filename;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s: Invalid number of arguments\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE* f = fopen(argv[1], "rt");
    if (f == NULL) {
        fprintf(stderr, "%s: could not open file '%s'\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }

    filename = argv[1];
    return parse(f);
}
