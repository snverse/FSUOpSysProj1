#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>

// true and false
#define true 1
#define false 0
// size of buffer for commands
#define CHARLENGTH 255

// define union for flags
typedef union
{
    unsigned int bitFlags : 32;
    
    struct {
       unsigned int testing    : 1;
       unsigned int verbose    : 1;
       unsigned int pipeIn     : 1;
       unsigned int pipeOut    : 1;
       unsigned int bg         : 1;
       unsigned int unassigned : 27; // subtract when adding new flag
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
int     isCommand                   (char **args, int i, BITFLAGS *f);
char *  expandPath                  (char *path, int cmd_p, BITFLAGS *f);
char ** split						(const char *path, const char ch);
char * 	buildPath					(char *path, const char *envVar);
char ** shiftArgs					(char **args);
char *	expandBuiltIn				(char * path, BITFLAGS *f);
bool	contains 					(const char *path, const char ch);
bool 	pathExist					(const char *path);
bool 	isFile						(const char *path);
bool 	isDir						(const char *path);
bool 	isPath2BuiltIn				(const char *path);
bool	isRoot						(char * path);
void    storeCommands               (char *line, char* saved, BITFLAGS* f);
char *  getCommands                 (char* saved, BITFLAGS *f);
void    flipPipe                    (BITFLAGS *f);

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
    char *command = (char*) malloc(CHARLENGTH * sizeof(char));
    char *saved = (char*) malloc(CHARLENGTH * sizeof(char));
    int i = 0;
	
    while(true) {
		user = getenv("USER");
		machine = getenv("MACHINE");
		path = getenv("PWD");
		
		strcpy(command, ""); // clear old command
        printf("%s@%s :: %s =>", user, machine, path);
        
        scanf(" %[^\n]", command);       
	    i = 0;
	
        // remove newline
        for (i; i < CHARLENGTH; i++) {
            if (command[i] == '\n') {
                command[i] = '\0';
                break;
            }
        }
       
        storeCommands(command, saved, f); // move all commands into parsed list
        
        while(saved[0] != '\0') {
            if (f->Flags.testing) {
                printf("entering reactor loop loop\n");
            }
            
            // free command and reassign
            free(command);
            command = getCommands(saved, f); //get next command
            
            // print flags
            if (f->Flags.testing) {
                printf("--- FLAGS ---\n");
                printf("\tpipeIn: %d\n", f->Flags.pipeIn);
                printf("\tpipeOut: %d\n", f->Flags.pipeOut);
                printf("\tbg: %d\n", f->Flags.bg);
                printf("-------------\n");
            }
            
			/*
            // exit shell
            if (strcmp(command, "exit") == 0) {
                printf("Exiting Shell...\n");
                exit(0);
            }
		*/
		
		
		    // parse the command 
		    parseCommand(command, f);
		}
    }
    
    free(command);
    free(saved);
    
    return 0;
}

void storeCommands (char *line, char* saved, BITFLAGS* f) {
    int i = 0;
    
    for ( i; i < CHARLENGTH; i++) {
        if(line[i] == '\n') {
            saved[i] = '\0';
            break;
        }
        saved[i] = line[i]; //copy user input
    }
    
    if(f->Flags.testing) {
        printf("storeCommands:\n\t%s\n", saved);
    }
    
    return;
}

char * getCommands (char* saved, BITFLAGS *f) {
    char *temp = (char*) malloc(CHARLENGTH * sizeof(char));
    int i = 0, j = 0;
    
    if (saved[0] == '\0') {
        temp[0] = '\0';
        
        return temp;
    }
    
    // copy till ; or |
    for ( i = 0; i < CHARLENGTH; i++) {
        if (saved[i] == ';') {
            i++;
            break;
        }
        
        if (saved[i] == '|') {
            i++;
            f->Flags.pipeIn = true;
            break;
        }
        
        if (saved[i] == '\0') {
            break;
        }
        
        temp[j++] = saved[i];
    }
    
    // add null terminator to temp
    temp[j] = '\0';
    
    //check for bg
    if (strstr(temp, " &\0")) {
        f->Flags.bg = true;
        temp[--j] = '\0';
    }
    
    // reset j, increment i
    j = 0;
    
    // copy saved over saved till null terminator
    for ( i; i < CHARLENGTH; i++) {
        if (saved[i] == '\0') {
            saved[j] = '\0';
            break;
        }
        
        saved[j++] = saved[i];
    }
    
    // testing output
    if (f->Flags.testing) {
        printf("getCommands:\n");
        printf("\tNext command: %s\n", temp);
        printf("\tRemaining commands: %s\n", saved);
    }
    
    return temp;
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

int argSize(char ** args)
{
	int i = 0;
	for(; args[i] != NULL; i++) {}
	return i; 
}


bool validCommand(char ** args)
{
	int i = 0;
	int j;
	int size = argSize(args); 
	//iterate through args
	for(; args[i] != NULL; i++) {
		//io redirection
		if(contains(args[i], '>') && size <= 2) {
			if(args[i][0] == '>') {return false;}
			if(args[i][1] != '>') {return false;}
		}
		else if(contains(args[i], '<') && size <= 2) {
			if(args[i][0] == '<') {return false;}
			if(args[i][1] != '<') {return false;}
		}
		else if(contains(args[i], '|') && size <= 2) {
			if(args[i][0] == '|') {return false;}
			if(args[i+1] == NULL && args[i][0] == '|') {return false;}
		}
		else if(contains(args[i], '&')) {
			for(j = 0; j < strlen(args[i]); j++) {
				if(args[i][j] == '&' && args[i][j+1] == '|') {return false;}
				if(args[i][j] == '>' && args[i][j+1] == '&') {return false;}
				if(args[i][j] == '<' && args[i][j+1] == '&') {return false;}
			}
		}
	}
	return true;
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
	
	if(validCommand(args)) {
		args = resolvePaths(args, f);
		executeCommands(args, f);
		free(args); 
	return args; 
	}
	
	else {
		printf("Command not Valid\n");
	}
	return NULL;
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
		args[i] = expandPath(args[i], isCommand(args, i, f), f); 
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
int isCommand(char **args, int i, BITFLAGS *f) 
{
	char * c = expandBuiltIn(args[i], f);
	
	if(strcmp(args[i], "cd") == 0) {return 2;}
	
	if(c != NULL && !isSpecialChar(args[i][0]) && args[i][0] != '.' && args[i][0] != '~') {
		return 3;
	}
	
	if(*args[i] == '>' || 
		*args[i] == '<' ||
		*args[i] == '&' ||
		*args[i] == '|') {return 1;}
	
	else {return 0;}
}

//removes ~/ from path
char * homePathBuilder(char * path, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("Building Home Path...\n"); 
	}
	
	char * home = getenv("HOME");
	int length = strlen(home) + strlen(path); 
	char *newPath = malloc(sizeof(char*) * length); 
	
	strcpy(newPath, home);
	if(strlen(path) == 1 || strlen(path) > 2) {strcat(newPath, path+1);}
	else{strcat(newPath, path+2);}
	
	if(pathExist(newPath)) {return newPath;} 	
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
	int length = strlen(currPath) + strlen(path); 
	char *newPath = malloc(sizeof(char*) * length);
	
	strcpy(newPath, currPath);
	strcat(newPath, path+1); 
	
	if(pathExist(newPath)) {return newPath;}
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
	
	if(isRoot(currPath)) {printf("Error: parent is root directory\n"); currPath ="/"; return currPath;}
	
	int nlength = getParentIndex(currPath);
	char *parentPath = malloc(sizeof(char*) * (nlength + (strlen(path)-2))); 
	
	strncpy(parentPath, currPath, nlength); 
	strcat(parentPath, path+2);
	
	if(pathExist(parentPath)) {return parentPath;} 
	return NULL; 
}

//returns name of current user
char * getUser()
{
	char *home = getenv("HOME");
	int offset = getParentIndex(home) + 1;
	
	char user[20];
	char *u = user; 
	 
	strcpy(u, home+offset);
	return u; 
}


//removes $KEYWORD in path and replaces with corresponding path 
char * envPathAmmend(char * path, char * envVar, BITFLAGS *f) 
{
	if ( f->Flags.testing == true) {
		printf("ammending envionrmental path...\n"); 
	}
	
	if(strcmp(envVar, "NAH") == 0) {printf("Input error. Invalid Environemntal Variable\n"); return NULL;}
	
	char *currPath = getenv(envVar); 
	int keyword = strlen(envVar) + 1; 
	int length = strlen(currPath) + (strlen(path)-keyword);
	char *newPath = malloc(sizeof(char*) * length); 
	
	strcpy(newPath, currPath);
	strcat(newPath, path+keyword);
	
	if(pathExist(newPath)) {return newPath;}
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
	else if(strncmp(path, "$HOME", 5) == 0) {env = "HOME";}
	else if(strncmp(path, "$SHELL", 6) == 0) {env = "SHELL";}
	else if(strncmp(path, "$USER", 5) == 0) {env = "USER";}
	else {env = "NAH";}
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
		//path = getenv("HOME");
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
			if(strcmp(path, "$USER") == 0) {char *q = getUser(); return q; }
			path = envPathAmmend(path, getEnvironment(path), f); 
			if(pathExist(path)) {return path;}
			else {printf("path does not exists\n"); return NULL;}
		}
		if(contains(path, '~')) {
			path = homePathBuilder(path, f);
			if(pathExist(path)) {return path;}
			else{printf("path does not exists\n"); return NULL;}
		}
		if(contains(path, '.')) { 
			//expand parent directory 
			if(path[0] == '.' && path[1] == '.') {
				path = parentDirBuilder(path, f);
				if(pathExist(path)) {return path;}
				else {printf("path does not exists\n"); return NULL;}
			}
			//expand current working directoy
			if(path[0] == '.') {
				path = currentDirPathBuilder(path, f);
				if(pathExist(path)) {return path;}
				else {printf("path does not exists\n"); return NULL;}
			}
		}
		if(contains(path, '/')) { 
			path = buildPath(path, "PWD");
			printf("After buildPath: %s\n", path); 
			if(pathExist(path)) {return path;}
			else{printf("path does not exists\n"); return NULL;}
		}
		//treat as either file or directory
		else {
			return path; 
		}
	}
	
	return NULL;
}

//determines if path leads to root 
bool isRoot(char * path)
{
	char * root1 = "/home";
	char * root2 = "/"; 
	if (strcmp(path, root1) == 0 ||
		strcmp(path,root2) == 0) {return true;}
	return false;
}

//gets value from environmental variable and tests each with passed in path
char * buildPath(char * path, const char * envVar)
{	
	char * currPath = getenv(envVar);
	int length = strlen(envVar) + strlen(path) + 1; 
	char * newPath = malloc(sizeof(char*) * length);
	
	strcpy(newPath, currPath);
	strcat(newPath, "/");
	strcat(newPath, path);
	
	if(pathExist(newPath)) {return newPath;}
	return NULL;
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

//list of builtin commands and corresponding functions
char *builtin_cmd[] = {
	"cd",
	"etime",
	"exit",
	"io"
};

int cd_lsh(char **args) 
{
	//printf("changing to directory: %s\n", args[1]); 
	
	if(args[2] != NULL) {
		printf("Error: too many arguments\n");
		return 2;
	}
	
	if(args[1] == NULL) {
		chdir(getenv("HOME")); 
		setenv("PWD", getenv("HOME"), 1);
		return 0;
	}
	
	//if arg is single word, append current directory 
	if(!contains(args[1], '/')) {
		//printf("!contains terminal running\n");
		args[1] = buildPath(args[1], "PWD");
	}
	
	/*
	if(args[1][0] == '>' || args[1][0] == '<' 
		|| args[1][0] == '&' || args[1][0] == '|') {
			printf("Error: changing to invalid directory\n");
			return 1;
		}
	*/
	if(pathExist(args[1]) && isDir(args[1])) {
		//printf("args[1]: %s\n", args[1]); 
		chdir(args[1]); 
		setenv("PWD", args[1], 1);
	}
	else{printf("Error: changing to invalid directory\n");}
	return 1; 
}

/*
facilitate etime function
*/
int etime_lsh(char ** args, int choice, struct timeval init_t)
{	
	double time; 
	int i = 1; 
	int j = 0; 
	struct timeval t; 

	//start 
	if(choice == 1) {
		//shift contents of args
		args = shiftArgs(args);
		return 1; 
	}
	
	//end 
	if(choice == 0) {
		gettimeofday(&t, NULL);
		time = (t.tv_sec - init_t.tv_sec) +
				((t.tv_usec - init_t.tv_usec)/1000000.0); 
		
		printf("Elapsed Time: %.6fs\n", time); 
		return 0; 
	}
}

//shift Args by 1
char **shiftArgs(char ** args)
{
	int i = 1;
	int j = 0; 
	
	//shift contents of args
	for(; args[i] != NULL; i++) {
		args[j] = args[i];
		//printf("io args[%i]: %s\n", j, args[j]);
		j++;
	}
	args[j] = NULL;
	return args;
}

//facilitate io 
int io_lsh(char **args, int flag)
{
	if(flag == 0) {args = shiftArgs(args);}
	if(flag == 1) {
		char newPath[255];
		memset(newPath, 0, 255);
		char * np = newPath; 
		char * proc = "/proc/";
		char p[10]; 
		
		snprintf(p, 10, "%d", ((int)getpid()));
		strcat(np, proc);
		strcat(np, p); 
		strcat(np, "/io");
			
		char buffer[255];
		FILE *fp = fopen(np, "r");
		while(fgets(buffer, 255, (FILE*) fp)) {
			printf("%s", buffer);
		}

		fclose(fp);
	}
	return 1;
}

//exit function 
int exit_lsh(char **args)
{
	printf("Exiting Shell...\n"); 
	exit(1); 
}

int (*builtin_func1[]) (char **) = {	
	&exit_lsh
};

int (*builtin_func2[]) (char **) = {
	&cd_lsh
};

int (*builtin_func3[]) (char **, int, struct timeval) = {
	&etime_lsh
};

int (*builtin_func4[]) (char **, int) = {
	&io_lsh
};

int getNumBuiltIns()
{
	return sizeof(builtin_cmd) / sizeof(char*); 
}

//determins if a commands is a builtin 
int isBuiltIn(char * command)
{
	int i = 0;
	for(; i < getNumBuiltIns(); i++) {
		if(strcmp(command, builtin_cmd[i])) {return i;}
	}
	//printf("BuiltIn Index: %i\n", i); 
	return -1;
}

//copies arg to an index and nulls the rest of the elements
char ** argsCopy(char ** args, int index)
{

	int j = 0; 
	int i = 0;
	int flag = 0;

	//shift contents of args
	for(; args[i] != NULL; i++) {
		if(args[i] == args[index] || flag == 1)
		{
			args[i] = NULL;
			flag = 1; 
		}
		args[i] = args[i]; 
		//printf("redirct args[%i]: %s\n", i, args[i]);
	}
	return args; 
}

//redirect input 
char ** inputRedirect(char **args, int index)
{
	char ** cmd = argsCopy(args, index);
	return cmd; 
}

//reorganizes cmd and adds buffer
char ** outputRedirect(char **args, char *buf, int index)
{
	char ** cmd = argsCopy(args, index); 
	cmd[1] = buf;	
	return cmd; 
}

//copy file, used for pointer issues 
char * getFile(char * file)
{
	char newFile[strlen(file)];
	char * nf = newFile; 
	int i = 0;
	for(; i < strlen(file); i++){
		newFile[i] = file[i];
	}
	newFile[i] = '\0';
	//printf("newFile: %s\n", nf); 
	return nf; 
}

//determines builtin functions and launches 
int builtinLauncher(char ** cmd, struct timeval start)
{
	int i = 0;
	
	if(isBuiltIn(cmd[0]) >= 0) {
		//exit the shell
		if(strncmp(cmd[0], "exit", 4) == 0) {
			(*builtin_func1[i])(cmd); 
			return -1;
		}
		//change directory 
		else if(strncmp(cmd[0], "cd", 2) == 0) {
			(*builtin_func2[i])(cmd); 
			return -1;
		}
		//manupulate cmd** and start timer 
		else if(strncmp(cmd[0], "etime", 5) == 0) {
			(*builtin_func3[i])(cmd, 1, start); 
			return 1;
		}
		//io
		else if(strncmp(cmd[0], "io", 2) == 0) {
			(*builtin_func4[i])(cmd, 0); 
			return 2;
		}	
	}
	
	return 0;
}

//handles builtins that run over execution
int builtinLander(char * c, char ** cmd, struct timeval start)
{
	int i = 0;
	//stop etime timer and print time lapse 
	if(strncmp(c, "etime", 5) == 0) {
		(*builtin_func3[i])(cmd, 0, start); 
		return 1;
	}	
	//build path to pid/io and read file 
	if(strncmp(c, "io", 2) == 0) {
		(*builtin_func4[i])(cmd, 1); 
		return 2;
	}
	
	return -1; 
}

//big ugly redirect swiss army knife 
//val = 0, returns fd
//val = 1, returns index 
//val = 2, returns flag 
int redirectHelper(char **cmd, int val)
{
	int j = 0;
	char * path2File; 
	int fd; 
	
	for(; cmd[j] != NULL; j++) {
		if(contains(cmd[j], '>')) {
			path2File = cmd[j+1];//getFile(cmd[j+1]);
			printf("path2File: %s\n", path2File); 
			fd = open(path2File, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			if(val == 1) {return j;}
			if(val == 2) {return 1;}
		}
		else if(contains(cmd[j], '<')) {
			path2File = cmd[j+1];	//getFile(cmd[j+1]); 
			fd = open(path2File, O_RDONLY, 0);
			if(val == 1) {return j;}
			if(val == 2) {return 2;}
		}
	}
		
	if(fd == -1) {printf("Error\n"); return -1;}
	else{return fd;}
}	

//removes new line for file buffers
char * newLineRemove(char * buffer)
{
	char newBuffer[255]; 
	char *nb = newBuffer;
	int i = 0;
	int j = 0;
	for(; i < strlen(buffer); i++)
	{
		if(isalpha(buffer[i])) {
			newBuffer[j++] = buffer[i];
		}
	}
	newBuffer[j] = '\0';
	return nb;
}

bool pipeScan(char ** args)
{
	int i = 0;
	int j = 0;
	
	for(; args[i] != NULL; i++) {
		for(j = 0; j < strlen(args[i]); j++) {
			if(args[i][j] == '|') {return true;}
		}
	}		
	return false;
}

int indexOfNextPipe(char ** cmd)
{
	int i =0;
	for(; cmd[i] != NULL; i++) {
		if(cmd[i][0] == '|') {printf("indexOFNextPipe: %i\n", i);return i;}
	}
	return -1; 
}

int getNumPipes(char ** cmd)
{
	int i = 0; 
	for(; cmd[i] != NULL; i++) {
		printf("cmd: %s\n", cmd);
		if(contains(cmd[i],'|')) {i++;}
	}
	//printf("numPipes: %s\n", i);
	return i;
	
}

int getCmdArraySize(char ** cmd)
{
	int i = 0;
	for(; cmd[i] != NULL; i++){}
	return i;
}

char ** pipeCopy(char ** cmd)
{
	char ** copy = (char**) malloc(sizeof(char**) * getCmdArraySize(cmd));
	int x = 0;
	int y = 0;
	int size; 
	
	
	for(; cmd[x] != NULL; x++) {
		size = strlen(cmd[x]);
		copy[x] = malloc(sizeof(char*) * size);
		for(y = 0; y < size; y++) {
			copy[x][y] = cmd[x][y];
		}
		//printf("copy[%i]: %s\n", x, copy[x]);
	}
	return copy;
}

int indexAtPipeItr(char **cmd, int itr)
{
	int i = 0;
	int j = -1;
	for(; cmd[i] != NULL; i++) {
		if(cmd[i][0] == '|') {
			j = j + 1;
			if(j == itr) {return i;}
		}
	}
	
}

char ** nextCommand(char ** pipeBay, int start, int end)
{
	printf("nextCommand called\n");
	char ** nCommand = (char**) malloc(sizeof(char**) * (start - end) + 1);
	int i = start;
	int j = 0;
	int k = 0;
	int size;
	
	printf("start %i\n", i);
	printf("end: %i\n", end);
	for(; i < end; i++) {
		size = strlen(pipeBay[i]);
		nCommand[k++] = malloc(sizeof(char*) * size);
		for(; j < size; j++) {
			nCommand[k][j] = pipeBay[i][j];
			printf("%c\n", nCommand[k][j]);
		}
		//printf("nCommnd[%i]: %s\n", k, nCommand[k]);
	}
	
	return nCommand; 
}

char ** nullify(char ** cmd) 
{
	int i = 0;
	for(; cmd[i] != NULL; i++) {
		cmd[i] = NULL;
	}
	return cmd; 
}

//https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
int spawn_proc (int in, int out, char **cmd)
{
 printf("spawn_proc called\n");
 printf("cmd[0]: %s\n", cmd[0]);

 pid_t pid1;

  if ((pid1 = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execv(cmd[0], cmd);
    }

  return pid1;
}


//execute commmands 
char ** executeCommands(char **cmd, BITFLAGS*f)
{
	printf("Executing...\n");
	
	pid_t pid;
	int status; 
	char * c = cmd[0];
	struct timeval start;
	int redirectFlag = 0; 
	int fd; 
	char * path2File;
	char buffer[255];
	char * b = buffer;
	int j = 0;
	int i = 0; 
	int launch = builtinLauncher(cmd, start);
	int land; 
	int pipeFlag = 0;
	int pipefd[2];
	int in;
	char ** pipeBay = (char**) malloc(sizeof(char**) * getCmdArraySize(cmd));
	int length; //= getNumPipes(cmd);
	
	//etime start  
	if(launch == 1) {gettimeofday(&start, NULL);}
	//if not a builtin
	if(launch  == -1) {return NULL;}
	
	
	
	//check if piping 
		if(pipeScan(cmd)) {
			
			printf("I CONTAIN A PIPE\n");
			int k = 0; 
			pipeBay = pipeCopy(cmd);
			int begin = 0;
			int end = 0;
			int x = 0;
			int index;
			int counter; 
			
			//get number of pipes 
			for(; cmd[x] != NULL; x++) {if(contains(cmd[x],'|')) {length++;}}
			printf("running pipe-loop: %i times\n", length); 
			
			//the first process should get its input from the original file descriptor 
			in = 0;
			
			//note the loop bound, we spawn ere all, but the last stage of the pipeline
			for(; k < length; k++) {
				pipe(pipefd);
				
				// builld current command 
				end = indexAtPipeItr(pipeBay, k); 
			
				cmd = nullify(cmd);
				index = 0; 
				
				//get current cmd 
				for(counter = begin; counter < end; counter++) {
					cmd[index++] = pipeBay[counter];}
				
				begin = end + 1;
				
				//f[1] is the write end of the pipe, we carry 'in' from the prev itr
				spawn_proc(in, pipefd[1], cmd);
				//no need for the write end of the pipe, the child will write here 
				close(pipefd[1]); 
				//keeps the read end of the pipe, the next child will read from there
				in = pipefd[0];
				//printf("nextCommand: %s\n", cmd[0]); 
				//int g = 0;
				//for(; cmd[g] != NULL; g++) {printf("nextCommand: cmd[%i]: %s\n", g, cmd[g]);}
				//printf("Return not expected. Must be an execv error.n\n");
			}		
			//final stage of pipeline -set stdin be the read end of the previous pipe 
			//and output to the original file descriptor 1
			if(in !=0) {dup2(in, 0);}
			
			//create final isntruction 
			index = 0;
			cmd = nullify(cmd);
			for(counter = begin; pipeBay[counter] != NULL; counter++) {
				cmd[index++] = pipeBay[counter];
			}
			printf("final command: %s\n", cmd[0]);
			
			//execute final instruction
			execv(cmd[0], cmd); 
			printf("Return not expected. Must be an execv error.n\n");
			return NULL;
		}
	
	//child 
	else {
    if (pid=fork() == 0) {
		
			
		
		//determine redirect values 
		//if not redirect, helper function 
		//will know and ignore 
		fd = redirectHelper(cmd, 0);
		j = redirectHelper(cmd, 1);
		redirectFlag = redirectHelper(cmd, 2);  
	
		//input redirection
		if(redirectFlag == 1)
		{
			cmd = inputRedirect(cmd, j);
			close(STDIN_FILENO);
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}
		//output redirection 
		else if (redirectFlag == 2)
		{	 
			read(fd, buffer, 255); 
			b = newLineRemove(b);
			cmd = outputRedirect(cmd, b, j);
		}
		//execute commads 
		execv(cmd[0], cmd);
        printf("Return not expected. Must be an execv error.n\n");
    }
	else {
		if(pipeFlag == 0) {//wait for child 
		waitpid(pid, &status, 0);} 
		
		if(redirectFlag == 1) {close(fd);}
		//if(pipeFlag == 1) {
			
			//printf("Pipe flag raised\n");
			//char buffer[255];
			//close the write end of the pipe in the parent
			//close(pipefd[1]); 
			
			//while(read(pipefd[0], buffer, sizeof(buffer)) != 0) {}
			//printf("buffer: %s\n", buffer);
		//}
		
		
		//facilitates closing phase of builtins 
		land = builtinLander(c, cmd, start); 
	}
	return NULL;
	}
}

/*	
	if (f->Flags.bg) {
	    // bg process
	    printf("run process in bg.\n");
	} else {
	    // not bg process
        if ((pid = fork()) == -1) {
    		perror("fork error");
    	}
           
        else if (pid == 0) {
    		execv(cmd[0], cmd);
            printf("Return not expected. Must be an execv error.n\n");
        }
    	else {
    		waitpid(pid, &status, 0);
    		
    		//stop etime timer and print time lapse 
    		if(strncmp(c, "etime", 5) == 0) {
    			(*builtin_func3[i])(cmd, 0, start); 
    		}
    		
    		//build path to pid/io and read file 
    		if(strncmp(c, "io", 2) == 0) {
    			(*builtin_func4[i])(cmd, 1); 
    		}		
        }
    }
	return NULL;
}

void flipPipe(BITFLAGS *f) {
    if(f->Flags.pipeIn) {
        f->Flags.pipeIn = false;
        f->Flags.pipeOut= true;
    }
}
*/
