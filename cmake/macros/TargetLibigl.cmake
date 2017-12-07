#
#  Copyright 2017 High Fidelity, Inc.
#  Created by Sam Gondelman on 12/4/2017
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_LIBIGL)
  add_dependency_external_projects(libigl)
  find_package(LIBIGL REQUIRED)
  target_include_directories(${TARGET_NAME} PUBLIC ${LIBIGL_INCLUDE_DIRS})
endmacro()