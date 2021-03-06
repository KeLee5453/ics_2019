#include "proc.h"

PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL; 
int fg_pcb =0;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void run_proc(PCB *pcb) {
  extern void _urun(_AddressSpace *as, void (*entry)());
  current = pcb;
  _urun(&pcb->as, (void (*)())pcb->cp->eip);
}

void init_proc() {
  extern void context_kload(PCB *pcb, void *entry);
  extern void context_uload(PCB *pcb, const char *filename);

  //context_kload(&pcb[0], (void *)hello_fun);
  //switch_boot_pcb();

  Log("Initializing processes...");
fg_pcb=1;
  // load program here
  //naive_uload(NULL, "/bin/dummy");
  context_uload(&pcb[0], "/bin/bmptest");
  context_uload(&pcb[1], "/bin/pal");
  context_uload(&pcb[2], "/bin/hello");
  context_uload(&pcb[3], "/bin/pal");

}

_Context* schedule(_Context *prev) {
    //return NULL;
  // save the context pointer
  current->cp = prev;

  // always select pcb[0] as the new process
  current = (current == &pcb[0] ? &pcb[fg_pcb] : &pcb[0]);
  //current = &pcb[0];
  // then return the new context
  return current->cp;
}
