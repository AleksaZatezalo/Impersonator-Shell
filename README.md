# Impersonator Shell

The Impersonator Shell is a post-exploitation security assessment tool that leverages Windows token manipulation. This project combines the functionality of traditional penetration testing tools with advanced Windows API integration for seamless privilege escalation capabilities.

## Key Features

- **Native Token Manipulation**: Utilizes Windows API for token operations without requiring external tools.
- **Zero External Dependencies**: Operates using only native Windows components.

We are looking to expand the Impersonator Shell with the following:

- Dumping passwords with LSASS
- Maintaining Persistance with the Windows API
- Process Injection

### Compilation

```
gcc .\doexec.c .\main.c .\token_info.c .\winserver.c -o impersonator -lws2_32 %windir%\system32\advapi32.dll
```

### Core Components

#### Token Management

- Process token acquisition
- Privilege elevation through SeImpersonatePrivilege
- Token impersonation and manipulation

#### Shell Operations

- Command execution environment
- Session handling

### Contributing

1. **Issue Tracking**: All changes must reference an existing issue
   - Create new issues for undocumented problems
   - Use detailed descriptions and steps to reproduce

2. **Build Process**
   - Remove build artifacts before commits
   - Verify clean compilation
   - Update documentation for interface changes
  
## Presentation Information

The Impersonator Shell has been presented at the following conferences:

- BSides Austin 2024
- Wild West Hack Fest Mile High 2025

## References

[Build Your Own Shell (BYOS) Project](https://github.com/AleksaZatezalo/BYOS)

[SEImpersonatePriveledge](https://learn.microsoft.com/en-us/answers/questions/1087721/how-to-disable-seimpersonate-privilege-for-a-user)

[Process Impersonator](https://github.com/AleksaZatezalo/ProcessImpersonator)

[Understanding and Abusing Process Tokens — Part I](https://securitytimes.medium.com/understanding-and-abusing-process-tokens-part-i-ee51671f2cfa)

[Understanding and Abusing Process Tokens — Part II](https://securitytimes.medium.com/understanding-and-abusing-access-tokens-part-ii-b9069f432962)

[Dump Passwords from LSASS without Mimikatz!](https://www.youtube.com/watch?v=w5trP4SrLIo&t)
