# Security Policy

## Supported versions

ByteHound is a portfolio and research repository. Security fixes are applied on a best-effort basis.

## Reporting

If you believe you have found a security issue in the **production TinyCFG parser** (not the educational legacy fixture), please open a private security advisory on GitHub or contact the maintainer directly.

## Scope

- **In scope:** `include/tinycfg/`, `src/`, CLI, fuzz harnesses for the main parser.
- **Out of scope / intentionally vulnerable:** `training/legacy_parser/` is documented educational code with a planted defect. Do not deploy it.

## Expectations

Fuzzing and sanitizers reduce risk but do not prove absence of vulnerabilities. See `docs/fuzzing-methodology.md`.
