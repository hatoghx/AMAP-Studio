import unreal

from toolset_registry.agent_skill import agent_skill


_INSTRUCTIONS = (
    "Before importing a VRM model, present the relevant material-type choices "
    "and their intended rendering behavior. For a first trial, recommend "
    "VRMIMT_SSSProfile first and VRMIMT_Unlit second. Mention that other modes "
    "are available, but do not enumerate them during the initial recommendation "
    "unless the user asks. Explicitly recommend VRMIMT_SSSProfile when the "
    "character will be combined with Unreal Engine PBR-rendered surfaces. Do "
    "not silently choose a material type when the user has not specified one.\n"
    "Inspect the imported VRM asset and its metadata before modifying generated assets.\n"
    "Preserve the source model and import settings so changes remain reproducible.\n"
    "Determine whether the asset follows VRM 0.x or VRM 1.0 conventions before "
    "changing humanoid mappings or expressions.\n"
    "VRM4U includes a Morph Target control widget for interactively changing "
    "the morph-target weights of a character. Inspect the bundled widget and "
    "confirm its target skeletal mesh or character binding before using it.\n"
    "VRM4U can receive character poses through the VMC protocol over OSC. "
    "Configure the receiver endpoint, connect the received pose to the intended "
    "character through the VMC animation path, and verify the bone and curve "
    "mapping against the model's VRM metadata. Avoid endpoint conflicts and "
    "stop receivers that are no longer needed.\n"
    "After a mutation, validate the affected assets and report warnings before saving.\n"
)


@agent_skill
class VRM4UWorkflowSkill(unreal.AgentSkill):
    """Use for VRM model imports, humanoid mappings, expressions, Morph Target
    control, animation retargeting, and VMC pose reception with VRM4U."""

    instructions = _INSTRUCTIONS
