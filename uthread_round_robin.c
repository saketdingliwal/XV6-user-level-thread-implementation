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
  char*   thread_name;
};
static thread_t all_thread[MAX_THREAD];
thread_p  current_thread;
thread_p  next_thread;
extern void thread_switch(void);
int starvation = 0;
int donation = 0;
void 
thread_init(void)
{
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
  current_thread->priority = -1e9;
  current_thread->counter = 0;
  current_thread->thread_name = (char *)malloc(strlen("init")*sizeof(char));
  strcpy(current_thread->thread_name,"init");
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
  if(starvation==1)
  {
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

int 
thread_create(char * name,void (*func)(),int priority)
{
  if(priority<0 && priority >20)
  {
    printf(2,"%s\n","priority value is wrong\n");
    return -1;
  }
  thread_p t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  if(t==all_thread+MAX_THREAD)
  {
    printf(2,"%s\n","No Free Thread is available\n");
    return -1;
  }

  t->sp = (int) (t->stack + STACK_SIZE);   // set sp to the top of the stack
  t->sp -= 4;                              // space for return address
  * (int *) (t->sp) = (int)func;           // push return address on stack
  t->sp -= 32;                             // space for registers that thread_switch will push
  t->state = RUNNABLE;
  t->thread_name = (char *)malloc(strlen(name)*sizeof(char));
  strcpy(t->thread_name,name);
  t->locked_on = -1;
  t->priority = priority;
  t->counter = 0;
  return 1;
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
void lock_busy_wait_acquire(int lock_no)
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
void lock_busy_wait_release(int lock_no)
{
  if(current_thread==lock_table[lock_no].aquirer)
    lock_table[lock_no].lock_value = 0;
}

void lock_acquire(int lock_no)
{
  if(lock_table[lock_no].lock_value==1)
  {
    if(donation==1)
    {
      if(current_thread->priority > lock_table[lock_no].aquirer->priority)
        lock_table[lock_no].aquirer->priority = current_thread->priority;
    }
    current_thread->state = WAITING;
    current_thread->locked_on = lock_no;
    thread_schedule();
  }
  lock_table[lock_no].lock_value = 1;
  lock_table[lock_no].aquirer = current_thread;
  lock_table[lock_no].aquirer_org_priority = current_thread->priority;
  // while(1)
  // {
  //   if(lock_table[lock_no].lock_value==0)
  //   {
  //     lock_table[lock_no].lock_value = 1;
  //     lock_table[lock_no].aquirer = current_thread;
  //     lock_table[lock_no].aquirer_org_priority = current_thread->priority;
  //     break;
  //   }
  //   if(donation==1)
  //   {
  //     if(current_thread->priority > lock_table[lock_no].aquirer->priority)
  //       lock_table[lock_no].aquirer->priority = current_thread->priority;
  //   }
  //   current_thread->state = WAITING;
  //   current_thread->locked_on = lock_no;
  //   thread_schedule();
  // }
}
void lock_release(int lock_no)
{
  if(current_thread==lock_table[lock_no].aquirer)
  {
    lock_table[lock_no].lock_value = 0;
    current_thread->priority = lock_table[lock_no].aquirer_org_priority;
    thread_p t;
    thread_p max_priority=0;
    for (t = all_thread; t < all_thread + MAX_THREAD; t++) 
    {
      if (t->state == WAITING && t->locked_on == lock_no) 
      {
        if(max_priority==0 || t->priority > max_priority->priority)
          max_priority = t;
      }
    }
    if(max_priority!=0)
    {
      max_priority->locked_on = -1;
      max_priority->state = RUNNABLE;            
    }
  }
}




static void 
test1(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  for(int i=0;i<10;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}

static void 
test2(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  thread_create("grand_child",test1,1);
  for(int i=0;i<10;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}


static void
test3(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  lock_busy_wait_acquire(1);
  for(int i=0;i<10;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  lock_busy_wait_release(1);
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}

static void
test4(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  lock_acquire(1);
  for(int i=0;i<10;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  lock_release(1);
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}

static void
test5(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  lock_acquire(2);
  for(int i=0;i<10;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  lock_release(2);
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}

static void
test8(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  for(int i=0;i<100;i++)
  {
    printf(1,"thread %s is yielding\n",current_thread->thread_name);
    thread_yield();
  }
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}

static void 
test9(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  lock_acquire(1);
  thread_create("mediocre",test1,10);
  thread_create("highest",test4,20);
  for (int i = 0; i < 10; i++) 
  {
   printf(1,"thread %s is yielding with priority %d \n",current_thread->thread_name,current_thread->priority);
    thread_yield();
  }
  lock_release(1);
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}



static void 
test11(void)
{
  printf(1, "thread %s created\n",current_thread->thread_name);
  lock_acquire(1);
  lock_acquire(2);
  thread_create("mediocre",test1,10);
  thread_create("less_supreme",test4,20);
  thread_create("supreme",test5,22);
  for (int i = 0; i < 10; i++) 
  {
   printf(1,"thread %s is yielding with priority %d \n",current_thread->thread_name,current_thread->priority);
    thread_yield();
  }
  lock_release(2);
  for (int i = 0; i < 10; i++) 
  {
   printf(1,"thread %s is yielding with priority %d \n",current_thread->thread_name,current_thread->priority);
    thread_yield();
  } 
  lock_release(1);
  printf(1, "thread %s exit \n" , current_thread->thread_name);
  current_thread->state = FREE;
  thread_schedule();
}


int 
main(int argc, char *argv[]) 
{
  lock_init();
  thread_init();
  thread_create("thread_1",test1,1);
  thread_create("thread_2",test1,1); 
  thread_yield();
  printf(1,"reached end of test1 simple threads \n");

  thread_create("child",test2,1);
  thread_yield();
  printf(1,"reached end of test2 nested threads \n");

  thread_create("thread_1",test3,1);
  thread_create("thread_2",test3,1);
  thread_yield();
  printf(1,"reached end of test3 busy wait lock implementation \n");

  thread_create("thread_1",test4,1);
  thread_create("thread_2",test4,1);
  thread_create("thread_3",test4,1);
  thread_yield();
  printf(1,"reached end of test4 non-busy wait lock implementation \n");

  thread_create("thread_1",test4,1);
  thread_create("thread_2",test5,1);
  thread_create("thread_3",test4,1);
  thread_yield();
  printf(1,"reached end of test4 non-busy wait multiple lock implementation \n");

  thread_create("thread_1",test1,1);
  thread_create("thread_2",test1,2); 
  thread_create("thread_3",test1,2);
  thread_create("thread_4",test1,2);
  thread_create("thread_5",test1,1);      
  thread_yield();
  printf(1,"reached end of test5 simple circular fifo priority scheduling  \n");

  thread_create("thread_1",test5,1);
  thread_create("thread_2",test5,2); 
  thread_create("thread_3",test5,2);
  thread_create("thread_4",test4,2);
  thread_create("thread_5",test4,1);      
  thread_yield();
  printf(1,"reached end of test6 locks based simple circular fifo priority scheduling  \n");

  thread_create("thread_1",test2,2);
  thread_create("thread_2",test5,1);      
  thread_yield();
  printf(1,"reached end of test7 nested based simple circular fifo priority scheduling  \n");

  starvation = 1;
  thread_create("thread_1",test8,2);
  thread_create("thread_2",test8,1);      
  thread_yield();
  printf(1,"reached end of test8  starvation based simple circular fifo priority scheduling  \n");

  thread_create("thread_1",test9,1);
  thread_yield();
  printf(1,"reached end of test9  priority inversion  \n");

  donation=1;
  thread_create("thread_1",test9,1);
  thread_yield();
  printf(1,"reached end of test10 priority donation \n");  

  thread_create("thread_1",test11,1);
  thread_yield();
  printf(1,"reached end of test11 priority donation with donation to multiple locks \n");  
  current_thread->state = FREE;
  exit();
}
