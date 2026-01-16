# mojomail-eas

Exchange ActiveSync (EAS) email transport for webOS.

## Overview

This is a rewrite of the proprietary mojomail-eas binary that was part of
webOS 3.x. The original binary was never open-sourced by HP/Palm/LG, so this
implementation is based on:

1. **Architecture from mojomail-imap** - Uses the same patterns and integrates
   with the same webOS services (db8, luna-service2, etc.)

2. **Symbol analysis of the original binary** - Class names and method signatures
   extracted via `nm` and demangling

3. **Microsoft EAS protocol specifications** - [MS-ASCMD], [MS-ASWBXML], etc.

## Project Structure

```
eas/
├── inc/                    # Header files
│   ├── activity/          # Activity management
│   ├── client/            # Client session management
│   ├── commands/          # EAS command implementations
│   ├── data/              # Data adapters (email, calendar, contacts, tasks)
│   ├── exceptions/        # Exception classes
│   ├── protocol/          # HTTP/HTTPS transport
│   ├── sync/              # Sync state management
│   └── wbxml/             # WBXML encoder/decoder
├── src/                    # Implementation files
│   └── [mirrors inc/]
├── files/                  # Configuration files
│   ├── db8/kinds/         # Database schema definitions
│   ├── sysbus/            # Luna service configuration
│   └── etc/palm/          # Service settings
└── test/                   # Unit tests
```

## Key Classes

| Class | Description |
|-------|-------------|
| `EasClient` | Main client, manages connection and command queue |
| `EasBusDispatcher` | Luna service handler, routes bus messages |
| `WbxmlEncoder` | Encodes requests to WBXML format |
| `WbxmlParser` | Parses WBXML responses from server |
| `EasCommand` | Base class for all EAS commands |
| `EasAccount` | Account configuration data |

## EAS Commands to Implement

| Command | Description | Priority |
|---------|-------------|----------|
| `Options` | Query server capabilities | High |
| `Provision` | Device security policy | High |
| `FolderSync` | Sync folder hierarchy | High |
| `Sync` | Sync collection items | High |
| `Ping` | Push notification | High |
| `SendMail` | Send email | High |
| `MoveItems` | Move emails between folders | Medium |
| `MeetingResponse` | Accept/decline meetings | Medium |
| `Search` | GAL search | Medium |
| `GetAttachment` | Download attachment | Medium |
| `SmartForward/Reply` | Reply with original | Low |
| `ItemOperations` | Fetch/empty folder | Low |

## Dependencies

- db8 (mojodb) - Database services
- luna-service2 - IPC bus
- email-common - Shared email library
- libcurl - HTTP/HTTPS transport
- libxml2 - XML handling (for autodiscover)
- boost - Shared pointers, regex

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Protocol References

- [MS-ASCMD] - ActiveSync Command Reference
- [MS-ASWBXML] - ActiveSync WBXML Algorithm
- [MS-ASHTTP] - ActiveSync HTTP Protocol
- [MS-ASPROV] - ActiveSync Provisioning Protocol

## License

Apache License 2.0

## Contributing

This is part of the webOS Ports project. Contributions welcome!
