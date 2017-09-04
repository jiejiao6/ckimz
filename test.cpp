#include "substrate.h"
#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
 
#define BUFLEN 1024
#define TAG "DEXDUMP"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
 
//get packagename from pid
int getProcessName(char * buffer){
    char path_t[256]={0};
    pid_t pid=getpid();
    char str[15];
    sprintf(str, "%d", pid);
    memset(path_t, 0 , sizeof(path_t));
    strcat(path_t, "/proc/");
    strcat(path_t, str);
    strcat(path_t, "/cmdline");
    //LOG_ERROR("zhw", "path:%s", path_t);
    int fd_t = open(path_t, O_RDONLY);
    if(fd_t>0){
        int read_count = read(fd_t, buffer, BUFLEN);
 
        if(read_count>0){
              int  processIndex=0;
              for(processIndex=0;processIndex<strlen(buffer);processIndex++){
                  if(buffer[processIndex]==‘:‘){
                      buffer[processIndex]=‘_‘;
                  }
 
              }
            return 1;
        }
    }
    return 0;
}
 
//指定要hook 的 lib 库
MSConfig(MSFilterLibrary,"/system/lib/libdvm.so")
 
//保留原来的地址  DexFile* dexFileParse(const u1* data, size_t length, int flags)
int (* oldDexFileParse)(const void * addr,int len,int flags);
 
//替换的函数
int myDexFileParse(const void * addr,int len,void ** dvmdex)
{
    LOGD("call my dvm dex!!:%d",getpid());
 
    {
        //write to file
        //char buf[200];
        // 导出dex文件
        char dexbuffer[64]={0};
        char dexbufferNamed[128]={0};
        char * bufferProcess=(char*)calloc(256,sizeof(char));
        int  processStatus= getProcessName(bufferProcess);
        sprintf(dexbuffer, "_dump_%d", len);
        strcat(dexbufferNamed,"/sdcard/");
        if (processStatus==1) {
          strcat(dexbufferNamed,bufferProcess);
            strcat(dexbufferNamed,dexbuffer);
 
        }else{
            LOGD("FAULT pid not  found\n");
        }
 
        if(bufferProcess!=NULL)
        {
 
          free(bufferProcess);
        }
 
        strcat(dexbufferNamed,".dex");
 
        //sprintf(buf,"/sdcard/dex.%d",len);
        FILE * f=fopen(dexbufferNamed,"wb");
        if(!f)
        {
            LOGD(dexbuffer + " : error open sdcard file to write");
        }
        else{
            fwrite(addr,1,len,f);
            fclose(f);
        }
 
 
 
    }
    //进行原来的调用,不影响程序运行
    return oldDexFileParse(addr,len,dvmdex);
}
 
//Substrate entry point
MSInitialize
{
    LOGD("Substrate initialized.");
    MSImageRef image;
    //载入lib
    image = MSGetImageByName("/system/lib/libdvm.so");
    if (image != NULL)
    {
 
        void * dexload=MSFindSymbol(image,"_Z21dvmDexFileOpenPartialPKviPP6DvmDex");
        if(dexload==NULL)
        {
            LOGD("error find _Z21dvmDexFileOpenPartialPKviPP6DvmDex ");
 
        }
        else{
            //替换函数
            //3.MSHookFunction
            MSHookFunction(dexload,(void*)&myDexFileParse,(void **)&oldDexFileParse);
        }
    }
    else{
        LOGD("ERROR FIND LIBDVM");
    }
}