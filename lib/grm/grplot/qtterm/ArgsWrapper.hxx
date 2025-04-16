#ifndef GRM_ARGS_T_WRAPPER_H
#define GRM_ARGS_T_WRAPPER_H
#include <QObject>
#include "grm.h"


class ArgsWrapper : public QObject
{
  Q_OBJECT
  grm_args_t *grm_args_t_object;

public:
  ArgsWrapper();
  ArgsWrapper(const ArgsWrapper &);
  grm_args_t *getWrapper();
  void setWrapper(grm_args_t *);
};

#endif // GRM_ARGS_T_WRAPPER_H
