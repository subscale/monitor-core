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
        int64_t input_files_found;
        int64_t input_files_read;
        float input_bps;
        int64_t input_bytes;
        float temp_bps;
        int64_t temp_bytes;
        int64_t temp_files;
        int64_t temp_chunks;
        float merge_bps;
        int64_t merge_bytes;
        int64_t merge_files;
        int64_t merge_chunks;
        int64_t output_files;
        float reduce_bps;
        int64_t reduce_bytes;
        float output_bps;
        int64_t output_bytes;
        float http_in_bps;
        int64_t http_in_bytes;
        float http_out_bps;
        int64_t http_out_bytes;
        int64_t rows_in;
        float rows_in_rate;
        int64_t rows_map;
        float rows_map_rate;
        int64_t rows_combine;
        float rows_combine_rate;
        int64_t rows_temp;
        float rows_temp_rate;
        int64_t rows_merge_in;
        float rows_merge_in_rate;
        int64_t rows_merge_out;
        float rows_merge_out_rate;
        int64_t rows_reduce;
        float rows_reduce_rate;
        int64_t rows_output;
        float rows_output_rate;
        int64_t heap_size;
        int64_t heap_count;
        float map_progress_pct;
        float reduce_progress_pct;
        float job_progress_pct;
        int64_t mempool_size;
        uint32_t mempool_count;
        uint32_t reduce_in_files;
        uint32_t reduce_in_chunks;
        uint32_t reduce_done_chunks;
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
    if (!shm || shm->version != 2)
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
    case 2:
        val.f = shm->input_bps;    
        shm->input_bps = 0;
        break;
    case 3:
        val.f = shm->input_bytes;    
        shm->input_bytes = 0;
        break;
    case 4:
        val.f = shm->temp_bps;    
        shm->temp_bps = 0;
        break;
    case 5:
        val.f = shm->temp_bytes;    
        shm->temp_bytes = 0;
        break;
    case 6:
        val.uint32 = shm->temp_files;    
        shm->temp_files = 0;
        break;
    case 7:
        val.uint32 = shm->temp_chunks;    
        shm->temp_chunks = 0;
        break;
    case 8:
        val.f = shm->merge_bps;    
        shm->merge_bps = 0;
        break;
    case 9:
        val.f = shm->merge_bytes;    
        shm->merge_bytes = 0;
        break;
    case 10:
        val.uint32 = shm->merge_files;    
        shm->merge_files = 0;
        break;
    case 11:
        val.uint32 = shm->merge_chunks;    
        shm->merge_chunks = 0;
        break;
    case 12:
        val.uint32 = shm->output_files;    
        shm->output_files = 0;
        break;
    case 13:
        val.f = shm->reduce_bps;    
        shm->reduce_bps = 0;
        break;
    case 14:
        val.f = shm->reduce_bytes;    
        shm->reduce_bytes = 0;
        break;
    case 15:
        val.f = shm->output_bps;    
        shm->output_bps = 0;
        break;
    case 16:
        val.f = shm->output_bytes;    
        shm->output_bytes = 0;
        break;
    case 17:
        val.f = shm->http_in_bps;    
        shm->http_in_bps = 0;
        break;
    case 18:
        val.f = shm->http_in_bytes;    
        shm->http_in_bytes = 0;
        break;
    case 19:
        val.f = shm->http_out_bps;    
        shm->http_out_bps = 0;
        break;
    case 20:
        val.f = shm->http_out_bytes;    
        shm->http_out_bytes = 0;
        break;
    case 21:
        val.f = shm->rows_in;    
        shm->rows_in = 0;
        break;
    case 22:
        val.f = shm->rows_in_rate;    
        shm->rows_in_rate = 0;
        break;
    case 23:
        val.f = shm->rows_map;    
        shm->rows_map = 0;
        break;
    case 24:
        val.f = shm->rows_map_rate;    
        shm->rows_map_rate = 0;
        break;
    case 25:
        val.f = shm->rows_combine;    
        shm->rows_combine = 0;
        break;
    case 26:
        val.f = shm->rows_combine_rate;    
        shm->rows_combine_rate = 0;
        break;
    case 27:
        val.f = shm->rows_temp;    
        shm->rows_temp = 0;
        break;
    case 28:
        val.f = shm->rows_temp_rate;    
        shm->rows_temp_rate = 0;
        break;
    case 29:
        val.f = shm->rows_reduce;    
        shm->rows_reduce = 0;
        break;
    case 30:
        val.f = shm->rows_reduce_rate;    
        shm->rows_reduce_rate = 0;
        break;
    case 31:
        val.f = shm->rows_output;    
        shm->rows_output = 0;
        break;
    case 32:
        val.f = shm->rows_output_rate;    
        shm->rows_output_rate = 0;
        break;
    case 33:
        val.f = shm->heap_size;    
        shm->heap_size = 0;
        break;
    case 34:
        val.uint32 = shm->heap_count;    
        shm->heap_count = 0;
        break;
    case 35:
        val.f = shm->map_progress_pct;    
        shm->map_progress_pct = 0;
        break;
    case 36:
        val.f = shm->reduce_progress_pct;    
        shm->reduce_progress_pct = 0;
        break;
    case 37:
        val.f = shm->job_progress_pct;    
        shm->job_progress_pct = 0;
        break;
    case 38:
        val.f = shm->mempool_size;    
        shm->mempool_size = 0;
        break;
    case 39:
        val.uint32 = shm->mempool_count;    
        shm->mempool_count = 0;
        break;
    case 40:
        val.uint32 = shm->reduce_in_files;    
        shm->reduce_in_files = 0;
        break;
    case 41:
        val.uint32 = shm->reduce_in_chunks;    
        shm->reduce_in_chunks = 0;
        break;
    case 42:
        val.uint32 = shm->reduce_done_chunks;    
        shm->reduce_done_chunks = 0;
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
    {0, "input_bytes",            20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.0f", UDP_HEADER_SIZE+8, "Read bytes"},
    {0, "temp_bps",               20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Temp bytes/sec"},
    {0, "temp_bytes",             20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Temp bytes"},
    {0, "temp_files",             20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Temp files count"},
    {0, "temp_chunks",            20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Temp chunks count"},
    {0, "merge_bps",              20, GANGLIA_VALUE_FLOAT, "Bytes/sec",    "both", "%.1f", UDP_HEADER_SIZE+8, "Merge bytes/sec"},
    {0, "merge_bytes",            20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Merge bytes"},
    {0, "merge_files",            20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Merge files count"},
    {0, "merge_chunks",           20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Merge chunks count"},
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
    {0, "rows_in_rate",           20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows read per second"},
    {0, "rows_map",               20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows map"},
    {0, "rows_map_rate",          20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows map per second"},
    {0, "rows_combine",           20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows combined"},
    {0, "rows_combine_rate",      20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows combined per second"},
    {0, "rows_temp",              20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows temp"},
    {0, "rows_temp_rate",         20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows temp per second"},
    {0, "rows_reduce",            20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows reduce"},
    {0, "rows_reduce_rate",       20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows reduce per second"},
    {0, "rows_output",            20, GANGLIA_VALUE_FLOAT, "Rows",         "both", "%.1f", UDP_HEADER_SIZE+8, "Rows output"},
    {0, "rows_output_rate",       20, GANGLIA_VALUE_FLOAT, "Rows/sec",     "both", "%.1f", UDP_HEADER_SIZE+8, "Rows output per second"},
    {0, "heap_size",              20, GANGLIA_VALUE_FLOAT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Heap blocks size, bytes"},
    {0, "heap_count",             20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Heap blocks count"},
    {0, "map_progress",           20, GANGLIA_VALUE_FLOAT, "%",            "both", "%.1f", UDP_HEADER_SIZE+8, "Map stage progress"},
    {0, "reduce_progress",        20, GANGLIA_VALUE_FLOAT, "%",            "both", "%.1f", UDP_HEADER_SIZE+8, "Reduce stage progress"},
    {0, "job_progress",           20, GANGLIA_VALUE_FLOAT, "%",            "both", "%.1f", UDP_HEADER_SIZE+8, "Overall progress"},
    {0, "mempool_size",           20, GANGLIA_VALUE_UNSIGNED_INT, "Bytes",        "both", "%.1f", UDP_HEADER_SIZE+8, "Memory bool size, bytes"},
    {0, "mempool_count",          20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Memory pool buffers count"},
    {0, "reduce_in_files",        20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Files for reduce input"},
    {0, "reduce_in_chunks",       20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Chunks for reduce intput"},
    {0, "reduce_done_chunks",     20, GANGLIA_VALUE_UNSIGNED_INT, "Num",   "both", "%u", UDP_HEADER_SIZE+8, "Reduced chunks"},
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
