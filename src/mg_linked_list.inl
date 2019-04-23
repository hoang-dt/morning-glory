#pragma once

#include "mg_assert.h"
#include "mg_macros.h"
#include "mg_types.h"

namespace mg {

template <typename t> mg_ForceInline
list<t>::list(allocator* Alloc) : Alloc(Alloc) {}

template <typename t>
list_iterator<t> Insert(list<t>* List, list_iterator<t> Where, const t& Payload)
{
  buffer Buf;
  List->Alloc->Alloc(&Buf, sizeof(list_node<t>));
  list_node<t>* NewNode = (list_node<t>*)Buf.Data;
  NewNode->Payload = Payload;
  NewNode->Next = nullptr;
  if (Where.Node) {
    NewNode->Next = Where->Next;
    Where->Next = NewNode;
  }
  ++List->Size;
  return list_iterator<t>{NewNode};
}

template <typename t>
list_iterator<t> PushBack(list<t>* List, const t& Payload) {
  auto Node = List->Head;
  list_node<t>* Prev = nullptr;
  while (Node) {
    Prev = Node;
    Node = Node->Next;
  }
  auto NewNode = Insert(List, list_iterator<t>{Prev}, Payload);
  if (!Prev) // this new node is the first node in the list
    List->Head = NewNode.Node;
  return NewNode;
}

template <typename t>
void Dealloc(list<t>* List) {
  auto Node = List->Head;
  while (Node) {
    buffer Buf((byte*)Node, sizeof(list_node<t>), List->Alloc);
    Node = Node->Next;
    List->Alloc->Dealloc(&Buf);
  }
  List->Head = nullptr;
  List->Size = 0;
}

template <typename t> mg_ForceInline
i64 Size(const list<t>& List) {
  return List.Size;
}

template <typename t> mg_ForceInline
list_iterator<t>& list_iterator<t>::operator++() {
  mg_Assert(Node);
  Node = Node->Next;
  return *this;
}

template <typename t> mg_ForceInline
list_node<t>* list_iterator<t>::operator->() const {
  mg_Assert(Node);
  return Node;
}

template <typename t> mg_ForceInline
t& list_iterator<t>::operator*() const {
  mg_Assert(Node);
  return Node->Payload;
}

template <typename t> mg_ForceInline
bool list_iterator<t>::operator!=(list_iterator<t> Other) const {
  return Node != Other.Node;
}
template <typename t> mg_ForceInline
bool list_iterator<t>::operator==(list_iterator<t> Other) const {
  return Node == Other.Node;
}

template <typename t> mg_ForceInline
list_iterator<t> Begin(list<t>& List) {
  return list_iterator<t>{List.Head};
}
template <typename t> mg_ForceInline
list_iterator<t> End(list<t>& List) {
  (void)List;
  return list_iterator<t>();
}
template <typename t> mg_ForceInline
list_iterator<const t> ConstBegin(const list<t>& List) {
  return list_iterator<const t>{(list_node<const t>*)List.Head};
}
template <typename t> mg_ForceInline
list_iterator<const t> ConstEnd(const list<t>& List) {
  (void)List;
  return list_iterator<const t>();
}

} // namespace mg

