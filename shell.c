#include <stdio.h>
#include <string.h>
#include <stdbool.h> 

// true and false
#define true 1
#define false 0


// define union for flags
typedef union
{
    unsigned int bitFlags : 32;
    
    struct {
       unsigned int testing    : 1;
       unsigned int verbose    : 1;
       unsigned int unassigned : 30; // subtract when adding new flag
    } Flags;
} BITFLAGS;

// function definitions
int initFlags (BITFLAGS *f);
int reactorLoop (BITFLAGS *f);
int setFlags (BITFLAGS *f, int argc, char **argv);
char *trimExternalWhiteSpace(char *line);
bool isSpecialChar(char ch);
char **specialCharWhiteSpaceAdder(char *line); 
char **parseWhitespace(char *line);
int getBucketLength(char *line); 
char **parseCommand(char *line); 
char **parseArguments(char *line);
char **executeArguments(char **args);

int main (int argc, char **argv) {   
    
    // --- SETUP ---
     
    BITFLAGS flags;
    setFlags(&flags, argc, argv);
    
    // --- MAIN SHELL ---
    
    reactorLoop(&flags);
    
	return 0;
}

int reactorLoop (BITFLAGS *f) {
    
    char *user = "USER";
    char *machine = "Machine";
    char *path = "PWD";
    char command[80];
    int i = 0;
	
    while(true) {
        strcpy(command, ""); // clear old command
        printf("%s@%s :: %s =>", user, machine, path);
        fgets(command, 80, stdin);
        
		
        // remove newline
        for (; i < 80; i++) {
            if (command[i] == '\n') {
                command[i] = '\0';
                break;
            }
        }
    
        // testing output
        if ( f->Flags.testing == true) {
            printf("%s\n", command);
        }
        
        // exit shell
        if (strcmp(command, "exit") == 0) {
            printf("Exiting Shell...\n");
            break;
        }
		// parse the command 
		else {parseCommand(command);}
    }
}

// sets default values for flags
int initFlags (BITFLAGS *f) {
    f->bitFlags = 0; // sets all flags off
    f->Flags.verbose = true; // turns on verbose
    return 0;
}

int setFlags (BITFLAGS *f, int argc, char **argv) {
    initFlags(f);

    int i;
    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--testing") == 0) {
            printf("Launching in testing mode.\n");
            f->Flags.testing = true;
        }
        if (strcmp(argv[i], "--testing=off") == 0) {
            printf("Launching with testing mode off.\n");
            f->Flags.testing = true;
        }
        if (strcmp(argv[i], "--verbose") == 0) {
            printf("Launching in verbose mode.\n");
            f->Flags.verbose = true;
        }
        if (strcmp(argv[i], "--verbose=off") == 0) {
            printf("Launching with verbose mode off.\n");
            f->Flags.verbose = false;
        }
    }
}

// handles leading and trailing white space
char *trimExternalWhiteSpace(char *line)
{
	char *end;
	
	while(isspace((unsigned char)*line)) line++;
	
	if(*line == 0)
		return line;
	
	end = line + strlen(line) - 1;
	while(end > line && isspace((unsigned char)*end)) end--;
	
	*(end+1) = 0;
	
	return line;
}

// determines if char is special 
bool isSpecialChar(char ch)
{	
	int i = 0;
	char *SPECIAL_CHAR = "|><&$~";
	for(; i < strlen(SPECIAL_CHAR); i++) {
		if (ch == SPECIAL_CHAR[i]) {return true;}
	}
	return false;
}

/* adds space to special chars that need it*/
char **specialCharWhiteSpaceAdjust(char* line)
{
	int i, j = 0;
	char str[255];
	for(i = 0; i < strlen(line); i++) {
		if(isSpecialChar(line[i])) {
			if(isspace((unsigned char) line[i-1]) && isspace((unsigned char) line[i+1])) {str[j++] = line[i];}			
			if(!isspace((unsigned char) line[i-1])) {str[j++] = ' ';
				if(isspace((unsigned char) line[i+1])) {str[j++] = line[i];}}
			if(!isspace((unsigned char) line[i+1])) {str[j++] = line [i]; str[j++] = ' ';}
		}
		else {str[j++] = line[i];}
	}
	str[j] = '\0';
	return trimExternalWhiteSpace(str);
}	

/* handles interior white space and special character spacing*/ 
char **parseWhitespace(char* line)
{
	line = trimExternalWhiteSpace(line); 
	int i, j = 0;
	char str[255]; 
	for(i = 0; i < strlen(line); i++) {
		/* multiple white space check */
		if(isspace((unsigned char) line[i])) {
			if(isspace((unsigned char) line[i+1]) && !isspace((unsigned char) line[i-1])) {str[j++] = line[i];}
			if(!isspace((unsigned char) line[i+1]) && !isspace((unsigned char) line[i-1])) {str[j++] = line[i];}
		}
		else {str[j++] = line[i];}
	}	
	str[j] = '\0';
	return specialCharWhiteSpaceAdjust(str); 
}

//determine number of buckets by space calculation
int getBucketLength(char *line)
{
	int i = 0;
	int counter = 0;
	
	for(; line[i] != '\0'; i++) {
		if (line[i] == ' ') {
			counter++;
		}
	}
	return counter * 2; 
}

//parses the command line into separate arguments 
char **parseCommand(char *line) 
{
	char **args = (char**) malloc(sizeof(char**) * getBucketLength(line));
	line = parseWhitespace(line);
	/*printf("inside my parse: %s\n", line);*/	
	args = parseArguments(line); 
	executeArguments(args);
	
	
	
	return NULL; 
}	

//parses line into array of string arguments 
//do not remove duplicate function delcarations 
//or unused character arrays 
char **parseArguments(char *line)
{
	/*printf("parse_arguments: %s\n", line);*/
	int offset = 0;
	int x = 0;
	char **bucket = malloc(sizeof(char* ) * getBucketLength(line)); 
	
	char temp[255];
	char *temp2[255]; 
	
	int i = 0;
	int j = 0;
	int k = 0;
		
	/* traverse input string */ 
	for(; i < strlen(line); i++) {
		
		/* special character */ 
		if(isSpecialChar(line[i])) { 
			bucket[k] = malloc(sizeof(char*) * 1); 
			bucket[k] = line[i];
			/*printf("bucket[%i]: %c\n", k, bucket[k]);*/
			k++;
		}
		
		/* build string from other symbols */
		if(!isspace((unsigned char) line[i]) && !isSpecialChar(line[i])) {
			temp[j++] = line[i];
		}

		/* add completed strings to bucket */
		if(i > 0)
		{
			if((isspace((unsigned char) line[i]) && !isSpecialChar((unsigned char) line[i-1])) ||
				i+1 == strlen(line)) {
				temp[j] = '\0';
				bucket[k] = malloc( sizeof(char*) * j);
				
				int x = 0;
				for(; x < j; x++) {
					bucket[k][x] = temp[x];
				}
				/* printf("bucket[%i]: %s\n", k, bucket[k]); */
				j = 0;
				k++;
			}
		}	
	}	
	bucket[k] = '\0';		
	return bucket;
}

/*
loop through args array and execute commands

~~~~~~~~~loop through char** args like list[][] in python~~~~~~~~~~~~
*/
char **executeArguments(char **args)
{
	/*
	THE GOODS 
	*/
	
	return NULL;
}


