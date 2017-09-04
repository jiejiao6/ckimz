#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include "inlineHook.h"
int my_init(void) __attribute__((constructor));
typedef int (*t_system_property_get)(const char *name, char *value);
t_system_property_get __system_property_get=NULL;
int (*old__system_property_get)(const char *name, char *value)=NULL;
int new__system_property_get(const char *name, char *value)
{
if(strstr(name,"ro.serialno")) return strlen(strcpy(value,"AABBCC"));
    else return old__system_property_get(name,value);
}
int my_init(void)
{
void* libc=dlopen("/system/lib/libc.so",0);
__system_property_get=(t_system_property_get)dlsym(libc,"__system_property_get");
if(registerInlineHook((uint32_t)__system_property_get,(uint32_t) new__system_property_get,(uint32_t **) &old__system_property_get)!= ELE7EN_OK) printf("error find __system_property_get ");
else if (inlineHook((uint32_t) __system_property_get) != ELE7EN_OK) printf("error hook __system_property_get ");
return 0;
}