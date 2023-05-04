file(REMOVE_RECURSE
  "libIMGUILIB.a"
  "libIMGUILIB.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/IMGUILIB.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
