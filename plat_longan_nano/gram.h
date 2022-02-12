#ifndef __GRAM_H__
#define __GRAM_H__

#ifndef __BYTE_ORDER__
#	error "__BYTE_ORDER__ is not defined by GCC"
#endif

#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__) && \
	(__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#	error "unknown __BYTE_ORDER__ value in GCC"
#endif

#define ATTR_UNUSED __attribute__ ((unused))

typedef unsigned char reg8_t;
typedef unsigned short reg16_t;
typedef unsigned int reg32_t;
typedef unsigned long long reg64_t;

typedef unsigned long addr_t;
typedef unsigned int offset_t;

#define RFIELD8(_end, _start, name) reg8_t name:(_end-_start+1)
#define RFIELD16(_end, _start, name) reg16_t name:(_end-_start+1)
#define RFIELD32(_end, _start, name) reg32_t name:(_end-_start+1)
#define RFIELD64(_end, _start, name) reg64_t name:(_end-_start+1)

#define RF_RSV8(_end, _start) reg8_t rsv##_end##_##_start:(_end-_start+1)
#define RF_RSV16(_end, _start) reg16_t rsv##_end##_##_start:(_end-_start+1)
#define RF_RSV32(_end, _start) reg32_t rsv##_end##_##_start:(_end-_start+1)
#define RF_RSV64(_end, _start) reg64_t rsv##_end##_##_start:(_end-_start+1)

#ifndef arch_sreg_read
#	define arch_sreg_read(_reg_key) 0xdeadbeef
#endif

#ifndef arch_sreg_write
#	define arch_sreg_write(_reg_key, _val)
#endif

/* Following scripts is used to from:

	#define GRAM32(_reg, _base, _off, ...) \
	typedef union { \
		struct { \
			__VA_ARGS__ \
		} f; \
		reg32_t v; \
	} _reg##_T; \
	static const addr_t _reg##_A ATTR_UNUSED = (addr_t)(_base + _off); \
	static const offset_t _reg##_O ATTR_UNUSED = (_off);

	For special registers, e.g., CP0_STATUS in MIPS or CSR_MSTATUS in RISCV,
		#define <_reg>_K <KEY>
	is also required.
*/

#define _GRAM_PREFIX_(...) \
	typedef union { \
		struct { \
			__VA_ARGS__ \
		} f;

#define _GRAM_POSTFIX_(_reg, _base, _off) \
	} _reg##_T; \
	static const addr_t _reg##_B ATTR_UNUSED = (_base); \
	static const offset_t _reg##_O ATTR_UNUSED = (_off); \
	static const addr_t _reg##_A ATTR_UNUSED = (addr_t)(_base + _off);

#define GRAM8(_reg, _base, _off, ...) \
	_GRAM_PREFIX_(__VA_ARGS__) \
	reg8_t v; \
	_GRAM_POSTFIX_(_reg, _base, _off)

#define GRAM16(_reg, _base, _off, ...) \
	_GRAM_PREFIX_(__VA_ARGS__) \
	reg16_t v; \
	_GRAM_POSTFIX_(_reg, _base, _off)

#define GRAM32(_reg, _base, _off, ...) \
	_GRAM_PREFIX_(__VA_ARGS__) \
	reg32_t v; \
	_GRAM_POSTFIX_(_reg, _base, _off)

#define GRAM64(_reg, _base, _off, ...) \
	_GRAM_PREFIX_(__VA_ARGS__) \
	reg64_t v; \
	_GRAM_POSTFIX_(_reg, _base, _off)

/* For both SREG and MREG */
#define RGET(_reg_name) \
	({ \
		_reg_name##_T dummy; \
		__typeof__(dummy.v) _ret; \
		if (_reg_name##_O == -1) { \
			_ret = (__typeof__(dummy.v))arch_sreg_read(_reg_name##_K);	\
		} else { \
			_ret = _RVAL(_reg_name); \
		} \
		_ret; \
	})

#define RSET(_reg_name, val) \
	({ \
		if (_reg_name##_O == -1) { \
			arch_sreg_write(_reg_name##_K, val); \
		} else { \
			_RVAL(_reg_name) = val; \
		} \
	})

#define RRMW(rtype, ...) do {										\
		rtype##_T dummyr = { .v = RGET(rtype) };		\
		PP_NARG(__VA_ARGS__)(dummyr, __VA_ARGS__);	\
		RSET(rtype, dummyr.v);											\
	} while(0)

#define RFLD(_reg, fld) ((_reg##_T)RGET(_reg)).f.fld

/* For MREG -only, change base addr. of the given `_reg' */
#define RRBS(_reg, _new_base) addr_t _reg##_A = (_new_base + _reg##_O)

/* Duplicate `_reg'_T to `_new_reg'_T, and use `_reg'_A as base addr. with offset `_new_off' */
#define GRAM_DUP(_reg, _new_reg, _new_off) \
	typedef _reg##_T _new_reg##_T; \
	static const offset_t _new_reg##_O ATTR_UNUSED = (_new_off); \
	static const addr_t _new_reg##_A ATTR_UNUSED = _reg##_A + _new_off;

#define _RVAL(_reg) (*((volatile addr_t *)(_reg##_A)))

/* for __VA_NARG__, https://groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s */
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
	_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, \
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
	_31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
	_41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
	_51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
	_61, _62, _63,   N, ...) N

#define PP_RSEQ_N() \
	_err, _rset62, _err, _rset60, _err, _rset58, _err, _rset56, \
	_err, _rset54, _err, _rset52, _err, _rset50, _err, _rset48, \
	_err, _rset46, _err, _rset44, _err, _rset42, _err, _rset40, \
	_err, _rset38, _err, _rset36, _err, _rset34, _err, _rset32, \
	_err, _rset30, _err, _rset28, _err, _rset26, _err, _rset24, \
	_err, _rset22, _err, _rset20, _err, _rset18, _err, _rset16, \
	_err, _rset14, _err, _rset12, _err, _rset10, _err,  _rset8, \
	_err,  _rset6, _err,  _rset4, _err,  _rset2, _err,    _err

/* A dummy function to halt compilation to indicate mismatching field/value pair */
#define _err(...) \
	RRMW_ERROR_field_n_value_pair_mismatched()

#define _rset2( \
	var, \
	f00, v00) do { \
		var.f.f00 = v00; \
	} while(0)

#define _rset4( \
	var, \
	f00, v00, f01, v01) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
	} while(0)

#define _rset6( \
	var, \
	f00, v00, f01, v01, f02, v02) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
	} while(0)

#define _rset8( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
	} while(0)

#define _rset10( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
	} while(0)

#define _rset12( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
	} while(0)

#define _rset14( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
	} while(0)

#define _rset16( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
	} while(0)

#define _rset18( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
	} while(0)

#define _rset20( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
	} while(0)

#define _rset22( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
	} while(0)

#define _rset24( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
	} while(0)

#define _rset26( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
	} while(0)

#define _rset28( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
	} while(0)

#define _rset30( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
	} while(0)

#define _rset32( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
	} while(0)

#define _rset34( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
	} while(0)

#define _rset36( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
	} while(0)

#define _rset38( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
	} while(0)

#define _rset40( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
	} while(0)

#define _rset42( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
	} while(0)

#define _rset44( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
	} while(0)

#define _rset46( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
	} while(0)

#define _rset48( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
	} while(0)

#define _rset50( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
	} while(0)

#define _rset52( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
	} while(0)

#define _rset54( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25, f26, v26) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
	} while(0)

#define _rset56( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25, f26, v26, f27, v27) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
	} while(0)

#define _rset58( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25, f26, v26, f27, v27, \
	f28, v28) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
	} while(0)

#define _rset60( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25, f26, v26, f27, v27, \
	f28, v28, f29, v29) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
		var.f.f29 = v29; \
	} while(0)

#define _rset62( \
	var, \
	f00, v00, f01, v01, f02, v02, f03, v03, \
	f04, v04, f05, v05, f06, v06, f07, v07, \
	f08, v08, f09, v09, f10, v10, f11, v11, \
	f12, v12, f13, v13, f14, v14, f15, v15, \
	f16, v16, f17, v17, f18, v18, f19, v19, \
	f20, v20, f21, v21, f22, v22, f23, v23, \
	f24, v24, f25, v25, f26, v26, f27, v27, \
	f28, v28, f29, v29, f30, v30) do { \
		var.f.f00 = v00; \
		var.f.f01 = v01; \
		var.f.f02 = v02; \
		var.f.f03 = v03; \
		var.f.f04 = v04; \
		var.f.f05 = v05; \
		var.f.f06 = v06; \
		var.f.f07 = v07; \
		var.f.f08 = v08; \
		var.f.f09 = v09; \
		var.f.f10 = v10; \
		var.f.f11 = v11; \
		var.f.f12 = v12; \
		var.f.f13 = v13; \
		var.f.f14 = v14; \
		var.f.f15 = v15; \
		var.f.f16 = v16; \
		var.f.f17 = v17; \
		var.f.f18 = v18; \
		var.f.f19 = v19; \
		var.f.f20 = v20; \
		var.f.f21 = v21; \
		var.f.f22 = v22; \
		var.f.f23 = v23; \
		var.f.f24 = v24; \
		var.f.f25 = v25; \
		var.f.f26 = v26; \
		var.f.f27 = v27; \
		var.f.f28 = v28; \
		var.f.f29 = v29; \
		var.f.f30 = v30; \
	} while(0)
#endif
