from unittest import mock

import unreal

from toolset_registry.tests.toolset_testcase import ToolCallTestCase
from vrm4u_ai.toolsets import vrm4u_asset
from vrm4u_ai.toolsets.vrm4u_asset import VRM4UAssetToolset


class VRM4UAssetToolsetTestCase(ToolCallTestCase):
    """Tests VRM4U asset import and identification."""

    def test_import_vrm_model_forwards_material_type(self):
        expected_asset = unreal.new_object(unreal.VrmAssetListObject)
        material_type = unreal.ImportOptionData().material_type

        with mock.patch.object(
            vrm4u_asset,
            "_import_vrm_file_with_options",
            return_value=expected_asset,
        ) as import_model:
            result = VRM4UAssetToolset.import_vrm_model(
                "C:/Models/Alicia.vrm",
                "/Game/Characters/Alicia",
                material_type,
            )

        self.assertIs(result, expected_asset)
        options = import_model.call_args.args[2]
        self.assertEqual(options.material_type, material_type)

    def test_import_vrm_model_raises_on_empty_source(self):
        material_type = unreal.ImportOptionData().material_type
        with self.assertToolRaisesRuntimeError() as error:
            VRM4UAssetToolset.import_vrm_model(
                "",
                "/Game/Characters/Alicia",
                material_type,
            )
        self.assertIn("Source file must not be empty", str(error.exception))

    def test_import_vrm_model_raises_on_empty_destination(self):
        material_type = unreal.ImportOptionData().material_type
        with self.assertToolRaisesRuntimeError() as error:
            VRM4UAssetToolset.import_vrm_model(
                "C:/Models/Alicia.vrm",
                "",
                material_type,
            )
        self.assertIn(
            "Destination package path must not be empty",
            str(error.exception),
        )

    def test_import_vrm_model_raises_when_import_fails(self):
        material_type = unreal.ImportOptionData().material_type
        with mock.patch.object(
            vrm4u_asset,
            "_import_vrm_file_with_options",
            return_value=None,
        ):
            with self.assertToolRaisesRuntimeError() as error:
                VRM4UAssetToolset.import_vrm_model(
                    "C:/Models/Alicia.vrm",
                    "/Game/Characters/Alicia",
                    material_type,
                )
        self.assertIn(
            "VRM4U failed to import the model",
            str(error.exception),
        )

    def test_vrm_asset_list_is_identified(self):
        asset = unreal.new_object(unreal.VrmAssetListObject)
        self.assertTrue(
            VRM4UAssetToolset.is_vrm4u_adjustment_target(asset)
        )

    def test_unrelated_asset_is_not_identified(self):
        asset = unreal.new_object(unreal.Texture2D)
        self.assertFalse(
            VRM4UAssetToolset.is_vrm4u_adjustment_target(asset)
        )

    def test_mesh_using_vrm4u_derived_material_is_identified(self):
        parent = unreal.load_asset(
            "/VRM4U/UE5/Material/M_VrmMToonBaseOpaque"
        )
        self.assertIsNotNone(parent)

        material = unreal.new_object(unreal.MaterialInstanceConstant)
        material.set_editor_property("parent", parent)

        mesh = unreal.new_object(unreal.SkeletalMesh)
        mesh.set_editor_property(
            "materials",
            [unreal.SkeletalMaterial(material_interface=material)],
        )

        self.assertTrue(
            VRM4UAssetToolset.is_vrm4u_adjustment_target(mesh)
        )

    def test_null_asset_raises(self):
        with self.assertToolRaisesRuntimeError() as error:
            VRM4UAssetToolset.is_vrm4u_adjustment_target(None)
        self.assertIn("Asset must not be null", str(error.exception))
