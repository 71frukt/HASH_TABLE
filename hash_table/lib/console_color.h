#ifndef CONSOLE_COLOR_H
#define CONSOLE_COLOR_H

#define BLACK   "30" 
#define RED     "31"
#define GREEN   "32"
#define YELLOW  "33"
#define BLUE    "34"
#define MAGENTA "35"
#define CYAN    "36"
#define WHITE   "37"

#define COLORED(STR, COLOR) "\x1b[" COLOR "m" STR "\x1b[0m"

#endif