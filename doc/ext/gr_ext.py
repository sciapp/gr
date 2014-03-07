
import examples

def init():
    print('Generating examples.')
    examples.main()


def clean(app, *args):
    examples.clean()


def setup(app):
    init()
    app.connect('build-finished', clean)
