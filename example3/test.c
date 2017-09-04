#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include "inlineHook.h"
int my_init(void) __attribute__((constructor));
char * getfilemac();
typedef int (*t_system_property_get)(const char *name, char *value);
t_system_property_get __system_property_get=NULL;
int (*old__system_property_get)(const char *name, char *value)=NULL;
int new__system_property_get(const char *name, char *value)
{
if(strstr(name,"ro.serialno")) return strlen(strcpy(value,getfilemac()));
    else return old__system_property_get(name,value);
}

typedef int (*t_fopen)(const char *file, const char *mode);
t_fopen _fopen = NULL;
void (*old_fopen)(const char *file, const char *mode)=NULL;
void new_fopen(const char *file, const char *mode)
{
	if(strstr(file,"/sys/class/net/wlan0/address")) return old_fopen("/sdcard/xx/file/address","r");
	else return old_fopen(file,mode);
}

int my_init(void)
{
void* libc=dlopen("/system/lib/libc.so",0);
__system_property_get=(t_system_property_get)dlsym(libc,"__system_property_get");
if(registerInlineHook((uint32_t)__system_property_get,(uint32_t) new__system_property_get,(uint32_t **) &old__system_property_get)!= ELE7EN_OK) printf("error find __system_property_get ");
else if (inlineHook((uint32_t) __system_property_get) != ELE7EN_OK) printf("error hook __system_property_get ");

_fopen = (t_fopen)dlsym(libc,"fopen");
if(registerInlineHook((uint32_t)_fopen,(uint32_t) new_fopen,(uint32_t **) &old_fopen)!=ELE7EN_OK) printf("error find fopen");
else if(inlineHook((uint32_t) _fopen)!=ELE7EN_OK) printf("error hook fopen");
return 0;
}
char * getfilemac(){
	FILE *fp = NULL;
	static char buff[255];
	fp = fopen("/sdcard/xx/file/param","r");
	if(fp != NULL){
		fgets(buff, 255, (FILE*)fp);
		return buff;
	}
	return "null=="+(fp==NULL);
}