/* tabber.c -- program to entab a file.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sun May 30 22:29:48 EEST 2021
 * Copyright: (C) 2019-2021 LUIS COLORADO.  All rights reserved.
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

#define F(_f) "%s:"__FILE__ ":%d:%s:" _f, progname, __LINE__, __func__

#define WRN(_f, ...)    fprintf(stderr, F("WARNG: " _f), ##__VA_ARGS__)
#define ERR(_f, ...)    fprintf(stderr, F("ERROR: " _f), ##__VA_ARGS__)

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
#define     FLAG_RIGHT_ALIGN    (1 << 6)
#define     FLAG_DONT_TRIM      (1 << 7)
#define     FLAG_DEBUG          (1 << 8)
#define     FLAG_COLOR          (1 << 9)

#define     EXIT_MASK           (FLAG_ERROR | FLAG_WARNING)
#define     EXIT_CODE           (flags & EXIT_MASK)

int   flags        = FLAG_COLOR;
char *out_file;
char *subst_string = DEFAULT_SUBSTSTRING;
char *progname;

static void
spaces(int start, /* column at which string s must go */
       int end,   /* number of spaces that should be printed */
       FILE *out) /* output file */
{
    static char *string_tab = "[////5////0////5////0////"
                              "5////0////5////0////5///>";
    static int string_tab_l = 0;
    if (string_tab_l == 0)
        string_tab_l = strlen(string_tab);

    size_t last_tab_pos = end - end % tabsz;

    if (start < last_tab_pos) {
        if (flags & FLAG_DEBUG && flags & FLAG_COLOR)
            fprintf(out, "\033[1;36m");
        while (start < last_tab_pos) {
            unsigned n = tabsz - start % tabsz;
            switch (n) {
            case 1: /* a simple space */
                fputc(' ', out);
            case 0: /* impossible */
                break;
            default:
                if (flags & FLAG_DEBUG) {
                    assert(n == fprintf(out,
                            "%c%s",
                            *string_tab,
                            string_tab + string_tab_l + 1 - n));
                } else {
                    fputc('\t', out);
                }
                break;
            } /* switch */
            start += n;
        } /* while */
        if (flags & FLAG_DEBUG && flags & FLAG_COLOR)
            fprintf(out, "\033[m");
    }

    static char *spaces_str = "          ";
    static int   spaces_len = 0;
    if (spaces_len == 0)
        spaces_len = strlen(spaces_str);

    int n = end - start;

    while (n > spaces_len) {
        fputs(spaces_str, out);
        n -= spaces_len;
    }
    if (n > 0) {
        fprintf(out, "%.*s", (unsigned) n, spaces_str);
    }
}

static void
process(FILE *f, char *n, FILE *o)
{
    static int
        start_spaces_column = 0,
        actual_column = 0;

    int c;
    while ((c = fgetc(f)) != EOF) {
        switch(c) {

        case '\t': {
            actual_column += tabsz;
            actual_column -= actual_column % tabsz;
            break;
        }

        case ' ': {
            actual_column++; break;
        }

        case '\n': {
            if (flags & FLAG_DONT_TRIM) {
                spaces( start_spaces_column,
                        actual_column,
                        o);
            }
            start_spaces_column = actual_column = 0;
            fputc('\n', o);
            break;
        }

        default: {
            if (actual_column > start_spaces_column) {
                spaces( start_spaces_column,
                        actual_column,
                        o);
            }
            fputc(c, o);
            actual_column++;
            start_spaces_column = actual_column;
            break;
        }} /* switch */
    } /* while */
} /* process */

static void
do_usage(void)
{
    fprintf(stderr,
        "Uso %s [ options ] [ file ... ]\n"
        " -c Prints debugging tabs in color.\n"
        " -d Activates debugging output instead of plain tabs.\n"
        " -h Show this help message.\n"
        " -n <tabsz> Sets the tabsize to its argument.  Default is\n"
        "    DEFAULT_TABSZ(%d).\n"
        " -o <outfile> Sets the output file to argument.  Default is\n"
        "    stdout.\n"
        " -s <subst_string> Sets the substitution string.\n"
        "    Default is spaces\n"
        " -t don't trim trailing spaces at end of line.\n",
        progname,
        DEFAULT_TABSZ);
} /* do_usage */

int
main(int argc, char **argv)
{
    progname = argv[0];
    int opt;
    while((opt = getopt(argc, argv, "cdhn:s:t")) != EOF) {
        switch(opt) {
        case 'c': {
            flags ^= FLAG_COLOR;
            break;
        }
        case 'd': {
            flags ^= FLAG_DEBUG;
            break;
        }
        case 'h':
            flags |= FLAG_DOUSAGE;
            break;
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
        case 's':
            flags |= FLAG_SUBSTSTRING;
            subst_string = optarg;
            break;
        case 't':
            flags |= FLAG_DONT_TRIM;
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
        pid_t pid = getpid();
        for (i = 0; i < argc; i++) {
            FILE *in = fopen(argv[i], "rt");
            if (!in) {
                WRN("%s: %s\n",
                    argv[i], strerror(errno));
                flags |= FLAG_WARNING;
                continue;
            }
            char out_name[PATH_MAX];
            snprintf(out_name, sizeof out_name,
                     "%s-%d", argv[i], pid);
            FILE *out = fopen(out_name, "wt");
            if (!out) {
                ERR("%s: %s\n",
                    out_name, strerror(errno));
                flags |= FLAG_ERROR;
                exit(EXIT_CODE);
            }
            process(in, argv[i], out);
            fclose(in); fclose(out);
            if (rename(out_name, argv[i]) < 0) {
                WRN("cannot rename %s to %s: %s\n",
                    out_name, argv[i], strerror(errno));
            }
        }
    } else {
        process(stdin, "stdin", stdout);
    }
    exit(EXIT_CODE);
} /* main */
