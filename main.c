#include "defs.h"
#include <signal.h>
#include <unistd.h>

char dflag;
char lflag;
char rflag;
char tflag;
char vflag;
int Eflag = 0;

char *file_prefix = "y";
char *myname = "yacc";
#ifdef __MSDOS__
#define DIR_CHAR '\\'
#define DEFAULT_TMPDIR "."
#else  /* Unix */
#define DIR_CHAR '/'
#define DEFAULT_TMPDIR "/tmp"
#endif
static char const temp_form[] = "yacc_t_XXXXXX";

int lineno;
int outline;

char *action_file_name;
char *code_file_name;
char *defines_file_name;
char *input_file_name = "";
char *output_file_name;
char *text_file_name;
char *union_file_name;
char *verbose_file_name;

FILE* action_file = NULL;	/*  a temp file, used to save actions associated    */
			/*  with rules until the parser is written	    */
FILE *code_file;	/*  y.code.c (used when the -r option is specified) */
FILE *defines_file;	/*  y.tab.h					    */
FILE *input_file;	/*  the input file				    */
FILE *output_file;	/*  y.tab.c					    */
FILE* text_file = NULL;	/*  a temp file, used to save text until all	    */
			/*  symbols have been defined			    */
FILE* union_file = NULL;	/*  a temp file, used to save the union		    */
			/*  definition until all symbol have been	    */
			/*  defined					    */
FILE *verbose_file;	/*  y.output					    */

int nitems;
int nrules;
int nsyms;
int ntokens;
int nvars;

int   start_symbol;
char  **symbol_name;
Yshort *symbol_value;
Yshort *symbol_prec;
char  *symbol_assoc;

Yshort *ritem;
Yshort *rlhs;
Yshort *rrhs;
Yshort *rprec;
char  *rassoc;
Yshort **derives;
char *nullable;


void done(int k)
{
    if (action_file) { fclose(action_file); unlink(action_file_name); }
    if (text_file) { fclose(text_file); unlink(text_file_name); }
    if (union_file) { fclose(union_file); unlink(union_file_name); }
    exit(k);
}


#ifdef BTYACC_USE_SIGNAL_HANDLING
sig_atomic_t BtYacc_is_interrupted = 0;

void BtYacc_stop_test(void)
{
    if (BtYacc_is_interrupted)
    {
       (void) fprintf(stderr, "BtYacc execution was interrupted.\n");
       done(BtYacc_is_interrupted);
    }
}


static void on_interruption(int x)
{
   BtYacc_is_interrupted = 255;
   (void) x;
}


static void set_signal_handler(int number)
{
    void (*function)(int) = signal(number, SIG_IGN);

    if (function == SIG_ERR)
    {
       perror("btyacc: set_signal_handler() failed to query a setting");
       exit(254);
    }
    else
       if (  (function != SIG_IGN)
          && (signal(number, &on_interruption) == SIG_ERR) )
       {
          perror("btyacc: set_signal_handler() failed to register a new function");
          exit(253);
       }
}


static void signal_setup(void)
{
#ifdef SIGINT
    set_signal_handler(SIGINT);
#endif
#ifdef SIGTERM
    set_signal_handler(SIGTERM);
#endif
#ifdef SIGHUP
    set_signal_handler(SIGHUP);
#endif
}
#endif


static void usage(void)
{
    fprintf(stderr, "usage: %s [-dlrtv] [-b file_prefix] [-S skeleton file] "
		    "filename\n", myname);
    exit(1);
}


static void getargs(int argc, char **argv)
{
    register int i;
    register char *s;

    if (argc > 0) myname = argv[0];
    for (i = 1; i < argc; ++i)
    {
	s = argv[i];
	if (*s != '-') break;
	switch (*++s)
	{
	case '\0':
	    input_file = stdin;
	    if (i + 1 < argc) usage();
	    return;

	case '-':
	    ++i;
	    goto no_more_options;

	case 'b':
	    if (*++s)
		 file_prefix = s;
	    else if (++i < argc)
		file_prefix = argv[i];
	    else
		usage();
	    continue;

	case 'd':
	    dflag = 1;
	    break;

	case 'D':
	    /* Find the preprocessor variable */
	    { char **ps;
	      char *var_name = s + 1;
	      extern char *defd_vars[];
	      for(ps=&defd_vars[0]; *ps; ps++) {
		if(strcmp(*ps,var_name)==0) {
		  error(lineno, 0, 0, "Preprocessor variable %s already defined", var_name);
		}
	      }
	      *ps = MALLOC(strlen(var_name)+1);
	      strcpy(*ps, var_name);
	      *++ps = NULL;
	    }
	    continue;
	      
	case 'E':
	    Eflag = 1;
	    break;

	case 'l':
	    lflag = 1;
	    break;

	case 'r':
	    rflag = 1;
	    break;

	case 't':
	    tflag = 1;
	    break;

	case 'v':
	    vflag = 1;
	    break;

	case 'S':
	    if (*++s)
		read_skel(s);
	    else if (++i < argc)
		read_skel(argv[i]);
	    else
		usage();
	    continue;

	default:
	    usage();
	}

	for (;;)
	{
	    switch (*++s)
	    {
	    case '\0':
		goto end_of_option;

	    case 'd':
		dflag = 1;
		break;

	    case 'l':
		lflag = 1;
		break;

	    case 'r':
		rflag = 1;
		break;

	    case 't':
		tflag = 1;
		break;

	    case 'v':
		vflag = 1;
		break;

	    default:
		usage();
	    }
	}
end_of_option:;
    }

no_more_options:;
    if (i + 1 != argc) usage();
    input_file_name = argv[i];

    if (!file_prefix) {
      if (input_file_name) {
	file_prefix = strdup(input_file_name);
	if ((s = strrchr(file_prefix, '.')))
	  *s = 0; 
      } else {
	file_prefix = "y"; 
      }
    }
}

char *allocate(unsigned n)
{
    register char *p;

    p = NULL;
    if (n)
    {
        /* VM: add a few bytes here, cause 
         * Linux calloc does not like sizes like 32768 */
	p = CALLOC(1, n+10);
	if (!p) no_space();
    }
    return (p);
}

static FILE* create_temporary_file(char* template)
{
    int descriptor = mkstemp(template);

    if (mkstemp(template) == -1)
    {
       perror("btyacc: Cannot create temporary file");
       (void) fprintf(stderr, "name template: %s\n", template);
       done(EXIT_FAILURE);
    }
    else
    {
       FILE* pointer = fdopen(descriptor, "w");

       if (pointer)
       {
          return pointer;
       }
       else
       {
          perror("btyacc: A stream can not be associated with an existing file descriptor.");
          done(EXIT_FAILURE);
       }
    }
}

static char const TEMPORARY_DIR_ENV_VAR[] = "TMPDIR";

static void create_union_file(void)
{
    size_t i, len;
    char* tmpdir = getenv(TEMPORARY_DIR_ENV_VAR);

    if (tmpdir == 0)
       tmpdir = DEFAULT_TMPDIR;

    len = strlen(tmpdir);
    i = len + (sizeof(temp_form) - 1);

    if (len && tmpdir[len - 1] != DIR_CHAR)
	++i;

    union_file_name = MALLOC(i);

    if (union_file_name == 0)
       no_space();

    strcpy(union_file_name, tmpdir);

    if (len && tmpdir[len - 1] != DIR_CHAR)
    {
	union_file_name[len] = DIR_CHAR;
	++len;
    }

    strcpy(union_file_name + len, temp_form);
    union_file_name[len + 5] = 'u';
    union_file = create_temporary_file(union_file_name);
}

static void create_files(void)
{
    size_t i, len;
    char* tmpdir = getenv(TEMPORARY_DIR_ENV_VAR);

    if (tmpdir == 0)
       tmpdir = DEFAULT_TMPDIR;

    len = strlen(tmpdir);
    i = len + (sizeof(temp_form) - 1);

    if (len && tmpdir[len - 1] != DIR_CHAR)
	++i;

    action_file_name = MALLOC(i);

    if (action_file_name == 0)
       no_space();

    text_file_name = MALLOC(i);

    if (text_file_name == 0)
       no_space();

    strcpy(action_file_name, tmpdir);
    strcpy(text_file_name, tmpdir);

    if (len && tmpdir[len - 1] != DIR_CHAR)
    {
	action_file_name[len] = DIR_CHAR;
	text_file_name[len] = DIR_CHAR;
	++len;
    }

    strcpy(action_file_name + len, temp_form);
    strcpy(text_file_name + len, temp_form);

    action_file_name[len + 5] = 'a';
    text_file_name[len + 5] = 't';

    action_file = create_temporary_file(action_file_name);
    text_file = create_temporary_file(text_file_name);

    len = strlen(file_prefix);

    output_file_name = MALLOC(len + 7);
    if (output_file_name == 0)
	no_space();
    strcpy(output_file_name, file_prefix);
    strcpy(output_file_name + len, OUTPUT_SUFFIX);

    if (rflag)
    {
	code_file_name = MALLOC(len + 8);
	if (code_file_name == 0)
	    no_space();
	strcpy(code_file_name, file_prefix);
	strcpy(code_file_name + len, CODE_SUFFIX);
    }
    else
	code_file_name = output_file_name;

    if (dflag)
    {
	defines_file_name = MALLOC(len + 7);
	if (defines_file_name == 0)
	    no_space();
	strcpy(defines_file_name, file_prefix);
	strcpy(defines_file_name + len, DEFINES_SUFFIX);
    }

    if (vflag)
    {
	verbose_file_name = MALLOC(len + 8);
	if (verbose_file_name == 0)
	    no_space();
	strcpy(verbose_file_name, file_prefix);
	strcpy(verbose_file_name + len, VERBOSE_SUFFIX);
    }
}


static void open_files(void)
{
    create_files();

    if (input_file == 0)
    {
	input_file = fopen(input_file_name, "r");
	if (input_file == 0)
	    open_error(input_file_name);
    }

    if (vflag)
    {
	verbose_file = fopen(verbose_file_name, "w");
	if (verbose_file == 0)
	    open_error(verbose_file_name);
    }

    if (dflag)
    {
	defines_file = fopen(defines_file_name, "w");
	if (defines_file == 0)
	    open_error(defines_file_name);

	create_union_file();
    }

    output_file = fopen(output_file_name, "w");
    if (output_file == 0)
	open_error(output_file_name);

    if (rflag)
    {
	code_file = fopen(code_file_name, "w");
	if (code_file == 0)
	    open_error(code_file_name);
    }
    else
	code_file = output_file;
}


int main(int argc, char **argv)
{
#ifdef BTYACC_USE_SIGNAL_HANDLING
    signal_setup();
#endif

    getargs(argc, argv);
    BTYACC_INTERRUPTION_CHECK
    open_files();
    BTYACC_INTERRUPTION_CHECK
    reader();
    BTYACC_INTERRUPTION_CHECK
    lr0();
    BTYACC_INTERRUPTION_CHECK
    lalr();
    BTYACC_INTERRUPTION_CHECK
    make_parser();
    BTYACC_INTERRUPTION_CHECK
    verbose();
    BTYACC_INTERRUPTION_CHECK
    output();
    BTYACC_INTERRUPTION_CHECK
    done(0);
    return 0;
}
