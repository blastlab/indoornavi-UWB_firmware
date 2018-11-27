# Readme file

## Code organization

Code is divided into platform dependent and independent part. First one is in
folder platform and each of those function has prefix 'PORT_', rest of files
comes from second part.

Huge part of code is called from interrupts and there is also main program loop
for functionalities without process time restrictions.

To build code GNU cross tool chain has been used.

## Interrupts

There should be implemented interrupt from:

1. DW1000 IRQ pin
1. Slot timer timeout

## Code style

```C
/**
 * @brief short description
 *
 * @param[in] arg_name1 description
 * @param[in] arg_name2 description
 *
 * @return description
 */
int PREFIX_FunctionName(int arg_name1, int arg_name2);
```

Files names should contains only small letters, underscores, dot and number.

## Bootloader

Firmware upgrade functionality need to have installed special bootloader
on the target platform. Moreover there is a few variables imported from
linker script to firmware upgrade module.

## Authors

Hardware:

1. L4V0.1 - Karol Trzciński
1. L4V0.2 - Karol Trzciński
1. L4V0.3 - Karol Trzciński
1. L4V0.4 - Karol Trzciński
1. L4V0.5 - Karol Trzciński
1. NrfV1.1 - DevaWave
1. AnV2.0 - Andrzej Kuczwara

Firmware:
1. Karol Trzciński:
    - system architecture
    - DW1000 usage
    - trilateration algorithms
    - radio protocols
2. Dawid Pepliński:
    - inertial navigation