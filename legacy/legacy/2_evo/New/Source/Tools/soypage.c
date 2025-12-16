#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STR 500
#define MAX_FILES 32
#define FALSE 0
#define TRUE 1



int main(int argc, char** argv) {
	int i;
	char outStr[MAX_STR];
	char inStr[MAX_STR];
	char styleStr[MAX_STR];
	char templStr[MAX_STR];
	
	char curChar = 0;
	FILE* styleFile;
	FILE* templateFile;
	FILE* inputFile;
	FILE* outputFile;


	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
				case('o'): {
					if(i+1 <= argc) {
						sprintf(outStr, "%s", argv[i+1]);
						i++;
					}
					break;
				}
				case('i'): {
					if(i+1 <= argc) {
						sprintf(inStr, "%s", argv[i+1]);
						i++;
					}
					break;
				}

				case('s'): {
					if(i+1 <= argc) {
						sprintf(styleStr, "%s", argv[i+1]);
						i++;
					}
					break;
				}
				case('t'): {
					if(i+1 <= argc) {
						sprintf(templStr, "%s", argv[i+1]);
						i++;
					}
					break;
				}
				default: { break; }
			}
		}
	}


	inputFile = fopen(inStr, "r");
	if(inputFile == NULL) {
		return -1;
	}
	styleFile = fopen(styleStr, "r");
	if(styleFile == NULL) {
		fclose(inputFile);
		return -1;
	}
	templateFile = fopen(templStr, "r");
	if(templateFile == NULL) {
		fclose(inputFile);
		fclose(styleFile);
		return -1;
	}

	outputFile = fopen(outStr, "w");
	if(outputFile == NULL) {
		fclose(inputFile);
		fclose(styleFile);
		fclose(templateFile);
		return -1;
	}
	
	while(fscanf(templateFile, "%c", &curChar) != EOF) {
		fprintf(outputFile, "%c", curChar);
	}
	while(fscanf(styleFile, "%c", &curChar) != EOF) {
		fprintf(outputFile, "%c", curChar);
	}
	while(fscanf(inputFile, "%c", &curChar) != EOF) {
		fprintf(outputFile, "%c", curChar);
	}

	fclose(outputFile);
	return 0;
}
