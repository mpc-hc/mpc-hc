/*
 * Copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Customized for swscaler by Haruhiko Yamagata
 */

//#define DEBUG
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

#include <inttypes.h>
#include "config.h"
#include "../libavutil/internal.h"
#include "swscale.h"
#include "swscale_internal.h"


typedef struct SwsThreadContext{
    SwsContext *swsctx;
    HANDLE thread;
    HANDLE work_sem;
    HANDLE done_sem;
    int (*func)(SwsContext *c);
    void *arg;
    int ret;
}SwsThreadContext;


static unsigned __stdcall sws_thread_func(void *v){
    SwsThreadContext *c= v;

    for(;;){
//printf("sws_thread_func %X enter wait\n", (int)v); fflush(stdout);
        WaitForSingleObject(c->work_sem, INFINITE);
//printf("sws_thread_func %X after wait (func=%X)\n", (int)v, (int)c->func); fflush(stdout);
        if(c->func)
            c->ret= c->func(c->swsctx);
        else
            return 0;
//printf("sws_thread_func %X signal complete\n", (int)v); fflush(stdout);
        ReleaseSemaphore(c->done_sem, 1, 0);
    }

    return 0;
}

/**
 * Free what has been allocated by sws_thread_init().
 * Must be called after decoding has finished, especially do not call while sws_thread_execute() is running
 */
void sws_thread_free(SwsContext *s){
    SwsThreadContext *c= s->thread_opaque;
    int i;

    for(i=0; i<s->thread_count; i++){

        c[i].func= NULL;
        ReleaseSemaphore(c[i].work_sem, 1, 0);
        WaitForSingleObject(c[i].thread, INFINITE);
        if(c[i].work_sem) CloseHandle(c[i].work_sem);
        if(c[i].done_sem) CloseHandle(c[i].done_sem);
        if(c[i].thread)   CloseHandle(c[i].thread); 
    }

    av_freep(&s->thread_opaque);
}

int sws_thread_execute(SwsContext *s, int (*func)(SwsContext *c2), int *ret, int count){ //CUSTOMIZED no void **arg
    SwsThreadContext *c= s->thread_opaque;
    int i;

    assert(s == c->swsctx);
    assert(count <= s->thread_count);

    /* note, we can be certain that this is not called with the same SwsContext by different threads at the same time */

    for(i=0; i<count; i++){
        c[i].arg = &s[i].stp;//CUSTOMIZED
        c[i].func= func;
        c[i].ret = 12345;

        ReleaseSemaphore(c[i].work_sem, 1, 0);
    }
    for(i=0; i<count; i++){
        WaitForSingleObject(c[i].done_sem, INFINITE);

        c[i].func= NULL;
        if(ret) ret[i]= c[i].ret;
    }
    return 0;
}

int sws_thread_init(SwsContext *s, int thread_count){
    int i;
    SwsThreadContext *c;
    uint32_t threadid;

    s->thread_count= thread_count;

    assert(!s->thread_opaque);
    c= av_mallocz(sizeof(SwsThreadContext)*thread_count);
    s->thread_opaque= c;

    for(i=0; i<thread_count; i++){
//printf("init semaphors %d\n", i); fflush(stdout);
        c[i].swsctx= &s[i]; //CUSTOMIZED

        if(!(c[i].work_sem = CreateSemaphore(NULL, 0, s->thread_count, NULL)))
            goto fail;
        if(!(c[i].done_sem = CreateSemaphore(NULL, 0, s->thread_count, NULL)))
            goto fail;

//printf("create thread %d\n", i); fflush(stdout);
        c[i].thread = (HANDLE)_beginthreadex(NULL, 0, sws_thread_func, &c[i], 0, &threadid );
        if( !c[i].thread ) goto fail;
    }
//printf("init done\n"); fflush(stdout);

    s->execute= sws_thread_execute;

    return 0;
fail:
    sws_thread_free(s);
    return -1;
}

int GetCPUCount(void){
    int CPUCount;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if(isP4HT() &&si.dwNumberOfProcessors>=2)
        CPUCount = si.dwNumberOfProcessors>>1;
    else
        CPUCount= si.dwNumberOfProcessors;
    return CPUCount;
}
