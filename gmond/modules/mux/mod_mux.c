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
extern mmodule mux_module;

static unsigned int random_max = 50;
static unsigned int constant_value = 20;

static key_t key = 7294192;
static int shmid = -1;
#define SHMSZ     1024

struct mux_metrics
{
    uint32_t version;
    float bid_requests_per_sec;
    float ready_requests_per_sec;
    float avg_request_size;
    float sum_bids;
    float active_connections;
    float avg_session_time;
    float connects_per_sec;
    float parquet_time;
    float rows_per_upload;
    float net_callback_time_usec;
};

static struct mux_metrics *shm = NULL;
static void open_shm()
{
    shm = NULL;
    shmid = -1;
    if (shmid > 0)
        return;
    
    shmid = shmget(key, SHMSZ, 0666);
    if (shmid < 0)
    {
        debug_msg("[mod_mux]Can't open shared memory handle");
        return;
    }
    shm = (struct mux_metrics*)shmat(shmid, NULL, 0);
    if (shm == (struct mux_metrics*)-1)
    {
        shmid = -1;
        shm = NULL;
        debug_msg("[mod_mux]Can't map shared memory handle");
        return;
    } 
}
static int ex_metric_init ( apr_pool_t *p )
{
    const char* str_params = mux_module.module_params;
    apr_array_header_t *list_params = mux_module.module_params_list;
    mmparam *params;
    int i;

    srand(time(NULL)%99);


    for (i = 0; mux_module.metrics_info[i].name != NULL; i++) {
        /* Initialize the metadata storage for each of the metrics and then
         *  store one or more key/value pairs.  The define MGROUPS defines
         *  the key for the grouping attribute. */
        MMETRIC_INIT_METADATA(&(mux_module.metrics_info[i]),p);
        MMETRIC_ADD_METADATA(&(mux_module.metrics_info[i]),MGROUP,"MUX");
    }
    debug_msg("[mod_mux]registered %d metrics", i);
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
    debug_msg("[mod_mux]Collecting %d metric", metric_index);

    if (!shm)
        open_shm();
    if (!shm || shm->version != 1)
    {
        val.f = 0;
        return val;
    }
    
    switch (metric_index) {
    case 0:
        val.f = shm->bid_requests_per_sec;    
        shm->bid_requests_per_sec = 0;
        break;
    case 1:
        val.f = shm->ready_requests_per_sec;    
        shm->ready_requests_per_sec = 0;
        break;
    case 2:
        val.f = shm->avg_request_size;    
        shm->avg_request_size = 0;
        break;
    case 3:
        val.f = shm->sum_bids;    
        shm->sum_bids = 0;
        break;
    case 4:
        val.f = shm->active_connections;    
        shm->active_connections = 0;
        break;
    case 5:
        val.f = shm->avg_session_time;    
        shm->avg_session_time = 0;
        break;
    case 6:
        val.f = shm->connects_per_sec;    
        shm->connects_per_sec = 0;
        break;
    case 7:
        val.f = shm->parquet_time;    
        shm->parquet_time = 0;
        break;
    case 8:
        val.f = shm->rows_per_upload;    
        shm->rows_per_upload = 0;
        break;
    case 9:
        val.f = shm->net_callback_time_usec;    
        shm->net_callback_time_usec = 0;
        break;
    default:
        val.uint32 = 0; /* default fallback */
    }

    debug_msg("[mod_mux] value = %f", val.f);
    return val;
}

static Ganglia_25metric ex_metric_info[] = 
{
    {0, "bid_requests_per_sec",   20, GANGLIA_VALUE_FLOAT, "Rps",   "both", "%.1f", UDP_HEADER_SIZE+8, "Bid requests per second"},
    {0, "ready_requests_per_sec", 20, GANGLIA_VALUE_FLOAT, "Rps",   "both", "%.1f", UDP_HEADER_SIZE+8, "Ready requests per second"},
    {0, "avg_request_size",       20, GANGLIA_VALUE_FLOAT, "Bytes", "both", "%.1f", UDP_HEADER_SIZE+8, "Avg request size"},
    {0, "sum_bids",               20, GANGLIA_VALUE_FLOAT, "USC",   "both", "%.1f", UDP_HEADER_SIZE+8, "Sum bids sent"},
    {0, "active_connections",     20, GANGLIA_VALUE_FLOAT, "Num",   "both", "%.1f", UDP_HEADER_SIZE+8, "Active connections"},
    {0, "avg_session_time",       20, GANGLIA_VALUE_FLOAT, "msec",  "both", "%.1f", UDP_HEADER_SIZE+8, "Average session time"},
    {0, "connects_per_sec",       20, GANGLIA_VALUE_FLOAT, "Cps",   "both", "%.1f", UDP_HEADER_SIZE+8, "Connections per second"},
    {0, "parquet_time",           20, GANGLIA_VALUE_FLOAT, "msec",  "both", "%.1f", UDP_HEADER_SIZE+8, "Parquet compression+serialization time"},
    {0, "rows_per_upload",        20, GANGLIA_VALUE_FLOAT, "Num",   "both", "%.1f", UDP_HEADER_SIZE+8, "Average number of rows per upload"},
    {0, "net_callback_time_usec", 20, GANGLIA_VALUE_FLOAT, "usec",  "both", "%.1f", UDP_HEADER_SIZE+8, "Average net request processing time"},
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
