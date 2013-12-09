//--------------------------------------------------------------------
//	
//	Util.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1996 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#include "Util.h"


//==========================================================
// Function: cistrncmp
// case-insensitive version of strncmp
//
//==========================================================
int		
cistrncmp(const char *str1, const char *str2, const int max)
{
	char	c1;
	char	c2;
	int		len;
	int		loop;

	len = min(strlen(str1) + 1, strlen(str2) + 1);
	len = min(len, max);
	for (loop = 0; loop < len; loop++) {
		c1 = *str1++;
		if ((c1 >= 'A') && (c1 <= 'Z'))
			c1 += ('a' - 'A');
		c2 = *str2++;
		if ((c2 >= 'A') && (c2 <= 'Z'))
			c2 += ('a' - 'A');
		if (c1 == c2) {
		}
		else if (c1 < c2)
			return -1;
		else if ((c1 > c2) || (!c2))
			return 1;
	}
	return 0;
}

//==========================================================
// Function: cistrstr
// case-insensitive version of strstr
//
//==========================================================
char*	 
cistrstr(const char *cs, const char *ct)
{
	char	c1;
	char	c2;
	int		cs_len;
	int		ct_len;
	int		loop1;
	int		loop2;

	cs_len = strlen(cs);
	ct_len = strlen(ct);
	for (loop1 = 0; loop1 < cs_len; loop1++) {
		if (cs_len - loop1 < ct_len)
			goto done;
		for (loop2 = 0; loop2 < ct_len; loop2++) {
			c1 = cs[loop1 + loop2];
			if ((c1 >= 'A') && (c1 <= 'Z'))
				c1 += ('a' - 'A');
			c2 = ct[loop2];
			if ((c2 >= 'A') && (c2 <= 'Z'))
				c2 += ('a' - 'A');
			if (c1 != c2)
				goto next;
		}
		return((char *)&cs[loop1]);
next:;
	}
done:;
	return(NULL);
}

//==========================================================
// Function: extract
//	Un-fold field and add items to dst
//
//==========================================================
void	
extract(char **dst, const char *src)
{
	bool	remove_ws = TRUE;
	int		comma = 0;
	int		count = 0;
	int		index = 0;
	int		len;

	if (strlen(*dst))
		comma = 2;

	for (;;) {
		if (src[index] == '\r') {
			if (count) {
				len = strlen(*dst);
				*dst = (char *)realloc(*dst, len + count + comma + 1);
				if (comma) {
					(*dst)[len++] = ',';
					(*dst)[len++] = ' ';
					comma = 0;
				}
				memcpy(&((*dst)[len]), &src[index - count], count);
				(*dst)[len + count] = 0;
				count = 0;

				if (src[index + 1] == '\n')
					index++;
				if ((src[index + 1] != ' ') && (src[index + 1] != '\t'))
					break;
			}
		}
		else {
			if ((remove_ws) && ((src[index] == ' ') || (src[index] == '\t'))) {
			}
			else {
				remove_ws = FALSE;
				count++;
			}
		}
		index++;
	}
}

//==========================================================
// Function: linelen
//
// Purpose: 
//	return length of \n terminated line
//
//==========================================================
int		
linelen(const char *str, const int len)
{
	int		loop;

	for (loop = 0; loop < len; loop++) {
		if (str[loop] == '\n')
			return loop + 1;
	}
	return len;
}


