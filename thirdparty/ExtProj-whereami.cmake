#
# included by PrepareExternalProjects.cmake
#
file(APPEND ${YOURGAME_EXT_PROJ_OUT_FILE}
"list(APPEND YOURGAME_INC_DIRS
  ${YOURGAME_EXT_PROJ_MODULE_DIR}/whereami
)
list(APPEND YOURGAME_SRC_FILES
  ${YOURGAME_EXT_PROJ_MODULE_DIR}/whereami/whereami.c
)
") # file()
