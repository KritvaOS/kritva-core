# Threat Model

R0.9 keeps the threat model lightweight and architectural.

Initial concerns to track:

- spoofed or incorrect capability claims;
- configuration supplied by an unauthorized caller;
- lifetime misuse across non-owning boundaries;
- malformed inputs to public contracts;
- accidental privilege or trust expansion through future API additions.

No security implementation is implied by this document.
