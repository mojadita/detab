/* detab.c -- program to detab a file.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sun Sep  8 10:29:40 EEST 2019
 * Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define F(_f) __FILE__ ":%d:%s:" _f, __LINE__, __func__

#define WRN(_f, ...)    fprintf(stderr, F("WARNG: " _f), ##__VA_ARGS__)
#define ERR(_f, ...)    fprintf(stderr, F("ERROR: " _f), ##__VA_ARGS__)

#ifndef PROGNAME
#define PROGNAME                "detab"
#endif

#ifndef MIN_TABSZ
#define MIN_TABSZ               (2)
#endif

#ifndef DEFAULT_TABSZ
#define DEFAULT_TABSZ           (4)
#endif

#ifndef DEFAULT_SUBSTSTRING
#define DEFAULT_SUBSTSTRING     "        "
#endif

int tabsz = DEFAULT_TABSZ;

#define     FLAG_WARNING        (1 << 0)
#define     FLAG_ERROR          (1 << 1)
#define     FLAG_DOUSAGE        (1 << 2)
#define     FLAG_TABSZ          (1 << 3)
#define     FLAG_OUTFILE        (1 << 4)
#define     FLAG_SUBSTSTRING    (1 << 5)
#define		FLAG_RIGHT_ALIGN	(1 << 6)
#define		FLAG_DONT_TRIM		(1 << 7)

#define     EXIT_MASK           (FLAG_ERROR | FLAG_WARNING)
#define     EXIT_CODE           (flags & EXIT_MASK)

int flags;
char *out_file;
char *subst_string = DEFAULT_SUBSTSTRING;

static void
do_usage(void)
{
    fprintf(stderr,
        "Uso " PROGNAME " [ options ] [ file ... ]\n"
        "   -n <tabsz> Sets the tabsize to its argument.  Default is\n"
        "     DEFAULT_TABSZ(%d).\n"
        "   -o <outfile> Sets the output file to argument.  Default is\n"
        "     stdout.\n"
        "   -s <subst_string> Sets the substitution string.\n"
        "      Default is spaces\n"
        "   -h Show this help message.\n",
        DEFAULT_TABSZ);
} /* do_usage */

static void
spaces(int pos,   /* column at which string s must go */
	   int n,     /* number of spaces that should be printed */
	   char *s,   /* string to fill the space. */
	   FILE *f)   /* output file */
{
	if (n == 1) {
		/* if we print only one
		 * space, do it normally */
		fputs(" ", f);
		return;
	}
    int l = strlen(s);
	int off0 = pos % l;
	int len0 = l - off0;
	if (len0 > n) len0 = n;
	if (len0 > 0) {
		fprintf(f, "%.*s", len0, s + off0);
		n -= len0;
	}
	while (n >= l) {
        fputs(s, f);
        n -= l;
    }
	if (n > 0)
		fprintf(f, "%.*s", n, s);
}

static void
process(FILE *f, char *n, FILE *o)
{
    fprintf(stderr, F("%s:\n"), n);
    int c;
    static int col = 0, sp = 0;

    while ((c = fgetc(f)) != EOF) {
        int n;
        switch(c) {
        case '\t':
            n = tabsz - (col + sp) % tabsz;
            sp += n; break;

        case ' ':
            sp++; break;

        case '\n':
			if (flags & FLAG_DONT_TRIM) {
				spaces(col, sp, subst_string, o);
			}
            col = sp = 0;
            fputc('\n', o);
            break;

        default:
            if (sp) {
                spaces(col, sp, subst_string, o);
                col += sp; sp = 0;
            }
            fputc(c, o);
            col++;
            break;
        } /* switch */
    } /* while */
} /* process */

int
main(int argc, char **argv)
{
    int opt;
    while((opt = getopt(argc, argv, "n:ho:S:s:t")) != EOF) {
        switch(opt) {
        case 'n':
            tabsz = atol(optarg);
            flags |= FLAG_TABSZ;
            if ( tabsz < MIN_TABSZ ) {
                WRN("tabsz(%d) adjusted to %d\n",
                    tabsz, DEFAULT_TABSZ);
                tabsz = DEFAULT_TABSZ;
                flags |= FLAG_WARNING;
            }
            break;
        case 'h':
            flags |= FLAG_DOUSAGE;
            break;
        case 'o':
            flags |= FLAG_OUTFILE;
            out_file = optarg;
            break;
        case 'S':
			flags |= FLAG_RIGHT_ALIGN;
			/* no break here */
        case 's':
            flags |= FLAG_SUBSTSTRING;
            subst_string = optarg;
            break;
		case 't':
			flags |= FLAG_DONT_TRIM;
			break;
        default:
            ERR("invalid option: %c\n", opt);
            flags |= FLAG_ERROR;
            break;
        } /* switch */
    } /* while */

    argv += optind; argc -= optind;

    if (flags & FLAG_DOUSAGE) {
        do_usage();
        exit(EXIT_CODE);
    }

    FILE *out = stdout;

    if (flags & FLAG_OUTFILE) {
        out = fopen(out_file, "w");
        if (!out) {
            ERR("%s: %s\n", out_file, strerror(errno));
            exit(EXIT_CODE);
        }
    }

    if (argc > 0) {
        int i;
        for (i = 0; i < argc; i++) {
            FILE *f = fopen(argv[i], "rt");
            if (!f) {
                WRN("%s: %s\n",
                    argv[i], strerror(errno));
                flags |= FLAG_ERROR;
                break;
            }
            process(f, argv[i], out);
            fclose(f);
        }
    } else {
        process(stdin, "stdin", out);
    }
    exit(EXIT_CODE);
} /* main */
