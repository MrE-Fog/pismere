/*
 * util.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Miscellaneous debug printing utilities
 */

#include "mit_copy.h"
#include <stdio.h>
#include "des.h"

des_cblock_print_file(x, fp)
    des_cblock *x;
    FILE *fp;
{
    unsigned char *y = (unsigned char *) x;
    int i = 0;

    fprintf(fp," 0x { ");

    while (i++ < 8) {
	fprintf(fp,"%x",*y++);
	if (i < 8)
	    fprintf(fp,", ");
    }
    fprintf(fp," }");
}
