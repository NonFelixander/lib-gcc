/* Routines required for instrumenting a program.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1989-2017 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#include "libgcov.h"

#if defined(inhibit_libc)

#ifdef L_gcov_flush
void __gcov_flush (void) {}
#endif

#ifdef L_gcov_reset
void __gcov_reset (void) {}
#endif

#ifdef L_gcov_dump
void __gcov_dump (void) {}
#endif

#else

/* Some functions we want to bind in this dynamic object, but have an
   overridable global alias.  Unfortunately not all targets support
   aliases, so we just have a forwarding function.  That'll be tail
   called, so the cost is a single jump instruction.*/

#define ALIAS_void_fn(src,dst) \
  void dst (void)	    \
  { src (); }

#ifdef L_gcov_reset

/* Reset all counters to zero.  */

static void
gcov_clear (const struct gcov_info *list)
{
  const struct gcov_info *gi_ptr;

  for (gi_ptr = list; gi_ptr; gi_ptr = gi_ptr->next)
    {
      unsigned f_ix;

      for (f_ix = 0; f_ix < gi_ptr->n_functions; f_ix++)
        {
          unsigned t_ix;
          const struct gcov_fn_info *gfi_ptr = gi_ptr->functions[f_ix];

          if (!gfi_ptr || gfi_ptr->key != gi_ptr)
            continue;
          const struct gcov_ctr_info *ci_ptr = gfi_ptr->ctrs;
          for (t_ix = 0; t_ix != GCOV_COUNTERS; t_ix++)
            {
              if (!gi_ptr->merge[t_ix])
                continue;

              memset (ci_ptr->values, 0, sizeof (gcov_type) * ci_ptr->num);
              ci_ptr++;
            }
        }
    }
}

/* Function that can be called from application to reset counters to zero,
   in order to collect profile in region of interest.  */

void
__gcov_reset_int (void)
{
  struct gcov_root *root;

  /* If we're compatible with the master, iterate over everything,
     otherise just do us.  */
  for (root = __gcov_master.version == GCOV_VERSION
	 ? __gcov_master.root : &__gcov_root; root; root = root->next)
    {
      gcov_clear (root->list);
      root->dumped = 0;
    }
}

ALIAS_void_fn (__gcov_reset_int, __gcov_reset);

#endif /* L_gcov_reset */

#ifdef L_gcov_dump
/* Function that can be called from application to write profile collected
   so far, in order to collect profile in region of interest.  */

void
__gcov_dump_int (void)
{
  struct gcov_root *root;

  /* If we're compatible with the master, iterate over everything,
     otherise just do us.  */
  for (root = __gcov_master.version == GCOV_VERSION
	 ? __gcov_master.root : &__gcov_root; root; root = root->next)
    __gcov_dump_one (root);
}

ALIAS_void_fn (__gcov_dump_int, __gcov_dump);

#endif /* L_gcov_dump */

#endif /* inhibit_libc */
