import unreal

from .compatibility import is_supported


if is_supported():
    from . import skills
    from . import tests
    from . import toolsets

    toolsets._registration.register()

    tests._test_runner = unreal.PythonTestRunner.create(
        "AI.Toolsets.VRM4U",
        unreal.PythonTestRunnerSearchOptions(root_module=tests.__name__),
    )
