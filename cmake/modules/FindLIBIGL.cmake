#
#  FindLibigl.cmake
#
#  Try to find LIBIGL include path.
#  Once done this will define
#
#  LIBIGL_INCLUDE_DIRS
#
#  Created on 12/4/2017 by Sam Gondelman
#  Copyright 2017 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

# setup hints for LIBIGL search
include("${MACRO_DIR}/HifiLibrarySearchHints.cmake")
hifi_library_search_hints("libigl")

# locate header
find_path(LIBIGL_INCLUDE_DIRS "include/igl/igl_inline.h" HINTS ${LIBIGL_SEARCH_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBIGL DEFAULT_MSG LIBIGL_INCLUDE_DIRS)

mark_as_advanced(LIBIGL_INCLUDE_DIRS LIBIGL_SEARCH_DIRS)