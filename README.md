# Impersonator Shell

A sophisticated security assessment tool that leverages Windows token manipulation for post-exploitation analysis. This project combines the functionality of traditional penetration testing tools with advanced Windows API integration for seamless privilege escalation capabilities.

## Key Features

- **Native Token Manipulation**: Utilizes Windows API for token operations without requiring external tools
- **Automatic Privilege Detection**: Identifies and leverages available SeImpersonatePrivilege
- **Fallback Mechanisms**: Gracefully degrades to non-administrative shell when privileges are unavailable
- **Metasploit Integration**: Supports session upgrade to Meterpreter shell
- **Zero External Dependencies**: Operates using only native Windows components

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
- Process spawning and management
- Session handling

## Roadmap

### Planned Features

- Advanced file transfer system with integrity verification
- LSASS memory analysis capabilities
- Comprehensive system enumeration module
- Build automation through Makefile integration
- Implementing the following architecture

#### Architecture Overview

The Impersonator Shell is built around three core components:

1. **Token Manipulation Engine**: Interfaces directly with Windows API for process token operations
2. **Shell Interface**: Provides command execution and session management
3. **Network Communication Layer**: Handles connections and protocol implementation

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

## Acknowledgments
- [Build Your Own Shell Project](https://github.com/AleksaZatezalo/BYOS)
- [Process Impersonator](https://github.com/AleksaZatezalo/ProcessImpersonator)
A list of known issues and features that are currently being addressed are maintained on the github issues tab. Please look at the list of known issues before raising a new issue.

## Donation Link

If you have benefited from this project and use Monero please consider donanting to the following address:
47RoH3K4j8STLSh8ZQ2vUXZdh7GTK6dBy7uEBopegzkp6kK4fBrznkKjE3doTamn3W7A5DHbWXNdjaz2nbZmSmAk8X19ezQ

## References

[Build Your Own Shell (BYOS) Project](https://github.com/AleksaZatezalo/BYOS)

[SEImpersonatePriveledge](https://learn.microsoft.com/en-us/answers/questions/1087721/how-to-disable-seimpersonate-privilege-for-a-user)

[Process Impersonator](https://github.com/AleksaZatezalo/ProcessImpersonator)

[Understanding and Abusing Process Tokens — Part I](https://securitytimes.medium.com/understanding-and-abusing-process-tokens-part-i-ee51671f2cfa)

[Understanding and Abusing Process Tokens — Part II](https://securitytimes.medium.com/understanding-and-abusing-access-tokens-part-ii-b9069f432962)
