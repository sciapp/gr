
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#define MAX(a,b) (a)>(b) ? (a) : (b)

typedef enum
{
  Plus, Minus, Mult, Div,
  Value, Lbrace, Rbrace, Lpar, Rpar,
  Exponent, Index, Sub, Over,
  Newline, End, Error, None,
  Greek, Underline
} token_t;

typedef enum
{
  false, true
} bool;

#define POS_COUNT 10

#define SUB_LEVEL   0
#define NEXT        1
#define INDEX       2
#define EXPONENT    3
#define OVER        4
#define UNDER       5
#define NEWLINE     6
#define NOMINATOR   7
#define DENOMINATOR 8
#define UNDERLINE   9

typedef struct string_tt
{
  char *subStr;
  int font, prec;
  double width;
  double x, y;
  struct string_tt *next;
} string_t;


typedef struct formula_tt
{
  string_t *string;
  double myWidth, myHeight, myDepth;
  double totWidth, totHeight, totDepth;
  double x, y;
  int font, prec;
  token_t operator;		/* only: Plus, Minus, Mult or None  */
  struct formula_tt *next[POS_COUNT];
} formula_t;


#define GREEK_COUNT 54
#define GREEK_FONT   7

static
char *greek[] = {
  "alpha", "beta", "gamma", "Gamma", "delta", "Delta", "epsilon",
  "varepsilon", "zeta", "eta", "theta", "Theta", "vartheta", "iota",
  "kappa", "lambda", "Lambda", "mu", "nu", "xi", "Xi", "pi", "Pi",
  "varpi", "rho", "varrho", "sigma", "Sigma", "varsigma", "tau",
  "upsilon", "Upsilon", "phi", "Phi", "varphi", "chi", "psi", "Psi",
  "omega", "Omega", "Alpha", "Beta", "Epsilon", "Zeta", "Eta", "Iota",
  "Kappa", "Mu", "Nu", "Rho", "Tau", "Chi", "omicron", "Omicron"
};

static
int greekcode[] = {
  97, 98, 103, 71, 100, 68, 101,
  101, 122, 104, 113, 81, 74, 105,
  107, 108, 76, 109, 110, 120, 88, 112, 80,
  118, 114, 114, 115, 83, 86, 116,
  117, 85, 102, 70, 102, 99, 121, 89,
  119, 87, 'A', 'B', 'E', 'Z', 'H', 'I',
  'K', 'M', 'N', 'R', 'T', 'C', 'o', 'O',
  0
};


/* tree Operations */

static
void saveString(string_t ** string, char *s, int font, int prec)
{
  *string = calloc(1, sizeof(string_t));
  (*string)->next = NULL;
  (*string)->subStr = s;
  (*string)->font = font;
  (*string)->prec = prec;
}


static
void freeString(string_t * string)
{
  if (string->next != NULL)
    freeString(string->next);

  if (string->subStr != NULL)
    free(string->subStr);

  free(string);
}


static
void freeFormula(formula_t * formula)
{
  int i;

  for (i = 0; i < POS_COUNT; i++)
    if (formula->next[i] != NULL)
      {
        freeFormula(formula->next[i]);
        formula->next[i] = NULL;
      }

  if (formula->string != NULL)
    freeString(formula->string);

  free(formula);
}


static
void saveFormula(formula_t ** formula, formula_t * toSave, int pos,
		 token_t tok, string_t * s, int font, int prec)
{
  int i;

  *formula = calloc(1, sizeof(formula_t));

  for (i = 0; i < POS_COUNT; i++)
    {
      if (i == pos)
	(*formula)->next[i] = toSave;
      else
	(*formula)->next[i] = NULL;
    }

  (*formula)->operator = tok;
  (*formula)->string = s;
  (*formula)->font = font;
  (*formula)->prec = prec;

  (*formula)->myHeight = (*formula)->myDepth = (*formula)->myWidth = 0;
  (*formula)->totHeight = (*formula)->totDepth = (*formula)->totWidth = 0;
}

static
void increaseNominator(formula_t ** formula, formula_t * denom, int font,
		       int prec)
{
  formula_t *new;

  saveFormula(&new, (*formula)->next[NOMINATOR], NOMINATOR, None, NULL, font,
	      prec);
  new->next[DENOMINATOR] = denom;
  (*formula)->next[NOMINATOR] = new;
}


/*******************************************************************/
/*                                                                 */
/*                ######  #####   #    #  ######                   */
/*                #       #    #  ##   #  #                        */
/*                #####   #####   # #  #  #####                    */
/*                #       #    #  #  # #  #                        */
/*                #       #    #  #   ##  #                        */
/*                ######  #####   #    #  #                        */
/*                                                                 */
/*******************************************************************/


/* |----> Expression     --------->   next
   |         |                        newline
   |         |
   |         |
   |         v
   |   simpleExpression  --------->   next   ( '+' or '-' )
   |         |
   |	     |
   |	     |
   |	     v
   |       term          --------->   next  ( when '*' )
   |	     |                        nominator, denominator ( when '/' )
   |	     |
   |	     |
   |	     v
   |      factor         --------->   exponent
   |	     |                        index
   |	     |                        sub
   |	     |                        over
   |	     v
   |	   value         --------->   string_t
   |         |
   |---------|

*/


static token_t token;
static char *chin;

static bool Expression(formula_t **, int font, int prec);
static bool simpleExpression(formula_t **, int font, int prec);
static bool term(formula_t **, int font, int prec);
static bool factor(formula_t **, int font, int prec);
static bool value(formula_t **, int font, int prec);



static
char *findEndingPosition(token_t tok)
{
  char *s = chin;
  int d = 1;

  while (*s != '\0' && d != 0)
    {
      if (*s == '\\')
	++s;
      else if (*s == '{' && tok == Lbrace)
	d++;
      else if (*s == '}' && tok == Lbrace)
	d--;
      else if (*s == '(' && tok == Lpar)
	d++;
      else if (*s == ')' && tok == Lpar)
	d--;

      ++s;
    }

  if (d != 0)
    return NULL;
  else
    return s - 1;
}

static
token_t getToken(void)
{
  int len = 0, i, found = 0;

  switch (*chin++)
    {
    case '+':
      token = Plus;
      break;
    case '-':
      token = Minus;
      break;
    case '*':
      token = Mult;
      break;
    case '/':
      token = Div;
      break;
    case '{':
      token = Lbrace;
      break;
    case '}':
      token = Rbrace;
      break;
    case '(':
      token = Lpar;
      break;
    case ')':
      token = Rpar;
      break;
    case '_':
      token = Index;
      break;
    case '^':
      token = Exponent;
      break;
    case '\0':
      token = End;
      break;
    case '\\':
      switch (*chin)
	{
	case '\\':
	  token = Value;
	  ++chin;
	  break;
	case '^':
	  token = Value;
	  ++chin;
	  break;
	case '_':
	  token = Value;
	  ++chin;
	  break;
	case '+':
	  token = Value;
	  ++chin;
	  break;
	case '-':
	  token = Value;
	  ++chin;
	  break;
	case '/':
	  token = Value;
	  ++chin;
	  break;
	case '*':
	  token = Value;
	  ++chin;
	  break;
	case '{':
	  token = Value;
	  ++chin;
	  break;
	case '}':
	  token = Value;
	  ++chin;
	  break;
	case ' ':
	  token = Value;
	  ++chin;
	  break;
	case '(':
	  token = Value;
	  ++chin;
	  break;
	case ')':
	  token = Value;
	  ++chin;
	  break;
	case 'n':
	  token = Newline;
	  ++chin;
	  break;
	default:
	  if (strncmp(chin, "sub", 3) == 0)
	    {
	      token = Sub;
	      len = 3;
	    }
	  else if (strncmp(chin, "over", 4) == 0)
	    {
	      token = Over;
	      len = 4;
	    }
	  else if (strncmp(chin, "underline", 9) == 0)
	    {
	      token = Underline;
	      len = 9;
	    }
	  else
	    {
	      for (i = 0; i < GREEK_COUNT && !found; i++)
		{
		  len = strlen(greek[i]);
		  if (strncmp(chin, greek[i], len) == 0)
		    {
		      found = 1;
		      token = Greek;
		    }
		}
	      if (!found)
		{
		  fprintf(stderr, "%s: undefined symbol\n", chin - 1);
		  token = Error;
		  len = 0;
		}
	    }
	  chin += len;
	  if (found && *chin != ' ' && *chin != '{' && *chin != '\0'
	      && *chin != '\\' && *chin != '^' && *chin != '_' && *chin != '+'
	      && *chin != '-' && *chin != '*' && *chin != '/' && *chin != '=')
	    {
	      if (!isalnum(*chin) && !ispunct(*chin))
		{
		  fprintf(stderr, "%s: missing delimiter\n", chin - len - 1);
		  token = Error;
		}
	    }
	  break;
	}
      break;
    default:
      token = Value;
      break;
    }
  return token;
}

static
char *toGreek(char *string)
{
  int i, len;
  char *g_ptr = string;
  char *ret = calloc(strlen(string) + 1, sizeof(char));
  char *r_ptr = ret;

  do
    {
      g_ptr++;			/* overread '\' */

      for (i = 0; i < GREEK_COUNT; i++)
	{
	  len = strlen(greek[i]);
	  if (strncmp(g_ptr, greek[i], len) == 0)
	    {
	      *r_ptr++ = greekcode[i];
	      g_ptr += len;
	      i = GREEK_COUNT;
	    }
	}
    }
  while (*g_ptr != '\0');
  *r_ptr = '\0';

  return ret;
}


static
char *transform(char *s)
{
  char *ret = calloc(strlen(s) + 1, sizeof(char));
  char *r_ptr = ret;

  do
    {
      if (*s == '\\')
	++s;
      *r_ptr++ = *s;
    }
  while (*s++ != '\0');

  return ret;
}


static
bool getString(string_t ** string, char *start, int font, int prec)
{
  string_t **current = string;
  token_t val_or_greek;
  char *end, c;

  while (token == Value || token == Greek)
    {
      val_or_greek = token;
      end = chin;

      if (token == Value)
	while (getToken() == Value)
	  end = chin;
      else if (token == Greek)
	while (getToken() == Greek)
	  end = chin;

      c = *end;
      *end = '\0';

      if (val_or_greek == Value)
	saveString(current, transform(start), font, prec);
      else
	saveString(current, toGreek(start), GREEK_FONT, prec);

      current = &((*current)->next);

      *end = c;
      start = end;
    }

  if (token == Error)
    return false;
  else
    return true;
}


static
bool value(formula_t ** formula, int font, int prec)
{
  char *st = chin;
  token_t sign = None, saveTok;
  formula_t *sub;
  string_t *string = NULL;

  getToken();

  if (token == Plus || token == Minus)
    {
      sign = token;
      ++st;
      getToken();
    }

  if (token == Value || token == Greek || token == Underline)
    {
      if (getString(&string, st, font, prec) == false)
	return false;

      saveFormula(formula, NULL, -1, sign, string, font, prec);
      if (token == Lbrace)
	--chin;
    }
  else if (token == Lbrace || token == Lpar)
    {
      saveTok = token;

      st = findEndingPosition(token);
      if (st == NULL)
	{
	  fprintf(stderr, "missing '%c'\n", saveTok == Lbrace ? '}' : ')');
	  return false;
	}
      *st = '\0';

      if (Expression(&sub, font, prec) == false)
	return false;

      saveFormula(formula, sub, SUB_LEVEL, sign, NULL, font, prec);
      if (saveTok == Lbrace)
	*st = '}';
      else
	*st = ')';

      st = chin;
      getToken();

      if (token == Value || token == Lbrace || token == Greek
	  || token == Lpar)
	{
	  chin = st;
	  token = Lbrace;
	}
    }

  else if (token == End || token == Newline)
    saveFormula(formula, NULL, -1, sign, NULL, font, prec);
  else if (token == Error)
    return false;
  else
    {
      fprintf(stderr, "%s: extra characters following a valid expression\n",
	      st);
      return false;
    }

  return true;
}

static
bool factor(formula_t ** formula, int font, int prec)
{

  formula_t *sub;
  int pos = -1, first = 1;

  do
    {
      if (value(&sub, font, prec) == false)
	return false;

      if (first)
	{
	  saveFormula(formula, sub, SUB_LEVEL, None, NULL, font, prec);
	  first = 0;
	}
      else if ((*formula)->next[pos] != NULL)
	{
	  fprintf(stderr, "duplicate expression\n");
	  return false;
	}
      else
	(*formula)->next[pos] = sub;

      if (token == Index)
	pos = INDEX;
      else if (token == Exponent)
	pos = EXPONENT;
      else if (token == Sub)
	pos = UNDER;
      else if (token == Over)
	pos = OVER;
      else if (token == Underline)
	pos = UNDERLINE;
    }
  while (token == Exponent || token == Index || token == Sub
	 || token == Over || token == Underline);

  if (token == Error)
    return false;
  else
    return true;
}

static
bool term(formula_t ** formula, int font, int prec)
{
  formula_t *sub = NULL;
  formula_t **current = formula;
  int frac_found = 0;

  do
    {
      if (factor(&sub, font, prec) == false)
	return false;

      if (token == Div)
	{
	  if (frac_found)
	    increaseNominator(current, sub, font, prec);
	  else
	    saveFormula(current, sub, NOMINATOR, None, NULL, font, prec);
	  frac_found = 1;
	}
      else
	{
	  if (frac_found)
	    {
	      (*current)->next[DENOMINATOR] = sub;
	      frac_found = 0;
	      (*current)->operator = token == Mult ? Mult : None;
	    }
	  else
	    saveFormula(current, sub, SUB_LEVEL, token == Mult ? Mult : None,
			NULL, font, prec);

	  current = &((*current)->next[NEXT]);
	}
    }
  while (token == Div || token == Mult);

  if (token == Error)
    return false;
  else
    return true;
}


static
bool simpleExpression(formula_t ** formula, int font, int prec)
{
  formula_t *sub;
  formula_t **current = formula;

  do
    {
      if (term(&sub, font, prec) == false)
	return false;

      saveFormula(current, sub, SUB_LEVEL,
		  token == Plus
		  || token == Minus ? token : None, NULL, font, prec);
      current = &((*current)->next[NEXT]);
    }
  while (token == Plus || token == Minus);

  if (token == Error)
    return false;
  else
    return true;
}


static
bool Expression(formula_t ** formula, int font, int prec)
{
  formula_t *sub;
  formula_t **current = formula, **first = formula;

  do
    {
      if (simpleExpression(&sub, font, prec) == false)
	return false;

      saveFormula(current, sub, SUB_LEVEL, None, NULL, font, prec);

      if (token == Newline)
	first = current = &((*first)->next[NEWLINE]);
      else
	{
	  current = &((*current)->next[NEXT]);
	}
    }
  while (token == Newline || token == Lbrace || token == Value);

  if (token == Error)
    return false;
  else if (token == Rbrace)
    {
      fprintf(stderr, "mismatched '}'\n");
      return false;
    }
  else
    return true;
}



/*******************************************************************/
/*                                                                 */
/*                       ####   #    #   ####                      */
/*                      #    #  #   #   #                          */
/*                      #       ####     ####                      */
/*                      #  ###  #  #         #                     */
/*                      #    #  #   #   #    #                     */
/*                       ####   #    #   ####                      */
/*                                                                 */
/*******************************************************************/

#include "gks.h"

#define TOP_PERCENT    0.12
#define BOTTOM_PERCENT 0.33
#define PERCENT_UNDERLINE 0.07
#define FRACTION_FAC 0.6
#define UNDERLINE_FAC 0.5

#define TOPRIGHT_POS    (3.0 / 6.4)
#define BOTTOMRIGHT_POS (3.3 / 6.4)

#define NEWLINE_SPACE   (0.3)

#define PAINT_FAC         (800)

#define FRAC_THICK        (.9 / 8.95)
#define FRAC_SPACE        (1 / 20.)
#define FRAC_WIDTH_FACTOR (1. / 10.)
#define FRAC_PAINT        FRAC_THICK * PAINT_FAC

static double sinphi, cosphi;

static double scales[] = { 1.0,	/* SUB_LEVEL */
  1.0,				/* NEXT */
  5.2 / 6.4,			/* INDEX */
  5.2 / 6.4,			/* EXPONENT */
  5.2 / 6.4,			/* OVER */
  5.2 / 6.4,			/* UNDER */
  1.0,				/* NEWLINE */

  5.8 / 6.4,			/* NOMINATOR */
  5.8 / 6.4,			/* DENOMINATOR */
  1.0				/* UNDERLINE */
};


static
double textheight(void)
{
  int errind;
  double height;

  gks_inq_text_height(&errind, &height);

  return height;
}

static
double textwidth(char *string, int font, int prec)
{
  double cpx, cpy, trx[5], try[5], qx = 0, qy = 0;
  int errind = 0, n = 0, wkid = 0;

  gks_inq_open_ws(1, &errind, &n, &wkid);

  gks_set_text_fontprec(font, prec);
  gks_set_text_upvec(0, 1);

  gks_inq_text_extent(wkid, qx, qy, string, &errind, &cpx, &cpy, trx, try);

  return trx[1] - trx[0];
}


static
double operatorLen(token_t operator, int font, int prec)
{
  double len;
  switch (operator)
    {
    case None:
      len = 0;
      break;
    case Mult:
      len = textwidth("*", font, prec);
      break;
    case Plus:
      len = textwidth("+", font, prec);
      break;
    case Minus:
      /* Minus.len < Plus.len => mLen = pLen */
      len = textwidth("+", font, prec);
      break;
    default:
      fprintf(stderr, "invalid operator\n");
      len = 0;
      break;
    }
  return len;
}

static
double stringWidth(string_t * str)
{
  double nxt = 0;

  str->width = textwidth(str->subStr, str->font, str->prec);
  if (str->next != NULL)
    nxt = stringWidth(str->next);

  return str->width + nxt;
}


static
void heightAndWidth(formula_t * formula, double scale)
{
  string_t *str;
  formula_t *help;
  double txt_h = scale * textheight();
  double w, addLeft, addRight;
  int i;

  for (i = 0; i < POS_COUNT; i++)
    if (formula->next[i] != NULL)
      heightAndWidth(formula->next[i], scale * scales[i]);


  /* SUBLEVEL */

  if ((help = formula->next[SUB_LEVEL]) != NULL)
    {
      formula->myHeight = help->totHeight;
      formula->myDepth = help->totDepth;
      formula->myWidth = help->totWidth;
    }

  /* FRACTION */

  else if ((help = formula->next[NOMINATOR]) != NULL)
    {
      formula->myHeight = txt_h * (.5 + FRAC_THICK / 2 + FRAC_SPACE)
	+ help->totHeight + help->totDepth;

      help = formula->next[DENOMINATOR];
      formula->myDepth = txt_h * (FRAC_THICK / 2 + FRAC_SPACE)
	+ help->totHeight + help->totDepth - txt_h / 2;

      if (formula->myDepth < 0)
	formula->myDepth = txt_h * BOTTOM_PERCENT;

      formula->myWidth = MAX(formula->next[DENOMINATOR]->totWidth,
			     formula->next[NOMINATOR]->totWidth)
	* (1.0 + 2 * FRAC_WIDTH_FACTOR);
    }

  /* STRING */

  else if (formula->string != NULL)
    {
      str = formula->string;

      formula->myHeight = txt_h * (1.0 + TOP_PERCENT);
      formula->myDepth = txt_h * BOTTOM_PERCENT;
      formula->myWidth = scale * stringWidth(str)
	+ scale * operatorLen(formula->operator, str->font, str->prec);
    }

  /* STRING can be NULL when input = "{}" */

  else
    {
      formula->myHeight = txt_h * (1.0 + TOP_PERCENT);
      formula->myDepth = txt_h * BOTTOM_PERCENT;
      formula->myWidth = 0;
    }

  /* setting total-attributes ( firstly ) */

  formula->totHeight = formula->myHeight;
  formula->totDepth = formula->myDepth;
  formula->totWidth = formula->myWidth;


  /* looking for optional attributes - exclusive */


  /* NEXT ? */

  if ((help = formula->next[NEXT]) != NULL)
    {
      formula->totWidth = formula->myWidth
	+ scale * operatorLen(formula->operator, formula->font, formula->prec)
	+ help->totWidth;
      formula->totHeight = MAX(formula->myHeight, help->totHeight);
      formula->totDepth = MAX(formula->myDepth, help->totDepth);
    }

  /* Exponent, Index, 'Over' or 'Sub' */

  else if (formula->next[EXPONENT] != NULL || formula->next[INDEX] != NULL ||
	   formula->next[OVER] != NULL || formula->next[UNDER] != NULL
	   || formula->next[UNDERLINE] != NULL)
    {
      if ((help = formula->next[EXPONENT]) != NULL)
	{
	  formula->totWidth = formula->myWidth + help->totWidth;
	  formula->totHeight = MAX(formula->myHeight,
				   formula->myHeight * TOPRIGHT_POS +
				   help->totDepth + help->totHeight);
	}

      if ((help = formula->next[INDEX]) != NULL)
	{
	  formula->totWidth =
	    MAX(formula->totWidth, formula->myWidth + help->totWidth);
	  formula->totDepth =
	    MAX(formula->myDepth,
		help->totDepth + help->totHeight - txt_h * BOTTOMRIGHT_POS);
	}
      if ((help = formula->next[UNDERLINE]) != NULL)
	{
	  formula->totWidth =
	    MAX(formula->totWidth, formula->myWidth + help->totWidth);
	  formula->totDepth =
	    MAX(formula->myDepth,
		help->totDepth + txt_h * PERCENT_UNDERLINE +
		txt_h * FRAC_THICK * UNDERLINE_FAC);
	}

      w = 0;

      if ((help = formula->next[OVER]) != NULL)
	{
	  w = help->totWidth;
	  formula->totHeight += help->totHeight + help->totDepth;
	}

      if ((help = formula->next[UNDER]) != NULL)
	{
	  w = MAX(w, help->totWidth);
	  formula->totDepth += help->totHeight + help->totDepth;
	}

      if (w > formula->myWidth)
	{			/* Value centered */
	  addLeft = (w - formula->myWidth) / 2;
	  addRight = MAX(formula->totWidth - formula->myWidth, addLeft);
	  formula->totWidth = addLeft + formula->myWidth + addRight;
	}
    }

  /* NEWLINE ? */

  if ((help = formula->next[NEWLINE]) != NULL)
    {
      formula->totWidth = MAX(formula->totWidth, help->totWidth);
      formula->totDepth +=
	help->totHeight + help->totDepth + txt_h * NEWLINE_SPACE;
    }
}

static
void xyStringPos(double x, double y, string_t * string, double scale)
{
  string->x = x;
  string->y = y;
  if (string->next != NULL)
    xyStringPos(x + string->width * scale, y, string->next, scale);
}

static
void xyPos(double x, double y, formula_t * formula, double scale)
{
  formula_t *help;
  string_t *str;
  double addLeft = 0, w, shift = 0;
  double txt_h = scale * textheight();

  formula->x = x;
  formula->y = y;


  /* SUBLEVEL */

  if (formula->next[SUB_LEVEL] != NULL)
    {
      w = 0;
      if ((help = formula->next[UNDER]) != NULL)
	w = help->totWidth;
      if ((help = formula->next[OVER]) != NULL)
	w = MAX(w, help->totWidth);

      if (w > formula->myWidth)
	{
	  addLeft = (w - formula->myWidth) / 2.;
	  formula->x += addLeft;
	}

      xyPos(formula->x,
	    formula->y, formula->next[SUB_LEVEL], scale * scales[SUB_LEVEL]);
    }

  /* FRACTION */

  else if ((help = formula->next[NOMINATOR]) != NULL)
    {
      xyPos(x + (formula->myWidth - help->totWidth) / 2.,
	    y + txt_h * (0.5 + FRAC_THICK / 2 + FRAC_SPACE) + help->totDepth,
	    help, scale * scales[NOMINATOR]);

      help = formula->next[DENOMINATOR];
      xyPos(x + (formula->myWidth - help->totWidth) / 2.,
	    y + txt_h * (.5 - FRAC_THICK / 2 - FRAC_SPACE) - help->totHeight,
	    help, scale * scales[DENOMINATOR]);
    }

  /* STRING */

  else if ((str = formula->string) != NULL)
    {
      xyStringPos(x +
		  scale * operatorLen(formula->operator, str->font,
				      str->prec), y, formula->string, scale);
    }


  /* looking for optional attributes - exclusive */


  /* NEXT ? */

  if ((help = formula->next[NEXT]) != NULL)
    {
      xyPos(x + formula->myWidth +
	    scale * operatorLen(formula->operator, formula->font,
				formula->prec), y, help,
	    scale * scales[NEXT]);
    }

  /* Exponent, Index, 'Over' or 'Sub' */

  else if (formula->next[EXPONENT] != NULL || formula->next[INDEX] != NULL ||
	   formula->next[OVER] != NULL || formula->next[UNDER] != NULL
	   || formula->next[UNDERLINE] != NULL)
    {
      if ((help = formula->next[EXPONENT]) != NULL)
	{
	  xyPos(x + addLeft + formula->myWidth,
		y + formula->myHeight * TOPRIGHT_POS + help->totDepth,
		help, scale * scales[EXPONENT]);
	}

      if ((help = formula->next[INDEX]) != NULL)
	{
	  xyPos(x + addLeft + formula->myWidth,
		y + txt_h * BOTTOMRIGHT_POS - help->totHeight,
		help, scale * scales[INDEX]);
	}
      if ((help = formula->next[UNDERLINE]) != NULL)
	{
	  xyPos(x + addLeft + formula->myWidth,
		y, help, scale * scales[UNDERLINE]);
	}
      if ((help = formula->next[OVER]) != NULL)
	{
	  if ((help->totWidth - formula->myWidth) / 2. == addLeft)
	    shift = 0.;
	  else if (help->totWidth < formula->myWidth)
	    shift = (formula->myWidth - help->totWidth) / 2. + addLeft;
	  else
	    shift = addLeft - (help->totWidth - formula->myWidth) / 2.;

	  xyPos(x + shift,
		y + formula->totHeight - help->totHeight,
		help, scale * scales[OVER]);
	}

      if ((help = formula->next[UNDER]) != NULL)
	{
	  if ((help->totWidth - formula->myWidth) / 2. == addLeft)
	    shift = 0.;
	  else if (help->totWidth < formula->myWidth)
	    shift = (formula->myWidth - help->totWidth) / 2. + addLeft;
	  else
	    shift = addLeft - (help->totWidth - formula->myWidth) / 2.;

	  xyPos(x + shift,
		y - formula->totDepth + help->totDepth,
		help, scale * scales[UNDER]);
	}
    }


  /* NEWLINE ? */

  if ((help = formula->next[NEWLINE]) != NULL)
    {
      xyPos(x,
	    y - (formula->totDepth - shift - help->totDepth),
	    help, scale * scales[NEWLINE]);
    }
}


static
void shiftString(string_t * string, double sx, double sy)
{
  string->x += sx;
  string->y += sy;

  if (string->next != NULL)
    shiftString(string->next, sx, sy);
}


static
void shiftFormula(formula_t * formula, double sx, double sy)
{
  int i;

  formula->x += sx;
  formula->y += sy;

  for (i = 0; i < POS_COUNT; i++)
    if (formula->next[i] != NULL)
      shiftFormula(formula->next[i], sx, sy);

  if (formula->string != NULL)
    shiftString(formula->string, sx, sy);
}

static
void setInnerAlignment(int align, formula_t * formula, double width)
{
  double x_shift = 0;
  double lineWidth;
  int i;
  formula_t *help;

  if (formula->next[NEWLINE] == NULL)

    lineWidth = formula->totWidth;

  else
    {
      lineWidth = formula->myWidth;
      if ((help = formula->next[NEXT]) != NULL)
	lineWidth += help->totWidth;
    }


  if (align == GKS_K_TEXT_HALIGN_CENTER)
    x_shift = (width - lineWidth) / 2;
  else
    x_shift = width - lineWidth;

  formula->x += x_shift;

  for (i = 0; i < POS_COUNT; i++)
    {
      if ((help = formula->next[i]) != NULL)
	{
	  if (i != NEWLINE)
	    {
	      if (x_shift != 0.)
		shiftFormula(help, x_shift, 0.);
	      setInnerAlignment(align, help, help->totWidth);
	    }
	  else
	    {
	      setInnerAlignment(align, help, width);
	    }
	}
    }
}



static
void rotateString(double x, double y, string_t * string)
{
  double delta_x = string->x - x;
  double delta_y = string->y - y;

  string->x = x + delta_x * cosphi + delta_y * sinphi;
  string->y = y - delta_x * sinphi + delta_y * cosphi;

  if (string->next != NULL)
    rotateString(x, y, string->next);
}

static
void rotate(double x, double y, formula_t * formula)
{
  int i;
  double delta_x = formula->x - x;
  double delta_y = formula->y - y;

  formula->x = x + delta_x * cosphi + delta_y * sinphi;
  formula->y = y - delta_x * sinphi + delta_y * cosphi;

  for (i = 0; i < POS_COUNT; i++)
    if (formula->next[i] != NULL)
      rotate(x, y, formula->next[i]);

  if (formula->string != NULL)
    rotateString(x, y, formula->string);
}

static
void drawOperator(double x, double y, token_t operator, double txt_h, int font,
		  int prec)
{
  gks_set_text_fontprec(font, prec);
  gks_set_text_height(txt_h);
  gks_set_text_upvec(sinphi, cosphi);

  switch (operator)
    {
    case Plus:
      gks_text(x, y, "+");
      break;
    case Minus:
      gks_text(x, y, "-");
      break;
    case Mult:
      gks_text(x, y, "*");
      break;
    default:
      break;
    }
}

static
void drawString(string_t * str, double txt_h)
{
  gks_set_text_fontprec(str->font, str->prec);
  gks_set_text_height(txt_h);
  gks_set_text_upvec(sinphi, cosphi);
  gks_text(str->x, str->y, str->subStr);

  if (str->next != NULL)
    drawString(str->next, txt_h);
}

static
void drawFormula(formula_t * formula, double Height, double scale)
{
  int i;
  double txt_h = Height * scale;
  double x[2], y[2];

  for (i = 0; i < POS_COUNT; i++)
    if (formula->next[i] != NULL)
      drawFormula(formula->next[i], Height, scale * scales[i]);


  if (formula->string != NULL)
    {
      drawOperator(formula->x, formula->y,	/* sign */
		   formula->operator, txt_h, formula->font, formula->prec);
      drawString(formula->string, txt_h);
    }
  else if (formula->next[UNDERLINE] != NULL)
    {
      x[0] =
	formula->next[UNDERLINE]->x - (formula->next[UNDERLINE]->totDepth +
				       txt_h * PERCENT_UNDERLINE) * sinphi;
      y[0] =
	formula->next[UNDERLINE]->y - (formula->next[UNDERLINE]->totDepth +
				       txt_h * PERCENT_UNDERLINE) * cosphi;

      x[1] = x[0] + formula->next[UNDERLINE]->totWidth * cosphi;
      y[1] = y[0] - formula->next[UNDERLINE]->totWidth * sinphi;

      gks_set_pline_linewidth(txt_h * FRAC_PAINT * UNDERLINE_FAC);
      gks_polyline(2, x, y);
    }
  else if (formula->next[NOMINATOR] != NULL)
    {
      x[0] = formula->x + txt_h / 2 * sinphi;
      y[0] = formula->y + txt_h / 2 * cosphi;

      x[1] = x[0] + formula->myWidth * cosphi;
      y[1] = y[0] - formula->myWidth * sinphi;

      gks_set_pline_linewidth(txt_h * FRAC_PAINT * FRACTION_FAC);
      gks_polyline(2, x, y);
    }

  if (formula->next[NEXT] != NULL)
    drawOperator(formula->x + formula->myWidth * cosphi,
		 formula->y - formula->myWidth * sinphi,
		 formula->operator, txt_h, formula->font, formula->prec);

}

static char *pre_parse_escape(const char *string) {
  const char *c;
  char *escaped_string;
  char *ec;
  int escapable_parentheses = 0;
  int escapable_braces = 0;
  int max_len = strlen(string)*3;
  escaped_string = malloc(max_len+1);
  assert(escaped_string);
  for (c = string, ec=escaped_string; c[0] != 0; c++, ec++) {
    ec[0] = c[0];
    if (c[0] == '\\' && c[1] == '\\') {
      ec[1] = '\\';
      c++;
    } else if (c[0] == '\\' && c[1] == ' ') {
      ec[0] = ' ';
      c++;
    } else if (c[0] == ' ' && (c[1] == '+' || c[1] == '-' || c[1] == '/' || c[1] == '*' || c[1] == '^' || c[1] == '_' || c[1] == '(' || c[1] == ')' || c[1] == '{' || c[1] == '}')) {
      ec[1] = '\\';
      ec[2] = c[1];
      c++;
      ec += 2;
      if (c[0] == '(') {
        escapable_parentheses++;
      } else if (c[0] == '{') {
        escapable_braces++;
      } else if (c[0] == ')') {
        escapable_parentheses--;
      } else if (c[0] == '}') {
        escapable_braces--;
      }
    } else if (escapable_parentheses && c[0] == ')') {
      ec[0] = '\\';
      ec[1] = ')';
      ec++;
      escapable_parentheses--;
    } else if (escapable_braces && c[0] == '}') {
      ec[0] = '\\';
      ec[1] = '}';
      ec++;
      escapable_braces--;
    }
  }
  ec[0] = 0;
  return escaped_string;
}

/***************************************************************/
/*                                                             */
/*                      gr_textex                              */
/*                                                             */
/***************************************************************/
int gr_textex(double x, double y, const char *string, int inquire,
	      double *tbx, double *tby)
{
  char *str = pre_parse_escape(string);
  int n, wkid = 0;
  double cpx, cpy;
  formula_t *formula = NULL;
  int errind, font, prec;
  int align_hor, align_ver;
  double height;
  double x_shift, y_shift;

  if (inquire && strlen(str) == 1)
    {
      gks_inq_open_ws(1, &errind, &n, &wkid);
      gks_inq_text_extent(wkid, x, y, str, &errind, &cpx, &cpy, tbx, tby);
      return 1;
    }

  chin = str;

  gks_inq_text_fontprec(&errind, &font, &prec);
  gks_inq_text_height(&errind, &height);
  gks_inq_text_upvec(&errind, &sinphi, &cosphi);
  gks_inq_text_align(&errind, &align_hor, &align_ver);

  if (Expression(&formula, font, prec) == false)
    {
      if (formula != NULL)
	freeFormula(formula);
      free(str);
      return 0;
    }

  if (formula == NULL)
    {
      fprintf(stderr, "string is empty\n");
      free(str);
      return 0;
    }


  gks_set_text_upvec(0, 1);
  gks_set_text_align(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_BASE);


  heightAndWidth(formula, 1.0);


  /* check alignment */

  switch (align_ver)
    {
    case GKS_K_TEXT_VALIGN_TOP:
      y_shift = -(formula->totHeight + height * TOP_PERCENT);
      break;
    case GKS_K_TEXT_VALIGN_CAP:
      y_shift = -(formula->totHeight);
      break;
    case GKS_K_TEXT_VALIGN_HALF:
      y_shift =
	-(formula->totHeight + formula->totDepth) / 2. + formula->totDepth;
      break;
    case GKS_K_TEXT_VALIGN_BOTTOM:
      y_shift = formula->totDepth;
      break;
    default:
      y_shift = 0;
    }

  switch (align_hor)
    {
    case GKS_K_TEXT_HALIGN_CENTER:
      x_shift = -formula->totWidth / 2.;
      break;
    case GKS_K_TEXT_HALIGN_RIGHT:
      x_shift = -formula->totWidth;
      break;
    default:
      x_shift = 0.;
      break;
    }

  /* draw or inquire */

  if (!inquire)
    {
      xyPos(x, y, formula, 1.0);	/* without rotating */

      if (x_shift != 0. || y_shift != 0.)
	{
	  shiftFormula(formula, x_shift, y_shift);	/* box alignment    */
	}

      if (align_hor == GKS_K_TEXT_HALIGN_CENTER ||
	  align_hor == GKS_K_TEXT_HALIGN_RIGHT)
	{
	  /* inner alignment  */
	  setInnerAlignment(align_hor, formula, formula->totWidth);
	}

      rotate(x, y, formula);	/* rotate           */

      drawFormula(formula, height, 1.0);
    }
  else
    {
      /*   3----2   */
      /*   |    |   */
      /*   0----1   */

      tbx[0] = x + x_shift * cosphi + (y_shift - formula->totDepth) * sinphi;
      tby[0] = y - x_shift * sinphi + (y_shift - formula->totDepth) * cosphi;

      tbx[1] = tbx[0] + formula->totWidth * cosphi;
      tby[1] = tby[0] - formula->totWidth * sinphi;

      tbx[3] = x + x_shift * cosphi + (y_shift + formula->totHeight) * sinphi;
      tby[3] = y - x_shift * sinphi + (y_shift + formula->totHeight) * cosphi;

      tbx[2] = tbx[3] + formula->totWidth * cosphi;
      tby[2] = tby[3] - formula->totWidth * sinphi;

    }

  gks_set_text_height(height);
  gks_set_text_upvec(sinphi, cosphi);
  gks_set_text_align(align_hor, align_ver);
  gks_set_text_fontprec(font, prec);


  freeFormula(formula);
  free(str);

  return 1;
}
