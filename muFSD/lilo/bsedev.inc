BIOSPARAMETERBLOCK      struc
bspblk_usBytesPerSector         dw      ?
bspblk_bSectorsPerCluster       db      ?
bspblk_usReservedSectors        dw      ?
bspblk_cFATs                    db      ?
bspblk_cRootEntries             dw      ?
bspblk_cSectors                 dw      ?
bspblk_bMedia                   db      ?
bspblk_usSectorsPerFAT          dw      ?
bspblk_usSectorsPerTrack        dw      ?
bspblk_cHeads                   dw      ?
bspblk_cHiddenSectors           dd      ?
bspblk_cLargeSectors            dd      ?
bspblk_abReserved               db      6 dup (?)
bspblk_cCylinders               dw      ?
bspblk_bDeviceType              db      ?
bspblk_fsDeviceAttr             dw      ?
BIOSPARAMETERBLOCK      ends
