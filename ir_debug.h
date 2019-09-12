#ifndef __IR_DEBUG_H__
#define __IR_DEBUG_H__

#include "compiler.h"

/*
 * This function can be used to print out the intermediate representation
 * in a human-readable form, useful for debugging purposes.
 */
void print_program(struct StatementNode* program);


#endif
