/****************************************************************************
 *
 * MODULE:             libJIP
 *
 * COMPONENT:          Threads.h
 *
 * REVISION:           $Revision: 52723 $
 *
 * DATED:              $Date: 2013-03-14 12:04:13 +0000 (Thu, 14 Mar 2013) $
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

#ifndef __THREADS_H__
#define __THREADS_H__

#include "utils.h"

typedef enum
{
    E_QUEUE_OK,
    E_QUEUE_ERROR_FAILED,
    E_QUEUE_ERROR_TIMEOUT,
    E_QUEUE_ERROR_NO_MEM,
} teQueueStatus;

typedef struct
{
    void **apvBuffer;
    uint32 u32Capacity;
    uint32 u32Size;
    uint32 u32In;
    uint32 u32Out;

    pthread_mutex_t mutex;    
    pthread_cond_t cond_space_available;
    pthread_cond_t cond_data_available;
} tsQueue;

teQueueStatus eQueueCreate(tsQueue *psQueue, uint32 u32Capacity);

teQueueStatus eQueueDestroy(tsQueue *psQueue);

teQueueStatus eQueueQueue(tsQueue *psQueue, void *pvData);

teQueueStatus eQueueDequeue(tsQueue *psQueue, void **ppvData);

teQueueStatus eQueueDequeueTimed(tsQueue *psQueue, uint32 u32WaitTimeout, void **ppvData);


/** Atomically add a 32 bit value to another.
 *  \param pu32Value        Pointer to value to update
 *  \param u32Operand       Value to add
 *  \return New value
 */
uint32 u32AtomicAdd(volatile uint32 *pu32Value, uint32 u32Operand);


/** Atomically get the value of a 32 bit value */
#define u32AtomicGet(pu32Value) u32AtomicAdd(pu32Value, 0)



#endif /* __THREADS_H__ */


