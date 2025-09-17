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
            "forg.c",
            "src/validate.c",
            "vendor/printfc.c",
        },
        .flags = &[_][]const u8{ "-std=c99" },
    });

    // Add include path
    exe.addIncludePath(.{ .cwd_relative = "src/" });
    exe.addIncludePath(.{ .cwd_relative = "vendor/" });
    exe.addIncludePath(.{ .cwd_relative = "include/" });

    // Link libc
    exe.linkLibC();

    // Install the executable
    b.installArtifact(exe);
}
