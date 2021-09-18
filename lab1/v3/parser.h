#ifndef PARSER_DOT_H    
#define PARSER_DOT_H    
#define PIPE_BUF 4096

char* token[PIPE_BUF];
char* req[PIPE_BUF];

// to parse commandline input
void parse(char buf[PIPE_BUF], char delimiter[2]);

#endif