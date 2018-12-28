#ifndef TEMPLATE_LIST_H
#define TEMPLATE_LIST_H

#include <stdlib.h>

// Node for the list class
template <class T>
class ListNode {
public:
	ListNode();
	~ListNode();

	T Get();
	ListNode* GetNext();
	ListNode* GetPrev();

	void Set(T Input);
	void SetNext(ListNode* Ptr);
	void SetPrev(ListNode* Ptr);

	ListNode<T>& operator=(const ListNode<T>& Source);
	bool operator>(const ListNode<T>& Source);
	bool operator<(const ListNode<T>& Source);

private:
	T Data;
	ListNode<T>* Next; 
	ListNode<T>* Prev;
};

// Constructor 
template <class T>
ListNode<T>::ListNode()
{
	Next = this;
	Prev = this;
}

// Destructor
template <class T>
ListNode<T>::~ListNode()
{

}

// Get methods
template <class T>
T ListNode<T>::Get()
{
	return Data;
}

template <class T>
ListNode<T>* ListNode<T>::GetNext()
{
	return Next;
}

template <class T>
ListNode<T>* ListNode<T>::GetPrev()
{
	return Prev;
}

// Set Methods
template <class T>
void ListNode<T>::Set(T Input)
{
	Data = Input;
}

template <class T>
void ListNode<T>::SetNext(ListNode<T>* Ptr)
{
	Next = Ptr;
}

template <class T>
void ListNode<T>::SetPrev(ListNode<T>* Ptr)
{
	Prev = Ptr;
}

/* Operators */
template <class T>
ListNode<T>& ListNode<T>::operator=(const ListNode<T>& Source)
{
	this->Data = Source.Data;
	this->Next = Source.Next;
	this->Prev = Source.Prev;
}


template <class T>
bool ListNode<T>::operator>(const ListNode<T>& Source)
{
	return (this->Data > Source.Data);
}

template <class T>
bool ListNode<T>::operator<(const ListNode<T>& Source)
{
	return (this->Data < Source.Data);
}


// List class.  The template assumes that the type T has the following operators define: ==, >,
//	<, and =.
template <class T>
class List {
public:
	List();
	~List();

	void DeleteList();
	int Length();
	bool Empty();
	bool Tail(const ListNode<T>* Ptr);
	bool Insert(T Data);
	bool Delete(T& Data);
	void Next();
	void Prev();
	T GetHead();
	T GetCurrent();
	void ResetCurrent();
	void Sort();
private:
	ListNode<T>* Head; 
	ListNode<T>* Current;
	int Len;
	
	void __Insert(ListNode<T>* Ptr, ListNode<T>* Next, ListNode<T> *Prev);
	void __Delete(ListNode<T>* Ptr);
	void Swap(ListNode<T>* Ptr1, ListNode<T>* Ptr2);
};

template <class T>
List<T>::List() 
{
	Current = Head = NULL;
	Len = 0;
}

template <class T>
List<T>::~List()
{
	DeleteList();
}

template <class T>
void List<T>::DeleteList()
{
	while (!Empty()) {
		__Delete(Head);
	}
}

template <class T>
int List<T>::Length()
{
	return Len;
}

template <class T>
bool List<T>::Empty()
{
	return (Len == 0);
}

template <class T>
bool List<T>::Tail(const ListNode<T>* Ptr)
{
	return (Head && Ptr &&(Ptr == Head->GetPrev()));
}


// Inserts at the tail of the list
template <class T>
bool List<T>::Insert(T Data)
{
	ListNode<T>* Ptr = new ListNode<T>;
	Ptr->Set(Data);
	
	if (!Empty()) {
		__Insert(Ptr, Head, Head->GetPrev());
	
	} else { // First node
		Current = Head = Ptr;
	}

	Len++;

	return true;
}

template <class T>
void List<T>::__Insert(ListNode<T>* Ptr, ListNode<T>* Next, ListNode<T> *Prev)
{
	Ptr->SetNext(Next);
	Ptr->SetPrev(Prev);
	Next->SetPrev(Ptr);
	Prev->SetNext(Ptr);
}

// Removes from the head of the list
template <class T>
bool List<T>::Delete(T& Data)
{
	if (!Empty()) {
		Data = Head->Get();
		__Delete(Head);
		return true;
	}

	return false;
}

template <class T>
void List<T>::__Delete(ListNode<T>* Ptr)
{
	if (!Ptr) {
		return;
	}

	ListNode<T>* Next = Ptr->GetNext();
	ListNode<T>* Prev = Ptr->GetPrev();


	if (Ptr == Head) {
		Head = Next;
	}

	if (Ptr == Current) {
		Current = Next;
	}

	Next->SetPrev(Prev);
	Prev->SetNext(Next);

	Len--;

	if (Empty()) {
		Current = Head = NULL;
	}

	delete Ptr;
	Ptr = NULL;
}

// Moves Current to the next or previous
template <class T>
void List<T>::Next()
{
	if (Current) {
		Current = Current->GetNext();
	}
}

template <class T>
void List<T>::Prev()
{
	if (Current) {
		Current = Current->GetPrev();
	}
}

/* Returns the data from the head of the list */
template <class T>
T List<T>::GetHead()
{
	return Head->Get();
}


// Returns the data from the current node of the list 
template <class T>
T List<T>::GetCurrent()
{
	return Current->Get();
}

// Reset the current pointer to the head of the list
template <class T>
void List<T>::ResetCurrent()
{
	Current = Head;
}

// Insertion sort - this is optimized for the default case (already sorted)
template <class T>
void List<T>::Sort()
{
	ListNode<T>* Ptr;
	ListNode<T>* Start;

	// Not not at the tail of the list
	for (Start = Head->GetNext(); Start != Head; Start = Start->GetNext()) {
		for (Ptr = Start; (Ptr != Head) && (Ptr->Get() < (Ptr->GetPrev())->Get()); Ptr = Ptr->GetPrev()) {
			Swap(Ptr, Ptr->GetPrev());
		}
	}

}

template <class T>
void List<T>::Swap(ListNode<T>* Ptr1, ListNode<T>* Ptr2)
{
	T Temp = Ptr1->Get();
	Ptr1->Set(Ptr2->Get());
	Ptr2->Set(Temp);

	if (Ptr1 == Current) {
		Current = Ptr2;
	} else if (Ptr2 == Current) {
		Current = Ptr1;
	}

}

#endif