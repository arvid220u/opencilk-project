//===- StmtCilk.h - Classes for Cilk statements -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// This file defines Cilk AST classes for executable statements and clauses.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_STMTCILK_H
#define LLVM_CLANG_AST_STMTCILK_H

#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {

/// CilkSpawnStmt - This represents a _Cilk_spawn.
///
class CilkSpawnStmt : public Stmt {
  SourceLocation SpawnLoc;
  Stmt *SpawnedStmt;

public:
  explicit CilkSpawnStmt(SourceLocation SL) : CilkSpawnStmt(SL, nullptr) {}

  CilkSpawnStmt(SourceLocation SL, Stmt *S)
      : Stmt(CilkSpawnStmtClass), SpawnLoc(SL), SpawnedStmt(S) { }

  // \brief Build an empty _Cilk_spawn statement.
  explicit CilkSpawnStmt(EmptyShell Empty) : Stmt(CilkSpawnStmtClass, Empty) {}

  const Stmt *getSpawnedStmt() const;
  Stmt *getSpawnedStmt();
  void setSpawnedStmt(Stmt *S) { SpawnedStmt = S; }

  SourceLocation getSpawnLoc() const { return SpawnLoc; }
  void setSpawnLoc(SourceLocation L) { SpawnLoc = L; }

  SourceLocation getBeginLoc() const LLVM_READONLY { return SpawnLoc; }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return SpawnedStmt->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CilkSpawnStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SpawnedStmt, &SpawnedStmt+1);
  }
};

/// CilkSyncStmt - This represents a _Cilk_sync.
///
class CilkSyncStmt : public Stmt {
  SourceLocation SyncLoc;

public:
  CilkSyncStmt(SourceLocation SL) : Stmt(CilkSyncStmtClass) {
    setSyncLoc(SL);
  }

  // Build an empty _Cilk_sync statement.
  explicit CilkSyncStmt(EmptyShell Empty) : Stmt(CilkSyncStmtClass, Empty) { }

  SourceLocation getSyncLoc() const { return SyncLoc; }
  void setSyncLoc(SourceLocation L) { SyncLoc = L; }

  SourceLocation getBeginLoc() const LLVM_READONLY { return getSyncLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY { return getSyncLoc(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CilkSyncStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(child_iterator(), child_iterator());
  }
};

/// CilkForRangeStmt - This represents a '_Cilk_for(range-declarator :
/// range-expression)' or a '_Cilk_for (init-statement range-declarator :
/// range-expression)', based on a CXXForRangeStmt which is a C++0x
/// [stmt.ranged]'s ranged for stmt
///
/// This is stored as a FORRANGE stmt embedded inside a CILKFORRANGE with some
/// other necessary semantic components.
class CilkForRangeStmt : public Stmt {
  enum {
    FORRANGE,
    LOOPINDEX,
    LOOPINDEXSTMT,
    LOCALLOOPINDEX,
    LIMIT,
    COND,
    INC,
    END
  };
  Stmt *SubExprs[END];

public:
  CilkForRangeStmt(const ASTContext &C, CXXForRangeStmt *ForRange,
                   VarDecl *LoopIndex, DeclStmt *LocalLoopIndex,
                   DeclStmt *Limit, Expr *Cond, Expr *Inc,
                   DeclStmt *LoopIndexStmt);

  /// \brief Build an empty cilk for range statement.
  explicit CilkForRangeStmt(EmptyShell Empty)
      : Stmt(CilkForRangeStmtClass, Empty) {}

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CilkForRangeStmtClass;
  }

  CXXForRangeStmt *getCXXForRangeStmt() const;

  void setForRange(Stmt *S) { SubExprs[FORRANGE] = S; }

  VarDecl *getLoopIndex() const;
  void setLoopIndex(const ASTContext &C, VarDecl *V);

  VarDecl *getLocalLoopIndex();
  const VarDecl *getLocalLoopIndex() const;

  Expr *getCond() { return reinterpret_cast<Expr *>(SubExprs[COND]); }
  Expr *getInc() { return reinterpret_cast<Expr *>(SubExprs[INC]); }
  DeclStmt *getLoopIndexStmt() {
    return cast_or_null<DeclStmt>(SubExprs[LOOPINDEXSTMT]);
  }
  DeclStmt *getLimitStmt() { return cast_or_null<DeclStmt>(SubExprs[LIMIT]); }
  DeclStmt *getLocalLoopIndexStmt() {
    return cast<DeclStmt>(SubExprs[LOCALLOOPINDEX]);
  }

  const Expr *getCond() const {
    return reinterpret_cast<Expr *>(SubExprs[COND]);
  }
  const Expr *getInc() const { return reinterpret_cast<Expr *>(SubExprs[INC]); }
  const DeclStmt *getLoopIndexStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[LOOPINDEXSTMT]);
  }
  const DeclStmt *getLimitStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[LIMIT]);
  }
  const DeclStmt *getLocalLoopIndexStmt() const {
    return cast<DeclStmt>(SubExprs[LOCALLOOPINDEX]);
  }

  SourceLocation getBeginLoc() const LLVM_READONLY;
  SourceLocation getEndLoc() const LLVM_READONLY;

  // Iterators
  child_range children() { return child_range(&SubExprs[0], &SubExprs[END]); }
};

/// CilkForStmt - This represents a '_Cilk_for(init;cond;inc)' stmt.
class CilkForStmt : public Stmt {
  SourceLocation CilkForLoc;
  enum { INIT, LIMIT, INITCOND, BEGINSTMT, ENDSTMT, COND, INC, LOOPVAR, BODY, END };
  Stmt* SubExprs[END]; // SubExprs[INIT] is an expression or declstmt.
  SourceLocation LParenLoc, RParenLoc;

public:
  CilkForStmt(const ASTContext &C, Stmt *Init, DeclStmt *Limit, Expr *InitCond,
              DeclStmt *Begin, DeclStmt *End, Expr *Cond, Expr *Inc,
              VarDecl *LoopVar, Stmt *Body, SourceLocation CFL,
              SourceLocation LP, SourceLocation RP);

  /// \brief Build an empty for statement.
  explicit CilkForStmt(EmptyShell Empty) : Stmt(CilkForStmtClass, Empty) { }

  Stmt *getInit() { return SubExprs[INIT]; }

  // /// \brief Retrieve the variable declared in this "for" statement, if any.
  // ///
  // /// In the following example, "y" is the condition variable.
  // /// \code
  // /// for (int x = random(); int y = mangle(x); ++x) {
  // ///   // ...
  // /// }
  // /// \endcode
  // VarDecl *getConditionVariable() const;
  // void setConditionVariable(const ASTContext &C, VarDecl *V);

  // /// If this CilkForStmt has a condition variable, return the faux DeclStmt
  // /// associated with the creation of that condition variable.
  // const DeclStmt *getConditionVariableDeclStmt() const {
  //   return reinterpret_cast<DeclStmt*>(SubExprs[CONDVAR]);
  // }

  VarDecl *getLoopVariable() const;
  void setLoopVariable(const ASTContext &C, VarDecl *V);
  DeclStmt *getLimitStmt() {
    return cast_or_null<DeclStmt>(SubExprs[LIMIT]);
  }
  Expr *getInitCond() { return cast_or_null<Expr>(SubExprs[INITCOND]); }
  DeclStmt *getBeginStmt() {
    return cast_or_null<DeclStmt>(SubExprs[BEGINSTMT]);
  }
  DeclStmt *getEndStmt() { return cast_or_null<DeclStmt>(SubExprs[ENDSTMT]); }
  Expr *getCond() { return reinterpret_cast<Expr*>(SubExprs[COND]); }
  Expr *getInc()  { return reinterpret_cast<Expr*>(SubExprs[INC]); }
  Stmt *getBody() { return SubExprs[BODY]; }

  const Stmt *getInit() const { return SubExprs[INIT]; }
  const DeclStmt *getLimitStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[LIMIT]);
  }
  const Expr *getInitCond() const {
    return cast_or_null<Expr>(SubExprs[INITCOND]);
  }
  const DeclStmt *getBeginStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[BEGINSTMT]);
  }
  const DeclStmt *getEndStmt() const {
    return cast_or_null<DeclStmt>(SubExprs[ENDSTMT]);
  }
  const Expr *getCond() const { return reinterpret_cast<Expr*>(SubExprs[COND]);}
  const Expr *getInc()  const { return reinterpret_cast<Expr*>(SubExprs[INC]); }
  const DeclStmt *getLoopVarDecl() const {
    return reinterpret_cast<DeclStmt*>(SubExprs[LOOPVAR]);
  }
  const Stmt *getBody() const { return SubExprs[BODY]; }

  void setInit(Stmt *S) { SubExprs[INIT] = S; }
  void setLimitStmt(Stmt *S) { SubExprs[LIMIT] = S; }
  void setInitCond(Expr *E) { SubExprs[INITCOND] = reinterpret_cast<Stmt*>(E); }
  void setBeginStmt(Stmt *S) { SubExprs[BEGINSTMT] = S; }
  void setEndStmt(Stmt *S) { SubExprs[ENDSTMT] = S; }
  void setCond(Expr *E) { SubExprs[COND] = reinterpret_cast<Stmt*>(E); }
  void setInc(Expr *E) { SubExprs[INC] = reinterpret_cast<Stmt*>(E); }
  void setBody(Stmt *S) { SubExprs[BODY] = S; }

  SourceLocation getCilkForLoc() const { return CilkForLoc; }
  void setCilkForLoc(SourceLocation L) { CilkForLoc = L; }
  SourceLocation getLParenLoc() const { return LParenLoc; }
  void setLParenLoc(SourceLocation L) { LParenLoc = L; }
  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getBeginLoc() const LLVM_READONLY { return getCilkForLoc(); }
  SourceLocation getEndLoc() const LLVM_READONLY {
    return getBody()->getEndLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CilkForStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[END]);
  }
};

}  // end namespace clang

#endif
