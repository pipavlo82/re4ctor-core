# Security Policy

## Threat Model

The `re4ctor-core` generator operates as an opaque DRBG — similar to Apple Secure Enclave or Intel SGX.  
Internal entropy sources and DRBG logic are **not** public.

Instead, transparency is achieved through:
- Continuous external validation via statistical suites (Dieharder, PractRand, NIST STS)
- SBOM publication for shipped binaries
- GPG-signed release bundles
- Reproducible verification by end users

Consumers can **verify output integrity**, but cannot reconstruct or reverse-engineer the entropy core.

In other words: we prove the tap water is clean, without giving you the plumbing diagram.

## Reporting a vulnerability
Please report any potential security issues privately via email:
**shtomko@gmail.com**
