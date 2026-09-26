//==============================================================================
// Copyright (c) 2026 KritvaOS
// SPDX-License-Identifier: Apache-2.0
//
// File        : intern-work-package.md
// Description : Intern implementation governance.
//
// Component   : Kritva Core
// Module      : Development
// Layer       : Core Foundation
//
// Requirements: CORE-DEV-001
// API         : CORE-API-DEV
//
// Author      : KritvaOS Core Team
// Created     : 26-09-2026
//==============================================================================


# Kritva Core Intern Work Package

Each task must reference an existing file and requirement ID.

An intern may:
1. implement TODOs in assigned files;
2. add tests under the assigned test domain;
3. fix implementation defects.

An intern may not, without architecture review:
- add a new top-level Core domain;
- rename a public API;
- change a public signature;
- add an OS/vendor dependency;
- add ROS 2/DDS/EtherCAT to Core;
- move files across architectural boundaries.

Workflow:
Requirement -> API -> behavior -> test -> implementation -> review.
