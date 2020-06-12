#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc)
{
  if (cpu.pc != ref_r->pc)
  {
    printf("input pc %x\n", cpu.pc);
    printf("ref pc %x\n", ref_r->pc);
    printf("wrong in pc\n");
    return false;
  }
  for (int index = 0; index < 8; index++)
  {
    if (cpu.gpr[index]._32 != ref_r->gpr[index]._32)
    {
      printf("wrong in gpr[%d], value is %x\n", index, ref_r->gpr[index]._32);
      return false;
    }
  }
    return true;
  }


void isa_difftest_attach(void)
{
}
