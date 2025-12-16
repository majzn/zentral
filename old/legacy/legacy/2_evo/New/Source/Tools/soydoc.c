#include <stdlib.h>
#include <stdio.h>


#define SOY_MAX_CSTRING 511

unsigned int soydoc_cstring_len(const char* str) {
	unsigned int index = 0;
	while(index < SOY_MAX_CSTRING && str[index] != '\0') {
		index++;
	}
	if(index == SOY_MAX_CSTRING) {
		index = 0;
	}
	return index;
}

unsigned int soydoc_cstring_lentok(const char* str, char token, int tokIndex) {
	unsigned int index = 0;
	while(index < SOY_MAX_CSTRING && (str[index] != '\0' && tokIndex >= 0)) {
		if(str[index] == token) {
			tokIndex--;
		}
		index++;
	}
	if(index == SOY_MAX_CSTRING) {
		index = 0;
	}
	return index;
}

unsigned int soydoc_cstring_lentok_back(const char* str, char token, int tokIndex) {
	unsigned int index = soydoc_cstring_lentok(str, 0, 0);
	while(index > 0 && tokIndex >= 0) {
		if(str[index-1] == token) {
			tokIndex--;
		}
		index--;
	}
	return index;
}

unsigned int soydoc_cstring_compare(const char* strA, const char* strB) {
	unsigned int index = 0;
    unsigned int dif = 0;
    unsigned int lenA = soydoc_cstring_len(strA);
    unsigned int lenB = soydoc_cstring_len(strB);
    if(lenA != lenB) { return -1; }
	while(index < lenA) {
      if(strA[index] != strB[index]) {
        dif++;
      }
      index++;
	}
	return dif;
}

int main(int argc, char** argv) {
  int args = argc-1;
  FILE* input;
  FILE* outputHtml;
  FILE* inputHtml;
  char outStr[512];
  char lineBuffer[512];
  int tokIndex = 0;
  int startDoc = 0;
  int comment = 0;
  int func = 0;
  int doHeader = 0;
  int doType = 0;
  char* inputStr;
  char curChar = 0;
  char inChar = 0;
  int insLen = 0;
  const char* pStyle = "class=\"color:SandyBrown; background-color:#1a1e1f;\"";
  const char* hStyle = "style=\"color:SandyBrown; background-color:#1a1e1f;\"";
  const char* iStyle = "style=\"color:Tomato; background-color:#11161a;\"";
  const char* fStyle = "style=\"color:MediumAquaMarine; background-color:#11161a;\"";
  const char* tStyle = "style=\"color:#6faaaa; background-color:#11161a;\"";
  if(argc < 2) {
    printf("Please provide an argument\n");
    return -1;
  }
  printf("soyDoc: Generating html documentation for \"%s\"...", argv[1]);
  inputHtml = fopen("template.soy", "r");
  if(inputHtml == NULL) {
    inputStr = NULL;
  }
  inputStr = malloc(1024*1024*100);
  if(inputStr == NULL) {
    fclose(inputHtml);
    printf("Could not allocate memory. Aborting...\n");
    return -1;
  }
  
  if(inputHtml != NULL) {
    while(fscanf(inputHtml, "%c", &inChar) != EOF) {
      sprintf(inputStr+insLen, "%c", inChar);
      insLen++; 
    }
    fclose(inputHtml);
  } else {
    inputStr = NULL;
  }
 

  while(args) {
    input = fopen(argv[1], "r");

    if(input != NULL) {
      sprintf(outStr, "%s", argv[1]);
      tokIndex = soydoc_cstring_lentok(outStr, '.', 3);
      if(tokIndex > 0) {
        outStr[tokIndex-1] = 0;
      }
      sprintf(outStr, "%shtml", outStr);
      outputHtml = fopen(outStr, "w");
      if(outputHtml == NULL) {
        fclose(input);
        return -1;
      }
      if(inputStr != NULL) {
        fprintf(outputHtml, "%s", inputStr);
      }
      
      while(fscanf(input, "%s", lineBuffer) != EOF) {
        if(soydoc_cstring_compare(lineBuffer, "_docstart") == 0) {
          fscanf(input, "%s", lineBuffer);
          startDoc = 1;
        } else if(soydoc_cstring_compare(lineBuffer, "_docend") == 0) {
          startDoc = 0;
        }
  
        if(soydoc_cstring_compare(lineBuffer, "#define") == 0) {
          while(fscanf(input, "%c", &curChar) != EOF && curChar != '\n') {
          }
        }
        
        if(startDoc) {        
          if(soydoc_cstring_compare(lineBuffer, "{") == 0) {
            func++;
          }
      
          if(soydoc_cstring_compare(lineBuffer, "_docheader") == 0) {
            doHeader = 1;
            fprintf(outputHtml, "<h2 %s>", hStyle);
            fscanf(input, "%s", lineBuffer);
          }
      
          if(soydoc_cstring_compare(lineBuffer, "/*") == 0) {
            comment = 1;
            fprintf(outputHtml, "<b %s>", pStyle);
          }
     
          if(curChar == '\n' && !func && !doHeader) {
            fprintf(outputHtml, "<div style=\"padding: 0px;\">\n</div>");
          }

          fscanf(input, "%c", &curChar);
          if(!func) {
            doType++;
            if(doType == 1 && soydoc_cstring_compare(lineBuffer, "/*") != 0) {
              fprintf(outputHtml, "<b %s> %s%c </b>", iStyle, lineBuffer, curChar);
            } else if(doType == 2 && !comment) {
              fprintf(outputHtml, "<b %s> %s%c </b>", fStyle, lineBuffer, curChar);
            } else if(comment && !doHeader) {
              fprintf(outputHtml, "<small %s> %s%c </small>", tStyle, lineBuffer, curChar);
            } else if(doHeader) {
              fprintf(outputHtml, "<b %s> %s%c </b>", hStyle, lineBuffer, curChar);
            } else {
              fprintf(outputHtml, "%s%c", lineBuffer, curChar);
            }
          } else {
          }
 
          fseek(input, -1, SEEK_CUR);

          if(soydoc_cstring_compare(lineBuffer, "*/") == 0) {
            comment = 0;
            doType = 0;
            fprintf(outputHtml, "</b>");
            if(doHeader) {
              fprintf(outputHtml, "</h2>");
              doHeader = 0;
            }
          }
          if(soydoc_cstring_compare(lineBuffer, "}") == 0) {
            func--;
            doType = 0;
          }
        }
      }
      fclose(outputHtml);
      fclose(input);
    }
    args--;
  }
  printf(" Done!\n\n");
  free(inputStr);
  return 0;
}
