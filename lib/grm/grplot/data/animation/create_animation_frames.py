#!/usr/bin/env -S uv run --script

__copyright__ = "Copyright © 2026 Forschungszentrum Jülich GmbH. All rights reserved."
__license__ = "MIT"

# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "numpy",
# ]
# ///

import math
from pathlib import Path

import numpy as np

PATH_TEMPLATE = "./animation_frame_{:04d}.out"
FRAME_COUNT = 60


def create_animation_frames(path_template: str = PATH_TEMPLATE, frame_count: int = FRAME_COUNT):
    t = np.linspace(0, 2 * math.pi, 100)
    for i in range(frame_count):
        print(f"Creating frame {i + 1}")
        t_0 = i * 2 * math.pi / frame_count
        y = np.sin(t + t_0)

        with Path(path_template.format(i + 1)).open("w", encoding="utf-8") as out_file:
            print("# title : Sinus", file=out_file)
            print("# x_label : t", file=out_file)
            print("# y_label : y", file=out_file)
            print("# x_columns : 1", file=out_file)
            print("# y_columns : 2", file=out_file)
            print(file=out_file)
            np.savetxt(out_file, np.vstack((t, y)).T, delimiter=" ", fmt="%f")


def main():
    create_animation_frames()


if __name__ == "__main__":
    main()
