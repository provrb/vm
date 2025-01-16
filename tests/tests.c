#include <stdio.h>
#include <assert.h>

#include "src/api/macros.h"

Instruction program[] = {    
    INST_PUSH(4),
    INST_PUSH(3),
    INST_ORB(),
    INST_PRNT(),
};

#include "api/inst.h"

int main() {
    printf("hello");
    return 0;
}