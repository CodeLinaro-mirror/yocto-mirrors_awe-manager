#!/bin/sh

#
# this script is mainly useful in DSPC AWE-Manager build environment,
# it helps defining the build version in Jenkins scripts.
# it implements the same logic as in CMake code in GenerateVersionHeader.cmake

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

WORKSPACE_DIR=$SCRIPT_DIR/..

# First, check if VERSION file exists and source it
if [ -f "$WORKSPACE_DIR/VERSION" ]; then
    . "$WORKSPACE_DIR/VERSION"
fi

# If AWEMGR_VERSION not set, try GIT
if [ -z "$AWEMGR_VERSION" ]; then
    AWEMGR_VERSION=`git describe --tags --dirty 2> /dev/null`
fi

# If still not set, use default
if [ -z "$AWEMGR_VERSION" ]; then
    AWEMGR_VERSION="0.0.0"
fi

# incorporate custom version if set (e.g. in Jenkins build environment)
if [ ! -z "$AWEMGR_CUSTOM_VERSION" ]; then
    AWEMGR_VERSION="${AWEMGR_VERSION}-${AWEMGR_CUSTOM_VERSION}"
fi

echo $AWEMGR_VERSION
