/*
 * quad_cksum.c
 *
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Quadratic Congruential Manipulation Dectection Code
 *
 * ref: "Message Authentication"
 *		R.R. Jueneman, S. M. Matyas, C.H. Meyer
 *		IEEE Communications Magazine,
 *		Sept 1985 Vol 23 No 9 p 29-40
 *
 * This routine, part of the Athena DES library built for the Kerberos
 * authentication system, calculates a manipulation detection code for
 * a message.  It is a much faster alternative to the DES-checksum
 * method. No guarantees are offered for its security.	Refer to the
 * paper noted above for more information
 *
 * Implementation for 4.2bsd
 * by S.P. Miller	Project Athena/MIT
 */

/*
 * Algorithm (per paper):
 *		define:
 *		message to be composed of n m-bit blocks X1,...,Xn
 *		optional secret seed S in block X1
 *		MDC in block Xn+1
 *		prime modulus N
 *		accumulator Z
 *		initial (secret) value of accumulator C
 *		N, C, and S are known at both ends
 *		C and , optionally, S, are hidden from the end users
 *		then
 *			(read array references as subscripts over time)
 *			Z[0] = c;
 *			for i = 1...n
 *				Z[i] = (Z[i+1] + X[i])**2 modulo N
 *			X[n+1] = Z[n] = MDC
 *
 *		Then pick
 *			N = 2**31 -1
 *			m = 16
 *			iterate 4 times over plaintext, also use Zn
 *			from iteration j as seed for iteration j+1,
 *			total MDC is then a 128 bit array of the four
 *			Zn;
 *
 *			return the last Zn and optionally, all
 *			four as output args.
 *
 * Modifications:
 *	To inhibit brute force searches of the seed space, this
 *	implementation is modified to have
 *	Z	= 64 bit accumulator
 *	C	= 64 bit C seed
 *	N	= 2**63 - 1
 *  S	= S seed is not implemented here
 *	arithmetic is not quite real double integer precision, since we
 *	can't get at the carry or high order results from multiply,
 *	but nonetheless is 64 bit arithmetic.
 */

#include "mit_copy.h"

/* System include files */
#include <stdio.h>

/* Application include files */
#include "des.h"
#ifdef OS2
#include "des_inte.h"
#else
//#include "des_internal.h"
#include "des_inte.h"
#endif
/* Definitions for byte swapping */

#ifdef LSBFIRST
#ifdef MUSTALIGN
static unsigned KRB_INT32 vaxtohl();
static unsigned short vaxtohs();
#else /* ! MUSTALIGN */
#define vaxtohl(x) *((unsigned KRB_INT32 *)(x))
#define vaxtohs(x) *((unsigned short *)(x))
#endif /* MUSTALIGN */
#else /* !LSBFIRST */
static unsigned KRB_INT32 four_bytes_vax_to_nets();
#define vaxtohl(x) four_bytes_vax_to_nets((char *)(x))
static unsigned short two_bytes_vax_to_nets();
#define vaxtohs(x) two_bytes_vax_to_nets((char *)(x))
#endif

/* Externals */
extern int des_debug;

/*** Routines ***************************************************** */

unsigned long DES_CALLCONV_C
des_quad_cksum(in,out,length,out_count,c_seed)
    unsigned char *in;		/* input block */
    unsigned KRB_INT32 *out;		/* optional longer output */
    long length;		/* original length in bytes */
    int out_count;		/* number of iterations */
    des_cblock *c_seed;		/* secret seed, 8 bytes */
{

    /*
     * this routine both returns the low order of the final (last in
     * time) 32bits of the checksum, and if "out" is not a null
     * pointer, a longer version, up to entire 32 bytes of the
     * checksum is written unto the address pointed to.
     */

    register unsigned KRB_INT32 z;
    register unsigned KRB_INT32 z2;
    register unsigned KRB_INT32 x;
    register unsigned KRB_INT32 x2;
    register unsigned char *p;
    register KRB_INT32 len;
    register int i;

    /* use all 8 bytes of seed */

    z = vaxtohl(c_seed);
    z2 = vaxtohl((char *)c_seed+4);
    if (out == NULL)
	out_count = 1;		/* default */

    /* This is repeated n times!! */
    for (i = 1; i <=4 && i<= out_count; i++) {
	len = length;
	p = in;
	while (len) {
	    if (len > 1) {
		x = (z + vaxtohs(p));
		p += 2;
		len -= 2;
	    }
	    else {
		x = (z + *p++);
		len = 0;
	    }
	    x2 = z2;
	    z  = ((x * x) + (x2 * x2)) % 0x7fffffff;
	    z2 = (x * (x2+83653421))   % 0x7fffffff; /* modulo */
#ifdef DEBUG
	    if (des_debug & 8)
		printf("%d %d\n",z,z2);
#endif
	}

	if (out != NULL) {
	    *out++ = z;
	    *out++ = z2;
	}
    }
    /* return final z value as 32 bit version of checksum */
    return z;
}
#ifdef MSBFIRST

static unsigned short two_bytes_vax_to_nets(p)
    char *p;
{
    union {
	char pieces[2];
	unsigned short result;
    } short_conv;

    short_conv.pieces[0] = p[1];
    short_conv.pieces[1] = p[0];
    return(short_conv.result);
}

static unsigned KRB_INT32 four_bytes_vax_to_nets(p)
    char *p;
{
    union {
	char pieces[4];
	unsigned KRB_INT32 result;
    } long_conv;

    long_conv.pieces[0] = p[3];
    long_conv.pieces[1] = p[2];
    long_conv.pieces[2] = p[1];
    long_conv.pieces[3] = p[0];
    return(long_conv.result);
}

#endif
#ifdef LSBFIRST
#ifdef MUSTALIGN
static unsigned KRB_INT32 vaxtohl(x)
char *x;
{
    unsigned KRB_INT32 val;
    memcpy((char *)&val, x, sizeof(val));
    return(val);
} 

static unsigned short vaxtohs(x)
char *x;
{
    unsigned short val;
    memcpy((char *)&val, x, sizeof(val));
    return(val);
} 
#endif /* MUSTALIGN */
#endif /* LSBFIRST */
