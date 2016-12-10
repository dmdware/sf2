



/*
 * Copyright (c) 2007 Alexey Vatchenko <av@bsdua.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "utf8.h"

static int retval = 0;

static void	test_utf82wchar(const char *src, int slen,
		    const unsigned int *dst, int dlen, int flags, int res,
		    const char *descr);
static void	test_wchar2utf8(const unsigned int *src, int slen,
		    const char *dst, int dlen, int flags, int res,
		    const char *descr);

static void
test_utf82wchar(const char *src, int slen, const unsigned int *dst, int dlen,
    int flags, int res, const char *descr)
{
	int size;
	unsigned int *mem;

	mem = NULL;
	if (dst != NULL) {
		mem = (unsigned int *)malloc(dlen * sizeof(*mem));
		if (mem == NULL) {
			fprintf(stderr, "u2w: %s: MALLOC FAILED\n", descr);
			return;
		}
	}

	do {
		size = utf8_to_wchar(src, slen, mem, dlen, flags);
		if (res != size) {
			retval = 1;
			fprintf(stderr,
			    "u2w: %s: FAILED (rv: %u, must be %u)\n",
			    descr, size, res);
			break;
		}

		if (mem == NULL)
			break;		/* OK */

		if (memcmp(mem, dst, size * sizeof(*mem)) != 0) {
			retval = 1;
			fprintf(stderr, "u2w: %s: BROKEN\n", descr);
			break;
		}

	} while (0);

	if (mem != NULL);
		free(mem);
}

static void
test_wchar2utf8(const unsigned int *src, int slen, const char *dst, int dlen,
    int flags, int res, const char *descr)
{
	int size;
	char *mem;

	mem = NULL;
	if (dst != NULL) {
		mem = (char *)malloc(dlen);
		if (mem == NULL) {
			fprintf(stderr, "w2u: %s: MALLOC FAILED\n", descr);
			return;
		}
	}

	do {
		size = wchar_to_utf8(src, slen, mem, dlen, flags);
		if (res != size) {
			retval = 1;
			fprintf(stderr,
			    "w2u: %s: FAILED (rv: %u, must be %u)\n",
			    descr, size, res);
			break;
		}

		if (mem == NULL)
			break;		/* OK */

		if (memcmp(mem, dst, size) != 0) {
			retval = 1;
			fprintf(stderr, "w2u: %s: BROKEN\n", descr);
			break;
		}

	} while (0);

	if (mem != NULL);
		free(mem);
}

#if 0
int
main(void)
{
	unsigned int w1[] = {0x54, 0x65, 0x73, 0x74};
	unsigned int w2[] = {0x0422, 0x0435, 0x0441, 0x0442};
	unsigned int w3[] = {0x800, 0x1e80, 0x98c4, 0x9910, 0xff00};
	unsigned int w4[] = {0x15555, 0xf7777, 0xa};
	unsigned int w5[] = {0x255555, 0x1fa04ff, 0xddfd04, 0xa};
	unsigned int w6[] = {0xf255555, 0x1dfa04ff, 0x7fddfd04, 0xa};
	unsigned int wb[] = {-2, 0xa, 0xffffffff, 0x0441};
	unsigned int wm[] = {0x41, 0x0441, 0x3042, 0xff67, 0x9b0d, 0x2e05da67};
	unsigned int wb1[] = {0xa, 0x0422};
	unsigned int wb2[] = {0xd800, 0xda00, 0x41, 0xdfff, 0xa};
	unsigned int wbom[] = {0xfeff, 0x41, 0xa};
	unsigned int wbom2[] = {0x41, 0xa};
	unsigned int wbom22[] = {0xfeff, 0x41, 0xa};
	unsigned char u1[] = {0x54, 0x65, 0x73, 0x74};
	unsigned char u2[] = {0xd0, 0xa2, 0xd0, 0xb5, 0xd1, 0x81, 0xd1, 0x82};
	unsigned char u3[] = {0xe0, 0xa0, 0x80, 0xe1, 0xba, 0x80, 0xe9, 0xa3, 0x84,
	    0xe9, 0xa4, 0x90, 0xef, 0xbc, 0x80};
	unsigned char u4[] = {0xf0, 0x95, 0x95, 0x95, 0xf3, 0xb7, 0x9d, 0xb7, 0xa};
	unsigned char u5[] = {0xf8, 0x89, 0x95, 0x95, 0x95, 0xf9, 0xbe, 0xa0, 0x93,
	    0xbf, 0xf8, 0xb7, 0x9f, 0xb4, 0x84, 0x0a};
	unsigned char u6[] = {0xfc, 0x8f, 0x89, 0x95, 0x95, 0x95, 0xfc, 0x9d, 0xbe,
	    0xa0, 0x93, 0xbf, 0xfd, 0xbf, 0xb7, 0x9f, 0xb4, 0x84, 0x0a};
	unsigned char ub[] = {0xa, 0xd1, 0x81};
	unsigned char um[] = {0x41, 0xd1, 0x81, 0xe3, 0x81, 0x82, 0xef, 0xbd, 0xa7,
	    0xe9, 0xac, 0x8d, 0xfc, 0xae, 0x81, 0x9d, 0xa9, 0xa7};
	unsigned char ub1[] = {0xa, 0xff, 0xd0, 0xa2, 0xfe, 0x8f, 0xe0, 0x80};
	unsigned char uc080[] = {0xc0, 0x80};
	unsigned char ub2[] = {0xed, 0xa1, 0x8c, 0xed, 0xbe, 0xb4, 0xa};
	unsigned char ubom[] = {0x41, 0xa};
	unsigned char ubom2[] = {0xef, 0xbb, 0xbf, 0x41, 0xa};

	/*
	 * UTF-8 -> UCS-4 string.
	 */
	test_utf82wchar(ubom2, sizeof(ubom2), wbom2,
	    sizeof(wbom2) / sizeof(*wbom2), UTF8_SKIP_BOM,
	    sizeof(wbom2) / sizeof(*wbom2), "skip BOM");
	test_utf82wchar(ubom2, sizeof(ubom2), wbom22,
	    sizeof(wbom22) / sizeof(*wbom22), 0,
	    sizeof(wbom22) / sizeof(*wbom22), "BOM");
	test_utf82wchar(uc080, sizeof(uc080), NULL, 0, 0, 0,
	    "c0 80 - forbitten by rfc3629");
	test_utf82wchar(ub2, sizeof(ub2), NULL, 0, 0, 3,
	    "resulted in forbitten wchars (len)");
	test_utf82wchar(ub2, sizeof(ub2), wb2, sizeof(wb2) / sizeof(*wb2), 0, 0,
	    "resulted in forbitten wchars");
	test_utf82wchar(ub2, sizeof(ub2), L"\x0a", 1, UTF8_IGNORE_ERROR,
	    1, "resulted in ignored forbitten wchars");
	test_utf82wchar(u1, sizeof(u1), w1, sizeof(w1) / sizeof(*w1), 0,
	    sizeof(w1) / sizeof(*w1), "1 octet chars");
	test_utf82wchar(u2, sizeof(u2), w2, sizeof(w2) / sizeof(*w2), 0,
	    sizeof(w2) / sizeof(*w2), "2 octets chars");
	test_utf82wchar(u3, sizeof(u3), w3, sizeof(w3) / sizeof(*w3), 0,
	    sizeof(w3) / sizeof(*w3), "3 octets chars");
	test_utf82wchar(u4, sizeof(u4), w4, sizeof(w4) / sizeof(*w4), 0,
	    sizeof(w4) / sizeof(*w4), "4 octets chars");
	test_utf82wchar(u5, sizeof(u5), w5, sizeof(w5) / sizeof(*w5), 0,
	    sizeof(w5) / sizeof(*w5), "5 octets chars");
	test_utf82wchar(u6, sizeof(u6), w6, sizeof(w6) / sizeof(*w6), 0,
	    sizeof(w6) / sizeof(*w6), "6 octets chars");
	test_utf82wchar("\xff", 1, NULL, 0, 0, 0, "broken utf-8 0xff symbol");
	test_utf82wchar("\xfe", 1, NULL, 0, 0, 0, "broken utf-8 0xfe symbol");
	test_utf82wchar("\x8f", 1, NULL, 0, 0, 0,
	    "broken utf-8, start from 10 higher bits");
	test_utf82wchar(ub1, sizeof(ub1), wb1, sizeof(wb1) / sizeof(*wb1),
	    UTF8_IGNORE_ERROR, sizeof(wb1) / sizeof(*wb1), "ignore bad chars");
	test_utf82wchar(um, sizeof(um), wm, sizeof(wm) / sizeof(*wm), 0,
	    sizeof(wm) / sizeof(*wm), "mixed languages");
	test_utf82wchar(um, sizeof(um), wm, sizeof(wm) / sizeof(*wm) - 1, 0,
	    0, "boundaries -1");
	test_utf82wchar(um, sizeof(um), wm, sizeof(wm) / sizeof(*wm) + 1, 0,
	    sizeof(wm) / sizeof(*wm), "boundaries +1");
	test_utf82wchar(um, sizeof(um), NULL, 0, 0,
	    sizeof(wm) / sizeof(*wm), "calculate length");
	test_utf82wchar(ub1, sizeof(ub1), NULL, 0, 0,
	    0, "calculate length of bad chars");
	test_utf82wchar(ub1, sizeof(ub1), NULL, 0,
	    UTF8_IGNORE_ERROR, sizeof(wb1) / sizeof(*wb1),
	    "calculate length, ignore bad chars");
	test_utf82wchar(NULL, 0, NULL, 0, 0, 0, "invalid params, all 0");
	test_utf82wchar(u1, 0, NULL, 0, 0, 0,
	    "invalid params, src buf not NULL");
	test_utf82wchar(NULL, 10, NULL, 0, 0, 0,
	    "invalid params, src length is not 0");
	test_utf82wchar(u1, sizeof(u1), w1, 0, 0, 0,
	    "invalid params, dst is not NULL");

	/*
	 * UCS-4 -> UTF-8 string.
	 */
	test_wchar2utf8(wbom, sizeof(wbom) / sizeof(*wbom), ubom, sizeof(ubom),
	    UTF8_SKIP_BOM, sizeof(ubom), "BOM");
	test_wchar2utf8(wb2, sizeof(wb2) / sizeof(*wb2), NULL, 0, 0,
	    0, "prohibited wchars");
	test_wchar2utf8(wb2, sizeof(wb2) / sizeof(*wb2), NULL, 0,
	    UTF8_IGNORE_ERROR, 2, "ignore prohibited wchars");
	test_wchar2utf8(w1, sizeof(w1) / sizeof(*w1), u1, sizeof(u1), 0,
	    sizeof(u1), "1 octet chars");
	test_wchar2utf8(w2, sizeof(w2) / sizeof(*w2), u2, sizeof(u2), 0,
	    sizeof(u2), "2 octets chars");
	test_wchar2utf8(w3, sizeof(w3) / sizeof(*w3), u3, sizeof(u3), 0,
	    sizeof(u3), "3 octets chars");
	test_wchar2utf8(w4, sizeof(w4) / sizeof(*w4), u4, sizeof(u4), 0,
	    sizeof(u4), "4 octets chars");
	test_wchar2utf8(w5, sizeof(w5) / sizeof(*w5), u5, sizeof(u5), 0,
	    sizeof(u5), "5 octets chars");
	test_wchar2utf8(w6, sizeof(w6) / sizeof(*w6), u6, sizeof(u6), 0,
	    sizeof(u6), "6 octets chars");
	test_wchar2utf8(wb, sizeof(wb) / sizeof(*wb), ub, sizeof(ub), 0,
	    0, "bad chars");
	test_wchar2utf8(wb, sizeof(wb) / sizeof(*wb), ub, sizeof(ub),
	    UTF8_IGNORE_ERROR, sizeof(ub), "ignore bad chars");
	test_wchar2utf8(wm, sizeof(wm) / sizeof(*wm), um, sizeof(um), 0,
	    sizeof(um), "mixed languages");
	test_wchar2utf8(wm, sizeof(wm) / sizeof(*wm), um, sizeof(um) - 1, 0,
	    0, "boundaries -1");
	test_wchar2utf8(wm, sizeof(wm) / sizeof(*wm), um, sizeof(um) + 1, 0,
	    sizeof(um), "boundaries +1");
	test_wchar2utf8(wm, sizeof(wm) / sizeof(*wm), NULL, 0, 0,
	    sizeof(um), "calculate length");
	test_wchar2utf8(wb, sizeof(wb) / sizeof(*wb), NULL, 0, 0,
	    0, "calculate length of bad chars");
	test_wchar2utf8(wb, sizeof(wb) / sizeof(*wb), NULL, 0,
	    UTF8_IGNORE_ERROR, sizeof(ub),
	    "calculate length, ignore bad chars");
	test_wchar2utf8(NULL, 0, NULL, 0, 0, 0, "invalid params, all 0");
	test_wchar2utf8(w1, 0, NULL, 0, 0, 0,
	    "invalid params, src buf not NULL");
	test_wchar2utf8(NULL, 10, NULL, 0, 0, 0,
	    "invalid params, src length is not 0");
	test_wchar2utf8(w1, sizeof(w1) / sizeof(*w1), u1, 0, 0, 0,
	    "invalid params, dst is not NULL");

	return (retval);
}
#endif