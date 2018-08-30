#include <string.h> /* for strchr() and strncmp() */
#include "ketopt.h"

ketopt_t KETOPT_INIT = { 1, 0, 0, 1, 0, 0 };

static void ketopt_permute(char *argv[], int j, int n) /* move argv[j] over n elements to the left */
{
	int k;
	char *p = argv[j];
	for (k = 0; k < n; ++k)
		argv[j - k] = argv[j - k - 1];
	argv[j - k] = p;
}

/* WARNING: O(mn) algorithm; m=#main arguments before options; n=#options after main arguments; no trial fix */
int ketopt(ketopt_t *s, int argc, char *argv[], int permute, const char *ostr, const ko_longopt_t *longopts)
{
	int opt = -1, i0, j;
	if (permute) {
		while (s->i < argc && (argv[s->i][0] != '-' || argv[s->i][1] == '\0'))
			++s->i, ++s->n_args;
	}
	s->arg = 0, i0 = s->i;
	if (s->i >= argc || argv[s->i][0] != '-' || argv[s->i][1] == '\0') {
		s->ind = s->i - s->n_args;
		return -1;
	}
	if (argv[s->i][0] == '-' && argv[s->i][1] == '-') { /* "--" or a long option */
		if (argv[s->i][2] == '\0') { /* a bare "--" */
			ketopt_permute(argv, s->i, s->n_args);
			++s->i, s->ind = s->i - s->n_args;
			return -1;
		}
		s->opt = 0, opt = '?', s->pos = -1;
		if (longopts) { /* parse long options */
			int k, n_matches = 0;
			const ko_longopt_t *o = 0;
			for (j = 2; argv[s->i][j] != '\0' && argv[s->i][j] != '='; ++j) {} /* find the end of the option name */
			for (k = 0; longopts[k].name != 0; ++k)
				if (strncmp(&argv[s->i][2], longopts[k].name, j - 2) == 0)
					++n_matches, o = &longopts[k];
			if (n_matches == 1) {
				s->opt = opt = o->val;
				if (argv[s->i][j] == '=') s->arg = &argv[s->i][j + 1];
				if (o->has_arg == 1 && argv[s->i][j] == '\0') {
					if (s->i < argc - 1) s->arg = argv[++s->i];
					else opt = ':'; /* missing option argument */
				}
			}
		}
	} else { /* a short option */
		char *p;
		if (s->pos == 0) s->pos = 1;
		opt = s->opt = argv[s->i][s->pos++];
		p = strchr(ostr, opt);
		if (p == 0) {
			opt = '?'; /* unknown option */
		} else if (p[1] == ':') {
			if (argv[s->i][s->pos] == 0) {
				if (s->i < argc - 1) s->arg = argv[++s->i];
				else opt = ':'; /* missing option argument */
			} else s->arg = &argv[s->i][s->pos];
			s->pos = -1;
		}
	}
	if (s->pos < 0 || argv[s->i][s->pos] == 0) {
		++s->i, s->pos = 0;
		if (s->n_args > 0) /* permute */
			for (j = i0; j < s->i; ++j)
				ketopt_permute(argv, j, s->n_args);
	}
	s->ind = s->i - s->n_args;
	return opt;
}