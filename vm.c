#include <stdio.h>
#include <assert.h>
#include "runtime.h"
#include "opcode.h"

// Helpers to play with the stack
#define STACK_MAX      10
#define STACK_PUSH(I)  do {                             \
                          assert(sp-stack < STACK_MAX); \
                          *(++sp) = (I);                \
                       } while(0)
#define STACK_POP()    (*sp--)


void run(long literals[], byte instructions[]) {
  byte    *ip = instructions;      // instruction pointer
  
  Object  *stack[STACK_MAX];       // the famous stack
  Object **sp = stack;             // stack pointer
  
  Object  *locals[STACK_MAX] = {}; // where we store our local variables
  
  // Setup the runtime
  Object *self = Object_new();
  
  // Start processing instructions
  while (1) {
    switch (*ip) {
      case CALL:{
        ip++; // move to the method literal index (1st operand)
        char *method = (char *)literals[*ip];
        ip++; // moving to the number of arguments (2nd operand)
        int argc = *ip;
        Object *argv[10]; // array of arguments
        
        int i;
        for(i = 0; i < argc; ++i) argv[i] = STACK_POP();
        Object *receiver = STACK_POP();
        
        Object *result = call(receiver, method, argv, argc);
        
        STACK_PUSH(result);
        
        break;
      }
      case PUSH_NUMBER:
        ip++; // index of number literal
        STACK_PUSH(Number_new((int)literals[*ip]));
        break;
      
      case PUSH_STRING:
        ip++; // index of string literal
        STACK_PUSH(String_new((char *)literals[*ip]));
        break;
      
      case PUSH_SELF:
        STACK_PUSH(self);
        break;
      
      case PUSH_NIL:
        STACK_PUSH(NilObject);
        break;
      
      case PUSH_BOOL:
        ip++;
        if (*ip == 0) {
          STACK_PUSH(FalseObject);
        } else {
          STACK_PUSH(TrueObject);
        }
        break;
      
      case GET_LOCAL:
        ip++; // index of local
        STACK_PUSH(locals[*ip]);
        break;
      
      case SET_LOCAL:
        ip++; // index of local
        locals[*ip] = STACK_POP();
        break;
      
      case ADD:{
        Object *a = STACK_POP();
        Object *b = STACK_POP();
        
        STACK_PUSH(Number_new(Number_value(a) + Number_value(b)));
        
        break;
      }
      case JUMP_UNLESS:{
        ip++; // offset, nb of butes to move forward unless true on stack
        byte offset = *ip;
        Object *test = STACK_POP();
        
        if (!Object_is_true(test)) ip += offset;
        
        break;
      }
      case RETURN:
        return;
      
    }
    ip++;
  }
}

int main (int argc, char const *argv[]) {
  // long can store a pointer (and numbers too).
  long literals[] = {
    (long) "the answer is:",
    (long) "print",
    (long) 30,
    (long) 2
  };
  
  // print("the answer is:")
  // a = 30 + 2
  // if true
  //   print(a)
  // end
  
  byte instructions[] = {
    PUSH_SELF,
    PUSH_STRING, 0, // [self, "the answer is:"]
    CALL,        1, 1, // print
    PUSH_NUMBER, 2, // [30]
    PUSH_NUMBER, 3, // [30, 2]
    ADD,            // [32]
    SET_LOCAL,   0, // a
    PUSH_BOOL,   1, // [true]
    JUMP_UNLESS, 6,
    PUSH_SELF,
    GET_LOCAL,   0, // [32]
    CALL,        1, 1,
    RETURN
  };
  
  init_runtime();
  run(literals, instructions);
  destroy_runtime();
  
  return 0;
}