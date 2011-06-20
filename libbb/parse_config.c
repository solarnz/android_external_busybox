/* vi: set sw=4 ts=4: */
/*
 * config file parser helper
 *
 * Copyright (C) 2008 by Vladimir Dronnikov <dronnikov@gmail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 * Also for use in uClibc (http://uclibc.org/) licensed under LGPLv2.1 or later.
 */

/* Uncomment to enable test applet */
////config:config PARSE
////config:	bool "Uniform config file parser debugging applet: parse"
////config:	default n
////config:	help
////config:	  Typical usage of parse API:
////config:		char *t[3];
////config:		parser_t *p = config_open(filename);
////config:		while (config_read(p, t, 3, 0, delimiters, flags)) { // 1..3 tokens
////config:			bb_error_msg("TOKENS: '%s''%s''%s'", t[0], t[1], t[2]);
////config:		}
////config:		config_close(p);

////applet:IF_PARSE(APPLET(parse, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-y += parse_config.o

//usage:#define parse_trivial_usage
//usage:       "[-x] [-n MAXTOKENS] [-m MINTOKENS] [-d DELIMS] [-f FLAGS] FILE..."
//usage:#define parse_full_usage "\n\n"
//usage:       "	-x	Suppress output (for benchmarking)"

#include "libbb.h"

#if defined ENABLE_PARSE && ENABLE_PARSE
int parse_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int parse_main(int argc UNUSED_PARAM, char **argv)
{
	const char *delims = "# \t";
	char **t;
	unsigned flags = PARSE_NORMAL;
	int mintokens = 0, ntokens = 128;
	unsigned noout;

	opt_complementary = "-1:n+:m+:f+";
	noout = 1 & getopt32(argv, "xn:m:d:f:", &ntokens, &mintokens, &delims, &flags);
	//argc -= optind;
	argv += optind;

	t = xmalloc(sizeof(t[0]) * ntokens);
	while (*argv) {
		int n;
		parser_t *p = config_open(*argv);
		while ((n = config_read(p, t, ntokens, mintokens, delims, flags)) != 0) {
			if (!noout) {
				for (int i = 0; i < n; ++i)
					printf("[%s]", t[i]);
				puts("");
			}
		}
		config_close(p);
		argv++;
	}
	return EXIT_SUCCESS;
}
#endif

parser_t* FAST_FUNC config_open2(const char *filename, FILE* FAST_FUNC (*fopen_func)(const char *path))
{
	FILE* fp;
	parser_t *parser;

	fp = fopen_func(filename);
	if (!fp)
		return NULL;
	parser = xzalloc(sizeof(*parser));
	parser->fp = fp;
	return parser;
}

parser_t* FAST_FUNC config_open(const char *filename)
{
	return config_open2(filename, fopen_or_warn_stdin);
}

static void config_free_data(parser_t *parser)
{
	free(parser->line);
	parser->line = NULL;
	if (PARSE_KEEP_COPY) { /* compile-time constant */
		free(parser->data);
		parser->data = NULL;
	}
}

void FAST_FUNC config_close(parser_t *parser)
{
	if (parser) {
		config_free_data(parser);
		fclose(parser->fp);
		free(parser);
	}
}

/* This function reads an entire line from a text file, up to a newline
 * or NUL byte, exclusive.  It returns a malloc'ed char*.
 * *lineno is incremented for each line.
 * Trailing '\' is recognized as line continuation.
 * Returns NULL if EOF/error.
 */
static char* get_line_with_continuation(FILE *file, int *lineno)
{
	int ch;
	unsigned idx = 0;
	char *linebuf = NULL;

	while ((ch = getc(file)) != EOF) {
		/* grow the line buffer as necessary */
		if (!(idx & 0xff))
			linebuf = xrealloc(linebuf, idx + 0x101);
		if (ch == '\n')
			ch = '\0';
		linebuf[idx] = (char) ch;
		if (ch == '\0') {
			(*lineno)++;
			if (idx == 0 || linebuf[idx-1] != '\\')
				break;
			idx--; /* go back to '/' */
			continue;
		}
		idx++;
	}
	if (ch == EOF) {
		/* handle corner case when the file is not ended with '\n' */
		(*lineno)++;
		if (linebuf)
			linebuf[idx] = '\0';
	}
	return linebuf;
}


/*
0. If parser is NULL return 0.
1. Read a line from config file. If nothing to read then return 0.
   Handle continuation character. Advance lineno for each physical line.
   Discard everything past comment character.
2. if PARSE_TRIM is set (default), remove leading and trailing delimiters.
3. If resulting line is empty goto 1.
4. Look for first delimiter. If !PARSE_COLLAPSE or !PARSE_TRIM is set then
   remember the token as empty.
5. Else (default) if number of seen tokens is equal to max number of tokens
   (token is the last one) and PARSE_GREEDY is set then the remainder
   of the line is the last token.
   Else (token is not last or PARSE_GREEDY is not set) just replace
   first delimiter with '\0' thus delimiting the token.
6. Advance line pointer past the end of token. If number of seen tokens
   is less than required number of tokens then goto 4.
7. Check the number of seen tokens is not less the min number of tokens.
   Complain or die otherwise depending on PARSE_MIN_DIE.
8. Return the number of seen tokens.

mintokens > 0 make config_read() print error message if less than mintokens
(but more than 0) are found. Empty lines are always skipped (not warned about).
*/
#undef config_read
int FAST_FUNC config_read(parser_t *parser, char **tokens, unsigned flags, const char *delims)
{
	char *line;
	int ntokens, mintokens;
	int t;

	if (!parser)
		return 0;

	ntokens = (uint8_t)flags;
	mintokens = (uint8_t)(flags >> 8);

again:
	memset(tokens, 0, sizeof(tokens[0]) * ntokens);
	config_free_data(parser);

	/* Read one line (handling continuations with backslash) */
	line = get_line_with_continuation(parser->fp, &parser->lineno);
	if (line == NULL)
		return 0;
	parser->line = line;

	/* Skip token in the start of line? */
	if (flags & PARSE_TRIM)
		line += strspn(line, delims + 1);

	if (line[0] == '\0' || line[0] == delims[0])
		goto again;

	if (flags & PARSE_KEEP_COPY)
		parser->data = xstrdup(line);

	/* Tokenize the line */
	t = 0;
	do {
		/* Pin token */
		tokens[t] = line;

		/* Combine remaining arguments? */
		if ((t != (ntokens-1)) || !(flags & PARSE_GREEDY)) {
			/* Vanilla token, find next delimiter */
			line += strcspn(line, delims[0] ? delims : delims + 1);
		} else {
			/* Combining, find comment char if any */
			line = strchrnul(line, delims[0]);

			/* Trim any extra delimiters from the end */
			if (flags & PARSE_TRIM) {
				while (strchr(delims + 1, line[-1]) != NULL)
					line--;
			}
		}

		/* Token not terminated? */
		if (*line == delims[0])
			*line = '\0';
		else if (*line != '\0')
			*line++ = '\0';

#if 0 /* unused so far */
		if (flags & PARSE_ESCAPE) {
			strcpy_and_process_escape_sequences(tokens[t], tokens[t]);
		}
#endif
		/* Skip possible delimiters */
		if (flags & PARSE_COLLAPSE)
			line += strspn(line, delims + 1);

		t++;
	} while (*line && *line != delims[0] && t < ntokens);

	if (t < mintokens) {
		bb_error_msg("bad line %u: %d tokens found, %d needed",
				parser->lineno, t, mintokens);
		if (flags & PARSE_MIN_DIE)
			xfunc_die();
		if (flags & PARSE_KEEP_COPY)
			free(parser->data);
		goto again;
	}

	return t;
}
