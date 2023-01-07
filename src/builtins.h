#pragma once

const char* __BUILTINS_CODE = R"(
def print(*args, sep=' ', end='\n'):
    s = sep.join([str(i) for i in args])
    __sys_stdout_write(s + end)

def round(x, ndigits=0):
    assert ndigits >= 0
    if ndigits == 0:
        return x >= 0 ? int(x + 0.5) : int(x - 0.5)
    if x >= 0:
        return int(x * 10**ndigits + 0.5) / 10**ndigits
    else:
        return int(x * 10**ndigits - 0.5) / 10**ndigits

def abs(x):
    return x < 0 ? -x : x

def max(a, b):
    return a > b ? a : b

def min(a, b):
    return a < b ? a : b

def sum(iterable):
    res = 0
    for i in iterable:
        res += i
    return res

def map(f, iterable):
    return [f(i) for i in iterable]

def zip(a, b):
    return [(a[i], b[i]) for i in range(min(len(a), len(b)))]

def reversed(iterable):
    a = list(iterable)
    return [a[i] for i in range(len(a)-1, -1, -1)]

def sorted(iterable, key=None, reverse=False):
    if key is None:
        key = lambda x: x
    a = [key(i) for i in iterable]
    b = list(iterable)
    for i in range(len(a)):
        for j in range(i+1, len(a)):
            if (a[i] > a[j]) ^ reverse:
                a[i], a[j] = a[j], a[i]
                b[i], b[j] = b[j], b[i]
    return b

##### str #####

str.__mul__ = lambda self, n: ''.join([self for _ in range(n)])

def __str4split(self, sep):
    if sep == "":
        return list(self)
    res = []
    i = 0
    while i < len(self):
        if self[i:i+len(sep)] == sep:
            res.append(self[:i])
            self = self[i+len(sep):]
            i = 0
        else:
            i += 1
    res.append(self)
    return res
str.split = __str4split
del __str4split

def __str4index(self, sub):
    for i in range(len(self)):
        if self[i:i+len(sub)] == sub:
            return i
    return -1
str.index = __str4index
del __str4index

def __str4strip(self, chars=None):
    chars = chars or ' \t\n\r'
    i = 0
    while i < len(self) and self[i] in chars:
        i += 1
    j = len(self) - 1
    while j >= 0 and self[j] in chars:
        j -= 1
    return self[i:j+1]
str.strip = __str4strip
del __str4strip

##### list #####

list.__repr__ = lambda self: '[' + ', '.join([repr(i) for i in self]) + ']'
tuple.__repr__ = lambda self: '(' + ', '.join([repr(i) for i in self]) + ')'
list.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'
tuple.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'

def __list4extend(self, other):
    for i in other:
        self.append(i)
list.extend = __list4extend
del __list4extend

def __list4remove(self, value):
    for i in range(len(self)):
        if self[i] == value:
            del self[i]
            return True
    return False
list.remove = __list4remove
del __list4remove

def __list4index(self, value):
    for i in range(len(self)):
        if self[i] == value:
            return i
    return -1
list.index = __list4index
del __list4index

def __list4pop(self, i=-1):
    res = self[i]
    del self[i]
    return res
list.pop = __list4pop
del __list4pop

def __list4__mul__(self, n):
    a = []
    for i in range(n):
        a.extend(self)
    return a
list.__mul__ = __list4__mul__
del __list4__mul__

def __iterable4__eq__(self, other):
    if len(self) != len(other):
        return False
    for i in range(len(self)):
        if self[i] != other[i]:
            return False
    return True
list.__eq__ = __iterable4__eq__
tuple.__eq__ = __iterable4__eq__
del __iterable4__eq__

def __iterable4count(self, x):
    res = 0
    for i in self:
        if i == x:
            res += 1
    return res
list.count = __iterable4count
tuple.count = __iterable4count
del __iterable4count

def __iterable4__contains__(self, item):
    for i in self:
        if i == item:
            return True
    return False
list.__contains__ = __iterable4__contains__
tuple.__contains__ = __iterable4__contains__
del __iterable4__contains__

list.__new__ = lambda obj: [i for i in obj]

# https://github.com/python/cpython/blob/main/Objects/dictobject.c
class dict:
    def __init__(self, capacity=16):
        self._capacity = capacity
        self._a = [None] * self._capacity
        self._len = 0
        
    def __len__(self):
        return self._len

    def __probe(self, key):
        i = hash(key) % self._capacity
        while self._a[i] is not None:
            if self._a[i][0] == key:
                return True, i
            i = (i + 1) % self._capacity
        return False, i

    def __getitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        return self._a[i][1]

    def __contains__(self, key):
        ok, i = self.__probe(key)
        return ok

    def __setitem__(self, key, value):
        ok, i = self.__probe(key)
        if ok:
            self._a[i][1] = value
        else:
            self._a[i] = [key, value]
            self._len += 1
            if self._len > self._capacity * 0.8:
                self._capacity *= 2
                self.__rehash()

    def __delitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        self._a[i] = None
        self._len -= 1

    def __rehash(self):
        old_a = self._a
        self._a = [None] * self._capacity
        self._len = 0
        for kv in old_a:
            if kv is not None:
                self[kv[0]] = kv[1]

    def keys(self):
        return [kv[0] for kv in self._a if kv is not None]

    def values(self):
        return [kv[1] for kv in self._a if kv is not None]

    def items(self):
        return [kv for kv in self._a if kv is not None]

    def clear(self):
        self._a = [None] * self._capacity
        self._len = 0

    def update(self, other):
        for k, v in other.items():
            self[k] = v

    def copy(self):
        d = dict()
        for kv in self._a:
            if kv is not None:
                d[kv[0]] = kv[1]
        return d

    def __repr__(self):
        a = [repr(k)+': '+repr(v) for k,v in self.items()]
        return '{'+ ', '.join(a) + '}'

    def __json__(self):
        a = []
        for k,v in self.items():
            if type(k) is not str:
                raise TypeError('json keys must be strings, got ' + repr(k) )
            a.append(k.__json__()+': '+v.__json__())
        return '{'+ ', '.join(a) + '}'

import json as _json

def jsonrpc(method, params, raw=False):
  assert type(method) is str
  assert type(params) is list or type(params) is tuple
  data = {
    'jsonrpc': '2.0',
    'method': method,
    'params': params,
  }
  ret = __string_channel_call(_json.dumps(data))
  ret = _json.loads(ret)
  if raw:
    return ret
  assert type(ret) is dict
  if 'result' in ret:
    return ret['result']
  raise JsonRpcError(ret['error']['message'])

def input(prompt=None):
  return jsonrpc('input', [prompt])
  
class FileIO:
  def __init__(self, path, mode):
    assert type(path) is str
    assert type(mode) is str
    assert mode in ['r', 'w', 'rt', 'wt']
    self.path = path
    self.mode = mode
    self.fp = jsonrpc('fopen', [path, mode])

  def read(self):
    assert self.mode in ['r', 'rt']
    return jsonrpc('fread', [self.fp])

  def write(self, s):
    assert self.mode in ['w', 'wt']
    assert type(s) is str
    jsonrpc('fwrite', [self.fp, s])

  def close(self):
    jsonrpc('fclose', [self.fp])

  def __enter__(self):
    pass

  def __exit__(self):
    self.close()

def open(path, mode='r'):
    return FileIO(path, mode)


class set:
    def __init__(self, iterable=None):
        iterable = iterable or []
        self._a = dict()
        for item in iterable:
            self.add(item)

    def add(self, elem):
        self._a[elem] = None
        
    def discard(self, elem):
        if elem in self._a:
            del self._a[elem]

    def remove(self, elem):
        del self._a[elem]
        
    def clear(self):
        self._a.clear()

    def update(self,other):
        for elem in other:
            self.add(elem)
        return self

    def __len__(self):
        return len(self._a)
    
    def copy(self):
        return set(self._a.keys())
    
    def __and__(self, other):
        ret = set()
        for elem in self:
            if elem in other:
                ret.add(elem)
        return ret
    
    def __or__(self, other):
        ret = self.copy()
        for elem in other:
            ret.add(elem)
        return ret

    def __sub__(self, other):
        ret = set() 
        for elem in self:
            if elem not in other: 
                ret.add(elem) 
        return ret
    
    def __xor__(self, other): 
        ret = set() 
        for elem in self: 
            if elem not in other: 
                ret.add(elem) 
        for elem in other: 
            if elem not in self: 
                ret.add(elem) 
        return ret

    def union(self, other):
        return self | other

    def intersection(self, other):
        return self & other

    def difference(self, other):
        return self - other

    def symmetric_difference(self, other):      
        return self ^ other
    
    def __eq__(self, other):
        return self.__xor__(other).__len__() == 0
    
    def isdisjoint(self, other):
        return self.__and__(other).__len__() == 0
    
    def issubset(self, other):
        return self.__sub__(other).__len__() == 0
    
    def issuperset(self, other):
        return other.__sub__(self).__len__() == 0

    def __contains__(self, elem):
        return elem in self._a
    
    def __repr__(self):
        if len(self) == 0:
            return 'set()'
        return '{'+ ', '.join(self._a.keys()) + '}'
    
    def __iter__(self):
        return self._a.keys().__iter__()
)";

const char* __OS_CODE = R"(
def listdir(path):
  assert type(path) is str
  return jsonrpc("os.listdir", [path])

def mkdir(path):
  assert type(path) is str
  return jsonrpc("os.mkdir", [path])

def rmdir(path):
  assert type(path) is str
  return jsonrpc("os.rmdir", [path])

def remove(path):
  assert type(path) is str
  return jsonrpc("os.remove", [path])

path = object()

def __path4exists(path):
  assert type(path) is str
  return jsonrpc("os.path.exists", [path])
path.exists = __path4exists
del __path4exists

def __path4join(*paths):
  s = '/'.join(paths)
  s = s.replace('\\', '/')
  s = s.replace('//', '/')
  s = s.replace('//', '/')
  return s

path.join = __path4join
del __path4join
)";

const char* __RANDOM_CODE = R"(
import time as _time

__all__ = ['Random', 'seed', 'random', 'randint', 'uniform']

def _int32(x):
	return int(0xffffffff & x)

class Random:
	def __init__(self, seed=None):
		if seed is None:
			seed = int(_time.time() * 1000000)
		seed = _int32(seed)
		self.mt = [0] * 624
		self.mt[0] = seed
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
	
	def extract_number(self):
		if self.mti == 0:
			self.twist()
		y = self.mt[self.mti]
		y = y ^ y >> 11
		y = y ^ y << 7 & 2636928640
		y = y ^ y << 15 & 4022730752
		y = y ^ y >> 18
		self.mti = (self.mti + 1) % 624
		return _int32(y)
	
	def twist(self):
		for i in range(0, 624):
			y = _int32((self.mt[i] & 0x80000000) + (self.mt[(i + 1) % 624] & 0x7fffffff))
			self.mt[i] = (y >> 1) ^ self.mt[(i + 397) % 624]
			
			if y % 2 != 0:
				self.mt[i] = self.mt[i] ^ 0x9908b0df
				
	def seed(self, x):
		assert type(x) is int
		self.mt = [0] * 624
		self.mt[0] = _int32(x)
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
			
	def random(self):
		return self.extract_number() / 2 ** 32
		
	def randint(self, a, b):
		assert type(a) is int and type(b) is int
		assert a <= b
		return int(self.random() * (b - a + 1)) + a
		
	def uniform(self, a, b):
        assert type(a) is int or type(a) is float
        assert type(b) is int or type(b) is float
		if a > b:
			a, b = b, a
		return self.random() * (b - a) + a

    def shuffle(self, L):
        for i in range(len(L)):
            j = self.randint(i, len(L) - 1)
            L[i], L[j] = L[j], L[i]

    def choice(self, L):
        return L[self.randint(0, len(L) - 1)]
		
_inst = Random()
seed = _inst.seed
random = _inst.random
randint = _inst.randint
uniform = _inst.uniform
shuffle = _inst.shuffle
choice = _inst.choice

)";