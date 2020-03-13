#include "glquake.hpp"

extern "C"
{
  int scr_width;
  int scr_height;

  void Sys_InitParms(int argc, char **argv);
  void Sys_RenderFrame();
}

//////////////////////////////////////////////////////////////////////////////
void GLQuake::render(QSize const& size)
{
  scr_width = size.width();
  scr_height = size.height();

  if (!inited_)
  {
    inited_ = true;

    auto sl(QCoreApplication::arguments());

    QScopedArrayPointer<QByteArray> bal(new QByteArray[sl.size()]);
    QScopedArrayPointer<char*> argv(new char*[sl.size()]);

    for (decltype(sl.size()) i{}, e(sl.size()); i != e; ++i)
    {
      bal[i] = sl[i].toUtf8();
      argv[i] = bal[i].data();
    }

    Sys_InitParms(sl.size(), argv.data());
  }

  // we render a quake frame
  Sys_RenderFrame();
}
