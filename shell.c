#include <stdio.h>
#include <string.h>

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
    
    while(true) {
        command = ""; // clear old command
        printf("%s@%s :: %s =>", user, machine, path);
        scanf("%79s", command);
        if ( f->Flags.testing == true) {
            printf("\n\n%s\n\n", command);
        }
        
        // exit shell
        if (strcmp(command, "exit") == 0) {
            printf("Exiting Shell...\n");
            break;
        }
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
