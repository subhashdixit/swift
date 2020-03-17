//===--- ParseRequests.h - Parsing Requests ---------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file defines parsing requests.
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_PARSE_REQUESTS_H
#define SWIFT_PARSE_REQUESTS_H

#include "swift/AST/ASTTypeIDs.h"
#include "swift/AST/SimpleRequest.h"

namespace swift {

/// Report that a request of the given kind is being evaluated, so it
/// can be recorded by the stats reporter.
template<typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

struct FingerprintAndMembers {
  Optional<std::string> fingerprint = None;
  ArrayRef<Decl *> members = {};
  bool operator==(const FingerprintAndMembers &x) const {
    return fingerprint == x.fingerprint && members == x.members;
  }
};

void simple_display(llvm::raw_ostream &out, const FingerprintAndMembers &value);

/// Parse the members of a nominal type declaration or extension.
/// Return a fingerprint and the members.
class ParseMembersRequest
    : public SimpleRequest<ParseMembersRequest,
                           FingerprintAndMembers(IterableDeclContext *),
                           CacheKind::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  FingerprintAndMembers evaluate(Evaluator &evaluator,
                                 IterableDeclContext *idc) const;

public:
  // Caching
  bool isCached() const { return true; }
};

/// Parse the body of a function, initializer, or deinitializer.
class ParseAbstractFunctionBodyRequest :
    public SimpleRequest<ParseAbstractFunctionBodyRequest,
                         BraceStmt *(AbstractFunctionDecl *),
                         CacheKind::SeparatelyCached>
{
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  BraceStmt *evaluate(Evaluator &evaluator, AbstractFunctionDecl *afd) const;

public:
  // Caching
  bool isCached() const { return true; }
  Optional<BraceStmt *> getCachedResult() const;
  void cacheResult(BraceStmt *value) const;
};

/// Parse the top-level decls of a SourceFile.
class ParseSourceFileRequest
    : public SimpleRequest<ParseSourceFileRequest,
                           ArrayRef<Decl *>(SourceFile *),
                           CacheKind::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ArrayRef<Decl *> evaluate(Evaluator &evaluator, SourceFile *SF) const;

public:
  // Caching.
  bool isCached() const { return true; }
  Optional<ArrayRef<Decl *>> getCachedResult() const;
  void cacheResult(ArrayRef<Decl *> decls) const;
};

/// The zone number for the parser.
#define SWIFT_TYPEID_ZONE Parse
#define SWIFT_TYPEID_HEADER "swift/AST/ParseTypeIDZone.def"
#include "swift/Basic/DefineTypeIDZone.h"
#undef SWIFT_TYPEID_ZONE
#undef SWIFT_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define SWIFT_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
  template <>                                                                  \
  inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,              \
                                     const RequestType &request) {             \
    ++stats.getFrontendCounters().RequestType;                                 \
  }
#include "swift/AST/ParseTypeIDZone.def"
#undef SWIFT_REQUEST

} // end namespace swift

#endif // SWIFT_PARSE_REQUESTS_H