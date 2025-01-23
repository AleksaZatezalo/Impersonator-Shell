# Impersonator Shell

A sophisticated security assessment tool that leverages Windows token manipulation for post-exploitation analysis. This project combines the functionality of traditional penetration testing tools with advanced Windows API integration for seamless privilege escalation capabilities.

## Key Features

- **Native Token Manipulation**: Utilizes Windows API for token operations without requiring external tools
- **Zero External Dependencies**: Operates using only native Windows components
- **Enumeration Capabilities**: The binary has a simplified version of [WinPeas](https://github.com/peass-ng/PEASS-ng/tree/master/winPEAS) and [Nmap](https://github.com/nmap/nmap) build into it.

## Technical Implementation

### Prerequisites

- Windows development environment
- GCC compiler
- Windows SDK (for API headers)

### Compilation

```bash
gcc .\doexec.c .\main.c .\token_info.c .\winserver.c -o impersonate -lws2_32 %windir%\system32\advapi32.dll
```

### Core Components

#### Token Management

- Process token acquisition
- Privilege elevation through SeImpersonatePrivilege
- Token impersonation and manipulation

#### Shell Operations

- Command execution environment
- Session handling

#### Post Exploitation Operations

- Transfering files between target and attacking machine
- Downloading files to the target machine
- Enumerating the target OS
- Extracting information from the underlying OS


## Roadmap

### Planned Features

- Advanced file transfer system with integrity verification
- LSASS memory analysis capabilities
- Comprehensive system enumeration module
- Build automation through Makefile integration
- Clean up architecture and code styles

### Security Considerations

- Token privilege verification
- Session isolation
- Secure communication protocols

### Contributing

1. **Issue Tracking**: All changes must reference an existing issue
   - Create new issues for undocumented problems
   - Use detailed descriptions and steps to reproduce

2. **Build Process**
   - Remove build artifacts before commits
   - Verify clean compilation
   - Update documentation for interface changes

3. **Testing**
   - Add test cases for new functionality
   - Verify existing test suite passes
   - Document test coverage

## Donation Link

If you have benefited from this project and use Monero please consider donanting to the following address:
47RoH3K4j8STLSh8ZQ2vUXZdh7GTK6dBy7uEBopegzkp6kK4fBrznkKjE3doTamn3W7A5DHbWXNdjaz2nbZmSmAk8X19ezQ

## References

[Build Your Own Shell (BYOS) Project](https://github.com/AleksaZatezalo/BYOS)

[SEImpersonatePriveledge](https://learn.microsoft.com/en-us/answers/questions/1087721/how-to-disable-seimpersonate-privilege-for-a-user)

[Process Impersonator](https://github.com/AleksaZatezalo/ProcessImpersonator)

[Understanding and Abusing Process Tokens — Part I](https://securitytimes.medium.com/understanding-and-abusing-process-tokens-part-i-ee51671f2cfa)

[Understanding and Abusing Process Tokens — Part II](https://securitytimes.medium.com/understanding-and-abusing-access-tokens-part-ii-b9069f432962)
