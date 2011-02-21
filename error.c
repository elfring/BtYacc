/* 
 * routines for printing error messages  
 */

#include "defs.h"
#include <stdarg.h>

extern FILE *inc_file;
extern char inc_file_name[];

/*
 * VM: print error message with file coordinates.
 * Do it in style acceptable to emacs.
 */
static void FileError(char const * fmt, ...) {
  va_list args;

  if (fprintf(stderr, "%s:%u: ", (inc_file ? inc_file_name : input_file_name), lineno) < 0)
  {
     perror("FileError: fprintf");
     abort();
  }

  va_start(args, fmt);

  if (vfprintf(stderr, fmt, args) < 0)
  {
     perror("FileError: vfprintf");
     va_end(args);
     abort();
  }

  va_end(args);

  if (fprintf(stderr, "\n") < 0)
  {
     perror("FileError: fprintf");
     abort();
  }
}

void fatal(char const * msg)
{
    if (fprintf(stderr, "fatal - %s\n", msg) < 0)
    {
       perror("fatal: fprintf");
       abort();
    }

    done(2);
}


void no_space()
{
    if (fputs("fatal - out of space\n", stderr) == EOF)
    {
       perror("no_space: fputs");
       abort();
    }

    done(2);
}


void open_error(char const * filename)
{
    if (fprintf(stderr, "fatal - cannot open \"%s\"\n", filename) < 0)
    {
       perror("open_error: fprintf");
       abort();
    }

    done(2);
}


void unexpected_EOF()
{
  FileError("unexpected end-of-file");
  done(1);
}


static void BtYacc_putc(char c)
{
    if (putc(c, stderr) == EOF)
    {
       perror("BtYacc_putc");
       abort();
    }
}


void print_pos(char const * st_line, char const * st_cptr)
{
    register char const * s;

    if (st_line == 0) return;
    for (s = st_line; *s != '\n'; ++s)
    {
	if (isprint(*s) || *s == '\t')
	   BtYacc_putc(*s);
	else
	   BtYacc_putc('?');
    }

    BtYacc_putc('\n');

    for (s = st_line; s < st_cptr; ++s)
    {
	if (*s == '\t')
	   BtYacc_putc('\t');
	else
	   BtYacc_putc(' ');
    }

    BtYacc_putc('^');
    BtYacc_putc('\n');
}

int unsigned read_errs = 0;

void error(int unsigned lineno, char const * line, char const * cptr, char const * msg, ...)
{
  char sbuf[512];
  va_list args;

  if (fprintf(stderr, "lineno: %u\n", lineno) < 0)
  {
     perror("error: fprintf");
     abort();
  }

  if (line && (fprintf(stderr, "line: %s\n", line) < 0) )
  {
     perror("error: fprintf");
     abort();
  }

  if (cptr && (fprintf(stderr, "cptr: %s\n", cptr) < 0) )
  {
     perror("error: fprintf");
     abort();
  }

  va_start(args, msg);

  if (vsprintf(sbuf, msg, args) < 0)
  {
     perror("error: vsprintf");
     va_end(args);
     abort();
  }

  va_end(args);
  FileError("%s", sbuf);
  read_errs++;
}

void syntax_error(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "syntax error"); 
  exit(1);
}

void unterminated_comment(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unmatched /*"); 
  exit(1);
}

void unterminated_string(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unterminated string"); 
  exit(1);
}

void unterminated_text(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unmatched %%{"); 
  exit(1);
}

void unterminated_union(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unterminated %%union"); 
  exit(1);
}

void over_unionized(char const * cptr) {
  error(lineno, line, cptr, "too many %%union declarations"); 
  exit(1);
}

void illegal_tag(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "illegal tag"); 
}

void illegal_character(char const * cptr) {
  error(lineno, line, cptr, "illegal character"); 
}

void used_reserved(char const * s) {
  error(lineno, 0, 0, "illegal use of reserved symbol %s", s); 
}

void tokenized_start(char const * s) {
  error(lineno, 0, 0, "the start symbol %s cannot be declared to be a token", s); 
}

void retyped_warning(char const * s) {
  FileError("the type of %s has been redeclared", s);
}

void reprec_warning(char const * s) {
  FileError("the precedence of %s has been redeclared", s);
}

void revalued_warning(char const * s) {
  FileError("the value of %s has been redeclared", s);
}

void terminal_start(char const * s) {
  error(lineno, 0, 0, "the start symbol %s is a token", s); 
}

void restarted_warning() {
  FileError("the start symbol has been redeclared");
}

void no_grammar() { 
  error(lineno, 0, 0, "no grammar has been specified"); 
}

void terminal_lhs(int lineno) { 
  error(lineno, 0, 0, "a token appears on the lhs of a production"); 
}

void prec_redeclared() { 
  error(lineno, 0, 0, "conflicting %%prec specifiers"); 
}

void unterminated_action(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unterminated action"); 
}

void unterminated_arglist(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "unterminated argument list"); 
}

void bad_formals() { 
  error(lineno, 0, 0, "bad formal argument list"); 
}

void dollar_warning(int unsigned a_lineno, int i) {
  int unsigned slineno = lineno;
  lineno = a_lineno;
  FileError("$%d references beyond the end of the current rule", i);
  lineno = slineno;
}

void dollar_error(int unsigned lineno, char const * line, char const * cptr) {
  error(lineno, line, cptr, "illegal $-name"); 
}

void untyped_lhs() { 
  error(lineno, 0, 0, "$$ is untyped"); 
}

void untyped_rhs(int i, char const * s) {
  error(lineno, 0, 0, "$%d (%s) is untyped", i, s); 
}

void unknown_rhs(int i) { 
  error(lineno, 0, 0, "$%d is untyped (out of range)", i); 
}

void default_action_warning() {
  FileError("the default action assigns an undefined value to $$");
}

void undefined_goal(char const * s) {
  error(lineno, 0, 0, "the start symbol %s is undefined", s); 
}

void undefined_symbol_warning(char const * s) {
  if (fprintf(stderr, "warning - the symbol %s is undefined\n", s) < 0)
  {
     perror("undefined_symbol_warning: fprintf");
     abort();
  }
}
