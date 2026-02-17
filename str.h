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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct s {
  char *m;
  size_t l;
  size_t c;
  size_t e;
};

static inline size_t
str_init(struct s *s, size_t c)
{
  s->m = NULL;
  s->e = 1;

  if (c) {
    s->c = c;
    s->l = 0;
    s->m = malloc(c);
    s->e = !s->m;
  }

  return !s->e;
}

static inline size_t
str_grow(struct s *s, size_t l)
{
  if (s->e || l > SIZE_MAX - s->l)
    goto e;
  size_t need = s->l + l;
  size_t cap = s->c;

  while (cap < need) {
    if (cap > SIZE_MAX / 2)
      goto e;
    cap <<= 1;
  }

  if (cap != s->c) {
    char *m = realloc(s->m, cap);
    if (!m) goto e;
    s->m = m;
    s->c = cap;
  }

  return 1;
e:
  s->e = 1;
  return 0;
}

static inline void
str_copy(struct s *s, const char *p, size_t l)
{
  memcpy(&s->m[s->l], p, l);
  s->l += l;
}

static inline void
str_push(struct s *s, const char *p, size_t l)
{
  if (str_grow(s, l))
    str_copy(s, p, l);
}

static inline void
str_push_c(struct s *s, int c)
{
  if (str_grow(s, 1))
    s->m[s->l++] = c;
}

static inline void
str_push_s(struct s *s, const char *p)
{
  str_push(s, p, strlen(p));
}

static inline void
str_free(struct s *s)
{
  free(s->m);
}

#define STR_PUSH(s, p) str_push((s), (p), sizeof(p) - 1)
#define STR_COPY(s, p) str_copy((s), (p), sizeof(p) - 1)

