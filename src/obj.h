#pragma once

#include "safestl.h"

typedef int64_t _Int;
typedef double _Float;

struct CodeObject;
struct BaseRef;
class VM;
class Frame;

typedef PyVar (*_CppFunc)(VM*, const pkpy::ArgList&);
typedef pkpy::shared_ptr<CodeObject> _Code;

struct Function {
    _Str name;
    _Code code;
    std::vector<_Str> args;
    _Str starredArg;        // empty if no *arg
    PyVarDict kwArgs;       // empty if no k=v
    std::vector<_Str> kwArgsOrder;

    bool hasName(const _Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        return _0 || _1 || _2;
    }
};

struct _BoundedMethod {
    PyVar obj;
    PyVar method;
};

struct _Range {
    _Int start = 0;
    _Int stop = -1;
    _Int step = 1;
};

struct _Slice {
    int start = 0;
    int stop = 0x7fffffff; 

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

class BaseIterator {
protected:
    VM* vm;
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    virtual bool hasNext() = 0;
    PyVarRef var;
    BaseIterator(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIterator() = default;
};

typedef pkpy::shared_ptr<Function> _Func;
typedef pkpy::shared_ptr<BaseIterator> _Iterator;

typedef PyVar PyAutoRef;
typedef PyObject* PyWeakRef;

struct PyObject {
    int ref_count;
    PyAutoRef type;
    PyVarDict attribs;

    PyObject(PyObject* type) : type(type) { ref_count = 1; }

    inline void incref() { ref_count++; }
    inline void decref() { if(--ref_count == 0) delete this; }
    [[nodiscard]] inline PyObject* copy() { incref(); return this; }
    [[nodiscard]] inline PyAutoRef share() { return PyAutoRef(this); }

    inline bool is_type(const PyObject* type) const{ return this->type.get() == type; }
    inline bool is_type(const PyAutoRef& type) const{ return this->type.get() == type.get(); }

    virtual void* value() = 0;
    virtual ~PyObject() = default;
};


template <typename T>
struct Py_ : PyObject {
    T _valueT;

    Py_(T val, const PyVar& type) : PyObject(type), _valueT(val) {}
    virtual void* value() override { return &_valueT; }
};

#define UNION_GET(T, obj) (( (Py_<T>*)(obj) )->_valueT)
#define UNION_NAME(obj) UNION_GET(_Str, (obj)->attribs[__name__].get())
#define UNION_TP_NAME(obj) UNION_NAME((obj)->type)