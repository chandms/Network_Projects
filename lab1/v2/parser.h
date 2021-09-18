#ifndef PARSER_DOT_H    
#define PARSER_DOT_H    
#define PIPE_BUF 4096

char* token[PIPE_BUF];
void parseComArg(int argc,char* argv[]);

void  parse(char buf[100]);

#endif