#include <stdio.h>
#include <string.h>
#include <stdbool.h> 
#include "utils.h"

void my_setup() {}
void my_prompt() {}

char *my_read() {
	char *str[255];
	char *str_trim; 
	
	fgets(str, sizeof str, stdin);
	str_trim = trimWhiteSpace(str);

	if (strlen(str_trim) == 0) {return NULL;}
	else {return str_trim;}
}

char **my_parse(char *line) {
	char **args = malloc(bucket_length(line));
	
	line = parse_whitespace(line);
	printf("inside **my_parse: %s\n", line); 
	args = parse_arguments(line);
	
	
	/*
	args = expand_variables(args);
	args = resolve_paths(args);
	return cmd;
	*/
	return NULL;
}

void my_execute(char **cmd) {
	/*
	int i = 0;
	
	/* empty command 
	if(cmd[0] == NULL) {
		return 1;
	}
	
	for(; i < NUM_BUILT_INS(); i++) {
		/* executing a built in function 
		if(strcmp(cmd[0], BUILT_INS[i]) == 0) {
		}
		
		if(isSpecialChar(cmd[0]) {
		}
		
	}	
	*/
}

void my_clean() {}

int main() {
	char *line;
	char **cmd;
	
	while(1) {
		/*
		my_setup();
		my_prompt();
		*/
		printf("$ ");
		line = my_read();
		cmd = my_parse(line);
		/*
		my_execute(cmd)
		my_clean();
		*/
	}
	return 0; 
}
