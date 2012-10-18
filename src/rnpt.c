#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <udis86.h>
#include <unistd.h>

#include "elfs.h"
#include "tracer.h"

// gets bytes pointed to by instruction pointer and disassembles them into
// a string for x86-64 intel syntex
char * ins_str (struct _tracer * tracer)
{
    unsigned char data[16];
    uint64_t      ip;
    ud_t          ud_obj;

    ip = tracer_ip(tracer);

    int i;
    for (i = 0; i < 4; i++) {
        *((uint32_t *) &(data[i*4])) = tracer_memory(tracer, ip + (4 * i));
    }

    ud_init(&ud_obj);
    ud_set_mode(&ud_obj, 64);
    ud_set_syntax(&ud_obj, UD_SYN_INTEL);

    ud_set_input_buffer(&ud_obj, data, 16);

    if (ud_disassemble(&ud_obj) == 0) {
        char * result = malloc(128);
        memset(result, 0, 64);
        char tmp[8];
        for (i = 0; i < 16; i++) {
            sprintf(tmp, "%x ", data[i]);
            strcat(result, tmp);
        }
        return result;
    }

    return strdup(ud_insn_asm(&ud_obj));
}


// prints a description of the bytes pointed to by instruction pointer
// in tracer
void ins_print (struct _tracer * tracer)
{
    char * description = ins_str(tracer);

    printf("%s %016llx %s\n",
           elfs_name(tracer->elfs, tracer_ip(tracer)),
           (unsigned long long) tracer_ip(tracer),
           description);
    free(description);
}


void set_step_overs (struct _tracer * tracer, const char * step_overs)
{
    int i, begin;
    char * tmp = strdup(step_overs);

    begin = 0;
    for (i = 0; i < strlen(step_overs); i++) {
        if (tmp[i] == ',') {
            tmp[i] = '\0';
            tracer_step_over(tracer, &(tmp[begin]));
            begin = i + 1;
        }
    }

    tracer_step_over(tracer, &(tmp[begin]));
}


int main (int argc, char * argv[])
{
    struct _tracer * tracer;
    int steps = -1;
    int print_instructions = 1;
    const char * step_over = NULL;
    int c;

    // get options
    while ((c = getopt(argc, argv, "io:s:")) != -1) {
        switch (c) {
        case 'i' :
            print_instructions = 0;
            break;
        case 'o' :
            step_over = optarg;
            break;
        case 's' :
            steps = atoi(optarg);
            break;
        case '?' :
            if (optopt == 's') {
                fprintf(stderr, "option %c requires argument\n", optopt);
                return -1;
            }
            fprintf(stderr, "unknown option character %c\n", optopt);
            return -1;
        }
    }

    // set up tracer
    if (argc - optind < 1) {
        fprintf(stderr, "usage: %s <executable> <arguments>\n", argv[0]);
        return 0;
    }
    tracer = tracer_create(argv[optind], &(argv[optind]));

    if (tracer == NULL) {
        fprintf(stderr, "error loading tracer\n");
        return -1;
    }

    if (step_over)
        set_step_overs(tracer, step_over);

    // print the first instruction
    if (print_instructions)
        ins_print(tracer);

    if (steps >= 0) {
        int i;
        for (i = 0; i < steps; i++) {
            tracer_step(tracer);
            if (tracer_term(tracer))
                break;

            const char * symbol = elfs_symbol(tracer->elfs, tracer_ip(tracer));

            if (symbol != NULL) {
                printf("%s %s\n", elfs_name(tracer->elfs, tracer_ip(tracer)), symbol);
            }

            if (print_instructions)
                ins_print(tracer);
        }
    }
    else {
        while (1) {
            tracer_step(tracer);
            if (tracer_term(tracer))
                break;

            const char * symbol = elfs_symbol(tracer->elfs, tracer_ip(tracer));

            if (symbol != NULL) {
                printf("%s %s\n", elfs_name(tracer->elfs, tracer_ip(tracer)), symbol);
            }

            if (print_instructions)
                ins_print(tracer);
        }
    }

    tracer_destroy(tracer);

    return 0;
}