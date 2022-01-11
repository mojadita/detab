DETAB/ENTAB(1)          FreeBSD General Commands Manual         DETAB/ENTAB(1)

NAME
     detab, entab - a program to eliminate/insert tabs from a list of files.

SYNOPSIS
     detab [-hstT] [-n tabsize] [-S space_subst_string] [file ...]
     entab [-hstT] [-n tabsize] [-S space_subst_string] [file ...]

DESCRIPTION
     The detab program eliminates all tabs (based on specified or default
     tabsize) an substitutes them by spaces in the program.  It converts every
     space in the correct number of spaces to generate a line with the same
     appearance as the original file.

     The entab program eliminates all spaces and substitutes them by the
     minimum amount of tabs necessary to generate the same spacing in the
     file.

     Both programs operate on the specified files in the command line (this
     is, the file acts as input to the program and the output to it, so the
     effect is that the whole file is tabbed or detabbed), or act as filters
     (operate on stdin and generate the output to stdout) when no filename is
     specified.

OPTIONS
     -h      This flag causes a small help screen to appear as output, that
             allows to consult the valid available options for each program.

     -s      Makes the called program to operate as detab, eliminating all
             tabs from the file and substituting them by spaces.  Both
             programs are links to the same executable so this option has the
             same result as if detab had been called.

     -t      This flag makes the called program to operate as entab
             substituting the spaces by as many tabs as possible to make the
             output to appear as the original file.

     -T      By default, detab or entab trim all whitespace at the end of the
             file lines.  By using this option, final space is subject to
             modification as interword spacing but it is not eliminated from
             the file.

     -n tabsize
             Allows to indicate the tab size.  Using detab with some tab size,
             and piping its output to entab with a different tab size is some
             means of retabbing a file with a different tabsize to allow
             printing on a hardwired tab size terminal.

     -S space_subst_string
             Uses space_subst_string as the string to be substituted for
             spacing.  The string is reused again and again, concatenating it
             to itself to show the spacing physically.  Only the printed
             spaces are substituted by space_subst_string.

AUTHOR
     Luis Colorado <luiscoloradourcola@gmail.com>

                               January 11, 2022
