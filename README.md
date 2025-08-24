**After repository creation:**
- [* ] Update this `README.md`. Update the Project Name, description, and all sections. Remove this checklist.
- [* ] If required, update `LICENSE.txt` and the License section with your project's approved license
- [* ] Search this repo for "REPLACE-ME" and update all instances accordingly
- [* ] Update `CONTRIBUTING.md` as needed
- [* ] Review the workflows in `.github/workflows`, updating as needed. See https://docs.github.com/en/actions for information on what these files do and how they work.
- [ ] Review and update the suggested Issue and PR templates as needed in `.github/ISSUE_TEMPLATE` and `.github/PULL_REQUEST_TEMPLATE`

# Resouce Tuner

Resource Tuner is a lightweight daemon that monitors and dynamically regulates CPU, memory, and I/O usage of user-space processes.
It leverages kernel interfaces like procfs, sysfs and cgroups to enforce runtime policies, ensuring system stability and performance in embedded and resource-constrained environments.

## Branches

**main**: Primary development branch. Contributors should develop submissions based on this branch, and submit pull requests to this branch.

## Requirements

This project depends on the following external libraries:
* yaml-cpp – Used for parsing and handling YAML configuration files.
* Installing yamlcpp:
  * Yocto: Add the following to your recipe or image
    ```bash
    DEPENDS += "yaml-cpp"
    ```

## Build and install Instructions
* Create a build directory
```bash
mkdir -p build && cd build
```
* Configure the project
```bash
cmake .. -DBUILD_SIGNALS=ON
```
* Build the project
```bash
cmake  --build .
```
* Install to default directory (/usr/local)
```bash
sudo cmake --install .
```
* Install to a custom temporary directory
```bash
cmake --install . --prefix /tmp/systune-install
```

## Usage

Describe how to use the project.

## Development

How to develop new features/fixes for the software. Maybe different than "usage". Also provide details on how to contribute via a [CONTRIBUTING.md file](CONTRIBUTING.md).

## Getting in Contact

How to contact maintainers. E.g. GitHub Issues, GitHub Discussions could be indicated for many cases. However a mail list or list of Maintainer e-mails could be shared for other types of discussions. E.g.

* [Report an Issue on GitHub](../../issues)
* [Open a Discussion on GitHub](../../discussions)
* [E-mail us](mailto:CSE.Perf.dev@qti.qualcomm.com) for general questions

## License

*userspace-resource-moderator* is licensed under the [BSD-3-Clause-Clear license](https://spdx.org/licenses/BSD-3-Clause-Clear.html). See [LICENSE.txt](LICENSE.txt) for the full license text.
