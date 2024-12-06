# What is DAWck?
DAWck is the [ChucK](https://github.com/ccrma/chuck) audio programming language as an integrated plugin for digital audio workstations.

## Building DAWck
To clone DAWck and dependencies:
```
git clone --recurse-submodules https://github.com/ccrma/dawck.git
```
`DAWck.jucer` (requires JUCE) contains the information to generate a project that upon successful compilation, produces one or more plugins named `ChucK.x` where `x` correspond to a plugin interface (e.g., VST or AudioUnit). For now, start with `DAWck.jucer`. In the future, DAWck may include pre-generated projects (XCode, Visual Studio, cmake).
