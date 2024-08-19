set(ADPLUG_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
set(ADPLUG_GENERATED_VERSION_FILE "${ADPLUG_GENERATED_INCLUDE_DIR}/adplug/version.h")

configure_file(version.h.in "${ADPLUG_GENERATED_VERSION_FILE}")

set(SOURCES
    adlibemu.c
    debug.c
    depack.c
    fmopl.c
    nukedopl.c
    sixdepack.cpp
    unlzh.c
    unlzss.c
    unlzw.c
    a2m.cpp
    a2m-v2.cpp
    adl.cpp
    adplug.cpp
    adtrack.cpp
    amd.cpp
    analopl.cpp
    bam.cpp
    bmf.cpp
    cff.cpp
    cmf.cpp
    cmfmcsop.cpp
    coktel.cpp
    composer.cpp
    d00.cpp
    database.cpp
    dfm.cpp
    diskopl.cpp
    dmo.cpp
    dro2.cpp
    dro.cpp
    dtm.cpp
    emuopl.cpp
    flash.cpp
    fmc.cpp
    fprovide.cpp
    got.cpp
    herad.cpp
    hsc.cpp
    hsp.cpp
    hybrid.cpp
    hyp.cpp
    imf.cpp
    jbm.cpp
    kemuopl.cpp
    ksm.cpp
    lds.cpp
    mad.cpp
    mdi.cpp
    mid.cpp
    mkj.cpp
    msc.cpp
    mtk.cpp
    mtr.cpp
    mus.cpp
    nemuopl.cpp
    pis.cpp
    player.cpp
    players.cpp
    protrack.cpp
    psi.cpp
    rad2.cpp
    rat.cpp
    raw.cpp
    realopl.cpp
    rix.cpp
    rol.cpp
    s3m.cpp
    sa2.cpp
    sng.cpp
    sop.cpp
    surroundopl.cpp
    temuopl.cpp
    u6m.cpp
    vgm.cpp
    woodyopl.cpp
    xad.cpp
    xsm.cpp
)

set(HEADERS
    adplug.h
    emuopl.h
    fmopl.h
    silentopl.h
    opl.h
    diskopl.h
    a2m.h
    a2m-v2.h
    amd.h
    bam.h
    d00.h
    dfm.h
    hsc.h
    hsp.h
    imf.h
    ksm.h
    lds.h
    mid.h
    mkj.h
    mtr.h
    mtk.h
    pis.h
    protrack.h
    rad2.h
    raw.h
    s3m.h
    sa2.h
    sng.h
    u6m.h
    player.h
    fmc.h
    mad.h
    xad.h
    bmf.h
    flash.h
    hyp.h
    psi.h
    rat.h
    hybrid.h
    rol.h
    adtrack.h
    cff.h
    dtm.h
    dmo.h
    fprovide.h
    database.h
    players.h
    xsm.h
    adlibemu.h
    kemuopl.h
    dro.h
    realopl.h
    analopl.h
    temuopl.h
    msc.h
    rix.h
    adl.h
    jbm.h
    cmf.h
    surroundopl.h
    dro2.h
    got.h
    wemuopl.h
    woodyopl.h
    nemuopl.h
    nukedopl.h
    mus.h
    mdi.h
    cmfmcsop.h
    coktel.h
    composer.h
    vgm.h
    sop.h
    herad.h
    strnlen.h
    load_helper.h
)

add_library(adplug)
add_library(adplug::adplug ALIAS adplug)

target_sources(adplug
	PRIVATE
		${SOURCES}
		${HEADERS}
    "${ADPLUG_GENERATED_VERSION_FILE}"
)

target_link_libraries(adplug PUBLIC libbinio::libbinio)

if(HAVE_STRICMP)
  target_compile_definitions(adplug PRIVATE stricmp=_stricmp)
elseif(HAVE_STRCASECMP)
  target_compile_definitions(adplug PRIVATE stricmp=strcasecmp)
endif()

target_include_directories(
  adplug
  INTERFACE "$<BUILD_INTERFACE:${ADPLUG_GENERATED_INCLUDE_DIR}>"
  PRIVATE "$<BUILD_INTERFACE:${ADPLUG_GENERATED_INCLUDE_DIR}/adplug>"
)

if(ADPLUG_PRECOMPILED_HEADERS)
  target_precompile_headers(adplug PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>")
endif()

set(OUTPUT_HEADERS ${HEADERS})
list(TRANSFORM OUTPUT_HEADERS PREPEND "$<BUILD_INTERFACE:${ADPLUG_GENERATED_INCLUDE_DIR}/adplug/")
list(TRANSFORM OUTPUT_HEADERS APPEND ">")

add_custom_command(
  COMMENT "Copy adplug headers"
  OUTPUT ${OUTPUT_HEADERS}
  DEPENDS ${HEADERS}
  COMMAND "${CMAKE_COMMAND}" -E make_directory
          "${ADPLUG_GENERATED_INCLUDE_DIR}/adplug/"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${HEADERS}
          "${ADPLUG_GENERATED_INCLUDE_DIR}/adplug/"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(adplug_headers DEPENDS ${OUTPUT_HEADERS})

add_dependencies(adplug adplug_headers)
