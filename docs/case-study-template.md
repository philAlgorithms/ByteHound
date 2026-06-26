# Case Study Template

> Copy this file when documenting a new finding. Do not claim exploitability without evidence.

## Finding title

Short descriptive name.

## Affected component

e.g. `src/parser.cpp`, `training/legacy_parser/legacy_parser.cpp`

## Discovery method

Fuzzer name, CI job, or unit test.

## Sanitizer

AddressSanitizer / UndefinedBehaviorSanitizer / other.

## Reproducer

Path to crashing input and command:

```bash
scripts/reproduce.sh ...
```

## Minimized input

Path and size in bytes.

## Observed failure

Paste sanitizer output summary (error type, fault address, stack excerpt).

## Root cause

Explain the violated assumption (e.g. unchecked length field).

## Exploitability discussion

State evidence-based assessment. Default to unknown/low unless demonstrated.

## Patch

Reference commit or file and describe the fix.

## Regression test

Test name and fixture path.

## Validation commands

```bash
# commands run after fix
```

## Limitations

What the fix does not address.

## Classification

- [ ] Organic TinyCFG parser finding
- [ ] Educational legacy fixture (intentional)
