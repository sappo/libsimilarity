/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2013-2015 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details.
 */

#include "config.h"
#include "common.h"
#include "harry.h"
#include "hconfig.h"
#include "util.h"
#include "input.h"
#include "measures.h"
#include "output.h"
#include "vcache.h"
#include "hmatrix.h"

/* Global variables */
int verbose = 0;
int log_line = 0;
config_t cfg;

static int print_conf = 0;
static char *measure = NULL;
static int benchmark = 0;

/* Option string */
#define OPTSTRING "i:o:p:zm:g:d:n:a:Gx:y:s:c:vlqMCDVh"


/**
 * Array of options of getopt_long()
 */
static struct option longopts[] = {
    {"input_format", 1, NULL, 'i'},
    {"decode_str", 0, NULL, 1000},
    {"reverse_str", 0, NULL, 1001},
    {"stoptoken_file", 1, NULL, 1002},
    {"soundex", 0, NULL, 1003},
    {"benchmark", 1, NULL, 1004},
    {"output_format", 1, NULL, 'o'},
    {"precision", 1, NULL, 'p'},
    {"compress", 0, NULL, 'z'},
    {"save_indices", 0, NULL, 1005},
    {"save_labels", 0, NULL, 1006},
    {"save_sources", 0, NULL, 1007},
    {"measure", 1, NULL, 'm'},
    {"granularity", 1, NULL, 'g'},
    {"token_delim", 1, NULL, 'd'},
    {"num_threads", 1, NULL, 'n'},
    {"cache_size", 1, NULL, 'a'},
    {"global_cache", 0, NULL, 'G'},
    {"col_range", 1, NULL, 'x'},
    {"row_range", 1, NULL, 'y'},
    {"split", 1, NULL, 's'},
    {"config_file", 1, NULL, 'c'},
    {"verbose", 0, NULL, 'v'},
    {"log_line", 0, NULL, 'l'},
    {"quiet", 0, NULL, 'q'},
    {"print_measures", 0, NULL, 'M'},
    {"print_config", 0, NULL, 'C'},
    {"print_defaults", 0, NULL, 'D'},
    {"version", 0, NULL, 'V'},
    {"help", 0, NULL, 'h'},
    {NULL, 0, NULL, 0}
};


/**
 * Prints version and copyright information to a file stream
 * @param f File pointer
 * @param p Prefix character
 * @param m Message
 * @return number of written characters
 */
int harry_version(FILE *f, char *p, char *m)
{
    return fprintf(f, "%sHarry %s - %s\n", p, PACKAGE_VERSION, m);
}

/**
 * Prints version and copyright information to a file stream
 * @param z File pointer
 * @param p Prefix character
 * @param m Message
 * @return number of written characters
 */
int harry_zversion(gzFile z, char *p, char *m)
{
    return gzprintf(z, "%sHarry %s - %s\n", p, PACKAGE_VERSION, m);
}

/**
 * Print configuration
 * @param msg Text to add to output
 */
static void print_config(char *msg)
{
    harry_version(stderr, "# ", msg);
    config_fprint(stderr, &cfg);
}

/**
 * Print usage of command line tool
 */
static void print_usage(void)
{
    printf("Usage: harry [options] <input> [<input>] <output>\n"
           "\nI/O options:\n"
           "  -i,  --input_format <format>   Set input format for strings.\n"
           "       --decode_str              Enable URI-decoding of strings.\n"
           "       --reverse_str             Reverse (flip) all strings.\n"
           "       --stoptoken_file <file>   Provide a file with stop tokens.\n"
           "       --soundex                 Enable soundex encoding of tokens.\n"
           "       --benchmark <num>         Perform benchmark for given seconds.\n"
           "  -o,  --output_format <format>  Set output format for matrix.\n"
           "  -p,  --precision <num>         Set precision of output.\n"
           "  -z,  --compress                Enable zlib compression of output.\n"
           "       --save_indices            Save indices of strings.\n"
           "       --save_labels             Save labels of strings.\n"
           "       --save_sources            Save sources of strings.\n"
           "\nMeasure options:\n"
           "  -m,  --measure <name>          Set similarity measure.\n"
           "  -g,  --granularity <type>      Set granularity: bytes, bits, tokens.\n"
           "  -d,  --token_delim <chars>     Set delimiters for tokens.\n"
           "  -n,  --num_threads <num>       Set number of threads.\n"
           "  -a,  --cache_size <num>        Set size of cache in megabytes.\n"
           "  -G,  --global_cache            Enable global cache.\n"
           "  -x,  --col_range <start:end>   Set the column range (x) of strings.\n"
           "  -y,  --row_range <start:end>   Set the row range (y) of strings.\n"
           "  -s,  --split <blocks:id>       Split matrix into blocks and compute one.\n"
           "\nGeneric options:\n"
           "  -c,  --config_file <file>      Set configuration file.\n"
           "  -v,  --verbose                 Increase verbosity.\n"
           "  -l,  --log_line                Print a log line every minutes.\n"
           "  -q,  --quiet                   Be quiet during processing.\n"
           "  -M,  --print_measures          Print list of similarity measures.\n"
           "  -C,  --print_config            Print the current configuration.\n"
           "  -D,  --print_defaults          Print the default configuration.\n"
           "  -V,  --version                 Print version and copyright.\n"
           "  -h,  --help                    Print help screen.\n"
           "\n");

}

/**
 * Print version of Harry tool
 */
static void print_version(void)
{
    fprintf(stderr, "Harry %s - A Tool for Measuring String Similarity\n"
            "Copyright (c) 2013-2015 Konrad Rieck (konrad@mlsec.org)\n",
            PACKAGE_VERSION);
}

/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 * @param in1 Return pointer to input filename
 * @param in2 Return pointer to optional input filename
 * @param out Return pointer to output filename
 */
static void harry_parse_options(int argc, char **argv, char **in1,
                                char **in2, char **out)
{
    int ch;
    const char *str;
    optind = 0;

    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            /* Skip. See harry_load_config(). */
            break;
        case 'i':
            config_set_string(&cfg, "input.input_format", optarg);
            break;
        case 1000:
            config_set_bool(&cfg, "input.decode_str", CONFIG_TRUE);
            break;
        case 1001:
            config_set_bool(&cfg, "input.reverse_str", CONFIG_TRUE);
            break;
        case 1002:
            config_set_string(&cfg, "input.stoptoken_file", optarg);
            break;
        case 1003:
            config_set_bool(&cfg, "input.soundex", CONFIG_TRUE);
            break;
        case 1004:
            benchmark = atoi(optarg);
            break;
        case 1005:
            config_set_bool(&cfg, "output.save_indices", CONFIG_TRUE);
            break;
        case 1006:
            config_set_bool(&cfg, "output.save_labels", CONFIG_TRUE);
            break;
        case 1007:
            config_set_bool(&cfg, "output.save_sources", CONFIG_TRUE);
            break;
        case 'o':
            config_set_string(&cfg, "output.output_format", optarg);
            break;
        case 'p':
            config_set_int(&cfg, "output.precision", atoi(optarg));
            break;
        case 'm':
            config_set_string(&cfg, "measures.measure", optarg);
            break;
        case 'd':
            config_set_string(&cfg, "measures.token_delim", optarg);
            break;
        case 'n':
            config_set_int(&cfg, "measures.num_threads", atoi(optarg));
            break;
        case 'a':
            config_set_int(&cfg, "measures.cache_size", atoi(optarg));
            break;
        case 'G':
            config_set_bool(&cfg, "measures.global_cache", CONFIG_TRUE);
            break;
        case 'g':
            config_set_string(&cfg, "measures.granularity", optarg);
            break;
        case 'z':
            config_set_bool(&cfg, "output.compress", CONFIG_TRUE);
            break;
        case 'x':
            config_set_string(&cfg, "measures.col_range", optarg);
            break;
        case 'y':
            config_set_string(&cfg, "measures.row_range", optarg);
            break;
        case 's':
            config_set_string(&cfg, "measures.split", optarg);
            break;
        case 'q':
            verbose = 0;
            log_line = 0;
            break;
        case 'l':
            log_line = 1;
            break;
        case 'v':
            verbose++;
            break;
        case 'M':
            measure_fprint(stderr);
            exit(EXIT_SUCCESS);
            break;
        case 'D':
            print_config("Default configuration");
            exit(EXIT_SUCCESS);
            break;
        case 'C':
            print_conf = 1;
            break;
        case 'V':
            print_version();
            exit(EXIT_SUCCESS);
            break;
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        case '?':
            print_usage();
            exit(EXIT_FAILURE);
            break;
        }
    }

    argc -= optind;
    argv += optind;

    /* Check for input and output arguments */
    if (argc == 2) {
        *in1 = argv[0];
        *in2 = NULL;
        *out = argv[1];
    } else if (argc == 3) {
        *in1 = argv[0];
        *in2 = argv[1];
        *out = argv[2];
    } else {
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* Check for stdin and stdout "filenames" */
    if (!strcmp(*in1, "-"))
        config_set_string(&cfg, "input.input_format", "stdin");
    if (!strcmp(*in1, "="))
        config_set_string(&cfg, "input.input_format", "raw");
    if (!strcmp(*out, "-"))
        config_set_string(&cfg, "output.output_format", "stdout");
    if (!strcmp(*out, "="))
        config_set_string(&cfg, "output.output_format", "raw");

    /* Check configuration */
    if (!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }

    /* Check for two input sources */
    config_lookup_string(&cfg, "input.input_format", &str);
    if (*in2 && (!strcasecmp(str, "stdin") || !strcasecmp(str, "raw")))
	fatal("Input mode '%s' does not support two inputs.", str);

    /* We are through with parsing. Print the config if requested */
    if (print_conf) {
        print_config("Current configuration");
        exit(EXIT_SUCCESS);
    }
}


/**
 * Load the configuration of Harry
 * @param argc number of arguments
 * @param argv arguments
 */
static void harry_load_config(int argc, char **argv)
{
    char *cfg_file = NULL;
    int ch, ret;
    const char *str;

    /* Check for config file in command line */
    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            cfg_file = optarg;
            break;
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        case '?':
            print_usage();
            exit(EXIT_FAILURE);
            break;
        default:
            /* empty */
            break;
        }
    }

    /* Init and load configuration */
    config_init(&cfg);

    if (cfg_file != NULL) {
        if (config_read_file(&cfg, cfg_file) != CONFIG_TRUE)
            fatal("Could not read configuration (%s in line %d)",
                  config_error_text(&cfg), config_error_line(&cfg));

        /* Check for new granularity parameter */
        ret = config_lookup_string(&cfg, "measures.granularity", &str);
        if (ret == CONFIG_FALSE)
            fatal("Your configuration is missing the new 'granularity' "
                  "parameter. Please consult the manual page and upgrade "
                  "your configuration.");
    }

    /* Check configuration and set defaults */
    if (!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }

}

/**
 * Init the Harry tool
 * @param argc number of arguments
 * @param argv arguments
 */
static void harry_init()
{
    const char *cfg_str;
    cfg_int nthreads = 0;

    if (verbose > 1)
        config_fprint(stderr, &cfg);

    /* Init value cache */
    vcache_init(&cfg);

    /* Configure module (init as first) */
    config_lookup_string(&cfg, "measures.measure", &cfg_str);
    measure = measure_config(cfg_str);

    config_lookup_int(&cfg, "measures.num_threads", &nthreads);
#ifdef HAVE_OPENMP
    if (nthreads <= 0)
        nthreads = omp_get_num_procs();
    omp_set_num_threads(nthreads);
#else
    warning("Harry has been compiled without OpenMP support.");
#endif

    /* Load stop tokens */
    config_lookup_string(&cfg, "input.stoptoken_file", &cfg_str);
    if (strlen(cfg_str) > 0)
        stoptokens_load(cfg_str);
}

/**
 * Read a set of strings to memory from input
 * @param input Input filename
 * @param input2 Optional input filename
 * @param num Pointer to number of strings
 * @return array of string objects
 */
static hstring_t *harry_read(char *input, char *input2, int *num)
{
    const char *cfg_str;
    int i, read;
    cfg_int chunk;
    hstring_t *strs = NULL;
    char buf[128];

    /* Get chunk size */
    config_lookup_int(&cfg, "input.chunk_size", &chunk);

    /* Open input */
    config_lookup_string(&cfg, "input.input_format", &cfg_str);
    info_msg(1, "Opening input '%0.40s' [%s].", input, cfg_str);
    input_config(cfg_str);
    if (!input_open(input))
        fatal("Could not open input source");

    info_msg(1, "Reading strings from %s", input);
    for (*num = 0, read = chunk; read == chunk; *num += read) {
        /* Allocate memory for strings */
        strs = realloc(strs, (*num + chunk) * sizeof(hstring_t));
        if (!strs)
            fatal("Could not allocate memory for strings");

        /* Read chunk */
        read = input_read(strs + *num, chunk);
    }

    /* Close input */
    input_close();

    /* Second input available */
    if (input2) {
        /* Store length of first input */
        int len1 = *num;
        if (!input_open(input2))
            fatal("Could not open second input source");

        info_msg(1, "Reading strings from %s", input2);
        for (read = chunk; read == chunk; *num += read) {
            /* Allocate memory for strings */
            strs = realloc(strs, (*num + chunk) * sizeof(hstring_t));
            if (!strs)
                fatal("Could not allocate memory for strings");

            /* Read chunk */
            read = input_read(strs + *num, chunk);
        }

        /* Close input */
        input_close();

        /* Overwrite row range and col range */
        snprintf(buf, 128, "%d:%d", 0, len1);
        config_set_string(&cfg, "measures.row_range", buf);
        snprintf(buf, 128, "%d:%d", len1, *num);
        config_set_string(&cfg, "measures.col_range", buf);
    }

    /* Symbolize strings if requested */
    for (i = 0; i < *num; i++)
        strs[i] = hstring_preproc(strs[i]);

    return strs;
}

/**
 * Compare a set of string objects
 * @param strs Array of string objects
 * @param num Number of strings
 * @return Matrix of similarity valurs
 */
void harry_compute(hmatrix_t *mat, hstring_t *strs, int num)
{
    /* Compute matrix */
#ifdef HAVE_OPENMP
    info_msg(1, "Computing similarity measure '%s' with %d threads.",
             measure, omp_get_max_threads());
#else
    info_msg(1, "Computing similarity measure '%s'", measure);
#endif
    hmatrix_compute(mat, strs, measure_compare);
}


/**
 * Benchmark runtime
 * @param strs Array of string objects
 * @param num Number of strings
 * @return Matrix of similarity valurs
 */
void harry_benchmark(hmatrix_t *mat, hstring_t *strs, int num)
{
    /* Compute matrix */
#ifdef HAVE_OPENMP
    info_msg(1, "Benchmarking similarity measure '%s' (%d thrd; %d sec).",
             measure, omp_get_max_threads(), benchmark);
    float cmps = hmatrix_benchmark(mat, strs, measure_compare, benchmark);
    printf("%.0f comparisons; %d seconds; %d threads;\n", cmps, benchmark,
           omp_get_max_threads());
#else
    info_msg(1, "Benchmarking similarity measure '%s' (%d sec).",
             measure, benchmark);
    float cmps = hmatrix_benchmark(mat, strs, measure_compare, benchmark);
    printf("%.0f comparisons; %d seconds;\n", cmps, benchmark);
#endif
}

/**
 * Init and allocate matrix for computation
 * @param strs Array of string objects
 * @param num Number of strings
 * @return Empty matrix
 */
static hmatrix_t *harry_alloc(hstring_t *strs, int num)
{
    char *cfg_str;
    int i;

    hmatrix_t *mat = hmatrix_init(strs, num);

    /* Set ranges */
    config_lookup_string(&cfg, "measures.col_range", (const char **) &cfg_str);
    hmatrix_col_range(mat, cfg_str);
    config_lookup_string(&cfg, "measures.row_range", (const char **) &cfg_str);
    hmatrix_row_range(mat, cfg_str);

    /* Set matrix split */
    config_lookup_string(&cfg, "measures.split", (const char **) &cfg_str);
    hmatrix_split(mat, cfg_str);

    /* Free unused memory */
    for (i = 0; i < num; i++) {
        if (i < mat->col.start && i < mat->row.start)
            hstring_destroy(&strs[i]);
        if (i >= mat->col.end && i >= mat->row.end)
            hstring_destroy(&strs[i]);
    }

    /* Allocate matrix */
    if (!hmatrix_alloc(mat))
        fatal("Could not allocate matrix for similarity measure");

    return mat;
}


/**
 * Write similarity values to an output file
 * @param output Output filename
 * @param mat Matrix of similarity values
 */
static void harry_write(char *output, hmatrix_t *mat)
{
    const char *cfg_str;

    /* Open output */
    config_lookup_string(&cfg, "output.output_format", &cfg_str);
    output_config(cfg_str);
    info_msg(1, "Writing %ld similarity values to '%0.40s' [%s].",
             mat->size, output, cfg_str);
    if (!output_open(output))
        fatal("Could not open output destination");

    output_write(mat);
    output_close();
}


/**
 * Exit Harry tool.
 */
static void harry_exit(hstring_t *strs, hmatrix_t *mat, int num)
{
    const char *cfg_str;

    /* Free memory */
    input_free(strs, num);
    free(strs);

    /* Destroy matrix */
    hmatrix_destroy(mat);

    config_lookup_string(&cfg, "input.stoptoken_file", &cfg_str);
    if (strlen(cfg_str) > 0)
        stoptokens_destroy();

    /* Destroy value cache */
    vcache_destroy();

    /* Destroy configuration */
    config_destroy(&cfg);
}

/**
 * Main function of Harry tool
 * @param argc Number of arguments
 * @param argv Argument values
 * @return exit code
 */
int main(int argc, char **argv)
{
    hmatrix_t *mat = NULL;
    hstring_t *strs = NULL;
    int num = 0;
    char *input1 = NULL, *input2 = NULL;
    char *output = NULL;

    harry_load_config(argc, argv);
    harry_parse_options(argc, argv, &input1, &input2, &output);

    harry_init();
    strs = harry_read(input1, input2, &num);
    mat = harry_alloc(strs, num);

    if (benchmark) {
        harry_benchmark(mat, strs, num);
    } else {
        harry_compute(mat, strs, num);
        harry_write(output, mat);
    }

    harry_exit(strs, mat, num);
    return EXIT_SUCCESS;
}
