# Impersonator Shell

The Impersonator Shell was conceived as a combination of commonly used penetration testing tools like netcat and the printspoofer exploit. It later evolved into a utility focused assist security professionals with all things that occur after obtaining initial access on a target machine. The tool interacts with the Windows SeImpersonatePrivilege, which is a security setting designed to allow services to impersonate clients. When analyzing potential exposures, security engineers may examine how services with this privilege enabled could affect the system's security posture.

## Impersonator Shell? Why Bother?

Most often users running a server on a Windows host will have the `SEImpersonatePriveledge` enabled by default. This means that security engineers who can obtain RCE on a windows server, can also obtain an administrative shell by abusing the server's`SEImpersonatePriveledge` by relying on known exploits. Instead of uploading Netcat and an exploit that abuse the `SEImpersonatePriveledge` security engineers can simply use the Impersonator shell. The Impersonator Shell uses native Windows API functions to grab a proccess token and impersonate the user running said proccess.  In the event that the `SEImpersonatePriveledge` is disabled or the inbuilt exploit does not work, security engineers will get a non-administrative shell. The Impersonator shell can connect to a metasploit listener and be upgraded to a meterpreter shell.

### Next Steps

A few of the following things will be implemented as a part of the Impersonator shell:

* More robust error handeling and messages
* Removing users ability to alter privs
* File transfer functionality & downloading
* LSASS Dumping
* Adding netexec functionality to the shell

### Compilation

To compile this project clone the repo. After entering the project directory run the following command:

```
gcc .\doexec.c .\main.c .\token_info.c .\winserver.c -o impersonate -lws2_32 %windir%\system32\advapi32.dll 
```

## Contribution Guidelines

When contributing to this repository, please first discuss the change you wish to make via issue here on GitHub. Make sure all pull requests are tagged with a specific ticket number found in the repositories issues section.Before making any changes please create your own branch. Follow all three points below before opening a PR:

1. Any changes you want to create must be tagged to an issue opened on this repo. If an issue you've found does not yet exit, please open it.
2. Ensure any install or build dependencies are removed before the end of the layer when doing a build.
3. Make sure all corresponding test cases pass.
4. Update the README.md with details of changes to the interface, this includes new environment variables, exposed ports, useful file locations and container parameters.

Note that we have a code of conduct. Follow it in all your interactions with the project.

## Known Issues

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
