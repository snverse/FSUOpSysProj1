#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>
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
int     isCommand                   (char **args, int i);
char *  expandPath                  (char *path, int cmd_p, BITFLAGS *f);
char * 	appendPath					(char *path, const char *append);
char * 	getPathFromEnv				(const char *env);
char ** split						(const char *path, const char ch);
char * 	buildPath					(char *path, const char *envVar);
char ** shiftArgs					(char **args);
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
            
            // exit shell
            if (strcmp(command, "exit") == 0) {
                printf("Exiting Shell...\n");
                exit(0);
            }
		
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
		if (f->Flags.testing) {
		    printf("ARGS[%i]: %s\n", i, args[i]);
	    }
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
		strcmp(args[i], "sleep") == 0 ||
		strcmp(args[i], "ls") == 0) {return 3;} 
			
	else {return 0;}
}

//removes ~/ from path
char * homePathBuilder(char * path, BITFLAGS *f)
{
	if ( f->Flags.testing == true) {
		printf("Building Home Path...\n"); 
	}
	
	//if(strlen(path) == 2) {return getenv("HOME");}
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
	
	if(isRoot(currPath)) {printf("ROOT REACHED!\n"); currPath ="/"; return currPath;}
	
	int nlength = getParentIndex(currPath);
	char *parentPath = malloc(sizeof(char*) * (nlength + (strlen(path)-2))); 
	
	strncpy(parentPath, currPath, nlength); 
	strcat(parentPath, path+2);
	
	if(pathExist(parentPath)) {return parentPath;} 
	return NULL; 
}

//removes $KEYWORD in path and replaces with corresponding path 
char * envPathAmmend(char * path, char * envVar, BITFLAGS *f) 
{
	if ( f->Flags.testing == true) {
		printf("ammending envionrmental path...\n"); 
	}
	
	//if(strcmp(envVar, "SHELL") == 0) {return path = "shell"; return path;}
	
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

		//printf("p: %s\n", p);
		if(isPath2BuiltIn(p)) {return p;}
		/*
		if(pathExist(p)) {return p;}
		if(isFile(p)) {return p;}
		if(isDir(p)) {return p;}
		*/
	}
	
	return NULL;
}

//checks if path leads to executable
bool isPath2BuiltIn(const char * path)
{
	/*
	struct stat sb; 
	if(stat(path, &sb) == 0 && sb.st_mode & S_IXUSR) {return true;}
	else {return false;}
	*/
	
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
			path = buildPath(path, "PWD");
			printf("After buildPath: %s\n", path); 
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
	char envPath[strlen(getenv(env))+1];
	char *eP = envPath;
	char *e = getenv(env);
	int i = 0;
	
	for(; i < strlen(getenv(env)); i++) {
		envPath[i] = e[i];
	}
	envPath[i] = '\0';	
	
	//printf("getPathFromEnv: %s\n", eP); 
	return eP; 
}

bool isRoot(char * path)
{
	char * root1 = "/home";
	char * root2 = "/"; 
	if (strcmp(path, root1) == 0 ||
		strcmp(path,root2) == 0) {return true;}
	return false;
}

//appends directory to end of path
char * appendPath(char * path, char const * append)
{
	return strcat(strcat(path, "/"), append);
}

//gets value from environmental variable and tests each with passed in path
char * buildPath(char * path, const char * envVar)
{	
	char * currPath = getenv(envVar);
	int length = strlen(envVar) + strlen(path) + 1; 
	char * newPath = malloc(sizeof(char*) * length);
	
	strcpy(newPath, currPath);
	
	//printf("newPath1: %s\n", newPath);
	strcat(newPath, "/");
	//printf("newPath2: %s\n", newPath);
	strcat(newPath, path);
	//printf("newPath3: %s\n", newPath); 
	
	//newPath[size-1] = '\0';
	//printf("finalPath: %s\n", np);
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

int cd_lsh(char  **args) 
{
	printf("changing to directory: %s\n", args[1]); 
	
	if(args[1] == NULL) {
		printf("NULL terminal running\n");
		//printf("defaultPath: %s\n", defaultPath); 
		chdir(getenv("HOME")); 
		setenv("PWD", getenv("HOME"), 1);
		return 0;
	}
	
	//if arg is single word, append current directory 
	if(!contains(args[1], '/')) {
		printf("!contains terminal running\n");
		args[1] = buildPath(args[1], "PWD");
	}
	
	printf("args[1]: %s\n", args[1]); 
	chdir(args[1]); 
	setenv("PWD", args[1], 1);
	
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

int isBuiltIn(char * command)
{
	int i = 0;
	for(; i < getNumBuiltIns(); i++) {
		if(strcmp(command, builtin_cmd[i])) {return i;}
	}
	//printf("BuiltIn Index: %i\n", i); 
	return -1;
}

//execute commmands 
char ** executeCommands(char **cmd, BITFLAGS*f)
{
    if (f->Flags.testing){
	    printf("Executing...\n");
	}
	
	pid_t pid;
	int status; 
	int i = 0;
	char * c = cmd[0];
	struct timeval start;

	
	/*if(isBuiltIn(cmd[0]) >= 0) {
		//exit the shell
		if(strncmp(cmd[0], "exit", 4) == 0) {
			(*builtin_func1[i])(cmd); 
			return NULL;
		}
		//change directory 
		if(strncmp(cmd[0], "cd", 2) == 0) {
			(*builtin_func2[i])(cmd); 
			return NULL;
		}
		//manupulate cmd** and start timer 
		if(strncmp(cmd[0], "etime", 5) == 0) {
			(*builtin_func3[i])(cmd, 1, start); 
			gettimeofday(&start, NULL);
		}
		//io
		if(strncmp(cmd[0], "io", 2) == 0) {
			//printf("IO detected\n");
			(*builtin_func4[i])(cmd, 0); 
		}
		
	}*/
	
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
