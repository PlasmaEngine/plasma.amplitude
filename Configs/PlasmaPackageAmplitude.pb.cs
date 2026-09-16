// #pl-version 1
//
// Locates an Amplitude Audio SDK to build against.
//
// Unlike FMOD, Amplitude is Apache-2.0, so its runtime library ships inside this package and users
// need install nothing. ThirdParty/Amplitude/LICENSE.txt is the licence copy that redistribution
// requires; it is packaged alongside the DLL.

using System;
using System.IO;
using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public static class PlasmaPackageAmplitude
{
    /// \brief Finds the Amplitude SDK root - the directory holding "include" and "lib".
    ///
    /// AM_SDK_PATH is the same variable the engine's own build honours, so a developer who already
    /// has it set builds this package against the same SDK without further configuration.
    public static bool TryGetSdk(out string sdkRoot, out string reason)
    {
        sdkRoot = string.Empty;
        reason = string.Empty;

        var fromEnv = Environment.GetEnvironmentVariable("AM_SDK_PATH");
        if (!string.IsNullOrWhiteSpace(fromEnv))
        {
            fromEnv = Path.GetFullPath(fromEnv.Trim());

            if (HasSdkFiles(fromEnv))
            {
                sdkRoot = fromEnv;
                return true;
            }

            reason = $"AM_SDK_PATH is '{fromEnv}', but no Amplitude SDK was found there. It should name the " +
                     "directory containing 'include' and 'lib'.";
            return false;
        }

        var vendored = Path.Combine(PlasmaPackageSdk.Root, "Code", "ThirdParty", "amplitude", "sdk");
        if (HasSdkFiles(vendored))
        {
            sdkRoot = vendored;
            return true;
        }

        reason = "No Amplitude SDK found. Set AM_SDK_PATH to the SDK root (the directory containing " +
                 "'include' and 'lib').";
        return false;
    }

    /// \brief Include paths, the import library, and the runtime DLL that ships with the package.
    public static void ConfigureConsumer(ModuleRules rules, BuildContext context, bool bCopyRuntime)
    {
        if (context.Platform != TargetPlatform.Windows)
        {
            throw new PlatformNotSupportedException("The Amplitude plugin is only supported on Windows desktop.");
        }

        if (!TryGetSdk(out var sdkRoot, out var reason))
        {
            throw new DirectoryNotFoundException(reason);
        }

        rules.PublicIncludePaths.Add(Path.Combine(sdkRoot, "include"));

        // Both defines are part of the binary interface, not conveniences: AM_BUILDSYSTEM_SHARED
        // changes how Amplitude's symbols are decorated, so every module that sees these headers
        // has to agree on it.
        rules.PublicDefinitions.Add("BUILDSYSTEM_ENABLE_AMPLITUDE_SUPPORT");
        rules.PublicDefinitions.Add("AM_BUILDSYSTEM_SHARED");

        rules.PublicLibraries.Add(Path.Combine(sdkRoot, "lib", "x64-windows", "shared", "Amplitude.lib"));

        if (bCopyRuntime)
        {
            var binaryDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

            CopyIfDifferent(Path.Combine(sdkRoot, "lib", "x64-windows", "shared", "Amplitude.dll"),
                Path.Combine(binaryDirectory, "Amplitude.dll"));
            CopyIfDifferent(Path.Combine(sdkRoot, "lib", "x64-windows", "shared", "samplerate.dll"),
                Path.Combine(binaryDirectory, "samplerate.dll"));
            CopyIfDifferent(Path.Combine(context.ProjectRoot, "ThirdParty", "Amplitude", "samplerate-LICENSE.txt"),
                Path.Combine(binaryDirectory, "samplerate-LICENSE.txt"));

            // Apache-2.0 asks that the licence travel with the binary, so it is staged next to the
            // DLL and listed in PackageDependencies - that puts both in the same component, and a
            // component is what gets unpacked as a unit.
            CopyIfDifferent(Path.Combine(context.ProjectRoot, "ThirdParty", "Amplitude", "LICENSE.txt"),
                Path.Combine(binaryDirectory, "Amplitude-LICENSE.txt"));
        }
    }

    /// \brief The engine's PlasmaBuildRuntimeFiles equivalent, minus the parts a package cannot use.
    ///
    /// The engine helper copies into <ProjectRoot>/Binaries and writes a manifest the deployment step
    /// reads. Out here the destination is the package's own output directory and there is no
    /// deployment step, so the copy is the whole job.
    private static void CopyIfDifferent(string source, string target)
    {
        if (!File.Exists(source))
        {
            throw new FileNotFoundException($"Missing Amplitude runtime library '{source}'.", source);
        }

        if (File.Exists(target))
        {
            var from = new FileInfo(source);
            var to = new FileInfo(target);

            if (from.Length == to.Length && from.LastWriteTimeUtc <= to.LastWriteTimeUtc)
            {
                return;
            }
        }

        Directory.CreateDirectory(Path.GetDirectoryName(target));
        File.Copy(source, target, overwrite: true);
    }

    private static bool HasSdkFiles(string sdkRoot)
    {
        return File.Exists(Path.Combine(sdkRoot, "include", "SparkyStudios", "Audio", "Amplitude", "Amplitude.h")) &&
               File.Exists(Path.Combine(sdkRoot, "lib", "x64-windows", "shared", "Amplitude.lib")) &&
               File.Exists(Path.Combine(sdkRoot, "lib", "x64-windows", "shared", "Amplitude.dll"));
    }
}
