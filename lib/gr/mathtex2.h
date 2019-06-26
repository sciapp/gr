#ifndef TEST3_H
#define TEST3_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


enum State
{
  OUTSIDE_SYMBOL = 0,
  INSIDE_SYMBOL = 1,
};

typedef enum ParserNodeType_
{
  NT_TERMINAL_SYMBOL = 0,
  NT_OTHER = 1,
  NT_INTEGER = 2,
  NT_FLOAT = 3,
  NT_SPACE = 4,
  NT_CUSTOMSPACE = 5,
  NT_FONT = 6,
  NT_MATH = 7,
  NT_FUNCTION = 8,
  NT_OPERATORNAME = 9,
  NT_SYMBOL = 10,
  NT_GROUP = 11,
  NT_GENFRAC = 12,
  NT_AUTO_DELIM = 13,
  NT_AUTO_DELIM_INNER = 14,
  NT_OVERLINE = 15,
  NT_C_OVER_C = 16,
  NT_ACCENT = 17,
  NT_SQRT = 18,
  /* TODO */
  NT_SUBSUPER,
} ParserNodeType;

typedef struct MathNode_
{
  size_t previous;
  size_t token;
} MathNode;

typedef struct OperatorNameNode_
{
  size_t previous;
  size_t token;
} OperatorNameNode;

typedef struct Overline_
{
  size_t body;
} Overline;

typedef struct Group_
{
  size_t previous;
  size_t token;
} Group;

typedef struct Subsuper_
{
  char operator;
  size_t token;
} Subsuper;

typedef struct Accent_
{
  size_t token;
} Accent;

typedef struct Sqrt_
{
  const char *index_start;
  size_t index_length;
  size_t token;
} Sqrt;

typedef struct Genfrac_
{
  const char *left_delim_start;
  size_t left_delim_length;
  const char *right_delim_start;
  size_t right_delim_length;
  double thickness;
  const char *style_text_start;
  size_t style_text_length;
  size_t numerator_group;
  size_t denominator_group;
} Genfrac;

typedef struct AutoDelim_
{
  const char *left_delim_start;
  size_t left_delim_length;
  const char *right_delim_start;
  size_t right_delim_length;
  size_t inner_node_index;
} AutoDelim;

typedef struct AutoDelimInner_
{
  size_t previous;
  size_t token;
} AutoDelimInner;

typedef struct ParserNode_
{
  size_t index;
  union
  {
    MathNode math;
    OperatorNameNode operatorname;
    Group group;
    Overline overline;
    Genfrac genfrac;
    Subsuper subsuper;
    AutoDelim autodelim;
    AutoDelimInner autodeliminner;
    Accent accent;
    Sqrt sqrt;
  } u;
  ParserNodeType type;
  const char *source;
  size_t length;
} ParserNode;

extern size_t result_parser_node_index;

size_t copy_parser_node(ParserNode node);

ParserNode *get_parser_node(size_t node_index);


#endif
