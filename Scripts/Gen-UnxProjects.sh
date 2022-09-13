#!/usr/bin/env bash

pushd ../
exec ./Tools/premake/premake5 gmake2
popd
