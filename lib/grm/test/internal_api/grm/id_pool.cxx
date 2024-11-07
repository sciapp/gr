#include <grm/utilcpp_int.hxx>
#include "test.h"


void test()
{
  {
    IdPool<int> id_pool;

    assert(id_pool.next() == 0);
    assert(id_pool.next() == 1);
    assert(id_pool.next() == 2);

    id_pool.release(1);
    assert(id_pool.next() == 1);

    id_pool.release(0);
    assert(id_pool.next() == 0);

    id_pool.release(1);
    id_pool.release(0);
    assert(id_pool.next() == 0);
    assert(id_pool.next() == 1);
    assert(id_pool.next() == 3);

    id_pool.release(3);
    assert(id_pool.next() == 3);
    assert(id_pool.current() == 3);

    id_pool.release(1);
    id_pool.release(0);
    id_pool.release(2);
    assert(id_pool.current() == 3);
    id_pool.release(3);
    try
      {
        id_pool.current();
        assert(false);
      }
    catch (const NoCurrentIdError &e)
      {
      }
    assert(id_pool.next() == 0);
    assert(id_pool.current() == 0);

    try
      {
        id_pool.release(1);
        assert(false);
      }
    catch (const IdNotFoundError<int> &e)
      {
        assert(e.id() == 1);
      }

    id_pool.reset();
    try
      {
        id_pool.current();
        assert(false);
      }
    catch (const NoCurrentIdError &e)
      {
      }
    assert(id_pool.next() == 0);
  }

  {
    IdPool<int> id_pool(3);

    assert(id_pool.next() == 3);
  }
}

DEFINE_TEST_MAIN
