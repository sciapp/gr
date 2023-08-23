#include "grm/dom_render/ManageCustomColorIndex.hxx"
#include "gr.h"
#include "grm/dom_render/render.hxx"

void ManageCustomColorIndex::savestate()
{
  int rgb;
  gr_inqcolor(PLOT_CUSTOM_COLOR_INDEX, &rgb);
  this->state.push(rgb);
}

void ManageCustomColorIndex::restorestate()
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

void ManageCustomColorIndex::savecontext(int contextId)
{
  int rgb;
  gr_inqcolor(PLOT_CUSTOM_COLOR_INDEX, &rgb);
  this->context[contextId] = rgb;
}

void ManageCustomColorIndex::selectcontext(int contextId)
{
  if (auto search = context.find(contextId); search != context.end())
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