# Binary Format Specification
### File header structure
| Section | Description |
|--|--|
| main | Contains information about executable such as offsets to sections |
| text | Executable machine code section |
| data | Program read-write data section |

### Main header
| Field | Size | Description |
|--|--|--|
| entry | 8 | Program entry point |
| txtsz | 4 | Text section size in bytes |
| datsz | 4 | Data section size in bytes |