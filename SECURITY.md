# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 0.1.x   | Yes       |

## Reporting a Vulnerability

Please report security vulnerabilities via GitHub Security Advisories (preferred)
or by email to the maintainers listed in the repository.

Do NOT create public issues for security vulnerabilities.

## Security Practices

- All code is tested with ASan, UBSan, MSan, and TSan
- LibFuzzer targets for parser and chunked decoder
- PVS-Studio and CodeChecker static analysis on every commit
- RFC 9112 strict mode by default
- Size limits on request-line and headers to prevent DoS
- No dynamic memory allocation in the hot path
