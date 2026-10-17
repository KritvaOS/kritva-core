# Kritva Core Security Policy

**Repository:** `kritva-core`
**Security Owner:** Dinesh (program decision D-032)

## 1. Scope

This policy covers reproducible security vulnerabilities in Kritva Core: code, build and release tooling, interfaces, and CI configuration maintained in this repository.

## 2. Do not disclose unfixed vulnerabilities publicly

Please do not open a public issue, pull request, discussion or post for an unpatched vulnerability.

## 3. Reporting

Use GitHub private vulnerability reporting for this repository (Security tab, "Report a vulnerability"). If that option is not available to you, contact the repository owner through the GitHub profile listed in CODEOWNERS and ask for a private channel; do not send details in the first message.

Include: affected component and version or commit, a description, steps to reproduce, observed and expected behaviour, the potential impact, and any mitigation you know of. Please leave out personal, customer, credential or production data.

## 4. What to expect

The Security Owner will make reasonable efforts to acknowledge a valid report, reproduce and assess it, identify affected versions, coordinate a fix or mitigation, and agree disclosure timing with the reporter. Response times depend on severity and maintainer availability; this policy sets no guaranteed deadlines.

## 5. Secrets

Never commit credentials, tokens, private keys or production data. If one is committed by mistake, tell the Security Owner at once so that it can be revoked or rotated; removing it from a later commit does not make it safe.
