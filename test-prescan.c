/* Test for prescan.c.
 *
 * Copyright (c) 2015 Zack Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "prescan.h"
#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static unsigned int successes = 0;
static unsigned int failures  = 0;
static bool verbose           = false;

static void
run_one_test(const char *data, size_t dlen,
             const char *exp_encoding,
             const char *fname,
             unsigned int tlineno)
{
  const char *got_encoding =
    prescan_a_byte_stream_to_determine_its_encoding(data, dlen);

  if ((exp_encoding == NULL && got_encoding == NULL) ||
      (exp_encoding != NULL && got_encoding != NULL &&
       !strcmp(got_encoding, exp_encoding))) {
    if (verbose)
      printf("%s:%u: pass\n", fname, tlineno);
    successes++;
  } else {
    if (exp_encoding == NULL) exp_encoding = "(null)";
    if (got_encoding == NULL) got_encoding = "(null)";
    printf("%s:%u: fail: expected %s got %s\n",
           fname, tlineno, exp_encoding, got_encoding);
    failures++;
  }
}

static void
run_one_file(const char *fname, FILE *fp)
{
  char dbuf[1025], *d;
  char ebuf[81], *e;
  int c;
  unsigned int lineno;

  enum state {
    HASH,
    dD1, dA1, dT1, dA2,
    eE1, eN1, eC1, eO1, eD1, eI1, eN2, eG1,
    DATA, ENCODING, IGNORE,
    BOL_DATA, BOL_IGNORE
  } s;

  s = BOL_IGNORE;
  d = dbuf;
  e = ebuf;
  lineno = 1;
  for (;;) {
    c = getc(fp);
    if (c == EOF) break;
    if (c == '\n') lineno++;
    switch (s) {
    case BOL_IGNORE:
      if (c == '#') {
        s = HASH;
        break;
      }
    case IGNORE:
      s = (c == '\n') ? BOL_IGNORE : IGNORE;
      break;

    case BOL_DATA:
      if (c == '#') {
        s = HASH;
        break;
      }
    case DATA:
      if (d == &dbuf[1024]) {
        s = (c == '\n') ? BOL_IGNORE : IGNORE;
        break;
      }
      *d++ = c;
      s = (c == '\n') ? BOL_DATA : DATA;
      break;

    case ENCODING:
      if (e == &ebuf[80] || c == '\n' || (c == '#' && e == ebuf)) {
        *d = '\0';
        *e = '\0';
        if (ebuf[0] == '\0' || !strcmp(ebuf, "none"))
          run_one_test(dbuf, d - dbuf, 0, fname, lineno - 1);
        else
          run_one_test(dbuf, d - dbuf, ebuf, fname, lineno - 1);

        if (e == &ebuf[80])
          s = IGNORE;
        else if (c == '\n')
          s = BOL_IGNORE;
        else
          s = HASH;

        d = dbuf;
        e = ebuf;
        break;
      }
      *e++ = tolower(c);
      break;

    case HASH:
           if (c == 'd') s = dD1;
      else if (c == 'e') s = eE1;
      else               s = IGNORE;
      break;

    case dD1: s = (c ==  'a') ? dA1      : IGNORE; break;
    case dA1: s = (c ==  't') ? dT1      : IGNORE; break;
    case dT1: s = (c ==  'a') ? dA2      : IGNORE; break;
    case dA2: s = (c == '\n') ? BOL_DATA : IGNORE; break;
    case eE1: s = (c ==  'n') ? eN1      : IGNORE; break;
    case eN1: s = (c ==  'c') ? eC1      : IGNORE; break;
    case eC1: s = (c ==  'o') ? eO1      : IGNORE; break;
    case eO1: s = (c ==  'd') ? eD1      : IGNORE; break;
    case eD1: s = (c ==  'i') ? eI1      : IGNORE; break;
    case eI1: s = (c ==  'n') ? eN2      : IGNORE; break;
    case eN2: s = (c ==  'g') ? eG1      : IGNORE; break;
    case eG1: s = (c == '\n') ? ENCODING : IGNORE; break;
    }
  }

  if (d != dbuf || e != ebuf) {
    printf("%s:%u: unexpected end of file\n", fname, lineno);
    failures++;
  }
}

int
main(int ac, char **av)
{
  int i;
  FILE *fp;
  bool oserror;

  setlocale(LC_ALL, "C");

  i = 1;
  if (ac > 1 && (!strcmp(av[1], "-v") ||
                 !strcmp(av[1], "--verbose"))) {
    i++;
    verbose = true;
  }
  if (i >= ac) {
    fprintf(stderr, "usage: %s [-v] testdata...\n", av[0]);
    return 2;
  }

  setlocale(LC_ALL, "C");

  oserror = false;
  for (; i < ac; i++) {
    fp = fopen(av[i], "rb");
    if (!fp) {
      perror(av[i]);
      oserror = true;
      continue;
    }

    run_one_file(av[i], fp);

    if (ferror(fp) || fclose(fp)) {
      perror(av[i]);
      oserror = true;
    }
  }

  if (oserror)
    return 1;

  printf("%u tests succeeded", successes);
  if (failures)
    printf(", %u failed", failures);
  fputs(".\n", stdout);

  if (failures || !successes)
    return 1;
  return 0;
}
