// RUN: %clang_cc1 -std=c++11 -fms-compatibility -fsyntax-only -verify %s

int foo() __attribute__((optnone));
int bar() __attribute__((optnone)) __attribute__((noinline));

int baz() __attribute__((always_inline)) __attribute__((optnone)); // expected-error{{'always_inline' and 'optnone' attributes are not compatible}}
int quz() __attribute__((optnone)) __attribute__((always_inline)); // expected-error{{'optnone' and 'always_inline' attributes are not compatible}}

__attribute__((always_inline)) int baz1(); // expected-warning{{'always_inline' attribute ignored}}
__attribute__((optnone)) int baz1() { return 1; } // expected-note{{conflicting attribute is here}}

__attribute__((optnone)) int quz1(); // expected-note{{conflicting attribute is here}}
__attribute__((always_inline)) int quz1() { return 1; } // expected-warning{{'always_inline' attribute ignored}}

int bay() __attribute__((minsize)) __attribute__((optnone)); // expected-error{{'minsize' and 'optnone' attributes are not compatible}}
int quy() __attribute__((optnone)) __attribute__((minsize)); // expected-error{{'optnone' and 'minsize' attributes are not compatible}}

__attribute__((minsize)) int bay1(); // expected-warning{{'minsize' attribute ignored}}
__attribute__((optnone)) int bay1() { return 1; } // expected-note{{conflicting attribute is here}}

__attribute__((optnone)) int quy1(); // expected-note{{conflicting attribute is here}}
__attribute__((minsize)) int quy1() { return 1; } // expected-warning{{'minsize' attribute ignored}}

__attribute__((always_inline)) // expected-warning{{'always_inline' attribute ignored}}
  __attribute__((minsize)) // expected-warning{{'minsize' attribute ignored}}
void bay2();
__attribute__((optnone)) // expected-note 2 {{conflicting}}
void bay2() {}

__forceinline __attribute__((optnone)) int bax(); // expected-error{{'__forceinline' and 'optnone' attributes are not compatible}}
__attribute__((optnone)) __forceinline int qux(); // expected-error{{'optnone' and '__forceinline' attributes are not compatible}}

int globalVar __attribute__((optnone)); // expected-warning{{'optnone' attribute only applies to functions}}

int fubar(int __attribute__((optnone)), int); // expected-warning{{'optnone' attribute only applies to functions}}

struct A {
  int aField __attribute__((optnone));  // expected-warning{{'optnone' attribute only applies to functions}}
};

struct B {
  void foo() __attribute__((optnone));
  static void bar() __attribute__((optnone));
};

// Verify that we can specify the [[clang::optnone]] syntax as well.

[[clang::optnone]]
int foo2();
[[clang::optnone]]
int bar2() __attribute__((noinline));

[[clang::optnone]]
int baz2() __attribute__((always_inline)); // expected-error{{'always_inline' and 'optnone' attributes are not compatible}}

[[clang::optnone]] int globalVar2; //expected-warning{{'optnone' attribute only applies to functions}}

struct A2 {
  [[clang::optnone]] int aField; // expected-warning{{'optnone' attribute only applies to functions}}
};

struct B2 {
  [[clang::optnone]]
  void foo();
  [[clang::optnone]]
  static void bar();
};

