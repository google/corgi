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

#include "gtest/gtest.h"
#include "corgi/vector_pool.h"

#define TEST_ALL_SIZES_F(MY_TEST) \
  TEST_F(VectorPoolTests, MY_TEST##_int8) { \
    MY_TEST##_Test<int8_t>(); \
  } \
  TEST_F(VectorPoolTests, MY_TEST##_uint8) { \
    MY_TEST##_Test<uint8_t>(); \
  } \
  TEST_F(VectorPoolTests, MY_TEST##_int16) { \
    MY_TEST##_Test<int16_t>(); \
  } \
  TEST_F(VectorPoolTests, MY_TEST##_uint16) { \
    MY_TEST##_Test<uint16_t>(); \
  } \
  TEST_F(VectorPoolTests, MY_TEST##_int32) { \
    MY_TEST##_Test<int32_t>(); \
  } \
  TEST_F(VectorPoolTests, MY_TEST##_uint32) { \
    MY_TEST##_Test<uint32_t>(); \
  }

class VectorPoolTests : public ::testing::Test {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

template <typename T>
struct TestStruct {
  TestStruct() { value = 123; }
  TestStruct(T val) : value(val) {}
  T value;
};

#define VECTORPOOL_REF typename \
    corgi::VectorPool<TestStruct<DataType>>::VectorPoolReference

// Test that allocated variables are initialized according to their constructor.
template<class DataType>
void AllocAndFree_Constructor_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  for (int i = 0; i < 100; i++) {
    VECTORPOOL_REF ref = pool.GetNewElement(corgi::kAddToFront);
    EXPECT_EQ(ref->value, static_cast<DataType>(123));
  }
}
TEST_ALL_SIZES_F(AllocAndFree_Constructor)

// Test allocating and freeing one element.
template<class DataType>
void AllocAndFree_OneElement_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  VECTORPOOL_REF ref;
  EXPECT_EQ(pool.active_count(), 0u);
  ref = pool.GetNewElement(corgi::kAddToFront);
  EXPECT_TRUE(ref.IsValid());
  EXPECT_EQ(pool.active_count(), 1u);
  pool.FreeElement(ref);
  EXPECT_EQ(pool.active_count(), 0u);
  EXPECT_FALSE(ref.IsValid());
}
TEST_ALL_SIZES_F(AllocAndFree_OneElement)

// Test allocating and freeing two elements.
template<class DataType>
void AllocAndFree_TwoElementsElement_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  VECTORPOOL_REF ref1;
  VECTORPOOL_REF ref2;
  EXPECT_EQ(pool.active_count(), 0u);
  ref1 = pool.GetNewElement(corgi::kAddToFront);
  ref2 = pool.GetNewElement(corgi::kAddToFront);
  EXPECT_TRUE(ref1.IsValid());
  EXPECT_TRUE(ref2.IsValid());
  EXPECT_EQ(pool.active_count(), 2u);

  pool.FreeElement(ref1);
  EXPECT_FALSE(ref1.IsValid());
  EXPECT_EQ(pool.active_count(), 1u);

  pool.FreeElement(ref2);
  EXPECT_FALSE(ref2.IsValid());
  EXPECT_EQ(pool.active_count(), 0u);
}
TEST_ALL_SIZES_F(AllocAndFree_TwoElementsElement)

// Test allocating and freeing many elements.
// Starts out allocating 100 elements, numbered 1 to 100.
// Then removes the odd numbered elements.  Then adds 50 more elements
// to the front and back of the list.  So the final list should look like:
// 0-49: [elements numbered 49-0]
// 50-99: [odd numbered elements numbered 1-99]
// 100-149: [elements numbered 50-99]
template<class DataType>
void AllocAndFree_ManyElementsElement_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  VECTORPOOL_REF ref;
  for (int i = 0; i < 100; i++) {
    ref = pool.GetNewElement(corgi::kAddToBack);
    EXPECT_TRUE(ref.IsValid());
    ref->value = i;
  }
  // Remove all the even numbers.
  for (auto iter = pool.begin(); iter != pool.end(); ++iter) {
    ref = iter.ToReference();
    ++iter;
    pool.FreeElement(ref);
    EXPECT_FALSE(ref.IsValid());
  }
  // The list now contains the even numbers between 0 and 98.
  // Add in more numbers before and after the list:
  for (int i = 0; i < 50; i++) {
    ref = pool.GetNewElement(corgi::kAddToFront);
    ref->value = i;
    ref = pool.GetNewElement(corgi::kAddToBack);
    ref->value = i + 50;
  }

  // Check our list contents:
  auto iter = pool.begin();

  // First 50 (numbered 0-49)
  for (int i = 49; i >= 0; i--) {
    EXPECT_EQ(iter->value, static_cast<DataType>(i));
    ++iter;
  }
  // Second 50, numbered 0-49
  for (int i = 1; i < 100; i += 2) {
    EXPECT_EQ(iter->value, static_cast<DataType>(i));
    ++iter;
  }
  // Final 50, numbered 50-99
  for (int i = 50; i < 100; i++) {
    EXPECT_EQ(iter->value, static_cast<DataType>(i));
    ++iter;
  }
  EXPECT_EQ(iter, pool.end());
}
TEST_ALL_SIZES_F(AllocAndFree_ManyElementsElement)

// Test adding a bunch of elements to the back, and making sure the
// order is what we expect.
template<class DataType>
void InsertionOrder_AddToBack_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  VECTORPOOL_REF ref;

  for (int i = 0; i < 100; i++) {
    ref = pool.GetNewElement(corgi::kAddToBack);
    EXPECT_TRUE(ref.IsValid());
    ref->value = i;
  }

  int i = 0;
  for (auto iter = pool.begin(); iter != pool.end(); ++iter) {
    EXPECT_EQ(i, static_cast<int>(iter->value));
    i++;
  }
}
TEST_ALL_SIZES_F(InsertionOrder_AddToBack)

// Test adding a bunch of elements to the front, and making sure the
// order is what we expect.
template<class DataType>
void InsertionOrder_AddToFront_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  VECTORPOOL_REF ref;

  for (int i = 0; i < 100; i++) {
    ref = pool.GetNewElement(corgi::kAddToFront);
    EXPECT_TRUE(ref.IsValid());
    ref->value = i;
  }

  int i = 99;
  for (auto iter = pool.begin(); iter != pool.end(); ++iter) {
    EXPECT_EQ(i, static_cast<int>(iter->value));
    i--;
  }
}
TEST_ALL_SIZES_F(InsertionOrder_AddToFront)

// Tests that begin and end point to each other in an empty pool,
// Don't point to each other in a non-empty-pool, and then do
// point to each other again if the non-empty pool has been emptied.
template<class DataType>
void Iterator_BeginEnd_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  EXPECT_EQ(pool.begin(), pool.end());
  VECTORPOOL_REF ref = pool.GetNewElement(corgi::kAddToFront);
  EXPECT_NE(pool.begin(), pool.end());
  pool.FreeElement(ref);
  EXPECT_EQ(pool.begin(), pool.end());
}
TEST_ALL_SIZES_F(Iterator_BeginEnd)

// Test that we can step through the pool via an iterator,
// and that it takes us the correct number of steps.
template<class DataType>
void Iterator_StepThrough_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  for (int i = 0; i < 100; i ++) {
    VECTORPOOL_REF ref = pool.GetNewElement(corgi::kAddToBack);
    ref->value = i;
  }

  int counter = 0;
  for (auto iter = pool.begin(); iter != pool.end(); ++iter) {
    EXPECT_EQ(static_cast<int>(iter->value), counter);
    counter++;
  }
  EXPECT_EQ(counter, 100);
}
TEST_ALL_SIZES_F(Iterator_StepThrough)

// Test that we can step backwards through the pool via an iterator,
// and that it takes us the correct number of steps.
template<class DataType>
void Iterator_StepBackwards_Test() {
  corgi::VectorPool<TestStruct<DataType>> pool;
  for (int i = 0; i < 100; i ++) {
    VECTORPOOL_REF ref = pool.GetNewElement(corgi::kAddToBack);
    ref->value = i;
  }

  int counter = 0;

  for (auto iter = --pool.end(); iter != pool.begin(); --iter) {
    counter++;
    EXPECT_EQ(static_cast<int>(iter->value), 100 - counter);
  }
  // Not 100 here, because we had to advance it once during initialization,
  // to get our iterator off of end().
  EXPECT_EQ(counter, 99);
}
TEST_ALL_SIZES_F(Iterator_StepBackwards)

#if !defined(__ANDROID__)
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif // !defined(__ANDROID__)
