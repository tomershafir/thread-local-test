#!/usr/bin/env python3
import os
import subprocess

ROOT_PATH = os.path.realpath(os.path.dirname(__file__))

DEFAULT_BUILD_DIR = "build"
DEFAULT_BUILD_PATH = os.path.join(ROOT_PATH, DEFAULT_BUILD_DIR)

CMAKE_CMD = "cmake"

def configure():
    os.makedirs(DEFAULT_BUILD_DIR, exist_ok=True)
    subprocess.run(
        [CMAKE_CMD, "-S", ROOT_PATH, "-B", DEFAULT_BUILD_PATH],
        check=True
    )

def build():
    subprocess.run(
        [CMAKE_CMD, "--build", DEFAULT_BUILD_PATH],
        check=True
    )

def main():
    configure()
    build()

if __name__ == "__main__":
    main()
