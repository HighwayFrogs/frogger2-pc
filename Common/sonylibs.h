#include <stddef.h>

#include <libgte.h>
#include <libsn.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include <libapi.h>

#include <inline_c.h>
#include <gtemac.h>

#define gte_SetLDDQA( r0 ) __asm__ volatile (		\
	"ctc2	%0, $27"					\
	:							\
	: "r"( r0 ) )

#define gte_SetLDDQB( r0 ) __asm__ volatile (		\
	"ctc2	%0, $28"					\
	:							\
	: "r"( r0 ) )
