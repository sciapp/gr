#include "grm/dom_render/ManageCustomColorIndex.hxx"
#include "gr.h"
#include "grm/dom_render/render.hxx"

void ManageCustomColorIndex::saveState()
{
  int rgb;
  gr_inqcolor(PLOT_CUSTOM_COLOR_INDEX, &rgb);
  this->state.push(rgb);
}

void ManageCustomColorIndex::restoreState()
{
  if (this->state.size() > 0)
    {
      int rgb = this->state.top();
      this->state.pop();

      double red = ((rgb)&0xFF) / 255.0;
      double green = ((rgb >> 8) & 0xFF) / 255.0;
      double blue = ((rgb >> 16) & 0xFF) / 255.0;

      gr_setcolorrep(PLOT_CUSTOM_COLOR_INDEX, red, green, blue);
    }
  else
    {
      printf("Tried to restore non existing customColorIndex state!\n");
    }
}

void ManageCustomColorIndex::saveContext(int context_id)
{
  int rgb;
  gr_inqcolor(PLOT_CUSTOM_COLOR_INDEX, &rgb);
  this->context[context_id] = rgb;
}

void ManageCustomColorIndex::selectContext(int context_id)
{
  if (auto search = context.find(context_id); search != context.end())
    {
      int rgb = search->second;

      double red = ((rgb)&0xFF) / 255.0;
      double green = ((rgb >> 8) & 0xFF) / 255.0;
      double blue = ((rgb >> 16) & 0xFF) / 255.0;

      gr_setcolorrep(PLOT_CUSTOM_COLOR_INDEX, red, green, blue);
    }
  else
    {
      printf("Invalid context id\n");
    }
}