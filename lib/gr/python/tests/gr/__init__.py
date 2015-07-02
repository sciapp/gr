# -*- coding: utf-8 -*-

# standard library

# third party
import nose
from nose.tools import assert_equal
# local library
import gr
from gr.pygr import Coords2DList, Coords2D

def test_char():
    gr.char("t")
    gr.char(u"t")


def test_coords2DList_minmax():
    a = Coords2D([10, 20, 30], [10, 20, 30])
    coords = Coords2DList([a])
    assert_equal(coords.xmin, 10)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 30)

    coords.append(Coords2D([5, 10], [20, 40]))
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 40)

    b = Coords2D([1, 2, 3], [1, 2, 3])
    coords += [b]
    assert_equal(coords.xmin, 1)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 1)
    assert_equal(coords.ymax, 40)

    tmp = coords.pop(coords.index(b))
    assert_equal(tmp, b)
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 40)

    coords.extend([b])
    assert_equal(coords.xmin, 1)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 1)
    assert_equal(coords.ymax, 40)

    coords.remove(b)
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 40)

    coords.append(Coords2D([10, 10], [0, 0]))
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 0)
    assert_equal(coords.ymax, 40)

    coords.append(Coords2D([10, 10], [200, 400]))
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 0)
    assert_equal(coords.ymax, 400)


def test_coords2DList_empty():
    a = Coords2D([10, 20, 30], [10, 20, 30])
    coords = Coords2DList([a])
    coords.remove(a)
    assert_equal(coords.xmin, None)
    assert_equal(coords.xmax, None)
    assert_equal(coords.ymin, None)
    assert_equal(coords.ymax, None)


def test_coords2DList_update():
    a = Coords2D([10, 20, 30], [10, 20, 30])
    b = Coords2D([5], [40])
    coords = Coords2DList([a, b])
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 40)

    b.x = [15]
    b.y = [25]
    coords.updateMinMax(b)
    assert_equal(coords.xmin, 5)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 40)

    coords.updateMinMax(*coords, reset=True)
    assert_equal(coords.xmin, 10)
    assert_equal(coords.xmax, 30)
    assert_equal(coords.ymin, 10)
    assert_equal(coords.ymax, 30)

    coords.updateMinMax(b, reset=True)
    assert_equal(coords.xmin, 15)
    assert_equal(coords.xmax, 15)
    assert_equal(coords.ymin, 25)
    assert_equal(coords.ymax, 25)