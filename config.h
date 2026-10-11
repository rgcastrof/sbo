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

#include <string.h>

static const char repo_path[] = "/home/rogi/sbo";
static const char sbo_branch[] = "current";
static const char sbo_mirror[] = "https://github.com/Ponce/slackbuilds.git";

static const char *sync_cmd[] = { "git", "clone", "--depth", "1", "--branch", sbo_branch, sbo_mirror, repo_path, NULL };
static const char *pull_cmd[] = { "git", "-C", repo_path, "pull", NULL };
