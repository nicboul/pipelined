
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "pipelined.h"


void option_dump(struct options *opts)
{
	int i = 0;
	while (opts[i].tag != NULL) {
		printf("%s => %s \n", opts[i].tag, *(char **)(opts[i].value));
		i++;
	}
}

void option_error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);


	vfprintf(stdout, fmt, ap);
	vsyslog(LOG_ERR, fmt, ap);
}

static char *trim(char *str)
{
	char *a, *z;

	a = str;
	while (*a == ' ') a++;

	z = a + strlen(a);
	while (*--z == ' ' && (z > a));
	*++z = '\0';

	return a;
}

static int is_comment(char *str)
{
	char *p;
	int ret;

	p = str;
	while (*p == ' ') p++;
	if (*p == '#' || *p == '[')
		ret = 1;
	else
		ret = 0;

	return ret;
}

#define MAX_OPT 256
int option_parse(struct options *opts)
{
	FILE *fp;

	char arg[MAX_OPT];
	char *p, *tag;

	int i = 0;
	int ret = 0;
	int err = 0;
	int line = 0;

	memset(arg, 0, MAX_OPT);

	fp = fopen("./conf.ini", "r");

	while (!err && (ret = fscanf(fp, "\n%255[^\n]\n", arg)) != 0)  {

		line++;

		if (!is_comment(arg)) {

			p = strstr(arg, "=");

			if (p == NULL) {
				err = 1;
			}
			else {
				*p = '\0'; p++;
				p = trim(p);

				//printf("p %s\n", p);

				if (*p == 0) {
					err = 1;
				}
			}

			tag = trim(arg); i = 0;
			while (!err && opts[i].tag != NULL) {

				if (strcmp(opts[i].tag, tag) == 0) {

					if (*(void **)(opts[i].value) != 0)
						err = 2;
					else {
						*(void **)(opts[i].value) = strdup(p); //strdup(trim(p));
					}
				}
				i++;
			}
		}

		if (!err && feof(fp))
			err = 3;

	}

	if (err == 1) {
		fprintf(stderr, "Invalid argument, near line %i\n", line);
        _exit(EXIT_ERR_PARS);
	}
	if (err == 2) {
		fprintf(stderr, "Tag '%s' have been provided twice, near line %i\n", tag ,line);
        _exit(EXIT_ERR_PARS);
	}
	if (err == 3) {
		// XXX we dont really need to tell about it...
		//fprintf(stdout, "eof have been reached, parsed %i lines\n", i);
	}

	if (ret == 0) {
		fprintf(stderr, "Unexpected parsing error\n");
        _exit(EXIT_ERR_PARS);
	}

	fclose(fp);

	return ret;
}
/*
int main(int argc, char *argv[])
{
	parse();

	int i = 0;
	while (opts[i].tag != NULL) {

		printf("%s==>", opts[i].tag);
		printf("%s\n", opts[i].value);

		i++;
	}
	return 0;
}*/
