// RUN: %clang_cc1 %s -std=c++11 -triple x86_64-unknown-linux-gnu -fcilkplus -ftapir=none -verify -S -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

namespace X {
struct C {
  C();
  struct It {
    int operator-(It &);
    It operator+(int);
    It operator++();
    It operator--();
    int operator*();
    bool operator!=(It &);
  };
  It begin() {
    return It();
  }
  It end();
};

}

void bar(int i);

void up(X::C c) {
  _Cilk_for (int x : c)
    bar(x);
}



// CHECK-LABEL: define void @_Z2upN1X1CE(

// CHECK: %c = alloca %"struct.X::C", align 1
// CHECK-NEXT: %syncreg = call token @llvm.syncregion.start()
// CHECK-NEXT: %__range1 = alloca %"struct.X::C"*, align 8
// CHECK-NEXT: %__begin1 = alloca %"struct.X::C::It", align 1
// CHECK-NEXT: %undef.agg.tmp = alloca %"struct.X::C::It", align 1
// CHECK-NEXT: %__end1 = alloca %"struct.X::C::It", align 1
// CHECK-NEXT: %undef.agg.tmp1 = alloca %"struct.X::C::It", align 1
// CHECK-NEXT: %__cilk_loopindex = alloca i32, align 4
// CHECK-NEXT: %__cilk_looplimit = alloca i32, align 4
// CHECK-NEXT: store %"struct.X::C"* %c, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: %0 = load %"struct.X::C"*, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: call void @_ZN1X1C5beginEv(%"struct.X::C"* %0)
// CHECK-NEXT: %1 = load %"struct.X::C"*, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: call void @_ZN1X1C3endEv(%"struct.X::C"* %1)
// CHECK-NEXT: store i32 0, i32* %__cilk_loopindex, align 4
// CHECK-NEXT: %call = call i32 @_ZN1X1C2ItmiERS1_(%"struct.X::C::It"* %__end1, %"struct.X::C::It"* dereferenceable(1) %__begin1)
// CHECK-NEXT: store i32 %call, i32* %__cilk_looplimit, align 4
// CHECK-NEXT: br label %[[PFORCOND:.+]]

// CHECK: %__range1 = alloca %"struct.X::
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
