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

  {
    IdPool<int> id_pool;

    for (int i = 0; i <= 100; ++i)
      {
        assert(id_pool.next() == i);
      }
    for (auto &id : std::vector<int>{21, 57, 64, 79, 80, 96, 98})
      {
        id_pool.release(id);
      }
    std::stringstream ss;
    id_pool.print(ss, true);
    assert(ss.str() == "Used id ranges: 0-20, 22-56, 58-63, 65-78, 81-95, 97, 99-100\n");
    ss.str("");
    id_pool.print(ss);
    assert(ss.str() ==
           "Used id ranges:\n    0 -  20\n   22 -  56\n   58 -  63\n   65 -  78\n   81 -  95\n   97\n   99 - 100\n");
    ss.str("");
    id_pool.reset();
    id_pool.print(ss, true);
    assert(ss.str() == "Used id ranges:\n");
    ss.str("");
    id_pool.print(ss);
    assert(ss.str() == "Used id ranges:\n");
  }
}

DEFINE_TEST_MAIN
