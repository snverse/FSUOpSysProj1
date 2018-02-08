#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>

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
int     initFlags                   (BITFLAGS *f);
int     reactorLoop                 (BITFLAGS *f);
int     setFlags                    (BITFLAGS *f, int argc, char **argv);
char *  trimExternalWhiteSpace      (char *line);
bool    isSpecialChar               (char ch);
char ** specialCharWhiteSpaceAdder  (char *line);
char *  specialCharWhiteSpaceAdjust (char *line);
char *  parseWhitespace             (char *line);
int     getBucketLength             (const char *line, const char ch); 
char ** parseCommand                (char *line, BITFLAGS *f); 
char ** parseArguments              (char *line, BITFLAGS *f);
char ** resolvePaths                (char **args, BITFLAGS *f);
char ** executeCommands             (char **args, BITFLAGS *f);
int     isCommand                   (char **args, int i);
char *  expandPath                  (char *path, int cmd_p, BITFLAGS *f);
char * 	appendPath					(char *path, const char *append);
char * 	getPathFromEnv				(const char *env);
char ** split						(const char *path, const char ch);
char * 	buildPath					(char *path, const char *envVar, BITFLAGS *f);
bool	contains 					(const char *path, const char ch);
bool 	pathExist					(const char *path);
bool 	isFile						(const char *path);
bool 	isDir						(const char *path);
bool 	isPath2BuiltIn				(const char *path);

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
		else {parseCommand(command, f);}
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
	char *SPECIAL_CHAR = "|><&";
	for(; i < strlen(SPECIAL_CHAR); i++) {
		if (ch == SPECIAL_CHAR[i]) {return true;}
	}
	return false;
}

/* adds space to special chars that need it*/
char * specialCharWhiteSpaceAdjust(char* line)
{
	int i, j = 0;
	char str[255];
	for(i = 0; i < strlen(line); i++) {
		if(isSpecialChar(line[i])) {
			if(isspace((unsigned char) line[i-1]) &&
			    isspace((unsigned char) line[i+1])) 
			{
			    str[j++] = line[i];
			}			
			if(!isspace((unsigned char) line[i-1])) {
			    str[j++] = ' ';
				if(isspace((unsigned char) line[i+1])) {
				    str[j++] = line[i];}
				}
			if(!isspace((unsigned char) line[i+1])) {
			    str[j++] = line [i]; str[j++] = ' ';
			}
		}
		else {str[j++] = line[i];}
	}
	str[j] = '\0';
	return trimExternalWhiteSpace(str);
}	

/* handles interior white space and special character spacing*/ 
char * parseWhitespace(char* line)
{
	line = trimExternalWhiteSpace(line); 
	int i, j = 0;
	char str[255]; 
	for(i = 0; i < strlen(line); i++) {
		/* multiple white space check */
		if(isspace((unsigned char) line[i])) {
			if(isspace((unsigned char) line[i+1]) &&
			    !isspace((unsigned char) line[i-1]))
			{
			    str[j++] = line[i];
			}
			if(!isspace((unsigned char) line[i+1]) &&
			    !isspace((unsigned char) line[i-1]))
			{
			    str[j++] = line[i];
			}
		}
		else {str[j++] = line[i];}
	}	
	str[j] = '\0';
	return specialCharWhiteSpaceAdjust(str); 
}

//determine number of buckets by space calculation
int getBucketLength(const char *line, const char ch)
{
	int i = 0;
	int counter = 0;
	
	for(; line[i] != '\0'; i++) {
		if (line[i] == ch) {
			counter++;
		}
	}
	return counter * 2; 
}

//parses the command line into separate arguments 
char ** parseCommand(char *line, BITFLAGS *f) 
{
	char **args = (char**) malloc(sizeof(char**) * getBucketLength(line, ' '));
	line = parseWhitespace(line);
	if ( f->Flags.testing == true) {
	    printf("inside my parse: %s\n", line);	
    }
	args = parseArguments(line, f); 
	args = resolvePaths(args, f);
	
	executeCommands(args, f);
	
	free(args); 
	return args; 
}	

//parses line into array of string arguments 
//do not remove duplicate function delcarations 
//or unused character arrays 
char ** parseArguments(char *line, BITFLAGS *f)
{
    if ( f->Flags.testing == true) {
	    printf("parse_arguments: %s\n", line);
	}
	
	int offset = 0;
	int x = 0;
	char **bucket = malloc(sizeof(char* ) * getBucketLength(line, ' ')); 
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
			*bucket[k] = line[i];
			k++;
		}
		
		/* build string from other symbols */
		if(!isspace((unsigned char) line[i]) && !isSpecialChar(line[i])) {
			temp[j++] = line[i];
		}

		/* add completed strings to bucket */
		if(i > 0)
		{
			if((isspace((unsigned char) line[i]) && 
			    !isSpecialChar((unsigned char) line[i-1])) ||
				i+1 == strlen(line)) 
			{
				temp[j] = '\0';
				bucket[k] = malloc( sizeof(char*) * j);
				
				int x = 0;
				for(; x < j; x++) {
					bucket[k][x] = temp[x];
				}
				j = 0;
				k++;
			}
		}	
	}	
	bucket[k] = '\0';		
	return bucket;
}

/* resolve path conflicts */
char ** resolvePaths(char **args, BITFLAGS *f)
{
	 if ( f->Flags.testing == true) {
		printf("Resolving Paths...\n"); 
	 }
	int i = 0;
	for(; args[i] != NULL; i++) {
		args[i] = expandPath(args[i], isCommand(args, i), f); 
		printf("ARGS[%i]: %s\n", i, args[i]);
	}
	return args;
}

/*
returns
0 for argument, 
1 for external command, 
2 cd, 
3 for other built-in commands 
*/
int isCommand(char **args, int i) 
{
	if(*args[i] == '>' || 
		*args[i] == '<' ||
		*args[i] == '&' ||
		*args[i] == '|') {return 1;}
	
	if(strcmp(args[i], "cd") == 0) {return 2;}
	
	if(strcmp(args[i], "echo") == 0 ||
		strcmp(args[i], "etime") == 0 ||
		strcmp(args[i], "io") == 0) {return 3;} 
			
	else {return 0;}
}

//removes ~/ from path
char * homePathBuilder(char * path, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("Building Home Path...\n"); 
	 }
	
	if(strlen(path) == 2) {return getenv("HOME");}
	char * e = getenv("HOME");
	strcat(e, path+1); 

	if(pathExist(e)) {return e;} 	
	return NULL;
}

//removes ./ from line 
char * currentDirPathBuilder(char * path, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("Building path to current directory...\n"); 
	 }
		
	if(strlen(path) == 2) {return getenv("PWD");}
	char *currPath = getenv("PWD");	
	strcat(currPath, path+1); 
	
	if(pathExist(currPath)) {return currPath;}
	return NULL;
}

//determines index of last "/" in path
int getParentIndex(const char * path)
{
	int index = -1; 
	int i = 0;
	
	for(; i < strlen(path); i++) {
		if(path[i] == '/') {index = i;}}
	return index;
}

//builds parent path 
char * parentDirBuilder(char * path, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("building parent directory...\n"); 
	}
	
	char *currPath = getenv("PWD");
	int nlength = getParentIndex(currPath);
	char *pp = malloc(sizeof(char*) * (nlength + (strlen(path)-2))); 
	
	strncpy(pp, currPath, nlength); 
	strcat(pp, path+2);
	
	if(pathExist(pp)) {return pp;} 
	return NULL; 
}

//removes $KEYWORD in path and replaces with corresponding path 
char * envPathAmmend(char * path, char * envVar, BITFLAGS *f) 
{
	if ( f->Flags.testing == true) {
		printf("ammending envionrmental path...\n"); 
	}
	
	int keyword = strlen(envVar) + 1; 
	char *currPath = getenv(envVar); 
	
	strcat(currPath, path+keyword);

	if(pathExist(currPath)) {return currPath;}
	return NULL; 
}

/* finds built in path */
char *expandBuiltIn(char *path, BITFLAGS *f) {
	
	if ( f->Flags.testing == true) {
		printf("expanding built-ins...\n"); 
	}
	
	char **prePath_split = split(getenv("PATH"), ':');
	char *p; 
	int i = 0;
	
	for(; prePath_split[i] != NULL; i++) {
		p = prePath_split[i]; 
		strcat(p, "/"); 
		strcat(p, path);
		
		if(isPath2BuiltIn(p)) {return p;}
	}
	
	return NULL;
}

//checks if path leads to executable
bool isPath2BuiltIn(const char * path)
{
	if(access(path, X_OK)) {
		return false;
	}
	else {return true;}  
}

//get environemt 
char * getEnvironment(const char *path) 
{
	char *env = NULL; 
	if(strncmp(path, "$PWD", 4) == 0) {env = "PWD";}
	if(strncmp(path, "$HOME", 5) == 0) {env = "HOME";}
	if(strncmp(path, "$SHELL", 6) == 0) {env = "SHELL";}
	if(strncmp(path, "$USER", 5) == 0) {env = "USER";}
	return env; 
}

//returns expanded argument, does nothing in many cases (determined by is_command)
char * expandPath(char *path, int cmd_p, BITFLAGS *f)
{	
	if ( f->Flags.testing == true) {
		printf("Expanding path...\n"); 
	}
	 
	//external commands  
	if(cmd_p == 1) {return path;}
	//cd command
	if(cmd_p == 2) {
		path = getenv("PWD");
		return path; 
	}
	//for other built-ins
	if(cmd_p == 3) {
		path = expandBuiltIn(path, f);
		return path; 
	}
	//for arguments
	if(cmd_p == 0) {
		if(contains(path, '$')) {
			path = envPathAmmend(path, getEnvironment(path), f); 
			if(pathExist(path)) {return path;}
			else {return NULL;}
		}
		if(contains(path, '~')) {
			path = homePathBuilder(path, f);
			if(pathExist(path)) {return path;}
			else{return NULL;}
		}
		if(contains(path, '.')) { 
			//expand parent directory 
			if(path[0] == '.' && path[1] == '.') {
				path = parentDirBuilder(path, f);
				if(pathExist(path)) {return path;}
				else {return NULL;}
			}
			//expand current working directoy
			if(path[0] == '.') {
				path = currentDirPathBuilder(path, f);
				if(pathExist(path)) {return path;}
				else {return NULL;}
			}
		}
		if(contains(path, '/')) { 
			path = buildPath(path, "PWD", f);
			if(pathExist(path)) {return path;}
			else{return NULL;}
		}
		//treat as either file or directory
		else {
			return path; 
		}
	}
	
	return NULL;
}

//returns environment path
char * getPathFromEnv(const char * env)
{
	char path[strlen(getenv(env))];
	char *p = path;
	char *e = getenv(env);
	int i = 0;
	
	for(; i < strlen(getenv(env)); i++) {
		p[i] = e[i];
	}
	path[i] = '\0';
	return p; 
}

bool isRoot(char * path)
{
	char * root = "/";
	if (strcmp(path, root) == 0) {return true;}
	return false;
}

//appends directory to end of path
char * appendPath(char * path, char const * append)
{
	return strcat(strcat(path, "/"), append);
}

//gets value from environmental variable and tests each with passed in path
char * buildPath(char * path, const char * envVar, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("Building Path...\n"); 
	 }
	
	int i = 0;
	char newPath[255];
	char *np = newPath;
	char ** path_split = split(path, '/'); 
	memset(newPath, 0, 255*sizeof(char));
	stpcpy(newPath, getPathFromEnv(envVar));
	
	for(; path_split[i] != NULL; i++) {
		appendPath(newPath, path_split[i]);
		if (!pathExist(newPath)) {return NULL;}			
	}
	return np; 
}

//splits a path into 2D array based on deliminating char
char ** split(const char * path, const char ch)
{
	char **path_split = malloc(sizeof(char* ) * getBucketLength(path, ch)); 
	char temp[255];
	int i = 0;
	int j = 0;
	int k = 0;
	int x;

	for(; i < strlen(path); i++) {
		if(path[i] != ch) {
			temp[j++] = path[i];
		}
		if(path[i] == ch || i+1 == strlen(path)) {
			temp[j] = '\0';
			path_split[k] = malloc( sizeof(char*) * j);
			for(x = 0; x < j; x++) {
				path_split[k][x] = temp[x];
			}
			j = 0;
			k++;
		}
	}
	path_split[k] ='\0';
	return path_split;
}

//determines if char exists in a string 
bool contains(const char *path, const char ch)
{
	int i = 0;
	for(; i < strlen(path); i++) {
		if (path[i] == ch) {return true;}
	}
	return false; 
}

//determines if the file exist 
bool pathExist(const char *path)
{
	if (access(path, F_OK) != -1) {return true;}
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

//execute commmands 
char ** executeCommands(char **cmd, BITFLAGS*f)
{
	printf("Executing...\n");
	
	pid_t pid;
	int status; 
	 
     if ((pid = fork()) == -1)
        perror("fork error");
     else if (pid == 0) {
        execv(cmd[0], cmd);
        printf("Return not expected. Must be an execv error.n\n");
     }
	 else {waitpid(pid, &status, 0);}
	
	return NULL;
}


