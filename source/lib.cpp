#include "BitVM.h"
#include <limits.h>
#include <stdlib.h>

#ifdef DESKTOP
inline int uBitRandom(int max)
{
    return rand() % max;
}
#else
#define uBitRandom uBit.random
#endif

namespace bitvm {

#ifdef DEBUG_MEMLEAKS
    std::set<RefObject*> allptrs;
    void debugMemLeaks()
    {
        printf("LIVE POINTERS:\n");
        for(std::set<RefObject*>::iterator itr = allptrs.begin();itr!=allptrs.end();itr++)
        {
            (*itr)->print();
        }    
        printf("\n");
    }
#endif


    namespace math {
        int max(int x, int y) { return x < y ? y : x; }
        int min(int x, int y) { return x < y ? x : y; }
        int random(int max) {
          if (max == INT_MIN)
            return -uBitRandom(INT_MAX);
          else if (max < 0)
            return -uBitRandom(-max);
          else if (max == 0)
            return 0;
          else
            return uBitRandom(max);
        }
        // Unspecified behavior for int_min
        int abs(int x) { return x < 0 ? -x : x; }
        int mod (int x, int y) { return x % y; }

        int pow(int x, int n) {
          if (n < 0)
          return 0;
          int r = 1;
          while (n) {
            if (n & 1)
              r *= x;
            n >>= 1;
            x *= x;
          }
          return r;
        }

        int clamp(int l, int h, int x) {
          return x < l ? l : x > h ? h : x;
        }

        int sqrt(int x) {
          return sqrt(x);
        }

        int sign(int x) {
          return x > 0 ? 1 : (x == 0 ? 0 : -1);
        }

        int pi() { return 3; }
    }

  namespace number {
    int lt(int x, int y) { return x < y; }
    int leq(int x, int y) { return x <= y; }
    int neq(int x, int y) { return x != y; }
    int eq(int x, int y) { return x == y; }
    int gt(int x, int y) { return x > y; }
    int geq(int x, int y) { return x >= y; }
    int plus(int x, int y) { return x + y; }
    int minus(int x, int y) { return x - y; }
    int div(int x, int y) { return x / y; }
    int times(int x, int y) { return x * y; }
    //ManagedString to_string(int x) { return ManagedString(x); }
    //ManagedString to_character(int x) { return ManagedString((char) x); }
    void post_to_wall(int n) { printf("%d\n", n); }
  }

  namespace bits {

    // See http://blog.regehr.org/archives/1063
    uint32_t rotl32c (uint32_t x, uint32_t n)
    {
      return (x<<n) | (x>>(-n&31));
    }

    int or_uint32(int x, int y) { return (uint32_t) x | (uint32_t) y; }
    int and_uint32(int x, int y) { return (uint32_t) x & (uint32_t) y; }
    int xor_uint32(int x, int y) { return (uint32_t) x ^ (uint32_t) y; }
    int shift_left_uint32(int x, int y) { return (uint32_t) x << y; }
    int shift_right_uint32(int x, int y) { return (uint32_t) x >> y; }
    int rotate_right_uint32(int x, int y) { return rotl32c((uint32_t) x, 32-y); }
    int rotate_left_uint32(int x, int y) { return rotl32c((uint32_t) x, y); }
  }

  namespace boolean {
    int or_(int x, int y) { return x || y; }
    int and_(int x, int y) { return x && y; }
    int not_(int x) { return !x; }
    int equals(int x, int y) { return x == y; }
  }

  namespace contract {
    void assert(int cond, char *msg)
    {
      if (cond == 0) {
        printf("Assertion failed: %s\n", msg);
        exit(1);
      }
    }
  }

    RefString *mkStringSz(uint32_t sz)
    {
        RefString *res = new RefString();
        res->len = sz;
        res->data = new char[res->len + 1];
        return res;
    }

    RefString *mkStringLen(const char *lit, uint32_t len)
    {
        RefString *res = mkStringSz(len);
        memcpy(res->data, lit, len);
        res->data[len] = 0;
        //printf("MK: %s\n", res->data);
        return res;
    }

    RefString *mkString(const char *lit)
    {
        return mkStringLen(lit, strlen(lit));
    }


    namespace string {

        RefString *dempty;

        RefString *fromLiteral(const char *p)
        {
            RefString *res = new RefString();
            res->refcnt = 2; // never unrefed
            res->len = strlen(p);
            res->data = (char*)p;
            res->canLeak();
            return res;
        }

        RefString *mkEmpty()
        {
            if (dempty == NULL)
                dempty = fromLiteral("");

            dempty->ref();
            return dempty;
        }

        RefString *concat_op(RefString *s1, RefString *s2)
        {
            RefString *res = mkStringSz(s1->len + s2->len);
            memcpy(res->data, s1->data, s1->len);
            memcpy(res->data + s1->len, s2->data, s2->len);
            res->data[res->len] = 0;
            //printf("MK2: %s\n", res->data);
            return res;
        }

        RefString *substring(RefString *s, int start, int length)
        {
            // If the parameters are illegal, just return a reference to the empty string.
            if (start >= s->len)
                return mkEmpty();

            // Compute a safe copy length;
            length = min(s->len-start, length);

            // Build a ManagedString from this.
            return mkStringLen(s->data+start, length);
        }

        int equals(RefString *s1, RefString *s2)
        {
            return s1->len == s2->len && memcmp(s1->data, s2->data, s1->len) == 0;
        }

        int count(RefString *s)
        {
          return s->len;
        }

        int code_at(RefString *s, int index) {
          return (index >=0 && index < s->len) ? s->data[index] : 0;
        }

        RefString *at(RefString *s, int i)
        {
            char b[2];
            b[0] = code_at(s, i);
            b[1] = 0;
            return mkString(b);
        }

        int to_character_code(RefString *s) {
            return code_at(s, 0);
        }

        int to_number(RefString *s) {
          return atoi(s->data);
        }

        void post_to_wall(RefString *s) { printf("%s\n", s->data); }
    }


    namespace boolean {
        RefString *sTrue, *sFalse;

        RefString *to_string(int v)
        {
            if (v) {
                if (sTrue == NULL) sTrue = string::fromLiteral("true");
                sTrue->ref();
                return sTrue;
            } else {
                if (sFalse == NULL) sFalse = string::fromLiteral("false");
                sFalse->ref();
                return sFalse;
            }            
        }
    }

    namespace number {
        RefString *to_character(int x)
        {
            char b[2];
            b[0] = x;
            b[1] = 0;
            return mkString(b);
        }

        RefString *to_string(int x)
        {
            char buf[30];
            snprintf(buf, 29, "%d", x);
            return mkString(buf);
        }
    }

    namespace collection {

        RefCollection *mk()
        {
            RefCollection *r = new RefCollection();
            return r;
        }

        int count(RefCollection *c) { return c->data.size(); }
        void add(RefCollection *c, int x) { c->data.push_back(x); }

        inline bool in_range(RefCollection *c, int x) {
            return (0 <= x && x < (int)c->data.size());
        }

        int at(RefCollection *c, int x) {
            if (in_range(c, x))
                return c->data.at(x);
            else {
                error(ERR_OUT_OF_BOUNDS);
                return 0;
            }
        }

        void remove_at(RefCollection *c, int x) {
            if (!in_range(c, x))
                return;

            c->data.erase(c->data.begin()+x);
        }

        void set_at(RefCollection *c, int x, int y) {
            if (!in_range(c, x))
                return;

            c->data.at(x) = y;
        }

        int index_of(RefCollection *c, uint32_t x, int start) {
            if (!in_range(c, start))
                return -1;

            for (uint32_t i = start; i < c->data.size(); ++i)
                if (c->data.at(i) == x)
                    return i;

            return -1;
        }

        int remove(RefCollection *c, int x) {
            int idx = index_of(c, x, 0);
            if (idx >= 0) {
                remove_at(c, idx);
                return 1;
            }

            return 0;
        }
    }

    namespace refcollection {

        RefRefCollection *mk()
        {
            RefRefCollection *r = new RefRefCollection();
            return r;
        }

        int count(RefRefCollection *c) { return c->data.size(); }

        void add(RefRefCollection *c, RefObject *x) {
            if (x) x->ref();
            c->data.push_back(x);
        }

        inline bool in_range(RefRefCollection *c, int x) {
            return (0 <= x && x < (int)c->data.size());
        }

        RefObject *at(RefRefCollection *c, int x) {
            if (in_range(c, x)) {
                RefObject *tmp = c->data.at(x);
                if (tmp) tmp->ref();
                return tmp;
            }
            else {
                error(ERR_OUT_OF_BOUNDS);
                return 0;
            }
        }

        void remove_at(RefRefCollection *c, int x) {
            if (!in_range(c, x))
                return;

            RefObject *tmp = c->data.at(x);
            if (tmp) tmp->unref();
            c->data.erase(c->data.begin()+x);
        }

        void set_at(RefRefCollection *c, int x, RefObject *y) {
            if (!in_range(c, x))
                return;

            RefObject *tmp = c->data.at(x);
            if (tmp) tmp->unref();
            if (y) y->ref();
            c->data.at(x) = y;
        }

        int index_of(RefRefCollection *c, RefObject *x, int start) {
            if (!in_range(c, start))
                return -1;

            for (uint32_t i = start; i < c->data.size(); ++i)
                if (c->data.at(i)->equals(x))
                    return i;

            return -1;
        }

        int remove(RefRefCollection *c, RefObject *x) {
            int idx = index_of(c, x, 0);
            if (idx >= 0) {
                remove_at(c, idx);
                return 1;
            }

            return 0;
        }
    }

    namespace record {
        RefRecord* mk(int reflen, int totallen)
        {
            check(0 <= reflen && reflen <= totallen, ERR_SIZE, 1);
            check(reflen <= totallen && totallen <= 255, ERR_SIZE, 2);

            void *ptr = ::operator new(sizeof(RefRecord) + totallen * sizeof(uint32_t));
            RefRecord *r = new (ptr) RefRecord();
            r->len = totallen;
            r->reflen = reflen;
            memset(r->fields, 0, r->len * sizeof(uint32_t));
            return r;
        }
    }

    namespace action {
        RefAction* mk(int reflen, int totallen, int startptr)
        {
            check(0 <= reflen && reflen <= totallen, ERR_SIZE, 1);
            check(reflen <= totallen && totallen <= 255, ERR_SIZE, 2);

            void *ptr = ::operator new(sizeof(RefAction) + totallen * sizeof(uint32_t));
            RefAction *r = new (ptr) RefAction();
            r->len = totallen;
            r->reflen = reflen;
            r->startptr = startptr;
            memset(r->fields, 0, r->len * sizeof(uint32_t));
            return r;
        }

        void run(RefAction *a)
        {
            a->run();
        }
    }
 




    const void *callProc0[] = {
    };

    const void *callProc1[] = {
        (void*)number::post_to_wall,
        (void*)string::post_to_wall,
        (void*)action::run,
    };

    const void *callProc2[] = {
        (void*)contract::assert,
        (void*)collection::add,
        (void*)collection::remove_at,
        (void*)refcollection::add,
        (void*)refcollection::remove_at,
    };

    const void *callProc3[] = {
        (void*)collection::set_at,
        (void*)refcollection::set_at,
    };

    const void *callProc4[] = {
    };


    const void *callFunc0[] = {
        (void*)string::mkEmpty,
        (void*)collection::mk,
        (void*)refcollection::mk,
        (void*)math::pi,
    };

    const void *callFunc1[] = {
        (void*)boolean::not_,
        (void*)math::random,
        (void*)math::abs,
        (void*)math::sqrt,
        (void*)math::sign,
        (void*)string::count,
        (void*)string::to_character_code,
        (void*)string::to_number,
        (void*)bitvm::mkString,
        (void*)number::to_character,
        (void*)number::to_string,
        (void*)collection::count,
        (void*)refcollection::count,
        (void*)boolean::to_string,
    };

    const void *callFunc2[] = {
        (void*)boolean::or_,
        (void*)boolean::and_,
        (void*)boolean::equals,
        (void*)bits::or_uint32,
        (void*)bits::and_uint32,
        (void*)bits::xor_uint32,
        (void*)bits::shift_left_uint32,
        (void*)bits::shift_right_uint32,
        (void*)bits::rotate_right_uint32,
        (void*)bits::rotate_left_uint32,
        (void*)number::lt,
        (void*)number::leq,
        (void*)number::neq,
        (void*)number::eq,
        (void*)number::gt,
        (void*)number::geq,
        (void*)number::plus,
        (void*)number::minus,
        (void*)number::div,
        (void*)number::times,
        (void*)math::max,
        (void*)math::min,
        (void*)math::mod,
        (void*)math::pow,
        (void*)string::concat_op,
        (void*)string::equals,
        (void*)string::code_at,
        (void*)string::at,
        (void*)collection::at,
        (void*)collection::remove,
        (void*)refcollection::at,
        (void*)refcollection::remove,
        (void*)record::mk,
    };

    const void *callFunc3[] = {
        (void*)math::clamp,
        (void*)string::substring,
        (void*)collection::index_of,
        (void*)refcollection::index_of,
        (void*)action::mk,
    };

    const void *callFunc4[] = {
    };

    const int enums[] = {
        ERR_BAD_OPCODE,
        ERR_STACK_OVERFLOW,
        ERR_STACK_UNDERFLOW,
        ERR_INVALID_FUNCTION_HEADER,
        ERR_INVALID_BINARY_HEADER,
        ERR_STACK_ONRETURN,
        ERR_REF_DELETED,
        ERR_OUT_OF_BOUNDS,
        ERR_SIZE,
    };

}

// vim: ts=4 sw=4
