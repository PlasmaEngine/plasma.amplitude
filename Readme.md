# plasma.amplitude

Amplitude Audio middleware for PlasmaEngine — a full audio system backend with editor support for
audio control collections. Moved out of the engine tree to be built and distributed as a package.

Published as [`plasma.amplitude`](https://plasmaengine.github.io/PlasmaPackages/index.plPackageIndex)
in the Plasma package registry.

## Everything is included

Unlike the FMOD package, this one is self-contained. The Amplitude Audio SDK is Apache-2.0, so
`Amplitude.dll` ships inside the package's runtime component along with `Amplitude-LICENSE.txt` —
nothing has to be downloaded or installed separately.

## Layout

```
Package.plPackage             the manifest - identity, compatibility, components
AmplitudeAudio.plPluginBundle legacy descriptor, kept until the engine reads manifests directly
Source/
  AmplitudeAudioPlugin/       runtime -> plAmplitudeAudioPlugin  (ships Amplitude.dll)
  EditorPluginAmplitudeAudio/ editor  -> plEditorPluginAmplitudeAudio  (asset window, settings dialog)
Configs/                      build rules for building against an installed engine SDK
ThirdParty/Amplitude/         the Apache-2.0 licence that ships with the redistributable
Branding/                     icon and banner
```

## Dependencies

None resolvable through the registry. The bundle declares `RequiredPlugins{"AudioSystem"}`, but
`AudioSystem` is `Code/Engine/AudioSystem` — an engine module, not a plugin bundle. It ships with
the engine and is linked directly.

`ExclusiveFeatures{"Audio", "AudioSystemMiddleware"}` means enabling this excludes the other audio
backends — a project plays sound through one of them.

## Building

Requires an engine SDK. An Amplitude SDK is optional; without one the build falls back to the copy
vendored in the engine SDK:

```bash
PLASMA_SDK_ROOT=/path/to/engine plasmabuild build EditorPluginAmplitudeAudio
```

Set `AM_SDK_PATH` to build against a different Amplitude SDK — the same variable the engine's own
build honours. Output lands in `Bin/Windows_x64_Development/`.

## Publishing a new version

Staging and publishing are driven from the engine and registry repositories:

```bash
# engine repo - stage and validate, emitting machine-readable output
python Utilities/PackageInspector.py <staged-package-dir> --json <out>.json

# registry repo - upload blobs and record the release
python tools/publish_package.py <out>.json
```

Versions are immutable. A mistake is fixed by publishing a new patch version, never by replacing an
existing one — every lock file that already trusts a version must keep resolving to the same bytes.
