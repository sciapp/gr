#include "ArgsWrapper.hxx"

void ArgsWrapper::setWrapper(grm_args_t *args)
{
  this->grm_args_t_object = args;
}

grm_args_t *ArgsWrapper::getWrapper()
{
  return this->grm_args_t_object;
}

ArgsWrapper::ArgsWrapper() : QObject()
{
  this->grm_args_t_object = nullptr;
}

ArgsWrapper::ArgsWrapper(const ArgsWrapper &arg)
{
  this->grm_args_t_object = arg.grm_args_t_object;
}
