import unreal


def is_supported() -> bool:
    """Returns whether the APIs required by the VRM4U AI integration exist."""
    try:
        import toolset_registry
        import toolset_registry.agent_skill
        import toolset_registry.registration
    except (ImportError, ModuleNotFoundError):
        return False

    required_unreal_types = (
        "AgentSkill",
        "ToolsetDefinition",
        "ToolsetRegistry",
        "PythonTestRunner",
        "PythonTestRunnerSearchOptions",
        "VrmAssetListObject",
        "VrmMetaObject",
        "VrmLicenseObject",
        "Vrm1LicenseObject",
        "VrmBPFunctionLibrary",
        "ImportOptionData",
        "VRMImportMaterialType",
        "VrmImporterBPFunctionLibrary",
    )
    if not all(hasattr(unreal, name) for name in required_unreal_types):
        return False

    if not callable(getattr(toolset_registry, "tool_call", None)):
        return False

    asset_lookup = getattr(
        unreal.VrmBPFunctionLibrary,
        "vrm_get_vrm_asset_list_object_from_asset",
        None,
    )
    import_vrm_model = getattr(
        unreal.VrmImporterBPFunctionLibrary,
        "import_vrm_file_with_options",
        None,
    )
    return callable(asset_lookup) and callable(import_vrm_model)
