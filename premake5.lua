workspace "LibYAML"
    configurations {"Debug", "Release"}

    platforms {"env_edk2", "env_c", }
    defaultplatform("env_c")

    location ("%{_WORKING_DIR}/output/build")
    objdir("%{_WORKING_DIR}/output/lib/%{cfg.buildcfg}_%{cfg.platform}/%{prj.name}")
    targetdir("%{_WORKING_DIR}/output/bin/%{cfg.buildcfg}_%{cfg.platform}/%{prj.name}")

    defines {"YAML_VERSION_STRING=\"0.0.0\""}
    defines {"YAML_VERSION_MAJOR=0"}
    defines {"YAML_VERSION_MINOR=0"}
    defines {"YAML_VERSION_PATCH=0"}

    filter "configurations:Debug"
        defines {"DEBUG"}
        symbols "On"
    filter "configurations:Release"
        defines {"NDEBUG"}
        optimize "On"
    filter {}

    include "BootApp/LibYAML"
    include "BootApp/consumer/SystemBootConfig"
    include "BootApp/App/c"

    links {"LibYAML", "App", "SystemBootConfig"}