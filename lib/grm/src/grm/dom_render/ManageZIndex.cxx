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
  this->currentZIndex = this->state.top();
  this->state.pop();
}
