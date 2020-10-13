execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    describe
    --always
    --dirty
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_REVISION
)

execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    log -1
    --format=%cd
    --date=short
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_DATE
)

execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    rev-list
    --count HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_COMMIT_CNT
)
