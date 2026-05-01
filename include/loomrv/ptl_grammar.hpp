/*
 * Copyright (c) 2019-2023 Dogan Ulus
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
 
#pragma once

namespace loomrv {

struct ptl_grammar {

  static constexpr auto grammar = R"(
    Expression  <- Implicative
    Implicative <- Disjunctive (LIMPLIES Disjunctive)?
    Disjunctive <- Conjunctive (LOR Conjunctive)*
    Conjunctive <- SinceExpr (LAND SinceExpr)*
    SinceExpr <- Unary (SINCE Bound? Unary)?
    Unary <- NotExpr / TimedOnceExpr / OnceExpr / TimedHistExpr / HistExpr / Atom / '(' Expression ')'
    NotExpr  <- LNOT Atom / LNOT '(' Expression ')'
    OnceExpr <- ONCE Atom / ONCE '(' Expression ')'
    HistExpr <- HIST Atom / HIST '(' Expression ')'
    TimedOnceExpr <- ONCE Bound Atom / ONCE Bound '(' Expression ')'
    TimedHistExpr <- HIST Bound Atom / HIST Bound '(' Expression ')'

    Atom <- LCURLY Name RCURLY 
              
    Bound <- FullBound / LowerBound / UpperBound
    FullBound <-  "[" Number ":" Number "]"
    LowerBound <- "[" Number ":" 'inf'? "]"
    UpperBound <- "["        ":" Number "]"

    Name   <- <[_a-zA-Z][_a-zA-Z0-9]*>
    Number <- <'-'? [0-9]+ ('.' [0-9]+)?>


    ~HIST  <- < 'H' / 'historically' >
    ~ONCE  <- < 'P' / 'once' >
    ~SINCE <- < 'S' / 'since' >
            
    ~LOR      <- < "||" / 'or' >
    ~LAND     <- < "&&" / 'and' >
    ~LNOT     <- < "!"  / 'not' >
    ~LIMPLIES <- < "->" / 'implies' >
    ~LCURLY <- < '{' >
    ~RCURLY <- < '}' >

    ~TRUE <- < 'true' >
    ~FALSE <- < 'false' >
            
    %whitespace <- [  \t\r\n]*
    )";
};

} // namespace loomrv