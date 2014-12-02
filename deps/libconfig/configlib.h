#ifndef _CONFIGLIB_HEAD_H_
#define _CONFIGLIB_HEAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ *
 * CONFIGLIB version information
 * ------------------------------------------------------------------------ */
#define CONFIGLIB_MAJOR_VERSION     1
#define CONFIGLIB_MINOR_VERSION     2
#define CONFIGLIB_REVISION_VERSION  1


#define CONF_ITEMNAME_MAXLEN  (512)
#define CONF_ITEMVALUE_MAXLEN (4096)
#define MAX_CONFIG_LINE_LEN       (CONF_ITEMNAME_MAXLEN+CONF_ITEMVALUE_MAXLEN)

struct configlist_type
{
	int index;
	char section[CONF_ITEMNAME_MAXLEN];
	char key[CONF_ITEMNAME_MAXLEN];
	char value[CONF_ITEMVALUE_MAXLEN];

	struct configlist_type *next;
};

int config_read_string(const char *file,char *section,char *key,char *value,char *default_val);
int config_read_integer(const char *file,char *section,char *key,int *value,int default_val);
int config_write_string(const char *file,char *section,char *key,char *value);
int config_write_integer(const char *file,char *section,char *key,int value);

#endif

