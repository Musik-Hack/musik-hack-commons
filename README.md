# Musik Hack Commons

JUCE modules and CMake structure for the common good!

## Prerequisites

- cmake 3.2+
- MAC: [XCode](https://apps.apple.com/us/app/xcode/id497799835)
- WIN: [Visual Studio](https://visualstudio.microsoft.com)

This repository contains submodules. After cloning your copy, run the following command to pull them into your local repo:

    git submodule update --init

In order to use this repository on Windows, enable [Developer Mode](https://learn.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development). Then, inside this repository, run the following command:

    git config --local core.symlinks true
    git restore .

Alternatively, you can clone the repository with symlinks enabled from the get-go:

    git clone -c core.symlinks=true <URL_FROM_GITHUB>

If the project complains about missing files on Windows, it is definitely an issue with the symlinks. Find the file that's "missing", manually delete and git restore the symlink.
