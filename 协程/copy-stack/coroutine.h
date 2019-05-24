#ifndef Sequencing
#define Sequencing(S) {char Dummy; StackBottom = &Dummy; S;}
#include <stddef.h>
#include <setjmp.h>

extern char *StackBottom;
class Coroutine {
friend void Resume(Coroutine *);
friend void Call(Coroutine *);
friend void Detach();



protected:
	Coroutine(size_t Dummy = 0);
	~Coroutine();
	virtual void Routine() = 0;
private:
	void Enter();
	void StoreStack();
	void RestoreStack();
	char *StackBuffer; //用来保存runtime stack
	char *Low, *High; // runtime stack的上下界
	size_t BufferSize; // runtime stack区的大小
	jmp_buf Environment; //用来保存挂起点的上下文(寄存器+执行点)
	Coroutine *Caller, *Callee;
};

void Resume(Coroutine *);
void Call(Coroutine *);
void Detach();
Coroutine *CurrentCoroutine();
Coroutine *MainCoroutine();
#define DEFAULT_STACK_SIZE 0
#endif