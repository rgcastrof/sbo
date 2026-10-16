#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "package.h"

static char *
package_get_pkgfile(Package *pkg, const char *filename)
{
	char *filepath;
	size_t path_len = strlen(pkg->path) + strlen(filename) + 2; /* Adds space to '/' character and '\0' */
	if((filepath = malloc(path_len)) == NULL) {
		fprintf(stderr, "malloc: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	filepath[0] = '\0';
	if (snprintf(filepath, path_len, "%s/%s", pkg->path, filename) < 0) {
		fprintf(stderr, "%s: %s", filepath, strerror(errno));
		free(filepath);
		exit(EXIT_FAILURE);
	}
	return filepath;
}

void
package_print_slackdesc(Package *pkg)
{
	FILE *fp;
	char buf[128];
	char *slackdesc_path = package_get_pkgfile(pkg, "slack-desc");
	if ((fp = fopen(slackdesc_path, "r")) == NULL) {
		fprintf(stderr, "fopen: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf("DESCRIPTION:\n");
	while(fgets(buf, sizeof(buf), fp) != NULL) {
		if (strstr(buf, pkg->name)!=NULL)
			printf("%s", buf);
	}
	free(slackdesc_path);
}
