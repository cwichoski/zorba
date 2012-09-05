
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C++ LALR(1) parser skeleton written by Akim Demaille.  */

#ifndef PARSER_HEADER_H
# define PARSER_HEADER_H

/* "%code requires" blocks.  */

/* Line 35 of lalr1.cc  */
#line 35 "/home/markos/zorba/repo/expr-memman2/src/compiler/parser/xquery_parser.y"


/*
 * Copyright 2006-2008 The FLWOR Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <list>
#include <string>
#include <vector>

#include "compiler/parsetree/parsenodes.h"
#include "zorbatypes/zstring.h"

#ifdef __GNUC__
    // disable a warning in location.hh which comes with bison
    // position.hh:141: warning: suggest parentheses around && within ||
#   pragma GCC diagnostic ignored "-Wparentheses"
#endif

#include "location.hh"

#ifdef __GNUC__
#  pragma GCC diagnostic warning "-Wparentheses"
#endif

typedef std::list<zorba::zstring> string_list_t;
typedef std::pair<zorba::zstring,zorba::zstring> string_pair_t;





/* Line 35 of lalr1.cc  */
#line 90 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"


#include <string>
#include <iostream>
#include "stack.hh"


/* Line 35 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace zorba {

/* Line 35 of lalr1.cc  */
#line 104 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"
  class position;
  class location;

/* Line 35 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // zorba

/* Line 35 of lalr1.cc  */
#line 114 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"

#include "location.hh"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
do {							\
  if (N)						\
    {							\
      (Current).begin = (Rhs)[1].begin;			\
      (Current).end   = (Rhs)[N].end;			\
    }							\
  else							\
    {							\
      (Current).begin = (Current).end = (Rhs)[0].end;	\
    }							\
} while (false)
#endif


/* Line 35 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace zorba {

/* Line 35 of lalr1.cc  */
#line 162 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"

  /// A Bison parser.
  class xquery_parser
  {
  public:
    /// Symbol semantic values.
#ifndef YYSTYPE
    union semantic_type
    {

/* Line 35 of lalr1.cc  */
#line 155 "/home/markos/zorba/repo/expr-memman2/src/compiler/parser/xquery_parser.y"

    zorba::parsenode *node;
    zorba::exprnode *expr;
    zorba::FunctionSig *fnsig;
    zorba::VarNameAndType *varnametype;
    off_t sval;
    const char *strval;
    xs_integer *ival;
    xs_double *dval;
    xs_decimal *decval;
    ZorbaParserError *err;
    string_list_t *strlist;
    string_pair_t *strpair;
    std::vector<string_pair_t> *vstrpair;
    CatchExpr::NameTestList *name_test_list;
    FTStopWordOption::incl_excl_list_t *incl_excl_list;
    FTSelection::pos_filter_list_t *pos_filter_list;
    FTThesaurusOption::thesaurus_id_list_t *thesaurus_id_list;
    ft_anyall_mode::type ft_anyall_value;



/* Line 35 of lalr1.cc  */
#line 198 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"
    };
#else
    typedef YYSTYPE semantic_type;
#endif
    /// Symbol locations.
    typedef location location_type;
    /// Tokens.
    struct token
    {
      /* Tokens.  */
   enum yytokentype {
     _EOF = 0,
     UNRECOGNIZED = 258,
     APOS_ATTR_CONTENT = 260,
     CHAR_LITERAL = 261,
     CHAR_LITERAL_AND_CDATA_END = 262,
     CHAR_LITERAL_AND_PI_END = 263,
     CHAR_REF_LITERAL = 264,
     ELEMENT_CONTENT = 265,
     ELEM_WILDCARD = 266,
     ELEM_EQNAME_WILDCARD = 267,
     ENTITY_REF = 268,
     EXPR_COMMENT_LITERAL = 269,
     PI_NCNAME_LBRACE = 270,
     NCNAME_SVAL = 271,
     PRAGMA_LITERAL_AND_END_PRAGMA = 272,
     QNAME_SVAL_AND_END_PRAGMA = 273,
     EQNAME_SVAL_AND_END_PRAGMA = 274,
     PREFIX_WILDCARD = 275,
     COMP_ELEMENT_QNAME_LBRACE = 276,
     COMP_ATTRIBUTE_QNAME_LBRACE = 277,
     COMP_PI_NCNAME_LBRACE = 278,
     QNAME_SVAL = 279,
     EQNAME_SVAL = 280,
     ANNOTATION_QNAME_SVAL = 281,
     ANNOTATION_EQNAME_SVAL = 282,
     QUOTE_ATTR_CONTENT = 283,
     STRING_LITERAL = 284,
     XML_COMMENT_LITERAL = 285,
     DECLARE = 288,
     MODULE = 289,
     BLANK = 290,
     ALLOWING = 291,
     BASE_URI = 292,
     BOUNDARY_SPACE = 293,
     COMMENT = 294,
     CONSTRUCTION = 295,
     COPY_NAMESPACES = 296,
     COUNT = 297,
     DOCUMENT = 298,
     DOCUMENT_NODE = 299,
     ELEMENT = 300,
     FOR = 301,
     FUNCTION = 302,
     IF = 303,
     IMPORT = 304,
     INSTANCE = 305,
     LET = 306,
     MOST = 307,
     NEXT = 308,
     NO = 309,
     ONLY = 310,
     OPTION = 311,
     ORDERING = 312,
     PREVIOUS = 313,
     PROCESSING_INSTRUCTION = 314,
     SCHEMA = 315,
     SCHEMA_ATTRIBUTE = 316,
     SCHEMA_ELEMENT = 317,
     SEQUENTIAL = 318,
     SET = 319,
     SIMPLE = 320,
     SLIDING = 321,
     SOME = 322,
     SPACE = 323,
     STABLE = 324,
     TEXT = 325,
     TUMBLING = 326,
     SWITCH = 327,
     TYPESWITCH = 328,
     UPDATING = 329,
     VALIDATE = 330,
     TYPE = 331,
     WHEN = 332,
     WORD = 333,
     DECIMAL_FORMAT = 334,
     DECIMAL_SEPARATOR = 335,
     GROUPING_SEPARATOR = 336,
     INFINITY_VALUE = 337,
     MINUS_SIGN = 338,
     NaN = 339,
     PERCENT = 340,
     PER_MILLE = 341,
     ZERO_DIGIT = 342,
     DIGIT = 343,
     PATTERN_SEPARATOR = 344,
     ANCESTOR = 345,
     ANCESTOR_OR_SELF = 346,
     AND = 347,
     APOS = 348,
     AS = 349,
     ASCENDING = 350,
     AT = 351,
     ATTRIBUTE = 352,
     AT_SIGN = 353,
     CONCAT = 354,
     CASE = 355,
     CASTABLE = 356,
     CAST = 357,
     CDATA_BEGIN = 358,
     CDATA_END = 359,
     CHILD = 360,
     COLLATION = 361,
     COMMA = 362,
     COMMENT_BEGIN = 363,
     COMMENT_END = 364,
     DECIMAL_LITERAL = 365,
     CONTEXT = 366,
     VARIABLE = 367,
     DEFAULT = 368,
     DESCENDANT = 369,
     DESCENDANT_OR_SELF = 370,
     DESCENDING = 371,
     DIV = 372,
     DOLLAR = 373,
     DOT = 374,
     DOT_DOT = 375,
     COLON = 376,
     DOUBLE_COLON = 377,
     DOUBLE_LBRACE = 378,
     DOUBLE_LITERAL = 379,
     DOUBLE_RBRACE = 380,
     ELSE = 381,
     _EMPTY = 382,
     GREATEST = 383,
     LEAST = 384,
     EMPTY_TAG_END = 385,
     ENCODING = 386,
     EQUALS = 387,
     ESCAPE_APOS = 388,
     ESCAPE_QUOTE = 389,
     EVERY = 390,
     EXCEPT = 391,
     EXTERNAL = 392,
     FOLLOWING = 393,
     FOLLOWING_SIBLING = 394,
     FOLLOWS = 395,
     GE = 396,
     GETS = 397,
     GT = 398,
     HOOK = 399,
     HASH = 400,
     IDIV = 401,
     _IN = 402,
     INHERIT = 403,
     INTEGER_LITERAL = 404,
     INTERSECT = 405,
     IS = 406,
     ITEM = 407,
     LBRACE = 408,
     LBRACK = 409,
     LE = 410,
     LPAR = 411,
     LT_OR_START_TAG = 412,
     MINUS = 413,
     MOD = 414,
     NAMESPACE = 415,
     _NAN = 416,
     NE = 417,
     NODECOMP = 418,
     NOT_OPERATOR_KEYWORD = 419,
     NO_INHERIT = 420,
     NO_PRESERVE = 421,
     OR = 422,
     ORDER = 423,
     ORDERED = 424,
     BY = 425,
     GROUP = 426,
     PARENT = 427,
     PERCENTAGE = 428,
     PI_BEGIN = 429,
     PI_END = 430,
     PLUS = 431,
     PRAGMA_BEGIN = 432,
     PRAGMA_END = 433,
     PRECEDES = 434,
     PRECEDING = 435,
     PRECEDING_SIBLING = 436,
     PRESERVE = 437,
     QUOTE = 438,
     RBRACE = 439,
     RBRACK = 440,
     RETURN = 441,
     RPAR = 442,
     SATISFIES = 443,
     SELF = 444,
     SEMI = 445,
     SLASH = 446,
     SLASH_SLASH = 447,
     STAR = 448,
     START_TAG_END = 449,
     STRIP = 450,
     TAG_END = 451,
     THEN = 452,
     TO = 453,
     TREAT = 454,
     UNION = 455,
     UNORDERED = 456,
     VAL_EQ = 457,
     VAL_GE = 458,
     VAL_GT = 459,
     VAL_LE = 460,
     VAL_LT = 461,
     VAL_NE = 462,
     VALUECOMP = 463,
     VBAR = 464,
     EMPTY_SEQUENCE = 465,
     WHERE = 466,
     XML_COMMENT_BEGIN = 467,
     XML_COMMENT_END = 468,
     XQUERY = 469,
     VERSION = 470,
     START = 471,
     AFTER = 472,
     BEFORE = 473,
     REVALIDATION = 474,
     _STRICT = 475,
     LAX = 476,
     SKIP = 477,
     _DELETE = 478,
     NODE = 479,
     INSERT = 480,
     NODES = 481,
     RENAME = 482,
     REPLACE = 483,
     VALUE = 484,
     OF = 485,
     FIRST = 486,
     INTO = 487,
     LAST = 488,
     MODIFY = 489,
     COPY = 490,
     WITH = 491,
     BREAK = 492,
     CONTINUE = 493,
     EXIT = 494,
     LOOP = 495,
     RETURNING = 496,
     WHILE = 497,
     TRY = 498,
     CATCH = 499,
     USING = 500,
     ALL = 501,
     ANY = 502,
     CONTAINS = 503,
     CONTENT = 504,
     DIACRITICS = 505,
     DIFFERENT = 506,
     DISTANCE = 507,
     END = 508,
     ENTIRE = 509,
     EXACTLY = 510,
     FROM = 511,
     FTAND = 512,
     FTNOT = 513,
     NOT = 514,
     FT_OPTION = 515,
     FTOR = 516,
     INSENSITIVE = 517,
     LANGUAGE = 518,
     LEVELS = 519,
     LOWERCASE = 520,
     OCCURS = 521,
     PARAGRAPH = 522,
     PARAGRAPHS = 523,
     PHRASE = 524,
     RELATIONSHIP = 525,
     SAME = 526,
     SCORE = 527,
     SENSITIVE = 528,
     SENTENCE = 529,
     SENTENCES = 530,
     STEMMING = 531,
     STOP = 532,
     THESAURUS = 533,
     TIMES = 534,
     UPPERCASE = 535,
     WEIGHT = 536,
     WILDCARDS = 537,
     WINDOW = 538,
     WITHOUT = 539,
     WORDS = 540,
     COLLECTION = 541,
     CONSTOPT = 542,
     APPEND_ONLY = 543,
     QUEUE = 544,
     MUTABLE = 545,
     READ_ONLY = 546,
     UNIQUE = 547,
     NON = 548,
     INDEX = 549,
     MANUALLY = 550,
     AUTOMATICALLY = 551,
     MAINTAINED = 552,
     ON = 553,
     RANGE = 554,
     EQUALITY = 555,
     GENERAL = 556,
     INTEGRITY = 557,
     CONSTRAINT = 558,
     CHECK = 559,
     KEY = 560,
     FOREACH = 561,
     FOREIGN = 562,
     KEYS = 563,
     L_SIMPLE_OBJ_UNION = 564,
     R_SIMPLE_OBJ_UNION = 565,
     L_ACCUMULATOR_OBJ_UNION = 566,
     R_ACCUMULATOR_OBJ_UNION = 567,
     JSON = 568,
     APPEND = 569,
     POSITION = 570,
     OBJECT = 571,
     ARRAY = 572,
     JSON_ITEM = 573,
     STRUCTURED_ITEM = 574,
     BYTE_ORDER_MARK_UTF8 = 575,
     SHEBANG = 576,
     RANGE_REDUCE = 577,
     SEQUENCE_TYPE_REDUCE = 578,
     ADDITIVE_REDUCE = 579,
     UNION_REDUCE = 580,
     INTERSECT_EXCEPT_REDUCE = 581,
     QVARINDECLLIST_REDUCE = 582,
     COMMA_DOLLAR = 583,
     UNARY_PREC = 584,
     STEP_REDUCE = 585,
     MULTIPLICATIVE_REDUCE = 586
   };

    };
    /// Token type.
    typedef token::yytokentype token_type;

    /// Build a parser object.
    xquery_parser (xquery_driver& driver_yyarg);
    virtual ~xquery_parser ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

  private:
    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Generate an error message.
    /// \param state   the state where the error occurred.
    /// \param tok     the lookahead token.
    virtual std::string yysyntax_error_ (int yystate, int tok);

#if YYDEBUG
    /// \brief Report a symbol value on the debug stream.
    /// \param yytype       The token type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    virtual void yy_symbol_value_print_ (int yytype,
					 const semantic_type* yyvaluep,
					 const location_type* yylocationp);
    /// \brief Report a symbol on the debug stream.
    /// \param yytype       The token type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    virtual void yy_symbol_print_ (int yytype,
				   const semantic_type* yyvaluep,
				   const location_type* yylocationp);
#endif


    /// State numbers.
    typedef int state_type;
    /// State stack type.
    typedef stack<state_type>    state_stack_type;
    /// Semantic value stack type.
    typedef stack<semantic_type> semantic_stack_type;
    /// location stack type.
    typedef stack<location_type> location_stack_type;

    /// The state stack.
    state_stack_type yystate_stack_;
    /// The semantic value stack.
    semantic_stack_type yysemantic_stack_;
    /// The location stack.
    location_stack_type yylocation_stack_;

    /// Internal symbol numbers.
    typedef unsigned short int token_number_type;
    /* Tables.  */
    /// For a state, the index in \a yytable_ of its portion.
    static const short int yypact_[];
    static const short int yypact_ninf_;

    /// For a state, default rule to reduce.
    /// Unless\a  yytable_ specifies something else to do.
    /// Zero means the default is an error.
    static const unsigned short int yydefact_[];

    static const short int yypgoto_[];
    static const short int yydefgoto_[];

    /// What to do in a state.
    /// \a yytable_[yypact_[s]]: what to do in state \a s.
    /// - if positive, shift that token.
    /// - if negative, reduce the rule which number is the opposite.
    /// - if zero, do what YYDEFACT says.
    static const short int yytable_[];
    static const short int yytable_ninf_;

    static const short int yycheck_[];

    /// For a state, its accessing symbol.
    static const unsigned short int yystos_[];

    /// For a rule, its LHS.
    static const unsigned short int yyr1_[];
    /// For a rule, its RHS length.
    static const unsigned char yyr2_[];

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
    /// For a symbol, its name in clear.
    static const char* const yytname_[];
#endif

#if YYERROR_VERBOSE
    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    virtual std::string yytnamerr_ (const char *n);
#endif

#if YYDEBUG
    /// A type to store symbol numbers and -1.
    typedef short int rhs_number_type;
    /// A `-1'-separated list of the rules' RHS.
    static const rhs_number_type yyrhs_[];
    /// For each rule, the index of the first RHS symbol in \a yyrhs_.
    static const unsigned short int yyprhs_[];
    /// For each rule, its source line number.
    static const unsigned short int yyrline_[];
    /// For each scanner token number, its symbol number.
    static const unsigned short int yytoken_number_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r);
    /// Print the state stack on the debug stream.
    virtual void yystack_print_ ();

    /* Debugging.  */
    int yydebug_;
    std::ostream* yycdebug_;
#endif

    /// Convert a scanner token number \a t to a symbol number.
    token_number_type yytranslate_ (int t);

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg        Why this token is reclaimed.
    /// \param yytype       The symbol type.
    /// \param yyvaluep     Its semantic value.
    /// \param yylocationp  Its location.
    inline void yydestruct_ (const char* yymsg,
			     int yytype,
			     semantic_type* yyvaluep,
			     location_type* yylocationp);

    /// Pop \a n symbols the three stacks.
    inline void yypop_ (unsigned int n = 1);

    /* Constants.  */
    static const int yyeof_;
    /* LAST_ -- Last index in TABLE_.  */
    static const int yylast_;
    static const int yynnts_;
    static const int yyempty_;
    static const int yyfinal_;
    static const int yyterror_;
    static const int yyerrcode_;
    static const int yyntokens_;
    static const unsigned int yyuser_token_number_max_;
    static const token_number_type yyundef_token_;

    /* User arguments.  */
    xquery_driver& driver;
  };

/* Line 35 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // zorba

/* Line 35 of lalr1.cc  */
#line 713 "/home/markos/zorba/repo/expr-memman2/build/src/compiler/parser/xquery_parser.hpp"



#endif /* ! defined PARSER_HEADER_H */
