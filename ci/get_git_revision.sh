#!/bin/sh

VERSION=`git describe --tags --dirty 2> /dev/null`
if [ -z "$VERSION" ]; then
    # echo "Git could not find a suitable TAG, will use 0.0.0"
    VERSION="0.0.0"
fi

echo $VERSION
