const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "forg",
        .target = target,
        .optimize = optimize,
    });

    // Add C source files
    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "src/forg.c",
            "src/validade.c",
            "src/colors.c",
        },
        .flags = &[_][]const u8{ "-std=c99" },
    });

    // Add include path
    exe.addIncludePath(.{ .cwd_relative = "src/" });

    // Link libc
    exe.linkLibC();

    // Install the executable
    b.installArtifact(exe);
}
