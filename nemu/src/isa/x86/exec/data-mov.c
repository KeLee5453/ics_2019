#include "cpu/exec.h"

make_EHelper(mov)
{
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push)
{
  switch (decinfo.opcode & 0xff)
  {
  case 0x0e: // push cs
    rtl_li(&s0, cpu.CS);
    rtl_push(&s0);
    break;
  case 0x16: // push ss
    rtl_li(&s0, cpu.SS);
    rtl_push(&s0);
    break;
  case 0x1e: // push ds
    rtl_li(&s0, cpu.DS);
    rtl_push(&s0);
    break;
  case 0x06: // push es
    rtl_li(&s0, cpu.ES);
    rtl_push(&s0);
    break;
  case 0xa0: // push fs
    rtl_li(&s0, cpu.FS);
    rtl_push(&s0);
    break;
  case 0xa8: // push gs
    rtl_li(&s0, cpu.GS);
    rtl_push(&s0);
    break;
  default:
    rtl_push(&id_dest->val);
    break;
  }

  print_asm_template1(push);
}

make_EHelper(pop)
{
  rtl_pop(&s1);
  switch (decinfo.opcode & 0xff)
  {
  case 0x1f: // pop ds
    cpu.DS = s1;
    break;
  case 0x07: // pop es
    cpu.ES = s1;
    break;
  case 0x17: // pop ss
    cpu.SS = s1;
    break;
  case 0xa1: // pop fs
    cpu.FS = s1;
    break;
  case 0xa9: // pop gs
    cpu.GS = s1;
    break;
  default:
    operand_write(id_dest, &s1);
    break;
  }

  print_asm_template1(pop);
}

make_EHelper(pusha)
{
  // TODO();
  int width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_lr(&s0, R_ESP, width);
  rtl_lr(&s1, R_EAX, width);
  rtl_push(&s1);
  rtl_lr(&s1, R_ECX, width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EDX, width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EBX, width);
  rtl_push(&s1);
  rtl_push(&s0);
  rtl_lr(&s1, R_EBP, width);
  rtl_push(&s1);
  rtl_lr(&s1, R_ESI, width);
  rtl_push(&s1);
  rtl_lr(&s1, R_EDI, width);
  rtl_push(&s1);

  print_asm("pusha");
}

make_EHelper(popa)
{
  // TODO();
  int width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_pop(&s0);
  rtl_sr(R_EDI, &s0, width);
  rtl_pop(&s0);
  rtl_sr(R_ESI, &s0, width);
  rtl_pop(&s0);
  rtl_sr(R_EBP, &s0, width);
  rtl_pop(&s0);
  rtl_pop(&s0);
  rtl_sr(R_EBX, &s0, width);
  rtl_pop(&s0);
  rtl_sr(R_EDX, &s0, width);
  rtl_pop(&s0);
  rtl_sr(R_ECX, &s0, width);
  rtl_pop(&s0);
  rtl_sr(R_EAX, &s0, width);

  print_asm("popa");
}

make_EHelper(leave)
{
  // TODO();
  rtl_lr(&s0, R_EBP, 4);
  rtl_sr(R_ESP, &s0, 4);
  rtl_pop(&s0);
  rtl_sr(R_EBP, &s0, 4);

  print_asm("leave");
}
make_EHelper(enter)
{
  rtl_push(&cpu.ebp);
  rtl_mv(&cpu.ebp, &cpu.esp);
  rtl_subi(&cpu.esp, &cpu.esp, 0);
  print_asm("enter");
}
make_EHelper(cltd)
{
  if (decinfo.isa.is_operand_size_16)
  {
    // TODO();
    rtl_sext(&s0, &cpu.eax, 2);
    rtl_shri(&cpu.edx, &s0, 16);
  }
  else
  {
    // TODO();
    rtl_sari(&cpu.edx, &cpu.eax, 31);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl)
{
  if (decinfo.isa.is_operand_size_16)
  {
    // TODO();

    rtl_shli(&reg_l(R_EAX), &reg_l(R_EAX), 24);
    rtl_sari(&reg_l(R_EAX), &reg_l(R_EAX), 8);
    rtl_shri(&reg_l(R_EAX), &reg_l(R_EAX), 16);
  }
  else
  {
    // TODO();
    rtl_sext(&reg_l(R_EAX), &reg_l(R_EAX), 2);
  }

  print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx)
{
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(&s0, &id_src->val, id_src->width);
  operand_write(id_dest, &s0);
  print_asm_template2(movsx);
}

make_EHelper(movzx)
{
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea)
{
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
make_EHelper(movsb)
{
  int in = 1;
  rtl_lr(&s0, R_ESI, 4);
  rtl_lm(&s1, &s0, 1);
  s0 += in;
  rtl_sr(R_ESI, &s0, 4);
  rtl_lr(&s0, R_EDI, 4);
  rtl_sm(&s0, &s1, 1);
  s0 += in;
  rtl_sr(R_EDI, &s0, 4);
  print_asm_template2(movsb);
}