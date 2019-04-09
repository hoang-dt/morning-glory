#pragma once

#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

template <typename t>
struct list_node {
  t Payload;
  list_node* Next = nullptr;
};

template <typename t>
struct list {
  list_node<t>* Head = nullptr;
  allocator* Alloc = nullptr;
  i64 Size = 0;
  list(allocator* Alloc = &Mallocator());
};

// TODO: add an actual *const* iterator
template <typename t>
struct list_iterator {
  list_node<t>* Node = nullptr;
  list_iterator& operator++();
  list_node<t>* operator->() const;
  t& operator*() const;
  bool operator!=(list_iterator Other) const;
  bool operator==(list_iterator Other) const;
};

template <typename t> list_iterator<t> Begin(list<t>& List);
template <typename t> list_iterator<t> End(list<t>& List);
template <typename t> list_iterator<const t> ConstBegin(const list<t>& List);
template <typename t> list_iterator<const t> ConstEnd(const list<t>& List);

struct allocator;

template <typename t>
list_iterator<t> Insert(list<t>* List, list_iterator<t> Where, const t& Payload);

template <typename t>
list_iterator<t> PushBack(list<t>* List, const t& Payload);

template <typename t>
void Dealloc(list<t>* List);

template <typename t>
i64 Size(const list<t>& List);


} // namespace mg

#include "mg_linked_list.inl"

