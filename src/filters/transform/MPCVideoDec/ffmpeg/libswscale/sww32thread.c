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
    SwsThreadContext *tc = v;

    for(;;){
//printf("sws_thread_func %X enter wait\n", (int)v); fflush(stdout);
        WaitForSingleObject(tc->work_sem, INFINITE);
//printf("sws_thread_func %X after wait (func=%X)\n", (int)v, (int)c->func); fflush(stdout);
        if(tc->func)
            tc->ret = tc->func(tc->swsctx);
        else
            return 0;
//printf("sws_thread_func %X signal complete\n", (int)v); fflush(stdout);
        ReleaseSemaphore(tc->done_sem, 1, 0);
    }

    return 0;
}

/**
 * Free what has been allocated by sws_thread_init().
 * Must be called after decoding has finished, especially do not call while sws_thread_execute() is running
 */
void sws_thread_free(SwsContext *c){
    SwsThreadContext *tc = c->thread_opaque;
    int i;

    for(i=0; i<c->thread_count; i++){

        tc[i].func = NULL;
        ReleaseSemaphore(tc[i].work_sem, 1, 0);
        WaitForSingleObject(tc[i].thread, INFINITE);
        if(tc[i].work_sem) CloseHandle(tc[i].work_sem);
        if(tc[i].done_sem) CloseHandle(tc[i].done_sem);
        if(tc[i].thread)   CloseHandle(tc[i].thread); 
    }

    av_freep(&c->thread_opaque);
}

int sws_thread_execute(SwsContext *c, int (*func)(SwsContext *c2), int *ret, int count){ //CUSTOMIZED no void **arg
    SwsThreadContext *tc= c->thread_opaque;
    int i;

    assert(c == tc->swsctx);
    assert(count <= c->thread_count);

    /* note, we can be certain that this is not called with the same SwsContext by different threads at the same time */

    for(i=0; i<count; i++){
        tc[i].arg = &c[i].stp;//CUSTOMIZED
        tc[i].func= func;
        tc[i].ret = 12345;

        ReleaseSemaphore(tc[i].work_sem, 1, 0);
    }
    for(i=0; i<count; i++){
        WaitForSingleObject(tc[i].done_sem, INFINITE);

        tc[i].func= NULL;
        if(ret) ret[i]= tc[i].ret;
    }
    return 0;
}

int sws_thread_init(SwsContext *c){
    int i;
    SwsThreadContext *tc;
    uint32_t threadid;

    assert(!c->thread_opaque);
    tc = av_mallocz(sizeof(SwsThreadContext) * c->thread_count);
    c->thread_opaque = tc;

    for(i=0; i<c->thread_count; i++){
//printf("init semaphors %d\n", i); fflush(stdout);
        tc[i].swsctx= &c[i]; //CUSTOMIZED

        if(!(tc[i].work_sem = CreateSemaphore(NULL, 0, c->thread_count, NULL)))
            goto fail;
        if(!(tc[i].done_sem = CreateSemaphore(NULL, 0, c->thread_count, NULL)))
            goto fail;

//printf("create thread %d\n", i); fflush(stdout);
        tc[i].thread = (HANDLE)_beginthreadex(NULL, 0, sws_thread_func, &tc[i], 0, &threadid );
        if( !tc[i].thread ) goto fail;
    }
//printf("init done\n"); fflush(stdout);

    c->execute= sws_thread_execute;

    return 0;
fail:
    sws_thread_free(c);
    return -1;
}
