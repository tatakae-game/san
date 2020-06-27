cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(VERSION 3.2)

project(san
    VERSION "1.0.1"
    DESCRIPTION "SAN compiler"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS=On)

    set(LLVM_USE_CRT_RELEASE MT)
    set(LLVM_USE_CRT_DEBUG MTd)
elseif((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang"))
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)

    if(SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
        list(APPEND CMAKE_CXX_FLAGS "-fvisibility-inlines-hidden")
    endif()
endif()

set(TARGET_WebAssembly WebAssemblyCodeGen WebAssemblyAsmParser WebAssemblyDesc WebAssemblyInfo)
set(TARGET_XCore XCoreCodeGen XCoreDesc XCoreInfo)
set(TARGET_SystemZ SystemZCodeGen SystemZAsmParser SystemZDesc SystemZInfo) 
set(TARGET_Sparc SparcCodeGen SparcAsmParser SparcDesc SparcInfo) 
set(TARGET_RISCV RISCVCodeGen RISCVAsmParser RISCVDesc RISCVUtils RISCVInfo) 
set(TARGET_PowerPC PowerPCCodeGen PowerPCAsmParser PowerPCDesc PowerPCInfo) 
set(TARGET_NVPTX NVPTXCodeGen NVPTXDesc NVPTXInfo) 
set(TARGET_MSP430 MSP430CodeGen MSP430AsmParser MSP430Desc MSP430Info) 
set(TARGET_Mips MipsCodeGen MipsAsmParser MipsDesc MipsInfo) 
set(TARGET_Lanai LanaiCodeGen LanaiAsmParser LanaiDesc LanaiInfo) 
set(TARGET_Hexagon HexagonCodeGen HexagonAsmParser HexagonDesc HexagonInfo) 
set(TARGET_BPF BPFCodeGen BPFAsmParser BPFDesc BPFInfo) 
set(TARGET_ARM ARMCodeGen ARMAsmParser ARMDesc ARMUtils ARMInfo) 
set(TARGET_AMDGPU AMDGPUCodeGen AMDGPUAsmParser AMDGPUDesc AMDGPUUtils AMDGPUInfo)
set(TARGET_X86 X86CodeGen X86AsmParser X86Desc X86Utils X86Info)
set(TARGET_AArch64 AArch64CodeGen AArch64AsmParser AArch64Desc AArch64Utils AArch64Info)

set(TARGETS_TO_BUILD "WebAssembly" "XCore" "SystemZ" "Sparc" "RISCV" "PowerPC" "NVPTX" "MSP430" "Mips" "Lanai" "Hexagon" "BPF" "ARM" "AMDGPU" "X86" "AArch64")

set(LLVM_TARGETS)
foreach (target IN ITEMS ${TARGETS_TO_BUILD})
    list(APPEND LLVM_TARGETS "${TARGET_${target}}")
endforeach(target)

list(JOIN TARGETS_TO_BUILD ";" LLVM_TARGETS_TO_BUILD)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/libs")

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${DEPENDENCIES_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

set(INCLUDES 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${DEPENDENCIES_HEADERS}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

llvm_map_components_to_libnames(llvm_libs 
    TextAPI
    OrcJIT
    JITLink
    ObjectYAML
    WindowsManifest
    Coverage
    TableGen
    LTO
    Passes
    ObjCARCOpts
    Coroutines
    LibDriver
    XRay
    ${LLVM_TARGETS}
    MIRParser
    ipo
    Instrumentation
    Vectorize
    Linker
    IRReader
    AsmParser
    Symbolize
    DebugInfoPDB
    FuzzMutate
    LineEditor
    MCA
    DebugInfoGSYM
    GlobalISel
    SelectionDAG
    AsmPrinter
    DebugInfoDWARF
    MCJIT
    Interpreter
    ExecutionEngine
    RuntimeDyld
    CodeGen
    Target
    ScalarOpts
    InstCombine
    AggressiveInstCombine
    TransformUtils
    BitWriter
    Analysis
    ProfileData
    DlltoolDriver
    Option
    Object
    MCParser
    MC
    DebugInfoCodeView
    DebugInfoMSF
    BitReader
    Core
    Remarks
    BinaryFormat
    BitstreamReader
    Support
    Demangle
)

set(NATIVE_LIBRARIES)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND NATIVE_LIBRARIES "stdc++fs")
endif()

message("CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

target_link_libraries(${PROJECT_NAME} 
    ${NATIVE_LIBRARIES}
    antlr4_static
    ${llvm_libs}
    lldCore
    lldCommon
    lldCOFF
    lldDriver
    lldELF
    lldMinGW
    lldWasm
)
