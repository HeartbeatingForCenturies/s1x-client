# S1-Mod

<p align="center">
  <img alig src="assets/github/banner.png?raw=true" />
</p>

Proof of concept for an Advanced Warfare client. <br>
[This project is based on IW6x](https://github.com/XLabsProject/iw6x-client)

<br/>

## Compile from source

- Clone the Git repo. Do NOT download it as ZIP, that won't work.
- Update the submodules and run `premake5 vs2019` or simply use the delivered `generate.bat`.
- Build via solution file in `build\s1-mod.sln`.

### Premake arguments

| Argument                    | Description                                    |
|:----------------------------|:-----------------------------------------------|
| `--copy-to=PATH`            | Optional, copy the EXE to a custom folder after build, define the path here if wanted. |
| `--dev-build`               | Enable development builds of the client. |

<br/>

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.
