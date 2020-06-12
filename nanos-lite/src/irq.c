#include "common.h"
#include "am.h"
static _Context *do_event(_Event e, _Context *c)
{
  extern _Context* do_syscall(_Context *c);
  extern _Context* schedule(_Context *prev);
  switch (e.event)
  {
  case _EVENT_IRQ_TIMER:
  _yield();
  break;
  case _EVENT_YIELD:
    // Log("This is yield.");
    return schedule(c);
    break;
  case _EVENT_SYSCALL:
    return do_syscall(c);
  default:
    panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void)
{
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
