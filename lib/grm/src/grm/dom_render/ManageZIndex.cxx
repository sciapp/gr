#include "grm/dom_render/ManageZIndex.hxx"

int ManageZIndex::getZIndex()
{
  return this->currentZIndex;
}

void ManageZIndex::setZIndex(int zIndex)
{
  this->currentZIndex = zIndex;
}

void ManageZIndex::savestate()
{
  this->state.push(this->currentZIndex);
}

void ManageZIndex::restorestate()
{
  if (this->state.size() > 0)
    {
      this->currentZIndex = this->state.top();
      this->state.pop();
    }
  else
    {
      printf("Tried to restore non existing zindex state!\n");
    }
}
