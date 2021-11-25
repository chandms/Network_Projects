#ifndef ZIGZAGCONF_DOT_H    
#define ZIGZAGCONF_DOT_H

/* zigzagconf.h header file */

void create_the_socket();
void close_the_socket();
void bind_to_the_socket();

void parse_line(char line[100], char delim[2]);

void zig_zag_conf();


#endif
