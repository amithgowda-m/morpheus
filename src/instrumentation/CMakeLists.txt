# Intel PIN tool CMakeLists.txt
if(EXISTS "$ENV{PIN_ROOT}")
    set(PIN_ROOT $ENV{PIN_ROOT})
    
    # Find PIN includes and libraries
    find_path(PIN_INCLUDE_DIR pin.H
        PATHS ${PIN_ROOT}/source/include
        NO_DEFAULT_PATH
    )
    
    find_library(PIN_LIBRARY pin
        PATHS ${PIN_ROOT}/intel64/lib
        NO_DEFAULT_PATH
    )
    
    find_library(PIN_LIBRARY_EXT pin3ed
        PATHS ${PIN_ROOT}/intel64/lib-ext
        NO_DEFAULT_PATH
    )
    
    if(PIN_INCLUDE_DIR AND PIN_LIBRARY AND PIN_LIBRARY_EXT)
        add_library(morpheus-instrumentation-tool MODULE
            morpheus_pin_tool.cpp
        )
        
        target_include_directories(morpheus-instrumentation-tool
            PRIVATE ${PIN_INCLUDE_DIR}
        )
        
        target_link_libraries(morpheus-instrumentation-tool
            ${PIN_LIBRARY}
            ${PIN_LIBRARY_EXT}
        )
        
        set_target_properties(morpheus-instrumentation-tool PROPERTIES
            COMPILE_FLAGS "-DBIGARRAY_MULTIPLIER=1 -DTARGET_IA32E -DHOST_IA32E -DTARGET_LINUX -fno-stack-protector"
            LINK_FLAGS "-Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=${PIN_ROOT}/source/include/pin/pintool.ver"
        )
    else()
        message(WARNING "PIN development files not found, skipping instrumentation tool")
    endif()
endif()