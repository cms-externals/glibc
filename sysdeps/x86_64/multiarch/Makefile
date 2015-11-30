ifeq ($(subdir),csu)
aux += init-arch
gen-as-const-headers += ifunc-defines.sym
endif

ifeq ($(subdir),string)
sysdep_routines += stpncpy-c strncpy-c strcmp-ssse3 strncmp-ssse3 \
		   strend-sse4 memcmp-sse4 \
		   strcasestr-nonascii strcasecmp_l-ssse3 \
		   strncase_l-ssse3 strlen-sse4 strlen-no-bsf \
		   memset-x86-64
ifeq (yes,$(config-cflags-sse4))
sysdep_routines += strcspn-c strpbrk-c strspn-c strstr-c strcasestr-c
CFLAGS-strcspn-c.c += -msse4
CFLAGS-strpbrk-c.c += -msse4
CFLAGS-strspn-c.c += -msse4
CFLAGS-strstr.c += -msse4
CFLAGS-strcasestr.c += -msse4
CFLAGS-strcasestr-nonascii.c += -msse4
endif
endif
