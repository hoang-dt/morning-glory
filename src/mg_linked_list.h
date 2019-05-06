#pragma once

#include "mg_common.h"
#include "mg_macros.h"
#include "mg_memory.h"

namespace mg {

mg_T(t)
struct list_node {
  t Payload;
  list_node* Next = nullptr;
};

mg_T(t)
struct list {
  list_node<t>* Head = nullptr;
  allocator* Alloc = nullptr;
  i64 Size = 0;
  list(allocator* Alloc = &Mallocator());
};

mg_T(t)
struct list_iterator {
  list_node<t>* Node = nullptr;
  list_iterator& operator++();
  list_node<t>* operator->();
  t& operator*();
  bool operator!=(const list_iterator& Other);
  bool operator==(const list_iterator& Other);
};

mg_T(t) list_iterator<t>
Begin(list<t>& List);
mg_T(t) list_iterator<t>
End(list<t>& List);

mg_T(t) list_iterator<t>
Insert(list<t>* List, const list_iterator<t>& Where, const t& Payload);

mg_T(t) list_iterator<t>
PushBack(list<t>* List, const t& Payload);

mg_T(t) void
Dealloc(list<t>* List);

mg_T(t) i64
Size(const list<t>& List);


} // namespace mg

#include "mg_linked_list.inl"

