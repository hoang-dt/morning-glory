#pragma once

#include "mg_memory.h"
#include "mg_types.h"

namespace mg {

template <typename t>
struct linked_list_node {
  t Payload;
  linked_list_node* Next = nullptr;
};

template <typename t>
struct linked_list {
  linked_list_node<t>* Head = nullptr;
  allocator* Alloc = nullptr;
  i64 Size = 0;
  linked_list(allocator* Alloc = &Mallocator());
};

template <typename t>
struct linked_list_iterator { 
  linked_list_node<t>* Node = nullptr; 
  linked_list_iterator& operator++();
  linked_list_node<t>* operator->();
  t& operator*();
  bool operator!=(linked_list_iterator Other) const;
  bool operator==(linked_list_iterator Other) const;
};

template <typename t> linked_list_iterator<t> Begin(linked_list<t>& List);
template <typename t> linked_list_iterator<t> End(linked_list<t>& List);
template <typename t> const linked_list_iterator<t> ConstBegin(const linked_list<t>& List);
template <typename t> const linked_list_iterator<t> ConstEnd(const linked_list<t>& List);

struct allocator;

template <typename t>
linked_list_iterator<t> Insert(linked_list<t>* List, linked_list_iterator<t> Where, const t& Payload);

template <typename t>
linked_list_iterator<t> PushBack(linked_list<t>* List, const t& Payload);

template <typename t>
void Deallocate(linked_list<t>* List);

template <typename t>
i64 Size(const linked_list<t>& List);


} // namespace mg

#include "mg_linked_list.inl"

