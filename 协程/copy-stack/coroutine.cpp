#include "coroutine.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

char *StackBottom;
using namespace std;

#define Terminated(C) ((C)->StackBuffer==NULL && (C)->BufferSize)

static Coroutine *Current = 0, *Next;

static void Error(const char *Message) 
{
	 cerr << "Error: " << Message << endl;
	 exit(0);
}

static class MainCoroutine : public Coroutine {
public:
 MainCoroutine() { Current = this; }
 void Routine() {}
} Main;

Coroutine::Coroutine(size_t Dummy) 
{
 char X;
 if (StackBottom)
 if(&X <= (char*) this && (char*) this <= StackBottom)
 	Error("Attempt to allocate a Coroutine on the stack");
 
 StackBuffer = 0; 
 Low = High = 0; 
 BufferSize = Dummy = 0;
 Callee = Caller = 0;
}

Coroutine::~Coroutine() {
 delete StackBuffer; 
 StackBuffer = 0;
}

inline void Coroutine::RestoreStack() 
{
	char X;
	if (&X >= Low && &X <= High) RestoreStack();
	Current = this;
	memcpy(Low, StackBuffer, High - Low);
	longjmp(this->Environment, 1);
}

inline void Coroutine::StoreStack() 
{
	 if (!Low) 
	 {
		 if (!StackBottom)
			 Error("StackBottom is not initialized");
		Low = High = StackBottom;
	 }
	 
	 char X;
	 
	 Low = &X;
 	
	 if (High - Low > BufferSize) 
	 {
	 	delete StackBuffer;
	 	BufferSize = High - Low;
	 	
	 	if (!(StackBuffer = new char[BufferSize]))
	 		Error("No more space available");
	 }
	 memcpy(StackBuffer, Low, High - Low);
}

inline void Coroutine::Enter() 
{
	if (!Terminated(Current)) 
	{
		Current->StoreStack(); //保存堆栈

		if (setjmp(Current->Environment)) //保存执行环境
			return;
	}
 
	Current = this;
	
	if (StackBuffer == NULL) 
	{
		Routine(); //执行用户指定操作
		delete this->StackBuffer;
		this->StackBuffer = 0;
		
		Detach();
	}
	RestoreStack();
}

void Resume(Coroutine *Next) {
 if (!Next)
 	Error("Attempt to Resume a non-existing Coroutine");
 if (Next == Current)
 	return;
 if (Terminated(Next))
 	Error("Attempt to Resume a terminated Coroutine");
 	
 if (Next->Caller)
 	Error("Attempt to Resume an attached Coroutine");
 	
 while (Next->Callee)
 	Next = Next->Callee;
 	
 Next->Enter();
}

void Call(Coroutine *Next) {
 if (!Next)
 	Error("Attempt to Call a non-existing Coroutine");

 cout << "\nbufsize:"<<Next->BufferSize<<endl;
 if (Terminated(Next))
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

void Detach() 
{
	Next = Current->Caller;
	
	if (Next)
		Current->Caller = Next->Callee = 0;
	else 
	{
		Next = &Main;
		while (Next->Callee)
			Next = Next->Callee;
	}
	
	Next->Enter();
}

Coroutine *CurrentCoroutine() { return Current; }
Coroutine *MainCoroutine() { return &Main; }