cmake_minimum_required (VERSION 3.14)

project(triangle_proj VERSION 1.0)


file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).
Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

set (
WARNS
-Wall
-Wextra
-Wpedantic
-Werror
-Waggressive-loop-optimizations
-Wc++0x-compat
-Wc++11-compat
-Wc++14-compat
-Wcast-align
-Wcast-qual
-Wchar-subscripts
-Wconditionally-supported
-Wctor-dtor-privacy
-Wempty-body
-Wformat-nonliteral
-Wformat-security
-Wformat-signedness
-Wformat=2
-Winline
-Wlogical-op
-Wmissing-declarations
-Wnon-virtual-dtor
-Wopenmp-simd
-Woverloaded-virtual
-Wpacked
-Wpointer-arith
-Wredundant-decls
-Wshadow
-Wsign-conversion
-Wsign-promo
-Wstack-usage=16384
-Wstrict-null-sentinel
-Wstrict-overflow=2
-Wsuggest-attribute=noreturn
-Wsuggest-final-methods
-Wsuggest-final-types
-Wsuggest-override
-Wswitch-default
#-Wswitch-enum
-Wsync-nand
-Wundef
-Wunreachable-code
-Wuseless-cast
-Wvariadic-macros
-Wno-literal-suffix
-Wno-missing-field-initializers
-Wno-narrowing
-Wno-old-style-cast
-Wno-varargs
-fcheck-new
-fstack-protector
-fstrict-overflow
-flto-odr-type-merging
-fno-omit-frame-pointer
)

set(
FLAGS 
-fsanitize=address
-fsanitize=alignment
-fsanitize=bool
-fsanitize=bounds
-fsanitize=enum
-fsanitize=float-cast-overflow
-fsanitize=float-divide-by-zero
-fsanitize=integer-divide-by-zero
-fsanitize=leak
-fsanitize=nonnull-attribute
-fsanitize=null
-fsanitize=object-size
-fsanitize=return
-fsanitize=returns-nonnull-attribute
-fsanitize=shift
-fsanitize=signed-integer-overflow
-fsanitize=undefined
-fsanitize=unreachable
-fsanitize=vla-bound
-fsanitize=vptr
-fsized-deallocation
)