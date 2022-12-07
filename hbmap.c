/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

*/

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <curl/curl.h>

struct string_builder {
	char *buffer;
	size_t len, cap;
};

static void
die(const char *fmt, ...)
{
	va_list args;

	fputs("hbmap: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(1);
}

static const char *
enotnull(const char *str, const char *name)
{
	if (NULL == str)
		die("%s cannot be null", name);
	return str;
}

static struct string_builder *
string_builder_create(size_t cap)
{
	struct string_builder *sb;
	if (NULL == (sb = malloc(sizeof(struct string_builder)))
		|| NULL == (sb->buffer = malloc(cap)))
		die("OOM");
	sb->cap = cap;
	sb->len = 0;
	sb->buffer[sb->len] = '\0';
	return sb;
}

static void
string_builder_resize(struct string_builder *sb, size_t cap)
{
	if (NULL == (sb->buffer = realloc(sb->buffer, cap)))
		die("OOM");
	sb->cap = cap;
}

static void
string_builder_append(struct string_builder *sb, size_t len, char *data)
{
	size_t i;
	while (sb->cap < sb->len + len)
		string_builder_resize(sb, sb->cap * 2);
	for (i = 0; i < len; ++i)
		sb->buffer[sb->len + i] = data[i];
	sb->len += len;
	sb->buffer[sb->len] = '\0';
}

static void
string_builder_free(struct string_builder *sb)
{
	free(sb->buffer);
	free(sb);
}

static unsigned int
write_cb(char *in, unsigned int size, unsigned int nmemb, struct string_builder *sb)
{
	string_builder_append(sb, nmemb, in);
	return size * nmemb;
}

static void
haxmaps_query_maps(const char *query)
{
	CURL *curl;
	CURLcode res;
	char curl_errbuf[CURL_ERROR_SIZE];
	char *query_escaped, post_fields[256];
	struct string_builder *sb;
	char *url_begin, *url_end;
	char *name_begin, *name_end;

	curl = curl_easy_init();
	query_escaped = curl_easy_escape(curl, query, strlen(query));
	sb = string_builder_create(256);

	snprintf(post_fields, sizeof(post_fields), "queryString=%s", query_escaped);

	curl_easy_setopt(curl, CURLOPT_URL, "https://haxmaps.com/hb/rpc");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "hbmap/"VERSION);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, sb);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		die("curl_easy_perform failed: %s", curl_easy_strerror(res));

	name_end = sb->buffer;

	if (sb->len > 0) {
		while (1) {
			url_begin = strchr(name_end+1, '\'');
			if (!url_begin) break;
			url_begin++;
			url_end = strchr(url_begin, '\'');
			if (!url_end) break;
			*url_end = '\0';
			name_begin = strstr(url_end+1, "</b> ");
			if (!name_begin) break;
			name_begin += 5;
			name_end = strstr(name_begin, "</li>");
			if (!name_end) break;
			*name_end = '\0';
			printf("%s-%s\n", url_begin, name_begin);
		}
	}

	free(query_escaped);
	curl_easy_cleanup(curl);
	string_builder_free(sb);
}

static void
haxmaps_query_random_map(void)
{
	CURL *curl;
	CURLcode res;
	char *location;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://haxmaps.com/random");
	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		die("curl_easy_perform failed: %s", curl_easy_strerror(res));

	res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &location);

	if ((res == CURLE_OK) && location)
		printf("%s\n", location);

	curl_easy_cleanup(curl);
}

static void
usage(void)
{
	puts("usage: hbmap [-hrv] [-s query]");
	exit(0);
}

static void
version(void)
{
	puts("hbmap version "VERSION);
	exit(0);
}

int
main(int argc, char **argv)
{
	const char *query;
	bool get_random_map;

	query = NULL;
	get_random_map = false;

	while (++argv, --argc > 0) {
		if ((*argv)[0] == '-' && (*argv)[1] != '\0' && (*argv)[2] == '\0') {
			switch ((*argv)[1]) {
				case 'h': usage(); break;
				case 'v': version(); break;
				case 'r': get_random_map = true; break;
				case 's': --argc; query = enotnull(*++argv, "query"); break;
				default: die("invalid option %s", *argv); break;
			}
		} else {
			die("unexpected argument: %s", *argv);
		}
	}

	if (NULL != query)
		haxmaps_query_maps(query);
	else if (get_random_map)
		haxmaps_query_random_map();

	return 0;
}
