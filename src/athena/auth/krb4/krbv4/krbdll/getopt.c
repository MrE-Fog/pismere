/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getopt.c	4.7 (Berkeley) 6/27/88";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <conf.h>

/*
 * get option letter from argument vector
 */
int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	EMSG	""
#define	tell(s)	{ \
	if (opterr) { \
		fputs(*nargv, stderr); \
		fputs(s, stderr); \
		fputc(optopt, stderr); \
		fputc((int)'\n', stderr); \
	} \
	return(BADCH); \
}

getopt(nargc, nargv, ostr)
	int nargc;
	char **nargv, *ostr;
{
	static char *place = EMSG;		/* option letter processing */
	register char *oli;			/* option letter list index */

	if (!*place) {				/* update scanning pointer */
		if (optind >= nargc || *(place = nargv[optind]) != '-')
			return(EOF);
		if (place[1] && *++place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}					/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    !(oli = index(ostr, optopt))) {
		if (!*place)
			++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {			/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else {					/* need an argument */
		if (*place)			/* no white space */
			optarg = place;
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else				/* white space */
			optarg = nargv[optind];
		place = EMSG;
		++optind;
	}
	return(optopt);				/* dump back option letter */
}