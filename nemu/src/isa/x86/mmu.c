#include "nemu.h"

inline paddr_t page_translate(vaddr_t addr)
{
  paddr_t PDT_base = PTE_ADDR(cpu.cr3.val);
  assert(paddr_read(PDT_base + PDX(addr) * sizeof(PDE), sizeof(PDE)) & PTE_P);
  paddr_t PTE_base = PTE_ADDR(paddr_read(PDT_base + PDX(addr) * sizeof(PDE), sizeof(PDE)));
  assert(paddr_read(PTE_base + PTX(addr) * sizeof(PTE), sizeof(PTE)) & PTE_P);
  paddr_t PF_base = PTE_ADDR(paddr_read(PTE_base + PTX(addr) * sizeof(PTE), sizeof(PTE)));
  paddr_t paddr = PF_base | OFF(addr);
  return paddr;
}

uint32_t isa_vaddr_read(vaddr_t addr, int len)
{
  if (cpu.cr0.paging)
  { // 开启分页
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1))
    { // 数据跨页
      uint8_t byte[4];
      for (int i = 0; i < len; i++)
        byte[i] = isa_vaddr_read(addr + i, 1);
      if (len == 2)
        return *(uint16_t *)byte;
      else
        return *(uint32_t *)byte;
    }
    else
    {
      paddr_t paddr = page_translate(addr);
      return paddr_read(paddr, len);
    }
  }
  else
  {
    return paddr_read(addr, len);
  }
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len)
{
  if (cpu.cr0.paging)
  { // 开启分页
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1))
    { // 数据跨页
      uint8_t byte[4];
      if (len == 2)
        *(uint16_t *)byte = data;
      else
        *(uint32_t *)byte = data;
      for (int i = 0; i < len; i++)
        isa_vaddr_write(addr + i, byte[i], 1);
    }
    else
    { // 数据没有跨页
      paddr_t paddr = page_translate(addr);
      paddr_write(paddr, data, len);
    }
  }
  else
  { // 不分页
    paddr_write(addr, data, len);
  }
}
