#ifndef GRM_ARGS_T_WRAPPER_H
#define GRM_ARGS_T_WRAPPER_H
#include <QObject>
#include "grm.h"


class grm_args_t_wrapper : public QObject
{
  Q_OBJECT
  grm_args_t *grm_args_t_object;

public:
  grm_args_t_wrapper();
  grm_args_t_wrapper(const grm_args_t_wrapper &);
  grm_args_t *get_wrapper();
  void set_wrapper(_grm_args_t *);
};

#endif // GRM_ARGS_T_WRAPPER_H
