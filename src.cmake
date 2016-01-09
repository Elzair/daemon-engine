set(SHAREDLIST
    ${MOUNT_DIR}/shared/CommandBufferClient.cpp
    ${MOUNT_DIR}/shared/CommandBufferClient.h
    ${MOUNT_DIR}/shared/CommonProxies.cpp
    ${MOUNT_DIR}/shared/CommonProxies.h
    ${MOUNT_DIR}/shared/VMMain.cpp
    ${MOUNT_DIR}/shared/VMMain.h
    PARENT_SCOPE
)

set(COMMONLIST
    ${COMMON_DIR}/Color.h
    ${COMMON_DIR}/Color.cpp
    ${COMMON_DIR}/Command.cpp
    ${COMMON_DIR}/Command.h
    ${COMMON_DIR}/Common.h
    ${COMMON_DIR}/Compiler.h
    ${COMMON_DIR}/Cvar.cpp
    ${COMMON_DIR}/Cvar.h
    ${COMMON_DIR}/DisjointSets.h
    ${COMMON_DIR}/Endian.h
    ${COMMON_DIR}/FileSystem.cpp
    ${COMMON_DIR}/FileSystem.h
    ${COMMON_DIR}/IPC/Channel.h
    ${COMMON_DIR}/IPC/CommandBuffer.cpp
    ${COMMON_DIR}/IPC/CommandBuffer.h
    ${COMMON_DIR}/IPC/Common.h
    ${COMMON_DIR}/IPC/CommonSyscalls.h
    ${COMMON_DIR}/IPC/Primitives.cpp
    ${COMMON_DIR}/IPC/Primitives.h
    ${COMMON_DIR}/LineEditData.cpp
    ${COMMON_DIR}/LineEditData.h
    ${COMMON_DIR}/Log.cpp
    ${COMMON_DIR}/Log.h
    ${COMMON_DIR}/Math.h
    ${COMMON_DIR}/Optional.h
    ${COMMON_DIR}/Platform.h
    ${COMMON_DIR}/Serialize.h
    ${COMMON_DIR}/String.cpp
    ${COMMON_DIR}/String.h
    ${COMMON_DIR}/System.cpp
    ${COMMON_DIR}/System.h
    ${COMMON_DIR}/Util.h
    ${COMMON_DIR}/cm/cm_load.cpp
    ${COMMON_DIR}/cm/cm_local.h
    ${COMMON_DIR}/cm/cm_patch.cpp
    ${COMMON_DIR}/cm/cm_patch.h
    ${COMMON_DIR}/cm/cm_plane.cpp
    ${COMMON_DIR}/cm/cm_polylib.cpp
    ${COMMON_DIR}/cm/cm_polylib.h
    ${COMMON_DIR}/cm/cm_public.h
    ${COMMON_DIR}/cm/cm_test.cpp
    ${COMMON_DIR}/cm/cm_trace.cpp
    ${COMMON_DIR}/cm/cm_trisoup.cpp
    ${COMMON_DIR}/math/Vector.h
    ${ENGINE_DIR}/qcommon/logging.h
    ${ENGINE_DIR}/qcommon/q_math.cpp
    ${ENGINE_DIR}/qcommon/q_shared.cpp
    ${ENGINE_DIR}/qcommon/q_shared.h
    ${ENGINE_DIR}/qcommon/q_unicode.cpp
    ${ENGINE_DIR}/qcommon/q_unicode.h
    ${ENGINE_DIR}/qcommon/unicode_data.h
)
set(COMMONLIST ${COMMONLIST} PARENT_SCOPE)

set(RENDERERLIST
    ${ENGINE_DIR}/renderer/gl_shader.cpp
    ${ENGINE_DIR}/renderer/gl_shader.h
    ${ENGINE_DIR}/renderer/iqm.h
    ${ENGINE_DIR}/renderer/shaders.cpp
    ${ENGINE_DIR}/renderer/tr_animation.cpp
    ${ENGINE_DIR}/renderer/tr_backend.cpp
    ${ENGINE_DIR}/renderer/tr_bsp.cpp
    ${ENGINE_DIR}/renderer/tr_cmds.cpp
    ${ENGINE_DIR}/renderer/tr_curve.cpp
    ${ENGINE_DIR}/renderer/tr_decals.cpp
    ${ENGINE_DIR}/renderer/tr_fbo.cpp
    ${ENGINE_DIR}/renderer/tr_flares.cpp
    ${ENGINE_DIR}/renderer/tr_font.cpp
    ${ENGINE_DIR}/renderer/tr_image.cpp
    ${ENGINE_DIR}/renderer/tr_image.h
    ${ENGINE_DIR}/renderer/tr_image_crn.cpp
    ${ENGINE_DIR}/renderer/tr_image_dds.cpp
    ${ENGINE_DIR}/renderer/tr_image_exr.cpp
    ${ENGINE_DIR}/renderer/tr_image_jpg.cpp
    ${ENGINE_DIR}/renderer/tr_image_ktx.cpp
    ${ENGINE_DIR}/renderer/tr_image_png.cpp
    ${ENGINE_DIR}/renderer/tr_image_tga.cpp
    ${ENGINE_DIR}/renderer/tr_image_webp.cpp
    ${ENGINE_DIR}/renderer/tr_init.cpp
    ${ENGINE_DIR}/renderer/tr_light.cpp
    ${ENGINE_DIR}/renderer/tr_local.h
    ${ENGINE_DIR}/renderer/tr_main.cpp
    ${ENGINE_DIR}/renderer/tr_marks.cpp
    ${ENGINE_DIR}/renderer/tr_mesh.cpp
    ${ENGINE_DIR}/renderer/tr_model.cpp
    ${ENGINE_DIR}/renderer/tr_model_iqm.cpp
    ${ENGINE_DIR}/renderer/tr_model_md3.cpp
    ${ENGINE_DIR}/renderer/tr_model_md5.cpp
    ${ENGINE_DIR}/renderer/tr_model_skel.cpp
    ${ENGINE_DIR}/renderer/tr_model_skel.h
    ${ENGINE_DIR}/renderer/tr_noise.cpp
    ${ENGINE_DIR}/renderer/tr_public.h
    ${ENGINE_DIR}/renderer/tr_scene.cpp
    ${ENGINE_DIR}/renderer/tr_shade.cpp
    ${ENGINE_DIR}/renderer/tr_shader.cpp
    ${ENGINE_DIR}/renderer/tr_shade_calc.cpp
    ${ENGINE_DIR}/renderer/tr_skin.cpp
    ${ENGINE_DIR}/renderer/tr_sky.cpp
    ${ENGINE_DIR}/renderer/tr_surface.cpp
    ${ENGINE_DIR}/renderer/tr_types.h
    ${ENGINE_DIR}/renderer/tr_vbo.cpp
    ${ENGINE_DIR}/renderer/tr_world.cpp
    ${ENGINE_DIR}/sys/sdl_glimp.cpp
    ${ENGINE_DIR}/sys/sdl_icon.h
)

set(SERVERLIST
    ${ENGINE_DIR}/botlib/bot_api.h
    ${ENGINE_DIR}/botlib/bot_convert.cpp
    ${ENGINE_DIR}/botlib/bot_load.cpp
    ${ENGINE_DIR}/botlib/bot_local.cpp
    ${ENGINE_DIR}/botlib/bot_nav.cpp
    ${ENGINE_DIR}/botlib/bot_navdraw.h
    ${ENGINE_DIR}/botlib/bot_types.h
    ${ENGINE_DIR}/botlib/nav.h
    ${ENGINE_DIR}/server/server.h
    ${ENGINE_DIR}/server/sg_api.h
    ${ENGINE_DIR}/server/sg_msgdef.h
    ${ENGINE_DIR}/server/sv_bot.cpp
    ${ENGINE_DIR}/server/sv_ccmds.cpp
    ${ENGINE_DIR}/server/sv_client.cpp
    ${ENGINE_DIR}/server/sv_init.cpp
    ${ENGINE_DIR}/server/sv_main.cpp
    ${ENGINE_DIR}/server/sv_net_chan.cpp
    ${ENGINE_DIR}/server/sv_sgame.cpp
    ${ENGINE_DIR}/server/sv_snapshot.cpp
)

set(ENGINELIST
    ${ENGINE_DIR}/framework/Application.cpp
    ${ENGINE_DIR}/framework/BaseCommands.cpp
    ${ENGINE_DIR}/framework/BaseCommands.h
    ${ENGINE_DIR}/framework/CommandBufferHost.cpp
    ${ENGINE_DIR}/framework/CommandBufferHost.h
    ${ENGINE_DIR}/framework/CommandSystem.cpp
    ${ENGINE_DIR}/framework/CommandSystem.h
    ${ENGINE_DIR}/framework/CommonVMServices.cpp
    ${ENGINE_DIR}/framework/CommonVMServices.h
    ${ENGINE_DIR}/framework/ConsoleField.cpp
    ${ENGINE_DIR}/framework/ConsoleField.h
    ${ENGINE_DIR}/framework/ConsoleHistory.cpp
    ${ENGINE_DIR}/framework/ConsoleHistory.h
    ${ENGINE_DIR}/framework/CrashDump.h
    ${ENGINE_DIR}/framework/CrashDump.cpp
    ${ENGINE_DIR}/framework/CvarSystem.cpp
    ${ENGINE_DIR}/framework/CvarSystem.h
    ${ENGINE_DIR}/framework/LogSystem.cpp
    ${ENGINE_DIR}/framework/LogSystem.h
    ${ENGINE_DIR}/framework/Resource.cpp
    ${ENGINE_DIR}/framework/Resource.h
    ${ENGINE_DIR}/framework/System.cpp
    ${ENGINE_DIR}/framework/System.h
    ${ENGINE_DIR}/framework/VirtualMachine.cpp
    ${ENGINE_DIR}/framework/VirtualMachine.h
    ${ENGINE_DIR}/sys/con_log.cpp
    ${ENGINE_DIR}/qcommon/md5.cpp
)

if (WIN32)
    set(ENGINELIST ${ENGINELIST}
        ${ENGINE_DIR}/sys/con_passive.cpp
    )
else()
    set(ENGINELIST ${ENGINELIST}
        ${ENGINE_DIR}/sys/con_tty.cpp
    )
endif()

set(QCOMMONLIST
    ${ENGINE_DIR}/qcommon/cmd.cpp
    ${ENGINE_DIR}/qcommon/common.cpp
    ${ENGINE_DIR}/qcommon/crypto.cpp
    ${ENGINE_DIR}/qcommon/crypto.h
    ${ENGINE_DIR}/qcommon/cvar.cpp
    ${ENGINE_DIR}/qcommon/cvar.h
    ${ENGINE_DIR}/qcommon/files.cpp
    ${ENGINE_DIR}/qcommon/huffman.cpp
    ${ENGINE_DIR}/qcommon/msg.cpp
    ${ENGINE_DIR}/qcommon/net_chan.cpp
    ${ENGINE_DIR}/qcommon/net_ip.cpp
    ${ENGINE_DIR}/qcommon/parse.cpp
    ${ENGINE_DIR}/qcommon/print_translated.h
    ${ENGINE_DIR}/qcommon/qcommon.h
    ${ENGINE_DIR}/qcommon/qfiles.h
    ${ENGINE_DIR}/qcommon/surfaceflags.h
    ${ENGINE_DIR}/qcommon/translation.cpp
    ${ENGINE_DIR}/sys/con_log.cpp
    ${ENGINE_DIR}/sys/con_common.h
    ${ENGINE_DIR}/sys/con_common.cpp
    ${ENGINE_DIR}/sys/sdl_compat.cpp
)

if (NOT APPLE)
    set(ENGINELIST ${ENGINELIST}
        ${ENGINE_DIR}/sys/con_curses.cpp
    )
endif()

set(CLIENTBASELIST
    ${ENGINE_DIR}/botlib/bot_debug.cpp
    ${ENGINE_DIR}/botlib/bot_nav_edit.cpp
    ${ENGINE_DIR}/client/cg_api.h
    ${ENGINE_DIR}/client/cg_msgdef.h
    ${ENGINE_DIR}/client/cin_ogm.cpp
    ${ENGINE_DIR}/client/client.h
    ${ENGINE_DIR}/client/cl_avi.cpp
    ${ENGINE_DIR}/client/cl_cgame.cpp
    ${ENGINE_DIR}/client/cl_cin.cpp
    ${ENGINE_DIR}/client/cl_console.cpp
    ${ENGINE_DIR}/client/cl_input.cpp
    ${ENGINE_DIR}/client/cl_irc.cpp
    ${ENGINE_DIR}/client/cl_keys.cpp
    ${ENGINE_DIR}/client/cl_main.cpp
    ${ENGINE_DIR}/client/cl_parse.cpp
    ${ENGINE_DIR}/client/cl_scrn.cpp
    ${ENGINE_DIR}/client/dl_main.cpp
    ${ENGINE_DIR}/client/keycodes.h
    ${ENGINE_DIR}/client/keys.h
    ${ENGINE_DIR}/client/ClientApplication.cpp
)

set(CLIENTLIST
    ${ENGINE_DIR}/audio/ALObjects.cpp
    ${ENGINE_DIR}/audio/ALObjects.h
    ${ENGINE_DIR}/audio/Audio.cpp
    ${ENGINE_DIR}/audio/Audio.h
    ${ENGINE_DIR}/audio/AudioData.h
    ${ENGINE_DIR}/audio/AudioPrivate.h
    ${ENGINE_DIR}/audio/Emitter.cpp
    ${ENGINE_DIR}/audio/Emitter.h
    ${ENGINE_DIR}/audio/OggCodec.cpp
    ${ENGINE_DIR}/audio/OpusCodec.cpp
    ${ENGINE_DIR}/audio/Sample.cpp
    ${ENGINE_DIR}/audio/Sample.h
    ${ENGINE_DIR}/audio/Sound.cpp
    ${ENGINE_DIR}/audio/Sound.h
    ${ENGINE_DIR}/audio/SoundCodec.cpp
    ${ENGINE_DIR}/audio/SoundCodec.h
    ${ENGINE_DIR}/audio/WavCodec.cpp
    ${ENGINE_DIR}/sys/sdl_input.cpp
    ${RENDERERLIST}
)

set(TTYCLIENTLIST
    ${ENGINE_DIR}/null/NullAudio.cpp
    ${ENGINE_DIR}/null/null_input.cpp
    ${ENGINE_DIR}/null/null_renderer.cpp
)

set(DEDSERVERLIST
    ${ENGINE_DIR}/null/null_client.cpp
    ${ENGINE_DIR}/null/null_input.cpp
    ${ENGINE_DIR}/server/ServerApplication.cpp
)

set(WIN_RC ${ENGINE_DIR}/sys/daemon.rc)
