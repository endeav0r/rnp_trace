#include "tracer.h"

#include "elf.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>


struct _tracer * tracer_create (const char * exec, char * const args[])
{
    int              status;
    struct _elf *    elf;
    struct _tracer * tracer;

    tracer = (struct _tracer *) malloc(sizeof(struct _tracer));

    elf = elf_load(exec);
    if (elf == NULL)
        return NULL;

    tracer->pid        = fork();
    tracer->terminated = 0;
    if (tracer->pid == -1)
        return NULL;

    if (tracer->pid == 0) {
        // start the child process, prepare for tracing
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execv(exec, args);
    }

    while (1) {
        waitpid(tracer->pid, &status, 0);
        if ((WIFSTOPPED(status)) && (WSTOPSIG(status) == SIGTRAP)) {
            break;
        }
    }

    uint64_t entry_address = elf_entry(elf);
    elf_destroy(elf);
    printf("entry address: %llx\n", (unsigned long long) entry_address);
    fflush(stdout);

    long saved_entry_bytes;
    long break_bytes;
    struct user_regs_struct regs;

    // set a breakpoint at the entry point
    saved_entry_bytes = ptrace(PTRACE_PEEKTEXT, tracer->pid, entry_address, NULL);
    break_bytes = (saved_entry_bytes & (~0xff)) | 0xcc;
    ptrace(PTRACE_POKETEXT, tracer->pid, entry_address, break_bytes);

    // execute to breakpoint
    ptrace(PTRACE_CONT, tracer->pid, NULL, NULL);

    while (1) {
        waitpid(tracer->pid, &status, 0);
        if ((WIFSTOPPED(status)) && (WSTOPSIG(status) == SIGTRAP)) {
            break;
        }
    }

    // restore memory at breakpoint
    ptrace(PTRACE_POKETEXT, tracer->pid, entry_address, saved_entry_bytes);

    // correct rip
    ptrace(PTRACE_GETREGS, tracer->pid, NULL, &regs);
    regs.rip = entry_address;
    ptrace(PTRACE_SETREGS, tracer->pid, NULL, &regs);

    // load elfs
    tracer->elfs = elfs_create(tracer->pid);
    if (tracer->elfs == NULL)
        return NULL;

    tracer->step_overs = NULL;

    return tracer;
}


void tracer_destroy (struct _tracer * tracer)
{
    ptrace(PTRACE_KILL, tracer->pid, NULL, NULL);
    free(tracer);
}


void tracer_step_over (struct _tracer * tracer, const char * pattern)
{
    struct _tracer_step_overs * next;
    struct _tracer_step_overs * new;

    new = (struct _tracer_step_overs *) malloc(sizeof(struct _tracer_step_overs));

    new->pattern = (char *) malloc(strlen(pattern) + 1);
    memset(new->pattern, 0, strlen(pattern) + 1);
    strcpy(new->pattern, pattern);
    new->next = NULL;

    next = tracer->step_overs;
    if (next != NULL) {
        while (next->next != NULL) next = next->next;
        next->next = new;
    }
    else
        tracer->step_overs = new;
}

int tracer_dis (struct _tracer * tracer, ud_t * ud_obj)
{
    int i;
    int bytes_disassembled;
    unsigned char data[16];
    uint64_t ip;

    ip = tracer_ip(tracer);

    ud_init(ud_obj);
    ud_set_mode(ud_obj, 64);

    for (i = 0; i < 4; i++) {
        *((uint32_t *) &(data[i*4])) = tracer_memory(tracer, ip + (4 * i));
    }

    ud_set_input_buffer(ud_obj, data, 16);

    bytes_disassembled = ud_disassemble(ud_obj);

    return bytes_disassembled;
}

uint64_t tracer_step (struct _tracer * tracer)
{
    uint64_t instruction_size;
    uint64_t ip;
    struct   user_regs_struct regs;
    int      step_over;
    ud_t     ud_obj;
    struct _tracer_step_overs * next;

    // if we are about to execute a call, we need to see if we should step
    // over it
    instruction_size = tracer_dis(tracer, &ud_obj);
    if ((instruction_size > 0) && (ud_obj.mnemonic == UD_Icall)) {
        // save return address
        uint64_t ret_addr = tracer_ip(tracer) + instruction_size;
        // execute next instruction
        ptrace(PTRACE_SINGLESTEP, tracer->pid, NULL, NULL);
        tracer_internal_wait(tracer);
        // is this instruction in one of our step_overs
        step_over = 0;
        ip = tracer_ip(tracer);
        for (next = tracer->step_overs; next != NULL; next = next->next) {
            if (strstr(elfs_name(tracer->elfs, ip), next->pattern)) {
                step_over = 1;
                break;
            }
        }
        if (step_over) {
            // set breakpoint at ret_addr
            long tmp = ptrace(PTRACE_PEEKTEXT, tracer->pid, ret_addr, NULL);
            ptrace(PTRACE_POKETEXT, tracer->pid, ret_addr, 0xcc);
            // continue until ret_addr
            ptrace(PTRACE_CONT, tracer->pid, NULL, NULL);
            tracer_internal_wait(tracer);
            // remove breakpoint
            ptrace(PTRACE_POKETEXT, tracer->pid, ret_addr, tmp);
            // back that rip up
            ptrace(PTRACE_GETREGS, tracer->pid, NULL, &regs);
            regs.rip--;
            ptrace(PTRACE_SETREGS, tracer->pid, NULL, &regs);
            return regs.rip;
        }
        else {
            ptrace(PTRACE_GETREGS, tracer->pid, NULL, &regs);
            return regs.rip;
        }
    }

    ptrace(PTRACE_SINGLESTEP, tracer->pid, NULL, NULL);
    tracer_internal_wait(tracer);

    ptrace(PTRACE_GETREGS, tracer->pid, NULL, &regs);
    return regs.rip;
}


int tracer_term (struct _tracer * tracer)
{
    return tracer->terminated;
}


uint64_t tracer_ip (struct _tracer * tracer)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, tracer->pid, NULL, &regs);
    return regs.rip;
}


pid_t tracer_pid (struct _tracer * tracer)
{
    return tracer->pid;
}


uint32_t tracer_memory (struct _tracer * tracer, uint64_t address)
{
    return ptrace(PTRACE_PEEKTEXT, tracer->pid, address, NULL);
}


void tracer_internal_wait (struct _tracer * tracer)
{
    int status;

    while (tracer->terminated == 0) {
        waitpid(tracer->pid, &status, 0);
        if (WIFSTOPPED(status)) {
            if (WSTOPSIG(status) == SIGTRAP)
                break;
            else {
                printf("WSTOPSIG(status)=%d\n", WSTOPSIG(status));
            }
        }
        else if ((WTERMSIG(status)) || (WIFEXITED(status))) {
            tracer->terminated = 1;
            break;
        }
    }
}