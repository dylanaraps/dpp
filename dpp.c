/*
 * Copyright (c) 2024-2026 Dylan Araps
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "str.h"

static inline const char *
xgetenv(const char *restrict e, const char *restrict f)
{
  char *r = getenv(e);
  return (r && *r) ? r : f;
}

#define die(...) do { fprintf(stderr, __VA_ARGS__); goto err; } while (0)

int
main(int argc, char *argv[])
{
  int x = EXIT_FAILURE;
  struct s s[1];

  if (!str_init(s, 8192))
    die("error: failed to allocate string: %s\n", strerror(errno));

  const char *sh = xgetenv("DPP_SHELL", DPP_SHELL);

  if (*sh != '/') 
    die("error: DPP_SHELL must be an absolute path.\n");

  STR_COPY(s, "#!");
  str_push_s(s, sh);
  STR_PUSH(s, "\n" DPP_HEADER);

  const char *in = xgetenv("DPP_INCLUDE", DPP_INCLUDE);

  if (*in) {
    STR_PUSH(s, ". ");
    str_push_s(s, in);
    str_push_c(s, '\n');
  }

  const char *bl = xgetenv("DPP_BLOCK", DPP_BLOCK);
  size_t bll = strlen(bl);

  if (!*bl)
    die("error: DPP_BLOCK cannot be empty.\n");

  size_t sta = 0;
  char d[4096];

  for (; fgets(d, sizeof(d), stdin);) {
    size_t o = 0;
    size_t e = strcspn(d, "\n");
    size_t n = d[e] == '\n';

    if (n) {
      if (e >= bll && memcmp(d, bl, bll) == 0) {
        o = bll + (d[bll] == ' ');

        if (sta == 1 && e > o)
          break;
        else if (sta == 2) {
          STR_PUSH(s, DPP_EOF "\n");
          sta = 0;
        }

        sta ^= e == o;

      } else if (!sta && e) {
        STR_PUSH(s, "${DPP_CAT:-\"" DPP_CAT "\"} <<" DPP_EOF "\n");
        sta = 2;
      }
    }

    str_push(s, &d[o], e + n - o);
  }

  if (ferror(stdin))
    die("error: failed to read stdin: %s\n", strerror(errno));

  switch (sta) {
    case 1:  die("error: DPP_BLOCK syntax error.\n");
    case 2:  STR_PUSH(s, DPP_EOF "\n\n"); /* fallthrough */
    default: STR_PUSH(s, "\0");
  }

  if (s->e)
    die("error: failed to grow string.\n");

#ifdef DPP_COMPILE
  (void) argc, (void) argv, (void) sh;

  if (fwrite(s->m, 1, s->l, stdout) < s->l)
    die("error: failed to write output: %s\n", strerror(errno));

  x = EXIT_SUCCESS;

#else
  char **c = calloc(argc + 4, sizeof(*c));

  if (!c)
    die("error: failed to allocate argc: %s\n", strerror(errno));

  memcpy(c, (const char *[]){ sh, DPP_SHELL_ARG, s->m }, sizeof(*c) * 3);
  memcpy(&c[3], argv, sizeof(*c) * argc);
  execv(sh, c);
  free(c);
  die("error: DPP_SHELL: '%s' failed: %s\n", sh, strerror(errno));
#endif

err:
  str_free(s);
  return x;
}

