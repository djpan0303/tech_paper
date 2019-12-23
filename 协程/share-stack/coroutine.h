#ifndef Sequencing
#include <stddef.h>
#include <setjmp.h>
#define DEFAULT_STACK_SIZE 10000
#define Sequencing(S) { InitSequencing(DEFAULT_STACK_SIZE); S; }
class Task;
class Coroutine 
{
	friend void Resume(Coroutine *);
	friend void Call(Coroutine *);
	friend void Detach();
	friend class Process;
	friend void InitSequencing(size_t main_StackSize);
	protected:
	 Coroutine(size_t StackSize = DEFAULT_STACK_SIZE);
	 virtual void Routine() = 0;
	private:
	 void Enter();
	 void Eat();
	 Task *MyTask;
	 size_t StackSize;
	 int Ready, Terminated;
	 Coroutine *Caller, *Callee;
	 static Coroutine *ToBeResumed;
};
void Resume(Coroutine *);
void Call(Coroutine *);
void Detach();
Coroutine *CurrentCoroutine();
Coroutine *MainCoroutine();
void InitSequencing(size_t main_StackSize);

struct Task {
 Coroutine *MyCoroutine;
 jmp_buf jmpb;
 int used;
 size_t size;
 struct Task *pred, *suc;
 struct Task *prev, *next;
};
#endif