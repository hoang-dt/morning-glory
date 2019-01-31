#pragma once

#include "mg_memory.h"

namespace mg {

template <typename t>
struct linked_list {
  t Payload;
  linked_list* Next = nullptr;
  linked_list();
  linked_list(const t& Payload, linked_list* Next);
};

struct allocator;

template <typename t>
linked_list<t>* AddNode(linked_list<t>* Where, const t& Payload, allocator* Alloc = &Mallocator());

template <typename t>
void Deallocate(linked_list<t>* List, allocator* Alloc = &Mallocator());

template <typename t>
i64 Size(const linked_list<t>& List);

} // namespace mg

#include "mg_linked_list.inl"

