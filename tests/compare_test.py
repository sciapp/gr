from __future__ import print_function

import os
import shutil
import platform

from nose import with_setup

from gr_test import CompareResult
from gr_test import c_image as image_data
from gr_test import c_video as video_data
base_path = os.path.abspath(os.path.dirname(os.path.realpath(__name__)) + '/../../test_result/')

if 'GR_TEST_BASE_PATH' in os.environ:
    base_path = os.path.abspath(os.environ['GR_TEST_BASE_PATH'])

results_path = os.path.abspath(base_path + '/' + platform.python_version())

def setup_func():
    try:
        os.mkdir(base_path)
    except OSError:
        pass

    try:
        os.mkdir(results_path)
    except OSError:
        pass

@with_setup(setup_func)
def test_images():
    image_data.create_files('TEST')
    consistency, pairs = image_data.get_test_data()
    for x in consistency:
        yield succeed_if_none, x
    for dir, ext, ref_name, test_name in pairs:
        yield compare, dir, ext, ref_name, test_name

@with_setup(setup_func)
def test_video():
    video_data.create_files('TEST')
    consistency, pairs = video_data.get_test_data()
    for x in consistency:
        yield succeed_if_none, x
    for dir, ext, ref_name, test_name in pairs:
        yield compare, dir, ext, ref_name, test_name

def succeed_if_none(x):
    assert x is None

def compare(dir, ext, ref_name, test_name):
    result = CompareResult(ref_name, test_name)

    file_name = os.path.basename(ref_name)
    if not result.is_equal():
        out_name = '%s/%s_%s_diff.png' % (results_path, dir, file_name)
        result.make_diff_png(out_name)
        shutil.copy(test_name, '%s/%s.%s' % (results_path, dir, file_name))
        print("Stats: %s diff png: %s" % (result.diff_stats(), out_name))

    assert result.is_equal()
