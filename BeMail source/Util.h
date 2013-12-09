#ifndef UTIL_H
#define UTIL_H

#include <OS.h>
#include <TextView.h>

//===========================================================
// These are some simple 'C' string management classes.
//
//
// cistrncmp	case-insensitive version of strncmp
//
// cistrstr		case-insensitive version of strstr
//
// extract		Un-fold field and add items to dst
//
// linelen		return length of \n terminated line
//
//===========================================================

int		cistrncmp(const char*, const char*, const int);
char*	cistrstr(const char*, const char*);
void	extract(char**, const char*);
int		linelen(const char*, const int);
bool	strip_it(char*, long, bool, BTextView*, sem_id*);

#endif
