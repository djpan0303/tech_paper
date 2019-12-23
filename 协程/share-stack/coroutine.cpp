#include "coroutine.h"
#include <iostream.h>
#include <stdlib.h>
#include <limits.h>
#define MIN_STACK_SIZE 500 // minimum stack size
static Task main_task; // the main task
static jmp_buf tmp_jmpb; // temporary jump buffer
static Coroutine *Current = 0; // current coroutine
static void Error(const char *Msg) {
 cerr << Msg << endl; exit(0);
}
Coroutine *Coroutine::ToBeResumed = 0;
class Main_Coroutine : public Coroutine {
friend class Process;
public:
 void Routine() {}
} Main;

Coroutine::Coroutine(size_t s = DEFAULT_STACK_SIZE) {
 Caller = Callee = 0; Ready = 1; Terminated = 0;
 StackSize = s;
}

void Coroutine::Enter() 
{
	 if (!Current)
	 	Error("InitSequencing has not been called");
	 	
	 if (Ready) 
	 { // find free block
		 for (MyTask = main_task.suc;MyTask != &main_task;MyTask = MyTask->suc)
			if (MyTask->size >= StackSize + MIN_STACK_SIZE)
		 		break;
		 	if (MyTask == &main_task)
		 		Error("No more space available\n");
		 MyTask->MyCoroutine = this;
		 
		 if (!setjmp(tmp_jmpb))
		 longjmp(MyTask->jmpb, 1);
		 Ready = 0;
	 }
	 if (!setjmp(Current->MyTask->jmpb)) 
	 { 
	 	// activate control block
	 	Current = this;
	 	longjmp(MyTask->jmpb, 1);
 	}
}
void Coroutine::Eat() {
 static size_t d;
 static Task *p;
 Task t;
 // eat stack
 if ((d = labs((char *) &t - (char *) MyTask)) <
 StackSize)
 Eat();
 t.size = MyTask->size - d; //set size
 MyTask->size = d;
 t.used = 0;
 t.suc = main_task.suc;
 t.pred = &main_task;
 t.suc->pred = main_task.suc = &t;
 if (MyTask->next != &t) {
 t.next = MyTask->next; // set link pointers
 MyTask->next = &t;
 t.prev = MyTask;
 if (t.next)
 t.next->prev = &t;
 }
 if (!setjmp(t.jmpb)) // wait
	longjmp(MyTask->jmpb, 1);
	
for (;;) {
 // test size
 if (StackSize + MIN_STACK_SIZE < t.size &&
 !setjmp(t.jmpb))
 t.MyCoroutine->Eat(); // split block
 t.used = 1; // mark as used
 t.pred->suc = t.suc;
 t.suc->pred = t.pred;
 if (!setjmp(t.jmpb)) // wait
 longjmp(tmp_jmpb, 1);
 t.MyCoroutine->Routine(); // execute Routine
 t.MyCoroutine->Terminated = 1;
 t.used = 0; // mark as free
 p = t.next;
 if (p && !p->used) { // merge with following block
 t.size += p->size;
 t.next = p->next;
 if (t.next)
 t.next->prev = &t;
 p->pred->suc = p->suc;
 p->suc->pred = p->pred;
 }
 p = t.prev;
 if (!p->used) { // merge with preceding block
 p->size += t.size;
 p->next = t.next;
 if (t.next)
 t.next->prev = p;
 }
 else {
 t.suc = main_task.suc;
 t.pred = &main_task;
 t.suc->pred = main_task.suc = &t;
 }
 if (!setjmp(t.jmpb)) { // save state
 if (ToBeResumed) {
 static Coroutine *Next;
 Next = ToBeResumed;
ToBeResumed = 0;
 Resume(Next);
 }
 else
 Detach();
 }
 }
}

void Resume(Coroutine *Next) {
 if (!Next)
 Error("Attempt to Resume a non-existing Coroutine");
 if (Next == Current)
 return;
 if (Next->Terminated)
 Error("Attempt to Resume a terminated Coroutine");
 if (Next->Caller)
 Error("Attempt to Resume an attached Coroutine");
 while (Next->Callee)
 Next = Next->Callee;
 Next->Enter();
}
void Call(Coroutine *Next) {
 if (!Next)
 Error("Attempt to Call a nonexisting Coroutine");
 if (Next->Terminated)
 Error("Attempt to Call a terminated Coroutine");
 if (Next->Caller)
 Error("Attempt to Call an attached Coroutine");
 Current->Callee = Next;
 Next->Caller = Current;
 while (Next->Callee)
 Next = Next->Callee;
 if (Next == Current)
 Error("Attempt to Call an operating Coroutine");
 Next->Enter();
}
void Detach() {
 Coroutine *Next = Current->Caller;
 if (Next)
 Current->Caller = Next->Callee = 0;
 else {
 Next = &Main;
 while (Next->Callee)
 Next = Next->Callee;
 }
 Next->Enter();
}
Coroutine *CurrentCoroutine() { return Current; }
Coroutine *MainCoroutine() { return &Main; } 

void InitSequencing(size_t main_StackSize) {
 Task tmp;
 tmp.size = ULONG_MAX;
 Main.StackSize = main_StackSize;
 tmp.next = 0;
 Main.MyTask = &tmp;
 main_task.pred = main_task.suc = &main_task;
 tmp.MyCoroutine = Current = &Main;
 if (!setjmp(tmp.jmpb))
 Main.Eat();
 tmp.pred = main_task.pred;
 tmp.suc = main_task.suc;
 main_task = tmp;
 main_task.next->prev = &main_task;
 Main.MyTask = &main_task;
 main_task.used = 1;
 Main.Ready = 0;
}