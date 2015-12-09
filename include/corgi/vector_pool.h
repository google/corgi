// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VECTOR_POOL_H
#define VECTOR_POOL_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

namespace corgi {

/// @file
/// @addtogroup corgi_vector_pool
/// @{
///
/// @enum AllocationLocation
///
/// @brief `kAddToFront` allocates from the front of the pool. `kAddToBack`
/// allocates from the back of the pool.
enum AllocationLocation { kAddToFront, kAddToBack };

/// @class VectorPool
///
/// @brief A pool allocator, implemented as a vector-based pair of linked
/// lists.
///
/// @tparam T The data type of the data stored by the VectorPool.
template <typename T>
class VectorPool {
  template <bool>
  friend class IteratorTemplate;
  friend class VectorPoolReference;
  typedef uint32_t UniqueIdType;

 public:
  template <bool>
  class IteratorTemplate;

  /// @typedef Iterator
  ///
  /// @brief A non-const IteratorTemplate.
  typedef IteratorTemplate<false> Iterator;

  /// @typedef ConstIterator
  ///
  /// @brief A const IteratorTemplate.
  typedef IteratorTemplate<true> ConstIterator;

  /// @class VectorPoolReference
  ///
  /// @brief A reference object for pointing into the vector pool.
  /// It acts as a pointer for vector pool elements and can be queried to
  /// check if it has become invalid. (i.e. If the element it pointed
  /// at has either been deallocated, or replaced with a new element).
  ///
  /// It also correctly handles situations where the underlying vector resizes,
  /// moving the elements around in memory.
  class VectorPoolReference {
    friend class VectorPool<T>;
    template <bool>
    friend class IteratorTemplate;

   public:
    /// @brief Default constructor for a VectorPoolReference.
    VectorPoolReference() : container_(nullptr), index_(0), unique_id_(0) {}

    /// @brief Constructor for a VectorPoolReference given a VectorPool.
    ///
    /// @param[in] container A VectorPool to be referenced.
    /// @param[in] index The index into the VectorPool's underlying vector.
    VectorPoolReference(VectorPool<T>* container, size_t index)
        : container_(container), index_(index) {
      unique_id_ = container->GetElement(index)->unique_id;
    }

    /// @brief Standard equality operator for VectorPoolReferences.
    ///
    /// @param[in] other The other VectorPoolReference to compare equality to.
    ///
    /// @return Returns true if the two VectorPoolReferences point to the same
    /// VectorPool with the same index. Otherwise, it returns false.
    bool operator==(const VectorPoolReference& other) const {
      return container_ == other.container_ && index_ == other.index_;
    }

    /// @brief Standard inequality operator for VectorPoolReferences.
    ///
    /// @param[in] other The other VectorPoolReference to compare in-equality
    /// to.
    ///
    /// @return Returns false if the two VectorPoolReferences point to the same
    /// VectorPool with the same index. Otherwise, it returns true.
    bool operator!=(const VectorPoolReference& other) const {
      return !operator==(other);
    }

    /// @brief Verifies that the reference is still valid.
    ///
    /// @return Will return false if the object pointed to has been freed,
    /// even if the location was later filled with a new object.
    bool IsValid() const {
      return container_ != nullptr &&
             (container_->GetElement(index_)->unique_id == unique_id_);
    }

    /// @brief An alternate way to check to make sure the reference is still
    /// valid.
    ///
    /// This is similar to most smart pointer types, which supply an
    /// operator bool as syntactic sugar to check if they are nullptrs.
    ///
    /// @return Will return false if the object pointed to has been freed,
    /// even if the location was later filled with a new object.
    operator bool() const { return IsValid(); }

    /// @brief The member access operator.
    ///
    /// @warning Throws an assert if the VectorPoolReference is no longer valid.
    /// (i.e. If something has deleted the thing we were pointing to.)
    ///
    /// @return Returns a pointer to the data that the VectorPoolReference
    /// is referring to, allowing you to use VectorPoolReferences like
    /// pointers, syntactically.
    /// (e.g. myVectorPoolReference->MyDataMember = x;)
    T* operator->() {
      assert(IsValid());
      VectorPoolElement* element = container_->GetElement(index_);
      return &(element->data);
    }

    /// @brief Const member access operator.
    ///
    /// @warning Throws an assert if the VectorPoolReference is no longer valid.
    /// (i.e. If something has deleted the thing we were pointing to).
    ///
    /// @return Returns a pointer to the data that the VectorPoolReference
    /// is referring to, allowing you to use VectorPoolReferences like
    /// pointers, syntactically.
    /// (e.g. x = myVectorPoolReference->MyDataMember;)
    const T* operator->() const {
      return const_cast<VectorPoolReference*>(this)->operator->();
    }

    /// @brief The dereference operator.
    ///
    /// @warning Throws an assert if the VectorPoolReference is no longer valid.
    /// (i.e. If something has deleted the thing we are pointing to).
    ///
    /// @return Returns a reference variable for the data the
    /// VectorPoolReference points to, allowing you to use VectorPoolReference
    /// like a pointer. (e.g. MyDataVariable = (*MyVectorPoolReference);)
    T& operator*() {
      assert(IsValid());
      VectorPoolElement* element = container_->GetElement(index_);
      return element->data;
    }

    /// @brief The const dereference operator.
    ///
    /// @note Throws an assert if the VectorPoolReference is no longer valid.
    /// (i.e. If something has deleted the thing we are pointing to).
    ///
    /// @return Returns a const reference variable for the data the
    /// VectorPoolReference points to, allowing you to use VectorPoolReference
    /// like a pointer. (e.g. MyDataVariable = (*MyVectorPoolReference);)
    const T& operator*() const {
      return const_cast<VectorPoolReference*>(this)->operator*();
    }

    /// @brief Get a direct pointer to the element the VectorPoolReference
    /// is referring to.
    ///
    /// @note This pointer is not guaranteed to remain valid, since the vector
    /// may need to relocate the data in memory. It is recommended that when
    /// working with data, it be left as a VectorPoolReference, and only
    /// converted to a pointer when needed.
    ///
    /// @return Returns a pointer to the element that the VectorPoolReference
    /// is referring to. If the VectorPoolReference is not referring to any
    /// data, it returns a nullptr.
    T* ToPointer() {
      return IsValid() ? &(container_->GetElement(index_)->data) : nullptr;
    }

    /// @brief Get a direct pointer to the element the
    /// VectorPoolReference is referring to.
    ///
    /// @note This pointer is not guaranteed to remain valid, since the vector
    /// may need to relocate the data in memory. It is recommended that when
    /// working with data, it be left as a VectorPoolReference, and only
    /// converted to a pointer when needed.
    ///
    /// @return Returns a const pointer to the element that the
    /// VectorPoolReference is referring to. If the VectorPoolReference
    /// is not referring to any data, it returns a nullptr.
    const T* ToPointer() const {
      return const_cast<VectorPoolReference*>(this)->ToPointer();
    }

    /// @brief Get an iterator that points to the element referenced by
    /// the VectorPoolReference.
    ///
    /// @return Returns an Iterator that points to the element referenced
    /// by the VectorPoolReference.
    Iterator ToIterator() const { return Iterator(container_, index_); }

    /// @brief Get the raw index into the underlying vector for this object.
    ///
    /// @return Returns a size_t corresponding to the index into the underlying
    /// vector.
    size_t index() const { return index_; }

    /// @brief Gets a pointer to the underlying vector for this object.
    ///
    /// @return Returns a VectorPool pointer to the underlying vector.
    VectorPool<T>* container() const { return container_; }

   private:
    VectorPool<T>* container_;
    size_t index_;
    UniqueIdType unique_id_;
  };

  // ---------------------------
  /// @class IteratorTemplate
  ///
  /// @brief An Iterator for the VectorPool.
  ///
  /// This has constant-time access, so it is a good choice for iterating
  /// over the active elements that the pool owns.
  ///
  /// @tparam is_const A bool that determines if the IteratorTemplate should
  /// be defined as const.
  template <bool is_const>
  class IteratorTemplate {
    /// @typedef reference
    ///
    /// @brief A reference that may be const, depending on the templated
    /// boolean provided to the IteratorTemplate<bool>.
    typedef typename std::conditional<is_const, const T&, T&>::type reference;

    /// @typedef pointer
    ///
    /// @brief A pointer that may be const, depending on the templated
    /// boolean provided to the IteratorTemplate<bool>.
    typedef typename std::conditional<is_const, const T*, T*>::type pointer;

    friend class VectorPool<T>;

   public:
    /// @brief Constructor for an IteratorTemplate to a given VectorPool
    /// index.
    ///
    /// @param[in] container The VectorPool to point to.
    /// @param[in] index The index into the VectorPool's underlying vector.
    IteratorTemplate(VectorPool<T>* container, size_t index)
        : container_(container), index_(index) {}

    /// @brief Destructor for an IteratorTemplate.
    ~IteratorTemplate() {}

    /// @brief The standard equality operator to compare two
    /// IteratorTemplates.
    ///
    /// @param[in] other The other IteratorTemplate to compare with
    /// to check for equality.
    ///
    /// @return Returns true if the IteratorTemplate references the
    /// same index into the same VectorPool. Otherwise, it returns
    /// false.
    bool operator==(const IteratorTemplate& other) const {
      return container_ == other.container_ && index_ == other.index_;
    }

    /// @brief The standard inequality operator to compare two
    /// IteratorTemplates.
    ///
    /// @param[in] other The other IteratorTemplate to compare with
    /// to check for inequality.
    ///
    /// @return Returns false if the IteratorTemplate references the
    /// same index into the same VectorPool. Otherwise, it returns
    /// true.
    bool operator!=(const IteratorTemplate& other) const {
      return !operator==(other);
    }

    /// @brief The prefix increment operator to move the iterator
    /// forward in the list.
    ///
    /// @return Returns a reference to the incremented iterator.
    IteratorTemplate& operator++() {
      index_ = container_->elements_[index_].next;
      return (*this);
    }

    /// @brief The postfix increment operator to move the iterator
    /// forward in the list.
    ///
    /// @return Returns a reference to the original, unincremented iterator.
    IteratorTemplate operator++(int) {
      IteratorTemplate temp = *this;
      ++(*this);
      return temp;
    }

    /// @brief The prefix decrement operator to move the iterator
    /// backward in the list.
    ///
    /// @return Returns a reference to the decremented iterator.
    IteratorTemplate& operator--() {
      index_ = container_->elements_[index_].prev;
      return (*this);
    }

    /// @brief The postfix decrement operator to move the iterator
    /// backward in the list.
    ///
    /// @return Returns a reference to the original, undecremented iterator.
    IteratorTemplate operator--(int) {
      IteratorTemplate temp = *this;
      --(*this);
      return temp;
    }

    /// @brief The dereference operator.
    ///
    /// @return Returns a reference to the VectorPool data referenced by the
    /// Iterator.
    reference operator*() { return *(container_->GetElementData(index_)); }

    /// @brief Member access on the iterator.
    ///
    /// @return Returns a pointer to the VectorPool data referenced by the
    /// Iterator.
    pointer operator->() { return container_->GetElementData(index_); }

    /// @brief Converts the Iterator into a VectorPoolReference, which is
    /// the preferred way for holding onto references into the VectorPool.
    ///
    /// @return Returns a VectorPoolReference pointing to the VectorPool
    /// at the index that the Iterator referred to.
    VectorPoolReference ToReference() const {
      return VectorPoolReference(container_, index_);
    }

    /// @brief Get the index into the VectorPool vector.
    ///
    /// @return Returns a size_t that represents the underlying index into the
    /// VectorPool vector that the Iterator refers to.
    size_t index() const { return index_; }

   private:
    VectorPool<T>* container_;
    size_t index_;
  };

  // ---------------------------

  /// @var kOutOfBounds
  ///
  /// @brief A sentinel value that represents an out-of-bounds index.
  static const size_t kOutOfBounds = static_cast<size_t>(-1);

  /// @var kInvalidId
  ///
  /// @brief A sentinel value that represents an invalid ID.
  ///
  /// @note Unique IDs start at 1.
  static const UniqueIdType kInvalidId = 0;

  /// @struct VectorPoolElement
  ///
  /// @brief A struct representing an element inside of a VectorPool.
  struct VectorPoolElement {
    /// @brief The default constructor for an empty VectorPoolElement.
    VectorPoolElement()
        : next(kOutOfBounds), prev(kOutOfBounds), unique_id(kInvalidId) {}

    /// @brief The standard operator to move a referenced
    /// VectorPoolElement into this VectorPoolElement.
    ///
    /// @param[in] src A referenced VectorPoolElement to move
    /// into this VectorPoolElement.
    VectorPoolElement& operator=(VectorPoolElement&& src) {
      next = std::move(src.next);
      prev = std::move(src.prev);
      unique_id = std::move(src.unique_id);
      data = std::move(src.data);
      return *this;
    }

    /// @brief A copy constructor to create a VectorPoolElement from an
    /// existing VectorPoolElement.
    ///
    /// @param[in] src An existing VectorPoolElement to copy into
    /// this VectorPoolElement.
    VectorPoolElement(VectorPoolElement&& src) {
      next = std::move(src.next);
      prev = std::move(src.prev);
      unique_id = std::move(src.unique_id);
      data = std::move(src.data);
    }

    /// @var data
    ///
    /// @brief Holds the data within a VectorPoolElement.
    T data;

    /// @var next
    ///
    /// @brief The index of the next element in the vector.
    size_t next;

    /// @var prev
    ///
    /// @brief The index of the previous element in the vector.
    size_t prev;

    /// @var unique_id
    ///
    /// @brief The unique ID of this VectorPoolElement.
    UniqueIdType unique_id;

   private:
    VectorPoolElement(const VectorPoolElement&);
    VectorPoolElement& operator=(const VectorPoolElement&);
  };

  /// @var kFirstUsed
  ///
  /// @brief Used to demarcate the first element of our used list.
  ///
  /// @note This is never given actual data. It is only used for list
  /// demarcation.
  static const size_t kFirstUsed = 0;

  /// @var kLastUsed
  ///
  /// @brief Used to demarcate the last element of our used list.
  ///
  /// @note This is never given actual data. It is only used for list
  /// demarcation.
  static const size_t kLastUsed = 1;

  /// @var kFirstFree
  ///
  /// @brief Used to demarcate the first element of our free list.
  ///
  /// @note This is never given actual data. It is only used for list
  /// demarcation.
  static const size_t kFirstFree = 2;

  /// @var kLastFree
  ///
  /// @brief Used to demarcate the last element of our free list.
  ///
  /// @note This is never given actual data. It is only used for list
  /// demarcation.
  static const size_t kLastFree = 3;

  /// @var kTotalReserved
  ///
  /// @brief Used to indicate the number of reserved elements.
  /// (e.g. kFirstUsed, kLastUsed, kFirstFree, kLastFree).
  static const size_t kTotalReserved = 4;

  /// @brief The default constructor for an empty VectorPool.
  VectorPool() : active_count_(0), next_unique_id_(kInvalidId + 1) { Clear(); }

  /// @brief Get the data at the given element index.
  ///
  /// @note The pointer is not guaranteed to remain valid, if the vector
  /// needs to relocate the data in memory. In general, if you need to hold
  /// on to a reference to a data element, it is recommended that you use
  /// a VectorPoolReference.
  ///
  /// @warning Asserts if the index is illegal (i.e. out of range for the
  /// underlying vector).
  ///
  /// @param[in] index The index of the data element to return.
  ///
  /// @return Returns a pointer to the data.
  T* GetElementData(size_t index) {
    assert(index < elements_.size());
    return (&elements_[index].data);
  }

  /// @brief Get the data at the given element index.
  ///
  /// @note The pointer is not guaranteed to remain valid, if the vector
  /// needs to relocate the data in memory. In general, if you need to hold
  /// on to a reference to a data element, it is recommended that you use
  /// a VectorPoolReference.
  ///
  /// @warning Asserts if the index is illegal (i.e. out of range for the
  /// underlying vector).
  ///
  /// @param[in] index The index of the data element to return.
  ///
  /// @return Returns a const pointer to the data.
  const T* GetElementData(size_t index) const {
    assert(index < elements_.size());
    return (&elements_[index].data);
  }

  /// @brief Get a VectorPoolReference to a new element.
  ///
  /// @note This function grabs the first free element (if one exists).
  /// Otherwise, it allocates a new one on the underlying vector.
  ///
  /// @param[in] alloc_location An AllocationLocation enum determining whether
  /// to add to the front or back of the underlying vector.
  ///
  /// @return Returns a VectorPoolReference pointing to the new element.
  VectorPoolReference GetNewElement(AllocationLocation alloc_location) {
    size_t index;
    if (elements_[kFirstFree].next != kLastFree) {
      index = elements_[kFirstFree].next;
      RemoveFromList(index);  // remove it from the list of free elements.
    } else {
      index = elements_.size();
      elements_.push_back(VectorPoolElement());
    }
    switch (alloc_location) {
      case kAddToFront:
        AddToListFront(index, kFirstUsed);
        break;
      case kAddToBack:
        AddToListBack(index, kLastUsed);
        break;
      default:
        assert(0);
    }
    active_count_++;
    // Placement new, to make sure we always give back a cleanly constructed
    // element:
    elements_[index].data.~T();
    new (&(elements_[index].data)) T;
    elements_[index].unique_id = AllocateUniqueId();
    return VectorPoolReference(this, index);
  }

  /// @brief Frees an element at a given index.
  ///
  /// @note This removes the element from the list of active elements, and
  /// adds it to the front of the inactive list (to be used later, when we
  /// add elements to the VectorPool).
  ///
  /// @param[in] index The index corresponding to the element that should be
  /// freed.
  void FreeElement(size_t index) {
    assert(elements_[index].unique_id != kInvalidId);
    // Don't call the destructor directly - instead, assign over it.
    // This ensures that data will always contain an object that is safe to
    // destruct.
    elements_[index].data.~T();
    new (&(elements_[index].data)) T;
    elements_[index].unique_id = kInvalidId;
    RemoveFromList(index);
    AddToListFront(index, kFirstFree);
    active_count_--;
  }

  /// @brief Frees a given element.
  ///
  /// @note This removes the element from the list of active elements, and
  /// adds it to the front of the inactive list (to be used later, when we
  /// add elements to the VectorPool).
  ///
  /// @param[in] element A VectorPoolReference to the element that should be
  /// freed.
  void FreeElement(VectorPoolReference element) {
    if (element.IsValid()) {
      FreeElement(element.index_);
    }
  }

  /// @ Frees an element that an Iterator points to.
  ///
  /// @note This removes the element from the list of active elements, and
  /// adds it to the front of the inactive list (to be used later, when we
  /// add elements to the VectorPool).
  ///
  /// @param[in] iter An Iterator that references an element that should be
  /// freed.
  ///
  /// @return Returns an incremented Iterator that refers to the element
  /// immediately after the freed element.
  Iterator FreeElement(Iterator iter) {
    Iterator temp = iter++;
    FreeElement(temp.index_);
    return iter;
  }

  /// @brief Get the total size of the vector pool.
  ///
  /// This is the total number of allocated elements (used AND free) by the
  /// underlying vector.
  ///
  /// @return A size_t representing the total size of the VectorPool (both
  /// used AND free elements).
  size_t Size() const { return elements_.size(); }

  /// @brief Gets the total number of active elements.
  ///
  /// @return A size_t representing the total number of active elements in the
  /// VectorPool.
  size_t active_count() const { return active_count_; }

  /// @brief Clears out all of the elements in the VectorPool and resizes
  /// the underlying vector to the minimum size.
  void Clear() {
    elements_.resize(kTotalReserved);
    elements_[kFirstUsed].next = kLastUsed;
    elements_[kLastUsed].prev = kFirstUsed;
    elements_[kFirstFree].next = kLastFree;
    elements_[kLastFree].prev = kFirstFree;
    active_count_ = 0;
  }

  /// @brief Get an Iterator to the first active element in the VectorPool.
  ///
  /// This is suitable for traversing all of the active elements in the
  /// VectorPool.
  ///
  /// @return Returns an Iterator to the first active element in the VectorPool.
  Iterator begin() { return Iterator(this, elements_[kFirstUsed].next); }

  /// @brief Gets an Iterator to the last active element in the VectorPool.
  ///
  /// This is suitable as an end condition when iterating over all active
  /// elements in the VectorPool.
  ///
  /// @return Returns An Iterator to the last active element in the VectorPool.
  Iterator end() { return Iterator(this, kLastUsed); }

  /// @brief Gets a ConstIterator to the first active element in the
  /// VectorPool.
  ///
  /// This is suitable for traversing all of the active elements in the
  /// VectorPool.
  ///
  /// @return Returns a ConstIterator to the first active element in the
  /// VectorPool.
  ConstIterator cbegin() {
    return ConstIterator(this, elements_[kFirstUsed].next);
  }

  /// @brief Gets a ConstIterator to the last active element in the VectorPool.
  ///
  /// This is suitable as an end condition when iterating over all active
  /// elements in the VectorPool.
  ///
  /// @return Returns a ConstIterator to the last active element in the
  /// VectorPool.
  ConstIterator cend() { return ConstIterator(this, kLastUsed); }

  /// @brief Expands the vector until it is at least as large as new_size.
  ///
  /// @note: If the vector already contains at least new_size elements,
  /// then there is no effect.
  ///
  /// @param[in] new_size A size_t indicating the new minimum size for the
  /// underlying vector.
  void Reserve(size_t new_size) {
    size_t current_size = elements_.size();
    if (current_size >= new_size) return;

    elements_.resize(new_size);
    for (; current_size < new_size; current_size++) {
      elements_[current_size].unique_id = kInvalidId;
      AddToListFront(current_size, kFirstFree);
    }
  }

 private:
  /// @brief A utility function for removing an element from whatever list
  /// it is a part of.
  ///
  /// @note This should always be followed by AddToList, to reassign it, so we
  /// don't lose track of the element.
  ///
  /// @param[in] index A size_t representing the index of the element to be
  /// removed from whatever list it is a part of.
  void RemoveFromList(size_t index) {
    assert(index < elements_.size() && index >= kTotalReserved);
    VectorPoolElement& element = elements_[index];
    elements_[element.prev].next = element.next;
    elements_[element.next].prev = element.prev;
  }

  /// @brief A utility function to add an element to the front of a list.
  ///
  /// @note It adds it to whichever list is specified.
  ///
  /// @param[in] index A size_t representing the index of the element
  /// to be added to the specified list.
  /// @param[in] start_index The start of the list that the element
  /// should be added to (usually either kFirstUsed or kFirstFree).
  void AddToListFront(size_t index, size_t start_index) {
    assert(index < elements_.size() && index >= kTotalReserved);
    VectorPoolElement& list_start = elements_[start_index];
    elements_[list_start.next].prev = index;
    elements_[index].prev = start_index;
    elements_[index].next = list_start.next;
    list_start.next = index;
  }

  /// @brief A utility function to add an element to the back of a list.
  ///
  /// @note It adds it to whichever list is specified.
  ///
  /// @param[in] index A size_t representing the index of the element
  /// to be added to the specified list.
  /// @param[in] end_index The end of the list that the element
  /// should be added to (usually either kLastUsed or kLastFree).
  void AddToListBack(size_t index, size_t end_index) {
    assert(index < elements_.size() && index >= kTotalReserved);
    VectorPoolElement& list_end = elements_[end_index];
    elements_[list_end.prev].next = index;
    elements_[index].next = end_index;
    elements_[index].prev = list_end.prev;
    list_end.prev = index;
  }

  /// @brief Get an element at a given index.
  ///
  /// @note This is different from GetElementData, in that this function
  /// returns a pointer to the full VectorPoolElement (as opposed to just
  /// the user data).
  ///
  /// @param[in] index A size_t representing the index of the VectorPoolElement
  /// to be returned.
  ///
  /// @return Returns a VectorPoolElement pointer to the element at the given
  /// index.
  VectorPoolElement* GetElement(size_t index) {
    return index < elements_.size() ? &elements_[index] : nullptr;
  }

  /// @brief Get an element at a given index.
  ///
  /// @note This is different from GetElementData, in that this function
  /// returns a pointer to the full VectorPoolElement (as opposed to just
  /// the user data).
  ///
  /// @param[in] index A size_t representing the index of the VectorPoolElement
  /// to be returned.
  ///
  /// @return Returns a VectorPoolElement const pointer to the element at the
  /// given index.
  const VectorPoolElement* GetElement(size_t index) const {
    return index < elements_.size() ? &elements_[index] : nullptr;
  }

  /// @brief Allocates a unique ID.
  ///
  /// This is usually used when a new element is allocated.
  ///
  /// @warning Since the unique ID is done by simply allocating a counter, it
  /// is theoretically possible that this function could wrap around
  /// (especially if something has kept a constant reference for the duration
  /// of the program). This would cause a lot of things to malfunction, so
  /// ideally, do not use this class in situations where there are likely to
  /// be over 4,294,967,295 allocations. (Or change the UniqueTypeID into an
  /// uint_64 or something larger.)
  ///
  /// @return Returns a UniqueIdType representing the unique ID that
  /// was generated.
  UniqueIdType AllocateUniqueId() {
    // untility function to make sure it rolls over correctly.
    UniqueIdType result = next_unique_id_;
    next_unique_id_++;
    if (next_unique_id_ == kInvalidId) next_unique_id_++;
    return result;
  }

  std::vector<VectorPoolElement> elements_;
  size_t active_count_;
  UniqueIdType next_unique_id_;
};
/// @}

}  // corgi

#endif  // VECTOR_POOL_H
