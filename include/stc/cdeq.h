/* MIT License
 *
 * Copyright (c) 2023 Tyge Løvset
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
#define _i_prefix cdeq_
#define _pop _pop_front
#ifdef i_more
  #include "cqueue.h"
  #define i_more
#else
  #define i_more
  #include "cqueue.h"
#endif
#undef _pop

STC_API _cx_value* _cx_memb(_push_front)(_cx_self* self, i_key value);
STC_API _cx_iter   _cx_memb(_insert_n)(_cx_self* self, intptr_t idx, const _cx_value* arr, intptr_t n);
STC_API _cx_iter   _cx_memb(_emplace_n)(_cx_self* self, intptr_t idx, const _cx_raw* raw, intptr_t n);
STC_API _cx_iter   _cx_memb(_insert_uninit)(_cx_self* self, intptr_t idx, intptr_t n);
STC_API void       _cx_memb(_erase_n)(_cx_self* self, intptr_t idx, intptr_t n);

STC_INLINE const _cx_value*
_cx_memb(_at)(const _cx_self* self, intptr_t idx)
    { return self->data + _cdeq_topos(self, idx); }

STC_INLINE _cx_value*
_cx_memb(_at_mut)(_cx_self* self, intptr_t idx)
    { return self->data + _cdeq_topos(self, idx); }

STC_INLINE _cx_value*
_cx_memb(_push_back)(_cx_self* self, _cx_value val)
    { return  _cx_memb(_push)(self, val); }

STC_INLINE void
_cx_memb(_pop_back)(_cx_self* self) {
    assert(!_cx_memb(_empty)(self));
    self->end = (self->end - 1) & self->capmask;
    i_keydrop((self->data + self->end));
}

STC_INLINE _cx_iter
_cx_memb(_insert_at)(_cx_self* self, _cx_iter it, const _cx_value val) {
    intptr_t idx = _cdeq_toidx(self, it.pos);
    return _cx_memb(_insert_n)(self, idx, &val, 1);
}

STC_INLINE _cx_iter
_cx_memb(_erase_at)(_cx_self* self, _cx_iter it) {
    _cx_memb(_erase_n)(self, _cdeq_toidx(self, it.pos), 1);
    if (it.pos == self->end) it.ref = NULL;
    return it;
}

STC_INLINE _cx_iter
_cx_memb(_erase_range)(_cx_self* self, _cx_iter it1, _cx_iter it2) {
    intptr_t idx1 = _cdeq_toidx(self, it1.pos);
    intptr_t idx2 = _cdeq_toidx(self, it2.pos);
    _cx_memb(_erase_n)(self, idx1, idx2 - idx1);
    if (it1.pos == self->end) it1.ref = NULL;
    return it1;
}

#if !defined i_no_emplace
STC_INLINE _cx_value*
_cx_memb(_emplace_front)(_cx_self* self, const _cx_raw raw)
    { return _cx_memb(_push_front)(self, i_keyfrom(raw)); }

STC_INLINE _cx_value*
_cx_memb(_emplace_back)(_cx_self* self, const _cx_raw raw)
    { return _cx_memb(_push)(self, i_keyfrom(raw)); }

STC_INLINE _cx_iter
_cx_memb(_emplace_at)(_cx_self* self, _cx_iter it, const _cx_raw raw) 
    { return _cx_memb(_insert_at)(self, it, i_keyfrom(raw)); }
#endif

#if defined _i_has_eq || defined _i_has_cmp
STC_API _cx_iter _cx_memb(_find_in)(_cx_iter p1, _cx_iter p2, _cx_raw raw);
STC_API bool _cx_memb(_eq)(const _cx_self* self, const _cx_self* other);

STC_INLINE _cx_iter
_cx_memb(_find)(const _cx_self* self, _cx_raw raw) {
    return _cx_memb(_find_in)(_cx_memb(_begin)(self), _cx_memb(_end)(self), raw);
}

STC_INLINE const _cx_value*
_cx_memb(_get)(const _cx_self* self, _cx_raw raw) {
    return _cx_memb(_find_in)(_cx_memb(_begin)(self), _cx_memb(_end)(self), raw).ref;
}

STC_INLINE _cx_value*
_cx_memb(_get_mut)(_cx_self* self, _cx_raw raw)
    { return (_cx_value *) _cx_memb(_get)(self, raw); }
#endif

/* -------------------------- IMPLEMENTATION ------------------------- */
#if defined(i_implement)

STC_DEF _cx_value*
_cx_memb(_push_front)(_cx_self* self, i_key value) {
    intptr_t start = (self->start - 1) & self->capmask;
    if (start == self->end) { // full
        _cx_memb(_reserve)(self, self->capmask + 3); // => 2x expand
        start = (self->start - 1) & self->capmask;
    }
    _cx_value *v = self->data + start;
    self->start = start;
    *v = value;
    return v;
}

STC_DEF void
_cx_memb(_erase_n)(_cx_self* self, const intptr_t idx, const intptr_t n) {
    const intptr_t len = _cx_memb(_size)(self);
    for (intptr_t i = idx + n - 1; i >= idx; --i) 
        i_keydrop(_cx_memb(_at_mut)(self, i));
    for (intptr_t i = idx, j = i + n; j < len; ++i, ++j)
        *_cx_memb(_at_mut)(self, i) = *_cx_memb(_at)(self, j);
    self->end = (self->end - n) & self->capmask;
}

STC_DEF _cx_iter
_cx_memb(_insert_uninit)(_cx_self* self, const intptr_t idx, const intptr_t n) {
    const intptr_t len = _cx_memb(_size)(self);
    _cx_iter it = {.pos=_cdeq_topos(self, idx), ._s=self};
    if (len + n > self->capmask)
        if (!_cx_memb(_reserve)(self, len + n))
            return it;
    for (intptr_t j = len - 1, i = j + n; j >= idx; --j, --i)
        *_cx_memb(_at_mut)(self, i) = *_cx_memb(_at)(self, j);
    self->end = (self->end + n) & self->capmask;
    it.ref = self->data + it.pos;
    return it;
}

STC_DEF _cx_iter
_cx_memb(_insert_n)(_cx_self* self, const intptr_t idx, const _cx_value* arr, const intptr_t n) {
    _cx_iter it = _cx_memb(_insert_uninit)(self, idx, n);
    for (intptr_t i = idx, j = 0; j < n; ++i, ++j)
        *_cx_memb(_at_mut)(self, i) = arr[j];
    return it;
}

STC_DEF _cx_iter
_cx_memb(_emplace_n)(_cx_self* self, const intptr_t idx, const _cx_raw* raw, const intptr_t n) {
    _cx_iter it = _cx_memb(_insert_uninit)(self, idx, n);
    for (intptr_t i = idx, j = 0; j < n; ++i, ++j)
        *_cx_memb(_at_mut)(self, i) = i_keyfrom(raw[j]);
    return it;
}

#if defined _i_has_eq || defined _i_has_cmp
STC_DEF _cx_iter
_cx_memb(_find_in)(_cx_iter i1, _cx_iter i2, _cx_raw raw) {
    for (; i1.pos != i2.pos; _cx_memb(_next)(&i1)) {
        const _cx_raw r = i_keyto(i1.ref);
        if (i_eq((&raw), (&r)))
            break;
    }
    return i1;
}

STC_DEF bool
_cx_memb(_eq)(const _cx_self* self, const _cx_self* other) {
    if (_cx_memb(_size)(self) != _cx_memb(_size)(other)) return false;
    for (_cx_iter i = _cx_memb(_begin)(self), j = _cx_memb(_begin)(other);
         i.ref; _cx_memb(_next)(&i), _cx_memb(_next)(&j))
    {
        const _cx_raw _rx = i_keyto(i.ref), _ry = i_keyto(j.ref);
        if (!(i_eq((&_rx), (&_ry)))) return false;
    }
    return true;
}
#endif
#endif // IMPLEMENTATION
#define CDEQ_H_INCLUDED
#include "priv/template2.h"
