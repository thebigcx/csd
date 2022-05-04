# Micro-C Programming Language
The Micro-C language is the bare minimum to qualify as high-level (in a good way). With syntax similar to C, all unnecessary languages features that encourage bloat and slow performance are removed, and the compiler is written in such a way that beginners can understand its operation with ease. It is geared towards low-level development, especially operating system development, where often times a mix of assembly and C is desirable.
## Notable Differences to C
### Functions
Functions are defined similar to K&R C, whereby parameters are arbitrary and return types can only be types that fit in registers (scalars). Parameters of functions are simply interpretations of stack memory (doesn't use registers for parameter passing).
Example:
```c
calculate()
	i8 *str;
	u64 num;
{
	if (str) return (num);
	else return (num + 1);
}
```

## Inline Assembly
No braces necessary.
```c
main()
{
	asm "cli";
	vulnerable_thread_code();
	
	asm "
		sti
		ret
	";
}
```

## Registers
Variables can be placed in whatever register you like, and then accessed with inline assembly. For this reason, there is no need for "clobbered" registers.
```c
main()
{
	u64 number: rax;
}
```