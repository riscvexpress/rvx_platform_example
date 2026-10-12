#include <string.h>

#include "ervp_printf.h"
#include "parser.h"

void parse_net_option_list(texpar_list_t *option_list);
int is_network(texpar_section_t *s);
void parse_conv(texpar_list_t *option_list);
void parse_fc(texpar_list_t *option_list);
void parse_neuron(texpar_list_t *option_list);

void parse_network_cfg(char *filename)
{
  texpar_list_t *section_list = texpar_read_file_with_section(filename);
  texpar_iter_t *n = section_list->front;
  if(!n) {
    printf("Config file has no section_list");
    return;
  }
  /* net info parsing */
  texpar_section_t *s = (texpar_section_t *)n->val;
  texpar_list_t *option_list = s->option_list;

  if(!is_network(s)) {
    printf("First texpar_section_t must be [net] or [network]");
    return;
  }
  parse_net_option_list(option_list);

  /* layers info parsing */
  n = n->next;  // next texpar_section_t texpar_iter_t
  int count = 0;
  texpar_free_section(s);
  while(n){
    s = (texpar_section_t *)n->val; // texpar_section_t
    option_list = s->option_list; // option texpar_list_t
    printf("\nlayer %2d: %s\n", count, s->type);
    // option texpar_list_t parsing
    if (strcmp(s->type, "[conv]")==0) {
      parse_conv(option_list);
    }
    else if (strcmp(s->type, "[fc]")==0) {
      parse_fc(option_list);
    }
    texpar_unused(option_list);

    texpar_free_section(s);
    n = n->next;
    count++;
  }
  free_list(section_list);
}

void parse_net_option_list(texpar_list_t *option_list)
{
  char *app_name = texpar_find_str(option_list, "app_name", "mnist_l2ff");
  printf("app_name: %s\n", app_name);
  char *neuron_type = texpar_find_str(option_list, "neuron_type", "q6ssf");
  printf("neuron_type: %s\n", neuron_type);
  int height = texpar_find_int_quiet(option_list, "height", 14);
  printf("height: %d\n", height);
  int width = texpar_find_int_quiet(option_list, "width", 14);
  printf("width: %d\n", width);
  int channels = texpar_find_int_quiet(option_list, "channels", 1);
  printf("channels: %d\n", channels);
}

int is_network(texpar_section_t *s)
{
    return (strcmp(s->type, "[net]")==0
            || strcmp(s->type, "[network]")==0);
}

void parse_conv(texpar_list_t *option_list){
  int input_height = texpar_find_int_quiet(option_list, "input_height", 14);
  printf("input_height: %d\n", input_height);
  int input_width = texpar_find_int_quiet(option_list, "input_width", 14);
  printf("input_width: %d\n", input_width);
  int input_channels = texpar_find_int_quiet(option_list, "input_channels", 1);
  printf("input_channels: %d\n", input_channels);
  int output_channels = texpar_find_int_quiet(option_list, "output_channels", 1);
  printf("output_channels: %d\n", output_channels);
  int kernel_size = texpar_find_int_quiet(option_list, "kernel_size", 5);
  printf("kernel_size: %d\n", kernel_size);
  parse_neuron(option_list);
}

void parse_fc(texpar_list_t *option_list){
  int input = texpar_find_int_quiet(option_list, "input", 200);
  printf("input: %d\n", input);
  int output = texpar_find_int_quiet(option_list, "output", 10);
  printf("output: %d\n", output);
  parse_neuron(option_list);
}

void parse_neuron(texpar_list_t *option_list){
  char *neuron_model = texpar_find_str(option_list, "neuron_model", "lif");
  printf("neuron_model: %s\n", neuron_model);
  float neuron_beta = texpar_find_float_quiet(option_list, "neuron_beta", 1.0f);
  printf("neuron_beta: %.1f\n", neuron_beta);
  float neuron_threshold = texpar_find_float_quiet(option_list, "neuron_threshold", 1.0f);
  printf("neuron_threshold: %.1f\n", neuron_threshold);
}
