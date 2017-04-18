/*
 * Copyright (c) 2017, Michel Martens <mail at soveran dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "linenoise.h"
#include "sds.h"

#define HINT_COLOR 33
#define OUTPUT_FMT "\x1b[33m= %g\x1b[0m\n"

static double stack[0xFF];
static double hole = 0;

static int top = 0;

static sds result;

static void push(double value) {
	stack[top++] = value;
}

static double pop() {
	if (top == 0) {
		return 0;
	}

	return stack[--top];
}

static void eval(const char *input) {
	int i, argc;
	double a, b;
	char *z;

	sds *argv = sdssplitargs(input, &argc);

	top = 0;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "+")) {
			a = pop();
			b = pop();
			push(a + b);
		} else if (!strcmp(argv[i], "-")) {
			a = pop();
			b = pop();
			push(b - a);
		} else if (!strcmp(argv[i], "*")) {
			a = pop();
			b = pop();
			push(b * a);
		} else if (!strcmp(argv[i], "/")) {
			a = pop();
			b = pop();
			push(b / a);
		} else if (!strcmp(argv[i], "//")) {
			a = pop();
			b = pop();
			push(floor(b / a));
		} else if (!strcmp(argv[i], "%")) {
			a = pop();
			b = pop();
			push(b - a * floor(b / a));
		} else if ( (!strcmp(argv[i], "**")) || (!strcmp(argv[i], "^")) ) {
			a = pop();
			b = pop();
			push(pow(b, a));
		} else if (!strcmp(argv[i], "_")) {
			push(hole);
		} else if (!strcasecmp(argv[i], "ceil")) {
			a = pop();
			push(ceil(a));
		} else if (!strcasecmp(argv[i], "floor")) {
			a = pop();
			push(floor(a));
		} else if (!strcasecmp(argv[i], "round")) {
			a = pop();
			push(round(a));
		} else if (!strcasecmp(argv[i], "swap")) {
			a = pop();
			b = pop();
			push(a);
			push(b);
		} else if (!strcasecmp(argv[i], "dup")) {
			a = pop();
			push(a);
			push(a);
		} else if (!strcmp(argv[i], "!")) {
			a = pop();
			if (a >= 1) {
				b = 1;
				for (int i = 1; i <= a; ++i) {
					b *= i;
				}
				push(b);
			} else {
				push(a);
			}
		} else if (!strcasecmp(argv[i], "pi")) {
			push(M_PI);
		} else if (!strcasecmp(argv[i], "e")) {
			push(M_E);
		} else if (!strcasecmp(argv[i], "root") || !strcasecmp(argv[i], "sqrt")) {
			a = pop();
			push(sqrt(a));
		} else if ( !strcasecmp(argv[i], "abs") || !strcmp(argv[i], "|") ) {
			a = pop();
			push(fabs(a));
		} else {
			a = strtod(argv[i], &z);

			if (*z == '\0') {
				push(a);
			}
		}
	}

	sdsclear(result);
	sdsfreesplitres(argv, argc);

	for (i = 0; i < top; i++) {
		result = sdscatprintf(result, " %g", stack[i]);
	}
}

static char *hints(const char *input, int *color, int *bold) {
	*color = HINT_COLOR;
	eval(input);
	return result;
}

int main(int argc, char **argv) {
	char *line;

	result = sdsempty();

	linenoiseSetHintsCallback(hints);

	while((line = linenoise("> ")) != NULL) {
		if (line[0] != '\0') {
			hole = stack[top-1];
			printf(OUTPUT_FMT, hole);
		}

		sdsclear(result);
		free(line);
	}

	sdsfree(result);

	return 0;
}
