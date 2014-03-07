import os
import sys
import shutil

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
DOC_DIR = os.path.abspath(os.path.join(THIS_DIR, '..'))
EXAMPLES_DIR = os.path.abspath(os.path.join(DOC_DIR, '..', 'examples'))
OUTPUT_DIR = os.path.join(DOC_DIR, 'examples')


def clean():
    if os.path.isdir(OUTPUT_DIR):
        shutil.rmtree(OUTPUT_DIR)
    fname = os.path.join(DOC_DIR, 'examples.rst')
    if os.path.isfile(fname):
        os.remove(fname)


def get_example_filenames(examples_dir):

    for (dirpath, dirnames, filenames) in os.walk(examples_dir):
        for fname in filenames:
            if not fname.endswith('.py'):
                continue
            filename = os.path.join(dirpath, fname)
            name = filename[len(examples_dir):].lstrip('/\\')[:-3]
            name = name.replace('\\', '/')
            f = open(filename, 'r')
            line = f.readline()
            f.close()
            if line.startswith('#!/usr/bin/env python'):
                yield filename, name


def create_examples(examples):

    # Create doc file for each example
    for filename, name in examples:
        print('Writing example %s' % name)

        # Create title
        lines = []
        lines.append(name)
        lines.append('-' * len(lines[-1]))
        lines.append('')

        # Get source
        doclines = []
        sourcelines = []
        with open(os.path.join(EXAMPLES_DIR, name + '.py')) as f:
            for line in f.readlines():
                line = line.rstrip()
                if not doclines:
                    if line.startswith('"""'):
                        doclines.append(line.lstrip('" '))
                        sourcelines = []
                    else:
                        sourcelines.append('    ' + line)
                elif not sourcelines:
                    if '"""' in line:
                        sourcelines.append('    ' + line.partition('"""')[0])
                    else:
                        doclines.append(line)
                else:
                    sourcelines.append('    ' + line)

        # Add desciprion
        lines.extend(doclines)
        lines.append('')

        # Add source code
        lines.append('.. code-block:: python')
        lines.append('    ')
        lines.extend(sourcelines)
        lines.append('')

        # Write
        output_filename = os.path.join(OUTPUT_DIR, name + '.rst')
        output_dir = os.path.dirname(output_filename)
        if not os.path.isdir(output_dir):
            os.mkdir(output_dir)
        with open(output_filename, 'w') as f:
            f.write('\n'.join(lines))


def create_examples_list(examples):

    # Create TOC
    lines = []
    lines.append('Examples')
    lines.append('=' * len(lines[-1]))
    lines.append('')

    # Add entry for each example that we know
    for _, name in examples:
        lines.append('.. include:: examples/%s.rst' % name)

    # Write file
    with open(os.path.join(DOC_DIR, 'examples.rst'), 'w') as f:
        f.write('\n'.join(lines))


def main():

    # Get examples and sort
    examples = list(get_example_filenames(EXAMPLES_DIR))
    examples.sort(key=lambda x: x[1])

    create_examples(examples)
    create_examples_list(examples)


if __name__ == '__main__':
    main()
