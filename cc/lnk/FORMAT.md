# Binary Format Specification
### File structure
| Section | Description |
|--|--|
| main | Contains information about executable such as offsets to sections |
| text | Executable machine code section |
| data | Program read-write data section |
| symtab | Symbol table consisting of symbol entries |
| strtab | String table consisting of null-terminated strings |

### Main header
```c
struct bin_main
{
    entry: u64,
    txtsz: u32,
    datsz: u32,
    txtrel: u32,
    datrel: u32,
    symtab: u32,
    strtab: u32
};
```

| Field | Size | Description |
|--|--|--|
| entry | 8 | Program entry point |
| txtsz | 4 | Text section size in bytes |
| datsz | 4 | Data section size in bytes |
| txtrel | 4 | Text section relocations (byte offset) |
| datrel | 4 | Data section relocations (byte offset) |
| symtab | 4 | Offset in file to symbol table |
| strtab | 4 | Offset in file to string table |

### Symbol structure
```c
struct symbol
{
    name: u32,
    value: u64
};
```

| Field | Size | Description |
|--|--|--|
| name | 4 | Offset into string table for name |
| value | 8 | Value of symbol |

### Relocation entry
```c
struct rel
{
    addr: u64,
    size: u8,
    sym: u32,
    pcrel: u8,
    addend: u64
};
```

| Field | Size | Description |
|--|--|--|
| addr | 8 | Byte offset into file for the relocation |
| size | 1 | Size (in bytes) of relocation |
| sym | 4 | Index in symbol table |
| pcrel | 1 | If this relocation is for an RIP-relative instruction |
| addend | 8 | Value added to symbol value for relocation |