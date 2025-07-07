


set (freetype_src
   "include/freetype/config/ftconfig.h"
   "include/freetype/config/ftheader.h"
   "include/freetype/config/ftmodule.h"
   "include/freetype/config/ftoption.h"
   "include/freetype/config/ftstdlib.h"
   "include/ft2build.h"

   "src/autofit/autofit.c"
   "src/base/ftbase.c"
   "src/base/ftbbox.c"
   "src/base/ftbitmap.c"
   "src/base/ftdebug.c"
   "src/base/ftfstype.c"
   "src/base/ftgasp.c"
   "src/base/ftglyph.c"
   "src/base/ftgxval.c"
   "src/base/ftinit.c"
   "src/base/ftlcdfil.c"
   "src/base/ftmm.c"
   "src/base/ftotval.c"
   "src/base/ftpatent.c"
   "src/base/ftpfr.c"
   "src/base/ftstroke.c"
   "src/base/ftsynth.c"
   "src/base/ftsystem.c"
   "src/base/fttype1.c"
   "src/base/ftwinfnt.c"
   "src/bdf/bdf.c"
   "src/cache/ftcache.c"
   "src/cff/cff.c"
   "src/cid/type1cid.c"
   "src/gzip/ftgzip.c"
   "src/lzw/ftlzw.c"
   "src/pcf/pcf.c"
   "src/pfr/pfr.c"
   "src/psaux/psaux.c"
   "src/pshinter/pshinter.c"
   "src/psnames/psmodule.c"
   "src/raster/raster.c"
   "src/sfnt/sfnt.c"
   "src/smooth/smooth.c"
   "src/truetype/truetype.c"
   "src/type1/type1.c"
   "src/type42/type42.c"
   "src/winfonts/winfnt.c"
)
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${freetype_src})
add_definitions(-DFT2_BUILD_LIBRARY)
add_library(freetype STATIC ${freetype_src})
target_include_directories(freetype PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
set_target_properties(freetype PROPERTIES FOLDER ${lib_folder}/freetype)