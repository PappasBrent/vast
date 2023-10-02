// RUN: %vast-cc --ccopts -xc --from-source %s | %file-check %s
// RUN: %vast-cc --ccopts -xc --from-source %s > %t && %vast-opt %t | diff -B %t -

// CHECK: hl.func @oposite_signs ([[A1:%arg[0-9]+]]: !hl.lvalue<!hl.int>, [[A2:%arg[0-9]+]]: !hl.lvalue<!hl.int>) -> !hl.bool {
_Bool oposite_signs(int x, int y) {
    // CHECK: hl.expr : !hl.int
    // CHECK:  hl.expr : !hl.int
    // CHECK:   hl.ref [[A1]]
    // CHECK:   hl.ref [[A2]]
    // CHECK:   hl.bin.xor
    // CHECK:  hl.const #core.integer<0> : !hl.int
    // CHECK:  hl.cmp slt
    // CHECK: hl.implicit_cast [[R:%[0-9]+]]  IntegralToBoolean : !hl.int -> !hl.bool
    return ((x ^ y) < 0);
}
