
# Aliens vs Predator Linux - http://icculus.org/avp/
# CMake 2.8 project
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# default to Desktop build
IF(NOT AVP_BUILD_TYPE)
    SET(AVP_BUILD_TYPE DESKTOP CACHE STRING "Executable type; one of: DESKTOP WEB" FORCE)
ENDIF(NOT AVP_BUILD_TYPE)

IF(NOT AVP_BUILD_TYPE STREQUAL "DESKTOP" AND NOT AVP_BUILD_TYPE STREQUAL "WEB")
    MESSAGE(FATAL_ERROR "Invalid AVP_BUILD_TYPE setting ${AVP_BUILD_TYPE}; must be one of DESKTOP WEB")
ENDIF(NOT AVP_BUILD_TYPE STREQUAL "DESKTOP" AND NOT AVP_BUILD_TYPE STREQUAL "WEB")

SET(AVP_WEB "NO")

PROJECT(avp)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

SET(SDL_TYPE "SDL2")
SET(OPENGL_TYPE "OPENGLES2")

# required dependencies
IF(NOT AVP_WEB)

    IF(NOT SDL_TYPE STREQUAL "AUTO" AND NOT SDL_TYPE STREQUAL "SDL" AND NOT SDL_TYPE STREQUAL "SDL2")
        MESSAGE(FATAL_ERROR "Invalid SDL_TYPE setting ${SDL_TYPE}; must be one of AUTO SDL SDL2")
    ENDIF(NOT SDL_TYPE STREQUAL "AUTO" AND NOT SDL_TYPE STREQUAL "SDL" AND NOT SDL_TYPE STREQUAL "SDL2")

ENDIF(NOT AVP_WEB)

# required source files
LIST(APPEND source src/bink.c)
LIST(APPEND source src/stubs.c)
LIST(APPEND source src/version.c)
LIST(APPEND source src/mathline.c)
LIST(APPEND source src/net.c)
LIST(APPEND source src/frustum.c)
LIST(APPEND source src/kshape.c)
LIST(APPEND source src/map.c)
LIST(APPEND source src/maths.c)
LIST(APPEND source src/md5.c)
LIST(APPEND source src/mem3dc.c)
LIST(APPEND source src/mem3dcpp.cpp)
LIST(APPEND source src/module.c)
LIST(APPEND source src/morph.c)
LIST(APPEND source src/object.c)
LIST(APPEND source src/shpanim.c)
LIST(APPEND source src/sphere.c)
LIST(APPEND source src/tables.c)
LIST(APPEND source src/vdb.c)
LIST(APPEND source src/avp/ai_sight.c)
LIST(APPEND source src/avp/avpview.c)
LIST(APPEND source src/avp/bh_agun.c)
LIST(APPEND source src/avp/bh_ais.c)
LIST(APPEND source src/avp/bh_alien.c)
LIST(APPEND source src/avp/bh_binsw.c)
LIST(APPEND source src/avp/bh_cable.c)
LIST(APPEND source src/avp/bh_corpse.c)
LIST(APPEND source src/avp/bh_deathvol.c)
LIST(APPEND source src/avp/bh_debri.c)
LIST(APPEND source src/avp/bh_dummy.c)
LIST(APPEND source src/avp/bh_fan.c)
LIST(APPEND source src/avp/bh_far.c)
LIST(APPEND source src/avp/bh_fhug.c)
LIST(APPEND source src/avp/bh_gener.c)
LIST(APPEND source src/avp/bh_ldoor.c)
LIST(APPEND source src/avp/bh_lift.c)
LIST(APPEND source src/avp/bh_light.c)
LIST(APPEND source src/avp/bh_lnksw.c)
LIST(APPEND source src/avp/bh_ltfx.c)
LIST(APPEND source src/avp/bh_marin.c)
LIST(APPEND source src/avp/bh_mission.c)
LIST(APPEND source src/avp/bh_near.c)
LIST(APPEND source src/avp/bh_pargen.c)
LIST(APPEND source src/avp/bh_plachier.c)
LIST(APPEND source src/avp/bh_plift.c)
LIST(APPEND source src/avp/bh_pred.c)
LIST(APPEND source src/avp/bh_queen.c)
LIST(APPEND source src/avp/bh_rubberduck.c)
LIST(APPEND source src/avp/bh_selfdest.c)
LIST(APPEND source src/avp/bh_snds.c)
LIST(APPEND source src/avp/bh_spcl.c)
LIST(APPEND source src/avp/bh_swdor.c)
LIST(APPEND source src/avp/bh_track.c)
LIST(APPEND source src/avp/bh_types.c)
LIST(APPEND source src/avp/bh_videoscreen.c)
LIST(APPEND source src/avp/bh_waypt.c)
LIST(APPEND source src/avp/bh_weap.c)
LIST(APPEND source src/avp/bh_xeno.c)
LIST(APPEND source src/avp/bonusabilities.c)
LIST(APPEND source src/avp/cconvars.cpp)
LIST(APPEND source src/avp/cdtrackselection.cpp)
LIST(APPEND source src/avp/cheatmodes.c)
LIST(APPEND source src/avp/comp_map.c)
LIST(APPEND source src/avp/comp_shp.c)
LIST(APPEND source src/avp/consolelog.cpp)
LIST(APPEND source src/avp/davehook.cpp)
LIST(APPEND source src/avp/deaths.c)
LIST(APPEND source src/avp/decal.c)
LIST(APPEND source src/avp/detaillevels.c)
LIST(APPEND source src/avp/dynamics.c)
LIST(APPEND source src/avp/dynblock.c)
LIST(APPEND source src/avp/equipmnt.c)
LIST(APPEND source src/avp/extents.c)
LIST(APPEND source src/avp/game.c)
LIST(APPEND source src/avp/game_statistics.c)
LIST(APPEND source src/avp/gamecmds.cpp)
LIST(APPEND source src/avp/gamevars.cpp)
LIST(APPEND source src/avp/hmodel.c)
LIST(APPEND source src/avp/hud.c)
LIST(APPEND source src/avp/inventry.c)
LIST(APPEND source src/avp/language.c)
LIST(APPEND source src/avp/lighting.c)
LIST(APPEND source src/avp/load_shp.c)
LIST(APPEND source src/avp/los.c)
LIST(APPEND source src/avp/mempool.c)
LIST(APPEND source src/avp/messagehistory.c)
LIST(APPEND source src/avp/missions.cpp)
LIST(APPEND source src/avp/movement.c)
LIST(APPEND source src/avp/paintball.c)
LIST(APPEND source src/avp/particle.c)
LIST(APPEND source src/avp/pfarlocs.c)
LIST(APPEND source src/avp/pheromon.c)
LIST(APPEND source src/avp/player.c)
LIST(APPEND source src/avp/pmove.c)
LIST(APPEND source src/avp/psnd.c)
LIST(APPEND source src/avp/psndproj.c)
LIST(APPEND source src/avp/pvisible.c)
LIST(APPEND source src/avp/savegame.c)
LIST(APPEND source src/avp/scream.cpp)
LIST(APPEND source src/avp/secstats.c)
LIST(APPEND source src/avp/sfx.c)
LIST(APPEND source src/avp/stratdef.c)
LIST(APPEND source src/avp/targeting.c)
LIST(APPEND source src/avp/track.c)
LIST(APPEND source src/avp/triggers.c)
LIST(APPEND source src/avp/weapons.c)
LIST(APPEND source src/avp/shapes/cube.c)
LIST(APPEND source src/avp/support/consbind.cpp)
LIST(APPEND source src/avp/support/consbtch.cpp)
LIST(APPEND source src/avp/support/coordstr.cpp)
LIST(APPEND source src/avp/support/daemon.cpp)
LIST(APPEND source src/avp/support/indexfnt.cpp)
LIST(APPEND source src/avp/support/r2base.cpp)
LIST(APPEND source src/avp/support/r2pos666.cpp)
LIST(APPEND source src/avp/support/reflist.cpp)
LIST(APPEND source src/avp/support/refobj.cpp)
LIST(APPEND source src/avp/support/rentrntq.cpp)
LIST(APPEND source src/avp/support/scstring.cpp)
LIST(APPEND source src/avp/support/strtab.cpp)
LIST(APPEND source src/avp/support/strutil.c)
LIST(APPEND source src/avp/support/trig666.cpp)
LIST(APPEND source src/avp/support/wrapstr.cpp)
LIST(APPEND source src/avp/win95/avpchunk.cpp)
LIST(APPEND source src/avp/win95/cheat.c)
LIST(APPEND source src/avp/win95/chtcodes.cpp)
LIST(APPEND source src/avp/win95/d3d_hud.cpp)
LIST(APPEND source src/avp/win95/ddplat.cpp)
LIST(APPEND source src/avp/win95/endianio.c)
LIST(APPEND source src/avp/win95/ffread.cpp)
LIST(APPEND source src/avp/win95/ffstdio.cpp)
LIST(APPEND source src/avp/win95/gammacontrol.cpp)
LIST(APPEND source src/avp/win95/hierplace.cpp)
LIST(APPEND source src/avp/win95/iofocus.cpp)
LIST(APPEND source src/avp/win95/jsndsup.cpp)
LIST(APPEND source src/avp/win95/kzsort.c)
LIST(APPEND source src/avp/win95/langplat.c)
LIST(APPEND source src/avp/win95/modcmds.cpp)
LIST(APPEND source src/avp/win95/npcsetup.cpp)
LIST(APPEND source src/avp/win95/objsetup.cpp)
LIST(APPEND source src/avp/win95/pathchnk.cpp)
LIST(APPEND source src/avp/win95/platsup.c)
LIST(APPEND source src/avp/win95/pldghost.c)
LIST(APPEND source src/avp/win95/pldnet.c)
LIST(APPEND source src/avp/win95/progress_bar.cpp)
LIST(APPEND source src/avp/win95/projload.cpp)
LIST(APPEND source src/avp/win95/scrshot.cpp)
LIST(APPEND source src/avp/win95/strachnk.cpp)
LIST(APPEND source src/avp/win95/system.c)
LIST(APPEND source src/avp/win95/usr_io.c)
LIST(APPEND source src/avp/win95/vision.c)
LIST(APPEND source src/avp/win95/frontend/avp_envinfo.c)
LIST(APPEND source src/avp/win95/frontend/avp_intro.cpp)
LIST(APPEND source src/avp/win95/frontend/avp_menudata.c)
LIST(APPEND source src/avp/win95/frontend/avp_menus.c)
LIST(APPEND source src/avp/win95/frontend/avp_mp_config.cpp)
LIST(APPEND source src/avp/win95/frontend/avp_userprofile.cpp)
LIST(APPEND source src/avp/win95/gadgets/ahudgadg.cpp)
LIST(APPEND source src/avp/win95/gadgets/conscmnd.cpp)
LIST(APPEND source src/avp/win95/gadgets/conssym.cpp)
LIST(APPEND source src/avp/win95/gadgets/consvar.cpp)
LIST(APPEND source src/avp/win95/gadgets/gadget.cpp)
LIST(APPEND source src/avp/win95/gadgets/hudgadg.cpp)
LIST(APPEND source src/avp/win95/gadgets/rootgadg.cpp)
LIST(APPEND source src/avp/win95/gadgets/t_ingadg.cpp)
LIST(APPEND source src/avp/win95/gadgets/teletype.cpp)
LIST(APPEND source src/avp/win95/gadgets/textexp.cpp)
LIST(APPEND source src/avp/win95/gadgets/textin.cpp)
LIST(APPEND source src/avp/win95/gadgets/trepgadg.cpp)
LIST(APPEND source src/win95/animchnk.cpp)
LIST(APPEND source src/win95/animobs.cpp)
LIST(APPEND source src/win95/awtexld.cpp)
LIST(APPEND source src/win95/awbmpld.cpp)
LIST(APPEND source src/win95/awiffld.cpp)
LIST(APPEND source src/win95/awpnmld.cpp)
LIST(APPEND source src/win95/bmpnames.cpp)
LIST(APPEND source src/win95/chnkload.cpp)
LIST(APPEND source src/win95/chnktexi.cpp)
LIST(APPEND source src/win95/chnktype.cpp)
LIST(APPEND source src/win95/chunk.cpp)
LIST(APPEND source src/win95/chunkpal.cpp)
LIST(APPEND source src/win95/db.c)
LIST(APPEND source src/win95/debuglog.cpp)
LIST(APPEND source src/win95/dummyobjectchunk.cpp)
LIST(APPEND source src/win95/enumchnk.cpp)
LIST(APPEND source src/win95/enumsch.cpp)
LIST(APPEND source src/win95/envchunk.cpp)
LIST(APPEND source src/win95/fail.c)
LIST(APPEND source src/win95/fragchnk.cpp)
LIST(APPEND source src/win95/gsprchnk.cpp)
LIST(APPEND source src/win95/hierchnk.cpp)
LIST(APPEND source src/win95/huffman.cpp)
LIST(APPEND source src/win95/iff.cpp)
LIST(APPEND source src/win95/iff_ilbm.cpp)
LIST(APPEND source src/win95/ilbm_ext.cpp)
LIST(APPEND source src/win95/io.c)
LIST(APPEND source src/win95/list_tem.cpp)
LIST(APPEND source src/win95/ltchunk.cpp)
LIST(APPEND source src/win95/media.cpp)
LIST(APPEND source src/win95/mishchnk.cpp)
LIST(APPEND source src/win95/obchunk.cpp)
LIST(APPEND source src/win95/oechunk.cpp)
LIST(APPEND source src/win95/our_mem.c)
LIST(APPEND source src/win95/plat_shp.c)
LIST(APPEND source src/win95/plspecfn.c)
LIST(APPEND source src/win95/shpchunk.cpp)
LIST(APPEND source src/win95/sndchunk.cpp)
LIST(APPEND source src/win95/sprchunk.cpp)
LIST(APPEND source src/win95/string.cpp)
LIST(APPEND source src/win95/texio.c)
LIST(APPEND source src/win95/toolchnk.cpp)
LIST(APPEND source src/win95/txioctrl.cpp)
LIST(APPEND source src/win95/wpchunk.cpp)
LIST(APPEND source src/win95/zsp.cpp)
LIST(APPEND source src/opengl.c)
LIST(APPEND source src/oglfunc.c)
LIST(APPEND source src/fmv.c)
LIST(APPEND source src/openal.c)
LIST(APPEND source src/cdplayer.c)
LIST(APPEND source src/menus.c)

IF(WIN32)
  LIST(APPEND source src/winfiles.c)
ELSE(WIN32)
  LIST(APPEND source src/files.c)
  LIST(APPEND source src/winapi.c)
ENDIF(WIN32)

IF(NOT AVP_WEB)
    IF(SDL_TYPE STREQUAL "SDL")
        LIST(APPEND source src/main.c)

        # SDL 1.2 on OS X requires this support file
        IF(APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            LIST(APPEND source src/sdl12/sdlmain.m)
        ENDIF(APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ENDIF(SDL_TYPE STREQUAL "SDL")

    IF(SDL_TYPE STREQUAL "SDL2")
        LIST(APPEND source src/main.c)
    ENDIF(SDL_TYPE STREQUAL "SDL2")
ENDIF(NOT AVP_WEB)

### build all source as C++
### (not normally used)
##SET_SOURCE_FILES_PROPERTIES(${source} PROPERTIES LANGUAGE CXX)

# auto-include directories with source files
FOREACH(sourcefile IN LISTS source)
    GET_FILENAME_COMPONENT(includedir ${sourcefile} PATH) # newer CMake uses DIRECTORY
    LIST(APPEND include ${includedir})
ENDFOREACH(sourcefile)
INCLUDE_DIRECTORIES(${include})

include("${VITASDK}/share/vita.cmake" REQUIRED)
message("PSVita Release Build..")
set(VITA_APP_NAME "Aliens vs Predator Gold")
set(VITA_TITLEID  "AVPGOLD20")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVITA -g -O2 -DNDEBUG -mfpu=neon -fno-optimize-sibling-calls -ftree-vectorize -fsingle-precision-constant -march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -ffast-math -Wno-write-strings -fno-short-enums")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVITA -g -O2 -DNDEBUG -mfpu=neon -fno-optimize-sibling-calls -ftree-vectorize -fsingle-precision-constant -march=armv7-a -mtune=cortex-a9 -mfloat-abi=hard -ffast-math -Wno-write-strings -fno-short-enums")

# manually include src/include
INCLUDE_DIRECTORIES(src/include)

IF(NOT AVP_WEB)
    add_executable(avp ${source})

    INCLUDE_DIRECTORIES(${VITASDK}/arm-vita-eabi/include/SDL2)

    target_link_libraries(avp
		vitashark
		mp3lame
		openal
		avutil
		avformat
		swscale
		swresample
		avcodec
		SceDisplay_stub
		SceGxm_stub
		ScePgf_stub
		ScePvf_stub
		SceSysmodule_stub
		SceCtrl_stub
		SceTouch_stub
		SceAudio_stub
		ScePower_stub
		SceHid_stub
		SceAppMgr_stub
		SceCommonDialog_stub
		SceShaccCg_stub
		SDL2
		vitaGL
		mathneon
		c
		m
		z
		bz2
		vita2d
		-Wl,--whole-archive pthread -Wl,--no-whole-archive
	)

ENDIF(NOT AVP_WEB)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
	VERSION ${VITA_VERSION}
	NAME ${VITA_APP_NAME}
	FILE ${PROJECT_SOURCE_DIR}/sce_sys/icon0.png sce_sys/icon0.png
	FILE ${PROJECT_SOURCE_DIR}/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
	FILE ${PROJECT_SOURCE_DIR}/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
	FILE ${PROJECT_SOURCE_DIR}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
