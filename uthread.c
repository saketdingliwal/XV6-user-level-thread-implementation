#include "types.h"
#include "stat.h"
#include "user.h"

/* Possible states of a thread; */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define WAITING     0x3

#define STACK_SIZE  8192
#define MAX_THREAD  10
#define MAX_LOCK    10


typedef struct thread thread_t, *thread_p;
typedef struct mutex mutex_t, *mutex_p;

struct thread {
  int        sp;                /* curent stack pointer */
  char stack[STACK_SIZE];       /* the thread's stack */
  int        state;             /* running, runnable, waiting */
  int        locked_on;           /* the one responsible for its waiting state */
  int         priority;
  int         counter;
};
static thread_t all_thread[MAX_THREAD];
thread_p  current_thread;
thread_p  next_thread;
extern void thread_switch(void);

void 
thread_init(void)
{
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
  current_thread->priority = -1e9;
  current_thread->counter = 0;
}


// assume that no thread have start address as 0
static void 
thread_schedule(void)
{
  thread_p t;
  /* Find another runnable thread. */
  // for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
  //   if (t->state == RUNNABLE && t != current_thread) {
  //     next_thread = t;
  //     break;
  //   }
  // }
  thread_p max_priority = 0;

  for (t = current_thread + 1; t < all_thread + MAX_THREAD; t++) 
  {
    if (t->state == RUNNABLE) 
    {

      if(max_priority == 0 || t->priority > max_priority->priority)
        max_priority = t;
    }
  }
  for(t = all_thread; t <= current_thread; t++)
  {
    if (t->state == RUNNABLE) 
    {
      if(max_priority == 0 || t->priority > max_priority->priority)
        max_priority = t;
    }
  }

  next_thread = max_priority;

  // if (t >= all_thread + MAX_THREAD && current_thread->state == RUNNABLE) {
  //    The current thread is the only runnable thread; run it. 
  //   next_thread = current_thread;
  // }
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) 
  {
    if (t->state == RUNNABLE && t != next_thread) 
    {
      t->counter +=1;
      if(t->counter==50)
      {
        t->counter=0;
        if(t->priority<20)
        {
          t->priority +=1;
        }
      }
    }
  }


  if (next_thread == 0) {
    printf(2, "thread_schedule: no runnable threads; deadlock\n");
    exit();
  }

  if(current_thread==next_thread)
    current_thread->state = RUNNING;

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    thread_switch();
  } else
    next_thread = 0;

}

void 
thread_create(void (*func)(),int priority)
{
  if(priority<0 && priority >20)
    return;
  thread_p t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->sp = (int) (t->stack + STACK_SIZE);   // set sp to the top of the stack
  t->sp -= 4;                              // space for return address
  * (int *) (t->sp) = (int)func;           // push return address on stack
  t->sp -= 32;                             // space for registers that thread_switch will push
  t->state = RUNNABLE;
  t->locked_on = -1;
  t->priority = priority;
  t->counter = 0;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

typedef struct lock_
{
  int lock_value;
  thread_p aquirer;
  int aquirer_org_priority;
}lock;

lock lock_table[MAX_LOCK];
void lock_init()
{
  for(int i=0;i<MAX_LOCK;i++)
  {
    lock_table[i].lock_value = 0;
    lock_table[i].aquirer = 0;
  }
}
void busy_wait_acquire(int lock_no)
{
  while(1)
  {
    if(lock_table[lock_no].lock_value==0)
    {
      lock_table[lock_no].lock_value = 1;
      lock_table[lock_no].aquirer = current_thread;
      break;
    }
    thread_yield();
  }
}
void busy_wait_release(int lock_no)
{
  if(current_thread==lock_table[lock_no].aquirer)
    lock_table[lock_no].lock_value = 0;
}

void non_busy_wait_acquire(int lock_no,int donation)
{
  while(1)
  {
    if(lock_table[lock_no].lock_value==0)
    {
      lock_table[lock_no].lock_value = 1;
      lock_table[lock_no].aquirer = current_thread;
      lock_table[lock_no].aquirer_org_priority = current_thread->priority;
      break;
    }
    if(donation==1)
    {
      if(current_thread->priority > lock_table[lock_no].aquirer->priority)
        lock_table[lock_no].aquirer->priority = current_thread->priority;
    }
    current_thread->state = WAITING;
    current_thread->locked_on = lock_no;
    thread_schedule();
  }
}
void non_busy_wait_release(int lock_no,int donation)
{
  if(current_thread==lock_table[lock_no].aquirer)
  {
    lock_table[lock_no].lock_value = 0;
    current_thread->priority = lock_table[lock_no].aquirer_org_priority;
    thread_p t;
    for (t = all_thread; t < all_thread + MAX_THREAD; t++) 
    {
      if (t->state == WAITING && t->locked_on == lock_no) 
      {
        t->locked_on = -1;
        t->state = RUNNABLE;
      }
    }
  }
}

static void 
mythread(void)
{
  int i;
  non_busy_wait_acquire(0,1);
  for (i = 0; i < 10; i++) {
    printf(1, "my thread 0x%x %d\n", (int) current_thread,current_thread->priority);
    thread_yield();
  }
  non_busy_wait_release(0,1);
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  thread_schedule();
}

static void 
mythread2(void)
{
  for(int i=0;i<10;i++)
  {
    printf(1,"%d is running\n", current_thread->priority);
    thread_yield();
  }
  printf(1, "my thread: exit\n");
  current_thread->state = FREE;
  thread_schedule();
}


int 
main(int argc, char *argv[]) 
{
  lock_init();
  thread_init();
  thread_create(mythread,1);
  thread_create(mythread2,2);
  thread_create(mythread,4);
  thread_yield();



  current_thread->state = FREE;
  exit();
}
