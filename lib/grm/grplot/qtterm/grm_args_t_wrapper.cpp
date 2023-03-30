#include "grm_args_t_wrapper.h"

void grm_args_t_wrapper::set_wrapper(grm_args_t *args)
{
  this->grm_args_t_object = args;
}

grm_args_t *grm_args_t_wrapper::get_wrapper()
{
  return this->grm_args_t_object;
}

grm_args_t_wrapper::grm_args_t_wrapper() : QObject()
{
  this->grm_args_t_object = nullptr;
}

grm_args_t_wrapper::grm_args_t_wrapper(const grm_args_t_wrapper &arg)
{
  this->grm_args_t_object = arg.grm_args_t_object;
}
