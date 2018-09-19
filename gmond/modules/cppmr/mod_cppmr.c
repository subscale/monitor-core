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

static key_t key = 7291192;
static int shmid = -1;
#define SHMSZ     1024

struct cppmr_metrics
{
    uint32_t version;
    uint32_t input_files_found;
    uint32_t input_files_read;
};

static struct cppmr_metrics *shm = NULL;
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
    shm = (struct cppmr_metrics*)shmat(shmid, NULL, 0);
    if (shm == (struct cppmr_metrics*)-1)
    {
        shmid = -1;
        shm = NULL;
        debug_msg("[mod_cppmr]Can't map shared memory handle");
        return;
    } 
}
static int ex_metric_init ( apr_pool_t *p )
{
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
    }
        
    return val;
}

static Ganglia_25metric ex_metric_info[] = 
{
    {0, "input_files_found",      20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Number of input files discovered"},
    {0, "input_files_read",       20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Number of input files successfully processed"},
    {0, "input_bps",              20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Read bytes/sec"},
    {0, "input_bytes",            20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Read bytes"},
    {0, "temp_bps",               20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Temp bytes/sec"},
    {0, "temp_bytes",             20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Temp bytes"},
    {0, "temp_files",             20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Temp files count"},
    {0, "temp_chunks",            20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Temp chunks count"},
    {0, "output_files",           20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Output files count"},
    {0, "reduce_bps",             20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Reduce bytes/sec"},
    {0, "reduce_bytes",           20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Reduce bytes"},
    {0, "output_bps",             20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Output bytes/sec"},
    {0, "output_bytes",           20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Output bytes"},
    {0, "http_in_bps",            20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "HTTP IN bytes/sec"},
    {0, "http_in_bytes",          20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "HTTP IN bytes"},
    {0, "http_out_bps",           20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "HTTP OUT bytes/sec"},
    {0, "http_out_bytes",         20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "HTTP OUT bytes"},
    {0, "rows_in",                20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows read"},
    {0, "rows_in_speed",          20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows read per second"},
    {0, "rows_map",               20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows map"},
    {0, "rows_map_speed",         20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows map per second"},
    {0, "rows_combine",           20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows combine"},
    {0, "rows_combine_speed",     20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows combine per second"},
    {0, "rows_temp",              20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows temp"},
    {0, "rows_temp_speed",        20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows temp per second"},
    {0, "rows_reduce",            20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows reduce"},
    {0, "rows_reduce_speed",      20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows reduce per second"},
    {0, "rows_output",            20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows output"},
    {0, "rows_output_speed",      20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows output per second"},
    {0, "heap_size",              20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Heap blocks size, bytes"},
    {0, "heap_count",             20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Heap blocks count"},
    {0, "map_progress",           20, GANGLIA_VALUE_FLOAT, "%",            "both", "%.1f", UDP_HEADER_SIZE+8, "Map stage progress"},
    {0, "reduce_progress",        20, GANGLIA_VALUE_FLOAT, "%",            "both", "%.1f", UDP_HEADER_SIZE+8, "Reduce stage progress"},
    {0, NULL}
};

mmodule cppmr_module =
{
    STD_MMODULE_STUFF,
    ex_metric_init,
    ex_metric_cleanup,
    ex_metric_info,
    ex_metric_handler,
};
