#!/bin/sh
# Script for testing in GHA environment - not used in build.

export CI_BUILD=Github
export BUILD_TYPE=Release
export RELEASE_TYPE=PR
export RELEASE_NUMBER=99
export VERSION_CODE=99
export GIT_PR_COMMIT=feedbeef
export GIT_PR_COMMIT_SHORT=feedbeef
export HIFI_VCPKG_BOOTSTRAP=true
export GA_TRACKING_ID=UA-39558647-8
export PreferredToolArchitecture=X64
export OCULUS_APP_ID=1255907384473836

cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TOOLS:BOOLEAN=FALSE
