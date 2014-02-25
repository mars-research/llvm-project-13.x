# RUN: llvm-mc %s -triple=mipsel-unknown-linux -show-encoding -mcpu=mips32r2 | \
# RUN: FileCheck -check-prefix=CHECK32  %s
# RUN: llvm-mc %s -triple=mips64el-unknown-linux -show-encoding -mcpu=mips64r2 | \
# RUN: FileCheck -check-prefix=CHECK64  %s

# Check that the assembler can handle the documented syntax
# for jumps and branches.
#------------------------------------------------------------------------------
# Branch instructions
#------------------------------------------------------------------------------
# CHECK32:   b 1332                 # encoding: [0x4d,0x01,0x00,0x10]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bc1f 1332              # encoding: [0x4d,0x01,0x00,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bc1t 1332              # encoding: [0x4d,0x01,0x01,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   beq $9, $6, 1332       # encoding: [0x4d,0x01,0x26,0x11]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgez $6, 1332          # encoding: [0x4d,0x01,0xc1,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgezal $6, 1332        # encoding: [0x4d,0x01,0xd1,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bltzal $6, 1332        # encoding: [0x4d,0x01,0xd0,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgtz $6, 1332          # encoding: [0x4d,0x01,0xc0,0x1c]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   blez $6, 1332          # encoding: [0x4d,0x01,0xc0,0x18]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bne $9, $6, 1332       # encoding: [0x4d,0x01,0x26,0x15]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bal  1332              # encoding: [0x4d,0x01,0x11,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bnez $11, 1332         # encoding: [0x4d,0x01,0x60,0x15]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   beqz $11, 1332         # encoding: [0x4d,0x01,0x60,0x11]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]

# CHECK64:   b 1332                 # encoding: [0x4d,0x01,0x00,0x10]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bc1f 1332              # encoding: [0x4d,0x01,0x00,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bc1t 1332              # encoding: [0x4d,0x01,0x01,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   beq $9, $6, 1332       # encoding: [0x4d,0x01,0x26,0x11]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgez $6, 1332          # encoding: [0x4d,0x01,0xc1,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgezal $6, 1332        # encoding: [0x4d,0x01,0xd1,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bltzal $6, 1332        # encoding: [0x4d,0x01,0xd0,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgtz $6, 1332          # encoding: [0x4d,0x01,0xc0,0x1c]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   blez $6, 1332          # encoding: [0x4d,0x01,0xc0,0x18]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bne $9, $6, 1332       # encoding: [0x4d,0x01,0x26,0x15]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bal     1332           # encoding: [0x4d,0x01,0x11,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bnez $11, 1332         # encoding: [0x4d,0x01,0x60,0x15]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   beqz $11, 1332         # encoding: [0x4d,0x01,0x60,0x11]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]

.set noreorder

         b 1332
         nop
         bc1f 1332
         nop
         bc1t 1332
         nop
         beq $9,$6,1332
         nop
         bgez $6,1332
         nop
         bgezal $6,1332
         nop
         bltzal $6,1332
         nop
         bgtz $6,1332
         nop
         blez $6,1332
         nop
         bne $9,$6,1332
         nop
         bal 1332
         nop
         bnez $11,1332
         nop
         beqz $11,1332
         nop

end_of_code:

#------------------------------------------------------------------------------
# Branch likely instructions
#------------------------------------------------------------------------------

# CHECK32:   bc1fl 1332             # encoding: [0x4d,0x01,0x02,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bc1tl 1332             # encoding: [0x4d,0x01,0x03,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   beql $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x51]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgezl $6, 1332         # encoding: [0x4d,0X01,0xc3,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgezall $6, 1332       # encoding: [0x4d,0x01,0xd3,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bltzall $6, 1332       # encoding: [0x4d,0x01,0xd2,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgtzl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x5c]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   blezl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x58]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bnel $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x55]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]

# CHECK64:   bc1fl 1332             # encoding: [0x4d,0x01,0x02,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bc1tl 1332             # encoding: [0x4d,0x01,0x03,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   beql $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x51]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgezl $6, 1332         # encoding: [0x4d,0X01,0xc3,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgezall $6, 1332       # encoding: [0x4d,0x01,0xd3,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bltzall $6, 1332       # encoding: [0x4d,0x01,0xd2,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgtzl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x5c]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   blezl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x58]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bnel $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x55]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]

        bc1fl 1332
        nop
	bc1tl 1332
        nop
        beql $9, $6, 1332
        nop
        bgezl $6, 1332
        nop
        bgezall $6, 1332
        nop
	bltzall $6, 1332
        nop
        bgtzl $6, 1332
        nop
        blezl $6, 1332
        nop
        bnel $9, $6, 1332
        nop

#------------------------------------------------------------------------------
# Jump instructions
#------------------------------------------------------------------------------
# CHECK32:   j 1328               # encoding: [0x4c,0x01,0x00,0x08]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jal 1328             # encoding: [0x4c,0x01,0x00,0x0c]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jalr $6              # encoding: [0x09,0xf8,0xc0,0x00]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jalr $25             # encoding: [0x09,0xf8,0x20,0x03]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jalr $10, $11        # encoding: [0x09,0x50,0x60,0x01]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jr $7                # encoding: [0x08,0x00,0xe0,0x00]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jr $7                # encoding: [0x08,0x00,0xe0,0x00]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jalr  $25            # encoding: [0x09,0xf8,0x20,0x03]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   jalr  $4, $25        # encoding: [0x09,0x20,0x20,0x03]
# CHECK32:   nop                  # encoding: [0x00,0x00,0x00,0x00]

# CHECK64:   j 1328               # encoding: [0x4c,0x01,0x00,0x08]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jal 1328             # encoding: [0x4c,0x01,0x00,0x0c]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jalr $6              # encoding: [0x09,0xf8,0xc0,0x00]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jalr $25             # encoding: [0x09,0xf8,0x20,0x03]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jalr $10, $11        # encoding: [0x09,0x50,0x60,0x01]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jr $7                # encoding: [0x08,0x00,0xe0,0x00]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jr $7                # encoding: [0x08,0x00,0xe0,0x00]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jalr  $25            # encoding: [0x09,0xf8,0x20,0x03]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   jalr  $4, $25        # encoding: [0x09,0x20,0x20,0x03]
# CHECK64:   nop                  # encoding: [0x00,0x00,0x00,0x00]


   j 1328
   nop
   jal 1328
   nop
   jalr $6
   nop
   jalr $31, $25
   nop
   jalr $10, $11
   nop
   jr $7
   nop
   j $7
   nop
   jal  $25
   nop
   jal  $4,$25
   nop
