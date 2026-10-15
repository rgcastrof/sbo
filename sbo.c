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
#include <linux/limits.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "config.h"


struct Config {
	const char *repo_path;
	const char *sbo_mirror;
	const char *sbo_branch;
	const void *v;
};


static void load_config(void);
static int is_directory(const char *);
static void git_exec_cmd(char **);
static void walk(void (*)(const char *, const char *), const char *, const char *);
static void query_pkg(const char *, const char *);
static void sync_repository(void);

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
walk(void (*callback)(const char *, const char *), const char *rootdir, const char *query)
{
	struct dirent *dent;
	char path[PATH_MAX];
	DIR *dir;

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
			callback(dent->d_name, query);
			walk(callback, path, query);
		} else
			continue;
	}
	closedir(dir);
}

static void
query_pkg(const char *entry, const char *query)
{
	if (strcasestr(entry, query) != NULL)
		printf("%s\n", entry);
}

int
main(int argc, char *argv[])
{ 
	const char *queryarg = NULL;
	int c, syncflag = 0;

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
				queryarg = optarg;
				break;
			case '?':
			default:
				break;
		}
	}

	load_config();
	if (syncflag)
		sync_repository();

	if (queryarg)
		walk(query_pkg, cfg.repo_path, queryarg);
	
	return 0;
}
