//-----------------------------------------------------------------------------
// lib for read INI file
//
// 2008/12/10 --- xuedezhang
//
//-----------------------------------------------------------------------------
#include "configlib.h"

#define _upper_case(c) (c&0xDF)

static int _str_case_equal(char *s1,char *s2);
static void _travel_config_tree(struct configlist_type *config);
static void _destroy_config_tree(struct configlist_type *config);
static struct configlist_type * _create_config_tree(const char *file);
static int _parse_key_value(char *key,char *value,char *line);
static int _is_comment(char *s);
static int _is_section(char *s);
static char *_str_trim(char *buff,char *s);
static int _readline(FILE *fp,char *buffer, int maxlen);
void _write_config_file(const char *file, struct configlist_type *pHead);
//-----------------------------------------------------------------------------
static int _readline(FILE *fp,char *buffer, int maxlen)
{
    int  i, j;
    char ch1;

    for(i = 0,j = 0; i < maxlen; j++)
    {
        if(fread(&ch1, sizeof(char), 1, fp) != 1)
        {
            if(feof(fp) != 0)
            {
                if(j == 0)
                    return -1;
                else break;
            }
            if(ferror(fp) != 0)
                return -2;
            return -2;
        }
        else
        {
            if(ch1 == '\n' || ch1 == 0x00)
                break;
            if(ch1 == '\f' || ch1 == 0x1A)
            {
                buffer[i++] = ch1;
                break;
            }
            if(ch1 != '\r')
                buffer[i++] = ch1;
        }
    }
    buffer[i] = '\0';
    return i;
}
//-----------------------------------------------------------------------------
static char *_str_trim(char *buff,char *s)
{
	size_t i;

	size_t leftpos = 0;
	size_t rightpos = strlen(s)-1;

	for(i=0;i<strlen(s);i++)
	{
		if(s[i] != ' ' && s[i] != '\t')
		{
			leftpos = i;
			break;
		}
	}
	for(i=strlen(s)-1;i>=0;i--)
	{
		if(s[i] != ' ' && s[i] != '\t')
		{
			rightpos=i;
			break;
		}
		
	}

	strncpy(buff,s+leftpos,rightpos-leftpos+1);

	return buff;
}
//-----------------------------------------------------------------------------
static int _is_section(char *s)
{
	if(s[0] == '[' && s[strlen(s)-1] == ']')
		return 0;

	return -1;
}
//-----------------------------------------------------------------------------
static int _is_comment(char *s)
{
	if(s[0] == ';' || s[0] == '#')
		return 0;
	
	return -1;
}
//-----------------------------------------------------------------------------
static int _parse_key_value(char *key,char *value,char *line)
{
	size_t i,pos=0;
	char buff[CONF_ITEMNAME_MAXLEN];
	
	for(i=0;i<strlen(line);i++)
	{
		if(line[i] == '=')
		{
			pos = i;
			break;
		}
	}	
	if(pos < 1)
	{
		return -1;
	}


	memset(buff,0x0,sizeof(buff));
	strncpy(buff,line,pos);
	_str_trim(key,buff);


	memset(buff,0x0,sizeof(buff));
	strncpy(buff,line+pos+1,strlen(line)-pos-1);
	_str_trim(value,buff);

	return 0;

}
//-----------------------------------------------------------------------------
static struct configlist_type * _create_config_tree(const char *file)
{
	struct configlist_type *pHead = NULL;
	struct configlist_type *pTail = NULL;
	struct configlist_type *p = NULL;
	FILE *fp;
	int n;

	char section[CONF_ITEMNAME_MAXLEN];
	char key[CONF_ITEMNAME_MAXLEN];
	char value[CONF_ITEMVALUE_MAXLEN];

	char buff[MAX_CONFIG_LINE_LEN];
	char line[MAX_CONFIG_LINE_LEN];
	int index;

	fp = fopen(file,"r");
	if(!fp)
		return NULL;

	memset(section,0x0,sizeof(section));
	while(!feof(fp))
	{
		memset(buff,0x0,sizeof(buff));
		memset(line,0x0,sizeof(line));

		memset(key,0x0,sizeof(key));
		memset(value,0x0,sizeof(value));

		n = _readline(fp,buff,sizeof(buff));	
		if(n <= 0)
			continue;
		
		_str_trim(line,buff);

		if(_is_comment(line) == 0)
			continue;

		if(_is_section(line) == 0)
		{
			//new section found;
			index = 0;
			memset(buff,0x0,sizeof(buff));
			strncpy(buff,line+1,strlen(line)-2);
			memset(section,0x0,sizeof(section));
			_str_trim(section,buff);
			continue;
		}

		if(_parse_key_value(key,value,line) == 0)
		{
			p = (struct configlist_type *)malloc(sizeof(struct configlist_type));
			if(p)
			{
				//printf("malloc 0x%p(%d bytes)\n",p,sizeof(struct configlist_type));
				strcpy(p->section,section);
				strcpy(p->key,key);
				strcpy(p->value,value);
				p->index = index++;
				p->next = NULL;
				
				if(pHead == NULL)
				{
					pHead = p;
					pHead->next = NULL;
					pTail = p;
					pTail->next = NULL;
				}
				else
				{
					pTail->next = p;
					pTail = p;
				}
			}
		}
	}


	return pHead;
}

//-----------------------------------------------------------------------------
static void _destroy_config_tree(struct configlist_type *config)
{
	struct configlist_type *p = NULL;
	struct configlist_type *pdel = NULL;

	p = config;
	pdel = config;

	while(pdel)
	{
		p = p->next;

		//printf("free 0x%p\n",pdel);
		free(pdel);

		pdel = p;
	}
}
//-----------------------------------------------------------------------------
static void _travel_config_tree(struct configlist_type *config)
{
	/*
	struct configlist_type *p = NULL;
	FILE *fp;

	p = config;
	fp = fopen("tree.log","w+");
	while(p)
	{
		if(fp)
			fprintf(fp,"%-4d [%s] %s=%s\n",p->index,p->section,p->key,p->value);
		p = p->next;
	}
	fclose(fp);
	*/
}
//-----------------------------------------------------------------------------
static int _str_case_equal(char *s1,char *s2)
{
	size_t len1,len2;
	size_t i;
	int ret=0;

	len1 = strlen(s1);
	len2 = strlen(s2);

	if(len1 != len2)
		return -1;

	for(i=0;i<len1;i++)
	{
		if(_upper_case(s1[i]) != _upper_case(s2[i]))
		{
			ret = -1;
			break;
		}
	}

	return ret;
}
//-----------------------------------------------------------------------------

int config_read_string(const char *file,char *section,char *key,char *value,char *default_val)
{
	int flag=0;
	struct configlist_type *pHead = NULL;
	struct configlist_type *p = NULL;

	pHead = _create_config_tree(file);

	_travel_config_tree(pHead);

	p = pHead;
	while(p)
	{
		if(_str_case_equal(p->section,section) == 0 && _str_case_equal(p->key,key) == 0)
		{
			flag = 1;

			//found;
			if(strlen(p->value) <= 0)
				strcpy(value,default_val);
			else
				strcpy(value,p->value);
			break;
		}
		p = p->next;
	}

	_destroy_config_tree(pHead);

	if(flag == 0)
	{
		strcpy(value,default_val);
	}

	return 0;

}
//-----------------------------------------------------------------------------
int config_read_integer(const char *file,char *section,char *key,int *value,int default_val)
{
	int flag = 0;
	struct configlist_type *pHead = NULL;
	struct configlist_type *p = NULL;

	pHead = _create_config_tree(file);

	_travel_config_tree(pHead);

	p = pHead;
	while(p)
	{
		if(_str_case_equal(p->section,section) == 0 && _str_case_equal(p->key,key) == 0)
		{
			flag = 1;

			//found;
			if(strlen(p->value) <= 0)
				*value = default_val;
			else
				*value = atoi(p->value);
			break;
		}
		p = p->next;
	}

	_destroy_config_tree(pHead);

	if(flag == 0)
	{
		*value = default_val;
	}
	return 0;

}
//-----------------------------------------------------------------------------
int config_write_string(const char *file,char *section,char *key,char *value)
{
	int flag = 0;
	struct configlist_type *pHead = NULL;
	struct configlist_type *p = NULL;

	pHead = _create_config_tree(file);

	_travel_config_tree(pHead);

	p = pHead;
	while(p)
	{
		if(_str_case_equal(p->section,section) == 0 && _str_case_equal(p->key,key) == 0)
		{
			flag = 1;

			//found;
			memset(p->value, 0x0, sizeof(p->value));
			sprintf(p->value, "%s", value);
			break;
		}
		p = p->next;
	}
	
	_write_config_file(file, pHead);

	_destroy_config_tree(pHead);
	return 0;
}
//-----------------------------------------------------------------------------
int config_write_integer(const char *file,char *section,char *key,int value)
{
	int flag = 0;
	struct configlist_type *pHead = NULL;
	struct configlist_type *p = NULL;

	pHead = _create_config_tree(file);

	_travel_config_tree(pHead);

	p = pHead;
	while(p)
	{
		if(_str_case_equal(p->section,section) == 0 && _str_case_equal(p->key,key) == 0)
		{
			flag = 1;

			//found;
			memset(p->value, 0x0, sizeof(p->value));
			sprintf(p->value, "%d", value);
			break;
		}
		p = p->next;
	}
	
	_write_config_file(file, pHead);

	_destroy_config_tree(pHead);
	return 0;
}
//-----------------------------------------------------------------------------
void _write_config_file(const char *file, struct configlist_type *pHead)
{
	FILE *fp;
	struct configlist_type *p;
	
	fp = fopen(file, "w+");
	if(fp)
	{
		p = pHead;
		while(p)
		{
			if(p->index == 0)
			{
				fprintf(fp, "[%s]\n", p->section);
				fprintf(fp, "%s = %s\n", p->key, p->value);
			}
			else
			{
				fprintf(fp, "%s = %s\n", p->key, p->value);
			}
			p = p->next;
		}
		fclose(fp);
	}
}
//-----------------------------------------------------------------------------
#if defined(TEST_LIB)
int main(int argc,char **argv)
{
	char buff[1024];
	int  value;

	memset(buff,0x0,sizeof(buff));
	config_read_string("test_lib.ini","SECTION1","KEY1",buff,"default");
	printf("[SECTION1],KEY1=%s\n",buff);

	config_read_integer("test_lib.ini","SECTION1","KEY2",&value,234);
	printf("[SECtiON1],KEY2=%d\n",value);



	config_write_string("test_lib.ini","SECTION1","KEY1","test write");
	config_write_integer("test_lib.ini","SECTION1","KEY2",54321);


	memset(buff,0x0,sizeof(buff));
	config_read_string("test_lib.ini","SECTION1","KEY1",buff,"default");
	printf("[SECTION1],KEY1=%s\n",buff);

	config_read_integer("test_lib.ini","SECTION1","KEY2",&value,234);
	printf("[SECtiON1],KEY2=%d\n",value);

	return 0;
}
#endif




