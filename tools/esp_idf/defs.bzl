"""Bazel rules for ESP-IDF builds."""

def _esp_idf_build_impl(ctx):
    """Implementation of esp_idf_build rule."""

    target = ctx.attr.target
    project_dir = ctx.attr.project_dir
    sdkconfig_defaults = ctx.attr.sdkconfig_defaults

    # Output files
    outputs = []
    bin_file = ctx.actions.declare_file("{}.bin".format(ctx.attr.name))
    outputs.append(bin_file)

    # Build command
    sdkconfig_arg = ""
    if sdkconfig_defaults:
        sdkconfig_arg = "-DSDKCONFIG_DEFAULTS='{}'".format(";".join(sdkconfig_defaults))

    script_content = """#!/bin/bash
set -euo pipefail

# Source ESP-IDF environment
if [[ -f "${{IDF_PATH}}/export.sh" ]]; then
    source "${{IDF_PATH}}/export.sh" > /dev/null
fi

# Source ESP-Matter environment if available
if [[ -f "${{ESP_MATTER_PATH}}/export.sh" ]]; then
    source "${{ESP_MATTER_PATH}}/export.sh" > /dev/null
fi

cd {project_dir}

# Clean and build
idf.py set-target {target}
idf.py {sdkconfig_arg} build

# Copy output
cp build/{name}.bin {output}
""".format(
        project_dir = project_dir,
        target = target,
        sdkconfig_arg = sdkconfig_arg,
        name = ctx.attr.name,
        output = bin_file.path,
    )

    script = ctx.actions.declare_file("{}_build.sh".format(ctx.attr.name))
    ctx.actions.write(
        output = script,
        content = script_content,
        is_executable = True,
    )

    ctx.actions.run(
        outputs = outputs,
        inputs = ctx.files.srcs,
        executable = script,
        mnemonic = "EspIdfBuild",
        progress_message = "Building ESP-IDF project for {}".format(target),
        use_default_shell_env = True,
    )

    return [DefaultInfo(files = depset(outputs))]

esp_idf_build = rule(
    implementation = _esp_idf_build_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            doc = "Source files for the ESP-IDF project",
        ),
        "target": attr.string(
            mandatory = True,
            doc = "ESP32 target (esp32c6, esp32h2)",
        ),
        "project_dir": attr.string(
            mandatory = True,
            doc = "Path to ESP-IDF project directory",
        ),
        "sdkconfig_defaults": attr.string_list(
            doc = "List of sdkconfig.defaults files",
        ),
    },
    doc = "Build an ESP-IDF project using idf.py",
)
