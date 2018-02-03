#include <stdio.h>
#include <string.h>
<<<<<<< HEAD
#include <stdbool.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
=======
#include <stdbool.h> 
#include <ctype.h>
#include <stdlib.h>
>>>>>>> 1fc80e490e6fffe915a533217a85455bf736aab0

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
<<<<<<< HEAD
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
char **resolvePaths(char **args);
char **executeArguments(char **args);
int isCommand(char** args, int i);
bool isDir(const char *path);
bool isFile(const char *path);
bool fileExist(const char *path);
char *expandPath(char *path, int cmd_p);
=======
int     initFlags                   (BITFLAGS *f);
int     reactorLoop                 (BITFLAGS *f);
int     setFlags                    (BITFLAGS *f, int argc, char **argv);
char *  trimExternalWhiteSpace      (char *line);
bool    isSpecialChar               (char ch);
char ** specialCharWhiteSpaceAdder  (char *line);
char *  specialCharWhiteSpaceAdjust (char* line);
char *  parseWhitespace             (char *line);
int     getBucketLength             (char *line); 
char ** parseCommand                (char *line); 
char ** parseArguments              (char *line);
char ** resolvePaths                (char **args);
char ** executeArguments            (char **args);
int     isCommand                   (char **args, int i);
char *  expandPath                  (char *path, int cmd_p);
>>>>>>> 1fc80e490e6fffe915a533217a85455bf736aab0

int main (int argc, char **argv) {   
    
    // --- SETUP ---
     
    BITFLAGS flags;
    setFlags(&flags, argc, argv);
    
    // --- MAIN SHELL ---
    
    reactorLoop(&flags);
    
	return 0;
}

int reactorLoop (BITFLAGS *f) {
    
    char *user = getenv("USER");
    char *machine = getenv("MACHINE");
    char *path = getenv("PWD");
    char command[255];
    int i = 0;
	
    while(true) {
        strcpy(command, ""); // clear old command
        printf("%s@%s :: %s =>", user, machine, path);
        fgets(command, 255, stdin);
        
		
        // remove newline
        for (; i < 255; i++) {
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
char * trimExternalWhiteSpace(char *line)
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
char * specialCharWhiteSpaceAdjust(char* line)
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
char * parseWhitespace(char* line)
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
char ** parseCommand(char *line) 
{
	char **args = (char**) malloc(sizeof(char**) * getBucketLength(line));
	line = parseWhitespace(line);
	/*printf("inside my parse: %s\n", line);*/	
	args = parseArguments(line); 
	args = resolvePaths(args);
	
	/*executeArguments(args);*/
	
	return args; 
}	

//parses line into array of string arguments 
//do not remove duplicate function delcarations 
//or unused character arrays 
char **parseArguments(char *line)
char ** parseArguments(char *line)
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
			*bucket[k] = line[i];
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

char ** resolvePaths(char **args)
{
	int i = 0;
	for(; args[i] != NULL; i++) {
		args[i] = expandPath(args[i], isCommand(args, i)); 
	}
	return args;
}

/*
/returns 0 for argument, 
1 for external command, 
2 cd, 
3 for other built-in commands 
*/
int isCommand(char **args, int i) 
{
	if(*args[i] == '>' || 
		*args[i] == '<' ||
		*args[i] == '&' ||
		*args[i] == '|') {printf("external command\n");return 1;}
	
	if(strcmp(args[i], "cd") == 0) {printf("cd command\n");return 2;}
	
	if(strcmp(args[i], "echo") == 0 ||
		strcmp(args[i], "etime") == 0 ||
		strcmp(args[i], "io") == 0 ||
			
	else {/*printf("argument\n");*/return 0;}
	else {printf("argument\n");return 0;}
}

/*
returns expanded argument, does nothing in many cases (determined by is_command)
*/
char * expandPath(char *path, int cmd_p)
{
	//external commands  
	if(cmd_p == 1) {}
	//cd command
	if(cmd_p == 2) {}
	//for other built-ins
	if(cmd_p == 3) {return path;}
	//for arguments
	if(cmd_p == 4) {}
	
	 
	return NULL;
}

//determines if the file exist 
bool fileExist(const char *fname)
{
	if (access(fname, F_OK) != -1) {return true;}
	else {return false;}
}

//determiens if path is to a file 
bool isFile(const char *path)
{
	struct stat buf;
	stat(path, &buf);
	return S_ISREG(buf.st_mode); 
}

//determine if path is to a directory
bool isDir(const char *path)
{
	struct stat buf;
	stat(path, &buf);
	return S_ISDIR(buf.st_mode); 
}

/*
loop through args array and execute commands

~~~~~~~~~loop through char** args like list[][] in python~~~~~~~~~~~~
*/
char **executeArguments(char **args)
char ** executeArguments(char **args)
{
	//more goods
	return NULL;
}


