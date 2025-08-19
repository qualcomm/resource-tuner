\mainpage Resource Tuner

\section intro_sec Introduction

Resource Tuner is a lightweight daemon that monitors and dynamically regulates CPU, memory, and I/O usage of user-space processes. It leverages kernel interfaces like procfs, sysfs and cgroups to enforce runtime policies, ensuring system stability and performance in embedded and resource-constrained environments.

\section features_sec Features

- Follows the One Software Package (OneSP) Strategy
- Highly Configurable and Extensible
- Light Weight and Flexible Packaging

\section usage_sec Getting Started

To get started with the project:

1. Clone the repository:
   \code{.sh}
   git clone https://github.com/qualcomm/resource-tuner.git
   \endcode

2. Build the project:
   \code{.sh}
   mkdir build && cd build
   cmake ..
   make
   \endcode

3. Run the application:
   \code{.sh}
   ./resource_tuner
   \endcode

Refer the **Examples** Tab for guidance on Resource Tuner API usage.

[GitHub Repo](https://github.com/qualcomm/resource-tuner/tree/main)

[Further Documentation](https://github.com/qualcomm/resource-tuner/blob/main/systune/docs/README.pdf)

\section structure_sec Project Structure

\verbatim
/Framework  → Core Resource Provisioning Request Logic
/Auxiliary  → Common Utilities and Components used across Resource Tuner Modules.
/Client     → Exposes the Client Facing APIs, and Defines the Client Communication Endpoint
/Server     → Defines the Server Communication Endpoint and other Common Server-Side Utils.
/Signals    → Optional Module, exposes Signal Tuning / Relay APIs
/Tests      → Unit and System Wide Tests
/docs       → Documentation
\endverbatim

\section contact_sec Contact

For questions, suggestions, or contributions, feel free to reach out:

- **Email**: CSE.Perf@qti.qualcomm.com

\section license_sec License

This project is licensed under the BSD 3-Clause Clear License.
