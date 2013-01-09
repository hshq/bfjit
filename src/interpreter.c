#include "interpreter.h"

#include "bytecode.h"
#include "compiler.h"

void interpret(program_t *program, byte *arena, int arena_size) {
  int arena_idx = 0;
  byte *pc = program->bytecode;

  static const void *labels [BC_NUM_BYTECODES] = {
    [BC_SHIFT] = &&bc_shift, [BC_ADD] = &&bc_add,
    [BC_OUTPUT] = &&bc_output, [BC_INPUT] = &&bc_input,
    [BC_LOOP_BEGIN] = &&bc_loop_begin,
    [BC_LOOP_END] = &&bc_loop_end,
    [BC_HLT] = &&bc_hlt, [BC_COMPILED_LOOP] = &&bc_compiled_loop,
  };

  uint32_t payload;
  byte bytecode;

#define dispatch(pc) do {                       \
    payload = get_payload(pc, 0);               \
    bytecode = get_bytecode(pc);                \
    goto *labels[bytecode];                     \
  } while(0)

  dispatch(pc);

bc_shift:
  arena_idx += (int32_t) payload;
  if (unlikely(arena_idx < 0 || arena_idx >= arena_size)) {
    die("arena pointer out of bounds!");
  }
  pc += get_total_length(BC_SHIFT);
  dispatch(pc);

bc_add:
  arena[arena_idx] += (int32_t) payload;
  pc += get_total_length(BC_ADD);
  dispatch(pc);

bc_output:
  printf("%c", arena[arena_idx]);
  pc += get_total_length(BC_OUTPUT);
  dispatch(pc);

bc_input:
  scanf("%c", &arena[arena_idx]);
  pc += get_total_length(BC_INPUT);
  dispatch(pc);

bc_loop_begin:
  /*  A loop is expected to run at least a few times -- hence the
   *  `unlikely'  */
  if (unlikely(!arena[arena_idx])) {
    uint32_t delta = get_payload(pc, 1);
    pc += delta;
    dispatch(pc);
  }
  program->heat_counters[payload] --;
  if (unlikely(program->heat_counters[payload] == 0)) {
    int location = compile_and_install(program, pc);
    if (likely(location != -1)) {
      pc = program->compiled_code[location](arena);
      dispatch(pc);
    }
    program->heat_counters[payload] = kHotFunctionThreshold;
  }
  pc += get_total_length(BC_LOOP_BEGIN);
  dispatch(pc);

bc_loop_end:
  pc -= payload;
  dispatch(pc);

bc_compiled_loop:
  pc = program->compiled_code[get_payload(pc, 0)](arena);
  dispatch(pc);

bc_hlt:
  return;
}