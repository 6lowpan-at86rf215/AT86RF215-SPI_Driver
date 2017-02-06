/**
 * @file qmm.h
 *
 * @brief This file contains the Queue Management Module definitions.
 *
 * $Id: qmm.h 37108 2015-03-30 09:07:42Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef QMM_INTERFACE_H
#define QMM_INTERFACE_H

/* === Includes ============================================================ */

#include "return_val.h"
#include "bmm.h"

/* === Macros ============================================================== */


/* === Types =============================================================== */

/**
 * @brief Structure to search for a buffer to be removed from a queue
 */
typedef struct
#if !defined(DOXYGEN)
        search_tag
#endif
{
    /** Pointer to search criteria function */
    uint8_t (*criteria_func)(void *buf, void *handle);
    /** Handle to callbck parameter */
    void *handle;
} search_t;

/**
 * @brief Queue structure
 *
 * This structur defines the queue structure.
 * The application should declare the queue of type queue_t
 * and call qmm_queue_init before invoking any other functionality of qmm.
 *
 * @ingroup apiMacTypes
 */
typedef struct
#if !defined(DOXYGEN)
        queue_tag
#endif
{
    /** Pointer to head of queue */
    buffer_t *head;
    /** Pointer to tail of queue */
    buffer_t *tail;
    /**
     * Number of buffers present in the current queue
     */
    uint8_t size;
} queue_t;

/* === Externals =========================================================== */


/* === Prototypes ========================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initializes the queue.
     *
     * This function initializes the queue. Note that this function
     * should be called before invoking any other functionality of QMM.
     *
     * @param q The queue which should be initialized.
     *
     * @ingroup apiResApi
     */
    void qmm_queue_init(queue_t *q);
    /**
     * @brief Appends a buffer into the queue.
     *
     * This function appends a buffer into the queue.
     *
     * @param q Queue into which buffer should be appended
     *
     * @param buf Pointer to the buffer that should be appended into the queue.
     * Note that this pointer should be same as the
     * pointer returned by bmm_buffer_alloc.
     *
     * @ingroup apiResApi
     */
    void qmm_queue_append(queue_t *q, buffer_t *buf);

    /**
     * @brief Removes a buffer from queue.
     *
     * This function removes a buffer from queue
     *
     * @param q Queue from which buffer should be removed
     *
     * @param search Search criteria. If this parameter is NULL, first buffer in the
     * queue will be removed. Otherwise buffer matching the criteria will be
     * removed.
     *
     * @return Pointer to the buffer header, if the buffer is
     * successfully removed, NULL otherwise.
     *
     * @ingroup apiResApi
     */
    buffer_t *qmm_queue_remove(queue_t *q, search_t *search);

    /**
     * @brief Reads a buffer from queue.
     *
     * This function reads either the first buffer if search is NULL or buffer
     * matching the given criteria from queue.
     *
     * @param q The queue from which buffer should be read.
     *
     * @param search If this parameter is NULL first buffer in the queue will be
     * read. Otherwise buffer matching the criteria will be read
     *
     * @return Pointer to the buffer header which is to be read, NULL if the buffer
     * is not available
     *
     * @ingroup apiResApi
     */
    buffer_t *qmm_queue_read(queue_t *q, search_t *search);

    /**
     * @brief Internal function for flushing a specific queue
     *
     * @param q Queue to be flushed
     *
     * @ingroup apiResApi
     */
    void qmm_queue_flush(queue_t *q);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* QMM_INTERFACE_H */

/* EOF */
