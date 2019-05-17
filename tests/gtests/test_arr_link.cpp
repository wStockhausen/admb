#include <gtest/gtest.h>
#include "fvar.hpp"

extern "C"
{
  void test_ad_exit(const int exit_code);
}

class test_arr_link: public ::testing::Test {};

TEST_F(test_arr_link, deallocate)
{
  gradient_structure gs;

  dvar_vector v;
  v.allocate(1, 4);
  arr_link* ptr = v.link_ptr;
  dvar_vector v2;
  v2.allocate(1, 10);
  arr_link* ptr2 = v2.link_ptr;

  ASSERT_TRUE(ptr2->get_status() == 1);
  ASSERT_TRUE(ptr2->get_next() == 0);
  ASSERT_TRUE(ptr2->get_prev() == ptr);
  ASSERT_TRUE(ptr2->get_prev()->get_status() == 1);
  ASSERT_TRUE(gradient_structure::ARR_LIST1->get_number_arr_links() == 2);
  ASSERT_TRUE(ptr->get_next() == ptr2);
  v2.deallocate();
  ASSERT_TRUE(ptr->get_next() == 0);
  ASSERT_TRUE(gradient_structure::ARR_LIST1->get_number_arr_links() == 1);
  ASSERT_TRUE(ptr2->get_status() == 0);
  ASSERT_TRUE(gradient_structure::ARR_LIST1->get_last() == ptr);
  ASSERT_TRUE(gradient_structure::ARR_LIST1->get_last_offset() == ptr->get_size());
}
TEST_F(test_arr_link, allocate)
{
  gradient_structure gs;

  dvar_vector v;
  v.allocate(1, 4);

  ASSERT_TRUE(*(arr_link**)v.shape->trueptr == v.link_ptr);
  ASSERT_TRUE(v.link_ptr->get_prev() == 0);
  ASSERT_TRUE(v.link_ptr->get_next() == 0);
  ASSERT_TRUE(v.link_ptr->get_status() == 1);
  ASSERT_TRUE(v.link_ptr->get_offset() == 0);
  ASSERT_TRUE(v.link_ptr->get_size() == sizeof(double_and_int) * 4);

  double_and_int* p = v.va;
  p += v.indexmin();
  ASSERT_TRUE(*(arr_link**)p == v.link_ptr);

  double* ptr = gradient_structure::get_ARRAY_MEMBLOCK_BASE();
  ASSERT_TRUE(ptr == (double*)v.shape->trueptr);

  v.initialize();
  ASSERT_DOUBLE_EQ(value(v(1)), 0.0);
  ASSERT_DOUBLE_EQ(value(v(2)), 0.0);
  ASSERT_DOUBLE_EQ(value(v(3)), 0.0);
  ASSERT_DOUBLE_EQ(value(v(4)), 0.0);

  dvar_vector v2;
  v2.allocate(1, 10);
  ASSERT_TRUE(*(arr_link**)v2.shape->trueptr == v2.link_ptr);
  ASSERT_TRUE(v2.link_ptr->get_prev() == v.link_ptr);
  ASSERT_TRUE(v.link_ptr->get_next() == v2.link_ptr);
  ASSERT_TRUE(v2.link_ptr->get_next() == 0);
  ASSERT_TRUE(v2.link_ptr->get_status() == 1);
  ASSERT_TRUE(v2.link_ptr->get_offset() == v.link_ptr->get_size());
  ASSERT_TRUE(v2.link_ptr->get_size() == sizeof(double_and_int) * 10);

  void* ptr2 = (void*)ptr + v.link_ptr->get_size();
  ASSERT_TRUE(ptr2 == v2.shape->trueptr);
}
TEST_F(test_arr_link, tryit)
{
  char* real_start = new char[100];
  memset(real_start, 1, 100);
  for (int i = 0; i < 100; ++i)
  {
    ASSERT_EQ(real_start[i], 1);
  }
  *(char**)real_start = 0;
  for (int i = 10; i < 100; ++i)
  {
	  cout << i << endl;
    ASSERT_EQ(real_start[i], 1);
  }
  delete [] real_start;
  real_start = NULL;
}
TEST_F(test_arr_link, arr_remove_null)
{
  void arr_remove(arr_link** pptr);
  ASSERT_DEATH(arr_remove(NULL), "Assertion");
}
TEST_F(test_arr_link, arr_remove)
{
  ad_exit=&test_ad_exit;
  arr_link** pptr = new arr_link*[1];
  pptr[0] = NULL;
  try
  {
    void arr_remove(arr_link** pptr);
    arr_remove(pptr);
  }
  catch (const int exit_code)
  {
    delete [] pptr;
    pptr = NULL;

    const int expected_exit_code = 23;
    if (exit_code == expected_exit_code)
    {
      return;
    }
  }
  FAIL();
}
TEST_F(test_arr_link, dvar_vector1to4)
{
  gradient_structure gs;

  dvar_vector v(1, 4);

  arr_link* last = gradient_structure::ARR_LIST1->get_last();

  ASSERT_TRUE(last->get_prev() == NULL);
  ASSERT_TRUE(last->get_next() == NULL);
  ASSERT_EQ(last->get_size(), sizeof(double_and_int) * 4);
  ASSERT_EQ(last->get_offset(), 0);
  ASSERT_EQ(last->get_status(), 1);
}
/*
TEST_F(test_arr_link, initial_state_xpool)
{
  ASSERT_TRUE(arr_link::get_xpool() == NULL);

  gradient_structure gs;

  vector_shape_pool* xpool = arr_link::get_xpool();
  ASSERT_TRUE(xpool != NULL);
  ASSERT_EQ(xpool->nvar, 0);
  ASSERT_TRUE(xpool->last_chunk == NULL);
  ASSERT_EQ(xpool->num_allocated, 0);
  ASSERT_EQ(xpool->num_chunks, 0);
  ASSERT_EQ(xpool->nelem, 0);
  ASSERT_EQ(xpool->size, sizeof(arr_link));
  ASSERT_TRUE(xpool->head == NULL);
  ASSERT_TRUE(xpool->first == NULL);

  dvar_vector v(1, 4);

  ASSERT_EQ(xpool->nvar, 0);
  ASSERT_EQ(xpool->num_allocated, 1);
  ASSERT_EQ(xpool->num_chunks, 1);
  ASSERT_EQ(xpool->size, sizeof(arr_link));
  const size_t overhead = 12 + sizeof(char*);
  const size_t chunk_size = 65000 - overhead;
  size_t expected = chunk_size / xpool->size;
  ASSERT_EQ(xpool->nelem, expected);
  ASSERT_TRUE(static_cast<void*>(xpool->last_chunk + sizeof(char*)) == static_cast<void*>(xpool->first));
  void* ptr = static_cast<void*>(xpool->last_chunk + sizeof(char*));
  dfpool::link* k = static_cast<dfpool::link*>(ptr);
  cout << k << ' ' << xpool->head << endl;
  cout << k->next << ' ' << xpool->head << endl;
  ASSERT_TRUE(k->next == xpool->head);
}
*/
