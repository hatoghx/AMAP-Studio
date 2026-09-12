import unreal

import toolset_registry


def _import_vrm_file_with_options(
    source_file: str,
    destination_package_path: str,
    options: unreal.ImportOptionData,
) -> unreal.VrmAssetListObject:
    return (
        unreal.VrmImporterBPFunctionLibrary
        .import_vrm_file_with_options(
            source_file,
            destination_package_path,
            options,
        )
    )


@unreal.uclass()
class VRM4UAssetToolset(unreal.ToolsetDefinition):
    """Imports VRM, VRMA, and other VRM4U-supported model or animation files,
    and identifies assets that need VRM4U-aware adjustment. VRM4U plugin
    settings can also enable PMX/MMD models and all Assimp-supported formats.
    Consult the registered VRM4U workflow guidance before modifying generated
    assets."""

    @toolset_registry.tool_call
    @staticmethod
    def import_vrm_model(
        source_file: str,
        destination_package_path: str,
        material_type: unreal.VRMImportMaterialType,
    ) -> unreal.VrmAssetListObject:
        """Imports a VRM4U-supported model or animation file with an explicit
        material type.

        VRM, VRMA, GLB, and BVH are supported by default. Additional extensions
        and all Assimp-supported formats, including PMX/MMD models, can be
        enabled in the VRM4U plugin settings. The import dialog is not
        displayed, and generated assets remain unsaved after the operation.

        Args:
            source_file: VRM4U-supported model or animation file on disk.
            destination_package_path: Long package name that will contain the
                generated assets, such as /Game/Characters/Alicia.
            material_type: Material implementation used for generated
                materials.

        Returns:
            The generated VRM asset list.
        """
        if not source_file:
            raise ValueError("Source file must not be empty.")
        if not destination_package_path:
            raise ValueError(
                "Destination package path must not be empty."
            )

        options = unreal.ImportOptionData(material_type=material_type)
        imported_asset = _import_vrm_file_with_options(
            source_file,
            destination_package_path,
            options,
        )
        if imported_asset is None:
            raise RuntimeError(
                "VRM4U failed to import the model. Check the source file, "
                "destination package path, and Output Log."
            )

        return imported_asset

    @toolset_registry.tool_call
    @staticmethod
    def is_vrm4u_adjustment_target(asset: unreal.Object) -> bool:
        """Returns whether the specified asset is a VRM4U adjustment target.

        Args:
            asset: The asset to inspect.

        Returns:
            True when the asset belongs to a VRM4U import, is a VRM4U-derived
            material, or is a skeletal mesh using such a material.
        """
        if asset is None:
            raise ValueError("Asset must not be null.")

        if VRM4UAssetToolset._is_direct_vrm4u_asset(asset):
            return True

        if isinstance(asset, unreal.MaterialInterface):
            return VRM4UAssetToolset._is_vrm4u_material(asset)

        if isinstance(asset, unreal.SkeletalMesh):
            return any(
                VRM4UAssetToolset._is_vrm4u_material(
                    slot.material_interface
                )
                for slot in asset.materials
                if slot.material_interface is not None
            )

        return False

    @staticmethod
    def _is_direct_vrm4u_asset(asset: unreal.Object) -> bool:
        vrm4u_types = (
            unreal.VrmAssetListObject,
            unreal.VrmMetaObject,
            unreal.VrmLicenseObject,
            unreal.Vrm1LicenseObject,
        )
        if isinstance(asset, vrm4u_types):
            return True

        asset_list = (
            unreal.VrmBPFunctionLibrary
            .vrm_get_vrm_asset_list_object_from_asset(asset)
        )
        return asset_list is not None

    @staticmethod
    def _is_vrm4u_material(material: unreal.MaterialInterface) -> bool:
        visited = set()
        current = material

        while current is not None:
            object_path = current.get_path_name()
            if object_path in visited:
                return False
            visited.add(object_path)

            if object_path.startswith("/VRM4U/"):
                return True
            if VRM4UAssetToolset._is_direct_vrm4u_asset(current):
                return True
            if not isinstance(current, unreal.MaterialInstance):
                return False

            current = current.parent

        return False
