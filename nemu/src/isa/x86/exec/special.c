#include "cpu/exec.h"
#include "monitor/monitor.h"
void isa_exec(vaddr_t *pc);
make_EHelper(nop)
{
  print_asm("nop");
}

make_EHelper(inv)
{
  /* invalid opcode */

  uint32_t temp[2];
  *pc = cpu.pc;
  temp[0] = instr_fetch(pc, 4);
  temp[1] = instr_fetch(pc, 4);

  uint8_t *p = (void *)temp;
  printf("invalid opcode(PC = 0x%08x): %02x %02x %02x %02x %02x %02x %02x %02x ...\n\n",
         cpu.pc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

  display_inv_msg(cpu.pc);

  rtl_exit(NEMU_ABORT, cpu.pc, -1);

  print_asm("invalid opcode");
}

make_EHelper(nemu_trap)
{
  difftest_skip_ref();

  rtl_exit(NEMU_END, cpu.pc, cpu.eax);

  print_asm("nemu trap");
  return;
}
make_EHelper(endbr32)
{
  decinfo.seq_pc += 3;
  // rtl_push(&cpu.ebp);
  // rtl_mv(&cpu.eax,&cpu.esp);
  // s0=4;
  // rtl_sub(&cpu.esp,&cpu.esp, &s0);
  print_asm("endbr32");
}

make_EHelper(notrack)
{
  isa_exec(pc);
}