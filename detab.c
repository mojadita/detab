/* detab.c -- program to detab a file.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sun Sep  8 10:29:40 EEST 2019
 * Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
 * License: BSD.
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef DEBUG
#define DEBUG                   (0)
#endif

#define F(_f) "%s:"__FILE__":%d:%s:"_f, \
        progname,__LINE__,__func__

#define WRN(_f, ...) do {               \
            flags |= FLAG_WARNING;      \
            fprintf(stderr,             \
                    F("WARNG: " _f),    \
                    ##__VA_ARGS__);     \
        } while (0)

#define ERR(_f, ...)  do {              \
            flags |= FLAG_ERROR;        \
            fprintf(stderr,             \
                    F("ERROR: " _f),    \
                    ##__VA_ARGS__);     \
            exit(EXIT_CODE);            \
        } while (0)

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
#define DEFAULT_SUBSTSTRING     \
        "                   "
#endif

int tabsz = DEFAULT_TABSZ;

#define FLAG_WARNING            (1 << 0)
#define FLAG_ERROR              (1 << 1)
#define FLAG_DOUSAGE            (1 << 2)
#define FLAG_TABSZ              (1 << 3)
#define FLAG_OUTFILE            (1 << 4)
#define FLAG_SUBSTSTRING        (1 << 5)
#define FLAG_RIGHT_ALIGN        (1 << 6)
#define FLAG_DONT_TRIM          (1 << 7)
#define FLAG_ANYFILETOUCHED     (1 << 8)

#define EXIT_MASK               (FLAG_ERROR | FLAG_WARNING)
#define EXIT_CODE               (flags & EXIT_MASK)

int flags;
char *out_file;
char *subst_string = DEFAULT_SUBSTSTRING;
char *progname;

static void
spaces(int pos,   /* column at which string s must go */
       int n,     /* number of spaces that should be printed */
       char *s,   /* string to fill the space. */
       FILE *f)   /* output file */
{
    int len = strlen(s);
    while (n >= len) {
        fputs(s, f);
        n -= len;
    }
    if (n > 0)
        fprintf(f, "%.*s", n, s);
}

static void
tabs(int pos,     /* column at which string s must start */
       int n,     /* number of spaces that should be printed */
       char *s,   /* string to fill the space. */
       FILE *f)   /* output file */
{
    if (n == 1) {
        fputc(s[0], f);
        return;
    }
    int end = pos + n; /* ending position */
    int next_tab = pos - (pos % tabsz) + tabsz;
    while (next_tab <= end) {
        fputc('\t', f);
        pos = next_tab;
        next_tab += tabsz;
    }
    /* next_tab > pos */
    n = end - pos; /* need still to write */
    spaces(pos, n, s, f);
}

static void (*tabs_spaces)(int pos, int n, char *s, FILE *f)
    = spaces;

static void
process(FILE *f, char *n, FILE *o)
{
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
                tabs_spaces(col, sp, subst_string, o);
            }
            col = sp = 0;
            fputc('\n', o);
            break;

        default:
            if (sp) {
                tabs_spaces(col, sp, subst_string, o);
                col += sp; sp = 0;
            }
            fputc(c, o);
            col++;
            break;
        } /* switch */
    } /* while */
} /* process */

static void
do_usage(void)
{
    fprintf(stderr,
        "Usage %s [ options ] [ file ... ]\n"
		" -f <file> Process file onto itself.\n"
        " -h Show this help message.\n"
        " -n <tabsz> Sets the tabsize to its argument.  Default is\n"
        "    DEFAULT_TABSZ(%d).\n"
        " -o <outfile> Sets the output file to argument.  Default is\n"
        "    stdout.\n"
        " -s act as detab (use spaces).\n"
        " -S <subst_string> Sets the substitution string.\n"
        "    Default is spaces\n"
        " -t act as tabber (use tabs).\n"
        " -T don't trim trailing spaces at end of line.\n",
		progname,
        DEFAULT_TABSZ);
} /* do_usage */

struct pers {
    char *name;
    void (*func)(int pos, int n, char *s, FILE *f);
    int flags_on;
    int flags_switch;
} personalities[] = {
    { .name = "detab",  .func = spaces },
    { .name = "entab",  .func = tabs },
    { .name = NULL,     .func = NULL }
};

int
main(int argc, char **argv)
{
    int opt;
    progname = strchr(argv[0], '/');
    if (progname) progname++;
    else progname = argv[0];

    for (struct pers *p = personalities; p->name; p++) {
        if (strcmp(p->name, progname) == 0) {
            tabs_spaces = p->func;
            flags |= p->flags_on;
            flags ^= p->flags_switch;
            break;
        }
    }
    while((opt = getopt(argc, argv, "f:hn:sS:tT")) != EOF) {
        switch(opt) {
        case 'f': {
                pid_t pid = getpid();
                flags |= FLAG_ANYFILETOUCHED;
                FILE *in = fopen(optarg, "rt");
                if (!in) {
                    WRN("%s: %s\n",
                        optarg, strerror(errno));
                    break;
                }
                char out_name[PATH_MAX];
                snprintf(out_name, sizeof out_name,
                         "%s-%s.%d", progname, optarg, pid);
                FILE *out = fopen(out_name, "wt");
                if (!out) {
                    ERR("%s: %s\n",
                        out_name, strerror(errno));
                }
                process(in, optarg, out);
                fclose(in);
                fclose(out);
                if (rename(out_name, optarg) < 0) {
                    WRN("cannot rename %s to %s: %s\n",
                        out_name, optarg, strerror(errno));
                    break;
                }
            }
            break;
        case 'h':
            flags |= FLAG_DOUSAGE;
            break;
        case 'n': {
                int res = sscanf(optarg, "%d", &tabsz);
                if (res != 1) {
                    WRN("couldn't parse int argument('%s')\n",
                            optarg);
                    break;
                }
                flags |= FLAG_TABSZ;
                if ( tabsz < MIN_TABSZ ) {
                    WRN("tabsz(%d) adjusted to %d\n",
                        tabsz, DEFAULT_TABSZ);
                    tabsz = DEFAULT_TABSZ;
                }
            }
            break;
        case 'S':
            flags |= FLAG_SUBSTSTRING;
            subst_string = optarg;
            break;
        case 's':
            tabs_spaces = spaces;
            break;
        case 'T':
            flags |= FLAG_DONT_TRIM;
            break;
        case 't':
            tabs_spaces = tabs;
            break;
        default:
            ERR("invalid option: %c\n", opt);
            flags |= FLAG_WARNING | FLAG_DOUSAGE;
            break;
        } /* switch */
    } /* while */

    argv += optind; argc -= optind;

    if (flags & FLAG_DOUSAGE) {
        do_usage();
        exit(EXIT_CODE);
    }

    if (argc > 0) {
        int i;
        for (i = 0; i < argc; i++) {
            FILE *in = fopen(argv[i], "r");
            if (!in) {
                ERR("%s: %s (errno = %d)\n",
                        argv[i], strerror(errno),
                        errno);
                /* NOTREACHED */
            }
            process(in, argv[i], stdout);
            fclose(in);
        }
    } else if (!(flags & FLAG_ANYFILETOUCHED)){
        process(stdin, "stdin", stdout);
    }
    exit(EXIT_CODE);
} /* main */
