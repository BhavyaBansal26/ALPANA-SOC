/* test vectors: name, kernel call, (golden filled by gen_golden on the host) */
#define STRESS_TESTS(X) \
  X(alu_a,    k_alu(0x1234567u, 3000))      \
  X(alu_b,    k_alu(0x9e3779b9u, 3000))     \
  FPX(X)                                    \
  X(ctrl,     k_ctrl(0xabcdef01u, 4000))

#if defined(HOST) || defined(__riscv_flen)
#define FPX(X) \
  X(fpu_norm, k_fpu(0xc0ffee11u, 1500, 0))  \
  X(fpu_spec, k_fpu(0x0badf00du, 1500, 0))  \
  X(fpu_subn, k_fpu(0x5eed5eedu, 800, 1))
#else
#define FPX(X)
#endif
