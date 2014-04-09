# REVIEW: should be in the standard includes

       .macro   STUB_START  module,d1,d2

                .section    .rodata.stubmodulename,"a"
                .word   0
__stub_modulestr_\module:
                .asciz  "\module"
                .align  2

                .section    .lib.stub,"wa",@progbits
                .word __stub_modulestr_\module
                .word \d1
                .word \d2
                .word __stub_idtable_\module
                .word __stub_text_\module

                .section    .rodata.stubidtable,"a"
__stub_idtable_\module:

                .section    .text.stub,"a",@progbits
__stub_text_\module:

        .endm


        .macro  STUB_END
        .endm


        .macro  STUB_FUNC   funcid,funcname

                .set push
                .set noreorder

                .section    .text.stub
                .weak   \funcname
\funcname:
                jr  $ra
                nop

                .section    .rodata.stubidtable
                .word   \funcid

                .set pop

        .endm



