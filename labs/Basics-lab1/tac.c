#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void my_read(FILE* in, char * line){

   	size_t len = 0;

   	char* line1 = (char*)malloc(20*sizeof(char));
   	if(line1 == NULL){
   		exit(EXIT_FAILURE);
   	}

	if(getline(&line1, &len, in) == -1){

		return;
	}
	
	my_read(in, line1);
	write(1, line1, 20);
	free(line1);
}

int main(void){

	write(1, "Tac:\n", 5);

	char filename[] = "da.txt";

	FILE *in = fopen(filename, "rt");

	if(in == NULL){

		write(1, "Failed to open\n", 15);
		exit(EXIT_FAILURE);
	}

	my_read(in, NULL);

	fclose(in);

	return 0;

}