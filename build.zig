const std = @import("std");

pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we don't set the target explicitly as it's typically provided via options.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const optimize = b.standardOptimizeOption(.{});

    // Add the executable
    const exe = b.addExecutable(.{
        .name = "meta24_zig",
        .target = target,
        .optimize = optimize,
        // .linkage = .static,
        .strip = true,
    });

    exe.addCSourceFile(.{
        .file = b.path("meta24.cc"),
        .flags = &.{ "-std=c++17", "-ftemplate-depth=4000", "-s" },
    });

    exe.root_module.c_macros.append(b.allocator, "-DMP11") catch @panic("OOM");
    // Add include path
    exe.addIncludePath(b.path("mp11/include"));
    exe.linkLibCpp();

    // Install the binary
    b.installArtifact(exe);

    // If you want to run the executable after building, you can add a run step like this:
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    // Create a build step with the standard "run" step
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
