[Defines]
    SUPPORTED_ARCHITECTURES = IA32|X64
    BUILD_TARGETS           = DEBUG|RELEASE
    PLATFORM_NAME           = BootLoaderPkg
    PLATFORM_GUID           = 0458dade-8b6e-4e45-b773-1b27cbda3e06
    PLATFORM_VERSION        = 0.01
    DSC_SPECIFICATION       = 0x00010006
    OUTPUT_DIRECTORY        = Build/BootLoaderPkg
    SKUID_IDENTIFIER        = DEFAULT

[Components]
    BootLoaderPkg/BootApp/App/edk2/App.inf
    BootLoaderPkg/BootApp/consumer/SystemBootConfig/SystemBootConfig.inf
    BootLoaderPkg/BootApp/LibYaml/LibYaml.inf

    BootLoaderPkg/MemoryInfo/memory_info.inf

[LibraryClasses]
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    !if $(DEBUG_ENABLE_OUTPUT)
        DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
        DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    !else   ## DEBUG_ENABLE_OUTPUT
        DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
    !endif  ## DEBUG_ENABLE_OUTPUT
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

    LibYaml|BootLoaderPkg/BootApp/LibYaml/LibYaml.inf
    SystemBootConfig|BootLoaderPkg/BootApp//consumer/SystemBootConfig/SystemBootConfig.inf

    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    PrintLib|MdePkg\Library\BasePrintLib\BasePrintLib.inf

    
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf

    BaseCpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf

    MemoryInfo|BootLoaderPkg/MemoryInfo/memory_info.inf