project "App"
kind "ConsoleApp"
language "C"

-- targetdir ("%{wks.targetdir}/%{cfg.buildcfg}/%{prj.name}")
files {"**.h", "**.c"}

includedirs
{
    "%{_WORKING_DIR}/LibYaml/include",
    "%{_WORKING_DIR}/consumer/SystemBootConfig/include",
}

filter "platforms:env_c"
includedirs {"%{_WORKING_DIR}/env/system/c_language_env",}
filter "platforms:env_UEFI"
includedirs {"%{_WORKING_DIR}/env/system/edk2",}
filter {}