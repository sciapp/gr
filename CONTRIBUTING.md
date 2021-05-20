# Contributing to GR

*GR* is open source and contributions by different users will help to make it as useful as possible. Thank you for considering contributing to GR!

## Reporting Issues

- If you encounter any issues using *GR*, please [open an issue on GitHub](https://github.com/sciapp/gr/issues/new), either in this repo or in the repo for the language binding you are using ([Python](https://github.com/sciapp/python-gr/issues/new), [Julia](https://github.com/jheinen/GR.jl/issues/new), [Ruby](https://github.com/red-data-tools/GR.rb/issues/new)).
- Describe the situation to help us reproduce it. This might include providing a [minimal working example](https://stackoverflow.com/help/minimal-reproducible-example).
- Describe what you expected to happen.
- Describe what actually happened. Please include logs, screenshots or screen recordings to help us understand the issue.
- List the version of *GR* you are using:
  - *Python*: `print(gr.version())`
  - *Julia*: `GR.version()`
  - *C*: `puts(gr_version());`
- If possible, try to reproduce your issue using the current `develop` branch of both the *GR* runtime and the language binding, if any.

## Building the *GR* Runtime

- If you want to build the *GR* runtime yourself during development, see the [*GR* documentation](https://gr-framework.org/building.html).

## Submitting Changes

- This repository contains the *GR* runtime. If you wish to submit a change to a language binding, please see the corresponding repository ([Python](https://github.com/sciapp/python-gr/), [Julia](https://github.com/jheinen/GR.jl/), [Ruby](https://github.com/red-data-tools/GR.rb/))
- We aim to support a wide variety of use cases. Please keep this in mind and prefer generic solutions to specialized ones.
- Adhere to the code style. You can use `clang-format` and `cmake-format` to apply the code style to individual files, or set up a [pre-commit githook](https://github.com/sciapp/gr/blob/develop/.githooks/pre-commit).
- Once you are done, push your changes to your fork of *GR* and open up a [pull request](https://github.com/sciapp/gr/compare).
- If you introduce a new feature or change existing functionality, please document it in the [*GR* documentation](https://github.com/sciapp/gr-documentation).
