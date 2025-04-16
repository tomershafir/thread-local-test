#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include "tl_defs.h"

#define handle_errno(err, msg) do { errno = err; perror(msg); exit(EXIT_FAILURE); } while (0);

typedef struct latch {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned count;
} latch;

// If count is negative or greater than INT_MAX, the behaviour is undefined.
void latch_init(latch *l, unsigned count);

void latch_destroy(latch *l);

// If the count down size is negative or greater than the internal counter, the behaviour is undefined.
void latch_count_down(latch *l);
void latch_count_down_n(latch *l, unsigned n);

void latch_wait(latch *l);

static void *thread_signal_and_wait(void *unused);

// Returns the size of data + stack virtual memory for the ciurrent process in KiBs.
static long vm_self_data_and_stack_size(void);

#if defined(OS_LINUX)
typedef struct statm {
    unsigned long size;
    unsigned long resident;
    unsigned long share;
    unsigned long text;
    unsigned long lib;
    unsigned long data;
    unsigned long dt;
} statm;

static void vm_self_statm(statm*);
#endif

static latch threads_created_latch;
static latch unblock_threads_latch;

int main() {
    const int THREAD_COUNT = 4;
    pthread_t threads[THREAD_COUNT];
    latch_init(&threads_created_latch, THREAD_COUNT);
    latch_init(&unblock_threads_latch, 1);

    // get_thread_area() syscall is highly architecture specific,
    // and the standard library doesn't provide a portable wrapper.
    // In addition, we dont want to depend on a specific userspace allocator (like mallinfo()).
    // Furthermore, per thread memory usage is more complex to retrieve,
    // and the total/sum provides similar information. 
    // So, we just check the total system allocated memory for the
    // program.
    // We record the memory usage before the threads creation and 
    // later subtract to improve the accuracy of the test.
    long static_memory_usage_kib_before = vm_self_data_and_stack_size();

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, thread_signal_and_wait, NULL);
    }

    latch_wait(&threads_created_latch);

    long static_memory_usage_kib_after = vm_self_data_and_stack_size();
    if (static_memory_usage_kib_after - static_memory_usage_kib_before < THREAD_COUNT * TL_SIZE_BYTES / 1024) {
        fprintf(stderr, "Memory usage including thread local area is less than expected\n");
        exit(EXIT_FAILURE);
    }

    latch_count_down(&unblock_threads_latch);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }
    latch_destroy(&threads_created_latch);
    latch_destroy(&unblock_threads_latch);
    printf("Test passed\n");
    return 0;
}

static void *thread_signal_and_wait(void *unused) {
    (void)unused; // suppress unused parameter warning
    latch_count_down(&threads_created_latch);
    latch_wait(&unblock_threads_latch);
    return NULL; // unused
}

void latch_init(latch *l, unsigned count) {
    pthread_mutex_init(&l->mutex, NULL);
    pthread_cond_init(&l->cond, NULL);
    l->count = count;
}

void latch_destroy(latch *l) {
    pthread_mutex_destroy(&l->mutex);
    pthread_cond_destroy(&l->cond);
}

void latch_count_down(latch *l) {
    latch_count_down_n(l, 1);
}

void latch_count_down_n(latch *l, unsigned n) {
    pthread_mutex_lock(&l->mutex);
    l->count -= n;
    if (!l->count) {
        pthread_cond_broadcast(&l->cond);
    }
    pthread_mutex_unlock(&l->mutex);
}

void latch_wait(latch *l) {
    pthread_mutex_lock(&l->mutex);
    while (l->count) {
        pthread_cond_wait(&l->cond, &l->mutex);
    }
    pthread_mutex_unlock(&l->mutex);
}


static long vm_self_data_and_stack_size(void) {
#if defined(OS_LINUX)
    statm s;
    vm_self_statm(&s);
    return s.data;
#else
    handle_errno(ENOTSUP, "Reading memory statistics is not supported on this platform");
#endif
}

#if defined(OS_LINUX)
static void vm_self_statm(statm *s) {
    int errno_restore = errno;
    const char *statm_path = "/proc/self/statm";
    FILE *statm_file = fopen(statm_path, "r");
    if (!statm_file) {
        errno_restore = errno;
        goto cleanup;
    }
    if(fscanf(statm_file, "%lu %lu %lu %lu %lu %lu %lu", 
            &s->size, &s->resident, &s->share, &s->text, &s->lib, &s->data, &s->dt) != 7) {
        errno_restore = errno;
        goto cleanup;
    }
    return;
cleanup:
    if (statm_file) {
        fclose(statm_file);
    }
    handle_errno(errno_restore, "Error reading /proc/self/statm");
}
#endif
