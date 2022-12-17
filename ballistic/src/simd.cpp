#if defined WIN32
#include <intrin.h>
#else
#include <immintrin.h>
#endif

bool avxsupport() {
    bool support{ false };
    bool cpusupport{ false };
    bool ossupport{ true };
#if defined WIN32
    int info[4]{};
	__cpuid(info, 1);
    ossupport = info[2] & (1 << 27) || false;
    cpusupport = info[2] & (1 << 28) || false;
	if (ossupport && cpusupport) {
		unsigned long long feature_mask = _xgetbv(0);// _XCR_XFEATURE_ENABLED_MASK);
		support = (feature_mask & 0x6) == 0x6;
	}
#else
    cpusupport = __builtin_cpu_supports("avx");
    //__builtin_os
    support = cpusupport & ossupport;
#endif
	return support;

}