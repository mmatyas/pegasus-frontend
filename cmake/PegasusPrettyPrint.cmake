set(BOX_W 60)

function(pegasus_print_boxline text)
    string(LENGTH "${text}" text_len)
    math(EXPR padding_len "${BOX_W} - 4 - ${text_len}")
    if(${padding_len} GREATER_EQUAL 0)
        string(REPEAT " " ${padding_len} padding)
        message("│ ${text}${padding} │")
    else()
        message("│ ${text}")
    endif()
endfunction()

function(pegasus_print_boxhead text)
    string(LENGTH "${text}" text_len)
    math(EXPR padding_len "${BOX_W} - 4 - ${text_len}")
    string(REPEAT "─" ${padding_len} padding)
    message("╭${padding} ${text} ╮")
endfunction()

function(pegasus_print_boxtail)
    math(EXPR edge_len "${BOX_W} - 2")
    string(REPEAT "─" ${edge_len} edge)
    message("╰${edge}╯")
endfunction()

function(pegasus_print_box)
    cmake_parse_arguments(BOX
        ""
        "TITLE"
        "CONTENT"
        ${ARGN})

    pegasus_print_boxhead("${BOX_TITLE}")
    foreach(LINE IN LISTS BOX_CONTENT)
        pegasus_print_boxline("${LINE}")
    endforeach()
    pegasus_print_boxtail()
endfunction()
