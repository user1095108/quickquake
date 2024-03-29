#include "glquake.hpp"
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_PAUSE			255

#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202

#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238

#define K_MWHEELUP		239
#define K_MWHEELDOWN	240

extern "C"
{
  extern int scr_width;
  extern int scr_height;

  void Sys_InitParms(int argc, char **argv);
  void Sys_RenderFrame();

  std::pair<int, bool> keys[64];
  unsigned kr{};
  unsigned kw{};

  void Key_Event(int, int);

  void Sys_SendKeyEvents()
  {
    while (kr != kw)
    {
      auto const& p(keys[kr++]);
      kr &= std::size(keys) - 1;

      Key_Event(p.first, p.second);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
void GLQuake::keyEvent(int const key, bool const pressed)
{
  int kk{-1};

  switch (key)
  {
    //
    case Qt::Key_Tab:
      kk = K_TAB;
      break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
      kk = K_ENTER;
      break;

    case Qt::Key_Escape:
      kk = K_ESCAPE;
      break;

    case Qt::Key_Space:
      kk = K_SPACE;
      break;

    //
    case Qt::Key_Backspace:
      kk = K_BACKSPACE;
      break;

    case Qt::Key_Left:
      kk = K_LEFTARROW;
      break;

    case Qt::Key_Up:
      kk = K_UPARROW;
      break;

    case Qt::Key_Right:
      kk = K_RIGHTARROW;
      break;

    case Qt::Key_Down:
      kk = K_DOWNARROW;
      break;

    //
    case Qt::Key_Alt:
      kk = K_ALT;
      break;

    case Qt::Key_Control:
      kk = K_CTRL;
      break;

    case Qt::Key_Shift:
      kk = K_SHIFT;
      break;

    //
    case Qt::Key_Insert:
      kk = K_INS;
      break;

    case Qt::Key_Delete:
      kk = K_DEL;
      break;

    case Qt::Key_PageDown:
      kk = K_PGDN;
      break;

    case Qt::Key_PageUp:
      kk = K_PGUP;
      break;

    case Qt::Key_Home:
      kk = K_HOME;
      break;

    case Qt::Key_End:
      kk = K_END;
      break;

    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12:
      kk = K_F1 + (key - Qt::Key_F1);
      break;

    //
    case Qt::Key_Dead_Tilde:
      kk = '~';
      break;

    default:
      if ((key >= Qt::Key_Space) && (key <= Qt::Key_AsciiTilde))
      {
        kk = key;
      }
      break;
  }

  if (-1 != kk)
  {
    keys[kw++] = {kk, pressed};
    kw &= std::size(keys) - 1;
  }
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
