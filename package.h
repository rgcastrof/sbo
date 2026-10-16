#ifndef PACKAGE_H
#define PACKAGE_H

#include <linux/limits.h>

typedef struct {
	const char *name;
	const char *category;
	char path[PATH_MAX];
} Package;

void package_print_info(Package *);
void package_print_slackdesc(Package *);

#endif
