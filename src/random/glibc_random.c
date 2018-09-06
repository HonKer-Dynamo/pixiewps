/*
 * Based on the code of Peter Selinger
 * Reference: http://www.mathstat.dal.ca/~selinger/random/
 *
 * The original code was modified to achieve better speed
 *
 * Note that in the original code two signed integers are added together
 * which results in undefined behavior if the sum overflows the content
 * of a signed integer while trying to preserve the sign.
 *
 * To avoid this, we exploit the 2's complement, thus using only
 * unsigned integers. Note that INT_MAX + INT_MAX <= UINT_MAX and that
 * adding two unsigned integers which sum exceeds UINT_MAX is not
 * undefined behavior, it causes the value to wrap around.
 */

#include <stdint.h>

/*
 * The +1 is used to keep the index inside the array after the increment,
 * it doesn't really have a purpose besides that
 */
struct glibc_prng {
	int index;
	uint32_t state[344 + GLIBC_MAX_GEN + 1];
};

/*
 * There are no checks of bounds (GLIBC_MAX_GEN is the maximum number of times it can be called)
 */
static uint32_t glibc_rand(struct glibc_prng *prng)
{
	uint32_t *state = prng->state;
	const int i = prng->index++;
	state[i] = (uint32_t)(state[i - 31] + state[i - 3]);
	return state[i] >> 1;
}

static void glibc_seed(struct glibc_prng *prng, uint32_t seed)
{
	int i = 0;
	uint32_t *state = prng->state;
	prng->index = 344;
	state[i++] = seed;
	for ( ; i < 31; i++) {

		/* This does: state[i] = (16807LL * state[i - 1]) % 0x7fffffff
		   using the sum of digits method which works for mod N, base N+1 */
		const uint64_t p = 16807ULL * state[i - 1];
		const uint64_t m = (p >> 31) + (p & 0x7fffffff);

		/* The result might still not fit in 31 bits, if not, repeat
		   (conditional seems to make it slighlty faster) */
		state[i] = (m & 0xffffffff80000000) ? ((m >> 31) + (m & 0x7fffffff)) : m;
	}
	for (i = 31; i < 34;  i++) state[i] = state[i - 31];
	for (i = 34; i < 344; i++) state[i] = state[i - 31] + state[i - 3];
}
