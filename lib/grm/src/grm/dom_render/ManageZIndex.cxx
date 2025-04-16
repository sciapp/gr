#include "grm/dom_render/ManageZIndex.hxx"
#include <stdio.h>

int ManageZIndex::getZIndex()
{
  return this->current_z_index;
}

void ManageZIndex::setZIndex(int z_index)
{
  this->current_z_index = z_index;
}

void ManageZIndex::saveState()
{
  this->state.push(this->current_z_index);
}

void ManageZIndex::restoreState()
{
  if (this->state.size() > 0)
    {
      this->current_z_index = this->state.top();
      this->state.pop();
    }
  else
    {
      printf("Tried to restore non existing zindex state!\n");
    }
}
