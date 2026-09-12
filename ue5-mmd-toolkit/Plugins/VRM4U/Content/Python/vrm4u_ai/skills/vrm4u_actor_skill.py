import unreal

from toolset_registry.agent_skill import agent_skill


_INSTRUCTIONS = (
    "Choose VRM4U actors by responsibility rather than replacing standard "
    "Unreal actors indiscriminately.\n"
    "VrmModelActor stores a skeletal mesh, materials, textures, and base "
    "materials, but its native class does not construct or animate a character "
    "by itself. Inspect the concrete Blueprint or setup using it before treating "
    "it as a complete character actor.\n"
    "VrmSceneCaptureComponent2D follows the level editor viewport associated "
    "with its world or Player 0's game view. It can obtain GBuffer-derived data "
    "such as base color, normal, and MRS, as well as scene-texture data such as "
    "depth, custom stencil, and custom depth, through separate render targets. "
    "Use the render target that corresponds to the buffer required by the "
    "downstream material or processing step. The captured GBuffer and "
    "scene-texture render targets can be supplied to materials and used during "
    "Base Pass rendering.\n"
    "VRM4U can also capture the active editor, game, or offline-render view "
    "into render targets. Use this screen-capture path for viewport or game-frame "
    "buffers such as color, depth, normal, velocity, GBuffer, ambient occlusion, "
    "or custom depth. It is separate from scene capture and does not capture "
    "SceneCapture views. Remove capture registrations when they are no longer "
    "needed.\n"
    "VRM4U also provides a screen-space rim-light filter. It requires SM6 and "
    "the project's Custom Depth-Stencil Pass to be enabled with stencil. Ensure "
    "the intended meshes write matching custom-stencil values, then configure "
    "the filter's stencil mask, light direction or position, color, intensity, "
    "edge behavior, and priority. When multiple rim filters are present, inspect "
    "their priorities and remove obsolete registrations before judging the "
    "result.\n"
    "To draw VRM4U outlines with BP_VrmOutlineComponent, add or attach it as a "
    "child of the intended SkeletalMeshComponent. Confirm that it is attached "
    "to the correct skeletal mesh, then verify the outline in the intended "
    "viewport before tuning its appearance.\n"
    "Add VrmCameraCheckComponent to the actor responsible for camera-dependent "
    "processing. Use it when "
    "that processing must also react to camera movement while editing. Bind the "
    "shared update logic to its camera-move event for the editor, and invoke the "
    "same logic from the runtime camera update or tick path in the game. Its "
    "native camera-move listener is editor-only, so do not rely on that event "
    "alone in a packaged game.\n"
    "Before editing a VRM4U-specific actor, inspect its class, components, "
    "attachments, and referenced assets. Preserve Blueprint behavior that is "
    "not defined by the native class.\n"
    "After changing camera, scene-capture, or rendering-filter setup, verify the "
    "result in the intended viewport or render path rather than relying only "
    "on property values.\n"
)


@agent_skill
class VRM4UActorSkill(unreal.AgentSkill):
    """Use when placing or configuring VRM4U model actors, scene-capture
    components, viewport or screen-buffer capture, capture filters, rim-light
    filters, outline components, or monitoring components."""

    instructions = _INSTRUCTIONS
