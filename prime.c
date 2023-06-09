#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mini-gmp.h"

/* find x, y such that ax + by = gcd(a, b)
 */
int extended_euclidean(int a, int b, int *x, int *y)
{
	if (b == 0) {
		*x = 1;
		*y = 0;
		return a;
	}

	int x1, y1;
	int d = extended_euclidean(b, a % b, &x1, &y1);

	// ax  +    by   = d (before)
	*x = y1;
	*y = x1 - (a / b) * y1;
	// bx1 + (a%b)y1 = d (after)

	return d;
}


/* return a^(-1) mod m
 */
int inverse_m(int a, int m)
{
	int inv, y;
	int d = extended_euclidean(a, m, &inv, &y);
	if (inv < 0)
		inv += m;
	return inv;
}

int msb32(unsigned int n)
{
	int ret = 0;
	if (n & 0xffff0000) { ret += 16; n &= 0xffff0000; }
	if (n & 0xff00ff00) { ret +=  8; n &= 0xff00ff00; }
	if (n & 0xf0f0f0f0) { ret +=  4; n &= 0xf0f0f0f0; }
	if (n & 0xcccccccc) { ret +=  2; n &= 0xcccccccc; }
	if (n & 0xaaaaaaaa) { ret +=  1; }
	return ret;
}

unsigned int next_pow2(unsigned int n)
{
	n |= n>>1;
	n |= n>>2;
	n |= n>>4;
	n |= n>>8;
	n |= n>>16;
	return n + 1;
}

int montgomery_n1(unsigned int n)
// TODO should be pre-calculated
{
	unsigned int r = next_pow2(n);
	int n1 = abs(inverse_m(n, r) - r);
	return n1;
}

int montgomery_r1(unsigned int n)
// TODO should be pre-calculated
{
	unsigned int r = next_pow2(n);
	int r1 = inverse_m(r, n);
	return r1;
}

/* 计算 (a = (x<<r) * (y<<r)) mod n 并不含任何除法操作, 假设 d = 1 即 n 与 2^t 互素
 * https://www.ams.org/journals/mcom/1985-44-170/S0025-5718-1985-0777282-X/S0025-5718-1985-0777282-X.pdf
 */
int montgomery_reduce(int a, int r, int n, int n1)
{
	// 要求 m 使得 a + m * n 整除 r
	// 如果用 eulidean 算出 r * inv(r) - n * n1 = d
	// 并让 m = a * n1
	// 就有 a * d + m * n = a * (d + n * n1) = inv(r) * r
	int R = (1 << r) - 1;
	int m = ((a & R) * n1) & R;
	a = (a + m * n) >> r;
	return a > n ? a - n : a;
}

int montgomery(int x, int d, unsigned int n)
{
	int r = msb32(n) + 1;
	int s = (1 << r) % n;
	int i = (x << r) % n;

	// TODO: n1 r1 can be pre-calulated
	int n1 = montgomery_n1(n);
	int r1 = montgomery_r1(n);
	while(d) {
		if (d & 1)
			s = montgomery_reduce(s * i, r, n, n1);
		d >>= 1;
		i = montgomery_reduce(i * i, r, n, n1);
	}
	s = (s * r1) % n;
	return s;
}

/* return a^d mod n
 */
int powm(int a, int d, int n)
{
	int r = 1;
	while(d) {
		if (d & 1)
			r = (r * a) % n;
		d >>= 1;
		a = (a * a) % n;
	}
	return r;
}

/* if a^(p-1) != 1 mod n, then n is not a prime
 */
int witness(int a, mpz_t n)
{
	return 0;
} 

/* Miller-Rabin
For u32, a = 2, 7, 61
For u64, a = 2, 325, 9375, 28178, 450775, 9780504, 1795265022
Specifically, a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
*/

int main()
{
	int a, d, n;
	n = 43;
	a = 32; d = 56;
	printf("%d ^ %d mod %d = %d %d\n", a, d, n, montgomery(a, d, n), powm(a, d, n));
	a = 35; d = 43;
	printf("%d ^ %d mod %d = %d %d\n", a, d, n, montgomery(a, d, n), powm(a, d, n));
	a = 23; d = 53;
	printf("%d ^ %d mod %d = %d %d\n", a, d, n, montgomery(a, d, n), powm(a, d, n));
}
