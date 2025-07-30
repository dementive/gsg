custom_modules = "../gsg/src"
use_llvm = "yes"
redirect_build_objects = "no"

# Release stuff
optimize = "speed"
# lto="full" # Takes too long. Probably want in final release build but is awful for testing since it adds 25 minutes to compile time.
use_static_cpp="yes"

# stuff to make compiling faster
linker = "mold"
progress = "no"
engine_update_check = "no"
c_compiler_launcher = "ccache"
cpp_compiler_launcher = "ccache"
deprecated = "no"

# disabled modules
disable_xr = "yes"
# disable_physics_2d="yes"
# disable_physics_3d="yes"
# disable_navigation_2d="yes"
# disable_navigation_3d="yes"

module_godot_physics_2d_enabled = "no"
module_godot_physics_3d_enabled = "no"
module_jolt_physics_enabled = "no"
module_navigation_2d_enabled = "no"
module_navigation_3d_enabled = "no"
module_enet_enabled = "no"
module_gridmap_enabled = "no"
module_mbedtls_enabled = "no"
module_mobile_vr_enabled = "no"
module_multiplayer_enabled = "no"
module_ogg_enabled = "no"
module_openxr_enabled = "no"
module_theora_enabled = "no"
module_upnp_enabled = "no"
module_vorbis_enabled = "no"
module_webrtc_enabled = "no"
module_websocket_enabled = "no"
module_webxr_enabled = "no"
