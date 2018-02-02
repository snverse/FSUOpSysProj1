#include "utils.h"
#include <stdbool.h>

char *SPECIAL_CHAR = "|><&$~";
char *BUILT_INS[] = {"exit", "cd", "echo", "etime", "io"};



int NUM_BUILT_INS() 
{
	return sizeof(BUILT_INS) / sizeof(char*); 
}

/* handles leading and trailing white space */
char *trimWhiteSpace(char *str)
{
	char *end;
	
	while(isspace((unsigned char)*str)) str++;
	
	if(*str == 0)
		return str;
	
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;
	
	*(end+1) = 0;
	
	return str;
}

/* determines if char is special */
bool isSpecialChar(char ch)
{	
	int i = 0;
	for(; i < strlen(SPECIAL_CHAR); i++) {
		if (ch == SPECIAL_CHAR[i]) {return true;}
	}
	return false;
}

/* adds space to special chars that need it*/
char **special_char_scan(char* line)
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
	return trimWhiteSpace(str);
}	

/* handles interior white space and special character spacing*/ 
char **parse_whitespace(char* line)
{
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
	return special_char_scan(str); 
}

int get_space_count(char *line)
{
	int i = 0;
	int counter = 0;
	
	for(; line[i] != '\0'; i++) {
		if (line[i] == ' ') {
			counter++;
		}
	}
	return counter; 
}

int bucket_length(char *line)
{
	return get_space_count(line) * 2;
}

char **parse_arguments(char *line)
{	
	printf("parse_arguments: %s\n", line);
	
	char **bucket = malloc(bucket_length(line)); 
	
	char temp[255];
	char *temp2[255];
	
	int i = 0;
	int j = 0;
	int k = 0;
	
	/* traverse input string */ 
	for(; i < strlen(line); i++) {
		
		/* special character */ 
		if(isSpecialChar(line[i])) {
			bucket[k] = line[i];
			printf("bucket[%i]: %c\n", k, bucket[k]);
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
				bucket[k] = temp;
				printf("bucket[%i]: %s\n", k, bucket[k]); 
				memset(temp, 0, sizeof(temp));
				j = 0;
				k++;
			}
		}
		
	}
	
	int g = 0;
	int size = bucket_length(line);
	int p = 0;
	
	/*
	for(; g < size; g++) {
		char *pos = bucket[g];
		printf("POS: %*s \n", &pos); 
		for(; p < strlen(pos); p++) {
			printf("pos[%i]: %c", p, pos[i]);
		}
		/*
		while(*pos != '\0') {
			printf("%s\n", &pos); 
		}
		
	}
*/
	/*free(bucket);*/
	return bucket;
}