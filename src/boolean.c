/*                                      -*- coding: utf-8 -*-
 *
 * b o o l e a n . c                    -- Booleans and Equivalence predicates
 *
 * Copyright © 1993-2023 Erick Gallesio <eg@stklos.net>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 *           Author: Erick Gallesio [eg@kaolin.unice.fr]
 *    Creation date: 23-Oct-1993 21:37
 */
#include <sys/resource.h>
#include "stklos.h"
#include "object.h"
#include "struct.h"


/* Define the maximum calls for equal-count (a version of equal bounded in
  recursive calls). The value depends on the way the program is compiled:
  if optimizations are used, the program grows stack faster
*/

#define DEFAULT_MAX_EQUAL_CALLS  50000 /* max recursive calls  */
#define STK_DIVISOR_OPTIM          100 /* divisisor if compiled with -O option */
#define STK_DIVISOR_NOT_OPTIM      200 /* divisisor if compiled without -O option */

#ifdef __OPTIMIZE__
   static int optimized = 1;
#else
   static int optimized = 0;
#endif

static int max_equal_calls = DEFAULT_MAX_EQUAL_CALLS;

static void limit_max_equal_calls(void) {
  struct rlimit rl;

  if (getrlimit(RLIMIT_STACK, &rl) == 0) {
    /* Determine a value for the maximum calls for equal-count depending on stack size*/
    max_equal_calls = rl.rlim_cur / (optimized? STK_DIVISOR_OPTIM: STK_DIVISOR_NOT_OPTIM);
  }
}


DEFINE_PRIMITIVE("not", not, subr1, (SCM x))
/*
<doc not
 * (not obj)
 *
 * Not returns |#t| if |obj| is false, and returns |#f| otherwise.
 *
 * @lisp
 *   (not #t)         =>  #f
 *   (not 3)          =>  #f
 *   (not (list 3))   =>  #f
 *   (not #f)         =>  #t
 *   (not '())        =>  #f
 *   (not (list))     =>  #f
 *   (not 'nil)       =>  #f
 * @end lisp
doc>
 */
{
  return MAKE_BOOLEAN(x==STk_false);
}

DEFINE_PRIMITIVE("boolean?", booleanp, subr1, (SCM x))
/*
<doc  boolean?
 * (boolean? obj)
 *
 * |Boolean?| returns |#t| if |obj| is either |#t| or |#f| and returns
 * |#f| otherwise.
 * @lisp
 *   (boolean? #f)         =>  #t
 *   (boolean? 0)          =>  #f
 *   (boolean? '())        =>  #f
 * @end lisp
doc>
 */
{
  return MAKE_BOOLEAN(BOOLEANP(x));
}



/*
<doc  eqv?
 * (eqv? obj1 obj2)
 *
 * The |eqv?| procedure defines a useful equivalence relation on objects.
 * Briefly, it returns |#t| if |obj1| and |obj2| should normally be regarded
 * as the same object. This relation is left slightly open to interpretation,
 * but the following partial specification of |eqv?| holds for all
 * implementations of Scheme.
 *
 * The |eqv?| procedure returns |#t| if:
 *
 * - |obj1| and |obj2| are both |#t| or both |#f|.
 *
 * - |obj1| and |obj2| are both symbols and
 * +
 * ```scheme
 * (string=? (symbol->string obj1)
 *           (symbol->string obj2))     =>  #t
 * ```
 * +
 * NOTE: This assumes that neither |obj1| nor |obj2| is an
 * "uninterned symbol".
 *
 * - |obj1| and |obj2| are both keywords and
 * +
 * ```scheme
 * (string=? (keyword->string obj1)
 *           (keyword->string obj2))    =>  #t
 * ```
 *
 * - |obj1| and |obj2| are both numbers, are _<<numeq,numerically equal>>_,
 *  and are either both exact or both inexact.
 *
 * - |obj1| and |obj2| are both characters and are the same character
 *   according to the _<<chareq, `char=?` procedure>>`_.
 *
 * -  both |obj1| and |obj2| are the empty list.
 *
 * - |obj1| and |obj2| are pairs, vectors, or strings that denote
 *   the same locations in the store.
 *
 * - |obj1| and |obj2| are procedures whose location tags are equal.
 *
 * STklos extends R5RS |eqv?| to take into account
 * the keyword type. Here are some examples:
 * @lisp
 * (eqv? 'a 'a)                     =>  #t
 * (eqv? 'a 'b)                     =>  #f
 * (eqv? 2 2)                       =>  #t
 * (eqv? :foo :foo)                 =>  #t
 * (eqv? #:foo :foo)                =>  #t
 * (eqv? :foo :bar)                 =>  #f
 * (eqv? '() '())                   =>  #t
 * (eqv? 100000000 100000000)       =>  #t
 * (eqv? (cons 1 2) (cons 1 2))     =>  #f
 * (eqv? (lambda () 1)
 *       (lambda () 2))             =>  #f
 * (eqv? #f 'nil)                   =>  #f
 * (let ((p (lambda (x) x)))
 *   (eqv? p p))                    =>  #t
 * @end lisp
 *
 * The following examples illustrate cases in which the above rules do
 * not fully specify the behavior of |eqv?|. All that can be said about
 * such cases is that the value returned by eqv? must be a boolean.
 * @lisp
 * (eqv? "" "")             =>  unspecified
 * (eqv? '#() '#())         =>  unspecified
 * (eqv? (lambda (x) x)
 *       (lambda (x) x))    =>  unspecified
 * (eqv? (lambda (x) x)
 *       (lambda (y) y))    =>  unspecified
 * @end lisp
 *
 * NOTE: In fact, the value returned by STklos depends on
 * the way code is entered and can yield |#t| in some cases and |#f|
 * in others.
 *
 * See R5RS for more details on |eqv?|.
doc>
 */
DEFINE_PRIMITIVE("eqv?", eqv, subr2, (SCM x, SCM y))
{
  if (x == y) return STk_true;

  switch (STYPE(x)) {
    case tc_symbol:
        if (SYMBOLP(y) && strcmp(SYMBOL_PNAME(x), SYMBOL_PNAME(y)) == 0)
          return STk_true;
        break;

    case tc_real:
    case tc_bignum:
    case tc_complex:
    case tc_rational:
      if (NUMBERP(y)) {
        if (EXACTP(x) != EXACTP(y))
          return STk_false;
        return MAKE_BOOLEAN(STk_numeq2(x, y));
      }
      break;
    case tc_instance:
      if (STk_oo_initialized) {
        SCM fg, res;

        fg = STk_lookup(STk_intern("object-eqv?"), STk_current_module(),
                        &res, FALSE);
        res = STk_C_apply(fg, 2, x, y);
        return res;
      }
      break;
    case tc_pointer:
      if (CPOINTERP(y) && (CPOINTER_VALUE(x) == CPOINTER_VALUE(y)))
        return STk_true;
      break;

    // The default case could handle those labels. They are just here to
    // avoid the complex test in the default case when we are sure
    // to return #f
    case tc_not_boxed:
    case tc_cons:         case tc_integer:     case tc_keyword:
    case tc_string:       case tc_module:      case tc_closure:
    case tc_subr0:        case tc_subr1:       case tc_subr2:
    case tc_subr3:        case tc_subr4:       case tc_subr5:
    case tc_subr01:       case tc_subr12:      case tc_subr23:
    case tc_subr34:       case tc_vsubr:       case tc_apply:
    case tc_vector:       case tc_uvector:     case tc_hash_table:
    case tc_port:         case tc_frame:       case tc_next_method:
    case tc_promise:      case tc_regexp:      case tc_process:
    case tc_continuation: case tc_values:      case tc_parameter:
    case tc_socket:       case tc_struct_type: case tc_struct:
    case tc_thread:       case tc_mutex:       case tc_condv:
    case tc_box:          case tc_ext_func:    case tc_callback:
    case tc_syntax:
      return STk_false;

    default:
     if ((HAS_USER_TYPEP(x) && HAS_USER_TYPEP(y)) &&
         (BOXED_TYPE(x) == BOXED_TYPE(y)))
       return STk_extended_eqv(x, y);
  }
  /* What can we do else? */
  return STk_false;
}



/*
<doc eq?
 * (eq? obj1 obj2)
 *
 * |Eq?| is similar to |eqv?| except that in some cases it is capable of
 * discerning distinctions finer than those detectable by |eqv?|.
 *
 * |Eq?| and |eqv?| are guaranteed to have the same behavior on symbols,
 * keywords, booleans, the empty list, pairs, procedures, and non-empty strings
 * and vectors. `|Eq?|`'s behavior on numbers and characters is
 * implementation-dependent, but it will always return either true or false,
 * and will return true only when |eqv?| would also return true.
 * |Eq?| may also behave differently from |eqv?| on empty vectors
 * and empty strings. +
 * Note that:
 *
 *   - STklos extends R5RS |eq?| to take into account  the keyword type.
 *   - In STklos, comparison of character returns |#t| for identical
 *     characters and |#f| otherwise.
 *
 *
 * @lisp
 * (eq? 'a 'a)                     =>  #t
 * (eq? '(a) '(a))                 =>  unspecified
 * (eq? (list 'a) (list 'a))       =>  #f
 * (eq? "a" "a")                   =>  unspecified
 * (eq? "" "")                     =>  unspecified
 * (eq? :foo :foo)                 =>  #t
 * (eq? :foo :bar)                 =>  #f
 * (eq? '() '())                   =>  #t
 * (eq? 2 2)                       =>  unspecified
 * (eq? #A #A)                     =>  #t (unspecified in r5rs)
 * (eq? car car)                   =>  #t
 * (let ((n (+ 2 3)))
 *   (eq? n n))                    =>  #t (unspecified in r5rs)
 * (let ((x '(a)))
 *   (eq? x x))                    =>  #t
 * (let ((x '#()))
 *   (eq? x x))                    =>  #t
 * (let ((p (lambda (x) x)))
 *   (eq? p p))                    =>  #t
 * (eq? :foo :foo)                 =>  #t
 * (eq? :bar bar:)                 =>  #t
 * (eq? :bar :foo)                 =>  #f
 * @end lisp
 *
doc>
 */
DEFINE_PRIMITIVE("eq?", eq, subr2, (SCM x,SCM y))
{
  return MAKE_BOOLEAN(x == y);
}


/*
<doc  equal?
 * (equal? obj1 obj2)
 *
 * |Equal?| recursively compares the contents of pairs, vectors, and
 * strings, applying |eqv?| on other objects such as numbers and symbols.
 * A rule of thumb is that objects are generally |equal?| if they print the
 * same. |Equal?| always terminates even if its arguments are circular
 * data structures.
 * @lisp
 * (equal? 'a 'a)                  =>  #t
 * (equal? '(a) '(a))              =>  #t
 * (equal? '(a (b) c)
 *         '(a (b) c))             =>  #t
 * (equal? "abc" "abc")            =>  #t
 * (equal? 2 2)                    =>  #t
 * (equal? (make-vector 5 'a)
 *         (make-vector 5 'a))     =>  #t
 * (equal? '#1=(a b . #1#)
 *         '#2=(a b a b . #2#))    =>  #t
 * @end lisp
 *
 * NOTE: A rule of thumb is that objects are generally
 * |equal?| if they print the same.
doc>
 */
DEFINE_PRIMITIVE("equal?", equal, subr2, (SCM x, SCM y))
{
 Top:
  if (STk_eqv(x, y) == STk_true) return STk_true;

  switch (STYPE(x)) {
    case tc_cons:
      if (CONSP(y)) {
        if (STk_equal(CAR(x), CAR(y)) == STk_false) return STk_false;
        x = CDR(x); y = CDR(y);
        goto Top;
      }
      break;
    case tc_string:
      if (STRINGP(y)) {
        return STk_streq(x, y);
      }
      break;
    case tc_vector:
      if (VECTORP(y)) {
        long lx, ly, i;
        SCM *vx, *vy;

        lx = VECTOR_SIZE(x); ly = VECTOR_SIZE(y);
        if (lx == ly) {
          vx = VECTOR_DATA(x);
          vy = VECTOR_DATA(y);
          for (i=0; i < lx;  i++) {
            if (STk_equal(vx[i], vy[i]) == STk_false) return STk_false;
          }
          return STk_true;
        }
      }
      break;
    case tc_instance:
      if (STk_oo_initialized) {
        SCM fg, res;

        fg = STk_lookup(STk_intern("object-equal?"),STk_current_module(),
                        &res,FALSE);
        res = STk_C_apply(fg, 2, x, y);
        return res;
      }
      break;
    case tc_struct:
      if (STRUCTP(y) && (STRUCT_TYPE(x) == STRUCT_TYPE(y)))
        return STk_equal(STk_struct2list(x), STk_struct2list(y));
      break;
    case tc_box:
      if (BOXP(y)) {
        long lx, ly, i;
        lx = BOX_ARITY(x); ly = BOX_ARITY(y);
        if (lx == ly) {
          SCM *vx = BOX_VALUES(x);
          SCM *vy = BOX_VALUES(y);
          for (i=0; i < lx;  i++) {
            if (STk_equal(vx[i], vy[i]) == STk_false) return STk_false;
          }
          return STk_true;
        }
      }
      break;
    case tc_uvector:
      if (BOXED_TYPE_EQ(y, tc_uvector))
        return MAKE_BOOLEAN(STk_uvector_equal(x, y));
      break;

    // The default case could handle those labels. They are just here to
    // avoid the complex test in the default case when we are sure
    // to return #f
    case tc_not_boxed:
    case tc_integer:      case tc_real:         case tc_bignum:
    case tc_rational:     case tc_complex:      case tc_symbol:
    case tc_keyword:      case tc_module:       case tc_closure:
    case tc_subr0:        case tc_subr1:        case tc_subr2:
    case tc_subr3:        case tc_subr4:        case tc_subr5:
    case tc_subr01:       case tc_subr12:       case tc_subr23:
    case tc_subr34:       case tc_vsubr:        case tc_apply:
    case tc_hash_table:   case tc_frame:        case tc_next_method:
    case tc_promise:      case tc_regexp:       case tc_process:
    case tc_continuation: case tc_values:       case tc_parameter:
    case tc_socket:       case tc_struct_type:  case tc_thread:
    case tc_mutex:        case tc_condv:        case tc_ext_func:
    case tc_pointer:      case tc_callback:     case tc_syntax:
      return STk_false;

  default:
      if ((HAS_USER_TYPEP(x) && HAS_USER_TYPEP(y)) &&
         (BOXED_TYPE(x) == BOXED_TYPE(y)))
       return STk_extended_equal(x, y);
  }
  return STk_false;
}

/*
 * The equal-count function is a variant of equal which is bounded in
 * recursion calls. This function returns a boolean (AND a boolean
 * which tells the caller if a cycle was detected)
 */
static SCM equal_count(SCM x, SCM y, int max, int *cycle)
{
 Top:
  if (STk_eqv(x, y) == STk_true) return STk_true;

  if (!max--) { *cycle = 1; return STk_false; }

  switch (STYPE(x)) {
    case tc_cons:
      if (CONSP(y)) {
        if (equal_count(CAR(x), CAR(y), max, cycle) == STk_false) return STk_false;
        x = CDR(x); y = CDR(y);
        goto Top;
      }
      break;
    case tc_string:
      if (STRINGP(y)) {
        return STk_streq(x, y);
      }
      break;
    case tc_vector:
      if (VECTORP(y)) {
        long lx, ly, i;
        SCM *vx, *vy;

        lx = VECTOR_SIZE(x); ly = VECTOR_SIZE(y);
        if (lx == ly) {
          vx = VECTOR_DATA(x);
          vy = VECTOR_DATA(y);
          for (i=0; i < lx;  i++) {
            if (equal_count(vx[i], vy[i], max, cycle) == STk_false) return STk_false;
          }
          return STk_true;
        }
      }
      break;
    case tc_instance:
      if (STk_oo_initialized) {
        SCM fg, res;

        fg = STk_lookup(STk_intern("object-equal?"),STk_current_module(),
                        &res,FALSE);
        res = STk_C_apply(fg, 2, x, y);
        return res;
      }
      break;
    case tc_struct:
      if (STRUCTP(y) && (STRUCT_TYPE(x) == STRUCT_TYPE(y)))
        return equal_count(STk_struct2list(x), STk_struct2list(y), max, cycle);
      break;
    case tc_box:
      if (BOXP(y)) {
        long lx, ly, i;
        lx = BOX_ARITY(x); ly = BOX_ARITY(y);
        if (lx == ly) {
          SCM *vx = BOX_VALUES(x);
          SCM *vy = BOX_VALUES(y);
          for (i=0; i < lx;  i++) {
            if (equal_count(vx[i], vy[i], max, cycle) == STk_false) return STk_false;
          }
          return STk_true;
        }
      }
      break;
    case tc_uvector:
      if (BOXED_TYPE_EQ(y, tc_uvector))
        return MAKE_BOOLEAN(STk_uvector_equal(x, y));
      break;

    // The default case could handle those labels. They are just here to
    // avoid the complex test in the default case when we are sure
    // to return #f
    case tc_not_boxed:
    case tc_integer:      case tc_real:         case tc_bignum:
    case tc_rational:     case tc_complex:      case tc_symbol:
    case tc_keyword:      case tc_module:       case tc_closure:
    case tc_subr0:        case tc_subr1:        case tc_subr2:
    case tc_subr3:        case tc_subr4:        case tc_subr5:
    case tc_subr01:       case tc_subr12:       case tc_subr23:
    case tc_subr34:       case tc_vsubr:        case tc_apply:
    case tc_hash_table:   case tc_frame:        case tc_next_method:
    case tc_promise:      case tc_regexp:       case tc_process:
    case tc_continuation: case tc_values:       case tc_parameter:
    case tc_socket:       case tc_struct_type:  case tc_thread:
    case tc_mutex:        case tc_condv:        case tc_ext_func:
    case tc_pointer:      case tc_callback:     case tc_syntax:
      return STk_false;

   default:
     // FIXME: The following code uses the above equal? . As a consequenece,
     // we will not be able to detecte cycles in extended types.
     if ((HAS_USER_TYPEP(x) && HAS_USER_TYPEP(y)) &&
          (BOXED_TYPE(x) == BOXED_TYPE(y)))
        return STk_extended_equal(x, y);
  }
  return STk_false;
}

/* %equal-try returns a boolean when it doesn't detect a cycle (in a
 * given amount of calls). It returns '() when it suspects a cycle.
 */
DEFINE_PRIMITIVE("%equal-try", equal_try, subr2, (SCM x, SCM y))
{
  int cycle = 0;
  SCM res = equal_count(x, y, max_equal_calls, &cycle);
  return (cycle) ? STk_nil : res;
}


int STk_init_boolean(void)
{
  limit_max_equal_calls();
  ADD_PRIMITIVE(not);
  ADD_PRIMITIVE(booleanp);
  ADD_PRIMITIVE(eq);
  ADD_PRIMITIVE(eqv);
  ADD_PRIMITIVE(equal);
  ADD_PRIMITIVE(equal_try);
  return TRUE;
}
