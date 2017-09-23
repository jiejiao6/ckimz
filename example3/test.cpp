#include <stdio.h>
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/types.h>  
#include <android/log.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include "inlineHook.h"
#include "headers/tinyxml2.h"
#define TAGF "HOOKFILETEST"
#define TAG "HOOKTEST"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGDF(...) __android_log_print(ANDROID_LOG_DEBUG, TAGF, __VA_ARGS__)

#define CAT_PATH "/sdcard/xx/file/native/cat.xml"

#define SETTING_PATH "/sdcard/xx/file/native/setting.xml"

#define REAL_WLAN_PATH "sys/class/net/wlan0/address"
#define FAKE_WLAN_PATH "/sdcard/xx/file/native/wlan0"

#define REAL_DUMMY_PATH "sys/class/net/dummy0/address"
#define FAKE_DUMMY_PATH "/sdcard/xx/file/native/dummy0"

#define REAL_P2P_PATH "sys/class/net/p2p0/address"
#define FAKE_P2P_PATH "/sdcard/xx/file/native/p2p0"

#define REAL_PPP_PATH "sys/class/net/ppp0/address"
#define FAKE_PPP_PATH "/sdcard/xx/file/native/ppp0"

#define REAL_USBNET_PATH "sys/class/net/usbnet0/address"
#define FAKE_USBNET_PATH "/sdcard/xx/file/native/usbnet0"

#define REAL_CID_PATH1 "sys/block/mmcblk0/device/cid"
#define REAL_CID_PATH2 "sys/block/mmcblk0boot0/device/device/cid"
#define REAL_CID_PATH3 "sys/block/mmcblk0boot1/device/device/cid"
#define FAKE_CID_PATH "/sdcard/xx/file/native/cid"

#define REAL_CSD_PATH1 "sys/block/mmcblk0/device/csd"
#define REAL_CSD_PATH2 "sys/block/mmcblk0boot0/device/device/csd"
#define REAL_CSD_PATH3 "sys/block/mmcblk0boot1/device/device/csd"
#define FAKE_CSD_PATH "/sdcard/xx/file/native/csd"

#define REAL_IF_INET6_PATH "proc/net/if_inet6"
#define FAKE_IF_INET6_PATH "/sdcard/xx/file/native/if_inet6"

#define REAL_PROC_VERSION "proc/version"
#define FAKE_PROC_VERSION "/sdcard/xx/file/native/version"

int my_init(void) __attribute__((constructor));
//char * getfilemac();
bool ishook();
bool ishook3();
int GetPropValue(const char* name, char *value);
static char AppName[256] = { 0 };
static char ishookflag[100] = { 0 };
char* GetAppName() {
	FILE *fp = NULL;
	char path[256] = { 0 };
	pid_t pid = getpid();
	char str[15];
	sprintf(str, "%d", pid);
	memset(path, 0, sizeof(path));
	strcat(path, "/proc/");
	strcat(path, str);
	strcat(path, "/cmdline");
	fp = fopen(path, "r");
	if (fp) {
		memset(AppName, sizeof(AppName), 0);
		fread(&AppName, sizeof(AppName), 1, fp);
		fclose(fp);
	}
	return AppName;
}
bool GetCatValue(char *key, char *value) {
	using namespace tinyxml2;
	XMLDocument *doc = new XMLDocument();
	XMLError err = doc->LoadFile(CAT_PATH);
	if (err == XML_SUCCESS) {
		XMLElement *map = doc->FirstChildElement("map");
		if (!map->NoChildren()) {
			XMLElement *ele = map->FirstChildElement();
			const char *result = NULL;
			while (ele) {
				const char *attr = ele->Attribute("name");
				if (strstr(key, attr)) {
					const char *r = ele->GetText();
					result = r;
					strcpy(value, result);
					//LOGD("GetCatValue key:%s,value:%s", key, value);
					break;
				}
				ele = ele->NextSiblingElement();
			}
			if (result) {
			} else {
				return false;
			}
		}
	} else {
		return false;
	}
	delete doc;
	return true;
}
bool GetSettingValue(char *key,char *value,const char *path){
	using namespace tinyxml2;
	XMLDocument *doc = new XMLDocument();
	XMLError err = doc->LoadFile(path);
	if (err == XML_SUCCESS) {
		XMLElement *map = doc->FirstChildElement("map");
		if (!map->NoChildren()) {
			XMLElement *ele = map->FirstChildElement();
			const char *result = NULL;
			while (ele) {
				const char *attr = ele->Attribute("name");
				if (strstr(key, attr)) {
					const char *r = ele->GetText();
					result = r;
					strcpy(value, result);
					//LOGD("GetCatValue key:%s,value:%s", key, value);
					break;
				}
				ele = ele->NextSiblingElement();
			}
			if (result) {
			} else {
				return false;
			}
		}
	} else {
		return false;
	}
	delete doc;
	return true;
}
bool SetPropResultValue(char *value, const char* format, ...) {
	va_list ap;
	int n=0,size=1000;
	va_start(ap, format);
	n = vsnprintf(value,size,format,ap);
	va_end(ap);
	//LOGD("79:value::%s format:%s ", value, format);
	return true;
}
typedef int (*t_system_property_get)(const char *name, char *value);
t_system_property_get __system_property_get=NULL;
int (*old__system_property_get)(const char *name, char *value)=NULL;
int new__system_property_get(const char *name, char *value)
{
	GetAppName();
	GetSettingValue("key_ishook_native",ishookflag,SETTING_PATH);
	int result = old__system_property_get(name, value);
	if(!ishook()){
		return result;
	}
	int PropResult = GetPropValue(name, value);
	if (PropResult > 0)
		return result = PropResult;
	return result;

}


typedef int (*t_fopen)(const char *file, const char *mode);
t_fopen _fopen = NULL;
void (*old_fopen)(const char *file, const char *mode)=NULL;
void new_fopen(const char *file, const char *mode)
{	
	if(strstr(file,SETTING_PATH)){
		return old_fopen(file,mode);
	}
	if(strstr(AppName,"com.donson.leplay.store2")){
		if(!strstr(file,"sdcard/xx/file/native")&&!strstr(file,"cmdline")){
		LOGDF("new_open:new_fopen:file:%s mode:%d ", file, mode);
		}
	}
	if(!ishook())
		return old_fopen(file,mode);
	//if(!strstr(file,"/cmdline"))
	//	LOGD("new_fopen-----------:value::%s format:%s ", file, mode);
	if(strstr(file,REAL_WLAN_PATH)) return old_fopen(FAKE_WLAN_PATH,mode);
	else if(strstr(file,REAL_DUMMY_PATH)) return old_fopen(FAKE_DUMMY_PATH,mode);
	else if(strstr(file,REAL_P2P_PATH)) return old_fopen(FAKE_P2P_PATH,mode);
	else if(strstr(file,REAL_PPP_PATH)) return old_fopen(FAKE_PPP_PATH,mode);
	else if(strstr(file,REAL_USBNET_PATH)) return old_fopen(FAKE_USBNET_PATH,mode);
	else if(strstr(file,REAL_CID_PATH1)) return old_fopen(FAKE_CID_PATH,mode);
	else if(strstr(file,REAL_CID_PATH2)) return old_fopen(FAKE_CID_PATH,mode);
	else if(strstr(file,REAL_CID_PATH3)) return old_fopen(FAKE_CID_PATH,mode);
	else if(strstr(file,REAL_CSD_PATH1)) return old_fopen(FAKE_CSD_PATH,mode);
	else if(strstr(file,REAL_CSD_PATH2)) return old_fopen(FAKE_CSD_PATH,mode);
	else if(strstr(file,REAL_CSD_PATH3)) return old_fopen(FAKE_CSD_PATH,mode);
	else if(strstr(file,REAL_IF_INET6_PATH))return old_fopen(FAKE_IF_INET6_PATH, mode);
	else if(strstr(file,REAL_PROC_VERSION))return old_fopen(FAKE_PROC_VERSION, mode);
	else return old_fopen(file,mode);
}
typedef int (*t_open)(const char *file, const int *mode);
t_open _open = NULL;
int (*old_open)(const char* path, int mode);
int new_open(const char* file, int mode) {
	//
	//return old_open(file,mode);
	if(strstr(file,SETTING_PATH)){
			return old_open(file,mode);
	}
	if(strstr(AppName,"com.donson.leplay.store2")){
		if(!strstr(file,"sdcard/xx/file/native")&&!strstr(file,"cmdline")){
		LOGDF("new_open:new_open:file:%s mode:%d ", file, mode);
		}
	}
	if(!ishook3())
		return old_open(file,mode);
	//else
	//LOGD("new_open:new_open:file:%s mode:%d appNAme:%s", file, mode,AppName);
	//return old_open(file,mode);
		//if(!strstr(file,"/cmdline"))
		//	LOGD("new_fopen-----------:value::%s format:%s ", file, mode);
		if(strstr(file,REAL_WLAN_PATH)) return old_open(FAKE_WLAN_PATH,mode);
		else if(strstr(file,REAL_DUMMY_PATH))
			return old_open(FAKE_DUMMY_PATH,mode);
		else if(strstr(file,REAL_P2P_PATH)) return old_open(FAKE_P2P_PATH,mode);
		else if(strstr(file,REAL_PPP_PATH)) return old_open(FAKE_PPP_PATH,mode);
		else if(strstr(file,REAL_USBNET_PATH)) return old_open(FAKE_USBNET_PATH,mode);
		else if(strstr(file,REAL_CID_PATH1)) return old_open(FAKE_CID_PATH,mode);
		else if(strstr(file,REAL_CID_PATH2)) return old_open(FAKE_CID_PATH,mode);
		else if(strstr(file,REAL_CID_PATH3)) return old_open(FAKE_CID_PATH,mode);
		else if(strstr(file,REAL_CSD_PATH1)) return old_open(FAKE_CSD_PATH,mode);
		else if(strstr(file,REAL_CSD_PATH2)) return old_open(FAKE_CSD_PATH,mode);
		else if(strstr(file,REAL_CSD_PATH3)) return old_open(FAKE_CSD_PATH,mode);
		else if(strstr(file,REAL_IF_INET6_PATH))return old_open(FAKE_IF_INET6_PATH, mode);
		else if(strstr(file,REAL_PROC_VERSION))return old_open(FAKE_PROC_VERSION, mode);
		else return old_open(file,mode);
}

int my_init(void)
{
void* libc=dlopen("/system/lib/libc.so",0);
__system_property_get=(t_system_property_get)dlsym(libc,"__system_property_get");
if(registerInlineHook((uint32_t)__system_property_get,(uint32_t) new__system_property_get,(uint32_t **) &old__system_property_get)!= ELE7EN_OK) printf("error find __system_property_get ");
else if (inlineHook((uint32_t) __system_property_get) != ELE7EN_OK) printf("error hook __system_property_get ");

_fopen = (t_fopen)dlsym(libc,"fopen");
if(registerInlineHook((uint32_t)_fopen,(uint32_t) new_fopen,(uint32_t **) &old_fopen)!=ELE7EN_OK) 
	printf("error find fopen");
else if(inlineHook((uint32_t) _fopen)!=ELE7EN_OK) printf("error hook fopen");

_open = (t_open)dlsym(libc,"open");
if(registerInlineHook((uint32_t)_open,(uint32_t) new_open,(uint32_t **) &old_open)!=ELE7EN_OK) 
LOGD("error hook fopen");
//printf("error find open");
else if(inlineHook((uint32_t) _open)!=ELE7EN_OK) LOGD("error hook fopen");//printf("error hook open");

return 0;
}

char brand[100] = {};
bool isBrand = false;
char model[100]={};
bool isModel = false;
int GetPropValue(const char* name, char *value) {
	if(strstr(AppName,"com.donson.leplay.store2")){
		LOGD("GetPropValue:%s,%s",name,value);
	}
	if (!name)
		return -1;
	bool bGet = false;
	if(!isBrand){
		isBrand = true;
		GetCatValue("brand",brand);
		//LOGD("38:%s ", brand);
	}
	if(!isModel){
		isModel = true;
		GetCatValue("model",model);
	}

//    if(!NeedHook()) return -1;
//    if(strstr(name,"ro.super.version")) bGet=SetPropResultValue(value,SUPERVERSION);
	if (strstr(name, "ro.product.model"))
		bGet = GetCatValue("model", value);
	else if (strstr(name, "ro.serialno"))
		bGet = GetCatValue("serial", value);
	else if (strstr(name, "ro.boot.serialno"))
		bGet = GetCatValue("serial", value);
	//else if (strstr(name, "ro.board.platform"))
	//	bGet = GetCatValue("board", value);
	else if (strstr(name, "ro.build.version.release"))
		bGet = GetCatValue("version", value);
	else if (strstr(name, "ro.product.manufacturer"))
		bGet = GetCatValue("manufacturer", value);
	else if (strstr(name, "ro.product.name"))
		bGet = GetCatValue("product", value);
	else if (strstr(name, "ro.hardware"))
		bGet = GetCatValue("hardware", value);
	else if (strstr(name, "ro.product.board"))
		bGet = GetCatValue("board", value);
	else if (strstr(name, "ro.product.brand"))
		bGet = GetCatValue("brand", value);
	else if (strstr(name, "ro.build.custom.display.id"))
		bGet = GetCatValue("id", value);
	else if (strstr(name, "ro.build.display.id"))
		bGet = GetCatValue("id", value);
	else if (strstr(name, "ro.product.device"))
		bGet = GetCatValue("device", value);
	else if (strstr(name, "ro.build.fingerprint"))
		bGet = GetCatValue("fingerprint", value);
	else if (strstr(name, "ro.build.description"))
		bGet = GetCatValue("description", value);
	else if (strstr(name, "ro.mediatek.version.release"))
		bGet = GetCatValue("display", value);
	else if (strstr(name, "ro.build.product"))
		bGet = GetCatValue("product", value);
	else if (strstr(name, "ro.build.internal.display.id"))
		bGet = GetCatValue("id", value);
	else if (strstr(name, "persist.radio.cfu.iccid.0"))
		bGet = GetCatValue("simserial", value);
	else if (strstr(name, "ril.iccid.sim1"))
		bGet = GetCatValue("simserial", value);
	else if (strstr(name, "persist.radio.imei"))
		bGet = GetCatValue("imei", value);
	else if (strstr(name, "ro.ril.oem.imei"))
		bGet = GetCatValue("imei", value);
	else if (strstr(name, "ro.ril.oem.sno"))
		bGet = GetCatValue("sno", value);
	else if (strstr(name, "ro.ril.oem.meid"))
		bGet = GetCatValue("meid", value);
	else if (strstr(name, "persist.radio.meid"))
		bGet = GetCatValue("meid", value);
    else if(strstr(name,"gsm.sim.state")) bGet = GetCatValue("simstate", value);//bGet=SetPropResultValue(value,"READY");
	else if (strstr(name, "gsm.operator.alpha"))
		bGet = GetCatValue("simoperator", value);
	else if (strstr(name, "gsm.sim.operator.alpha"))
		bGet = GetCatValue("networkoperatorname", value);
	else if (strstr(name, "gsm.operator.numeric"))
		bGet = GetCatValue("simoperator", value);
	else if (strstr(name, "permanent.radio.modem"))
		bGet = GetCatValue("radiomodem", value);
    else if(strstr(name,"persist.radio.modem")) bGet = GetCatValue("radiomodem", value);//bGet=SetPropResultValue(value,"TD");
	else if (strstr(name, "ro.build.host"))
		bGet = GetCatValue("host", value);
	else if (strstr(name, "ro.ril.miui.imei"))
		bGet = GetCatValue("imei", value);
	else if (strstr(name, "ro.runtime.firstboot"))
		bGet = GetCatValue("firstboot", value);
	else if (strstr(name, "persist.sys.xtrainject.time"))
		bGet = GetCatValue("buildtime", value);
    else if(strstr(name,"ro.ckis")) bGet=SetPropResultValue(value,"OK!");
	else if (strstr(name, "ro.ckis.model"))
		bGet = GetCatValue("model", value);
	else if (strstr(name, "ro.ckis.apilevel"))
		bGet = GetCatValue("apilevel", value);
	else if (strstr(name, "ro.build.date.utc"))
		bGet = GetCatValue("buildtime", value);
	
    else if(strstr(name,"ro.product.cuptsm")) bGet=SetPropResultValue(value,"%s|ESE|02|01",brand);
    else if(strstr(name,"gsm.version.ril-impl")) bGet=SetPropResultValue(value,"android %s-ril for %s 1.1",brand,brand);
    else if(strstr(name,"rild.libpath")) bGet=SetPropResultValue(value,"/system/lib/libril-%s-sprd.so",brand);
    else if(strstr(name,"persist.sys.NV_PROFILE_MODEL")) bGet=SetPropResultValue(value,model);
	else if (strstr(name, "persist.service.bdroid.bdaddr"))
		bGet = GetCatValue("bluemac", value);
	else if (strstr(name, "persist.sys.klo.rec_start"))
		bGet = GetCatValue("firstboot", value);
    else if(strstr(name,"net.hostname")) bGet=SetPropResultValue(value,"%s-Android",model);
	else if (strstr(name, "ril.timediff"))
		bGet = GetCatValue("timediff", value);
	else if (strstr(name, "gsm.version.baseband"))
		bGet = GetCatValue("radioversion", value);
	else if (strstr(name, "ro.build.version.sdk"))
		bGet = GetCatValue("apilevel", value);
	else if (strstr(name, "ro.build.id"))
		bGet = GetCatValue("id", value);
    else if(strstr(name,"dhcp.wlan0.dns1") || strstr(name,"dhcp.wlan0.gateway") || strstr(name,"dhcp.wlan0.server") || strstr(name,"net.dns1"))
//        bGet=SetPropResultValue(value,"192.168.%d.%d",GetRand(nSeed^0x1111,0,254),GetRand(nSeed^0x2222,1,254));
    	bGet = GetCatValue("dns",value);
		
    else if(strstr(name,"dhcp.wlan0.ipaddress"))
    	bGet = GetCatValue("innerip",value);
//    	bGet=SetPropResultValue(value,"192.168.%d.%d",GetRand(nSeed^0x3333,0,254),GetRand(nSeed^0x4444,1,254));
    else if(strstr(name,"dhcp.wlan0.dns2") || strstr(name,"net.dns2"))
//    	bGet=SetPropResultValue(value,"192.168.%d.%d",GetRand(nSeed^0x5555,0,254),GetRand(nSeed^0x6666,1,254));
    	bGet = GetCatValue("dns",value);
	else if (strstr(name, "persist.sys.NV_DISPXRES"))
		bGet = GetCatValue("swidth", value);
	else if (strstr(name, "persist.sys.NV_DISPYRES"))
		bGet = GetCatValue("sheight", value);
	else if (strstr(name,"ro.build.characteristics")) //default or nosdcard
		bGet = GetCatValue("characteristics", value);
	else if (strstr(name,"ro.build.date"))
		bGet = GetCatValue("builddate",value);
	//else if (strstr(name,"gsm.network.type"))
	//	bGet = GetCatValue("",value);
	else if (strstr(name,"rild.libargs"))
		bGet=SetPropResultValue(value,"");
	//else if (strstr(name,"gsm.serial"))
	//	bGet = GetCatValue("",value);
	else if (strstr(name,"persist.radio.data.iccid"))
		bGet = GetCatValue("simserial",value);
	else if (strstr(name,"ro.mediatek.project.path"))
		bGet=SetPropResultValue(value,"device/%s/%model",brand,model);
	else if(strstr(name,"ro.miui.mcc"))
		bGet=SetPropResultValue(value,"");
	else if(strstr(name,"ro.miui.mnc"))
		bGet=SetPropResultValue(value,"");
	else if(strstr(name,"ro.ril.miui.imei0"))
		bGet=SetPropResultValue(value,"");
	else if(strstr(name,"ro.miui.has_real_blur"))
		bGet=SetPropResultValue(value,"");
	else if(strstr(name,"ro.opengles.version"))
		bGet=SetPropResultValue(value,"196608");
		
/**/
	int result = -1;
	if (bGet) {
		result = strlen(value);
		//LOGD("GetPropValue::: Call:%s|%s|%d", name, value, result);
	}
	return result;
}
bool ishook2(){
	static char ishookflag[100] = { 0 };
	bool bGet = GetSettingValue("key_ishook_native",ishookflag,SETTING_PATH);
	//bool bGet = GetCatValue("ishook",ishookflag);
	if(bGet){
		if(strstr(ishookflag,"true")){
			return true;
		}else
			return false;
	}
	return false;
	
}

bool ishook(){
	//LOGD("ishook::: AppName Call:%s:", AppName);
	
	//bool bGet = GetSettingValue("key_ishook_native",ishookflag,SETTING_PATH);
	//LOGD("ishook:::ishookflag Call:%s|%s:", ishookflag,AppName);
	if((getuid()<=1000)&&(!strstr(AppName,"getprop"))){
		return false;
	}else if (strstr(AppName,"org.wuji")||strstr(AppName,"com.donson.xxxiugaiqi")){
		return false;
	}else
		return ishook2();
}
bool ishook3(){
	if((getuid()<=1000)&&(!strstr(AppName,"getprop"))){
		return false;
	}else if (strstr(AppName,"org.wuji")||strstr(AppName,"com.donson.xxxiugaiqi")){
		return false;
	}else
		return true;
}
