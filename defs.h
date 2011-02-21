#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>


/*  machine-dependent definitions			*/
/*  the following definitions are for the Tahoe		*/
/*  they might have to be changed for other machines	*/

/*  MAXCHAR is the largest unsigned character value	*/
/*  MAXSHORT is the largest value of a C short		*/
/*  MINSHORT is the most negative value of a C short	*/
/*  MAXTABLE is the maximum table size			*/
/*  BITS_PER_WORD is the number of bits in a C unsigned	*/
/*  WORDSIZE computes the number of words needed to	*/
/*	store n bits					*/
/*  BIT returns the value of the n-th bit starting	*/
/*	from r (0-indexed)				*/
/*  SETBIT sets the n-th bit starting from r		*/

#define	MAXCHAR		255
#define	MAXSHORT	((int)0x7FFFFFFF)
#define MINSHORT	((int)0x80000000)
#define MAXTABLE	120000

#ifdef __MSDOS__
#define BITS_PER_WORD   16
#define LOG2_BPW    4
#else    /* Real computers... */
#define BITS_PER_WORD	32
#define LOG2_BPW    5
#endif
#define BITS_PER_WORD_1 (BITS_PER_WORD-1)

#define WORDSIZE(n) (((n)+(BITS_PER_WORD_1))/BITS_PER_WORD)
#define BIT(r, n)   ((((r)[(n)>>LOG2_BPW])>>((n)&BITS_PER_WORD_1))&1)
#define SETBIT(r, n) ((r)[(n)>>LOG2_BPW]|=((unsigned)1<<((n)&BITS_PER_WORD_1)))

/* VM: this is a 32-bit replacement for original 16-bit short */
typedef int Yshort;


/*  character names  */

#define	NUL		'\0'    /*  the null character  */
#define	NEWLINE		'\n'    /*  line feed  */
#define	SP		' '     /*  space  */
#define	BS		'\b'    /*  backspace  */
#define	HT		'\t'    /*  horizontal tab  */
#define	VT		'\013'  /*  vertical tab  */
#define	CR		'\r'    /*  carriage return  */
#define	FF		'\f'    /*  form feed  */
#define	QUOTE		'\''    /*  single quote  */
#define	DOUBLE_QUOTE	'\"'    /*  double quote  */
#define	BACKSLASH	'\\'    /*  backslash  */


/* defines for constructing filenames */

#define DEFINES_SUFFIX  "_tab.h"
#define OUTPUT_SUFFIX   "_tab.c"
#define CODE_SUFFIX     "_code.c"
#define VERBOSE_SUFFIX  ".output"

typedef enum keyword_code_enumeration
{
TOKEN = 0,
LEFT = 1,
RIGHT = 2,
NONASSOC = 3,
MARK = 4,
TEXT = 5,
TYPE = 6,
START = 7,
UNION = 8,
IDENT = 9
} BtYacc_keyword_code;

typedef enum symbol_class_enumeration
{
UNKNOWN = 0,
TERM = 1,
NONTERM = 2,
ACTION = 3,
ARGUMENT = 4
} BtYacc_symbol_class;


/*  the undefined value  */

#define UNDEFINED (-1)


typedef enum action_code_enumeration
{
SHIFT = 1,
REDUCE = 2
} BtYacc_action_code;


/*  character macros  */

#define IS_IDENT(c)	(isalnum(c) || (c) == '_' || (c) == '.' || (c) == '$')
#define	IS_OCTAL(c)	((c) >= '0' && (c) <= '7')
#define	NUMERIC_VALUE(c)	((c) - '0')


/*  symbol macros  */

#define ISTOKEN(s)	((s) < start_symbol)
#define ISVAR(s)	((s) >= start_symbol)


/*  storage allocation macros  */

#define CALLOC(k,n)	(calloc((unsigned)(k),(unsigned)(n)))
#define	FREE(x)		(free((char*)(x)))
#define MALLOC(n)	(malloc((unsigned)(n)))
#define	NEW(t)		((t*)allocate(sizeof(t)))
#define	NEW2(n,t)	((t*)allocate((unsigned)((n)*sizeof(t))))
#define REALLOC(p,n)	(realloc((char*)(p),(unsigned)(n)))
#define RENEW(p,n,t)	((t*)realloc((char*)(p),(unsigned)((n)*sizeof(t))))


/*  the structure of a symbol table entry  */

typedef struct bucket bucket;
struct bucket
{
    struct bucket *link;
    struct bucket *next;
    char *name;
    char *tag;
    char **argnames;
    char **argtags;
    Yshort args;
    Yshort value;
    Yshort index;
    Yshort prec;
    char class;
    char assoc;
};


/*  the structure of the LR(0) state machine  */

typedef struct core core;
struct core
{
    struct core *next;
    struct core *link;
    Yshort number;
    Yshort accessing_symbol;
    Yshort nitems;
    Yshort items[1];
};


/*  the structure used to record shifts  */

typedef struct shifts shifts;
struct shifts
{
    struct shifts *next;
    Yshort number;
    Yshort nshifts;
    Yshort shift[1];
};


/*  the structure used to store reductions  */

typedef struct reductions reductions;
struct reductions
{
    struct reductions *next;
    Yshort number;
    Yshort nreds;
    Yshort rules[1];
};


/*  the structure used to represent parser actions  */

typedef struct action action;
struct action
{
    struct action *next;
    Yshort symbol;
    Yshort number;
    Yshort prec;
    char   action_code;
    char   assoc;
    char   suppressed;
};

struct section {
    char const * name;
    char const * const * ptr;
};

extern struct section section_list[];


/* global variables */

extern char dflag;
extern char lflag;
extern char rflag;
extern char tflag;
extern char vflag;

extern char *myname;
extern char *cptr;
extern char *line;
extern int unsigned lineno;
extern int outline;

extern char const * const banner[];
extern char const * const tables[];
extern char const * const header[];
extern char const * const body[];
extern char const * const trailer[];

extern char *action_file_name;
extern char *code_file_name;
extern char *defines_file_name;
extern char *input_file_name;
extern char *output_file_name;
extern char *text_file_name;
extern char *union_file_name;
extern char *verbose_file_name;

extern FILE *inc_file;
extern char  inc_file_name[];

extern FILE *action_file;
extern FILE *code_file;
extern FILE *defines_file;
extern FILE *input_file;
extern FILE *output_file;
extern FILE *text_file;
extern FILE *union_file;
extern FILE *verbose_file;

extern int unsigned nitems;
extern int unsigned nrules;
extern int unsigned nsyms;
extern int unsigned ntokens;
extern int unsigned nvars;
extern int ntags;

extern char unionized;
extern char const line_format[];

extern int   start_symbol;
extern char  **symbol_name;
extern Yshort *symbol_value;
extern Yshort *symbol_prec;
extern char  *symbol_assoc;

extern Yshort *ritem;
extern Yshort *rlhs;
extern Yshort *rrhs;
extern Yshort *rprec;
extern char  *rassoc;

extern Yshort **derives;
extern char *nullable;

extern bucket *first_symbol;
extern bucket *last_symbol;

extern int unsigned nstates;
extern core *first_state;
extern shifts *first_shift;
extern reductions *first_reduction;
extern Yshort *accessing_symbol;
extern core **state_table;
extern shifts **shift_table;
extern reductions **reduction_table;
extern unsigned *LA;
extern Yshort *LAruleno;
extern Yshort *lookaheads;
extern Yshort *goto_map;
extern Yshort *from_state;
extern Yshort *to_state;

extern action **parser;
extern int unsigned SRtotal;
extern int unsigned RRtotal;
extern Yshort *SRconflicts;
extern Yshort *RRconflicts;
extern Yshort *defred;
extern Yshort *rules_used;
extern Yshort nunused;
extern Yshort final_state;

/* system variable */
#include <errno.h>

#if defined(__GNUC__)
#  if defined(__ICC)
#    define ATTRIBUTE(x)
#  else
#    if ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#      define ATTRIBUTE(x) __attribute__(x)
#      define CAN_USE_ATTRIBUTE 1
#    else
#      define ATTRIBUTE(x)
#    endif
#  endif
#else
#  define ATTRIBUTE(x)
#endif

#define GCC_NO_RETURN ATTRIBUTE((noreturn))

#ifndef S_SPLINT_S
#  define SPLINT_NO_RETURN
#else
#  define SPLINT_NO_RETURN /*@noreturn@*/
#endif

/* global functions */

/* closure.c */
void set_first_derives(void);
void closure(Yshort *, int);
void finalize_closure(void);

/* error.c */
SPLINT_NO_RETURN void fatal(char const * msg) GCC_NO_RETURN;
SPLINT_NO_RETURN void no_space(void) GCC_NO_RETURN;
SPLINT_NO_RETURN void open_error(char const * filename) GCC_NO_RETURN;
SPLINT_NO_RETURN void unexpected_EOF(void) GCC_NO_RETURN;
void print_pos(char const * st_line, char const * st_cptr);
void error(int unsigned lineno, char const * line, char const * cptr, char const * msg, ...);
SPLINT_NO_RETURN void syntax_error(int unsigned lineno, char const * line, char const * cptr) GCC_NO_RETURN;
SPLINT_NO_RETURN void unterminated_comment(int unsigned lineno, char const * line, char const * cptr) GCC_NO_RETURN;
SPLINT_NO_RETURN void unterminated_string(int unsigned lineno, char const * line, char const * cptr) GCC_NO_RETURN;
SPLINT_NO_RETURN void unterminated_text(int unsigned lineno, char const * line, char const * cptr) GCC_NO_RETURN;
SPLINT_NO_RETURN void unterminated_union(int unsigned lineno, char const * line, char const * cptr) GCC_NO_RETURN;
SPLINT_NO_RETURN void over_unionized(char const * cptr) GCC_NO_RETURN;
void illegal_tag(int unsigned lineno, char const * line, char const * cptr);
void illegal_character(char const * cptr);
void used_reserved(char const * s);
void tokenized_start(char const * s);
void retyped_warning(char const * s);
void reprec_warning(char const * s);
void revalued_warning(char const * s);
void terminal_start(char const * s);
void restarted_warning(void);
void no_grammar(void);
void terminal_lhs(int);
void prec_redeclared(void);
void unterminated_action(int unsigned lineno, char const * line, char const * cptr);
void unterminated_arglist(int unsigned lineno, char const * line, char const * cptr);
void bad_formals(void);
void dollar_warning(int unsigned, int);
void dollar_error(int unsigned lineno, char const * line, char const * cptr);
void untyped_lhs(void);
void untyped_rhs(int i, char const * s);
void unknown_rhs(int);
void default_action_warning(void);
void undefined_goal(char const * s);
void undefined_symbol_warning(char const * s);

/* lalr.c */
void lalr(void);

/* lr0.c */
void show_cores(void);
void show_ritems(void);
void show_rrhs(void);
void show_shifts(void);
void free_derives(void);
void free_nullable(void);
void lr0(void);

/* main.c */
SPLINT_NO_RETURN void done(int) GCC_NO_RETURN;

#if defined(SIGINT) || defined(SIGTERM) || defined(SIGHUP)
#define BTYACC_USE_SIGNAL_HANDLING
void BtYacc_stop_test(void);
#define BTYACC_INTERRUPTION_CHECK BtYacc_stop_test();
#else
#define BTYACC_INTERRUPTION_CHECK
#endif

char *allocate(unsigned);
int main(int, char **);

/* mkpar.c */
void make_parser(void);
action* parse_actions(int unsigned);
action* get_shifts(int unsigned);
action* add_reductions(int unsigned, action*);
action* add_reduce(action*, int unsigned, int);
int unsigned sole_reduction(int unsigned);
void free_action_row(action *);
void free_parser(void);

/* output.c */
void output(void);
void output_rule_data(void);
void output_yydefred(void);
void output_actions(void);
int matching_vector(size_t);
int pack_vector(size_t);
void output_base(void);
void output_table(void);
void output_check(void);
void output_ctable(void);
int is_C_identifier(char const * name);
void output_defines(void);
void output_stored_text(void);
void output_debug(void);
void output_stype(void);
void output_trailing_text(void);
void output_semantic_actions(void);
void free_itemsets(void);
void free_shifts(void);
void free_reductions(void);
void write_section(char const * section_name);

/* reader.c */
int cachec(int);
char *get_line(void);
char *dup_line(void);
char *skip_comment(void);
int nextc(void);
int keyword(void);
void copy_ident(void);
void copy_string(int, FILE *, FILE *);
void copy_comment(FILE *, FILE *);
void copy_text(void);
void copy_union(void);
int hexval(int);
bucket *get_literal(void);
int is_reserved(char const * name);
bucket *get_name(void);
int get_number(void);
char *get_tag(void);
void declare_tokens(int);
void declare_types(void);
void declare_start(void);
void read_declarations(void);
void initialize_grammar(void);
void expand_items(void);
void expand_rules(void);
void advance_to_start(void);
void start_rule(bucket *, int);
void end_rule(void);
void insert_empty_rule(void);
void add_symbol(void);
void copy_action(void);
int mark_symbol(void);
void read_grammar(void);
void free_tags(void);
void pack_names(void);
void check_symbols(void);
void pack_symbols(void);
void pack_grammar(void);
void print_grammar(void);
void reader(void);

/* readskel.c */
void read_skel(char const * name);

/* symtab.c */
int hash(char const * name);
bucket* make_bucket(char const * name);
bucket* lookup(char const * name);
void create_symbol_table(void);
void free_symbol_table(void);
void free_symbols(void);

/* verbose.c */
void verbose(void);
void log_unused(void);
void log_conflicts(void);
void print_state(int unsigned);
void print_conflicts(int unsigned);
void print_core(int unsigned);
void print_nulls(int unsigned);
void print_actions(int unsigned);
void print_shifts(action const * p);
void print_reductions(action const * p, int defred);
void print_gotos(int);

/* warshall.c */
void transitive_closure(unsigned *, int);
void reflexive_transitive_closure(unsigned *, int);
