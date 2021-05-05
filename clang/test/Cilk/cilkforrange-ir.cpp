// RUN: %clang_cc1 %s -std=c++11 -triple x86_64-unknown-linux-gnu -fcilkplus -ftapir=none -verify -S -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

namespace X {
struct C {
  C();
  struct It {
    int val;
    operator int &() { return val; }
    It operator+(It &);
    It operator-(It &);
    It operator++();
    It operator--();
//    int operator*();
  };
  It begin();
  It end();
};

int operator*(const C::It &) { return 0; }
}

void bar(int i);

void up(X::C c) {
  _Cilk_for (int x : c)
    bar(x);
}

// CHECK-LABEL: define void @_Z2upmm(

// CHECK: %[[START:.+]] = load i64, i64*
// CHECK-NEXT: store i64 %[[START]], i64* %[[INIT:.+]], align 8
// CHECK-NEXT: %[[END:.+]] = load i64, i64*
// CHECK-NEXT: store i64 %[[END]], i64* %[[LIMIT:.+]], align 8
// CHECK-NEXT: %[[INITCMPINIT:.+]] = load i64, i64* %[[INIT]]
// CHECK-NEXT: %[[INITCMPLIMIT:.+]] = load i64, i64* %[[LIMIT]]
// CHECK-NEXT: %[[INITCMP:.+]] = icmp ult i64 %[[INITCMPINIT]], %[[INITCMPLIMIT]]
// CHECK-NEXT: br i1 %[[INITCMP]], label %[[PFORPH:.+]], label %[[PFOREND:.+]]

// CHECK: [[PFORPH]]:
// CHECK-NEXT: store i64 0, i64* %[[BEGIN:.+]], align 8
// CHECK-NEXT: %[[ENDLIMIT:.+]] = load i64, i64* %[[LIMIT]]
// CHECK-NEXT: %[[ENDINIT:.+]] = load i64, i64* %[[INIT]]
// CHECK-NEXT: %[[ENDSUB:.+]] = sub i64 %[[ENDLIMIT]], %[[ENDINIT]]
// CHECK-NEXT: %[[ENDSUB1:.+]] = sub i64 %[[ENDSUB]], 1
// CHECK-NEXT: %[[ENDDIV:.+]] = udiv i64 %[[ENDSUB1]], 1
// CHECK-NEXT: %[[ENDADD:.+]] = add i64 %[[ENDDIV]], 1
// CHECK-NEXT: store i64 %[[ENDADD]], i64* %[[END:.+]], align 8

// CHECK: %[[INITITER:.+]] = load i64, i64* %[[INIT]]
// CHECK-NEXT: %[[BEGINITER:.+]] = load i64, i64* %[[BEGIN]]
// CHECK-NEXT: %[[ITERMUL:.+]] = mul i64 %[[BEGINITER]], 1
// CHECK-NEXT: %[[ITERADD:.+]] = add i64 %[[INITITER]], %[[ITERMUL]]
// CHECK-NEXT: detach within %[[SYNCREG:.+]], label %[[DETACHED:.+]], label %[[PFORINC:.+]]

// CHECK: [[DETACHED]]:
// CHECK: %[[ITERSLOT:.+]] = alloca i64, align 8
// CHECK: store i64 %[[ITERADD]], i64* %[[ITERSLOT]]

// CHECK: [[PFORINC]]:
// CHECK-NEXT: %[[INCBEGIN:.+]] = load i64, i64* %[[BEGIN]]
// CHECK-NEXT: %[[INC:.+]] = add i64 %[[INCBEGIN]], 1
// CHECK-NEXT: store i64 %[[INC]], i64* %[[BEGIN]]
// CHECK-NEXT: %[[CONDBEGIN:.+]] = load i64, i64* %[[BEGIN]]
// CHECK-NEXT: %[[CONDEND:.+]] = load i64, i64* %[[END]]
// CHECK-NEXT: %[[COND:.+]] = icmp ult i64 %[[CONDBEGIN]], %[[CONDEND]]
// CHECK-NEXT: br i1 %[[COND]], label %{{.+}}, label %[[PFORCONDCLEANUP:.+]], !llvm.loop ![[LOOPMD:.+]]

// CHECK: [[PFORCONDCLEANUP]]:
// CHECK-NEXT: sync within %[[SYNCREG]]
