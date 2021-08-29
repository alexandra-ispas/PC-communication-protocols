#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void){

	write(1, "Cat:\n", 5);

	char filename[] = "da.txt";

	FILE *in = fopen(filename, "rt");

	if(in == NULL){
		write(1, "Failed to open\n", 15);
		exit(EXIT_FAILURE);
	}

	char *line = (char*)malloc(20*sizeof(char));
	if(line == NULL){
		exit(EXIT_FAILURE);
	}

    size_t len = 0;

	while(getline(&line, &len, in) != -1){

    	write(1, line, 20);
    }

    write(1, "\n", 1);

    free(line);
    fclose(in);

    return 0;
}
