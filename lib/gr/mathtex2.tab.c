/* A Bison parser, made by GNU Bison 3.8.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30800

/* Bison version string.  */
#define YYBISON_VERSION "3.8"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* First part of user prologue.  */
#line 11 "mathtex2.y"

#include "mathtex2.h"
#include "strlib.h"


#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

int yylex(void);
void yyerror(char const *);

#line 84 "mathtex2.tab.c"

#ifndef YY_CAST
#ifdef __cplusplus
#define YY_CAST(Type, Val) static_cast<Type>(Val)
#define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type>(Val)
#else
#define YY_CAST(Type, Val) ((Type)(Val))
#define YY_REINTERPRET_CAST(Type, Val) ((Type)(Val))
#endif
#endif
#ifndef YY_NULLPTR
#if defined __cplusplus
#if 201103L <= __cplusplus
#define YY_NULLPTR nullptr
#else
#define YY_NULLPTR 0
#endif
#else
#define YY_NULLPTR ((void *)0)
#endif
#endif


/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype
{
  YYEMPTY = -2,
  YYEOF = 0,            /* "end of file"  */
  YYerror = 256,        /* error  */
  YYUNDEF = 257,        /* "invalid token"  */
  SINGLE_SYMBOL = 258,  /* SINGLE_SYMBOL  */
  SUBSUPEROP = 259,     /* SUBSUPEROP  */
  APOSTROPHE = 260,     /* APOSTROPHE  */
  FRAC = 261,           /* FRAC  */
  DFRAC = 262,          /* DFRAC  */
  STACKREL = 263,       /* STACKREL  */
  BINOM = 264,          /* BINOM  */
  GENFRAC = 265,        /* GENFRAC  */
  SNOWFLAKE = 266,      /* SNOWFLAKE  */
  ACCENT = 267,         /* ACCENT  */
  UNKNOWN_SYMBOL = 268, /* UNKNOWN_SYMBOL  */
  FONT = 269,           /* FONT  */
  LATEXFONT = 270,      /* LATEXFONT  */
  LATEXTEXT = 271,      /* LATEXTEXT  */
  FUNCTION = 272,       /* FUNCTION  */
  C_OVER_C = 273,       /* C_OVER_C  */
  SPACE = 274,          /* SPACE  */
  HSPACE = 275,         /* HSPACE  */
  LEFT = 276,           /* LEFT  */
  RIGHT = 277,          /* RIGHT  */
  LEFT_DELIM = 278,     /* LEFT_DELIM  */
  AMBI_DELIM = 279,     /* AMBI_DELIM  */
  RIGHT_DELIM = 280,    /* RIGHT_DELIM  */
  LBRACE = 281,         /* LBRACE  */
  RBRACE = 282,         /* RBRACE  */
  PIPE = 283,           /* PIPE  */
  OPERATORNAME = 284,   /* OPERATORNAME  */
  OVERLINE = 285,       /* OVERLINE  */
  SQRT = 286,           /* SQRT  */
  DIGIT = 287,          /* DIGIT  */
  PLUSMINUS = 288       /* PLUSMINUS  */
};
typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef ParserNode YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse(void);


/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                 /* "end of file"  */
  YYSYMBOL_YYerror = 1,               /* error  */
  YYSYMBOL_YYUNDEF = 2,               /* "invalid token"  */
  YYSYMBOL_SINGLE_SYMBOL = 3,         /* SINGLE_SYMBOL  */
  YYSYMBOL_SUBSUPEROP = 4,            /* SUBSUPEROP  */
  YYSYMBOL_APOSTROPHE = 5,            /* APOSTROPHE  */
  YYSYMBOL_FRAC = 6,                  /* FRAC  */
  YYSYMBOL_DFRAC = 7,                 /* DFRAC  */
  YYSYMBOL_STACKREL = 8,              /* STACKREL  */
  YYSYMBOL_BINOM = 9,                 /* BINOM  */
  YYSYMBOL_GENFRAC = 10,              /* GENFRAC  */
  YYSYMBOL_SNOWFLAKE = 11,            /* SNOWFLAKE  */
  YYSYMBOL_ACCENT = 12,               /* ACCENT  */
  YYSYMBOL_UNKNOWN_SYMBOL = 13,       /* UNKNOWN_SYMBOL  */
  YYSYMBOL_FONT = 14,                 /* FONT  */
  YYSYMBOL_LATEXFONT = 15,            /* LATEXFONT  */
  YYSYMBOL_LATEXTEXT = 16,            /* LATEXTEXT  */
  YYSYMBOL_FUNCTION = 17,             /* FUNCTION  */
  YYSYMBOL_C_OVER_C = 18,             /* C_OVER_C  */
  YYSYMBOL_SPACE = 19,                /* SPACE  */
  YYSYMBOL_HSPACE = 20,               /* HSPACE  */
  YYSYMBOL_LEFT = 21,                 /* LEFT  */
  YYSYMBOL_RIGHT = 22,                /* RIGHT  */
  YYSYMBOL_LEFT_DELIM = 23,           /* LEFT_DELIM  */
  YYSYMBOL_AMBI_DELIM = 24,           /* AMBI_DELIM  */
  YYSYMBOL_RIGHT_DELIM = 25,          /* RIGHT_DELIM  */
  YYSYMBOL_LBRACE = 26,               /* LBRACE  */
  YYSYMBOL_RBRACE = 27,               /* RBRACE  */
  YYSYMBOL_PIPE = 28,                 /* PIPE  */
  YYSYMBOL_OPERATORNAME = 29,         /* OPERATORNAME  */
  YYSYMBOL_OVERLINE = 30,             /* OVERLINE  */
  YYSYMBOL_SQRT = 31,                 /* SQRT  */
  YYSYMBOL_DIGIT = 32,                /* DIGIT  */
  YYSYMBOL_PLUSMINUS = 33,            /* PLUSMINUS  */
  YYSYMBOL_34_ = 34,                  /* '['  */
  YYSYMBOL_35_ = 35,                  /* ']'  */
  YYSYMBOL_36_ = 36,                  /* '('  */
  YYSYMBOL_37_ = 37,                  /* ')'  */
  YYSYMBOL_38_ = 38,                  /* '|'  */
  YYSYMBOL_39_ = 39,                  /* '<'  */
  YYSYMBOL_40_ = 40,                  /* '>'  */
  YYSYMBOL_41_ = 41,                  /* '/'  */
  YYSYMBOL_42_ = 42,                  /* '.'  */
  YYSYMBOL_43_ = 43,                  /* '{'  */
  YYSYMBOL_44_ = 44,                  /* '}'  */
  YYSYMBOL_YYACCEPT = 45,             /* $accept  */
  YYSYMBOL_result = 46,               /* result  */
  YYSYMBOL_math = 47,                 /* math  */
  YYSYMBOL_single_symbol = 48,        /* single_symbol  */
  YYSYMBOL_token = 49,                /* token  */
  YYSYMBOL_simple = 50,               /* simple  */
  YYSYMBOL_customspace = 51,          /* customspace  */
  YYSYMBOL_subsuper = 52,             /* subsuper  */
  YYSYMBOL_auto_delim = 53,           /* auto_delim  */
  YYSYMBOL_ambi_delim_symbol = 54,    /* ambi_delim_symbol  */
  YYSYMBOL_left_delim_symbol = 55,    /* left_delim_symbol  */
  YYSYMBOL_right_delim_symbol = 56,   /* right_delim_symbol  */
  YYSYMBOL_left_delim = 57,           /* left_delim  */
  YYSYMBOL_right_delim = 58,          /* right_delim  */
  YYSYMBOL_auto_delim_inner = 59,     /* auto_delim_inner  */
  YYSYMBOL_accent = 60,               /* accent  */
  YYSYMBOL_placeable = 61,            /* placeable  */
  YYSYMBOL_genfrac = 62,              /* genfrac  */
  YYSYMBOL_overline = 63,             /* overline  */
  YYSYMBOL_operatorname = 64,         /* operatorname  */
  YYSYMBOL_operatorname_inner = 65,   /* operatorname_inner  */
  YYSYMBOL_sqrt = 66,                 /* sqrt  */
  YYSYMBOL_group = 67,                /* group  */
  YYSYMBOL_simple_group = 68,         /* simple_group  */
  YYSYMBOL_required_group = 69,       /* required_group  */
  YYSYMBOL_required_group_inner = 70, /* required_group_inner  */
  YYSYMBOL_start_group = 71,          /* start_group  */
  YYSYMBOL_group_inner = 72,          /* group_inner  */
  YYSYMBOL_frac = 73,                 /* frac  */
  YYSYMBOL_dfrac = 74,                /* dfrac  */
  YYSYMBOL_stackrel = 75,             /* stackrel  */
  YYSYMBOL_binom = 76,                /* binom  */
  YYSYMBOL_NT_FLOAT = 77,             /* NT_FLOAT  */
  YYSYMBOL_int = 78,                  /* int  */
  YYSYMBOL_uint = 79                  /* uint  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


#ifdef short
#undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
#include <limits.h> /* INFRINGES ON USER NAME SPACE */
#if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#define YY_STDINT_H
#endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
#undef UINT_LEAST8_MAX
#undef UINT_LEAST16_MAX
#define UINT_LEAST8_MAX 255
#define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
#if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#define YYPTRDIFF_T __PTRDIFF_TYPE__
#define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
#elif defined PTRDIFF_MAX
#ifndef ptrdiff_t
#include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#endif
#define YYPTRDIFF_T ptrdiff_t
#define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
#else
#define YYPTRDIFF_T long
#define YYPTRDIFF_MAXIMUM LONG_MAX
#endif
#endif

#ifndef YYSIZE_T
#ifdef __SIZE_TYPE__
#define YYSIZE_T __SIZE_TYPE__
#elif defined size_t
#define YYSIZE_T size_t
#elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#define YYSIZE_T size_t
#else
#define YYSIZE_T unsigned
#endif
#endif

#define YYSIZE_MAXIMUM \
  YY_CAST(YYPTRDIFF_T, (YYPTRDIFF_MAXIMUM < YY_CAST(YYSIZE_T, -1) ? YYPTRDIFF_MAXIMUM : YY_CAST(YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST(YYPTRDIFF_T, sizeof(X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
#if defined YYENABLE_NLS && YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#define YY_(Msgid) dgettext("bison-runtime", Msgid)
#endif
#endif
#ifndef YY_
#define YY_(Msgid) Msgid
#endif
#endif


#ifndef YY_ATTRIBUTE_PURE
#if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#define YY_ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define YY_ATTRIBUTE_PURE
#endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
#if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#define YY_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define YY_ATTRIBUTE_UNUSED
#endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if !defined lint || defined __GNUC__
#define YY_USE(E) ((void)(E))
#else
#define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && !defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
#if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
#else
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                            \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuninitialized\"") \
      _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#endif
#define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && !defined __ICC && 6 <= __GNUC__
#define YY_IGNORE_USELESS_CAST_BEGIN _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuseless-cast\"")
#define YY_IGNORE_USELESS_CAST_END _Pragma("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
#define YY_IGNORE_USELESS_CAST_BEGIN
#define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void)(0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#ifdef __GNUC__
#define YYSTACK_ALLOC __builtin_alloca
#elif defined __BUILTIN_VA_ARG_INCR
#include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#elif defined _AIX
#define YYSTACK_ALLOC __alloca
#elif defined _MSC_VER
#include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#define alloca _alloca
#else
#define YYSTACK_ALLOC alloca
#if !defined _ALLOCA_H && !defined EXIT_SUCCESS
#include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
/* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#endif
#endif
#endif

#ifdef YYSTACK_ALLOC
/* Pacify GCC's 'empty if-body' warning.  */
#define YYSTACK_FREE(Ptr) \
  do                      \
    { /* empty */         \
      ;                   \
    }                     \
  while (0)
#ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#endif
#else
#define YYSTACK_ALLOC YYMALLOC
#define YYSTACK_FREE YYFREE
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#endif
#if (defined __cplusplus && !defined EXIT_SUCCESS && \
     !((defined YYMALLOC || defined malloc) && (defined YYFREE || defined free)))
#include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#ifndef YYMALLOC
#define YYMALLOC malloc
#if !defined malloc && !defined EXIT_SUCCESS
void *malloc(YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#ifndef YYFREE
#define YYFREE free
#if !defined free && !defined EXIT_SUCCESS
void free(void *);      /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#endif
#endif /* !defined yyoverflow */

#if (!defined yyoverflow && (!defined __cplusplus || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
#define YYSTACK_GAP_MAXIMUM (YYSIZEOF(union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
#define YYSTACK_BYTES(N) ((N) * (YYSIZEOF(yy_state_t) + YYSIZEOF(YYSTYPE)) + YYSTACK_GAP_MAXIMUM)

#define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
#define YYSTACK_RELOCATE(Stack_alloc, Stack)                             \
  do                                                                     \
    {                                                                    \
      YYPTRDIFF_T yynewbytes;                                            \
      YYCOPY(&yyptr->Stack_alloc, Stack, yysize);                        \
      Stack = &yyptr->Stack_alloc;                                       \
      yynewbytes = yystacksize * YYSIZEOF(*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / YYSIZEOF(*yyptr);                            \
    }                                                                    \
  while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined __GNUC__ && 1 < __GNUC__
#define YYCOPY(Dst, Src, Count) __builtin_memcpy(Dst, Src, YY_CAST(YYSIZE_T, (Count)) * sizeof(*(Src)))
#else
#define YYCOPY(Dst, Src, Count)                                    \
  do                                                               \
    {                                                              \
      YYPTRDIFF_T yyi;                                             \
      for (yyi = 0; yyi < (Count); yyi++) (Dst)[yyi] = (Src)[yyi]; \
    }                                                              \
  while (0)
#endif
#endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL 88
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST 288

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS 45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS 35
/* YYNRULES -- Number of rules.  */
#define YYNRULES 102
/* YYNSTATES -- Number of states.  */
#define YYNSTATES 154

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK 288


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX) \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? YY_CAST(yysymbol_kind_t, yytranslate[YYX]) : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] = {
    0, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  36, 37, 2,  2,  2,  2,  42, 41, 2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 39, 2, 40, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 34, 2,  35, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  43, 38, 44, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2,
    2, 2, 2,  2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  1,  2,  3,  4, 5,
    6, 7, 8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] = {
    0,   60,  60,  65,  71,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,
    97,  98,  103, 106, 109, 116, 120, 123, 127, 130, 136, 144, 151, 162, 176, 180, 184, 188, 192, 199, 203,
    207, 211, 215, 219, 226, 230, 234, 238, 242, 246, 253, 262, 271, 279, 287, 298, 307, 311, 314, 318, 322,
    326, 329, 333, 336, 339, 342, 345, 348, 351, 354, 360, 383, 393, 403, 411, 419, 430, 439, 451, 461, 471,
    481, 487, 498, 504, 511, 519, 530, 548, 566, 584, 603, 604, 610, 616, 622, 631, 635, 643, 647};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST(yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name(yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] = {"\"end of file\"",
                                      "error",
                                      "\"invalid token\"",
                                      "SINGLE_SYMBOL",
                                      "SUBSUPEROP",
                                      "APOSTROPHE",
                                      "FRAC",
                                      "DFRAC",
                                      "STACKREL",
                                      "BINOM",
                                      "GENFRAC",
                                      "SNOWFLAKE",
                                      "ACCENT",
                                      "UNKNOWN_SYMBOL",
                                      "FONT",
                                      "LATEXFONT",
                                      "LATEXTEXT",
                                      "FUNCTION",
                                      "C_OVER_C",
                                      "SPACE",
                                      "HSPACE",
                                      "LEFT",
                                      "RIGHT",
                                      "LEFT_DELIM",
                                      "AMBI_DELIM",
                                      "RIGHT_DELIM",
                                      "LBRACE",
                                      "RBRACE",
                                      "PIPE",
                                      "OPERATORNAME",
                                      "OVERLINE",
                                      "SQRT",
                                      "DIGIT",
                                      "PLUSMINUS",
                                      "'['",
                                      "']'",
                                      "'('",
                                      "')'",
                                      "'|'",
                                      "'<'",
                                      "'>'",
                                      "'/'",
                                      "'.'",
                                      "'{'",
                                      "'}'",
                                      "$accept",
                                      "result",
                                      "math",
                                      "single_symbol",
                                      "token",
                                      "simple",
                                      "customspace",
                                      "subsuper",
                                      "auto_delim",
                                      "ambi_delim_symbol",
                                      "left_delim_symbol",
                                      "right_delim_symbol",
                                      "left_delim",
                                      "right_delim",
                                      "auto_delim_inner",
                                      "accent",
                                      "placeable",
                                      "genfrac",
                                      "overline",
                                      "operatorname",
                                      "operatorname_inner",
                                      "sqrt",
                                      "group",
                                      "simple_group",
                                      "required_group",
                                      "required_group_inner",
                                      "start_group",
                                      "group_inner",
                                      "frac",
                                      "dfrac",
                                      "stackrel",
                                      "binom",
                                      "NT_FLOAT",
                                      "int",
                                      "uint",
                                      YY_NULLPTR};

static const char *yysymbol_name(yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-112)

#define yypact_value_is_default(Yyn) ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) 0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] = {
    76,   -112, 226,  -112, -30,  -30,  -30,  -30,  -16,  -112, 226,  -112, -112, -11,  -112, -112, -112, -112,
    -9,   97,   -112, -112, -112, -112, -112, -112, -8,   -30,  -22,  -112, -112, -112, -112, -112, -112, -112,
    -112, -112, -112, -112, -112, 36,   76,   -112, -112, -112, -112, -112, -112, 144,  -112, -112, -112, -112,
    -112, -112, -112, 76,   -112, -112, -112, -112, -112, 76,   -30,  -30,  -30,  -30,  97,   -112, -112, -18,
    -112, -112, -112, -112, -112, -112, -112, -112, -112, -112, -112, -112, 185,  -112, -13,  -112, -112, -112,
    144,  144,  15,   76,   -6,   76,   -3,   -112, -112, -112, -112, -2,   -112, -26,  11,   0,    8,    19,
    185,  185,  9,    11,   4,    -112, -112, 246,  -112, -112, -112, -112, -112, 12,   11,   19,   19,   -112,
    11,   -112, -112, -112, -112, -30,  -112, -112, -112, -112, -112, -112, -112, 246,  19,   19,   -112, 10,
    13,   -18,  14,   24,   76,   -30,  25,   -30,  -112, -112};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] = {
    0,  5,  0,  32, 0,  0,  0,  0,  0,  58, 0,  25, 28, 0,  64, 62, 61, 26, 0,  0,   6,  7,  8,  18,  19,  20,
    0,  0,  0,  21, 22, 9,  10, 11, 12, 13, 14, 15, 16, 17, 87, 0,  2,  60, 3,  24,  27, 29, 23, 54,  59,  30,
    69, 71, 72, 70, 63, 88, 65, 66, 67, 68, 33, 0,  0,  0,  0,  0,  0,  57, 86, 0,   44, 38, 45, 39,  41,  40,
    35, 42, 36, 37, 43, 52, 76, 74, 0,  79, 1,  4,  54, 54, 0,  88, 0,  84, 0,  90,  91, 92, 93, 0,   101, 0,
    0,  0,  94, 99, 76, 76, 0,  0,  0,  55, 56, 0,  34, 89, 81, 85, 83, 0,  0,  100, 95, 31, 97, 102, 78,  77,
    75, 0,  50, 51, 47, 46, 48, 49, 53, 0,  96, 98, 80, 0,  0,  0,  0,  0,  88, 0,   0,  0,  82, 73};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] = {-112, -112, -112, -112, 3,    -44,  -112, -112, -42, -111, 2,    -87,
                                      -112, -112, -65,  -112, 7,    -112, -112, -112, -78, -112, -112, -112,
                                      -5,   -38,  -112, -85,  -112, -112, -112, -112, -77, -15,  -93};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] = {0,  41, 42,  43, 93, 45,  46, 47, 48, 82, 83, 138, 49, 116, 92,  50,  51, 52,
                                         53, 54, 110, 55, 56, 149, 64, 96, 57, 94, 58, 59,  60, 61,  105, 106, 107};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] = {
    65,  66,  67,  44,  137, 90,  102, 91,  117, 62,  123, 124, 86,  63, 102, 103, 122, 69,  123, 102, 111, 63,  85,
    87,  104, 113, 114, 68,  137, 140, 128, 129, 70,  141, 71,  84,  88, 115, 118, 131, 109, 120, 121, 102, 125, 89,
    90,  90,  91,  91,  126, 127, 143, 130, 144, 139, 145, 119, 147, 97, 98,  99,  100, 150, 109, 109, 95,  148, 146,
    152, 101, 112, 0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,  5,   6,   7,   8,   9,   10,  11,  12,  13,
    14,  15,  16,  17,  18,  19,  95,  20,  21,  22,  23,  24,  25,  26, 27,  28,  29,  30,  31,  32,  33,  34,  35,
    36,  37,  38,  39,  40,  72,  73,  0,   74,  0,   75,  142, 0,   0,  0,   0,   76,  0,   77,  0,   78,  79,  0,
    80,  81,  0,   0,   0,   0,   151, 0,   153, 1,   2,   3,   4,   5,  6,   7,   8,   9,   10,  0,   12,  13,  14,
    15,  16,  17,  18,  19,  0,   20,  21,  22,  23,  24,  25,  26,  27, 28,  29,  30,  31,  32,  33,  34,  35,  36,
    37,  38,  39,  40,  1,   2,   3,   4,   5,   6,   7,   8,   9,   10, 108, 12,  13,  14,  15,  16,  17,  18,  0,
    0,   20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32, 33,  34,  35,  36,  37,  38,  39,  40,  1,
    0,   0,   4,   5,   6,   7,   8,   9,   10,  0,   0,   13,  14,  15, 16,  0,   0,   0,   0,   20,  21,  22,  23,
    24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37, 38,  39,  40,  73,  132, 0,   133, 75,  0,
    0,   0,   0,   0,   0,   134, 0,   135, 78,  0,   136, 80,  81};

static const yytype_int16 yycheck[] = {
    5,  6,   7,   0,   115, 49,  32,  49, 93,  2,  103, 104, 34, 43,  32,  33,  42, 10, 111, 32,  33, 43,  27, 28, 42,
    90, 91,  43,  139, 122, 108, 109, 43, 126, 43, 43,  0,   22, 44,  35,  84,  44, 44, 32,  44,  42, 90,  91, 90, 91,
    42, 32,  139, 44,  44,  43,  43,  95, 44,  64, 65,  66,  67, 148, 108, 109, 63, 43, 145, 44,  68, 86,  -1, -1, -1,
    -1, -1,  -1,  -1,  3,   4,   5,   6,  7,   8,  9,   10,  11, 12,  13,  14,  15, 16, 17,  18,  19, 20,  21, 95, 23,
    24, 25,  26,  27,  28,  29,  30,  31, 32,  33, 34,  35,  36, 37,  38,  39,  40, 41, 42,  43,  23, 24,  -1, 26, -1,
    28, 131, -1,  -1,  -1,  -1,  34,  -1, 36,  -1, 38,  39,  -1, 41,  42,  -1,  -1, -1, -1,  149, -1, 151, 3,  4,  5,
    6,  7,   8,   9,   10,  11,  12,  -1, 14,  15, 16,  17,  18, 19,  20,  21,  -1, 23, 24,  25,  26, 27,  28, 29, 30,
    31, 32,  33,  34,  35,  36,  37,  38, 39,  40, 41,  42,  43, 3,   4,   5,   6,  7,  8,   9,   10, 11,  12, 13, 14,
    15, 16,  17,  18,  19,  20,  -1,  -1, 23,  24, 25,  26,  27, 28,  29,  30,  31, 32, 33,  34,  35, 36,  37, 38, 39,
    40, 41,  42,  43,  3,   -1,  -1,  6,  7,   8,  9,   10,  11, 12,  -1,  -1,  15, 16, 17,  18,  -1, -1,  -1, -1, 23,
    24, 25,  26,  27,  28,  29,  30,  31, 32,  33, 34,  35,  36, 37,  38,  39,  40, 41, 42,  43,  24, 25,  -1, 27, 28,
    -1, -1,  -1,  -1,  -1,  -1,  35,  -1, 37,  38, -1,  40,  41, 42};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] = {
    0,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 46, 47, 48, 49, 50, 51, 52, 53, 57, 60, 61,
    62, 63, 64, 66, 67, 71, 73, 74, 75, 76, 61, 43, 69, 69, 69, 69, 43, 61, 43, 43, 23, 24, 26, 28, 34, 36,
    38, 39, 41, 42, 54, 55, 43, 69, 34, 69, 0,  49, 50, 53, 59, 49, 72, 49, 70, 69, 69, 69, 69, 55, 32, 33,
    42, 77, 78, 79, 13, 50, 65, 33, 78, 59, 59, 22, 58, 72, 44, 70, 44, 44, 42, 79, 79, 44, 42, 32, 65, 65,
    44, 35, 25, 27, 35, 37, 40, 54, 56, 43, 79, 79, 69, 56, 44, 43, 77, 44, 43, 68, 72, 69, 44, 69};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] = {0,  45, 46, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
                                   48, 48, 49, 49, 49, 50, 50, 50, 50, 50, 51, 52, 52, 53, 54, 54, 54, 54, 54, 55, 55,
                                   55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 57, 58, 59, 59, 59, 60, 61, 61, 61, 61, 61,
                                   61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 63, 64, 65, 65, 65, 66, 66, 67, 68, 69,
                                   70, 70, 71, 71, 72, 72, 73, 74, 75, 76, 77, 77, 77, 77, 77, 78, 78, 79, 79};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] = {0, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,
                                   1, 1, 1, 1, 1, 4, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,
                                   2, 2, 0, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 13, 2, 4, 0, 2,
                                   2, 2, 5, 3, 3, 3, 1, 2, 2, 1, 0, 2, 3, 3, 3, 3, 1, 2, 3, 2, 3, 1,  2, 1, 2};


enum
{
  YYENOMEM = -2
};

#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYNOMEM goto yyexhaustedlab


#define YYRECOVERING() (!!yyerrstatus)

#define YYBACKUP(Token, Value)                        \
  do                                                  \
    if (yychar == YYEMPTY)                            \
      {                                               \
        yychar = (Token);                             \
        yylval = (Value);                             \
        YYPOPSTACK(yylen);                            \
        yystate = *yyssp;                             \
        goto yybackup;                                \
      }                                               \
    else                                              \
      {                                               \
        yyerror(YY_("syntax error: cannot back up")); \
        YYERROR;                                      \
      }                                               \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

#ifndef YYFPRINTF
#include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#define YYFPRINTF fprintf
#endif

#define YYDPRINTF(Args)            \
  do                               \
    {                              \
      if (yydebug) YYFPRINTF Args; \
    }                              \
  while (0)


#define YY_SYMBOL_PRINT(Title, Kind, Value, Location) \
  do                                                  \
    {                                                 \
      if (yydebug)                                    \
        {                                             \
          YYFPRINTF(stderr, "%s ", Title);            \
          yy_symbol_print(stderr, Kind, Value);       \
          YYFPRINTF(stderr, "\n");                    \
        }                                             \
    }                                                 \
  while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void yy_symbol_value_print(FILE *yyo, yysymbol_kind_t yykind, YYSTYPE const *const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE(yyoutput);
  if (!yyvaluep) return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE(yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void yy_symbol_print(FILE *yyo, yysymbol_kind_t yykind, YYSTYPE const *const yyvaluep)
{
  YYFPRINTF(yyo, "%s %s (", yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name(yykind));

  yy_symbol_value_print(yyo, yykind, yyvaluep);
  YYFPRINTF(yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void yy_stack_print(yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF(stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF(stderr, " %d", yybot);
    }
  YYFPRINTF(stderr, "\n");
}

#define YY_STACK_PRINT(Bottom, Top)                 \
  do                                                \
    {                                               \
      if (yydebug) yy_stack_print((Bottom), (Top)); \
    }                                               \
  while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void yy_reduce_print(yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF(stderr, "Reducing stack by rule %d (line %d):\n", yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF(stderr, "   $%d = ", yyi + 1);
      yy_symbol_print(stderr, YY_ACCESSING_SYMBOL(+yyssp[yyi + 1 - yynrhs]), &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF(stderr, "\n");
    }
}

#define YY_REDUCE_PRINT(Rule)                           \
  do                                                    \
    {                                                   \
      if (yydebug) yy_reduce_print(yyssp, yyvsp, Rule); \
    }                                                   \
  while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
#define YYDPRINTF(Args) ((void)0)
#define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
#define YY_STACK_PRINT(Bottom, Top)
#define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void yydestruct(const char *yymsg, yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE(yyvaluep);
  if (!yymsg) yymsg = "Deleting";
  YY_SYMBOL_PRINT(yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE(yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int yyparse(void)
{
  yy_state_fast_t yystate = 0;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus = 0;

  /* Refer to the stacks through separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* Their size.  */
  YYPTRDIFF_T yystacksize = YYINITDEPTH;

  /* The state stack: array, bottom, top.  */
  yy_state_t yyssa[YYINITDEPTH];
  yy_state_t *yyss = yyssa;
  yy_state_t *yyssp = yyss;

  /* The semantic value stack: array, bottom, top.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


#define YYPOPSTACK(N) (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF((stderr, "Entering state %d\n", yystate));
  YY_ASSERT(0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST(yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT(yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

#if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow(YY_("memory exhausted"), &yyss1, yysize * YYSIZEOF(*yyssp), &yyvs1, yysize * YYSIZEOF(*yyvsp),
                   &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize) YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize) yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr = YY_CAST(union yyalloc *, YYSTACK_ALLOC(YY_CAST(YYSIZE_T, YYSTACK_BYTES(yystacksize))));
        if (!yyptr) YYNOMEM;
        YYSTACK_RELOCATE(yyss_alloc, yyss);
        YYSTACK_RELOCATE(yyvs_alloc, yyvs);
#undef YYSTACK_RELOCATE
        if (yyss1 != yyssa) YYSTACK_FREE(yyss1);
      }
#endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF((stderr, "Stack size increased to %ld\n", YY_CAST(long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp) YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL) YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default(yyn)) goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF((stderr, "Reading a token\n"));
      yychar = yylex();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE(yychar);
      YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken) goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error(yyn)) goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus) yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0) goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1 - yylen];


  YY_REDUCE_PRINT(yyn);
  switch (yyn)
    {
    case 2: /* result: math  */
#line 60 "mathtex2.y"
      {
        result_parser_node_index = copy_parser_node(yyvsp[0]);
      }
#line 1357 "mathtex2.tab.c"
      break;

    case 3: /* math: token  */
#line 65 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.u.math.previous = 0;
        yyval.u.math.token = copy_parser_node(yyvsp[0]);
        yyval.type = NT_MATH;
      }
#line 1368 "mathtex2.tab.c"
      break;

    case 4: /* math: math token  */
#line 71 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.u.math.previous = copy_parser_node(yyvsp[-1]);
        yyval.u.math.token = copy_parser_node(yyvsp[0]);
        yyval.length += yyvsp[0].length;
      }
#line 1379 "mathtex2.tab.c"
      break;

    case 5: /* single_symbol: SINGLE_SYMBOL  */
#line 81 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1385 "mathtex2.tab.c"
      break;

    case 6: /* single_symbol: LEFT_DELIM  */
#line 82 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1391 "mathtex2.tab.c"
      break;

    case 7: /* single_symbol: AMBI_DELIM  */
#line 83 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1397 "mathtex2.tab.c"
      break;

    case 8: /* single_symbol: RIGHT_DELIM  */
#line 84 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1403 "mathtex2.tab.c"
      break;

    case 9: /* single_symbol: '['  */
#line 85 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1409 "mathtex2.tab.c"
      break;

    case 10: /* single_symbol: ']'  */
#line 86 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1415 "mathtex2.tab.c"
      break;

    case 11: /* single_symbol: '('  */
#line 87 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1421 "mathtex2.tab.c"
      break;

    case 12: /* single_symbol: ')'  */
#line 88 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1427 "mathtex2.tab.c"
      break;

    case 13: /* single_symbol: '|'  */
#line 89 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1433 "mathtex2.tab.c"
      break;

    case 14: /* single_symbol: '<'  */
#line 90 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1439 "mathtex2.tab.c"
      break;

    case 15: /* single_symbol: '>'  */
#line 91 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1445 "mathtex2.tab.c"
      break;

    case 16: /* single_symbol: '/'  */
#line 92 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1451 "mathtex2.tab.c"
      break;

    case 17: /* single_symbol: '.'  */
#line 93 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1457 "mathtex2.tab.c"
      break;

    case 18: /* single_symbol: LBRACE  */
#line 94 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1463 "mathtex2.tab.c"
      break;

    case 19: /* single_symbol: RBRACE  */
#line 95 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1469 "mathtex2.tab.c"
      break;

    case 20: /* single_symbol: PIPE  */
#line 96 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1475 "mathtex2.tab.c"
      break;

    case 21: /* single_symbol: DIGIT  */
#line 97 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1481 "mathtex2.tab.c"
      break;

    case 22: /* single_symbol: PLUSMINUS  */
#line 98 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1487 "mathtex2.tab.c"
      break;

    case 23: /* token: auto_delim  */
#line 103 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1495 "mathtex2.tab.c"
      break;

    case 24: /* token: simple  */
#line 106 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1503 "mathtex2.tab.c"
      break;

    case 25: /* token: UNKNOWN_SYMBOL  */
#line 109 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_SYMBOL;
      }
#line 1512 "mathtex2.tab.c"
      break;

    case 26: /* simple: SPACE  */
#line 116 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_SPACE;
      }
#line 1521 "mathtex2.tab.c"
      break;

    case 27: /* simple: customspace  */
#line 120 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1529 "mathtex2.tab.c"
      break;

    case 28: /* simple: FONT  */
#line 123 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_FONT;
      }
#line 1538 "mathtex2.tab.c"
      break;

    case 29: /* simple: subsuper  */
#line 127 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1546 "mathtex2.tab.c"
      break;

    case 30: /* simple: placeable  */
#line 130 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1554 "mathtex2.tab.c"
      break;

    case 31: /* customspace: HSPACE '{' NT_FLOAT '}'  */
#line 136 "mathtex2.y"
      {
        yyval = yyvsp[-3];
        yyval.length += yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_CUSTOMSPACE;
      }
#line 1564 "mathtex2.tab.c"
      break;

    case 32: /* subsuper: APOSTROPHE  */
#line 144 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.index = 0;
        yyval.type = NT_SUBSUPER;
        yyval.u.subsuper.operator= '\'';
        yyval.u.subsuper.token = 0;
      }
#line 1576 "mathtex2.tab.c"
      break;

    case 33: /* subsuper: SUBSUPEROP placeable  */
#line 151 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.length += yyvsp[0].length;
        yyval.type = NT_SUBSUPER;
        yyval.u.subsuper.operator= yyvsp[-1].source[0];
        yyval.u.subsuper.token = copy_parser_node(yyvsp[0]);
      }
#line 1589 "mathtex2.tab.c"
      break;

    case 34: /* auto_delim: left_delim auto_delim_inner right_delim  */
#line 162 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_AUTO_DELIM;
        yyval.u.autodelim.left_delim_start = yyvsp[-2].source + 5;
        yyval.u.autodelim.left_delim_length = yyvsp[-2].length - 5;
        yyval.u.autodelim.right_delim_start = yyvsp[0].source + 6;
        yyval.u.autodelim.right_delim_length = yyvsp[0].length - 6;
        yyval.u.autodelim.inner_node_index = copy_parser_node(yyvsp[-1]);
      }
#line 1605 "mathtex2.tab.c"
      break;

    case 35: /* ambi_delim_symbol: '|'  */
#line 176 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1614 "mathtex2.tab.c"
      break;

    case 36: /* ambi_delim_symbol: '/'  */
#line 180 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1623 "mathtex2.tab.c"
      break;

    case 37: /* ambi_delim_symbol: '.'  */
#line 184 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1632 "mathtex2.tab.c"
      break;

    case 38: /* ambi_delim_symbol: AMBI_DELIM  */
#line 188 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1641 "mathtex2.tab.c"
      break;

    case 39: /* ambi_delim_symbol: PIPE  */
#line 192 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1650 "mathtex2.tab.c"
      break;

    case 40: /* left_delim_symbol: '('  */
#line 199 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1659 "mathtex2.tab.c"
      break;

    case 41: /* left_delim_symbol: '['  */
#line 203 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1668 "mathtex2.tab.c"
      break;

    case 42: /* left_delim_symbol: '<'  */
#line 207 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1677 "mathtex2.tab.c"
      break;

    case 43: /* left_delim_symbol: ambi_delim_symbol  */
#line 211 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1686 "mathtex2.tab.c"
      break;

    case 44: /* left_delim_symbol: LEFT_DELIM  */
#line 215 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1695 "mathtex2.tab.c"
      break;

    case 45: /* left_delim_symbol: LBRACE  */
#line 219 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1704 "mathtex2.tab.c"
      break;

    case 46: /* right_delim_symbol: ')'  */
#line 226 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1713 "mathtex2.tab.c"
      break;

    case 47: /* right_delim_symbol: ']'  */
#line 230 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1722 "mathtex2.tab.c"
      break;

    case 48: /* right_delim_symbol: '>'  */
#line 234 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1731 "mathtex2.tab.c"
      break;

    case 49: /* right_delim_symbol: ambi_delim_symbol  */
#line 238 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1740 "mathtex2.tab.c"
      break;

    case 50: /* right_delim_symbol: RIGHT_DELIM  */
#line 242 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1749 "mathtex2.tab.c"
      break;

    case 51: /* right_delim_symbol: RBRACE  */
#line 246 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 1758 "mathtex2.tab.c"
      break;

    case 52: /* left_delim: LEFT left_delim_symbol  */
#line 253 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
      }
#line 1769 "mathtex2.tab.c"
      break;

    case 53: /* right_delim: RIGHT right_delim_symbol  */
#line 262 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
      }
#line 1780 "mathtex2.tab.c"
      break;

    case 54: /* auto_delim_inner: %empty  */
#line 271 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = NULL;
        yyval.length = 0;
        yyval.u.autodeliminner.previous = 0;
        yyval.u.autodeliminner.token = 0;
        yyval.type = NT_AUTO_DELIM_INNER;
      }
#line 1793 "mathtex2.tab.c"
      break;

    case 55: /* auto_delim_inner: simple auto_delim_inner  */
#line 279 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.type = NT_AUTO_DELIM_INNER;
        yyval.u.autodeliminner.previous = copy_parser_node(yyvsp[0]);
        yyval.u.autodeliminner.token = copy_parser_node(yyvsp[-1]);
        yyval.length += yyvsp[0].length;
      }
#line 1806 "mathtex2.tab.c"
      break;

    case 56: /* auto_delim_inner: auto_delim auto_delim_inner  */
#line 287 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.type = NT_AUTO_DELIM_INNER;
        yyval.u.autodeliminner.previous = copy_parser_node(yyvsp[0]);
        yyval.u.autodeliminner.token = copy_parser_node(yyvsp[-1]);
        yyval.length += yyvsp[0].length;
      }
#line 1819 "mathtex2.tab.c"
      break;

    case 57: /* accent: ACCENT placeable  */
#line 298 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.type = NT_ACCENT;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.u.accent.token = copy_parser_node(yyvsp[0]);
      }
#line 1831 "mathtex2.tab.c"
      break;

    case 58: /* placeable: SNOWFLAKE  */
#line 307 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_SYMBOL;
      }
#line 1840 "mathtex2.tab.c"
      break;

    case 59: /* placeable: accent  */
#line 311 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1848 "mathtex2.tab.c"
      break;

    case 60: /* placeable: single_symbol  */
#line 314 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_SYMBOL;
      }
#line 1857 "mathtex2.tab.c"
      break;

    case 61: /* placeable: C_OVER_C  */
#line 318 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_C_OVER_C;
      }
#line 1866 "mathtex2.tab.c"
      break;

    case 62: /* placeable: FUNCTION  */
#line 322 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_FUNCTION;
      }
#line 1875 "mathtex2.tab.c"
      break;

    case 63: /* placeable: group  */
#line 326 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1883 "mathtex2.tab.c"
      break;

    case 64: /* placeable: LATEXTEXT  */
#line 329 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_LATEXTEXT;
      }
#line 1892 "mathtex2.tab.c"
      break;

    case 65: /* placeable: frac  */
#line 333 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1900 "mathtex2.tab.c"
      break;

    case 66: /* placeable: dfrac  */
#line 336 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1908 "mathtex2.tab.c"
      break;

    case 67: /* placeable: stackrel  */
#line 339 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1916 "mathtex2.tab.c"
      break;

    case 68: /* placeable: binom  */
#line 342 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1924 "mathtex2.tab.c"
      break;

    case 69: /* placeable: genfrac  */
#line 345 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1932 "mathtex2.tab.c"
      break;

    case 70: /* placeable: sqrt  */
#line 348 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1940 "mathtex2.tab.c"
      break;

    case 71: /* placeable: overline  */
#line 351 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1948 "mathtex2.tab.c"
      break;

    case 72: /* placeable: operatorname  */
#line 354 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 1956 "mathtex2.tab.c"
      break;

    case 73: /* genfrac: GENFRAC '{' left_delim_symbol '}' '{' right_delim_symbol '}' '{' NT_FLOAT '}' simple_group
                required_group required_group  */
#line 360 "mathtex2.y"
      {
        double thickness = 0;
        int n = sscanf(yyvsp[-4].source, "\\hspace{%lf}", &thickness);
        yyval.index = 0;
        yyval.source = yyvsp[-12].source;
        yyval.length = yyvsp[-12].length + yyvsp[-11].length + yyvsp[-10].length + yyvsp[-9].length + yyvsp[-8].length +
                       yyvsp[-7].length + yyvsp[-6].length + yyvsp[-5].length + yyvsp[-4].length + yyvsp[-3].length +
                       yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GENFRAC;
        yyval.u.genfrac.left_delim_start = yyvsp[-10].source;
        yyval.u.genfrac.left_delim_length = yyvsp[-10].length;
        yyval.u.genfrac.right_delim_start = yyvsp[-7].source;
        yyval.u.genfrac.right_delim_length = yyvsp[-7].length;
        if (n != 1)
          {
            thickness = NAN;
          }
        yyval.u.genfrac.thickness = thickness;
        yyval.u.genfrac.style_text_start = yyvsp[-2].source;
        yyval.u.genfrac.style_text_length = yyvsp[-2].length;
        yyval.u.genfrac.numerator_group = copy_parser_node(yyvsp[-1]);
        yyval.u.genfrac.denominator_group = copy_parser_node(yyvsp[0]);
      }
#line 1981 "mathtex2.tab.c"
      break;

    case 74: /* overline: OVERLINE required_group  */
#line 383 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OVERLINE;
        yyval.u.overline.body = copy_parser_node(yyvsp[0]);
      }
#line 1993 "mathtex2.tab.c"
      break;

    case 75: /* operatorname: OPERATORNAME '{' operatorname_inner '}'  */
#line 393 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.source = yyvsp[-3].source;
        yyval.length = yyvsp[-3].length + yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OPERATORNAME;
      }
#line 2005 "mathtex2.tab.c"
      break;

    case 76: /* operatorname_inner: %empty  */
#line 403 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = NULL;
        yyval.length = 0;
        yyval.type = NT_OTHER;
        yyval.u.operatorname.previous = 0;
        yyval.u.operatorname.token = 0;
      }
#line 2018 "mathtex2.tab.c"
      break;

    case 77: /* operatorname_inner: simple operatorname_inner  */
#line 411 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
        yyval.u.operatorname.previous = copy_parser_node(yyvsp[0]);
        yyval.u.operatorname.token = copy_parser_node(yyvsp[-1]);
      }
#line 2031 "mathtex2.tab.c"
      break;

    case 78: /* operatorname_inner: UNKNOWN_SYMBOL operatorname_inner  */
#line 419 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
        yyval.u.operatorname.previous = copy_parser_node(yyvsp[0]);
        yyval.u.operatorname.token = copy_parser_node(yyvsp[-1]);
      }
#line 2044 "mathtex2.tab.c"
      break;

    case 79: /* sqrt: SQRT required_group  */
#line 430 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_SQRT;
        yyval.u.sqrt.index_start = "";
        yyval.u.sqrt.index_length = 0;
        yyval.u.sqrt.token = copy_parser_node(yyvsp[0]);
      }
#line 2058 "mathtex2.tab.c"
      break;

    case 80: /* sqrt: SQRT '[' int ']' required_group  */
#line 439 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-4].source;
        yyval.length = yyvsp[-4].length + yyvsp[-3].length + yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_SQRT;
        yyval.u.sqrt.index_start = yyvsp[-2].source;
        yyval.u.sqrt.index_length = yyvsp[-2].length;
        yyval.u.sqrt.token = copy_parser_node(yyvsp[0]);
      }
#line 2072 "mathtex2.tab.c"
      break;

    case 81: /* group: start_group group_inner '}'  */
#line 451 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GROUP;
      }
#line 2084 "mathtex2.tab.c"
      break;

    case 82: /* simple_group: '{' group_inner '}'  */
#line 461 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GROUP;
      }
#line 2096 "mathtex2.tab.c"
      break;

    case 83: /* required_group: '{' required_group_inner '}'  */
#line 471 "mathtex2.y"
      {
        yyval = yyvsp[-1];
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GROUP;
      }
#line 2108 "mathtex2.tab.c"
      break;

    case 84: /* required_group_inner: token  */
#line 481 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
        yyval.u.group.previous = 0;
        yyval.u.group.token = copy_parser_node(yyvsp[0]);
      }
#line 2119 "mathtex2.tab.c"
      break;

    case 85: /* required_group_inner: token required_group_inner  */
#line 487 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
        yyval.u.group.previous = copy_parser_node(yyvsp[0]);
        yyval.u.group.token = copy_parser_node(yyvsp[-1]);
      }
#line 2132 "mathtex2.tab.c"
      break;

    case 86: /* start_group: LATEXFONT '{'  */
#line 498 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
      }
#line 2143 "mathtex2.tab.c"
      break;

    case 87: /* start_group: '{'  */
#line 504 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 2152 "mathtex2.tab.c"
      break;

    case 88: /* group_inner: %empty  */
#line 511 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = NULL;
        yyval.length = 0;
        yyval.type = NT_OTHER;
        yyval.u.group.previous = 0;
        yyval.u.group.token = 0;
      }
#line 2165 "mathtex2.tab.c"
      break;

    case 89: /* group_inner: token group_inner  */
#line 519 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
        yyval.u.group.previous = copy_parser_node(yyvsp[0]);
        yyval.u.group.token = copy_parser_node(yyvsp[-1]);
      }
#line 2178 "mathtex2.tab.c"
      break;

    case 90: /* frac: FRAC required_group required_group  */
#line 530 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GENFRAC;
        yyval.u.genfrac.left_delim_start = "";
        yyval.u.genfrac.left_delim_length = 0;
        yyval.u.genfrac.right_delim_start = "";
        yyval.u.genfrac.right_delim_length = 0;
        yyval.u.genfrac.thickness = NAN;
        yyval.u.genfrac.style_text_start = "{0}";
        yyval.u.genfrac.style_text_length = 3;
        yyval.u.genfrac.numerator_group = copy_parser_node(yyvsp[-1]);
        yyval.u.genfrac.denominator_group = copy_parser_node(yyvsp[0]);
      }
#line 2198 "mathtex2.tab.c"
      break;

    case 91: /* dfrac: DFRAC required_group required_group  */
#line 548 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GENFRAC;
        yyval.u.genfrac.left_delim_start = "";
        yyval.u.genfrac.left_delim_length = 0;
        yyval.u.genfrac.right_delim_start = "";
        yyval.u.genfrac.right_delim_length = 0;
        yyval.u.genfrac.thickness = NAN;
        yyval.u.genfrac.style_text_start = "{1}";
        yyval.u.genfrac.style_text_length = 3;
        yyval.u.genfrac.numerator_group = copy_parser_node(yyvsp[-1]);
        yyval.u.genfrac.denominator_group = copy_parser_node(yyvsp[0]);
      }
#line 2218 "mathtex2.tab.c"
      break;

    case 92: /* stackrel: STACKREL required_group required_group  */
#line 566 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GENFRAC;
        yyval.u.genfrac.left_delim_start = "";
        yyval.u.genfrac.left_delim_length = 0;
        yyval.u.genfrac.right_delim_start = "";
        yyval.u.genfrac.right_delim_length = 0;
        yyval.u.genfrac.thickness = 0.0;
        yyval.u.genfrac.style_text_start = "{1}";
        yyval.u.genfrac.style_text_length = 3;
        yyval.u.genfrac.numerator_group = copy_parser_node(yyvsp[-1]);
        yyval.u.genfrac.denominator_group = copy_parser_node(yyvsp[0]);
      }
#line 2238 "mathtex2.tab.c"
      break;

    case 93: /* binom: BINOM required_group required_group  */
#line 584 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_GENFRAC;
        yyval.u.genfrac.left_delim_start = "(";
        yyval.u.genfrac.left_delim_length = 1;
        yyval.u.genfrac.right_delim_start = ")";
        yyval.u.genfrac.right_delim_length = 1;
        yyval.u.genfrac.thickness = 0.0;
        yyval.u.genfrac.style_text_start = "{1}";
        yyval.u.genfrac.style_text_length = 3;
        yyval.u.genfrac.numerator_group = copy_parser_node(yyvsp[-1]);
        yyval.u.genfrac.denominator_group = copy_parser_node(yyvsp[0]);
      }
#line 2258 "mathtex2.tab.c"
      break;

    case 94: /* NT_FLOAT: int  */
#line 603 "mathtex2.y"
      {
        yyval = yyvsp[0];
      }
#line 2264 "mathtex2.tab.c"
      break;

    case 95: /* NT_FLOAT: '.' uint  */
#line 604 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_FLOAT;
      }
#line 2275 "mathtex2.tab.c"
      break;

    case 96: /* NT_FLOAT: PLUSMINUS '.' uint  */
#line 610 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_FLOAT;
      }
#line 2286 "mathtex2.tab.c"
      break;

    case 97: /* NT_FLOAT: int '.'  */
#line 616 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_FLOAT;
      }
#line 2297 "mathtex2.tab.c"
      break;

    case 98: /* NT_FLOAT: int '.' uint  */
#line 622 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-2].source;
        yyval.length = yyvsp[-2].length + yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_FLOAT;
      }
#line 2308 "mathtex2.tab.c"
      break;

    case 99: /* int: uint  */
#line 631 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_INTEGER;
      }
#line 2317 "mathtex2.tab.c"
      break;

    case 100: /* int: PLUSMINUS uint  */
#line 635 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_INTEGER;
      }
#line 2328 "mathtex2.tab.c"
      break;

    case 101: /* uint: DIGIT  */
#line 643 "mathtex2.y"
      {
        yyval = yyvsp[0];
        yyval.type = NT_OTHER;
      }
#line 2337 "mathtex2.tab.c"
      break;

    case 102: /* uint: uint DIGIT  */
#line 647 "mathtex2.y"
      {
        yyval.index = 0;
        yyval.source = yyvsp[-1].source;
        yyval.length = yyvsp[-1].length + yyvsp[0].length;
        yyval.type = NT_OTHER;
      }
#line 2348 "mathtex2.tab.c"
      break;


#line 2352 "mathtex2.tab.c"

    default:
      break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT("-> $$ =", YY_CAST(yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK(yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp ? yytable[yyi] : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE(yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror(YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF) YYABORT;
        }
      else
        {
          yydestruct("Error: discarding", yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0) YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK(yylen);
  yylen = 0;
  YY_STACK_PRINT(yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3; /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default(yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn) break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss) YYABORT;


      yydestruct("Error: popping", YY_ACCESSING_SYMBOL(yystate), yyvsp);
      YYPOPSTACK(1);
      yystate = *yyssp;
      YY_STACK_PRINT(yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT("Shifting", YY_ACCESSING_SYMBOL(yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror(YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE(yychar);
      yydestruct("Cleanup: discarding lookahead", yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK(yylen);
  YY_STACK_PRINT(yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct("Cleanup: popping", YY_ACCESSING_SYMBOL(+*yyssp), yyvsp);
      YYPOPSTACK(1);
    }
#ifndef yyoverflow
  if (yyss != yyssa) YYSTACK_FREE(yyss);
#endif

  return yyresult;
}

#line 655 "mathtex2.y"


const char *snowflake_symbols[] = {"\\doteqdot", "\\doteq",   "\\dotminus", "\\barleftarrow",
                                   "\\ddots",    "\\dotplus", "\\dots",     "\\barwedge"};
const char *accent_symbols[] = {"\\hat",           "\\breve",    "\\bar",     "\\grave",   "\\acute",
                                "\\tilde",         "\\dot",      "\\ddot",    "\\vec",     "\\overrightarrow",
                                "\\overleftarrow", "\\mathring", "\\widebar", "\\widehat", "\\widetilde"};
const char *font_symbols[] = {"\\rm",      "\\cal", "\\it",   "\\tt",      "\\sf",  "\\bf",
                              "\\default", "\\bb",  "\\frak", "\\circled", "\\scr", "\\regular"};
const char *latexfont_symbols[] = {"\\mathrm",  "\\mathcal",     "\\mathit",    "\\mathtt",   "\\mathsf",
                                   "\\mathbf",  "\\mathdefault", "\\mathbb",    "\\mathfrak", "\\mathcircled",
                                   "\\mathscr", "\\mathregular", "\\textrm",    "\\textit",   "\\textbf",
                                   "\\texttt",  "\\textsf",      "\\textnormal"};
const char *c_over_c_symbols[] = {"\\AA"};
const char *space_symbols[] = {"\\thinspace", "\\enspace", "\\quad", "\\qquad"};
const char *left_delim_symbols[] = {"\\int", "\\lfloor", "\\langle", "\\lceil", "\\sum"};
const char *ambi_delim_symbols[] = {"\\backslash", "\\uparrow",     "\\downarrow", "\\updownarrow", "\\Uparrow",
                                    "\\Downarrow", "\\Updownarrow", "\\vert",      "\\Vert"};
const char *right_delim_symbols[] = {"\\rfloor", "\\rangle", "\\rceil"};
const char *function_symbols[] = {"\\arccos", "\\csc", "\\ker",    "\\min",  "\\arcsin", "\\deg", "\\lg",     "\\Pr",
                                  "\\arctan", "\\det", "\\lim",    "\\sec",  "\\arg",    "\\dim", "\\liminf", "\\sin",
                                  "\\cos",    "\\exp", "\\limsup", "\\sinh", "\\cosh",   "\\gcd", "\\ln",     "\\sup",
                                  "\\cot",    "\\hom", "\\log",    "\\tan",  "\\coth",   "\\inf", "\\max",    "\\tanh"};

int symbol_in_symbol_list(const char *symbol, size_t length, const char **symbol_list, size_t num_symbols)
{
  size_t i;
  for (i = 0; i < num_symbols; i++)
    {
      if (strncmp(symbol, symbol_list[i], length) == 0 && symbol_list[i][length] == 0)
        {
          return 1;
        }
    }
  return 0;
}

int symbol_is_snowflake(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, snowflake_symbols, sizeof(snowflake_symbols) / sizeof(const char *));
}

int symbol_is_accent(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, accent_symbols, sizeof(accent_symbols) / sizeof(const char *));
}

int symbol_is_font(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, font_symbols, sizeof(font_symbols) / sizeof(const char *));
}

int symbol_is_latexfont(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, latexfont_symbols, sizeof(latexfont_symbols) / sizeof(const char *));
}

int symbol_is_c_over_c(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, c_over_c_symbols, sizeof(c_over_c_symbols) / sizeof(const char *));
}

int symbol_is_space(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, space_symbols, sizeof(space_symbols) / sizeof(const char *));
}

int symbol_is_left_delim(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, left_delim_symbols, sizeof(left_delim_symbols) / sizeof(const char *));
}

int symbol_is_ambi_delim(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, ambi_delim_symbols, sizeof(ambi_delim_symbols) / sizeof(const char *));
}

int symbol_is_right_delim(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, right_delim_symbols, sizeof(right_delim_symbols) / sizeof(const char *));
}
int symbol_is_function(const char *symbol, size_t length)
{
  return symbol_in_symbol_list(symbol, length, function_symbols, sizeof(function_symbols) / sizeof(const char *));
}


const char *cursor;
enum State state;
const char *symbol_start;
int ignore_whitespace;

int yylex(void)
{
  yylval.index = 0;
  for (; *cursor != 0 || state == INSIDE_SYMBOL; cursor++)
    {
      int c_bytes = 1;
      int c = str_utf8_to_unicode((const unsigned char *)cursor, &c_bytes);
      if (c == ' ' && ignore_whitespace)
        {
          ignore_whitespace = 0;
          continue;
        }
      ignore_whitespace = 0;
      switch (state)
        {
        case OUTSIDE_SYMBOL:
          {
            if ('0' <= c && c <= '9')
              {
                yylval.source = cursor;
                yylval.length = 1;
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return DIGIT;
              }
            else if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || (0x80 <= c && c <= 0x1ffff) ||
                     strchr(" *,=:;!?&'@", c) != NULL)
              {
                if (c == ' ')
                  {
                    break;
                  }
                yylval.source = cursor;
                yylval.length = c_bytes;
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += c_bytes;
                /* special case for := */
                if (c == ':' && *cursor == '=')
                  {
                    yylval.length += 1;
                    cursor += 1;
                  }
                return SINGLE_SYMBOL;
              }
            else if (strchr("()[]<>./{}|", c) != NULL)
              {
                yylval.source = cursor;
                yylval.length = 1;
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return c;
              }
            else
              {
                switch (c)
                  {
                  case '\\':
                    state = INSIDE_SYMBOL;
                    symbol_start = cursor;
                    break;
                  case '_':
                  case '^':
                    yylval.source = cursor;
                    yylval.length = 1;
                    yylval.type = NT_TERMINAL_SYMBOL;
                    cursor += 1;
                    return SUBSUPEROP;
                  case '\'':
                    yylval.source = cursor;
                    yylval.length = 1;
                    yylval.type = NT_TERMINAL_SYMBOL;
                    cursor += 1;
                    while (*cursor == '\'')
                      {
                        yylval.length = 1;
                        cursor += 1;
                      }
                    return APOSTROPHE;
                  case '~':
                    yylval.source = cursor;
                    yylval.length = 1;
                    yylval.type = NT_TERMINAL_SYMBOL;
                    cursor += 1;
                    return SPACE;
                  case '-':
                  case '+':
                    yylval.source = cursor;
                    yylval.length = 1;
                    yylval.type = NT_TERMINAL_SYMBOL;
                    cursor += 1;
                    return PLUSMINUS;
                  default:
                    yylval.source = cursor;
                    yylval.length = 1;
                    yylval.type = NT_TERMINAL_SYMBOL;
                    cursor += 1;
                    return c;
                  }
              }
          }
          break;
        case INSIDE_SYMBOL:
          {
            if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
              {
                /* valid part of symbol */
              }
            else if (c == '{' && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return LBRACE;
              }
            else if (c == '}' && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return RBRACE;
              }
            else if (c == '|' && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return PIPE;
              }
            else if (strchr(",/>:; !", c) != NULL && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return SPACE;
              }
            else if (strchr("%$[]_#", c) != NULL && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return SINGLE_SYMBOL;
              }
            else if (strchr("\"`'~.^", c) != NULL && cursor == symbol_start + 1)
              {
                state = OUTSIDE_SYMBOL;
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start + 1);
                yylval.type = NT_TERMINAL_SYMBOL;
                cursor += 1;
                return ACCENT;
              }
            else
              {
                int result;
                state = OUTSIDE_SYMBOL;
                yylval.type = NT_TERMINAL_SYMBOL;
                if (strncmp("\\frac", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = FRAC;
                  }
                else if (strncmp("\\dfrac", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = DFRAC;
                  }
                else if (strncmp("\\stackrel", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = STACKREL;
                  }
                else if (strncmp("\\binom", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = BINOM;
                  }
                else if (strncmp("\\genfrac", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = GENFRAC;
                  }
                else if (strncmp("\\operatorname", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = OPERATORNAME;
                  }
                else if (strncmp("\\overline", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = OVERLINE;
                  }
                else if (strncmp("\\sqrt", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = SQRT;
                  }
                else if (strncmp("\\hspace", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = HSPACE;
                  }
                else if (strncmp("\\left", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = LEFT;
                  }
                else if (strncmp("\\right", symbol_start, (int)(cursor - symbol_start)) == 0)
                  {
                    result = RIGHT;
                  }
                else if (symbol_is_snowflake(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = SNOWFLAKE;
                  }
                else if (symbol_is_accent(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = ACCENT;
                  }
                else if (symbol_is_font(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = FONT;
                  }
                else if (symbol_is_latexfont(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = LATEXFONT;
                  }
                else if (symbol_is_function(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = FUNCTION;
                  }
                else if (symbol_is_c_over_c(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = C_OVER_C;
                  }
                else if (symbol_is_space(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = SPACE;
                  }
                else if (symbol_is_left_delim(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = LEFT_DELIM;
                  }
                else if (symbol_is_ambi_delim(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = AMBI_DELIM;
                  }
                else if (symbol_is_right_delim(symbol_start, (int)(cursor - symbol_start)))
                  {
                    result = RIGHT_DELIM;
                  }
                else
                  {
                    yylval.type = NT_SYMBOL;
                    result = SINGLE_SYMBOL;
                    ignore_whitespace = 1;
                  }
                yylval.source = symbol_start;
                yylval.length = (int)(cursor - symbol_start);
                if (result == LATEXFONT && *cursor == '{' && strncmp(symbol_start, "\\text", 5) == 0)
                  {
                    const char *text_end = strchr(symbol_start, '}');
                    if (text_end && (strrchr(text_end, '{') == cursor))
                      {
                        yylval.length = (int)(text_end - symbol_start);
                        cursor = text_end + 1;
                        result = LATEXTEXT;
                      }
                  }
                return result;
              }
          }
          break;
        default:
          break;
        }
    }
  return 0;
}
