## Compile and run:
```shell
  gcc -o linkmon linkmon.c
  sudo ./linkmon eth0
```

## Sample output
```
  Monitoring eth0 for link events (Ctrl+C to stop)...

  === LINK DOWN at 14:32:15.482913 ===
    Reg  0 (BMCR    ): 0x0100
    Reg  1 (BMSR    ): 0x7809  [Link:DOWN AuNeg:...]
    Reg  4 (ANAR    ): 0x01e1
    Reg  5 (ANLPAR  ): 0x0000
    Reg  6 (ANER    ): 0x0000
    Reg 17 (MODE_STS): 0x0000
    Reg 29 (INT_SRC ): 0x0010  [LinkDn ]
    Reg 31 (SPECIAL ): 0x0000  [Speed:???]

  === LINK UP at 14:32:17.129847 ===
    Reg  0 (BMCR    ): 0x0100
    Reg  1 (BMSR    ): 0x780d  [Link:UP AuNeg:...]
    Reg  4 (ANAR    ): 0x01e1
    Reg  5 (ANLPAR  ): 0x41e1
    Reg  6 (ANER    ): 0x0001
    Reg 17 (MODE_STS): 0x0002
    Reg 29 (INT_SRC ): 0x0040  [ANcmpl ]
    Reg 31 (SPECIAL ): 0x0014  [Speed:10F]
```
