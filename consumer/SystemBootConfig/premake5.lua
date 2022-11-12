project "SystemBootConfig"
kind "StaticLib"
language "C"

-- targetdir ("%{wks.targetdir}/%{cfg.buildcfg}/%{prj.name}")
files {"src/**.h", "src/**.c"}

includedirs
{
    "include",
    "%{_WORKING_DIR}/LibYaml/include",
    "%{_WORKING_DIR}/env/lib",
    -- "%{_WORKING_DIR}/env/c_language_env",
}

filter "platforms:env_c"
includedirs {"%{_WORKING_DIR}/env/system/c_language_env",}
filter "platforms:env_edk2"
includedirs {"%{_WORKING_DIR}/env/system/edk2",}
filter {}