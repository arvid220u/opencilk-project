// RUN: %clang_cc1 -std=c++17 -verify -verify-ignore-unexpected=note %s

namespace StdMock {
template <class T>
struct Vector {
  Vector();
  struct It {
    T value;
    int operator-(It &);
    It operator+(int);
    It operator++();
    It operator--();
    T operator*();
    bool operator!=(It &);
  };
  It begin();
  It end();
};
}

int foo(int n);

int Cilk_for_range_tests(int n) {
  StdMock::Vector<int> v(n);
  for (int i = 0; i < n; i++) v[i] = i;

  _Cilk_for(auto x : v); // expected-warning {{Cilk for loop has empty body}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
  _Cilk_for(auto& x : v); // expected-warning {{Cilk for loop has empty body}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
  _Cilk_for(int x : v); // expected-warning {{Cilk for loop has empty body}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
  _Cilk_for(long x : v); // expected-warning {{Cilk for loop has empty body}} expected-error {{no viable conversion from 'int' to 'std::string' (aka 'basic_string<char>')}}       expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
  _Cilk_for(long x : {1, 1234}); // expected-warning {{Cilk for loop has empty body}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}

  // Pairs are aggregate types, which initially had a bug. Assert that they work
//  std::vector<std::pair<int,int>> vp(n);
//  for (int i = 0; i < n; i++) {
//    vp[i] = std::make_pair(i,i+1);
//  }
//  _Cilk_for(auto p : vp) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//    continue;
//  _Cilk_for(auto& p : vp) { // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//    continue;
//  }
//
//  int a[5];
//  _Cilk_for(int x : a) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//      continue;
//
//  std::set<long> s(v.begin(), v.end());
//  _Cilk_for(int x : s); // expected-error {{Cannot determine length with '__end - __begin'. Please use a random access iterator.}} expected-error {{invalid operands to binary expression ('std::_Rb_tree_const_iterator<long>' and 'std::_Rb_tree_const_iterator<long>')}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//  std::unordered_set<long> us(v.begin(), v.end());
//  _Cilk_for(int x : us); // expected-error {{Cannot determine length with '__end - __begin'. Please use a random access iterator.}} expected-error {{invalid operands to binary expression ('std::__detail::_Node_iterator<long, true, false>' and 'std::__detail::_Node_iterator<long, true, false>')}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//
//  // Check for return statements, which cannot appear anywhere in the body of a
//  // _Cilk_for loop.
//  _Cilk_for (int i : v) return 7; // expected-error{{cannot return}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//  _Cilk_for (int i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//    for (int j = 1; j < i; ++j)
//      return 7; // expected-error{{cannot return}}
//
//  // Check for illegal break statements, which cannot bind to the scope of a
//  // _Cilk_for loop, but can bind to loops nested within.
//  _Cilk_for (int i : v) break; // expected-error{{cannot break}} expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//  _Cilk_for (int i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
//    for (int j = 1; j < i; ++j)
//      break;
//
  return 0;
}

int range_pragma_tests(int n) {
  StdMock::Vector<int> v(n);
  for (int i = 0; i < n; i++) v[i] = i;

#pragma clang loop unroll_count(4)
  _Cilk_for (auto i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
    foo(i);

#pragma cilk grainsize(4)
  _Cilk_for (int i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
    foo(i);

#pragma cilk grainsize 4
  _Cilk_for (auto i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
    foo(i);

#pragma cilk grainsize = 4 \
// expected-warning{{'#pragma cilk grainsize' no longer requires '='}}
  _Cilk_for (int i : v) // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
    foo(i);

  return 0;
}

int range_scope_tests(int n) {
  StdMock::Vector<int> v(n);
  for (int i = 0; i < n; i++) v[i] = i;
  int A[5];
  _Cilk_for(int i : v) { // expected-warning {{'_Cilk_for' support for for-range loops is currently EXPERIMENTAL only!}}
    int A[5];
    A[i%5] = i;
  }
  for(int i : v) {
    A[i%5] = i%5;
  }
  return 0;
}
