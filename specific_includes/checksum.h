#if 0
#ifdef CONFIG_X86_32
# include <specific_includes/checksum_32.h>
#else
# include <specific_includes/checksum_64.h>
#endif
#else
#ifdef CONFIG_X86_32
# include <specific_includes/arch/x86/include/asm/checksum_32.h>
#else
# include <specific_includes/arch/x86/include/asm/checksum_64.h>
#endif
#endif
