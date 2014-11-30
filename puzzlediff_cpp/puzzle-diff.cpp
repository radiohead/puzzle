extern "C" {
  #include "puzzle_common.h"
  #include "puzzle.h"
  #include <time.h>
}
#include "pgetopt.hpp"
#include "listdir.hpp"
#include "advisor-annotate.h"
#include <iostream>

using namespace std;

typedef struct Opts_ {
    const char *inputFile;
    const char *outputFile;
    const char *dirName;
	vector<string> directory;
    int exit;
	int fix_for_texts;
    double similarity_threshold;
} Opts;

void usage()
{
    puts("Usage: puzzle-diff [-o <output file>] <image to compare against> <directory with images for comparison>");
}

int parse_opts(Opts * const opts, PuzzleContext * context,int argc, char **argv) 
{
    int opt;
    extern char *poptarg;
    extern int poptind;

    opts->exit = 0;
    opts->similarity_threshold = PUZZLE_CVEC_SIMILARITY_THRESHOLD;

    if (argc < 3 || argc > 5) 
    {
        usage();
        return 0;
    }
    // jumping first call argument (program name)
    *argv++;

    if (!strcmp(argv[0], "-o")) 
    {
		//jump to file name
    	*argv ++;
    	opts->outputFile = *argv++;
    }

    opts->inputFile = *argv++;
    opts->dirName = *argv;
    // filling the vector inside opts struct with found files in directory
	listDir(opts->dirName, opts->directory);
    
    return 1;
}

int main(int argc, char *argv[])
{
    Opts opts;
    PuzzleContext context;
    PuzzleCvec cvec1, cvec2;
    double d;
	double start, end;
    
    puzzle_init_context(&context);
	opts.fix_for_texts = 1;

    if(!parse_opts(&opts, &context, argc, argv))
        exit(EXIT_FAILURE);

	start = clock();

	puzzle_init_cvec(&context, &cvec1);
    
	if (puzzle_fill_cvec_from_file(&context, &cvec1, opts.inputFile)) {
		fprintf(stderr, "Unable to read [%s]\n", opts.inputFile);
		return 1;
	}

	ANNOTATE_SITE_BEGIN(walkDir);
	for (int i = 0; i < opts.directory.size(); i++)
	{
		ANNOTATE_ITERATION_TASK(comparePictures);
		puzzle_init_cvec(&context, &cvec2);
		if (puzzle_fill_cvec_from_file(&context, &cvec2, opts.directory[i].c_str()))
			cerr << "Unable to read " << opts.directory[i] << endl;
		else
			cout << puzzle_vector_normalized_distance(&context, &cvec1, &cvec2,opts.fix_for_texts) << endl;
		puzzle_free_cvec(&context, &cvec2);
	}
	ANNOTATE_SITE_END();

    puzzle_free_cvec(&context, &cvec1);
    puzzle_free_context(&context);
	end = clock();

	cout << "computation time: " << (double)(end - start) / CLOCKS_PER_SEC << endl;
    return 0;
}
