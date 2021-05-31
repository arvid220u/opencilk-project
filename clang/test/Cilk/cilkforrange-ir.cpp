// RUN: %clang_cc1 %s -std=c++11 -triple x86_64-unknown-linux-gnu -fcilkplus -ftapir=none -verify -S -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

namespace X {
struct C {
  C();
  struct It {
    int value;
    int operator-(It &);
    It operator+(int);
    It operator++();
    It operator--();
    int& operator*();
    bool operator!=(It &);
  };
  It begin();
  It end();
};

}

void bar(int i);

void iterate(X::C c) {
  _Cilk_for (int x : c)
    bar(x);
}

// TODO: why does the @_ZN1X1C3endEv call return void? it feels like this IR is wrong
//  but it also seems like the IR for the corresponding normal for-range is wrong...

// CHECK-LABEL: define void @_Z2iterateN1X1CE(

// CHECK: %c = alloca %"struct.X::C", align 1
// CHECK-NEXT: %syncreg = call token @llvm.syncregion.start()
// CHECK-NEXT: %__range1 = alloca %"struct.X::C"*, align 8
// CHECK-NEXT: %__begin1 = alloca %"struct.X::C::It", align 4
// CHECK-NEXT: %__end1 = alloca %"struct.X::C::It", align 4
// CHECK-NEXT: %__cilk_loopindex = alloca i32, align 4
// CHECK-NEXT: %__cilk_looplimit = alloca i32, align 4
// CHECK-NEXT: store %"struct.X::C"* %c, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: %0 = load %"struct.X::C"*, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: %call = call i32 @_ZN1X1C5beginEv(%"struct.X::C"* %0)
// CHECK-NEXT: %coerce.dive = getelementptr inbounds %"struct.X::C::It", %"struct.X::C::It"* %__begin1, i32 0, i32 0
// CHECK-NEXT: store i32 %call, i32* %coerce.dive, align 4
// CHECK-NEXT: %1 = load %"struct.X::C"*, %"struct.X::C"** %__range1, align 8
// CHECK-NEXT: %call1 = call i32 @_ZN1X1C3endEv(%"struct.X::C"* %1)
// CHECK-NEXT: %coerce.dive2 = getelementptr inbounds %"struct.X::C::It", %"struct.X::C::It"* %__end1, i32 0, i32 0
// CHECK-NEXT: store i32 %call1, i32* %coerce.dive2, align 4
// CHECK-NEXT: store i32 0, i32* %__cilk_loopindex, align 4
// CHECK-NEXT: %call3 = call i32 @_ZN1X1C2ItmiERS1_(%"struct.X::C::It"* %__end1, %"struct.X::C::It"* dereferenceable(1) %__begin1)
// CHECK-NEXT: store i32 %call3, i32* %__cilk_looplimit, align 4
// CHECK-NEXT: br label %[[PFORCOND:.+]]

// TODO: continue with the IR checks :))
//  useful commands:
//    ./clang++ -std=c++11 -fopencilk -ftapir=none -S -emit-llvm ../opencilk-project/clang/test/Cilk/cilkforrange-ir.cpp
//    cat cilkforrange-ir.ll | grep Z2upN1 -C 50

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

//void iterate_ref(X::C c) {
//  _Cilk_for (int& x : c)
//    bar(x);
//}
//
//void iterate_auto(X::C c) {
//  _Cilk_for (auto x : c)
//    bar(x);
//}
//
//void iterate_autoref(X::C c) {
//  _Cilk_for (auto& x : c)
//    bar(x);
//}
