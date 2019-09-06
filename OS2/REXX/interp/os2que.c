#include "rexx.h"

int addr_same_queue( const tsd_t *TSD, const Queue *q1, const Queue *q2 )
{
    return 0;
}

Queue *addr_redir_queue( const tsd_t *TSD, Queue *q )
{
    return NULL;
}

void addr_purge_queue( const tsd_t *TSD, Queue *q )
{
}

Queue *addr_reopen_queue( tsd_t *TSD, const streng *queuename, char code )
{
    return NULL;
}

void addr_close_queue( const tsd_t *TSD, Queue *q )
{
}

streng *addr_io_queue( tsd_t *TSD, Queue *q, streng *buffer, int isFIFO )\
{
    return NULL;
}

int create_queue( tsd_t *TSD, const streng *queue_name, streng **result )
{
    return 0;
}

int delete_queue( tsd_t *TSD, const streng *queue_name )
{
    return 0;
}

streng *get_queue( tsd_t *TSD )
{
    return NULL;
}

streng *set_queue( tsd_t *TSD, const streng *queue_name )
{
    return NULL;
}

int timeout_queue( tsd_t *TSD, const streng *timeout, const streng *queue_name )
{
    return 0;
}

void flush_stack( const tsd_t *TSD, Queue *src, Queue *dst, int is_fifo )
{
}

Queue *find_free_slot( const tsd_t *TSD )
{
    return NULL;
}

void purge_stacks( const tsd_t *TSD )
{
}

int lines_in_stack( tsd_t *TSD, const streng *queue_name )
{
    return 0;
}

int init_stacks( tsd_t *TSD )
{
    return 0;
}

int stack_lifo( tsd_t *TSD, streng *line, const streng *queue_name )
{
    return 0;
}

int stack_fifo( tsd_t *TSD, streng *line, const streng *queue_name )
{
    return 0;
}

streng *stack_to_line( const tsd_t *TSD, Queue *q )
{
    return NULL;
}

streng *popline( tsd_t *TSD, const streng *queue_name, int *result, unsigned long waitflag )
{
    return NULL;
}

int make_buffer( tsd_t *TSD )
{
    return 0;
}

int drop_buffer( const tsd_t *TSD, int num )
{
    return 0;
}

void type_buffer( tsd_t *TSD )
{
}

int external_queues_used( const tsd_t *TSD )
{
    return 0;
}

Queue *fill_input_queue_stem( tsd_t *TSD, streng *stemname, int stem0 )
{
    return NULL;
}

Queue *fill_input_queue_stream( tsd_t *TSD, void *fileptr )
{
    return NULL;
}

void fill_queue_name( const tsd_t *TSD, int *len, char **name )
{
}
