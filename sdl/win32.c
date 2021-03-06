
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#define WANT_WMINFO
#include <SDL.h>
#include <SDL_syswm.h>

// The amiga wrapper and the windows header don't get along...
typedef Uint32   uint32;
typedef Sint32   int32;
typedef char     TEXT;

int32 gui_req( uint32 img, TEXT *title, TEXT *reqtxt, TEXT *buttons )
{
  SDL_SysWMinfo wmi;
  HWND hwnd;

  hwnd = NULL;
  SDL_VERSION(&wmi.version);
  if( SDL_GetWMInfo( &wmi ) )
    hwnd = (HWND)wmi.window;

  return MessageBoxA(hwnd, reqtxt, title, strcmp(buttons, "OK") ? MB_OKCANCEL : MB_OK ) == IDOK;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (((LPARAM)NULL) != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}

BOOL directoryrequester( char *title, char *path )
{
  SDL_SysWMinfo wmi;
  HWND hwnd;
	LPITEMIDLIST pidl     = NULL;
	BROWSEINFO   bi       = { 0 };
	BOOL         bResult  = FALSE;
  static BOOL coinitialised = FALSE;
  static char tmp[MAX_PATH];

  GetFullPathName(path, MAX_PATH, tmp, NULL);

  if (!coinitialised)
  {
    CoInitialize(NULL);
    coinitialised = TRUE;
  }

  hwnd = NULL;
  SDL_VERSION(&wmi.version);
  if( SDL_GetWMInfo( &wmi ) )
    hwnd = (HWND)wmi.window;

	bi.hwndOwner      = hwnd;
	bi.pszDisplayName = tmp;
	bi.pidlRoot       = NULL;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = (LPARAM) tmp;

  if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		bResult = SHGetPathFromIDList(pidl, tmp);
    CoTaskMemFree(pidl);
    if (bResult)
    {
      strncpy(path, tmp, 512);
      path[511] = 0;
      SDL_WM_SetCaption(path, path);
    }
    else
    {
      SDL_WM_SetCaption("No", "No");
    }
	}
  else
  {
    SDL_WM_SetCaption("No 2", "No 2");
  }

	return bResult;
}

#define FR_HVLSAVE 0
#define FR_AHXSAVE 1
#define FR_INSSAVE 2
#define FR_MODLOAD 3
#define FR_INSLOAD 4

char *filerequester( char *title, char *path, char *fname, int type )
{
  SDL_SysWMinfo wmi;
  static OPENFILENAME ofn;
  HWND hwnd;
  char *result;
  char *odir;
  char tmp[4096];
  
  strcpy(tmp, fname);

  hwnd = NULL;
  SDL_VERSION(&wmi.version);
  if( SDL_GetWMInfo( &wmi ) )
    hwnd = (HWND)wmi.window;

  ZeroMemory( &ofn, sizeof( ofn ) );
  ofn.lStructSize = sizeof( ofn );
  ofn.hwndOwner   = hwnd;
  ofn.nMaxFile    = 4096;
  ofn.lpstrFile   = tmp;

  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  switch( type )
  {
    case FR_HVLSAVE:
      ofn.Flags = OFN_PATHMUSTEXIST;
      ofn.lpstrFilter = "All Files\0*.*\0HVL module (*.hvl)\0*.HVL\0";
      ofn.nFilterIndex = 2;
      break;
    
    case FR_AHXSAVE:
      ofn.Flags = OFN_PATHMUSTEXIST;
      ofn.lpstrFilter = "All Files\0*.*\0AHX module (*.ahx)\0*.AHX\0";
      ofn.nFilterIndex = 2;
      break;

    case FR_INSSAVE:
      ofn.Flags = OFN_PATHMUSTEXIST;
      ofn.lpstrFilter = "All Files\0*.*\0Instrument (*.ins)\0*.INS\0";
      ofn.nFilterIndex = 2;
      break;

    case FR_MODLOAD:
      ofn.lpstrFilter = "All Files\0*.*\0Modules (*.ahx, *.hvl, *.mod)\0*.AHX;*.HVL;*.MOD;HVL.*;AHX.*;MOD.*\0";
      ofn.nFilterIndex = 2;
      break;

    case FR_INSLOAD:
      ofn.lpstrFilter = "All Files\0*.*\0Instrument (*.ins)\0*.INS;INS.*\0";
      ofn.nFilterIndex = 2;
      break;

    default:
      ofn.lpstrFilter = "All Files\0*.*\0";
      ofn.nFilterIndex = 1;
      break;
  }

  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = path;

  odir = getcwd( NULL, 0 );

  switch( type )
  {
    case FR_HVLSAVE:
    case FR_AHXSAVE:
    case FR_INSSAVE:
      if( !GetSaveFileName( &ofn ) )
      {
        chdir( odir );
        free( odir );
        return NULL;
      }
      break;
    
    default:
      if( !GetOpenFileName( &ofn ) )
      {
        chdir( odir );
        free( odir );
        return NULL;
      }
      break;
  }

  chdir( odir );
  free( odir );

  if (strlen(ofn.lpstrFile)==0) return NULL;

  result = malloc(strlen(ofn.lpstrFile)+1);
  if (result)
  {
    strcpy(result, ofn.lpstrFile);
  }

  return result;
}
