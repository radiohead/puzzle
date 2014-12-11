extern "C" {
  #include "puzzle_common.h"
  #include "puzzle.h"
  #include <time.h>
}

#include "pgetopt.hpp"
#include "listdir.hpp"
#include "cilk/cilk.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

typedef struct Opts_ 
{
	std::vector <std::string> directory;
	const char *inputFile;
    const char *outputFile;
    const char *dirName;
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

    opts->similarity_threshold = PUZZLE_CVEC_SIMILARITY_THRESHOLD;

    if (argc < 3 || argc > 5)
    {
        usage();
        return 0;
    }
    // jumping first call argument (program name)
    *argv++;

    if (!strcmp(argv[0], "-o")) {
    	// jump to file name
    	*argv ++;
    	opts->outputFile = *argv++;
    }
    else {
    	opts->outputFile = "";
    }

    opts->inputFile = *argv++;
    opts->dirName = *argv;
    // filling the vector inside opts struct with found files in directory
    listDir(opts->dirName, opts->directory);

    return 1;
}

std::pair<double, string> compare_image_files(PuzzleContext puzzle_context, PuzzleCvec original_vector, PuzzleCvec comparison_vector, string file_name)
{
  double distance = 10;

  puzzle_init_cvec(&puzzle_context, &comparison_vector);

  if (puzzle_fill_cvec_from_file(&puzzle_context, &comparison_vector, file_name.c_str())) {
    cerr << "Unable to read comparison file: " << file_name << endl;
  }
  else {
    distance = puzzle_vector_normalized_distance(&puzzle_context, &original_vector, &comparison_vector, 1);
  }

  puzzle_free_cvec(&puzzle_context, &comparison_vector);

  return std::pair<double, std::string>(distance, file_name);
}

void print_results(ostream &output,multimap<double, std::string, std::less<double> > &results,Opts &opts)
{
	unsigned int count = 0;
	std::multimap <double, std::string>::iterator it;

	output << endl << "*** Pictures found to be similar to "<< opts.inputFile << " ***" << endl;

	for (it = results.begin(); it != results.end(); ++it)
	{
	  if (count > 10) 
		  break;
	  output << (*it).first << ' ' << (*it).second << endl;
	  ++count;
	}

	count = 0;
	output << endl << "*** Pictures found to be identical/close resemblance to "<< opts.inputFile << " ***" << endl;

	for (it = results.begin(); it != results.end(); ++it) 
	{
	  if (count > 10)
		  break;
	  if ((*it).first <= 0.12) 
	  {
		output << (*it).first << ' ' << (*it).second << endl;
		++count;
	  }
	}
}

int main(int argc, char *argv[])
{
    Opts opts;
    PuzzleContext puzzle_context;
    PuzzleCvec original_vector, comparison_vector;
    std::multimap <double, std::string, std::less<double> > results;
	ofstream of;

    puzzle_init_context(&puzzle_context);

    if (!parse_opts(&opts, &puzzle_context, argc, argv)) {
        exit(EXIT_FAILURE);
    }

	puzzle_init_cvec(&puzzle_context, &original_vector);

	if (puzzle_fill_cvec_from_file(&puzzle_context, &original_vector, opts.inputFile)) {
		cerr << "Unable to read input file: " << opts.inputFile << endl;
		exit(EXIT_FAILURE);
	}

	cilk_for (unsigned int i = 0; i < opts.directory.size(); ++i) {
		std::string file_name = opts.directory[i];
		std::pair<double, std::string> comparison_result = compare_image_files(puzzle_context, original_vector, comparison_vector, file_name);
	    results.insert(comparison_result);
	}

	puzzle_free_cvec(&puzzle_context, &original_vector);
	puzzle_free_context(&puzzle_context);
	
	// select output depending on execution arguments
	if (opts.outputFile != "")
	{
		of.open(opts.outputFile, ios::out);
		print_results(of, results, opts);
		of.close();
	}
	else {
		print_results(cout, results, opts);
	}

	return 0;
}
