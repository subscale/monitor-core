/*******************************************************************************
* Copyright (C) 2007 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Novell, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Author: Brad Nicholes (bnicholes novell.com)
******************************************************************************/

/*
 * The ganglia metric "C" interface, required for building DSO modules.
 */
#include <gm_metric.h>

#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
 * Declare ourselves so the configuration routines can find and know us.
 * We'll fill it in at the end of the module.
 */
extern mmodule cppmr_module;

static unsigned int random_max = 50;
static unsigned int constant_value = 20;

static key_t key = 7294192;
static int shmid = -1;
#define SHMSZ     1024

struct cppmr_metrics
{
    uint32_t version;
    uint32_t input_files_found;
    uint32_t input_files_read;
};

static const struct cppmr_metrics *shm = NULL;
static void open_shm()
{
    shm = NULL;
    shmid = -1;
    if (shmid > 0)
        return;
    
    shmid = shmget(key, SHMSZ, 0666);
    if (shmid < 0)
    {
        debug_msg("[mod_cppmr]Can't open shared memory handle");
        return;
    }
    shm = (const struct cppmr_metrics*)shmat(shmid, NULL, 0);
    if (shm == (const struct cppmr_metrics*)-1)
    {
        shmid = -1;
        shm = NULL;
        debug_msg("[mod_cppmr]Can't map shared memory handle");
        return;
    } 
}
static int ex_metric_init ( apr_pool_t *p )
{
    const char* str_params = cppmr_module.module_params;
    apr_array_header_t *list_params = cppmr_module.module_params_list;
    mmparam *params;
    int i;

    srand(time(NULL)%99);


    for (i = 0; cppmr_module.metrics_info[i].name != NULL; i++) {
        /* Initialize the metadata storage for each of the metrics and then
         *  store one or more key/value pairs.  The define MGROUPS defines
         *  the key for the grouping attribute. */
        MMETRIC_INIT_METADATA(&(cppmr_module.metrics_info[i]),p);
        MMETRIC_ADD_METADATA(&(cppmr_module.metrics_info[i]),MGROUP,"CPPMR");
    }
    debug_msg("[mod_cppmr]registered %d metrics", i);
    return 0;
}

static void ex_metric_cleanup ( void )
{
}

static g_val_t ex_metric_handler ( int metric_index )
{
    g_val_t val;

    /* The metric_index corresponds to the order in which
       the metrics appear in the metric_info array
    */
    debug_msg("[mod_cppmr]Collecting %d metric", metric_index);

    if (!shm)
        open_shm();
    if (!shm || shm->version != 1)
    {
        val.f = 0;
        return val;
    }
    
    switch (metric_index) {
    case 0:
        val.uint32 = shm->input_files_found;    
        shm->input_files_found = 0;
        break;
    case 1:
        val.uint32 = shm->input_files_read;    
        shm->input_files_read = 0;
        break;
    default:
        val.uint32 = 0;
        
    return val;
}

static Ganglia_25metric ex_metric_info[] = 
{
    {0, "input_files_found",      20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Number of input files discovered"},
    {0, "input_files_read",       20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Number of input files successfully processed"},
    {0, NULL}
};

mmodule mux_module =
{
    STD_MMODULE_STUFF,
    ex_metric_init,
    ex_metric_cleanup,
    ex_metric_info,
    ex_metric_handler,
};
