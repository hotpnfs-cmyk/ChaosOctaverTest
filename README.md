# Octave Chaos — Windows VST3 build project

This is an **experimental, original** octave/pitch-chaos guitar-effect prototype for Studio One. It is not Tallon Electric's plugin and does not include their code or assets. The plugin has not been independently tested or verified in Studio One; audio quality and compatibility are not guaranteed.

## Easiest route: let GitHub build the VST3 for you

You do **not** need to install Visual Studio, CMake, or JUCE on your own PC. GitHub Actions is configured to build the plugin on a Windows runner and attach the compiled file as a downloadable artifact.

1. Sign in to GitHub (or create an account).
2. Create a **new empty repository** on GitHub. Do not add a README or license there.
3. Upload the contents of this ZIP to that repository, preserving the `.github/workflows/build-windows-vst3.yml` path. You can use GitHub's web upload interface; no Git command line is required.
4. Open the repository's **Actions** tab. If prompted, enable Actions.
5. Select **Build Windows VST3**. If the workflow did not start automatically after upload, use **Run workflow**.
6. Wait for the run to finish. Open the successful run and download the artifact named **OctaveChaos-Windows-VST3**.
7. Unzip the downloaded artifact. Copy the contained `Octave Chaos.vst3` bundle to:
   `%LOCALAPPDATA%\Programs\Common\VST3`
   Create the folder if it does not exist. Alternatively, use Studio One's configured VST3 location.
8. Restart Studio One and rescan plug-ins if needed.

## Controls in the prototype

- **Octave +1 / +2**: enable the octave-up voices.
- **Blend**: dry/wet mix.
- **Panic / Chaos / Speed / Rise**: experimental random pitch modulation controls.

## Important notes

- This is a prototype, not a polished commercial effect. Pitch-shifting artifacts, latency, and build issues are possible.
- The workflow compiles the project; it does not mean the resulting plugin has been tested in Studio One.
- If the GitHub Actions run fails, open the failed run and share the first relevant error log; the project may need a code adjustment.
