#pragma once

#include "mg_macros.h"
#include "mg_types.h"

namespace mg {

template <typename t> mg_ForceInline linked_list<t>::linked_list() = default;

template <typename t> mg_ForceInline
linked_list<t>::linked_list(const t& Payload, linked_list<t>* Next)
  : Payload(Payload), Next(Next) {}

template <typename t> 
linked_list<t>* AddNode(linked_list<t>* Where, const t& Payload, allocator* Alloc) {
  buffer Buf;
  Alloc->Allocate(&Buf, sizeof(linked_list<t>));
  linked_list<t>* NewNode = (linked_list<t>*)Buf.Data;
  NewNode->Payload = Payload;
  if (Where) {
    NewNode->Next = Where->Next;
    Where->Next = NewNode;
  }
  return NewNode;
}

template <typename t>
void Deallocate(linked_list<t>* List, allocator* Alloc) {
  while (List) {
    buffer Buf((byte*)List, sizeof(linked_list<t>), Alloc);
    List = List->Next;
    Alloc->Deallocate(&Buf);
  }
}

} // namespace mg

