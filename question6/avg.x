/*
 * The average procedure receives an array of real
 * numbers and returns the average of their
 * values. This toy service handles a maximum of
 * 200 numbers.
 * http://www.linuxjournal.com/article/2204?page=0,1
 */

const MAXAVGSIZE  = 100;
typedef double numarr<100>;
struct input_data 
  {
  double input_data<100>;
  };

struct output_data {
  double output_data<100>;
};


typedef struct input_data input_data;
typedef struct output_data output_data;

program AVERAGEPROG {
    version AVERAGEVERS {
        string AVERAGE(input_data) = 1;
    } = 1;
} = 31245;

