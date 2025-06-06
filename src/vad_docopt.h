#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#endif


typedef struct {
    /* options without arguments */
    int help;
    int verbose;
    int version;
    /* options with arguments */
    char *alpha1;
    char *alpha2;
    char *input_wav;
    char *output_vad;
    char *output_wav;
    /* special */
    const char *usage_pattern;
    const char *help_message;
} DocoptArgs;

const char help_message[] =
"VAD - Voice Activity Detector\n"
"\n"
"Usage:\n"
"   vad [options] -i <input-wav> -o <output-vad> [-w <output-wav>]\n"
"   vad (-h | --help)\n"
"   vad --version\n"
"\n"
"Options:\n"
"   -i FILE, --input-wav=FILE     WAVE file for voice activity detection\n"
"   -o FILE, --output-vad=FILE    Label file with the result of VAD\n"
"   -w FILE, --output-wav=FILE    WAVE file with silences cleared\n"
"   -1 FLOAT, --alpha1=FLOAT      Increase the threshold 1 [default: 5.1]\n"
"   -2 FLOAT, --alpha2=FLOAT      Increase the threshold 2 [default: 6.6]\n"
"   -v, --verbose  Show debug information\n"
"   -h, --help     Show this screen\n"
"   --version      Show the version of the project\n"
"";

const char usage_pattern[] =
"Usage:\n"
"   vad [options] -i <input-wav> -o <output-vad> [-w <output-wav>]\n"
"   vad (-h | --help)\n"
"   vad --version";

typedef struct {
    const char *name;
    bool value;
} Command;

typedef struct {
    const char *name;
    char *value;
    char **array;
} Argument;

typedef struct {
    const char *oshort;
    const char *olong;
    bool argcount;
    bool value;
    char *argument;
} Option;

typedef struct {
    int n_commands;
    int n_arguments;
    int n_options;
    Command *commands;
    Argument *arguments;
    Option *options;
} Elements;


/*
 * Tokens object
 */

typedef struct Tokens {
    int argc;
    char **argv;
    int i;
    char *current;
} Tokens;

Tokens tokens_new(int argc, char **argv) {
    Tokens ts = {argc, argv, 0, argv[0]};
    return ts;
}

Tokens* tokens_move(Tokens *ts) {
    if (ts->i < ts->argc) {
        ts->current = ts->argv[++ts->i];
    }
    if (ts->i == ts->argc) {
        ts->current = NULL;
    }
    return ts;
}


/*
 * ARGV parsing functions
 */

int parse_doubledash(Tokens *ts, Elements *elements) {
    //int n_commands = elements->n_commands;
    //int n_arguments = elements->n_arguments;
    //Command *commands = elements->commands;
    //Argument *arguments = elements->arguments;

    // not implemented yet
    // return parsed + [Argument(None, v) for v in tokens]
    return 0;
}

int parse_long(Tokens *ts, Elements *elements) {
    int i;
    int len_prefix;
    int n_options = elements->n_options;
    char *eq = strchr(ts->current, '=');
    Option *option;
    Option *options = elements->options;

    len_prefix = (eq-(ts->current))/sizeof(char);
    for (i=0; i < n_options; i++) {
        option = &options[i];
        if (!strncmp(ts->current, option->olong, len_prefix))
            break;
    }
    if (i == n_options) {
        // TODO '%s is not a unique prefix
        fprintf(stderr, "%s is not recognized\n", ts->current);
        return 1;
    }
    tokens_move(ts);
    if (option->argcount) {
        if (eq == NULL) {
            if (ts->current == NULL) {
                fprintf(stderr, "%s requires argument\n", option->olong);
                return 1;
            }
            option->argument = ts->current;
            tokens_move(ts);
        } else {
            option->argument = eq + 1;
        }
    } else {
        if (eq != NULL) {
            fprintf(stderr, "%s must not have an argument\n", option->olong);
            return 1;
        }
        option->value = true;
    }
    return 0;
}

int parse_shorts(Tokens *ts, Elements *elements) {
    char *raw;
    int i;
    int n_options = elements->n_options;
    Option *option;
    Option *options = elements->options;

    raw = &ts->current[1];
    tokens_move(ts);
    while (raw[0] != '\0') {
        for (i=0; i < n_options; i++) {
            option = &options[i];
            if (option->oshort != NULL && option->oshort[1] == raw[0])
                break;
        }
        if (i == n_options) {
            // TODO -%s is specified ambiguously %d times
            fprintf(stderr, "-%c is not recognized\n", raw[0]);
            return 1;
        }
        raw++;
        if (!option->argcount) {
            option->value = true;
        } else {
            if (raw[0] == '\0') {
                if (ts->current == NULL) {
                    fprintf(stderr, "%s requires argument\n", option->oshort);
                    return 1;
                }
                raw = ts->current;
                tokens_move(ts);
            }
            option->argument = raw;
            break;
        }
    }
    return 0;
}

int parse_argcmd(Tokens *ts, Elements *elements) {
    int i;
    int n_commands = elements->n_commands;
    //int n_arguments = elements->n_arguments;
    Command *command;
    Command *commands = elements->commands;
    //Argument *arguments = elements->arguments;

    for (i=0; i < n_commands; i++) {
        command = &commands[i];
        if (!strcmp(command->name, ts->current)){
            command->value = true;
            tokens_move(ts);
            return 0;
        }
    }
    // not implemented yet, just skip for now
    // parsed.append(Argument(None, tokens.move()))
    /*fprintf(stderr, "! argument '%s' has been ignored\n", ts->current);
    fprintf(stderr, "  '");
    for (i=0; i<ts->argc ; i++)
        fprintf(stderr, "%s ", ts->argv[i]);
    fprintf(stderr, "'\n");*/
    tokens_move(ts);
    return 0;
}

int parse_args(Tokens *ts, Elements *elements) {
    int ret;

    while (ts->current != NULL) {
        if (strcmp(ts->current, "--") == 0) {
            ret = parse_doubledash(ts, elements);
            if (!ret) break;
        } else if (ts->current[0] == '-' && ts->current[1] == '-') {
            ret = parse_long(ts, elements);
        } else if (ts->current[0] == '-' && ts->current[1] != '\0') {
            ret = parse_shorts(ts, elements);
        } else
            ret = parse_argcmd(ts, elements);
        if (ret) return ret;
    }
    return 0;
}

int elems_to_args(Elements *elements, DocoptArgs *args, bool help,
                  const char *version){
    Command *command;
    Argument *argument;
    Option *option;
    int i;

    // fix gcc-related compiler warnings (unused)
    (void)command;
    (void)argument;

    /* options */
    for (i=0; i < elements->n_options; i++) {
        option = &elements->options[i];
        if (help && option->value && !strcmp(option->olong, "--help")) {
            printf("%s", args->help_message);
            return 1;
        } else if (version && option->value &&
                   !strcmp(option->olong, "--version")) {
            printf("%s\n", version);
            return 1;
        } else if (!strcmp(option->olong, "--help")) {
            args->help = option->value;
        } else if (!strcmp(option->olong, "--verbose")) {
            args->verbose = option->value;
        } else if (!strcmp(option->olong, "--version")) {
            args->version = option->value;
        } else if (!strcmp(option->olong, "--alpha1")) {
            if (option->argument)
                args->alpha1 = option->argument;
        } else if (!strcmp(option->olong, "--alpha2")) {
            if (option->argument)
                args->alpha2 = option->argument;
        } else if (!strcmp(option->olong, "--input-wav")) {
            if (option->argument)
                args->input_wav = option->argument;
        } else if (!strcmp(option->olong, "--output-vad")) {
            if (option->argument)
                args->output_vad = option->argument;
        } else if (!strcmp(option->olong, "--output-wav")) {
            if (option->argument)
                args->output_wav = option->argument;
        }
    }
    /* commands */
    for (i=0; i < elements->n_commands; i++) {
        command = &elements->commands[i];
    }
    /* arguments */
    for (i=0; i < elements->n_arguments; i++) {
        argument = &elements->arguments[i];
    }
    return 0;
}


/*
 * Main docopt function
 */

DocoptArgs docopt(int argc, char *argv[], bool help, const char *version) {
    DocoptArgs args = {
        0, 0, 0, (char*) "5.1", (char*) "6.6", NULL, NULL, NULL,
        usage_pattern, help_message
    };
    Tokens ts;
    Command commands[] = {
    };
    Argument arguments[] = {
    };
    Option options[] = {
        {"-h", "--help", 0, 0, NULL},
        {"-v", "--verbose", 0, 0, NULL},
        {NULL, "--version", 0, 0, NULL},
        {"-1", "--alpha1", 1, 0, NULL},
        {"-2", "--alpha2", 1, 0, NULL},
        {"-i", "--input-wav", 1, 0, NULL},
        {"-o", "--output-vad", 1, 0, NULL},
        {"-w", "--output-wav", 1, 0, NULL}
    };
    Elements elements = {0, 0, 8, commands, arguments, options};

    ts = tokens_new(argc, argv);
    if (parse_args(&ts, &elements))
        exit(EXIT_FAILURE);
    if (elems_to_args(&elements, &args, help, version))
        exit(EXIT_SUCCESS);
    return args;
}