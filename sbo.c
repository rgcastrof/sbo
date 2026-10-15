/*
 * sbo
 * Copyright (C) 2026  Rogério Girão
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Project Page: https://github.com/rgcastrof/sbo
 * Rogério Girão rogeriogirao1@proton.me
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "package.h"

struct Config {
	const char *repo_path;
	const char *sbo_mirror;
	const char *sbo_branch;
	const void *v;
};

typedef struct {
	Package *pkg;    /* Stores the found package */
	const void *arg; /* The commandline argument */
	int status;      /* Package search status */
} WalkArg;

static void load_config(void);
static int is_directory(const char *);
static void git_exec_cmd(char **);
static void sync_repository(void);
static void walk(void (*)(WalkArg *), WalkArg *, const char *);
static void query_pkg(WalkArg *);

#include "config.h"

static struct Config cfg;

static void
load_config(void)
{
	cfg.repo_path = repo_path;
	cfg.sbo_mirror = sbo_mirror;
	cfg.sbo_branch = sbo_branch;
}

static int
is_directory(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static void
git_exec_cmd(char **argv)
{
	pid_t pid, wpid;
	if ((pid = fork()) == 0) {
		execvp(argv[0], argv);
		fprintf(stderr, "execvp failed\n");
		exit(EXIT_FAILURE);
	}
	int status;
	while ((wpid = wait(&status) > 0));
}

static void
sync_repository(void)
{
	if (!is_directory(cfg.repo_path)) {
		if (mkdir(cfg.repo_path, S_IRWXU) == -1) {
			fprintf(stderr, "%s: %s\n", cfg.repo_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
		cfg.v = sync_cmd;
	} else {
		cfg.v = pull_cmd;
	}
	git_exec_cmd((char**)cfg.v);
}

static void
walk(void (*callback)(WalkArg *), WalkArg *wa, const char *rootdir)
{
	struct dirent *dent;
	DIR *dir;
	char path[PATH_MAX];

	if ((dir = opendir(rootdir)) == NULL) {
		fprintf(stderr, "%s: %s\n", rootdir, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while((dent = readdir(dir)) != NULL) {
		if (dent->d_type == DT_DIR) {
			if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
				continue;
			if (snprintf(path, PATH_MAX, "%s/%s", rootdir, dent->d_name) < 0) {
				fprintf(stderr, "%s: %s\n", path, strerror(errno));
				exit(EXIT_FAILURE);
			}
			wa->pkg->name = dent->d_name;
			wa->pkg->category = &rootdir[strlen(cfg.repo_path)+1];
			if (snprintf(wa->pkg->path, PATH_MAX, "%s/%s/%s", cfg.repo_path, wa->pkg->category, wa->pkg->name) < 0) {
				fprintf(stderr, "%s: %s\n", wa->pkg->path, strerror(errno));
				exit(EXIT_FAILURE);
			}
			callback(wa);
			walk(callback, wa, path);
		} else
			continue;
	}
	closedir(dir);
}

static void
query_pkg(WalkArg *wa)
{
	if (strcasestr(wa->pkg->name, (char*)wa->arg) != NULL) {
		printf("%s/%s\n", wa->pkg->category, wa->pkg->name);
		wa->status = 1;
	}
}
}

int
main(int argc, char *argv[])
{ 
	Package pkg;
	WalkArg wa;
	int c, syncflag = 0, queryflag = 0, infoflag = 0;

	const char *shortopts = "SQ:";
	const struct option longopts[] = {
		{"sync", no_argument, NULL, 'S'},
		{"query", required_argument, NULL, 'Q'},
	};

	while((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
			case 'S':
				syncflag = 1;
				break;
			case 'Q':
				queryflag = 1;
				wa.arg = optarg;
				break;
			case '?':
			default:
				break;
		}
	}

	load_config();
	wa.pkg = &pkg;
	wa.status = 0; /* status starts with 0, and if a package is found, is set to 1 */

	if (syncflag)
		sync_repository();

	if (queryflag && wa.arg) {
		printf("Searching for %s:\n", (char*)wa.arg);
		walk(query_pkg, &wa, cfg.repo_path);
		if (!wa.status) {
			fprintf(stderr, "No match for %s found\n", (char*)wa.arg);
			return 1;
		}
	}
	
	return 0;
}
