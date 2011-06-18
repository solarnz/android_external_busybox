/* vi: set sw=4 ts=4: */
/*
 * Mini groups implementation for busybox
 *
 * Copyright (C) 2011 Tanguy Pruvot <tanguy.pruvot@gmail.com>.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

//usage:#define groups_trivial_usage
//usage:       ""
//usage:#define groups_full_usage "\n\n"
//usage:       "Print the group names associated with the current user id"

#include "libbb.h"

extern int id_main(int argc, char **argv);

int groups_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int groups_main(int argc UNUSED_PARAM, char **argv UNUSED_PARAM)
{
	if (argv[1])
		bb_show_usage();

	char *id_argv[4];
	id_argv[0] = xstrdup("id");
	id_argv[1] = xstrdup("-G");
	id_argv[2] = xstrdup("-n");
	id_argv[3] = 0;

	return id_main(3, id_argv);
}
