#ifndef tracer_HEADER
#define tracer_HEADER

#include <inttypes.h>
#include <sys/types.h>
#include <udis86.h>

#include "elfs.h"

struct _tracer_step_overs {
	char * pattern;
	struct _tracer_step_overs * next;
};

struct _tracer {
    pid_t pid;
    int terminated;
    struct _elfs * elfs;
    struct _tracer_step_overs * step_overs;
};

struct _tracer * tracer_create  (const char * exec, char * const args[]);
void             tracer_destroy (struct _tracer *);

void             tracer_step_over (struct _tracer *, const char * pattern);

// returns number of bytes disassembled
int              tracer_dis       (struct _tracer *, ud_t * ud_obj);

// returns instruction pointer
uint64_t         tracer_step    (struct _tracer *);
int              tracer_term    (struct _tracer *);

uint64_t         tracer_ip      (struct _tracer *);
pid_t            tracer_pid     (struct _tracer *);
// returns a pointer to memory at address
uint32_t         tracer_memory  (struct _tracer *, uint64_t);

void tracer_internal_wait (struct _tracer *);

#endif