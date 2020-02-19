typedef string str_t<255>;


struct input_string_data {
  string input_data<255>;
};

typedef struct input_string_data input_string_data;

program HELLOPROG {
  version HELLOVERS {
      string HELLO(string) = 1;
  } = 1; 
} = 12345;