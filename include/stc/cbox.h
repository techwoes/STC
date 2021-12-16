/* MIT License
 *
 * Copyright (c) 2021 Tyge Løvset, NORCE, www.norceresearch.no
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

/* cbox: heap allocated boxed type
#include <stc/cstr.h>

typedef struct { cstr name, last; } Person;

Person Person_from(const char* name, const char* last) {
    return (Person){.name = cstr_from(name), .last = cstr_from(last)};
}
Person Person_clone(Person p) {
    p.name = cstr_clone(p.name);
    p.last = cstr_clone(p.last);
    return p;
}
void Person_del(Person* p) {
    printf("del: %s %s\n", p->name.str, p->last.str);
    c_del(cstr, &p->name, &p->last);
}

#define i_val Person
#define i_valdel Person_del
#define i_valfrom Person_clone
#define i_opt c_no_compare // compare by .get addresses only
#define i_tag prs
#include <stc/cbox.h>

int main() {
    c_autovar (cbox_prs p = cbox_prs_new(Person_from("John", "Smiths")), cbox_prs_del(&p))
    c_autovar (cbox_prs q = cbox_prs_clone(p), cbox_prs_del(&q))
    {
        cstr_assign(&q.get->name, "Joe");

        printf("%s %s.\n", p.get->name.str, p.get->last.str);
        printf("%s %s.\n", q.get->name.str, q.get->last.str);
    }
}
*/

#ifndef CBOX_H_INCLUDED
#define CBOX_H_INCLUDED
#include "ccommon.h"
#include "forward.h"
#include <stdlib.h>

#define cbox_null {NULL}
#endif // CBOX_H_INCLUDED

#ifndef _i_prefix
#define _i_prefix cbox_
#endif
#include "template.h"

#if !c_option(c_is_fwd)
_cx_deftypes(_c_cbox_types, _cx_self, i_val);
#endif

// constructors (takes ownsership)
STC_INLINE _cx_self
_cx_memb(_init)(void) { return c_make(_cx_self){NULL}; }

STC_INLINE _cx_self
_cx_memb(_with)(i_val* p) { return c_make(_cx_self){p}; }

STC_INLINE _cx_self
_cx_memb(_new)(i_val val) { 
    return c_make(_cx_self){c_new(i_val, val)};
}

// destructor
STC_INLINE void
_cx_memb(_del)(_cx_self* self) {
    if (self->get) { i_valdel(self->get); c_free(self->get); }
}

STC_INLINE _cx_self
_cx_memb(_move)(_cx_self* self) {
    _cx_self ptr = *self; self->get = NULL;
    return ptr;
}

STC_INLINE void
_cx_memb(_reset)(_cx_self* self) {
    _cx_memb(_del)(self); self->get = NULL;
}

// take ownership of *p
STC_INLINE void
_cx_memb(_reset_with)(_cx_self* self, _cx_value* p) {
    _cx_memb(_del)(self); self->get = p;
}

// take ownership of val
STC_INLINE void
_cx_memb(_reset_new)(_cx_self* self, i_val val) {
    if (self->get) { i_valdel(self->get); *self->get = val; }
    else self->get = c_new(i_val, val);
}

#if !c_option(c_no_clone)
    STC_INLINE _cx_self
    _cx_memb(_from)(i_valraw raw) { 
        return c_make(_cx_self){c_new(i_val, i_valfrom(raw))};
    }

    STC_INLINE void
    _cx_memb(_reset_from)(_cx_self* self, i_valraw raw) {
        _cx_memb(_reset_new)(self, i_valfrom(raw));
    }    

    STC_INLINE _cx_self
    _cx_memb(_clone)(_cx_self other) {
        if (!other.get) return other;
        return c_make(_cx_self){c_new(i_val, i_valfrom(i_valto(other.get)))};
    }

    STC_INLINE void
    _cx_memb(_copy)(_cx_self* self, _cx_self other) {
        if (self->get == other.get) return;
        if (other.get) _cx_memb(_reset_new)(self, *other.get);
        else _cx_memb(_reset)(self);
    }
#endif

STC_INLINE void
_cx_memb(_take)(_cx_self* self, _cx_self other) {
    if (other.get != self->get) _cx_memb(_del)(self);
    *self = other;
}

STC_INLINE uint64_t
_cx_memb(_hash)(const _cx_self* self, size_t n) {
    #if (c_option(c_no_compare) || defined _i_default_compare) && SIZE_MAX >> 32
        return c_hash64(&self->get, 8);
    #elif (c_option(c_no_compare) || defined _i_default_compare)
        return c_hash32(&self->get, 4);
    #else
        i_valraw raw = i_valto(self->get);
        return i_hash(&raw, sizeof raw);
    #endif
}

STC_INLINE int
_cx_memb(_compare)(const _cx_self* x, const _cx_self* y) {
    #if (c_option(c_no_compare) || defined _i_default_compare)
        return (int)(x->get - y->get);
    #else
        i_valraw rx = i_valto(x->get);
        i_valraw ry = i_valto(y->get);
        return i_cmp(&rx, &ry);
    #endif
}
#include "template.h"