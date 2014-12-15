.data
.global _actual_handler
.align 4
_actual_handler: .word _default_handler

.text
.global _exception_handler
.align 4
_exception_handler:
        lw   $t0, _actual_handler
        j    $t0
        nop

