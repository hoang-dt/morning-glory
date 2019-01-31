#pragma once

#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_types.h"

namespace mg {

template <typename t> mg_ForceInline
linked_list<t>::linked_list(allocator* Alloc) : Alloc(Alloc) {}

template <typename t> 
linked_list_iterator<t> Insert(linked_list<t>* List, linked_list_iterator<t> Where, const t& Payload) {
  buffer Buf;
  List->Alloc->Allocate(&Buf, sizeof(linked_list_node<t>));
  linked_list_node<t>* NewNode = (linked_list_node<t>*)Buf.Data;
  NewNode->Payload = Payload;
  NewNode->Next = nullptr;
  if (Where.Node) {
    NewNode->Next = Where->Next;
    Where->Next = NewNode;
  }
  return linked_list_iterator<t>{NewNode};
}

template <typename t>
linked_list_iterator<t> PushBack(linked_list<t>* List, const t& Payload) {
  auto Node = List->Head;
  linked_list_node<t>* Prev = nullptr;
  while (Node) {
    Prev = Node;
    Node = Node->Next;
  }
  auto NewNode = Insert(List, linked_list_iterator<t>{Prev}, Payload);
  if (!Prev) // this new node is the first node in the list
    List->Head = NewNode.Node;
  return NewNode;
}

template <typename t>
void Deallocate(linked_list<t>* List) {
  auto Node = List->Head;
  while (Node) {
    buffer Buf((byte*)Node, sizeof(linked_list_node<t>), List->Alloc);
    Node = Node->Next;
    List->Alloc->Deallocate(&Buf);
  }
}

template <typename t> mg_ForceInline
i64 Size(const linked_list<t>& List) {
  i64 Result = 0;
  auto Node = List.Head;
  while (Node) {
    ++Result;
    Node = Node->Next;
  }
  return Result;
}

template <typename t> mg_ForceInline
linked_list_iterator<t>& linked_list_iterator<t>::operator++() {
  mg_Assert(Node); Node = Node->Next; return *this;
} 

template <typename t> mg_ForceInline
linked_list_node<t>* linked_list_iterator<t>::operator->() {
  mg_Assert(Node); return Node;
}

template <typename t> mg_ForceInline
bool linked_list_iterator<t>::operator!=(linked_list_iterator<t> Other) const {
  return Node != Other.Node;
}
template <typename t> mg_ForceInline
bool linked_list_iterator<t>::operator==(linked_list_iterator<t> Other) const {
  return Node == Other.Node;
}

template <typename t> mg_ForceInline
linked_list_iterator<t> Begin(linked_list<t>& List) {
  return linked_list_iterator<t>{List.Head};
}
template <typename t> mg_ForceInline
linked_list_iterator<t> End(linked_list<t>& List) {
  (void)List;
  return linked_list_iterator<t>();
}
template <typename t> mg_ForceInline
const linked_list_iterator<t> ConstBegin(const linked_list<t>& List) {
  return linked_list_iterator<t>{List.Head};
}
template <typename t> mg_ForceInline
const linked_list_iterator<t> ConstEnd(const linked_list<t>& List) {
  (void)List;
  return linked_list_iterator<t>();
}

} // namespace mg

