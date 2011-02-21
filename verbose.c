#ifdef BTYACC_BUILD_USE_CONFIGURATION_HEADER
#include "build.h"   // System settings from the build configuration
#endif

#include "defs.h"
#include <stdarg.h>


static Yshort *null_rules;

static void BtYacc_puts(char const * text)
{
    if (fputs(text, verbose_file) == EOF)
    {
       perror("BtYacc_puts");
       abort();
    }
}


static void BtYacc_printf(char const * format, ...)
{
  va_list vl;

  va_start(vl, format);

  if (vfprintf(verbose_file, format, vl) < 0)
  {
     perror("BtYacc: vfprintf");
     va_end(vl);
     abort();
  }

  va_end(vl);
}


static void BtYacc_putc(char c)
{
    if (putc(c, verbose_file) == EOF)
    {
       perror("BtYacc_putc");
       abort();
    }
}


void verbose()
{
    register int unsigned i;

    if (!vflag) return;

    null_rules = (Yshort *) MALLOC(nrules*sizeof(Yshort));
    if (null_rules == 0) no_space();

    BtYacc_puts("\f\n");

    for (i = 0; i < nstates; ++i)
	print_state(i);
    FREE(null_rules);

    if (nunused)
	log_unused();
    if (SRtotal || RRtotal)
	log_conflicts();

    BtYacc_printf("\n\n%u terminals, %u nonterminals\n%u grammar rules, %u states\n",
                  ntokens, nvars, nrules - 2, nstates);
}


void log_unused()
{
    register size_t i;
    register Yshort *p;

    BtYacc_puts("\n\nRules never reduced:\n");

    for (i = 3; i < nrules; ++i)
    {
	if (!rules_used[i])
	{
	    BtYacc_printf("\t%s :", symbol_name[rlhs[i]]);

	    for (p = ritem + rrhs[i]; *p >= 0; ++p)
		BtYacc_printf(" %s", symbol_name[*p]);

	    BtYacc_printf("  (%d)\n", i - 2);
	}
    }
}


void log_conflicts()
{
    register size_t i;

    BtYacc_puts("\n\n");

    for (i = 0; i < nstates; ++i)
    {
	if (SRconflicts[i] || RRconflicts[i])
	{
	    BtYacc_printf("State %d contains ", i);

	    if (SRconflicts[i] == 1)
		BtYacc_puts("1 shift/reduce conflict");
	    else if (SRconflicts[i] > 1)
		BtYacc_printf("%d shift/reduce conflicts", SRconflicts[i]);

	    if (SRconflicts[i] && RRconflicts[i])
		BtYacc_puts(", ");

	    if (RRconflicts[i] == 1)
		BtYacc_puts("1 reduce/reduce conflict");
	    else if (RRconflicts[i] > 1)
		BtYacc_printf("%d reduce/reduce conflicts", RRconflicts[i]);

	    BtYacc_puts(".\n");
	}
    }
}


void print_state(int unsigned state)
{
    if (state)
	BtYacc_puts("\n\n");

    if (SRconflicts[state] || RRconflicts[state])
	print_conflicts(state);

    BtYacc_printf("state %d\n", state);
    print_core(state);
    print_nulls(state);
    print_actions(state);
}


void print_conflicts(int unsigned state)
{
    register int symbol, act, number;
    register action *p;

    symbol = act = number = -1;
    for (p = parser[state]; p; p = p->next)
    {
	if (p->suppressed == 2)
	    continue;

	if (p->symbol != symbol)
	{
	    symbol = p->symbol;
	    number = p->number;
	    if (p->action_code == SHIFT)
		act = SHIFT;
	    else
		act = REDUCE;
	}
	else if (p->suppressed == 1)
	{
	    if (state == final_state && symbol == 0)
	    {
		BtYacc_printf("%d: shift/reduce conflict (accept, reduce %d) on $end\n",
			state, p->number - 2);
	    }
	    else
	    {
		if (act == SHIFT)
		{
		    BtYacc_printf("%d: shift/reduce conflict (shift %d, reduce %d) on %s\n",
			    state, number, p->number - 2, symbol_name[symbol]);
		}
		else
		{
		    BtYacc_printf("%d: reduce/reduce conflict (reduce %d, reduce %d) on %s\n",
			    state, number - 2, p->number - 2, symbol_name[symbol]);
		}
	    }
	}
    }
}


void print_core(int unsigned state)
{
    register size_t i;
    register int k;
    register size_t rule;
    register core *statep;
    register Yshort *sp;
    register Yshort *sp1;

    statep = state_table[state];
    k = statep->nitems;

    for (i = 0; i < k; ++i)
    {
	sp1 = sp = ritem + statep->items[i];

	while (*sp >= 0) ++sp;
	rule = -(*sp);
	BtYacc_printf("\t%s : ", symbol_name[rlhs[rule]]);

        for (sp = ritem + rrhs[rule]; sp < sp1; ++sp)
	    BtYacc_printf("%s ", symbol_name[*sp]);

	BtYacc_putc('.');

	while (*sp >= 0)
	{
	    BtYacc_printf(" %s", symbol_name[*sp]);
	    ++sp;
	}

	BtYacc_printf("  (%d)\n", -2 - *sp);
    }
}


void print_nulls(int unsigned state)
{
    register action *p;
    register size_t i, j, k, nnulls;

    nnulls = 0;
    for (p = parser[state]; p; p = p->next)
    {
	if (p->action_code == REDUCE &&
		(p->suppressed == 0 || p->suppressed == 1))
	{
	    i = p->number;
	    if (rrhs[i] + 1 == rrhs[i+1])
	    {
		for (j = 0; j < nnulls && i > null_rules[j]; ++j)
		    continue;

		if (j == nnulls)
		{
		    ++nnulls;
		    null_rules[j] = i;
		}
		else if (i != null_rules[j])
		{
		    ++nnulls;
		    for (k = nnulls - 1; k > j; --k)
			null_rules[k] = null_rules[k-1];
		    null_rules[j] = i;
		}
	    }
	}
    }

    for (i = 0; i < nnulls; ++i)
    {
	j = null_rules[i];
	BtYacc_printf("\t%s : .  (%d)\n", symbol_name[rlhs[j]],
		j - 2);
    }

    BtYacc_puts("\n");
}


void print_actions(int unsigned stateno)
{
    register action *p;
    register shifts *sp;
    register int as;

    if (stateno == final_state)
	BtYacc_puts("\t$end  accept\n");

    p = parser[stateno];
    if (p)
    {
	print_shifts(p);
	print_reductions(p, defred[stateno]);
    }

    sp = shift_table[stateno];
    if (sp && sp->nshifts > 0)
    {
	as = accessing_symbol[sp->shift[sp->nshifts - 1]];
	if (ISVAR(as))
	    print_gotos(stateno);
    }
}


void print_shifts(action const * p)
{
    register int count;
    register action const * q;

    count = 0;
    for (q = p; q; q = q->next)
    {
	if (q->suppressed < 2 && q->action_code == SHIFT)
	    ++count;
    }

    if (count > 0)
    {
	for (; p; p = p->next)
	{
	    if (p->action_code == SHIFT && p->suppressed == 0)
		BtYacc_printf("\t%s  shift %d\n",
			    symbol_name[p->symbol], p->number);
	}
    }
}


void print_reductions(action const * p, int defred)
{
    register int k, anyreds;
    register action const * q;

    anyreds = 0;
    for (q = p; q ; q = q->next)
    {
	if (q->action_code == REDUCE && q->suppressed < 2)
	{
	    anyreds = 1;
	    break;
	}
    }

    if (anyreds == 0)
	BtYacc_puts("\t.  error\n");
    else
    {
	for (; p; p = p->next)
	{
	    if (p->action_code == REDUCE && p->number != defred)
	    {
		k = p->number - 2;
		if (p->suppressed == 0)
		    BtYacc_printf("\t%s  reduce %d\n",
			    symbol_name[p->symbol], k);
	    }
	}

        if (defred > 0)
	    BtYacc_printf("\t.  reduce %d\n", defred - 2);
    }
}


void print_gotos(int stateno)
{
    register size_t i, k;
    register int as;
    register Yshort *to_state;
    register shifts *sp;

    BtYacc_putc('\n');
    sp = shift_table[stateno];
    to_state = sp->shift;
    for (i = 0; i < sp->nshifts; ++i)
    {
	k = to_state[i];
	as = accessing_symbol[k];
	if (ISVAR(as))
	    BtYacc_printf("\t%s  goto %d\n", symbol_name[as], k);
    }
}

