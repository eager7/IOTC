/****************************************************************************
 *
 * MODULE:             libJIP
 *
 * COMPONENT:          Thread.c
 *
 * REVISION:           $Revision: 53151 $
 *
 * DATED:              $Date: 2013-04-09 16:14:34 +0100 (Tue, 09 Apr 2013) $
 *
 * AUTHOR:             Matt Redfearn
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139]. 
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the 
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#include <Threads.h>

#define DBG_QUEUE   0

/************************** Queue Functionality ******************************/





teQueueStatus eQueueCreate(tsQueue *psQueue, uint32 u32Capacity)
{
    psQueue->apvBuffer = malloc(sizeof(void *) * u32Capacity);
    
    if (!psQueue->apvBuffer)
    {
        return E_QUEUE_ERROR_NO_MEM;
    }
    
    psQueue->u32Capacity = u32Capacity;
    psQueue->u32Size = 0;
    psQueue->u32In = 0;
    psQueue->u32Out = 0;
    
    pthread_mutex_init(&psQueue->mutex, NULL);
    pthread_cond_init(&psQueue->cond_space_available, NULL);
    pthread_cond_init(&psQueue->cond_data_available, NULL);
    
    return E_QUEUE_OK;
}


teQueueStatus eQueueDestroy(tsQueue *psQueue)
{
    if (NULL == psQueue->apvBuffer)
    {
        return E_QUEUE_ERROR_FAILED;
    }
    free(psQueue->apvBuffer);
    
    pthread_mutex_destroy(&psQueue->mutex);
    pthread_cond_destroy(&psQueue->cond_space_available);
    pthread_cond_destroy(&psQueue->cond_data_available);

    return E_QUEUE_OK;
}


teQueueStatus eQueueQueue(tsQueue *psQueue, void *pvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == psQueue->u32Capacity)
        pthread_cond_wait(&psQueue->cond_space_available, &psQueue->mutex);
    psQueue->apvBuffer[psQueue->u32In] = pvData;
    ++psQueue->u32Size;
    
    psQueue->u32In = (psQueue->u32In+1) % psQueue->u32Capacity;
    
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_data_available);
    return E_QUEUE_OK;
}


teQueueStatus eQueueDequeue(tsQueue *psQueue, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == 0)
        pthread_cond_wait(&psQueue->cond_data_available, &psQueue->mutex);
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Out];
    --psQueue->u32Size;
    
    psQueue->u32Out = (psQueue->u32Out + 1) % psQueue->u32Capacity;
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_space_available);
    return E_QUEUE_OK;
}


teQueueStatus eQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeout, void **ppvData)
{
    pthread_mutex_lock(&psQueue->mutex);
    while (psQueue->u32Size == 0)
    {
        struct timeval sNow;
        struct timespec sTimeout;
        
        memset(&sNow, 0, sizeof(struct timeval));
        gettimeofday(&sNow, NULL);
        sTimeout.tv_sec = sNow.tv_sec + (u32WaitTimeout/1000);
        sTimeout.tv_nsec = (sNow.tv_usec + ((u32WaitTimeout % 1000) * 1000)) * 1000;
        if (sTimeout.tv_nsec > 1000000000)
        {
            sTimeout.tv_sec++;
            sTimeout.tv_nsec -= 1000000000;
        }
        DBG_vPrintf(DBG_QUEUE, "Dequeue timed: now    %lu s, %lu ns\n", sNow.tv_sec, sNow.tv_usec * 1000);
        DBG_vPrintf(DBG_QUEUE, "Dequeue timed: until  %lu s, %lu ns\n", sTimeout.tv_sec, sTimeout.tv_nsec);

        switch (pthread_cond_timedwait(&psQueue->cond_data_available, &psQueue->mutex, &sTimeout))
        {
            case (0):
                break;
            
            case (ETIMEDOUT):
                pthread_mutex_unlock(&psQueue->mutex);
                return E_QUEUE_ERROR_TIMEOUT;
                break;
            
            default:
                pthread_mutex_unlock(&psQueue->mutex);
                return E_QUEUE_ERROR_FAILED;
        }
    }
    
    *ppvData = psQueue->apvBuffer[psQueue->u32Out];
    --psQueue->u32Size;
    
    psQueue->u32Out = (psQueue->u32Out + 1) % psQueue->u32Capacity;
    pthread_mutex_unlock(&psQueue->mutex);
    pthread_cond_broadcast(&psQueue->cond_space_available);
    return E_QUEUE_OK;
}



uint32 u32AtomicAdd(volatile uint32 *pu32Value, uint32 u32Operand)
{
#if defined(_MSC_VER)
        return add_value + InterlockedExchangeAdd(pu32Value, u32Operand);
#else
        return __sync_add_and_fetch (pu32Value, u32Operand);
#endif
}

