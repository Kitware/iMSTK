#!/bin/sh

set -e

# Install build requirements.
dnf install -y --setopt=install_weak_deps=False \
    mesa-dri-drivers mesa-libGL-devel libXcursor-devel

# Install development tools
dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ \
    git-core \
    git-lfs

dnf clean all
