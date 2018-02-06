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
char *  specialCharWhiteSpaceAdjust (char* line);
char *  parseWhitespace             (char *line);
int     getBucketLength             (const char *line, const char ch); 
char ** parseCommand                (char *line); 
char ** parseArguments              (char *line);
char ** resolvePaths                (char **args);
char ** executeArguments            (char **args);
int     isCommand                   (char **args, int i);
char *  expandPath                  (char *path, int cmd_p);
char * 	appendPath					(char *path, const char *append);
char * 	getPathFromEnv				(const char *env);
char ** split						(const char *path, const char ch);
char * 	buildPath					(char *path, const char *envVar);
bool	contains 					(const char *path, const char ch);
bool 	pathExist					(const char *path);
bool 	isFile						(const char *path);
bool 	isDir						(const char *path);


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
char ** parseCommand(char *line) 
{
	char **args = (char**) malloc(sizeof(char**) * getBucketLength(line, ' '));
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
char ** parseArguments(char *line)
{
	/*printf("parse_arguments: %s\n", line);*/
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

/* resolve path conflicts */
char ** resolvePaths(char **args)
{
	int i = 0;
	for(; args[i] != NULL; i++) {
		args[i] = expandPath(args[i], isCommand(args, i)); 
		printf("ARGS[%i]: %s\n", i, args[i]);
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
		strcmp(args[i], "ls") == 0) {printf("built-in command\n");return 3;} 
			
	else {printf("argument\n");return 0;}
}

//removes ~/ from path
char * homePathBuilder(char * path)
{
	char expandedHomePath[255]; 
	char * ehp = expandedHomePath;
	
	char newPath[strlen(path)-2]; 
	char *np = newPath; 

	strcpy(np, path+2);

	printf("homePathBuilder path: %s\n", np);
	char * e = getPathFromEnv("HOME");
	printf("getPathFromEnv(HOME): %s\n", e); 
	
	int i = 0;
	for(; i < strlen(e); i++) {
		expandedHomePath[i] = e[i];
	}
	expandedHomePath[i] = '\0';
	strcat(ehp, "/");
	strcat(ehp, np);
	
	printf("EHP: %s\n", ehp);
	
	return ehp;
}

//removes ./ from line 
char * currentDirPathBuilder(char * path)
{
	char newPath[strlen(path)-2];
	char * np = newPath;
	strcpy(np, path+2); 
	printf("currentDirPathBuilder: %s\n", np); 
	newPath[strlen(path)-2] = '\0';
	return np;
}

//determines index of last "/" in path
int getParentIndex(const char * path)
{
	int index = -1; 
	int i = 0;
	
	for(; i < strlen(path); i++) {
		if(path[i] == '/') {index = i;}
	}
	
	return index;
}

//builds parent path 
char * parentDirBuilder(char * path)
{
	char *currPath = getenv("PWD");
	int nlength = getParentIndex(currPath);
	printf("currPath: %s\n", currPath);
	char newPath[strlen(path)-1];
	char * np = newPath; 
	strcpy(np, path+2); 
	newPath[strlen(newPath)] = '\0';
	printf("curr newPath: %s\n", np);
	char parentPath[nlength + strlen(newPath)+1]; 
	char *pp = parentPath; 
	strncpy(pp, currPath, nlength);
	strncat(pp, np, strlen(newPath)); 
	printf("pp: %s\n", pp);
	parentPath[strlen(parentPath)] = '\0'; 
	return pp; 
	/*
	char parentPath[255];
	char * pp = parentPath;

	int i = 0;
	int j = 0;
	for(; i < nlength; i++) {
		parentPath[j++] = currPath[i];
	}		
	for(i = 2; i < strlen(path); i++) {
		parentPath[j++] = path[i];
	}
	
	parentPath[j] = '\0';
	path = pp;
	printf("ParentPath: %s\n", path); 
	*/
	return path; 
}




//removes $KEYWORD in path and replaces with corresponding path 
char * envPathAmmend(char * path, const char * envVar) 
{
	int i= strlen(envVar)+1; 
	int j = 0;
	char envPath[strlen(path)-i+1];
	char * eP = envPath;
	
	for(; i < strlen(path); i++) {envPath[j++] = path[i];}
	
	envPath[j] = '\0';
	/*printf("envPath: %s\n", eP);*/ 
	return eP; 
}

//returns expanded argument, does nothing in many cases (determined by is_command)
char * expandPath(char *path, int cmd_p)
{	
	//external commands  
	if(cmd_p == 1) {printf("expandPath: %s\n", path);}
	//cd command
	if(cmd_p == 2) {
		path = getenv("PWD");
		printf("expandPath: %s\n", path);
	}
	//for other built-ins
	if(cmd_p == 3) {printf("expandPath: %s\n", path);}
	//for arguments
	if(cmd_p == 0) {
		
		printf("Building path...\n");
		char *envVar; 
		if(contains(path, '$')) {
			printf("$ detected\n");
			printf("befor envPathAmmend: %s\n", path); 
			if(strncmp(path, "$PWD", 4) == 0) {	
				path = envPathAmmend(path, "$PWD");
				envVar = "PWD";
			}
			if(strncmp(path, "$HOME", 5) == 0) {
				path = envPathAmmend(path, "$HOME");
				envVar = "HOME";
			}
			if(strncmp(path, "$SHELL", 6) == 0) {
				path = envPathAmmend(path, "$SHELL");
				envVar = "SHELL"; 
			}
			
			printf("envPathAmmend: %s\n", path);
			path = buildPath(path, envVar); 
			printf("BUILDED PATH: %s\n", path); 
			
			if(pathExist(path)) {
				printf("WE DID IT\n");
				return path;
			}
			printf("freshPath: %s\n", path);
		}
		if(contains(path, '~')) {
			printf("path: %s\n", path);
			path = homePathBuilder(path);
			printf("~path: %s\n", path);
			if(pathExist(path)) {printf("This ~ path exists\n"); return path;}
		}
		if(contains(path, '.')) { 
			if(path[0] == '.' && path[1] == '.') {
				printf("PATH BEFORE: %s\n", path);
				char * b = parentDirBuilder(path);
	
				path = b;
				printf("parentDir: %s\n", b); 
				//printf("DOT DOT PATH: %s\n", b);
				return path;
				
			
			}
			//expand current working directoy
			else {
				char * c = currentDirPathBuilder(path);
				path = c;
				//printf("DOT PATH: %s\n", path);
			}
		}
		if(contains(path, '/')) {
		
			//printf("Entering buildPath function...\n"); 
			path = buildPath(path, "PWD");
			
			if(pathExist(path)) {
				printf("This other path exists\n");
				if(isFile(path)) {printf("This is another valid file\n");}
				if(isDir(path)) {printf("This is another valid directory\n");}
			}
			else{
				printf("other path does not exist\n");
			}
		}
		//treat as either file or directory
		else {
			
			char *p = malloc(sizeof(char* ) * strlen(getenv("PWD")));
			p = getPathFromEnv("PWD");
			
			path = appendPath(p, path);
			
			if(pathExist(path)) {
				printf("This path exists\n");
				if(isFile(path)) {printf("This is a valid file\n");}
				if(isDir(path)) {printf("This is a valid directory\n");}
			}
			else {
				printf("path does not exist\n");
			}
		}
	}
	
	 
	return path;
}


char * getPathFromEnv(const char * env)
{
	char *p = malloc(sizeof(char* ) * strlen(getenv(env)));
	char *e = getenv(env);
	int i = 0;
	
	for(; i < strlen(getenv(env)); i++) {
		p[i] = e[i];
	}
	return p; 
}

bool isRoot(char * path)
{
	char * root = "/";
	if (strcmp(path, root) == 0) {return true;}
	return false;
}

//gets value in $HOME and attaches it to passed in path 
char * expandHome(char * path)
{
	char expandedPath[255]; 
	return strcat(strcat(expandedPath, getenv("HOME")), path);
}

//appends directory to end of path
char * appendPath(char * path, char const * append)
{
	//printf("path of append: %s\n", path);
	//printf("append: %s\n", append);
	return strcat(strcat(path, "/"), append);
}

//gets value from environmental variable and tests each with passed in path
char * buildPath(char * path, const char * envVar)
{
	//printf("Inside buildPath....\n");
	int i = 0;
	char newPath[255];
	char *np = newPath;
	
	if(strcmp(envVar, "NAH") != 0) {stpcpy(newPath, getPathFromEnv(envVar));}
	
	int envPathSize = strlen(getPathFromEnv(envVar));
	
	char ** path_split = split(path, '/'); 
	
	
	for(; path_split[i] != NULL; i++) {
		/*
		printf("newPath before: %s\n", newPath); 
		printf("path_split[%i]: %s\n", i, path_split[i]);
		*/
		appendPath(newPath, path_split[i]);
		
		envPathSize += strlen(path_split[i]); 
		//printf("newPath after: %s\n", newPath); 
		if (!pathExist(newPath)) {/*printf("Path does not exist\n");*/ return NULL;}			
	}
	printf("SIZE: %i\n", strlen(newPath) + getBucketLength(path, '/'));
	//newPath[envPathSize] = '\0';
	printf("Completed building path...\n");
	printf("NEW PATH: %s\n", np);
	return np; 
}

//splits a path into 2D array based on deliminating char
char ** split(const char * path, const char ch)
{
	//printf("Entering split...\n");
	char **path_split = malloc(sizeof(char* ) * getBucketLength(path, ch)); 
	char temp[255];
	int i = 0;
	int j = 0;
	int k = 0;
	int x;

	//printf("path: %s\n", path);
	for(; i < strlen(path); i++) {
		if(path[i] != ch) {
			temp[j++] = path[i];
		}
		if(path[i] == ch || i+1 == strlen(path)) {
			temp[j] = '\0';
			path_split[k] = malloc( sizeof(char*) * j);
			for(x = 0; x < j; x++) {
				path_split[k][x] = temp[x];
				//printf("path_split[%i]: %s\n", k, path_split[k]);
			}
			j = 0;
			k++;
		}
	}
	//printf("returnting split path...\n");
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

//loop through args array and execute commands
char ** executeArguments(char **args)
{
	//more goods
	return NULL;
}


