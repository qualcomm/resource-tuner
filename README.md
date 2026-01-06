# Userspace Resource Manager (URM)

Official repo of URM project. Includes:
- resource-tuner
- contextual-classifier

## Branches

**main**: Primary development branch. Contributors should develop submissions based on this branch, and submit pull requests to this branch.

**gh-pages**: Used for hosting and deploying doxygen generated source documentation

## Requirements

This project depends on the following external libraries:
* libyaml – Used for parsing and handling YAML configuration files.
* libsystemd - Used for communication over dbus (optional)
* Installing Dependencies:
  * Yocto: Add the following to your recipe or image
    ```bash
    DEPENDS += "libyaml systemd"
    ```
  * Ubuntu:
    ```bash
    apt-get install -y cmake
    apt-get install -y pkg-config
    apt-get install -y libyaml-dev
    apt-get install -y libsystemd-dev
    # Optional packages
    apt-get install -y fasttext
    apt-get install -y libfasttext-dev
    ```

## Build and install Instructions
* Create a build directory
```bash
mkdir -p build && cd build
```
* Configure the project:
Default Build
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/
```

With classifier support enabled:
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/ -DBUILD_CLASSIFIER=ON
```
* Build the project
```bash
cmake  --build .
```
* Install to default directory (/usr/local)
```bash
sudo cmake --install .
```
* Start the URM Server
```bash
/usr/bin/urm
```

* Install to a custom temporary directory [Optional]
```bash
cmake --install . --prefix /tmp/urm-install
```
## Build Debian packages
* Install build tools
```
sudo apt-get install debhelper dh-sequence-cmake cmake pkg-config build-essential
```
* Build URM
```bash 
dpkg-buildpackage -us -uc -ui
```
* Above step generated deb packages one level above the workspace 
```
ls -l ../userspace-resource-manager_1.0.0_amd64.deb   
```
* Install urm package
```
sudo dpkg -i   ../userspace-resource-manager_1.0.0_amd64.deb
```
* Check urm service status
```
#systemctl status urm
● urm.service - URM Service
     Loaded: loaded (/usr/lib/systemd/system/urm.service; enabled; preset: enabled)
     Active: active (running) since Tue 2026-01-06 14:06:04 +0530; 6s ago
   Main PID: 4048154 (urm)
      Tasks: 35 (limit: 77057)
     Memory: 8.6M (peak: 11.3M)
        CPU: 24ms
     CGroup: /system.slice/urm.service
             └─4048154 /usr/bin/urm
```

## Documentation

Refer: https://qualcomm.github.io/userspace-resource-manager/

## Development

How to develop new features/fixes for the software. Maybe different than "usage". Also provide details on how to contribute via a [CONTRIBUTING.md file](CONTRIBUTING.md).

## Getting in Contact

How to contact maintainers. E.g. GitHub Issues, GitHub Discussions could be indicated for many cases. However a mail list or list of Maintainer e-mails could be shared for other types of discussions. E.g.

* [Report an Issue on GitHub](../../issues)
* [Open a Discussion on GitHub](../../discussions)
* [E-mail us](mailto:maintainers.resource-tuner-moderator@qti.qualcomm.com) for general questions

## License

*userspace-resource-moderator* is licensed under the [BSD-3-Clause-Clear license](https://spdx.org/licenses/BSD-3-Clause-Clear.html). See [LICENSE.txt](LICENSE.txt) for the full license text.
