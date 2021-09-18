#ifndef PARSER_DOT_H    
#define PARSER_DOT_H    
#define PIPE_BUF 4096

char* token[PIPE_BUF];
char* req[PIPE_BUF];
void parseComArg(int argc,char* argv[]);

void parse(char buf[PIPE_BUF]);

void parseRequest(char buf[PIPE_BUF]);

#endif