#ifndef PARSER_DOT_H    
#define PARSER_DOT_H    
#define PIPE_BUF 4096

// shared among parser, server.
char* token[PIPE_BUF];

// parse function declaration
void  parse(char buf[PIPE_BUF]);

#endif